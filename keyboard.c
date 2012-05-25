/**
 * File:	keyboard.c
 * Author:	Grant Kurtz
 *
 * Description:
 *		Handles processing PS/2 Keyboard input.  This module specifically only
 *		deals with buffered-blocking input.
 *
 */

#include "headers.h"
#include "ps2.h"
#include "system.h"
#include "startup.h"
#include "queues.h"
#include "pcbs.h"
#include "scheduler.h"
#include "ulib.h"
#include "win_man.h"
#include "keyboard.h"
#include "vmemL2.h"
#include "vmem_isr.h"
#include "gl_print.h"

// Struct to handle IO-Request information
typedef struct ps2_io_req{
	Uint32 pdt;
	char *buf;
	int size;
	int index;
	Pid pid;
} ps2_io_req;

// Array of IO Requests currently outstanding
static ps2_io_req *requests [ TOTAL_IO_REQS ];
//static Pcb *pcb_reqs [ TOTAL_IO_REQS ];

// Buffered-Blocking Queue
Queue *_buf_block;

// Scan Code Set #1
// (copied from c_io.c)
unsigned char _ps2_scan_code[ 2 ][ 128 ] = {
	{
/* 00-07 */	'\377',	'\033',	'1',	'2',	'3',	'4',	'5',	'6',
/* 08-0f */	'7',	'8',	'9',	'0',	'-',	'=',	'\b',	'\t',
/* 10-17 */	'q',	'w',	'e',	'r',	't',	'y',	'u',	'i',
/* 18-1f */	'o',	'p',	'[',	']',	'\n',	'\377',	'a',	's',
/* 20-27 */	'd',	'f',	'g',	'h',	'j',	'k',	'l',	';',
/* 28-2f */	'\'',	'`',	'\377',	'\\',	'z',	'x',	'c',	'v',
/* 30-37 */	'b',	'n',	'm',	',',	'.',	'/',	'\377',	'*',
/* 38-3f */	'\377',	' ',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',
/* 40-47 */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'7',
/* 48-4f */	'8',	'9',	'-',	'4',	'5',	'6',	'+',	'1',
/* 50-57 */	'2',	'3',	'0',	'.',	'\377',	'\377',	'\377',	'\377',
/* 58-5f */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',
/* 60-67 */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',
/* 68-6f */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',
/* 70-77 */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',
/* 78-7f */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377'
	},

	{
/* 00-07 */	'\377',	'\033',	'!',	'@',	'#',	'$',	'%',	'^',
/* 08-0f */	'&',	'*',	'(',	')',	'_',	'+',	'\b',	'\t',
/* 10-17 */	'Q',	'W',	'E',	'R',	'T',	'Y',	'U',	'I',
/* 18-1f */	'O',	'P',	'{',	'}',	'\n',	'\377',	'A',	'S',
/* 20-27 */	'D',	'F',	'G',	'H',	'J',	'K',	'L',	':',
/* 28-2f */	'"',	'~',	'\377',	'|',	'Z',	'X',	'C',	'V',
/* 30-37 */	'B',	'N',	'M',	'<',	'>',	'?',	'\377',	'*',
/* 38-3f */	'\377',	' ',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',
/* 40-47 */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'7',
/* 48-4f */	'8',	'9',	'-',	'4',	'5',	'6',	'+',	'1',
/* 50-57 */	'2',	'3',	'0',	'.',	'\377',	'\377',	'\377',	'\377',
/* 58-5f */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',
/* 60-67 */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',
/* 68-6f */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',
/* 70-77 */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',
/* 78-7f */	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377',	'\377'
	}
};

// Key Modifiers
static Uint8 shift_pressed = 0;
static char ctrl_pressed = 0;
static char alt_pressed = 0;
//static char num_lock = 1;
static char caps_lock = 0;
//static char scroll_lock = 0;
static char win_pressed = 0;

// Used to await another byte for an extended key
static char await_next = 0;


/**
 * Just prepares the isr-callback and IO-request array.
 */
void _ps2_keyboard_init( void ){

	// temp vars
	Status status;

	// For simplicity, null-out the io-requests
	ps2_io_req *req;
	int i;
	for( i = 0; i < TOTAL_IO_REQS; i++){
		req = requests[i];
		req->pid = 0;
		req->buf = 0;
		req->size = -1;
		req->index = -1;
	}

	// Create the Buffered-Blocking Process Queue
	status = _q_alloc( &_buf_block, NULL );
	if( status != SUCCESS ) {
		prt_status( "keyboard, init: Unable to create buf_block queue for IO"
					" requests, keyboard input disabled!\n Error: %s\n", 
					status);
	}


	// install our ISR
	__install_isr( PS2_K_VEC, _ps2_keyboard_isr );
}


/**
 * Handles processing of interrupts from the PS/2 controller.
 *
 * @param	vec			The number of the interrupt code that triggered this
 *						call.
 */
void _ps2_keyboard_isr( int vec, int code ){
	
	// temp vars
	Uint quad;

	Uint key = __inb( PS2_PORT );

	// Handle extended keys first
	if( await_next ){
		if( key == 0xFE ){
			_ps2_ack_int();
			return;
		}
		else{

			// Check if we are going to perform an active window switch
			if( win_pressed ){
				quad = get_active();

				// Determine offset arrow-keys to change active window
				switch( key ){
					case PS2_KEY_UP_P:
						if( quad > 1 )
							quad -= 2;
						break;
					case PS2_KEY_DOWN_P:
						if( quad < 2 )
							quad += 2;
						break;
					case PS2_KEY_LEFT_P:
						if( quad % 2 == 1 )
							quad -= 1;
						break;
					case PS2_KEY_RIGHT_P:
						if( quad % 2 == 0 )
							quad += 1;
				}
				switch_active( quad );
			}

			// resolve the extended key
			switch( key ){
				case PS2_KEY_LWIN_P:
				case PS2_KEY_RWIN_P:
					win_pressed = 1;
					break;
				case PS2_KEY_CTRL_P:
					ctrl_pressed = 1;
					break;
				case PS2_KEY_CTRL_R:
					ctrl_pressed = 0;
					break;
				case PS2_KEY_ALT_P:
					alt_pressed = 1;
					break;
				case PS2_KEY_ALT_R:
					alt_pressed = 0;
					break;
			}
			await_next = 0;
		}
	}

	// Next, handle special keys
	switch( key ){
		case PS2_KEY_LSHIFT_P:
		case PS2_KEY_RSHIFT_P:
			shift_pressed = 1;
			return;
		case PS2_KEY_LSHIFT_R:
		case PS2_KEY_RSHIFT_R:
			shift_pressed = 0;
			return;
		case PS2_KEY_CAPLCK_P:
			caps_lock = 1;
			return;
		case PS2_KEY_CAPLCK_R:
			caps_lock = 0;
			return;
		case PS2_KEY_CTRL_P:
			ctrl_pressed = 1;
			return;
		case PS2_KEY_ALT_P:
			alt_pressed = 1;
			return;
		case PS2_KEY_ALT_R:
			alt_pressed = 0;
			return;
	}

	// normal ASCII characters
	if( key < 0x80 ){

		// check if function-key
		if( key >= PS2_KEY_F1_P && key <= PS2_KEY_F10_P ){
			replace_active( key - PS2_KEY_F1_P );
		}
		else if( key == PS2_KEY_F11_P || key == PS2_KEY_F12_P ){
			replace_active( key - PS2_KEY_F11_P + 10 );
		}
		else{

			// otherwise give the character to the waiting focused process
			char letter = _ps2_scan_code[ shift_pressed ][ key ];
			if( letter != '\377' && letter != '\033' && letter != '\b'
					&& letter != '\t'){
				_ps2_write_to_active( letter );
			}
		}
	}

	// Just the ASCII characters being released
	else if( key >= 0x80 && key <= 0xD8){
		// key released!
	}
	else{
		
		// If we get this byte then that means we should expect an extended key
		if( key == 0xE0 ){
			await_next = 1;
			__outb( PS2_PORT, 0x0 );
		}
	}
	
	_ps2_ack_int();
}

/**
 * A conveinence function for telling the PIC_SLAVE and PIC_MASTER that we have
 * serviced the interrupt.
 */
void _ps2_ack_int( void ){
	__outb( 0xA0, 0x20 );
	__outb( 0x20, 0x20 );
}

/**
 * Attempts to find an unused IO-Request block.
 *
 * @returns				-1 if an unused block could not be found, otherwise
 *						a positive value up to (but not including)
 *						TOTAL_IO_REQS is returned.
 */
Uint8 _ps2_get_io_req( void ){
	int index = -1;
	int i;
	for( i = 0; i < TOTAL_IO_REQS; i++){
		if( requests[i]->index == -1 ){
			index = i;
			break;
		}
	}
	return index;
}

/**
 * Writes the given character to the focused process, if one exists. Otherwise
 * the character is discarded.
 * 
 * @param	c		The character to write
 */
void _ps2_write_to_active( char c ){

	// Grab focused process
	Pid active_p = get_active_pid();

	// Throw away the character if there is no focused process
	if( active_p == 0 ){
		return;
	}


	// Find the IO-request for this process
	int index = 0;
	while( index < TOTAL_IO_REQS ){
		if( requests[ index ]->pid == active_p )
			break;
		index ++;
	}
	if( index == TOTAL_IO_REQS )
		return; // The focused process does not want input
	
	// Write the character
	// Check if we need to return a special character, or write to buf
	//c_printf( "Index: %d    Size: %d\n", requests[ index ]->index, requests[ index ]->size);
	char *buf = requests[ index ]->buf;
	Uint8 flags = 0;
	//c_printf(" Writing Buf Addr: 0x%x\n", buf);
	Uint32 currentCr3 = _isr_vmem_getcr3();
	_vmeml2_change_page(requests[ index ]->pdt);
	//c_printf(" Writing Buf Addr: 0x%x 0x%x\n", buf, buf+sizeof(char));
	if( requests[ index ]->size == 0 ){
		buf[1] = c;
		if( shift_pressed )
			flags = flags | 4;
		if( alt_pressed )
			flags = flags | 2;
		if( ctrl_pressed )
			flags = flags | 1;
		buf[0] = (char) flags;
		Pcb *pcb = _ps2_remove_from_queue( index );
		_ps2_delete_request( index );
		_sched( pcb );
	}
	else{
		// We need to print the character for the user
        screen_info* si = get_screen_info( active_p );
        gl_putchar_s( c, si );

		// store the character
		int i = requests[ index ]->index;
		if( c != '\n'){
			buf[ i ] = c;
			requests[ index ]->index = i + 1;
		}
		
		// stop reading if full, or newline
		if( i == requests[ index ]->size || c == '\n' ){
			buf[ i ] = '\0';
			
			// pull from IO-blocking queue
			if( !_q_empty( _buf_block ) ){
				Pcb *pcb = _ps2_remove_from_queue( index );
				_sched( pcb );
			}
			else{
				// did someone forcefully remove it!?!
				c_printf( "keyboard, write active: buffered block queue is"
							" empty???\n" );
			}
			
			// Delete the process from our IO request array
			_ps2_delete_request( index );
		}
	}
	_vmeml2_change_page( currentCr3);
}

/**
 * Frees the IO request.
 *
 * @param	index		The index of the request to remove from the requests
 *						array.
 */
void _ps2_delete_request( Uint8 index ){
	if( index >= TOTAL_IO_REQS )
		return;
	requests[ index ]->pid = 0;
	requests[ index ]->buf = 0;
	requests[ index ]->size = -1;
	requests[ index ]->index = -1;
}

/**
 * Removes the PCB from the buf_block queue. Note, the PCB of the process is
 * still returned even if the PCB could not be removed from the buf_block
 * queue, but the PCB may be 0 if the corresponding process could not be found.
 *
 * @param	index		The index in the requests array of the process to 
 *						remove.
 * returns				The PCB of the process that was removed, or 0 if the
 *						index was too large. Note, the PCB returned may be 0
 *						if the corresponding process could not be found.
 */
Pcb *_ps2_remove_from_queue( Uint8 index ){
	if( index >= TOTAL_IO_REQS )
		return 0;
	void *data;
	Key key;
	key.u = requests[ index ]->pid;
	Status status = _q_remove_by_key( _buf_block, &data, key );
	Pcb *pcb = ( Pcb* ) data;
	if( status != SUCCESS ){
		prt_status( "keyboard, write active: Unable to remove process"
				" in buf block queue!\nError: %s\n", status);
	}
	return pcb;
}


/**
 * Performs a blocking buffered read for keyboard input.
 *
 * Keystrokes are stored in the buffer and will not stop filling the buffer
 * until 'size' characters has been read. If there is an error when processing
 * the read request, the buffer will be left untouched.  Otherwise undefined
 * behavior is defined for errors after the read request is processed. Note,
 * only user programs which have focus will recieve characters.
 *
 * @param	buf		The buffer to fill with character input from the keyboard.
 * @param	size	The number of characters to read.
 */
int buf_read( char* buf, int size, Pcb* cur ){
	
	// Create an IO-Request block
	int index = _ps2_get_io_req();
	if( index == -1 ){
		Key key;
		key.u = cur->pid;
		void *data;
		_q_remove_by_key( _buf_block, &data, key );
		_sched( cur );
		return 0;
	}

	// Initialize IO-request
	requests[index]->pid = cur->pid;
	requests[index]->pdt = (Uint32)cur->pdt;
	requests[index]->buf = buf;
	requests[index]->size = size;
	requests[index]->index = 0;

	return 1;
}

/**
 * Grabs the next available character, and also stores modifier key data at the
 * time of the key press.
 *
 * @param	pid		The process that made the request
 * @returns			1 if a proper IO-request was created, otherwise 0
 */
int char_read( char *buf, Pcb* pc ){
	return buf_read( buf, 0, pc );
}


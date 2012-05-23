/**
 *
 *
 */

#include "headers.h"
#include "startup.h"
#include "ps2.h"
#include "mouse.h"
#include "win_man.h"

// Globals
short _init;
int _x_move;
int _y_move;
int _x_t_move;
int _y_t_move;
int _x_pos;
int _y_pos;
char _left_button;
char _right_button;
char _middle_button;

/**
 * Source for init steps taken from the following forum.  
 * http://forum.osdev.org/viewtopic.php?t=10247
 */
void _ps2_mouse_init( void ){

	// Setup initial values
	_init = 0;
	_x_move = 0;
	_y_move = 0;
	_left_button = 0;
	_right_button = 0;
	_middle_button = 0;
	
	// vars
	Uint resp = 0;
	
	// Hook in our interrupt vector, we do this first because there are some
	// weird interrupt timing issues.
	__install_isr( PS2_M_VEC, _ps2_mouse_isr );

	// First, disable the mouse to avoid having it mess up initialization
	_ps2_mouse_clear();
	__outb( PS2_STAT, 0xAD );
	_ps2_mouse_clear();
	__outb( PS2_STAT, 0xA7 );

	// Clear the mouse's buffer
	__inb( 0x60 );

	// enable the PS/2 auxillary device
	_ps2_mouse_clear();
	__outb( PS2_STAT, 0xA8 );

	// Enable the interrupts
	_ps2_mouse_clear();
	__outb( PS2_STAT, 0x20 );
	_ps2_mouse_ready();
	resp = ( __inb(0x60) | 2 );
	_ps2_mouse_clear();
	__outb( PS2_STAT, 0x60 );
	_ps2_mouse_clear();
	__outb( 0x60, resp );

	// Tell the mouse to use default settings
	_ps2_write_mouse( 0xF6 );
	_ps2_read_mouse();  //Acknowledge

	/*
	// Override the Sample Rate to 200/s
	_ps2_write_mouse( PS2_M_SAMP );
	_ps2_read_mouse();
	c_puts( "ACK! Awaiting VALUE!\n" );
	_ps2_mouse_clear();
	__outb( PS2_STAT, 0xD4 );
	while( (__inb(PS2_STAT) & 2) != 0 )
		;
	__outb( PS2_PORT, 200 );
	//_ps2_read_mouse();
	
	// next, override the resolution to 8count/mm
	_ps2_write_mouse( PS2_M_SRES );
	_ps2_read_mouse();
	c_puts( "ACK! Awaiting VALUE!\n" );
	_ps2_mouse_clear();
	__outb( PS2_STAT, 0xD4 );
	while( (__inb(PS2_STAT) & 2) != 0 )
		;
	__outb( PS2_PORT, 0x03 );
	*/

	// Enable the mouse to begin data packet transfers
	_ps2_write_mouse( 0xF4 );
	_ps2_read_mouse();  //Acknowledge

	// Reset everything (settings)
	_ps2_mouse_clear();
	__outb( PS2_STAT, PS2_M_RST );
	
	// Done!
	c_puts( "Mouse driver successfully attached!\n" );
	c_clearscreen();
	_init = 1;

	// Draw mouse at center
	_x_pos = 0;
	_y_pos = 0;
}

void _ps2_mouse_isr( int vec, int code ){

	static int byte_c = 0;
	static char mouse_p = '+';
	static char m_bytes[3];

	// temp vars
	char button_change = 0;

	if(_init)
		m_bytes[byte_c++] = __inb(PS2_PORT);

	// Only dump information once we have everything the mouse sent
	if(byte_c == 3){
		if( (m_bytes[0] & 0x80) || (m_bytes[0] & 0x40) ){
			// overflow! throw away packet
			c_puts( "Mouse overflow!\n" );
		}
		else{

			// Check left-button status
			if( m_bytes[0] & 0x1 ){
				if( _left_button != 1)
					//c_puts( "Left Button Pressed!\n" );
				_left_button = 1;
				button_change = 1;
			}
			else if( _left_button ){
				//c_puts( "Left Button Released!\n" );
				_left_button = 0;
				button_change = 1;
			}
				
			// check right-button status
			if( m_bytes[0] & 0x2 ){
				if(_right_button != 1)
					//c_puts( "Right Button Pressed!\n" );
				_right_button = 1;
				button_change = 1;
			}
			else if( _right_button ){
				//c_puts( "Right Button Released!\n" );
				_right_button = 0;
				button_change = 1;
			}

			// check middle-button status
			if( m_bytes[0] & 0x4 ){
				if(_middle_button != 1)
					//c_puts( "Middle Button Pressed!\n" );
				_middle_button = 1;
				button_change = 1;
			}
			else if( _middle_button ){
				//c_puts( "Middle Button Released!\n" );
				_middle_button = 0;
				button_change = 1;
			}
			
			_x_move = m_bytes[1];
			if( (m_bytes[0] & 0x8) == 0 ){
				_x_move *= -1;
			}
			if( (m_bytes[0] & 0x20) != 0 ){
				_y_move *= -1;
			}
			_y_move = m_bytes[2];
			//c_printf( "X: %dmm\n", _x_move );
			//c_printf( "Y: %dmm\n", _y_move );
		}
		byte_c = 0;
	}

	// remove the mouse cursor at the previous location
	c_putchar_at(_x_pos, _y_pos, ' ');

	// limit total possible movement
	_x_move = _x_move > 80 ? 80 : _x_move;
	_x_move = _x_move < -80 ? -80 : _x_move;
	_y_move = _y_move > 64 ? 64 : _y_move;
	_y_move = _y_move < -64 ? -64 : _y_move;

	// move mouse cursor
	_x_t_move += ( _x_move / 2 );
	_y_t_move += ( _y_move / 6 );

	/*
	// apply a minimum bound of movement
	if( _x_move < 0 && _x_move > -16 )
		_x_t_move -= 1;
	else if( _x_move > 0 && _x_move < 16 )
		_x_t_move += 1;
	if( _y_move < 0 && _y_move > -16 )
		_y_t_move += 1;
	else if( _y_move > 0 && _y_move < 16 )
		_y_t_move -= 1;
	*/

	if(_x_t_move > 6){
		_x_pos++;
		_x_t_move = 0;
	}
	else if(_x_t_move < -6){
		_x_pos--;
		_x_t_move = 0;
	}
	if(_y_t_move > 8){
		_y_pos--;
		_y_t_move = 0;
	}
	else if(_y_t_move < -8){
		_y_pos++;
		_y_t_move = 0;
	}

	// enforce boundaries
	if( _x_pos > 79 )
		_x_pos = 79;
	if( _x_pos < 0 )
		_x_pos = 0;
	if( _y_pos > 24)
		_y_pos = 24;
	if( _y_pos < 0)
		_y_pos = 0;

	if(_left_button)
		mouse_p = 'x';
	else if(_right_button)
		mouse_p = '#';
	else if(_middle_button)
		mouse_p = '|';
	else
		mouse_p = '+';

	c_putchar_at( _x_pos, _y_pos, mouse_p );

	update_cursor_pos( _x_pos, _y_pos );
	if( button_change == 1 )
		update_mouse_button( _left_button, _right_button, _middle_button );

	// indicate that we have read the interrupt
	__outb( 0x20, 0x20 );
	__outb( 0xA0, 0x20 );
}

Uint _ps2_read_mouse(){	

	Uint ret_char;

	// Wait for the mouse to become ready
	_ps2_mouse_ready();
	ret_char = __inb(PS2_PORT);

	return ret_char;
}

void _ps2_write_mouse(char b){

	// vars
	//Uint resp;

	_ps2_mouse_clear();

	// let the PS2 controller know we want to send a command to the mouse
	//c_printf("Letting Mouse know we are sending a command...\n");
	__outb(PS2_STAT, PS2_M_SCOM);
	_ps2_mouse_clear();

	// Now, send the mouse our command
	//c_printf("Sending Command: 0x%x...\n", ( b & 0xFF) );
	__outb(PS2_PORT, b);
}

void _ps2_mouse_clear( void ){
	
	// vars
	char b;

	//c_printf("Waiting on Mouse to be clear...\n");
	// wait for the mouse to be clear for commands
	while( 1 ){
		b = __inb(PS2_STAT);
		//c_printf("CLEAR: Read a byte from Port PS2_STAT, 0x%x...\n", b);
		if( (b & 2) == 0 )
			return;
	}
}

Uint _ps2_mouse_ready( void ){
	
	// vars
	Uint b;

	//c_printf("Waiting on Mouse to be ready...\n");
	// wait for the mouse to be ready for commands
	while( 1 ){
		b = __inb(PS2_STAT);
		//c_printf("READY: Read a byte from Port PS2_STAT, 0x%x...\n", b);
		if( (b & 1) == 1 )
			return b;
	}
}

void _ps2_setup_isr( void ){
/*	
	_ps2_write_mouse(PS2_M_CMPQ);
	_ps2_write
	__install_isr(PS2_M_VEC, _ps2_mouse_isr);
*/
}

void _ps2_nonack( Uint resp ){
	c_printf( "Did not recieve ACK (0xFF) from MOUSE, instead got: '0x%x'.\n",
			resp );
	__panic( "YOU SHALL NOT CYCLE!!!\n" );
}

void _ps2_debug( const char * comp, Uint resp ){
	c_printf("DEBUG: %s, RCVD: '0x%x'\n", comp, resp);
}


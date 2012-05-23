#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

// Debug Print controls
#define DEBUG_G

// Constraints on Input
#define TOTAL_IO_REQS	12

// Special Keys
#define PS2_KEY_RELEASE		0x80
#define PS2_KEY_CTRL_P		0x1D
#define PS2_KEY_CTRL_R		PS2_KEY_CTRL_P + PS2_KEY_RELEASE
#define PS2_KEY_EXT			0xE0
#define PS2_KEY_LSHIFT_P	0x2A
#define PS2_KEY_RSHIFT_P	0x36
#define PS2_KEY_LSHIFT_R	PS2_KEY_LSHIFT_P + PS2_KEY_RELEASE
#define PS2_KEY_RSHIFT_R	PS2_KEY_RSHIFT_P + PS2_KEY_RELEASE
#define PS2_KEY_ALT_P		0x38
#define PS2_KEY_ALT_R		PS2_KEY_ALT_P + PS2_KEY_RELEASE
#define PS2_KEY_CAPLCK_P	0x3A
#define PS2_KEY_NUMLCK_P	0x45
#define PS2_KEY_SROLCK_P	0x46
#define PS2_KEY_CAPLCK_R	PS2_KEY_CAPLCK_P + PS2_KEY_RELEASE
#define PS2_KEY_NUMLCK_R	PS2_KEY_NUMLCK_P + PS2_KEY_RELEASE
#define PS2_KEY_SROLCK_R	PS2_KEY_SROLCK_P + PS2_KEY_RELEASE
#define PS2_KEY_F1_P		0x3B
#define PS2_KEY_F10_P		0x44
#define PS2_KEY_F11_P		0x57
#define PS2_KEY_F12_P		0x58
#define PS2_KEY_LWIN_P		0x5B
#define PS2_KEY_RWIN_P		0x5C
#define PS2_KEY_UP_P		0x48
#define PS2_KEY_LEFT_P		0x4B
#define PS2_KEY_RIGHT_P		0x4D
#define PS2_KEY_DOWN_P		0x50

// IRQs
#define PS2_K_VEC	0x21


#include "queues.h"
#include "pcbs.h"

extern Queue *_buf_block;

// Kernel Module Functions
void _ps2_keyboard_init( void );
void _ps2_keyboard_isr( int vec, int code );
Uint8 _ps2_get_io_req( void );
void _ps2_delete_request( Uint8 index );
int _ps2_search_pid( void *pid1, void *pid2);
void _ps2_write_to_active( char c );
void _ps2_ack_int( void );
Pcb *_ps2_remove_from_queue( Uint8 index );

// Open functions
int buf_read( char* buf, int size, Pcb* cur );
int char_read( char *buf, Pcb* pc );

#endif

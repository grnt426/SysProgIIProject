#ifndef MOUSE_H
#define MOUSE_H

// Command Set
// Source: http://www.computer-engineering.org/ps2mouse/
#define PS2_M_RST	0xFF
#define PS2_M_RESET				PS2_M_RST
#define PS2_M_RSND	0xFE
#define PS2_M_RESEND_LAST		PS2_M_RSND
#define PS2_M_SDEF	0xF6
#define PS2_M_DDR	0xF5
#define PS2_M_EDR	0xF4
#define PS2_M_SAMP	0xF3
#define PS2_M_SET_SAMPLE_RATE	PS2_M_SAMP
#define PS2_M_DEVI	0xF2
#define PS2_M_SREM	0xF0
#define PS2_M_SET_REMOTE_MODE	PS2_M_SREM
#define PS2_M_SWRP	0xEE
#define PS2_M_SET_WRAP_MODE		PS2_M_SWRP
#define PS2_M_RWRP	0xFA
#define PS2_M_RDAT	0xEB
#define PS2_M_STRM	0xEA
#define PS2_M_STAT	0xE9
#define PS2_M_SRES	0xE8
#define	PS2_M_SET_RESOLUTION	PS2_M_SRES
#define PS2_M_SCA2	0xE7
#define PS2_M_SET_SCALING_2		PS2_M_SCA2
#define PS2_M_SCA1	0xE6
#define PS2_M_SET_SCALING_1		PS2_M_SCA1
#define PS2_M_SCOM	0xD4
#define PS2_M_SBYT	0x60
#define PS2_M_ENAB	0xA8
#define PS2_M_CMPQ	0x20
#define PS2_M_SET_COMPAQ_BYTE	PS2_M_CMPQ

// Response Set
#define PS2_M_ERR	0xFC
#define PS2_M_ERROR				PS2_M_ERR
#define PS2_M_ACK	0xFA
#define PS2_M_ACKNOWLEDGE		PS2_M_ACK
#define PS2_M_BATC	0xAA
#define PS2_M_PASSED_BAT_TEST	PS2_M_BATC

// IRQs
#define PS2_M_VEC	0x2C

// Init Functions
void _ps2_mouse_init( void );

// ISR Functions
void _ps2_mouse_isr( int vec, int code ); 

// Read/Write functions
Uint _ps2_read_mouse( void );
void _ps2_write_mouse(char b );
Uint _ps2_mouse_ready( void );
void _ps2_mouse_clear( void );

// Error messages
void _ps2_nonack( Uint b );
void _ps2_debug( const char * comp, Uint resp );

#endif

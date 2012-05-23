/**
 * File:	ps2.h
 * Author:	Grant Kurtz
 *
 * Description:
 *		Defines several constants for use with communicating to the PS/2
 *		device controller.
 */


// Comm ports
#define PS2_PORT	0x60
#define PS2_STAT	0x64

int _ps2_init( void );

/*
** File:	vga_define.h
**
** Author:	Gideon Williams
**
** Description:	Defines for the VGA Driver
*/

#define VGA_DEBUG

#define V_MODE_800_600	0x011B
#define LIN_BUF			0x4000
#define V_MODE_TEXT		0x03

#define VESA_INFO_ADDR 	0x9000
#define	VGA__INFO_ADDR	0x8000

//VESA FUNCTIONS
#define GET_VESA_INFO 	0x4F00
#define GET_MODE_INFO	0x4F01
#define SET_MODE		0x4F02

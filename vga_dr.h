/*
** File:	vga_dr.h
**
** Author:	Gideon Williams
**
** Description:	VESA and VGA mode setting functions and types.
**
** The structs were copied from:
** 			http://www.delorie.com/djgpp/doc/ug/graphics/vesa
*/
#ifndef _VGA_H
#define _VGA_H

#include "headers.h"

#ifndef __ASM__20113__

	typedef struct VESA_INFO
	{ 
		unsigned char  VESASignature[4]    ;
		unsigned short VESAVersion         ;
		unsigned long  OEMStringPtr        ;
		unsigned char  Capabilities[4]     ;
		unsigned long  VideoModePtr        ;
		unsigned short TotalMemory         ;
		unsigned short OemSoftwareRev      ;
		unsigned long  OemVendorNamePtr    ;
		unsigned long  OemProductNamePtr   ;
		unsigned long  OemProductRevPtr    ;
		unsigned char  Reserved[222]       ;
		unsigned char  OemData[256]        ;
	}  __attribute__ ((packed)) VESA_INFO;

	typedef struct MODE_INFO
	{
		unsigned short 	ModeAttributes;
		unsigned char  	WinAAttributes      ;
		unsigned char  	WinBAttributes      ;
		unsigned short 	WinGranularity      ;
		unsigned short 	WinSize             ;
		unsigned short 	WinASegment         ;
		unsigned short 	WinBSegment         ;
		unsigned long  	WinFuncPtr          ;
		unsigned short 	BytesPerScanLine    ;
		unsigned short 	XResolution         ;
		unsigned short 	YResolution         ;
		unsigned char  	XCharSize           ;
		unsigned char  	YCharSize           ;
		unsigned char  	NumberOfPlanes      ;
		unsigned char  	BitsPerPixel        ;
		unsigned char  	NumberOfBanks       ;
		unsigned char  	MemoryModel         ;
		unsigned char  	BankSize            ;
		unsigned char  	NumberOfImagePages  ;
		unsigned char  	Reserved_page       ;
		unsigned char  	RedMaskSize         ;
		unsigned char  	RedMaskPos          ;
		unsigned char  	GreenMaskSize       ;
		unsigned char  	GreenMaskPos        ;
		unsigned char  	BlueMaskSize        ;
		unsigned char  	BlueMaskPos         ;
		unsigned char  	ReservedMaskSize    ;
		unsigned char  	ReservedMaskPos     ;
		unsigned char  	DirectColorModeInfo ;
		unsigned long  	PhysBasePtr         ;
		unsigned long  	OffScreenMemOffset  ;
		unsigned short	OffScreenMemSize    ;
		unsigned short 	LinbytesPerScanLine	;
		unsigned char 	BankNumberOfImagePages;
		unsigned char 	LinNumberOfImagePages;
		unsigned char 	RedMaskS		;
		unsigned char 	RedFPos	;
		unsigned char 	GreenMaskS	;
		unsigned char 	GreenFPos;
		unsigned char 	BlueMaskS		;
		unsigned char 	BlueFPos;
		unsigned char 	RsvdMaskS		;
		unsigned char 	RsvdFPos;
		unsigned char 	Reserved[194];
	}  __attribute__ ((packed)) MODE_INFO;


VESA_INFO *vga_vesa_info;
MODE_INFO *vga_mode_info;

void _vga_init( void );

void clear_display( void );

void _vga_print_info( void );

Uint32* _vga_get_end_mem( void );
Uint32* _vga_get_start_mem( void );

#endif // ACM

#endif // VGA

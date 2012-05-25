/*
** File:	vga_dr.c
**
** Author:	Gideon Williams
**
** Description:	VESA and VGA mode setting functions.
*/
#define	__KERNEL__20113__

#include "headers.h"
#include "vga_dr.h"
#include "vga_define.h"

/*
** _vga_init()
**
** initialize the VGA driver module
*/
void _vga_init( void ) {
	//Check the contents of the information extracted from REAL MODE 
	vga_vesa_info = (VESA_INFO *)(VESA_INFO_ADDR << 4);
	vga_mode_info = (MODE_INFO *)(VGA__INFO_ADDR << 4);
	
	#ifdef VGA_DEBUG
	//_vga_print_info();
	#endif
}

/**
 * Gets the starting address of the linear buffer/video memory.
 * 
 * @return			The pointer to the start of video meory.
 */
Uint32* _vga_get_start_mem( void ) {
	return (Uint32 *)(vga_mode_info->PhysBasePtr);
}

/**
 * Gets the ending address of the linear buffer/video memory.
 * 
 * @return			The pointer to the end of video meory.
 */
Uint32* _vga_get_end_mem( void ) {
	//setup my buffer space @ PhysBasePtr + TotalMemory
	return (Uint32 *)(vga_mode_info->PhysBasePtr + (vga_vesa_info->TotalMemory * 64 * 1024));
}

/**
 * Debug function that prints a lot of important stuffz. XD
 * 
 */
void _vga_print_info( void ) {
	#ifdef VGA_DEBUG
	c_printf("\nVESA_INFO(%x):\n", vga_vesa_info);
	c_printf("-version: %d\n", vga_vesa_info->VESAVersion);
	c_printf("-VideoModePtr: %x\n", vga_vesa_info->VideoModePtr);
	c_printf("-TtlMem: %d, %d\n", vga_vesa_info->TotalMemory, (vga_vesa_info->TotalMemory * 64)/1024);
	c_printf("\nMODE_INFO(%x):\n", vga_mode_info);
	c_printf("-ModeAttr: %x\n", vga_mode_info->ModeAttributes);
	c_printf("-MemModel: %x\n", vga_mode_info->MemoryModel);
	c_printf("-WinSize: %d\n", vga_mode_info->WinSize);
	c_printf("-WinSize (X, Y): (%d, %d)\n", vga_mode_info->XResolution, vga_mode_info->YResolution);
	c_printf("-CharSize (X, Y): (%d, %d)\n", vga_mode_info->XCharSize, vga_mode_info->YCharSize);
	c_printf("-WinFuncPrt: %x\n", vga_mode_info->WinFuncPtr);
	c_printf("-PhysBasePrt: %x\n", vga_mode_info->PhysBasePtr);
	c_printf("-LinbytesPerScanLine: %d\n", vga_mode_info->LinbytesPerScanLine);
	c_printf("-BPP: %d\n", vga_mode_info->BitsPerPixel);
	c_printf("-RED: %d - ", vga_mode_info->RedMaskS);
	c_printf("-RED: %d\n", vga_mode_info->RedFPos);
	c_printf("-BLUE: %d - ", vga_mode_info->BlueMaskS);
	c_printf("-BLUE: %d\n", vga_mode_info->BlueFPos);
	c_printf("-GREEN: %d - ", vga_mode_info->GreenMaskS);
	c_printf("-GREEN: %d\n", vga_mode_info->GreenFPos);
	c_printf("-RSVD: %d - ", vga_mode_info->RsvdMaskS);
	c_printf("-RSVD: %d\n", vga_mode_info->RsvdFPos);
	#endif	
}

/**
 * Utility function to clear the screen.
 * 
 */
void clear_display( void ) {
	Uint32 *ptr = (Uint32*)(vga_mode_info->PhysBasePtr);
    int i = 0;
    int j = 0;
	/*
    for(i = 0; i < vga_mode_info->XResolution; i++){
        for( j = 0; j < vga_mode_info->YResolution; j++){
			ptr[(j*vga_mode_info->LinbytesPerScanLine) + i] = 0xc0c0c0c0;
        }
    }*/
    for(i = 0; i < vga_mode_info->XResolution; i++){
        for( j = 0; j < vga_mode_info->YResolution; j++){
        	
			ptr[j*vga_mode_info->XResolution+i] = 0x00000000;
        }
    }
    
}

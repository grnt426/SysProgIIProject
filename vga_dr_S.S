/*
** http://www.delorie.com/djgpp/doc/ug/graphics/vesa.html
*/
#include "vga_define.h"

	.text
.globl _get_vesa_info
_get_vesa_info:
	//get VESA_INFO
	pushw 	%ax
	pushw	%bx

	movw	$VESA_INFO_ADDR, %ax
	movw	%ax, %es
	movw	$0x0, %di
	xorw 	%ax, %ax
	movw	$GET_VESA_INFO, %ax
	int 	$0x10
	
	cmpb	$0x0, %ah
	jne		_vga_die
	cmpb	$0x4F, %al
	jne		_vga_die
	
	popw	%bx
    popw	%ax
    ret

.globl _get_mode_info
_get_mode_info:
	//get VESA_MODEINFO
	pushw 	%ax
	pushw	%bx
	pushw	%cx

	movw	$VGA__INFO_ADDR, %ax
	movw 	%ax, %es
	movw	$0x0, %di
	xorw 	%ax, %ax
	movw   	$V_MODE_800_600, %cx
	movw	$GET_MODE_INFO, %ax
	int		$0x10
	
	cmpb	$0x0, %ah
	jne		_vga_die
	cmpb	$0x4F, %al
	jne		_vga_die
	
	popw	%cx
	popw	%bx
    popw	%ax
    ret
	
	/* 
	** get mode info????
	** --->he had one more step I though it read as above
	** maybe setting the memory model from banks to linear buffer????????
	*/
.globl _select_vga_mode
_select_vga_mode:
	pushw 	%ax
	pushw	%bx
	
	//select mode
	xorw   	%ax, %ax
    movw   	$V_MODE_800_600, %bx
    orw		$LIN_BUF, %bx
    movw 	$SET_MODE, %ax
    int		$0x10
    
    popw	%bx
    popw	%ax
    ret
    
.globl _select_text_mode
_select_text_mode:
	pushw 	%ax
	pushw	%bx
	
	//select mode
	xorw   	%ax, %ax
    movw 	$V_MODE_TEXT, %ax
    int		$0x10
    
    popw	%bx
    popw	%ax
    ret

_vga_die:
	call 	_select_text_mode
	hlt

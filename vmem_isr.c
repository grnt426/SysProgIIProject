/*
** File:	vmem_isr.c
**
** Author:	Sean McDermott
**
** Contributor:
**
** Description:	 These are the code isrs dealing with virtual memory.
*/

#include "vmem_isr.h"
#include "sio.h"

//aka falling down THREE flights of stairs
void _isr_vmem_general_protect( int vector, int code )
{
	c_printf("\n Page Directory: %x \n", _isr_vmem_getcr3() );
	c_printf(" EIP: %x    CS: %x\n", _isr_vmem_geteip(), _isr_vmem_getcs() );
	c_printf("\n %x=%d %x=%d", vector, vector, code, code);
	__panic("MUST PROTECT");
}

void _isr_vmem_page_fault( int vector, int code )
{
	c_printf("\n Page Directory: %x \n", _isr_vmem_getcr3() );

	_sio_writes("F Address:",10);
	write_x(_isr_vmem_getcr2() );
	_sio_writes(" code:",6);
	write_x( code);
	c_printf(" %x=%d %x=%d\n", vector, vector, code, code);
	c_printf("Fautly address %x\n", _isr_vmem_getcr2() );
	__panic("fault");
}


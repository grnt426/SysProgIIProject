/*
** File:	vmem.h
**
** Author:	Sean McDermott
**
** Contributor:
**
** Description:	Virtual Memory Definitions
*/

#ifndef _VMEM_ISR_H
#define _VMEM_ISR_H
//#define _VMEM_ISR_DEBUG

#include "headers.h"

/*
** Name:	_isr_vmem_general_protect
** Description:	this isr protects the system from triple faulting and gives 
		system feed back on what address caused the protection fault
** Arguments:	vector - int - which interrupt got called
		code - int - non used
** Return:	
*/
void _isr_vmem_general_protect( int vector, int code );

/*
** Name:	_isr_vmem_page_fault
** Description:	this isr catches page faults and prints information regarding it
** Arguments:	vector - int - which interrupt got called
		code - int - information about the address that faulted
** Return:	none 
*/
void _isr_vmem_page_fault( int vector, int code );

/*
** Name:	_isr_vmem_getcr2
** Description:	gets the value from cr2 for debugging purposes
** Arguments:	none
** Return:	Uint32 - the value of cr2
*/
Uint32 _isr_vmem_getcr2(void);
/*
** Name:	_isr_vmem_getcr3
** Description:	gets the value from cr3 for debugging purposes
** Arguments:	none
** Return:	Uint32 - the value of cr3
*/
Uint32 _isr_vmem_getcr3(void);

/*
** Name:	_isr_vmem_geteip
** Description:	gets the value from eip for debugging purposes
** Arguments:	none
** Return:	Uint32 - the value of eip
*/
Uint32 _isr_vmem_geteip(void);

/*
** Name:	_isr_vmem_getcs
** Description:	gets the value from cs for debugging purposes
** Arguments:	none
** Return:	Uint32 - the value of cs
*/
Uint32 _isr_vmem_getcs(void);

#endif

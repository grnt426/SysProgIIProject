/*
** File:	vmem_ref.h
**
** Author:	Sean McDermott
**
** Contributor:
**
** Description:	These are definitions for counting share pages.
**		Defines are ways of getting values out of a 32 bit word
*/

#ifndef _VMEM_REF_H
#define _VMEM_REF_H
//#define _VMEM_REF_DEBUG

#include "headers.h"

#define REF_EMPTY	0x0 //entry is null and being used
#define REF_ADDRESS 	0xFFFF000 //the physical address to the page being shared
#define REF_VALID	0x02 //entry is being used for reference counting
#define REF_4MB		0x01 //means it is a 4MB page
#define REF_COUNT	0xFFC //means a page can only be shared 1024 times
#define REF_COUNT_ADD	0x04 //to increment by one
#define REF_START	0x5000000 //where to store reference counting
#define REF_END		0x5400000 //ending address for reference counting
#define REF_SIZE	1048576 //4MB / 4 bytes


/*
** Name:	_vmem_ref_init
** Description:	initializes all structure need to be able to use reference
		counting
** Arguments:	none
** Return:	none
*/
void _vmem_ref_init( void );

/*
** Name:	_vmem_ref_get_count
** Description:	This function looks up the reference count for a specfic page
** Arguments:	address - Uint32 - the physical address of the page being look
		for
		is4MB - Uint8 - 1 means the page is 4 MB page 
				0 menas the page is 4 KB page
** Return:	Uint32 - the count of the number references to the page
*/
Uint32 _vmem_ref_get_count( Uint32 address, Uint8 is4Mb );

/*
** Name:	_vmem_ref_inc_count
** Description:	this increase the reference count to a page
** Arguments:	address - Uint32 - the physical address of the page being 
		increase
		is4MB - Uint8 - 1 means the page is 4 MB page 
				0 menas the page is 4 KB page
** Return:	none
*/
void _vmem_ref_inc_count( Uint32 address, Uint8 is4Mb );

/*
** Name:	_vmem_ref_dec_count
** Description:	this decreases the reference count to a page
** Arguments:	address - Uint32 - the physical address of the page being
		decreased
		is4MB - Uint8 - 1 means the page is 4 MB page 
				0 menas the page is 4 KB page
** Return:	none
*/
void _vmem_ref_dec_count( Uint32 address, Uint8 is4Mb );

#endif

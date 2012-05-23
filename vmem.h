/*
** File:	vmem.h
**
** Author:	Sean McDermott
**
** Contributor:
**
** Description:	Virtual Memory Definitions
*/

#ifndef _VMEM_H
#define _VMEM_H
//#define _VMEM_DEBUG

#include "headers.h"

#define PAGE_BITS 12
#define PAGE_SIZE (1 << PAGE_BITS )
#define PAGE_ADDRESS_LOC ( 0xFFFFFFFF << PAGE_BITS)
#define PAGE_TABLE_SIZE (PAGE_SIZE * 1024)
#define PAGE_RESERVE_ENTRIES 10
#define BITMAP_NORMAL ( 32 * ( PAGE_RESERVE_ENTRIES + 2) )
#define BITMAP_MAX 26265

#define PAGE_DIR_4KB_ADDRESS 0xffff000
#define PAGE_DIR_4MB_ADDRESS 0xFFC0000
#define PAGE_DIR_RESERV 0x600
#define PAGE_DIR_IGNORE 0x100
#define PAGE_DIR_SIZE 0x80
#define PAGE_DIR_DIRTY 0x40
#define PAGE_DIR_ACCESS 0x20
#define PAGE_DIR_CACHE_DISABLE 0x10
#define PAGE_DIR_WRITE_THROUGH 0x8
#define PAGE_DIR_USER_MODE 0x4
#define PAGE_DIR_WRITE 0x2
#define PAGE_DIR_PRESENT 0x1

#define PAGE_TABLE_ADDRESS 0xffff000
#define PAGE_TABLE_RESERV 0x600
#define PAGE_TABLE_GLOBAL 0x100
#define PAGE_TABLE_ZERO 0x80
#define PAGE_TABLE_DIRTY 0x40
#define PAGE_TABLE_ACCESSED 0x20
#define PAGE_TABLE_CACHE_DISABLE 0x10
#define PAGE_TABLE_WRITE_THROUGH 0x8
#define PAGE_TABLE_USER_MODE 0x4
#define PAGE_TABLE_WRITE 0x2
#define PAGE_TABLE_PRESENT 0x1

/*
**_vmem_page_dir - this is a globally accesuible variable which is tells where 
**			the first page direcoty is 
		 - this page directory will have all kernel level address
**
*/
extern Uint32* _vmem_page_dir;

/*
** _vmem_init()
**
*/
void _vmem_init( void );

/*
** Name:	_vmem_first_4MB
** Description:	the function indentity maps the first 4MB of memory for the kernel
** Arguments:	none
** Return:	Uint32 to the ending addres of the idenity map, hint is it isn't
**		 0x400000 there is a problem
*/
Uint32 _vmem_first_4MB( void );

/*
** Name:	_vmem_init_bitmap
** Description:	 This function reserve 4MB for use for a bitmap and initilizes it
** Arguments:	none
** Return:	the address to start mapping the bitmap at
		the bitmap is designed to start immeditely following the 4MB
*/
void _vmem_init_bitmap( Uint32 addr );

/*
** Name:	_vmem_make_reserve
** Description:	this function maps a set of addresses into everyone pages for the 
		ability to write to these address directly before mapping them
** Arguments:	none
** Return:	none
*/
void _vmem_make_reserve(void);

/*
** Name:	_vmem_read_bit
** Description:	reads the specified bit from specified Uint32 array frome a specified
**		index into the array and index into thw word
** Arguments:	address	- Uint32 bit array starting address
**		index	- Uint32 specifing the index into the array
**		index	- Int8 specifing the bit index int the selected Uint32
** Return:	0 if the bit is not set, 1 if the bit is set
*/
Int8 _vmem_read_bit( Uint32* address, Uint32 index, Int8 index2 );
/*
** Name:	_vmem_set_bit
** Description:	turns on a specified bit in the bitmap
** Arguments:	address	- Uint32 bit array starting address
**		index	- Uint32 specifing the index into the array
**		index	- Int8 specifing the bit index int the selected Uint32
** Return:	none
*/
void _vmem_set_bit( Uint32* address, Uint32 index, Int8 index2 );
/*
** Name:	_vmem_clear_bit
** Description:	turns off a specified bit in the bitmap
** Arguments:	address	- Uint32 bit array starting address
**		index	- Uint32 specifing the index into the array
**		index	- Int8 specifing the bit index int the selected Uint32
** Return:	none
*/
void _vmem_clear_bit( Uint32* address, Uint32 index, Int8 index2);

/*
** Name:	_vmem_set_address
** Description:	marks the 4KB address as taken
		(under the hood this is setting the bit to 1)
** Arguments:	address - Uint32 - physical address being taken
** Return:	none
*/
void _vmem_set_address( Uint32 address );
/*
** Name:	_vmem_set_4MB_address
** Description:	marks a 4MB address as taken
** Arguments:	address - Uint32 - is the physical address taken
** Return:	none
*/
void _vmem_set_4MB_address( Uint32 addres );
/*
** Name:	_vmem_clear_address
** Description:	marks the 4KB address as avaible
		(under the hood this is setting the bit to 0)
** Arguments:	address - Uint32 - physical address being taken
** Return:	none
*/
void _vmem_clear_address( Uint32 address );
/*
** Name:	_vmem_clear_4MB_address
** Description:	marks a 4MB address as clear
** Arguments:	address - Uint32 - is the physical address taken
** Return:	none
*/
void _vmem_clear_4MB_address( Uint32 address );

/*
** Name:	_vmem_get_next_address
** Description:	get the next 4KB physical address
** Arguments:	none
** Return:	Uint32 - starting physical address 
*/
Uint32 _vmem_get_next_address(void);
/*
** Name:	_vmem_get_next_reserve_address
** Description:	get the next 4Kb that is in eveyone address range
** Arguments:	none
** Return:	Uint32 - starting physical adress 
*/
Uint32 _vmem_get_next_reserve_address(void);
/*
** Name:	_vmem_get_next_4MB_address
** Description:	gets the next 4MB block that is free
** Arguments:	none
** Return:	Uint32 = starting pyhsical address
*/
Uint32 _vmem_get_next_4MB_address(void);

/*
** Name:	_vmem_address_calc
** Description:	converts an address for a 4KB page to approiate indexes 
** Arguments:	Uint32 - Address convert
**		Uint32* - pointer to index value ( which tells which value in the array)
**		Uint8*	- pointer to index2 value (which tells which bit in the word to select)
** Return:	none
*/
void _vmem_address_calc( Uint32 address, Uint32* index, Uint8* index2 );
/*
** Name:	_vmem_address_4MB_calc
** Description:	calculates the index of a 4MB page to the approiate index
** Arguments:	Uint32 - Address convert
**		Uint32* - pointer to i value ( which tells which value in the array)
** Return:	
*/
void _vmem_address_4MB_calc( Uint32 address, Uint32* i );
/*
** Name:	_vmem_get_address
** Description:	calculates the address based on indexes in the bitmap
** Arguments:	Uint32 - index value ( which tells which value in the array)
**		Uint8  - index2 value (which tells which bit in the word to select)
** Return:	Uint32 - physical address
*/
Uint32 _vmem_get_address( Uint32 index, Uint8 index2 );
/*
** Name:	_vmem_get_4MB_address
** Description:	caculates the addres based on the index in the bitmap
** Arguments:	Uint32 - index ( which tells which value in the array)
** Return:	Uint32 - phyical address
*/
Uint32 _vmem_get_4MB_address( Uint32 index); 

//assembly functions

/*
** Name:	_vmem_turnon
** Description:	does all function neccessary to startup up paging
** Arguments:	Uint32 - pointer to the page directory table
** Return:	none
*/
void _vmem_turnon(Uint32 page);

/*
** Name:	_vmem_getcr0
** Description:	This function get the value of cr0 for debugging purposes
** Arguments:	none
** Return:	int - the value of cr0
*/
int _vmem_getcr0(void);

/*
** Name:	_vmem_bsf
** Description:	this function search through a word for the least significant bit that is set to 1
** Arguments:	Uint32 - 32 bit word to be searched
** Return:	int - index
*/
int _vmem_bsf(Uint32 word);

#ifdef _VMEM_DEBUG

/*
** Name:	_vmem_address_test
** Description:	Runs a suite of address tests 
** Arguments:	none
** Return:	none
*/
void _vmem_addresses_test(void);

/*
** Name:	_vmem_address_test
** Description:	address test the 4KB address to index
** Arguments:	Uint32 - addr - address to a 4KB block
	`	Uint32 - index - index to into the array
		Uint8 - index2 - index into the 32 bit word
** Return:	none
*/
void _vmem_address_test(Uint32 addr, Uint32 index, Uint8 index2);

/*
** Name:	_vmem_address_4MB_test
** Description:	addres tests the 4MB address to index
** Arguments:	Uint32 - addr - address to a 4MB block
	`	Uint32 - i - index to into the array
** Return:	none
*/
void _vmem_address_4MB_test( Uint32 addr, Uint32 i );
#endif	
#endif

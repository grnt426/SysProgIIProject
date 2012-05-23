#ifndef _VMEML2_H
#define _VMEML2_H
//#define _VMEML2_DEBUG

#include "headers.h"

#define STACK_ADDRESS	0x3000000

/*
** Name:	_vmeml2_create_page_dir
** Description:	creates a page directory with kernel level pages mapped
** Arguments:	none
** Return:	Uint32* - which is the pointer to the page table
*/
Uint32* _vmeml2_create_page_dir( void );

/*
** Name:	_vmeml2_create_page_table
** Description:	This function creates a blank page directory and sets in the 
		page directory given
** Arguments:	dir - Uint32* - the directory in which to map the page directory
		index - Uint16 - the index to place the page table at
** Return:	Uint32* - the address to the page table 
*/
Uint32* _vmeml2_create_page_table( Uint32* dir, Uint16 index );

/*
** Name:	_vmeml2_create_page
** Description	get a new valid 4KB page and set is in the page table
** Arguments:	table - Uint32* - the address of the page table 
		index - Uint16 - the index to place the page in table 
** Return:	none
*/
void _vmeml2_create_page( Uint32* table, Uint16 index );

/*
** Name:	_vmeml2_create_page_reserved
** Description:	get a new valid 4KB page and set is in the page table that
		can be written to in all pages
** Arguments:	table - Uint32* - the address of the page table 
		index - Uint16 - the index to place the page in table 
** Return:	Uint32* - pointer to the new page in phyical memory
*/
Uint32* _vmeml2_create_page_reserved( Uint32* table, Uint16 index );

/*
** Name:	_vmeml2_create_4MB_page
** Description:	get a new valid 4MB page and set is in page directory table
** Arguments:	dir - Uint32* - the directory in which to map the page directory
		index - Uint16 - the index to place the 4MB page at
** Return:	Uint32* - pointer to the page in phyiscal memory
*/
Uint32* _vmeml2_create_4MB_page( Uint32* dir, Uint16 index); 

/*
** Name:	_vmeml2_release_page_dir
** Description:	Goes through all pages and releases all pages except for the 
		ones assigned to other processes
** Arguments:	dir - Uint32* - pointer to page directory table
** Return:	none
*/
void _vmeml2_release_page_dir( Uint32* dir );

/*
** Name:	_vmeml2_static_address
** Description:	It sets asside a chuck of memory for control outside of vmem's
		control. This we also map all this entry to all new page
		table directories.
** Arguments:	addr1 - starting address to set aside (must be 4MB aligned)
		addr2 - ending address
		mark - Uint8 - 0 to not mark addresses as taken
			       1 to mark addresses as taken
** Return:	none	
*/
void _vmeml2_static_address( Uint32 addr1, Uint32 addr2, Uint8 mark);

/*
** Name:	_vmeml2_static_dir_entry
** Description:	this function records that and entry is static
** Arguments:	index - Uint32 - the entry in the page directory table to 
		make as static
** Return:	none
*/
void _vmeml2_static_dir_entry( Uint32 index);


/*
** Name:	_vmeml2_is_empty_dir_entry
** Description:	checkes where or not and entry in the page directory 
		table is empty or not
** Arguments:	dir - Uint32* - the page directory table to check
		index - Uint16 - the index to check in the pdt
** Return:	Uint8 - 0 meaning there is an entry in the table	
		      - 1 memaning there was no entry in the table
*/
Uint8 _vmeml2_is_empty_dir_entry( Uint32* dir, Uint32 index);

/*
** Name:	_vmeml2_is_empty_page_entry
** Description:	check wheather or not a page exists in a page table
** Arguments:	table - Uint32* - the address of the page table 
		index - Uint16 - the index to check
** Return:	Uint8 - 0 meaning there is an entry in the table	
		      - 1 memaning there was no entry in the table
*/
Uint8 _vmeml2_is_empty_page_entry( Uint32* table, Uint32 index);

/*
** Name:	_vmeml2_get_phyical_address
** Description:	read a page table entry and get is real address
** Arguments:	table - Uint32* - the address of the page table 
		index - Uint16 - the index to convert
** Return:	Uint32 - the actual address of the page
*/
Uint32 _vmeml2_get_phyical_address( Uint32* table, Uint16 index );
/*
** Name:	_vmeml2_get_phyical_address_table
** Description:	gets the actuall phyiscal memory address
** Arguments:	table - Uint32* - the address of the page table 
		index - Uint16 - the index to convert
** Return:	Uint32 - the actual address of the page
*/
Uint32 _vmeml2_get_phyical_address_table( Uint32* dir, Uint16 index );
/*
** Name:	_vmeml2_get_phyical_address_4MB
** Description:	get the physical memory address of a 4MB page
** Arguments:	dir - Uint32* - the directory in which to map the page directory
		index - Uint16 - the index to physical memory address
** Return:	Uint32 - the actual address of the page
*/
Uint32 _vmeml2_get_phyical_address_4MB( Uint32* dir, Uint16 index);

/*
** Name:	_vmeml2_init
** Description:	does the initializing procedure for the vmeml2
** Arguments:	none
** Return:	none
*/
void _vmeml2_init( void );

//assembly function found in vmemA.S
/*
** Name:	_vmeml2_change_page
** Description:	changes the page directory table being used 
** Arguments:	Uint32 - address of the page directory table
** Return:	none
*/
void _vmeml2_change_page(Uint32 page);

#endif

/*
** File:	vmemL2.c
**
** Author:	Sean McDermott
**
** Contributor:
**
** Description:	Contains the code for common memory aplications 
**		This will prompt doing these operations in a common way
**		If there is any problem all calls can be fix once instead of all over the place
*/

#include "vmem.h"
#include "vmemL2.h"

//This maintains a list of all pages the need to be shared to all page directory tables
static Uint32 _vmeml2_all_pages_tables[100];
//the number of pages that need to be shared to all page directories
static Uint8 _vmeml2_all_pages_tables_size = 0;

Uint32* _vmeml2_create_page_dir( void )
{
	//get an address we can write to
	Uint32 addrRaw = _vmem_get_next_reserve_address();
	//mark address as taken
	_vmem_set_address( addrRaw );

	
	Uint32* addr = (Uint32*) addrRaw;
	
	//clear table
	int i;
	for(i = 0; i < 1024; i++ )
	{
		addr[i] = PAGE_DIR_WRITE;
	}

	//copy all shared pages to this page table (they are 4MB pages)
	for( i=0; i < _vmeml2_all_pages_tables_size; i++ )
	{
		addr[_vmeml2_all_pages_tables[i]] = _vmem_page_dir[_vmeml2_all_pages_tables[i]];
#ifdef _VMEML2_DEBUG
		//c_printf( "Address Info %x %d\n",  addr[_vmeml2_all_pages_tables[i]], _vmeml2_all_pages_tables[i] );
#endif
	}
	return addr;
}

Uint32* _vmeml2_create_page_table( Uint32* dir, Uint16 index )
{
	//make sure the entry is empty 
	if( !_vmeml2_is_empty_dir_entry( dir, index) )
	{
		return 0x00;
	}
	
	//get an address we can write to because we need to set pages it to
	Uint32 tableAddr = _vmem_get_next_reserve_address();
	//mark address taken
	_vmem_set_address(tableAddr);
	
	//clear page table
	Uint32* tAddr = (Uint32*)tableAddr;
	int i;
	for( i=0; i < 1024; i++ )
	{
		tAddr[i] = PAGE_TABLE_WRITE;
	}

	//set the page table in the page directory
	dir[index] = tableAddr | PAGE_DIR_PRESENT | PAGE_DIR_WRITE;
	return tAddr;
}

void _vmeml2_create_page( Uint32* table, Uint16 index )
{
	if( !_vmeml2_is_empty_page_entry(table, index))
	{
		__panic("Page already maped");
	}

	Uint32 page = _vmem_get_next_address();
	_vmem_set_address(page);
	
	table[index] = page | PAGE_TABLE_WRITE | PAGE_TABLE_PRESENT;
}

Uint32* _vmeml2_create_page_reserved( Uint32* table, Uint16 index )
{
	if( !_vmeml2_is_empty_page_entry(table, index))
	{
		__panic("Page already maped");
	}

	Uint32 page = _vmem_get_next_reserve_address();
	_vmem_set_address(page);
	
	table[index] = page | PAGE_TABLE_WRITE | PAGE_TABLE_PRESENT;
	return (Uint32*)page;
}

Uint32* _vmeml2_create_4MB_page( Uint32* dir, Uint16 index )
{
	if( !_vmeml2_is_empty_dir_entry( dir, index) )
	{
		__panic("4MB page already maped");
		return 0x00;
	}

	Uint32 page = _vmem_get_next_4MB_address();
	_vmem_set_4MB_address(page);
	
	dir[index] = page | PAGE_DIR_PRESENT | PAGE_DIR_WRITE | PAGE_DIR_SIZE;
	return (Uint32*)page;
}

void _vmeml2_release_page_dir( Uint32* dir )
{
	int i;
	//get rid of share pages, but do not free them because they are shared
	for( i=0; i < _vmeml2_all_pages_tables_size; i++ )
	{
		dir[_vmeml2_all_pages_tables[i]] = PAGE_TABLE_WRITE; 
	}
	
	//loop over all entries in the page directory
	Uint32 temp;
	Uint32 temp2;
	Uint32* table;
	int a;
	for( i=0; i < 1024; i++ )
	{
		//if there is nothing present skip
		temp = dir[i];
		if( ( temp & PAGE_DIR_PRESENT ) != PAGE_DIR_PRESENT )
		{
			continue;
		}
		//detele 4MB page
		if( ( temp & PAGE_DIR_SIZE ) == PAGE_DIR_SIZE )
		{

#ifdef _VMEML2_DEBUG
			c_printf( "%x %x : ", temp, ( temp & PAGE_DIR_4MB_ADDRESS ));  
			c_printf( "Goodbye Address %x \n", ( temp & PAGE_DIR_4MB_ADDRESS ) );
#endif
			_vmem_clear_4MB_address( temp & PAGE_DIR_4MB_ADDRESS );
			continue;
		} 

		table = (Uint32*)( temp & PAGE_DIR_4KB_ADDRESS);
		//else go through page table 
		for( a=0; a < 1024; a++ )
		{
			//if it is a non present page skip
			temp2 = table[a];
			if( ( temp2 & PAGE_TABLE_PRESENT ) != PAGE_TABLE_PRESENT )
			{
				continue;
			}
			
#ifdef _VMEML2_DEBUG
			c_printf( "%x %x : ", temp2, ( temp2 & PAGE_TABLE_ADDRESS ));  
			c_printf( "Goodbye Address2 %x \n", ( temp2 & PAGE_TABLE_ADDRESS ) );
#endif
			_vmem_clear_address( temp2 & PAGE_TABLE_ADDRESS );
			
			
		}
		
		//clear
#ifdef _VMEML2_DEBUG
		c_printf( "%x %x : ", temp, ( temp & PAGE_DIR_4KB_ADDRESS ));  
		c_printf( "Goodbye Address3 %x \n", ( temp & PAGE_DIR_4KB_ADDRESS ) );
#endif
		//release page table address
		_vmem_clear_address( temp & PAGE_DIR_4KB_ADDRESS);

	}
#ifdef _VMEML2_DEBUG
	c_printf( "Goodbye Address4 %x \n", dir );
#endif
	//release page directory table address
	_vmem_clear_address( (Uint32)dir );
}

void _vmeml2_static_address( Uint32 addr1, Uint32 addr2, Uint8 mark)
{
	//c_printf( "|%x,%x,%x|", addr1, PAGE_TABLE_SIZE, (addr1 % PAGE_TABLE_SIZE));
	if( addr1 % PAGE_TABLE_SIZE != 0 ) 
	{
		__panic("Static address not aligned on 4MB boundary");
	}

	//for every 4MB chuck ( rounded up) in the the address space 
	Uint32 tempAddr;
	Uint32 index;
	for( tempAddr = addr1; tempAddr < addr2 && tempAddr != 0; tempAddr = tempAddr + PAGE_TABLE_SIZE )
	{
		//mark address taken if told to
		if( mark )
		{
			_vmem_set_4MB_address( tempAddr);
		}
		index = tempAddr / PAGE_TABLE_SIZE;
		//record entry as static
		_vmeml2_static_dir_entry( index);
		//assign to the main page directory table
		_vmem_page_dir[index] = tempAddr | PAGE_DIR_PRESENT | PAGE_DIR_WRITE | PAGE_DIR_SIZE;
		//c_printf("Index: %x Address: %x \n", index, tempAddr);
	}
}

void _vmeml2_static_dir_entry( Uint32 index)
{
	_vmeml2_all_pages_tables[_vmeml2_all_pages_tables_size] = index;
	_vmeml2_all_pages_tables_size++;
}

Uint8 _vmeml2_is_empty_dir_entry( Uint32* dir, Uint32 index)
{
	Uint32 present = dir[index] & PAGE_DIR_PRESENT;
	if( present == PAGE_DIR_PRESENT )
	{
		return FALSE;
	}
	return TRUE;
}

Uint8 _vmeml2_is_empty_page_entry( Uint32* table, Uint32 index)
{
	Uint32 present = table[index] & PAGE_TABLE_PRESENT;
	if( present == PAGE_TABLE_PRESENT )
	{
		return FALSE;
	}
	return TRUE;
}


Uint32 _vmeml2_get_phyical_address( Uint32* table, Uint16 index )
{
	return table[index] & PAGE_TABLE_ADDRESS;
}

Uint32 _vmeml2_get_phyical_address_table( Uint32* dir, Uint16 index )
{
	return dir[index] & PAGE_DIR_4KB_ADDRESS;
}

Uint32 _vmeml2_get_phyical_address_4MB( Uint32* dir, Uint16 index)
{
	return dir[index] & PAGE_DIR_4MB_ADDRESS;
}

void _vmeml2_init(void)
{
	//marks the first two 4MB pages and the number specfied by reserver entries as static
	int i;
	for( i =0; i < PAGE_RESERVE_ENTRIES + 2; i++ )
	{ 
		_vmeml2_all_pages_tables[i] = i;
		_vmeml2_all_pages_tables_size++;
	}

#ifdef _VMEML2_DEBUG
	Uint32* dirTable = _vmeml2_create_page_dir(); 
	Uint32* dirTable2 = _vmeml2_create_page_dir(); 
	c_printf("Dir Table %x\n", (Uint32) dirTable );
	c_printf("Page made physical %x \n",_vmeml2_create_4MB_page( dirTable, 90));
	_vmeml2_release_page_dir( dirTable );
	//_vmeml2_change_page( (Uint32) dirTable );*/

	c_printf("\nDir Table2 %x\n", (Uint32) dirTable2 );
	Uint32* pTable = _vmeml2_create_page_table( dirTable2, 89);
	_vmeml2_create_page( pTable, 1);
	c_printf("Page made physical %x \n",_vmeml2_create_4MB_page( dirTable2, 90));
	_vmeml2_release_page_dir( dirTable2 );

	Uint32* dirTable3 = _vmeml2_create_page_dir(); 
	c_printf("\nDir Tabl3 %x\n", (Uint32) dirTable3 );
	c_printf("Page made physical %x \n",_vmeml2_create_4MB_page( dirTable3, 90));
	Uint32* pTable2 = _vmeml2_create_page_table( dirTable3, 89);
	_vmeml2_create_page( pTable2, 1);
	
	c_printf( "\n%x |",_vmeml2_get_phyical_address( pTable2, 1) );	
	c_printf( "%x |",_vmeml2_get_phyical_address_table( dirTable3, 89));	
	c_printf( "%x |\n",_vmeml2_get_phyical_address_4MB( dirTable3, 90));	
	_vmeml2_release_page_dir( dirTable3 );
	__panic("Please Inspect");

#endif 
#ifdef _VMEM_DEBUG
	_vmeml2_static_address(0xD0000000, ( 0xD0000000 + PAGE_TABLE_SIZE ), TRUE );
	c_printf( "%x \n", _vmem_page_dir[0]); 
	c_printf( "%x \n", _vmem_page_dir[0x340]);
	c_printf("Info %x \n", *((Uint32*)0xD0000000) );
#endif
	c_puts(" vmeml2" );
}

/*
** File:	vmem.c
**
** Author:	Sean McDermott
**
** Contributor:
**
** Description:
*/

#include "startup.h"
#include "vmem.h"

//defines where the bitmap starts
Uint32* _vmem_bitmap_start = (Uint32*)(PAGE_TABLE_SIZE);
//defines the first page directory (maps everthing need by the kernel)
Uint32* _vmem_page_dir = (Uint32*)0xFFFFFFFF;

void _vmem_init( void )
{
	//builds the first 4MB of memory
	Uint32 address = _vmem_first_4MB();
	_vmem_init_bitmap( address );
	//build the memeory mapped into every process
	_vmem_make_reserve();

	c_puts(" vmem" );
#ifdef _VMEM_DEBUG
	//*((Uint32*)0xD0000000) = 0x1234;
	_vmem_addresses_test();
	c_printf("Next availible address %x \n", _vmem_get_next_address());
	c_printf("Next availible 4MB address %x \n", _vmem_get_next_4MB_address());
	_vmem_set_address(0x3000000);
	c_printf("Next availible address %x \n", _vmem_get_next_address());
	c_printf("Next availible 4MB address %x \n", _vmem_get_next_4MB_address());
	_vmem_set_address(0x3001000);
	c_printf("Next availible address %x \n", _vmem_get_next_address());
	_vmem_set_address(0x3008000);
	c_printf("Next availible address %x \n", _vmem_get_next_address());
	_vmem_clear_address(0x3000000);
	c_printf("Next availible address %x \n", _vmem_get_next_address());
	_vmem_clear_address(0x3001000);
	_vmem_clear_address(0x3008000);
	_vmem_set_address(0x3300000);
	c_printf("Next availible 4MB address %x \n", _vmem_get_next_4MB_address());
#endif
	
}

Uint32 _vmem_first_4MB( void )
{
	if( __get_end() >= PAGE_TABLE_SIZE - sizeof(Uint32) ){
		__panic( "Houston we have a problem, the kernel is too fat.");
	}

	//get the end of memory and get the next aligned address
	_vmem_page_dir = (Uint32*)((__get_end() & PAGE_ADDRESS_LOC) + PAGE_SIZE);
	
#ifdef _VMEM_DEBUG
	c_printf( "%x\n", (__get_end() & PAGE_ADDRESS_LOC) + PAGE_SIZE ) ;
#endif

	//initialize the directory so no pages are prsent
	int i;
	for(i = 0; i < 1024; i++ )
	{
		_vmem_page_dir[i] = PAGE_DIR_WRITE;
	}

	//maps the first 4 MBs 0x400000
	_vmem_page_dir[0] = (Uint32) 0x00 | PAGE_DIR_PRESENT | PAGE_DIR_WRITE | PAGE_DIR_SIZE;
	
	//send the pdt to be placed in the correct stop and pagging turned on
	_vmem_turnon((Uint32)_vmem_page_dir);

#ifdef _VMEM_DEBUG
	c_printf( "\n\nEnd is at position %x \n", __get_end() );
	c_printf( "Aligned address is %x \n", _vmem_page_dir);
	//c_printf( "First page table address is %x \n", pageTableStart);
	c_printf( "Cr0 is %x \n", _vmem_getcr0());
	c_printf( "Cr0 High is %x \n", ( (_vmem_getcr0()>>31) & 0x01));
#endif	
	//return address;
	return PAGE_TABLE_SIZE;
}

void _vmem_init_bitmap( Uint32 addr )
{
	//make sure we start the bitmap where we think we should
	if ( addr != PAGE_TABLE_SIZE )
	{
		__panic( "vmem: bitmap start is not where expected" );
	}

	//maps the next 4 MBs
	_vmem_page_dir[1] = (Uint32) PAGE_TABLE_SIZE | PAGE_DIR_PRESENT | PAGE_DIR_WRITE | PAGE_DIR_SIZE;

	//initilize all bits to be set 1 in the bitmap (aka the address is free)
	int i;
	for( i = 0; i < (PAGE_TABLE_SIZE/32); i++ )
	{
		_vmem_bitmap_start[i] = 0xFFFFFFFF;
	}

	//set the first 8MB are in use
	_vmem_set_4MB_address( 0x00 );
	_vmem_set_4MB_address( PAGE_TABLE_SIZE );
#ifdef _VMEM_DEBUG
	c_printf("Bitmap ending init addr: %x \n", _vmem_bitmap_start );
	c_printf(" %x : ", PAGE_TABLE_SIZE);
	c_printf(" %x : ", PAGE_SIZE);
	c_printf( "Ending of bitmap table is %x \n", addr);
	c_printf("0: %x \n", _vmem_page_dir[0] );
	c_printf("1: %x \n", _vmem_page_dir[1] );
	c_printf( "Bitmap entry 0 is %x \n", _vmem_bitmap_start[0]);
	c_printf( "Bitmap entry 0 is %x \n", _vmem_read_bit( _vmem_bitmap_start, 0, 0));
	c_printf( "Bitmap entry 1 is %x \n", _vmem_read_bit( _vmem_bitmap_start, 0, 1));
	c_printf( "Bitmap entry 2 is %x \n", _vmem_read_bit( _vmem_bitmap_start, 0, 2));
	c_printf( "Bitmap entry n is %x \n", _vmem_read_bit( _vmem_bitmap_start, 63, 32));
	c_printf( "Bitmap entry n is %x \n", _vmem_read_bit( _vmem_bitmap_start, 64, 0));
#endif	
}

void _vmem_make_reserve(void)
{
	//map the addresses into the orginal pdt
	int i;
	int end = PAGE_RESERVE_ENTRIES + 2;
	for( i = 2; i < end; i++ )
	{
		_vmem_page_dir[i] = (Uint32) ( PAGE_TABLE_SIZE * i) | PAGE_DIR_PRESENT | PAGE_DIR_WRITE | PAGE_DIR_SIZE;
	}
}

Int8 _vmem_read_bit( Uint32* address, Uint32 index, Int8 index2 )
{
	return ( address[index] >> index2 ) & 0x1;

}

void _vmem_set_bit( Uint32* address, Uint32 index, Int8 index2 )
{
	address[index] |=  1 << index2;
}

void _vmem_clear_bit( Uint32* address, Uint32 index, Int8 index2)
{
	address[index] &= ~(1 << index2);

}

void _vmem_set_address( Uint32 address )
{
	Uint32 index[1];
	Uint8 index2[1];
	_vmem_address_calc( address, index, index2);
	
	if ( _vmem_read_bit( _vmem_bitmap_start, *index, *index2) == 0)
	{
		__panic( "Vmem: address already in use");
	}
	_vmem_clear_bit( _vmem_bitmap_start, *index, *index2);
	//c_printf("%x %d %d ", address, *index, *index2);
}

void _vmem_set_4MB_address( Uint32 address )
{
	int i;
	for( i = 0; i < 1024;  i++ )
	{	
		_vmem_set_address(address);
		address = address + PAGE_SIZE;
	}
}

void _vmem_clear_address( Uint32 address )
{
	Uint32 index[1];
	Uint8 index2[1];
	_vmem_address_calc( address, index, index2);
	
	if ( _vmem_read_bit( _vmem_bitmap_start, *index, *index2) == 1)
	{
		__panic( "Vmem: address already cleared");
	}
	_vmem_set_bit( _vmem_bitmap_start, *index, *index2 );
}

void _vmem_clear_4MB_address( Uint32 address )
{
	int i;
	for( i = 0; i < 1024;  i++ )
	{	
		_vmem_clear_address(address);
		address = address + PAGE_SIZE;
	}
}

Uint32 _vmem_get_next_address(void)
{
	int i;
	int memory_size = BITMAP_MAX; 
	for( i = BITMAP_NORMAL; i < memory_size; i++ )
	{
		//if it is any other thing but zero one bit must be on and therfore free
		if( 0 != _vmem_bitmap_start[i] )
		{
#ifdef _VMEM_DEBUG
			c_printf( "%x %d\n", _vmem_bitmap_start[i],_vmem_bsf(_vmem_bitmap_start[i]));
#endif
			//find the sub index for the given word
			return _vmem_get_address(i,_vmem_bsf(_vmem_bitmap_start[i]));
		}
	}

	__panic( "vmem: OUT OF PHYISCAL MEMORY :(" );
	return 0x0;
}

Uint32 _vmem_get_next_reserve_address(void)
{
	Uint32 i;
	Uint32 memory_size = BITMAP_NORMAL;
	Uint32 start = 0;
	for( i = start; i < memory_size; i++ )
	{
		//if it is any other thing but zero one bit must be on and therfore free
		if( 0 != _vmem_bitmap_start[i] )
		{
			//find the sub index for the given word
			return _vmem_get_address(i,_vmem_bsf(_vmem_bitmap_start[i]));
		}
	}

	__panic( "vmem: OUT OF RESERVE ADDRESSES");
	return 0x0;
}

Uint32 _vmem_get_next_4MB_address(void)
{
	int i;
	int memory_size = BITMAP_MAX/32;	
	//make sure the we check every 32 index because that is how many word reprsenting 4KB pages it take to make 4MB
	for( i =(BITMAP_NORMAL/32); i < memory_size; i++ )
	{
		int l;
		int end = i * 32 + 32;
		Uint8 flag = TRUE;
		//loop over all 32 words
		for ( l=i*32; l < end; l++ )
		{
			if( 0xFFFFFFFF != _vmem_bitmap_start[l] )
			{
				flag = FALSE;
				break;
			}
		}
		//if the flag is still true we found an empty 4MB chunk
		if( flag )
		{
			return _vmem_get_4MB_address(i);

		}
	}

	__panic( "vmem: OUT OF PHYISCAL MEMORY :(" );
	return 0x00;
}

void _vmem_address_calc( Uint32 address, Uint32* index, Uint8* index2 )
{
	Uint32 new = address / PAGE_SIZE;
	*index = new / 32;
	*index2 = new % 32;
}

void _vmem_address_4MB_calc( Uint32 address, Uint32* i )
{
	*i = address / PAGE_TABLE_SIZE;
}

Uint32 _vmem_get_address( Uint32 index, Uint8 index2 )
{
	Uint32 addr = index * 32 + index2;
	addr = addr * PAGE_SIZE;
	return addr;
}

Uint32 _vmem_get_4MB_address( Uint32 index) 
{
	return index * PAGE_TABLE_SIZE;
}


#ifdef _VMEM_DEBUG
void _vmem_addresses_test(void)
{
	_vmem_address_test( 0x00, 0, 0 );
	_vmem_address_test( 0x1000, 0, 1 );
	_vmem_address_test( 0x2000, 0, 2 );
	_vmem_address_test( 0x3000, 0, 3 );
	_vmem_address_test( 0x4000, 0, 4 );
	_vmem_address_test( 0x400000, 32, 0);
	_vmem_address_test( 0x408000, 32, 8);
	_vmem_address_test( 0x800000, 64, 0);
	_vmem_address_4MB_test( 0x00, 0 );
	_vmem_address_4MB_test( 0x400000, 1);
	_vmem_address_4MB_test( 0x800000, 2);
}

void _vmem_address_test(Uint32 addr, Uint32 i, Uint8 i2)
{  
	Uint32 index[1];
	Uint8 index2[1];
	Uint32 result;

	c_printf( "Index: %d Index2: %d address %x ==", i, i2, addr);
	_vmem_address_calc( addr, index, index2);
	result = _vmem_get_address( i, i2);
	c_printf( "Index: %d Index2: %d address %x\n", *index, *index2, result);
}

void _vmem_address_4MB_test( Uint32 addr, Uint32 i )
{
	Uint32 index[1];
	Uint32 result;

	c_printf( "Index: %d address %x ==", i, addr);
	_vmem_address_4MB_calc( addr, index);
	result = _vmem_get_4MB_address( i);
	c_printf( "Index: %d address %x\n", *index, result);
	
}
#endif	

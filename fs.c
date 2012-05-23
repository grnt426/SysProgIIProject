////////////////////////////////////////////////////////////////////////////
// DERP_OS - DERP_FS
//
// @file	fs.c
// @descrip	This file defines all the functions needed for writing to and from
//			a DERP_FS. This also includes all necessary initialization tools.
// @author	Benjamin Russell
////////////////////////////////////////////////////////////////////////////

// REQUIRED FILES //////////////////////////////////////////////////////////

#include "headers.h"
#include "pci.h"
#include "startup.h"
#include "ata.h"
#include "fs.h"

// FUNCTIONS ///////////////////////////////////////////////////////////////

/**
 * Initializes each of the ata devices in the hopes that there are derp fs on
 * at least one of them. This initializes the global array of mountpoints.
 */
void _fs_init(void) {
    c_puts("... Searching for Filesystems\n");
   
#ifdef DERPFORMAT
	c_printf("... Creating partition 1\n");
	_fs_create_partition(&ata_devices[1], 1, 20480, 0);
	_fs_format(&mount_points[0], &ata_devices[1], 0);
	c_printf("... Creating partition 2\n");
	_fs_create_partition(&ata_devices[1], 20497, 20480, 1);
	_fs_format(&mount_points[1], &ata_devices[1], 1);
#endif
 
    if(ata_device_count == 0) {
		c_puts("*** No ATA Devices Found!\n");
    }

    // Iterate over the devices and probe for filesystems
	Uint8 i;
	for(i = 0; i < ata_device_count; i++) {
		_fs_probe(&ata_devices[i]);
	}

	if(mount_point_count == 0) {
		c_puts("*** No DERP_FS Filesystems Found!\n");
	}
}

/**
 * Probes the given ata device for any DERP_FS partitions. If a partition is
 * found, the partition is mounted to the mount_points global variable and the
 * value of mount_point_count is incremented.
 * @param	ATADevice*	dev		The device to scan for DERP_FS partitions
 */
void _fs_probe(ATADevice *dev) {
	// Create a blank sector for reading in the mbr
	ATASector mbr;
	_ata_blank_sector(&mbr);

	// Read in the MBR
	_ata_read_sector(dev, SECT_MBR, &mbr);
	
	// Go to the partition table and iterate over each entry
	Uint16 i;
	Uint16 endOfTable = FS_PART_TABLE_OFF + (FS_PART_TABLE_SIZE * FS_PART_TABLE_ENTRIES);
	for(i = FS_PART_TABLE_OFF; i < endOfTable; i += FS_PART_TABLE_SIZE) {
		// What type is the partiton?
		if(mbr[i + FS_PART_ENTRY_PT] == FS_PARTITION_TYPE) {
			// We found a DERP_FS partition!!
			// Read the first sector of the partiton to see if it's derp
			ATASector br;
			Uint32 brAddr = _sector_get_long(&mbr, i + FS_PART_ENTRY_LBA);
			_ata_read_sector(dev, brAddr, &br);

			// Is it DERP?
			Uint32 header = br[0]<<24|br[1]<<16|br[2]<<8|br[3];
			Uint32 size   = _sector_get_long(&br, FS_BR_SIZE);
			if(header == FS_PARTITION_ID) {
				// Increment the count of mount points and set up the mp
				Uint8 mp_id = mount_point_count;
				mount_point_count++;
				MountPoint mp;
				
				// Build the MountPoint
				mp.device = dev;
				mp.bootRecord.header = header;
				mp.bootRecord.size = size;
				mp.offset = brAddr;
				mp.letter = mp_id + 0x41;

				// Store the MountPoint
				mount_points[mp_id] = mp;
			}
		}
	}
}

FS_STATUS _fs_create_partition(ATADevice *dev, Uint32 start, Uint32 size, Uint8 index) {
	// Detect stupidity errors
	if(index >= FS_PART_TABLE_ENTRIES) {
 		// Can't have > 4 partitions
		return FS_ERR_BADINDEX; 
	}
	if(size >= dev->size || size + start >= dev->size) { 
		// Can't partition off more sectors that the drive can hold
		return FS_ERR_TOOBIG; 
	}
	if(size <= 4) {
		// Can't have less sectors than required for metadata
		return FS_ERR_TOOSMALL;
	}
	if(start == SECT_MBR) {
		// Can't place a partition over the MBR
		return FS_ERR_BADSECT;
	}

	// Make the size divisible by 112
	if(size % FS_SECT_PER_IB != 0) {		
		// Size of partition is not divisible by 112. Make it so.
		size += FS_SECT_PER_IB - (size % FS_SECT_PER_IB);
		size++; // Making room for the BootRecord
	}

	// Read in the current MBR
	ATASector mbr;
	_ata_read_sector(dev, SECT_MBR, &mbr);

	// Verify that it's actually an MBR
	if(!(mbr[510] == 0x55 && mbr[511] == 0xAA)) {
		// It's not a valid MBR. We need to rewrite it.
		_ata_blank_sector(&mbr);
		mbr[510] = 0x55;	// The MBR signature
		mbr[511] = 0xAA;
	}

	// Now jump to the index of the partition
	Uint32 indexAddr = FS_PART_TABLE_OFF + index * FS_PART_TABLE_SIZE;

	// Write the type of the partition
	mbr[indexAddr + FS_PART_ENTRY_PT] = FS_PARTITION_TYPE;

	// Write the geometry of the partition
	_sector_put_long(&mbr, indexAddr + FS_PART_ENTRY_LBA, start);
	_sector_put_long(&mbr, indexAddr + FS_PART_ENTRY_SIZE, size);

	// Write the mbr back to the disk
	_ata_write_sector(dev, SECT_MBR, &mbr);
	return FS_SUCCESS;
}

FS_STATUS _fs_format(MountPoint *mp, ATADevice *dev, Uint8 index) {
	// Load up the mbr of the device
	ATASector mbr;
	_ata_read_sector(dev, SECT_MBR, &mbr);

	// Verify that the partition is DERP
	Uint32 indexAddr = FS_PART_TABLE_OFF + index * FS_PART_TABLE_SIZE;
	if(mbr[indexAddr + FS_PART_ENTRY_PT] != FS_PARTITION_TYPE) {
		return FS_ERR_NOTDERP;
	}

	// BOOT RECORD /////////////////////////////////////////////////////////
	// Grab the LBA for the partition
	Uint32 lba  = _sector_get_long(&mbr, indexAddr + FS_PART_ENTRY_LBA);
	Uint32 size = _sector_get_long(&mbr, indexAddr + FS_PART_ENTRY_SIZE);

	// Build a boot record for the partition
	ATASector br;
	_ata_blank_sector(&br);

	// Write the header of the br
	br[0]=0x44; br[1]=0x45; br[2]=0x52; br[3]=0x50;

	// Write the size of the partition to the br
	_sector_put_long(&br, FS_BR_SIZE, size);

	// Write out the bootrecord
	_ata_write_sector(dev, lba, &br);
	
	// INDEX BLOCKS ////////////////////////////////////////////////////////
	// Build an index block
	ATASector fsTable;
	ATASector nameTable;
	_ata_blank_sector(&fsTable);
	_ata_blank_sector(&nameTable);

	// Write the header of the index block
	fsTable[0]=0x49; fsTable[1]=0x42;

	// Write the header of the name table
	nameTable[0]=0x4E; nameTable[1]=0x41; nameTable[2]=0x4D; nameTable[3]=0x45;

	// These are written every FS_MOD_INDEXBLK sectors, offset by 1 for the mbr
	Uint32 i, count;
	for(i = 1; i < size; i += FS_SECT_PER_IB) {
		// Mark the first 3 sectors are in use
		Uint32 bits = 0x7;
		fsTable[FS_BT_END - 1] = bits;

		// Write the FSTable
		_ata_write_sector(dev, lba + i, &fsTable);

		// Write the blank name table
		_ata_write_sector(dev, lba + i + 1, &nameTable);
		_ata_write_sector(dev, lba + i + 2, &nameTable);
		count++;
	} 

	return FS_SUCCESS;
}

Uint32 _fs_find_empty_sector(MountPoint *mp) {
	// Start checking index blocks
	Uint32 i, j, byte;
	Uint32 end = mp->offset + mp->bootRecord.size;
	ATASector s;

	// Loop from the first index block to the last index block
	for(i = mp->offset + 1; i < end; i += FS_SECT_PER_IB) {
		// Load the sector
		_ata_read_sector(mp->device, i, &s);
		
		// Start iterating over the bits in the bitfield
		for(j = 0; j < FS_BT_SIZE * 8; j++) {
			// Which byte do we need 
			byte = FS_BT_OFFSET + (FS_BT_END - (FS_BT_OFFSET + (j / 8)));
			byte -= 1;

			// Grab that byte and check if the bit we need is allocated
			if(((s[byte] >> (j % 8)) & FS_BT_ALLOCATED) == 0) {
				// Sector isn't allocated. This is our golden nugget
				// Calculate the sector into the partition this is
				return (i - mp->offset) + j;
			}
		}
	}

	// If we make it here, there's no free sectors
	return 0x0;
}

/**
 * Finds the first empty file pointer in the filesystem. If one is
 * not found, the MountPoint (mp) of the returned FSPointer will be
 * NULL. There are no guarantees on the buffered sector in the FSPointer.
 * @param	MountPoint*	mp	The MountPoint to search for free FSPointers.
 * @return	FSPointer	A FSPointer representing the first free pointer
 *						index of the MountPoint. mp will be NULL if there
 *						are no FSPointers available.
 */
FILE _fs_find_empty_fspointer(MountPoint *mp) {
	// Start checking index blocks
	Uint32 i, j;
	Uint32 end = mp->offset + mp->bootRecord.size;
	ATASector s;
	FILE f;
	
	// Loop from the first index block to the last index block
	for(i = mp->offset + 1; i < end; i += FS_SECT_PER_IB) {
		// Load the sector
		_ata_read_sector(mp->device, i, &s);

		// Start iterating over the pointers in the table
		for(j = FS_FP_OFFSET; j < FS_FP_END; j += FS_FP_LENGTH) {
			if(_sector_get_long(&s, j) == FS_FP_FREE) {
				// This file pointer entry is free
				// Create a file to represent it and return it
				f.mp       = mp;
				f.ib       = i - mp->offset;
				f.ibindex  = (j - FS_FP_OFFSET) / FS_FP_LENGTH;
				f.bufsect  = 0;
				f.bufindex = 0;
				f.offset   = 0;
				f.code     = FS_SUCCESS_EMPTYFP;
				return f;
			}
		}
	}

	// If we make it here, there are no free pointers
	f.code = FS_ERR_NO_FP;
	return f;
}

/**
 * Searches for and deletes the requested filename from the provided MountPoint.
 * If the file cannot be found FS_ERR_FILENOTFOUND is returned.
 * @param	MountPoint*	mp			The mountpoint to delete the file from
 * @param	char[8]		filename	The name of the file to delete. Filename
 *									only please, no mountpoint letter.
 * @return	FS_STATUS	FS_SUCCESS on successful deletion. Anything else on
 *						failure.
 */
FS_STATUS _fs_delete_file(FILE *file) {
	// Verify this is a valid file
	if(file->code == FS_AVAILABLE) {
		return FS_ERR_BADFILE;
	}

	// Remove the name of the file by loading and removing the name table
	Uint32 sector = file->mp->offset + file->ib;
	sector += (file->ibindex >= FS_NAME_S1ENTRIES) ? 2 : 1;
	_ata_read_sector(file->mp->device, sector, &(file->buffer));

	Uint32 offset = file->ibindex;
	offset -= (file->ibindex >= FS_NAME_S1ENTRIES) ? FS_NAME_S1ENTRIES : 0;
	offset = (offset * FS_NAME_SIZE) + FS_NAME_OFFSET;
	_sector_put_long(&(file->buffer), offset, 0x0);
	_sector_put_long(&(file->buffer), offset + 4, 0x0);
	// IF THE SIZE EVER CHANGES, THESE LINES ARE BORKED.

	// Write the name table back to the disk
	_ata_write_sector(file->mp->device, sector, &(file->buffer));

	// Load the ib of the file
	// Using the file's buffer for temp actions
	_ata_read_sector(file->mp->device, file->mp->offset + file->ib, &(file->buffer));

	// @TODO: Verify it's an IB
	// Get the sector value from the table
	offset = (file->ibindex * FS_FP_LENGTH) + FS_FP_OFFSET;
	sector = _sector_get_long(&(file->buffer), offset);

	// Mark the filepointer as available
	_sector_put_long(&(file->buffer), offset, FS_FP_FREE);

	// Write it back to the disk
	_ata_write_sector(file->mp->device, file->mp->offset + file->ib, &(file->buffer));
	
	// Traverse the file chain's sector's and unallocate as we go
	while(sector != FS_FILE_EOC) {
		// Grab the sector
		_ata_read_sector(file->mp->device, file->mp->offset + sector, &(file->buffer));

		// Unallocate the sector
		_fs_unallocate_sector(file->mp, sector);

		// Go to next sector
		sector = _sector_get_long(&(file->buffer), FS_FILE_SECT_OFF);
	}

	// Mark the file pointer as available
	file->code = FS_AVAILABLE;

	return FS_SUCCESS;
}

FILE _fs_create_file(MountPoint *mp, char filename[8]) {
	// Get a free file pointer
	FILE file = _fs_find_empty_fspointer(mp);

	// Get a free sector 
	Uint32 sector = _fs_find_empty_sector(mp);

	// Make sure they're valid
	if(file.code == FS_ERR_NO_FP || sector == 0) { 
		file.code = FS_ERR_FULL;
		return file;
	}

	// Allocate the sector that's free
	_fs_allocate_sector(mp, sector);

	// Allocate the FSPointer ----------------------------------------------
	// Grab the ib sector for the fp
	// Using the file's buffer to save space for now
	ATASector *ibSector = &(file.buffer);
	_ata_read_sector(mp->device, mp->offset + file.ib, ibSector);

	// Put a pointer to the file's first sector
	Uint32 ibindexOff = (file.ibindex * FS_FP_LENGTH) + FS_FP_OFFSET;
	_sector_put_long(ibSector, ibindexOff, sector);

	// Write it back to the disk
	_ata_write_sector(mp->device, mp->offset + file.ib, ibSector);

	// Allocate the filename -----------------------------------------------
	// Using the buffer again to save space
	ATASector *nameSector = &(file.buffer);

	Uint32 nameSectNumber = file.ib;
	nameSectNumber += (file.ibindex >= FS_NAME_S1ENTRIES) ? 2 : 1;

	Uint32 index = file.ibindex;
	index -= (file.ibindex >= FS_NAME_S1ENTRIES) ? FS_NAME_S1ENTRIES : 0;

	// Read from the name sector
	_ata_read_sector(mp->device, mp->offset + nameSectNumber, nameSector);

	// Write the name to the sector
	Uint8 i;
	for(i = 0; i < 8; i++) {
		(*nameSector)[FS_NAME_OFFSET + (index * FS_NAME_SIZE) + i] = filename[i];
	}

	// Write the sector back to the disk
	_ata_write_sector(mp->device, mp->offset + nameSectNumber, nameSector);

	// Allocate 0th Sector of File -----------------------------------------
	// Grab the sector that the file will start on
	// This is what the buffer will stay at
	ATASector *fileSector = &(file.buffer);
	_ata_blank_sector(fileSector);

	// Just set the header and write it back to the disk
	(*fileSector)[0]=0x46;(*fileSector)[1]=0x49;(*fileSector)[2]=0x4C;(*fileSector)[3]=0x45;
	_ata_write_sector(mp->device, mp->offset + sector, fileSector);

	// Store the filesector as the bufferred sector
	file.bufsect  = sector;
	file.bufindex = 0;

	// Return a FILE pointer -----------------------------------------------
	file.offset = 0;
	
	// @TODO: Find errors
	return file;
}

FILE _fs_find_file(MountPoint *mp, char filename[8]) {
	// Start iterating over all the index blocks in the partition
	//Uint32 start = mp->offset + 1;
	//Uint32 end   = mp->offset + mp->bootRecord.size;
	Uint32 i, j, offset, sector;
	FILE file;
	file.bufsect = 0;
	// We'll use the file's buffer for storing temp data

	// Copy the name of the file
	for(i = 0; i < FS_NAME_SIZE; i++) {
		file.name[i] = filename[i];
	}
	
	// Prebuild the file pointer to save code
	file.mp = mp;

	// Iterate over the index blocks
	for(i = 0; i <= mp->bootRecord.size; i += FS_SECT_PER_IB) {
		// Iterate over the total number of files per index block
		for(j = 0; j < FS_SECT_PER_IB; j++) {
			// Calculate the offset to read and the sector to read it from
			offset = (j > FS_NAME_S1ENTRIES) ? j - FS_NAME_S1ENTRIES - 1 : j;
			offset *= FS_NAME_SIZE;
			offset += FS_NAME_OFFSET;

			// Calculate the sector to read it from
			sector = i + mp->offset + 1;
			sector += (j > FS_NAME_S1ENTRIES) ? 2 : 1;

			// Do we need to read it in?
			if(file.bufsect != sector) {
				// Reading in a new sector
				_ata_read_sector(mp->device, sector, &(file.buffer));
				file.bufsect = sector;
			}

			// Are the names the same?
			if(_fs_namecmp(&(file.buffer), offset, filename) == 0) {
				// We have a match!
                _ata_read_sector(mp->device, mp->offset + i + 1, &(file.buffer));
                Uint32 fsect = _sector_get_long(&(file.buffer), FS_FP_OFFSET + (j * FS_FP_LENGTH));
                
                // Grab the first sector of the file for buffering
                _ata_read_sector(mp->device, mp->offset + fsect, &(file.buffer));

                // Return the found file
                file.ib       = i / FS_SECT_PER_IB +1;
                file.ibindex  = j;
                file.bufsect  = fsect;
                file.bufindex = 0;
                return file;
			}
		}
	}
	
	// We didn't find it
	file.code = FS_ERR_FILENOTFOUND;
	return file;
}

int _fs_namecmp(ATASector *sect, Uint16 index, char name[8]) {
	// Iterate over the chars and see if they match
	Uint8 i;
	for(i = 0; i < FS_NAME_SIZE; i++) {
		if((*sect)[index + i] != name[i]) {
			return -1;
		}
	}

	// They match!
	return 0;
}

/**
 * Given a filepointer, figure out what the size of the file is
 * @param	FSPointer*	fp	A file pointer to the file we want to size
 * @return	Uint32	The size of the file in bytes (up to 3.9999gb)
 */
Uint32 _fs_get_file_size(FILE *file) {
	// Check for sanity
	if(file->mp == NULL) {
		return 0;
	}

	// NOTE: We are leaving the file's buffer in tact to avoid having to
	// flush, trash it, and reload it. However, we're only allocating one
	// sector and reusing it to save stack space.

	// Get the sector of the ib for this file
	ATASector sect;
	_ata_read_sector(file->mp->device, file->mp->offset + file->ib, &sect);
	if(sect[0] != 'I' || sect[1] != 'B') {
		__panic("NOT AN IB!!");
	}

	// Read the sector number for the file
	Uint32 sector = _sector_get_long(&sect, FS_FP_OFFSET + (file->ibindex * FS_FP_LENGTH));
	Uint32 size = 0;

	// Read the sector of the file
	while(sector != FS_FILE_EOC) {
		// Read the sector
		_ata_read_sector(file->mp->device, file->mp->offset + sector, &sect);

		// Add the bytes allocated
		if(sector == file->bufsect) {
			size += _sector_get_long(&(file->buffer), FS_FILE_BYTE_OFF);
		} else {
			size += _sector_get_long(&sect, FS_FILE_BYTE_OFF);
		}

		// Get the next sector number
		sector = _sector_get_long(&sect, FS_FILE_SECT_OFF);
	}

	return size;
}

/**
 * Marks a sector on a given mountpoint as allocated. Chains into
 * toggle_sector.
 * @param	MountPoint*	mp		The mountpoint of the sector
 * @param	Uint32		sector	The sector to allocate
 */
void _fs_allocate_sector(MountPoint *mp, Uint32 sector) {
	_fs_toggle_sector(mp, sector);
}

/**
 * Marks a sector on a given mountpoint as unallocated. Chains into
 * toggle_sector.
 * @param	MountPoint*	mp		The mountpoint of the sector
 * @param	Uint32		sector	The sector to allocate
 */
void _fs_unallocate_sector(MountPoint *mp, Uint32 sector) {
	_fs_toggle_sector(mp, sector);
}

/**
 * Marks a sector on a given mountpoint as allocated. Toggles the sector
 * by xor'ing the bit that marks the sector's allocation.
 * @param	MountPoint*	mp		The mountpoint of the sector
 * @param	Uint32		sector	The sector to toggle
 */
void _fs_toggle_sector(MountPoint *mp, Uint32 sector) {
	// For our math we're zero indexing the sectors.
	sector--;

	// Read the ib sector for the bitfield
	Uint32 ibAddr = mp->offset + ((sector / FS_SECT_PER_IB) * FS_SECT_PER_IB) + 1;
	ATASector s;
	_ata_read_sector(mp->device, ibAddr, &s);

	// Toggle the appropriate sector bit
	Uint8 mask  = FS_BT_ALLOCATED << ((sector % FS_SECT_PER_IB) % 8);
	Uint16 byte = FS_BT_END - ((sector % FS_SECT_PER_IB) / 8) - 1;
	s[byte] ^= mask;

	// Write it back to the disk
	_ata_write_sector(mp->device, ibAddr, &s);
}

FS_STATUS _fs_allocate_filepointer(const FILE *source, FILE **dest) {
	// Seek through the file pointers to find the available ones
	Uint8 i;
	for(i = 0; i < FS_MAX_FILEPOINTERS; i++) {
		if(file_pointers[i].code == FS_AVAILABLE) {
			// This is the free fs pointer we deserve
			// Copy the data from source into the free pointer
			file_pointers[i].mp = source->mp;
			file_pointers[i].ib = source->ib;
			file_pointers[i].ibindex = source->ibindex;
			file_pointers[i].bufsect = source->bufsect;
			file_pointers[i].bufindex = source->bufindex;
			file_pointers[i].offset = source->offset;
			file_pointers[i].code = source->code;

			// Copy the buffer into the pointer
			_fs_copy_sector(&(source->buffer), &(file_pointers[i].buffer));

			// Give back the pointer to the file
			*dest = &file_pointers[i];

			return FS_SUCCESS;
		}
	}

	// If we made it here, none of the file pointers were free
	return FS_ERR_NO_FP;
}

FS_STATUS _fs_unallocate_filepointer(FILE *file) {
	// Theoretically, the file we have here will ALWAYS be an address into
	// the global array. If it isn't we dun goof'd, hardcore. Therefore, we
	// really only need to change the status of the file we were given...
	// No traversal required!
	file->code = FS_AVAILABLE;

	return FS_SUCCESS;
}

FS_STATUS _fs_file_inuse(FILE *file) {
	// Step 1) Is the file one in one of the file pointers?
	Uint8 i;
	for(i = 0; i < FS_MAX_FILEPOINTERS; i++) {
		// Do the mountpoints match/do the filenames match?
		if(file->mp == file_pointers[i].mp) {
			// Mountpoints match. Do the filenames?
			Uint8 j;
			for(j = 0; j < FS_NAME_FILENAME; j++) {
				if(file->name[j] != file_pointers[i].name[j]) {
					break;
				}
			} 

			// Still don't know... 
			if(j == FS_NAME_FILENAME - 1) {
				// Filenames match. File's in use.
				return FS_ERR_FILE_INUSE;
			}
		}
	}

	// Step 2) Is the file tagged as in use on the drive?
	// Seriously... I don't give a damn about checking this.

	return FS_AVAILABLE;
}

int _fs_is_free_filepointer(void) {
	// Iterate over the filepointers to find an available one
	Uint8 i;
	for(i = 0; i < FS_MAX_FILEPOINTERS; i++) {
		if(file_pointers[i].code == FS_AVAILABLE) {
			return 1;	// There is a free pointer
		}
	}

	return 0;	// No free pointers here
}	

void _fs_copy_sector(const ATASector *source, ATASector *dest) {
	// Start copying bytes
	Uint16 b;
	for(b = 0; b < FS_SECTOR_SIZE; b++) {
		(*dest)[b] = (*source)[b];
	}
}

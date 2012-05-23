fs - DERP OS man page

Name -------------------

fs - DERP_FS Filesystem Definitions

Synopsis ---------------

#include "fs.h"

Description ------------

The DERP_FS filesystem is defined through the lines of this file. A DERP_FS
partition can be broken down into four parts.

Each sector of a DERP_FS partition is 512 bytes long. ALL NUMBERS STORED IN
DERP_FS ARE LITTLE-ENDIAN. A DERP_FS partition is identified as type 0xDE on a
master boot record.

The BootRecord ----

The boot record is the first sector of the partition. If is defined by the
first four bytes of the sector 'D''E''R''P'. This is followed by 4 bytes of
alignment bytes followed by the size of the partition in a 32-bit unsigned
integer. The rest of the bytes up to 512 are reserved for future use.

The IndexBlock ----

The index block immediately follows the boot record sector and is identified
by the first two bytes 'I''B'. The following 112 bits are used as a BIG-ENDIAN
bitmap for the allocation of the next 112 sectors of the filesystem. This bit-
map also includes this sector and the next two.

The next 112 32-bit doublewords are used to point to the sectors of files in
the filesystem. The value of the integer is the sector number, relative to the
start of the filesystem.

This sector and the next two nametables are interspersed every 112 sectors of
the filesystem.

The NameTable -----

The NameTable is a set of two sectors that are used to store the names of the
next 112 files in the filesystem. The sectors can be identified by a header in
the first four bytes of 'N''A''M''E'. The filenames are all 8 characters. The
index into the list of filenames coorespond to the index of the file in the
previous 

There are 56 files per NameTable sector.

The FileBlock -----

The fileblock is composed of a header of 'F''I''L''E', a 32-bit integer of the
next sector of the file (like a linked list 'next' pointer) or 0x0 to indicate
the end of the chain, and a 32-bit integer to indicate the number of bytes
allocated in this sector.

Dependencies -----------

ata.h, headers.h

List of Functions ------

_fs_unallocate_filepointer() - Deallocates a filepointer
_fs_allocate_filepointer() - Allocates a filepointer
_fs_copy_sector() - Copies a sector
_fs_create_file() - Deletes a file from the filesystem
_fs_create_partition() - Creates a partition on the master boot record
_fs_delete_file() - Deletes a file from the filesystem
_fs_file_inuse() - Determines if the file is in use
_fs_find_empty_fspointer() - find the first fspointer entry in the index blocks
_fs_find_empty_sector() - find the first unallocated sector of the MountPoint
_fs_find_file() - Finds a file in the filesystem
_fs_format() - Format a partition with DERP_FS
_fs_get_file_size() - Finds the size of a file
_fs_init() - Initialize global ATA devices
_fs_is_free_filepointer() - Determines if there are any free filepointers
_fs_probe() - Initialize global ATA devices
_fs_toggle_Sector() - Toggles the status of a sector
_fs_allocate_sector() - Allocates a sector
_fs_unallocate_sector() - Unallocates a sector

Global Variables -------

MountPoint mount_points[26] - An array of Mountpoints for use in holding
pointers to filesystems on the disks. There is one for every letter of the ABCs
 
Uint8 mount_point_count - The number of mountpoints currently mounted

FILE file_pointers[FS_MAX_FILEPOINTERS] - A global collection of pointers to
FILE structs.

Data Structures --------

The following data structures are defined with typedefs

FILE struct
ATASector buffer - A buffer of 512 bytes
MountPoint *mp - Pointer to the mountpoint the file lives on
Uint32 ib - The sector number (relative to partition) of the index block the file is indexed on
Uint32 ibindex - Index into the index block of the file
Uint32 bufindex - The sector number (relative to the file) of the buffered sector
Uint32 bufsect - The sector number (relative to partition) of the buffered sector
Uint32 offset - Byte offset into the file.
char name[8] - The name (for identification in the global array
FS_STATUS code - Status code for the file

MountPoint struct
ATADevice *device - Pointer to the device that the partition is on
BootRecord bootRecord - The boot record of the partition
Uint32 offset - The offset into the drive where the partition begins
char letter - The mount point. A-Z

BootRecord struct
Uint16 header - If it's DERP it should be DERP
char alignment[4] - 0x00000000 for alignment in memory
Uint32 size - Size in sectors
char reserved[496] - Reserved for future use

Constants --------------

SECT_MBR - The sector of the master boot record (MBR) of a hard drive

FS_MAX_FILEPOINTERS - The maximum number of file pointers that can be open at
	a time.
FS_PART_TABLE_OFF - The offset into the MBR of the partition table
FS_PART_TABLE_SIZE - The size of the MBR partition table entry in bytes
FS_PART_TABLE_ENTRIES - The number of entries in a MBR partition table

FS_PART_ENTRY_PT - Offset into a MBR partition table entry for the partition type
FS_PART_ENTRY_LBA - Offset into a MBR partition entry for the partition start LBA
FS_PART_ENTRY_SIZE - Offset into a MBR partition entry for the partition size

FS_PARTITION_ID	- The bootrecord header
FS_PARTITION_IB_ID - The index block header 
FS_PARTITION_TYPE - The partition type for an MBR partition table entry

FS_SECTOR_SIZE - The size of sectors of a hard drive
FS_SECT_PER_IB - The number of sectors that are indexed by an index block
FS_BR_SIZE - Offset into the bootrecord of the size of the partition

FS_BT_SIZE - Size of the indexblock bitmap
FS_BT_OFFSET - Offset into the indexblock of the bitmap
FS_BT_END - The end of the bitmap in the indexblock
FS_BT_ALLOCATED - Mask for allocated sectors in the indexblock bitmap
FS_BT_FREE - Mask for free sectors in the indexblock bitmap

FS_FP_SIZE - The size all the filepointers in the indexblock
FS_FP_LENGTH - The size of an individual filepointer in the indexblock
FS_FP_OFFSET - The offset of the filepointers in an indexblock
FS_FP_END - The end of the filepointers in an indexblock
FS_FP_FREE - The value of an unallocated filepointer in a indexblock

FS_NAME_SIZE - The size of the name of a file
FS_NAME_OFFSET - Offset into a nametable for names
FS_NAME_S1ENTRIES - How many names are in the first sector of nametable
FS_NAME_S2ENTRIES - How many names are in the second sector of nametable
FS_NAME_FILENAME - The length of a filename in bytes

FS_FILE_DATA_OFF - Offset into a fileblock of the data 
FS_FILE_DATA_LENGTH - The number of bytes of data in the fileblock
FS_FILE_SECT_OFF - Offset into a fileblock of the next sector of the file
FS_FILE_BYTE_OFF - Offset into a fileblock for the bytes allocated in the file
FS_FILE_EOC - The value to indicate the end of the file chain

Enums ------------------

FS_STATUS
	FS_SUCCESS - Success
	FS_SUCCESS_EMPTYFP - Successfully found an empty filepointer in an IB
	FS_SUCCESS_NEWFILE - Successfully created a new file
	FS_ERR_TOOBIG - Sector count is too large for the drive
	FS_ERR_TOOSMALL - Sector count is too small for partition
	FS_ERR_BADINDEX - Index of file is invalid or incorrect
	FS_ERR_BADSECT - Sector is ivalid or incorrect
	FS_ERR_BADFILE - FILE struct is unacceptable for purpose
	FS_ERR_NOTDERP - The partition is not initialized for DERP_FS
	FS_ERR_FILENOTFOUND - File could not be found in partition
	FS_ERR_FULL - The partition is full
	FS_ERR_NO_FP - There are no available filepointers in the filesystem
	FS_ERR_FILE_INUSE - The file is currently in use
	FS_INVALID_FILENAME - The filename is invalid
	FS_INVALID_OFFSET - The offset into the file is invalid
	FS_AVAILABLE - The FILE struct is available



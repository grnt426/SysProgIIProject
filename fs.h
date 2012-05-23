////////////////////////////////////////////////////////////////////////////
// DERP_OS - DERP_FS Header
// 
// @file	fs.h
// @descrip	This header declares all constants needed for the DERP filesystem
//			includeing mod constants and size restrictions. This header also
//			declares functions necessary for the creation of a filesystem,
//			reading to the filesystem, and writing to the filesystem.
// @author	Benjamin Russell
////////////////////////////////////////////////////////////////////////////

#ifndef DERP_FS_H
#define DERP_FS_H

#include "headers.h"
#include "ata.h"

// CONSTANTS ///////////////////////////////////////////////////////////////
#define SECT_MBR				0x0

// Maximums for globals
#define	FS_MAX_FILEPOINTERS		50

// Partition table info
#define FS_PART_TABLE_OFF		0x01BE
#define FS_PART_TABLE_SIZE		16				// bytes
#define FS_PART_TABLE_ENTRIES	0x04
#define FS_PART_ENTRY_PT		0x04			// Offset of the partition type
#define FS_PART_ENTRY_LBA		0x08			// Offset of LBA of first sect
#define FS_PART_ENTRY_SIZE		0x0C			// Offset of number of sectors

// DERP_FS identification info
#define FS_PARTITION_ID			0x44455250		// DERP
#define FS_PARTITION_IB_ID		0x4942			// IB
#define FS_PARTITION_TYPE		0xDE			// Technically Dell diagnostics

#define	FS_SECTOR_SIZE			512
#define FS_SECT_PER_IB			112

// Boot Record Offsets
#define FS_BR_SIZE				0x8

// BitTable Definitions
#define FS_BT_SIZE				0x0E
#define FS_BT_OFFSET			0x02
#define FS_BT_END				0x10
#define FS_BT_ALLOCATED			0x01
#define FS_BT_FREE				0x00

// FilePointer Table Definitions
#define FS_FP_SIZE				0x1F0			// 496 bytes of pointers
#define FS_FP_LENGTH			0x04			// 4 bytes per pointer
#define FS_FP_OFFSET			0x10			// Starts at byte 16
#define FS_FP_END				0x1D0			// Ends at byte 464
#define FS_FP_FREE				0x00

// NameTable definitions
#define FS_NAME_SIZE			0x8				// 8 bytes of file name
#define FS_NAME_OFFSET			0x4				// 4 bytes of header
#define FS_NAME_S1ENTRIES		0x38			// 38 Entries on sector 1
#define FS_NAME_S2ENTRIES		0x38			// 38 Entries of sector 2
#define FS_NAME_FILENAME		0x8				// Filename is 8 bytes
#define FS_NAME_HEADER			0x4E414D45		// NAME

// File sector information
#define FS_FILE_DATA_OFF		0x0C			// Data starts at 12 bytes in
#define FS_FILE_DATA_LENGTH		0x01f4			// 500 bytes of data
#define FS_FILE_SECT_OFF		0x04			// Next sector is 4 bytes in
#define FS_FILE_BYTE_OFF		0x08			// Bytes allocated in sector
#define FS_FILE_HEADER			0x46494C45		// FILE
#define FS_FILE_EOC				0x00			// End of the chain

// Success/Error Codes
typedef enum {
	FS_SUCCESS = 0x00,
	FS_SUCCESS_EMPTYFP,
	FS_SUCCESS_NEWFILE,
	FS_ERR_TOOBIG,
	FS_ERR_TOOSMALL,
	FS_ERR_BADINDEX,
	FS_ERR_BADSECT,
	FS_ERR_BADFILE,
	FS_ERR_NOTDERP,
	FS_ERR_FILENOTFOUND,
	FS_ERR_FULL,
	FS_ERR_NO_FP,
	FS_ERR_FILE_INUSE,
	FS_INVALID_FILENAME,
	FS_INVALID_OFFSET,
	FS_AVAILABLE
} FS_STATUS;

// TYPEDEFS ////////////////////////////////////////////////////////////////

/**
 * The boot record of the device
 * Word-------- Definition---------
 * 0			Hard coded to 0x44455250 ("DERP")
 * 1			Hard coded to 0x00000000 for alignment
 * 3:2			Size in sectors (multiply by FS_SECTOR_SIZE for bytes)
 * 255:4		Reserved.
 */
typedef struct {
	Uint16	header;			// If it's DERP it should be DERP
	char	alignment[4];	// 0x00000000 for alignment in memory
	Uint32	size;			// Size in sectors
	char	reserved[496];	// Reserved for future use
} BootRecord;

/**
 * MountPoint - Points to a ATADevice, the offset into the drive where the
 * partition starts at, and stores the BootRecord of the partition.
 */
typedef struct {
	ATADevice 	*device;	// Pointer to the device that the partition is on
	BootRecord	bootRecord;	// The boot record of the partition
	Uint32		offset;		// The offset into the drive where the partition
							// begins
	char		letter;		// The mount point. A-Z
} MountPoint;

/**
 * FILE - A representation of a file in the filesystem.
 */
typedef struct {
	ATASector	buffer;		// A buffered sector of the hard drive
	MountPoint	*mp;		// Pointer to the mountpoint the file lives on
	Uint32		ib;			// The sector number (relative to partition) of the
							// index block the file is indexed on
	Uint32		ibindex;	// Index into the index block of the file
	Uint32		bufindex;	// The sector number (relative to the file) of the
							// buffered sector
	Uint32		bufsect;	// The sector number (relative to partition) of the
							// buffered sector
	Uint32		offset;		// Byte offset into the file.
	char		name[8];	// The name (for identification in the global array
	FS_STATUS	code;		// Status code for the file
} FILE;

// GLOBALS /////////////////////////////////////////////////////////////////
// We can only do A-Z (26) mount points
MountPoint mount_points[26];

// The current number of mount points
Uint8 mount_point_count;

// We can only have up to 50 files open at the same time
FILE file_pointers[FS_MAX_FILEPOINTERS];

// FUNCTIONS ///////////////////////////////////////////////////////////////
// Initialization
void _fs_init(void);
void _fs_probe(ATADevice *dev);
FS_STATUS _fs_create_partition(ATADevice *dev, Uint32 start, Uint32 size, Uint8 index);
FS_STATUS _fs_format(MountPoint *mp, ATADevice *dev, Uint8 index);

// File creation/deletion
FILE _fs_create_file(MountPoint *mp, char filename[8]);
FS_STATUS _fs_delete_file(FILE *file);

// Helper functions
Uint32 _fs_find_empty_sector(MountPoint *mp);
FILE _fs_find_empty_fspointer(MountPoint *mp);
FILE _fs_find_file(MountPoint *mp, char filename[8]);
void _fs_allocate_sector(MountPoint *mp, Uint32 sector);
void _fs_unallocate_sector(MountPoint *mp, Uint32 sector);
void _fs_toggle_sector(MountPoint *mp, Uint32 sector);
int _fs_namecmp(ATASector *sect, Uint16 index, char name[8]);
Uint32 _fs_get_file_size(FILE *file);
FS_STATUS _fs_allocate_filepointer(const FILE *source, FILE **dest);
FS_STATUS _fs_unallocate_filepointer(FILE *file);
int _fs_is_free_filepointer(void);
FS_STATUS _fs_file_inuse(FILE *file);
void _fs_copy_sector(const ATASector *source, ATASector *dest);

#endif

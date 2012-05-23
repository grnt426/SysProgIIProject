////////////////////////////////////////////////////////////////////////////
// DERP_OS - Userspace Filesystem Header
//
// @descrip	Declares all the functions for manipulating the filesystem from
//			userspace. These functions allow the user to create, delete,
//			read and write to files.
// @file	ufs.h
// @author	Benjamin Russell
////////////////////////////////////////////////////////////////////////////

#ifndef		DERP_UFS
#define		DERP_UFS

// REQUIRED HEADERS ////////////////////////////////////////////////////////
#include "fs.h"

// CONSTANTS ///////////////////////////////////////////////////////////////
// Seek start points
typedef enum {
	FS_SEEK_ABS,
	FS_SEEK_REL,
	FS_SEEK_REL_REV
} FS_FILE_SEEK;

// FUNCTIONS ///////////////////////////////////////////////////////////////

// Open/Create/Close/Delete
FILE *fopen(char filepath[10]);
FS_STATUS fclose(FILE *file);
FS_STATUS fdelete(FILE *file);

// Reading/Writing
FS_STATUS fseek(FILE *file, Uint32 offset, FS_FILE_SEEK dir);
Uint32 fread(FILE *file, char *buffer, Uint32 size);
Uint32 fwrite(FILE *file, char *buffer, Uint32 size);
FS_STATUS fflush(FILE *file);

// OH GOD.
FILE *fnamefile(const char mountpoint);

#endif

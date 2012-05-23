ufs - DERP OS man page

Name -------------------

ufs - userspace filesystem libraries

Synopsis ---------------

#include "ufs.h"

Description ------------

The userspace filesystem library provides a simple an (somewhat) efficient
buffered interface to accessing to files in multiple filesystems across
multiple hard drives. The system relies on a globally allocated array of FILE
structs that contain information about the file and the status of the file
within the system. The struct also contains information to tie the system
back to a partition which ultimately ties it back to a physical hard drive.

A file can be opened or created by using the fopen() function. This allocates
a filepointer for the file and reserves the file such that it cannot be used
by other processes in the system.

At startup, FS_MAX_FILEPOINTERS, are declared so processes can call functions
from this library to initialize the file pointers. 

When done with a file, it can either be passed to fclose() or fdelete() to
close or delete the file from the filesystem, respectively. In either case
the file pointer is deallocated from the system, allowing further calls to the
filesystem to utilize files. 

Note: If the FILE struct is not closed properly after use, the global pointer
will become unavailable until the next system reboot.

Dependencies -----------

fs.h

List of Functions ------

fclose() - close a file pointer and deallocate it

fdelete() - delete a file and unallocate it

fflush() - flush a file pointer

fnamefile() - creates a file of mountpoint files and file sizes

fopen() - file pointer open functions

fread() - read bytes from a file into a buffer

fseek() - reposition a filepointer

fwrite() - read bytes from a file into a buffer

Data Structures --------

[ None ]

Constants --------------

[ None ]

Enums ------------------

FS_FILE_SEEK - Directions to seek when calling fseek
	FS_SEEK_ABS - Seek to an absolute position in the file
	FS_SEEK_REL - Seek to a position relative to the current offset
	FS_SEEK_REL_REV - Seek backwards from the current offset


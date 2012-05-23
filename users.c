/*
** SCCS ID:	@(#)users.c	1.1	4/5/12
**
** File:	user.c
**
** Author:	4003-506 class of 20133
**
** Contributor:
**
** Description:	User routines.
*/

#include "headers.h"

#include "users.h"
#include "ufs.h"
#include "string.h"
#include "mouse.h"
#include "gl.h"
#include "gl_print.h"
#include "syscalls.h"

void fileshell(void) {

//	draw_rect(0,0,100,100, ((pixel){0xff,0xff,0xff,0xff}));

	Uint16 i;
	
	char buffer[65];
	char s_buf[2];

	// Loop indefinitely
	while(1) {
		// Print a prompt
		gl_puts("DERP_FS Shell> ");
		Pid p;
		get_pid( &p );
		// Read a buffer from the user
		read_buf(buffer, 64);

		// Figure out which command to execute
		char *command = strtok(buffer, " ");

		// BIG ASS SWITCH ON THE COMMAND
		if(strncmp(command, "touch", 20) == 0) {
			write('t');
			// TOUCH -------------------------------------------------------
			// Figure out the name of the file
			char *filename = strtok(NULL, " ");
			if(filename == NULL) {
				gl_puts("*** You must provide a filename\n");
				continue;
			}
			if(filename[1] != ':') {
				gl_puts("*** Invalid filename. X:yyyyyyyy\n");
				continue;
			}

			char filen[10];
            for(i = 0; i < 10 && filename[i] != 0x0; i++) { 
                filen[i] = filename[i]; 
            }
            for(; i < 10; i++) {
                filen[i] = 0x0;
            }

			// Open the file (aka create it)
			FILE *f = fopen(filen);
			if(f == NULL) {
				gl_puts("*** Touch failed!\n");
				continue;
			} else if(f->code == FS_SUCCESS) {
				gl_puts("--- File already exists\n");
			} else if(f->code != FS_SUCCESS_NEWFILE) {
				gl_printf("*** fopen failed with code 0x%x\n");
				continue;
			}

			// Close the file to free the pointer
			fclose(f);

		} else if(strncmp(command, "rm", 20) == 0) {
			write('r');
			// RM ----------------------------------------------------------
			// Figure out the name of the file
			char *filename = strtok(NULL, " ");
			if(filename == NULL) {
				gl_puts("*** You must provide a filename!\n");
				continue;
			}
			if(filename[1] != ':') {
				gl_puts("*** Invalid filename. X:yyyyyyyyy\n");
				continue;
			}

			char filen[10];
			for(i = 0; i < 10 && filename[i] != 0x0; i++) { 
				filen[i] = filename[i]; 
			}
			for(; i < 10; i++) {
				filen[i] = 0x0;
			}

			// Open the file (or create it, no big deal)
			FILE *f = fopen(filen);
			if(f == NULL) {
				gl_puts("*** RM Failed!\n");
				continue;
			} else if(f->code == FS_SUCCESS_NEWFILE) {
				gl_puts("*** File does not exist!\n");
			}

			// Delete the file (if it was created, we delete it as well!)
			fdelete(f);
			
		} else if(strncmp(command, "ls", 20) == 0) {
			write('l');
			// LS ----------------------------------------------------------
			// Figure out which mountpoint we want to read
			char *mountpoint = strtok(NULL, " ");
			if(mountpoint == NULL) {
				gl_puts("*** You must provide a MountPoint to print the files of\n");
				continue;
			}
			if(strlen(mountpoint) != 1 ||((*mountpoint)-0x41)>mount_point_count){
				gl_puts("*** Invalid mountpoint.\n");
				continue;
			}
			
			gl_printf("MountPoint '%c' Contents:\n", *mountpoint);

			// Tell the filesystem to generate a name file
			FILE *nameFile = fnamefile(*mountpoint);
			char buffer[12];
			while(fread(nameFile, buffer, 12) == 12) {
				// NOTE: This will report files > 2^16 bytes as negative
				gl_printf("%c%c%c%c%c%c%c%c    %-db\n", 
					buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],
					buffer[5],buffer[6],buffer[7],
					buffer[8]|buffer[9]<<8|buffer[10]<<16|buffer[11]<<24);
			}
			
			// Delete the filename
			fdelete(nameFile);

		} else if(strncmp(command, "cat", 20) == 0) {
			write('c');
			// CAT ---------------------------------------------------------
			// Figure out which file to open
			char *filename = strtok(NULL, " ");
			if(filename == NULL) {
				gl_puts("*** You must provide a file to print\n");
				continue;
			}
			if(filename[1] != ':') {
				gl_puts("*** Invalid Filename. X:yyyyyyyy\n");
				continue;
			}

			char filen[10];
            for(i = 0; i < 10 && filename[i] != 0x0; i++) { 
                filen[i] = filename[i]; 
            }
            for(; i < 10; i++) {
                filen[i] = 0x0;
            }

			// Load the file and print its characters
			FILE *f = fopen(filen);
			char buf[1];

			while(fread(f, buf, 1) == 1) {
				gl_printf("%c", *buf);
			}

			gl_puts("\n");

		} else if(strncmp(command, "write", 20) == 0) {
			write('w');
			// WRITE ------------------------------------------------------
			// Figure out which file to open
			char *filename = strtok(NULL, " ");
			char *offsetc = "-0";
			if(filename == NULL) {
				gl_puts("*** You must provide a file to write to!\n");
				continue;
			}
			if(filename[0] == '-') {
				// We're declaring an offset instead of a filename
				offsetc = filename;
				filename = strtok(NULL, " ");
				if(filename == NULL) {
					gl_puts("*** You must provide a file to write to!\n");
					continue;
				}
			}
			if(filename[1] != ':') {
				gl_printf("*** Invalid filename. X:YYYYYYYY %d\n", strlen(filename));
				continue;
			}

			// Process the offset
			Uint32 offset = atoi(offsetc + 1);

			char filen[10];
            for(i = 0; i < 10 && filename[i] != 0x0; i++) { 
                filen[i] = filename[i]; 
            }
            for(; i < 10; i++) {
                filen[i] = 0x0;
            }

			// Load the file
			FILE *file = fopen(filen);
			if(file == NULL) {
				gl_puts("*** Could not open file!\n");
				continue;
			}

			// Seek to the offset
			if(fseek(file, offset, FS_SEEK_ABS) != FS_SUCCESS) {
				gl_puts("*** Invalid offset into file\n");
				continue;
			}

			// Loop until the end of input
			
			char c = 'q';
			char mod = 0;
			Uint32 bytes = 0;
			gl_puts("\n");
			while(1) {
				// Get a character
				read_char( s_buf );
				c = s_buf[ 1 ];
				mod = s_buf[ 0 ];
				
				// Will it terminate input?
				if((mod & 1) && c == 'd') { 
					break;
				} else if(c == '\n') {
					// Emit the newline
					gl_puts("\n");
				} else {
					// Echo it back
					gl_printf("%c", c);
				}

				// It didn't terminate input so print dump it to the file
				bytes += fwrite(file, &c, 1);
			}

			// Output the number of bytes we wrote
			gl_puts("\n--------------\n");
			gl_printf("Wrote %d bytes\n", bytes);

			fclose(file);

		} else if(strncmp(command, "drives", 20) == 0) {
			write('d');
			// DRIVES ------------------------------------------------------
			// We're breaking all the rules. Iterate over the drives found
			for(i = 0; i < ata_device_count; i++) {
				gl_printf("Drive %d: %s 0x%x sectors (512b)\n",
					i, 
					ata_devices[i].model,
					ata_devices[i].size
					);
			}

		} else if(strncmp(command, "part", 20) == 0) {
			write('p');
			// PART --------------------------------------------------------
			// Grab the drive to partition
			char *drivec = strtok(NULL, " ");
			if(drivec == NULL) {
				gl_puts("*** You must include a drive id\n");
			}
			Uint8 drive = atoi(drivec);
			if(drive > ata_device_count) {
				gl_puts("*** Invalid drive id!\n");
				continue;
			}

			// Grab the partition number
			char *partitionc = strtok(NULL, " ");
			if(partitionc == NULL) {
				gl_puts("*** You must include a partition index 1-4\n");
				continue;
			}
			Uint8 index = atoi(partitionc) - 1;
			if(index > 4) {
				gl_puts("*** Invalid partition index!\n");
				continue;
			}

			// Grab the starting sector of the partition
			char *startc = strtok(NULL, " ");
			if(startc == NULL) {
				gl_puts("*** You must include a starting sector for the partition\n");
				continue;
			}
			Uint32 start = atoi(startc);
			if(start == 0) {
				gl_puts("*** You cannot overwrite the master boot record!\n");
				continue;
			}

			// Grab the size of the partition in sectors
			char *sectc = strtok(NULL, " ");
			if(sectc == NULL) {
				gl_puts("*** You must include a partition size in sectors\n");
				continue;
			}
			Uint32 sect = atoi(sectc);
			
			// Call the partitioner
			Uint8 result = _fs_create_partition(&ata_devices[drive], 
				start, sect, index);
			if(result != FS_SUCCESS) {
				gl_printf("*** Partition failed with code 0x%x\n");
			}	

		} else if(strncmp(command, "format", 20) == 0) {
			write('f');
			// FORMAT ------------------------------------------------------
			// Grab the drive to format
			char *drivec = strtok(NULL, " ");
			if(drivec == NULL) {
				gl_puts("*** You must specify a drive for the partition!\n");
			}
			Uint8 drive = atoi(drivec);
			if(drive > ata_device_count) {
				gl_puts("*** Invalid drive id!\n");
				continue;
			}

			// Grab the partition to format
			char *partitionc = strtok(NULL, " ");
			if(partitionc == NULL) {
				gl_puts("*** You must specify a partition index to format 1-4");
			}
			Uint8 index = atoi(partitionc) - 1;
			if(index > 4) {
				gl_puts("*** Invalid partition index!\n");
				continue;
			}
			
			// Call the format function
			if(_fs_format(&mount_points[mount_point_count], 
					&ata_devices[drive], index) == FS_ERR_NOTDERP) {
				gl_puts("*** Could not format -- partition does not have a DERP_FS bootrecord\n");
			}

		} else if(strncmp(command, "mounts", 20) == 0) {
			write('m');
			// MOUNTS ------------------------------------------------------
			// Breaking the rules again. Iterate over the mountpoints found
			if(mount_point_count == 0) {
				gl_puts("*** No DERP_FS MountPoints found\n");
			}

			for(i = 0; i < mount_point_count; i++) {
				gl_printf("%c: 0x%x sectors (512b)\n",
					mount_points[i].letter,
					mount_points[i].bootRecord.size
					); 
			}

		} else if(strncmp(command, "exit", 20) == 0) {
			// EXIT --------------------------------------------------------
			gl_puts("Shell is exiting!\n");
			return;
		} else if(strncmp(command, "p", 20) == 0) {
			__panic("This is fun");
		} else {
			// INVALID COMMAND ---------------------------------------------
			write('I');
			gl_puts("*** Invalid command!\n");
		}
	}
	write('e');
}

/*
** USER PROCESSES
**
** Each is designed to test some facility of the OS:
**
**	User(s)		Tests/Features
**	=======		===============================================
**	A, B, C		Basic operation
**	D		Spawns Z and exits
**	E, F, G		Sleep for different lengths of time
**	H		Doesn't call exit()
**	J		Tries to spawn 2*N_PCBS copies of Y
**	K		Spawns several copies of X, sleeping between
**	L		Spawns several copies of X, preempting between
**	M		Spawns W three times at low priority, reporting PIDs
**	N		Like M, but spawns X at high priority and W at low
**	P		Iterates three times, printing system time
**	Q		Tries to execute a bogus system call (bad code)
**	R		Reading and writing
**	S		Loops forever, sleeping 30 seconds at a time
**	T		Loops, fiddles with priority
**
**	W, X, Y, Z	Print characters (spawned by other processes)
**
** Output from user processes is always alphabetic.  Uppercase 
** characters are "expected" output; lowercase are "erroneous"
** output.
**
** More specific information about each user process can be found in
** the header comment for that function (below).
**
** To spawn a specific user process, uncomment its SPAWN_x
** definition in the user.h header file.
*/

/*
** Prototypes for all one-letter user main routines
*/

void user_a( void ); void user_b( void ); void user_c( void );
void user_d( void ); void user_e( void ); void user_f( void );
void user_g( void ); void user_h( void ); void user_i( void );
void user_j( void ); void user_k( void ); void user_l( void );
void user_m( void ); void user_n( void ); void user_o( void );
void user_p( void ); void user_q( void ); void user_r( void );
void user_s( void ); void user_t( void ); void user_u( void );
void user_v( void ); void user_w( void ); void user_x( void );
void user_y( void ); void user_z( void ); void user_keyboard(void);

/*
** Users A, B, and C are identical, except for the character they
** print out via write().  Each prints its ID, then loops 30
** times delaying and printing, before exiting.  They also verify
** the status return from write().
*/

void user_a( void ) {
	int i, j;
	Status status;

	status = write( 'A' );
	if( status != SUCCESS ) {
		prt_status( "User A, write 1 status %s\n", status );
	}
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		status = write( 'A' );
		if( status != SUCCESS ) {
			prt_status( "User A, write 2 status %s\n", status );
		}
	}

	gl_puts( "User A exiting\n" );
	exit();

	status = write( 'a' );	/* shouldn't happen! */
	if( status != SUCCESS ) {
		prt_status( "User A, write 3 status %s\n", status );
	}

}

void user_b( void ) {
	int i, j;
	Status status;

	gl_puts( "User B running\n" );
	status = write( 'B' );
	if( status != SUCCESS ) {
		prt_status( "User B, write 1 status %s\n", status );
	}
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		status = write( 'B' );
		if( status != SUCCESS ) {
			prt_status( "User B, write 2 status %s\n", status );
		}
	}

	gl_puts( "User B exiting\n" );
	exit();

	status = write( 'b' );	/* shouldn't happen! */
	if( status != SUCCESS ) {
		prt_status( "User B, write 3 status %s\n", status );
	}

}

void user_c( void ) {
	int i, j;
	Status status;

	gl_puts( "User C running\n" );
	status = write( 'C' );
	if( status != SUCCESS ) {
		prt_status( "User C, write 1 status %s\n", status );
	}
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		status = write( 'C' );
		if( status != SUCCESS ) {
			prt_status( "User C, write 2 status %s\n", status );
		}
	}

	gl_puts( "User C exiting\n" );
	exit();

	status = write( 'c' );	/* shouldn't happen! */
	if( status != SUCCESS ) {
		prt_status( "User C, write 3 status %s\n", status );
	}

}


/*
** User D spawns user Z.
*/

void user_d( void ) {
	Status status;
	Pid pid;

	gl_puts( "User D running\n" );
	write( 'D' );
	status = spawn( &pid, user_z );
	if( status != SUCCESS ) {
		prt_status( "User D, exec of Z status %s\n", status );
	}
	write( 'D' );

	gl_puts( "User D exiting\n" );
	exit();

}


/*
** Users E, F, and G test the sleep facility.
**
** User E sleeps for 10 seconds at a time.
*/

void user_e( void ) {
	int i;
	Pid pid;
	Status status;

	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User E get_pid status %s\n", status );
	}
	gl_printf( "User E (%d) running\n", pid );
	write( 'E' );
	for( i = 0; i < 5 ; ++i ) {
		sleep( 10 );
		write( 'E' );
	}

	gl_puts( "User E exiting\n" );
	exit();

}


/*
** User F sleeps for 5 seconds at a time.
*/

void user_f( void ) {
	int i;
	Pid pid;
	Status status;

	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User F get_pid status %s\n", status );
	}
	gl_printf( "User F (%d) running\n", pid );
	write( 'F' );
	for( i = 0; i < 5 ; ++i ) {
		sleep( 5 );
		write( 'F' );
	}

	gl_puts( "User F exiting\n" );
	exit();

}


/*
** User G sleeps for 15 seconds at a time.
*/

void user_g( void ) {
	int i;
	Pid pid;
	Status status;

	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User G get_pid status %s\n", status );
	}
	gl_printf( "User G (%d) running\n", pid );
	write( 'G' );
	for( i = 0; i < 5; ++i ) {
		sleep( 15 );
		write( 'G' );
	}

	gl_puts( "User G exiting\n" );
	exit();

}


/*
** User H is like A-C except it only loops 5 times and doesn't
** call exit().
*/

void user_h( void ) {
	int i, j;

	gl_puts( "User H running\n" );
	write( 'H' );
	for( i = 0; i < 5; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		write( 'H' );
	}

	gl_puts( "User H returning without exiting!\n" );

}


/*
** User J tries to spawn 2*N_PCBS copies of user_y.
*/

void user_j( void ) {
	int i;
	Pid pid;
	Status status;

	gl_puts( "User J running\n" );
	write( 'J' );

	for( i = 0; i < N_PCBS * 2 ; ++i ) {
		status = spawn( &pid, user_y );
		if( status != SUCCESS ) {
			prt_status( "User J, exec of y status %s\n", status );
		} else {
			write( 'J' );
		}
	}

	gl_puts( "User J exiting\n" );
	exit();

}


/*
** User K prints, goes into a loop which runs three times, and exits.
** In the loop, it does a spawn of user_x, sleeps 30 seconds, and prints.
*/

void user_k( void ) {
	int i;
	Pid pid;
	Status status;

	gl_puts( "User K running\n" );
	write( 'K' );

	for( i = 0; i < 3 ; ++i ) {
		status = spawn( &pid, user_x );
		if( status != SUCCESS ) {
			prt_status( "User K, user x exec status %s\n", status );
		} else {
			sleep( 30 );
			write( 'K' );
		}
	}

	gl_puts( "User K exiting\n" );
	exit();

}


/*
** User L is like user K, except that it prints times and doesn't sleep
** each time, it just preempts itself.
*/

void user_l( void ) {
	int i;
	Pid pid;
	Time time;
	Status status;

	gl_puts( "User L running, " );
	status = get_time( &time );
	if( status != SUCCESS ) {
		prt_status( "User L, get time status %s\n", status );
	} else {
		gl_printf( " initial time %u\n", time );
	}
	write( 'L' );

	for( i = 0; i < 3 ; ++i ) {
		status = get_time( &time );
		if( status != SUCCESS ) {
			prt_status( "User L, get_time status %s\n", status );
		}
		status = spawn( &pid, user_x );
		if( status != SUCCESS ) {
			prt_status( "User L, user_x spawn status %s\n", status );
		} else {
			sleep( 30 );
			write( 'L' );
		}
	}

	status = get_time( &time );
	if( status != SUCCESS ) {
		prt_status( "User L, exiting get time status %s\n", status );
	} else {
		gl_printf( "User L exiting at time %u\n", time );
	}
	exit();

}


/*
** User M iterates spawns three copies of user W at low priority,
** reporting their PIDs.
*/

void user_m( void ) {
	int i;
	Pid pid;
	Status status;

	gl_puts( "User M running\n" );
	for( i = 0; i < 3; ++i ) {
		status = spawnp( &pid, PRIO_LOW, user_w );
		if( status != SUCCESS ) {
			prt_status( "User M, user w exec status %s\n", status );
		} else {
			gl_printf( "User M spawned W, PID %d\n", pid );
			write( 'M' );
		}
	}

	gl_puts( "User M exiting\n" );
	exit();

}


/*
** User N is like user M, except that it spawns user W at low priority
** and user Z at high priority.
*/

void user_n( void ) {
	int i;
	Pid pid;
	Status status;

	gl_puts( "User N running\n" );
	for( i = 0; i < 3; ++i ) {
		status = spawnp( &pid, PRIO_LOW, user_w );
		if( status != SUCCESS ) {
			prt_status( "User N, user w exec status %s\n", status );
		} else {
			gl_printf( "User N spawned W, PID %d\n", pid );
			write( 'N' );
		}
		status = spawnp( &pid, PRIO_HIGH, user_z );
		if( status != SUCCESS ) {
			prt_status( "User N, user z exec status %s\n", status );
		} else {
			gl_printf( "User N spawned Z, PID %d\n", pid );
			write( 'N' );
		}
	}

	gl_puts( "User N exiting\n" );
	exit();

}


/*
** User P iterates three times.  Each iteration sleeps for two seconds,
** then gets and prints the system time.
*/

void user_p( void ) {
	Time time;
	int i;
	Status status;

	gl_printf( "User P running, " );
	status = get_time( &time );
	if( status != SUCCESS ) {
		prt_status( "get_time status %s\n", status );
	} else {
		gl_printf( " start at %08x\n", time );
	}

	write( 'P' );

	for( i = 0; i < 3; ++i ) {
		sleep( 2 );
		status = get_time( &time );
		if( status != SUCCESS ) {
			prt_status( "get_time status %s\n", status );
		} else {
			gl_printf( "User P reporting time %08x\n", time );
		}
		write( 'P' );
	}

	gl_printf( "User P exiting\n" );
	exit();

}


/*
** User Q does a bogus system call
*/

void user_q( void ) {

	gl_puts( "User Q running\n" );
	write( 'Q' );
	bogus();
	gl_puts( "User Q returned from bogus syscall!?!?!\n" );
	exit();

}


/*
** User R loops 3 times reading/writing, then exits.
*/

void user_r( void ) {
	int i;
	int ch = '&';
	Status status;

	gl_puts( "User R running\n" );
	sleep( 10 );
	for( i = 0; i < 3; ++i ) {
		do {
			write( 'R' );
			status = read( &ch );
			if( status != SUCCESS ) {
				prt_status( "User R, read status %s\n", status );
			} else if( ch == -1 ) {	/* wait a bit */
				sleep( 1 );
			}
		} while( ch == -1 );
		write( ch );
	}

	gl_puts( "User R exiting\n" );
	exit();

}


/*
** User S sleeps for 30 seconds at a time, and loops forever.
*/

void user_s( void ) {

	gl_puts( "User S running\n" );
	write( 'S' );
	for(;;) {
		sleep( 30 );
		write( 'S' );
	}

	gl_puts( "User S exiting!?!?!n" );
	exit();

}


/*
** User T changes its priority back and forth several times
*/

void user_t( void ) {
	Prio priority, prio2;
	int i, j;
	Pid pid;
	Status status;

	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User T get_pid status %s\n", status );
	}
	status = get_priority( &priority );
	if( status != SUCCESS ) {
		prt_status( "User T, get prio status %s\n", status );
	} else {
		gl_printf( "User T (%d) running, initial prio %d\n",
		  	  pid, priority );
	}
	
	write( 'T' );
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		write( 'T' );
	}

	status = set_priority( PRIO_HIGH );
	if( status != SUCCESS ) {
		write( 't' );
		prt_status( "User T, set prio #1 status %s\n", status );
	}
	status = get_priority( &prio2 );
	if( status != SUCCESS ) {
		prt_status( "User T, get prio2 #1 status %s\n", status );
	} else {
		gl_printf( "User T, prio was %d now %d\n", priority, prio2 );
	}
	priority = prio2;
	
	write( 'T' );
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		write( 'T' );
	}

	status = set_priority( PRIO_LOW );
	if( status != SUCCESS ) {
		write( 't' );
		prt_status( "User T, set prio #2 status %s\n", status );
	}
	status = get_priority( &prio2 );
	if( status != SUCCESS ) {
		prt_status( "User T, get prio2 #2 status %s\n", status );
	} else {
		gl_printf( "User T, prio was %d now %d\n", priority, prio2 );
	}
	priority = prio2;

	write( 'T' );
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		write( 'T' );
	}

	status = set_priority( PRIO_STD );
	if( status != SUCCESS ) {
		write( 't' );
		prt_status( "User T, set prio #3 status %s\n", status );
	}
	status = get_priority( &prio2 );
	if( status != SUCCESS ) {
		prt_status( "User T, get prio2 #3 status %s\n", status );
	} else {
		gl_printf( "User T, prio was %d now %d\n", priority, prio2 );
	}
	priority = prio2;
	
	write( 'T' );
	for( i = 0; i < 30; ++i ) {
		for( j = 0; j < DELAY_STD; ++j )
			continue;
		write( 'T' );
	}

	gl_puts( "User T exiting\n" );
	exit();

}


/*
** User W prints W characters 20 times, sleeping 3 seconds between.
*/

void user_w( void ) {
	int i;
	Pid pid;
	Status status;

	gl_printf( "User W running, " );
	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User W get_pid status %s\n", status );
	}
	gl_printf( " PID %d\n", pid );
	for( i = 0; i < 20 ; ++i ) {
		write( 'W' );
		sleep( 3 );
	}

	gl_printf( "User W exiting, PID %d\n", pid );
	exit();

}


/*
** User X prints X characters 20 times.  It is spawned multiple
** times, and prints its PID when started and exiting.
*/

void user_x( void ) {
	int i, j;
	Pid pid;
	Status status;

	gl_puts( "User X running, " );
	status = get_pid( &pid );
	if( status != SUCCESS ) {
		prt_status( "User X get_pid status %s\n", status );
	}
	gl_printf( "PID %d, ", pid );

	for( i = 0; i < 20 ; ++i ) {
		write( 'X' );
		for( j = 0; j < DELAY_STD; ++j )
			continue;
	}

	gl_printf( "User X exiting, PID %d\n", pid );
	exit();

}


/*
** User Y prints Y characters 10 times.
*/

void user_y( void ) {
	int i, j;

	gl_puts( "User Y running\n" );
	for( i = 0; i < 10 ; ++i ) {
		write( 'Y' );
		for( j = 0; j < DELAY_ALT; ++j )
			continue;
		sleep( 1 );
	}

	gl_puts( "User Y exiting\n" );
	exit();

}


/*
** User Z prints Z characters 10 times.
*/

void user_z( void ) {
	int i, j;

	gl_puts( "User Z running\n" );
	for( i = 0; i < 10 ; ++i ) {
		write( 'Z' );
		for( j = 0; j < DELAY_STD; ++j )
			continue;
	}

	gl_puts( "User Z exiting\n" );
	exit();

}

/*
** User MOUSE launches the mouse module.
*/

void user_mouse( void ) {
	gl_puts( "User MOUSE running\n" );
	_ps2_mouse_init();	
	gl_puts( "User MOUSE exiting\n" );
	exit();
}

/*
** User MOUSE launches the mouse module.
*/

void user_keyboard( void ) {
	gl_puts( "User KEYBOARD running\n" );
	_ps2_keyboard_init();	
	gl_puts( "User KEYBOARD exiting\n" );
	char buf[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\0' };
	gl_printf( "CUR BUF CONTENTS: %s\n", buf );
	read_buf( buf, 10 );
	gl_printf( "\nNew BUF CONTENTS: %s\n", buf );
	exit();
}

/*
** SYSTEM PROCESSES
*/


/*
** Initial process; it starts the other top-level user processes.
*/

void init( void ) {
	int i;
	Pid pid;
	Time time;
	Status status;
	#ifndef VIDEO_TEXT
	//c_set_device( 1 );
	#endif
	gl_puts( "Init started\n" );


	write( '$' );

	// we'll start the first three "manually"
	// by doing fork() and exec() ourselves

#ifdef SPAWN_GRAPHICS
	status = spawn( &pid, draw_active_screens );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user GRAPGICS, status %s\n", status );
	}
	
	status = spawn( &pid, draw_scr_0);
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user GRAPGICS, status %s\n", status );
	}
	status = spawn( &pid, draw_scr_1);
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user GRAPGICS, status %s\n", status );
	}
	status = spawn( &pid, draw_scr_2);
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user GRAPGICS, status %s\n", status );
	}
	status = spawn( &pid, draw_scr_3);
	status = spawn( &pid, draw_scr_4);
	status = spawn( &pid, draw_scr_8);
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user GRAPGICS, status %s\n", status );
	}
#endif
	spawn(&pid, fileshell);
/*
>>>>>>> sata_user
#ifdef SPAWN_A
	status = fork( &pid );
	if( status != SUCCESS ) {
		prt_status( "init: can't fork() user A, status %s\n", status );
	} else if( pid == 0 ) {
		status = exec( user_a );
		prt_status( "init: can't exec() user A, status %s\n", status );
		exit();
	}
#endif

#ifdef SPAWN_B
	status = fork( &pid );
	if( status != SUCCESS ) {
		prt_status( "init: can't fork() user B, status %s\n", status );
	} else if( pid == 0 ) {
		status = exec( user_b );
		prt_status( "init: can't exec() user B, status %s\n", status );
		exit();
	}
#endif

#ifdef SPAWN_C
	status = fork( &pid );
	if( status != SUCCESS ) {
		prt_status( "init: can't fork() user C, status %s\n", status );
	} else if( pid == 0 ) {
		status = exec( user_c );
		prt_status( "init: can't exec() user C, status %s\n", status );
		exit();
	}
#endif

	// for most of the rest, we'll use spawn()

#ifdef SPAWN_D
	status = spawn( &pid, user_d );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user D, status %s\n", status );
	}
#endif

#ifdef SPAWN_E
	status = spawn( &pid, user_e );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user E, status %s\n", status );
	}
#endif

#ifdef SPAWN_F
	status = spawn( &pid, user_f );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user F, status %s\n", status );
	}
#endif

#ifdef SPAWN_G
	status = spawn( &pid, user_g );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user G, status %s\n", status );
	}
#endif

#ifdef SPAWN_H
	status = spawn( &pid, user_h );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user H, status %s\n", status );
	}
#endif

#ifdef SPAWN_J
	status = spawn( &pid, user_j );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user J, status %s\n", status );
	}
#endif

#ifdef SPAWN_K
	status = spawn( &pid, user_k );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user K, status %s\n", status );
	}
#endif

#ifdef SPAWN_L
	status = spawn( &pid, user_l );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user L, status %s\n", status );
	}
#endif

#ifdef SPAWN_M
	status = spawn( &pid, user_m );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user N, status %s\n", status );
	}
#endif

#ifdef SPAWN_N
	status = spawn( &pid, user_n );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user N, status %s\n", status );
	}
#endif

#ifdef SPAWN_P
	status = spawn( &pid, user_p );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user P, status %s\n", status );
	}
#endif

#ifdef SPAWN_Q
	status = spawn( &pid, user_q );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user Q, status %s\n", status );
	}
#endif

#ifdef SPAWN_R
	status = spawn( &pid, user_r );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user R, status %s\n", status );
	}
#endif

#ifdef SPAWN_S
	status = spawn( &pid, user_s );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user S, status %s\n", status );
	}
#endif

#ifdef SPAWN_T
	status = spawn( &pid, user_t );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user T, status %s\n", status );
	}
#endif

#ifdef SPAWN_MOUSE
	status = spawn( &pid, user_mouse );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user MOUSE, status %s\n", status );
	}

#endif

#ifdef SPAWN_KEYBOARD
	status = spawn( &pid, user_keyboard );
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user KEYBOARD, status %s\n", status );
	}
#endif

#ifdef SPAWN_GRAPHICS
	status = spawn( &pid, draw_scr_4);
	status = spawn( &pid, draw_scr_5);
	status = spawn( &pid, draw_scr_6);
	status = spawn( &pid, draw_scr_7);
	if( status != SUCCESS ) {
		prt_status( "init: can't spawn() user GRAPGICS, status %s\n", status );
	}
#endif

	write( '!' );
*/
	/*
	** And now we start twiddling our thumbs
	*/

	status = get_time( &time );
	if( status != SUCCESS ) {
		prt_status( "idle: get_time status %s\n", status );
	}
	gl_printf( "init => idle at time %08x\n", time );

	status = set_priority( PRIO_IDLE );
	if( status != SUCCESS ) {
		prt_status( "idle: priority change status %s\n", status );
	}

	write( '.' );

	for(;;) {
		for( i = 0; i < DELAY_LONG; ++i )
			continue;
		write( '.' );
	}

	/*
	** SHOULD NEVER REACH HERE
	*/
	write( '#' );
	gl_printf( "*** IDLE IS EXITING???\n" );
	exit();

}

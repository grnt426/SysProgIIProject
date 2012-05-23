/*
** SCCS ID:	@(#)stacks.h	1.1	4/5/12
**
** File:	stacks.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Stack module definitions
*/

#ifndef _PROCESSES_H
#define _PROCESSES_H

#include "headers.h"

/*
** General (C and/or assembly) definitions
*/

// size of a user stack (in longwords)

#define	STACK_SIZE	4096

// number of stacks

#define	N_STACKS	N_PROCESSES

#ifndef __ASM__20113__

/*
** Start of C-only definitions
*/

/*
** Types
*/

// user stacks

typedef Uint32		Stack[ STACK_SIZE ];

/*
** Globals
*/

extern Stack _system_stack;	// stack for the OS
extern Uint32 *_system_esp;	// OS stack pointer

/*
** Prototypes
*/

#endif

#endif

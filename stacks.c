/*
** SCCS ID:	@(#)stacks.c	1.1	4/5/12
**
** File:	stacks.c
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Stack module
*/

#define	__KERNEL__20113__

#include "headers.h"

#include "queues.h"
#include "stacks.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

/*
** PUBLIC GLOBAL VARIABLES
*/

Stack _system_stack;	// stack for the OS
Uint32 *_system_esp;	// OS stack pointer

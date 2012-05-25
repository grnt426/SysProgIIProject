/*
** File:	gl_print.h
**
** Author:	Gideon Williams
**
** Description:	Definitions for the graphics library print functions.
*/
#ifndef _GL_PRINT_H_
#define _GL_PRINT_H_

#include "headers.h"

#define gl_printf printf

void gl_putchar( char c );

void gl_putchar_s( char c, screen_info* curr_si );

void gl_putchar_at( unsigned int x, unsigned int y, char c );

void gl_puts( char *str );

void gl_puts_s( char *str, screen_info* curr_si );

void gl_puts_at(  unsigned int x, unsigned int y, char *str );

char * gl_cvtdec0( char *buf, int value );
int gl_cvtdec( char *buf, int value );

int gl_cvthex( char *buf, int value );

int gl_cvtoct( char *buf, int value );

#endif //_WIN_MAN_H_

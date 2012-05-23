#ifndef __TFP_PRINTF__
#define __TFP_PRINTF__

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12
#define PAD_RIGHT 1
#define PAD_ZERO 2

#include "gl_print.h"
#define putchar(c) gl_putchar( c )

int printf(const char *format, ...);
int sprintf(char *out, const char *format, ...);

#endif


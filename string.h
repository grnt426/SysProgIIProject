////////////////////////////////////////////////////////////////////////////
// String.h
//
// @descrip	Header for the string library provided by Atmel
////////////////////////////////////////////////////////////////////////////

#ifndef STRING_H
#define STRING_H

// INCLUDES ////////////////////////////////////////////////////////////////
#include "headers.h"

// FUNCTIONS ///////////////////////////////////////////////////////////////
int strncmp(const char *pString1, const char *pString2, Uint32 count);
char *strtok(char *s, const char *delim);
char *strtok_r(char *s, const char *delim, char **last);
Uint32 strlen(const char *pString);
char * strchr(const char *pString, int character);
int atoi(const char *s);

#endif

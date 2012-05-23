////////////////////////////////////////////////////////////////////////////
// String.c
// 
// @descrip	Library for really simple string coding
// @source	http://code.google.com/p/lpc1343codebase/source/browse/trunk/core/libc/string.c
//			(unless noted)
// @author	Atmel (unless noted)
////////////////////////////////////////////////////////////////////////////

/*
 * Software License Agreement (BSD License)
 *
 * Based on original stdio.c released by Atmel
 * Copyright (c) 2008, Atmel Corporation
 * All rights reserved.
 *
 * Modified by Roel Verdult, Copyright (c) 2010
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "string.h"
#include "headers.h"

int strncmp(const char *pString1, const char *pString2, Uint32 count)
{
    int r;

    while(count) {
        r = *pString1 - *pString2;
        if (r == 0) {
            if (*pString1 == 0) {
                break;
            }
            pString1++;
            pString2++;
            count--;
            continue;
        }
        return r;
    }
    return 0;
}

char *strtok(char *s, const char *delim)
{
  static char *last;
  return strtok_r(s, delim, &last);
}

char *strtok_r(char *s, const char *delim, char **last)
{
  char *spanp;
  int c, sc;
  char *tok;
  
  
  if (s == NULL && (s = *last) == NULL)
    return (NULL);
  
  /*
   * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
   */
cont:
  c = *s++;
  for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
    if (c == sc)
      goto cont;
  }
  
  if (c == 0) {           /* no non-delimiter characters */
    *last = NULL;
    return (NULL);
  }
  tok = s - 1;
  
  /*
   * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
   * Note that delim must have one NUL; we stop if we see that, too.
   */
  for (;;) {
    c = *s++;
    spanp = (char *)delim;
    do {
      if ((sc = *spanp++) == c) {
        if (c == 0)
          s = NULL;
        else
          s[-1] = 0;
        *last = s;
        return (tok);
      }
    } while (sc != 0);
  }
  /* NOTREACHED */
}

Uint32 strlen(const char *pString)
{
    unsigned int length = 0;

    while(*pString++ != 0) {
        length++;
    }
    return length;
}

//-----------------------------------------------------------------------------
/// Search a character in the given string.
/// Returns a pointer to the character location.
/// \param pString   Pointer to the start of the string to search.
/// \param character The character to find.
//-----------------------------------------------------------------------------
char * strchr(const char *pString, int character)
{
    char * p = (char *)pString;
    char   c = character & 0xFF;

    while(*p != c) {
        if (*p == 0) {
            return 0;
        }
        p++;
    }
    return p;
}

/**
 * Converts a string to an integer.
 * @source: http://www.student.cs.uwaterloo.ca/~cs350/common/os161-src-html/atoi_8c-source.html
 */
int atoi(const char *s)
{
        static const char digits[] = "0123456789";  /* legal digits in order */
        unsigned val=0;         /* value we're accumulating */
        int neg=0;              /* set to true if we see a minus sign */

        /* skip whitespace */
        while (*s==' ' || *s=='\t') {
                s++;
        }

        /* check for sign */
        if (*s=='-') {
                neg=1;
                s++;
        }
        else if (*s=='+') {
                s++;
        }

        /* process each digit */
        while (*s) {
                const char *where;
                unsigned digit;
                
                /* look for the digit in the list of digits */
                where = strchr(digits, *s);
                if (where==NULL) {
                        /* not found; not a digit, so stop */
                        break;
                }

                /* get the index into the digit list, which is the value */
                digit = (where - digits);

                /* could (should?) check for overflow here */

                /* shift the number over and add in the new digit */
                val = val*10 + digit;

                /* look at the next character */
                s++;
        }
        
        /* handle negative numbers */
        if (neg) {
                return -val;
        }
        
        /* done */
        return val;
}

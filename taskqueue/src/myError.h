/* 
 * This header contrains error reporting marcos 
 */

#ifndef _MY_ERROR_H
#define _MY_ERROR_H

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// This marco prints an error gererated by the command cmd.
// Standard Unix error handling and posix error handling will be supported.
#define HANDLE_ERR(cmd) \
{ int e = cmd; \
    if (e) { \
       printf( "Error in line %d of file %s: %s\n", __LINE__, __FILE__, strerror((e==-1)?errno:e)); \
       exit(1); \
    } \
}

#endif

// EOF


#ifndef GBEMU_EMUTYPES_H
#define GBEMU_EMUTYPES_H

//====================================================================================================
// Includes
//====================================================================================================

#include <stdlib.h>
#include <stdio.h>

//====================================================================================================
// Global typedefs
//====================================================================================================

typedef unsigned int    uint32;
typedef signed int      sint32;

typedef unsigned short  uint16;
typedef signed short    sint16;

typedef unsigned char   ubyte;
typedef signed char     sbyte;

#define RUN_PROFILE 0

inline void _assert( const char* expression, const char* file, int line )
{
    fprintf( stderr, "Assertion '%s' failed, file '%s' line '%d'.", expression, file, line );
    __asm
    {
        int 3;
    }
}

#ifdef NDEBUG
#define assert(EXPRESSION) ((void)0)
#else
#define assert(EXPRESSION) ((EXPRESSION) ? (void)0 : _assert(#EXPRESSION, __FILE__, __LINE__))
#endif

#endif
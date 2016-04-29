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

//====================================================================================================
// Global enums
//====================================================================================================

enum GBGlobalConstants
{
    kGBBiosSizeBytes        = 0x100,    // GB bios is 256 bytes
    kGBBiosXorChecksum      = 0xF4,     // Pre-calculated 8 bit XOR checksum of GB bios
    kGBTotalMemSizeBytes    = 0x10000,  // GB has 64k total memory (~16k ROM, ~16k swap ROM, ~8k swap RAM, ~8k RAM, ~16k other)
    kGBMMIORegisterCount    = 0x100,    // Address space 0xFF00~0xFFFF are reserved for MMIO registers
};


//====================================================================================================
// Misc defines
//====================================================================================================

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
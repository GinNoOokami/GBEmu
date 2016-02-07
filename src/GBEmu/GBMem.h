#ifndef GBEMU_GBMEM_H
#define GBEMU_GBMEM_H

//====================================================================================================
// Filename:    GBMem.h
// Created by:  Jeff Padgham
// Description: The memory module for a Game Boy emulator. This handles all memory storage and operations.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

//====================================================================================================
// Global Enums
//====================================================================================================

// Special registers
enum MMIO
{
    MMIOJoypad                  = 0xFF00,
    MMIODivider                 = 0xFF04,
    MMIOTimerCounter            = 0xFF05,
    MMIOTimerModulo             = 0xFF06,
    MMIOTimerControl            = 0xFF07,
    MMIOLCDControl              = 0xFF40,
    MMIOLCDStatus               = 0xFF41,
    MMIOScrollY                 = 0xFF42,
    MMIOScrollX                 = 0xFF43,
    MMIOLCDScanline             = 0xFF44,
    MMIOLCDYCompare             = 0xFF45,
    MMIODMATransfer             = 0xFF46,
    MMIOBGPalette               = 0xFF47,
    MMIOObjectPalette0          = 0xFF48,
    MMIOObjectPalette1          = 0xFF49,
    MMIOWindowY                 = 0xFF4A,
    MMIOWindowX                 = 0xFF4B,
    MMIOKey1                    = 0xFF4D, // CGB only
    MMIOInterruptFlags          = 0xFF0F,
    MMIOBiosDisabled            = 0xFF50,
    MMIOInterruptEnable         = 0xFFFF
};

//====================================================================================================
// Global Structs
//====================================================================================================
struct OamData
{
    ubyte y;
    ubyte x;
    ubyte tileIndex;
    ubyte attributeFlags;
};

//====================================================================================================
// Forward Declarations
//====================================================================================================

class IGBMemBankController;

//====================================================================================================
// Class
//====================================================================================================

class GBMem
{
public:
    // Constructor / destructor
    GBMem( void );
    ~GBMem( void );

    void                    Reset();

    inline bool             IsBootRomEnabled()                                      { return m_bBiosEnabled;            }

    void                    LoadMemory( ubyte* pData, uint32 u32Offset, size_t size );

    ubyte                   ReadMemory( uint16 u16Address );
    void                    WriteMemory( uint16 u16Address, ubyte u8Data );

    OamData                 ReadSpriteData( uint32 u32Slot ) const;
    void                    WriteSpriteData( uint32 u32Slot );

    inline void             SetMemBankController( IGBMemBankController* pMBC )      { m_pMemBankController = pMBC;      }

    inline ubyte            ReadMMIO( MMIO ioRegister )                             { return m_Memory[ ioRegister ];    }
    inline void             WriteMMIO( MMIO ioRegister, ubyte u8Data )              { m_Memory[ ioRegister ] = u8Data;  }

    ubyte                   DebugReadMemory( uint16 u16Address );
    void                    DebugWriteMemory( uint16 u16Address, ubyte u8Data );
    void                    DebugWriteMemory( uint16 u16Address, ubyte u8Data[], uint16 u16Size );

private:
    static ubyte            GBBios[ 0x100 ];

    ubyte                   m_Memory[ 0x10000 ];

    IGBMemBankController*   m_pMemBankController;

    bool                    m_bBiosEnabled;
    bool                    m_bResetTimer;
};

#endif
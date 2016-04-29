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

#include <map>
#include <utility>

#include "GBMMIORegister.h"

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

class GBMem : public GBMMIORegister
{
    //====================================================================================================
    // Class Typedefs
    //====================================================================================================
    typedef std::map<MMIORegister, std::pair<GBMMIORegister*, MMIOReadHandler>> MMIOReadHandlers;
    typedef std::map<MMIORegister, std::pair<GBMMIORegister*, MMIOWriteHandler>> MMIOWriteHandlers;

public:
    // Constructor / destructor
    GBMem( void );
    virtual ~GBMem( void );

    void                    Reset();

    void                    LoadMemory( ubyte* pData, uint32 u32Offset, size_t size );

    ubyte                   ReadMemory( uint16 u16Address );
    void                    WriteMemory( uint16 u16Address, ubyte u8Data );

    OamData                 ReadSpriteData( uint32 u32Slot ) const;
    void                    WriteSpriteData( uint32 u32Slot );

    inline void             SetMemBankController( IGBMemBankController* pMBC )              { m_pMemBankController = pMBC;      }

    void                    RegisterMMIOReadHandler( MMIORegister eMMIORegister, GBMMIORegister* pRegisterController, MMIOReadHandler fnHandler );
    void                    RegisterMMIOWriteHandler( MMIORegister eMMIORegister, GBMMIORegister* pRegisterController, MMIOWriteHandler fnHandler );
    void                    RegisterMMIOHandlers( MMIORegister eMMIORegister, GBMMIORegister* pRegisterController, MMIOReadHandler fnReadHandler, MMIOWriteHandler fnWriteHandler );

    ubyte                   DebugReadMemory( uint16 u16Address );
    void                    DebugWriteMemory( uint16 u16Address, ubyte u8Data );
    void                    DebugWriteMemory( uint16 u16Address, ubyte u8Data[], uint16 u16Size );

private:
    inline ubyte            GetBiosDisabledRegister() const                                 { return !m_bBiosEnabled;           }
    inline void             SetBiosDisabledRegister( ubyte u8Data )                         { m_bBiosEnabled = ( 0 == u8Data ); }

private:
    ubyte                   m_pu8GBBios[ kGBBiosSizeBytes ];

    ubyte                   m_pu8Memory[ kGBTotalMemSizeBytes ];

    GBNullMMIORegister      m_oNullMMIORegister[ kGBMMIORegisterCount ];
    MMIOReadHandlers        m_MMIROReadHandlers;
    MMIOWriteHandlers       m_MMIOWriteHandlers;

    IGBMemBankController*   m_pMemBankController;

    bool                    m_bBiosEnabled;
};

#endif
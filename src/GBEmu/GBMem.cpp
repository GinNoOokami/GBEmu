//====================================================================================================
// Filename:    GBMem.cpp
// Created by:  Jeff Padgham
// Description: The memory module for a Game Boy emulator. This handles all memory storage and operations.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBMem.h"

#include "emutypes.h"
#include "IGBMemBankController.h"
#include "GBUserPrefs.h"

#include <memory.h>

//====================================================================================================
// Class
//====================================================================================================

GBMem::GBMem( void ) :
    m_pMemBankController( NULL ),
    m_bBiosEnabled( true )
{
    // Map the default MMIO handlers
    for( int i = MMIORegisterStart; i <= MMIORegisterEnd; ++i )
    {
        RegisterMMIOHandlers( (MMIORegister)i, 
                              &m_oNullMMIORegister[ i & 0xff ], 
                              static_cast<MMIOReadHandler>( &GBNullMMIORegister::ReadRegisterNoop ),
                              static_cast<MMIOWriteHandler>( &GBNullMMIORegister::WriteRegisterNoop ) );
    }

    SetMMIORegisterHandlers<GBMem>( this, MMIOBiosDisabled, &GBMem::GetBiosDisabledRegister, &GBMem::SetBiosDisabledRegister );

    // Load the bios if necessary
    memset( m_pu8GBBios, 0, sizeof( m_pu8GBBios ) );
    if( UserPrefs()->IsBiosEnabled() )
    {
        UserPrefs()->LoadBiosData( m_pu8GBBios );
    }

    Reset();
}

//----------------------------------------------------------------------------------------------------
GBMem::~GBMem( void )
{
}

//----------------------------------------------------------------------------------------------------
void GBMem::Reset()
{
    m_pMemBankController    = NULL;

    m_bBiosEnabled          = GBUserPrefs::Instance()->IsBiosEnabled();

    memset( m_pu8Memory, 0, sizeof( m_pu8Memory ) );

    // Reset all of the MMIO handlers
    // TODO: Investigate if setting predetermined values is necessary when skipping bios
    for( int i = MMIORegisterStart; i <= MMIORegisterEnd; ++i )
    {
        auto callback = m_MMIOWriteHandlers[ (MMIORegister)i ];
        auto pController = callback.first;
        auto pfnHandler = callback.second;

        // Invoke the register handler
        ( pController->*pfnHandler )( 0 );
    }
}

//----------------------------------------------------------------------------------------------------
void GBMem::LoadMemory( ubyte* pData, uint32 u32Offset, size_t size )
{
    memcpy( m_pu8Memory + u32Offset, pData, size );
}

//----------------------------------------------------------------------------------------------------
ubyte GBMem::ReadMemory( uint16 u16Address )
{
    if(     m_bBiosEnabled
        &&  u16Address < kGBBiosSizeBytes )
    {
        return m_pu8GBBios[ u16Address ];
    }

    // Handle MMIO registers if the address begins with 0xffxx
    if( 0xFF00 == ( u16Address & 0xFF00 ) )
    {
        auto callback    = m_MMIROReadHandlers[ (MMIORegister)u16Address ];
        auto pController = callback.first;
        auto pfnHandler  = callback.second;

        // Invoke the register handler
        return ( pController->*pfnHandler )( );
    }

    if( u16Address >= 0x4000 && u16Address < 0x8000 )
    {
        return m_pMemBankController->ReadRomBank( u16Address );
    }

    if( u16Address >= 0xA000 && u16Address < 0xC000 )
    {
        if( m_pMemBankController->IsRamEnabled() )
        {
            return m_pMemBankController->ReadRamBank( u16Address );
        }
        else
        {
            return 0x00;
        }
    }

    return m_pu8Memory[ u16Address ];
}

//----------------------------------------------------------------------------------------------------
void GBMem::WriteMemory( uint16 u16Address, ubyte u8Data )
{
    // Handle MMIO registers if the address begins with 0xffxx
    if( 0xFF00 == ( u16Address & 0xFF00 ) )
    {
        auto callback    = m_MMIOWriteHandlers[ (MMIORegister)u16Address ];
        auto pController = callback.first;
        auto pfnHandler  = callback.second;

        // Invoke the register handler
        ( pController->*pfnHandler )( u8Data );
        return;
    }

    if(     u16Address < 0x8000
        ||    ( u16Address >= 0xA000
            &&  u16Address < 0xC000
            &&  m_pMemBankController->IsRamEnabled() ) )
    {
        m_pMemBankController->WriteMemory( u16Address, u8Data );
    }
    else
    {
        m_pu8Memory[ u16Address ] = u8Data;

        // Handle echo ram
        if( u16Address >= 0xC000 && u16Address < 0xDE00 )
        {
            m_pu8Memory[ u16Address + 0x2000 ] = u8Data;
        }
        if( u16Address >= 0xE000 && u16Address < 0xFE00 )
        {
            m_pu8Memory[ u16Address - 0x2000 ] = u8Data;
        }
    }
}

//----------------------------------------------------------------------------------------------------
void GBMem::RegisterMMIOReadHandler( MMIORegister eMMIORegister, GBMMIORegister* pRegisterController, MMIOReadHandler fnHandler )
{
    m_MMIROReadHandlers[ eMMIORegister ] = std::make_pair( pRegisterController, fnHandler );
}

//----------------------------------------------------------------------------------------------------
void GBMem::RegisterMMIOWriteHandler( MMIORegister eMMIORegister, GBMMIORegister* pRegisterController, MMIOWriteHandler fnHandler )
{
    m_MMIOWriteHandlers[ eMMIORegister ] = std::make_pair( pRegisterController, fnHandler );
}

//----------------------------------------------------------------------------------------------------
void GBMem::RegisterMMIOHandlers( MMIORegister eMMIORegister, GBMMIORegister* pRegisterController, MMIOReadHandler fnReadHandler, MMIOWriteHandler fnWriteHandler )
{
    RegisterMMIOReadHandler( eMMIORegister, pRegisterController, fnReadHandler );
    RegisterMMIOWriteHandler( eMMIORegister, pRegisterController, fnWriteHandler );
}

//----------------------------------------------------------------------------------------------------
OamData GBMem::ReadSpriteData( uint32 u32Slot ) const
{
    OamData   oSpriteData;
    uint16    u16BaseAddress    = 0xFE00 + ( u32Slot * sizeof( oSpriteData ) );

    oSpriteData.y               = m_pu8Memory[ u16BaseAddress + 0 ];
    oSpriteData.x               = m_pu8Memory[ u16BaseAddress + 1 ];
    oSpriteData.tileIndex       = m_pu8Memory[ u16BaseAddress + 2 ];
    oSpriteData.attributeFlags  = m_pu8Memory[ u16BaseAddress + 3 ];

    return oSpriteData;
}

//----------------------------------------------------------------------------------------------------
ubyte GBMem::DebugReadMemory( uint16 u16Address )
{
    return m_pu8Memory[ u16Address ];
}

//----------------------------------------------------------------------------------------------------
void GBMem::DebugWriteMemory( uint16 u16Address, ubyte u8Data )
{
    m_pu8Memory[ u16Address ] = u8Data;
}

//----------------------------------------------------------------------------------------------------
void GBMem::DebugWriteMemory( uint16 u16Address, ubyte u8Data[], uint16 u16Size )
{
    for( uint16 i = 0; i < u16Size; ++i )
    {
        m_pu8Memory[ u16Address + i ] = u8Data[ i ];
    }
}
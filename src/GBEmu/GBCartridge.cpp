//====================================================================================================
// Filename:    GBCartridge.h
// Created by:  Jeff Padgham
// Description: Emulates a Game Boy cartridge.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBCartridge.h"

#include <string.h>
#include <fstream>

#include "emutypes.h"
#include "GBMem.h"
#include "GBMemBankController0.h"
#include "GBMemBankController1.h"
#include "GBMemBankController2.h"
#include "GBMemBankController3.h"

#include "CLog.h"

//====================================================================================================
// Static initializers
//====================================================================================================

const uint16 GBCartridge::CARTRIDGE_ENTRY_POINT = 0x100;

//====================================================================================================
// Class
//====================================================================================================

GBCartridge::GBCartridge( GBMem* pMemoryModule ) :
    m_pMem( pMemoryModule ),
    m_pRom( NULL ),
    m_pRam( NULL ),
    m_pMemBankController( NULL ),
    m_szBatteryDirectory( NULL ),
    m_bLoaded( false )
{
}

//----------------------------------------------------------------------------------------------------
GBCartridge::~GBCartridge()
{
}

//----------------------------------------------------------------------------------------------------
void GBCartridge::Reset()
{
    if( m_bLoaded )
    {
        memset( m_pRam, 0, sizeof( m_pRam ) );
        m_pMem->LoadMemory( m_pRom, 0, sizeof( ubyte ) * 0x4000 );
        m_pMem->SetMemBankController( CreateMemBankController( m_oHeader.u8CartridgeType ) );

        // Attempt to load the .sav file for this rom
        LoadBattery();
    }
}

//----------------------------------------------------------------------------------------------------
bool GBCartridge::LoadFromFile( const char* szFilepath, const char* szBatteryDirectory )
{
    bool bLoaded = false;

    ifstream oFile( szFilepath, ios::binary );
    if( oFile )
    {
        ubyte u8CartridgeHeader[ 0x150 ];
        oFile.read( reinterpret_cast<char*>( u8CartridgeHeader ), sizeof( u8CartridgeHeader ) );

        if( ValidateCartridgeHeader( u8CartridgeHeader ) )
        {
            m_szBatteryDirectory = szBatteryDirectory;

            LoadCartridgeHeader( u8CartridgeHeader );
            bLoaded = LoadCartridge( oFile );
        }

        oFile.close();
    }
    else
    {
        Log()->Write( LOG_COLOR_YELLOW, "Unable to load ROM file!" );
    }

    return bLoaded;
}

//----------------------------------------------------------------------------------------------------
bool GBCartridge::HasBattery() const
{
    switch( m_oHeader.u8CartridgeType )
    {
        case 0x03:
        case 0x06:
        case 0x09:
        case 0x0D:
        case 0x0F:
        case 0x10:
        case 0x13:
        case 0x1B:
        case 0x1E:
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------------------------------

bool GBCartridge::IsRamDirty() const
{
    return m_pMemBankController->IsRamDirty();
}

//----------------------------------------------------------------------------------------------------
void GBCartridge::FlushRamToSaveFile( const char *szBatteryDirectory )
{
    string oPath;
    oPath.append( szBatteryDirectory )
         .append( m_oHeader.szTitle )
         .append( ".sav" );

    ofstream oFile( oPath, ios::binary );
    if( oFile )
    {
        oFile.write( reinterpret_cast<char*>( m_pRam ), GetRamSize() );
        oFile.close();
    }
    else
    {
        Log()->Write( LOG_COLOR_YELLOW, "Unable to write save file!" );
    }
}

//----------------------------------------------------------------------------------------------------
void GBCartridge::Unload()
{
    delete m_pRom;
    m_pRom = NULL;

    delete m_pRam;
    m_pRam = NULL;

    m_pMem->SetMemBankController( NULL );
    delete m_pMemBankController;
    m_pMemBankController = NULL;

    m_szBatteryDirectory = NULL;

    m_bLoaded = false;
}

//----------------------------------------------------------------------------------------------------
void GBCartridge::LoadCartridgeHeader( ubyte *pHeaderData )
{
    memcpy( &m_oHeader, pHeaderData + CARTRIDGE_ENTRY_POINT, sizeof( m_oHeader ) );
}

//----------------------------------------------------------------------------------------------------
bool GBCartridge::ValidateCartridgeHeader( ubyte* pHeaderData )
{
    ubyte x = 0;

    for( int i = 0x134; i <= 0x14C; ++i )
    {
        x = x - pHeaderData[ i ] - 1;
    }

    return pHeaderData[ 0x14D ] == x;
}

//----------------------------------------------------------------------------------------------------
bool GBCartridge::LoadCartridge( ifstream& oFile )
{
    uint32 u32RomSize = GetRomSize();
    uint32 u32RamSize = GetRamSize();

    m_pRom = new ubyte[ u32RomSize ];
    memset( m_pRom, 0, u32RomSize );

    oFile.clear();
    oFile.seekg( 0 );
    oFile.read( reinterpret_cast<char*>( m_pRom ), u32RomSize );

    // Minimum 2k ram
    if( u32RamSize < 2048 )
    {
        u32RamSize = 2048;
    }
    m_pRam = new ubyte[ u32RamSize ];

    m_bLoaded = true;

    Reset();

    return true;
}

//----------------------------------------------------------------------------------------------------
void GBCartridge::LoadBattery()
{
    if( m_szBatteryDirectory )
    {
        string oPath;
        oPath.append( m_szBatteryDirectory )
             .append( m_oHeader.szTitle )
             .append( ".sav" );

        ifstream oFile( oPath, ios::binary );
        if( oFile )
        {
            uint32 u32RamSize = GetRamSize();
            oFile.read( reinterpret_cast<char*>( m_pRam ), u32RamSize );

            Log()->Write( LOG_COLOR_WHITE, "Save file loaded." );

            oFile.close();
        }
        else
        {
            Log()->Write( LOG_COLOR_YELLOW, "Save file could not be loaded!" );
        }
    }
}

//----------------------------------------------------------------------------------------------------
IGBMemBankController* GBCartridge::CreateMemBankController( ubyte u8CartridgeType )
{
    IGBMemBankController* pMBC    = NULL;

    // 0-ROM ONLY                   12-ROM+MBC3+RAM
    // 1-ROM+MBC1                   13-ROM+MBC3+RAM+BATT
    // 2-ROM+MBC1+RAM               19-ROM+MBC5
    // 3-ROM+MBC1+RAM+BATT          1A-ROM+MBC5+RAM
    // 5-ROM+MBC2                   1B-ROM+MBC5+RAM+BATT
    // 6-ROM+MBC2+BATTERY           1C-ROM+MBC5+RUMBLE
    // 8-ROM+RAM                    1D-ROM+MBC5+RUMBLE+SRAM
    // 9-ROM+RAM+BATTERY            1E-ROM+MBC5+RUMBLE+SRAM+BATT
    // B-ROM+MMM01                  1F-Pocket Camera
    // C-ROM+MMM01+SRAM             FD-Bandai TAMA5
    // D-ROM+MMM01+SRAM+BATT        FE - Hudson HuC-3
    // F-ROM+MBC3+TIMER+BATT        FF - Hudson HuC-1
    // 10-ROM+MBC3+TIMER+RAM+BATT
    // 11-ROM+MBC3

    if( NULL != m_pMemBankController )
    {
        delete m_pMemBankController;
        m_pMemBankController = NULL;
    }

    switch( u8CartridgeType )
    {
        case 0x00:
        case 0x08:
        case 0x09:
            pMBC = new GBMemBankController0( m_pRom );
            break;
        case 0x01:
        case 0x02:
        case 0x03:
            pMBC = new GBMemBankController1( m_pRom, m_pRam );
            break;
        case 0x05:
        case 0x06:
            pMBC = new GBMemBankController2( m_pRom, m_pRam );
            break;
        case 0x0F:
        case 0x10:
            pMBC = new GBMemBankController3( m_pRom, m_pRam, true );
        case 0x11:
        case 0x12:
        case 0x13:
            pMBC = new GBMemBankController3( m_pRom, m_pRam );
            break;
        default:
            assert( "Unimplemented cartridge type!" );
            __asm int 3;
    }

    m_pMemBankController = pMBC;

    return pMBC;
}

//----------------------------------------------------------------------------------------------------
uint32 GBCartridge::GetRomSize() const
{
    return ( 1 << 15 ) << m_oHeader.u8RomSize;
}

//----------------------------------------------------------------------------------------------------
uint32 GBCartridge::GetRamSize() const
{
    return ( 1 << 9 ) << m_oHeader.u8RamSize * 2;
}
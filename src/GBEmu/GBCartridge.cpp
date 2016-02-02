//====================================================================================================
// Filename:    GBCartridge.h
// Created by:  Jeff Padgham
// Description: Emulates a Game Boy cartridge.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBCartridge.h"

#include "emutypes.h"
#include "GBMem.h"
#include "GBMemBankController0.h"
#include "GBMemBankController1.h"
#include "GBMemBankController2.h"
#include "GBMemBankController3.h"

#include <memory.h>
#include <assert.h>

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
	m_pRam( NULL )
{
}

//----------------------------------------------------------------------------------------------------
GBCartridge::~GBCartridge()
{
}

//----------------------------------------------------------------------------------------------------
bool GBCartridge::LoadFromFile( const char *szFilepath, const char* szBatteryDirectory )
{
	FILE*	pFile			= NULL;
	FILE*	pBatteryFile	= NULL;
	bool	bLoaded			= false;
		
	fopen_s( &pFile, szFilepath, "rb" );
	if( NULL != pFile )
	{
		ubyte u8CartridgeHeader[ 0x150 ];

		fread_s( u8CartridgeHeader, sizeof( u8CartridgeHeader ), sizeof( u8CartridgeHeader ), 1, pFile );
		
		if( ValidateCartridgeHeader( u8CartridgeHeader ) )
		{
			LoadCartridgeHeader( u8CartridgeHeader );
			bLoaded = LoadCartridge( pFile );
		}
		
		// Attempt to load the .sav file for this rom
		if(		bLoaded
			&&	szBatteryDirectory )
		{
			char szBatteryFilepath[ 1024 ] = { 0 };
			sprintf( szBatteryFilepath, "%s%s%s", szBatteryDirectory, m_oHeader.szTitle, ".sav" );

			fopen_s( &pBatteryFile, szBatteryFilepath, "rb" );

			if( NULL != pBatteryFile )
			{
				LoadBatteryFile( pBatteryFile );

				fclose( pBatteryFile );
			}
		}

		fclose( pFile );
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
	FILE*	pBatteryFile	= NULL;
	char szBatteryFilepath[ 1024 ] = { 0 };

	sprintf( szBatteryFilepath, "%s%s%s", szBatteryDirectory, m_oHeader.szTitle, ".sav" );

	fopen_s( &pBatteryFile, szBatteryFilepath, "wb" );
	if( NULL != pBatteryFile )
	{
		fwrite( m_pRam, GetRamSize(), 1, pBatteryFile );

		fclose( pBatteryFile );
	}
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
bool GBCartridge::LoadCartridge( FILE* pFile )
{
	uint32 u32RomSize = GetRomSize();
	uint32 u32RamSize = GetRamSize();

	m_pRom = new ubyte[ u32RomSize ];
	memset( m_pRom, 0, u32RomSize );

	if( u32RamSize < 2048 )
	{
		u32RamSize = 2048;
	}
		m_pRam = new ubyte[ u32RamSize ];
		memset( m_pRam, 0, u32RamSize );

	rewind( pFile );
	fread_s( m_pRom, u32RomSize, 1, u32RomSize, pFile );

	m_pMem->LoadMemory( m_pRom, 0, sizeof( ubyte ) * 0x4000 );
	m_pMem->SetMemBankController( CreateMemBankController( m_oHeader.u8CartridgeType ) );

	return true;
}

//----------------------------------------------------------------------------------------------------
void GBCartridge::LoadBatteryFile(FILE *pFile)
{
	uint32 u32RamSize = GetRamSize();
	
	fread_s( m_pRam, u32RamSize, 1, u32RamSize, pFile );
}

//----------------------------------------------------------------------------------------------------
IGBMemBankController* GBCartridge::CreateMemBankController( ubyte u8CartridgeType )
{
	IGBMemBankController* pMBC;

	// 0-ROM ONLY				12-ROM+MBC3+RAM
	// 1-ROM+MBC1				13-ROM+MBC3+RAM+BATT
	// 2-ROM+MBC1+RAM			19-ROM+MBC5
	// 3-ROM+MBC1+RAM+BATT		1A-ROM+MBC5+RAM
	// 5-ROM+MBC2				1B-ROM+MBC5+RAM+BATT
	// 6-ROM+MBC2+BATTERY		1C-ROM+MBC5+RUMBLE
	// 8-ROM+RAM				1D-ROM+MBC5+RUMBLE+SRAM
	// 9-ROM+RAM+BATTERY		1E-ROM+MBC5+RUMBLE+SRAM+BATT
	// B-ROM+MMM01				1F-Pocket Camera
	// C-ROM+MMM01+SRAM			FD-Bandai TAMA5
	// D-ROM+MMM01+SRAM+BATT	FE - Hudson HuC-3
	// F-ROM+MBC3+TIMER+BATT	FF - Hudson HuC-1
	// 10-ROM+MBC3+TIMER+RAM+BATT
	// 11-ROM+MBC3

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
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

#include <memory.h>

//====================================================================================================
// Static intializers
//====================================================================================================

ubyte GBMem::GBBios[ 0x100 ] = 
{
	0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xF2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x4C,
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

//====================================================================================================
// Class
//====================================================================================================

GBMem::GBMem( void ) :
	m_pMemBankController( NULL ),
	m_bBiosEnabled( true ),
	m_bResetTimer( false )
{
	Reset();
}

//----------------------------------------------------------------------------------------------------
GBMem::~GBMem( void )
{
}

//----------------------------------------------------------------------------------------------------
void GBMem::Reset()
{
	m_pMemBankController	= NULL;

	m_bBiosEnabled			= true;
	m_bResetTimer			= false;

	memset( m_Memory, 0, sizeof( m_Memory ) );
}

//----------------------------------------------------------------------------------------------------
void GBMem::LoadMemory( ubyte* pData, uint32 u32Offset, size_t size )
{
	memcpy( m_Memory + u32Offset, pData, size );
}

//----------------------------------------------------------------------------------------------------
ubyte GBMem::ReadMemory( uint16 u16Address )
{
	if(		m_bBiosEnabled
		&&	u16Address < 0x100	)
	{
		return GBBios[ u16Address ];
	}

	if( u16Address >= 0x4000 && u16Address < 0x8000 )
	{
		return m_pMemBankController->ReadRomBank( u16Address );
	}

	if(	u16Address >= 0xA000 && u16Address < 0xC000 )
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

	return m_Memory[ u16Address ];
}

//----------------------------------------------------------------------------------------------------
void GBMem::WriteMemory( uint16 u16Address, ubyte u8Data )
{
	if( 0xFF == ( u16Address >> 8 ) )
	{
		// Handle writes to the IO registers
		switch( u16Address )
		{
			case MMIOTimerCounter:
				m_bResetTimer = true;
				break;

			case MMIOLCDControl:
				m_Memory[ u16Address ] = u8Data;
				break;

			case MMIOLCDStatus:
				// Lower 3 bits are read-only, msb is unused
				m_Memory[ u16Address ] = u8Data & 0x78;
				break;

			case MMIOLCDScanline:
				m_Memory[ u16Address ] = 0;
				return;

			case MMIODMATransfer:
				for( int i = 0; i < 0x100; ++i )
				{
					WriteMemory( 0xFE00 + i, ReadMemory( ( u8Data << 8 ) + i ) );
				}
				return;

			case MMIOBiosDisabled:
				m_bBiosEnabled = ( 0 == u8Data );
				break;

			case MMIODivider:
				m_Memory[ u16Address ] = 0;
				break;

			// KEY1 register not supported by DMG, always read default speed
			case MMIOKey1:
				m_Memory[ u16Address ] = 0;
				break;
		}
	}

	if(		u16Address >= 0xA000
		&&	u16Address < 0xC000 )
	{
		int a = 1;
	}

	if(		u16Address < 0x8000
		||	(	u16Address >= 0xA000
			&&	u16Address < 0xC000
			&&	m_pMemBankController->IsRamEnabled() ) )
	{
		m_pMemBankController->WriteMemory( u16Address, u8Data );
	}
	else
	{
		m_Memory[ u16Address ] = u8Data;

		// Handle echo ram
		if( u16Address >= 0xC000 && u16Address < 0xDE00 )
		{
			m_Memory[ u16Address + 0x2000 ] = u8Data;
		}
		if( u16Address >= 0xE000 && u16Address < 0xFE00 )
		{
			m_Memory[ u16Address - 0x2000 ] = u8Data;
		}
	}
}

//----------------------------------------------------------------------------------------------------
OamData GBMem::ReadSpriteData( uint32 u32Slot ) const
{
	OamData	oSpriteData;
	uint16	u16BaseAddress		= 0xFE00 + ( u32Slot * sizeof( oSpriteData ) );

	oSpriteData.y				= m_Memory[ u16BaseAddress + 0 ];
	oSpriteData.x				= m_Memory[ u16BaseAddress + 1 ];
	oSpriteData.tileIndex		= m_Memory[ u16BaseAddress + 2 ];
	oSpriteData.attributeFlags	= m_Memory[ u16BaseAddress + 3 ];

	return oSpriteData;
}

//----------------------------------------------------------------------------------------------------
ubyte GBMem::DebugReadMemory( uint16 u16Address )
{
	return m_Memory[ u16Address ];
}

//----------------------------------------------------------------------------------------------------
void GBMem::DebugWriteMemory( uint16 u16Address, ubyte u8Data )
{
	m_Memory[ u16Address ] = u8Data;
}

//----------------------------------------------------------------------------------------------------
void GBMem::DebugWriteMemory( uint16 u16Address, ubyte u8Data[], uint16 u16Size )
{
	for( uint16 i = 0; i < u16Size; ++i )
	{
		m_Memory[ u16Address + i ] = u8Data[ i ];
	}
}
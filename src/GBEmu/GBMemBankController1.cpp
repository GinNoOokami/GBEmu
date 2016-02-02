//====================================================================================================
// Filename:    GBMemBankController1.cpp
// Created by:  Jeff Padgham
// Description: Memory bank controller 1.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBMemBankController1.h"

#include <cassert>

//====================================================================================================
// Class
//====================================================================================================
GBMemBankController1::GBMemBankController1( ubyte* pRomBank, ubyte* pRamBank ) :
	m_pRomBank( pRomBank ),
	m_pRamBank( pRamBank ),
	m_bRamEnabled( false ),
	m_bRamBankMode( false ),
	m_bRamDirty( false ),
	m_u8BankSelect0( 0 ),
	m_u8BankSelect1( 0 ),
	m_u8SelectedRomBank( 1 ),
	m_u8SelectedRamBank( 0 )
{

}

//----------------------------------------------------------------------------------------------------
GBMemBankController1::~GBMemBankController1()
{

}

//----------------------------------------------------------------------------------------------------
ubyte GBMemBankController1::ReadRomBank( uint16 u16Address )
{
	assert( m_u8SelectedRomBank < 128 );

	uint32 u32Address = u16Address + ( ( m_u8SelectedRomBank - 1 ) << 14 );
	return m_pRomBank[ u32Address ];
}

//----------------------------------------------------------------------------------------------------
ubyte GBMemBankController1::ReadRamBank( uint16 u16Address )
{
	uint32 u32RamOffset	= m_bRamEnabled ? ( ( 1 << 13 ) * ( m_u8SelectedRamBank ) ) : 0;
	uint32 u32Address = u16Address + u32RamOffset - 0xA000;
	return m_pRamBank[ u32Address ];
}

//----------------------------------------------------------------------------------------------------
void GBMemBankController1::WriteMemory( uint16 u16Address, ubyte u8Data )
{
	if( u16Address >= 0x0000 && u16Address < 0x2000 )
	{
		m_bRamEnabled = 0 != ( u8Data & 0x0A );
	}
	else if( u16Address >= 0x2000 && u16Address < 0x4000 )
	{
		m_u8BankSelect0 = u8Data & 0x1F;
		UpdateBankValues();
	}
	else if( u16Address >= 0x4000 && u16Address < 0x6000 )
	{
		m_u8BankSelect1 = u8Data & 0x03;
		UpdateBankValues();
	}
	else if( u16Address >= 0x6000 && u16Address < 0x8000 )
	{
		m_bRamBankMode	= 0 != u8Data;
		m_u8BankSelect1	= 0;

		UpdateBankValues();
	}
	else if( u16Address >= 0xA000 && u16Address < 0xC000 )
	{
		uint32 u32RamOffset	= m_bRamEnabled ? ( ( 1 << 13 ) * ( m_u8SelectedRamBank ) ) : 0;
		uint32 u32Address = u16Address + u32RamOffset - 0xA000;
		
		m_pRamBank[ u32Address ] = u8Data;

		m_bRamDirty = true;
	}
}

//----------------------------------------------------------------------------------------------------
bool GBMemBankController1::IsRamEnabled() const
{
	return m_bRamEnabled && NULL != m_pRamBank;
}

//----------------------------------------------------------------------------------------------------
void GBMemBankController1::UpdateBankValues()
{
	if( m_bRamBankMode )
	{
		m_u8SelectedRomBank = m_u8BankSelect0;
		m_u8SelectedRamBank = m_u8BankSelect1;
	}
	else
	{
		m_u8SelectedRomBank = m_u8BankSelect0 | ( m_u8BankSelect1 << 5 );
		m_u8SelectedRamBank = 0;
	}

	// Rom banks 0x00, 0x20, 0x40, and 0x60 are unusable - the GB hardware will automatically switch to 0x01/0x21/0x41/0x61 when used
	switch( m_u8SelectedRomBank )
	{
		case 0x00:
		case 0x20:
		case 0x40:
		case 0x60:
			m_u8SelectedRomBank++;
		break;
	}
}
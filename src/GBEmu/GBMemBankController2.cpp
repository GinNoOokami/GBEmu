//====================================================================================================
// Filename:    GBMemBankController2.cpp
// Created by:  Jeff Padgham
// Description: Memory bank controller 1.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBMemBankController2.h"

#include <cassert>

//====================================================================================================
// Class
//====================================================================================================
GBMemBankController2::GBMemBankController2( ubyte* pRomBank, ubyte* pRamBank ) :
	m_pRomBank( pRomBank ),
	m_pRamBank( pRamBank ),
	m_bRamEnabled( false ),
	m_bRamDirty( false ),
	m_u8SelectedRomBank( 1 )
{

}

//----------------------------------------------------------------------------------------------------
GBMemBankController2::~GBMemBankController2()
{

}

//----------------------------------------------------------------------------------------------------
ubyte GBMemBankController2::ReadRomBank( uint16 u16Address )
{
	assert( m_u8SelectedRomBank > 0 );

	uint32 u32Address = ( u16Address & 0x3FFF ) + ( ( m_u8SelectedRomBank ) << 14 );
	return m_pRomBank[ u32Address ];
}

//----------------------------------------------------------------------------------------------------
ubyte GBMemBankController2::ReadRamBank( uint16 u16Address )
{
	if( m_bRamEnabled )
	{
		uint32 u32Address = u16Address - 0xA000;
		return m_pRamBank[ u32Address ];
	}
	return 0x00;
}

//----------------------------------------------------------------------------------------------------
void GBMemBankController2::WriteMemory( uint16 u16Address, ubyte u8Data )
{
	if( u16Address >= 0x0000 && u16Address < 0x2000 )
	{
		if( !( u16Address & 0x0100 ) )
		{
			m_bRamEnabled = ( 0x0A == ( u8Data & 0x0F ) );
		}
	}
	else if( u16Address >= 0x2000 && u16Address < 0x4000 )
	{
		if( u16Address & 0x0100 )
		{
			m_u8SelectedRomBank = ( u8Data & 0x0F ) + ( (u8Data & 0x0F ) == 0 );
		}
	}
	else if( u16Address >= 0xA000 && u16Address < 0xA200 )
	{
		uint32 u32Address = u16Address - 0xA000;
		
		m_pRamBank[ u32Address ] = u8Data & 0x0F;

		m_bRamDirty = true;
	}
}

//----------------------------------------------------------------------------------------------------
bool GBMemBankController2::IsRamEnabled() const
{
	return m_bRamEnabled && NULL != m_pRamBank;
}
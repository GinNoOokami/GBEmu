//====================================================================================================
// Filename:    GBMemBankController3.cpp
// Created by:  Jeff Padgham
// Description: Memory bank controller 3.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBMemBankController3.h"

#include <cassert>

//====================================================================================================
// Class
//====================================================================================================
GBMemBankController3::GBMemBankController3( ubyte* pRomBank, ubyte* pRamBank, bool bHasTimer ) :
	m_pRomBank( pRomBank ),
	m_pRamBank( pRamBank ),
	m_bHasTimer( bHasTimer ),
	m_bLatched( false ),
	m_bRamEnabled( false ),
	m_bRamDirty( false ),
	m_u8SelectedRomBank( 1 ),
	m_u8SelectedRamBank( 0 ),
	m_u8RtcSeconds( 0 ),
	m_u8RtcMinutes( 0 ),
	m_u8RtcHours( 0 ),
	m_u16RtcDays( 0 ),
	m_u8LatchedSeconds( 0 ),
	m_u8LatchedMinutes( 0 ),
	m_u8LatchedHours( 0 ),
	m_u16LatchedDays( 0 )
{

}

//----------------------------------------------------------------------------------------------------
GBMemBankController3::~GBMemBankController3()
{

}

//----------------------------------------------------------------------------------------------------
ubyte GBMemBankController3::ReadRomBank( uint16 u16Address )
{
	assert( m_u8SelectedRomBank < 128 );

	uint32 u32Address = u16Address + ( ( m_u8SelectedRomBank - 1 ) << 14 );
	return m_pRomBank[ u32Address ];
}

//----------------------------------------------------------------------------------------------------
ubyte GBMemBankController3::ReadRamBank( uint16 u16Address )
{
	if( m_bRamEnabled )
	{
		if( m_u8SelectedRamBank < 0x04 )
		{
			uint32 u32RamOffset	= ( 1 << 13 ) * ( m_u8SelectedRamBank );
			uint32 u32Address = u16Address + u32RamOffset - 0xA000;
			return m_pRamBank[ u32Address ];
		}
		else if( m_bHasTimer )
		{
			if( m_bLatched )
			{
				switch( m_u8SelectedRamBank )
				{
					case 0x08:
						return m_u8LatchedSeconds;
					case 0x09:
						return m_u8LatchedMinutes;
					case 0x0A:
						return m_u8LatchedHours;
					case 0x0B:
						return m_u16LatchedDays & 0xFF;
					case 0x0C:
						return m_u16LatchedDays >> 8;
				}
			}
		}
	}

	return 0xFF;
}

//----------------------------------------------------------------------------------------------------
void GBMemBankController3::WriteMemory( uint16 u16Address, ubyte u8Data )
{
	if( u16Address >= 0x0000 && u16Address < 0x2000 )
	{
		m_bRamEnabled = 0 != ( u8Data & 0x0A );
	}
	else if( u16Address >= 0x2000 && u16Address < 0x4000 )
	{
		m_u8SelectedRomBank = u8Data & 0x7F;
	}
	else if( u16Address >= 0x4000 && u16Address < 0x6000 )
	{
		m_u8SelectedRamBank = u8Data;
	}
	else if( u16Address >= 0x6000 && u16Address < 0x8000 )
	{
		if( m_bHasTimer )
		{
			if(		!m_bLatched
				&&	1 == u8Data )
			{
				m_u8LatchedSeconds	= m_u8RtcSeconds;
				m_u8LatchedMinutes	= m_u8RtcMinutes;
				m_u8LatchedHours	= m_u8RtcHours;
				m_u16LatchedDays	= m_u16RtcDays;
			}

			m_bLatched = 1 == u8Data;
		}
	}
	else if( u16Address >= 0xA000 && u16Address < 0xC000 )
	{
		if( m_bRamEnabled )
		{
			if( m_u8SelectedRamBank < 4 )
			{
				uint32 u32RamOffset	= m_bRamEnabled ? ( ( 1 << 13 ) * ( m_u8SelectedRamBank ) ) : 0;
				uint32 u32Address = u16Address + u32RamOffset - 0xA000;
				
				m_pRamBank[ u32Address ] = u8Data;
			}
			else if( m_bHasTimer )
			{
				switch( m_u8SelectedRamBank )
				{
					case 0x08:
						m_u8RtcSeconds = u8Data & 0x3B;
					case 0x09:
						m_u8RtcMinutes = u8Data & 0x3B;
					case 0x0A:
						m_u8RtcHours = u8Data & 0x17;
					case 0x0B:
						m_u16RtcDays = m_u16RtcDays & 0xFF00 | u8Data & 0xFF;
					case 0x0C:
						m_u16RtcDays = ( ( u8Data & 0xC1 ) << 8 ) | m_u16RtcDays & 0x00FF;
				}
			}

			m_bRamDirty = true;
		}
	}
}

//----------------------------------------------------------------------------------------------------
bool GBMemBankController3::IsRamEnabled() const
{
	return m_bRamEnabled && NULL != m_pRamBank;
}
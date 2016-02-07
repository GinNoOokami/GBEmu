//====================================================================================================
// Filename:    GBMemBankController0.cpp
// Created by:  Jeff Padgham
// Description: Memory bank controller 0 - The simplest MBC which only has one memory bank to swap.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBMemBankController0.h"

//====================================================================================================
// Class
//====================================================================================================
GBMemBankController0::GBMemBankController0( ubyte* pRomBank ) :
    m_pRomBank( pRomBank )
{

}

//----------------------------------------------------------------------------------------------------
GBMemBankController0::~GBMemBankController0()
{

}

//----------------------------------------------------------------------------------------------------
ubyte GBMemBankController0::ReadRomBank( uint16 u16Address )
{
    return m_pRomBank[ u16Address ];
}

//----------------------------------------------------------------------------------------------------
ubyte GBMemBankController0::ReadRamBank( uint16 u16Address )
{
    // TODO: Implement MBC0 ram bank
    return 0x00;
}

//----------------------------------------------------------------------------------------------------
void GBMemBankController0::WriteMemory( uint16 u16Address, ubyte u8Data )
{

}
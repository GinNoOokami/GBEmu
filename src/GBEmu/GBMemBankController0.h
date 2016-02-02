#ifndef GBEMU_GBMBC0_H
#define GBEMU_GBMBC0_H

//====================================================================================================
// Filename:    GBMemBankController0.h
// Created by:  Jeff Padgham
// Description: Memory bank controller 0 - The simplest MBC which only has one memory bank to swap.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

#include "IGBMemBankController.h"

//====================================================================================================
// Class
//====================================================================================================

class GBMemBankController0 : public IGBMemBankController
{
public:
	GBMemBankController0( ubyte* pRomBank );
	~GBMemBankController0();

	ubyte			ReadRomBank( uint16 u16Address );
	ubyte			ReadRamBank( uint16 u16Address );
	void			WriteMemory( uint16 u16Address, ubyte u8Data );

	bool			IsRamEnabled() const			{ return false;		}
	bool			IsRamDirty() const				{ return false;		}
	void			SetRamDirty( bool bFlag )		{}

private:
	ubyte*			m_pRomBank;
};

#endif
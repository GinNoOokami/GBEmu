#ifndef GBEMU_GBMBC2_H
#define GBEMU_GBMBC2_H

//====================================================================================================
// Filename:    GBMemBankController2.h
// Created by:  Jeff Padgham
// Description: Memory bank controller 2.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

#include "IGBMemBankController.h"

//====================================================================================================
// Class
//====================================================================================================

class GBMemBankController2 : public IGBMemBankController
{
public:
	GBMemBankController2( ubyte* pRomBank, ubyte* pRamBank );
	~GBMemBankController2();

	ubyte			ReadRomBank( uint16 u16Address );
	ubyte			ReadRamBank( uint16 u16Address );
	void			WriteMemory( uint16 u16Address, ubyte u8Data );

	bool			IsRamEnabled() const;
	bool			IsRamDirty() const				{ return m_bRamDirty;		}
	inline void		SetRamDirty( bool bFlag )		{ m_bRamDirty = bFlag;		}

private:
	ubyte*			m_pRomBank;
	ubyte*			m_pRamBank;

	bool			m_bRamEnabled;
	bool			m_bRamDirty;
	ubyte			m_u8SelectedRomBank;
};

#endif
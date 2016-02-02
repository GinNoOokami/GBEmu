#ifndef GBEMU_GBMBC1_H
#define GBEMU_GBMBC1_H

//====================================================================================================
// Filename:    GBMemBankController1.h
// Created by:  Jeff Padgham
// Description: Memory bank controller 1.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

#include "IGBMemBankController.h"

//====================================================================================================
// Class
//====================================================================================================

class GBMemBankController1 : public IGBMemBankController
{
public:
	GBMemBankController1( ubyte* pRomBank, ubyte* pRamBank );
	~GBMemBankController1();

	ubyte			ReadRomBank( uint16 u16Address );
	ubyte			ReadRamBank( uint16 u16Address );
	void			WriteMemory( uint16 u16Address, ubyte u8Data );

	bool			IsRamEnabled() const;
	bool			IsRamDirty() const				{ return m_bRamDirty;		}
	inline void		SetRamDirty( bool bFlag )		{ m_bRamDirty = bFlag;		}

private:
	void			UpdateBankValues();

	ubyte*			m_pRomBank;
	ubyte*			m_pRamBank;

	ubyte			m_u8BankSelect0;
	ubyte			m_u8BankSelect1;

	bool			m_bRamEnabled;
	bool			m_bRamBankMode;
	bool			m_bRamDirty;
	ubyte			m_u8SelectedRomBank;
	ubyte			m_u8SelectedRamBank;
};

#endif
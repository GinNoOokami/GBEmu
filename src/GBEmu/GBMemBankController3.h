#ifndef GBEMU_GBMBC3_H
#define GBEMU_GBMBC3_H

//====================================================================================================
// Filename:    GBMemBankController3.h
// Created by:  Jeff Padgham
// Description: Memory bank controller 3.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

#include "IGBMemBankController.h"

//====================================================================================================
// Class
//====================================================================================================

class GBMemBankController3 : public IGBMemBankController
{
public:
    GBMemBankController3( ubyte* pRomBank, ubyte* pRamBank, bool bHasTimer = false );
    ~GBMemBankController3();

    ubyte           ReadRomBank( uint16 u16Address );
    ubyte           ReadRamBank( uint16 u16Address );
    void            WriteMemory( uint16 u16Address, ubyte u8Data );

    bool            IsRamEnabled() const;
    bool            IsRamDirty() const              { return m_bRamDirty;   }
    inline void     SetRamDirty( bool bFlag )       { m_bRamDirty = bFlag;  }

private:
    ubyte*          m_pRomBank;
    ubyte*          m_pRamBank;

    bool            m_bHasTimer;
    bool            m_bLatched;

    bool            m_bRamEnabled;
    bool            m_bRamDirty;
    ubyte           m_u8SelectedRomBank;
    ubyte           m_u8SelectedRamBank;

    ubyte           m_u8RtcSeconds;
    ubyte           m_u8RtcMinutes;
    ubyte           m_u8RtcHours;
    uint16          m_u16RtcDays;

    ubyte           m_u8LatchedSeconds;
    ubyte           m_u8LatchedMinutes;
    ubyte           m_u8LatchedHours;
    uint16          m_u16LatchedDays;
};

#endif
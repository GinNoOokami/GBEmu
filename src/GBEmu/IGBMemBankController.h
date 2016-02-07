#ifndef GBEMU_IGBMBC_H
#define GBEMU_IGBMBC_H

//====================================================================================================
// Filename:    IGBMemBankController.h
// Created by:  Jeff Padgham
// Description: Interface definition for all Game Boy memory bank controllers.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

//====================================================================================================
// Class
//====================================================================================================

class IGBMemBankController
{
public:
    virtual ubyte   ReadRomBank( uint16 u16Address )                = 0;
    virtual ubyte   ReadRamBank( uint16 u16Address )                = 0;
    virtual void    WriteMemory( uint16 u16Address, ubyte u8Data )  = 0;
    virtual bool    IsRamEnabled() const                            = 0;
    virtual bool    IsRamDirty() const                              = 0;
    virtual void    SetRamDirty( bool bFlag )                       = 0;
};

#endif
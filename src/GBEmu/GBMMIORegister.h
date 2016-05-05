#ifndef GBEMU_GBMMIOREGISTER_H
#define GBEMU_GBMMIOREGISTER_H

//====================================================================================================
// Filename:    GBMMIORegister.h
// Created by:  Jeff Padgham
// Description: An interface for a memory mapped IO register.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

//====================================================================================================
// Foward Declarations
//====================================================================================================

class GBMem;

//====================================================================================================
// Global Enums
//====================================================================================================

// Special registers
enum MMIORegister
{
    MMIOJoypad                  = 0xFF00,
    MMIODivider                 = 0xFF04,
    MMIOTimerCounter            = 0xFF05,
    MMIOTimerModulo             = 0xFF06,
    MMIOTimerControl            = 0xFF07,
    MMIOSoundVolume             = 0xFF24,
    MMIOSoundOutput             = 0xFF25,
    MMIOSoundControl            = 0xFF26,
    MMIOLCDControl              = 0xFF40,
    MMIOLCDStatus               = 0xFF41,
    MMIOScrollY                 = 0xFF42,
    MMIOScrollX                 = 0xFF43,
    MMIOLCDScanline             = 0xFF44,
    MMIOLCDYCompare             = 0xFF45,
    MMIODMATransfer             = 0xFF46,
    MMIOBGPalette               = 0xFF47,
    MMIOObjectPalette0          = 0xFF48,
    MMIOObjectPalette1          = 0xFF49,
    MMIOWindowY                 = 0xFF4A,
    MMIOWindowX                 = 0xFF4B,
    MMIOKey1                    = 0xFF4D, // CGB only
    MMIOInterruptFlags          = 0xFF0F,
    MMIOBiosDisabled            = 0xFF50,
    MMIOInterruptEnable         = 0xFFFF,
    MMIORegisterStart           = MMIOJoypad,
    MMIORegisterEnd             = MMIOInterruptEnable
};

//====================================================================================================
// Interface
//====================================================================================================
class GBMMIORegister
{
public:
    virtual ~GBMMIORegister() {}

protected:
    template <class T>
    void SetMMIORegisterHandlers( GBMem* pMem, MMIORegister eMMIORegister, ubyte (T::*fnReadHandler)() const, void (T::*fnWriteHandler)(ubyte) );
};

//====================================================================================================
// Global Typedefs
//====================================================================================================
typedef void ( GBMMIORegister::*MMIOWriteHandler )( ubyte u8Data );
typedef ubyte ( GBMMIORegister::*MMIOReadHandler )() const;

//====================================================================================================
template <class T>
void GBMMIORegister::SetMMIORegisterHandlers( GBMem* pMem, MMIORegister eMMIORegister, ubyte(T::*fnReadHandler)() const, void (T::*fnWriteHandler)(ubyte) )
{
    pMem->RegisterMMIOHandlers( eMMIORegister,
                                this,
                                static_cast<MMIOReadHandler>( fnReadHandler ),
                                static_cast<MMIOWriteHandler>( fnWriteHandler ) );
}

//====================================================================================================
// Class
//====================================================================================================
class GBNullMMIORegister : public GBMMIORegister
{
public:
    GBNullMMIORegister( void )                              { m_u8Data = 0x00;      }
    virtual ~GBNullMMIORegister() {}

    void    WriteRegisterNoop( ubyte u8Data )               { m_u8Data = u8Data;    }
    ubyte   ReadRegisterNoop() const                        { return m_u8Data;      }

private:
    ubyte   m_u8Data;
};
#endif

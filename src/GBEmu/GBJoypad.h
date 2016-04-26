#ifndef GBEMU_GBJOYPAD_H
#define GBEMU_GBJOYPAD_H

//====================================================================================================
// Filename:    GBJoypad.h
// Created by:  Jeff Padgham
// Description: This class emulates the joypad functionality of a Game Boy.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

#include "GBMMIORegister.h"

//====================================================================================================
// Foward Declarations
//====================================================================================================

class GBEmulator;
class GBMem;

//====================================================================================================
// Global Enums
//====================================================================================================

enum JoypadButton
{
    ButtonInvalid   = 0,
    ButtonA         = 1 << 0,
    ButtonB         = 1 << 1,
    ButtonSelect    = 1 << 2,
    ButtonStart     = 1 << 3,
    ButtonRight     = 1 << 4,
    ButtonLeft      = 1 << 5,
    ButtonUp        = 1 << 6,
    ButtonDown      = 1 << 7
};

//====================================================================================================
// Class
//====================================================================================================

class GBJoypad : public GBMMIORegister
{
public:
    GBJoypad( GBEmulator* pEmulator, GBMem* pMemoryModule );
    virtual ~GBJoypad();

    void            Reset();

    void            Update();
    void            SimulateKeyDown( JoypadButton button );
    void            SimulateKeyUp( JoypadButton button );

    inline ubyte    GetStateRegister() const                { return m_u8StateRegister;             }
    inline void     SetStateRegister( ubyte u8Data )        { m_u8StateRegister = u8Data & 0x3f;    }

private:
    GBEmulator*     m_pEmulator;
    GBMem*          m_pMem;

    ubyte           m_u8StateRegister;

    uint32          m_u32KeyStatus;
};

#endif
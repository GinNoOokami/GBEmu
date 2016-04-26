//====================================================================================================
// Filename:    GBJoypad.cpp
// Created by:  Jeff Padgham
// Description: This class emulates the joypad functionality of a Game Boy.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBJoypad.h"

#include "GBEmulator.h"
#include "GBMem.h"

#include "CProfileManager.h"
#include "CLog.h"
    
//====================================================================================================
// Class
//====================================================================================================
GBJoypad::GBJoypad( GBEmulator* pEmulator, GBMem* pMemoryModule ) :
    m_pEmulator( pEmulator ),
    m_pMem( pMemoryModule ),
    m_u8StateRegister( -1 ),
    m_u32KeyStatus( -1 )
{
    m_pMem->RegisterMMIOHandlers( MMIOJoypad,
                                  this,
                                  static_cast<MMIOReadHandler>( &GBJoypad::GetStateRegister ),
                                  static_cast<MMIOWriteHandler>( &GBJoypad::SetStateRegister ) );
}

//----------------------------------------------------------------------------------------------------
GBJoypad::~GBJoypad()
{

}

//----------------------------------------------------------------------------------------------------
void GBJoypad::Reset()
{
    m_u8StateRegister = -1;
    m_u32KeyStatus = -1;
}

//----------------------------------------------------------------------------------------------------
void GBJoypad::Update()
{
    PROFILE( "Joypad::Update" );
    //                     ________
    //                   7|        |
    //                   6|        |
    //        __________ 5|        |
    //       /  ________ 4| Joypad |
    //      /  /          |  Chip  |
    // Down |__|Start___ 3|        |
    // Up   |__|Select__ 2|        |
    // Left |__|B_______ 1|        |
    // Right|__|A_______ 0|________|

    // By default, input will poll high (ignoring bits 4 and 5)
    m_u8StateRegister |= 0xcf;

    // Check if we need to poll the input
    if( 0x30 != ( m_u8StateRegister & 0x30 ) )
    {
        int buttons = 0;
        if( 0 == ( m_u8StateRegister & 0x20 ) )
        {
            buttons |= ( m_u32KeyStatus & ButtonA );
            buttons |= ( m_u32KeyStatus & ButtonB );
            buttons |= ( m_u32KeyStatus & ButtonSelect );
            buttons |= ( m_u32KeyStatus & ButtonStart );
        }
        else if( 0 == ( m_u8StateRegister & 0x10 ) )
        {
            // Shift right by four to offset the upper bits of the bitmask into the lower four bits of the register
            buttons |= ( m_u32KeyStatus & ButtonRight ) >> 4;
            buttons |= ( m_u32KeyStatus & ButtonLeft ) >> 4;
            buttons |= ( m_u32KeyStatus & ButtonUp ) >> 4;
            buttons |= ( m_u32KeyStatus & ButtonDown ) >> 4;
        }

        m_u8StateRegister &= ( 0xf0 | buttons );

        // Raise an interrupt if any of the edges fell low
        // TODO: Input state currently does not persist across update cycles, so this interrupt may trigger more than intended
        if( 0x0f != ( m_u8StateRegister & 0x0f ) )
        {
            m_pEmulator->RaiseInterrupt( Input );
        }
    }
}

//----------------------------------------------------------------------------------------------------
void GBJoypad::SimulateKeyDown( JoypadButton button )
{
    // Turn bit off when key is down
    m_u32KeyStatus &= ~button;
}

//----------------------------------------------------------------------------------------------------
void GBJoypad::SimulateKeyUp( JoypadButton button )
{
    // Turn bit on when key is up
    m_u32KeyStatus |= button;
}
//====================================================================================================
// Filename:    GBJoypad.cpp
// Created by:  Jeff Padgham
// Description: This class emulates the joypad functionality of a Game Boy.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBJoypad.h"

#include "GBMem.h"
	
//====================================================================================================
// Class
//====================================================================================================
GBJoypad::GBJoypad( GBEmulator* pEmulator, GBMem* pMemoryModule ) :
	m_pEmulator( pEmulator ),
	m_pMem( pMemoryModule ),
	m_u32KeyStatus( -1 )
{
	m_pMem->WriteMMIO( MMIOJoypad, 0xFF );
}

//----------------------------------------------------------------------------------------------------
GBJoypad::~GBJoypad()
{

}


//----------------------------------------------------------------------------------------------------
void GBJoypad::Update()
{
	//                     ________
	//                   7|        |
	//                   6|        |
	//		  __________ 5|        |
	//		 /  ________ 4| Joypad |
	//	    /  /          |  Chip  |
	//  Down|__|Start___ 3|        |
	//	  Up|__|Select__ 2|        |
	//	Left|__|B_______ 1|        |
	// Right|__|A_______ 0|________|

	ubyte u8JoypadStatus = m_pMem->ReadMMIO( MMIOJoypad );
	if( 0x30 != ( u8JoypadStatus & 0x30 ) )
	{
		if( u8JoypadStatus & 0x10 )
		{
			u8JoypadStatus |= ( m_u32KeyStatus & ButtonA );
			u8JoypadStatus |= ( m_u32KeyStatus & ButtonB );
			u8JoypadStatus |= ( m_u32KeyStatus & ButtonSelect );
			u8JoypadStatus |= ( m_u32KeyStatus & ButtonStart );
		}
		else if( u8JoypadStatus & 0x20 )
		{
			u8JoypadStatus |= ( m_u32KeyStatus & ButtonRight )	>> 4;
			u8JoypadStatus |= ( m_u32KeyStatus & ButtonLeft )	>> 4;
			u8JoypadStatus |= ( m_u32KeyStatus & ButtonUp )		>> 4;
			u8JoypadStatus |= ( m_u32KeyStatus & ButtonDown )	>> 4;
		}
		
		// Reset the select bits
		u8JoypadStatus |= 0x30;

		// Update the joypad register
		m_pMem->WriteMMIO( MMIOJoypad, u8JoypadStatus );
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
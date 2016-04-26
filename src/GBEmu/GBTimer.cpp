//====================================================================================================
// Filename:    GBTimer.cpp
// Created by:  Jeff Padgham
// Description: The timer emulates the CPU clock rate at different frequencies and generates interrupts.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBTimer.h"

#include "GBEmulator.h"
#include "GBMem.h"
#include "GBCpu.h"

#include "CProfileManager.h"

//====================================================================================================
// Class
//====================================================================================================
GBTimer::GBTimer( GBEmulator* pEmulator, GBMem* pMemoryModule ) :
    m_pEmulator( pEmulator ),
    m_pMem( pMemoryModule ),
    m_u8ControlRegister( 0 ),
    m_u8DividerRegister( 0 ),
    m_u8CounterRegister( 0 ),
    m_u8ModuloRegister( 0 ),
    m_u32TimerCycles( 0 ),
    m_u32TimerDiv( 0 )
{
    SetMMIORegisterHandlers<GBTimer>( m_pMem, MMIOTimerControl, &GBTimer::GetControlRegister, &GBTimer::SetControlRegister );
    SetMMIORegisterHandlers<GBTimer>( m_pMem, MMIODivider,      &GBTimer::GetDividerRegister, &GBTimer::SetDividerRegister );
    SetMMIORegisterHandlers<GBTimer>( m_pMem, MMIOTimerCounter, &GBTimer::GetCounterRegister, &GBTimer::SetCounterRegister );
    SetMMIORegisterHandlers<GBTimer>( m_pMem, MMIOTimerModulo,  &GBTimer::GetModuloRegister,  &GBTimer::SetModuloRegister );
}

//----------------------------------------------------------------------------------------------------
GBTimer::~GBTimer()
{
}

//----------------------------------------------------------------------------------------------------
void GBTimer::Reset()
{
    m_u8ControlRegister    = 0;
    m_u8DividerRegister    = 0;
    m_u8CounterRegister    = 0;
    m_u8ModuloRegister     = 0;

    m_u32TimerCycles       = 0;
    m_u32TimerDiv          = 0;
}

//----------------------------------------------------------------------------------------------------
void GBTimer::Update( uint32 u32ElapsedClockCycles )
{
    PROFILE( "Timer::Update" );

    int multiples = u32ElapsedClockCycles >> 2;
    for( int i = 0; i < multiples; ++i )
    {
        // Update the div counter
        m_u32TimerDiv += 4;

        // Update the timer counter
        m_u32TimerCycles += 4;

        // Keep the counter from overflowing
        // TODO: Update MBC3's timer here
        if( m_u32TimerCycles >= GBCpu::CLOCK_SPEED )
        {
            m_u32TimerCycles -= GBCpu::CLOCK_SPEED;
        }

        if( m_u32TimerDiv >= 256 )
        {
            // Update the divider register
            ++m_u8DividerRegister;
            m_u32TimerDiv -= 256;
        }

        // Update the timer at the set frequency
        if( IsTimerEnabled( m_u8ControlRegister ) )
        {
            bool bUpdateTimer = false;

            switch( GetTimerMode( m_u8ControlRegister ) )
            {
                case Freq4096Hz:
                    if( 0 == ( m_u32TimerCycles & 1023 ) )
                    {
                        bUpdateTimer = true;
                    }
                    break;
                case Freq262144Hz:    
                    if( 0 == ( m_u32TimerCycles & 15 ) )
                    {
                        bUpdateTimer = true;
                    }
                    break;
                case Freq65536Hz:
                    if( 0 == ( m_u32TimerCycles & 63 ) )
                    {
                        bUpdateTimer = true;
                    }    
                    break;
                case Freq16384Hz:
                    if( 0 == ( m_u32TimerCycles & 255 ) )
                    {
                        bUpdateTimer = true;
                    }        
                    break;
            }

            if( bUpdateTimer )
            {
                if( 0xFF != m_u8CounterRegister )
                {
                    // Just write the updated value to TIMA
                    ++m_u8CounterRegister;
                }
                else
                {
                    // Set the TIMA value to TMA
                    m_u8CounterRegister = m_u8ModuloRegister;

                    // Raise a timer interrupt
                    m_pEmulator->RaiseInterrupt( Timer );
                }
            }
        }
    }
}
#ifndef GBEMU_GBTIMER_H
#define GBEMU_GBTIMER_H

//====================================================================================================
// Filename:    GBTimer.h
// Created by:  Jeff Padgham
// Description: The timer emulates the CPU clock rate at different frequencies and generates interrupts.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

//====================================================================================================
// Foward Declarations
//====================================================================================================

class GBEmulator;
class GBMem;

//====================================================================================================
// Class
//====================================================================================================

class GBTimer
{
    // Class enums
    enum Frequency
    {
        Freq4096Hz,
        Freq262144Hz,
        Freq65536Hz,
        Freq16384Hz
    };

public:
    // Constructor / destructor
    GBTimer( GBEmulator* pEmulator, GBMem* pMemoryModule );
    ~GBTimer( void );

    void            Reset();

    void            Update( uint32 u32ElapsedClockCycles );
    inline uint32   GetTotalCycles()                                { return m_u32TimerCycles;                  }

private:
    inline bool     IsTimerEnabled( ubyte u8TimerControl ) const    { return 0 != ( u8TimerControl & 0x04 );    }
    inline ubyte    GetTimerMode( ubyte u8TimerControl ) const      { return u8TimerControl & 0x03;             }

private:
    GBEmulator*     m_pEmulator;
    GBMem*          m_pMem;

    uint32          m_u32TimerCycles;
    uint32          m_u32TimerDiv;
};

#endif
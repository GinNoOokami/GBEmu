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

#include "GBMMIORegister.h"

//====================================================================================================
// Foward Declarations
//====================================================================================================

class GBEmulator;
class GBMem;

//====================================================================================================
// Class
//====================================================================================================

class GBTimer : public GBMMIORegister
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
    virtual ~GBTimer( void );

    void            Reset();

    void            Update( uint32 u32ElapsedClockCycles );
    inline uint32   GetTotalCycles()                                { return m_u32TimerCycles;                  }

    inline ubyte    GetControlRegister() const                      { return m_u8ControlRegister;               }
    inline void     SetControlRegister( ubyte u8Data )              { m_u8ControlRegister = u8Data;             }

    inline ubyte    GetDividerRegister() const                      { return m_u8DividerRegister;               }
    inline void     SetDividerRegister( ubyte u8Data )              { m_u8DividerRegister = 0;                  }

    inline ubyte    GetCounterRegister() const                      { return m_u8CounterRegister;               }
    inline void     SetCounterRegister( ubyte u8Data )              { m_u8CounterRegister = u8Data;             }

    inline ubyte    GetModuloRegister() const                       { return m_u8ModuloRegister;                }
    inline void     SetModuloRegister( ubyte u8Data )               { m_u8ModuloRegister = u8Data;              }

private:
    inline bool     IsTimerEnabled( ubyte u8TimerControl ) const    { return 0 != ( u8TimerControl & 0x04 );    }
    inline ubyte    GetTimerMode( ubyte u8TimerControl ) const      { return u8TimerControl & 0x03;             }

private:
    GBEmulator*     m_pEmulator;
    GBMem*          m_pMem;

    // MMIO registers
    ubyte           m_u8ControlRegister;
    ubyte           m_u8DividerRegister;
    ubyte           m_u8CounterRegister;
    ubyte           m_u8ModuloRegister;

    uint32          m_u32TimerCycles;
    uint32          m_u32TimerDiv;
};

#endif
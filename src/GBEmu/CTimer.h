#ifndef TIMER_H
#define TIMER_H

//====================================================================================================
// Filename:    CTimer.h
// Description: Class for hi-res timer.
// Created by:  Peter Chan
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include <windows.h>

//====================================================================================================
// Class
//====================================================================================================

class CTimer
{
public:
    // Accessor function for singleton instance
    static CTimer* Get( void );

public:
    // Function to startup and shutdown the timer
    HRESULT Initialize( void );
    HRESULT Terminate( void );

    // Function to update the timer can calculate the new elapsed time
    HRESULT Update( void );

    // Function to get the elapsed time since last frame
    float GetElapsedSeconds( void ) const;

    // Function to get the frame per second
    float GetFPS( void ) const;

protected:
    // Protected constructor for singleton
    CTimer( void );

private:
    static CTimer* s_Instance;  // Static instance for singleton

    // http://msdn2.microsoft.com/en-us/library/aa383713.aspx
    LARGE_INTEGER   m_TicksPerSecond;   // System clock frequency
    LARGE_INTEGER   m_LastTick;
    LARGE_INTEGER   m_CurrentTick;

    float   m_ElapsedSeconds;   // Time passed since the last call to update
    float   m_FPS;              // Calculated frame per second

    bool    m_Initialized;      // Init flag
};

//====================================================================================================
// Global Functions
//====================================================================================================

static CTimer* GTimer( void )
{
    return CTimer::Get();
}

#endif
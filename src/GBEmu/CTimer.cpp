//====================================================================================================
// Filename:    CTimer.cpp
// Created by:  Peter Chan
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "CTimer.h"

//====================================================================================================
// Statics
//====================================================================================================

CTimer* CTimer::s_Instance = NULL;

//====================================================================================================
// Class
//====================================================================================================

CTimer* CTimer::Get( void )
{
    // If we do not yet have an instance created
    if( NULL == s_Instance )
    {
        // Create a new instance
        s_Instance = new CTimer;
    }

    return s_Instance;
}

//----------------------------------------------------------------------------------------------------

CTimer::CTimer( void ) :
    m_ElapsedSeconds( 0.0f ),
    m_FPS( 0.0f ),
    m_Initialized( false )
{
    m_TicksPerSecond.QuadPart = 0;
    m_LastTick.QuadPart = 0;
    m_CurrentTick.QuadPart = 0;
}

//----------------------------------------------------------------------------------------------------

HRESULT CTimer::Initialize( void )
{
    // Get the system clock frequency, for time calculation later
    if( !QueryPerformanceFrequency( &m_TicksPerSecond ) )
    {
        return E_FAIL;
    }

    // Get the current tick count
    QueryPerformanceCounter( &m_CurrentTick );
    m_LastTick = m_CurrentTick;

    // Set flag
    m_Initialized = true;

    return S_OK;
}

//----------------------------------------------------------------------------------------------------

HRESULT CTimer::Terminate( void )
{
    // Reset values
    m_TicksPerSecond.QuadPart = 0;
    m_LastTick.QuadPart = 0;
    m_CurrentTick.QuadPart = 0;
    m_ElapsedSeconds = 0.0f;
    m_FPS = 0.0f;

    // Set flag
    m_Initialized = false;

    return S_OK;
}

//----------------------------------------------------------------------------------------------------

HRESULT CTimer::Update( void )
{
    // Make sure the timer is initialized
    if( !m_Initialized )
    {
        return E_FAIL;
    }

    // Get the current tick count
    QueryPerformanceCounter( &m_CurrentTick );

    // Calculate the elapsed time
    m_ElapsedSeconds =  static_cast< float >( m_CurrentTick.QuadPart - m_LastTick.QuadPart ) / m_TicksPerSecond.QuadPart;

    // Update the last tick count
    m_LastTick = m_CurrentTick;

    static float s_FrameSinceLastSecond = 0.0f;
    static float s_AccumulatedTime = 0.0f;

    // Calculate the FPS
    s_AccumulatedTime += m_ElapsedSeconds;
    s_FrameSinceLastSecond += 1.0f;

    // Update FPS is one second has passed
    if( s_AccumulatedTime >= 1.0f )
    {
        m_FPS = s_FrameSinceLastSecond / s_AccumulatedTime;
        s_AccumulatedTime = 0.0f;
        s_FrameSinceLastSecond = 0.0f;
    }
    
    return S_OK;
}

//----------------------------------------------------------------------------------------------------

float CTimer::GetElapsedSeconds( void ) const
{
    return m_ElapsedSeconds;
}

//----------------------------------------------------------------------------------------------------

float CTimer::GetFPS( void ) const
{
    return m_FPS;
}
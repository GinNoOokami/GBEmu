//====================================================================================================
// Filename:    CProfiler.cpp
// Created by:  Jeff Padgham
// Description: Performance profiler designed to target slow areas of the game.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "CProfiler.h"
#include <float.h>

//====================================================================================================
// Class
//====================================================================================================

CProfiler::CProfiler( const char* pProfileId ) :
    m_ProfileId( pProfileId ),
    m_Initialized( false ),
    m_Profiling( false ),
    m_AccumulatedTime( 0.0f ),
    m_ShortestTime( FLT_MAX ),
    m_LongestTime( 0.0f ),
    m_Count( 0 )
{
    // Empty
}

//----------------------------------------------------------------------------------------------------

CProfiler::~CProfiler( void )
{
    // Empty
}

//----------------------------------------------------------------------------------------------------

void CProfiler::StartProfile( void )
{
    if( !m_Initialized )
    {
        // Get the system clock frequency, for time calculation later
        if( !QueryPerformanceFrequency( &m_TicksPerSecond ) )
        {
            return;
        }

        // Set flag
        m_Initialized = true;
    }

    // Get the current tick count
    QueryPerformanceCounter( &m_StartTime );

    // Set the profiling flag to on
    m_Profiling = true;
}

//----------------------------------------------------------------------------------------------------

void CProfiler::StopProfile( void )
{
    if( m_Profiling )
    {
        // Get the current tick count
        QueryPerformanceCounter( &m_EndTime );

        // Increment the count by one
        m_Count++;

        // Update the total accumulated time
        float delta = static_cast< float >( m_EndTime.QuadPart - m_StartTime.QuadPart ) / m_TicksPerSecond.QuadPart;
        
        // Add to total time
        m_AccumulatedTime += delta;
        
        // Check if it's the longest loop
        if( delta > m_LongestTime )
        {
            m_LongestTime = delta;
        }
        
        // Check if it's the shortest loop
        if( delta < m_ShortestTime )
        {
            m_ShortestTime = delta;
        }

        // Turn profiling off
        m_Profiling = false;
    }
}

//----------------------------------------------------------------------------------------------------

float CProfiler::GetAverageTime( void )
{
    return ( m_AccumulatedTime / m_Count ) * 1000;
}

//----------------------------------------------------------------------------------------------------

float CProfiler::GetLongestTime( void )
{
    return m_LongestTime * 1000;
}

//----------------------------------------------------------------------------------------------------

float CProfiler::GetShortestTime( void )
{
    return m_ShortestTime * 1000;
}
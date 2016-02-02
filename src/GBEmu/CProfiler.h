#ifndef PROFILER_H
#define PROFILER_H

//====================================================================================================
// Filename:    CProfiler.h
// Created by:  Jeff Padgham
// Description: Performance profiler designed to target slow areas of the game.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include <windows.h>
#include <string>

//====================================================================================================
// Class
//====================================================================================================

class CProfiler
{
public:
    // Constructor / destructor
	CProfiler( const char* pProfileId );
    ~CProfiler( void );

	inline std::string GetProfileId() const		{ return m_ProfileId;	}

    // Functions to start and stop the profiler around suspicious code
    void StartProfile( void );
    void StopProfile( void );

    // Gets the average time it takes for the code to run (in ms )
    float GetAverageTime( void );
    
    // Gets the longest time it took for the code to run (in ms )
    float GetLongestTime( void );
    
    // Gets the shortest time it took for the code to run (in ms )
    float GetShortestTime( void );

    int   GetCount( void ) { return m_Count; }

    int             m_Count;
private:
	std::string		m_ProfileId;
    bool            m_Initialized;
    bool            m_Profiling;
    LARGE_INTEGER   m_TicksPerSecond;
    LARGE_INTEGER   m_StartTime;
    LARGE_INTEGER   m_EndTime;
    float           m_AccumulatedTime;
    float           m_ShortestTime;
    float           m_LongestTime;

};

#endif
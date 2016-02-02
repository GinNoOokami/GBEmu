//====================================================================================================
// Filename:    CProfileManager.h
// Created by:  Jeff Padgham
// Description: A real time profiling system that keeps track of multiple profiles via a hashmap. Just
//				use the macro PROFILE( tagname ) at the beginning of the code block you wish to profile.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "CProfileManager.h"

#include "CProfiler.h"
#include "CLog.h"

#include <vector>
#include <algorithm>

//====================================================================================================
// Globals
//====================================================================================================

CProfileManager* CProfileManager::s_Instance = NULL;

//====================================================================================================
// Class
//====================================================================================================

CProfileManager::CProfileManager( void )
{
    // Empty
}

//----------------------------------------------------------------------------------------------------

CProfileManager::~CProfileManager( void )
{
    DisplayProfiles();
    Cleanup();
}

//----------------------------------------------------------------------------------------------------

CProfileManager* CProfileManager::Get( void )
{
    if( NULL == s_Instance )
    {
        s_Instance = new CProfileManager;
    }

    return s_Instance;
}

//----------------------------------------------------------------------------------------------------

void CProfileManager::StartProfile( const char* pProfileId )
{
    std::string id( pProfileId );
    ProfileMapIter iter;
    iter = m_Profiles.find( id );
    if( iter != m_Profiles.end() )
    {
        CProfiler* pProfiler = ( *iter ).second;
        if( NULL != pProfiler )
        {
            pProfiler->StartProfile();
        }
    }
    else
    {
        m_Profiles[ id ] = new CProfiler( pProfileId );
        m_Profiles[ id ]->StartProfile();
    }
}

//----------------------------------------------------------------------------------------------------

void CProfileManager::StopProfile( const char* pProfileId )
{
    std::string id( pProfileId );
    if( !m_Profiles.empty() )
    {
        ProfileMapIter iter;
        iter = m_Profiles.find( id );
        if( iter != m_Profiles.end() )
        {
            CProfiler* pProfiler = ( *iter ).second;
            if( NULL != pProfiler )
            {
                pProfiler->StopProfile();
            }
        }
        else
        {
            Log()->Write( LOG_COLOR_YELLOW, "[Profile Manager] Start/stop mismatch! Trying to stop a profiler with tag \"%s\".", pProfileId );
        }
    }
}

//----------------------------------------------------------------------------------------------------

void CProfileManager::DisplayProfiles( void )
{
    ProfileMapIter iter;
	
	std::vector< CProfiler* > profileList;
	std::vector< CProfiler* >::iterator itr;

	// Push all of the entries into a vector to be sorted
    for( iter = m_Profiles.begin(); iter != m_Profiles.end(); ++iter )
    {
		profileList.push_back( (*iter).second );
    }

	// Sort the entries
	std::sort( profileList.begin(), profileList.end(), SortProfiles );
	
	// Display the entries
    for( itr = profileList.begin(); itr != profileList.end(); ++itr )
    {
        CProfiler* pProfiler = *itr;
        if( NULL != pProfiler )
        {
            Log()->Write( LOG_COLOR_WHITE, "\"%s\"\t-\tAverage: %fms\tShortest: %fms\tLongest: %fms\tCount: %d", 
						  pProfiler->GetProfileId().c_str(), 
						  pProfiler->GetAverageTime(), 
						  pProfiler->GetShortestTime(),
						  pProfiler->GetLongestTime(), 
						  pProfiler->GetCount() );
        }
    }
}

//----------------------------------------------------------------------------------------------------

void CProfileManager::Cleanup( void )
{
    ProfileMapIter iter;
    for( iter = m_Profiles.begin(); iter != m_Profiles.end(); ++iter )
    {
        if( NULL != ( *iter ).second )
        {
            delete ( *iter ).second;
            ( *iter ).second = NULL;
        }
    }
    m_Profiles.clear();
}

//----------------------------------------------------------------------------------------------------

int SortProfiles( CProfiler* a, CProfiler* b )
{
	return a->GetAverageTime() * a->GetCount() > b->GetAverageTime() * b->GetCount();
}
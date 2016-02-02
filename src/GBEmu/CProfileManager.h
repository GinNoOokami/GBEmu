#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

//====================================================================================================
// Filename:    CProfileManager.h
// Created by:  Jeff Padgham
// Description: A real time profiling system that keeps track of multiple profiles via a hashmap. Just
//				use the macro PROFILE( tagname ) at the beginning of the code block you wish to profile.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include <map>
#include <string>

//====================================================================================================
// Forward declarations
//====================================================================================================

class CProfiler;

//====================================================================================================
// Class
//====================================================================================================

class CProfileManager
{
    typedef std::map< std::string, CProfiler* > ProfileMap;
    typedef std::map< std::string, CProfiler* >::iterator ProfileMapIter;
public:
    void StartProfile( const char* pProfileId );
    void StopProfile( const char* pProfileId );

    static CProfileManager* Get( void );

    void DisplayProfiles( void );
    void Cleanup( void );
protected:
    CProfileManager( void );
    ~CProfileManager( void );

private:
    ProfileMap m_Profiles;

    // Static singleton instance
    static CProfileManager* s_Instance;
};

class CAutoProfile
{
public:
    CAutoProfile( const char* pTag )
    {
        CProfileManager::Get()->StartProfile( pTag );
        strcpy_s( m_Tag, pTag );
    }
    ~CAutoProfile( void )
    {
        CProfileManager::Get()->StopProfile( m_Tag );
    }
private:
    char m_Tag[1024];
};

//====================================================================================================
// Globals
//====================================================================================================

static CProfileManager* Profiler( void )
{
    return CProfileManager::Get();
}

int	SortProfiles( CProfiler* a, CProfiler* b );

#ifndef NOPROFILE
#define PROFILE( name ) CAutoProfile profile( ( const char* )( name ) )
#else
#define PROFILE( name ) (void)sizeof( name )
#endif

#endif
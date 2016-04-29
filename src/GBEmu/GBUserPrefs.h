#ifndef GBEMU_GBUSERPREFS_H
#define GBEMU_GBUSERPREFS_H

//====================================================================================================
// Filename:    GBUserPrefs.h
// Created by:  Jeff Padgham
// Description: A singleton class to load, save, and access user preferences for the emulator.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

#include <vector>
#include <map>
#include <string>

//====================================================================================================
// Namespaces
//====================================================================================================

using namespace std;

//====================================================================================================
// Class
//====================================================================================================

class GBUserPrefs
{
public:
    static GBUserPrefs* Instance();

public:
    void                Load();
    bool                IsBiosEnabled() const;
    void                LoadBiosData( ubyte* pDstBuffer );

private:
    const string&       GetPref( const string& strKey, const string& strDefault = "" ) const;

    bool                ValidateBiosFile() const;

    // TODO: Utility method, should probably be moved elsewhere
    vector<string>&     SplitString( const string& strInput, char chDelimiter, vector<string>& oOutput );

private:
    map<string,string>  m_UserPrefsMap;
    string              m_strBiosFilepath;
    bool                m_bBiosEnabled;

protected:
    // Protected constructor for singleton
    GBUserPrefs( void );
    ~GBUserPrefs( void );

private:
    // Static singleton instance
    static GBUserPrefs* s_pInstance;
};

//====================================================================================================
// Global Functions
//====================================================================================================

static GBUserPrefs* UserPrefs()
{
    return GBUserPrefs::Instance();
}

#endif
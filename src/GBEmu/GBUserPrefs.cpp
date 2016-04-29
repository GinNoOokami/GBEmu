//====================================================================================================
// Filename:    GBUserPrefs.cpp
// Created by:  Jeff Padgham
// Description: A singleton class to load, save, and access user preferences for the emulator.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBUserPrefs.h"

#include <fstream>
#include <sstream>

//====================================================================================================
// Globals
//====================================================================================================

GBUserPrefs* GBUserPrefs::s_pInstance = NULL;

//====================================================================================================
// Class
//====================================================================================================
GBUserPrefs::GBUserPrefs( void ) :
    m_bBiosEnabled( false )
{
}

//----------------------------------------------------------------------------------------------------
GBUserPrefs::~GBUserPrefs( void )
{
}

//----------------------------------------------------------------------------------------------------
GBUserPrefs* GBUserPrefs::Instance()
{
    if( NULL == s_pInstance )
    {
        s_pInstance = new GBUserPrefs();
    }

    return s_pInstance;
}

//----------------------------------------------------------------------------------------------------
void GBUserPrefs::Load()
{
    string strLine;
    vector<string> oKeyValue;
    ifstream oPrefsFile( "userprefs.ini" );
    if( oPrefsFile )
    {
        while( getline( oPrefsFile, strLine ) )
        {
            if( strLine.front() != ';' )
            {
                SplitString( strLine, '=', oKeyValue );
                if( oKeyValue.size() > 1 )
                {
                    m_UserPrefsMap[ oKeyValue[ 0 ] ] = oKeyValue[ 1 ];
                }
            }
        }
        oPrefsFile.close();
    }

    m_strBiosFilepath = GetPref( "bios_file" );
    m_bBiosEnabled = ValidateBiosFile();
}

//----------------------------------------------------------------------------------------------------
bool GBUserPrefs::IsBiosEnabled() const
{
    return m_bBiosEnabled;
}

//----------------------------------------------------------------------------------------------------
void GBUserPrefs::LoadBiosData( ubyte* pDstBuffer )
{
    ifstream oBiosFile( m_strBiosFilepath, ios::binary );
    if( oBiosFile )
    {
        oBiosFile.read( reinterpret_cast<char*>( pDstBuffer ), kGBBiosSizeBytes );
        oBiosFile.close();
    }
}

//----------------------------------------------------------------------------------------------------
const string& GBUserPrefs::GetPref( const string& strKey, const string& strDefault ) const
{
    map<string,string>::const_iterator iter = m_UserPrefsMap.find( strKey );
    if( m_UserPrefsMap.end() != iter )
    {
        return iter->second;
    }
    return strDefault;
}

//----------------------------------------------------------------------------------------------------
bool GBUserPrefs::ValidateBiosFile() const
{
    bool bValid = false;
    ifstream oBiosFile( m_strBiosFilepath );
    if( oBiosFile )
    {
        ubyte u8Bios[ kGBBiosSizeBytes ] = { 0 };
        oBiosFile.read( reinterpret_cast<char*>( u8Bios ), sizeof( u8Bios ) );

        // TODO: A simple 8bit xor checksum probably isn't sufficient to validate the bios, perform a more robust check in the future
        int iChecksum = 0;
        for( int i = 0; i < kGBBiosSizeBytes; ++i )
        {
            iChecksum += u8Bios[ i ] & 0xFF;
        }
        iChecksum = ( ( iChecksum ^ 0xFF ) + 1 ) & 0xFF;

        bValid = kGBBiosXorChecksum == iChecksum;
        oBiosFile.close();
    }

    return bValid;
}

//----------------------------------------------------------------------------------------------------
vector<string>& GBUserPrefs::SplitString( const string& strInput, char chDelimiter, vector<string>& oOutput )
{
    stringstream oStream( strInput );
    string strCurrent;

    while( getline( oStream, strCurrent, chDelimiter ) )
    {
        oOutput.push_back( ( strCurrent ) );
    }

    return oOutput;
}

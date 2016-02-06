//====================================================================================================
// File:        CLog.cpp
// Created by:  Peter Chan
//====================================================================================================

//====================================================================================================
//  Includes
//====================================================================================================

#include "CLog.h"

#include <cstdarg>
#include <ctime>
#include <windows.h>

//====================================================================================================
// Statics
//====================================================================================================

CLog* CLog::s_Instance = NULL;

//====================================================================================================
// Class
//====================================================================================================

CLog* CLog::Get( void )
{
    // If we do not yet have an instance created
    if( NULL == s_Instance )
    {
        // Create a new instance
        s_Instance = new CLog;
    }

    return s_Instance;
}

//----------------------------------------------------------------------------------------------------

CLog::CLog( void ) :
    m_pFilename( NULL ),
	m_pFile( NULL ),
    m_Initialized( false )
{
    // Empty
}

//----------------------------------------------------------------------------------------------------

void CLog::Initialize( const char* pFile )
{
    // Open a file for writing
	FILE* file = fopen( pFile, "wt" );

    // Check if we can open a file
	if( NULL == file )
    {
        MessageBox( NULL, "Error: Cannot open file for error log", "CLog", MB_OK | MB_ICONERROR );
        return;
    }

    // Write the header for our log file
	fprintf( file, "<HTML>\n<HEAD><TITLE>Application Log</TITLE></HEAD>\n<BODY BGCOLOR = \"#000000\">\n" );

    // Get time and date
    char time[ 32 ];
    char date[ 32 ];
	_strtime( time );
	_strdate( date );

    // Write the time and date into the file
	fprintf( file , "<FONT COLOR = \"#FFFFFF\">Log Started at %s on %s</FONT><BR><BR>\n", time, date );
	fprintf( file , "</BODY></HTML>" );

    // Close the file
	fclose( file );

    // Remember the file name for logging later
	m_pFilename = const_cast< char* >( pFile );

    // Set flag
    m_Initialized = true;

    // Create the console log and set the title
    AllocConsole();
    SetConsoleTitle( "Log" );
}

//----------------------------------------------------------------------------------------------------

void CLog::Terminate( void )
{
    // Clear the file name
    m_pFilename = NULL;

    // Set flag
    m_Initialized = false;

    // Destroy the console
    FreeConsole();
}

//----------------------------------------------------------------------------------------------------

void CLog::Write( const char* pColor, const char* pMessage, ... )
{
	BeginBatchWrite();

	if( NULL == m_pFile )
	{
		return;
	}

    // Get message
    va_list va;
    char msg[ 1024 ];
	va_start( va, pMessage );
	vsprintf( msg, pMessage, va );
	va_end( va );

    // Get time
    char time[ 32 ];
	_strtime( time );

    // Write the message and time into the file
	fprintf( m_pFile, "<FONT COLOR = \"%s\">%s&nbsp;&nbsp;&nbsp;&nbsp;%s</FONT><BR>\n", pColor, time, msg );
	fprintf( m_pFile, "</BODY></HTML>" );

	EndBatchWrite();

    // Get the handle to the console
    HANDLE console = GetStdHandle( STD_OUTPUT_HANDLE );

    // Grab the length of the string, add a newline
    size_t len = strlen( msg )+1;
    msg[len-1] = '\n';

    // Finally, write the message to the console
    WriteConsole( console, msg, len, NULL, NULL );
}

//----------------------------------------------------------------------------------------------------
void CLog::BeginBatchWrite()
{
	// Make sure we have initialized first
	if( !m_Initialized )
	{
		MessageBox( NULL, "Error: Log must be initialized first before writing any messages.", "CLog", MB_OK | MB_ICONERROR );
		return;
	}

	// Open a file for writing
	m_pFile = fopen( m_pFilename, "r+" );

	// Check if we can open a file
	if( NULL == m_pFile )
	{
		MessageBox( NULL, "Error: Cannot open file for error log", "CLog", MB_OK | MB_ICONERROR );
		return;
	}

	// Move file cursor to before the end of file body
	fseek( m_pFile, -14, SEEK_END );
}

//----------------------------------------------------------------------------------------------------
void CLog::EndBatchWrite()
{
	if( NULL != m_pFile )
	{
		// Close the file
		fclose( m_pFile );
		m_pFile = NULL;
	}
}

//----------------------------------------------------------------------------------------------------
void CLog::BatchWrite( const char* pColor, const char* pMessage, ... )
{
	if( NULL == m_pFile )
	{
		return;
	}

	// Get message
	va_list va;
	char msg[ 1024 ];
	va_start( va, pMessage );
	vsprintf( msg, pMessage, va );
	va_end( va );

	// Get time
	char time[ 32 ];
	_strtime( time );

	// Write the message and time into the file
	fprintf( m_pFile, "<FONT COLOR = \"%s\">%s&nbsp;&nbsp;&nbsp;&nbsp;%s</FONT><BR>\n", pColor, time, msg );
	fprintf( m_pFile, "</BODY></HTML>" );
}
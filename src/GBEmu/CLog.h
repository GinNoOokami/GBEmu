#ifndef LOG_H
#define LOG_H

//====================================================================================================
// File:        CLog.h
// Description: Singleton class for error log.
// Created by:  Peter Chan
//====================================================================================================

//====================================================================================================
// Defines
//====================================================================================================

// Predefined color code strings
#define LOG_COLOR_RED       "#FF0000"
#define LOG_COLOR_ORANGE	"#FF9900"
#define LOG_COLOR_YELLOW	"#FFFF00"
#define LOG_COLOR_GREEN		"#00FF00"
#define LOG_COLOR_BLUE		"#0000FF"
#define LOG_COLOR_PURPLE	"#FF00FF"
#define LOG_COLOR_BLACK		"#000000"
#define LOG_COLOR_WHITE		"#FFFFFF"
#define LOG_COLOR_GRAY		"#CCCCCC"

//====================================================================================================
// Includes
//====================================================================================================

#include <cstdio>

//====================================================================================================
// Class
//====================================================================================================

class CLog
{
public:
    // Accessor function for singleton instance
    static CLog* Get( void );

public:
	// Initialize the log
	void Initialize( const char* pFile = "Log.html" );

    // Terminate the log
    void Terminate( void );

	// Write to log
	void Write( const char* pColor, const char* pMessage, ... );

	// Batch log functions
	void BeginBatchWrite();
	void EndBatchWrite();
	void BatchWrite( const char* pColor, const char* pMessage, ... );

protected:
    // Protected constructor for singleton
	CLog( void );

private:
    static CLog* s_Instance;    // Static instance for singleton

	char* m_pFilename;          // Name of log file

	FILE* m_pFile;

    bool m_Initialized;         // Init flag
};

//====================================================================================================
// Global Functions
//====================================================================================================

static CLog* Log( void )
{
    return CLog::Get();
}

#endif
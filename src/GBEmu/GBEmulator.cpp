//====================================================================================================
// Filename:    GBEmulator.cpp
// Created by:  Jeff Padgham
// Description: The main Game Boy emulator component. This class ties all of the modules together and
//              acts like the system board.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBEmulator.h"

#include "emutypes.h"
#include "GBMem.h"
#include "GBCpu.h"
#include "GBGpu.h"
#include "GBTimer.h"
#include "GBJoypad.h"
#include "GBCartridge.h"

#include "CProfileManager.h"
#include "CTimer.h"

#include <windows.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <direct.h>
#include <io.h>

// Disable warnings in external library
#pragma warning( disable:4996 )
#include "NFont.h"
#pragma warning( default:4996 )

// TODO: Store these is a home/user directory, and ideally make it a user config
const char* GB_BATTERY_DIRECTORY = "battery\\";

//====================================================================================================
// Class
//====================================================================================================

GBEmulator::GBEmulator() :
    m_pMem( NULL ),
    m_pCpu( NULL ),
    m_pGpu( NULL ),
    m_pJoypad( NULL ),
    m_pTimer( NULL ),
    m_pCartridge( NULL ),
    m_bRunning( false ),
    m_bCartridgeLoaded( false ),
    m_bDebugPaused( false ),
    m_fNextFrame( 0 ),
    m_fElapsedTime( 0 ),
    m_u32LastFrameCycles( 0 ),
    m_pFpsText( NULL ),
    m_pWindow( NULL ),
    m_pRenderer( NULL )
{
}

//----------------------------------------------------------------------------------------------------
GBEmulator::~GBEmulator()
{
    Terminate();
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::Initialize()
{
    if( -1 == SDL_Init( SDL_INIT_EVERYTHING ) )
    {
        fprintf( stderr, "Failed to initialize SDL!\n" );
        exit( 1 );
    }

    if( -1 == SDL_CreateWindowAndRenderer(  GBScreenWidth * kScreenScaleFactor, 
                                            GBScreenHeight * kScreenScaleFactor, 
                                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE, 
                                            &m_pWindow, 
                                            &m_pRenderer ) )
    {
        fprintf( stderr, "Failed to initialize window!\n" );
        exit( 1 );
    }

    SDL_SetWindowTitle( m_pWindow, "Gameboy Emulator" );

    // Clear color is white
    SDL_SetRenderDrawColor( m_pRenderer, 255, 255, 255, 255 );

    // Create main display surface
    m_pTexture = SDL_CreateTexture( m_pRenderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    GBScreenWidth, 
                                    GBScreenHeight );

    // Make sure our battery directory exists
    if( 0 != _access( GB_BATTERY_DIRECTORY, 0 ) )
    {
        _mkdir( GB_BATTERY_DIRECTORY );
    }

    m_pMem          = new GBMem;
    m_pTimer        = new GBTimer( this, m_pMem );
    m_pCpu          = new GBCpu( m_pMem, m_pTimer );
    m_pGpu          = new GBGpu( this, m_pMem );
    m_pJoypad       = new GBJoypad( this, m_pMem );

    m_pCartridge    = new GBCartridge( m_pMem );

    m_pCpu->Initialize();

    TTF_Init();

    TTF_Font* pFont = TTF_OpenFont( "assets\\Charybdis.ttf", 24 );
    if( NULL == pFont )
    {
        char szBuffer[ 1024 ] = { 0 };
        GetWindowsDirectory( szBuffer, 1024 );
        sprintf_s( szBuffer, 1024, "%s%s", szBuffer, "\\fonts\\arial.ttf" );

        pFont = TTF_OpenFont( szBuffer, 14 );
    }
    m_pFpsText = new NFont;
    m_pFpsText->load( m_pRenderer, pFont, NFont::Color::Color() );

    GTimer()->Initialize();

    m_bInitialized = true;

    Reset();
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::Terminate()
{
    if( m_bInitialized )
    {
        StopAndUnloadCartridge();

        delete m_pCartridge;
        m_pCartridge = NULL;

        delete m_pJoypad;
        m_pJoypad = NULL;

        delete m_pGpu;
        m_pGpu = NULL;

        delete m_pCpu;
        m_pCpu = NULL;

        delete m_pTimer;
        m_pTimer = NULL;

        delete m_pMem;
        m_pMem = NULL;

        delete m_pFpsText;
        m_pFpsText = NULL;

        TTF_Quit();
        SDL_Quit();

        m_bInitialized = false;
    }
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::Run()
{
    m_bRunning = true;

#ifdef _DEBUG
    FILE * file = NULL;
    char szBuffer[ 1024 ];

    fopen_s( &file, "debug_load.ini", "r" );
    if( NULL != file )
    {
        while( fgets( szBuffer, 1024, file ) )
        {
            if(     strlen( szBuffer ) > 0
                &&  szBuffer[ 0 ] != ';' )
            {
                char* p = szBuffer;
                while( NULL != *(p++) )
                {
                    // NULL out newline char
                    if( *p == '\n' )
                    {
                        *p = '\0';
                    }
                }

                LoadCartridge( szBuffer );
                break;
            }
        }
        fclose( file );
    }
#endif

    while( m_bRunning )
    {
        SDL_Event oEvent;
        while( SDL_PollEvent( &oEvent ) )
        {
            switch( oEvent.type )
            {
                case SDL_KEYDOWN:
                    HandleShortcuts( &oEvent );
                    // Intentional fall-through
                case SDL_KEYUP:
                    SimulateInput( &oEvent );
                    break;

                case SDL_DROPFILE:
                    LoadCartridge( oEvent.drop.file );
                    SDL_free( oEvent.drop.file );
                    break;

                case SDL_QUIT:
                    m_bRunning = false;
                    break;
            }
        }

        Update();
    }

    Profiler()->DisplayProfiles();
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::Reset()
{
    m_u32LastFrameCycles    = 0;

    m_fElapsedTime          = 0.f;
    m_fNextFrame            = static_cast<float>( SDL_GetTicks() );

    m_pMem->Reset();
    m_pTimer->Reset();
    m_pCpu->Reset();
    m_pGpu->Reset();
    m_pJoypad->Reset();
    m_pCartridge->Reset();
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::LoadCartridge( const char* szFilepath )
{
    StopAndUnloadCartridge();

    if( m_pCartridge->LoadFromFile( szFilepath, GB_BATTERY_DIRECTORY ) )
    {
        m_bCartridgeLoaded = true;
    }
    else
    {
        SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Error loading ROM", "Unable to load the selected ROM.", m_pWindow );

        // Draw once to refresh the screen
        Draw();
    }
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::RaiseInterrupt( Interrupt interrupt )
{
    m_pCpu->RaiseInterrupt( interrupt );

    if( VBlank == interrupt )
    {
        // In order to avoid screen tearing, the draw must be done during the VSync
        Draw();
    }
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::Update()
{
    const float    k_fFrameRate     = static_cast<float>( GBCpu::CLOCK_SPEED ) / static_cast<float>( kMaxCyclesPerFrame );
    const float    k_fFrameTime     = 1000.f / static_cast<float>( k_fFrameRate );

    if( m_bCartridgeLoaded )
    {
        m_fElapsedTime += static_cast<float>( SDL_GetTicks() ) - m_fElapsedTime;

        if( m_fElapsedTime >= m_fNextFrame )
        {
            float fDelta = m_fElapsedTime - m_fNextFrame;

            // Make sure we don't render faster than our framerate
            fDelta = fDelta > k_fFrameTime ? k_fFrameTime : fDelta;

            // Set the next frame render time
            m_fNextFrame = m_fElapsedTime - fDelta + k_fFrameTime;

            if( !m_bDebugPaused )
            {
                Step();
            }

            GTimer()->Update();

            // If the cart has a battery and something has changed, we need to update our .sav file
            if(     m_pCartridge->HasBattery()
                &&  m_pCartridge->IsRamDirty() )
            {
                m_pCartridge->FlushRamToSaveFile( GB_BATTERY_DIRECTORY );
            }

            SDL_Delay( 1 );
        }
    }
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::Step()
{
    int iFrameCycles    = 0;
    int iStepCycles     = 0;

    // Handle the emulation for this frame
    while( iFrameCycles < kMaxCyclesPerFrame )
    {
        iStepCycles = 0;

        // Execute the next opcode
        iStepCycles += m_pCpu->ExecuteOpcode();

        // Handle interrupts
        iStepCycles += m_pCpu->HandleInterrupts();

        iFrameCycles += iStepCycles;

        // Update the GPU
        m_pGpu->Update( iStepCycles );

        // Update the joypad
        m_pJoypad->Update();
    }
    m_u32LastFrameCycles = iFrameCycles;
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::Draw()
{
    const uint32* pu32ScreenData     = m_pGpu->GetScreenData();

    SDL_RenderClear( m_pRenderer );

    if( m_bRunning && m_bCartridgeLoaded )
    {
        SDL_UpdateTexture( m_pTexture, NULL, pu32ScreenData, GBScreenWidth * sizeof( uint32 ) );
        SDL_RenderCopy( m_pRenderer, m_pTexture, NULL, NULL );
    }

    // Note that this speed indicator is not really accurate; it does not take into account actual elapsed frame time
    //m_pFpsText->draw( m_pFrameSurface, 0, GBScreenHeight * ScreenScaleFactor - 20, "%.1f%%", static_cast<float>( m_u32LastFrameCycles ) / 70224.f * 100.f );

    m_pFpsText->draw( m_pRenderer, 0, GBScreenHeight * kScreenScaleFactor - 20, "%.1f", GTimer()->GetFPS() );
    //m_pFpsText->draw( m_pFrameSurface, 0, GBScreenHeight * ScreenScaleFactor - 20, "0x%x", m_pMem->ReadMemory( 0xFF85 ) );
    
    SDL_RenderPresent( m_pRenderer );
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::StopAndUnloadCartridge()
{
    m_pCartridge->Unload();
    m_bCartridgeLoaded = false;
    
    Reset();
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::SimulateInput( SDL_Event *pEvent )
{
    JoypadButton button = ButtonInvalid;

    switch( pEvent->key.keysym.sym )
    {
        case SDLK_a:
            button = ButtonA;
            break;
        case SDLK_s:
            button = ButtonB;
            break;
        case SDLK_RETURN:
            button = ButtonStart;
            break;
        case SDLK_RSHIFT:
            button = ButtonSelect;
            break;
        case SDLK_UP:
            button = ButtonUp;
            break;
        case SDLK_DOWN:
            button = ButtonDown;
            break;
        case SDLK_LEFT:
            button = ButtonLeft;
            break;
        case SDLK_RIGHT:
            button = ButtonRight;
            break;
    }

    if( ButtonInvalid != button )
    {
        if( SDL_KEYDOWN == pEvent->type )
        {
            m_pJoypad->SimulateKeyDown( button );
        }
        else
        {
            m_pJoypad->SimulateKeyUp( button );
        }
    }
}

//----------------------------------------------------------------------------------------------------
void GBEmulator::HandleShortcuts( SDL_Event* pEvent )
{
    switch( pEvent->key.keysym.sym )
    {
        case SDLK_r:
            Reset();
            break;
        case SDLK_SPACE:
            m_bDebugPaused = !m_bDebugPaused;
            break;
        case SDLK_RIGHT:
            if( m_bDebugPaused )
            {
                Step();
            }
            break;
    }
}
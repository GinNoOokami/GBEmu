#ifndef GBEMU_GBEMULATOR_H
#define GBEMU_GBEMULATOR_H

//====================================================================================================
// Filename:    GBEmulator.h
// Created by:  Jeff Padgham
// Description: The main Game Boy emulator component. This class ties all of the modules together and
//              acts like the system board.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

//====================================================================================================
// Namespaces
//====================================================================================================

using namespace std;

//====================================================================================================
// Foward Declarations
//====================================================================================================

class GBMem;
class GBCpu;
class GBGpu;
class GBTimer;
class GBJoypad;
class GBCartridge;

class NFont;
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
union SDL_Event;

//====================================================================================================
// Global enums
//====================================================================================================
enum Interrupt
{
    VBlank      = 1 << 0,
    LCDStatus   = 1 << 1,
    Timer       = 1 << 2,
    Serial      = 1 << 3,
    Input       = 1 << 4
};

//====================================================================================================
// Class
//====================================================================================================

class GBEmulator
{
    // Internal constants
    enum
    {
        kScreenScaleFactor = 4, // TODO: Make this more flexible, such as dynamic based on the window size or provide static scaling options in a menu
        kMaxCyclesPerFrame = 70224 // 154 scanlines (including vblank) * 456 clock cycles per line (see lcd timing docs)
    };

public:
    typedef void (GBEmulator::*GBInterruptHandler)( Interrupt interrupt );

public:
    // Constructor / destructor
    GBEmulator();
    ~GBEmulator();

    void    Initialize();
    void    Terminate();

    void    Run();
    void    Reset();

    void    LoadCartridge( const char* szFilepath );
    void    RaiseInterrupt( Interrupt interrupt );

private:
    void    Update();
    void    Step();
    void    Draw();

    void    SimulateInput( SDL_Event* pEvent );
    void    StopAndUnloadCartridge();
    void    HandleShortcuts( SDL_Event* oEvent );

private:
    GBCpu*          m_pCpu;
    GBMem*          m_pMem;
    GBGpu*          m_pGpu;
    GBTimer*        m_pTimer;
    GBJoypad*       m_pJoypad;

    GBCartridge*    m_pCartridge;

    bool            m_bInitialized;
    bool            m_bRunning;
    bool            m_bCartridgeLoaded;

    bool            m_bDebugPaused;

    float           m_fElapsedTime;
    float           m_fNextFrame;
    float           m_fLastFrame;
    float           m_fIdleTime;
    float           m_fAvgIdleTime;
    uint32          m_u32TotalFrames;
    uint32          m_u32LastFrameCycles;

    SDL_Window*     m_pWindow;
    SDL_Renderer*   m_pRenderer;
    SDL_Texture*    m_pTexture;

    NFont*          m_pFpsText;

};

#endif
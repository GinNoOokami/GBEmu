#ifndef GBEMU_GBEMULATOR_H
#define GBEMU_GBEMULATOR_H

//====================================================================================================
// Filename:    GBEmulator.h
// Created by:  Jeff Padgham
// Description: The main Game Boy emulator component. This class ties all of the modules together and
//				acts like the system board.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

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
struct SDL_Surface;
union SDL_Event;

//====================================================================================================
// Global enums
//====================================================================================================
enum Interrupt
{
	VBlank		= 1 << 0,
	LCDStatus	= 1 << 1,
	Timer		= 1 << 2,
	Serial		= 1 << 3,
	Input		= 1 << 4
};

//====================================================================================================
// Class
//====================================================================================================

class GBEmulator
{
	enum
	{
		ScreenScaleFactor = 4
	};

public:
	typedef void (GBEmulator::*GBInterruptHandler)( Interrupt interrupt );

public:
    // Constructor / destructor
    GBEmulator();
    ~GBEmulator();

	void	Initialize();
	void	Terminate();

	void	Run();
	void	Reset();

	void	LoadCartridge( const char* szFilepath );
	void	RaiseInterrupt( Interrupt interrupt );

private:
	void	Update();
	void	Draw();

	void	SimulateInput( SDL_Event* pEvent );
	void	StopAndUnloadCartridge();

private:
	GBCpu*			m_pCpu;
	GBMem*			m_pMem;
	GBGpu*			m_pGpu;
	GBTimer*		m_pTimer;
	GBJoypad*		m_pJoypad;

	GBCartridge*	m_pCartridge;

	bool			m_bInitialized;
	bool			m_bRunning;
	bool			m_bCartridgeLoaded;

	float			m_fElapsedTime;
	float			m_fNextFrame;
	uint32			m_u32LastFrameCycles;

	SDL_Surface*	m_pFrameSurface;

	NFont*			m_pFpsText;

};

#endif
#ifndef GBEMU_GBGPU_H
#define GBEMU_GBGPU_H

//====================================================================================================
// Filename:    GBGpu.h
// Created by:  Jeff Padgham
// Description: The GPU module controls the LCD screen and updates the graphics.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

#include <stdio.h>

//====================================================================================================
// Foward Declarations
//====================================================================================================

class GBEmulator;
class GBMem;

//====================================================================================================
// Global enums
//====================================================================================================
enum
{
	GBScreenWidth		= 160,
	GBScreenHeight		= 144
};

//====================================================================================================
// Class
//====================================================================================================

class GBGpu
{
	enum
	{
		PaletteWhite	=	0x00,
		PaletteLight	=	0x01,
		PaletteMedium	=	0x02,
		PaletteDark		=	0x03
	};

	enum LCDColor
	{
		ColorWhite		=	0xff9bbc0f,
		ColorLight		=	0xff82a80f,
		ColorMedium		=	0xff306230,
		ColorDark		=	0xff0f380f
	};

	enum LCDMode
	{
		ModeHBlank,
		ModeVBlank,
		ModeOam,
		ModeOamRam
	};

	enum LCDStatInterrupt
	{
		LCDIntHBlank	= 1 << 3,
		LCDIntVBlank	= 1 << 4,
		LCDIntOam		= 1 << 5,
		LCDIntLyc		= 1 << 6
	};

	enum VRamAddressMap
	{
		BgTileMapSelect0		= 0x9800, /*0=9800-9BFF*/
		BgTileMapSelect1		= 0x9C00, /*1=9C00-9FFF*/
		WindowTileMapSelect0	= 0x9800, /*0=9800-9BFF*/
		WindowTileMapSelect1	= 0x9C00, /*1=9C00-9FFF*/
		TileDataSelect0			= 0x9000, /*0=8800-97FF*/ // Tile index is -128 to 127, so base address is in the middle of the range (ie - $9000)
		TileDataSelect1			= 0x8000, /*1=8000-8FFF*/
		SpriteDataSelect		= 0x8000,
	};

	enum OamAttributeFlags
	{
		OamAttrPalette			= 0x10,
		OamAttrFlipX			= 0x20,
		OamAttrFlipY			= 0x40,
		OamAttrPriority			= 0x80
	};

public:
    // Constructor / destructor
    GBGpu( GBEmulator* pEmulator, GBMem* pMemoryModule );
    ~GBGpu( void );

	void			Reset();

	void			Update( uint32 u32ElapsedClockCycles );
	const uint32*	GetScreenData() const;

	bool			IsVSyncOrHBlank() const															{ return m_bIsVSync || m_bIsHBlank;							}
	bool			IsVSync() const																	{ return m_bIsVSync;										}

	bool			IsLCDEnabled() const;
	uint32			GetBlankColor() const															{ return ColorWhite;										}

private:
	inline bool		IsLCDEnabled( ubyte u8LCDControl ) const										{ return 0 != ( u8LCDControl & 0x80 );						}
	inline bool		IsLCDInterruptEnabled( ubyte u8LCDStatus, LCDStatInterrupt interrupt ) const	{ return 0 != ( u8LCDStatus & interrupt );					}

	inline bool		IsBackgroundEnabled( ubyte u8LCDControl ) const									{ return 0 != ( u8LCDControl & 0x01 );						}
	inline bool		IsWindowEnabled( ubyte u8LCDControl ) const										{ return 0 != ( u8LCDControl & 0x20 );						}
	inline bool		IsSpriteEnabled( ubyte u8LCDControl ) const										{ return 0 != ( u8LCDControl & 0x02 );						}

	inline ubyte	GetSpriteSize( ubyte u8LCDControl ) const										{ return 0 != ( u8LCDControl & 0x04 ) ? 16 : 8;				}
	inline bool		GetTileDataSelect( ubyte u8LCDControl ) const									{ return 0 != ( u8LCDControl & 0x10 );						} /*(0=8800-97FF, 1=8000-8FFF)*/
	inline bool		GetBackgroundTileMapSelect( ubyte u8LCDControl ) const							{ return 0 != ( u8LCDControl & 0x08 );						} /*(0=9800-9BFF, 1=9C00-9FFF)*/
	inline bool		GetWindowTileMapSelect( ubyte u8LCDControl ) const								{ return 0 != ( u8LCDControl & 0x40 );						} /*(0=9800-9BFF, 1=9C00-9FFF)*/

	inline ubyte	GetLCDMode( ubyte u8LCDStatus ) const											{ return u8LCDStatus & 0x03;								}
	inline void		SetLCDMode( ubyte& u8LCDStatus, LCDMode mode ) const							{ u8LCDStatus &= 0xFC; u8LCDStatus |= ( mode & 0x03 );		}

	inline bool		GetCoincidenceFlag( ubyte u8LCDStatus ) const									{ return 0 != ( u8LCDStatus & 0x04 );						}
	inline void		SetCoincidenceFlag( ubyte& u8LCDStatus, bool bFlag ) const						{ u8LCDStatus &= 0xFB; u8LCDStatus |= ( bFlag << 2);	}

	void			DoScanline( ubyte& u8LCDControl, ubyte& u8LCDStatus );
	void			DrawLine( ubyte& u8LCDControl, ubyte u8Scanline );
	void			DrawBackground( ubyte u8LCDControl, ubyte u8Scanline );
	void			DrawWindow( ubyte u8LCDControl, ubyte u8Scanline );
	void			DrawSprites( ubyte u8LCDControl, ubyte u8Scanline );
	uint16			GetMapTileData( uint16 u16MapAddr, uint16 u16DataAddr, ubyte u8TileX, ubyte u8TileY, ubyte u8Row );
	uint16			GetSpriteTileData( ubyte u8TileIndex, ubyte u8Row );
	uint16			GetTileData( uint16 u16DataAddr );
	uint16			FlipSpriteHorizontal( uint16 u16TileData );

private:
	GBEmulator*		m_pEmulator;
	GBMem*			m_pMem;

	LCDColor		m_PaletteLookup[ 4 ];
	uint32			m_u32ScanTime;
	bool			m_bIsVSync;
	bool			m_bIsHBlank;

	uint32			m_u32ScreenData[ GBScreenWidth * GBScreenHeight ];
};

#endif
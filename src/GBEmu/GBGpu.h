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

#include "GBMMIORegister.h"

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
    GBScreenWidth       = 160,
    GBScreenHeight      = 144
};

//====================================================================================================
// Class
//====================================================================================================

class GBGpu : public GBMMIORegister
{
    enum
    {
        PaletteWhite    = 0x00,
        PaletteLight    = 0x01,
        PaletteMedium   = 0x02,
        PaletteDark     = 0x03
    };

    enum LCDColor
    {
        ColorWhite      = 0xff9bbc0f,
        ColorLight      = 0xff82a80f,
        ColorMedium     = 0xff306230,
        ColorDark       = 0xff0f380f
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
        LCDIntHBlank    = 1 << 3,
        LCDIntVBlank    = 1 << 4,
        LCDIntOam       = 1 << 5,
        LCDIntLyc       = 1 << 6
    };

    enum VRamAddressMap
    {
        BgTileMapSelect0        = 0x9800, /*0=9800-9BFF*/
        BgTileMapSelect1        = 0x9C00, /*1=9C00-9FFF*/
        WindowTileMapSelect0    = 0x9800, /*0=9800-9BFF*/
        WindowTileMapSelect1    = 0x9C00, /*1=9C00-9FFF*/
        TileDataSelect0         = 0x9000, /*0=8800-97FF*/ // Tile index is -128 to 127, so base address is in the middle of the range (ie - $9000)
        TileDataSelect1         = 0x8000, /*1=8000-8FFF*/
        SpriteDataSelect        = 0x8000,
    };

    enum OamAttributeFlags
    {
        OamAttrPalette          = 0x10,
        OamAttrFlipX            = 0x20,
        OamAttrFlipY            = 0x40,
        OamAttrPriority         = 0x80
    };

public:
    // Constructor / destructor
    GBGpu( GBEmulator* pEmulator, GBMem* pMemoryModule );
    virtual ~GBGpu( void );

    void            Reset();

    void            Update( uint32 u32ElapsedClockCycles );
    const uint32*   GetScreenData() const;

    bool            IsVSyncOrHBlank() const                                     { return m_bIsVSync || m_bIsHBlank;                         }
    bool            IsVSync() const                                             { return m_bIsVSync;                                        }

    uint32          GetBlankColor() const                                       { return ColorWhite;                                        }

    ubyte           GetLCDControlRegister() const                               { return m_u8LCDControl;                                    }
    void            SetLCDControlRegister( ubyte u8Data )                       { m_u8LCDControl = u8Data;                                  }

    ubyte           GetLCDStatusRegister() const                                { return m_u8LCDStatus;                                     }
    void            SetLCDStatusRegister( ubyte u8Data )                        { m_u8LCDStatus = u8Data & 0x78;                            } // Lower 3 bits are read-only, msb is unused
    
    ubyte           GetScrollXRegister() const                                  { return m_u8ScrollX;                                       }
    void            SetScrollXRegister( ubyte u8Data )                          { m_u8ScrollX = u8Data;                                     }

    ubyte           GetScrollYRegister() const                                  { return m_u8ScrollY;                                       }
    void            SetScrollYRegister( ubyte u8Data )                          { m_u8ScrollY = u8Data;                                     }

    ubyte           GetLCDScanlineRegister() const                              { return m_u8LCDScanline;                                   }
    void            SetLCDScanlineRegister( ubyte u8Data )                      { m_u8LCDScanline = 0;                                      } // Writing to scanline resets it

    ubyte           GetLCDYCompareRegister() const                              { return m_u8LCDYCompare;                                   }
    void            SetLCDYCompareRegister( ubyte u8Data )                      { m_u8LCDYCompare = u8Data;                                 }
    
    ubyte           GetBGPaletteRegister() const                                { return m_u8BGPalette;                                     }
    void            SetBGPaletteRegister( ubyte u8Data )                        { m_u8BGPalette = u8Data;                                   }
    
    ubyte           GetObjectPalette0Register() const                           { return m_u8ObjectPalette0;                                }
    void            SetObjectPalette0Register( ubyte u8Data )                   { m_u8ObjectPalette0 = u8Data;                              }
    
    ubyte           GetObjectPalette1Register() const                           { return m_u8ObjectPalette1;                                }
    void            SetObjectPalette1Register( ubyte u8Data )                   { m_u8ObjectPalette1 = u8Data;                              }
    
    ubyte           GetWindowYRegister() const                                  { return m_u8WindowY;                                       }
    void            SetWindowYRegister( ubyte u8Data )                          { m_u8WindowY = u8Data;                                     }
    
    ubyte           GetWindowXRegister() const                                  { return m_u8WindowX;                                       }
    void            SetWindowXRegister( ubyte u8Data )                          { m_u8WindowX = u8Data;                                     }
    
    ubyte           GetDMATransferRegister() const                              { return m_u8DMATransfer;                                   }
    void            SetDMATransferRegister( ubyte u8Data );
private:
    inline bool     IsLCDEnabled() const                                        { return 0 != ( m_u8LCDControl & 0x80 );                    }
    inline bool     IsLCDInterruptEnabled( LCDStatInterrupt interrupt ) const   { return 0 != ( m_u8LCDStatus & interrupt );                }

    inline bool     IsBackgroundEnabled() const                                 { return 0 != ( m_u8LCDControl & 0x01 );                    }
    inline bool     IsWindowEnabled() const                                     { return 0 != ( m_u8LCDControl & 0x20 );                    }
    inline bool     IsSpriteEnabled() const                                     { return 0 != ( m_u8LCDControl & 0x02 );                    }

    inline ubyte    GetSpriteSize() const                                       { return 0 != ( m_u8LCDControl & 0x04 ) ? 16 : 8;           }
    inline bool     GetTileDataSelect() const                                   { return 0 != ( m_u8LCDControl & 0x10 );                    } /*(0=8800-97FF, 1=8000-8FFF)*/
    inline bool     GetBackgroundTileMapSelect() const                          { return 0 != ( m_u8LCDControl & 0x08 );                    } /*(0=9800-9BFF, 1=9C00-9FFF)*/
    inline bool     GetWindowTileMapSelect() const                              { return 0 != ( m_u8LCDControl & 0x40 );                    } /*(0=9800-9BFF, 1=9C00-9FFF)*/

    inline ubyte    GetLCDMode() const                                          { return m_u8LCDStatus & 0x03;                              }
    inline void     SetLCDMode(LCDMode mode )                                   { m_u8LCDStatus &= 0xFC; m_u8LCDStatus |= ( mode & 0x03 );  }

    inline bool     GetCoincidenceFlag() const                                  { return 0 != ( m_u8LCDStatus & 0x04 );                     }
    inline void     SetCoincidenceFlag(bool bFlag )                             { m_u8LCDStatus &= 0xFB; m_u8LCDStatus |= ( bFlag << 2);    }

    void            DoScanline();
    void            DrawLine();
    void            DrawBackground();
    void            DrawWindow();
    void            DrawSprites();
    uint16          GetMapTileData( uint16 u16MapAddr, uint16 u16DataAddr, ubyte u8TileX, ubyte u8TileY, ubyte u8Row );
    uint16          GetSpriteTileData( ubyte u8TileIndex, ubyte u8Row );
    uint16          GetTileData( uint16 u16DataAddr );
    uint16          FlipSpriteHorizontal( uint16 u16TileData );

private:
    GBEmulator*     m_pEmulator;
    GBMem*          m_pMem;

    // GPU registers
    ubyte           m_u8LCDControl;
    ubyte           m_u8LCDStatus;
    ubyte           m_u8ScrollX;
    ubyte           m_u8ScrollY;
    ubyte           m_u8LCDScanline;
    ubyte           m_u8LCDYCompare;
    ubyte           m_u8DMATransfer;
    ubyte           m_u8BGPalette;
    ubyte           m_u8ObjectPalette0;
    ubyte           m_u8ObjectPalette1;
    ubyte           m_u8WindowY;
    ubyte           m_u8WindowX;

    LCDColor        m_PaletteLookup[ 4 ];
    uint32          m_u32ScanTime;
    bool            m_bIsVSync;
    bool            m_bIsHBlank;

    uint32          m_u32ScreenData[ GBScreenWidth * GBScreenHeight ];
};

#endif
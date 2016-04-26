//====================================================================================================
// Filename:    GBGpu.cpp
// Created by:  Jeff Padgham
// Description: The GPU module controls the LCD screen and updates the graphics.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBGpu.h"

#include <memory.h>

#include "GBEmulator.h"
#include "GBMem.h"

#include "CProfileManager.h"
    
//====================================================================================================
// Class
//====================================================================================================
GBGpu::GBGpu( GBEmulator* pEmulator, GBMem* pMemoryModule ) :
    m_pEmulator( pEmulator ),
    m_pMem( pMemoryModule ),
    m_u8LCDControl( 0 ),
    m_u8LCDStatus( 0 ),
    m_u8ScrollX( 0 ),
    m_u8ScrollY( 0 ),
    m_u8LCDScanline( 0 ),
    m_u8LCDYCompare( 0 ),
    m_u8DMATransfer( 0 ),
    m_u8BGPalette( 0 ),
    m_u8ObjectPalette0( 0 ),
    m_u8ObjectPalette1( 0 ),
    m_u8WindowY( 0 ),
    m_u8WindowX( 0 ),
    m_u32ScanTime( 0 ),
    m_bIsVSync( false ),
    m_bIsHBlank( false )
{
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIOLCDControl,     &GBGpu::GetLCDControlRegister,      &GBGpu::SetLCDControlRegister );
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIOLCDStatus,      &GBGpu::GetLCDStatusRegister,       &GBGpu::SetLCDStatusRegister );
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIOScrollX,        &GBGpu::GetScrollXRegister,         &GBGpu::SetScrollXRegister );
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIOScrollY,        &GBGpu::GetScrollYRegister,         &GBGpu::SetScrollYRegister );
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIOLCDScanline,    &GBGpu::GetLCDScanlineRegister,     &GBGpu::SetLCDScanlineRegister );
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIOLCDYCompare,    &GBGpu::GetLCDYCompareRegister,     &GBGpu::SetLCDYCompareRegister );
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIODMATransfer,    &GBGpu::GetDMATransferRegister,     &GBGpu::SetDMATransferRegister );
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIOBGPalette,      &GBGpu::GetBGPaletteRegister,       &GBGpu::SetBGPaletteRegister );
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIOObjectPalette0, &GBGpu::GetObjectPalette0Register,  &GBGpu::SetObjectPalette0Register );
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIOObjectPalette1, &GBGpu::GetObjectPalette1Register,  &GBGpu::SetObjectPalette1Register );
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIOWindowY,        &GBGpu::GetWindowYRegister,         &GBGpu::SetWindowYRegister );
    SetMMIORegisterHandlers<GBGpu>( m_pMem, MMIOWindowX,        &GBGpu::GetWindowXRegister,         &GBGpu::SetWindowXRegister );

    Reset();

    m_PaletteLookup[ 0 ] = ColorWhite;
    m_PaletteLookup[ 1 ] = ColorLight;
    m_PaletteLookup[ 2 ] = ColorMedium;
    m_PaletteLookup[ 3 ] = ColorDark;
}

//----------------------------------------------------------------------------------------------------
GBGpu::~GBGpu()
{
}

//----------------------------------------------------------------------------------------------------
void GBGpu::Reset()
{
    for( int i = 0; i < GBScreenWidth * GBScreenHeight; ++i )
    {
        m_u32ScreenData[ i ] = ColorWhite;
    }

    m_u8LCDControl      = 0;
    m_u8LCDStatus       = 0;
    m_u8ScrollX         = 0;
    m_u8ScrollY         = 0;
    m_u8LCDScanline     = 0;
    m_u8LCDYCompare     = 0;
    m_u8DMATransfer     = 0;
    m_u8BGPalette       = 0;
    m_u8ObjectPalette0  = 0;
    m_u8ObjectPalette1  = 0;
    m_u8WindowY         = 0;
    m_u8WindowX         = 0;

    m_u32ScanTime       = 0;

    m_bIsVSync          = false;
    m_bIsHBlank         = false;
}

//----------------------------------------------------------------------------------------------------
void GBGpu::Update( uint32 u32ElapsedClockCycles )
{
    PROFILE( "Gpu::Update" );

    m_u32ScanTime += u32ElapsedClockCycles;

    switch( GetLCDMode() )
    {
        case ModeOam:
            if( m_u32ScanTime >= 80 )
            {
                SetLCDMode( ModeOamRam );
            }
            break;

        case ModeOamRam:
            if( m_u32ScanTime >= 252 )
            {
                SetLCDMode( ModeHBlank );

                if(     IsLCDEnabled()
                    &&  IsLCDInterruptEnabled( LCDIntHBlank ) )
                {
                    m_pEmulator->RaiseInterrupt( LCDStatus );
                }

                m_bIsHBlank = true;
            }
            break;
    }
    
    if( m_u32ScanTime >= 456 )
    {
        DoScanline();
    }

    if( !IsVSyncOrHBlank() )
    {
        //printf( "Scantime: %d LCD Mode: %d Scanline: %d\n", m_u32ScanTime, GetLCDMode( u8LCDStatus ), m_pMem->ReadMMIO( MMIOLCDScanline ) );
    }
}

//----------------------------------------------------------------------------------------------------
const uint32* GBGpu::GetScreenData() const
{
    return m_u32ScreenData;
}

//----------------------------------------------------------------------------------------------------
void GBGpu::SetDMATransferRegister( ubyte u8Data )
{
    m_u8DMATransfer = u8Data;
    for( int i = 0; i < 0x100; ++i )
    {
        m_pMem->WriteMemory( 0xFE00 + i, m_pMem->ReadMemory( ( u8Data << 8 ) + i ) );
    }
}

//----------------------------------------------------------------------------------------------------
void GBGpu::DoScanline()
{
    bool bLCDEnabled = IsLCDEnabled();
    bool bEndVsync   = false;

    m_u32ScanTime -= 456;

    if( m_u8LCDScanline < 144 )
    {
        // Render the current line
        if( bLCDEnabled )
        {
            DrawLine();
        }

        // Entering OAM mode
        SetLCDMode( ModeOam );

        m_bIsHBlank = false;
    }
    else if( m_u8LCDScanline == 144 )
    {
        // Entering V-Blank
        SetLCDMode( ModeVBlank );

        m_pEmulator->RaiseInterrupt( VBlank );

        // Raise an interrupt if we can
        if(     bLCDEnabled
            &&  IsLCDInterruptEnabled( LCDIntVBlank ) )
        {
            m_pEmulator->RaiseInterrupt( LCDStatus );
        }

        m_bIsVSync = true;
    }
    else if( m_u8LCDScanline > 153 )
    {
        // Raise an interrupt if we can
        if(     bLCDEnabled
            &&  IsLCDInterruptEnabled( LCDIntOam ) )
        {
            m_pEmulator->RaiseInterrupt( LCDStatus );
        }

        m_bIsVSync = false;
        bEndVsync = true;
    }

    // Set the coincidence flag if LY == LYC
    if( m_u8LCDYCompare == m_u8LCDScanline )
    {
        SetCoincidenceFlag( true );

        // Raise an interrupt if necessary
        if( IsLCDInterruptEnabled( LCDIntLyc ) )
        {
            m_pEmulator->RaiseInterrupt( LCDStatus );
        }
    }
    else
    {
        SetCoincidenceFlag( false );
    }

    if( !bEndVsync )
    {
        ++m_u8LCDScanline;
    }
    else
    {
        m_u8LCDScanline = 0;
    }
}

//----------------------------------------------------------------------------------------------------
void GBGpu::DrawLine()
{
    uint32 u32Offset = m_u8LCDScanline * GBScreenWidth;
    for( uint32 i = u32Offset; i < u32Offset + GBScreenWidth; ++i )
    {
        m_u32ScreenData[ i ] = ColorWhite;
    }

    if( IsBackgroundEnabled() )
    {
        DrawBackground();
    }

    if( IsWindowEnabled() )
    {
        DrawWindow();
    }

    if( IsSpriteEnabled() )
    {
        DrawSprites();
    }
}

//----------------------------------------------------------------------------------------------------
void GBGpu::DrawBackground()
{
    ubyte u8TileY           = ( ( ( m_u8LCDScanline + m_u8ScrollY ) & 0xFF ) >> 3 ) & 0x1F;
    ubyte u8TileX           = m_u8ScrollX >> 3;
    ubyte py                = ( m_u8ScrollY + m_u8LCDScanline ) & 7;
    ubyte px                = m_u8ScrollX & 7;
    uint16 u16TileDataAddr  = GetTileDataSelect() ? TileDataSelect1 : TileDataSelect0;
    uint16 u16TileMapAddr   = GetBackgroundTileMapSelect() ? BgTileMapSelect1 : BgTileMapSelect0;
    uint16 u16TileData      = GetMapTileData( u16TileMapAddr, u16TileDataAddr, u8TileX, u8TileY, py );
    ubyte palette           = 0;

    uint32 u32Offset = m_u8LCDScanline * GBScreenWidth;
    for( uint32 i = u32Offset; i < u32Offset + GBScreenWidth; ++i )
    {
        // Grab the palette index for the current pixel from the tile data 
        palette = ( u16TileData >> ( ( 7 - px++ ) << 1 ) ) & 3;

        // Clamp pixel data to 8
        px &= 7;

        // Fill in the screen data with the current pixel color
        m_u32ScreenData[ i ] = m_PaletteLookup[ ( m_u8BGPalette >> ( palette * 2 ) ) & 3 ];

        // If we're on a new tile, we need to grab new data
        if( 0 == px )
        {
            u16TileData = GetMapTileData( u16TileMapAddr, u16TileDataAddr, ++u8TileX, u8TileY, py );
        }
    }
}

//----------------------------------------------------------------------------------------------------
void GBGpu::DrawWindow()
{
    ubyte u8WindowY = m_u8WindowY;
    ubyte u8WindowX         = m_u8WindowX;
    ubyte u8TileY           = ( ( m_u8LCDScanline - m_u8WindowY ) & 0xFF ) >> 3;
    ubyte u8TileX           = 0;
    ubyte py                = ( m_u8LCDScanline - m_u8WindowY ) & 7;
    ubyte px                = 0;
    uint16 u16DeltaY        = m_u8LCDScanline - m_u8WindowY;

    if(     u8WindowX <= 166
        &&  u8WindowY <= 143
        &&  u16DeltaY < 144 )
    {
        uint16 u16TileDataAddr  = GetTileDataSelect() ? TileDataSelect1 : TileDataSelect0;
        uint16 u16TileMapAddr   = GetWindowTileMapSelect() ? WindowTileMapSelect1 : WindowTileMapSelect0;
        uint16 u16TileData      = GetMapTileData( u16TileMapAddr, u16TileDataAddr, u8TileX, u8TileY, py );
        ubyte palette           = 0;

        // LCD window is offset by 7 on the x axis
        u8WindowX -= 7;

        // TODO: Comment this behavior
        if( u8WindowX > 166 )
        {
            px = 7 - ( u8WindowX + 7 );
            u8WindowX = 0;
        }

        uint32 u32Offset = m_u8LCDScanline * GBScreenWidth;
        for( uint32 i = u32Offset + u8WindowX; i < u32Offset + GBScreenWidth; ++i )
        {
            // Grab the palette index for the current pixel from the tile data 
            palette = ( u16TileData >> ( ( 7 - px++ ) << 1 ) ) & 3;

            // Clamp pixel data to 8
            px &= 7;

            // Fill in the screen data with the current pixel color
            m_u32ScreenData[ i ] = m_PaletteLookup[ ( m_u8BGPalette >> ( palette * 2 ) ) & 3 ];

            // If we're on a new tile, we need to grab new data
            if( 0 == px )
            {
                u16TileData = GetMapTileData( u16TileMapAddr, u16TileDataAddr, ++u8TileX, u8TileY, py );
            }
        }
    }

}

//----------------------------------------------------------------------------------------------------
void GBGpu::DrawSprites()
{
    OamData oSpriteData;
    OamData arSpriteData[ 10 ]  = {};
    ubyte    u8Palette          = 0;
    ubyte    u8PaletteIndex     = 0;
    ubyte    u8Height           = GetSpriteSize();
    ubyte    u8LineY            = 0;
    uint16   u16TileData        = 0;
    uint32   u32Offset          = m_u8LCDScanline * GBScreenWidth;
    int      iSpriteCount       = 0;
    int      i;
    int      j;

    for( i = 0; i < 40 && iSpriteCount < 10; ++i )
    {
        oSpriteData = m_pMem->ReadSpriteData( i );
        
        // Only include the sprites that are within the current scanline
        // Negative unsigned overflow will fail the height check
        u8LineY = m_u8LCDScanline - ( oSpriteData.y - 16 );
        if( u8LineY < u8Height )
        {
            arSpriteData[ iSpriteCount++ ] = oSpriteData;
        }
    }
    
    // Sort the sprites on the x-axis, with the lower value being higher priority
    for( i = 0; i < iSpriteCount; ++i )
    {
        for( j = i + 1; j < iSpriteCount; ++j )
        {
            if( arSpriteData[ j ].x - 8 < arSpriteData[ i ].x - 8 )
            {
                oSpriteData = arSpriteData[ i ];
                arSpriteData[ i ] = arSpriteData[ j ];
                arSpriteData[ j ] = oSpriteData;
            }
        }
    }

    // Draw sprites backwards, so the lower ones have higher priority
    for( i = iSpriteCount - 1; i >= 0; --i )
    {
        oSpriteData = arSpriteData[ i ];

        ubyte sx    = oSpriteData.x - 8;
        ubyte tile    = oSpriteData.tileIndex;

        // Least significant bit is 0 if using 16 height sprite mode
        tile = tile & ~( ( m_u8LCDControl & 0x04 ) >> 2 );

        // Only include the sprites that are within the current scanline
        // Negative unsigned overflow will fail the height check
        u8LineY = m_u8LCDScanline - ( oSpriteData.y - 16 );

        // Swap the line if the Y flip attribute is set for this sprite
        if( oSpriteData.attributeFlags & OamAttrFlipY )
        {
            u8LineY ^= ( u8Height - 1 );
        }

        // Grab the current object palette
        u8Palette = 0 != ( oSpriteData.attributeFlags & OamAttrPalette ) ? m_u8ObjectPalette1 : m_u8ObjectPalette0;

        // Grab the current object tile data
        u16TileData = GetSpriteTileData( tile, u8LineY );

        // Swap the tile data bits if the X flip attribute is set for this sprite
        if( oSpriteData.attributeFlags & OamAttrFlipX )
        {
            u16TileData = FlipSpriteHorizontal( u16TileData );
        }

        for( ubyte px = 0; px < 8; ++px )
        {
            // Grab the palette index for the current pixel from the tile data 
            u8PaletteIndex = ( u16TileData >> ( ( 7 - px ) << 1 ) ) & 3;

            ubyte lx = sx + px;
            if( lx < GBScreenWidth )
            {
                // If the priority flag is set and the background/window color is not white, do not draw this pixel
                if(     0 == u8PaletteIndex
                    ||  (   oSpriteData.attributeFlags & OamAttrPriority
                        &&  ColorWhite != m_u32ScreenData[ u32Offset + lx ] ) )
                {
                    continue;
                }
                u8PaletteIndex = ( u8Palette >> ( u8PaletteIndex * 2 ) ) & 3;

                // Fill in the screen data with the current pixel color
                m_u32ScreenData[ u32Offset + lx ] = m_PaletteLookup[ u8PaletteIndex ];
                
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------
uint16 GBGpu::GetMapTileData( uint16 u16MapAddr, uint16 u16DataAddr, ubyte u8TileX, ubyte u8TileY, ubyte u8Row )
{
    ubyte u8TileIndex   = m_pMem->ReadMemory( u16MapAddr + ( u8TileY << 5 ) + ( u8TileX & 0x1F ) );
    u16DataAddr         = u16DataAddr + ( ( ( u16DataAddr == TileDataSelect1 ) ? u8TileIndex : static_cast<sbyte>( u8TileIndex ) ) << 4 ) + ( u8Row << 1 );

    return GetTileData( u16DataAddr );
}

//----------------------------------------------------------------------------------------------------
uint16 GBGpu::GetSpriteTileData( ubyte u8TileIndex, ubyte u8Row )
{
    uint16 u16DataAddr = SpriteDataSelect + ( u8TileIndex << 4 ) + ( u8Row << 1 );

    return GetTileData( u16DataAddr );
}

//----------------------------------------------------------------------------------------------------
uint16 GBGpu::GetTileData( uint16 u16DataAddr )
{
    ubyte u8RawDataLo   = m_pMem->ReadMemory( u16DataAddr );
    ubyte u8RawDataHi   = m_pMem->ReadMemory( u16DataAddr + 1 );
    uint16 u16TileData  = 0;

    // The raw tile format is difficult to use, so parse it into a more software-friendly format.
    // Give the following tile data: 0x00C6, the first byte represents the least significant bits and
    // the second byte represents the most significant bits. The gameboy hardware interprets these as 2 bit
    // values to form an index into the current palette.
    //
    // 00000000 -> 0x00  
    // 11000110 -> 0xC6
    // 
    // Will form the following like of pixel data (numbers represent palette index):
    // 22000220
    //
    // The formatted result will be an array of 2 bit values packed into a 16 bit integer. 
    // Following the above example, the result will look like:
    // 10100000 00101000 -> 0xA028

    for( int i = 7; i >= 0; --i )
    {
        // Extract the low and high bits and put them together
        ubyte hi        = ( u8RawDataHi & (1 << i) ) >> i;
        ubyte lo        = ( u8RawDataLo & (1 << i) ) >> i;
        ubyte palette   = hi << 1 | lo;

        // Concat the current palette index into our tile data
        u16TileData |= palette << i * 2;
    }

    return u16TileData;
}

//----------------------------------------------------------------------------------------------------
uint16 GBGpu::FlipSpriteHorizontal( uint16 u16TileData )
{
    uint16 u16FlippedData = 0;

    u16FlippedData |= ( u16TileData & 0x0003 ) << 14 | ( u16TileData & 0xC000 ) >> 14;
    u16FlippedData |= ( u16TileData & 0x000C ) << 10 | ( u16TileData & 0x3000 ) >> 10;
    u16FlippedData |= ( u16TileData & 0x0030 ) << 6  | ( u16TileData & 0x0C00 ) >> 6;
    u16FlippedData |= ( u16TileData & 0x00C0 ) << 2  | ( u16TileData & 0x0300 ) >> 2;

    return u16FlippedData;
}
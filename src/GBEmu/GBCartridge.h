#ifndef GBEMU_GBCARTRIDGE_H
#define GBEMU_GBCARTRIDGE_H

//====================================================================================================
// Filename:    GBCartridge.h
// Created by:  Jeff Padgham
// Description: Emulates a Game Boy cartridge.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

#include <fstream>

//====================================================================================================
// Namespaces
//====================================================================================================

using namespace std;

//====================================================================================================
// Foward Declarations
//====================================================================================================

class GBMem;
class IGBMemBankController;

//====================================================================================================
// Class
//====================================================================================================

class GBCartridge
{
    static const uint16        CARTRIDGE_ENTRY_POINT;

    // Class structs
    typedef struct
    {
        ubyte   u8EntryPoint[ 4 ];
        ubyte   u8NintendoLogo[ 48 ];
        char    szTitle[ 11 ];
        char    szManuCode[ 4 ];
        ubyte   u8CgbFlag;
        char    szNewLicenseeCode[ 2 ];
        ubyte   u8SgbFlag;
        ubyte   u8CartridgeType;
        ubyte   u8RomSize;
        ubyte   u8RamSize;
        ubyte   u8DestCode;
        ubyte   u8OldLicenseeCode;
        ubyte   u8RomVersion;
        ubyte   u8HeaderChecksum;
        uint16  u16GlobalChecksum;
    } GBCartridgeHeader;

public:
    // Constructor / destructor
    GBCartridge( GBMem* pMemoryModule );
    ~GBCartridge();

    void                    Reset();

    bool                    LoadFromFile( const char* szFilepath, const char* szBatteryDirectory );
    bool                    HasBattery() const;
    bool                    IsRamDirty() const;
    void                    FlushRamToSaveFile( const char* szBatteryDirectory );
    void                    Unload();

    inline bool             IsLoaded() const                        { return m_bLoaded;                 }
    inline ubyte            ReadRom( uint16 u16Address )            { return m_pRom[ u16Address ];      }

private:
    bool                    ValidateCartridgeHeader( ubyte* pHeaderData );
    void                    LoadCartridgeHeader( ubyte* pHeaderData );
    bool                    LoadCartridge( ifstream& oFile );
    void                    LoadBattery();
    IGBMemBankController*   CreateMemBankController( ubyte u8CartridgeType );
    uint32                  GetRomSize() const;
    uint32                  GetRamSize() const;

private:
    GBCartridgeHeader       m_oHeader;
    ubyte*                  m_pRom;
    ubyte*                  m_pRam;

    GBMem*                  m_pMem;
    IGBMemBankController*   m_pMemBankController;

    bool                    m_bLoaded;

    const char*             m_szBatteryDirectory;
};

#endif
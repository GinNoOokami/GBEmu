//====================================================================================================
// Filename:    GBCpu.cpp
// Created by:  Jeff Padgham
// Description: The CPU module for a Game Boy emulator. This is a modified version of the Z80 CPU.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBCpu.h"

#include "emutypes.h"
#include "GBMem.h"
#include "GBTimer.h"
#include "CProfileManager.h"
#include "CLog.h"

#include <stdio.h>

//====================================================================================================
// Static initializers
//====================================================================================================
const uint32 GBCpu::CLOCK_SPEED = 4194304;

//====================================================================================================
// Class
//====================================================================================================
GBCpu::GBCpu( GBMem* pMemoryModule, GBTimer* pTimer ) :
    m_pMem( pMemoryModule ),
    m_pTimer( pTimer ),
    m_PC( 0 ),
    m_SP( 0 ),
    m_bInitialized( false ),
    m_bHalt( false ),
    m_bStop( false ),
    m_DebugPC( 0 ),
    m_u32HistoryIndex( 0 )
{
}
    
//----------------------------------------------------------------------------------------------------
GBCpu::~GBCpu()
{
    Terminate();

    m_pMem = NULL;
}

//----------------------------------------------------------------------------------------------------
void GBCpu::Initialize()
{
    // Initialize regular opcode handlers
    m_OpcodeHandlers[ 0x00 ] = &GBCpu::OpNOP;
    m_OpcodeHandlers[ 0x01 ] = &GBCpu::OpLD_rr_nn<BC>;
    m_OpcodeHandlers[ 0x02 ] = &GBCpu::OpLD_rr_r<BC,A>;
    m_OpcodeHandlers[ 0x03 ] = &GBCpu::OpINC_rr<BC>;
    m_OpcodeHandlers[ 0x04 ] = &GBCpu::OpINC_r<B>;
    m_OpcodeHandlers[ 0x05 ] = &GBCpu::OpDEC_r<B>;
    m_OpcodeHandlers[ 0x06 ] = &GBCpu::OpLD_r_n<B>;
    m_OpcodeHandlers[ 0x07 ] = &GBCpu::OpRLCA;
    m_OpcodeHandlers[ 0x08 ] = &GBCpu::OpLD_nn_SP;
    m_OpcodeHandlers[ 0x09 ] = &GBCpu::OpADD_HL_rr<BC>;
    m_OpcodeHandlers[ 0x0a ] = &GBCpu::OpLD_r_rr<A,BC>;
    m_OpcodeHandlers[ 0x0b ] = &GBCpu::OpDEC_rr<BC>;
    m_OpcodeHandlers[ 0x0c ] = &GBCpu::OpINC_r<C>;
    m_OpcodeHandlers[ 0x0d ] = &GBCpu::OpDEC_r<C>;
    m_OpcodeHandlers[ 0x0e ] = &GBCpu::OpLD_r_n<C>;
    m_OpcodeHandlers[ 0x0f ] = &GBCpu::OpRRCA;
    m_OpcodeHandlers[ 0x10 ] = &GBCpu::OpSTOP;
    m_OpcodeHandlers[ 0x11 ] = &GBCpu::OpLD_rr_nn<DE>;
    m_OpcodeHandlers[ 0x12 ] = &GBCpu::OpLD_rr_r<DE,A>;
    m_OpcodeHandlers[ 0x13 ] = &GBCpu::OpINC_rr<DE>;
    m_OpcodeHandlers[ 0x14 ] = &GBCpu::OpINC_r<D>;
    m_OpcodeHandlers[ 0x15 ] = &GBCpu::OpDEC_r<D>;
    m_OpcodeHandlers[ 0x16 ] = &GBCpu::OpLD_r_n<D>;
    m_OpcodeHandlers[ 0x17 ] = &GBCpu::OpRLA;
    m_OpcodeHandlers[ 0x18 ] = &GBCpu::OpJR_n;
    m_OpcodeHandlers[ 0x19 ] = &GBCpu::OpADD_HL_rr<DE>;
    m_OpcodeHandlers[ 0x1a ] = &GBCpu::OpLD_r_rr<A,DE>;
    m_OpcodeHandlers[ 0x1b ] = &GBCpu::OpDEC_rr<DE>;
    m_OpcodeHandlers[ 0x1c ] = &GBCpu::OpINC_r<E>;
    m_OpcodeHandlers[ 0x1d ] = &GBCpu::OpDEC_r<E>;
    m_OpcodeHandlers[ 0x1e ] = &GBCpu::OpLD_r_n<E>;
    m_OpcodeHandlers[ 0x1f ] = &GBCpu::OpRRA;
    m_OpcodeHandlers[ 0x20 ] = &GBCpu::OpJR_NZ_n;
    m_OpcodeHandlers[ 0x21 ] = &GBCpu::OpLD_rr_nn<HL>;
    m_OpcodeHandlers[ 0x22 ] = &GBCpu::OpLDI_HL_A;
    m_OpcodeHandlers[ 0x23 ] = &GBCpu::OpINC_rr<HL>;
    m_OpcodeHandlers[ 0x24 ] = &GBCpu::OpINC_r<H>;
    m_OpcodeHandlers[ 0x25 ] = &GBCpu::OpDEC_r<H>;
    m_OpcodeHandlers[ 0x26 ] = &GBCpu::OpLD_r_n<H>;
    m_OpcodeHandlers[ 0x27 ] = &GBCpu::OpDAA;
    m_OpcodeHandlers[ 0x28 ] = &GBCpu::OpJR_Z_n;
    m_OpcodeHandlers[ 0x29 ] = &GBCpu::OpADD_HL_rr<HL>;
    m_OpcodeHandlers[ 0x2a ] = &GBCpu::OpLDI_A_HL;
    m_OpcodeHandlers[ 0x2b ] = &GBCpu::OpDEC_rr<HL>;
    m_OpcodeHandlers[ 0x2c ] = &GBCpu::OpINC_r<L>;
    m_OpcodeHandlers[ 0x2d ] = &GBCpu::OpDEC_r<L>;
    m_OpcodeHandlers[ 0x2e ] = &GBCpu::OpLD_r_n<L>;
    m_OpcodeHandlers[ 0x2f ] = &GBCpu::OpCPL;
    m_OpcodeHandlers[ 0x30 ] = &GBCpu::OpJR_NC_n;
    m_OpcodeHandlers[ 0x31 ] = &GBCpu::OpLD_SP_nn;
    m_OpcodeHandlers[ 0x32 ] = &GBCpu::OpLDD_HL_A;
    m_OpcodeHandlers[ 0x33 ] = &GBCpu::OpINC_SP;
    m_OpcodeHandlers[ 0x34 ] = &GBCpu::OpINC_HL;
    m_OpcodeHandlers[ 0x35 ] = &GBCpu::OpDEC_HL;
    m_OpcodeHandlers[ 0x36 ] = &GBCpu::OpLD_rr_n<HL>;
    m_OpcodeHandlers[ 0x37 ] = &GBCpu::OpSCF;
    m_OpcodeHandlers[ 0x38 ] = &GBCpu::OpJR_C_n;
    m_OpcodeHandlers[ 0x39 ] = &GBCpu::OpADD_HL_SP;
    m_OpcodeHandlers[ 0x3a ] = &GBCpu::OpLDD_A_HL;
    m_OpcodeHandlers[ 0x3b ] = &GBCpu::OpDEC_SP;
    m_OpcodeHandlers[ 0x3c ] = &GBCpu::OpINC_r<A>;
    m_OpcodeHandlers[ 0x3d ] = &GBCpu::OpDEC_r<A>;
    m_OpcodeHandlers[ 0x3e ] = &GBCpu::OpLD_r_n<A>;
    m_OpcodeHandlers[ 0x3f ] = &GBCpu::OpCCF;
    m_OpcodeHandlers[ 0x40 ] = &GBCpu::OpLD_r_r<B,B>;
    m_OpcodeHandlers[ 0x41 ] = &GBCpu::OpLD_r_r<B,C>;
    m_OpcodeHandlers[ 0x42 ] = &GBCpu::OpLD_r_r<B,D>;
    m_OpcodeHandlers[ 0x43 ] = &GBCpu::OpLD_r_r<B,E>;
    m_OpcodeHandlers[ 0x44 ] = &GBCpu::OpLD_r_r<B,H>;
    m_OpcodeHandlers[ 0x45 ] = &GBCpu::OpLD_r_r<B,L>;
    m_OpcodeHandlers[ 0x46 ] = &GBCpu::OpLD_r_rr<B,HL>;
    m_OpcodeHandlers[ 0x47 ] = &GBCpu::OpLD_r_r<B,A>;
    m_OpcodeHandlers[ 0x48 ] = &GBCpu::OpLD_r_r<C,B>;
    m_OpcodeHandlers[ 0x49 ] = &GBCpu::OpLD_r_r<C,C>;
    m_OpcodeHandlers[ 0x4a ] = &GBCpu::OpLD_r_r<C,D>;
    m_OpcodeHandlers[ 0x4b ] = &GBCpu::OpLD_r_r<C,E>;
    m_OpcodeHandlers[ 0x4c ] = &GBCpu::OpLD_r_r<C,H>;
    m_OpcodeHandlers[ 0x4d ] = &GBCpu::OpLD_r_r<C,L>;
    m_OpcodeHandlers[ 0x4e ] = &GBCpu::OpLD_r_rr<C,HL>;
    m_OpcodeHandlers[ 0x4f ] = &GBCpu::OpLD_r_r<C,A>;
    m_OpcodeHandlers[ 0x50 ] = &GBCpu::OpLD_r_r<D,B>;
    m_OpcodeHandlers[ 0x51 ] = &GBCpu::OpLD_r_r<D,C>;
    m_OpcodeHandlers[ 0x52 ] = &GBCpu::OpLD_r_r<D,D>;
    m_OpcodeHandlers[ 0x53 ] = &GBCpu::OpLD_r_r<D,E>;
    m_OpcodeHandlers[ 0x54 ] = &GBCpu::OpLD_r_r<D,H>;
    m_OpcodeHandlers[ 0x55 ] = &GBCpu::OpLD_r_r<D,L>;
    m_OpcodeHandlers[ 0x56 ] = &GBCpu::OpLD_r_rr<D,HL>;
    m_OpcodeHandlers[ 0x57 ] = &GBCpu::OpLD_r_r<D,A>;
    m_OpcodeHandlers[ 0x58 ] = &GBCpu::OpLD_r_r<E,B>;
    m_OpcodeHandlers[ 0x59 ] = &GBCpu::OpLD_r_r<E,C>;
    m_OpcodeHandlers[ 0x5a ] = &GBCpu::OpLD_r_r<E,D>;
    m_OpcodeHandlers[ 0x5b ] = &GBCpu::OpLD_r_r<E,E>;
    m_OpcodeHandlers[ 0x5c ] = &GBCpu::OpLD_r_r<E,H>;
    m_OpcodeHandlers[ 0x5d ] = &GBCpu::OpLD_r_r<E,L>;
    m_OpcodeHandlers[ 0x5e ] = &GBCpu::OpLD_r_rr<E,HL>;
    m_OpcodeHandlers[ 0x5f ] = &GBCpu::OpLD_r_r<E,A>;
    m_OpcodeHandlers[ 0x60 ] = &GBCpu::OpLD_r_r<H,B>;
    m_OpcodeHandlers[ 0x61 ] = &GBCpu::OpLD_r_r<H,C>;
    m_OpcodeHandlers[ 0x62 ] = &GBCpu::OpLD_r_r<H,D>;
    m_OpcodeHandlers[ 0x63 ] = &GBCpu::OpLD_r_r<H,E>;
    m_OpcodeHandlers[ 0x64 ] = &GBCpu::OpLD_r_r<H,H>;
    m_OpcodeHandlers[ 0x65 ] = &GBCpu::OpLD_r_r<H,L>;
    m_OpcodeHandlers[ 0x66 ] = &GBCpu::OpLD_r_rr<H,HL>;
    m_OpcodeHandlers[ 0x67 ] = &GBCpu::OpLD_r_r<H,A>;
    m_OpcodeHandlers[ 0x68 ] = &GBCpu::OpLD_r_r<L,B>;
    m_OpcodeHandlers[ 0x69 ] = &GBCpu::OpLD_r_r<L,C>;
    m_OpcodeHandlers[ 0x6a ] = &GBCpu::OpLD_r_r<L,D>;
    m_OpcodeHandlers[ 0x6b ] = &GBCpu::OpLD_r_r<L,E>;
    m_OpcodeHandlers[ 0x6c ] = &GBCpu::OpLD_r_r<L,H>;
    m_OpcodeHandlers[ 0x6d ] = &GBCpu::OpLD_r_r<L,L>;
    m_OpcodeHandlers[ 0x6e ] = &GBCpu::OpLD_r_rr<L,HL>;
    m_OpcodeHandlers[ 0x6f ] = &GBCpu::OpLD_r_r<L,A>;
    m_OpcodeHandlers[ 0x70 ] = &GBCpu::OpLD_rr_r<HL,B>;
    m_OpcodeHandlers[ 0x71 ] = &GBCpu::OpLD_rr_r<HL,C>;
    m_OpcodeHandlers[ 0x72 ] = &GBCpu::OpLD_rr_r<HL,D>;
    m_OpcodeHandlers[ 0x73 ] = &GBCpu::OpLD_rr_r<HL,E>;
    m_OpcodeHandlers[ 0x74 ] = &GBCpu::OpLD_rr_r<HL,H>;
    m_OpcodeHandlers[ 0x75 ] = &GBCpu::OpLD_rr_r<HL,L>;
    m_OpcodeHandlers[ 0x76 ] = &GBCpu::OpHALT;
    m_OpcodeHandlers[ 0x77 ] = &GBCpu::OpLD_rr_r<HL,A>;
    m_OpcodeHandlers[ 0x78 ] = &GBCpu::OpLD_r_r<A,B>;
    m_OpcodeHandlers[ 0x79 ] = &GBCpu::OpLD_r_r<A,C>;
    m_OpcodeHandlers[ 0x7a ] = &GBCpu::OpLD_r_r<A,D>;
    m_OpcodeHandlers[ 0x7b ] = &GBCpu::OpLD_r_r<A,E>;
    m_OpcodeHandlers[ 0x7c ] = &GBCpu::OpLD_r_r<A,H>;
    m_OpcodeHandlers[ 0x7d ] = &GBCpu::OpLD_r_r<A,L>;
    m_OpcodeHandlers[ 0x7e ] = &GBCpu::OpLD_r_rr<A,HL>;
    m_OpcodeHandlers[ 0x7f ] = &GBCpu::OpLD_r_r<A,A>;
    m_OpcodeHandlers[ 0x80 ] = &GBCpu::OpADD_r_r<A,B>;
    m_OpcodeHandlers[ 0x81 ] = &GBCpu::OpADD_r_r<A,C>;
    m_OpcodeHandlers[ 0x82 ] = &GBCpu::OpADD_r_r<A,D>;
    m_OpcodeHandlers[ 0x83 ] = &GBCpu::OpADD_r_r<A,E>;
    m_OpcodeHandlers[ 0x84 ] = &GBCpu::OpADD_r_r<A,H>;
    m_OpcodeHandlers[ 0x85 ] = &GBCpu::OpADD_r_r<A,L>;
    m_OpcodeHandlers[ 0x86 ] = &GBCpu::OpADD_r_rr<A,HL>;
    m_OpcodeHandlers[ 0x87 ] = &GBCpu::OpADD_r_r<A,A>;
    m_OpcodeHandlers[ 0x88 ] = &GBCpu::OpADC_r_r<A,B>;
    m_OpcodeHandlers[ 0x89 ] = &GBCpu::OpADC_r_r<A,C>;
    m_OpcodeHandlers[ 0x8a ] = &GBCpu::OpADC_r_r<A,D>;
    m_OpcodeHandlers[ 0x8b ] = &GBCpu::OpADC_r_r<A,E>;
    m_OpcodeHandlers[ 0x8c ] = &GBCpu::OpADC_r_r<A,H>;
    m_OpcodeHandlers[ 0x8d ] = &GBCpu::OpADC_r_r<A,L>;
    m_OpcodeHandlers[ 0x8e ] = &GBCpu::OpADC_r_rr<A,HL>;
    m_OpcodeHandlers[ 0x8f ] = &GBCpu::OpADC_r_r<A,A>;
    m_OpcodeHandlers[ 0x90 ] = &GBCpu::OpSUB_r_r<A,B>;
    m_OpcodeHandlers[ 0x91 ] = &GBCpu::OpSUB_r_r<A,C>;
    m_OpcodeHandlers[ 0x92 ] = &GBCpu::OpSUB_r_r<A,D>;
    m_OpcodeHandlers[ 0x93 ] = &GBCpu::OpSUB_r_r<A,E>;
    m_OpcodeHandlers[ 0x94 ] = &GBCpu::OpSUB_r_r<A,H>;
    m_OpcodeHandlers[ 0x95 ] = &GBCpu::OpSUB_r_r<A,L>;
    m_OpcodeHandlers[ 0x96 ] = &GBCpu::OpSUB_r_rr<A,HL>;
    m_OpcodeHandlers[ 0x97 ] = &GBCpu::OpSUB_r_r<A,A>;
    m_OpcodeHandlers[ 0x98 ] = &GBCpu::OpSBC_r_r<A,B>;
    m_OpcodeHandlers[ 0x99 ] = &GBCpu::OpSBC_r_r<A,C>;
    m_OpcodeHandlers[ 0x9a ] = &GBCpu::OpSBC_r_r<A,D>;
    m_OpcodeHandlers[ 0x9b ] = &GBCpu::OpSBC_r_r<A,E>;
    m_OpcodeHandlers[ 0x9c ] = &GBCpu::OpSBC_r_r<A,H>;
    m_OpcodeHandlers[ 0x9d ] = &GBCpu::OpSBC_r_r<A,L>;
    m_OpcodeHandlers[ 0x9e ] = &GBCpu::OpSBC_r_rr<A,HL>;
    m_OpcodeHandlers[ 0x9f ] = &GBCpu::OpSBC_r_r<A,A>;
    m_OpcodeHandlers[ 0xa0 ] = &GBCpu::OpAND_r_r<A,B>;
    m_OpcodeHandlers[ 0xa1 ] = &GBCpu::OpAND_r_r<A,C>;
    m_OpcodeHandlers[ 0xa2 ] = &GBCpu::OpAND_r_r<A,D>;
    m_OpcodeHandlers[ 0xa3 ] = &GBCpu::OpAND_r_r<A,E>;
    m_OpcodeHandlers[ 0xa4 ] = &GBCpu::OpAND_r_r<A,H>;
    m_OpcodeHandlers[ 0xa5 ] = &GBCpu::OpAND_r_r<A,L>;
    m_OpcodeHandlers[ 0xa6 ] = &GBCpu::OpAND_r_rr<A,HL>;
    m_OpcodeHandlers[ 0xa7 ] = &GBCpu::OpAND_r_r<A,A>;
    m_OpcodeHandlers[ 0xa8 ] = &GBCpu::OpXOR_r_r<A,B>;
    m_OpcodeHandlers[ 0xa9 ] = &GBCpu::OpXOR_r_r<A,C>;
    m_OpcodeHandlers[ 0xaa ] = &GBCpu::OpXOR_r_r<A,D>;
    m_OpcodeHandlers[ 0xab ] = &GBCpu::OpXOR_r_r<A,E>;
    m_OpcodeHandlers[ 0xac ] = &GBCpu::OpXOR_r_r<A,H>;
    m_OpcodeHandlers[ 0xad ] = &GBCpu::OpXOR_r_r<A,L>;
    m_OpcodeHandlers[ 0xae ] = &GBCpu::OpXOR_r_rr<A,HL>;
    m_OpcodeHandlers[ 0xaf ] = &GBCpu::OpXOR_r_r<A,A>;
    m_OpcodeHandlers[ 0xb0 ] = &GBCpu::OpOR_r_r<A,B>;
    m_OpcodeHandlers[ 0xb1 ] = &GBCpu::OpOR_r_r<A,C>;
    m_OpcodeHandlers[ 0xb2 ] = &GBCpu::OpOR_r_r<A,D>;
    m_OpcodeHandlers[ 0xb3 ] = &GBCpu::OpOR_r_r<A,E>;
    m_OpcodeHandlers[ 0xb4 ] = &GBCpu::OpOR_r_r<A,H>;
    m_OpcodeHandlers[ 0xb5 ] = &GBCpu::OpOR_r_r<A,L>;
    m_OpcodeHandlers[ 0xb6 ] = &GBCpu::OpOR_r_rr<A,HL>;
    m_OpcodeHandlers[ 0xb7 ] = &GBCpu::OpOR_r_r<A,A>;
    m_OpcodeHandlers[ 0xb8 ] = &GBCpu::OpCMP_r_r<A,B>;
    m_OpcodeHandlers[ 0xb9 ] = &GBCpu::OpCMP_r_r<A,C>;
    m_OpcodeHandlers[ 0xba ] = &GBCpu::OpCMP_r_r<A,D>;
    m_OpcodeHandlers[ 0xbb ] = &GBCpu::OpCMP_r_r<A,E>;
    m_OpcodeHandlers[ 0xbc ] = &GBCpu::OpCMP_r_r<A,H>;
    m_OpcodeHandlers[ 0xbd ] = &GBCpu::OpCMP_r_r<A,L>;
    m_OpcodeHandlers[ 0xbe ] = &GBCpu::OpCMP_r_rr<A,HL>;
    m_OpcodeHandlers[ 0xbf ] = &GBCpu::OpCMP_r_r<A,A>;
    m_OpcodeHandlers[ 0xc0 ] = &GBCpu::OpRET_NZ_nn;
    m_OpcodeHandlers[ 0xc1 ] = &GBCpu::OpPOP_rr_nn<BC>;
    m_OpcodeHandlers[ 0xc2 ] = &GBCpu::OpJP_NZ_nn;
    m_OpcodeHandlers[ 0xc3 ] = &GBCpu::OpJP_nn;
    m_OpcodeHandlers[ 0xc4 ] = &GBCpu::OpCALL_NZ_nn;
    m_OpcodeHandlers[ 0xc5 ] = &GBCpu::OpPUSH_rr_nn<BC>;
    m_OpcodeHandlers[ 0xc6 ] = &GBCpu::OpADD_r_n<A>;
    m_OpcodeHandlers[ 0xc7 ] = &GBCpu::OpRST_nn<0x00>;
    m_OpcodeHandlers[ 0xc8 ] = &GBCpu::OpRET_Z_nn;
    m_OpcodeHandlers[ 0xc9 ] = &GBCpu::OpRET_nn;
    m_OpcodeHandlers[ 0xca ] = &GBCpu::OpJP_Z_nn;
    m_OpcodeHandlers[ 0xcb ] = &GBCpu::OpExecuteExtOp;
    m_OpcodeHandlers[ 0xcc ] = &GBCpu::OpCALL_Z_nn;
    m_OpcodeHandlers[ 0xcd ] = &GBCpu::OpCALL_nn;
    m_OpcodeHandlers[ 0xce ] = &GBCpu::OpADC_r_n<A>;
    m_OpcodeHandlers[ 0xcf ] = &GBCpu::OpRST_nn<0x08>;
    m_OpcodeHandlers[ 0xd0 ] = &GBCpu::OpRET_NC_nn;
    m_OpcodeHandlers[ 0xd1 ] = &GBCpu::OpPOP_rr_nn<DE>;
    m_OpcodeHandlers[ 0xd2 ] = &GBCpu::OpJP_NC_nn;
    m_OpcodeHandlers[ 0xd3 ] = &GBCpu::OpInvalid<0xD3>;
    m_OpcodeHandlers[ 0xd4 ] = &GBCpu::OpCALL_NC_nn;
    m_OpcodeHandlers[ 0xd5 ] = &GBCpu::OpPUSH_rr_nn<DE>;
    m_OpcodeHandlers[ 0xd6 ] = &GBCpu::OpSUB_r_n<A>;
    m_OpcodeHandlers[ 0xd7 ] = &GBCpu::OpRST_nn<0x10>;
    m_OpcodeHandlers[ 0xd8 ] = &GBCpu::OpRET_C_nn;
    m_OpcodeHandlers[ 0xd9 ] = &GBCpu::OpRETI_nn;
    m_OpcodeHandlers[ 0xda ] = &GBCpu::OpJP_C_nn;
    m_OpcodeHandlers[ 0xdb ] = &GBCpu::OpInvalid<0xDB>;
    m_OpcodeHandlers[ 0xdc ] = &GBCpu::OpCALL_C_nn;
    m_OpcodeHandlers[ 0xdd ] = &GBCpu::OpInvalid<0xDD>;
    m_OpcodeHandlers[ 0xde ] = &GBCpu::OpSBC_r_n<A>;
    m_OpcodeHandlers[ 0xdf ] = &GBCpu::OpRST_nn<0x18>;
    m_OpcodeHandlers[ 0xe0 ] = &GBCpu::OpLDH_n_A;
    m_OpcodeHandlers[ 0xe1 ] = &GBCpu::OpPOP_rr_nn<HL>;
    m_OpcodeHandlers[ 0xe2 ] = &GBCpu::OpLDH_C_A;
    m_OpcodeHandlers[ 0xe3 ] = &GBCpu::OpInvalid<0xE3>;
    m_OpcodeHandlers[ 0xe4 ] = &GBCpu::OpInvalid<0xE4>;
    m_OpcodeHandlers[ 0xe5 ] = &GBCpu::OpPUSH_rr_nn<HL>;
    m_OpcodeHandlers[ 0xe6 ] = &GBCpu::OpAND_r_n<A>;
    m_OpcodeHandlers[ 0xe7 ] = &GBCpu::OpRST_nn<0x20>;
    m_OpcodeHandlers[ 0xe8 ] = &GBCpu::OpADD_SP_n;
    m_OpcodeHandlers[ 0xe9 ] = &GBCpu::OpJP_HL;
    m_OpcodeHandlers[ 0xea ] = &GBCpu::OpLD_nn_r<A>;
    m_OpcodeHandlers[ 0xeb ] = &GBCpu::OpInvalid<0xEB>;
    m_OpcodeHandlers[ 0xec ] = &GBCpu::OpInvalid<0xEC>;
    m_OpcodeHandlers[ 0xed ] = &GBCpu::OpInvalid<0xED>;
    m_OpcodeHandlers[ 0xee ] = &GBCpu::OpXOR_r_n<A>;
    m_OpcodeHandlers[ 0xef ] = &GBCpu::OpRST_nn<0x28>;
    m_OpcodeHandlers[ 0xf0 ] = &GBCpu::OpLDH_A_n;
    m_OpcodeHandlers[ 0xf1 ] = &GBCpu::OpPOP_rr_nn<AF>;
    m_OpcodeHandlers[ 0xf2 ] = &GBCpu::OpLDH_A_C;
    m_OpcodeHandlers[ 0xf3 ] = &GBCpu::OpDI;
    m_OpcodeHandlers[ 0xf4 ] = &GBCpu::OpInvalid<0xF4>;
    m_OpcodeHandlers[ 0xf5 ] = &GBCpu::OpPUSH_rr_nn<AF>;
    m_OpcodeHandlers[ 0xf6 ] = &GBCpu::OpOR_r_n<A>;
    m_OpcodeHandlers[ 0xf7 ] = &GBCpu::OpRST_nn<0x30>;
    m_OpcodeHandlers[ 0xf8 ] = &GBCpu::OpLD_HL_SP_n;
    m_OpcodeHandlers[ 0xf9 ] = &GBCpu::OpLD_SP_HL;
    m_OpcodeHandlers[ 0xfa ] = &GBCpu::OpLD_r_nn<A>;
    m_OpcodeHandlers[ 0xfb ] = &GBCpu::OpEI;
    m_OpcodeHandlers[ 0xfc ] = &GBCpu::OpInvalid<0xFC>;
    m_OpcodeHandlers[ 0xfd ] = &GBCpu::OpInvalid<0xFD>;
    m_OpcodeHandlers[ 0xfe ] = &GBCpu::OpCMP_r_n<A>;
    m_OpcodeHandlers[ 0xff ] = &GBCpu::OpRST_nn<0x38>;

    // Initialize extended opcode handlers
    m_OpcodeExtHandlers[ 0x00 ] = &GBCpu::OpRLC_r<B>;
    m_OpcodeExtHandlers[ 0x01 ] = &GBCpu::OpRLC_r<C>;
    m_OpcodeExtHandlers[ 0x02 ] = &GBCpu::OpRLC_r<D>;
    m_OpcodeExtHandlers[ 0x03 ] = &GBCpu::OpRLC_r<E>;
    m_OpcodeExtHandlers[ 0x04 ] = &GBCpu::OpRLC_r<H>;
    m_OpcodeExtHandlers[ 0x05 ] = &GBCpu::OpRLC_r<L>;
    m_OpcodeExtHandlers[ 0x06 ] = &GBCpu::OpRLC_HL;
    m_OpcodeExtHandlers[ 0x07 ] = &GBCpu::OpRLC_r<A>;
    m_OpcodeExtHandlers[ 0x08 ] = &GBCpu::OpRRC_r<B>;
    m_OpcodeExtHandlers[ 0x09 ] = &GBCpu::OpRRC_r<C>;
    m_OpcodeExtHandlers[ 0x0a ] = &GBCpu::OpRRC_r<D>;
    m_OpcodeExtHandlers[ 0x0b ] = &GBCpu::OpRRC_r<E>;
    m_OpcodeExtHandlers[ 0x0c ] = &GBCpu::OpRRC_r<H>;
    m_OpcodeExtHandlers[ 0x0d ] = &GBCpu::OpRRC_r<L>;
    m_OpcodeExtHandlers[ 0x0e ] = &GBCpu::OpRRC_HL;
    m_OpcodeExtHandlers[ 0x0f ] = &GBCpu::OpRRC_r<A>;
    m_OpcodeExtHandlers[ 0x10 ] = &GBCpu::OpRL_r<B>;
    m_OpcodeExtHandlers[ 0x11 ] = &GBCpu::OpRL_r<C>;
    m_OpcodeExtHandlers[ 0x12 ] = &GBCpu::OpRL_r<D>;
    m_OpcodeExtHandlers[ 0x13 ] = &GBCpu::OpRL_r<E>;
    m_OpcodeExtHandlers[ 0x14 ] = &GBCpu::OpRL_r<H>;
    m_OpcodeExtHandlers[ 0x15 ] = &GBCpu::OpRL_r<L>;
    m_OpcodeExtHandlers[ 0x16 ] = &GBCpu::OpRL_HL;
    m_OpcodeExtHandlers[ 0x17 ] = &GBCpu::OpRL_r<A>;
    m_OpcodeExtHandlers[ 0x18 ] = &GBCpu::OpRR_r<B>;
    m_OpcodeExtHandlers[ 0x19 ] = &GBCpu::OpRR_r<C>;
    m_OpcodeExtHandlers[ 0x1a ] = &GBCpu::OpRR_r<D>;
    m_OpcodeExtHandlers[ 0x1b ] = &GBCpu::OpRR_r<E>;
    m_OpcodeExtHandlers[ 0x1c ] = &GBCpu::OpRR_r<H>;
    m_OpcodeExtHandlers[ 0x1d ] = &GBCpu::OpRR_r<L>;
    m_OpcodeExtHandlers[ 0x1e ] = &GBCpu::OpRR_HL;
    m_OpcodeExtHandlers[ 0x1f ] = &GBCpu::OpRR_r<A>;
    m_OpcodeExtHandlers[ 0x20 ] = &GBCpu::OpSLA_r<B>;
    m_OpcodeExtHandlers[ 0x21 ] = &GBCpu::OpSLA_r<C>;
    m_OpcodeExtHandlers[ 0x22 ] = &GBCpu::OpSLA_r<D>;
    m_OpcodeExtHandlers[ 0x23 ] = &GBCpu::OpSLA_r<E>;
    m_OpcodeExtHandlers[ 0x24 ] = &GBCpu::OpSLA_r<H>;
    m_OpcodeExtHandlers[ 0x25 ] = &GBCpu::OpSLA_r<L>;
    m_OpcodeExtHandlers[ 0x26 ] = &GBCpu::OpSLA_HL;
    m_OpcodeExtHandlers[ 0x27 ] = &GBCpu::OpSLA_r<A>;
    m_OpcodeExtHandlers[ 0x28 ] = &GBCpu::OpSRA_r<B>;
    m_OpcodeExtHandlers[ 0x29 ] = &GBCpu::OpSRA_r<C>;
    m_OpcodeExtHandlers[ 0x2a ] = &GBCpu::OpSRA_r<D>;
    m_OpcodeExtHandlers[ 0x2b ] = &GBCpu::OpSRA_r<E>;
    m_OpcodeExtHandlers[ 0x2c ] = &GBCpu::OpSRA_r<H>;
    m_OpcodeExtHandlers[ 0x2d ] = &GBCpu::OpSRA_r<L>;
    m_OpcodeExtHandlers[ 0x2e ] = &GBCpu::OpSRA_HL;
    m_OpcodeExtHandlers[ 0x2f ] = &GBCpu::OpSRA_r<A>;
    m_OpcodeExtHandlers[ 0x30 ] = &GBCpu::OpSWAP_r<B>;
    m_OpcodeExtHandlers[ 0x31 ] = &GBCpu::OpSWAP_r<C>;
    m_OpcodeExtHandlers[ 0x32 ] = &GBCpu::OpSWAP_r<D>;
    m_OpcodeExtHandlers[ 0x33 ] = &GBCpu::OpSWAP_r<E>;
    m_OpcodeExtHandlers[ 0x34 ] = &GBCpu::OpSWAP_r<H>;
    m_OpcodeExtHandlers[ 0x35 ] = &GBCpu::OpSWAP_r<L>;
    m_OpcodeExtHandlers[ 0x36 ] = &GBCpu::OpSWAP_HL;
    m_OpcodeExtHandlers[ 0x37 ] = &GBCpu::OpSWAP_r<A>;
    m_OpcodeExtHandlers[ 0x38 ] = &GBCpu::OpSRL_r<B>;
    m_OpcodeExtHandlers[ 0x39 ] = &GBCpu::OpSRL_r<C>;
    m_OpcodeExtHandlers[ 0x3a ] = &GBCpu::OpSRL_r<D>;
    m_OpcodeExtHandlers[ 0x3b ] = &GBCpu::OpSRL_r<E>;
    m_OpcodeExtHandlers[ 0x3c ] = &GBCpu::OpSRL_r<H>;
    m_OpcodeExtHandlers[ 0x3d ] = &GBCpu::OpSRL_r<L>;
    m_OpcodeExtHandlers[ 0x3e ] = &GBCpu::OpSRL_HL;
    m_OpcodeExtHandlers[ 0x3f ] = &GBCpu::OpSRL_r<A>;
    m_OpcodeExtHandlers[ 0x40 ] = &GBCpu::OpBIT_r<0,B>;
    m_OpcodeExtHandlers[ 0x41 ] = &GBCpu::OpBIT_r<0,C>;
    m_OpcodeExtHandlers[ 0x42 ] = &GBCpu::OpBIT_r<0,D>;
    m_OpcodeExtHandlers[ 0x43 ] = &GBCpu::OpBIT_r<0,E>;
    m_OpcodeExtHandlers[ 0x44 ] = &GBCpu::OpBIT_r<0,H>;
    m_OpcodeExtHandlers[ 0x45 ] = &GBCpu::OpBIT_r<0,L>;
    m_OpcodeExtHandlers[ 0x46 ] = &GBCpu::OpBIT_HL<0>;
    m_OpcodeExtHandlers[ 0x47 ] = &GBCpu::OpBIT_r<0,A>;
    m_OpcodeExtHandlers[ 0x48 ] = &GBCpu::OpBIT_r<1,B>;
    m_OpcodeExtHandlers[ 0x49 ] = &GBCpu::OpBIT_r<1,C>;
    m_OpcodeExtHandlers[ 0x4a ] = &GBCpu::OpBIT_r<1,D>;
    m_OpcodeExtHandlers[ 0x4b ] = &GBCpu::OpBIT_r<1,E>;
    m_OpcodeExtHandlers[ 0x4c ] = &GBCpu::OpBIT_r<1,H>;
    m_OpcodeExtHandlers[ 0x4d ] = &GBCpu::OpBIT_r<1,L>;
    m_OpcodeExtHandlers[ 0x4e ] = &GBCpu::OpBIT_HL<1>;
    m_OpcodeExtHandlers[ 0x4f ] = &GBCpu::OpBIT_r<1,A>;
    m_OpcodeExtHandlers[ 0x50 ] = &GBCpu::OpBIT_r<2,B>;
    m_OpcodeExtHandlers[ 0x51 ] = &GBCpu::OpBIT_r<2,C>;
    m_OpcodeExtHandlers[ 0x52 ] = &GBCpu::OpBIT_r<2,D>;
    m_OpcodeExtHandlers[ 0x53 ] = &GBCpu::OpBIT_r<2,E>;
    m_OpcodeExtHandlers[ 0x54 ] = &GBCpu::OpBIT_r<2,H>;
    m_OpcodeExtHandlers[ 0x55 ] = &GBCpu::OpBIT_r<2,L>;
    m_OpcodeExtHandlers[ 0x56 ] = &GBCpu::OpBIT_HL<2>;
    m_OpcodeExtHandlers[ 0x57 ] = &GBCpu::OpBIT_r<2,A>;
    m_OpcodeExtHandlers[ 0x58 ] = &GBCpu::OpBIT_r<3,B>;
    m_OpcodeExtHandlers[ 0x59 ] = &GBCpu::OpBIT_r<3,C>;
    m_OpcodeExtHandlers[ 0x5a ] = &GBCpu::OpBIT_r<3,D>;
    m_OpcodeExtHandlers[ 0x5b ] = &GBCpu::OpBIT_r<3,E>;
    m_OpcodeExtHandlers[ 0x5c ] = &GBCpu::OpBIT_r<3,H>;
    m_OpcodeExtHandlers[ 0x5d ] = &GBCpu::OpBIT_r<3,L>;
    m_OpcodeExtHandlers[ 0x5e ] = &GBCpu::OpBIT_HL<3>;
    m_OpcodeExtHandlers[ 0x5f ] = &GBCpu::OpBIT_r<3,A>;
    m_OpcodeExtHandlers[ 0x60 ] = &GBCpu::OpBIT_r<4,B>;
    m_OpcodeExtHandlers[ 0x61 ] = &GBCpu::OpBIT_r<4,C>;
    m_OpcodeExtHandlers[ 0x62 ] = &GBCpu::OpBIT_r<4,D>;
    m_OpcodeExtHandlers[ 0x63 ] = &GBCpu::OpBIT_r<4,E>;
    m_OpcodeExtHandlers[ 0x64 ] = &GBCpu::OpBIT_r<4,H>;
    m_OpcodeExtHandlers[ 0x65 ] = &GBCpu::OpBIT_r<4,L>;
    m_OpcodeExtHandlers[ 0x66 ] = &GBCpu::OpBIT_HL<4>;
    m_OpcodeExtHandlers[ 0x67 ] = &GBCpu::OpBIT_r<4,A>;
    m_OpcodeExtHandlers[ 0x68 ] = &GBCpu::OpBIT_r<5,B>;
    m_OpcodeExtHandlers[ 0x69 ] = &GBCpu::OpBIT_r<5,C>;
    m_OpcodeExtHandlers[ 0x6a ] = &GBCpu::OpBIT_r<5,D>;
    m_OpcodeExtHandlers[ 0x6b ] = &GBCpu::OpBIT_r<5,E>;
    m_OpcodeExtHandlers[ 0x6c ] = &GBCpu::OpBIT_r<5,H>;
    m_OpcodeExtHandlers[ 0x6d ] = &GBCpu::OpBIT_r<5,L>;
    m_OpcodeExtHandlers[ 0x6e ] = &GBCpu::OpBIT_HL<5>;
    m_OpcodeExtHandlers[ 0x6f ] = &GBCpu::OpBIT_r<5,A>;
    m_OpcodeExtHandlers[ 0x70 ] = &GBCpu::OpBIT_r<6,B>;
    m_OpcodeExtHandlers[ 0x71 ] = &GBCpu::OpBIT_r<6,C>;
    m_OpcodeExtHandlers[ 0x72 ] = &GBCpu::OpBIT_r<6,D>;
    m_OpcodeExtHandlers[ 0x73 ] = &GBCpu::OpBIT_r<6,E>;
    m_OpcodeExtHandlers[ 0x74 ] = &GBCpu::OpBIT_r<6,H>;
    m_OpcodeExtHandlers[ 0x75 ] = &GBCpu::OpBIT_r<6,L>;
    m_OpcodeExtHandlers[ 0x76 ] = &GBCpu::OpBIT_HL<6>;
    m_OpcodeExtHandlers[ 0x77 ] = &GBCpu::OpBIT_r<6,A>;
    m_OpcodeExtHandlers[ 0x78 ] = &GBCpu::OpBIT_r<7,B>;
    m_OpcodeExtHandlers[ 0x79 ] = &GBCpu::OpBIT_r<7,C>;
    m_OpcodeExtHandlers[ 0x7a ] = &GBCpu::OpBIT_r<7,D>;
    m_OpcodeExtHandlers[ 0x7b ] = &GBCpu::OpBIT_r<7,E>;
    m_OpcodeExtHandlers[ 0x7c ] = &GBCpu::OpBIT_r<7,H>;
    m_OpcodeExtHandlers[ 0x7d ] = &GBCpu::OpBIT_r<7,L>;
    m_OpcodeExtHandlers[ 0x7e ] = &GBCpu::OpBIT_HL<7>;
    m_OpcodeExtHandlers[ 0x7f ] = &GBCpu::OpBIT_r<7,A>;
    m_OpcodeExtHandlers[ 0x80 ] = &GBCpu::OpRES_r<0,B>;
    m_OpcodeExtHandlers[ 0x81 ] = &GBCpu::OpRES_r<0,C>;
    m_OpcodeExtHandlers[ 0x82 ] = &GBCpu::OpRES_r<0,D>;
    m_OpcodeExtHandlers[ 0x83 ] = &GBCpu::OpRES_r<0,E>;
    m_OpcodeExtHandlers[ 0x84 ] = &GBCpu::OpRES_r<0,H>;
    m_OpcodeExtHandlers[ 0x85 ] = &GBCpu::OpRES_r<0,L>;
    m_OpcodeExtHandlers[ 0x86 ] = &GBCpu::OpRES_HL<0>;
    m_OpcodeExtHandlers[ 0x87 ] = &GBCpu::OpRES_r<0,A>;
    m_OpcodeExtHandlers[ 0x88 ] = &GBCpu::OpRES_r<1,B>;
    m_OpcodeExtHandlers[ 0x89 ] = &GBCpu::OpRES_r<1,C>;
    m_OpcodeExtHandlers[ 0x8a ] = &GBCpu::OpRES_r<1,D>;
    m_OpcodeExtHandlers[ 0x8b ] = &GBCpu::OpRES_r<1,E>;
    m_OpcodeExtHandlers[ 0x8c ] = &GBCpu::OpRES_r<1,H>;
    m_OpcodeExtHandlers[ 0x8d ] = &GBCpu::OpRES_r<1,L>;
    m_OpcodeExtHandlers[ 0x8e ] = &GBCpu::OpRES_HL<1>;
    m_OpcodeExtHandlers[ 0x8f ] = &GBCpu::OpRES_r<1,A>;
    m_OpcodeExtHandlers[ 0x90 ] = &GBCpu::OpRES_r<2,B>;
    m_OpcodeExtHandlers[ 0x91 ] = &GBCpu::OpRES_r<2,C>;
    m_OpcodeExtHandlers[ 0x92 ] = &GBCpu::OpRES_r<2,D>;
    m_OpcodeExtHandlers[ 0x93 ] = &GBCpu::OpRES_r<2,E>;
    m_OpcodeExtHandlers[ 0x94 ] = &GBCpu::OpRES_r<2,H>;
    m_OpcodeExtHandlers[ 0x95 ] = &GBCpu::OpRES_r<2,L>;
    m_OpcodeExtHandlers[ 0x96 ] = &GBCpu::OpRES_HL<2>;
    m_OpcodeExtHandlers[ 0x97 ] = &GBCpu::OpRES_r<2,A>;
    m_OpcodeExtHandlers[ 0x98 ] = &GBCpu::OpRES_r<3,B>;
    m_OpcodeExtHandlers[ 0x99 ] = &GBCpu::OpRES_r<3,C>;
    m_OpcodeExtHandlers[ 0x9a ] = &GBCpu::OpRES_r<3,D>;
    m_OpcodeExtHandlers[ 0x9b ] = &GBCpu::OpRES_r<3,E>;
    m_OpcodeExtHandlers[ 0x9c ] = &GBCpu::OpRES_r<3,H>;
    m_OpcodeExtHandlers[ 0x9d ] = &GBCpu::OpRES_r<3,L>;
    m_OpcodeExtHandlers[ 0x9e ] = &GBCpu::OpRES_HL<3>;
    m_OpcodeExtHandlers[ 0x9f ] = &GBCpu::OpRES_r<3,A>;
    m_OpcodeExtHandlers[ 0xa0 ] = &GBCpu::OpRES_r<4,B>;
    m_OpcodeExtHandlers[ 0xa1 ] = &GBCpu::OpRES_r<4,C>;
    m_OpcodeExtHandlers[ 0xa2 ] = &GBCpu::OpRES_r<4,D>;
    m_OpcodeExtHandlers[ 0xa3 ] = &GBCpu::OpRES_r<4,E>;
    m_OpcodeExtHandlers[ 0xa4 ] = &GBCpu::OpRES_r<4,H>;
    m_OpcodeExtHandlers[ 0xa5 ] = &GBCpu::OpRES_r<4,L>;
    m_OpcodeExtHandlers[ 0xa6 ] = &GBCpu::OpRES_HL<4>;
    m_OpcodeExtHandlers[ 0xa7 ] = &GBCpu::OpRES_r<4,A>;
    m_OpcodeExtHandlers[ 0xa8 ] = &GBCpu::OpRES_r<5,B>;
    m_OpcodeExtHandlers[ 0xa9 ] = &GBCpu::OpRES_r<5,C>;
    m_OpcodeExtHandlers[ 0xaa ] = &GBCpu::OpRES_r<5,D>;
    m_OpcodeExtHandlers[ 0xab ] = &GBCpu::OpRES_r<5,E>;
    m_OpcodeExtHandlers[ 0xac ] = &GBCpu::OpRES_r<5,H>;
    m_OpcodeExtHandlers[ 0xad ] = &GBCpu::OpRES_r<5,L>;
    m_OpcodeExtHandlers[ 0xae ] = &GBCpu::OpRES_HL<5>;
    m_OpcodeExtHandlers[ 0xaf ] = &GBCpu::OpRES_r<5,A>;
    m_OpcodeExtHandlers[ 0xb0 ] = &GBCpu::OpRES_r<6,B>;
    m_OpcodeExtHandlers[ 0xb1 ] = &GBCpu::OpRES_r<6,C>;
    m_OpcodeExtHandlers[ 0xb2 ] = &GBCpu::OpRES_r<6,D>;
    m_OpcodeExtHandlers[ 0xb3 ] = &GBCpu::OpRES_r<6,E>;
    m_OpcodeExtHandlers[ 0xb4 ] = &GBCpu::OpRES_r<6,H>;
    m_OpcodeExtHandlers[ 0xb5 ] = &GBCpu::OpRES_r<6,L>;
    m_OpcodeExtHandlers[ 0xb6 ] = &GBCpu::OpRES_HL<6>;
    m_OpcodeExtHandlers[ 0xb7 ] = &GBCpu::OpRES_r<6,A>;
    m_OpcodeExtHandlers[ 0xb8 ] = &GBCpu::OpRES_r<7,B>;
    m_OpcodeExtHandlers[ 0xb9 ] = &GBCpu::OpRES_r<7,C>;
    m_OpcodeExtHandlers[ 0xba ] = &GBCpu::OpRES_r<7,D>;
    m_OpcodeExtHandlers[ 0xbb ] = &GBCpu::OpRES_r<7,E>;
    m_OpcodeExtHandlers[ 0xbc ] = &GBCpu::OpRES_r<7,H>;
    m_OpcodeExtHandlers[ 0xbd ] = &GBCpu::OpRES_r<7,L>;
    m_OpcodeExtHandlers[ 0xbe ] = &GBCpu::OpRES_HL<7>;
    m_OpcodeExtHandlers[ 0xbf ] = &GBCpu::OpRES_r<7,A>;
    m_OpcodeExtHandlers[ 0xc0 ] = &GBCpu::OpSET_r<0,B>;
    m_OpcodeExtHandlers[ 0xc1 ] = &GBCpu::OpSET_r<0,C>;
    m_OpcodeExtHandlers[ 0xc2 ] = &GBCpu::OpSET_r<0,D>;
    m_OpcodeExtHandlers[ 0xc3 ] = &GBCpu::OpSET_r<0,E>;
    m_OpcodeExtHandlers[ 0xc4 ] = &GBCpu::OpSET_r<0,H>;
    m_OpcodeExtHandlers[ 0xc5 ] = &GBCpu::OpSET_r<0,L>;
    m_OpcodeExtHandlers[ 0xc6 ] = &GBCpu::OpSET_HL<0>;
    m_OpcodeExtHandlers[ 0xc7 ] = &GBCpu::OpSET_r<0,A>;
    m_OpcodeExtHandlers[ 0xc8 ] = &GBCpu::OpSET_r<1,B>;
    m_OpcodeExtHandlers[ 0xc9 ] = &GBCpu::OpSET_r<1,C>;
    m_OpcodeExtHandlers[ 0xca ] = &GBCpu::OpSET_r<1,D>;
    m_OpcodeExtHandlers[ 0xcb ] = &GBCpu::OpSET_r<1,E>;
    m_OpcodeExtHandlers[ 0xcc ] = &GBCpu::OpSET_r<1,H>;
    m_OpcodeExtHandlers[ 0xcd ] = &GBCpu::OpSET_r<1,L>;
    m_OpcodeExtHandlers[ 0xce ] = &GBCpu::OpSET_HL<1>;
    m_OpcodeExtHandlers[ 0xcf ] = &GBCpu::OpSET_r<1,A>;
    m_OpcodeExtHandlers[ 0xd0 ] = &GBCpu::OpSET_r<2,B>;
    m_OpcodeExtHandlers[ 0xd1 ] = &GBCpu::OpSET_r<2,C>;
    m_OpcodeExtHandlers[ 0xd2 ] = &GBCpu::OpSET_r<2,D>;
    m_OpcodeExtHandlers[ 0xd3 ] = &GBCpu::OpSET_r<2,E>;
    m_OpcodeExtHandlers[ 0xd4 ] = &GBCpu::OpSET_r<2,H>;
    m_OpcodeExtHandlers[ 0xd5 ] = &GBCpu::OpSET_r<2,L>;
    m_OpcodeExtHandlers[ 0xd6 ] = &GBCpu::OpSET_HL<2>;
    m_OpcodeExtHandlers[ 0xd7 ] = &GBCpu::OpSET_r<2,A>;
    m_OpcodeExtHandlers[ 0xd8 ] = &GBCpu::OpSET_r<3,B>;
    m_OpcodeExtHandlers[ 0xd9 ] = &GBCpu::OpSET_r<3,C>;
    m_OpcodeExtHandlers[ 0xda ] = &GBCpu::OpSET_r<3,D>;
    m_OpcodeExtHandlers[ 0xdb ] = &GBCpu::OpSET_r<3,E>;
    m_OpcodeExtHandlers[ 0xdc ] = &GBCpu::OpSET_r<3,H>;
    m_OpcodeExtHandlers[ 0xdd ] = &GBCpu::OpSET_r<3,L>;
    m_OpcodeExtHandlers[ 0xde ] = &GBCpu::OpSET_HL<3>;
    m_OpcodeExtHandlers[ 0xdf ] = &GBCpu::OpSET_r<3,A>;
    m_OpcodeExtHandlers[ 0xe0 ] = &GBCpu::OpSET_r<4,B>;
    m_OpcodeExtHandlers[ 0xe1 ] = &GBCpu::OpSET_r<4,C>;
    m_OpcodeExtHandlers[ 0xe2 ] = &GBCpu::OpSET_r<4,D>;
    m_OpcodeExtHandlers[ 0xe3 ] = &GBCpu::OpSET_r<4,E>;
    m_OpcodeExtHandlers[ 0xe4 ] = &GBCpu::OpSET_r<4,H>;
    m_OpcodeExtHandlers[ 0xe5 ] = &GBCpu::OpSET_r<4,L>;
    m_OpcodeExtHandlers[ 0xe6 ] = &GBCpu::OpSET_HL<4>;
    m_OpcodeExtHandlers[ 0xe7 ] = &GBCpu::OpSET_r<4,A>;
    m_OpcodeExtHandlers[ 0xe8 ] = &GBCpu::OpSET_r<5,B>;
    m_OpcodeExtHandlers[ 0xe9 ] = &GBCpu::OpSET_r<5,C>;
    m_OpcodeExtHandlers[ 0xea ] = &GBCpu::OpSET_r<5,D>;
    m_OpcodeExtHandlers[ 0xeb ] = &GBCpu::OpSET_r<5,E>;
    m_OpcodeExtHandlers[ 0xec ] = &GBCpu::OpSET_r<5,H>;
    m_OpcodeExtHandlers[ 0xed ] = &GBCpu::OpSET_r<5,L>;
    m_OpcodeExtHandlers[ 0xee ] = &GBCpu::OpSET_HL<5>;
    m_OpcodeExtHandlers[ 0xef ] = &GBCpu::OpSET_r<5,A>;
    m_OpcodeExtHandlers[ 0xf0 ] = &GBCpu::OpSET_r<6,B>;
    m_OpcodeExtHandlers[ 0xf1 ] = &GBCpu::OpSET_r<6,C>;
    m_OpcodeExtHandlers[ 0xf2 ] = &GBCpu::OpSET_r<6,D>;
    m_OpcodeExtHandlers[ 0xf3 ] = &GBCpu::OpSET_r<6,E>;
    m_OpcodeExtHandlers[ 0xf4 ] = &GBCpu::OpSET_r<6,H>;
    m_OpcodeExtHandlers[ 0xf5 ] = &GBCpu::OpSET_r<6,L>;
    m_OpcodeExtHandlers[ 0xf6 ] = &GBCpu::OpSET_HL<6>;
    m_OpcodeExtHandlers[ 0xf7 ] = &GBCpu::OpSET_r<6,A>;
    m_OpcodeExtHandlers[ 0xf8 ] = &GBCpu::OpSET_r<7,B>;
    m_OpcodeExtHandlers[ 0xf9 ] = &GBCpu::OpSET_r<7,C>;
    m_OpcodeExtHandlers[ 0xfa ] = &GBCpu::OpSET_r<7,D>;
    m_OpcodeExtHandlers[ 0xfb ] = &GBCpu::OpSET_r<7,E>;
    m_OpcodeExtHandlers[ 0xfc ] = &GBCpu::OpSET_r<7,H>;
    m_OpcodeExtHandlers[ 0xfd ] = &GBCpu::OpSET_r<7,L>;
    m_OpcodeExtHandlers[ 0xfe ] = &GBCpu::OpSET_HL<7>;
    m_OpcodeExtHandlers[ 0xff ] = &GBCpu::OpSET_r<7,A>;

    m_bInitialized = true;
}

//----------------------------------------------------------------------------------------------------
void GBCpu::Terminate()
{
    m_bInitialized = false;
}

//----------------------------------------------------------------------------------------------------
void GBCpu::Reset()
{
    m_PC                = 0;
    m_SP                = 0xFFFF;

    m_Registers[ A ]    = 0;
    m_Registers[ B ]    = 0;
    m_Registers[ C ]    = 0;
    m_Registers[ D ]    = 0;
    m_Registers[ E ]    = 0;
    m_Registers[ F ]    = 0;
    m_Registers[ H ]    = 0;
    m_Registers[ L ]    = 0;

    m_bIME              = true;
    m_bHalt             = false;
    m_bStop             = false;

    m_pMem->WriteMMIO( MMIOBiosDisabled, 0x00 );

    /*
    // Init code to skip bios

    m_Registers[ A ]    = 0x01;
    m_Registers[ B ]    = 0x00;
    m_Registers[ C ]    = 0x13;
    m_Registers[ D ]    = 0x00;
    m_Registers[ E ]    = 0xD8;
    m_Registers[ F ]    = 0xB0;
    m_Registers[ H ]    = 0x01;
    m_Registers[ L ]    = 0x4D;

    m_SP                = 0xFFFE;

    [$FF05] = $00   ; TIMA
    [$FF06] = $00   ; TMA
    [$FF07] = $00   ; TAC
    [$FF10] = $80   ; NR10
    [$FF11] = $BF   ; NR11
    [$FF12] = $F3   ; NR12
    [$FF14] = $BF   ; NR14
    [$FF16] = $3F   ; NR21
    [$FF17] = $00   ; NR22
    [$FF19] = $BF   ; NR24
    [$FF1A] = $7F   ; NR30
    [$FF1B] = $FF   ; NR31
    [$FF1C] = $9F   ; NR32
    [$FF1E] = $BF   ; NR33
    [$FF20] = $FF   ; NR41
    [$FF21] = $00   ; NR42
    [$FF22] = $00   ; NR43
    [$FF23] = $BF   ; NR30
    [$FF24] = $77   ; NR50
    [$FF25] = $F3   ; NR51
    [$FF26] = $F1-GB, $F0-SGB ; NR52
    [$FF40] = $91   ; LCDC
    [$FF42] = $00   ; SCY
    [$FF43] = $00   ; SCX
    [$FF45] = $00   ; LYC
    [$FF47] = $FC   ; BGP
    [$FF48] = $FF   ; OBP0
    [$FF49] = $FF   ; OBP1
    [$FF4A] = $00   ; WY
    [$FF4B] = $00   ; WX
    [$FFFF] = $00   ; IE
    */

    CpuState state;
    m_u32HistoryIndex = 0;
    for( int i = 0; i < DebugHistorySize; ++i )
    {
        m_u16History[ i ] = state;
    }

    m_DebugPC = 0x60a7;
    m_bInit = false;
}

//----------------------------------------------------------------------------------------------------
void GBCpu::SetPC( uint16 u16PC )
{
    static int i = 0;
    /*
    if( m_bInit )
    {
        if( i > DebugHistorySize )
        {
            DebugDumpHistory();
            __asm int 3;
        }
        ++i;
        //DebugDumpHistory();
    }
    */
}

//----------------------------------------------------------------------------------------------------
int GBCpu::ExecuteOpcode()
{
    uint32              iCycles        = 4;
    GBCpuOpcodeHandler  pfnHandler;
    ubyte               opcode;

    if( IsRunning() )
    {
#ifdef _DEBUG
        if( !m_pMem->IsBootRomEnabled() )
        {
            if( m_PC > 0 && m_PC == m_DebugPC )
            {
                m_bInit = true;
            }

            if( m_bInit )
            {
                CpuState state( this );
                m_u16History[ m_u32HistoryIndex++ ] = state;
                if( m_u32HistoryIndex >= DebugHistorySize )
                {
                    m_u32HistoryIndex = 0;
                }
            }
        }
#endif
        SetPC( m_PC );

        // Grab the current opcode
        opcode = ReadMemory( m_PC++ );

        // Get the handler for this opcode
        pfnHandler  = m_OpcodeHandlers[ opcode ];

        // Invoke the handler
        iCycles = (this->*pfnHandler)();
    }
    else
    {
        SimulateIO( 4 );
    }

    // Return the number of cycles taken
    return iCycles;
}

//----------------------------------------------------------------------------------------------------
void GBCpu::RaiseInterrupt( Interrupt interrupt )
{
    // Read the interrupt flag byte
    ubyte u8InterruptFlags = m_pMem->ReadMMIO( MMIOInterruptFlags );

    // Set the interrupt request
    u8InterruptFlags |= interrupt;

    // Write the interrupt flag byte back to MMIO
    m_pMem->WriteMMIO( MMIOInterruptFlags, u8InterruptFlags );
}

//----------------------------------------------------------------------------------------------------
int GBCpu::HandleInterrupts()
{
    ubyte u8InterruptFlags  = m_pMem->ReadMMIO( MMIOInterruptFlags );
    ubyte u8InterruptEnable = m_pMem->ReadMMIO( MMIOInterruptEnable );

    if(     u8InterruptEnable & VBlank
        &&  u8InterruptFlags & VBlank )
    {
        m_bHalt = false;

        if( m_bIME )
        {
            m_pMem->WriteMMIO( MMIOInterruptFlags, u8InterruptFlags &= ~VBlank );
            return ExecuteInterrupt( 0x40 );
        }
    }
    
    if(     u8InterruptEnable & LCDStatus
        &&  u8InterruptFlags & LCDStatus )
    {
        m_bHalt = false;
        
        if( m_bIME )
        {
            m_pMem->WriteMMIO( MMIOInterruptFlags, u8InterruptFlags &= ~LCDStatus );
            return ExecuteInterrupt( 0x48 );
        }
    }
    
    if(     u8InterruptEnable & Timer
        &&  u8InterruptFlags & Timer )
    {
        m_bHalt = false;
        
        if( m_bIME )
        {
            m_pMem->WriteMMIO( MMIOInterruptFlags, u8InterruptFlags &= ~Timer );
            return ExecuteInterrupt( 0x50 );
        }
    }
    
    if(     u8InterruptEnable & Serial
        &&  u8InterruptFlags & Serial )
    {
        m_bHalt = false;
        
        if( m_bIME )
        {
            m_pMem->WriteMMIO( MMIOInterruptFlags, u8InterruptFlags &= ~Serial );
            return ExecuteInterrupt( 0x58 );
        }
    }
    
    if(     u8InterruptEnable & Input
        &&  u8InterruptFlags & Input )
    {
        m_bHalt = false;
        m_bStop = false;
        
        if( m_bIME )
        {
            m_pMem->WriteMMIO( MMIOInterruptFlags, u8InterruptFlags &= ~Input );
            return ExecuteInterrupt( 0x60 );
        }
    }

    return 0;
}

//----------------------------------------------------------------------------------------------------
ubyte GBCpu::ReadMemory( uint16 u16Addr )
{
    ubyte u8Data = m_pMem->ReadMemory( u16Addr );
    m_pTimer->Update( 4 );
    return u8Data;
}

//----------------------------------------------------------------------------------------------------
void GBCpu::WriteMemory( uint16 u16Addr, ubyte u8Data )
{
    m_pMem->WriteMemory( u16Addr, u8Data );
    m_pTimer->Update( 4 );
}

//----------------------------------------------------------------------------------------------------
void GBCpu::SimulateIO( ubyte u8Clocks )
{
    m_pTimer->Update( u8Clocks );
}

//----------------------------------------------------------------------------------------------------
void GBCpu::DebugDumpHistory()
{
    CpuState* state;
    Log()->BeginBatchWrite();
    for( int i = 0; i < DebugHistorySize; ++i )
    {
        state = &m_u16History[ m_u32HistoryIndex++ ];
        Log()->BatchWrite( LOG_COLOR_WHITE, "PC=0x%04x AF=0x%04x BC=0x%04x DE=0x%04x HL=0x%04x SP=0x%04x ", state->PC, state->AF, state->BC, state->DE, state->HL, state->SP );
        if( m_u32HistoryIndex >= DebugHistorySize )
        {
            m_u32HistoryIndex = 0;
        }
    }
    Log()->EndBatchWrite();
}

//----------------------------------------------------------------------------------------------------
int GBCpu::ExecuteExtOpcode()
{
    GBCpuOpcodeHandler  pfnHandler;
    ubyte               extopcode;
    uint32              iCycles        = 0;

    // Grab the current opcode
    extopcode   = ReadMemory( m_PC++ );

    // Get the handler for this opcode
    pfnHandler  = m_OpcodeExtHandlers[ extopcode ];

    // Invoke the handler
    iCycles = (this->*pfnHandler)();

    // Return the number of cycles taken
    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::ExecuteInterrupt( uint16 addr )
{
    PROFILE( "ExecuteInterrupt" );

    m_bIME = false;

    WriteMemory( --m_SP, m_PC >> 8 );
    WriteMemory( --m_SP, m_PC & 0xFF );

    SimulateIO( 8 );

    m_PC = addr;

    return 20;
}

//----------------------------------------------------------------------------------------------------
template<unsigned OP>
int GBCpu::OpInvalid()
{
    // Unknown or invalid opcode
    printf( "UnknownOpcode: 0x%X\n", OP );

    DebugDumpHistory();

    _asm int 3;

    return 0;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpLD_r_n()
{
    PROFILE( "OpLD_r_n" );

    m_Registers[ X ] = ReadMemory( m_PC++ );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, unsigned Y>
int GBCpu::OpLD_r_r()
{
    PROFILE( "OpLD_r_r" );

    m_Registers[ X ] = m_Registers[ Y ];

    return 4;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, GBCpu::RegisterPair XY>
int GBCpu::OpLD_r_rr()
{
    PROFILE( "OpLD_r_rr" );

    m_Registers[ X ] = ReadMemory( GetRegisterPair( XY ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<GBCpu::RegisterPair XY, unsigned X>
int GBCpu::OpLD_rr_r()
{
    PROFILE( "OpLD_rr_r" );

    WriteMemory( GetRegisterPair( XY ), m_Registers[ X ] );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<GBCpu::RegisterPair XY>
int GBCpu::OpLD_rr_n()
{
    PROFILE( "OpLD_rr_n" );

    WriteMemory( GetRegisterPair( XY ), ReadMemory( m_PC++ ) );

    return 12;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpLD_r_nn()
{
    PROFILE( "OpLD_r_nn" );

    ubyte lo = ReadMemory( m_PC++ );
    ubyte hi = ReadMemory( m_PC++ );
    
    m_Registers[ X ] = ReadMemory( ( hi << 8 ) | lo );

    return 16;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpLD_nn_r()
{
    PROFILE( "OpLD_nn_r" );

    ubyte lo = ReadMemory( m_PC++ );
    ubyte hi = ReadMemory( m_PC++ );

    WriteMemory( ( hi << 8 ) | lo, m_Registers[ X ] );

    return 16;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLDH_A_C()
{
    PROFILE( "OpLDH_A_C" );

    m_Registers[ A ] = ReadMemory( 0xFF00 + m_Registers[ C ] );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLDH_A_n()
{
    PROFILE( "OpLDH_A_n" );

    m_Registers[ A ] = ReadMemory( 0xFF00 + ReadMemory( m_PC++ ) );

    return 12;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLDH_C_A()
{
    PROFILE( "OpLDH_C_A" );

    WriteMemory( 0xFF00 + m_Registers[ C ], m_Registers[ A ] );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLDH_n_A()
{
    PROFILE( "OpLDH_n_A" );

    WriteMemory( 0xFF00 + ReadMemory( m_PC++ ), m_Registers[ A ] );

    return 12;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLDD_A_HL()
{
    PROFILE( "OpLDD_A_HL" );

    uint16 u16HL = GetRegisterPair( HL );

    m_Registers[ A ] = ReadMemory( u16HL );
    SetRegisterPair( HL, --u16HL );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLDD_HL_A()
{
    PROFILE( "OpLDD_HL_A" );

    uint16 u16HL = GetRegisterPair( HL );

    WriteMemory( u16HL, m_Registers[ A ] );
    SetRegisterPair( HL, --u16HL );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLDI_A_HL()
{
    PROFILE( "OpLDI_A_HL" );

    uint16 u16HL = GetRegisterPair( HL );

    m_Registers[ A ] = ReadMemory( u16HL );
    SetRegisterPair( HL, ++u16HL );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLDI_HL_A()
{
    PROFILE( "OpLDI_HL_A" );

    uint16 u16HL = GetRegisterPair( HL );

    WriteMemory( u16HL, m_Registers[ A ] );
    SetRegisterPair( HL, ++u16HL );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<GBCpu::RegisterPair XY>
int GBCpu::OpLD_rr_nn()
{
    PROFILE( "OpLD_rr_nn" );

    uint16 nn = ReadMemory( m_PC++ ) | ( ReadMemory( m_PC++ ) << 8 );
    SetRegisterPair( XY, nn );

    return 12;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLD_SP_nn()
{
    PROFILE( "OpLD_SP_nn" );

    uint16 nn = ReadMemory( m_PC++ ) | ( ReadMemory( m_PC++ ) << 8 );
    m_SP = nn;

    return 12;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLD_SP_HL()
{
    PROFILE( "OpLD_SP_HL" );

    m_SP = GetRegisterPair( HL );

    SimulateIO( 4 );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLD_HL_SP_n()
{
    PROFILE( "OpLD_HL_SP_n" );

    sbyte n = ReadMemory( m_PC++ );
    SetRegisterPair( HL, m_SP + n );

    // Adjust flags
    SetRegisterFlag( ZF, false );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, ( ( m_SP & 0x0F ) + ( n & 0x0F ) ) > 0x0F );
    SetRegisterFlag( CF, ( ( m_SP & 0xFF ) + ( n & 0xFF ) ) > 0xFF );

    SimulateIO( 4 );

    return 12;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpLD_nn_SP()
{
    PROFILE( "OpLD_nn_SP" );

    uint16 addr    = ReadMemory( m_PC++ ) | ( ReadMemory( m_PC++ ) << 8 );

    WriteMemory( addr, m_SP & 0xFF );
    WriteMemory( addr + 1, m_SP >> 8 );

    return 20;
}

//----------------------------------------------------------------------------------------------------
template<GBCpu::RegisterPair XY>
int GBCpu::OpPUSH_rr_nn()
{
    PROFILE( "OpPUSH_rr_nn" );

    uint16 u16XY = GetRegisterPair( XY );

    WriteMemory( --m_SP, u16XY >> 8 );
    WriteMemory( --m_SP, u16XY & 0xFF );

    SimulateIO( 4 );

    return 16;
}

//----------------------------------------------------------------------------------------------------
template<GBCpu::RegisterPair XY>
int GBCpu::OpPOP_rr_nn()
{
    PROFILE( "OpPOP_rr_nn" );

    ubyte lo = ReadMemory( m_SP++ );
    ubyte hi = ReadMemory( m_SP++ );

    SetRegisterPair( XY, ( hi << 8 ) | lo );

    return 12;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
void GBCpu::OpADD_r_v( ubyte value )
{
    PROFILE( "OpADD_r_v" );

    ubyte total = m_Registers[ X ] + value;

    // Adjust flags
    SetRegisterFlag( ZF, !total );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, ( ( m_Registers[ X ] & 0x0F ) + ( value & 0x0F ) ) > 0x0F );
    SetRegisterFlag( CF, ( ( m_Registers[ X ] & 0xFF ) + ( value & 0xFF ) ) > 0xFF );

    m_Registers[ X ] = total;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, unsigned Y>
int GBCpu::OpADD_r_r()
{
    PROFILE( "OpADD_r_r" );

    OpADD_r_v<X>( m_Registers[ Y ] );

    return 4;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, GBCpu::RegisterPair XY>
int GBCpu::OpADD_r_rr()
{
    PROFILE( "OpADD_r_rr" );

    OpADD_r_v<X>( ReadMemory( GetRegisterPair( XY ) ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpADD_r_n()
{
    PROFILE( "OpADD_r_n" );

    OpADD_r_v<X>( ReadMemory( m_PC++ ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
void GBCpu::OpADC_r_v( ubyte value )
{
    PROFILE( "OpADC_r_v" );

    ubyte carry    = GetRegisterFlag( CF );
    ubyte total = m_Registers[ X ] + value + carry;

    // Adjust flags
    SetRegisterFlag( ZF, !total );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, ( ( m_Registers[ X ] & 0x0F ) + ( value & 0x0F ) + carry ) > 0x0F );
    SetRegisterFlag( CF, ( ( m_Registers[ X ] & 0xFF ) + ( value & 0xFF ) + carry ) > 0xFF );

    m_Registers[ X ] = total;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, unsigned Y>
int GBCpu::OpADC_r_r()
{
    PROFILE( "OpADC_r_r" );

    OpADC_r_v<X>( m_Registers[ Y ] );

    return 4;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, GBCpu::RegisterPair XY>
int GBCpu::OpADC_r_rr()
{
    PROFILE( "OpADC_r_rr" );

    OpADC_r_v<X>( ReadMemory( GetRegisterPair( XY ) ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpADC_r_n()
{
    PROFILE( "OpADC_r_n" );

    OpADC_r_v<X>( ReadMemory( m_PC++ ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
void GBCpu::OpSUB_r_v( ubyte value )
{
    PROFILE( "OpSUB_r_v" );

    uint16 total = m_Registers[ X ] - value;
    uint16 lower = ( m_Registers[ X ] & 0x0F ) - ( value & 0x0F );

    // Adjust flags
    SetRegisterFlag( ZF, !total );
    SetRegisterFlag( NF, true );
    SetRegisterFlag( HF, lower > 0x0F );
    SetRegisterFlag( CF, total > 0xFF );

    m_Registers[ X ] = static_cast<ubyte>( total );
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, unsigned Y>
int GBCpu::OpSUB_r_r()
{
    PROFILE( "OpSUB_r_r" );

    OpSUB_r_v<X>( m_Registers[ Y ] );

    return 4;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, GBCpu::RegisterPair XY>
int GBCpu::OpSUB_r_rr()
{
    PROFILE( "OpSUB_r_rr" );

    OpSUB_r_v<X>( ReadMemory( GetRegisterPair( XY ) ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpSUB_r_n()
{
    PROFILE( "OpSUB_r_n" );

    OpSUB_r_v<X>( ReadMemory( m_PC++ ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
void GBCpu::OpSBC_r_v( ubyte value )
{
    PROFILE( "OpSBC_r_v" );

    ubyte carry        = GetRegisterFlag( CF );
    uint16 total    = m_Registers[ X ] - value - carry;
    uint16 lower    = ( m_Registers[ X ] & 0x0F ) - ( value & 0x0F ) - carry;

    // Adjust flags
    SetRegisterFlag( ZF, !static_cast<ubyte>( total ) );
    SetRegisterFlag( NF, true );
    SetRegisterFlag( HF, lower > 0x0F );
    SetRegisterFlag( CF, total > 0xFF );

    m_Registers[ X ] = static_cast<ubyte>( total );
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, unsigned Y>
int GBCpu::OpSBC_r_r()
{
    PROFILE( "OpSBC_r_r" );

    OpSBC_r_v<X>( m_Registers[ Y ] );

    return 4;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, GBCpu::RegisterPair XY>
int GBCpu::OpSBC_r_rr()
{
    PROFILE( "OpSBC_r_rr" );

    OpSBC_r_v<X>( ReadMemory( GetRegisterPair( XY ) ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpSBC_r_n()
{
    PROFILE( "OpSBC_r_n" );

    OpSBC_r_v<X>( ReadMemory( m_PC++ ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
void GBCpu::OpAND_r_v( ubyte value )
{
    PROFILE( "OpAND_r_v" );

    ubyte mask = m_Registers[ X ] & value;

    // Adjust flags
    SetRegisterFlag( ZF, !mask );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, true );
    SetRegisterFlag( CF, false );

    m_Registers[ X ] = mask;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, unsigned Y>
int GBCpu::OpAND_r_r()
{
    PROFILE( "OpAND_r_r" );

    OpAND_r_v<X>( m_Registers[ Y ] );

    return 4;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, GBCpu::RegisterPair XY>
int GBCpu::OpAND_r_rr()
{
    PROFILE( "OpAND_r_rr" );

    OpAND_r_v<X>( ReadMemory( GetRegisterPair( XY ) ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpAND_r_n()
{
    PROFILE( "OpAND_r_n" );

    OpAND_r_v<X>( ReadMemory( m_PC++ ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
void GBCpu::OpOR_r_v( ubyte value )
{
    PROFILE( "OpOR_r_v" );

    ubyte mask = m_Registers[ X ] | value;

    // Adjust flags
    SetRegisterFlag( ZF, !mask );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, false );

    m_Registers[ X ] = mask;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, unsigned Y>
int GBCpu::OpOR_r_r()
{
    PROFILE( "OpOR_r_r" );

    OpOR_r_v<X>( m_Registers[ Y ] );

    return 4;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, GBCpu::RegisterPair XY>
int GBCpu::OpOR_r_rr()
{
    PROFILE( "OpOR_r_rr" );

    OpOR_r_v<X>( ReadMemory( GetRegisterPair( XY ) ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpOR_r_n()
{
    PROFILE( "OpOR_r_n" );

    OpOR_r_v<X>( ReadMemory( m_PC++ ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
void GBCpu::OpXOR_r_v( ubyte value )
{
    PROFILE( "OpXOR_r_v" );

    ubyte mask = m_Registers[ X ] ^ value;

    // Adjust flags
    SetRegisterFlag( ZF, !mask );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, false );

    m_Registers[ X ] = mask;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, unsigned Y>
int GBCpu::OpXOR_r_r()
{
    PROFILE( "OpXOR_r_r" );

    OpXOR_r_v<X>( m_Registers[ Y ] );

    return 4;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, GBCpu::RegisterPair XY>
int GBCpu::OpXOR_r_rr()
{
    PROFILE( "OpXOR_r_rr" );

    OpXOR_r_v<X>( ReadMemory( GetRegisterPair( XY ) ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpXOR_r_n()
{
    PROFILE( "OpXOR_r_n" );

    OpXOR_r_v<X>( ReadMemory( m_PC++ ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
void GBCpu::OpCMP_r_v( ubyte value )
{
    uint16 total = m_Registers[ X ] - value;
    uint16 lower = ( m_Registers[ X ] & 0x0F ) - ( value & 0x0F );

    // Adjust flags
    SetRegisterFlag( ZF, !total );
    SetRegisterFlag( NF, true );
    SetRegisterFlag( HF, lower > 0x0F );
    SetRegisterFlag( CF, total > 0xFF );
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, unsigned Y>
int GBCpu::OpCMP_r_r()
{
    PROFILE( "OpCMP_r_r" );

    OpCMP_r_v<X>( m_Registers[ Y ] );

    return 4;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X, GBCpu::RegisterPair XY>
int GBCpu::OpCMP_r_rr()
{
    PROFILE( "OpCMP_r_rr" );

    OpCMP_r_v<X>( ReadMemory( GetRegisterPair( XY ) ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpCMP_r_n()
{
    PROFILE( "OpCMP_r_n" );

    OpCMP_r_v<X>( ReadMemory( m_PC++ ) );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpINC_r()
{
    PROFILE( "OpINC_r" );

    ubyte value = m_Registers[ X ] + 1;

    // Adjust flags
    SetRegisterFlag( ZF, !value );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, ( ( m_Registers[ X ] & 0x0F ) + 1 ) > 0x0F );
    
    m_Registers[ X ] = value;

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpINC_HL()
{
    PROFILE( "OpINC_HL" );

    uint16 addr     = GetRegisterPair( HL );
    ubyte value     = ReadMemory( addr );
    ubyte newValue  = value + 1;

    // Adjust flags
    SetRegisterFlag( ZF, !newValue );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, ( ( value & 0x0F ) + 1 ) > 0x0F );

    WriteMemory( addr, newValue );

    return 12;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpDEC_r()
{
    PROFILE( "OpDEC_r" );

    ubyte value = m_Registers[ X ] - 1;

    // Adjust flags
    SetRegisterFlag( ZF, !value );
    SetRegisterFlag( NF, true );
    SetRegisterFlag( HF, ( value & 0x0F ) == 0x0F );
    
    m_Registers[ X ] = value;

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpDEC_HL()
{
    PROFILE( "OpDEC_HL" );

    uint16 addr     = GetRegisterPair( HL );
    ubyte value     = ReadMemory( addr );
    ubyte newValue  = value - 1;

    // Adjust flags
    SetRegisterFlag( ZF, !newValue );
    SetRegisterFlag( NF, true );
    SetRegisterFlag( HF, ( newValue & 0x0F ) == 0x0F );

    WriteMemory( addr, newValue );

    return 12;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpDAA()
{
    PROFILE( "OpDAA" );

    bool n      = GetRegisterFlag( NF );
    bool h      = GetRegisterFlag( HF );
    bool c      = GetRegisterFlag( CF );
    uint16 a    = m_Registers[ A ];

    // Handle addition
    if( !n )
    {
        if( h || ( a & 0x0F ) > 0x09 )
        {
            a += 0x06;
        }
        if( c || a > 0x9F )
        {
            a += 0x60;
        }
    }
    else // Handle subtraction
    {
        if( h )
        {
            a -= 0x06;
            if( !c )
            {
                a &= 0xFF;
            }
        }
        if( c )
        {
            a -= 0x60;
        }
    }

    m_Registers[ A ] = static_cast<ubyte>( a );

    SetRegisterFlag( ZF, !m_Registers[ A ] );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c | ( 0 != ( a & 0x100 ) ) );

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpCPL()
{
    PROFILE( "OpCPL" );

    m_Registers[ A ] ^= 0xFF;

    // Adjust flags
    SetRegisterFlag( NF, true );
    SetRegisterFlag( HF, true );

    return 4;
}

//----------------------------------------------------------------------------------------------------
template<GBCpu::RegisterPair XY>
int GBCpu::OpADD_HL_rr()
{
    PROFILE( "OpADD_HL_rr" );

    uint16 u16HL    = GetRegisterPair( HL );
    uint16 value    = GetRegisterPair( XY );

    // Adjust flags
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, ( ( u16HL & 0x0FFF ) + ( value & 0x0FFF ) ) > 0x0FFF );
    SetRegisterFlag( CF, ( ( u16HL & 0xFFFF ) + ( value & 0xFFFF ) ) > 0xFFFF );

    SetRegisterPair( HL, u16HL + value );

    SimulateIO( 4 );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpADD_HL_SP()
{
    PROFILE( "OpADD_HL_SP" );

    uint16 u16HL    = GetRegisterPair( HL );

    // Adjust flags
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, ( ( u16HL & 0x0FFF ) + ( m_SP & 0x0FFF ) ) > 0x0FFF );
    SetRegisterFlag( CF, ( ( u16HL & 0xFFFF ) + ( m_SP & 0xFFFF ) ) > 0xFFFF );

    SetRegisterPair( HL, u16HL + m_SP );

    SimulateIO( 4 );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpADD_SP_n()
{
    PROFILE( "OpADD_SP_n" );

    sbyte n = static_cast<sbyte>( ReadMemory( m_PC++ ) );

    // Adjust flags
    SetRegisterFlag( ZF, false );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, ( ( m_SP & 0x0F ) + ( n & 0x0F ) ) > 0x0F );
    SetRegisterFlag( CF, ( ( m_SP & 0xFF ) + ( n & 0xFF ) ) > 0xFF );

    m_SP += n;

    SimulateIO( 8 );

    return 16;
}

//----------------------------------------------------------------------------------------------------
template<GBCpu::RegisterPair XY>
int GBCpu::OpINC_rr()
{
    PROFILE( "OpINC_rr" );

    uint16 u16XY = GetRegisterPair( XY );
    SetRegisterPair( XY, u16XY + 1 );

    SimulateIO( 4 );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpINC_SP()
{
    PROFILE( "OpINC_SP" );

    ++m_SP;

    SimulateIO( 4 );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<GBCpu::RegisterPair XY>
int GBCpu::OpDEC_rr()
{
    PROFILE( "OpDEC_rr" );

    uint16 u16XY = GetRegisterPair( XY );
    SetRegisterPair( XY, u16XY - 1 );

    SimulateIO( 4 );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpDEC_SP()
{
    PROFILE( "OpDEC_SP" );

    --m_SP;

    SimulateIO( 4 );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpSWAP_r()
{
    PROFILE( "OpSWAP_r" );

    m_Registers[ X ] = ( m_Registers[ X ] << 4 ) | ( m_Registers[ X ] >> 4 );

    SetRegisterFlag( ZF, !m_Registers[ X ] );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, false );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpSWAP_HL()
{
    PROFILE( "OpSWAP_HL" );

    uint16 addr    = GetRegisterPair( HL );
    ubyte value = ReadMemory( addr );

    value = ( value << 4 ) | ( value >> 4 );

    // Adjust flags
    SetRegisterFlag( ZF, !value );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, false );

    WriteMemory( addr, value );

    return 16;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpCCF()
{
    PROFILE( "OpCCF" );

    bool c = GetRegisterFlag( CF );

    // Adjust flags
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, !c );

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpSCF()
{
    PROFILE( "OpSCF" );

    // Adjust flags
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, true );

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpNOP()
{
    PROFILE( "OpNOP" );

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpHALT()
{
    PROFILE( "OpHALT" );

    /*
    Short information
    -----------------
    HALT stops the CPU until an interrupt occurs. Nintendo recommends using this 
    command in your main game loop in order to save battery power while the CPU has 
    nothing else to do.

    When an interrupt occurs while in HALT, the CPU starts back up and pushes the 
    Program Counter onto the stack before servicing the interrupt(s). Except it 
    doesn't push the address after HALT as one might expect but rather the address 
    of HALT itself.

    Nintendo also recommends that you put a NOP after HALT commands. The reason for 
    this is that the Program Counter will not increment properly (CPU bug) if you 
    try to do a HALT while IME = 0 and an interrupt is pending. A single-byte 
    instruction immediately following HALT will get executed twice if IME = 0 and an 
    interrupt is pending. If the instruction following HALT is a multi-byte 
    instruction then the game could hang or registers could get scrambled.
    */

    m_bHalt = true;

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpSTOP()
{
    PROFILE( "OpSTOP" );

    m_bStop = true;

    DebugDumpHistory();
    __asm int 3;

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpDI()
{
    PROFILE( "OpDI" );

    m_bIME = false;

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpEI()
{
    PROFILE( "OpEI" );

    m_bIME = true;

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRLCA()
{
    PROFILE( "OpRLCA" );

    bool c = 0 != ( m_Registers[ A ] & 0x80 );

    m_Registers[ A ] = ( m_Registers[ A ] << 1 ) | ( m_Registers[ A ] >> 7 );

    // Adjust flags
    SetRegisterFlag( ZF, false );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRLA()
{
    PROFILE( "OpRLA" );

    bool c = 0 != ( m_Registers[ A ] & 0x80 );

    m_Registers[ A ] = ( m_Registers[ A ] << 1 ) | static_cast<ubyte>( GetRegisterFlag( CF ) );

    // Adjust flags
    SetRegisterFlag( ZF, false );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRRCA()
{
    PROFILE( "OpRRCA" );

    bool c = 0 != ( m_Registers[ A ] & 0x01 );

    m_Registers[ A ] = ( m_Registers[ A ] >> 1 ) | ( m_Registers[ A ] << 7 );

    // Adjust flags
    SetRegisterFlag( ZF, false );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRRA()
{
    PROFILE( "OpRRA" );

    bool c = 0 != ( m_Registers[ A ] & 0x01 );

    m_Registers[ A ] = ( m_Registers[ A ] >> 1 ) | (  static_cast<ubyte>( GetRegisterFlag( CF ) ) << 7 );

    // Adjust flags
    SetRegisterFlag( ZF, false );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 4;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpRLC_r()
{
    PROFILE( "OpRLC_r" );

    bool c = 0 != ( m_Registers[ X ] & 0x80 );

    m_Registers[ X ] = ( m_Registers[ X ] << 1 ) | ( m_Registers[ X ] >> 7 );

    // Adjust flags
    SetRegisterFlag( ZF, !m_Registers[ X ] );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRLC_HL()
{
    PROFILE( "OpRLC_HL" );

    uint16 addr    = GetRegisterPair( HL );
    ubyte n        = ReadMemory( addr );
    bool c        = 0 != ( n & 0x80 );

    n = ( n << 1 ) | ( n >> 7 );

    WriteMemory( addr, n );

    // Adjust flags
    SetRegisterFlag( ZF, !n );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 16;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpRL_r()
{
    PROFILE( "OpRL_r" );

    bool c = 0 != ( m_Registers[ X ] & 0x80 );

    m_Registers[ X ] = ( m_Registers[ X ] << 1 ) | static_cast<ubyte>( GetRegisterFlag( CF ) );

    // Adjust flags
    SetRegisterFlag( ZF, !m_Registers[ X ] );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRL_HL()
{
    PROFILE( "OpRL_HL" );

    uint16 addr     = GetRegisterPair( HL );
    ubyte n         = ReadMemory( addr );
    bool c          = 0 != ( n & 0x80 );

    n = ( n << 1 ) | static_cast<ubyte>( GetRegisterFlag( CF ) );

    WriteMemory( addr, n );

    // Adjust flags
    SetRegisterFlag( ZF, !n );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 16;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpRRC_r()
{
    PROFILE( "OpRRC_r" );

    bool c = 0 != ( m_Registers[ X ] & 0x01 );

    m_Registers[ X ] = ( m_Registers[ X ] >> 1 ) | ( m_Registers[ X ] << 7 );

    // Adjust flags
    SetRegisterFlag( ZF, !m_Registers[ X ] );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRRC_HL()
{
    PROFILE( "OpRRC_HL" );

    uint16 addr     = GetRegisterPair( HL );
    ubyte n         = ReadMemory( addr );
    bool c          = 0 != ( n & 0x01 );

    n = ( n >> 1 ) | ( n << 7 );

    WriteMemory( addr, n );

    // Adjust flags
    SetRegisterFlag( ZF, !n );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 16;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpRR_r()
{
    PROFILE( "OpRR_r" );

    bool c = 0 != ( m_Registers[ X ] & 0x01 );

    m_Registers[ X ] = ( m_Registers[ X ] >> 1 ) | (  static_cast<ubyte>( GetRegisterFlag( CF ) ) << 7 );

    // Adjust flags
    SetRegisterFlag( ZF, !m_Registers[ X ] );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRR_HL()
{
    PROFILE( "OpRR_HL" );

    uint16 addr    = GetRegisterPair( HL );
    ubyte n        = ReadMemory( addr );
    bool c = 0 != ( n & 0x01 );

    n = ( n >> 1 ) | (  static_cast<ubyte>( GetRegisterFlag( CF ) ) << 7 );

    WriteMemory( addr, n );

    // Adjust flags
    SetRegisterFlag( ZF, !n );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 16;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpSLA_r()
{
    PROFILE( "OpSLA_r" );

    bool c = 0 != ( m_Registers[ X ] & 0x80 );

    m_Registers[ X ] = m_Registers[ X ] << 1;

    // Adjust flags
    SetRegisterFlag( ZF, !m_Registers[ X ] );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpSLA_HL()
{
    PROFILE( "OpSLA_HL" );

    uint16 addr     = GetRegisterPair( HL );
    ubyte n         = ReadMemory( addr );
    bool c          = 0 != ( n & 0x80 );

    n = n << 1;

    WriteMemory( addr, n );

    // Adjust flags
    SetRegisterFlag( ZF, !n );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 16;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpSRA_r()
{
    PROFILE( "OpSRA_r" );

    bool c = 0 != ( m_Registers[ X ] & 0x01 );

    m_Registers[ X ] = ( m_Registers[ X ] & 0x80 ) | m_Registers[ X ] >> 1;

    // Adjust flags
    SetRegisterFlag( ZF, !m_Registers[ X ] );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpSRA_HL()
{
    PROFILE( "OpSRA_HL" );

    uint16 addr     = GetRegisterPair( HL );
    ubyte n         = ReadMemory( addr );
    bool c          = 0 != ( n & 0x01 );

    n = ( n & 0x80 ) | n >> 1;

    WriteMemory( addr, n );

    // Adjust flags
    SetRegisterFlag( ZF, !n );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 16;
}

//----------------------------------------------------------------------------------------------------
template<unsigned X>
int GBCpu::OpSRL_r()
{
    PROFILE( "OpSRL_r" );

    bool c = 0 != ( m_Registers[ X ] & 0x01 );

    m_Registers[ X ] = m_Registers[ X ] >> 1;

    // Adjust flags
    SetRegisterFlag( ZF, !m_Registers[ X ] );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 8;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpSRL_HL()
{
    PROFILE( "OpSRL_HL" );

    uint16 addr     = GetRegisterPair( HL );
    ubyte n         = ReadMemory( addr );
    bool c          = 0 != ( n & 0x01 );

    n = n >> 1;

    WriteMemory( addr, n );

    // Adjust flags
    SetRegisterFlag( ZF, !n );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, false );
    SetRegisterFlag( CF, c );

    return 16;
}

//----------------------------------------------------------------------------------------------------
template<unsigned b, unsigned X>
int GBCpu::OpBIT_r()
{
    PROFILE( "OpBIT_r" );

    ubyte test = m_Registers[ X ] & ( 1 << b );

    SetRegisterFlag( ZF, !test );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, true );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned b>
int GBCpu::OpBIT_HL()
{
    PROFILE( "OpBIT_HL" );

    uint16 addr     = GetRegisterPair( HL );
    ubyte n         = ReadMemory( addr );
    ubyte test      = n & ( 1 << b );

    SetRegisterFlag( ZF, !test );
    SetRegisterFlag( NF, false );
    SetRegisterFlag( HF, true );

    return 12;
}

//----------------------------------------------------------------------------------------------------
template<unsigned b, unsigned X>
int GBCpu::OpSET_r()
{
    PROFILE( "OpSET_r" );

    m_Registers[ X ] |= ( 1 << b );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned b>
int GBCpu::OpSET_HL()
{
    PROFILE( "OpSET_HL" );

    uint16 addr    = GetRegisterPair( HL );
    ubyte n        = ReadMemory( addr );
    
    WriteMemory( addr, n | ( 1 << b ) );

    return 16;
}

//----------------------------------------------------------------------------------------------------
template<unsigned b, unsigned X>
int GBCpu::OpRES_r()
{
    PROFILE( "OpRES_r" );

    m_Registers[ X ] &= ~( 1 << b );

    return 8;
}

//----------------------------------------------------------------------------------------------------
template<unsigned b>
int GBCpu::OpRES_HL()
{
    PROFILE( "OpRES_HL" );

    uint16 addr    = GetRegisterPair( HL );
    ubyte n        = ReadMemory( addr );
    
    WriteMemory( addr, n & ~( 1 << b ) );

    return 16;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpJP_nn()
{
    PROFILE( "OpJP_nn" );

    m_PC = ReadMemory( m_PC++ ) | ( ReadMemory( m_PC++ ) << 8 );

    SimulateIO( 4 );

    return 16;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpJP_NZ_nn()
{
    PROFILE( "OpJP_NZ_nn" );

    int iCycles = 12;
    
    ubyte lo = ReadMemory( m_PC++ );
    ubyte hi = ReadMemory( m_PC++ );

    if( !GetRegisterFlag( ZF ) )
    {
        m_PC = ( hi << 8 ) | lo;
        iCycles += 4;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpJP_Z_nn()
{
    PROFILE( "OpJP_Z_nn" );

    int    iCycles = 12;

    ubyte lo = ReadMemory( m_PC++ );
    ubyte hi = ReadMemory( m_PC++ );

    if( GetRegisterFlag( ZF ) )
    {
        m_PC = ( hi << 8 ) | lo;
        iCycles += 4;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpJP_NC_nn()
{
    PROFILE( "OpJP_NC_nn" );

    int iCycles = 12;

    ubyte lo = ReadMemory( m_PC++ );
    ubyte hi = ReadMemory( m_PC++ );

    if( !GetRegisterFlag( CF ) )
    {
        m_PC = ( hi << 8 ) | lo;
        iCycles += 4;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpJP_C_nn()
{
    PROFILE( "OpJP_C_nn" );

    int iCycles = 12;

    ubyte lo = ReadMemory( m_PC++ );
    ubyte hi = ReadMemory( m_PC++ );

    if( GetRegisterFlag( CF ) )
    {
        m_PC = ( hi << 8 ) | lo;
        iCycles += 4;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpJP_HL()
{
    PROFILE( "OpJP_HL" );

    m_PC = GetRegisterPair( HL );

    return 4;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpJR_n()
{
    PROFILE( "OpJR_n" );

    m_PC += static_cast<sbyte>( ReadMemory( m_PC++ ) );

    SimulateIO( 4 );

    return 12;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpJR_NZ_n()
{
    PROFILE( "OpJR_NZ_n" );

    int iCycles     = 8;
    sbyte n         = ReadMemory( m_PC++ );

    if( !GetRegisterFlag( ZF ) )
    {
        m_PC += n;
        iCycles += 4;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpJR_Z_n()
{
    PROFILE( "OpJR_Z_n" );

    int iCycles     = 8;
    sbyte n         = ReadMemory( m_PC++ );

    if( GetRegisterFlag( ZF ) )
    {
        m_PC += n;
        iCycles += 4;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpJR_NC_n()
{
    PROFILE( "OpJR_NC_n" );

    int iCycles     = 8;
    sbyte n         = ReadMemory( m_PC++ );

    if( !GetRegisterFlag( CF ) )
    {
        m_PC += n;
        iCycles += 4;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpJR_C_n()
{
    PROFILE( "OpJR_C_n" );

    int iCycles     = 8;
    sbyte n         = ReadMemory( m_PC++ );

    if( GetRegisterFlag( CF ) )
    {
        m_PC += n;
        iCycles += 4;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpCALL_nn()
{
    PROFILE( "OpCALL_nn" );

    uint16 addr = ReadMemory( m_PC++ ) | ( ReadMemory( m_PC++ ) << 8 );

    WriteMemory( --m_SP, m_PC >> 8 );
    WriteMemory( --m_SP, m_PC & 0xFF );

    m_PC = addr;

    SimulateIO( 4 );

    return 24;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpCALL_NZ_nn()
{
    PROFILE( "OpCALL_NZ_nn" );

    int iCycles    = 12;
    uint16 addr    = ReadMemory( m_PC++ ) | ( ReadMemory( m_PC++ ) << 8 );

    if( !GetRegisterFlag( ZF ) )
    {
        WriteMemory( --m_SP, m_PC >> 8 );
        WriteMemory( --m_SP, m_PC & 0xFF );

        m_PC = addr;

        iCycles += 12;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpCALL_Z_nn()
{
    PROFILE( "OpCALL_Z_nn" );

    int iCycles    = 12;
    uint16 addr    = ReadMemory( m_PC++ ) | ( ReadMemory( m_PC++ ) << 8 );

    if( GetRegisterFlag( ZF ) )
    {
        WriteMemory( --m_SP, m_PC >> 8 );
        WriteMemory( --m_SP, m_PC & 0xFF );

        m_PC = addr;
        iCycles += 12;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpCALL_NC_nn()
{
    PROFILE( "OpCALL_NC_nn" );

    int iCycles    = 12;
    uint16 addr    = ReadMemory( m_PC++ ) | ( ReadMemory( m_PC++ ) << 8 );

    if( !GetRegisterFlag( CF ) )
    {
        WriteMemory( --m_SP, m_PC >> 8 );
        WriteMemory( --m_SP, m_PC & 0xFF );

        m_PC = addr;
        iCycles += 12;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpCALL_C_nn()
{
    PROFILE( "OpCALL_C_nn" );

    int iCycles    = 12;
    uint16 addr    = ReadMemory( m_PC++ ) | ( ReadMemory( m_PC++ ) << 8 );

    if( GetRegisterFlag( CF ) )
    {
        WriteMemory( --m_SP, m_PC >> 8 );
        WriteMemory( --m_SP, m_PC & 0xFF );

        m_PC = addr;
        iCycles += 12;

        SimulateIO( 4 );
    }

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
template<unsigned ADDR>
int GBCpu::OpRST_nn()
{
    PROFILE( "OpRST_nn" );

    WriteMemory( --m_SP, m_PC >> 8 );
    WriteMemory( --m_SP, m_PC & 0xFF );

    m_PC = ADDR;

    SimulateIO( 4 );

    return 16;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRET_nn()
{
    PROFILE( "OpRET_nn" );

    ubyte lo = ReadMemory( m_SP++ );
    ubyte hi = ReadMemory( m_SP++ );

    m_PC = ( hi << 8 ) | lo;

    SimulateIO( 4 );

    return 16;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRET_NZ_nn()
{
    PROFILE( "OpRET_NZ_nn" );

    int iCycles = 8;

    if( !GetRegisterFlag( ZF ) )
    {
        ubyte    lo    = ReadMemory( m_SP++ );
        ubyte    hi    = ReadMemory( m_SP++ );

        m_PC = ( hi << 8 ) | lo;
        iCycles += 12;

        SimulateIO( 4 );
    }

    SimulateIO( 4 );

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRET_Z_nn()
{
    PROFILE( "OpRET_Z_nn" );

    int iCycles = 8;

    if( GetRegisterFlag( ZF ) )
    {
        ubyte    lo    = ReadMemory( m_SP++ );
        ubyte    hi    = ReadMemory( m_SP++ );

        m_PC = ( hi << 8 ) | lo;
        iCycles += 12;

        SimulateIO( 4 );
    }

    SimulateIO( 4 );

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRET_NC_nn()
{
    PROFILE( "OpRET_NC_nn" );

    int iCycles = 8;

    if( !GetRegisterFlag( CF ) )
    {
        ubyte    lo    = ReadMemory( m_SP++ );
        ubyte    hi    = ReadMemory( m_SP++ );

        m_PC = ( hi << 8 ) | lo;
        iCycles += 12;

        SimulateIO( 4 );
    }

    SimulateIO( 4 );

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRET_C_nn()
{
    PROFILE( "OpRET_C_nn" );

    int iCycles = 8;

    if( GetRegisterFlag( CF ) )
    {
        ubyte    lo    = ReadMemory( m_SP++ );
        ubyte    hi    = ReadMemory( m_SP++ );

        m_PC = ( hi << 8 ) | lo;
        iCycles += 12;

        SimulateIO( 4 );
    }

    SimulateIO( 4 );

    return iCycles;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpRETI_nn()
{
    PROFILE( "OpRETI_nn" );

    ubyte lo = ReadMemory( m_SP++ );
    ubyte hi = ReadMemory( m_SP++ );

    m_PC = ( hi << 8 ) | lo;

    // Enable interrupts
    m_bIME = true;
    
    SimulateIO( 4 );

    return 16;
}

//----------------------------------------------------------------------------------------------------
int GBCpu::OpExecuteExtOp()
{
    return ExecuteExtOpcode();
}
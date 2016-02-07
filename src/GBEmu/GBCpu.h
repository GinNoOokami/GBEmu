#ifndef GBEMU_GBCPU_H
#define GBEMU_GBCPU_H

//====================================================================================================
// Filename:    GBCpu.h
// Created by:  Jeff Padgham
// Description: The CPU module for a Game Boy emulator. This is a modified version of the Z80 CPU.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

#include "GBEmulator.h"

//====================================================================================================
// Foward Declarations
//====================================================================================================

class GBMem;
class GBTimer;
class GBCpuUnitTest;

//====================================================================================================
// Class
//====================================================================================================

class GBCpu
{
    friend class GBCpuUnitTest;

    // Enums
    enum
    {
        A, F,
        B, C,
        D, E,
        H, L
    };

    enum RegisterPair
    {
        AF = A,
        BC = B,
        DE = D,
        HL = H
    };

    enum RegisterFlag
    {
        ZF = 1 << 7,
        NF = 1 << 6,
        HF = 1 << 5,
        CF = 1 << 4
    };

    enum
    {
        DebugHistorySize    = 0x10000
    };

public:

    // Structs
    struct CpuState
    {
        friend GBCpu;

        uint16 AF;
        uint16 BC;
        uint16 DE;
        uint16 HL;
        uint16 PC;
        uint16 SP;

        CpuState()
        {
            AF = BC = DE = HL = PC = SP = 0;
        }

        CpuState( GBCpu* pCpu )
        {
            AF = pCpu->GetRegisterPair( GBCpu::AF );
            BC = pCpu->GetRegisterPair( GBCpu::BC );
            DE = pCpu->GetRegisterPair( GBCpu::DE );
            HL = pCpu->GetRegisterPair( GBCpu::HL );
            PC = pCpu->m_PC;
            SP = pCpu->m_SP;
        }
    };

public:
    // Class typedefs
    typedef int (GBCpu::*GBCpuOpcodeHandler)();

    // Static constants
    static const uint32 CLOCK_SPEED;

public:
    // Constructor / destructor
    GBCpu( GBMem* pMemoryModule, GBTimer* pTimer );
    ~GBCpu( void );

    void            SetPC( uint16 u16PC );
    bool            m_bInit;

    // Startup and cleanup
    void            Initialize();
    void            Terminate();

    // Cpu functions
    void            Reset();
    inline bool     IsRunning()                                         { return !m_bHalt && !m_bStop;    }
    int             ExecuteOpcode();
    void            RaiseInterrupt( Interrupt interrupt );
    int             HandleInterrupts();

private:
    // Utility functions
    inline uint16   GetRegisterPair( RegisterPair pair )                { return m_Registers[ pair ] << 8 | m_Registers[ pair + 1 ];                                                    }
    inline void     SetRegisterPair( RegisterPair pair, uint16 value )  { m_Registers[ pair ] = value >> 8; m_Registers[ pair + 1 ]    = ( AF != pair ) ? value & 0xFF : value & 0xF0;  }

    inline bool     GetRegisterFlag( RegisterFlag flag )                { return 0 != ( m_Registers[ F ] & flag );                                                                      }
    inline void     SetRegisterFlag( RegisterFlag flag, bool value )    { value ? m_Registers[ F ] |= flag : m_Registers[ F ] &= ~flag;                                                 }
    
    ubyte           ReadMemory( uint16 u16Addr );
    void            WriteMemory( uint16 u16Addr, ubyte u8Data );
    void            SimulateIO( ubyte u8Clocks );

    void            DebugDumpHistory();

    // Executes an extended opcode (0xCB)
    int             ExecuteExtOpcode();
    int             ExecuteInterrupt( uint16 addr );

    // Opcode handlers
    template<unsigned OP>                   int     OpInvalid();

    // 8 bit loads
    template<unsigned X>                    int     OpLD_r_n();
    template<unsigned X, unsigned Y>        int     OpLD_r_r();
    template<unsigned X, RegisterPair XY>   int     OpLD_r_rr();
    template<RegisterPair XY, unsigned X>   int     OpLD_rr_r();
    template<RegisterPair XY>               int     OpLD_rr_n();
    template<unsigned X>                    int     OpLD_r_nn();
    template<unsigned X>                    int     OpLD_nn_r();
                                            int     OpLDH_A_C();
                                            int     OpLDH_A_n();
                                            int     OpLDH_C_A();
                                            int     OpLDH_n_A();
                                            int     OpLDD_A_HL();
                                            int     OpLDD_HL_A();
                                            int     OpLDI_A_HL();
                                            int     OpLDI_HL_A();

    // 16 bit loads
    template<RegisterPair XY>               int     OpLD_rr_nn();
                                            int     OpLD_SP_nn();
                                            int     OpLD_SP_HL();
                                            int     OpLD_HL_SP_n();
                                            int     OpLD_nn_SP();
    template<RegisterPair XY>               int     OpPUSH_rr_nn();
    template<RegisterPair XY>               int     OpPOP_rr_nn();

    // 8 bit arithmetic/logic
    template<unsigned X>                    void    OpADD_r_v( ubyte value );
    template<unsigned X, unsigned Y>        int     OpADD_r_r();
    template<unsigned X, RegisterPair XY>   int     OpADD_r_rr();
    template<unsigned X>                    int     OpADD_r_n();
    template<unsigned X>                    void    OpADC_r_v( ubyte value );
    template<unsigned X, unsigned Y>        int     OpADC_r_r();
    template<unsigned X, RegisterPair XY>   int     OpADC_r_rr();
    template<unsigned X>                    int     OpADC_r_n();
    template<unsigned X>                    void    OpSUB_r_v( ubyte value );
    template<unsigned X, unsigned Y>        int     OpSUB_r_r();
    template<unsigned X, RegisterPair XY>   int     OpSUB_r_rr();
    template<unsigned X>                    int     OpSUB_r_n();
    template<unsigned X>                    void    OpSBC_r_v( ubyte value );
    template<unsigned X, unsigned Y>        int     OpSBC_r_r();
    template<unsigned X, RegisterPair XY>   int     OpSBC_r_rr();
    template<unsigned X>                    int     OpSBC_r_n();
    template<unsigned X>                    void    OpAND_r_v( ubyte value );
    template<unsigned X, unsigned Y>        int     OpAND_r_r();
    template<unsigned X, RegisterPair XY>   int     OpAND_r_rr();
    template<unsigned X>                    int     OpAND_r_n();
    template<unsigned X>                    void    OpOR_r_v( ubyte value );
    template<unsigned X, unsigned Y>        int     OpOR_r_r();
    template<unsigned X, RegisterPair XY>   int     OpOR_r_rr();
    template<unsigned X>                    int     OpOR_r_n();
    template<unsigned X>                    void    OpXOR_r_v( ubyte value );
    template<unsigned X, unsigned Y>        int     OpXOR_r_r();
    template<unsigned X, RegisterPair XY>   int     OpXOR_r_rr();
    template<unsigned X>                    int     OpXOR_r_n();
    template<unsigned X>                    void    OpCMP_r_v( ubyte value );
    template<unsigned X, unsigned Y>        int     OpCMP_r_r();
    template<unsigned X, RegisterPair XY>   int     OpCMP_r_rr();
    template<unsigned X>                    int     OpCMP_r_n();
    template<unsigned X>                    int     OpINC_r();
                                            int     OpINC_HL();
    template<unsigned X>                    int     OpDEC_r();
                                            int     OpDEC_HL();
                                            int     OpDAA();
                                            int     OpCPL();
    
    // 16 bit arithmetic/logic
    template<RegisterPair XY>               int     OpADD_HL_rr();
                                            int     OpADD_HL_SP();
                                            int     OpADD_SP_n();
    template<RegisterPair XY>               int     OpINC_rr();
                                            int     OpINC_SP();
    template<RegisterPair XY>               int     OpDEC_rr();
                                            int     OpDEC_SP();
                                            
    // Shift/rotate/swap
    template<unsigned X>                    int     OpSWAP_r();
                                            int     OpSWAP_HL();
                                            int     OpRLCA();
                                            int     OpRLA();
                                            int     OpRRCA();
                                            int     OpRRA();
    template<unsigned X>                    int     OpRLC_r();
                                            int     OpRLC_HL();
    template<unsigned X>                    int     OpRL_r();
                                            int     OpRL_HL();
    template<unsigned X>                    int     OpRRC_r();
                                            int     OpRRC_HL();
    template<unsigned X>                    int     OpRR_r();
                                            int     OpRR_HL();
    template<unsigned X>                    int     OpSLA_r();
                                            int     OpSLA_HL();
    template<unsigned X>                    int     OpSRA_r();
                                            int     OpSRA_HL();
    template<unsigned X>                    int     OpSRL_r();
                                            int     OpSRL_HL();

    // Bit manipulation
    template<unsigned b, unsigned X>        int     OpBIT_r();
    template<unsigned b>                    int     OpBIT_HL();
    template<unsigned b, unsigned X>        int     OpSET_r();
    template<unsigned b>                    int     OpSET_HL();
    template<unsigned b, unsigned X>        int     OpRES_r();
    template<unsigned b>                    int     OpRES_HL();
    
    // CPU control
                                            int     OpCCF();
                                            int     OpSCF();
                                            int     OpNOP();
                                            int     OpHALT();
                                            int     OpSTOP();
                                            int     OpDI();
                                            int     OpEI();

    // Jump/branch commands
                                            int     OpJP_nn();
                                            int     OpJP_NZ_nn();
                                            int     OpJP_Z_nn();
                                            int     OpJP_NC_nn();
                                            int     OpJP_C_nn();
                                            int     OpJP_HL();
                                            int     OpJR_n();
                                            int     OpJR_NZ_n();
                                            int     OpJR_Z_n();
                                            int     OpJR_NC_n();
                                            int     OpJR_C_n();
                                            int     OpCALL_nn();
                                            int     OpCALL_NZ_nn();
                                            int     OpCALL_Z_nn();
                                            int     OpCALL_NC_nn();
                                            int     OpCALL_C_nn();
    template<unsigned ADDR>                 int     OpRST_nn();
                                            int     OpRET_nn();
                                            int     OpRET_NZ_nn();
                                            int     OpRET_Z_nn();
                                            int     OpRET_NC_nn();
                                            int     OpRET_C_nn();
                                            int     OpRETI_nn();

    // Misc
                                            int     OpExecuteExtOp();
    

private:
    GBCpuOpcodeHandler  m_OpcodeHandlers[ 256 ];
    GBCpuOpcodeHandler  m_OpcodeExtHandlers[ 256 ];

    GBMem*              m_pMem;
    GBTimer*            m_pTimer;
    bool                m_bInitialized;

    // Registers
    ubyte               m_Registers[ 8 ];

    // Control & flow
    uint16              m_PC;
    uint16              m_SP;
    bool                m_bHalt;
    bool                m_bStop;
    bool                m_bIME;

    // Debug data
    uint16              m_DebugPC;
    CpuState            m_u16History[ DebugHistorySize ];
    uint32              m_u32HistoryIndex;
};

#endif
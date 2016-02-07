#ifndef GBEMU_GBCPU_UNIT_TEST_H
#define GBEMU_GBCPU_UNIT_TEST_H

//====================================================================================================
// Filename:    GBCpuUnitTest.h
// Created by:  Jeff Padgham
// Description: Unit test designed to validate all of the opcodes of the emulated GB CPU.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "emutypes.h"

//====================================================================================================
// Class
//====================================================================================================

class GBCpuUnitTest
{
    friend class GBCpu;
    
public:
    // Constructor / destructor
    GBCpuUnitTest();
    ~GBCpuUnitTest();

    void    ExecuteTests();

private:
    void    TestOpLD_r_n();
};

#endif
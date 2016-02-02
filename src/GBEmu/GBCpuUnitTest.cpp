//====================================================================================================
// Filename:    GBCpuUnitTest.cpp
// Created by:  Jeff Padgham
// Description: Unit test designed to validate all of the opcodes of the emulated GB CPU.
//====================================================================================================

//====================================================================================================
// Includes
//====================================================================================================

#include "GBCpuUnitTest.h"

#include "GBCpu.h"
#include "GBMem.h"

//====================================================================================================
// Class
//====================================================================================================

GBCpuUnitTest::GBCpuUnitTest()
{
}
	
//----------------------------------------------------------------------------------------------------
GBCpuUnitTest::~GBCpuUnitTest()
{
}

//----------------------------------------------------------------------------------------------------
void GBCpuUnitTest::ExecuteTests()
{
	TestOpLD_r_n();
}

//----------------------------------------------------------------------------------------------------
void GBCpuUnitTest::TestOpLD_r_n()
{
}
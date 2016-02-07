#include <stdio.h>

#include "CLog.h"
#include "GBEmulator.h"

int main( int argc, char* argv[] )
{
    Log()->Initialize();

    GBEmulator oEmulator;

    oEmulator.Initialize();
    oEmulator.Run();
    oEmulator.Terminate();

    Log()->Terminate();

    return 0;
}
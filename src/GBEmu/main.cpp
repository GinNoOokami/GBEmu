#include <stdio.h>

#include "CLog.h"
#include "GBEmulator.h"

int main( int argc, char* argv[] )
{
    Log()->Initialize();

    GBEmulator oEmulator;
    oEmulator.Run();

    Log()->Terminate();

    return 0;
}
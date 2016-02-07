# GBEmu
A Gameboy emulator written in C++ with SDL for educational purposes. The emulator is in a state which allows most games to be played (without sound).

## Usage
Currently the only way to load a rom is to create a file called "debug_load.ini" containing a relative file path to a Gameboy Rom. This will be improved in the future to a more user-friendly method.

## Features
Currently implemented:
 - All CPU opcodes
 - LCD Controller
 - Basic cartridge loading (with ini file)
 - Support for 3/5 memory bank controllers
 - Input
 
## TODO
 - User interface or menu to allow easier cartridge loading
 - Sound
 - Remaining memory bank controllers
 - GBC support

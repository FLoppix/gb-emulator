#include <iostream>

#include "Component/CPU.h"
#include "Component/Memory.h"
#include "Component/ROMReader.h"
#include "Component/Config.h"

using namespace std;

int main( int argc, const char* argv[] )
{
	
	std::string romName;
	if( argc == 3 && string(argv[2]) == "d") {
		Config::enableDebug();
		romName = string(argv[1]);
	} else if (argc == 2 && argv[1] != NULL ) {
	 	romName = string(argv[1]);
	} else {
		printf("Please specify a valid ROM!\n");
		exit(0);
	}


	cout << "Starting Gameboy Emulator" << endl;
	Memory* mem = new Memory();
	CPU cpu(mem);

	ROMReader::setMemory(mem);
	cout << "Copying Boot ROM..." << endl;
	ROMReader::copy("../ROM/GB_ROM.bin");

	cout << "Loading Cartridge..." << endl;
	ROMReader::load(romName);
	//r.load("../ROM/Wario Land - Super Mario Land 3 (World).gb");
	//r.load("../ROM/Motorcross Maniacs.gb");	
	//r.load("../ROM/Super Mario Land 2 - 6 Golden Coins (USA, Europe).gb");
	//r.load("../ROM/Flipull.gb");
	//r.load("../ROM/Alleyway (World).gb");
	//r.load("../ROM/World Bowling.gb");
	//r.load("../ROM/Minesweeper.gb");
	//r.load("../ROM/Dr. Mario.gb");	
	//r.load("../ROM/Volley Fire.gb");
	//r.load("../ROM/Space Invaders (Europe).gb");
	//r.load("../ROM/Tetris.gb");
	//r.load("../ROM/cpu_instrs.gb");
	//r.load("../ROM/testrom/01-special.gb");
	//r.load("../ROM/testrom/02-interrupts.gb");
	//r.load("../ROM/testrom/03-op sp,hl.gb");
	//r.load("../ROM/testrom/04-op r,imm.gb");
	//r.load("../ROM/testrom/05-op rp.gb");
	//r.load("../ROM/testrom/06-ld r,r.gb");
	//r.load("../ROM/testrom/07-jr,jp,call,ret,rst.gb");
	//r.load("../ROM/testrom/08-misc instrs.gb");
	//r.load("../ROM/testrom/09-op r,r.gb");
	//r.load("../ROM/testrom/10-bit ops.gb");
	//r.load("../ROM/testrom/11-op a,(hl).gb");


	if( argc == 2 && string(argv[1]) == "c") {
		Config::enableDebug();
		for(;;) {
			cpu.disassemble();
		}
		exit(0);
	}

	if( argc == 2 && string(argv[1]) == "w") {
		for(int i = 0; i < 10000; i++) {
			cpu.disassemble();
		}
		exit(0);
	}
	
	cpu.run();
}
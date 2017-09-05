#include "../Component/CPU.h"
#include "../Component/Interrupts.h"
#include "../Component/Config.h"
#include "../Component/Output.h"
#include "../Component/Joypad.h"
#include "../Component/ROMReader.h"

#include <regex>
#include <chrono>
#include <thread>

#define FLAGS_ZERO (1 << 7)
#define FLAGS_NEGATIVE (1 << 6)
#define FLAGS_HALFCARRY (1 << 5)
#define FLAGS_CARRY (1 << 4)

using std::cout;
using std::cin;
using std::endl;

using std::regex;
using std::sregex_iterator;

/* Constructor */
CPU::CPU(Memory* m) {
	mem = m;
	timer = new Timer(mem);
	gpu = new GPU(mem);
	interruptsEnabled = false;
	isHalt = false;
	reg.pc = 0;
	reg.f = 0;
	ext = 0;

	globalTicks = 0;

	Joypad::mem = m;
}


/*  Util Methods */
void CPU::run() {
	mem->initialize();
	readRomHeader();

	// Try to load savegame
	string romName = getRomName();
	if(ROMReader::tryToLoadSavegame(string("../ROM/" + romName + string(".sav")))) {
		cout << "> Found savegame for " + romName << endl;
	}

	start = std::chrono::steady_clock::now();
	while(true) {
		exec();

		// Dump savegame
		if(Joypad::buttons[static_cast<int>(Joypad::Button::Y)]) {
			string romName = getRomName();
			ROMReader::dumpSavegame(string("../ROM/" + romName + string(".sav")));
			cout << "> Stored current RAM to '" + romName + ".sav'" << endl;

			while(Joypad::buttons[static_cast<int>(Joypad::Button::Y)]) {
				gpu->handleEvents();
			}
		}		
	}
}

void CPU::exec() {
	if (isHalt) {
		debug();
		if (isAnyInterruptTriggered()) {
			isHalt = false;
		}
		timer->update(4);
		gpu->update(4);

		globalTicks += 4;
		wait();

		return;
	}

	if (!ext) {
		manageMemory();
		handleInterrupts();	
	}
	
	uint8_t opcode = mem->read_8u(reg.pc);
	dumpInstr(opcode);

	uint8_t ticks;
	if(ext == 1) {
		ticks = (this->*ext_instruction[opcode].function)();
		debug();
		ticks = ticks + ext_instruction[opcode].ticks;
		reg.pc += ext_instruction[opcode].length;
		ext = 0;
	}
	else {
		ticks = (this->*instruction[opcode].function)();
		debug();
		ticks = ticks + instruction[opcode].ticks;
		reg.pc += instruction[opcode].length;
	}
	timer->update(ticks);
	gpu->update(ticks);

	globalTicks += ticks;
	wait();
}

void CPU::disassemble() {
	mem->initialize();
	uint8_t opcode = mem->read_8u(reg.pc);
	dumpInstr(opcode);
	if(ext == 1) {
		debug();
		reg.pc += ext_instruction[opcode].size;
		ext = 0;
	}
	else {
		debug();
		reg.pc += instruction[opcode].size;
	}
}

void CPU::wait() {
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::chrono::nanoseconds t = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start);

	while(t < std::chrono::nanoseconds(globalTicks * 238)) {  // Orginal time would be 238
		now = std::chrono::steady_clock::now();
		t = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start);
	}
}

/* Interrupt Methods */
/* Enable Global Interrupts */
void CPU::enableInterrupts() {
	interruptsEnabled = true;
}

/* Disable Global Interrupts */
void CPU::disableInterrupts() {
	interruptsEnabled = false;
}

/* Enable Request Interrupts */
void CPU::enableRequestInterrupt(uint8_t type) {
	uint8_t current = mem->read_8u(INTERRUPT_ENABLE_REGISTER);
	mem->write_8u(INTERRUPT_ENABLE_REGISTER, (current | type));
	printf("current | type: 0x%02X\n", (current | type));
}

/* Disable Request Interrupts */
void CPU::disableRequestInterrupt(uint8_t type) {
	uint8_t current = mem->read_8u(INTERRUPT_ENABLE_REGISTER);
	mem->write_8u(INTERRUPT_ENABLE_REGISTER, (current & ~type));
}

/* Trigger Request Interrupts */
void CPU::triggerRequestInterrupt(uint8_t type) {
	uint8_t current = mem->read_8u(INTERRUPT_REQUEST_REGISTER);
	mem->write_8u(INTERRUPT_REQUEST_REGISTER, (current | type));
}

/* Reset Request Interrupts */
void CPU::resetRequestInterrupt(uint8_t type) {
	uint8_t current = mem->read_8u(INTERRUPT_REQUEST_REGISTER);
	mem->write_8u(INTERRUPT_REQUEST_REGISTER, (current & ~type));
}

bool CPU::isGlobalInterrupt() {
	return interruptsEnabled;
}

bool CPU::isRequestInterrupt(uint8_t type) {
	return isRequestInterruptEnabled(type) && isRequestInterruptTriggered(type); 
}

bool CPU::isRequestInterruptEnabled(uint8_t type) { 
	uint8_t current = mem->read_8u(INTERRUPT_ENABLE_REGISTER);
	return (current & type);
}

bool CPU::isRequestInterruptTriggered(uint8_t type) {
	uint8_t current = mem->read_8u(INTERRUPT_REQUEST_REGISTER);
	return (current & type);
}

bool CPU::isAnyInterrupt() {
	if( isGlobalInterrupt()) {
		return isRequestInterrupt(INTERRUPT_VBLANK) || isRequestInterrupt(INTERRUPT_LCDSTAT) || isRequestInterrupt(INTERRUPT_TIMER) || isRequestInterrupt(INTERRUPT_SERIAL) || isRequestInterrupt(INTERRUPT_JOYPAD);
	}
	return false;
}

bool CPU::isAnyInterruptTriggered() {
	return isRequestInterruptTriggered(INTERRUPT_VBLANK) || isRequestInterruptTriggered(INTERRUPT_LCDSTAT) || isRequestInterruptTriggered(INTERRUPT_TIMER) || isRequestInterruptTriggered(INTERRUPT_SERIAL) || isRequestInterruptTriggered(INTERRUPT_JOYPAD);
}

void CPU::handleInterrupts() {
	if ( !isGlobalInterrupt()) {
		return;
	}

	if (isRequestInterrupt(INTERRUPT_VBLANK)) {
		disableInterrupts();
		resetRequestInterrupt(INTERRUPT_VBLANK);
		push(reg.pc);
		reg.pc = OFFSET_VBLANK;
	}
	else if (isRequestInterrupt(INTERRUPT_LCDSTAT)) {
		disableInterrupts();
		resetRequestInterrupt(INTERRUPT_LCDSTAT);
		push(reg.pc);
		reg.pc = OFFSET_LCDSTAT;
	}
	else if (isRequestInterrupt(INTERRUPT_TIMER)) {
		disableInterrupts();
		resetRequestInterrupt(INTERRUPT_TIMER);
		push(reg.pc);
		reg.pc = OFFSET_TIMER;
	}
	else if (isRequestInterrupt(INTERRUPT_SERIAL)) {
		disableInterrupts();
		resetRequestInterrupt(INTERRUPT_SERIAL);
		push(reg.pc);
		reg.pc = OFFSET_SERIAL;
	}
	else if (isRequestInterrupt(INTERRUPT_JOYPAD)) {
		disableInterrupts();
		resetRequestInterrupt(INTERRUPT_JOYPAD);
		push(reg.pc);
		reg.pc = OFFSET_JOYPAD;
	}
}

void CPU::manageMemory() {
	mem->remapUnit();
}

/* Register Methods */
void CPU::setZeroFlag(bool zeroFlag) {
	if (zeroFlag) {
		reg.f |= (1 << 7);
	} else {
		reg.f &= ~(1 << 7);
	}
	
}

void CPU::setSubtractFlag(bool subtractFlag) {
	if (subtractFlag) {
		reg.f |= (1 << 6);
	} else {
		reg.f &= ~(1 << 6);
	}
}

void CPU::setHalfCarryFlag(bool halfCarryFlag) {
	if (halfCarryFlag) {
		reg.f |= (1 << 5);
	} else {
		reg.f &= ~(1 << 5);
	}
	
}

void CPU::setCarryFlag(bool carryFlag) {
	if (carryFlag) {
		reg.f |= (1 << 4);
	} else {
		reg.f &= ~(1 << 4);
	}
	
}

bool CPU::isZeroFlag() {
	return (reg.f >> 7) & 1;
}

bool CPU::isSubstractFlag() {
	return (reg.f >> 6) & 1;
}

bool CPU::isHalfCarryFlag() {
	return (reg.f >> 5) & 1;
}

bool CPU::isCarryFlag() {
	return (reg.f >> 4) & 1;
}


/* Debug Methods */
void CPU::debug() {
	if(Config::isDebug() && !Config::isWaiting(reg.pc)) {
		string input;
		for(;;) {
			getline(cin, input);
			if (input.empty()) {
				// Clears current line and move cursor one line up in terminal
				printf("\33[2K\033[A");
				return;
			}
			if (input.substr(0,3).compare("con") == 0) {
				printf("\33[2K\033[A\33[2K");
				Config::disableDebug();
				return;
			}
			if (input.substr(0,1).compare("r") == 0) {
				printf("\33[2K\033[A\33[2K");
				dump();
			}
			if (input.substr(0,1).compare("s") == 0) {
				printf("\33[2K\033[A\33[2K");
				stackdump();
			}
			if (input.substr(0,2).compare("mm") == 0) {
				printf("\33[2K\033[A\33[2K");
				regex re("([0-9A-Fa-fxX]+)");
				sregex_iterator it(input.begin(), input.end(), re);
				sregex_iterator nit;

				if (it->size() == 2) {
					uint16_t start = strtoul(it->str().c_str(), NULL, 16);
					it++;
					uint16_t end;
					
					if (it != nit) {
						end = strtoul(it->str().c_str(), NULL, 16);
					}
					else {
						end = start;
					}

					end = (end == 0) ? start : end;
					
					memdump(start, end); 					
				}
				else {
					printf(RED "Unknown format. Please enter mm {start_addr} {end_addr}\n" DEFAULT);
				}
			}
			if (input.substr(0,2).compare("mi") == 0) {
				printf("\33[2K\033[A\33[2K");
				uint8_t opcode = mem->read_8u(reg.pc);
				if(!ext && instruction[opcode].size == 3) {
					uint16_t start = mem->read_16u(reg.pc+1);

					memdump(start, start);
				}
				else {
					printf(YELLOW "Not available for this instruction!\n" DEFAULT);
				}
			}
			if (input.substr(0,2).compare("mr") == 0) {
				printf("\33[2K\033[A\33[2K");

				if (input.length() >= 5) {
					if (input.substr(3,2).compare("af") == 0) {
						memdumpr(reg.af, "AF");
					}
					else if (input.substr(3,2).compare("bc") == 0) {
						memdumpr(reg.bc, "BC");
					}
					else if (input.substr(3,2).compare("de") == 0) {
						memdumpr(reg.de, "DE");
					}
					else if (input.substr(3,2).compare("hl") == 0) {
						memdumpr(reg.hl, "HL");
					}
					else {
						printf(RED "Unknown register. Only 16bit registers are allowed. Please enter mr {reg}\n" DEFAULT);
					}
				}
				else {
					printf(RED "Command wrong: Please enter 16bit register: mr {reg}\n" DEFAULT);
				}
			}
			if (input.substr(0,2).compare("mc") == 0) {
				input = input.substr(2);
				printf("\33[2K\033[A\33[2K");
				regex re("([0-9A-Fa-fxX]+)");
				sregex_iterator it(input.begin(), input.end(), re);
				if (it->size() == 2) {
					uint32_t start = strtoul(it->str().c_str(), NULL, 16);
					it++;
					uint32_t end = strtoul(it->str().c_str(), NULL, 16);

					end = (end == 0) ? start : end;
					
					memdumpc(start, end);
				}
				else {
					printf(RED "Unknown format. Please enter mm {start_addr} {end_addr}\n" DEFAULT);
				}
			}
			if (input.substr(0,2).compare("gt") == 0) {
				printf("\33[2K\033[A\33[2K");
				regex re("([0-9A-Fa-fxX]+)");
				sregex_iterator it(input.begin(), input.end(), re);
				uint16_t w = strtoul(it->str().c_str(), NULL, 16);

				Config::waitForPC(w, false);
				return;
			}
			if (input.substr(0,2).compare("gg") == 0) {
				printf("\33[2K\033[A\33[2K");
				regex re("([0-9A-Fa-fxX]+)");
				sregex_iterator it(input.begin(), input.end(), re);
				uint16_t w = strtoul(it->str().c_str(), NULL, 16);

				Config::waitForPC(w, true);
				return;
			}
			if (input.substr(0,4).compare("exit") == 0) {
				exit(0);
			}
		}
	}
}

void CPU::dumpInstr(uint16_t opcode) {
	if (!(Config::isDebug() && !Config::isWaiting(reg.pc))) {
		return;
	}

	printf(RED "[PC: 0x%04X] " DEFAULT, reg.pc);
	if(ext == 1) {
		printf(ext_instruction[opcode].name, mem->read_8u(reg.pc+1));
	}
	else {
		switch(instruction[opcode].size) {
			case 0:
				printf("%s", instruction[opcode].name);
				break;
			case 1:
				printf("%s", instruction[opcode].name);
				break;
			case 2:
				printf(instruction[opcode].name, mem->read_8u(reg.pc+1));
				break;
			case 3:
				printf(instruction[opcode].name, mem->read_16u(reg.pc+1));
				break;
			default:
				printf(RED "Error: No valid instruction!" DEFAULT);
				break;
		}
	}
	cout << endl;
}

void CPU::dump() {
	printf(GRAY "\n----------------- REGISTER DUMP -----------------\n\n" DEFAULT);
	printf(CRED "[PC]" DEFAULT " 0x%04X \t" CGREEN "[SP]" DEFAULT " 0x%04X\n\n", reg.pc, reg.sp);

	printf(CLBLUE "[A]" DEFAULT " %d   \t" CLBLUE "[F]" DEFAULT " %d   \t" CLBLUE "[AF]" DEFAULT " 0x%04X\n", reg.a, reg.f, (uint16_t) reg.af);
	printf(CLBLUE "[B]" DEFAULT " %d   \t" CLBLUE "[C]" DEFAULT " %d   \t" CLBLUE "[BC]" DEFAULT " 0x%04X\n", reg.b, reg.c, (uint16_t) reg.bc);
	printf(CLBLUE "[D]" DEFAULT " %d   \t" CLBLUE "[E]" DEFAULT " %d   \t" CLBLUE "[DE]" DEFAULT " 0x%04X\n", reg.d, reg.e, (uint16_t) reg.de);
	printf(CLBLUE "[H]" DEFAULT " %d   \t" CLBLUE "[L]" DEFAULT " %d   \t" CLBLUE "[HL]" DEFAULT " 0x%04X\n\n", reg.h, reg.l, (uint16_t) reg.hl);

	printf(CLYELLOW "STATUS" DEFAULT "\t\t" CLMAGENTA "INTERRUPT\n" DEFAULT);
	printf("%d %d %d %d\t\t%d " GRAY "|" DEFAULT " %s %s %s %s %s\n", isZeroFlag(), isSubstractFlag(), isHalfCarryFlag(), isCarryFlag(), isGlobalInterrupt(), printRequestInterrupt(INTERRUPT_VBLANK), printRequestInterrupt(INTERRUPT_LCDSTAT), printRequestInterrupt(INTERRUPT_TIMER), printRequestInterrupt(INTERRUPT_SERIAL), printRequestInterrupt(INTERRUPT_JOYPAD));
	//printf(DGRAY "- - - -\t\t-   - - - - -\n" DEFAULT);
	printf(CYELLOW "Z N H C" DEFAULT "\t\t" CMAGENTA "G " GRAY "|" CMAGENTA " V L T S J\n" DEFAULT);

	printf(GRAY "\n-------------------------------------------------\n\n" DEFAULT);
}

const char* CPU::printRequestInterrupt(uint8_t type) {
	if (isRequestInterruptEnabled(type) && !isRequestInterruptTriggered(type)) {
		return CGREEN "1" DEFAULT;
	}
	else if (isRequestInterruptEnabled(type) && isRequestInterruptTriggered(type)) {
		return CLGREEN "!" DEFAULT;
	}
	else if (!isRequestInterruptEnabled(type) && isRequestInterruptTriggered(type)) {
		return CORANGE "*" DEFAULT;
	}
	else {
		return CRED "0" DEFAULT;
	}
}

void CPU::memdump(uint16_t start, uint16_t end) {
	printf(GRAY "\n------------------ MEMORY DUMP ------------------\n\n" DEFAULT);
	printf(" Start: " LYELLOW "0x%04X" DEFAULT " | End: " LYELLOW "0x%04X" DEFAULT "\n\n", start, end);
	for(uint16_t i = start; i <= end; i++) {
		printf(CYAN "[0x%04X]" DEFAULT " 0x%02X   \t" CYAN "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", i, mem->read_8u(i), i, (i+1), mem->read_16u(i));
		
		// prevent uint16_t overflow
		if (i == 0xffff) {
			break;
		}
	}
	printf(GRAY "\n-------------------------------------------------\n\n" DEFAULT);
}

void CPU::memdumpr(uint16_t value, string name) {
	printf(GRAY "\n------------------ MEMORY DUMP ------------------\n\n" DEFAULT);
	uint8_t opcode = mem->read_8u(reg.pc);
	string instr = instruction[opcode].name;
	if (instr.substr(0, 3).compare("LDD") == 0) {
		printf(BLUE "[%s]" DEFAULT " 0x%04X \t" GREEN "[%s']" DEFAULT " 0x%04X\n\n", name.c_str(), value, name.c_str(), value+1);
		
		printf(CYAN "[0x%04X]" DEFAULT " 0x%02X   \t" CYAN "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", (value-1), mem->read_8u(value-1), (value-1), value, mem->read_16u(value-1));
		printf(BLUE "[0x%04X]" DEFAULT " 0x%02X   \t" BLUE "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", value, mem->read_8u(value), value, (value+1), mem->read_16u(value));
		printf(GREEN "[0x%04X]" DEFAULT " 0x%02X   \t" GREEN "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", (value+1), mem->read_8u(value+1), (value+1), (value+2), mem->read_16u(value+1));
		printf(CYAN "[0x%04X]" DEFAULT " 0x%02X   \t" CYAN "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", (value+2), mem->read_8u(value+2), (value+2), (value+3), mem->read_16u(value+2));
	}
	else if (instr.substr(0, 3).compare("LDI") == 0) {
		printf(BLUE "[%s]" DEFAULT " 0x%04X \t" GREEN "[%s']" DEFAULT " 0x%04X\n\n", name.c_str(), value, name.c_str(), value-1);
		
		printf(CYAN "[0x%04X]" DEFAULT " 0x%02X   \t" CYAN "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", (value-2), mem->read_8u(value-2), (value-2), (value-1), mem->read_16u(value-2));
		printf(GREEN "[0x%04X]" DEFAULT " 0x%02X   \t" GREEN "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", (value-1), mem->read_8u(value-1), (value-1), value, mem->read_16u(value-1));
		printf(BLUE "[0x%04X]" DEFAULT " 0x%02X   \t" BLUE "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", value, mem->read_8u(value), value, (value+1), mem->read_16u(value));
		printf(CYAN "[0x%04X]" DEFAULT " 0x%02X   \t" CYAN "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", (value+1), mem->read_8u(value+1), (value+1), (value+2), mem->read_16u(value+1));
	}
	else {
		printf(GREEN "[%s]" DEFAULT " 0x%04X \n\n", name.c_str(), value);
		
		printf(CYAN "[0x%04X]" DEFAULT " 0x%02X   \t" CYAN "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", (value-1), mem->read_8u(value-1), (value-1), value, mem->read_16u(value-1));
		printf(GREEN "[0x%04X]" DEFAULT " 0x%02X   \t" BLUE "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", value, mem->read_8u(value), value, (value+1), mem->read_16u(value));
		printf(CYAN "[0x%04X]" DEFAULT " 0x%02X   \t" CYAN "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", (value+1), mem->read_8u(value+1), (value+1), (value+2), mem->read_16u(value+1));
	}
	
	printf(GRAY "\n-------------------------------------------------\n\n" DEFAULT);
}

void CPU::memdumpc(uint32_t start, uint32_t end) {
	printf(GRAY "\n----------------- CARTRIDGE DUMP ----------------\n\n" DEFAULT);
	printf(" Start: " LYELLOW "0x%04X" DEFAULT " | End: " LYELLOW "0x%04X" DEFAULT "\n\n", start, end);
	for(uint32_t i = start; i <= end; i++) {
		printf(CYAN "[0x%04X]" DEFAULT " 0x%02X   \t" CYAN "[0x%04X + 0x%04X]" DEFAULT " 0x%04X\n", i, mem->readCartridge8u(i), i, (i+1), mem->readCartridge16u(i));
	}
	printf(GRAY "\n-------------------------------------------------\n\n" DEFAULT);
}

void CPU::stackdump() {
	printf(GRAY "\n------------------ STACK DUMP -------------------\n\n" DEFAULT);
	printf(GREEN "[SP]" DEFAULT " 0x%04X\n\n", reg.sp);
	if (reg.sp < 0xF000) {
		printf("Stack too large! Something must be wrong! Skip!\n");
	}
	for(int i = reg.sp; i <= 0xFFFE; i = i+0x2) {
		printf(LGREEN "[0x%04X] " DEFAULT "0x%04X\n", i, mem->read_16u(i));
	}

	printf(GRAY "\n-------------------------------------------------\n\n" DEFAULT);
}

void CPU::readRomHeader() {
	string type;
	uint8_t value = mem->read_8s(0x147);

	switch(value) {
		case 0x0:
			type = "ROM ONLY";
			break;
		case 0x1:
			type = "MBC1";
			break;
		case 0x2: 
			type = "MBC1+RAM";
			break;
		case 0x3:
			type = "MBC1+RAM+BATTERY";
			break;
		case 0x5:
			type = "MBC2";
			break;
		case 0x6:
			type = "MBC2+BATTERY";
			break;
		case 0x8:
			type = "ROM+RAM";
			break;
		case 0x9:
			type = "ROM+RAM+BATTERY";
			break;
		case 0xB:
			type = "MMM01";
			break;
		case 0xC:
			type = "MMM01+RAM";
			break;
		case 0xD:
			type = "MMM01+RAM+BATTERY";
			break;
		case 0xF:
			type = "MBC3+TIMER+BATTERY";
			break;
		case 0x10:
			type = "MBC3+TIMER+RAM+BATTERY";
			break;
		case 0x11:
			type = "MBC3";
			break;
		case 0x12:
			type = "MBC3+RAM";
			break;
		case 0x13:
			type = "MBC3+RAM+BATTERY";
			break;
		case 0x15:
			type = "MBC4";
			break;
		case 0x16:
			type = "MBC4+RAM";
			break;
		case 0x17:
			type = "MBC4+RAM+BATTERY";
			break;
		case 0x19:
			type = "MBC5";
			break;
		case 0x1A:
			type = "MBC5+RAM";
			break;
		case 0x1B:
			type = "MBC5+RAM+BATTERY";
			break;
		case 0x1C:
			type = "MBC5+RUMBLE";
			break;
		case 0x1D:
			type = "MBC5+RUMBLE+RAM";
			break;
		case 0x1E:
			type = "MBC5+RUMBLE+RAM+BATTERY";
			break;
		case 0x20:
			type = "MBC6";
			break;
		case 0x22:
			type = "MBC7+SENSOR+RUMBLE+RAM+BATTERY";
			break;
		case 0xFC:
			type = "POCKET CAMERA";
			break;
		case 0xFD:
			type = "BANDAI TAMA5";
			break;
		case 0xFE:
			type = "HuC3";
			break;
		case 0xFF:
			type = "HuC1+RAM+BATTERY";
			break;
		default:
			type = "UNKNOWN";
			break;
	} 
	string sgbFlag;
	value = mem->read_8s(0x146);
	switch (value) {
		case 0x0:
			sgbFlag = "No SGB functions (Normal Gameboy or CGB only game)";
			break;
		case 0x3:
			sgbFlag = "Game supports SGB functions";
			break;
		default:
			sgbFlag = "Unknown";
			break;
	}

	string romSize;
	value = mem->read_8u(0x148);
	switch (value) {
		case 0x0:
			romSize = "32KByte (no ROM banking)";
			break;
		case 0x01:
			romSize = "64KByte (4 banks)";
			break;
		case 0x2:
			romSize = "128KByte (8 banks)";
			break;
		case 0x3:
			romSize = "256KByte (16 banks)";
			break;
		case 0x4:
			romSize = "512KByte (32 banks)";
			break;
		case 0x5:
			romSize = "1MByte (64 banks)  - only 63 banks used by MBC1";
			break;
		case 0x6:
			romSize = "2MByte (128 banks) - only 125 banks used by MBC1";
			break;
		case 0x7:
			romSize = "4MByte (256 banks";
			break;
		case 0x8:
			romSize = "8MByte (512 banks)";
			break;
		case 0x52:
			romSize = "1.1MByte (72 banks)";
			break;
		case 0x53:
			romSize = "1.2MByte (80 banks)";
			break;
		case 0x54:
			romSize = "54h - 1.5MByte (96 banks)";
			break;
		default: 
			romSize = "Unknown";
			break;
	}

	value = mem->read_8u(0x149);
	string ramSize;
	switch (value) {
		case 0x0:
			ramSize = "None";
			break;
		case 0x01:
			ramSize = "2 KBytes";
			break;
		case 0x2:
			ramSize = "8 Kbytes";
			break;
		case 0x3:
			ramSize = "32 KBytes (4 banks of 8KBytes each)";
		case 0x4:
			ramSize = "128 KBytes (16 banks of 8KBytes each)";
			break;
		case 0x5:
			ramSize = "64 KBytes (8 banks of 8KBytes each)";
			break;
	}

	string romName = getRomName();

	printf(GRAY "\n----------------- ROM HEADER -----------------\n\n" DEFAULT);

	printf("ROM Name:      \t %s \n", romName.c_str());
	printf("Cartridge Type:\t %s \n", type.c_str());
	printf("SGB Flag:      \t %s \n", sgbFlag.c_str());
	printf("ROM Size:      \t %s \n", romSize.c_str());
	printf("RAM Size:      \t %s \n", ramSize.c_str());

	printf(GRAY "\n----------------------------------------------\n\n" DEFAULT);

}

string CPU::getRomName() {
	string romName;
	for(uint i = 0x134; i <= 0x143U; i++) {
		char c = mem->read_8u(i);
		if(c != '\0') {
			romName += c;
		} else {
			break;
		}
	}
	return romName;
}


/* Instruction Methods */
uint8_t CPU::nop() {
	return 0;
}

uint8_t CPU::ld_bc_d16() {
	reg.bc = mem->read_16u(reg.pc+1);
	return 0;
}

uint8_t CPU::ld_bcp_a() {
	mem->write_8u(reg.bc, reg.a);
	return 0;
}

uint8_t CPU::inc_bc() {
	inc16(&reg.bc);
	return 0;
}

uint8_t CPU::inc_b() {
	inc8(&reg.b);
	return 0;
}

uint8_t CPU::dec_b() {
	dec8(&reg.b);
	return 0;
}

uint8_t CPU::ld_b_d8() {
	reg.b = mem->read_8u(reg.pc + 1);
	return 0;
}

uint8_t CPU::rlca() {
	rlc(&reg.a);
	setZeroFlag(false);

	return 0;
}

uint8_t CPU::ld_a16p_sp() {
	uint16_t addr = mem->read_16u(reg.pc + 1);
	mem->write_16u(addr, reg.sp);
	return 0;
}

uint8_t CPU::add_hl_bc() {
	add16(reg.bc);
	return 0;
}

uint8_t CPU::ld_a_bcp() {
	reg.a = mem->read_8s(reg.bc);
	return 0;
}

uint8_t CPU::dec_bc() {
	reg.bc--;
	return 0;
}

uint8_t CPU::inc_c() {
	inc8(&reg.c);
	return 0;
}

uint8_t CPU::dec_c() {
	dec8(&reg.c);
	return 0;
}

uint8_t CPU::ld_c_d8() {
	reg.c = mem->read_8u(reg.pc + 1);
	return 0;
}

uint8_t CPU::rrca() {
	rrc(&reg.a);
	setZeroFlag(false);

	return 0;
}

uint8_t CPU::stop() {
	printf("STOP at 0x%04x\n", reg.pc);
	while(!Joypad::isAnyButtonPressed()) {
		gpu->handleEvents();
	}
	return 0;
}

uint8_t CPU::ld_de_d16() {
	reg.de = mem->read_16u(reg.pc+1);
	return 0;
}

uint8_t CPU::ld_dep_a() {
	mem->write_8u(reg.de, reg.a);
	return 0;
}

uint8_t CPU::inc_de() {
	inc16(&reg.de);
	return 0;
}

uint8_t CPU::inc_d() {
	inc8(&reg.d);
	return 0;
}

uint8_t CPU::dec_d() {
	dec8(&reg.d);
	return 0;
}

uint8_t CPU::ld_d_d8() {
	reg.d = mem->read_8u(reg.pc + 1);
	return 0;
}

uint8_t CPU::rla() {
	rl(&reg.a);
	setZeroFlag(false);
	return 0;
}

uint8_t CPU::jr_r8() {
	reg.pc += mem->read_8s(reg.pc+1);
	return 0;
}

uint8_t CPU::add_hl_de() {
	add16(reg.de);
	return 0;
}

uint8_t CPU::ld_a_dep() {
	reg.a = mem->read_8u(reg.de);
	return 0;
}

uint8_t CPU::dec_de() {
	reg.de--;
	return 0;
}

uint8_t CPU::inc_e() {
	inc8(&reg.e);
	return 0;
}

uint8_t CPU::dec_e() {
	dec8(&reg.e);
	return 0;
}

uint8_t CPU::ld_e_d8() {
	reg.e = mem->read_8u(reg.pc + 1);
	return 0;
}

uint8_t CPU::rra() {
	rr(&reg.a);
	setZeroFlag(false);

	return 0;
}

uint8_t CPU::jr_nz_r8() {
	if (!isZeroFlag()) {
		reg.pc += mem->read_8s(reg.pc+1);
		return 12;
	}
	return 8;
}

uint8_t CPU::ld_hl_d16() {
	reg.hl = mem->read_16u(reg.pc+1);
	return 0;
}

uint8_t CPU::ldi_hlp_a() {
	mem->write_8u(reg.hl, reg.a);
	reg.hl++;
	return 0;
}

uint8_t CPU::inc_hl() {
	inc16(&reg.hl);
	return 0;
}

uint8_t CPU::inc_h() {
	inc8(&reg.h);
	return 0;
}

uint8_t CPU::dec_h() {
	dec8(&reg.h);
	return 0;
}

uint8_t CPU::ld_h_d8() {
	reg.h = mem->read_8u(reg.pc + 1);
	return 0;
}

uint8_t CPU::daa() {
	uint16_t a = reg.a;

	if (!isSubstractFlag()) {
		if (isHalfCarryFlag() || (a & 0xF) > 9) {
			a += 0x06;
		}

		if (isCarryFlag() || (a > 0x9F)) {
			a += 0x60;
		}
	} else {
		if (isHalfCarryFlag()) {
			a = (a-6) & 0xFF;
		}

		if (isCarryFlag()) {
			a -= 0x60;
		}
	}

	reg.f &= ~(FLAGS_HALFCARRY | FLAGS_ZERO);

	if ((a & 0x100) == 0x100) {
		reg.f |= FLAGS_CARRY;
	}

	a &= 0xFF;

	if (a == 0) {
		reg.f |= FLAGS_ZERO;
	}

	reg.a = a;

	return 0;
}

uint8_t CPU::jr_z_r8() {
	if (isZeroFlag()) {
		reg.pc += mem->read_8s(reg.pc+1);
		return 12;
	}
	return 8;
}

uint8_t CPU::add_hl_hl() {
	add16(reg.hl);
	return 0;
}

uint8_t CPU::ldi_a_hlp() {
	reg.a = mem->read_8u(reg.hl);
	reg.hl++;
	return 0;
}

uint8_t CPU::dec_hl() {
	reg.hl--;
	return 0;
}
uint8_t CPU::inc_l() {
	inc8(&reg.l);
	return 0;
}
uint8_t CPU::dec_l() {
	dec8(&reg.l);
	return 0;
}
uint8_t CPU::ld_l_d8() {
	reg.l = mem->read_8u(reg.pc + 1);
	return 0;
}
uint8_t CPU::cpl() {
	reg.a = ~reg.a; // flip all bits in register A
	setSubtractFlag(true);
	setHalfCarryFlag(true);
	return 0;
}

uint8_t CPU::jr_nc_r8() {
	if (!isCarryFlag()) {
		reg.pc += mem->read_8s(reg.pc+1);
		return 12;
	}
	return 8;
}

uint8_t CPU::ld_sp_d16() {
	reg.sp = mem->read_16u(reg.pc+1);
	return 0;
}

uint8_t CPU::ldd_hlp_a() {
	mem->write_8u(reg.hl, reg.a);
	reg.hl--;
	return 0;
}

uint8_t CPU::inc_sp() {
	inc16(&reg.sp);
	return 0;
}

uint8_t CPU::inc_hlp() {
	uint8_t value = mem->read_8u(reg.hl);

	setHalfCarryFlag((value & 0xF) == 0xF);
	value++;

	mem->write_8u(reg.hl, value);

	setZeroFlag(value == 0);
	setSubtractFlag(false);
	return 0;
}

uint8_t CPU::dec_hlp() {
	uint8_t value = mem->read_8u(reg.hl);

	setHalfCarryFlag((value & 0xF) == 0x0);
	value--;

	mem->write_8u(reg.hl, value);

	setZeroFlag(value == 0);
	setSubtractFlag(true);
	return 0;
}

uint8_t CPU::ld_hlp_d8() {
	mem->write_8u(reg.hl, mem->read_8u(reg.pc + 1));
	return 0;
}

uint8_t CPU::scf() {
	setCarryFlag(true);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	return 0;
}

uint8_t CPU::jr_c_r8() {
	if (isCarryFlag()) {
		reg.pc += mem->read_8s(reg.pc+1);
		return 12;
	}
	return 8;
}

uint8_t CPU::add_hl_sp() {
	add16(reg.sp);
	return 0;
}

uint8_t CPU::ldd_a_hlp() {
	reg.a = mem->read_8u(reg.hl);
	reg.hl--;
	return 0;
}

uint8_t CPU::dec_sp() {
	reg.sp--;
	return 0;
}

uint8_t CPU::inc_a() {
	inc8(&reg.a);
	return 0;
}

uint8_t CPU::dec_a() {
	dec8(&reg.a);
	return 0;
}

uint8_t CPU::ld_a_d8() {
	reg.a = mem->read_8u(reg.pc + 1);
	return 0;
}

uint8_t CPU::ccf() {
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(!isCarryFlag());
	return 0;
}

uint8_t CPU::ld_b_c() {
	reg.b = reg.c;
	return 0;
}

uint8_t CPU::ld_b_d() {
	reg.b = reg.d;
	return 0;
}

uint8_t CPU::ld_b_e() {
	reg.b = reg.e;
	return 0;
}

uint8_t CPU::ld_b_h() {
	reg.b = reg.h;
	return 0;
}

uint8_t CPU::ld_b_l() {
	reg.b = reg.l;
	return 0;
}

uint8_t CPU::ld_b_hlp() {
	reg.b = mem->read_8u(reg.hl);
	return 0;
}

uint8_t CPU::ld_b_a() {
	reg.b = reg.a;
	return 0;
}

uint8_t CPU::ld_c_b() {
	reg.c = reg.b;
	return 0;
}

uint8_t CPU::ld_c_d() {
	reg.c = reg.d;
	return 0;
}

uint8_t CPU::ld_c_e() {
	reg.c = reg.e;
	return 0;
}

uint8_t CPU::ld_c_h() {
	reg.c = reg.h;
	return 0;
}

uint8_t CPU::ld_c_l() {
	reg.c = reg.l;
	return 0;
}

uint8_t CPU::ld_c_hlp() {
	reg.c = mem->read_8u(reg.hl);
	return 0;
}

uint8_t CPU::ld_c_a() {
	reg.c = reg.a;
	return 0;
}

uint8_t CPU::ld_d_b() {
	reg.d = reg.b;
	return 0;
}

uint8_t CPU::ld_d_c() {
	reg.d = reg.c;
	return 0;
}

uint8_t CPU::ld_d_e() {
	reg.d = reg.e;
	return 0;
}

uint8_t CPU::ld_d_h() {
	reg.d = reg.h;
	return 0;
}

uint8_t CPU::ld_d_l() {
	reg.d = reg.l;
	return 0;
}

uint8_t CPU::ld_d_hlp() {
	reg.d = mem->read_8u(reg.hl);
	return 0;
}

uint8_t CPU::ld_d_a() {
	reg.d = reg.a;
	return 0;
}

uint8_t CPU::ld_e_b() {
	reg.e = reg.b;
	return 0;
}

uint8_t CPU::ld_e_c() {
	reg.e = reg.c;
	return 0;
}

uint8_t CPU::ld_e_d() {
	reg.e = reg.d;
	return 0;
}

uint8_t CPU::ld_e_h() {
	reg.e = reg.h;
	return 0;
}

uint8_t CPU::ld_e_l() {
	reg.e = reg.l;
	return 0;
}

uint8_t CPU::ld_e_hlp() {
	reg.e = mem->read_8u(reg.hl);
	return 0;
}

uint8_t CPU::ld_e_a() {
	reg.e = reg.a;
	return 0;
}

uint8_t CPU::ld_h_b() {
	reg.h = reg.b;
	return 0;
}

uint8_t CPU::ld_h_c() {
	reg.h = reg.c;
	return 0;
}

uint8_t CPU::ld_h_d() {
	reg.h = reg.d;
	return 0;
}

uint8_t CPU::ld_h_e() {
	reg.h = reg.e;
	return 0;
}

uint8_t CPU::ld_h_l() {
	reg.h = reg.l;
	return 0;
}

uint8_t CPU::ld_h_hlp() {
	reg.h = mem->read_8u(reg.hl);
	return 0;
}

uint8_t CPU::ld_h_a() {
	reg.h = reg.a;
	return 0;
}

uint8_t CPU::ld_l_b() {
	reg.l = reg.b;
	return 0;
}

uint8_t CPU::ld_l_c() {
	reg.l = reg.c;
	return 0;
}

uint8_t CPU::ld_l_d() {
	reg.l = reg.d;
	return 0;
}

uint8_t CPU::ld_l_e() {
	reg.l = reg.e;
	return 0;
}

uint8_t CPU::ld_l_h() {
	reg.l = reg.h;
	return 0;
}

uint8_t CPU::ld_l_hlp() {
	reg.l = mem->read_8u(reg.hl);
	return 0;
}

uint8_t CPU::ld_l_a() {
	reg.l = reg.a;
	return 0;
}

uint8_t CPU::ld_hlp_b() {
	mem->write_8u(reg.hl, reg.b);
	return 0;
}

uint8_t CPU::ld_hlp_c() {
	mem->write_8u(reg.hl, reg.c);
	return 0;
}

uint8_t CPU::ld_hlp_d() {
	mem->write_8u(reg.hl, reg.d);
	return 0;
}

uint8_t CPU::ld_hlp_e() {
	mem->write_8u(reg.hl, reg.e);
	return 0;
}

uint8_t CPU::ld_hlp_h() {
	mem->write_8u(reg.hl, reg.h);
	return 0;
}

uint8_t CPU::ld_hlp_l() {
	mem->write_8u(reg.hl, reg.l);
	return 0;
}

uint8_t CPU::halt() {
	isHalt = true;
	return 0;
}

uint8_t CPU::ld_hlp_a() {
	mem->write_8u(reg.hl, reg.a);
	return 0;
}

uint8_t CPU::ld_a_b() {
	reg.a = reg.b;
	return 0;
}

uint8_t CPU::ld_a_c() {
	reg.a = reg.c;
	return 0;
}

uint8_t CPU::ld_a_d() {
	reg.a = reg.d;
	return 0;
}

uint8_t CPU::ld_a_e() {
	reg.a = reg.e;
	return 0;
}

uint8_t CPU::ld_a_h() {
	reg.a = reg.h;
	return 0;
}

uint8_t CPU::ld_a_l() {
	reg.a = reg.l;
	return 0;
}

uint8_t CPU::ld_a_hlp() {
	reg.a = mem->read_8u(reg.hl);
	return 0;
}

uint8_t CPU::add_a_b() {
	add8(reg.b);
	return 0;
}

uint8_t CPU::add_a_c() {
	add8(reg.c);
	return 0;
}

uint8_t CPU::add_a_d() {
	add8(reg.d);
	return 0;
}

uint8_t CPU::add_a_e() {
	add8(reg.e);
	return 0;
}

uint8_t CPU::add_a_h() {
	add8(reg.h);
	return 0;
}

uint8_t CPU::add_a_l() {
	add8(reg.l);
	return 0;
}

uint8_t CPU::add_a_hlp() {
	add8(mem->read_8u(reg.hl));
	return 0;
}

uint8_t CPU::add_a_a() {
	add8(reg.a);
	return 0;
}

uint8_t CPU::adc_a_b() {
	adc8(reg.b);
	return 0;
}

uint8_t CPU::adc_a_c() {
	adc8(reg.c);
	return 0;
}

uint8_t CPU::adc_a_d() {
	adc8(reg.d);
	return 0;
}

uint8_t CPU::adc_a_e() {
	adc8(reg.e);
	return 0;
}

uint8_t CPU::adc_a_h() {
	adc8(reg.h);
	return 0;
}

uint8_t CPU::adc_a_l() {
	adc8(reg.l);
	return 0;
}

uint8_t CPU::adc_a_hlp() {
	adc8(mem->read_8u(reg.hl));
	return 0;
}

uint8_t CPU::adc_a_a() {
	adc8(reg.a);
	return 0;
}

uint8_t CPU::sub_a_b() {
	sub8(reg.b);
	return 0;
}

uint8_t CPU::sub_a_c() {
	sub8(reg.c);
	return 0;
}

uint8_t CPU::sub_a_d() {
	sub8(reg.d);
	return 0;
}

uint8_t CPU::sub_a_e() {
	sub8(reg.e);
	return 0;
}

uint8_t CPU::sub_a_h() {
	sub8(reg.h);
	return 0;
}

uint8_t CPU::sub_a_l() {
	sub8(reg.l);
	return 0;
}

uint8_t CPU::sub_a_hlp() {
	sub8(mem->read_8u(reg.hl));
	return 0;
}

uint8_t CPU::sub_a_a() {
	sub8(reg.a);
	return 0;
}

uint8_t CPU::sbc_a_b() {
	sbc8(reg.b);
	return 0;
}

uint8_t CPU::sbc_a_c() {
	sbc8(reg.c);
	return 0;
}

uint8_t CPU::sbc_a_d() {
	sbc8(reg.d);
	return 0;
}

uint8_t CPU::sbc_a_e() {
	sbc8(reg.e);
	return 0;
}

uint8_t CPU::sbc_a_h() {
	sbc8(reg.h);
	return 0;
}

uint8_t CPU::sbc_a_l() {
	sbc8(reg.l);
	return 0;
}

uint8_t CPU::sbc_a_hlp() {
	sbc8(mem->read_8u(reg.hl));
	return 0;
}

uint8_t CPU::sbc_a_a() {
	sbc8(reg.a);
	return 0;
}

uint8_t CPU::and_a_b() {
	and8(reg.b);
	return 0;
}

uint8_t CPU::and_a_c() {
	and8(reg.c);
	return 0;
}

uint8_t CPU::and_a_d() {
	and8(reg.d);
	return 0;
}

uint8_t CPU::and_a_e() {
	and8(reg.e);
	return 0;
}

uint8_t CPU::and_a_h() {
	and8(reg.h);
	return 0;
}

uint8_t CPU::and_a_l() {
	and8(reg.l);
	return 0;
}

uint8_t CPU::and_a_hlp() {
	and8(mem->read_8u(reg.hl));
	return 0;
}

uint8_t CPU::and_a_a() {
	and8(reg.a);
	return 0;
}

uint8_t CPU::xor_a_b() {
	xor8(reg.b);
	return 0;
}

uint8_t CPU::xor_a_c() {
	xor8(reg.c);
	return 0;
}

uint8_t CPU::xor_a_d() {
	xor8(reg.d);	
	return 0;
}

uint8_t CPU::xor_a_e() {
	xor8(reg.e);
	return 0;
}

uint8_t CPU::xor_a_h() {
	xor8(reg.h);
	return 0;
}

uint8_t CPU::xor_a_l() {
	xor8(reg.l);
	return 0;
}

uint8_t CPU::xor_a_hlp() {
	xor8(mem->read_8u(reg.hl));
	return 0;
}

uint8_t CPU::xor_a_a() {
	xor8(reg.a);
	return 0;
}

uint8_t CPU::or_a_b() {
	or8(reg.b);
	return 0;
}

uint8_t CPU::or_a_c() {
	or8(reg.c);
	return 0;
}

uint8_t CPU::or_a_d() {
	or8(reg.d);
	return 0;
}

uint8_t CPU::or_a_e() {
	or8(reg.e);
	return 0;
}

uint8_t CPU::or_a_h() {
	or8(reg.h);
	return 0;
}

uint8_t CPU::or_a_l() {
	or8(reg.l);
	return 0;
}

uint8_t CPU::or_a_hlp() {
	or8(mem->read_8u(reg.hl));
	return 0;
}

uint8_t CPU::or_a_a() {
	or8(reg.a);
	return 0;
}

uint8_t CPU::cp_a_b() {
	cp8(reg.b);
	return 0;
}

uint8_t CPU::cp_a_c() {
	cp8(reg.c);
	return 0;
}

uint8_t CPU::cp_a_d() {
	cp8(reg.d);
	return 0;
}

uint8_t CPU::cp_a_e() {
	cp8(reg.e);
	return 0;
}

uint8_t CPU::cp_a_h() {
	cp8(reg.h);
	return 0;
}

uint8_t CPU::cp_a_l() {
	cp8(reg.l);
	return 0;
}

uint8_t CPU::cp_hlp() {
	cp8(mem->read_8u(reg.hl));
	return 0;
}

uint8_t CPU::cp_a_a() {
	cp8(reg.a);
	return 0;
}

uint8_t CPU::ret_nz() {
	if (!isZeroFlag()) {
		reg.pc = pop();
		return 20;
	} else {
		reg.pc += 1;
		return 8;
	}
}

uint8_t CPU::pop_bc() {
	reg.bc = pop();
	return 0;
}

uint8_t CPU::jp_nz_a16() {
	if (!isZeroFlag()) {
		reg.pc = mem->read_16u(reg.pc+1);
		return 16;
	} else {
		reg.pc += 3;
		return 12;
	}
}

uint8_t CPU::jp_a16() {
	reg.pc = mem->read_16u(reg.pc+1);
	return 0;
}

uint8_t CPU::call_nz_a16() {
	if (!isZeroFlag()) {
		push(reg.pc+3);
		reg.pc = mem->read_16u(reg.pc+1);
		return 24;
	} else {
		reg.pc += 3;
		return 12;
	}
}

uint8_t CPU::push_bc() {
	push(reg.bc);
	return 0;
}

uint8_t CPU::add_a_d8() {
	add8(mem->read_8u(reg.pc + 1));
	return 0;
}

uint8_t CPU::rst_00h() {
	push(reg.pc+1);
	reg.pc = 0x0;
	return 0;
}

uint8_t CPU::ret_z() {
	if (isZeroFlag()) {
		reg.pc = pop();
		return 20;
	} else {
		reg.pc += 1;
		return 8;
	}
}

uint8_t CPU::ret() {
	reg.pc = pop();
	return 0;
}

uint8_t CPU::jp_z_a16() {
	if (isZeroFlag()) {
		reg.pc = mem->read_16u(reg.pc+1);
		return 16;
	} else {
		reg.pc += 3;
		return 12;
	}
}

uint8_t CPU::cb() {
	ext = 1;
	return 0;
}

uint8_t CPU::call_z_a16() {
	if (isZeroFlag()) {
		push(reg.pc+3);
		reg.pc = mem->read_16u(reg.pc+1);
		return 24;
	} else {
		reg.pc += 3;
		return 12;
	}
}

uint8_t CPU::call_a16() {
	push(reg.pc+3);
	reg.pc = mem->read_16u(reg.pc+1);
	return 0;
}

uint8_t CPU::adc_d8() {
	adc8(mem->read_8u(reg.pc + 1));
	return 0;
}

uint8_t CPU::rst_08h() {
	push(reg.pc+1);
	reg.pc = 0x8;
	return 0;
}

uint8_t CPU::ret_nc() {
	if (!isCarryFlag()) {
		reg.pc = pop();
		return 20;
	} else {
		reg.pc += 1;
		return 8;
	}
}

uint8_t CPU::pop_de() {
	reg.de = pop();
	return 0;
}

uint8_t CPU::jp_nc_a16() {
	if (!isCarryFlag()) {
		reg.pc = mem->read_16u(reg.pc+1);
		return 16;
	} else {
		reg.pc += 3;
		return 12;
	}
}

uint8_t CPU::undefined() {
	return 0;
}

uint8_t CPU::call_nc_a16() {
	if (!isCarryFlag()) {
		push(reg.pc+3);
		reg.pc = mem->read_16u(reg.pc+1);
		return 24;
	} else {
		reg.pc += 3;
		return 12;
	}
}

uint8_t CPU::push_de() {
	push(reg.de);
	return 0;
}

uint8_t CPU::sub_a_d8() {
	sub8(mem->read_8u(reg.pc + 1));
	return 0;
}

uint8_t CPU::rst_10h() {
	push(reg.pc+1);
	reg.pc = 0x10;
	return 0;
}

uint8_t CPU::ret_c() {
	if (isCarryFlag()) {
		reg.pc = pop();
		return 20;
	} else {
		reg.pc += 1;
		return 8;
	}
}

uint8_t CPU::reti() {
	enableInterrupts();
	reg.pc = pop();
	return 0;
}

uint8_t CPU::jp_c_a16() {
	if (isCarryFlag()) {
		reg.pc = mem->read_16u(reg.pc+1);
		return 16;
	} else {
		reg.pc += 3;
		return 12;
	}
}

uint8_t CPU::call_c_a16() {
	if (isCarryFlag()) {
		push(reg.pc+3);
		reg.pc = mem->read_16u(reg.pc+1);
		return 24;
	} else {
		reg.pc += 3;
		return 12;
	}
}

uint8_t CPU::sbc_a_d8() {
	sbc8(mem->read_8u(reg.pc + 1));
	return 0;
}

uint8_t CPU::rst_18h() {
	push(reg.pc+1);
	reg.pc = 0x18;
	return 0;
}

uint8_t CPU::ldh_a8_ap() {
	mem->write_8u(0xFF00 + mem->read_8u(reg.pc + 1), reg.a);
	return 0;
}

uint8_t CPU::pop_hl() {
	reg.hl = pop();
	return 0;
}

uint8_t CPU::ldh_cp_a() {
	mem->write_8u(0xFF00 + reg.c, reg.a);
	return 0;
}

uint8_t CPU::push_hl() {
	push(reg.hl);
	return 0;
}

uint8_t CPU::and_a_d8() {
	and8(mem->read_8u(reg.pc + 1));
	return 0;
}

uint8_t CPU::rst_20h() {
	push(reg.pc+1);
	reg.pc = 0x20;
	return 0;
}

uint8_t CPU::add_sp_r8() {
	int8_t value = mem->read_8s(reg.pc + 1);
	setCarryFlag(((reg.sp ^ value ^ ((reg.sp+value) & 0xFFFF)) & 0x100) == 0x100);
	setHalfCarryFlag(((reg.sp ^ value ^ ((reg.sp+value) & 0xFFFF)) & 0x10) == 0x10);

	reg.sp += value;

	setZeroFlag(false);
	setSubtractFlag(false);
	return 0;
}

uint8_t CPU::jp_hlp() {
	reg.pc = reg.hl;
	return 0;
}

uint8_t CPU::ld_a16p_a() {
	mem->write_8u(mem->read_16u(reg.pc + 1), reg.a);
	return 0;
}

uint8_t CPU::xor_a_d8() {
	xor8(mem->read_8u(reg.pc + 1));
	return 0;
}

uint8_t CPU::rst_28h() {
	push(reg.pc+1);
	reg.pc = 0x28;
	return 0;
}

uint8_t CPU::ldh_a_a8p() {
	reg.a = mem->read_8u(0xFF00 + mem->read_8u(reg.pc + 1));
	return 0;
}

uint8_t CPU::pop_af() {
	reg.af = mem->read_16u(reg.sp) & 0xFFF0;
	reg.sp += 2;
	
	return 0;
}

uint8_t CPU::ldh_a_cp() {
	reg.a = mem->read_8u(0xFF00 + reg.c);
	return 0;
}

uint8_t CPU::di() {
	disableInterrupts();
	return 0;
}

uint8_t CPU::push_af() {
	push(reg.af);
	return 0;
}

uint8_t CPU::or_a_d8() {
	or8(mem->read_8u(reg.pc + 1));
	return 0;
}

uint8_t CPU::rst_30h() {
	push(reg.pc+1);
	reg.pc = 0x30;
	return 0;
}

uint8_t CPU::ld_hl_sp_r8() {
	int8_t value = mem->read_8s(reg.pc + 1);
	setCarryFlag(((reg.sp ^ value ^ (reg.sp + value)) & 0x100) == 0x100);
	setHalfCarryFlag(((reg.sp ^ value ^ (reg.sp + value)) & 0x10) == 0x10);
	
	reg.hl = reg.sp + value;

	setZeroFlag(false);
	setSubtractFlag(false);
	
	return 0;
}

uint8_t CPU::ld_sp_hl() {
	reg.sp = reg.hl;
	return 0;
}

uint8_t CPU::ld_a_a16p() {
	reg.a = mem->read_8u(mem->read_16u(reg.pc + 1));
	return 0;
}

uint8_t CPU::ei() {
	enableInterrupts();
	return 0;
}

uint8_t CPU::cp_d8() {
	cp8(mem->read_8u(reg.pc + 1));
	return 0;
}

uint8_t CPU::rst_38h() {
	push(reg.pc+1);
	reg.pc = 0x38;
	return 0;
}



/* Extented Instructions Methods */
uint8_t CPU::rlc_b() {
	rlc(&reg.b);
	return 0;
}
uint8_t CPU::rlc_c() {
	rlc(&reg.c);
	return 0;
}
uint8_t CPU::rlc_d() {
	rlc(&reg.d);
	return 0;
}
uint8_t CPU::rlc_e() {
	rlc(&reg.e);
	return 0;
}
uint8_t CPU::rlc_h() {
	rlc(&reg.h);
	return 0;
} 
uint8_t CPU::rlc_l() {
	rlc(&reg.l);
	return 0;
} 
uint8_t CPU::rlc_hlp() {
	rlc(reg.hl);
	return 0;
}
uint8_t CPU::rlc_a() {
	rlc(&reg.a);
	return 0;
}
uint8_t CPU::rrc_b() {
	rrc(&reg.b);
	return 0;
}
uint8_t CPU::rrc_c() {
	rrc(&reg.c);
	return 0;
}
uint8_t CPU::rrc_d() {
	rrc(&reg.d);
	return 0;
} 
uint8_t CPU::rrc_e() {
	rrc(&reg.e);
	return 0;
}
uint8_t CPU::rrc_h() {
	rrc(&reg.h);
	return 0;
}
uint8_t CPU::rrc_l() {
	rrc(&reg.l);
	return 0;
}
uint8_t CPU::rrc_hlp() {
	rrc(reg.hl);
	return 0;
}	
uint8_t CPU::rrc_a() {
	rrc(&reg.a);
	return 0;
} 
uint8_t CPU::rl_b() {
	rl(&reg.b);
	return 0;
}
uint8_t CPU::rl_c() {
	rl(&reg.c);
	return 0;
} 
uint8_t CPU::rl_d() {
	rl(&reg.d);
	return 0;
}
uint8_t CPU::rl_e() {
	rl(&reg.e);
	return 0;
} 
uint8_t CPU::rl_h() {
	rl(&reg.h);
	return 0;
}
uint8_t CPU::rl_l() {
	rl(&reg.l);
	return 0;
}
uint8_t CPU::rl_hlp() {
	rl(reg.hl);
	return 0;
}
uint8_t CPU::rl_a() {
	rl(&reg.a);
	return 0;
}
uint8_t CPU::rr_b() {
	rr(&reg.b);
	return 0;
}
uint8_t CPU::rr_c() {
	rr(&reg.c);
	return 0;
}
uint8_t CPU::rr_d() {
	rr(&reg.d);
	return 0;
} 
uint8_t CPU::rr_e() {
	rr(&reg.e);
	return 0;
}
uint8_t CPU::rr_h() {
	rr(&reg.h);
	return 0;
}
uint8_t CPU::rr_l() {
	rr(&reg.l);
	return 0;
}
uint8_t CPU::rr_hlp() {
	rr(reg.hl);
	return 0;
} 
uint8_t CPU::rr_a() {
	rr(&reg.a);
	return 0;
}
uint8_t CPU::sla_b() {
	sla(&reg.b);
	return 0;
}
uint8_t CPU::sla_c() {
	sla(&reg.c);
	return 0;
}
uint8_t CPU::sla_d() {
	sla(&reg.d);
	return 0;
}
uint8_t CPU::sla_e() {
	sla(&reg.e);
	return 0;
}
uint8_t CPU::sla_h() {
	sla(&reg.h);
	return 0;
}
uint8_t CPU::sla_l() {
	sla(&reg.l);
	return 0;
}
uint8_t CPU::sla_hlp() {
	sla(reg.hl);
	return 0;
}
uint8_t CPU::sla_a() {
	sla(&reg.a);
	return 0;
}
uint8_t CPU::sra_b() {
	sra(&reg.b);
	return 0;
} 
uint8_t CPU::sra_c() {
	sra(&reg.c);
	return 0;
}
uint8_t CPU::sra_d() {
	sra(&reg.d);
	return 0;
}
uint8_t CPU::sra_e() {
	sra(&reg.e);
	return 0;
} 
uint8_t CPU::sra_h() {
	sra(&reg.h);
	return 0;
} 
uint8_t CPU::sra_l() {
	sra(&reg.l);
	return 0;
}
uint8_t CPU::sra_hlp() {
	sra(reg.hl);
	return 0;
}
uint8_t CPU::sra_a() {
	sra(&reg.a);
	return 0;
} 
uint8_t CPU::swap_b() {
	reg.b = swap(reg.b);
	return 0;
}
uint8_t CPU::swap_c() {
	reg.c = swap(reg.c);
	return 0;
}
uint8_t CPU::swap_d() {
	reg.d = swap(reg.d);
	return 0;
}
uint8_t CPU::swap_e() {
	reg.e = swap(reg.e);
	return 0;
}
uint8_t CPU::swap_h() {
	reg.h = swap(reg.h);
	return 0;
}
uint8_t CPU::swap_l() {
	reg.l = swap(reg.l);
	return 0;
}
uint8_t CPU::swap_hlp() {
	mem->write_8u(reg.hl, swap(mem->read_8u(reg.hl)));
	return 0;
}
uint8_t CPU::swap_a() {
	reg.a = swap(reg.a);
	return 0;
}
uint8_t CPU::srl_b() {
	srl(&reg.b);
	return 0;
}
uint8_t CPU::srl_c() {
	srl(&reg.c);
	return 0;
}
uint8_t CPU::srl_d() {
	srl(&reg.d);
	return 0;
} 
uint8_t CPU::srl_e() {
	srl(&reg.e);
	return 0;
}
uint8_t CPU::srl_h() {
	srl(&reg.h);
	return 0;
}
uint8_t CPU::srl_l() {
	srl(&reg.l);
	return 0;
} 
uint8_t CPU::srl_hlp() {
	srl(reg.hl);
	return 0;
}
uint8_t CPU::srl_a() {
	srl(&reg.a);
	return 0;
}
uint8_t CPU::bit_0_b() {
	bit(0, reg.b);
	return 0;
}
uint8_t CPU::bit_0_c() {
	bit(0, reg.c);
	return 0;
}
uint8_t CPU::bit_0_d() {
	bit(0, reg.d);
	return 0;
}
uint8_t CPU::bit_0_e() {
	bit(0, reg.e);
	return 0;
} 
uint8_t CPU::bit_0_h() {
	bit(0, reg.h);
	return 0;
}
uint8_t CPU::bit_0_l() {
	bit(0, reg.l);
	return 0;
}
uint8_t CPU::bit_0_hlp() {
	bit(0, mem->read_8u(reg.hl));
	return 0;
}
uint8_t CPU::bit_0_a() {
	bit(0, reg.a);
	return 0;
}
uint8_t CPU::bit_1_b() {
	bit(1, reg.b);
	return 0;
}
uint8_t CPU::bit_1_c() {
	bit(1, reg.c);
	return 0;
}
uint8_t CPU::bit_1_d() {
	bit(1, reg.d);
	return 0;
}
uint8_t CPU::bit_1_e() {
	bit(1, reg.e);
	return 0;
}
uint8_t CPU::bit_1_h() {
	bit(1, reg.h);
	return 0;
}
uint8_t CPU::bit_1_l() {
	bit(1, reg.l);
	return 0;
}
uint8_t CPU::bit_1_hlp() {
	bit(1, mem->read_8u(reg.hl));
	return 0;
}
uint8_t CPU::bit_1_a() {
	bit(1, reg.a);
	return 0;
}
uint8_t CPU::bit_2_b() {
	bit(2, reg.b);
	return 0;
}
uint8_t CPU::bit_2_c() {
	bit(2, reg.c);
	return 0;
}
uint8_t CPU::bit_2_d() {
	bit(2, reg.d);
	return 0;
}
uint8_t CPU::bit_2_e() {
	bit(2, reg.e);
	return 0;
}
uint8_t CPU::bit_2_h() {
	bit(2, reg.h);
	return 0;
}
uint8_t CPU::bit_2_l() {
	bit(2, reg.l);
	return 0;
}
uint8_t CPU::bit_2_hlp() {
	bit(2, mem->read_8u(reg.hl));
	return 0;
}
uint8_t CPU::bit_2_a() {
	bit(2, reg.a);
	return 0;
}
uint8_t CPU::bit_3_b() {
	bit(3, reg.b);
	return 0;
}
uint8_t CPU::bit_3_c() {
	bit(3, reg.c);
	return 0;
}
uint8_t CPU::bit_3_d() {
	bit(3, reg.d);
	return 0;
}
uint8_t CPU::bit_3_e() {
	bit(3, reg.e);
	return 0;
}
uint8_t CPU::bit_3_h() {
	bit(3, reg.h);
	return 0;
}
uint8_t CPU::bit_3_l() {
	bit(3, reg.l);
	return 0;
}
uint8_t CPU::bit_3_hlp() {
	bit(3, mem->read_8u(reg.hl));
	return 0;
}
uint8_t CPU::bit_3_a() {
	bit(3, reg.a);
	return 0;
}
uint8_t CPU::bit_4_b() {
	bit(4, reg.b);
	return 0;
}
uint8_t CPU::bit_4_c() {
	bit(4, reg.c);
	return 0;
}
uint8_t CPU::bit_4_d() {
	bit(4, reg.d);
	return 0;
}
uint8_t CPU::bit_4_e() {
	bit(4, reg.e);
	return 0;
}
uint8_t CPU::bit_4_h() {
	bit(4, reg.h);
	return 0;
}
uint8_t CPU::bit_4_l() {
	bit(4, reg.l);
	return 0;
}
uint8_t CPU::bit_4_hlp() {
	bit(4, mem->read_8u(reg.hl));
	return 0;
}
uint8_t CPU::bit_4_a() {
	bit(4, reg.a);
	return 0;
}
uint8_t CPU::bit_5_b() {
	bit(5, reg.b);
	return 0;
}
uint8_t CPU::bit_5_c() {
	bit(5, reg.c);
	return 0;
}
uint8_t CPU::bit_5_d() {
	bit(5, reg.d);
	return 0;
}
uint8_t CPU::bit_5_e() {
	bit(5, reg.e);
	return 0;
}
uint8_t CPU::bit_5_h() {
	bit(5, reg.h);
	return 0;
}
uint8_t CPU::bit_5_l() {
	bit(5, reg.l);
	return 0;
}
uint8_t CPU::bit_5_hlp() {
	bit(5, mem->read_8u(reg.hl));
	return 0;
}
uint8_t CPU::bit_5_a() {
	bit(5, reg.a);
	return 0;
}
uint8_t CPU::bit_6_b() {
	bit(6, reg.b);
	return 0;
}
uint8_t CPU::bit_6_c() {
	bit(6, reg.c);
	return 0;
}
uint8_t CPU::bit_6_d() {
	bit(6, reg.d);
	return 0;
}
uint8_t CPU::bit_6_e() {
	bit(6, reg.e);
	return 0;
}
uint8_t CPU::bit_6_h() {
	bit(6, reg.h);
	return 0;
}
uint8_t CPU::bit_6_l() {
	bit(6, reg.l);
	return 0;
}
uint8_t CPU::bit_6_hlp() {
	bit(6, mem->read_8u(reg.hl));
	return 0;
}
uint8_t CPU::bit_6_a() {
	bit(6, reg.a);
	return 0;
}
uint8_t CPU::bit_7_b() {
	bit(7, reg.b);
	return 0;
}
uint8_t CPU::bit_7_c() {
	bit(7, reg.c);
	return 0;
}
uint8_t CPU::bit_7_d() {
	bit(7, reg.d);
	return 0;
}
uint8_t CPU::bit_7_e() {
	bit(7, reg.e);
	return 0;
}
uint8_t CPU::bit_7_h() {
	bit(7, reg.h);
	return 0;
}
uint8_t CPU::bit_7_l() {
	bit(7, reg.l);
	return 0;
}
uint8_t CPU::bit_7_hlp() {
	bit(7, mem->read_8u(reg.hl));
	return 0;
}
uint8_t CPU::bit_7_a() {
	bit(7, reg.a);
	return 0;
}
uint8_t CPU::res_0_b() {
	res(0, &reg.b);
	return 0;
}
uint8_t CPU::res_0_c() {
	res(0, &reg.c);
	return 0;
}
uint8_t CPU::res_0_d() {
	res(0, &reg.d);
	return 0;
}
uint8_t CPU::res_0_e() {
	res(0, &reg.e);
	return 0;
}
uint8_t CPU::res_0_h() {
	res(0, &reg.h);
	return 0;
}
uint8_t CPU::res_0_l() {
	res(0, &reg.l);
	return 0;
}
uint8_t CPU::res_0_hlp() {
	res(0, reg.hl);
	return 0;
}
uint8_t CPU::res_0_a() {
	res(0, &reg.a);
	return 0;
}
uint8_t CPU::res_1_b() {
	res(1, &reg.b);
	return 0;
}
uint8_t CPU::res_1_c() {
	res(1, &reg.c);
	return 0;
}
uint8_t CPU::res_1_d() {
	res(1, &reg.d);
	return 0;
}
uint8_t CPU::res_1_e() {
	res(1, &reg.e);
	return 0;
}
uint8_t CPU::res_1_h() {
	res(1, &reg.h);
	return 0;
}
uint8_t CPU::res_1_l() {
	res(1, &reg.l);
	return 0;
}
uint8_t CPU::res_1_hlp() {
	res(1, reg.hl);
	return 0;
}
uint8_t CPU::res_1_a() {
	res(1, &reg.a);
	return 0;
}
uint8_t CPU::res_2_b() {
	res(2, &reg.b);
	return 0;
}
uint8_t CPU::res_2_c() {
	res(2, &reg.c);
	return 0;
}
uint8_t CPU::res_2_d() {
	res(2, &reg.d);
	return 0;
}
uint8_t CPU::res_2_e() {
	res(2, &reg.e);
	return 0;
}
uint8_t CPU::res_2_h() {
	res(2, &reg.h);
	return 0;
}
uint8_t CPU::res_2_l() {
	res(2, &reg.l);
	return 0;
}
uint8_t CPU::res_2_hlp() {
	res(2, reg.hl);
	return 0;
}
uint8_t CPU::res_2_a() {
	res(2, &reg.a);
	return 0;
}
uint8_t CPU::res_3_b() {
	res(3, &reg.b);
	return 0;
}
uint8_t CPU::res_3_c() {
	res(3, &reg.c);
	return 0;
}
uint8_t CPU::res_3_d() {
	res(3, &reg.d);
	return 0;
}
uint8_t CPU::res_3_e() {
	res(3, &reg.e);
	return 0;
}
uint8_t CPU::res_3_h() {
	res(3, &reg.h);
	return 0;
}
uint8_t CPU::res_3_l() {
	res(3, &reg.l);
	return 0;
}
uint8_t CPU::res_3_hlp() {
	res(3, reg.hl);
	return 0;
}
uint8_t CPU::res_3_a() {
	res(3, &reg.a);
	return 0;
}
uint8_t CPU::res_4_b() {
	res(4, &reg.b);
	return 0;
}
uint8_t CPU::res_4_c() {
	res(4, &reg.c);
	return 0;
}
uint8_t CPU::res_4_d() {
	res(4, &reg.d);
	return 0;
}
uint8_t CPU::res_4_e() {
	res(4, &reg.e);
	return 0;
}
uint8_t CPU::res_4_h() {
	res(4, &reg.h);
	return 0;
}
uint8_t CPU::res_4_l() {
	res(4, &reg.l);
	return 0;
}
uint8_t CPU::res_4_hlp() {
	res(4, reg.hl);
	return 0;
}
uint8_t CPU::res_4_a() {
	res(4, &reg.a);
	return 0;
}
uint8_t CPU::res_5_b() {
	res(5, &reg.b);
	return 0;
}
uint8_t CPU::res_5_c() {
	res(5, &reg.c);
	return 0;
}
uint8_t CPU::res_5_d() {
	res(5, &reg.d);
	return 0;
}
uint8_t CPU::res_5_e() {
	res(5, &reg.e);
	return 0;
}
uint8_t CPU::res_5_h() {
	res(5, &reg.h);
	return 0;
}
uint8_t CPU::res_5_l() {
	res(5, &reg.l);
	return 0;
}
uint8_t CPU::res_5_hlp() {
	res(5, reg.hl);
	return 0;
}
uint8_t CPU::res_5_a() {
	res(5, &reg.a);
	return 0;
}
uint8_t CPU::res_6_b() {
	res(6, &reg.b);
	return 0;
}
uint8_t CPU::res_6_c() {
	res(6, &reg.c);
	return 0;
}
uint8_t CPU::res_6_d() {
	res(6, &reg.d);
	return 0;
}
uint8_t CPU::res_6_e() {
	res(6, &reg.e);
	return 0;
}
uint8_t CPU::res_6_h() {
	res(6, &reg.h);
	return 0;
}
uint8_t CPU::res_6_l() {
	res(6, &reg.l);
	return 0;
}
uint8_t CPU::res_6_hlp() {
	res(6, reg.hl);
	return 0;
}
uint8_t CPU::res_6_a() {
	res(6, &reg.a);
	return 0;
}
uint8_t CPU::res_7_b() {
	res(7, &reg.b);
	return 0;
}
uint8_t CPU::res_7_c() {
	res(7, &reg.c);
	return 0;
}
uint8_t CPU::res_7_d() {
	res(7, &reg.d);
	return 0;
}
uint8_t CPU::res_7_e() {
	res(7, &reg.e);
	return 0;
}
uint8_t CPU::res_7_h() {
	res(7, &reg.h);
	return 0;
}
uint8_t CPU::res_7_l() {
	res(7, &reg.l);
	return 0;
}
uint8_t CPU::res_7_hlp() {
	res(7, reg.hl);
	return 0;
}
uint8_t CPU::res_7_a() {
	res(7, &reg.a);
	return 0;
}
uint8_t CPU::set_0_b() {
	set(0, &reg.b);
	return 0;
}
uint8_t CPU::set_0_c() {
	set(0, &reg.c);
	return 0;
}
uint8_t CPU::set_0_d() {
	set(0, &reg.d);
	return 0;
}
uint8_t CPU::set_0_e() {
	set(0, &reg.e);
	return 0;
}
uint8_t CPU::set_0_h() {
	set(0, &reg.h);
	return 0;
}
uint8_t CPU::set_0_l() {
	set(0, &reg.l);
	return 0;
}
uint8_t CPU::set_0_hlp() {
	set(0, reg.hl);
	return 0;
}
uint8_t CPU::set_0_a() {
	set(0, &reg.a);
	return 0;
}
uint8_t CPU::set_1_b() {
	set(1, &reg.b);
	return 0;
}
uint8_t CPU::set_1_c() {
	set(1, &reg.c);
	return 0;
}
uint8_t CPU::set_1_d() {
	set(1, &reg.d);
	return 0;
}
uint8_t CPU::set_1_e() {
	set(1, &reg.e);
	return 0;
}
uint8_t CPU::set_1_h() {
	set(1, &reg.h);
	return 0;
}
uint8_t CPU::set_1_l() {
	set(1, &reg.l);
	return 0;
}
uint8_t CPU::set_1_hlp() {
	set(1, reg.hl);
	return 0;
}
uint8_t CPU::set_1_a() {
	set(1, &reg.a);
	return 0;
}
uint8_t CPU::set_2_b() {
	set(2, &reg.b);
	return 0;
}
uint8_t CPU::set_2_c() {
	set(2, &reg.c);
	return 0;
}
uint8_t CPU::set_2_d() {
	set(2, &reg.d);
	return 0;
}
uint8_t CPU::set_2_e() {
	set(2, &reg.e);
	return 0;
}
uint8_t CPU::set_2_h() {
	set(2, &reg.h);
	return 0;
}
uint8_t CPU::set_2_l() {
	set(2, &reg.l);
	return 0;
}
uint8_t CPU::set_2_hlp() {
	set(2, reg.hl);
	return 0;
}
uint8_t CPU::set_2_a() {
	set(2, &reg.a);
	return 0;
}
uint8_t CPU::set_3_b() {
	set(3, &reg.b);
	return 0;
}
uint8_t CPU::set_3_c() {
	set(3, &reg.c);
	return 0;
}
uint8_t CPU::set_3_d() {
	set(3, &reg.d);
	return 0;
}
uint8_t CPU::set_3_e() {
	set(3, &reg.e);
	return 0;
}
uint8_t CPU::set_3_h() {
	set(3, &reg.h);
	return 0;
}
uint8_t CPU::set_3_l() {
	set(3, &reg.l);
	return 0;
}
uint8_t CPU::set_3_hlp() {
	set(3, reg.hl);
	return 0;
}
uint8_t CPU::set_3_a() {
	set(3, &reg.a);
	return 0;
}
uint8_t CPU::set_4_b() {
	set(4, &reg.b);
	return 0;
}
uint8_t CPU::set_4_c() {
	set(4, &reg.c);
	return 0;
}
uint8_t CPU::set_4_d() {
	set(4, &reg.d);
	return 0;
}
uint8_t CPU::set_4_e() {
	set(4, &reg.e);
	return 0;
}
uint8_t CPU::set_4_h() {
	set(4, &reg.h);
	return 0;
}
uint8_t CPU::set_4_l() {
	set(4, &reg.l);
	return 0;
}
uint8_t CPU::set_4_hlp() {
	set(4, reg.hl);
	return 0;
}
uint8_t CPU::set_4_a() {
	set(4, &reg.a);
	return 0;
}
uint8_t CPU::set_5_b() {
	set(5, &reg.b);
	return 0;
}
uint8_t CPU::set_5_c() {
	set(5, &reg.c);
	return 0;
}
uint8_t CPU::set_5_d() {
	set(5, &reg.d);
	return 0;
}
uint8_t CPU::set_5_e() {
	set(5, &reg.e);
	return 0;
}
uint8_t CPU::set_5_h() {
	set(5, &reg.h);
	return 0;
}
uint8_t CPU::set_5_l() {
	set(5, &reg.l);
	return 0;
}
uint8_t CPU::set_5_hlp() {
	set(5, reg.hl);
	return 0;
}
uint8_t CPU::set_5_a() {
	set(5, &reg.a);
	return 0;
}
uint8_t CPU::set_6_b() {
	set(6, &reg.b);
	return 0;
}
uint8_t CPU::set_6_c() {
	set(6, &reg.c);
	return 0;
}
uint8_t CPU::set_6_d() {
	set(6, &reg.d);
	return 0;
}
uint8_t CPU::set_6_e() {
	set(6, &reg.e);
	return 0;
}
uint8_t CPU::set_6_h() {
	set(6, &reg.h);
	return 0;
}
uint8_t CPU::set_6_l() {
	set(6, &reg.l);
	return 0;
}
uint8_t CPU::set_6_hlp() {
	set(6, reg.hl);
	return 0;
}
uint8_t CPU::set_6_a() {
	set(6, &reg.a);
	return 0;
}
uint8_t CPU::set_7_b() {
	set(7, &reg.b);
	return 0;
}
uint8_t CPU::set_7_c() {
	set(7, &reg.c);
	return 0;
}
uint8_t CPU::set_7_d() {
	set(7, &reg.d);
	return 0;
}
uint8_t CPU::set_7_e() {
	set(7, &reg.e);
	return 0;
}
uint8_t CPU::set_7_h() {
	set(7, &reg.h);
	return 0;
}
uint8_t CPU::set_7_l() {
	set(7, &reg.l);
	return 0;
}
uint8_t CPU::set_7_hlp() {
	set(7, reg.hl);
	return 0;
}
uint8_t CPU::set_7_a() {
	set(7, &reg.a);
	return 0;
}

/* CPU Helper Functions */
void CPU::inc8(uint8_t* r) {
	setHalfCarryFlag((*r & 0xF) == 0xF);
	++*r;

	setZeroFlag(*r == 0);
	setSubtractFlag(false);
}

void CPU::inc16(uint16_t* r) {
	++*r;
}

void CPU::dec8(uint8_t* r) {
	setHalfCarryFlag((*r & 0xF) == 0);
	--*r;

	setZeroFlag(*r == 0);
	setSubtractFlag(true);
}

void CPU::dec16(uint16_t* r) {
	--*r;
}

void CPU::add8(uint8_t value) {
	setCarryFlag(((uint16_t) reg.a + (uint16_t) value) > 0xFF);
	setHalfCarryFlag((((reg.a & 0xf) + (value & 0xf)) & 0x10) == 0x10);

	reg.a += value;

	setZeroFlag(reg.a == 0);
	setSubtractFlag(false);
}

void CPU::add16(uint16_t value) {
	setCarryFlag(((uint32_t) reg.hl + (uint32_t) value) > 0xFFFF);
	setHalfCarryFlag((((reg.hl & 0xfff) + (value & 0xfff)) & 0x1000) == 0x1000);	
	
	reg.hl += value;
	
	// Zero flag not affected
	setSubtractFlag(false);
}

void CPU::adc8(uint8_t value) {
	uint8_t carry = (uint8_t) isCarryFlag();
	setCarryFlag(((uint16_t) reg.a + (uint16_t) value + carry) > 0xFF);
	setHalfCarryFlag((((reg.a & 0xF) + (value & 0xF) + carry) & 0x10) == 0x10);

	reg.a += value + carry;

	setZeroFlag(reg.a == 0);
	setSubtractFlag(false);
}

void CPU::sub8(uint8_t value) {
	setCarryFlag((int16_t) reg.a - (int16_t) value < 0);
	setHalfCarryFlag((((reg.a & 0xf) - (value & 0xf)) & 0x10) == 0x10);

	reg.a -= value;

	setZeroFlag(reg.a == 0);
	setSubtractFlag(true);
}

void CPU::sbc8(uint8_t value) {
	uint8_t carry = (uint8_t) isCarryFlag();
	setCarryFlag(((uint16_t) reg.a - (uint16_t) value - carry) < 0);
	setHalfCarryFlag((((reg.a & 0xF) - (value & 0xF) - carry) & 0x10) == 0x10);

	reg.a -= value + carry;

	setZeroFlag(reg.a == 0);
	setSubtractFlag(true);
}

void CPU::and8(uint8_t value) {
	reg.a = reg.a & value;
	setZeroFlag(reg.a == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(true);
	setCarryFlag(false);
}

void CPU::or8(uint8_t value) {
	reg.a = reg.a | value;
	setZeroFlag(reg.a == 0);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setCarryFlag(false);
}

void CPU::xor8(uint8_t value) {
	reg.a = reg.a ^ value;
	setZeroFlag(reg.a == 0);
	setCarryFlag(false);
	setSubtractFlag(false);
	setHalfCarryFlag(false);
}

void CPU::cp8(uint8_t value) {
	setCarryFlag((uint8_t) reg.a < value);
	setZeroFlag((uint8_t) reg.a == value);
	setSubtractFlag(true);
	setHalfCarryFlag((value & 0x0f) > (reg.a & 0x0f));
}

uint8_t CPU::swap(uint8_t value) {
	value = ((value & 0x0F)<<4 | (value & 0xF0)>>4); 
	setZeroFlag(value == 0);
	setHalfCarryFlag(false);
	setSubtractFlag(false);
	setCarryFlag(false);
	return value;
}

void CPU::push(uint16_t value) {
	reg.sp -= 2;
	mem->write_16u(reg.sp, value);
}

uint16_t CPU::pop() {
	uint16_t value = mem->read_16u(reg.sp);
	reg.sp += 2;
	
	return value;
}

void CPU::bit(uint8_t bit, uint8_t value) {
	setZeroFlag(!((value >> bit) & 0x01));
	setHalfCarryFlag(true);
	setSubtractFlag(false);
}

void CPU::set(uint8_t bit, uint8_t* r) {
	*r |= (1 << bit);
}

void CPU::set(uint8_t bit, uint16_t addr) {
	uint8_t v = mem->read_8u(addr);
	v |= (1 << bit);
	mem->write_8u(addr, v);
}

void CPU::res(uint8_t bit, uint8_t* r) {
	*r &= ~(1 << bit);
}
void CPU::res(uint8_t bit, uint16_t addr) {
	uint8_t v = mem->read_8u(addr);
	v &= ~(1 << bit);
	mem->write_8u(addr, v);
}

void CPU::rl(uint8_t* r) {
	uint8_t carry = isCarryFlag();

	setCarryFlag(*r & 0x80);

	*r <<= 1;
	*r += carry;

	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setZeroFlag(*r == 0);
}
void CPU::rl(uint16_t addr) {
	uint8_t carry = isCarryFlag();
	uint8_t v = mem->read_8u(addr);

	setCarryFlag(v & 0x80);

	v <<= 1;
	v += carry;

	mem->write_8u(addr, v);

	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setZeroFlag(v == 0);
}

void CPU::rlc(uint8_t* r) {
	setCarryFlag((*r >> 7) & 1);

	*r <<= 1;
	*r |= isCarryFlag();

	setZeroFlag(*r == 0);
	setHalfCarryFlag(false);
	setSubtractFlag(false);
}
void CPU::rlc(uint16_t addr) {
	uint8_t v = mem->read_8u(addr);

	setCarryFlag((v >> 7) & 1);

	v <<= 1;
	v |= isCarryFlag();

	mem->write_8u(addr, v);

	setZeroFlag(v == 0);
	setHalfCarryFlag(false);
	setSubtractFlag(false);
}

void CPU::rr(uint8_t* r) {
	uint8_t carry = isCarryFlag() << 7;

	setCarryFlag(*r & 0x1);

	*r >>= 1;
	*r += carry;

	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setZeroFlag(*r == 0);
}
void CPU::rr(uint16_t addr) {
	uint8_t carry = isCarryFlag() << 7;
	uint8_t v = mem->read_8u(addr);

	setCarryFlag(v & 0x1);

	v >>= 1;
	v += carry;

	mem->write_8u(addr, v);

	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setZeroFlag(v == 0);
}

void CPU::rrc(uint8_t* r) {
	setCarryFlag(*r & 0x1);

	*r >>= 1;
	*r |= (isCarryFlag() << 7);

	setZeroFlag(*r == 0);
	setHalfCarryFlag(false);
	setSubtractFlag(false);	
}
void CPU::rrc(uint16_t addr) {
	uint8_t v = mem->read_8u(addr);

	setCarryFlag(v & 0x1);

	v >>= 1;
	v |= (isCarryFlag() << 7);

	mem->write_8u(addr, v);

	setZeroFlag(v == 0);
	setHalfCarryFlag(false);
	setSubtractFlag(false);
}

void CPU::sla(uint8_t* r) {
	setCarryFlag(*r & 0x80);

	*r <<= 1;

	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setZeroFlag(*r == 0);
}
void CPU::sla(uint16_t addr) {
	uint8_t v = mem->read_8u(addr);

	setCarryFlag(v & 0x80);

	v <<= 1;

	mem->write_8u(addr, v);

	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setZeroFlag(v == 0);
}

void CPU::sra(uint8_t* r) {
	setCarryFlag(*r & 0x1);

	uint8_t msb = *r & 0x80;
	*r >>= 1;
	*r |= msb;

	setZeroFlag(*r == 0);
	setHalfCarryFlag(false);
	setSubtractFlag(false);
}
void CPU::sra(uint16_t addr) {
	uint8_t v = mem->read_8u(addr);

	setCarryFlag(v & 0x1);

	uint8_t msb = v & 0x80;
	v >>= 1;
	v |= msb;

	mem->write_8u(addr, v);

	setZeroFlag(v == 0);
	setHalfCarryFlag(false);
	setSubtractFlag(false);
}

void CPU::srl(uint8_t* r) {
	setCarryFlag(*r & 0x1);

	*r >>= 1;

	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setZeroFlag(*r == 0);
}
void CPU::srl(uint16_t addr) {
	uint8_t v = mem->read_8u(addr);

	setCarryFlag(v & 0x1);

	v >>= 1;

	mem->write_8u(addr, v);

	setSubtractFlag(false);
	setHalfCarryFlag(false);
	setZeroFlag(v == 0);
}

/* Testing Methods */
void CPU::testEndianness() {
	reg.h = 0x3;
	reg.l = 0x2;
	cout << "H: " << (int) reg.h << " | L: " << (int) reg.l << " | HL: " << (int) reg.hl << endl;
	cout << "STORE to 0x0" << endl;
	mem->write_16u(0x0, reg.hl);
	reg.hl = mem->read_16u(0x0);
	cout << "0x0: " << (int) mem->read_8u(0x0) << endl;
	cout << "0x1: " << (int) mem->read_8u(0x1) << endl;
	cout << "READ FROM 0x0" << endl;
	cout << "H: " << (int) reg.h << " | L: " << (int) reg.l << " | HL: " << (int) reg.hl << endl;
}

void CPU::testStatusFlags() {
	cout << "Testing status flags" << endl;
	reg.f = 0x0;
	cout << "Register F: " << std::bitset<8>(reg.f) << endl;
	cout << "Setting flags..." << endl;
	setZeroFlag(1);
	setCarryFlag(1);
	setSubtractFlag(1);
	setHalfCarryFlag(1);
	cout << "Register F: " << std::bitset<8>(reg.f) << endl;
	cout << "getZeroFlag: " << isZeroFlag() << endl;
	cout << "getSubtractFlag: " << isZeroFlag() << endl;
	cout << "getCarryFlag: " << isZeroFlag() << endl;
	cout << "getHalfCarryFlag: " << isZeroFlag() << endl;
	reg.f = 0;
}
#include "../Component/Memory.h"
#include "../Component/Config.h"
#include "../Component/Joypad.h"

#include <cstring>
#include <iostream>


/* Constructor */
Memory::Memory() {
	isMapped = 0;
	bankingController = 0;
	bankMode = 0;
	romBank = 1;
	ramBank = 0;

	/* Set whole Memory to 0b11111111 (0xFF) at start.
	for(int i = 0; i < MEM_SIZE; i++) {
		memory[i] = 0xFF;
	}
	*/
}

/* Methods */
void Memory::initialize() {
	// Copy Cartidge into GBs address space
	copyFromCartridge(0x100, 0x100, getSize(0x100, 0x8000));

	// Select MBC by reading Cartridges Header
	switch(read_8u(0x147)) {
		case 1:
		case 2:
		case 3:
			bankingController = 1;
			break;
		case 5:
		case 6:
			bankingController = 2;
			break;
		case 0xF:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			bankingController = 3;
			break;
		case 0x15:
		case 0x16:
		case 0x17:
			bankingController = 4;
			break;
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
			bankingController = 5;
			break;
		default:
			bankingController = 0;
			break;
	}
	printf("== Current Banking Controller: %d\n", bankingController);
}

void Memory::remapUnit() {
	if (read_8u(0xFF50) == 1 && !isMapped) {
		copyFromCartridge(0x0, 0x0, 0xFF);
		isMapped = 1;
	}
}

void Memory::bankUnit(uint16_t addr, uint8_t value) {
	switch(bankingController) {
		// No MBC
		case 0:
			return;
		// MBC1
		case 1:
			handleMBC1(addr, value);
			return;
		// MBC2
		case 2:
			handleMBC2(addr, value);
			return;
		//MBC3
		case 3:
			handleMBC3(addr, value);
			return;
		case 4: 
			handleMBC4(addr, value);
			return;
		case 5: 
			handleMBC5(addr, value);
			return;
		default:
			printf("Current Memory Banking Controller not supported!\n");
			return;
	}
}


void Memory::handleMBC1(uint16_t addr, uint8_t value) {
	// Handle RAM Banking
	if(addr >= 0x0000 && addr <= 0x1FFF) {
		if ((value & 0xF) == 0xA) {
			// Enable RAM
			isRAMEnabled = 1;
			//printf("---- Enable RAM Banking!\n");
		}
		else if ((value & 0xF) == 0x0) {
			// Disable RAM
			isRAMEnabled = 0;
			//printf("---- Disable RAM Banking!\n");
		}
	}
	// Handle Lo ROM Banking
	if (addr >= 0x2000 && addr <= 0x3FFF) {
		// Prevent setting romBank to 0h, 20h, 40h, 60h
		if ((value & 0x1F) == 0) {
			value = 1;
		}

		// Clear and set the lower 5 bits of romBank
		romBank &= 0xE0;
		romBank |= (value & 0x1F);
		// Switch ROM Banks in memory
		copyFromCartridge(0x4000, romBank*0x4000, getSize(0x4000, 0x8000));
	}
	// Handle Hi ROM Banking OR RAM Banking
	if (addr >= 0x4000 && addr <= 0x5FFF) {
		//printf("Hi Banking! Current value: %d \n", value);
		if (bankMode == 0) {
			// ROM Banking Mode
			// Clear and set the upper 3 bits of romBank
			romBank &= 0x1F;
			romBank |= (value & 0xE0);

			// Switch ROM Banks in memory
			copyFromCartridge(0x4000, romBank*0x4000, getSize(0x4000, 0x8000));
		} else {
			// RAM Banking Mode
			copyToRAM(ramBank*0x2000, 0xA000, getSize(0xA000, 0xC000));
			ramBank = (value & 0x3);
			copyFromRAM(0xA000, ramBank*0x2000, getSize(0xA000, 0xC000));
		}
	}
	// Handle ROM/RAM Mode Select
	if (addr >= 0x6000 && addr <= 0x7FFF) {
		bankMode = (value & 0x1);

		if (bankMode == 0) {
			ramBank = 0;
		}
	}
	return;
}

void Memory::handleMBC2(uint16_t addr, uint8_t value) {
	//TODO handle MBC2
	return;
}

void Memory::handleMBC3(uint16_t addr, uint8_t value) {
	// Handle RAM Banking
	if(addr >= 0x0000 && addr <= 0x1FFF) {
		if ((value & 0xF) == 0xA) {
			// Enable RAM
			// TODO: Check for RTC register
			isRAMEnabled = 1;
		}
		else if ((value & 0xF) == 0x0) {
			// Disable RAM
			// TODO: Check for RTC register
			isRAMEnabled = 0;
		}
	}
	// Handle ROM/RAM Mode Select
	if (addr >= 0x0000 && addr <= 0x1fff) {
		bankMode = (value & 0x1);
		
		if (bankMode == 0) {
			ramBank = 0;
		}
	}
	// HANDLE RAM BANKING

	// Handle Lo ROM Banking
	if (addr >= 0x2000 && addr <= 0x3FFF) {
		// bank 0 -> bank 1
		if (value == 0)
			value = 1;
		// Clear and set the lower 7 bits of romBank
		romBank &= 0x80;
		romBank |= (value & 0x7f);
		// Switch ROM Banks in memory
		copyFromCartridge(0x4000, romBank*0x4000, getSize(0x4000, 0x8000));
	}
	
	if (addr >= 0x4000 && addr <= 0x5FFF) {
		if (bankMode == 0) {
			// ROM Banking Mode
			// Clear and set the upper 3 bits of romBank
			romBank &= 0x1F;
			romBank |= (value & 0xE0);
			// Switch ROM Banks in memory
			copyFromCartridge(0x4000, romBank*0x4000, getSize(0x4000, 0x8000));
		} else {
			// RAM Banking Mode
			copyToRAM(ramBank*0x2000, 0xA000, getSize(0xA000, 0xC000));
			ramBank = (value & 0x3);
			copyFromRAM(0xA000, ramBank*0x2000, getSize(0xA000, 0xC000));
		}
	}			
	return;
}

void Memory::handleMBC4(uint16_t addr, uint8_t value) {
	return;
}

void Memory::handleMBC5(uint16_t addr, uint8_t value) {
	// Handle RAM Banking
	if(addr >= 0x0000 && addr <= 0x1FFF) {
		if ((value & 0xF) == 0xA) {
			// Enable RAM
			// TODO: Check for RTC register
			isRAMEnabled = 1;
		}
		else if ((value & 0xF) == 0x0) {
			// Disable RAM
			// TODO: Check for RTC register
			isRAMEnabled = 0;
		}
	}
	// Handle ROM/RAM Mode Select
	if (addr >= 0x0000 && addr <= 0x1fff) {
		bankMode = (value & 0x1);
		
		if (bankMode == 0) {
			ramBank = 0;
		}
	}
	// HANDLE RAM BANKING

	// Handle Lo ROM Banking
	if (addr >= 0x2000 && addr <= 0x3FFF) {
		// bank 0 -> bank 1
		if (value == 0)
			value = 1;
		// Clear and set the lower  8 bits of romBank
		romBank &= 0x100;
		romBank |= (value & 0xFF);
		// Switch ROM Banks in memory
		copyFromCartridge(0x4000, romBank*0x4000, getSize(0x4000, 0x8000));
	}
	
	if (addr >= 0x3000 && addr <= 0x3FFF) {
		// Clear and set the 9th bit of romBank
		romBank &= 0xFF;
		romBank |= ((value & 0x1) << 8); 
		// Switch ROM Banks in memory
		copyFromCartridge(0x4000, romBank*0x4000, getSize(0x4000, 0x8000));
	}
	if (addr >= 0x4000 && addr <= 0x5FFF) {
		if (bankMode == 0) {
			// ROM Banking Mode
			// Clear and set the upper 3 bits of romBank
			romBank &= 0x1F;
			romBank |= (value & 0xE0);
			// Switch ROM Banks in memory
			copyFromCartridge(0x4000, romBank*0x4000, getSize(0x4000, 0x8000));
		} else {
			// RAM Banking Mode
			copyToRAM(ramBank*0x2000, 0xA000, getSize(0xA000, 0xC000));
			ramBank = (value & 0x3);
			copyFromRAM(0xA000, ramBank*0x2000, getSize(0xA000, 0xC000));
		}
	}			
	return;
}

void Memory::storeRAM() {
	copyToRAM(ramBank*0x2000, 0xA000, getSize(0xA000, 0xC000));
}

void Memory::loadRAM() {
	copyFromRAM(0xA000, ramBank*0x2000, getSize(0xA000, 0xC000));
}

bool Memory::isBanking(uint16_t addr, uint8_t value) {
	if (addr <= 0x7FFF) {
		bankUnit(addr, value);
		return true;
	}

	return false;
}

/* Trigger Events for read request */
// INFO: Not implemented yet, as there is only one workaround case for read requests
uint8_t Memory::triggerEvent(uint16_t addr) {
	switch(addr) {
		case 0xFF01:
			return 0xFF;
		default:
			return 0x0;
	}
}

/* Trigger Events for write requests */
void Memory::triggerEvent(uint16_t addr, uint8_t value) {
	switch(addr) {
		// Write requested layout for joypads to memory
		case 0xFF00:
			Joypad::triggerLayoutChange(value);
			return;
		
		// Linkport I/O
		case 0xFF02:
			if (value & 0x3F) {
				memory[0xFF02] = memory[0xFF02] & 0x3F;
				uint8_t current = read_8u(0xFF0F);
				write_8u(0xFF0F, (current | (1 << 3)));
			}
			return;
		
		// Reset timer divider to zero
		case 0xFF04:
			memory[addr] = 0;
			return;
		
		// Reset scanline to zero
		case 0xFF44:
			memory[addr] = 0;
			return;
		
		// Trigger DMA Transfer for Sprite Attributes table
		case 0xFF46:
			DMATransfer(value);
			return;
		
		// If no trigger is available for address, return
		default:
			return;
	}
}

void Memory::DMATransfer(uint8_t data) {
	uint16_t addr = data << 8;
	for(int i = 0; i < 0xA0; i++) {
		memory[0xFE00+i] = memory[addr+i];
	}
}


/* Read 8bit */
uint8_t Memory::read_8u(uint16_t addr) {
	/* Linkport I/O return 0xFF for unused serial interface */
	if (addr == 0xFF01) {
		return 0xFF;
	}

	return memory[addr];
}

int8_t Memory::read_8s(uint16_t addr) {
	return (int8_t) read_8u(addr);
}

/* Write 8bit */
void Memory::write_8u(uint16_t addr, uint8_t value) {
	// Prevent writing to ROM by MBC and perform bank switch
	if (isBanking(addr, value)) {
		return;
	}

	
	// Write to memory
	memory[addr] = value;

	// Trigger Events
	triggerEvent(addr, value);
	
}

void Memory::privilegedWrite8u(uint16_t addr, uint8_t value) {
	memory[addr] = value;
}

/* Read 16bit in correct endianness */
uint16_t Memory::read_16u(uint16_t addr) {
	return memory[addr+1] << 8 | memory[addr];
}

int16_t Memory::read_16s(uint16_t addr) {
	return (int16_t) read_16u(addr);
}

/* Write 16bit in correct endianess */
void Memory::write_16u(uint16_t addr, uint16_t value) {
	// write L to addr and H to addr+1
	memory[addr] = (value & 0xFF);
	memory[addr+1] = (value >> 8);
}

void Memory::writeCartridge8u(uint32_t addr, uint8_t value) {
	if (addr <= CARTRIDGE_SIZE) {
		cartridge[addr] = value;
	}
}

uint8_t Memory::readCartridge8u(uint32_t addr) {
	if (addr <= CARTRIDGE_SIZE) {
		return cartridge[addr];
	}
	else { 
		return 0x0;
	}
}

uint16_t Memory::readCartridge16u(uint32_t addr) {
	if (addr < CARTRIDGE_SIZE) {
		return cartridge[addr+1] << 8 | cartridge[addr];
	}
	else {
		return 0x0;
	}
}

void Memory::writeRAM8u(uint32_t addr, uint8_t value) {
	if (addr <= RAM_SIZE) {
		ram[addr] = value;
	}
}

uint8_t Memory::readRAM8u(uint32_t addr) {
	if (addr <= RAM_SIZE) {
		return ram[addr];
	}
	else { 
		return 0x0;
	}
}

/* Helper Methods */
void Memory::copyFromCartridge(uint16_t dest, uint32_t src, uint16_t size) {
	memcpy(&memory[dest], &cartridge[src], size);
}

void Memory::copyFromRAM(uint16_t dest, uint16_t src, uint16_t size) {
	memcpy(&memory[dest], &ram[src], size);
}

void Memory::copyToRAM(uint16_t dest, uint16_t src, uint16_t size) {
	memcpy(&ram[dest], &memory[src], size);
}

uint16_t Memory::getSize(uint16_t start, uint16_t end) {
	return (end-start);
}

uint32_t* Memory::getMemoryPointer() {
	return (uint32_t*) memory;
}
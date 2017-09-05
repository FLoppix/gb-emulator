#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>

#define MEM_SIZE 0xFFFF+1
#define CARTRIDGE_SIZE 0x800000+1
#define RAM_SIZE 0x8000+1


class Memory {
private:
	/* Attributes */
	uint8_t cartridge[CARTRIDGE_SIZE] = {};
	uint8_t memory[MEM_SIZE] = {};
	uint8_t ram[RAM_SIZE] = {};

	/* true, when cartridge is remapped to the first 128kb */
	bool isMapped;

	/* Set MBC type by reading Cartridge Header */
	uint8_t bankingController;

	/* true, when ram banking is enabled */
	bool isRAMEnabled;

	/* Select if higher two bits change RAM or ROM bank */
	uint8_t bankMode;

	/* The current romBank. This is not allowed to be 0 */
	uint8_t romBank;
	
	/* The current ramBank. This can be 0-3 */
	uint8_t ramBank;

public:
	/* Constrcutor */
	Memory();

	/* Bank Unit */
	void initialize();

	/* Remap ROM with Cartridge */
	void remapUnit();

	/* Select and Copy select memory bank */
	void bankUnit(uint16_t addr, uint8_t value);

	/* Handle MBC Banking */
	void handleMBC1(uint16_t addr, uint8_t value);
	void handleMBC2(uint16_t addr, uint8_t value);
	void handleMBC3(uint16_t addr, uint8_t value);
	void handleMBC4(uint16_t addr, uint8_t value);
	void handleMBC5(uint16_t addr, uint8_t value);

	/* Copy current bank to RAM */
	void storeRAM();

	/* Copy current RAM to memory */
	void loadRAM();

	/* Check if memory address is in ROM and only affects Banking */
	bool isBanking(uint16_t addr, uint8_t value);

	/* Trigger Event for reads to special memory addresses */
	uint8_t triggerEvent(uint16_t addr);

	/* Trigger Events for writes to special memory addresses */
	void triggerEvent(uint16_t addr, uint8_t value);

	/* Do DMA Transfer */
	void DMATransfer(uint8_t data);

	/* Methods */
	uint8_t read_8u(uint16_t addr);
	int8_t read_8s(uint16_t addr);

	void write_8u(uint16_t addr, uint8_t value);
	void privilegedWrite8u(uint16_t addr, uint8_t value);

	uint16_t read_16u(uint16_t addr);
	int16_t read_16s(uint16_t addr);

	void write_16u(uint16_t addr, uint16_t value);

	void writeCartridge8u(uint32_t addr, uint8_t value);
	uint8_t readCartridge8u(uint32_t addr);
	uint16_t readCartridge16u(uint32_t addr);

	void writeRAM8u(uint32_t addr, uint8_t value);
	uint8_t readRAM8u(uint32_t addr);

	/* Helper Methods */
	void copyFromCartridge(uint16_t dest, uint32_t src, uint16_t size);

	void copyFromRAM(uint16_t dest, uint16_t src, uint16_t size);
	void copyToRAM(uint16_t dest, uint16_t src, uint16_t size);


	uint16_t getSize(uint16_t start, uint16_t end);

	uint32_t* getMemoryPointer();

};

#endif /* MEMORY_H */
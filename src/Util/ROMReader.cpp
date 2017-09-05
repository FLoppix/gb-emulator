#include "../Component/ROMReader.h"
#include <iostream>

Memory* ROMReader::mem = NULL;

void ROMReader::setMemory(Memory* m) {
	mem = m;
}

void ROMReader::copy(string rom) {
	FILE *fileptr;
	long filelen;
	long i;
	int8_t buffer;

	fileptr = fopen(rom.c_str(), "rb");

	fseek(fileptr, 0, SEEK_END);
	filelen = ftell(fileptr);         
	rewind(fileptr);
							
	for(i = 0; i < filelen; i++) {
		if(fread(&buffer, 1, 1, fileptr)) {
			mem->privilegedWrite8u((uint16_t) i, (uint8_t) buffer);
		}
	}

	fclose(fileptr);
}

void ROMReader::load(string rom) {
	FILE *fileptr;
	long filelen;
	long i;
	int8_t buffer;

	fileptr = fopen(rom.c_str(), "rb");

	fseek(fileptr, 0, SEEK_END);
	filelen = ftell(fileptr);         
	rewind(fileptr);
			
	for(i = 0; i < filelen; i++) {
		if(fread(&buffer, 1, 1, fileptr)) {
			mem->writeCartridge8u((uint32_t) i, (uint8_t) buffer);	
		}
	}

	fclose(fileptr);
}

string ROMReader::getRomName(string rom) {
	FILE *fileptr;
	char buffer;

	fileptr = fopen(rom.c_str(), "rb");
	fseek(fileptr, 0x134, SEEK_SET);

	string romName;
	for(uint i = 0x134; i <= 0x143U; i++) {
		if(fread(&buffer, 1, 1, fileptr)) {
			if(buffer != '\0') {
				romName += buffer;
			} else {
				break;
			}
		}
	}

	fclose(fileptr);

	return romName;
}

void ROMReader::dumpSavegame(string file) {
	FILE *fileptr;
	fileptr = fopen(file.c_str(), "wb");

	mem->storeRAM();

	for(long i = 0; i < RAM_SIZE; i++) {
		fputc(mem->readRAM8u(i), fileptr);
	}

	fclose(fileptr);
}

bool ROMReader::tryToLoadSavegame(string file) {
	FILE *fileptr;
	long filelen;
	long i;
	int8_t buffer;

	fileptr = fopen(file.c_str(), "rb");

	if(fileptr == NULL) {
		return false;
	}

	fseek(fileptr, 0, SEEK_END);
	filelen = ftell(fileptr);         
	rewind(fileptr);
			
	for(i = 0; i < filelen; i++) {
		if(fread(&buffer, 1, 1, fileptr)) {
			mem->writeRAM8u((uint32_t) i, (uint8_t) buffer);
		}
	}

	mem->loadRAM();

	fclose(fileptr);
	return true;
}
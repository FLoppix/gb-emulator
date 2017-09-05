#ifndef ROMREADER_H
#define ROMREADER_H

#include <cstdint>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "../Component/Memory.h"

using namespace std;

class ROMReader {
private:
	/* Attributes */
	static Memory* mem;

	/* Private constructor to prevent initialisation */
	ROMReader() {};

public:
	/* Methods */
	static void setMemory(Memory* m);

	static void copy(string file);
	static void load(string file);
	static string getRomName(string file);
	static void dumpSavegame(string file);
	static bool tryToLoadSavegame(string file);
};

#endif /* ROMREADER_H */
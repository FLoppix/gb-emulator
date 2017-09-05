#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>

class CPU;

struct instructions {
    char name[25];
    unsigned char size;
    unsigned char length;
    unsigned char ticks;
    uint8_t (CPU::*function)();
};


extern const instructions instruction[256];

#endif /* INSTRUCTION_H */
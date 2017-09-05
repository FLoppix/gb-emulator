#ifndef TIMER_H
#define TIMER_H

#define DIV 0xFF04  // Divider Register(R/W)
#define TIMA 0xFF05 // Timer Counter(R/W)
#define TMA 0xFF06  // Timer Modulo (R/W)

/*
 Bit  2   - Timer Enable
 Bits 1-0 - Input Clock Select
            00: CPU Clock / 1024 (DMG, CGB:   4096 Hz, SGB:   ~4194 Hz)
            01: CPU Clock / 16   (DMG, CGB: 262144 Hz, SGB: ~268400 Hz)
            10: CPU Clock / 64   (DMG, CGB:  65536 Hz, SGB:  ~67110 Hz)
            11: CPU Clock / 256  (DMG, CGB:  16384 Hz, SGB:  ~16780 Hz)
 
 Note: The "Timer Enable" bit only affects the timer, the divider is ALWAYS counting.
*/
#define TMC 0xFF07  // Timer Controller

#include "Memory.h"

class Timer {
private:
    Memory* mem;
    int timerCounter;
    int dividerCounter;
    uint8_t currentFreq;

public:
    Timer(Memory* mem);

    void update(uint8_t cycles);

private:
    bool isClockEnabled();
    uint8_t getClockFreq();
    void setClockFreq();
    void checkFreqChange();

    void updateDividerRegister(uint8_t cycles);

    void triggerInterrupt();
};



#endif /* TIMER_H */
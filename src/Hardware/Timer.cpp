#include "../Component/Timer.h"

#include "../Component/Interrupts.h"


/* Constructor */
Timer::Timer(Memory* m) {
    mem = m;
    currentFreq = 0;
    timerCounter = 1024;
    dividerCounter = 0;
}

bool Timer::isClockEnabled() {
    return mem->read_8u(TMC) & (1 << 2);

}

uint8_t Timer::getClockFreq() {
    return mem->read_8u(TMC) & 0x3;
}

void Timer::setClockFreq() {
    currentFreq = getClockFreq();
    switch (currentFreq) {
        case 0: timerCounter = 1024; break;
        case 1: timerCounter = 16;   break;
        case 2: timerCounter = 64;   break;
        case 3: timerCounter = 256;  break;
    }
}

void Timer::updateDividerRegister(uint8_t cycles) {
    dividerCounter += cycles;
    
    if (dividerCounter >= 255) {
        dividerCounter = 0;
        mem->privilegedWrite8u(DIV, mem->read_8u(DIV)+1);
    }
}

void Timer::checkFreqChange() {
    if (currentFreq != getClockFreq()) {
        setClockFreq();
    }
}

void Timer::update(uint8_t cycles) {
    checkFreqChange();
    updateDividerRegister(cycles);

    if (isClockEnabled()) {
        timerCounter -= cycles;

        if (timerCounter <= 0) {
            setClockFreq();

            if (mem->read_8u(TIMA) == 0xFF) {
                mem->write_8u(TIMA, mem->read_8u(TMA));
                triggerInterrupt();
            }
            else {
                mem->write_8u(TIMA, mem->read_8u(TIMA)+1);
            }
        }
    }
}


void Timer::triggerInterrupt() {
	uint8_t current = mem->read_8u(INTERRUPT_REQUEST_REGISTER);
	mem->write_8u(INTERRUPT_REQUEST_REGISTER, (current | INTERRUPT_TIMER));
}
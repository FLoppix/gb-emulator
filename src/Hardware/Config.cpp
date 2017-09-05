#include "../Component/Config.h"

bool Config::debug = false;

bool Config::waiting = false;
bool Config::equals = false;
uint16_t Config::waitPc = 0;


bool Config::moveToEnabled = false;
uint16_t Config::moveToCounter = 0;
uint16_t Config::counter = 0;

void Config::enableDebug() {
    debug = true;
}

void Config::disableDebug() {
    debug = false;
}

bool Config::isDebug() {
    return debug;
}

void Config::disableWaiting() {
    waiting = false;
}

void Config::waitForPC(uint16_t pc, bool eq) {
    waiting = true;
    waitPc = pc;
    equals = eq;
}

bool Config::isWaiting(uint16_t pc) {
    if (waiting) {
        if (equals) {
            if (waitPc != pc) {
                return true;
            } else {
                waiting = false;
                equals = false;
                waitPc = 0;
                return false;
            }
        } else {
            if (waitPc > pc ) {
                return true;
            } else {
                waiting = false;
                equals = false;
                waitPc = 0;
                return false;
            }
        }
    }
    return false;
}

void Config::enableCounter(uint16_t c) {
    moveToEnabled = true;
    counter = 0;
    moveToCounter = c;
}

bool Config::isCounting() {
    if (!moveToEnabled) {
        return true;
    }

    if (counter < moveToCounter) {
        counter++;
        return false;
    } else {
        moveToEnabled = false;
        counter = 0;
        moveToCounter = 0;
        return false;
    }
}

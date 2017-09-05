#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

class Config {
private:
    static bool debug;

    static bool waiting;
    static bool equals;
    static uint16_t waitPc;

    static bool moveToEnabled;
    static uint16_t moveToCounter;
    static uint16_t counter;

public:
    static void enableDebug();
    static void disableDebug();
    static bool isDebug();
    static void waitForPC(uint16_t pc, bool eq);
    static bool isWaiting(uint16_t pc);
    static void disableWaiting();
    
    static void enableCounter(uint16_t c);
    static bool isCounting();
};

#endif /* CONFIG_H */
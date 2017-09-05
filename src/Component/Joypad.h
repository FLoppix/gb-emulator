#ifndef JOYPAD_H
#define JOYPAD_H

#include "Memory.h"


class Joypad {
private:
	/* Private constructor to prevent instantiation of this class */
	Joypad() {}

public:
	/* Enum */
	enum class Button {A, B, Select, Start, Right, Left, Up, Down, X, Y};

    static bool buttons[10];

    static Memory* mem;

	/* Methods */
	static void pressButton(Button button);
	static void releaseButton(Button button);
	static void triggerLayoutChange(uint8_t reg);
	static bool isAnyButtonPressed();

};

#endif /* JOYPAD_H */
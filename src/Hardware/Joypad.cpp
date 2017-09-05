#include "../Component/Joypad.h"

/* Instantiate static member variable */
bool Joypad::buttons[] = {false};
Memory* Joypad::mem;

void Joypad::pressButton(Button b) {
	bool buttonChanged = !buttons[static_cast<int>(b)];

	buttons[static_cast<int>(b)] = true;

	// If additional button (X or Y)
	if(static_cast<int>(b) > 7) {
		return;
	}

	uint8_t value = mem->read_8u(0xFF00);

	bool buttonInteresting = false;

	// If button is currently interesting -> clear bit (since 0 means pressed)
	if (((value >> 5) & 1) && (b == Button::A || b == Button::B || b == Button::Start || b == Button::Select)) {
		value &= ~(1 << static_cast<int>(b));
		buttonInteresting = true;
	}
	else if (((value >> 4) & 1) && (b == Button::Up || b == Button::Down || b == Button::Left || b == Button::Right)) {
		value &= ~(1 << (static_cast<int>(b) - 4));	
		buttonInteresting = true;
	}

	mem->privilegedWrite8u(0xFF00, value);

	// Trigger interrupt
	if (buttonChanged && buttonInteresting) {
		uint8_t current = mem->read_8u(0xFF0F);
		mem->write_8u(0xFF0F, (current | 0x60));
	}

}

void Joypad::releaseButton(Button b) {
	buttons[static_cast<int>(b)] = false;

	// If additional button (X or Y)
	if(static_cast<int>(b) > 7) {
		return;
	}

	uint8_t value = mem->read_8u(0xFF00);

	// If button is currently interesting -> set bit (since 1 means not pressed)
	if (((value >> 4) & 1) && (b == Button::A || b == Button::B || b == Button::Start || b == Button::Select)) {
		value |= 1 << static_cast<int>(b);
	}
	else if (((value >> 5) & 1) && (b == Button::Up || b == Button::Down || b == Button::Left || b == Button::Right)) {
		value |= 1 << (static_cast<int>(b) - 4);	
	}

	mem->privilegedWrite8u(0xFF00, value);	
}

void Joypad::triggerLayoutChange(uint8_t value) {
	// Standard buttons
	if ((value >> 4) & 1) {
		value = (value & ~(1 << 0)) | (!buttons[0] << 0);
		value = (value & ~(1 << 1)) | (!buttons[1] << 1);
		value = (value & ~(1 << 2)) | (!buttons[2] << 2);
		value = (value & ~(1 << 3)) | (!buttons[3] << 3);
	}
	// Directional buttons
	else if ((value >> 5) & 1) {
		value = (value & ~(1 << 0)) | (!buttons[4] << 0);
		value = (value & ~(1 << 1)) | (!buttons[5] << 1);
		value = (value & ~(1 << 2)) | (!buttons[6] << 2);
		value = (value & ~(1 << 3)) | (!buttons[7] << 3);
	}

	// Check for valid layout bits
	if ((value & 0x30 ) != 0x10 && (value & 0x30 ) != 0x20) {
		value = 0xF;
	}

	mem->privilegedWrite8u(0xFF00, value);	
}

bool Joypad::isAnyButtonPressed() {
	for(uint8_t i = 0; i < 8; i++) {
		if (buttons[i] == true) {
			return true;
		}
	}

	return false;
}
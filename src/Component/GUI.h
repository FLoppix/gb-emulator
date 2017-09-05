#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h>
#include <stdio.h>

#include "Memory.h"

// Screen dimension constants
const int WIDTH = 160;
const int HEIGHT = 144;

class GUI {
private:
	/* Attributes */
	uint factor;

    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_Texture* texture;

    /* Button Mapping */
    SDL_Keycode buttonA;
    SDL_Keycode buttonB;
    SDL_Keycode buttonStart;
    SDL_Keycode buttonSelect;
    SDL_Keycode buttonUp;
    SDL_Keycode buttonDown;
    SDL_Keycode buttonLeft;
    SDL_Keycode buttonRight;
    SDL_Keycode buttonX;
    SDL_Keycode buttonY;

public:
	/* Constructor and Destructor */
	GUI();
	~GUI();

	/* Methods */
	void render(uint8_t framebuffer[][WIDTH]);
	uint8_t getPixelColor(uint8_t, uint8_t, uint8_t);
	uint32_t getColor(uint8_t c);
	void handleEvents();
};

#endif /* GUI_H */
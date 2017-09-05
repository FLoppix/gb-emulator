#include "../Component/GUI.h"
#include "../Component/Config.h"
#include "../Component/Joypad.h"

#include <SDL2/SDL.h>

#include <iostream>
#include <unistd.h>

/* Constructor */
GUI::GUI() {
	/* Hardcode factor here for now */
	factor = 4;

	buttonA = SDLK_y;
	buttonB = SDLK_x;
	buttonStart = SDLK_a;
	buttonSelect = SDLK_s;
	buttonUp = SDLK_UP;
	buttonDown = SDLK_DOWN;
	buttonLeft = SDLK_LEFT;
	buttonRight = SDLK_RIGHT;
	buttonX = SDLK_n;
	buttonY = SDLK_m;

	SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(factor * WIDTH, factor * HEIGHT, 0, &window, &renderer);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

GUI::~GUI() {
	SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GUI::render(uint8_t framebuffer[][WIDTH]) {
	void *pixels;
	int pitch;

	SDL_LockTexture(texture, NULL, &pixels, &pitch);

	for(uint y = 0; y < HEIGHT; y++) {
		uint32_t* dst = (uint32_t*)((uint8_t*)pixels + y * pitch);
		for(uint x = 0; x < WIDTH; x++) {
			*dst++ = getColor(framebuffer[y][x]);
		}
	}

	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_UnlockTexture(texture);
	SDL_RenderPresent(renderer);
}	

uint8_t GUI::getPixelColor(uint8_t b1, uint8_t b2, uint8_t x) {
	// Read both tile lines from memory
	uint8_t tileLine1 = b1;
	uint8_t tileLine2 = b2;

	// Calculate pixel position in tile
	int8_t bit = x % 8;

	// Convert tile position to bit inversion order: pos ---X- ---- should be: 0000 0100
	bit = (bit-7)*(-1);

	// Combine both tile lines for color value
	uint8_t color = ((tileLine2 >> bit) & 0x1) << 1;
	color |= (tileLine1 >> bit) & 0x1;

	return color;
}

uint32_t GUI::getColor(uint8_t c) {
	switch(c) {
		case 0:
			return 0x9CBD0FFF;
		case 1:
			return 0x8CAD0FFF;
		case 2:
			return 0x306230FF;
		case 3:
			return 0x0F380FFF;
		default:
			return 0x0F380FFF;
	}
}

void GUI::handleEvents() {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_QUIT) {
			exit(0);
		}

		else if(event.type == SDL_KEYDOWN) {
			/* Key P for debugging */
			if(event.key.keysym.sym == SDLK_p) {
				Config::enableDebug();
				Config::disableWaiting();
			}

			if(event.key.keysym.sym == buttonA) {
				Joypad::pressButton(Joypad::Button::A);
			}
			else if(event.key.keysym.sym == buttonB) {
				Joypad::pressButton(Joypad::Button::B);
			}
			else if(event.key.keysym.sym == buttonStart) {
				Joypad::pressButton(Joypad::Button::Start);
			}
			else if(event.key.keysym.sym == buttonSelect) {
				Joypad::pressButton(Joypad::Button::Select);
			}
			else if(event.key.keysym.sym == buttonUp) {
				Joypad::pressButton(Joypad::Button::Up);
			}
			else if(event.key.keysym.sym == buttonDown) {
				Joypad::pressButton(Joypad::Button::Down);
			}
			else if(event.key.keysym.sym == buttonLeft) {
				Joypad::pressButton(Joypad::Button::Left);
			}
			else if(event.key.keysym.sym == buttonRight) {
				Joypad::pressButton(Joypad::Button::Right);
			}
			else if(event.key.keysym.sym == buttonX) {
				Joypad::pressButton(Joypad::Button::X);
			}
			else if(event.key.keysym.sym == buttonY) {
				Joypad::pressButton(Joypad::Button::Y);
			}
		}
		else if(event.type == SDL_KEYUP) {
			if(event.key.keysym.sym == buttonA) {
				Joypad::releaseButton(Joypad::Button::A);
			}
			else if(event.key.keysym.sym == buttonB) {
				Joypad::releaseButton(Joypad::Button::B);
			}
			else if(event.key.keysym.sym == buttonStart) {
				Joypad::releaseButton(Joypad::Button::Start);
			}
			else if(event.key.keysym.sym == buttonSelect) {
				Joypad::releaseButton(Joypad::Button::Select);
			}
			else if(event.key.keysym.sym == buttonUp) {
				Joypad::releaseButton(Joypad::Button::Up);
			}
			else if(event.key.keysym.sym == buttonDown) {
				Joypad::releaseButton(Joypad::Button::Down);
			}
			else if(event.key.keysym.sym == buttonLeft) {
				Joypad::releaseButton(Joypad::Button::Left);
			}
			else if(event.key.keysym.sym == buttonRight) {
				Joypad::releaseButton(Joypad::Button::Right);
			}
			else if(event.key.keysym.sym == buttonX) {
				Joypad::releaseButton(Joypad::Button::X);
			}
			else if(event.key.keysym.sym == buttonY) {
				Joypad::releaseButton(Joypad::Button::Y);
			}
		}
	}
}
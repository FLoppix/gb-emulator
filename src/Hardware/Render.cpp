#include "../Component/Render.h"
#include "../Component/Memory.h"
#include <thread>
#include <chrono>

#define MAX_WIDTH 256
#define MAX_HEIGHT 256
#define VIEW_WIDTH 160
#define VIEW_HEIGHT 144
#define RENDER_SDL_ 1

Render::Render(Memory* m){
	mem = m;
	data = (uint8_t*) malloc(256 * 256 * 2 / 8); // 256 x 256 screen size, each pixel consists of 2 bit 
	v_mem = (struct video_memory *) (((uint64_t) (mem->getMemoryPointer())) + 0x8000);
	o_mem = (struct oam_memory *) (((uint64_t) (mem->getMemoryPointer())) + 0xFE00);

    color_palette = (uint8_t* ) malloc(12); // [(255,255,255), (192, 192, 192),(96,96,96), (0,0,0)] #
	color_palette[0] = 255;
	color_palette[1] = 255;
	color_palette[2] = 255;

	color_palette[3] = 192;
	color_palette[4] = 192;
	color_palette[5] = 192;

	color_palette[6] = 96;
	color_palette[7] = 96;
	color_palette[8] = 96;

	color_palette[9] = 0;
	color_palette[10] = 0;
	color_palette[11] = 0;


	#ifdef RENDER_SDL_
		printf("Init SDL Video Transfer\n");
		SDL_Init(SDL_INIT_VIDEO); 
	    // create the window

	    this->window = SDL_CreateWindow("Game",SDL_WINDOWPOS_UNDEFINED,
	        SDL_WINDOWPOS_UNDEFINED, 256, 256, SDL_WINDOW_SHOWN);

	    // Check that the window was successfully created
	    if (this->window == NULL) {
	        // In the case that the window could not be made...
	        printf("Could not create window: %s\n", SDL_GetError());
	        return;
	    }

	    this->renderer = SDL_CreateRenderer(this->window, -1, 0);
	    SDL_RenderClear(this->renderer);
	#else
	    printf("Init FPGA Video Transfer \n");
	#endif

}
void Render::renderFrame() {
		
	build_map(v_mem, o_mem, data, true);
	render(data, color_palette, 0, 0);

}

/**
	@param v_mem video memory struct
	@param data bitstream data where to write to
	@bool map_1 get the background content from map_1, true or false

	Returns the complete 256x256 map as a bitstream with 2 bit a colour
**/
void Render::build_map(struct video_memory* v_mem, struct oam_memory* oam_mem, uint8_t* data, bool map_1){
	/* Extracts the tile number from each map entry and get the bitstream for the background */
	int index = 0;
	uint8_t tile, mask, p = 0;
	uint32_t d_addr = (uint64_t) data;
	uint32_t dest, source;
	for (int y= 0; y<0x20; y++)
	{
		for (int x= 0; x<0x20; x++) {
			if (map_1)
				tile = v_mem->bg_map_data_1[y][x];
			else
				tile = v_mem->bg_map_data_2[y][x];
			// 1 tile is 8x8 pixels , each pixel 2 bit
			// 1 row is 2 byte = 2 * 8 pixel
			for (int i = 0; i<0x10; i+=2) {

				dest = d_addr + x * 2 + y * MAX_WIDTH/4 + i/2 * MAX_WIDTH/4;
				source = ((uint64_t) v_mem->tiles) + tile * 0x10 + i;
				memcpy((uint32_t*)(dest), (uint32_t*)(source), 1);
				memcpy((uint32_t*)(dest + 1), (uint32_t*)(source + 1), 1);
				index+= 2;
				// extract the palett number
				/**
				for (int k = 0; k<0x4; k++) {
					// print entries of the tile
					mask = (1 << (k*2) | 1 << (k*2+1));
					p = (v_mem->tiles[tile][i] & mask) >> (k*2);
					//printf("value: %i\n", p);
					//std::this_thread::sleep_for (std::chrono::seconds(1));
					//printf("Write %i at %i\n", (p << (k*2)), bit_index / 8);
					data[bit_index / 8] |= (p << (k*2));
					bit_index += 2;
				}
			}*/
			}
		}    
	}
	/* Iterate through all the sprites and override the background with the foreground if nessessary */


}

uint8_t* Render::conv_to_rgb(uint8_t value) {
	uint8_t* rgb;
	return rgb;
}

void Render::sdl_put_pixel_rgb(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	SDL_SetRenderDrawColor(this->renderer, r, g, b, 255);
	SDL_RenderDrawPoint(this->renderer, x, y);	
}

/**
	needs a 12 byte color palette and a 256 * 256 * 2 / 8 byte data
**/

void Render::render(uint8_t* data, uint8_t* color_palette, uint8_t offset_x, uint8_t offset_y)
{
	#ifdef RENDER_SDL_
		printf("Render SDL\n");
		this->render_sdl(data, color_palette, offset_x, offset_y);
	#else
		printf("Render for FPGA \n");
		this->render_fpga(data, color_palette, offset_x, offset_y);
	#endif
}

Render::~Render() {

	#ifdef RENDER_SDL_pp
		SDL_DestroyRenderer(this->renderer);
	    SDL_DestroyWindow(this->window);
	    SDL_Quit();
	#endif
}

void Render::render_fpga(uint8_t* data, uint8_t* color_palette, uint8_t offset_x, uint8_t offset_y) {
	// do nothing so far
}
void Render::render_sdl(uint8_t* data, uint8_t* color_palette, uint8_t offset_x, uint8_t offset_y) {
	
	// Clear the view
	SDL_RenderClear(this->renderer);

   	// Render here
	int bit_index, color_index, mask = 0;
    for (int y = 0; y<256; y++){
		for (int x = 0; x <256; x++){
			bit_index = y * 2 + x * 2;
			mask = (1 << (bit_index % 8) | 1 << (bit_index % 8 + 1));
			color_index = (data[bit_index / 8] & mask) >> (bit_index % 8);
			this->sdl_put_pixel_rgb(x, y, color_palette[color_index * 3], color_palette[color_index * 3 + 1], color_palette[color_index * 3 + 2]);
		}
	}

    // Set it to visible
	SDL_RenderPresent(this->renderer);
	
}
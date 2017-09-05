#ifndef RENDER_H
#define RENDER_H

#include "Memory.h"

#include <cstdint>
#include <stdint.h>


// install libsdl2-dev
#include <SDL2/SDL.h>
#include <stdio.h>
const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 256;


/* Sprites masks **
    Bit7   OBJ-to-BG Priority (0=OBJ Above BG, 1=OBJ Behind BG color 1-3)
         (Used for both BG and Window. BG color 0 is always behind OBJ)
  Bit6   Y flip          (0=Normal, 1=Vertically mirrored)
  Bit5   X flip          (0=Normal, 1=Horizontally mirrored)
  Bit4   Palette number  **Non CGB Mode Only** (0=OBP0, 1=OBP1)
  Bit3   Tile VRAM-Bank  **CGB Mode Only**     (0=Bank 0, 1=Bank 1)
  Bit2-0 Palette number  **CGB Mode Only**     (OBP0-7)

*/
#define ATTR_BG_P_NUM 0x7
#define ATTR_VRAM_BANK_NUM (1 << 3)
#define ATTR_PALETT_NUM (1 << 4)
#define ATTR_HORI_FLIP (1 << 5)
#define ATTR_VERT_FLIP (1 << 6)
#define ATTR_PRIO (1 << 7)

/* 
0x8000-0x97FF - Character RAM
0x9800-0x9BFF - BG Map Data 1
0x9C00-0x9FFF - BG Map Data 2
-> character ram holds 384 tiles of which 256 are unique
-> gbc uses 384 tiles, gb only 192 tiles
-> tiles are numbered from 0 to 255 and -128 to 127
    -> each tile holds 8x8 pixels, each pixel has a 2 bit color depth
-> background maps hold the number of the tiles to be drawnd */
struct video_memory {

    uint8_t tiles[384][16]; // 2 * 8 * 8 = 128 bit => 16 byte
    uint8_t bg_map_data_1[32][32];
    uint8_t bg_map_data_2[32][32];
    
};


/* Resolution   - 160x144 (20x18 tiles) */

const int width = 160;
const int height = 144;


/* sprite numbers are always unsigned 
    Byte0  Y position on the screen
    Byte1  X position on the screen
    Byte2  Pattern number 0-255 [notice that unlike tile numbers, sprite
            pattern numbers are unsigned] 
    Byte3  Flags:
            Bit7  Priority
                Sprite is displayed in front of the window if this bit
                is set to 1. Otherwise, sprite is shown behind the
                window but in front of the background.
            Bit6  Y flip
                Sprite pattern is flipped vertically if this bit is
                set to 1.
            Bit5  X flip
                Sprite pattern is flipped horizontally if this bit is
                set to 1.
            Bit4  Palette number
                Sprite colors are taken from OBJ1PAL if this bit is
                set to 1 and from OBJ0PAL otherwise.

*/

/* 3 colors, because color 0 is transparent */
struct sprite {
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t pattern_number;
    uint8_t attributes;
};

/* 0xFE00-0xFE9F - Object Attribute Memory (OAM) */
struct oam_memory {
    sprite sprites[40];
};


/* Tiles is 8x8 bit array */

class Render {

    public:
        Render(Memory * mem); 
        ~Render();
        void renderFrame();
    private:
        void build_map(struct video_memory *v_mem, struct oam_memory* oam_mem, uint8_t* data, bool map_1);
        void render(uint8_t* data, uint8_t* color_palette, uint8_t offset_x, uint8_t offset_y);
        void render_sdl(uint8_t* data, uint8_t* color_palette, uint8_t offset_x, uint8_t offset_y);
        void render_fpga(uint8_t* data, uint8_t* color_palette, uint8_t offset_x, uint8_t offset_y);
        void sdl_put_pixel_rgb(int x, int y, unsigned char r, unsigned char g, unsigned char b);
        uint8_t* conv_to_rgb(uint8_t value);
        uint8_t* data;
        uint8_t* color_palette ;
        struct video_memory* v_mem;
        struct oam_memory * o_mem;
        Memory* mem;
        
        
        // SDL renderer
        SDL_Window* window;
        SDL_Renderer* renderer;
        bool tile_table_num; // 0 = $8000-8FFF (unsigned), 1 = $8800-97FF (signed)
};


#endif
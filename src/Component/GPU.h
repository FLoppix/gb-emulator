#ifndef GPU_H
#define GPU_H

#include "Memory.h"
#include "GUI.h"

#include <queue>

#define SPRITE_OAM 0xFE00

/*
    LCD goes through to 4 different modes 
    00: H-Blank will take the remaining clock cycles of the 456 (see Mode 10 and 11)
    01: V-Blank 
    10: Searching Sprites Atts will take the first 80 of the 456 clock cycles. 
    11: Transfering Data to LCD Driver will take 172 clock cycles of the 456 clock cycles.
*/

/* LCD Modes
  Mode 0: The LCD controller is in the H-Blank period and
          the CPU can access both the display RAM (8000h-9FFFh)
          and OAM (FE00h-FE9Fh) [VRAM 8000-9FFF can be accessed by CPU]

  Mode 1: The LCD contoller is in the V-Blank period (or the
          display is disabled) and the CPU can access both the
          display RAM (8000h-9FFFh) and OAM (FE00h-FE9Fh)

  Mode 2: The LCD controller is reading from OAM memory.
          The CPU <cannot> access OAM memory (FE00h-FE9Fh)
          during this period.
          OAM FE00-FE90 is accessed by LCD controller

  Mode 3: The LCD controller is reading from both OAM and VRAM,
          The CPU <cannot> access OAM and VRAM during this period.
          CGB Mode: Cannot access Palette Data (FF69,FF6B) either.
          Both OAM FE00-FE90 and VRAM 8000-9FFF are accessed by LCD controller
*/

#define LCD_STAT_REG 0xFF41

/*
  Bit 7 - LCD Display Enable             (0=Off, 1=On)
  Bit 6 - Window Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
  Bit 5 - Window Display Enable          (0=Off, 1=On)
  Bit 4 - BG & Window Tile Data Select   (0=8800-97FF, 1=8000-8FFF) Window Tile Pattern table
  Bit 3 - BG Tile Map Display Select     (0=9800-9BFF, 1=9C00-9FFF) 
  Bit 2 - OBJ (Sprite) Size              (0=8x8, 1=8x16)
  Bit 1 - OBJ (Sprite) Display Enable    (0=Off, 1=On)
  Bit 0 - BG Display (for CGB see below) (0=Off, 1=On)
*/

/* LCD Logic based on http://www.codeslinger.co.uk/pages/projects/gameboy/lcd.html */

#define LCD_CTRL_REG 0xFF40

#define LCD_CUR_SCANLINE 0xFF44
#define LCD_LYC 0xFF45

#define SCROLL_Y 0xFF42
#define SCROLL_X 0xFF43
#define WINDOW_Y 0xFF4A
#define WINDOW_X 0xFF4B /* WINDOW X Position - 7 !!!!! */

/* This register assigns gray shades to the color numbers of the BG and Window tiles.
  Bit 7-6 - Shade for Color Number 3
  Bit 5-4 - Shade for Color Number 2
  Bit 3-2 - Shade for Color Number 1
  Bit 1-0 - Shade for Color Number 0
The four possible gray shades are:
  0  White
  1  Light gray
  2  Dark gray
  3  Black
*/
#define MONOCHROME_COLOR_PALETTE 0xFF47 
#define OBJECT_PALETTE_0 0xFF48 
#define OBJECT_PALETTE_1 0xFF49

#define SCREEN_HEIGHT 144
#define SCREEN_WIDTH 160

/* Struct used for sprite ordering */
struct Sprite {
  uint8_t id;
    uint16_t priority;

    Sprite(uint8_t id, uint16_t priority) : id(id), priority(priority) {}

    bool operator< (const Sprite& other) const {
        return priority > other.priority;
    }
};

class GPU {
private:
    /* Attributes */
    int gpuTicks;
    uint8_t frameBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

    Memory* mem;
    GUI* gui;

    //Render* render;
    enum GPUMode{ GPU_H_BLANK = 0, GPU_V_BLANK = 1, GPU_OAM = 2, GPU_VRAM = 3 };

    bool isLCDenabled();
    bool tileDataSelect();
    bool windowMapSelect();
    bool isWindowEnabled();
    bool backgroundMapSelect();
    bool isSpriteSizeLarge();
    bool isTileUnsigned();
    bool isSpriteDisplayEnabled();
    bool isBackgroundDisplayEnabled();

    bool isHBlankInterruptRequested();
    bool isVBlankInterruptRequested();
    bool isOAMInterruptRequested();
    bool isLYCInterruptRequested();
    
    void setCoincidenceFlag(bool coincidenceFlag);

    void incrementScanLine();
    void resetScanLine();
    uint8_t getScanline();
    uint8_t getLYC();

    uint8_t getLCDMode();
    bool isLCDMode(GPUMode mode);
    void updateLCDMode(GPUMode mode);

    uint8_t getScrollX();
    uint8_t getScrollY();
    uint8_t getWindowX();
    uint8_t getWindowY();

    uint16_t getActiveTileMemory();
    uint16_t getActiveBackgroundMemory();
    uint16_t getActiveWindowMemory();
    bool isInWindow(uint8_t x, uint8_t y);

    void drawLine();

    uint16_t calculateTileRow(uint8_t y);
    uint16_t calculateTileColumn(uint8_t x);
    void renderTile();

    bool isSpriteLarge();
    bool getSpriteBackgroundPriority(uint8_t index);

    uint8_t getSpritePosX(uint8_t index);
    uint8_t getSpritePosY(uint8_t index);
    uint8_t getSpriteTileLocation(uint8_t index);
    uint8_t getSpriteAttribute(uint8_t index);
    bool isSpriteFlipY(uint8_t attribute);
    bool isSpriteFlipX(uint8_t attribute);
    uint8_t getSpriteSizeY();
    void renderSprite();
    std::priority_queue<Sprite> getOrderedSprites();
    uint8_t getColorPaletteFromNumber(uint8_t color, uint16_t PALETTE);
    uint16_t getActiveColorPaletteAddress(uint8_t index);

    uint8_t getSpriteLine(uint8_t index);
  
    uint8_t getTilePixelValue(uint16_t tileLineAddress, uint8_t x, uint8_t y);
    uint8_t getSpritePixelValue(uint8_t index, int8_t pixel, uint16_t address);

    void triggerInterrupt(uint8_t type);

public:
    /* Constructor */
    GPU(Memory* m); 
    void update(uint8_t cycles);

    void handleEvents();
};

#endif /* GPU_H */
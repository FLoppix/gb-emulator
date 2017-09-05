#include "../Component/GPU.h"
#include "../Component/Interrupts.h"

#include <stdio.h>
#include <queue>

/* Constructor */
GPU::GPU(Memory* m) {
	mem = m;
	gpuTicks = 0;
	gui = new GUI();

	// Initialize frameBuffer
	for(int i = 0; i < SCREEN_HEIGHT; i++) {
		for(int j = 0; j < SCREEN_WIDTH; j++) {
			frameBuffer[i][j] = 3;
		}
	}
}

/* Private Methods */
bool GPU::isLCDenabled() {
	return (mem->read_8u(LCD_CTRL_REG) >> 7) & 0x1;
}

/* 0 = 9800-9BFF, 1 = 9C00-9FFF */
bool GPU::windowMapSelect() {
	return (mem->read_8u(LCD_CTRL_REG) >> 6) & 0x1;		
}

bool GPU::isWindowEnabled() {
	return (mem->read_8u(LCD_CTRL_REG) >> 5) & 0x1;
}


/* 0 = 8800-97FF 1 = 8000-8FFF */
bool GPU::tileDataSelect() { 
	return (mem->read_8u(LCD_CTRL_REG) >> 4) & 0x1;
}

/* 0 = 9800-9BFF, 1 =  1=9C00-9FFF */
bool GPU::backgroundMapSelect() {
	return (mem->read_8u(LCD_CTRL_REG) >> 3) & 0x1;
}

/* 0 = 8x8, 1 = 8x16 */
bool GPU::isSpriteSizeLarge() {
	return (mem->read_8u(LCD_CTRL_REG) >> 2) & 0x1;
}

bool GPU::isSpriteDisplayEnabled() { 
	return (mem->read_8u(LCD_CTRL_REG) >> 1) & 0x1;
}

bool GPU::isBackgroundDisplayEnabled() {
	return mem->read_8u(LCD_CTRL_REG) & 0x1;
}

bool GPU::isHBlankInterruptRequested() {
	return ((mem->read_8u(LCD_STAT_REG) >> 3) & 0x1);
}

bool GPU::isVBlankInterruptRequested() {
	return ((mem->read_8u(LCD_STAT_REG) >> 4) & 0x1);
}

bool GPU::isOAMInterruptRequested() {
	return ((mem->read_8u(LCD_STAT_REG) >> 5) & 0x1);
}

bool GPU::isLYCInterruptRequested() {
	return ((mem->read_8u(LCD_STAT_REG) >> 6) & 0x1);
}

void GPU::setCoincidenceFlag(bool coincidenceFlag) {
	uint8_t current = mem->read_8u(LCD_STAT_REG);
	if (coincidenceFlag) {
		mem->write_8u(LCD_STAT_REG, current | (1 << 2));
	} else {
		mem->write_8u(LCD_STAT_REG, current & ~(1 << 2));
	}
}

bool GPU::isTileUnsigned() {
	if (tileDataSelect()) {
		return true;
	} else { 
		return false;
	}
}

uint8_t GPU::getScanline() {
	return mem->read_8u(LCD_CUR_SCANLINE);
}

uint8_t GPU::getLYC() {
	return mem->read_8u(LCD_LYC);
}

uint8_t GPU::getLCDMode() {
	return mem->read_8u(LCD_CTRL_REG) & 0x3;
}

bool GPU::isLCDMode(GPUMode mode) {
	return (mode == getLCDMode());
}

void GPU::updateLCDMode(GPUMode mode) {
	uint8_t currentMode = mem->read_8u(LCD_STAT_REG);
	currentMode &= ~3;
	mem->privilegedWrite8u(LCD_STAT_REG, (currentMode | mode));
}

uint8_t GPU::getScrollX() {
	return mem->read_8u(SCROLL_X);
}
uint8_t GPU::getScrollY() {
	return mem->read_8u(SCROLL_Y);
}
uint8_t GPU::getWindowX() {
	return mem->read_8u(WINDOW_X);
}
uint8_t GPU::getWindowY() {
	return mem->read_8u(WINDOW_Y);
}

uint8_t GPU::getColorPaletteFromNumber(uint8_t color, uint16_t PALETTE) {
	uint8_t palette = mem->read_8u(PALETTE);
	return (palette >> (color * 2)) & 0x3;
}

uint16_t GPU::getActiveTileMemory() {
	if (tileDataSelect()) {
		return 0x8000;
	} else {
		return 0x8800;
	}
}

uint16_t GPU::getActiveBackgroundMemory() {
	 if (backgroundMapSelect()) {
		return 0x9C00;
	} else {
		return 0x9800;
	}
}

uint16_t GPU::getActiveWindowMemory() {
	if (windowMapSelect()) {
		return 0x9C00;
	} else {
		return 0x9800;
	}
}

bool GPU::isInWindow(uint8_t x, uint8_t y) {
	if (!isWindowEnabled()) {
		return false;
	} else {
		if (y >= getWindowY() && x >= getWindowX() - 7) {
			return true;
		}
		return false;
	}
}

void GPU::renderTile() {
	// Get y position of tile (considering the current scrollY)
	uint8_t posY;

	// Build current line of screen
	for (int x = 0; x < 160; x++) {
		// Get x position of tile (considering the current scrollX)
		uint8_t posX;

		uint16_t tileAddress;
		// // Calculate address of current screen position to lookup tile in this area according to Window Position
		if (isInWindow(x, getScanline())) {
			posY = getScanline() - getWindowY();
			posX = x - getWindowX() + 7;
			tileAddress = getActiveWindowMemory() + calculateTileRow(posY) + calculateTileColumn(posX);
		}
		else {
			posY = getScanline() + getScrollY();
			posX = x + getScrollX();
			tileAddress = getActiveBackgroundMemory() + calculateTileRow(posY) + calculateTileColumn(posX);
		}

		uint16_t tileLocation;
		// Separate between unsigned and signed tile identifier
		if (isTileUnsigned()) {
			// Calculate address of tile data with unsigned identifier
			uint8_t tileNum = mem->read_8u(tileAddress);
			tileLocation = getActiveTileMemory() + tileNum*16;
		} else {
			// Calculate address of tile data with signed identifier
			int8_t tileNum = mem->read_8s(tileAddress);
			tileLocation = getActiveTileMemory() + (tileNum+128) * 16;
		}

		// Get color of current pixel
		uint8_t colorNum = getTilePixelValue(tileLocation, posX, posY);
		uint8_t color = getColorPaletteFromNumber(colorNum, MONOCHROME_COLOR_PALETTE);

		// Check if scanline is in range
		if (getScanline() < 144) {
			frameBuffer[getScanline()][x] = color;
		}
	}
}

uint16_t GPU::calculateTileRow(uint8_t y) {
	return (uint16_t) (y/8)*32;
}

uint16_t GPU::calculateTileColumn(uint8_t x) {
	return (uint16_t) (x/8);
}

std::priority_queue<Sprite> GPU::getOrderedSprites() {
	std::priority_queue<Sprite> queue;

	for(uint8_t i = 0; i < 40; i++) {
		if((getScanline() >= getSpritePosY(i)) && (getScanline() < (getSpritePosY(i)+getSpriteSizeY()))) {
			Sprite sprite(i, getSpritePosX(i) * 50 + i);
			queue.push(sprite);
		}
	}

	return queue;
}

void GPU::renderSprite() {
	std::priority_queue<Sprite> queue = getOrderedSprites();

	uint8_t i;	
	while(!queue.empty()) {
		i = queue.top().id;
		queue.pop();

		uint8_t line = getSpriteLine(i);

		uint16_t spriteAddress = (0x8000 + getSpriteTileLocation(i)*16 + line*2);

		for(int8_t pixel = 7; pixel >= 0; pixel--) {
			uint8_t colorNum = getSpritePixelValue(i, pixel, spriteAddress);

			/* Lower 2 bits of color palette (equals colorNum 0) are transparent for sprites */
			if (colorNum == 0) {
				continue;
			}

			uint8_t color = getColorPaletteFromNumber(colorNum, getActiveColorPaletteAddress(i));

			uint8_t posX = getSpritePosX(i) + ((0 - pixel) + 7);

			if (getScanline() < 144) {
				if (getSpriteBackgroundPriority(i) && frameBuffer[getScanline()][posX] != 0) {
					continue;
				}
				frameBuffer[getScanline()][posX] = color;
			}
		}
	}
}

uint8_t GPU::getSpriteLine(uint8_t index) {
	int16_t line = getScanline() - getSpritePosY(index);

	if (isSpriteFlipY(getSpriteAttribute(index))) {
		if (isSpriteLarge()) {
			line = 15 - line;
		} else {
			line = 7 - line;
		}
	}

	return (uint8_t) line;
}

uint16_t GPU::getActiveColorPaletteAddress(uint8_t index) {
	return ((getSpriteAttribute(index) >> 4) & 0x1) ? OBJECT_PALETTE_1 : OBJECT_PALETTE_0;
}

uint8_t GPU::getSpriteSizeY() {
	return isSpriteLarge() ? 16 : 8;
}

bool GPU::isSpriteLarge() {
	return isSpriteSizeLarge();
}

bool GPU::getSpriteBackgroundPriority(uint8_t index) {
	return (getSpriteAttribute(index) >> 7) & 0x1;
}


uint8_t GPU::getSpritePosY(uint8_t index) {
	return mem->read_8u(SPRITE_OAM + index*4) - 16;
}

uint8_t GPU::getSpritePosX(uint8_t index) {
	return mem->read_8u(SPRITE_OAM + index*4 + 1) - 8;
}

uint8_t GPU::getSpriteTileLocation(uint8_t index) {
	return mem->read_8u(SPRITE_OAM + index*4 + 2);
}

uint8_t GPU::getSpriteAttribute(uint8_t index) {
	return mem->read_8u(SPRITE_OAM + index*4 + 3);
}

bool GPU::isSpriteFlipY(uint8_t attribute) {
	return (attribute >> 6) & 0x1;
}

bool GPU::isSpriteFlipX(uint8_t attribute) {
	return (attribute >> 5) & 0x1;
}

uint8_t GPU::getTilePixelValue(uint16_t tileLineAddress, uint8_t x, uint8_t y) {
	// Calculate vertical line of curren tile. Multiply by 2, because each line takes up 2 bytes (tileLine1 and tileLine2)
	uint8_t line = (y % 8) * 2;

	// Read both tile lines from memory
	uint8_t tileByte1 = mem->read_8u(tileLineAddress + line);
	uint8_t tileByte2 = mem->read_8u(tileLineAddress + line + 1);

	// Calculate pixel position in tile
	int8_t bit = x % 8;

	// Convert tile position to bit inversion order: pos ---X- ---- should be: 0000 0100
	bit = (bit-7)*(-1);

	// Combine both tile lines for color value
	uint8_t colorNum = ((tileByte2 >> bit) & 0x1) << 1;
	colorNum |= (tileByte1 >> bit) & 0x1;

	return colorNum;
}

uint8_t GPU::getSpritePixelValue(uint8_t index, int8_t pixel, uint16_t address) {
	uint8_t spriteByte1 = mem->read_8u(address);
	uint8_t spriteByte2 = mem->read_8u(address + 1);

	if (isSpriteFlipX(getSpriteAttribute(index))) {
		pixel = 7 - pixel;
	}

	uint8_t colorNum = ((spriteByte2 >> pixel) & 0x1) << 1;
	colorNum |= (spriteByte1 >> pixel) & 0x1;

	return colorNum;
}

void GPU::incrementScanLine() {
	mem->privilegedWrite8u(LCD_CUR_SCANLINE, mem->read_8u(LCD_CUR_SCANLINE) + 1);
}

void GPU::resetScanLine() {
	mem->privilegedWrite8u(LCD_CUR_SCANLINE, 0);
}

void GPU::drawLine() {
	if (isBackgroundDisplayEnabled()) {
		renderTile();
	}

	if (isSpriteDisplayEnabled()) {
		renderSprite();
	}
}

void GPU::triggerInterrupt(uint8_t type) {
	uint8_t current = mem->read_8u(INTERRUPT_REQUEST_REGISTER);
	mem->write_8u(INTERRUPT_REQUEST_REGISTER, (current | type));
}

/* Public Methods */
void GPU::update(uint8_t cycles) {
	/* Reset when lcd is disabled */
	if (!isLCDenabled()) {
		gpuTicks = 0;
		resetScanLine();
		updateLCDMode(GPU_H_BLANK);
		return;
	}

	/* Increase current ticks */
	gpuTicks += cycles;

	/* Set LCD Mode */
	if (getScanline() >= 144) {
		// MODE 1: V_BLANK
		if (!isLCDMode(GPU_V_BLANK)) {
			updateLCDMode(GPU_V_BLANK);
			if (isVBlankInterruptRequested()) {
				triggerInterrupt(INTERRUPT_LCDSTAT);
			}
		}
	}
	else {
		if (gpuTicks <= 80) {
			// MODE 2: OAM RAM
			if (!isLCDMode(GPU_OAM)) {
				updateLCDMode(GPU_OAM);
				if (isOAMInterruptRequested()) {
					triggerInterrupt(INTERRUPT_LCDSTAT);
				}
			}
		}
		else if (gpuTicks <= 252) {
			// MODE 3: VRAM
			if (!isLCDMode(GPU_VRAM)) {
				updateLCDMode(GPU_VRAM);
				// no interrupt!
			}
		}
		else {
			// MODE 0: H BLANK
			if (!isLCDMode(GPU_H_BLANK)) {
				updateLCDMode(GPU_H_BLANK);
				if (isHBlankInterruptRequested()) {
					triggerInterrupt(INTERRUPT_LCDSTAT);
				}
			}
		}
	}

	/* Compare Scanline LY to LYC */
	if (getScanline() == getLYC()) {
		setCoincidenceFlag(true);

		if (isLYCInterruptRequested()) {
			triggerInterrupt(INTERRUPT_LCDSTAT);
		}
	}
	else {
		setCoincidenceFlag(false);
	}

	/* Draw Image and Render */
	if (gpuTicks >= 456) {
		gpuTicks -= 456;

		if (getScanline() < 144) {
			drawLine();
		}
		else if (getScanline() == 144) {
			triggerInterrupt(GPU_V_BLANK);
			gui->render(frameBuffer);
			gui->handleEvents();
		}
		else if (getScanline() > 153) {
			resetScanLine();
			return;
		}
		
		incrementScanLine();
	}
}

void GPU::handleEvents() {
	gui->handleEvents();
}
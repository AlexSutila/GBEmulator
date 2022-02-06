#include "objs.h"
#include "mem.h"
#include "ppu.h"

// For each sprite, determines if it is on the current scanline and writes a 1 to the sprites 
//		representative bit if so and zero otherwise
void search_OAM(struct GB* gb, uint8_t* visibilities, uint8_t curr_LY) 
{
	struct sprite* sprites = (struct sprite*)(gb->memory + 0xFE00);
	for (int i = 0; i <= 40; i++) {

		int yPos = sprites[i].yPos - 16;
		uint8_t height = (gb->memory[index_LCDC] & LCDC_OBJS_MASK) ? 16 : 8;

		if ((yPos + height > curr_LY) && (yPos <= curr_LY)) {
			visibilities[i] = 1;
		}
		else visibilities[i] = 0;
	}
}

// Draws a sprite from a given index and a pointer to the start of a given scanline. Once again
//		this should only be called if the objects are enabled through the LCDC register
void draw_Sprite(struct GB* gb, uint8_t* bitmapPtr, uint8_t* visibilities, uint8_t index) {

	// Used to index sprites in OAM using the index parameter
	struct sprite* sprites = (struct sprite*)(gb->memory + 0xFE00);

	uint8_t curr_LY = gb->memory[index_LY];
	uint8_t height = (gb->memory[index_LCDC] & LCDC_OBJS_MASK) ? 16 : 8;

	int xPos = sprites[index].xPos - 8;
	int yPos = sprites[index].yPos - 16;

	uint8_t pallete = sprites[index].attribs & ATTRIBS_PALNUM_MASK ?
		gb->memory[index_OBP1] : gb->memory[index_OBP0];

	// Fetch first tile of sprite
	struct tileStruct* currentTile = (height == 16) ? (struct tileStruct*)&gb->memory[0x8000 + (((uint16_t)sprites[index].tileIndex & 0xFFFE) << 4)]
		: (struct tileStruct*)&gb->memory[0x8000 + (((uint16_t)sprites[index].tileIndex) << 4)];

	// Calculate row offset based on yFlip bit
	// uint8_t rowOffset = curr_LY - yPos;
	uint8_t rowOffset = (sprites[index].attribs & ATTRIBS_YFLIP_MASK) 
		? (height - 1) - (curr_LY - yPos) : curr_LY - yPos;

	// If the scanline lies within a second sprite tile, adjust such conditions
	if (rowOffset >= 8) {
		rowOffset -= 8;
		currentTile++;
	}

	// Determine horizontal flip of sprite
	int startBit, endBit, increment;
	if (sprites[index].attribs & ATTRIBS_XFLIP_MASK) {
		startBit = 0; endBit = 8; increment = 1;
	} else {
		startBit = 7; endBit = -1; increment = -1;
	}

	int counter = 0;
	for (int i = startBit; i != endBit; i += increment) {

		// Write color data to frame buffer
		uint8_t color = 0xFF;
		uint8_t lo = (currentTile->bytes[rowOffset][1]) >> i;
		uint8_t hi = (currentTile->bytes[rowOffset][0]) >> i;
		switch (((lo & 0x1) << 1) | (hi & 0x1)) {
			case 0: /* Transparent */					 break;
			case 1: color = (pallete & 0b00001100) >> 2; break;
			case 2: color = (pallete & 0b00110000) >> 4; break;
			case 3: color = (pallete & 0b11000000) >> 6; break;
		}
		
		// Need to make sure xPos + counter is in bounds
		if (xPos + counter >= 0 && xPos + counter < 160) {
			if (sprites[index].attribs & ATTRIBS_OVER_MASK) {
				uint8_t BG_color = gb->memory[index_BGP] & 0x3; // Color to overlap if bit 7 enabled
				bitmapPtr[xPos + counter] = (bitmapPtr[xPos + counter] == BG_color && color != 0xFF)
					? color + 4 : bitmapPtr[xPos + counter];
			}
			else if (color != 0xFF) bitmapPtr[xPos + counter] = color + 4; // Plus 4 to enter my own extra sprite pallete
		}

		counter++;
	}
}

// Does OAM DMA in the duration of a single write (not accurate)
void DMA_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	static uint8_t DMA_reg = 0xFF; // Boot value

	// Source:      $XX00 - $XX9F; XX = $00 to $DF
	// Destination : $FE00 - $FE9F

	gb->memory[index_DMA] = val;

	// Transfer data when different register value
	if (gb->memory[index_DMA] != DMA_reg) {
		DMA_reg = gb->memory[index_DMA];

		// Copy data (not cycle accurate)
		uint16_t start_addr = (DMA_reg << 8);

		if (DMA_reg >= 0x00 && DMA_reg <= 0x3F) {
			memcpy(gb->memory + 0xFE00, gb->cart.ROM00_ptr + start_addr, 0xA0);
		}
		else if (DMA_reg >= 0x40 && DMA_reg <= 0x7F) {
			start_addr = start_addr - 0x4000;
			memcpy(gb->memory + 0xFE00, gb->cart.ROMnn_ptr + start_addr, 0xA0);
		}
		else if (DMA_reg >= 0x80 && DMA_reg <= 0x9F) {
			memcpy(gb->memory + 0xFE00, gb->memory + start_addr, 0xA0);
		}
		else if (DMA_reg >= 0xA0 && DMA_reg <= 0xBF) {
			start_addr = start_addr - 0xA000;
			memcpy(gb->memory + 0xFE00, gb->cart.RAM_ptr + start_addr, 0xA0);
		}
		else if (DMA_reg >= 0xC0 && DMA_reg <= 0xDF) {
			memcpy(gb->memory + 0xFE00, gb->memory + start_addr, 0xA0);
		}
		else if (DMA_reg >= 0xE0 && DMA_reg <= 0xFD) {
			start_addr = start_addr - 0x1000;
			memcpy(gb->memory + 0xFE00, gb->memory + start_addr, 0xA0);
		}
	}

	DMA_reg = 0xFF;
}

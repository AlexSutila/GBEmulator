#include "ppu.h"
#include "mem.h"

// Not really the size as much as it is the value of the index
//		of the last pixel with respect to the first, only used
//		to handle vertical flipping with varying sprite height
#define OBJ_SIZE_8X8  7
#define OBJ_SIZE_8X16 15

// Extracts the given x position with respect to the start of a tile, given a pointer
//		to a sprites attributes and the current x pixel being written to the frame 
inline uint8_t calc_col_mod(const struct spriteStruct* sprite_ptr, uint8_t pixel_x)
{
	return sprite_ptr->x_flip ? OBJ_SIZE_8X8 - pixel_x : pixel_x;
}
// Extracts the given y position with respect to the start of a tile, given a pointer
//		to a sprites attributes and the current y pixel being written to the frame. In
//		addition, size must be provided as the height of sprites can vary.
inline uint8_t calc_row_mod(const struct spriteStruct* sprite_ptr, uint8_t pixel_y, uint8_t size)
{
	const uint8_t YPOS_OFFSET = 16; // the ypos attribute is not the exact position on the screen
	uint8_t temp = pixel_y - sprite_ptr->yPos + YPOS_OFFSET;
	return sprite_ptr->y_flip ? size - temp : temp;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

void draw_sprite(struct GB* gb, uint8_t* bitmap_ptr, const struct spriteStruct* sprite_ptr, uint8_t ly_val)
{	
	const uint8_t XPOS_OFFSET = 8; // the xpos attribute is not the exact position on the screen
	// If object size is 8x16, the least significant bit is ignored when indexing the tile data
	uint8_t tile_index = sprite_ptr->tileIndex;
	if (gb->ppu.obj_size) tile_index &= 0xFE;
	// Fetch the tile based on the tile index in the object attributes, sprites always
	//		use the unsigned addressing mode, it cannot be changed.
	const struct tileStruct* cur_tile = tiledata_access(gb, tileAddrModeUnsigned, tile_index);
	// Determine the object size from the objsize bit
	uint8_t obj_size = gb->ppu.obj_size ? OBJ_SIZE_8X16 : OBJ_SIZE_8X8;
	// Obtain the pallete from the sprite attributes
	uint8_t obj_palette = sprite_ptr->paletteNumber ? gb->ppu.reg_obp1 : gb->ppu.reg_obp0;
	// Calculate positional related stuff
	uint8_t row_mod = calc_row_mod(sprite_ptr, ly_val, obj_size);
	int16_t sprite_x = sprite_ptr->xPos - XPOS_OFFSET;
	// Write the pixels to the frame buffer
	for (int pixel_x = 0; pixel_x < 8 /* Sprites are 8 pixels in width */; pixel_x++)
	{
		uint8_t col_mod = calc_col_mod(sprite_ptr, pixel_x), color_index = 0;
		// Create color, don't write if it is zero or if it goes off the scanline
		if (sprite_x + pixel_x >= 0 && sprite_x + pixel_x < v_HRES)
		{	// Pull pixel info from the fetched tile
			uint8_t lo = (cur_tile->bytes[row_mod][1]) >> (7 - col_mod); // Lo corresponds to index 1 and hi to 0
			uint8_t hi = (cur_tile->bytes[row_mod][0]) >> (7 - col_mod); // on purpose
			// Construct color value from the given pixel data and write
			switch (((lo & 0x1) << 1) | (hi & 0x1))
			{
			case 0x0: /* The pixel is ignored */                  continue;
			case 0x1: color_index = (obj_palette & 0b00001100) >> 2; break;
			case 0x2: color_index = (obj_palette & 0b00110000) >> 4; break;
			case 0x3: color_index = (obj_palette & 0b11000000) >> 6; break;
			}
			// Handle the weirdness with the bgwin priority bit
			if ((sprite_ptr->bgwin_over_objs && bitmap_ptr[sprite_x + pixel_x] == gb->ppu.bg_color_idx0) || !sprite_ptr->bgwin_over_objs) 
			{
				bitmap_ptr[sprite_x + pixel_x] = color_index;
			}
		}
	}
}

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
	uint8_t temp = pixel_y - sprite_ptr->yPos + 16;
	return sprite_ptr->y_flip ? size - temp : temp;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

void draw_sprite(struct GB* gb, uint8_t* bitmap_ptr, const struct spriteStruct* sprite_ptr, uint8_t ly_val)
{	// If object size is 8x16, the least significant bit is ignored when indexing the tile data
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
	uint8_t sprite_x = sprite_ptr->xPos - 8;
	// Write the pixels to the frame buffer
	for (int pixel_x = 0; pixel_x < 8; pixel_x++)
	{
		uint8_t col_mod = calc_col_mod(sprite_ptr, pixel_x);
		// Calculate the color index from the earlier information
		uint8_t color_index = create_color(cur_tile, obj_palette, row_mod, col_mod);
		// Create color, don't write if it is zero or if it goes off the scanline
		if (color_index != 0 && sprite_x + pixel_x >= 0 && sprite_x + pixel_x < v_HRES) 
			bitmap_ptr[sprite_x + pixel_x] = color_index;
	}
}

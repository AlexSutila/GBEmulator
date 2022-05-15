#include "types.h"
#include "mem.h"
#include "ppu.h"

// PPU Mode transitions ///////////////////////////////////////////////////////////////////////////

// My convention here is if it starts with e, the function represents what happens on entry
//		of the given mode, and if it starts with l, the function represetns what happens upon
//		leaving the given mode
//		...
// Modes dont necessarily always transition to the same next mode for every scanline. Specifically
//		horizontal blank could either transition to oam search, or into vblank and vblank could 
//		either transition to oam search or another vblank. Functions starting with the letter l,
//		(exit functions) will return the next state the PPU should enter. 

/* Horizontal Blank */
uint8_t lHBlank(struct GB* gb)
{
	// End of scanline, increment the value of the LY register
	gb->ppu.reg_ly = gb->ppu.reg_ly + 1;
	// HBlank usually switches to OAM search, but on scanline 143 it enters the first VBlank
	return (gb->ppu.reg_ly >= 144) ? statModeVBlank : statModeOamSearch;
}
void eHBlank(struct GB* gb)
{
	// Update the PPU mode
	gb->ppu.state = statModeHBlank;
}
/* Vertical Blank */
uint8_t lVBlank(struct GB* gb)
{
	// End of scanline, increment the value of the LY register
	gb->ppu.reg_ly = gb->ppu.reg_ly + 1;
	// VBlank usually switches to the start of another VBlank, but on scanline 153 it goes
	//		back to OAM search
	if (gb->ppu.reg_ly == 154)
	{	// Reset LY register back to zero, notify that the frame is complete
		gb->ppu.frameIncomplete = 0;
		gb->ppu.reg_ly = 0;
		return statModeOamSearch;
	}
	else return statModeVBlank;
}
void eVBlank(struct GB* gb)
{
	// Update the PPU mode
	gb->ppu.state = statModeVBlank;
}
/* Oam Search */
uint8_t lOamSearch(struct GB* gb)
{

	return statModeDataTrans;
}
void eOamSearch(struct GB* gb)
{
	// Update the PPU mode
	gb->ppu.state = statModeOamSearch;
}
/* Data Transfer */
uint8_t lDataTrans(struct GB* gb)
{	// Determine the tile map base address for the background and window
	if (gb->ppu.bgwin_enable)
	{
		uint16_t win_map_base = gb->ppu.window_tilemap ? 0x9C00 : 0x9800;
		uint16_t bg_map_base  = gb->ppu.bg_tilemap     ? 0x9C00 : 0x9800;
		// Row calculations used later for indexing tilemaps and tiles
		const int row_index = gb->ppu.reg_ly / 8;
		const int row_mod = gb->ppu.reg_ly % 8;
		// Create a pointer to the start of the scanline in the frame buffer
		uint8_t* bitmap_ptr = (uint8_t*)(gb->ppu.bitmap) + (gb->ppu.reg_ly * v_HRES);
		// An index integer to keep track of the current pixel x position
		unsigned int cur_pixel;
		// Start writing the background to the frame buffer
		for (cur_pixel = 0; cur_pixel < 160; cur_pixel++)
		{	// Column calculations used for indexing tilemap and tile
			const int col_index = cur_pixel / 8;
			const int col_mod = cur_pixel % 8;
			// Obtain the base address of the tile with calculated index
			uint16_t bg_map_index = col_index + (row_index * 32); // tilemaps are 32 x 32 tiles
			const struct tileStruct* cur_tile = tile_access(gb, bg_map_base, bg_map_index);
			// Pull pixel info from the fetched tile
			uint8_t lo = (cur_tile->bytes[row_mod][1]) >> (7-col_mod); // Lo corresponds to index 1 and hi to 0
			uint8_t hi = (cur_tile->bytes[row_mod][0]) >> (7-col_mod); // on purpose
			// Construct color value from the given pixel data and write
			switch (((lo & 0x1) << 1) | (hi & 0x1))
			{
			case 0x0: bitmap_ptr[cur_pixel] =  gb->ppu.reg_bgp & 0b00000011;       break;
			case 0x1: bitmap_ptr[cur_pixel] = (gb->ppu.reg_bgp & 0b00001100) >> 2; break;
			case 0x2: bitmap_ptr[cur_pixel] = (gb->ppu.reg_bgp & 0b00110000) >> 4; break;
			case 0x3: bitmap_ptr[cur_pixel] = (gb->ppu.reg_bgp & 0b11000000) >> 6; break;
			}
		}
	}
	return statModeHBlank;
}
void eDataTrans(struct GB* gb)
{
	// Update the PPU mode
	gb->ppu.state = statModeDataTrans;
}


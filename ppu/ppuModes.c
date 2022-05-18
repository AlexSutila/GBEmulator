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
	// HBlank usually switches to OAM search, but after scanline 143 it enters the first VBlank
	return (gb->ppu.reg_ly >= 144) ? statModeVBlank : statModeOamSearch;
}
void eHBlank(struct GB* gb)
{
	// Update the stat mode bits
	gb->ppu.mode_bits = 0b00;
	// Update the PPU mode
	gb->ppu.state = statModeHBlank;
}
/* Vertical Blank */
uint8_t lVBlank(struct GB* gb)
{
	// End of scanline, increment the value of the LY register
	gb->ppu.reg_ly = gb->ppu.reg_ly + 1;
	// VBlank usually switches to the start of another VBlank, but on scanline 153 it
	//		needs to mimic the weird behavior that occurs during that scanline
	return gb->ppu.reg_ly == 153 ? scanln153First4Cycles : statModeVBlank;
}
void eVBlank(struct GB* gb)
{
	// Update the stat mode bits
	gb->ppu.mode_bits = 0b01;
	// Update the PPU mode
	gb->ppu.state = statModeVBlank;
	// If on scanline 144, trigger vblank interrupt
	if (gb->ppu.reg_ly == 144) setIFBit(gb, 0);
}
/* Oam Search */
uint8_t lOamSearch(struct GB* gb)
{
	// Always enters data transfer
	return statModeDataTrans;
}
void eOamSearch(struct GB* gb)
{
	// Update the stat mode bits
	gb->ppu.mode_bits = 0b10;
	// Update the PPU mode
	gb->ppu.state = statModeOamSearch;
}
/* Data Transfer */
inline uint8_t create_color(const struct tileStruct* cur_tile, uint8_t pallete, int row_mod, int col_mod)
{
	// Pull pixel info from the fetched tile
	uint8_t lo = (cur_tile->bytes[row_mod][1]) >> (7 - col_mod); // Lo corresponds to index 1 and hi to 0
	uint8_t hi = (cur_tile->bytes[row_mod][0]) >> (7 - col_mod); // on purpose
	// Construct color value from the given pixel data and write
	switch (((lo & 0x1) << 1) | (hi & 0x1))
	{
	case 0x0: return  pallete & 0b00000011;
	case 0x1: return (pallete & 0b00001100) >> 2;
	case 0x2: return (pallete & 0b00110000) >> 4;
	case 0x3: return (pallete & 0b11000000) >> 6;
	}
	// The switch should always return a value, this is here to avoid compiler warnings
	return 0x00;
}
#define TILEMAP_RES  256 // 32 * 8 - Tile maps are 32 tiles times 8 pixels, both directions
uint8_t lDataTrans(struct GB* gb)
{	// If the background and window are enabled, then draw the scanline to the frame buffer
	if (gb->ppu.bgwin_enable)
	{	// Determine the point at which to stop rendering the background and start rendering the window
		int bg_stop = (gb->ppu.reg_wx-7<v_HRES) && (gb->ppu.reg_wy<=gb->ppu.reg_ly) && gb->ppu.window_enable
			? gb->ppu.reg_wx - 7 : v_HRES;
		// Determine the tile map base address for the background and window
		uint16_t win_map_base = gb->ppu.window_tilemap ? 0x9C00 : 0x9800;
		uint16_t bg_map_base  = gb->ppu.bg_tilemap     ? 0x9C00 : 0x9800;
		// Row calculations used later for indexing tilemaps and tiles
		int row_index = ((gb->ppu.reg_ly + gb->ppu.reg_scy) % TILEMAP_RES) / 8;
		int row_mod   = ((gb->ppu.reg_ly + gb->ppu.reg_scy) % TILEMAP_RES) % 8;
		// Create a pointer to the start of the scanline in the frame buffer
		uint8_t* bitmap_ptr = (uint8_t*)(gb->ppu.bitmap) + (gb->ppu.reg_ly * v_HRES);
		// An index integer to keep track of the current pixel x position on the screen
		int cur_pixel = 0;
		// Start writing the background to the frame buffer
		for (int i = 0; i < bg_stop; i++)
		{	// Column calculations used for indexing tilemap and tile
			const int col_index = ((cur_pixel + gb->ppu.reg_scx) % TILEMAP_RES) / 8;
			const int col_mod   = ((cur_pixel + gb->ppu.reg_scx) % TILEMAP_RES) % 8;
			// Obtain the base address of the tile with calculated index
			uint16_t bg_map_index = col_index + (row_index * 32); // tilemaps are 32 x 32 tiles
			const struct tileStruct* cur_tile = tile_access(gb, bg_map_base, bg_map_index);
			// Pull pixel info from the fetched tile
			bitmap_ptr[cur_pixel] = create_color(cur_tile, gb->ppu.reg_bgp, row_mod, col_mod);
			++cur_pixel;
		}
		// Update the row_index and row_mod to fit the needs of the window
		row_index = (gb->ppu.win_ly % TILEMAP_RES) / 8;
		row_mod   = (gb->ppu.win_ly % TILEMAP_RES) % 8;
		// An index integer to keep track of the current pixel x position within the window
		int win_cur_pixel = 0;
		// Continue writing the window to the frame buffer
		for (int i = cur_pixel; i < v_HRES; i++)
		{	// Column calculations used for intexing the tilemap and tile, like before
			const int col_index = (win_cur_pixel % TILEMAP_RES) / 8;
			const int col_mod   = (win_cur_pixel % TILEMAP_RES) % 8;
			// Obtain the base address of the tile with calculated index
			uint16_t win_map_index = col_index + (row_index * 32);
			const struct tileStruct* cur_tile = tile_access(gb, win_map_base, win_map_index);
			// Pull pixel info from the fetched tile
			bitmap_ptr[cur_pixel] = create_color(cur_tile, gb->ppu.reg_bgp, row_mod, col_mod);
			++cur_pixel; ++win_cur_pixel;
		}
		// If the window was visible on this scanline, move the internal scanline counter along
		if (bg_stop < v_HRES) gb->ppu.win_ly++;
	}
	return statModeHBlank;
}
#undef TILEMAP_RES
void eDataTrans(struct GB* gb)
{
	// Update the stat mode bits
	gb->ppu.mode_bits = 0b11;
	// Update the PPU mode
	gb->ppu.state = statModeDataTrans;
}
/* The first four cycles of scanline 153 */
uint8_t lScanln153First4Cycles(struct GB* gb)
{
	// At this point, the frame has been completed. The main loop executes
	//		instructions until this ppu field is set to zero. Set this to
	//		zero to break out of the loop and render the frame
	gb->ppu.frameIncomplete = 0;
	// It will always continue with the remaining cycles of the same scanline
	return scanln153RemainingCycles;
}
void eScanln153First4Cycles(struct GB* gb)
{
	// Update the stat mode bits, not needed but doing it to be safe
	gb->ppu.mode_bits = 0b01;
	// Update the PPU mode
	gb->ppu.state = scanln153First4Cycles;
}
/* The remains of scanline 153 after the four cycles */
uint8_t lScanln153RemainingCycles(struct GB* gb)
{
	// Reset the internal scanline counter for the window
	gb->ppu.win_ly = 0;
	// This will always go back to OAM search
	return statModeOamSearch;
}
void eScanln153RemainingCycles(struct GB* gb)
{
	// Update the stat mode bits, not needed but doing it to be safe
	gb->ppu.mode_bits = 0b01;
	// Update the PPU mode
	gb->ppu.state = scanln153RemainingCycles;
	// Here, the LY register is reset early
	gb->ppu.reg_ly = 0x00;
}

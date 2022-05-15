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
	// End of a scanline, increment the LY register (it shouldn't wrap around, but I'll make it
	//		to be safe, I don't know)
	gb->ppu.reg_ly = (gb->ppu.reg_ly + 1) % 154;
	// HBlank usually switches to OAM search, but on scanline 143 it enters the first VBlank
	return (gb->ppu.reg_ly == 143) ? statModeVBlank : statModeOamSearch;
}
void eHBlank(struct GB* gb)
{
	// Update the PPU mode
	gb->ppu.state = statModeHBlank;
}
/* Vertical Blank */
uint8_t lVBlank(struct GB* gb)
{
	// End of a scanline, increment the LY register, but wrap around past 153
	gb->ppu.reg_ly = (gb->ppu.reg_ly + 1) % 154;
	// VBlank usually switches to the start of another VBlank, but on scanline 153 it goes
	//		back to OAM search
	return (gb->ppu.reg_ly == 153) ? statModeOamSearch : statModeVBlank;
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
{

	return statModeHBlank;
}
void eDataTrans(struct GB* gb)
{
	// Update the PPU mode
	gb->ppu.state = statModeDataTrans;
}


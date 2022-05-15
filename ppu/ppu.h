#ifndef PPU_H_
#define PPU_H_

#include <Windows.h>
#include "types.h"

#define SCANLINE_LENGTH 456
#define SCANLINE_COUNT 154

// Screen definitions
#define v_pixelSize  4
#define v_VRES 144
#define v_HRES 160
#define v_WIDTH (v_HRES * v_pixelSize)
#define v_HEIGHT (v_VRES * v_pixelSize)

struct ppuCycleTimingInfo
{	// This is a struct that will help determine how many cycles were spent in a specific
	//		mode, and if there are any left over cycles from that mode to execute in the
	//		next ppu mode when syncrhonizing the ppu a given amount of cycles
	//		...
	// This naming should be fairly self explanitory
	uint8_t spentCycles, leftOverCycles;
};

// Calculate how many cycles were spent in a ppu mode and how many cycles are left over
//		given the following:
//		current - the current value of the dot counter (1 dot = 1 clock cycles, people
//		          just call them dots when talking about the PPU, even in the docs
//		max     - the max value of the dot counter for the current mode
//		amount  - the amount of clock cycles to be stepped, determined by the
//		          most recent cpu instruction executed
inline struct ppuCycleTimingInfo calcPpuTimingInfo(int current, int max, uint8_t amount)
{	// Calculate how many cycles are left over
	uint8_t leftOverCycles = (current + amount >= max) ?
		current + amount - max : 0;
	// spentCycles are all cycles that are not leftover
	uint8_t spentCycles = amount - leftOverCycles;
	// See the ppuCycleTimingInfo struct above
	return (struct ppuCycleTimingInfo) { spentCycles, leftOverCycles };
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

struct bitmapStruct
{	// BMI Struct which contains information for the bitmap, as well as the color palette
	BITMAPINFO bmi;
	unsigned char palette[256];
};

// Basic PPU functions
void init_ppu(struct PPU* ppu);
void ppu_step(struct GB* gb, int cycles);
void free_ppu(struct PPU* ppu);

// Functions representing each PPU mode and how they behave upon their entries and completions
/* ----------------------------------- */
uint8_t lHBlank(struct GB* gb);    // Mode 0
void eHBlank(struct GB* gb);
/* ----------------------------------- */
uint8_t lVBlank(struct GB* gb);    // Mode 1
void eVBlank(struct GB* gb);
/* ----------------------------------- */
uint8_t lOamSearch(struct GB* gb); // Mode 2
void eOamSearch(struct GB* gb);
/* ----------------------------------- */
uint8_t lDataTrans(struct GB* gb); // Mode 3
void eDataTrans(struct GB* gb);

// For real time synchronized video output
void draw_to_screen(struct GB* gb, HWND window, HDC hdc);

// Enumeration for representing the mode values in the stat register
//		for each PPU mode. The bottom two bits will align with the
//		corresponding mode bits value that will appear when the PPU
//		is in the respective mode. 
enum statModeFlags
{
	statModeHBlank    = 0b00,
	statModeVBlank    = 0b01,
	statModeOamSearch = 0b10,
	statModeDataTrans = 0b11,
	// TODO, work scanline 153 in because its weird and the reason why
	//		I made my ppu_step function super flexible
};

struct PPU 
{	// Bitmap stuff
	struct bitmapStruct* bitmapBMI;
	void* bitmap;
	// Actual PPU stuff
	int dotCounter, frameIncomplete;
	// FF40 - LCD Control register
	union
	{
		uint8_t reg_lcdc;
		struct
		{
			/* Bit0 */ uint8_t bgwin_enable   : 1; // 0 = Off, 1 = On
			/* Bit1 */ uint8_t obj_enable     : 1; // 0 = Off, 1 = On
			/* Bit2 */ uint8_t obj_size       : 1; // 0=8x8, 1=8x16
			/* Bit3 */ uint8_t bg_tilemap     : 1; // 0=9800-9BFF, 1=9C00-9FFF
			/* Bit4 */ uint8_t bgwin_tiledata : 1; // 0=8800-97FF, 1=8000-8FFF
			/* Bit5 */ uint8_t window_enable  : 1; // 0=Off, 1=On
			/* Bit6 */ uint8_t window_tilemap : 1; // 0=9800-9BFF, 1=9C00-9FFF
			/* Bit7 */ uint8_t ppu_enable     : 1; // 0=Off, 1=On
		};
	};
	// FF41 - LCD Status register
	union
	{
		uint8_t reg_stat;
		struct
		{
			/* Bits0-1 */ uint8_t mode_bits          : 2; // see statModeFlags enumeration (Read Only)
			/* Bit2    */ uint8_t ly_is_lyc          : 1; // (0=Different, 1=Equal) (Read Only)
			// The following are enables for different events that trigger a stat interrupt
			/* Bit3    */ uint8_t hblank_stat_src    : 1; // (1 = Enable) (Read / Write)
			/* Bit4    */ uint8_t vblank_stat_src    : 1; // (1 = Enable) (Read / Write)
			/* Bit5    */ uint8_t oam_stat_src       : 1; // (1 = Enable) (Read / Write)
			/* Bit6    */ uint8_t ly_is_lyc_stat_src : 1; // (1 = Enable) (Read / Write)
			// The next bit is unused, thats why it has funny name so I don't accidentally use it
			/* Bit7    */ uint8_t amongUs            : 1; // Unused bit
		};
	};
	// FF42 - SCY register
	uint8_t reg_scy; // Scrolls vertically
	// FF43 - SCX register
	uint8_t reg_scx; // Scrolls horizontally
	// FF44 - LY register (Read only)
	uint8_t reg_ly;  // LCD Y coordinate, or the current scanline
	// FF45 - LYC register
	uint8_t reg_lyc; // LY compare value
	// FF46 - DMA register
	uint8_t reg_dma; // Used to start a DMA transfer
	// FF47 - BGP regisetr
	union
	{	// Background pallete
		uint8_t reg_bgp;
		struct
		{
			/* Bits0-1 */ uint8_t bg_color_idx0 : 2;
			/* Bits2-3 */ uint8_t bg_color_idx1 : 2;
			/* Bits4-5 */ uint8_t bg_color_idx2 : 2;
			/* Bits6-7 */ uint8_t bg_color_idx3 : 2;
		};
	};
	// FF48 - OBP0 register
	union
	{	// Background pallete
		uint8_t reg_obp0;
		struct
		{
			/* Bits0-1 */ uint8_t obp0_color_idx0 : 2;
			/* Bits2-3 */ uint8_t obp0_color_idx1 : 2;
			/* Bits4-5 */ uint8_t obp0_color_idx2 : 2;
			/* Bits6-7 */ uint8_t obp0_color_idx3 : 2;
		};
	};
	// FF49 - OBP0 register
	union
	{	// Background pallete
		uint8_t reg_obp1;
		struct
		{
			/* Bits0-1 */ uint8_t obp1_color_idx0 : 2;
			/* Bits2-3 */ uint8_t obp1_color_idx1 : 2;
			/* Bits4-5 */ uint8_t obp1_color_idx2 : 2;
			/* Bits6-7 */ uint8_t obp1_color_idx3 : 2;
		};
	};
	// FF4A - WY register
	uint8_t reg_wy;  // Window y position
	// FF4B - WX register
	uint8_t reg_wx;  // Window x + 7
};

#endif

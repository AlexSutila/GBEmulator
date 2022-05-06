#include <Windows.h>
#include "ppu.h"
#include "hwio.h"
#include "objs.h"
#include "mem.h"

#define OVERFLOW(old, cur) (old > cur)

/*
		The PPU has 4 specific modes, see the ppu.h header file for documentation from pandocs ab that.
				However, there are also hardware bugs, mainly within the first and final scanlines.

		At the beginning of the first scanline, the state is latched incorrectly as it enters OAM, causing
				it to come across as HBlank. This mislatched mode is also 4 cycles shorter due to the weird
				latching. I decided to emulate the timing, as well as how the STAT register behaves for this 
				mode. Looking back at it, not sure why, but its here so no point in going back. Note that 
				this only happens on the first frame after the PPU leaving a disabled state
		
		The final scanline resets the LY register to 0, 4 cycles into it's VBlank period. This causes
				weird timing with the LY = LYC bit and also the LY = LYC interrupt which might cause
				weird flickering in some games. Consequentially, this bug NEEDS to be emulated. 

		In addition, I am not going to emulate each mode with cycle accuracy, because for 99% of instances
				it actually is not that important. I want specific things to happen at the starts and ends
				of each mode, and all the intermediate cycles will just poll and increment a scanline
				counter to determine when its ready to leave that mode. Because of this, I have entry modes
				(labeled E) and idle modes (labeled I) for the entries and idles (polling) of each mode. 
*/

// PPU states
#define STATE_DISABLED_E		0b00000 // I wanted the STAT mode bits for each respective mode to be
#define STATE_DISABLED_I		0b00100	//		represented within these values. And they are...
#define STATE_MISLATCH_E		0b01000
#define STATE_MISLATCH_I		0b01100 // The two least significant bits are the mode bits when it is
#define STATE_OAMSEARCH_E		0b00010 //		in that respective mode, and the remaining three are just
#define STATE_OAMSEARCH_I		0b00110 //		to be able to be able to tell the actual modes apart from
#define STATE_DATATRANS_E		0b00011 //		one another
#define STATE_DATATRANS_I		0b00111
#define STATE_HBLANK_E			0b10000
#define STATE_HBLANK_I			0b10100
#define STATE_VBLANK_E			0b00001
#define STATE_VBLANK_I			0b00101
#define STATE_SCAN_LN153_E		0b01001
#define STATE_SCAN_LN153_I		0b10001

// Interrupt bit masks
#define INTMASK_VBLANK	0b00000001
#define INTMASK_STAT	0b00000010
#define INTMASK_TIMER	0b00000100
#define INTMASK_SERIAL	0b00001000
#define INTMASK_JOYPAD	0b00010000

// Forward declaration
void draw_to_screen(struct GB*, HWND window, HDC hdc);
void draw_scanline(struct GB*, uint8_t* bitmapPtr);

// PPU initialize
void init_ppu(struct PPU* ppu) 
{
	ppu->enabled = 0;
	ppu->win_LY = 0;
	ppu->scanline = 0;
	ppu->nextMode = STATE_DISABLED_I;
	ppu->frameIncomplete = 1;

	struct pixelColor 
	{
		uint8_t rVal, gVal, bVal;
	};

	/*
			I've added two palletes for objects and other layers, just because
					I can. If anyone ever sees this and wishes to have their sprites
					a different color for what ever reason, this is where that 
					change would be made.
	*/

	// For window and background
	const struct pixelColor BGWIN_white = { 196, 207, 161 };
	const struct pixelColor BGWIN_lgrey = { 139, 149, 109 };
	const struct pixelColor BGWIN_dgrey = { 77, 83, 60 };
	const struct pixelColor BGWIN_black = { 31, 31, 31 };

	// For sprites
	const struct pixelColor OBJS_white = { 196, 207, 161 };
	const struct pixelColor OBJS_lgrey = { 139, 149, 109 };
	const struct pixelColor OBJS_dgrey = { 77, 83, 60 };
	const struct pixelColor OBJS_black = { 31, 31, 31 };

	ppu->bitmap = VirtualAlloc(0, v_VRES * v_HRES, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	ppu->bitmap_PTR = (uint8_t*)ppu->bitmap;
	ppu->bitmapBMI = (struct bitmapStruct*)malloc(sizeof(struct bitmapStruct));

	// Initialize bitmap header
	ppu->bitmapBMI->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	ppu->bitmapBMI->bmi.bmiHeader.biWidth = v_HRES;
	ppu->bitmapBMI->bmi.bmiHeader.biHeight = -(signed)v_VRES;
	ppu->bitmapBMI->bmi.bmiHeader.biPlanes = 1;
	ppu->bitmapBMI->bmi.bmiHeader.biCompression = BI_RGB;
	ppu->bitmapBMI->bmi.bmiHeader.biBitCount = 8;
	ppu->bitmapBMI->bmi.bmiHeader.biSizeImage = 0;
	ppu->bitmapBMI->bmi.bmiHeader.biXPelsPerMeter = 0;
	ppu->bitmapBMI->bmi.bmiHeader.biYPelsPerMeter = 0;
	ppu->bitmapBMI->bmi.bmiHeader.biClrUsed = 8;
	ppu->bitmapBMI->bmi.bmiHeader.biClrImportant = 0;

	// Initialize OBJ pallete
	ppu->bitmapBMI->bmi.bmiColors[7].rgbRed = OBJS_black.rVal;		// Index three is black
	ppu->bitmapBMI->bmi.bmiColors[7].rgbBlue = OBJS_black.bVal;
	ppu->bitmapBMI->bmi.bmiColors[7].rgbGreen = OBJS_black.gVal;
	ppu->bitmapBMI->bmi.bmiColors[7].rgbReserved = 0;
	ppu->bitmapBMI->bmi.bmiColors[6].rgbRed = OBJS_dgrey.rVal;		// Index two is dark grey
	ppu->bitmapBMI->bmi.bmiColors[6].rgbBlue = OBJS_dgrey.bVal;
	ppu->bitmapBMI->bmi.bmiColors[6].rgbGreen = OBJS_dgrey.gVal;
	ppu->bitmapBMI->bmi.bmiColors[6].rgbReserved = 0;
	ppu->bitmapBMI->bmi.bmiColors[5].rgbRed = OBJS_lgrey.rVal;		// Index one is light grey
	ppu->bitmapBMI->bmi.bmiColors[5].rgbBlue = OBJS_lgrey.bVal;
	ppu->bitmapBMI->bmi.bmiColors[5].rgbGreen = OBJS_lgrey.gVal;
	ppu->bitmapBMI->bmi.bmiColors[5].rgbReserved = 0;
	ppu->bitmapBMI->bmi.bmiColors[4].rgbRed = OBJS_white.rVal;		// Index zero is white
	ppu->bitmapBMI->bmi.bmiColors[4].rgbBlue = OBJS_white.bVal;
	ppu->bitmapBMI->bmi.bmiColors[4].rgbGreen = OBJS_white.gVal;
	ppu->bitmapBMI->bmi.bmiColors[4].rgbReserved = 0;

	// Initialize BG/WIN palette
	ppu->bitmapBMI->bmi.bmiColors[3].rgbRed = BGWIN_black.rVal;		// Index three is black
	ppu->bitmapBMI->bmi.bmiColors[3].rgbBlue = BGWIN_black.bVal;
	ppu->bitmapBMI->bmi.bmiColors[3].rgbGreen = BGWIN_black.gVal;
	ppu->bitmapBMI->bmi.bmiColors[3].rgbReserved = 0;
	ppu->bitmapBMI->bmi.bmiColors[2].rgbRed = BGWIN_dgrey.rVal;		// Index two is dark grey
	ppu->bitmapBMI->bmi.bmiColors[2].rgbBlue = BGWIN_dgrey.bVal;
	ppu->bitmapBMI->bmi.bmiColors[2].rgbGreen = BGWIN_dgrey.gVal;
	ppu->bitmapBMI->bmi.bmiColors[2].rgbReserved = 0;
	ppu->bitmapBMI->bmi.bmiColors[1].rgbRed = BGWIN_lgrey.rVal;		// Index one is light grey
	ppu->bitmapBMI->bmi.bmiColors[1].rgbBlue = BGWIN_lgrey.bVal;
	ppu->bitmapBMI->bmi.bmiColors[1].rgbGreen = BGWIN_lgrey.gVal;
	ppu->bitmapBMI->bmi.bmiColors[1].rgbReserved = 0;
	ppu->bitmapBMI->bmi.bmiColors[0].rgbRed = BGWIN_white.rVal;		// Index zero is white
	ppu->bitmapBMI->bmi.bmiColors[0].rgbBlue = BGWIN_white.bVal;
	ppu->bitmapBMI->bmi.bmiColors[0].rgbGreen = BGWIN_white.gVal;
	ppu->bitmapBMI->bmi.bmiColors[0].rgbReserved = 0;
}

void free_ppu(struct PPU* ppu) 
{
	VirtualFree(&ppu->bitmap, n_TILE_COUNT * n_TILE_HEIGHT * n_TILE_WIDTH, MEM_RESET);
	DeleteObject(&ppu->bitmapBMI->bmi);
	free(ppu->bitmapBMI);
}

// PPU step function
void ppu_step(struct GB* gb, int cycles) 
{
	static struct sprite* selectedSprites[10];
	gb->ppu.mode = gb->ppu.nextMode;
	int prev_scanline, spriteCount;
	uint8_t equivalence;

	switch (gb->ppu.mode) 
	{
	case STATE_DISABLED_E:
		// Reset LY
		gb->memory[index_LY] = 0x00;
		// Enter next mode
		gb->ppu.nextMode = STATE_DISABLED_I;
		break;
	case STATE_DISABLED_I:
		break;
	case STATE_MISLATCH_E:
		// Set scanline counter, as this is sort of initialization
		gb->ppu.scanline = cycles + 4;
		// Enter next mode
		gb->ppu.nextMode = STATE_MISLATCH_I;
		break;
	case STATE_MISLATCH_I:
		// Increment scanline counter, overflow at 80
		prev_scanline = gb->ppu.scanline;
		gb->ppu.scanline = (gb->ppu.scanline + cycles) % 80;
		// Detect overflow and enter next mode accordingly
		if (prev_scanline > gb->ppu.scanline) {
			gb->memory[index_STAT] |= 0x3;
			gb->ppu.nextMode = STATE_DATATRANS_E;
		} break;
	case STATE_OAMSEARCH_E:
		// Increment LY, overflow at 154
		gb->memory[index_LY] = (gb->memory[index_LY] + 1) % SCANLINE_COUNT;
		// Test equivalence between LY and LYC and update stat register
		equivalence = gb->memory[index_LY] == gb->memory[index_LYC];
		gb->memory[index_STAT] &= ~0x04;
		gb->memory[index_STAT] |= equivalence << 2;
		// Request stat interrupt if required
		if ((gb->memory[index_STAT] & 0x40) && equivalence) gb->memory[0xFF0F] |= INTMASK_STAT;
		// Increment scanline counter
		gb->ppu.scanline += cycles;
		// Enter next mode
		gb->ppu.nextMode = STATE_OAMSEARCH_I;
		break;
	case STATE_OAMSEARCH_I:
		// Increment scanline counter, overflow at 80
		prev_scanline = gb->ppu.scanline;
		gb->ppu.scanline = (gb->ppu.scanline + cycles) % 80;
		// Detect overflow and enter next mode accordingly
		if (prev_scanline > gb->ppu.scanline) {
			gb->memory[index_STAT] |= 0x3;
			gb->ppu.nextMode = STATE_DATATRANS_E;
		} break;
	case STATE_DATATRANS_E:
		// Search OAM for visible sprites
		spriteCount = search_OAM(gb, selectedSprites, gb->memory[index_LY]);
		// Draws entire scanline for BG and win (not accurate to original timings)
		draw_scanline(gb, gb->ppu.bitmap_PTR);
		// Places sprites on the same scanline
		if (gb->memory[index_LCDC] & LCDC_OBJEN_MASK)
			for (int i = 0; i < spriteCount; i++) 
				draw_Sprite(gb, gb->ppu.bitmap_PTR, selectedSprites[i]);
		// Increment scanline counter and move bitmap pointer to the next scanline
		gb->ppu.scanline += cycles;
		gb->ppu.bitmap_PTR += 160;
		// Enter next mode
		gb->ppu.nextMode = STATE_DATATRANS_I;
		break;
	case STATE_DATATRANS_I:
		// Increment scanline counter, overflow at 172
		prev_scanline = gb->ppu.scanline;
		gb->ppu.scanline = (gb->ppu.scanline + cycles) % 172;
		// Detect overflow and enter next mode accordingly
		if (prev_scanline > gb->ppu.scanline) {
			gb->memory[index_STAT] &= ~0x3;
			gb->ppu.nextMode = STATE_HBLANK_E;
		} break;
	case STATE_HBLANK_E:
		// Increment scanline counter
		gb->ppu.scanline += cycles;
		// Enter next mode
		gb->ppu.nextMode = STATE_HBLANK_I;
		break;
	case STATE_HBLANK_I:
		// Increment scanline counter, overflow at 204
		prev_scanline = gb->ppu.scanline;
		gb->ppu.scanline = (gb->ppu.scanline + cycles) % 204;
		// Detect overflow and enter next mode accordingly based on LY
		if (prev_scanline > gb->ppu.scanline) {
			if (gb->memory[index_LY] == 143) {
				gb->memory[0xFF0F] |= INTMASK_VBLANK;
				gb->memory[index_STAT] |= 0x1;
				gb->ppu.nextMode = STATE_VBLANK_E;
			}
			else {
				gb->memory[index_STAT] |= 0x2;
				gb->ppu.nextMode = STATE_OAMSEARCH_E;
			}
		} break;
	case STATE_VBLANK_E:
		// Increment LY, overflow at 154
		gb->memory[index_LY] = (gb->memory[index_LY] + 1) % SCANLINE_COUNT;
		// Increment scanline counter
		gb->ppu.scanline += cycles;
		// Enter next mode based on scanline value
		if (gb->memory[index_LY] == 153) 
		{
			// Will need to handle scanline 153 weirdness
			gb->ppu.nextMode = STATE_SCAN_LN153_E;
		}
		else gb->ppu.nextMode = STATE_VBLANK_I;
		// Test equivalence between LYC and zero and update stat register
		equivalence = gb->memory[index_LYC] == gb->memory[index_LY];
		gb->memory[index_STAT] &= ~0x04;
		gb->memory[index_STAT] |= equivalence << 2;
		// Request stat interrupt if required
		if ((gb->memory[index_STAT] & 0x40) && equivalence) gb->memory[0xFF0F] |= INTMASK_STAT;
		break;
	case STATE_VBLANK_I:
		// Increment scanline counter, overflow at 456
		prev_scanline = gb->ppu.scanline;
		gb->ppu.scanline = (gb->ppu.scanline + cycles) % 456;
		// Detect overflow and enter next mode accordingly
		if (prev_scanline > gb->ppu.scanline) gb->ppu.nextMode = STATE_VBLANK_E;
		break;
	case STATE_SCAN_LN153_E: // Pre LY reset
		// LY resets 4 cycles into the last scanline because of a weird hardware quirk
		if (gb->ppu.scanline >= 4) {
			gb->memory[index_LY] = 0x00;
			// Update equivalence bit
			equivalence = gb->memory[index_LYC] == 0x00;
			gb->memory[index_STAT] &= ~0x04;
			gb->memory[index_STAT] |= equivalence << 2;
			// Fire interrupt if needed
			if ((gb->memory[index_STAT] & 0x40) && equivalence) gb->memory[0xFF0F] |= INTMASK_STAT;
			// No longer need to check this, move to next state
			gb->ppu.nextMode = STATE_SCAN_LN153_I;
		}
		// Increments scanline counter
		prev_scanline = gb->ppu.scanline;
		gb->ppu.scanline = (gb->ppu.scanline + cycles) % 456;
		// Detect overflow and enter next mode accordingly
		if (prev_scanline > gb->ppu.scanline)
		{
			// Push frame buffer to the screen
			gb->ppu.frameIncomplete = 0;
			gb->ppu.nextMode = STATE_OAMSEARCH_I;
		} break;
	case STATE_SCAN_LN153_I:
		// Increments scanline counter
		prev_scanline = gb->ppu.scanline;
		gb->ppu.scanline = (gb->ppu.scanline + cycles) % 456;
		// Detect overflow and enter next mode accordingly
		if (prev_scanline > gb->ppu.scanline)
		{	// Push frame buffer to the screen
			gb->ppu.frameIncomplete = 0;
			gb->ppu.nextMode = STATE_OAMSEARCH_I;
		} break;
	}

	// Set an IF bit based on STAT enabled, if able to
	switch (gb->ppu.mode & 0x3) 
	{
		case 0x0: if (gb->memory[index_STAT] & 0x08) gb->memory[0xFF0F] |= INTMASK_STAT; break;
		case 0x1: if (gb->memory[index_STAT] & 0x10) gb->memory[0xFF0F] |= INTMASK_STAT; break;
		case 0x2: if (gb->memory[index_STAT] & 0x20) gb->memory[0xFF0F] |= INTMASK_STAT; break;
		// data transfer entry wont trigger a stat interrupt ... ever, no 0x3
	}
}

// Tile addressing modes for WIN/BG (LCDC bit 4)
struct tileStruct* unsigned_tile_addressing(struct GB* gb, uint16_t map_base, uint8_t LY, int tileColumn) 
{ // LCDC bit 4 = 1
	uint8_t index = gb->memory[map_base + tileColumn + (((uint16_t)LY / 8) * 32)];
	return (struct tileStruct*)&gb->memory[0x8000 + (uint16_t)index * 16];
}
struct tileStruct* signed_tile_addressing(struct GB* gb, uint16_t map_base, uint8_t LY, int tileColumn) 
{ // LCDC bit 4 = 0
	uint8_t index = gb->memory[map_base + tileColumn + (((uint16_t)LY / 8) * 32)];
	// Byte must be converted to signed 16 bit value
	int8_t temp = (int8_t)index;
	int16_t signed_index = (int16_t)temp;
	return (struct tileStruct*)&gb->memory[0x9000 + (signed_index * 16)];
}

// Draws an entire scanline - not accurate to original hardware, but both this emulator and
//		the original hardware should still eventually catch up and end up with the same result
void draw_scanline(struct GB* gb, uint8_t* bitmapPtr)
{
	int currentPixel = 0;    // To keep track of current pixel on scanline

	// Tile map base addresses
	uint16_t bg_map_base  = gb->memory[index_LCDC] & LCDC_BGMAP_MASK  ? 0x9C00 : 0x9800;
	uint16_t win_map_base = gb->memory[index_LCDC] & LCDC_WINMAP_MASK ? 0x9C00 : 0x9800;

	// Get tile addressing mode based on tile data area bit, tile base address
	struct tileStruct*(*addressing_mode)(struct GB*, uint16_t, uint8_t, int) = 
		(gb->memory[index_LCDC] & LCDC_WINBG_MASK) ? &unsigned_tile_addressing : &signed_tile_addressing;

	// If BG/Window are enabled
	if (gb->memory[index_LCDC] & LCDC_BGWINP_MASK) {

		// Used to locate tile in memory
		uint8_t BG_scrolledY = gb->memory[index_LY] + gb->memory[index_SCY];
		uint8_t BG_rowOffset = BG_scrolledY % 8; // determines which row index from tile to read
		uint8_t BG_scrolledX = (gb->memory[index_SCX] >> 3) % 32;

		// Used to keep track of current tile being drawn
		struct tileStruct* currentTile = (*addressing_mode)(gb, bg_map_base, BG_scrolledY, BG_scrolledX);
		uint8_t tileBit = 7 - (gb->memory[index_SCX] % 8);
		
		// Prep for drawing BG
		int bg_stop = 160; // When to stop drawing BG pixels
		if ((gb->memory[index_WX] - 7 < 160) && (gb->memory[index_WY] <= gb->memory[index_LY]) && (gb->memory[index_LCDC] & LCDC_WINEN_MASK))
			bg_stop = gb->memory[index_WX] - 7;

		// Drawing background
		while (currentPixel < bg_stop) 
		{
			// Write color data to frame buffer
			uint8_t lo = (currentTile->bytes[BG_rowOffset][1]) >> tileBit;
			uint8_t hi = (currentTile->bytes[BG_rowOffset][0]) >> tileBit;
			switch (((lo & 0x1) << 1) | (hi & 0x1)) 
			{
				case 0: *bitmapPtr =  gb->memory[index_BGP] & 0b00000011;	    break; 
				case 1: *bitmapPtr = (gb->memory[index_BGP] & 0b00001100) >> 2; break;
				case 2: *bitmapPtr = (gb->memory[index_BGP] & 0b00110000) >> 4; break;
				case 3: *bitmapPtr = (gb->memory[index_BGP] & 0b11000000) >> 6; break;
			}

			// Reset tile bit if needed
			if (tileBit == 0) 
			{
				// Move X tile along, and refetch the tile data
				BG_scrolledX = (BG_scrolledX + 1) % 32;
				currentTile = (*addressing_mode)(gb, bg_map_base, BG_scrolledY, BG_scrolledX);
				tileBit = 7;
			}
			else tileBit--;
			bitmapPtr++; currentPixel++;
		}
		
		int windowVisible = 0;

		// Prep for drawing win
		uint8_t win_scrolledX = 0; // Always starts at tile zero
		uint8_t win_rowOffset = gb->ppu.win_LY % 8;
		currentTile = (*addressing_mode)(gb, win_map_base, gb->ppu.win_LY, win_scrolledX);
		tileBit = (bg_stop < 0) ? (7 - bg_stop) : 7;

		// Drawing window
		while (currentPixel < 160) 
		{
			windowVisible = 1;
			// Write color data to frame buffer
			uint8_t lo = (currentTile->bytes[win_rowOffset][1]) >> tileBit;
			uint8_t hi = (currentTile->bytes[win_rowOffset][0]) >> tileBit;
			switch (((lo & 0x1) << 1) | (hi & 0x1)) {
				case 0: *bitmapPtr =  gb->memory[index_BGP] & 0b00000011;		break;
				case 1: *bitmapPtr = (gb->memory[index_BGP] & 0b00001100) >> 2; break;
				case 2: *bitmapPtr = (gb->memory[index_BGP] & 0b00110000) >> 4; break;
				case 3: *bitmapPtr = (gb->memory[index_BGP] & 0b11000000) >> 6; break;
			}

			// Reset tile bit if needed
			if (tileBit == 0) {

				// Move X tile along, and refetch tile data
				win_scrolledX = (win_scrolledX + 1) % 32; // Shouldn't wrap but screw it
				currentTile = (*addressing_mode)(gb, win_map_base, gb->ppu.win_LY, win_scrolledX);
				tileBit = 7;
			}
			else tileBit--;
			bitmapPtr++; currentPixel++;
		}

		// If the window was visible, the internal scanline counter must be incremented
		if (windowVisible) gb->ppu.win_LY++;
	}

	else {
		// Draw white for the entire scanline
		for (int i = 0; i < 160; i++) {
			*bitmapPtr = 0x00;
			bitmapPtr++;
		}
	}

}

// Pushes bitmap to screen - also contains commented out code which
//		synchronizes the emulation to real time around video. It is
//		better to synchronize the emulation to audio, which I plan
//		on doing. I will leave this here until I'm fully committed
//		to my APU and I'm sure it won't cause any problems
void draw_to_screen(struct GB* gb, HWND window, HDC hdc)
{
	gb->ppu.frameIncomplete = 1;
	static LARGE_INTEGER start = { 0,0 };

	// Push bitmap
	gb->ppu.bitmap_PTR = (uint8_t*)gb->ppu.bitmap;
	StretchDIBits(hdc, 0, 0, v_WIDTH, v_HEIGHT, 0, 0, v_HRES, v_VRES, gb->ppu.bitmap, &gb->ppu.bitmapBMI->bmi, DIB_RGB_COLORS, SRCCOPY);
	
	// Handle sync to video
	LARGE_INTEGER end, frequency;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&end);
	float secsPerFrame = ((float)(end.QuadPart - start.QuadPart) / (float)frequency.QuadPart); 
	while (secsPerFrame < 0.01666667f) {
		QueryPerformanceCounter(&end);
		secsPerFrame = ((float)(end.QuadPart - start.QuadPart) / (float)frequency.QuadPart);
	}
	MSG msg;
	while (PeekMessage(&msg, window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Reset starting timer and internal counter for WL
	gb->ppu.win_LY = 0x00;
	QueryPerformanceCounter(&start);
}

// PPU Lookahead Reads
uint8_t STAT_RB(struct GB* gb, uint8_t cycles)
{
	// TODO: Handle lookahead
	return gb->memory[index_STAT] | 0x80;
}
uint8_t LY_RB(struct GB* gb, uint8_t cycles) 
{
	uint8_t curr_LY = gb->memory[index_LY];
	if (gb->ppu.enabled) {
		int new_scanline = (gb->ppu.scanline + cycles) % SCANLINE_LENGTH;
		if (OVERFLOW(gb->ppu.scanline, new_scanline)) curr_LY = (curr_LY + 1) % 153;
	}
	return curr_LY;
}

// PPU Registers Synchronized Writes
void LCDC_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->memory[index_LCDC] = val;
	if (!gb->ppu.enabled) {
		if (val & 0x80) {
			gb->ppu.bitmap_PTR = (uint8_t*)gb->ppu.bitmap;
			gb->ppu.nextMode = STATE_MISLATCH_E;
			gb->ppu.enabled = 1;
			gb->ppu.win_LY = 0;
		}
	}
	else {
		if (!(val & 0x80)) {
			gb->ppu.nextMode = STATE_DISABLED_E;
			gb->ppu.enabled = 0;
		}
	}
	gb->sync_sel = 1;
}
void STAT_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	// Lower two bits are read only
	gb->memory[index_STAT] &= 0x7;
	gb->memory[index_STAT] |= (val & 0xF8);
	gb->sync_sel = 1;
}
void backgroundY_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->memory[index_SCY] = val;
	gb->sync_sel = 1;
}
void backgroundX_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->memory[index_SCX] = val;
	gb->sync_sel = 1;
}
void LY_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	// Byte is read only
	ppu_step(gb, cycles);
	gb->sync_sel = 1;
}
void LYC_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->memory[index_LYC] = val;

	// Test equivalence between LY and LYC and update stat register
	uint8_t equivalence = gb->memory[index_LY] == gb->memory[index_LYC];
	gb->memory[index_STAT] &= ~0x04;
	gb->memory[index_STAT] |= equivalence << 2;
	// Request stat interrupt if required
	if ((gb->memory[index_STAT] & 0x40) && equivalence) gb->memory[0xFF0F] |= INTMASK_STAT;

	gb->sync_sel = 1;
}
void windowY_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->memory[index_WY] = val;
	gb->sync_sel = 1;
}
void windowX_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->memory[index_WX] = val;
	gb->sync_sel = 1;
}

#include <Windows.h>
#include "ppu.h"
#include "hwio.h"
#include "mem.h"

// PPU initialize
void init_ppu(struct PPU* ppu) 
{	// Set the PPU state, just to be safe
	ppu->state = statModeOamSearch;
	// Reset the PPU scanline dot counter and stat irq signal
	ppu->stat_irq_signal = 0;
	ppu->dotCounter = 0;
	// By default, the frame will be incomplete, this variable
	//		is intended to act like a boolean
	ppu->frameIncomplete = 1;
	// Initialize known register values
	ppu->reg_lcdc = 0x00;
	ppu->reg_stat = 0x80;
	ppu->reg_scx  = 0x00;
	ppu->reg_scy  = 0x00;
	ppu->reg_ly   = 0x00;
	ppu->reg_lyc  = 0x00;
	ppu->reg_dma  = 0xFF;
	ppu->reg_bgp  = 0xFC;
	ppu->reg_obp0 = 0xFF;
	ppu->reg_obp1 = 0xFF;
	// Initialize internal scanline counter for the window
	ppu->win_ly = 0x00;

	struct pixelColor 
	{	// Helper structure for initializing the bitmap palette
		uint8_t rVal, gVal, bVal;
	};

	// If one wishes to change the color pallete, that is done here
	const struct pixelColor BGWIN_white = { 196, 207, 161 };
	const struct pixelColor BGWIN_lgrey = { 139, 149, 109 };
	const struct pixelColor BGWIN_dgrey = { 77, 83, 60 };
	const struct pixelColor BGWIN_black = { 31, 31, 31 };

	ppu->bitmap = (uint8_t*)VirtualAlloc(0, v_VRES * v_HRES, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
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
	VirtualFree(&ppu->bitmap, v_VRES * v_HRES, MEM_RESET);
	DeleteObject(&ppu->bitmapBMI->bmi);
	free(ppu->bitmapBMI);
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

// PPU step function
void ppu_step(struct GB* gb, int cycles) 
{	// Look up tables for entry and exit functions for each PPU mode
	static const uint8_t(*exitFuncs[6])(struct GB*)
		= { &lHBlank, &lVBlank, &lOamSearch, &lDataTrans, &lScanln153First4Cycles, &lScanln153RemainingCycles };
	static const void(*entryFuncs[6])(struct GB*)
		= { &eHBlank, &eVBlank, &eOamSearch, &eDataTrans, &eScanln153First4Cycles, &eScanln153RemainingCycles };
	// Look up table for the maximum value of the dot counter for a given mode,
	//		note, this value actually fluctuates, but I don't plan on emulating
	//		the timing here perfectly, at least not just yet.
	static const int maxDots[6] = { 456, 456, 80, 252, 4, 456 };
	
	// All cycles passed into the cycles parameter will be spent here. If cycles from one instruction
	//		bleed into another PPU mode, then those cycles will still be spent here, in a seperate
	//		recursive call. 
	if (gb->ppu.ppu_enable)
	{
		int curMaxDot = maxDots[gb->ppu.state];
		// Calculate timing information and update the dot counter
		struct ppuCycleTimingInfo timing = calcPpuTimingInfo(gb->ppu.dotCounter, curMaxDot, cycles);
		int oldDotCounter  = gb->ppu.dotCounter; // Keep track of this to check for overflow
		gb->ppu.dotCounter = (gb->ppu.dotCounter + timing.spentCycles) % 456;
		// Determine if there are any bleed over cycles, and if there are the mode the PPU 
		//		was in is now complete and it is time to start executing the next mode.
		if (timing.leftOverCycles > 0 || oldDotCounter > gb->ppu.dotCounter)
		{	// Since there are clock cycles that bled into the next mode, the exit function of the
			//		old mode can be called. 
			uint8_t newMode = (*exitFuncs[gb->ppu.state])(gb);
			// In addition, the entry function of the new mode being entered can be called. Which
			//		function is called is determined by the next state logic of the exit function.
			(*entryFuncs[newMode])(gb);
			// The dot counter has only been incremented by the amount of spent cycles up to this
			//		point. There are still cycles that have bled over into the new mode. The rest
			//		is handled recursively. Note, ppu.state is updated in exit functions. 
			ppu_step(gb, timing.leftOverCycles);
		}
		// Update the internal stat irq signal, and trigger an interrupt when appropriate
		update_stat_signal(gb);
	}
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

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
	QueryPerformanceCounter(&start);
}

void update_stat_signal(struct GB* gb)
{
	uint8_t old = gb->ppu.stat_irq_signal;
	// The conditions following the boolean ands are the enables for the 
	//		respective interrupt sources
	uint8_t cond1 = (gb->ppu.reg_ly == gb->ppu.reg_lyc) && (gb->ppu.ly_is_lyc_stat_src);
	uint8_t cond2 = (gb->ppu.mode_bits == 0x0) && (gb->ppu.hblank_stat_src);
	uint8_t cond3 = (gb->ppu.mode_bits == 0x2) && (gb->ppu.oam_stat_src);
	// According to the docs, for what ever reason either one of the enables below satisfies
	//		this condition and will set the internal signal. Makes no sense, but okay
	uint8_t cond4 = (gb->ppu.mode_bits == 0x1) && (gb->ppu.oam_stat_src || gb->ppu.vblank_stat_src);
	// Update the signal, and trigger an interrupt if a rising edge is detected
	gb->ppu.stat_irq_signal = cond1 || cond2 || cond3 || cond4;
	if (gb->ppu.stat_irq_signal && !old) setIFBit(gb, 1);
	// Update the ly is lyc bit
	gb->ppu.ly_is_lyc = (gb->ppu.reg_ly == gb->ppu.reg_lyc);
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t LCDC_RB(struct GB* gb, uint8_t cycles)
{
	return gb->ppu.reg_lcdc;
}
uint8_t STAT_RB(struct GB* gb, uint8_t cycles) // NEEDS WORK
{
	// Big 7 is unused and always returns one
	return gb->ppu.reg_stat | 0x80;
}
uint8_t SCY_RB(struct GB* gb, uint8_t cycles)
{
	return gb->ppu.reg_scy;
}
uint8_t SCX_RB(struct GB* gb, uint8_t cycles) 
{
	return gb->ppu.reg_scx;
}
uint8_t LY_RB(struct GB* gb, uint8_t cycles)  // NEEDS WORK
{
	return gb->ppu.reg_ly;
}
uint8_t LYC_RB(struct GB* gb, uint8_t cycles)
{
	return gb->ppu.reg_lyc;
}
uint8_t DMA_RB(struct GB* gb, uint8_t cycles) 
{
	return gb->ppu.reg_dma;
}
uint8_t BGP_RB(struct GB* gb, uint8_t cycles)
{
	return gb->ppu.reg_bgp;
}
uint8_t OBP0_RB(struct GB* gb, uint8_t cycles)
{
	return gb->ppu.reg_obp0;
}
uint8_t OBP1_RB(struct GB* gb, uint8_t cycles)
{
	return gb->ppu.reg_obp1;
}
uint8_t WY_RB(struct GB* gb, uint8_t cycles)
{
	return gb->ppu.reg_wy;
}
uint8_t WX_RB(struct GB* gb, uint8_t cycles)
{
	return gb->ppu.reg_wx;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

void LCDC_WB(struct GB* gb, uint8_t val, uint8_t cycles) // NEEDS WORK
{
	ppu_step(gb, cycles);
	uint8_t oldEnableValue = gb->ppu.ppu_enable;
	// Need to check the PPU enable to determine if the PPU is going from on
	//		to off or vice versa. I don't believe writing to this register 
	//		without changing this bit does anything significant.
	if (oldEnableValue && !(val & 0x80) /* PPU turning off */)
	{	// Reset the dot counter
		gb->ppu.dotCounter = 0;
		// Reset stat internal signal
		gb->ppu.stat_irq_signal = 0;
		// Reset LY
		gb->ppu.reg_ly = 0x00;
		// This is just zero when its off
		gb->ppu.mode_bits = 0b00;
	}
	else if (!oldEnableValue && (val & 0x80) /* PPU turning on */)
	{	// Reset the dot counter
		gb->ppu.dotCounter = 0;
		// Reset LY
		gb->ppu.reg_ly = 0x00;
		// Reset the state to OAM search, make sure to update mode bits
		gb->ppu.state = statModeOamSearch;
		gb->ppu.mode_bits = 0b10;
	}
	// Update the actual value of the register
	gb->ppu.reg_lcdc = val;
	gb->sync_sel = 1;
}
void STAT_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	// Mode bits (0 and 1) are read only and 8 is unused, don't write them
	gb->ppu.reg_stat &= 0x87;
	gb->ppu.reg_stat |= (val & ~0x87);
	gb->sync_sel = 1;
}
void SCY_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->ppu.reg_scy = val;
	gb->sync_sel = 1;
}
void SCX_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->ppu.reg_scx = val;
	gb->sync_sel = 1;
}
void LY_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	// This value is read only, do not mess with this at all
	gb->sync_sel = 1;
}
void LYC_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->ppu.reg_lyc = val;
	gb->sync_sel = 1;
}
// Note, I am doing my DMA transfers using a single memcpy within a single instruction. This is obviously
//		not accurate at all. Software usually copies a short routine to wait for it to complete and this
//		emulator will still emulate that just fine so it shouldn't matter much
#define OAM_SIZE_BYTES 0xA0
void DMA_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->ppu.reg_dma = val;
	// Perform data transfer
	uint16_t start_addr = val << 8;
	if (val >= 0x00 && val <= 0x3F) {
		memcpy(gb->memory + 0xFE00, gb->cart.ROM00_ptr + start_addr, OAM_SIZE_BYTES);
	}
	else if (val >= 0x40 && val <= 0x7F) {
		start_addr = start_addr - 0x4000;
		memcpy(gb->memory + 0xFE00, gb->cart.ROMnn_ptr + start_addr, OAM_SIZE_BYTES);
	}
	else if (val >= 0x80 && val <= 0x9F) {
		memcpy(gb->memory + 0xFE00, gb->memory + start_addr, OAM_SIZE_BYTES);
	}
	else if (val >= 0xA0 && val <= 0xBF) {
		// TODO: This is not safe, do a check on this ram pointer before calling memcpy
		start_addr = start_addr - 0xA000;
		memcpy(gb->memory + 0xFE00, gb->cart.RAM_ptr + start_addr, OAM_SIZE_BYTES);
	}
	else if (val >= 0xC0 && val <= 0xDF) {
		memcpy(gb->memory + 0xFE00, gb->memory + start_addr, OAM_SIZE_BYTES);
	}
	else if (val >= 0xE0 && val <= 0xFD) {
		start_addr = start_addr - 0x2000; // Mirror of the address range 8 kilobytes back
		memcpy(gb->memory + 0xFE00, gb->memory + start_addr, OAM_SIZE_BYTES);
	}
	gb->sync_sel = 1;
}
#undef OAM_SIZE_BYTES
void BGP_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->ppu.reg_bgp = val;
	gb->sync_sel = 1;
}
void OBP0_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->ppu.reg_obp0 = val;
	gb->sync_sel = 1;
}
void OBP1_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->ppu.reg_obp1 = val;
	gb->sync_sel = 1;
}
void WY_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->ppu.reg_wy = val;
	gb->sync_sel = 1;
}
void WX_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	ppu_step(gb, cycles);
	gb->ppu.reg_wx = val;
	gb->sync_sel = 1;
}

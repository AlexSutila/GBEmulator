#include <Windows.h>
#include "ppu.h"
#include "hwio.h"
#include "mem.h"

// PPU initialize
void init_ppu(struct PPU* ppu) 
{
	struct pixelColor 
	{
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

// PPU step function
void ppu_step(struct GB* gb, int cycles) 
{
	
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

// ////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t LCDC_RB(struct GB* gb, uint8_t cycles)
{

	return 0x00;
}
uint8_t STAT_RB(struct GB* gb, uint8_t cycles)
{

	return 0x00;
}
uint8_t SCY_RB(struct GB* gb, uint8_t cycles)
{

	return 0x00;
}
uint8_t SCX_RB(struct GB* gb, uint8_t cycles) 
{

	return 0x00;
}
uint8_t LY_RB(struct GB* gb, uint8_t cycles)
{

	return 0x00;
}
uint8_t LYC_RB(struct GB* gb, uint8_t cycles)
{

	return 0x00;
}
uint8_t DMA_RB(struct GB* gb, uint8_t cycles)
{

	return 0x00;
}
uint8_t BGP_RB(struct GB* gb, uint8_t cycles)
{

	return 0x00;
}
uint8_t OBP0_RB(struct GB* gb, uint8_t cycles)
{

	return 0x00;
}
uint8_t OBP1_RB(struct GB* gb, uint8_t cycles)
{

	return 0x00;
}
uint8_t WY_RB(struct GB* gb, uint8_t cycles)
{

	return 0x00;
}
uint8_t WX_RB(struct GB* gb, uint8_t cycles)
{

	return 0x00;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

void LCDC_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	
	gb->sync_sel = 1;
}
void STAT_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{

	gb->sync_sel = 1;
}
void SCY_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{

	gb->sync_sel = 1;
}
void SCX_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{

	gb->sync_sel = 1;
}
void LY_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{

	gb->sync_sel = 1;
}
void LYC_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{

	gb->sync_sel = 1;
}
void DMA_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{

	gb->sync_sel = 1;
}
void BGP_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{

	gb->sync_sel = 1;
}
void OBP0_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{

	gb->sync_sel = 1;
}
void OBP1_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{

	gb->sync_sel = 1;
}
void WY_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{

	gb->sync_sel = 1;
}
void WX_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{

	gb->sync_sel = 1;
}

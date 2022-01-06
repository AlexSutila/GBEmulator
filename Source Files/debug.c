#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include "debug.h"
#include "mem.h"

#define CONSOLE_WIDTH 120
#define CONSOLE_HEIGHT 40

static DWORD bytesWritten = 0;
static wchar_t buffer[CONSOLE_WIDTH * CONSOLE_HEIGHT] = { 
	L"                                                                                                                        "
	L" AF: 0000 BC: 0000 DE: 0000   Flags: - - - - - - - -                                                                    "
	L" HL: 0000 SP: 0000 PC: 0000   IME: 0      Halted: 0                                                                     "
	L"                                                                                                                        "
	L" MEM: -0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -A -B -C -D -E -F                                                                   "
	L" 000- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 001- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 002- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 003- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 004- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 005- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 006- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 007- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 008- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 009- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00A- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00B- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00C- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00D- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00E- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00F- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L"                                                                                                                        "
	L" JOYP: 00    DMA: 00             IE    IF                                                                               "
	L"  DIV: 00     LY: 00   VBLNK:                                                                                           "
	L" TIMA: 00    LYC: 00    STAT:                                                                                           "
	L"  TMA: 00     WY: 00   TIMER:                                                                                           "
	L"  TAC: 00     WX: 00   SRIAL:                                                                                           "
	L" LCDC: 00    SCY: 00    JOYP:                                                                                           "
	L" STAT: 00    SCX: 00                                                                                                    "
	L"                                                                                                                        "
};

void debug_init(HANDLE* hConsole)
{
	// Create debugger console
	AllocConsole();
	DWORD id = GetCurrentProcessId();
	AttachConsole(id);

	// Initialize console buffer
	*hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(*hConsole);
	WriteConsoleOutputCharacter(*hConsole, buffer, CONSOLE_WIDTH * CONSOLE_HEIGHT, (COORD){ 0,0 }, &bytesWritten);
}

void debug_deinit()
{
	FreeConsole();
}

// Updates console buffer with contents from gb
void refresh_console(struct GB* gb, HANDLE* hConsole, uint16_t memViewBase)
{
	// Update CPU State
	swprintf(buffer + 125, 5, L"%04X", gb->cpu.AF);
	swprintf(buffer + 134, 5, L"%04X", gb->cpu.BC);
	swprintf(buffer + 143, 5, L"%04X", gb->cpu.DE);
	swprintf(buffer + 245, 5, L"%04X", gb->cpu.HL);
	swprintf(buffer + 254, 5, L"%04X", gb->cpu.SP);
	swprintf(buffer + 263, 5, L"%04X", gb->cpu.PC);

	swprintf(buffer + 290, 2, L"%d", gb->cpu.halt);
	swprintf(buffer + 275, 2, L"%d", gb->cpu.IME);
	buffer[157] = (gb->cpu.F & 0x80) ? 'Z' : '-';
	buffer[159] = (gb->cpu.F & 0x40) ? 'N' : '-';
	buffer[161] = (gb->cpu.F & 0x20) ? 'H' : '-';
	buffer[163] = (gb->cpu.F & 0x10) ? 'C' : '-';

	// Update important register view
	swprintf(buffer + 2647, 3, L"%02X", RB(gb, 0xFF00, 0));
	swprintf(buffer + 2767, 3, L"%02X", RB(gb, 0xFF04, 0));
	swprintf(buffer + 2887, 3, L"%02X", RB(gb, 0xFF05, 0));
	swprintf(buffer + 3007, 3, L"%02X", RB(gb, 0xFF06, 0));
	swprintf(buffer + 3127, 3, L"%02X", RB(gb, 0xFF07, 0));
	swprintf(buffer + 3247, 3, L"%02X", RB(gb, 0xFF40, 0));
	swprintf(buffer + 3367, 3, L"%02X", RB(gb, 0xFF41, 0));
	swprintf(buffer + 2658, 3, L"%02X", RB(gb, 0xFF46, 0));
	swprintf(buffer + 2778, 3, L"%02X", RB(gb, 0xFF44, 0));
	swprintf(buffer + 2898, 3, L"%02X", RB(gb, 0xFF45, 0));
	swprintf(buffer + 3018, 3, L"%02X", RB(gb, 0xFF4A, 0));
	swprintf(buffer + 3138, 3, L"%02X", RB(gb, 0xFF4B, 0));
	swprintf(buffer + 3258, 3, L"%02X", RB(gb, 0xFF42, 0));
	swprintf(buffer + 3378, 3, L"%02X", RB(gb, 0xFF43, 0));

	// Update interrupt registers IE and IF
	uint8_t IE = RB(gb, 0xFFFF, 0), IF = RB(gb, 0xFF0F, 0);
	buffer[2794] = (IE & 0x01) ? 'X' : '-'; buffer[2800] = (IF & 0x01) ? 'X' : '-';
	buffer[2914] = (IE & 0x02) ? 'X' : '-'; buffer[2920] = (IF & 0x02) ? 'X' : '-';
	buffer[3034] = (IE & 0x04) ? 'X' : '-'; buffer[3040] = (IF & 0x04) ? 'X' : '-';
	buffer[3154] = (IE & 0x08) ? 'X' : '-'; buffer[3160] = (IF & 0x08) ? 'X' : '-';
	buffer[3274] = (IE & 0x10) ? 'X' : '-'; buffer[3280] = (IF & 0x10) ? 'X' : '-';

	// Update Memory View
	for (int row = 5; row <= 20; row++) {
		swprintf(buffer + 1 + (120 * row), 5, L"%02X%X-", memViewBase >> 8, row - 5);
		for (int col = 0; col <= 0xF; col++) {
			swprintf((120 * row) + (buffer + 6) + (col * 3), 3, L"%02X", RB(gb, memViewBase, 0));
			memViewBase++;
		}
	}

	// Write results to console buffer
	WriteConsoleOutputCharacter(*hConsole, buffer, CONSOLE_WIDTH * CONSOLE_HEIGHT, (COORD) { 0, 0 }, & bytesWritten);
}

// Steps one instruction (Mostly copied from main)
void step_emulation(struct GB* gb, HWND window, HDC hdc)
{
	// Execute instruction and record timing
	struct instrTimingInfo timing = (gb->cpu.halt != 1) ?
		cpu_execute(gb) : (struct instrTimingInfo) { 4, 0 };

	// Handle interrupts and record timing, int_cycles = 0 if no interrupts serviced
	int int_cycles = int_request(gb, timing.cycles);

	// Sync components based on this timing info 
	int total = timing.cycles + int_cycles;
	switch (gb->sync_sel) {
	case 0: // ALL
		timers_step(gb, total);
		ppu_step(gb, total);
		break;
	case 1: // PPU
		timers_step(gb, total);
		ppu_step(gb, timing.postWrite_cycles + int_cycles);
		gb->sync_sel = 0;
		break;
	case 2: // Timers
		timers_step(gb, timing.postWrite_cycles + int_cycles);
		ppu_step(gb, total);
		gb->sync_sel = 0;
		break;
	}

	if (!gb->ppu.frameIncomplete)
	{
		gb->ppu.bitmap_PTR = (uint8_t*)gb->ppu.bitmap;
		gb->ppu.win_LY = 0x00; 
		StretchDIBits(hdc, 0, 0, v_WIDTH, v_HEIGHT, 0, 0, v_HRES, v_VRES, gb->ppu.bitmap, &gb->ppu.bitmapBMI->bmi, DIB_RGB_COLORS, SRCCOPY);
		gb->ppu.frameIncomplete = 1;
	}
}

void debug_break(struct GB* gb, HANDLE* hConsole, HWND window, HDC hdc)
{
	char c = ' ';
	static uint16_t memViewBase = 0x0000; 

	while (c != 'c')
	{
		refresh_console(gb, hConsole, memViewBase);
		c = _getch();

		switch (c)
		{
		case 'n': // Step emulation by one instruction
			step_emulation(gb, window, hdc);                                            break;
		case 'N': // Step emulation by one hundred isntructions
			for (int i = 0; i < 100; i++) step_emulation(gb, window, hdc);              break;
		case 'w': // Scroll memview up (decrement address)
			memViewBase = (memViewBase != 0x0000) ? memViewBase - 0x0100 : memViewBase; break;
		case 's': // Scroll memview down (increment address)
			memViewBase = (memViewBase != 0xFF00) ? memViewBase + 0x0100 : memViewBase; break;
		}
	}
}

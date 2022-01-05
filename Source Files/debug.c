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
	L" AF: 0000 BC: 0000 DE: 0000   Flags: - - - -                                                                            "
	L" HL: 0000 SP: 0000 PC: 0000   IME: 0                                                                                    "
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
	L"                                                                                                                        "
	L"                                                                                                                        "
	L"                                                                                                                        "
	L"                                                                                                                        "
	L"                                                                                                                        "
	L"                                                                                                                        "
	L"                                                                                                                        "
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
	// Update Register Values
	swprintf(buffer + 125, 5, L"%04X", gb->cpu.AF);
	swprintf(buffer + 134, 5, L"%04X", gb->cpu.BC);
	swprintf(buffer + 143, 5, L"%04X", gb->cpu.DE);
	swprintf(buffer + 245, 5, L"%04X", gb->cpu.HL);
	swprintf(buffer + 254, 5, L"%04X", gb->cpu.SP);
	swprintf(buffer + 263, 5, L"%04X", gb->cpu.PC);

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

// Steps one instruction (copied from main)
void step_emulation(struct GB* gb)
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
}

void debug_break(struct GB* gb, HANDLE* hConsole)
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
			step_emulation(gb);
			break;
		case 'w': // Scroll memview up (decrement address)
			memViewBase = (memViewBase != 0x0000) ? memViewBase - 0x0100 : memViewBase;
			break;
		case 's': // Scroll memview down (increment address)
			memViewBase = (memViewBase != 0xFF00) ? memViewBase + 0x0100 : memViewBase;
			break;
		}
	}
}

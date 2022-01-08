#include <Windows.h>
#include "mem.h"

// TEMPORARY
#include "apu.h"

#ifndef NDEBUG
#include "debug.h"
#endif

// Global running parameter
int running = 1;

// Callback
LRESULT CALLBACK WindowProc(HWND win, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	LRESULT result = 0;
	switch (msg) 
	{
	case WM_CLOSE:
		running = 0;
		break;
	default:
		result = DefWindowProc(win, msg, wParam, lParam);
		break;
	}
	return result;
}

// Main
int WINAPI WinMain(HINSTANCE instance, HINSTANCE pInstance, PWSTR cmdLine, int cmdShow) 
{
	WNDCLASSW windowClass = { 0 };
	wchar_t className[] = L"GameBoy Emulator";

	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = className;

	RegisterClass(&windowClass);
	HWND window = CreateWindowEx(0, className, L"GameBoy Emulator", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, (v_HRES + 4) * 4, (v_VRES + 10) * 4, 0, 0, instance, 0);
	HDC hdc = GetDC(window);

	// Debugger initialization
	#ifndef NDEBUG
	HANDLE hConsole;
	debug_init(&hConsole);
	#endif

	// Emulator initialization
	struct GB emulator;
	gb_init(&emulator);

	// Main loop
	while (running) 
	{
		while (emulator.ppu.frameIncomplete) 
		{
			// Execute instruction and record timing
			struct instrTimingInfo timing = (emulator.cpu.halt != 1) ?
				cpu_execute(&emulator) : (struct instrTimingInfo) { 4, 0 };
	
			// Handle interrupts and record timing, int_cycles = 0 if no interrupts serviced
			int int_cycles = int_request(&emulator, timing.cycles);
	
			// Sync components based on this timing info 
			int total = timing.cycles + int_cycles;
			switch (emulator.sync_sel) {
			case 0: // ALL
				timers_step(&emulator, total);
				ppu_step(&emulator, total);
				break;
			case 1: // PPU
				timers_step(&emulator, total);
				ppu_step(&emulator, timing.postWrite_cycles + int_cycles);
				emulator.sync_sel = 0;
				break;
			case 2: // Timers
				timers_step(&emulator, timing.postWrite_cycles + int_cycles);
				ppu_step(&emulator, total);
				emulator.sync_sel = 0;
				break;
			}
			// I don't think precise timing with the APU is as important, still thinking
			apu_step(&emulator, window, total);
		}
	
		// Push frame to screen also handles real time synchronization
		draw_to_screen(&emulator, window, hdc);
		update_keyStates(&emulator);

		#ifndef NDEBUG
		// Update debug info and show console if requested
		if (GetAsyncKeyState(0x42)) debug_break(&emulator, &hConsole, window, hdc);
		#endif
	}

	#ifndef NDEBUG
	debug_deinit();
	#endif

	gb_free(&emulator);
	return 0;
}

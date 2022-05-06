#include <Windows.h>
#include "mem.h"

#ifndef NDEBUG
#include "debug.h"
#endif

// Condition to keep emulator running, to be set in callback upon exit
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

// Win32 doesn't do int main like normal, so this is where the entry point is
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
	struct breakpoint breakpoints[6];
	debug_init(&hConsole, breakpoints);
	#endif

	// Emulator initialization
	struct GB emulator;
	gb_init(&emulator);

	// Emulator runs the WindowProc callback function tells it to stop upon a request to close the window.
	while (running) 
	{
		while (emulator.ppu.frameIncomplete) 
		{
			// The CPU's halt mode will stop the execution of instructions until an interrupt signal is recieved.
			//		To my current understanding, all other components will still be clocked despite the CPU not
			//		executing any instructions based on how many cycles the instruction needed. 
			//		...
			// Since instructions are not executed in halt mode, the timing info is set to { 4, 0 } in order to
			//		cycle all components 4 clock cycles. 
			//		...
			// This emulator currently has a terribly incomplete implementation of the stop mode, and I will return
			//		to it eventually (hopefully)

			// Execute instruction and record timing
			struct instrTimingInfo timing = (emulator.cpu.halt != 1) ?
				cpu_execute(&emulator) : (struct instrTimingInfo) { 4, 0 };
	
			// The total amount of clock cycles elapsed will be the sum of those used by the instruction (or the 4
			//		clock cycles that do nothing if in stop mode) plus any clock cycles used to service an interrupt.
			//		...
			// If no interrupts were serviced, int_cycles will be zero
			int int_cycles = int_request(&emulator, timing.cycles);
			int total = timing.cycles + int_cycles;
			
			// Depending on the instruction just executed, one component may be ahead of the others in terms of
			//		how many times in has been cycled. All components, by the end of this loop, should be caught
			//		up with the CPU. The component will set the sync_sel itself to let the switch case below know
			//		that a specific component has already been stepped a few more cycles more than others.
			switch (emulator.sync_sel) 
			{
			case 0: 
				// All components must be syncrhonized equally
				timers_step(&emulator, total);
				ppu_step(&emulator, total);
				break;
			case 1: 
				// PPU is ahead, only synchronize postWrite_cycles for PPU
				timers_step(&emulator, total);
				ppu_step(&emulator, timing.postWrite_cycles + int_cycles);
				emulator.sync_sel = 0;
				break;
			case 2: 
				// Timer is ahead, only synchronize postWrite_cycles for Timer
				timers_step(&emulator, timing.postWrite_cycles + int_cycles);
				ppu_step(&emulator, total);
				emulator.sync_sel = 0;
				break;
			}

			// For an explanation on how the fields of the instrTimingInfo were determined and what exactly they
			//		do, see cpu.h

			// Break if bp reached, update cycle counter
			#ifndef NDEBUG
			tCycles += total;

			int breakpointHit = 0;
			for (int i = 0; i < 6; i++)
			{
				if (breakpoints[i].enabled && breakpoints[i].addr == emulator.cpu.PC)
					breakpointHit = 1;
			}
			if (breakpointHit) debug_break(&emulator, &hConsole, window, hdc, breakpoints);
			#endif
		}
	
		// Push frame to screen also handles real time synchronization for the time being
		draw_to_screen(&emulator, window, hdc);

		// Update the key states, this happens once a frame which is not as often as it does not real hardware
		//		but what ever. Unless you are doing some weird stuff you shouldn't really be able to tell.
		update_keyStates(&emulator);

		#ifndef NDEBUG
		// Update debug info and show console if requested (press B)
		if (GetAsyncKeyState(0x42)) debug_break(&emulator, &hConsole, window, hdc, breakpoints);
		#endif
	}

	#ifndef NDEBUG
	debug_deinit();
	#endif

	gb_free(&emulator);
	return 0;
}

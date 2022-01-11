#ifndef _DEBUG_H_
#define _DEBUG_H_

struct breakpoint
{
	unsigned short addr;
	int enabled;
};

void debug_init(HANDLE* hConsole, struct breakpoint* breakpoints);
void debug_deinit();
void debug_break(struct GB* gb, HANDLE* hConsole, HWND window, HDC hdc, struct breakpoint* breakpoints);

#endif // _DEBUG_H_
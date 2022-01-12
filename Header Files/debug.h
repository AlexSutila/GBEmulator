#ifndef _DEBUG_H_
#define _DEBUG_H_

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

struct breakpoint
{
	unsigned short addr;
	int enabled;
};

// I really do not want to pass this in as a parameter to functions as I don't even want the
//		debugger to make it to any release builds, so for the sake of simplicity I'm using extern
struct callStack
{
	struct callStack *next, *prev;
	int entryNumber; // To simplify console refreshing
	uint16_t addr;
} 
extern *callstack;

// Break for debugging
void debug_break(struct GB* gb, HANDLE* hConsole, HWND window, HDC hdc, struct breakpoint* breakpoints);

// Init/deinit
void debug_init(HANDLE* hConsole, struct breakpoint* breakpoints);
void debug_deinit();

// Push/Pop call frames (to be called within instruction functions)
void callStackPush(uint16_t addr);
void callStackPop();

#endif // _DEBUG_H_
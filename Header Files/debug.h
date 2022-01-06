#ifndef _DEBUG_H_
#define _DEBUG_H_

void debug_init(HANDLE* hConsole);
void debug_deinit();
void debug_break(struct GB* gb, HANDLE* hConsole, HWND window, HDC hdc);

#endif // _DEBUG_H_
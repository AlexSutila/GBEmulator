#include "joypad.h"

#include <Windows.h>
#include "io_rw.h"
#include "mem.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

// Masks for active low selects
#define JOYP_BOTH_BUTTONS_MASK      0b00000000
#define JOYP_ACTION_BUTTONS_MASK	0b00010000
#define JOYP_DIRECTION_BUTTONS_MASK 0b00100000
#define JOYP_NONE_BUTTONS_MASK      0b00110000

// Joypad modes - really just all combinations between the
//		states on both selects
#define JOYP_MODE_ACTION	0
#define JOYP_MODE_DIRECTION 1
#define JOYP_MODE_BOTH      2
#define JOYP_MODE_NONE      3

// Win32 doesn't have defines for these so...
#define KEY_W 0x57
#define KEY_S 0x53
#define KEY_A 0x41
#define KEY_D 0x44

void init_JOYP(struct joypad* joyp)
{
	joyp->keyStates[keyState_up] = 1 << 2;
	joyp->keyStates[keyState_down] = 1 << 3;
	joyp->keyStates[keyState_left] = 1 << 1;
	joyp->keyStates[keyState_right] = 1;

	joyp->keyStates[keyState_A] = 1;
	joyp->keyStates[keyState_B] = 1 << 1;
	joyp->keyStates[keyState_select] = 1 << 2;
	joyp->keyStates[keyState_start] = 1 << 3;
}

// Updates the keystates
void update_JOYP(struct GB* gb)
{
	if (GetAsyncKeyState(KEY_W)) gb->joyp.keyStates[keyState_up] = 0;
	else gb->joyp.keyStates[keyState_up] = 1 << 2;
	
	if (GetAsyncKeyState(KEY_S)) gb->joyp.keyStates[keyState_down] = 0;
	else gb->joyp.keyStates[keyState_down] = 1 << 3;
	
	if (GetAsyncKeyState(KEY_A)) gb->joyp.keyStates[keyState_left] = 0;
	else gb->joyp.keyStates[keyState_left] = 1 << 1;
	
	if (GetAsyncKeyState(KEY_D)) gb->joyp.keyStates[keyState_right] = 0;
	else gb->joyp.keyStates[keyState_right] = 1;
	
	if (GetAsyncKeyState(VK_ESCAPE)) gb->joyp.keyStates[keyState_start] = 0;
	else gb->joyp.keyStates[keyState_start] = 1 << 3;
	
	if (GetAsyncKeyState(VK_BACK)) gb->joyp.keyStates[keyState_select] = 0;
	else gb->joyp.keyStates[keyState_select] = 1 << 2;
	
	if (GetAsyncKeyState(VK_LEFT)) gb->joyp.keyStates[keyState_B] = 0;
	else gb->joyp.keyStates[keyState_B] = 1 << 1;
	
	if (GetAsyncKeyState(VK_RIGHT)) gb->joyp.keyStates[keyState_A] = 0;
	else gb->joyp.keyStates[keyState_A] = 1;
}

uint8_t JOYP_RB(struct GB* gb, uint8_t cycles)
{
	// Update register - do this here rather than on read because it won't change
	gb->memory[0xFF00] = 0xFF;
	switch (gb->joyp.mode)
	{
	case JOYP_MODE_ACTION:
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_A];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_B];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_select];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_start];
		// Invert 
		gb->memory[0xFF00] = ~gb->memory[0xFF00];
		break;
	case JOYP_MODE_DIRECTION:
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_right];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_left];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_up];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_down];
		// Invert 
		gb->memory[0xFF00] = ~gb->memory[0xFF00];
		break;
	case JOYP_MODE_BOTH:
		// Update action
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_A];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_B];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_select];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_start];
		// Update direction
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_right];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_left];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_up];
		gb->memory[0xFF00] &= ~gb->joyp.keyStates[keyState_down];
		// Invert 
		gb->memory[0xFF00] = ~gb->memory[0xFF00];
		break;
		// In the case where neither direction or action is selected,
		//		no bits should be pulled low, meaning ($FF00) = 0xFF
	}
	return gb->memory[0xFF00];
}

void JOYP_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	// All bits except for those that are masked are read only
	//		this updates the enables for both sets of button bits
	switch (val & 0x30) 
	{
	case JOYP_DIRECTION_BUTTONS_MASK:
		gb->joyp.mode = JOYP_MODE_DIRECTION;
		break;
	case JOYP_ACTION_BUTTONS_MASK:
		gb->joyp.mode = JOYP_MODE_ACTION;
		break;
	case JOYP_BOTH_BUTTONS_MASK:
		gb->joyp.mode = JOYP_MODE_BOTH;
		break;
	case JOYP_NONE_BUTTONS_MASK:
		gb->joyp.mode = JOYP_MODE_NONE;
		break;
	}
}

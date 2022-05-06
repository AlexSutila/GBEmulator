#ifndef _JOYPAD_H_
#define _JOYPAD_H_

#include "types.h"

/*
	Bit 7 - Not used
	Bit 6 - Not used
	Bit 5 - P15 Select Action buttons    (0=Select)
	Bit 4 - P14 Select Direction buttons (0=Select)
	Bit 3 - P13 Input: Down  or Start    (0=Pressed) (Read Only)
	Bit 2 - P12 Input: Up    or Select   (0=Pressed) (Read Only)
	Bit 1 - P11 Input: Left  or B        (0=Pressed) (Read Only)
	Bit 0 - P10 Input: Right or A        (0=Pressed) (Read Only)
*/

// KeyState indices
enum keyStateIndices 
{ 
	keyState_up, 
	keyState_down, 
	keyState_left, 
	keyState_right, 
	keyState_A, 
	keyState_B, 
	keyState_start, 
	keyState_select 
};

struct joypad 
{
	// 8 buttons, indexed by keyStateIndices
	uint8_t keyStates[8];
	int mode;
};

// Lmao, self explanitory
void init_JOYP(struct joypad* joyp);
void update_JOYP(struct GB* gb);
void update_keyStates(struct GB* gb);

#endif // _JOYPAD_H_
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

struct joypad 
{
	// each entry in keyStates represents one button
	//		...
	// reg_joyp represents the memory mapped joyp register
	uint8_t keyStates[8], reg_joyp;
	// An interrupt is triggered when a falling edge of the 
	//		output of an AND gate of bits 0 - 4 is detected
	uint8_t fallingEdgeDetector;
	// mode indicates which selects are currently selected 
	//		(Action, Direction, None or Both)
	int mode;
};

// Just sets all keys to unpressed
void init_JOYP(struct joypad* joyp);

// Refreshes the contents of reg_joyp given the conents of
//		keyStates and mode
void update_JOYP(struct GB* gb);

// Updates keyStates based on keys pressed
void update_keyStates(struct GB* gb);

#endif // _JOYPAD_H_
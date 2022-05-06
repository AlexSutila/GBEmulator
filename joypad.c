#include "joypad.h"

#include <Windows.h>
#include "hwio.h"
#include "mem.h"

// The IF bit for the joyp interrupt
#define INTBIT_JOYPAD 4

// Masks for active low selects
#define JOYP_BOTH_BUTTONS_MASK      0b00000000
#define JOYP_ACTION_BUTTONS_MASK	0b00010000
#define JOYP_DIRECTION_BUTTONS_MASK 0b00100000
#define JOYP_NONE_BUTTONS_MASK      0b00110000

// Win32 doesn't have defines for these so...
#define KEY_W 0x57
#define KEY_S 0x53
#define KEY_A 0x41
#define KEY_D 0x44

// Joypad modes - really just all combinations between the
//		states of both selects 
enum joypadModes
{
	joypadModeAction    = 0,
	joypadModeDirection = 1,
	joypadModeBoth      = 2,
	joypadModeNone      = 3
};

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

void init_JOYP(struct joypad* joyp)
{
	// Set to one as the output of the AND of the first
	//		four bits should be one 
	joyp->fallingEdgeDetector = 1;
	// Initilaize keystates all to the unpressed state
	joyp->keyStates[keyState_up] = 1 << 2;
	joyp->keyStates[keyState_down] = 1 << 3;
	joyp->keyStates[keyState_left] = 1 << 1;
	joyp->keyStates[keyState_right] = 1;
	joyp->keyStates[keyState_A] = 1;
	joyp->keyStates[keyState_B] = 1 << 1;
	joyp->keyStates[keyState_select] = 1 << 2;
	joyp->keyStates[keyState_start] = 1 << 3;
	// Initialize JOYP register to boot value - double check
	joyp->reg_joyp = 0xCF;
}

// Updates the JOYP register itself
void update_JOYP(struct GB* gb)
{
	// Used to observe the new and old state of the falling edge
	//		detector to determine if there was a falling edge for
	//		interrupts
	uint8_t oldFallingEdgeDetector = gb->joyp.fallingEdgeDetector;
	// Select mask will be have the two corresponding select bits
	//		set based on the mode, this will be used for updating
	//		those bits
	//		...
	// Temp is a temporary all ones value which will have zeros
	//		poked into it based on the button states. Inverting
	//		this after poking those zeros gives the new lower 4
	//		bits
	uint8_t selectMask = 0x00, temp = 0xFF;

	gb->joyp.reg_joyp &= ~0x3F;
	switch (gb->joyp.mode)
	{
	case joypadModeAction:
		gb->joyp.reg_joyp |= JOYP_ACTION_BUTTONS_MASK;
		// Poke zeros into the temporary variable based on
		//		the action button states
		temp &= ~gb->joyp.keyStates[keyState_A]; 
		temp &= ~gb->joyp.keyStates[keyState_B]; 
		temp &= ~gb->joyp.keyStates[keyState_select];
		temp &= ~gb->joyp.keyStates[keyState_start];
		temp =  ~temp;
		break;
	case joypadModeDirection:
		gb->joyp.reg_joyp |= JOYP_DIRECTION_BUTTONS_MASK;
		// Poke zeros into the temporary variable based on
		//		the direction button states
		temp &= ~gb->joyp.keyStates[keyState_right];
		temp &= ~gb->joyp.keyStates[keyState_left];
		temp &= ~gb->joyp.keyStates[keyState_up];
		temp &= ~gb->joyp.keyStates[keyState_down];
		temp =  ~temp;
		break;
	case joypadModeBoth:
		gb->joyp.reg_joyp |= JOYP_BOTH_BUTTONS_MASK;
		// Poke zeros into the temporary variable based on
		//		the action button states
		temp &= ~gb->joyp.keyStates[keyState_A];
		temp &= ~gb->joyp.keyStates[keyState_B];
		temp &= ~gb->joyp.keyStates[keyState_select];
		temp &= ~gb->joyp.keyStates[keyState_start];
		// Poke zeros into the temporary variable based on
		//		the direction button states
		temp &= ~gb->joyp.keyStates[keyState_right];
		temp &= ~gb->joyp.keyStates[keyState_left];
		temp &= ~gb->joyp.keyStates[keyState_up];
		temp &= ~gb->joyp.keyStates[keyState_down];
		temp =  ~temp;
		break;
	default:
		// In the case where neither direction or action is selected,
		//		no bits should be pulled low
		gb->joyp.reg_joyp |= JOYP_NONE_BUTTONS_MASK;
		break;
	}
	// Store updated four bits to register
	gb->joyp.reg_joyp |= temp & 0xF;

	// Update the falling edge detector and trigger an interrupt if
	//		the AND of the lower 4 bits creates a falling edge
	gb->joyp.fallingEdgeDetector =
		(gb->joyp.reg_joyp & 1 << 0) &&
		(gb->joyp.reg_joyp & 1 << 1) &&
		(gb->joyp.reg_joyp & 1 << 2) &&
		(gb->joyp.reg_joyp & 1 << 3);
	// 1 to 0, check if old is greater than current
	if (oldFallingEdgeDetector > gb->joyp.fallingEdgeDetector)
		setIFBit(gb, 4);
}

// Updates the keystates
void update_keyStates(struct GB* gb)
{
	// Sets each key state to zero or a one in the corresponding location
	//		of the JOYP bit which represents that button when it's 
	//		respective select has it selected

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

	// Refresh contents of register given the new key states
	update_JOYP(gb);
}

uint8_t JOYP_RB(struct GB* gb, uint8_t cycles)
{
	// This does not necessarily need to be accurate as this emulator
	//		only samples button input once a frame 
	//		...
	// Bits 6 and 7 are always one 
	return gb->joyp.reg_joyp | 0xC0;
}

void JOYP_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	// All bits except for those that are masked are read only
	//		this updates the enables for both sets of button bits
	switch (val & 0x30) 
	{
	case JOYP_DIRECTION_BUTTONS_MASK:
		gb->joyp.mode = joypadModeDirection;
		break;
	case JOYP_ACTION_BUTTONS_MASK:
		gb->joyp.mode = joypadModeAction;
		break;
	case JOYP_BOTH_BUTTONS_MASK:
		gb->joyp.mode = joypadModeBoth;
		break;
	case JOYP_NONE_BUTTONS_MASK:
		gb->joyp.mode = joypadModeNone;
		break;
	}

	// Refresh contents of register given the selects' states
	update_JOYP(gb);
}

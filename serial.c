#include "hwio.h"
#include "mem.h"

/* I personally can not be bothered to actually implement serial 
*  communication for this emulator. That would mean I would need
*  to find some way to actually transfer data to another emulator.
* 
*  I simply do not care for this feature, at least as of now.
*  
*  This function only exists because if it didn't, the gameboy
*  would never be notified that a transfer it might request is
*  complete, therefor entering a forever loop in some cases. 
* 
*  I discovered this issue in tetris, my solution was to just 
*  notify the system that a transfer is complete immediately after
*  it requests a transfer. Consequentially, since it is connected
*  to nothing, it should only read all ones into its data transfer
*  register - important */

void SC_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	if (val == 0x81) // Using internal clock and transfer in progress
	{
		// Note, only bits 7 and 0 are actually relevant

		gb->memory[0xFF02] = 0x01; // Clear transfer in progress bit
		gb->memory[0xFF01] = 0xFF; // No connection, read all ones

		// Request serial interrupt immediately because I am lazy
		gb->memory[0xFF0F] |= 0x08; 
	}
}

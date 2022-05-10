#include "timers.h"
#include "mem.h"

// A lookup table to act as the multiplexer that determines which 
//		bit is used to dictate the frequency of tima increments
//		...
// The timer.freqSelect will be the input provided to the lookup table
static const uint8_t freqMuxBits[4] = { 9,3,5,7 };

// Returns the number of falling edges that have occured at a specific
//		bit of a counter given its values before and after the increment
inline int fallingEdges(uint16_t old, uint16_t cur, uint8_t bit)
{
	// The best way to see how this works is just to work it out on paper.
	//		It took me a while to come up with and I still don't really 
	//		know how to explain it.
	if (cur > old)
		// If the timer has not overflowed ...
		return ((cur >> (bit + 1)) - (old >> (bit + 1)));
	else
		// Note that this only works with a 16 bit counter becaues of this line
		//		the 0x8000 would need to be changed
		return ((0x8000 >> bit) + (cur >> (bit + 1)) - (old >> (bit + 1)));
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

// Initialize registers and counter value
void init_timers(struct timers* timer) 
{
	timer->reg_tac  = 0xF8; // Upper unused bits return one
	timer->reg_tima = 0x00;
	timer->reg_tma  = 0x00;
	// Note, due to union structure this initializes DIV 
	//		register to zero as well
	timer->counterValue = 0x0000;
}

void timers_step(struct GB* gb, int cycles)
{
	// Increments the internal 16 bit counter once per cycle, consequentially
	//		also updates reg_div because of the union. 
	uint16_t oldCounterValue = gb->timer.counterValue;
	gb->timer.counterValue += cycles;

	// Unless the timer is enabled, that is all that needs to happen here
	if (gb->timer.enable)
	{
		uint8_t oldTimaValue = gb->timer.reg_tima;
		// Increment the timer by the number of falling edges that occur at the
		//		bit selected by the frequency select.
		gb->timer.reg_tima += fallingEdges(oldCounterValue, gb->timer.counterValue,
			freqMuxBits[gb->timer.freqSelect] /* Frequency dictaded by muxbit */);
		// Detect overflow in the tima register, if it overflows increment by tma
		//		and set the corresponding interrupt flag bit
		if (oldTimaValue > gb->timer.reg_tima)
		{
			setIFBit(gb, 2); // Request interrupt
			gb->timer.reg_tima += gb->timer.reg_tma;
		}
	}
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

// Div counter
uint8_t DIV_RB(struct GB* gb, uint8_t cycles)
{
	// Increment a duplicate timer to determine value of the 
	//		div register 'cycles' cycles into the future
	uint16_t futureCounter = gb->timer.counterValue + cycles;
	// The div is the upper 8 bits of this counter
	return futureCounter >> 8;
}

// Tima counter
uint8_t TIMA_RB(struct GB* gb, uint8_t cycles)
{
	// Increment a duplicate internal timer 
	uint16_t futureCounter = gb->timer.counterValue + cycles;
	// Use new timer duplicate value to calculate how many
	//		falling edges will occur at the freq mux bit
	//		if it is enabled.
	uint8_t numFallingEdges = gb->timer.enable ? fallingEdges(gb->timer.counterValue, 
		futureCounter, freqMuxBits[gb->timer.freqSelect]) : 0;
	// Future value is tima+fallingEdges, if timer is disabled numFalling 
	//		edges will be equal to zero, still need to check for overflow
	uint8_t newTimaValue = gb->timer.reg_tima + numFallingEdges;
	// If it overflows increment the final return value by the mod
	if (gb->timer.reg_tima > newTimaValue)
	{	// For the first four clock cycles after an overflow, the tima
		//		reads zero. Diff calculates the difference between the
		//		counter at the read cycle, and the counter at the overflow
		//		cycle. The difference between these two values can be used
		//		to determine how far apart these two cycles occur
		//		...
		// If they are within four clock cycles, then simply return zero
		int diff = futureCounter - ((0xFFFF<<freqMuxBits[gb->timer.freqSelect]) & futureCounter);
		return diff < 4 ? 0 : newTimaValue + gb->timer.reg_tma;
	}
	else return newTimaValue;
}

uint8_t TMA_RB(struct GB* gb, uint8_t cycles)
{
	// This doesn't particularly need to be accessed on the
	//		correct clock cycle so I'm not going to bother
	return gb->timer.reg_tma;
}

uint8_t TAC_RB(struct GB* gb, uint8_t cycles)
{
	// This doesn't need to be accessed on the correct clock
	//		clock cycle either as so I'm not going to bother
	return gb->timer.reg_tac | 0xF8; // Upper bits always 1
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

// Div counter
void DIV_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	// Synchronize the component to catch it up before the write and
	//		then update the internal counter. All writes clear it.
	timers_step(gb, cycles);
	uint16_t oldCounterValue = gb->timer.counterValue;
	gb->timer.counterValue = 0x00;
	// A write to div resetting the internal counter also brings up 
	//		potential to trigger the falling edge detector as one of
	//		this bits drops from one to zero. This will increment 
	//		the TIMA if it is enabled.
	//		...
	// This is where that hardware qwirk is handled
	if (gb->timer.enable && oldCounterValue & (1 << freqMuxBits[gb->timer.freqSelect]))
	{
		// Falling edges do not need to be computed as there is only
		//		potential for a single falling edge. It just checks
		//		it the mux bit is one and if so then falling edge.
		uint8_t oldTimaValue = gb->timer.reg_tima;
		gb->timer.reg_tima++;
		// Detect overflow in the tima register, if it overflows increment by tma
		//		and set the corresponding interrupt flag bit
		if (oldTimaValue > gb->timer.reg_tima)
		{
			setIFBit(gb, 2); // Request interrupt
			gb->timer.reg_tima += gb->timer.reg_tma;
		}
	}
	gb->sync_sel = 2;
}

// Timer counter
void TIMA_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	// Suncrhonize the component to catch it up before the write and
	//		then update the value of the tima register
	timers_step(gb, cycles);
	gb->timer.reg_tima = val;
	gb->sync_sel = 2;
}

// Timer Modulo
void TMA_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	// Innacurate timing could lead to a bad sample during a tima
	//		overflow so write this register with accurate timing
	timers_step(gb, cycles); 
	gb->timer.reg_tma = val;
	gb->sync_sel = 2;
}

// Timer control
void TAC_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	// Accurate read timing isn't important here but write timing is
	timers_step(gb, cycles); 
	// The falling edge detector can also be triggered by changing the
	//		enable bit or frequency bits if the timer is already enabled.
	//		...
	// This annoying behavior is taken into account here 
	if (gb->timer.enable)
	{
		uint16_t oldCounterValue = gb->timer.counterValue;
		uint8_t  oldTimaValue = gb->timer.reg_tima;

		/* Both integers below are used and treated as booleans */

		// Detect falling edge due to timer disabling
		int disableFallingEdge = (oldCounterValue & (1 << freqMuxBits[gb->timer.freqSelect]));
		// Detect falling edge due to timer frequency change
		int freqChangeFallingEdge = (oldCounterValue & (1 << freqMuxBits[gb->timer.freqSelect]))
			&& !(oldCounterValue & (1 << freqMuxBits[val & 0x3]));

		// If either of these is detected, increment the tima regiseter
		if (disableFallingEdge || freqChangeFallingEdge)
		{
			gb->timer.reg_tima++;
			// Detect overflow in the tima register, if it overflows increment by tma
			//		and set the corresponding interrupt flag bit
			if (oldTimaValue > gb->timer.reg_tima)
			{
				setIFBit(gb, 2); // Request interrupt
				gb->timer.reg_tima += gb->timer.reg_tma;
			}
		}
	}
	// Update the actual value of the tac, and set sync select
	gb->timer.reg_tac = val & 0x07;
	gb->sync_sel = 2;
}

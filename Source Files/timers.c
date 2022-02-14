#include "timers.h"
#include "mem.h"

// falling_edge - num of falling edges at a certain 
//		bit as 'old' is incremented to 'cur'
#define FALLING_EDGES(old, cur, bit) (cur > old) ? ((cur >> (bit + 1)) - (old >> (bit + 1))) \
            : ((0x8000 >> bit) + (cur >> (bit + 1)) - (old >> (bit + 1)))
#define OVERFLOW(old, cur) (old > cur)

// counter value bits for tima frequency
enum frequency_Bits { freqMux_bit0 = 9, freqMux_bit1 = 3, freqMux_bit2 = 5, freqMux_bit3 = 7 };

// Sync select initialization and counter init
void init_timers(struct timers* timer) {
	timer->counter.value = 0x0000;
}

void timers_step(struct GB* gb, int cycles) 
{	
	uint16_t old_counter = gb->timer.counter.value;
	gb->timer.counter.value += cycles;
	gb->memory[0xFF04] = (gb->timer.counter.value >> 8);

	uint8_t MOD = gb->memory[0xFF06];
	uint8_t TAC = gb->memory[0xFF07];
	
	if (!(TAC & 0x4) /* TAC disabled */) return;
	uint8_t old_tima = gb->memory[0xFF05];
	uint8_t freq_bit = 0x00;

	switch (TAC & 0x3) {
		case 0b00: freq_bit = freqMux_bit0; break;
		case 0b01: freq_bit = freqMux_bit1; break;
		case 0b10: freq_bit = freqMux_bit2; break;
		case 0b11: freq_bit = freqMux_bit3; break;
	}

	// Detect falling edge at bit masked by freq_mask
	int increment = FALLING_EDGES(old_counter, gb->timer.counter.value, freq_bit);
	gb->memory[0xFF05] += increment;

	// Set interrupt bit if overflow
	if (OVERFLOW(old_tima, gb->memory[0xFF05])) {
		gb->memory[0xFF0F] |= 0x04;
		gb->memory[0xFF05] += MOD;
	}
}


// Div counter
uint8_t DIV_RB(struct GB* gb, uint8_t cycles)
{
	struct timers temp;
	temp.counter.value = gb->timer.counter.value + cycles;
	return (temp.counter.value >> 8);
}

// Tima counter
uint8_t TIMA_RB(struct GB* gb, uint8_t cycles)
{
	
	// If tima is disabled, it doesn't need to look ahead
	//		return TIMA value since it will not increment
	uint8_t TAC = gb->memory[0xFF07];
	if (!(TAC & 0x04)) return gb->memory[0xFF05];

	// Otherwise, make fake timer to determine the value to be read
	//		on the cycle it will actually be read on
	struct timers temp;
	temp.counter.value = gb->timer.counter.value + cycles;

	uint8_t MOD = gb->memory[0xFF06];
	uint8_t freq_bit = 0x0000;

	switch (TAC & 0x3) {
		case 0b00: freq_bit = freqMux_bit0; break;
		case 0b01: freq_bit = freqMux_bit1; break;
		case 0b10: freq_bit = freqMux_bit2; break;
		case 0b11: freq_bit = freqMux_bit3; break;
	}

	// Detect falling edge at bit masked by freq_mask
	int increment = FALLING_EDGES(gb->timer.counter.value, temp.counter.value, freq_bit);
	uint8_t new_tima = gb->memory[0xFF05] + increment;

	if (OVERFLOW(gb->memory[0xFF05], new_tima)) 
		return new_tima + MOD;
	
	else return new_tima;
}



// Div counter
void DIV_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	// A write to DIV has will always reset the gameboy's internal counter.
	//		In addition, doing this could also potentially increment TIMA.
	//		Depending on the current frequency of the timer, the bit of
	//		focus which drives the incrementing of the tima register may
	//		be set high before this reset. This results in a falling edge
	//		behavior at that specific bit causing the tma to increment.
	//
	// ... This behavior is taken into account in this function

	timers_step(gb, cycles);
	
	uint8_t MOD = gb->memory[0xFF06];
	uint8_t TAC = gb->memory[0xFF07];
	
	// Check for falling edge on frequency bit if the timer is enabled
	if (TAC & 0x04)
	{
		uint8_t old_tima = gb->memory[0xFF05], freq_bit = 0x00;
	
		switch (TAC & 0x3) {
		case 0b00: freq_bit = freqMux_bit0; break;
		case 0b01: freq_bit = freqMux_bit1; break;
		case 0b10: freq_bit = freqMux_bit2; break;
		case 0b11: freq_bit = freqMux_bit3; break;
		}
	
		// Detect falling edge at bit masked by freq_mask
		if (gb->timer.counter.value & (1 << freq_bit))
			gb->memory[0xFF05]++;
	
		if (OVERFLOW(old_tima, gb->memory[0xFF05])) {
			gb->memory[0xFF0F] |= 0x04; 
			gb->memory[0xFF05] += MOD;
		}
	}
	
	// Internal counter reset
	gb->timer.counter.value = 0x0000;
	gb->memory[0xFF04] = 0x00;
	gb->sync_sel = 2;
}

// Timer counter
void TIMA_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	timers_step(gb, cycles);
	gb->memory[0xFF05] = val;

	gb->sync_sel = 2;
}

// Timer Modulo
void TMA_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	timers_step(gb, cycles);
	gb->memory[0xFF06] = val;

	gb->sync_sel = 2;
}

// Timer control
void TAC_WB(struct GB* gb, uint8_t val, uint8_t cycles)
{
	timers_step(gb, cycles);

	// Only first 3 bits are used
	gb->memory[0xFF07] &= ~0x7;
	gb->memory[0xFF07] |= (val & 0x7);
	
	gb->sync_sel = 2;
}

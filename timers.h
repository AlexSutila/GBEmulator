#ifndef _TIMERS_H_
#define _TIMERS_H_

#include "types.h"

void init_timers(struct timers* timer);
void timers_step(struct GB* gb, int cycles);

struct timers 
{
	// Counter which contains div register
	union 
	{
		uint16_t counterValue;
		struct {
			uint8_t reg_div; // Div register is mapped to 0xFF04
			uint8_t amongUs; // This byte is not memory mapped or used for
			                 //		... anything
		};
	};
	// Timer and modulo registers respectively
	uint8_t reg_tima, reg_tma;
	// Union for timer control register
	union
	{
		uint8_t reg_tac;
		struct
		{
			uint8_t freqSelect : 2;
			uint8_t enable     : 1;
		};
	};
};

#endif // _TIMERS_H_
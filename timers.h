#ifndef _TIMERS_H_
#define _TIMERS_H_

#include "types.h"

void init_timers(struct timers* timer);
void timers_step(struct GB* gb, int cycles);

struct timers 
{
	// Counter which contains div byte
	// union_counter counter;
	union {
		uint16_t value;
		struct {
			uint8_t div; // This byte is mapped to 0xFF04
			uint8_t jnk; // This byte is not memory mapped
		};
	} counter;
};

#endif // _TIMERS_H_
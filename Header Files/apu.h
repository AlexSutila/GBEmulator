#ifndef _APU_H_
#define _APU_H_

#include <Windows.h>

#define CYCLES_PER_SAMPLE 95
#define SAMPLES_PER_QUEUE 739
#define CYCLES_PER_QUEUE (CYCLES_PER_SAMPLE * SAMPLES_PER_QUEUE)

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

void apu_step(struct GB* gb, HWND window, uint8_t cycles);

#endif
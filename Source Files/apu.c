#include "apu.h"
#include "mem.h"

#define OVERFLOW(old, cur) (old > cur)

/*
*  Still thinking this through, entirely subject to change
* 
*  As of right now, my plan is to sample audio at a fixed sample rate
*		and place it in a buffer which win32 will read from and hopefully
*		then send to the audio drivers or whatever (not sure how this works
*		yet, haven't read any documentation) after roughly a frames worth of
*		emulated clock cycles. 
* 
*  GOAL: have a queue be close to a frames worth of audio time for now,
*		I may change this later on, I'm not entirely sure
* 
*  4.19MHz = GameBoy clock speed (definite)
*  44100Hz = My chosen audio sample rate (subject to change)
*  456 clock cycles = clocks per scanline (definite)
*  154 = number of scanlines (definite)
* 
*  4.19MHz / 44100Hz = ~95.011, so roughly 95 cycles per sample
*  456 cycles * 154 scanlines = 70,224 cycles per frame 
*  70,224 (cycles / frame) / 95 (cycles / sample) = 739.2 
*									~739 samples per frame (or audio queue)
* 
*  Not the biggest fan that the last value is not exact, but I cant notice
*		any difference playing games with this setup. To avoid starving the
*		speaker of audio to play, I think I can get away with populating the
*		buffer starting half way between the beginning and end, or maybe 
*		closer to the start, which would be better because less latency?
*
*  I will play around with it when I actually have audio playing 
*/

void apu_step(struct GB* gb, HWND window, uint8_t cycles)
{
	static int cycleCount = 0;
	static int sampleCount = 0;
	static LARGE_INTEGER start = { 0,0 };

	int prevCycles = cycleCount;
	int prevSamples = sampleCount;

	cycleCount = (cycleCount + cycles) % CYCLES_PER_SAMPLE;
	if (OVERFLOW(prevCycles, cycleCount))
		sampleCount = (sampleCount + 1) % SAMPLES_PER_QUEUE;

	if (OVERFLOW(prevSamples, sampleCount))
	{
		// Handle sync to audio
		LARGE_INTEGER end, frequency;
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&end);
		float secsPerFrame = ((float)(end.QuadPart - start.QuadPart) / (float)frequency.QuadPart);
		while (secsPerFrame < 0.01666667f) {
			QueryPerformanceCounter(&end);
			secsPerFrame = ((float)(end.QuadPart - start.QuadPart) / (float)frequency.QuadPart);
		}
		// Hangle message thingy
		MSG msg;
		while (PeekMessage(&msg, window, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		QueryPerformanceCounter(&start);
	}
}

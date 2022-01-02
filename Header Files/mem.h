#ifndef MEM_H_
#define MEM_H_

#include "joypad.h"
#include "timers.h"
#include "cart.h"
#include "cpu.h"
#include "ppu.h"

// Type definitions
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef signed short int16_t;

// Start addresses (s)
#define s_ROM_00 	0x0000
#define s_ROM_NN 	0x4000
#define s_VRAM		0x8000
#define s_ERAM		0xA000
#define s_WRAM		0xC000
#define s_ECHO		0xE000
#define s_OAM		0xFE00
#define s_UNUSABLE	0xFEA0
#define s_IO_REGS	0xFF00
#define s_HRAM		0xFF80
#define s_IE		0xFFFF

// Memory Block Size (n)
#define n_ROM_00 	0x4000
#define n_ROM_NN 	0x4000
#define n_VRAM		0x2000
#define n_ERAM		0x2000
#define n_WRAM		0x2000
#define n_ECHO		0x1E00
#define n_OAM		0x00A0
#define n_UNUSABLE	0x0060
#define n_IO_REGS	0x0080
#define n_HRAM		0x007F
// IE is a register ... n one byte

struct GB 
{
	// Memory pointers
	uint8_t *memory, *bootstrap;
	uint8_t sync_sel;

	// Components
	struct cartridge cart;
	struct sharp_SM83 cpu;
	struct timers timer;
	struct joypad joyp;
	struct PPU ppu;
};

uint8_t RB(struct GB* gb, uint16_t addr, uint8_t cycles);
void WB(struct GB* gb, uint16_t addr, uint8_t val, uint8_t cycles);

void gb_init(struct GB* gb);
void gb_free(struct GB* gb);

#endif

















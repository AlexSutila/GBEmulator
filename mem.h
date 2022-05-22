#ifndef MEM_H_
#define MEM_H_

#include "types.h"
#include "joypad.h"
#include "timers.h"
#include "cart.h"
#include "cpu.h"
#include "ppu.h"

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
	uint8_t *vram, *iram, *oam, *ioregs, *hram, reg_IE, *bootstrap;
	uint8_t sync_sel;

	// Components
	struct cartridge cart;
	struct sharp_SM83 cpu;
	struct timers timer;
	struct joypad joyp;
	struct PPU ppu;
};

// Fairly self explanitory, sets the corresponding interrupt flag bit, to be 
//		used by components other than the CPU.
inline void setIFBit(struct GB* gb, uint8_t bitNum)
{
	// FFOF is the address of the IF register
	gb->ioregs[0x0F] |= 1 << bitNum;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////

// These read/write functions are to be used specifically by the CPU and by the
//		CPU only. The cycles field serves as an indicator as to how many cycles
//		ahead a sample should be read, or how many cycles a given component should
//		be stepped before writing to the given address in the scenario where the
//		cycle of which the memory address is read/written matters. 
//		...
// These look aheads or mid instruction component synchronizations are only done
//		when working in the IO regs region of memory when the address is mapped
//		to a particularly important IO register (see lookup tables within these
//		functions, if null cycles won't be used)
uint8_t RB(struct GB* gb, uint16_t addr, uint8_t cycles);
void WB(struct GB* gb, uint16_t addr, uint8_t val, uint8_t cycles);

// ////////////////////////////////////////////////////////////////////////////////////////////////

// Two different ways of indexing tile data
enum tileAccessAddressingModes
{	// These are based directly off of their corresponding values
	//		in the LCDC register, single bit so either 0 or 1
	tileAddrModeSigned   = 0,
	tileAddrModeUnsigned = 1,
};

// Perform a lookup into a specific tile map to obtain a tile index, note there are only
//		two tilemaps in memory, make sure the correct start addresses are used
inline uint8_t tilemap_access(struct GB* gb, uint16_t map_base, uint16_t map_index)
{	// Obtain the tile index from the tile map, selected by the map_base. Important, map_base
	//		should only ever be 0x9800 or 0x9C00 as there are only two tile maps
	return gb->vram[map_base + map_index - 0x8000];
}
// Access a specific tile, given the addressing mode (see tileAccessAddressingModes) and
//		respective tile index
const struct tileStruct* tiledata_access(struct GB* gb, int addr_mode, uint8_t tile_index);
// Access a specific sprite, given the index into object attribute memory
const struct spriteStruct* objattr_access(struct GB* gb, uint16_t index);

// ////////////////////////////////////////////////////////////////////////////////////////////////

void gb_init(struct GB* gb);
void gb_free(struct GB* gb);

#endif

















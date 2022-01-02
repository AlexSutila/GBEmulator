#ifndef MAP_H_
#define MAP_H_

/*
	Mapper Chip Types:
		ROM_ONLY
		ROM_RAM
		MBC1
		MBC2
		*more, unimplemented*
*/

// Type definitions
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef signed short int16_t;

// Read Bytes
uint8_t NO_MBC_RB(uint16_t addr);
uint8_t MBC1_RB(uint16_t addr);
uint8_t MBC2_RB(uint16_t addr);

// Write Bytes
void NO_MBC_WB(uint16_t addr, uint8_t value);
void MBC1_WB(uint16_t addr, uint8_t value);
void MBC2_WB(uint16_t addr, uint8_t value);

#endif

#ifndef CART_H_
#define CART_H_

#include "types.h"

// Not representative of any specific cartridge signal, simply just used
//		for memory access to the cart to differentiate between reading
//		and writing within the memAccess functions associated with 
//		different memory bank controller types...
#define CART_READ  0
#define CART_WRITE 1

enum rom_sizes 
{
	romSize_32KB 		= 0x00, // two banks (no ROM banking)
	romSize_64KB		= 0x01, // four banks
	romSize_128KB		= 0x02, // eight banks
	romSize_256KB		= 0x03, // sixteen banks
	romSize_512KB 		= 0x04, // thirty two banks
	romSize_1MB			= 0x05, // sixty four banks
	romSize_2MB			= 0x06, // one hundred twenty eight banks
	romSize_4MB			= 0x07, // two hundred fifty six banks
	romSize_8MB			= 0x08, // five hundred twelve banks
	romSize_1_1MB		= 0x52, // seventy two banks (3)
	romSize_1_2MB		= 0x53, // eighty banks (3)
	romSize_1_5MB		= 0x54, // nintey six banks (3)
};

enum ram_sizes 
{
	/*
		(4) - When using a MBC2 chip, 0x00 must be specified as the RAM size, even
			though the MBC2 includes a built in RAM of 512 x 4 bits

		(5) - Listed in various unofficial docs as 2KB. However, a 2KB RAM chip was
			never used in a cartridge. The source for this value is unknown.
	*/
	ramSize_NONE		= 0x00, // No RAM (4)
	// 0x01 (5) Unused - unknown size
	ramSize_1BANKS		= 0x02, // 8KB
	ramSize_4BANKS		= 0x03, // 4Banks of 8KB each
	ramSize_16BANKS		= 0x04, // 16Banks of 8KB each
	ramSize_8BANKS		= 0x05, // 8Banks of 8KB each
};

// These functions will initialize memory (see mem.h), based on the information
// 	read in from the cartridge header
int init_cart(struct cartridge* cart_ptr, char* rom_dir);
void free_cart(struct cartridge* cart_ptr);

// Cartridge structure
struct cartridge
{
	// Cart attributes
	char title[0x10]; // 0x0134 - 0x0143
	uint8_t cartType, romSize, ramSize;

	// Memory pointers
	uint8_t *ROM_full, *RAM_full;               // Full contents of ROM and RAM
	uint8_t *ROM00_ptr, *ROMnn_ptr, *RAM_ptr;   // Pointer to start of selected bank

	// Function pointer for mem access and re-mapping function based on cart-type
	uint8_t(*memAccess)(struct cartridge*, uint16_t /*addr*/, uint8_t /*val*/, uint8_t /*R\W*/);
};

#endif
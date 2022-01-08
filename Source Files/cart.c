#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include "cart.h"
#include "mem.h"

uint8_t memAccess_NOMBC(struct cartridge* cart_ptr, uint16_t addr, uint8_t val, uint8_t rw);
uint8_t memAccess_MBC1(struct cartridge* cart_ptr, uint16_t addr, uint8_t val, uint8_t rw);
uint8_t memAccess_MBC3(struct cartridge* cart_ptr, uint16_t addr, uint8_t val, uint8_t rw);

enum cart_info_addresses {
	addrTitle		= 0x0134,
	addrCgbFlag		= 0x0143,
	addrSgbFlag		= 0x0146,
	addrCartType	= 0x0147,
	addrRomSize		= 0x0148,
	addrRamSize		= 0x0149
};

enum cart_types {
	type_romOnly		= 0x00,
	type_mbc1			= 0x01,
	type_mbc1_ram		= 0x02,
	type_mbc1_ram_batt	= 0x03,
	type_rom_ram		= 0x08,
	type_rom_ram_batt	= 0x09,
	type_mbc3			= 0x11,
	type_mbc3_ram		= 0x12,
	type_mbc3_ram_batt	= 0x13,
	/* More I may or may not implement */
};

int init_cart(struct cartridge* cart_ptr, char* rom_dir)
{
	FILE* rom_file;
	rom_file = fopen(rom_dir, "rb");
	// If valid file
	if (rom_file != NULL) 
	{
		// Read cart type
		fseek(rom_file, addrCartType, SEEK_SET);
		fread(&cart_ptr->cartType, sizeof(uint8_t), 1, rom_file);

		// Read ROM size
		fseek(rom_file, addrRomSize, SEEK_SET);
		fread(&cart_ptr->romSize, sizeof(uint8_t), 1, rom_file);

		// Read RAM size
		fseek(rom_file, addrRamSize, SEEK_SET);
		fread(&cart_ptr->ramSize, sizeof(uint8_t), 1, rom_file);

		// Read cartridge title
		fseek(rom_file, addrTitle, SEEK_SET);
		fread(cart_ptr->title, sizeof(uint8_t), 0x10, rom_file);

		/* Initialize mapping based on cart type */
		switch (cart_ptr->cartType) {
		case type_romOnly:
		case type_rom_ram:
		case type_rom_ram_batt:
			cart_ptr->memAccess = &memAccess_NOMBC;
			break;
		case type_mbc1:
		case type_mbc1_ram:
		case type_mbc1_ram_batt:
			cart_ptr->memAccess = &memAccess_MBC1;
			break;
		case type_mbc3:
		case type_mbc3_ram:
		case type_mbc3_ram_batt:
			cart_ptr->memAccess = &memAccess_MBC3;
			break;
		
		default:
			// -1 return type indicates invalid/unimplemented cart type
			fclose(rom_file);
			return -1;
		}

		// Reads and allocated memory for ROM_NN
		fseek(rom_file, 0, SEEK_SET);
		switch (cart_ptr->romSize) {
		case romSize_32KB:
			cart_ptr->ROM_full = (uint8_t*)calloc(n_ROM_NN * 2, sizeof(uint8_t));
			fread(cart_ptr->ROM_full, sizeof(uint8_t), n_ROM_NN * 2, rom_file);
			break;
		case romSize_64KB:
			cart_ptr->ROM_full = (uint8_t*)calloc(n_ROM_NN * 4, sizeof(uint8_t));
			fread(cart_ptr->ROM_full, sizeof(uint8_t), n_ROM_NN * 4, rom_file);
			break;
		case romSize_128KB:
			cart_ptr->ROM_full = (uint8_t*)calloc(n_ROM_NN * 8, sizeof(uint8_t));
			fread(cart_ptr->ROM_full, sizeof(uint8_t), n_ROM_NN * 8, rom_file);
			break;
		case romSize_256KB:
			cart_ptr->ROM_full = (uint8_t*)calloc(n_ROM_NN * 16, sizeof(uint8_t));
			fread(cart_ptr->ROM_full, sizeof(uint8_t), n_ROM_NN * 16, rom_file);
			break;
		case romSize_512KB:
			cart_ptr->ROM_full = (uint8_t*)calloc(n_ROM_NN * 32, sizeof(uint8_t));
			fread(cart_ptr->ROM_full, sizeof(uint8_t), n_ROM_NN * 32, rom_file);
			break;
		case romSize_1MB:
			cart_ptr->ROM_full = (uint8_t*)calloc(n_ROM_NN * 64, sizeof(uint8_t));
			fread(cart_ptr->ROM_full, sizeof(uint8_t), n_ROM_NN * 64, rom_file);
			break;
		case romSize_2MB:
			cart_ptr->ROM_full = (uint8_t*)calloc(n_ROM_NN * 128, sizeof(uint8_t));
			fread(cart_ptr->ROM_full, sizeof(uint8_t), n_ROM_NN * 128, rom_file);
			break;
		case romSize_4MB:
			cart_ptr->ROM_full = (uint8_t*)calloc(n_ROM_NN * 256, sizeof(uint8_t));
			fread(cart_ptr->ROM_full, sizeof(uint8_t), n_ROM_NN * 256, rom_file);
			break;
		case romSize_8MB:
			cart_ptr->ROM_full = (uint8_t*)calloc(n_ROM_NN * 512, sizeof(uint8_t));
			fread(cart_ptr->ROM_full, sizeof(uint8_t), n_ROM_NN * 512, rom_file);
			break;
		default:
			// -2 return value indicates bad ROM size
			fclose(rom_file);
			return -2;
		}
		cart_ptr->ROM00_ptr = cart_ptr->ROM_full + 0x0000;
		cart_ptr->ROMnn_ptr = cart_ptr->ROM_full + 0x4000;

		/* Initialize RAM based on cart info */
		switch (cart_ptr->ramSize) {
		case ramSize_NONE:
			cart_ptr->RAM_full = NULL;
			break;
		case ramSize_1BANKS:
			cart_ptr->RAM_full = (uint8_t*)calloc(n_ERAM * 1, sizeof(uint8_t));
			break;
		case ramSize_4BANKS:
			cart_ptr->RAM_full = (uint8_t*)calloc(n_ERAM * 4, sizeof(uint8_t));
			break;
		case ramSize_16BANKS:
			cart_ptr->RAM_full = (uint8_t*)calloc(n_ERAM * 16, sizeof(uint8_t));
			break;
		case ramSize_8BANKS:
			cart_ptr->RAM_full = (uint8_t*)calloc(n_ERAM * 8, sizeof(uint8_t));
			break;
		default:
			// -3 return value indicates bad RAM size
			fclose(rom_file);
			return -3;
		}
		cart_ptr->RAM_ptr = cart_ptr->RAM_full;

		// Read from save file, if it exists and if the cart is battery buffered
		char nt_title[0x1A] = "savs\\";
		memcpy(nt_title + 5, cart_ptr->title, 0x10);
		int i;
		for (i = 5; i < 22; i++)
			if (nt_title[i] == 0x00)
			{
				nt_title[i] = '\0';
				break;
			}
		nt_title[i] = '.';   nt_title[i + 1] = 's';
		nt_title[i + 2] = 'a'; nt_title[i + 3] = 'v';
		nt_title[i + 4] = '\0';

		if (cart_ptr->cartType == type_mbc1_ram_batt ||
			cart_ptr->cartType == type_mbc3_ram_batt ||
			cart_ptr->cartType == type_rom_ram_batt) {

			FILE* sav_file = fopen(nt_title, "rb");
			if (sav_file)
			{
				switch (cart_ptr->ramSize)
				{
				case ramSize_1BANKS:
					fread(cart_ptr->RAM_full, sizeof(uint8_t), 1 * n_ERAM, sav_file);
					break;
				case ramSize_4BANKS:
					fread(cart_ptr->RAM_full, sizeof(uint8_t), 4 * n_ERAM, sav_file);
					break;
				case ramSize_16BANKS:
					fread(cart_ptr->RAM_full, sizeof(uint8_t), 16 * n_ERAM, sav_file);
					break;
				case ramSize_8BANKS:
					fread(cart_ptr->RAM_full, sizeof(uint8_t), 8 * n_ERAM, sav_file);
					break;
				}
				fclose(sav_file);
			}
		}

		fclose(rom_file);
		return 1; // Success
	}

	// Zero return value specifies invalid path
	return 0;
}

void free_cart(struct cartridge* cart_ptr)
{
	// Open .sav file to dump ram contents, if any, to mimic battery buffering
	//		assuming the cartridge is battery buffered.
	char nt_title[0x1A] = "savs\\";
	memcpy(nt_title + 5, cart_ptr->title, 0x10);
	// Format string
	int i;
	for (i = 5; i < 22; i++)
		if (nt_title[i] == 0x00) 
		{
			nt_title[i] = '\0';
			break;
		}
	nt_title[i] = '.';   nt_title[i+1] = 's';
	nt_title[i+2] = 'a'; nt_title[i+3] = 'v';
	nt_title[i+4] = '\0';

	// Dump ram contents
	FILE* sav_file = fopen(nt_title, "wb");
	if (cart_ptr->cartType == type_mbc1_ram_batt ||
		cart_ptr->cartType == type_mbc3_ram_batt ||
		cart_ptr->cartType == type_rom_ram_batt  )
		switch (cart_ptr->ramSize) 
		{
		case ramSize_1BANKS:
			fwrite(cart_ptr->RAM_full, sizeof(uint8_t), 1 * n_ERAM, sav_file);
			break;
		case ramSize_4BANKS:
			fwrite(cart_ptr->RAM_full, sizeof(uint8_t), 4 * n_ERAM, sav_file);
			break;
		case ramSize_16BANKS:
			fwrite(cart_ptr->RAM_full, sizeof(uint8_t), 16 * n_ERAM, sav_file);
			break;
		case ramSize_8BANKS:
			fwrite(cart_ptr->RAM_full, sizeof(uint8_t), 8 * n_ERAM, sav_file);
			break;
		}
	fclose(sav_file);

	// Other pointers within cartridge point to the memory pointed to
	//		by the full rom/ram pointers, so they do not need a free call
	free(cart_ptr->ROM_full);
	free(cart_ptr->RAM_full);
}

//////////////////////////////////////////////////////////////////////////////////////////////

uint8_t memAccess_NOMBC(struct cartridge* cart_ptr, uint16_t addr, uint8_t val, uint8_t rw)
{
	if (rw == CART_READ)
	{
		// ROM
		if (addr >= 0x0000 && addr < 0x8000)
			return cart_ptr->ROM00_ptr[addr];
		// RAM, if existent
		else if (addr >= 0xA000 && addr < 0xC000) 
		{
			if (cart_ptr->ramSize != ramSize_NONE)
				return cart_ptr->RAM_ptr[addr - s_ERAM];
			else return 0xFF;
		}
	}
	else /* rw == CART_WRITE */
	{
		if (addr >= 0xA000 && addr < 0xB000 && cart_ptr->ramSize != ramSize_NONE)
		{
			cart_ptr->RAM_ptr[addr - s_ERAM] = val;
			return 0xFF; // Irrelevant value
		}
	}

	// Default return value
	return 0xFF;
}

uint8_t memAccess_MBC1(struct cartridge* cart_ptr, uint16_t addr, uint8_t val, uint8_t rw)
{
	// Cartridge properties
	static uint8_t ramEnable = 0x00;
	static uint8_t romBank_lo = 0x01; 
	static uint8_t romBank_hi = 0x00;
	static uint8_t bankMode = 0x00;

	if (rw == CART_READ)
	{
		// ROM - lower selected bank
		if (addr >= 0x0000 && addr < 0x4000)
			return cart_ptr->ROM00_ptr[addr];
		// ROM - upper selected bank
		else if (addr >= 0x4000 && addr < 0x8000)
			return cart_ptr->ROMnn_ptr[addr - 0x4000];
		// RAM - only selected bank assuming it has ram
		else if (addr >= 0xA000 && addr < 0xC000) 
		{
			// specifically 0x0A enables ram for some reason
			if (ramEnable == 0x0A && cart_ptr->ramSize != ramSize_NONE)
				return cart_ptr->RAM_ptr[addr - 0xA000];
			else return 0xFF;
		}
	}
	else /* rw == CART_WRITE */
	{
		// RAM enable register
		if (addr >= 0x0000 && addr < 0x2000)
		{
			ramEnable = val & 0xF;
			return 0xFF; // Value irrelevant, reconfiguration of everything
			             //		farther down can be skipped
		}
		// ROM bank number
		else if (addr >= 0x2000 && addr < 0x4000)
		{
			if ((val & 0x1F) == 0x00) val++;
			switch (cart_ptr->romSize)
			{
				case romSize_32KB:  romBank_lo = val & 0x01; break;
				case romSize_64KB:  romBank_lo = val & 0x03; break;
				case romSize_128KB: romBank_lo = val & 0x07; break;
				case romSize_256KB: romBank_lo = val & 0x0F; break;
				default:            romBank_lo = val & 0x1F; break;
			}
		}
		// RAM bank number
		else if (addr >= 0x4000 && addr < 0x6000)
			romBank_hi = val & 0x3;
		// Banking mode select
		else if (addr >= 0x6000 && addr < 0x8000)
			bankMode = val & 0x1;
		// Write to ram if existant and enabled
		else if (addr >= 0xA000 && addr < 0xC000)
			if ((ramEnable == 0x0A) && (cart_ptr->ramSize != ramSize_NONE))
				cart_ptr->RAM_ptr[addr - 0xA000] = val;

		// Re-map 0x4000 - 0x7FFFF, this happens in either mode. Pandocs is wrong
		uint8_t bank_number = romBank_lo;
		if (cart_ptr->romSize == romSize_1MB || cart_ptr->romSize == romSize_2MB)
			bank_number |= (romBank_hi << 5); 
		cart_ptr->ROMnn_ptr = cart_ptr->ROM_full + (bank_number * 0x4000);

		// Do stuff based on banking mode
		if (bankMode) 
		{
			cart_ptr->ROM00_ptr = cart_ptr->ROM_full + ((romBank_hi << 5) * 0x4000);
			switch (cart_ptr->ramSize)
			{
				case ramSize_NONE: case ramSize_1BANKS:
					cart_ptr->RAM_ptr = cart_ptr->RAM_full;
					break;
				case ramSize_4BANKS:
					cart_ptr->RAM_ptr = cart_ptr->RAM_full + (romBank_hi * 0x2000);
					break;
			}
		}
		else
		{
			cart_ptr->ROM00_ptr = cart_ptr->ROM_full;
			cart_ptr->RAM_ptr = cart_ptr->RAM_full;
		}
	}
	// Default return value
	return 0xFF;
}

// Without timer, I might implement MBC3 with a timer later on because pokemon :D
uint8_t memAccess_MBC3(struct cartridge* cart_ptr, uint16_t addr, uint8_t val, uint8_t rw)
{
	// Cart properties
	static uint8_t ramEnable = 0x00;

	if (rw == CART_READ)
	{
		// Low bank, always bank zero
		if (addr >= 0x0000 && addr < 0x4000)
			return cart_ptr->ROM_full[addr];
		// Upper bank, now supports banks 20, 40, 60 etc
		else if (addr >= 0x4000 && addr < 0x8000)
			return cart_ptr->ROMnn_ptr[addr - 0x4000];
		// RAM Bank, if any (00h - 03h only)
		else if (addr >= 0xA000 && addr < 0xC000)
		{
			if ((ramEnable == 0x0A) && (cart_ptr->ramSize != ramSize_NONE))
				return cart_ptr->RAM_ptr[addr - 0xA000];
			else return 0xFF;
		}
	}
	else /* rw == CART_WRITE */
	{
		// RAM enable
		if (addr >= 0x0000 && addr < 0x2000)
			ramEnable = val & 0xF;
		// ROM bank number (7 bits)
		else if (addr >= 0x2000 && addr < 0x4000)
		{
			val &= 0x7F; // Mapping this to bank zero not supported
			if (!val) val++;
			cart_ptr->ROMnn_ptr = cart_ptr->ROM_full + (n_ROM_NN * val);
		}
		// RAM bank number (2 bits)
		else if (addr >= 0x4000 && addr < 0x6000)
			cart_ptr->RAM_ptr = cart_ptr->RAM_full + (n_ERAM * (val & 0x3));
		// Write to RAM banks, if enabled and existant
		else if (addr >= 0xA000 && addr < 0xC000)
		{
			if ((ramEnable == 0x0A) && (cart_ptr->ramSize != ramSize_NONE))
				cart_ptr->RAM_ptr[addr - 0xA000] = val;
		}
	}
	// Default return value
	return 0xFF;
}

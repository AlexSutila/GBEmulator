#include "map.h"
#include "mem.h"
#include "cart.h"

// Type definitions
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef signed short int16_t;

// Read Bytes
uint8_t NO_MBC_RB(uint16_t addr) {
	switch (addr & 0xF000) {
		case 0x0000: case 0x1000: case 0x2000: case 0x3000:
		case 0x4000: case 0x5000: case 0x6000: case 0x7000:
			return m_ROM_00[addr];
		case 0xA000: case 0xB000:
			if (cart.ramSize != ramSize_NONE) {
				return m_ERAM[addr - s_ERAM];
			} else return 0x00;
	}
}
uint8_t MBC1_RB(uint16_t addr) {
	switch (addr & 0xF000) {
		case 0x0000: case 0x1000: case 0x2000: case 0x3000:
			return m_ROM_00[addr];
		case 0x4000: case 0x5000: case 0x6000: case 0x7000:
			return m_ROM_NN[addr - s_ROM_NN];
		case 0xA000: case 0xB000:
			if (((cart.cartState & 0xF) == 0xA) && (cart.ramSize != ramSize_NONE)) {
				return m_ERAM[addr - s_ERAM];
			} else return 0x0000;
	}
}
uint8_t MBC2_RB(uint16_t addr) {
        return 0x00;
}

// Write Bytes
void NO_MBC_WB(uint16_t addr, uint8_t value) {
	switch(addr & 0xF000) {
		case 0xA000: case 0xB000:
			if (cart.ramSize != ramSize_NONE) {
				m_ERAM[addr - s_ERAM] = value;
			}
			break;
	}
}
void MBC1_WB(uint16_t addr, uint8_t value) {
	switch(addr & 0xF000) {
		case 0x0000: case 0x1000:
			cart.cartState &= ~0x000F;
			cart.cartState |= (value & 0xF);
			break;
		case 0x2000: case 0x3000:
			cart.cartState &= ~0x1F0;
			if ((value & 0x1F) == 0x0000) value = value + 1;
			switch(cart.romSize) {
				case romSize_32KB: cart.cartState |= (value & 0x1) << 4; break;
				case romSize_64KB: cart.cartState |= (value & 0x3) << 4; break;
				case romSize_128KB: cart.cartState |= (value & 0x7) << 4; break;
				case romSize_256KB: cart.cartState |= (value & 0xF) << 4; break;
				default: cart.cartState |= (value & 0x1F) << 4; break;
			}
			break;
		case 0x4000: case 0x5000:
			cart.cartState &= ~0x0600;
			cart.cartState |= (value & 0x3) << 9;
			break;
		case 0x6000: case 0x7000:
			cart.cartState &= ~0x0800;
			cart.cartState |= (value & 0x1) << 11;
			break;
		case 0xA000: case 0xB000:
			if (((cart.cartState & 0xF) == 0xA) && (cart.ramSize != ramSize_NONE)) {
				m_ERAM[addr - s_ERAM] = value;
			}
			break;
	}

	/* apply changes */

	// Re-map 0x4000-0x7FFF, this happens in either mode. Pandocs is wrong
	uint8_t bank_number = (cart.cartState & 0x1F0) >> 4;
	switch (cart.romSize) {
		case romSize_1MB: bank_number |= (cart.cartState & 0x0200) >> 4; break;
		case romSize_2MB: bank_number |= (cart.cartState & 0x0600) >> 4; break;
	}
	m_ROM_NN = m_ROM + (bank_number * n_ROM_NN);

	// Do stuff based on mode
	switch (cart.cartState & 0x0800) {
		case 0x0000:
			m_ROM_00 = m_ROM;
			m_ERAM = startAddr_RAM;
			break;
		case 0x0800:
			bank_number = (cart.cartState & 0x0600) >> 9;
			m_ROM_00 = m_ROM + ((bank_number << 5) * n_ROM_NN);
			switch(cart.ramSize) {
				case ramSize_NONE: case ramSize_1BANKS:
					m_ERAM = startAddr_RAM;
					break;
				case ramSize_4BANKS:
					m_ERAM = startAddr_RAM + (bank_number * n_ERAM);
					break;
			}
			break;
	}
}
void MBC2_WB(uint16_t addr, uint8_t value) {
	
}



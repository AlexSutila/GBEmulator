#ifndef CPU_H_
#define CPU_H_

// Type definitions
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef signed short int16_t;

// The flags register bits (lower 8 bits of AF register)
#define cb_FLAG_Z 7 // Zero flag
#define cb_FLAG_N 6 // Subtraction flag (BCD)
#define cb_FLAG_H 5 // Half Carry flag (BCD)
#define cb_FLAG_C 4 // Carry flag 

// The flags register mask
#define cm_FLAG_Z 0b00000001 // Zero flag
#define cm_FLAG_N 0b00000010 // Subtraction flag (BCD)
#define cm_FLAG_H 0b00000100 // Half Carry flag (BCD)
#define cm_FLAG_C 0b00001000 // Carry flag

struct instrTimingInfo 
{
	uint8_t cycles, postWrite_cycles;
};

// Conditional Table
enum conditionTable { cond_NZ, cond_Z, cond_NC, cond_C };

struct sharp_SM83 {

	// Registers AF, BC, DE, and HL can all be used as both
	//		8 bit registers, aswell as two individual 16 bit
	//		registers
	union {
		uint16_t AF;
		struct {
			uint8_t F, A;
		};
	};
	union {
		uint16_t BC;
		struct {
			uint8_t C, B;
		};
	};
	union {
		uint16_t DE;
		struct {
			uint8_t E, D;
		};
	};
	union {
		uint16_t HL;
		struct {
			uint8_t L, H;
		};
	};
	uint16_t SP, PC;

	// Interrupt stuff
	uint16_t IME_scheduler;
	uint8_t IME, halt;

};

// Function Definitions 
void cpu_init(struct sharp_SM83* cpu_ptr);
int int_request(struct GB* gb, int cycles);
struct instrTimingInfo cpu_execute(struct GB* gb);

#endif









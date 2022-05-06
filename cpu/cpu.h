#ifndef CPU_H_
#define CPU_H_

#include "types.h"

// The flags register bits (lower 8 bits of AF register)
enum flagBits
{
	flagBitZ = 7,
	flagBitN = 6,
	flagBitH = 5,
	flagBitC = 4
};

// Enumeration for the different conditions for conditional jumps
enum conditionTable
{
	// https://gb-archive.github.io/salvage/decoding_gbz80_opcodes/Decoding%20Gamboy%20Z80%20Opcodes.html
	//		... see Table "cc" 
	/*0b00*/ cond_NZ, // Zero flag not set
	/*0b01*/ cond_Z,  // Zero flag set
	/*0b10*/ cond_NC, // Carry flag not set
	/*0b11*/ cond_C   // Carry flag set
};

// Correct memory access timing can be achieved by splitting instructions into micro instructions
//		based on where memory accesses within those instructions occur. However, not every single
//		instruction executed needs to be split into micro instructions every single time. 
struct instrTimingInfo 
{
	// Cycles: The total amount of clock cycles a given instruction takes. I've decided to 
	//		base things in terms of T-cycles rather than M-cycles. If memory access timing
	//		is not a factor in this instruction, all components will be stepped 'cycles' 
	//		times.
	//		...
	// postWrite_cycles: When an instruction needs to write to a memory location that is
	//		sensative to timing, it will be stepped postWrite_cycles times in the main loop.
	//		This is the number of clock cycles after any writes to the memory address.
	//		...
	uint8_t cycles, postWrite_cycles;
};
// With this structure, I am in a nutshell able to achieve being able to split instructions
//		into micro instructions and only synchronize the components affected by the memory
//		access after the end of each micro instruction when it's necessary. All clock cycles
//		for instructions that don't run the risk of a poor memory access timing are executed
//		in bulk, saving time by synchronizing less, and when the timing is actually important
//		it only synchronizes the component it needs to in between micro instructions. This 
//		decision is made entirely at runtime using the lookup tables in the RB/WB functions in
//		mem.h

// Structure for the CPU itself
struct sharp_SM83 
{
	// Registers AF, BC, DE, and HL can all be used as both
	//		16 bit registers, aswell as two individual 8 bit
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

	// IME stands for interrupt master enable and can be set using
	//		the EI and DI instructions.
	//		...
	// halt will be zero or one depending on if the CPU is in halt
	//		mode or not. My implementation of halt is subject to 
	//		change (implementing stop might complicate things)
	uint8_t IME, halt;
	// The EI instruction sets the IME, but the setting of IME does
	//		not happen immediately. See int_request and implementation
	//		of EI instruction to see how writing this variable queues
	//		the IME to be set after a few clock cycles. 
	// NOTE: No documentation talks about any IME scheduler, this is
	//		just my solution to this problem. 
	uint16_t IME_scheduler;
};

// Populates all registers with zero values and intializes some other
//		cpu mode and interrupts related stuff
void cpu_init(struct sharp_SM83* cpu_ptr);

// Handles interrupts and returns the number of cycles it took to service
//		any interrupts that are signaled. If no interrupts are triggered
//		zero cycles are returned. 
//		...
// This function also takes care of the whole IME_scheduler thing plus the
//		setting of IME after a certain amount of clock cycles. 
int int_request(struct GB* gb, int cycles);

// This will execute a single instruction or do nothing for four clock 
//		cycles depending on if the CPU is in halt mode. The timing info
//		returned is specific to the instruction that is pointed to by 
//		the PC register, instrTimingInfo fields are explained farther up. 
struct instrTimingInfo cpu_execute(struct GB* gb);

#endif

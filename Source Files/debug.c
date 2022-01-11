#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <wchar.h>
#include "debug.h"
#include "mem.h"

#define CONSOLE_WIDTH 120
#define CONSOLE_HEIGHT 40 

static wchar_t buffer[CONSOLE_WIDTH * CONSOLE_HEIGHT] = { 
	L"                                                                                                                        "
	L" AF: 0000 BC: 0000 DE: 0000   Flags: - - - - - - - -    Disassembly:                                                    "
	L" HL: 0000 SP: 0000 PC: 0000   IME: 0      Halted: 0                                                                     "
	L"                                                          NOP                                                           "
	L" MEM: -0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -A -B -C -D -E -F     NOP                                                           "
	L" 000- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 001- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 002- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 003- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 004- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 005- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 006- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00   >> NOP                                                          "
	L" 007- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 008- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 009- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 00A- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 00B- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 00C- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 00D- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 00E- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L" 00F- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     NOP                                                           "
	L"                                                                                                                        "
	L" JOYP: 00    DMA: 00             IE    IF               Breakpoints:                                                    "
	L"  DIV: 00     LY: 00   VBLNK:                            1:                                                             "
	L" TIMA: 00    LYC: 00    STAT:                            2:                                                             "
	L"  TMA: 00     WY: 00   TIMER:                            3:                                                             "
	L"  TAC: 00     WX: 00   SRIAL:                            4:                                                             "
	L" LCDC: 00    SCY: 00    JOYP:                            5:                                                             "
	L" STAT: 00    SCX: 00                                     6:                                                             "
	L"                                                                                                                        "
};

struct instruction
{
	wchar_t* str;  // Instruction name
	int byteCount; // Instruction length
};

// '##'   - Immediate value
// '####' - Immediate address
// '&&&&' - Signed offset + addr
static const struct instruction lookup[256] = {
	{ L"NOP",				1 },
	{ L"LD BC,$####",		3 },
	{ L"LD (BC),A",			1 },
	{ L"INC BC",			1 },
	{ L"INC B",				1 },
	{ L"DEC B",				1 },
	{ L"LD B,$##",			2 },
	{ L"RLC A",				1 },
	{ L"LD ($####),SP",		3 },
	{ L"ADD HL,BC",			1 },
	{ L"LD A,(BC)",			1 },
	{ L"DEC BC",			1 },
	{ L"INC C",				1 },
	{ L"DEC C",				1 },
	{ L"LD C,$##",			2 },
	{ L"RRC A",				1 },
	{ L"STOP",				1 },
	{ L"LD DE,$####",		3 },
	{ L"LD (DE),A",			1 },
	{ L"INC DE",			1 },
	{ L"INC D",				1 },
	{ L"DEC D",				1 },
	{ L"LD D,$##",			2 },
	{ L"RL A",				1 },
	{ L"JR $&&&&",			2 },
	{ L"ADD HL,DE",			1 },
	{ L"LD A,(DE)",			1 },
	{ L"DEC DE",			1 },
	{ L"INC E",				1 },
	{ L"DEC E",				1 },
	{ L"LD E,$##",			2 },
	{ L"RR A",				1 },
	{ L"JR NZ,$&&&&",		2 },
	{ L"LD HL,$####",		3 },
	{ L"LDI (HL),A",		1 },
	{ L"INC HL",			1 },
	{ L"INC H",				1 },
	{ L"DEC H",				1 },
	{ L"LD H,$##",			2 },
	{ L"DAA",				1 },
	{ L"JR Z,$&&&&",		2 },
	{ L"ADD HL,HL",			1 },
	{ L"LDI A,(HL)",		1 },
	{ L"DEC HL",			1 },
	{ L"INC L",				1 },
	{ L"DEC L",				1 },
	{ L"LD L,$##",			2 },
	{ L"CPL",				1 },
	{ L"JR NC,$&&&&",		2 },
	{ L"LD SP,$####",		3 },
	{ L"LDD (HL),A",		1 },
	{ L"INC SP",			1 },
	{ L"INC (HL)",			1 },
	{ L"DEC (HL)",			1 },
	{ L"LD (HL),$##",		2 },
	{ L"SCF",				1 },
	{ L"JR C,$&&&&",		2 },
	{ L"ADD HL,SP",			1 },
	{ L"LDD A,(HL)",		1 },
	{ L"DEC SP",			1 },
	{ L"INC A",				1 },
	{ L"DEC A",				1 },
	{ L"LD A,$##",			2 },
	{ L"CCF",				1 },
	{ L"LD B,B",			1 },
	{ L"LD B,C",			1 },
	{ L"LD B,D",			1 },
	{ L"LD B,E",			1 },
	{ L"LD B,H",			1 },
	{ L"LD B,L",			1 },
	{ L"LD B,(HL)",			1 },
	{ L"LD B,A",			1 },
	{ L"LD C,B",			1 },
	{ L"LD C,C",			1 },
	{ L"LD C,D",			1 },
	{ L"LD C,E",			1 },
	{ L"LD C,H",			1 },
	{ L"LD C,L",			1 },
	{ L"LD C,(HL)",			1 },
	{ L"LD C,A",			1 },
	{ L"LD D,B",			1 },
	{ L"LD D,C",			1 },
	{ L"LD D,D",			1 },
	{ L"LD D,E",			1 },
	{ L"LD D,H",			1 },
	{ L"LD D,L",			1 },
	{ L"LD D,(HL)",			1 },
	{ L"LD D,A",			1 },
	{ L"LD E,B",			1 },
	{ L"LD E,C",			1 },
	{ L"LD E,D",			1 },
	{ L"LD E,E",			1 },
	{ L"LD E,H",			1 },
	{ L"LD E,L",			1 },
	{ L"LD E,(HL)",			1 },
	{ L"LD E,A",			1 },
	{ L"LD H,B",			1 },
	{ L"LD H,C",			1 },
	{ L"LD H,D",			1 },
	{ L"LD H,E",			1 },
	{ L"LD H,H",			1 },
	{ L"LD H,L",			1 },
	{ L"LD H,(HL)",			1 },
	{ L"LD H,A",			1 },
	{ L"LD L,B",			1 },
	{ L"LD L,C",			1 },
	{ L"LD L,D",			1 },
	{ L"LD L,E",			1 },
	{ L"LD L,H",			1 },
	{ L"LD L,L",			1 },
	{ L"LD L,(HL)",			1 },
	{ L"LD L,A",			1 },
	{ L"LD (HL),B",			1 },
	{ L"LD (HL),C",			1 },
	{ L"LD (HL),D",			1 },
	{ L"LD (HL),E",			1 },
	{ L"LD (HL),H",			1 },
	{ L"LD (HL),L",			1 },
	{ L"HALT",				1 },
	{ L"LD (HL),A",			1 },
	{ L"LD A,B",			1 },
	{ L"LD A,C",			1 },
	{ L"LD A,D",			1 },
	{ L"LD A,E",			1 },
	{ L"LD A,H",			1 },
	{ L"LD A,L",			1 },
	{ L"LD A,(HL)",			1 },
	{ L"LD A,A",			1 },
	{ L"ADD A,B",			1 },
	{ L"ADD A,C",			1 },
	{ L"ADD A,D",			1 },
	{ L"ADD A,E",			1 },
	{ L"ADD A,H",			1 },
	{ L"ADD A,L",			1 },
	{ L"ADD A,(HL)",		1 },
	{ L"ADD A,A",			1 },
	{ L"ADC A,B",			1 },
	{ L"ADC A,C",			1 },
	{ L"ADC A,D",			1 },
	{ L"ADC A,E",			1 },
	{ L"ADC A,H",			1 },
	{ L"ADC A,L",			1 },
	{ L"ADC A,(HL)",		1 },
	{ L"ADC A,A",			1 },
	{ L"SUB A,B",			1 },
	{ L"SUB A,C",			1 },
	{ L"SUB A,D",			1 },
	{ L"SUB A,E",			1 },
	{ L"SUB A,H",			1 },
	{ L"SUB A,L",			1 },
	{ L"SUB A,(HL)",		1 },
	{ L"SUB A,A",			1 },
	{ L"SBC A,B",			1 },
	{ L"SBC A,C",			1 },
	{ L"SBC A,D",			1 },
	{ L"SBC A,E",			1 },
	{ L"SBC A,H",			1 },
	{ L"SBC A,L",			1 },
	{ L"SBC A,(HL)",		1 },
	{ L"SBC A,A",			1 },
	{ L"AND B",				1 },
	{ L"AND C",				1 },
	{ L"AND D",				1 },
	{ L"AND E",				1 },
	{ L"AND H",				1 },
	{ L"AND L",				1 },
	{ L"AND (HL)",			1 },
	{ L"AND A",				1 },
	{ L"XOR B",				1 },
	{ L"XOR C",				1 },
	{ L"XOR D",				1 },
	{ L"XOR E",				1 },
	{ L"XOR H",				1 },
	{ L"XOR L",				1 },
	{ L"XOR (HL)",			1 },
	{ L"XOR A",				1 },
	{ L"OR B",				1 },
	{ L"OR C",				1 },
	{ L"OR D",				1 },
	{ L"OR E",				1 },
	{ L"OR H",				1 },
	{ L"OR L",				1 },
	{ L"OR (HL)",			1 },
	{ L"OR A",				1 },
	{ L"CP B",				1 },
	{ L"CP C",				1 },
	{ L"CP D",				1 },
	{ L"CP E",				1 },
	{ L"CP H",				1 },
	{ L"CP L",				1 },
	{ L"CP (HL)",			1 },
	{ L"CP A",				1 },
	{ L"RET NZ",			1 },
	{ L"POP BC",			1 },
	{ L"JP NZ,$####",		3 },
	{ L"JP $####",			3 },
	{ L"CALL NZ,$####",		3 },
	{ L"PUSH BC",			1 },
	{ L"ADD A,$##",			2 },
	{ L"RST 0",				1 },
	{ L"RET Z",				1 },
	{ L"RET",				1 },
	{ L"JP Z,$####",		3 },
	{ L"Ext ops",			1 },
	{ L"CALL Z,$####",		3 },
	{ L"CALL $####",		3 },
	{ L"ADC A,$##",			2 },
	{ L"RST 8",				1 },
	{ L"RET NC",			1 },
	{ L"POP DE",			1 },
	{ L"JP NC,$####",		3 },
	{ L"XX",				1 },
	{ L"CALL NC,$####",		3 },
	{ L"PUSH DE",			1 },
	{ L"SUB A,$##",			2 },
	{ L"RST 10",			1 },
	{ L"RET C",				1 },
	{ L"RETI",				1 },
	{ L"JP C,$####",		3 },
	{ L"XX",				1 },
	{ L"CALL C,$####",		3 },
	{ L"XX",				1 },
	{ L"SBC A,$##",			2 },
	{ L"RST 18",			1 },
	{ L"LDH ($##),A",		2 },
	{ L"POP HL",			1 },
	{ L"LDH (C),A",			1 },
	{ L"XX",				1 },
	{ L"XX",				1 },
	{ L"PUSH HL",			1 },
	{ L"AND $##",			2 },
	{ L"RST 20",			1 },
	{ L"ADD SP,d",			1 },
	{ L"JP (HL)",			1 },
	{ L"LD ($####),A",		3 },
	{ L"XX",				1 },
	{ L"XX",				1 },
	{ L"XX",				1 },
	{ L"XOR $##",			2 },
	{ L"RST 28",			1 },
	{ L"LDH A,($##)",		2 },
	{ L"POP AF",			1 },
	{ L"XX",				1 },
	{ L"DI",				1 },
	{ L"XX",				1 },
	{ L"PUSH AF",			1 },
	{ L"OR $##",			2 },
	{ L"RST 30",			1 },
	{ L"LDHL SP,d",			1 },
	{ L"LD SP,HL",			1 },
	{ L"LD A,($####)",		3 },
	{ L"EI",				1 },
	{ L"XX",				1 },
	{ L"XX",				1 },
	{ L"CP $##",			2 },
	{ L"RST 38",			1 }
};
static const struct instruction lookup_CB[256] = {
	{ L"RLC B",			2 },
	{ L"RLC C",			2 },
	{ L"RLC D",			2 },
	{ L"RLC E",			2 },
	{ L"RLC H",			2 },
	{ L"RLC L",			2 },
	{ L"RLC (HL)",		2 },
	{ L"RLC A",			2 },
	{ L"RRC B",			2 },
	{ L"RRC C",			2 },
	{ L"RRC D",			2 },
	{ L"RRC E",			2 },
	{ L"RRC H",			2 },
	{ L"RRC L",			2 },
	{ L"RRC (HL)",		2 },
	{ L"RRC A",			2 },
	{ L"RL B",			2 },
	{ L"RL C",			2 },
	{ L"RL D",			2 },
	{ L"RL E",			2 },
	{ L"RL H",			2 },
	{ L"RL L",			2 },
	{ L"RL (HL)",		2 },
	{ L"RL A",			2 },
	{ L"RR B",			2 },
	{ L"RR C",			2 },
	{ L"RR D",			2 },
	{ L"RR E",			2 },
	{ L"RR H",			2 },
	{ L"RR L",			2 },
	{ L"RR (HL)",		2 },
	{ L"RR A",			2 },
	{ L"SLA B",			2 },
	{ L"SLA C",			2 },
	{ L"SLA D",			2 },
	{ L"SLA E",			2 },
	{ L"SLA H",			2 },
	{ L"SLA L",			2 },
	{ L"SLA (HL)",		2 },
	{ L"SLA A",			2 },
	{ L"SRA B",			2 },
	{ L"SRA C",			2 },
	{ L"SRA D",			2 },
	{ L"SRA E",			2 },
	{ L"SRA H",			2 },
	{ L"SRA L",			2 },
	{ L"SRA (HL)",		2 },
	{ L"SRA A",			2 },
	{ L"SWAP B",		2 },
	{ L"SWAP C",		2 },
	{ L"SWAP D",		2 },
	{ L"SWAP E",		2 },
	{ L"SWAP H",		2 },
	{ L"SWAP L",		2 },
	{ L"SWAP (HL)",		2 },
	{ L"SWAP A",		2 },
	{ L"SRL B",			2 },
	{ L"SRL C",			2 },
	{ L"SRL D",			2 },
	{ L"SRL E",			2 },
	{ L"SRL H",			2 },
	{ L"SRL L",			2 },
	{ L"SRL (HL)",		2 },
	{ L"SRL A",			2 },
	{ L"BIT 0,B",		2 },
	{ L"BIT 0,C",		2 },
	{ L"BIT 0,D",		2 },
	{ L"BIT 0,E",		2 },
	{ L"BIT 0,H",		2 },
	{ L"BIT 0,L",		2 },
	{ L"BIT 0,(HL)",	2 },
	{ L"BIT 0,A",		2 },
	{ L"BIT 1,B",		2 },
	{ L"BIT 1,C",		2 },
	{ L"BIT 1,D",		2 },
	{ L"BIT 1,E",		2 },
	{ L"BIT 1,H",		2 },
	{ L"BIT 1,L",		2 },
	{ L"BIT 1,(HL)",	2 },
	{ L"BIT 1,A",		2 },
	{ L"BIT 2,B",		2 },
	{ L"BIT 2,C",		2 },
	{ L"BIT 2,D",		2 },
	{ L"BIT 2,E",		2 },
	{ L"BIT 2,H",		2 },
	{ L"BIT 2,L",		2 },
	{ L"BIT 2,(HL)",	2 },
	{ L"BIT 2,A",		2 },
	{ L"BIT 3,B",		2 },
	{ L"BIT 3,C",		2 },
	{ L"BIT 3,D",		2 },
	{ L"BIT 3,E",		2 },
	{ L"BIT 3,H",		2 },
	{ L"BIT 3,L",		2 },
	{ L"BIT 3,(HL)",	2 },
	{ L"BIT 3,A",		2 },
	{ L"BIT 4,B",		2 },
	{ L"BIT 4,C",		2 },
	{ L"BIT 4,D",		2 },
	{ L"BIT 4,E",		2 },
	{ L"BIT 4,H",		2 },
	{ L"BIT 4,L",		2 },
	{ L"BIT 4,(HL)",	2 },
	{ L"BIT 4,A",		2 },
	{ L"BIT 5,B",		2 },
	{ L"BIT 5,C",		2 },
	{ L"BIT 5,D",		2 },
	{ L"BIT 5,E",		2 },
	{ L"BIT 5,H",		2 },
	{ L"BIT 5,L",		2 },
	{ L"BIT 5,(HL)",	2 },
	{ L"BIT 5,A",		2 },
	{ L"BIT 6,B",		2 },
	{ L"BIT 6,C",		2 },
	{ L"BIT 6,D",		2 },
	{ L"BIT 6,E",		2 },
	{ L"BIT 6,H",		2 },
	{ L"BIT 6,L",		2 },
	{ L"BIT 6,(HL)",	2 },
	{ L"BIT 6,A",		2 },
	{ L"BIT 7,B",		2 },
	{ L"BIT 7,C",		2 },
	{ L"BIT 7,D",		2 },
	{ L"BIT 7,E",		2 },
	{ L"BIT 7,H",		2 },
	{ L"BIT 7,L",		2 },
	{ L"BIT 7,(HL)",	2 },
	{ L"BIT 7,A",		2 },
	{ L"RES 0,B",		2 },
	{ L"RES 0,C",		2 },
	{ L"RES 0,D",		2 },
	{ L"RES 0,E",		2 },
	{ L"RES 0,H",		2 },
	{ L"RES 0,L",		2 },
	{ L"RES 0,(HL)",	2 },
	{ L"RES 0,A",		2 },
	{ L"RES 1,B",		2 },
	{ L"RES 1,C",		2 },
	{ L"RES 1,D",		2 },
	{ L"RES 1,E",		2 },
	{ L"RES 1,H",		2 },
	{ L"RES 1,L",		2 },
	{ L"RES 1,(HL)",	2 },
	{ L"RES 1,A",		2 },
	{ L"RES 2,B",		2 },
	{ L"RES 2,C",		2 },
	{ L"RES 2,D",		2 },
	{ L"RES 2,E",		2 },
	{ L"RES 2,H",		2 },
	{ L"RES 2,L",		2 },
	{ L"RES 2,(HL)",	2 },
	{ L"RES 2,A",		2 },
	{ L"RES 3,B",		2 },
	{ L"RES 3,C",		2 },
	{ L"RES 3,D",		2 },
	{ L"RES 3,E",		2 },
	{ L"RES 3,H",		2 },
	{ L"RES 3,L",		2 },
	{ L"RES 3,(HL)",	2 },
	{ L"RES 3,A",		2 },
	{ L"RES 4,B",		2 },
	{ L"RES 4,C",		2 },
	{ L"RES 4,D",		2 },
	{ L"RES 4,E",		2 },
	{ L"RES 4,H",		2 },
	{ L"RES 4,L",		2 },
	{ L"RES 4,(HL)",	2 },
	{ L"RES 4,A",		2 },
	{ L"RES 5,B",		2 },
	{ L"RES 5,C",		2 },
	{ L"RES 5,D",		2 },
	{ L"RES 5,E",		2 },
	{ L"RES 5,H",		2 },
	{ L"RES 5,L",		2 },
	{ L"RES 5,(HL)",	2 },
	{ L"RES 5,A",		2 },
	{ L"RES 6,B",		2 },
	{ L"RES 6,C",		2 },
	{ L"RES 6,D",		2 },
	{ L"RES 6,E",		2 },
	{ L"RES 6,H",		2 },
	{ L"RES 6,L",		2 },
	{ L"RES 6,(HL)",	2 },
	{ L"RES 6,A",		2 },
	{ L"RES 7,B",		2 },
	{ L"RES 7,C",		2 },
	{ L"RES 7,D",		2 },
	{ L"RES 7,E",		2 },
	{ L"RES 7,H",		2 },
	{ L"RES 7,L",		2 },
	{ L"RES 7,(HL)",	2 },
	{ L"RES 7,A",		2 },
	{ L"SET 0,B",		2 },
	{ L"SET 0,C",		2 },
	{ L"SET 0,D",		2 },
	{ L"SET 0,E",		2 },
	{ L"SET 0,H",		2 },
	{ L"SET 0,L",		2 },
	{ L"SET 0,(HL)",	2 },
	{ L"SET 0,A",		2 },
	{ L"SET 1,B",		2 },
	{ L"SET 1,C",		2 },
	{ L"SET 1,D",		2 },
	{ L"SET 1,E",		2 },
	{ L"SET 1,H",		2 },
	{ L"SET 1,L",		2 },
	{ L"SET 1,(HL)",	2 },
	{ L"SET 1,A",		2 },
	{ L"SET 2,B",		2 },
	{ L"SET 2,C",		2 },
	{ L"SET 2,D",		2 },
	{ L"SET 2,E",		2 },
	{ L"SET 2,H",		2 },
	{ L"SET 2,L",		2 },
	{ L"SET 2,(HL)",	2 },
	{ L"SET 2,A",		2 },
	{ L"SET 3,B",		2 },
	{ L"SET 3,C",		2 },
	{ L"SET 3,D",		2 },
	{ L"SET 3,E",		2 },
	{ L"SET 3,H",		2 },
	{ L"SET 3,L",		2 },
	{ L"SET 3,(HL)",	2 },
	{ L"SET 3,A",		2 },
	{ L"SET 4,B",		2 },
	{ L"SET 4,C",		2 },
	{ L"SET 4,D",		2 },
	{ L"SET 4,E",		2 },
	{ L"SET 4,H",		2 },
	{ L"SET 4,L",		2 },
	{ L"SET 4,(HL)",	2 },
	{ L"SET 4,A",		2 },
	{ L"SET 5,B",		2 },
	{ L"SET 5,C",		2 },
	{ L"SET 5,D",		2 },
	{ L"SET 5,E",		2 },
	{ L"SET 5,H",		2 },
	{ L"SET 5,L",		2 },
	{ L"SET 5,(HL)",	2 },
	{ L"SET 5,A",		2 },
	{ L"SET 6,B",		2 },
	{ L"SET 6,C",		2 },
	{ L"SET 6,D",		2 },
	{ L"SET 6,E",		2 },
	{ L"SET 6,H",		2 },
	{ L"SET 6,L",		2 },
	{ L"SET 6,(HL)",	2 },
	{ L"SET 6,A",		2 },
	{ L"SET 7,B",		2 },
	{ L"SET 7,C",		2 },
	{ L"SET 7,D",		2 },
	{ L"SET 7,E",		2 },
	{ L"SET 7,H",		2 },
	{ L"SET 7,L",		2 },
	{ L"SET 7,(HL)",	2 },
	{ L"SET 7,A",		2 }
};

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Given address of instruction, return address of previous instruction
//		this is not unfortunetally is not exact
inline uint16_t prevInstruction(struct GB* gb, uint16_t addr)
{
	uint8_t opcode = RB(gb, addr - 3, 0);
	if (lookup[opcode].byteCount == 3) return addr - 3;

	opcode = RB(gb, addr - 2, 0);
	if (opcode == 0xCB || lookup[opcode].byteCount == 2)
		return addr - 2;

	return addr - 1;
}

// Given address of instruction, return address of next instruction
inline uint16_t nextInstruction(struct GB* gb, uint16_t addr)
{
	uint8_t opcode = RB(gb, addr, 0);
	struct instruction cur_instr = (opcode == 0xCB) ?
		lookup_CB[RB(gb, addr + 1, 0)] : lookup[opcode];

	return addr + cur_instr.byteCount;
}


void disassemble(struct GB* gb, wchar_t instrBuf[25], uint16_t addr)
{
	wmemset(instrBuf, ' ', 25);
	uint8_t opcode = RB(gb, addr & 0xFFFF, 0);

	if (opcode == 0xCB)
	{
		opcode = RB(gb, ++addr & 0xFFFF, 0);
		wcscpy(instrBuf, lookup_CB[opcode].str);
	}
	else
	{
		// swprintf includes a null terminator
		wcscpy(instrBuf, lookup[opcode].str);
		wchar_t* imm = wcsstr(instrBuf, L"####");
		if (imm != NULL) {
			wchar_t temp[5];
			uint16_t u16 = RB(gb, addr + 1, 0) | (RB(gb, addr + 2, 0) << 8);
			swprintf(temp, 5, L"%04X", u16);
			wmemcpy(imm, temp, 4);
			return;
		}
		imm = wcsstr(instrBuf, L"##");
		if (imm != NULL) {
			wchar_t temp[3];
			swprintf(temp, 3, L"%02X", RB(gb, ++addr, 0));
			wmemcpy(imm, temp, 2);
			return;
		}
		imm = wcsstr(instrBuf, L"&&&&");
		if (imm != NULL) {
			wchar_t temp[5];
			int8_t offset = RB(gb, ++addr, 0);
			// 2 bytes are read, so PC is incremented 
			//		prior to the branch, if its taken
			uint16_t u16 = ++addr + offset;
			swprintf(temp, 5, L"%04X", u16);
			wmemcpy(imm, temp, 4);
			return;
		}
	}
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void debug_init(HANDLE* hConsole, struct breakpoint* breakpoints)
{
	// Create debugger console
	AllocConsole();
	DWORD id = GetCurrentProcessId();
	AttachConsole(id);

	// Initialize console buffer
	*hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(*hConsole);

	// Initialize break points
	for (int i = 0; i < 6; i++)
	{
		breakpoints[i].enabled = 0;
		breakpoints[i].addr = 0x0000;
	}

	DWORD bytesWritten = 0;
	WriteConsoleOutputCharacter(*hConsole, buffer, CONSOLE_WIDTH * CONSOLE_HEIGHT, (COORD){ 0,0 }, &bytesWritten);
}

void debug_deinit()
{
	FreeConsole();
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Updates console buffer with contents from gb
void refresh_console(struct GB* gb, HANDLE* hConsole, uint16_t memViewBase)
{
	// Update CPU State
	swprintf(buffer + 125, 5, L"%04X", gb->cpu.AF);
	swprintf(buffer + 134, 5, L"%04X", gb->cpu.BC);
	swprintf(buffer + 143, 5, L"%04X", gb->cpu.DE);
	swprintf(buffer + 245, 5, L"%04X", gb->cpu.HL);
	swprintf(buffer + 254, 5, L"%04X", gb->cpu.SP);
	swprintf(buffer + 263, 5, L"%04X", gb->cpu.PC);

	swprintf(buffer + 290, 2, L"%d", gb->cpu.halt);
	swprintf(buffer + 275, 2, L"%d", gb->cpu.IME);
	buffer[157] = (gb->cpu.F & 0x80) ? 'Z' : '-';
	buffer[159] = (gb->cpu.F & 0x40) ? 'N' : '-';
	buffer[161] = (gb->cpu.F & 0x20) ? 'H' : '-';
	buffer[163] = (gb->cpu.F & 0x10) ? 'C' : '-';

	// Update important register view
	swprintf(buffer + 2647, 3, L"%02X", RB(gb, 0xFF00, 0));
	swprintf(buffer + 2767, 3, L"%02X", RB(gb, 0xFF04, 0));
	swprintf(buffer + 2887, 3, L"%02X", RB(gb, 0xFF05, 0));
	swprintf(buffer + 3007, 3, L"%02X", RB(gb, 0xFF06, 0));
	swprintf(buffer + 3127, 3, L"%02X", RB(gb, 0xFF07, 0));
	swprintf(buffer + 3247, 3, L"%02X", RB(gb, 0xFF40, 0));
	swprintf(buffer + 3367, 3, L"%02X", RB(gb, 0xFF41, 0));
	swprintf(buffer + 2658, 3, L"%02X", RB(gb, 0xFF46, 0));
	swprintf(buffer + 2778, 3, L"%02X", RB(gb, 0xFF44, 0));
	swprintf(buffer + 2898, 3, L"%02X", RB(gb, 0xFF45, 0));
	swprintf(buffer + 3018, 3, L"%02X", RB(gb, 0xFF4A, 0));
	swprintf(buffer + 3138, 3, L"%02X", RB(gb, 0xFF4B, 0));
	swprintf(buffer + 3258, 3, L"%02X", RB(gb, 0xFF42, 0));
	swprintf(buffer + 3378, 3, L"%02X", RB(gb, 0xFF43, 0));

	// Update interrupt registers IE and IF
	uint8_t IE = RB(gb, 0xFFFF, 0), IF = RB(gb, 0xFF0F, 0);
	buffer[2794] = (IE & 0x01) ? 'X' : '-'; buffer[2800] = (IF & 0x01) ? 'X' : '-';
	buffer[2914] = (IE & 0x02) ? 'X' : '-'; buffer[2920] = (IF & 0x02) ? 'X' : '-';
	buffer[3034] = (IE & 0x04) ? 'X' : '-'; buffer[3040] = (IF & 0x04) ? 'X' : '-';
	buffer[3154] = (IE & 0x08) ? 'X' : '-'; buffer[3160] = (IF & 0x08) ? 'X' : '-';
	buffer[3274] = (IE & 0x10) ? 'X' : '-'; buffer[3280] = (IF & 0x10) ? 'X' : '-';

	// Update Memory View
	for (int row = 5; row <= 20; row++) {
		swprintf(buffer + 1 + (120 * row), 5, L"%02X%X-", memViewBase >> 8, row - 5);
		for (int col = 0; col <= 0xF; col++) {
			swprintf((120 * row) + (buffer + 6) + (col * 3), 3, L"%02X", RB(gb, memViewBase, 0));
			memViewBase++;
		}
	}

	// Update disassembler
	disassemble(gb, buffer + 1379, gb->cpu.PC);
	int buf_pos = 1498; // To update following instructions
	uint16_t nextAddr = nextInstruction(gb, gb->cpu.PC);
	for (int i = 0; i < 9; i++) 
	{
		disassemble(gb, buffer + buf_pos, nextAddr);
		nextAddr = nextInstruction(gb, nextAddr);
		buf_pos += 120;
	}
	buf_pos = 1258; // To update the previous instructions - this could still use some work
	nextAddr = prevInstruction(gb, gb->cpu.PC);
	for (int i = 0; i < 8; i++)
	{
		disassemble(gb, buffer + buf_pos, nextAddr);
		nextAddr = prevInstruction(gb, nextAddr);
		buf_pos -= 120;
	}

	// Write results to console buffer
	DWORD bytesWritten = 0;
	WriteConsoleOutputCharacter(*hConsole, buffer, CONSOLE_WIDTH * CONSOLE_HEIGHT, (COORD) { 0, 0 }, &bytesWritten);
}

// Steps one instruction (Mostly copied from main)
void step_emulation(struct GB* gb, HWND window, HDC hdc)
{
	// Execute instruction and record timing
	struct instrTimingInfo timing = (gb->cpu.halt != 1) ?
		cpu_execute(gb) : (struct instrTimingInfo) { 4, 0 };

	// Handle interrupts and record timing, int_cycles = 0 if no interrupts serviced
	int int_cycles = int_request(gb, timing.cycles);

	// Sync components based on this timing info 
	int total = timing.cycles + int_cycles;
	switch (gb->sync_sel) {
	case 0: // ALL
		timers_step(gb, total);
		ppu_step(gb, total);
		break;
	case 1: // PPU
		timers_step(gb, total);
		ppu_step(gb, timing.postWrite_cycles + int_cycles);
		gb->sync_sel = 0;
		break;
	case 2: // Timers
		timers_step(gb, timing.postWrite_cycles + int_cycles);
		ppu_step(gb, total);
		gb->sync_sel = 0;
		break;
	}

	if (!gb->ppu.frameIncomplete)
	{
		gb->ppu.bitmap_PTR = (uint8_t*)gb->ppu.bitmap;
		gb->ppu.win_LY = 0x00; 
		StretchDIBits(hdc, 0, 0, v_WIDTH, v_HEIGHT, 0, 0, v_HRES, v_VRES, gb->ppu.bitmap, &gb->ppu.bitmapBMI->bmi, DIB_RGB_COLORS, SRCCOPY);
		gb->ppu.frameIncomplete = 1;
	}
}

// Not particularly trying to make this efficient
void editBreakPoint(HANDLE* hConsole, struct breakpoint* breakpoints, int bp)
{
	DWORD bytesWritten = 0;
	int buf_pos = 2820 + (bp * 120);
	if (breakpoints[bp].enabled)
	{
		wmemset(buffer + buf_pos, L' ', 5);
		breakpoints[bp].enabled = 0; 
	}
	else
	{
		uint16_t addr = 0x0000;
		*(buffer + buf_pos) = '$';
		WriteConsoleOutputCharacter(*hConsole, buffer, CONSOLE_WIDTH * CONSOLE_HEIGHT, (COORD) { 0, 0 }, &bytesWritten);

		int count = 3;
		char c = ' ';
		while (count >= 0) // Reads hex values and calculates break point address as they're entered
		{
			uint8_t offset = 0;
			c = _getch();
			
			if      (c >= '0' && c <= '9') offset = 48;
			else if (c >= 'a' && c <= 'f') offset = 87;
			else if (c >= 'A' && c <= 'F') offset = 55;
			else /* Non-hex value */       continue;

			*(buffer + buf_pos + 4 - count) = c;
			addr += ((uint16_t)(c - offset)) << (count * 4);
			WriteConsoleOutputCharacter(*hConsole, buffer, CONSOLE_WIDTH* CONSOLE_HEIGHT, (COORD) { 0, 0 }, &bytesWritten);
			count--;
		}
		breakpoints[bp].addr = addr;
		breakpoints[bp].enabled = 1;
	}
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void debug_break(struct GB* gb, HANDLE* hConsole, HWND window, HDC hdc, struct breakpoint* breakpoints)
{
	char c = ' ';
	static uint16_t memViewBase = 0x0000; 

	while (c != 'c')
	{
		refresh_console(gb, hConsole, memViewBase);
		c = _getch();
		
		switch (c)
		{
		case 'n': // Step emulation by one instruction
			step_emulation(gb, window, hdc);                                            break;
		case 'N': // Step emulation by one hundred isntructions
			for (int i = 0; i < 100; i++) step_emulation(gb, window, hdc);              break;
		case 'w': // Scroll memview up (decrement address)
			memViewBase = (memViewBase != 0x0000) ? memViewBase - 0x0100 : memViewBase; break;
		case 's': // Scroll memview down (increment address)
			memViewBase = (memViewBase != 0xFF00) ? memViewBase + 0x0100 : memViewBase; break;
		case '1': case '2': case '3': // Add or remove break point
		case '4': case '5': case '6':
			editBreakPoint(hConsole, breakpoints, c - 49 /* Convert from ascii to integer */);
			break;
		}
	}
}

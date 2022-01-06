#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <wchar.h>
#include "debug.h"
#include "mem.h"

#define CONSOLE_WIDTH 120
#define CONSOLE_HEIGHT 40

static DWORD bytesWritten = 0;
static wchar_t buffer[CONSOLE_WIDTH * CONSOLE_HEIGHT] = { 
	L"                                                                                                                        "
	L" AF: 0000 BC: 0000 DE: 0000   Flags: - - - - - - - -                                                                    "
	L" HL: 0000 SP: 0000 PC: 0000   IME: 0      Halted: 0                                                                     "
	L"                                                                                                                        "
	L" MEM: -0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -A -B -C -D -E -F                                                                   "
	L" 000- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 001- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 002- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 003- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 004- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 005- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 006- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 007- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 008- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 009- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00A- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00B- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00C- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00D- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00E- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L" 00F- 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00                                                                   "
	L"                                                                                                                        "
	L" JOYP: 00    DMA: 00             IE    IF                                                                               "
	L"  DIV: 00     LY: 00   VBLNK:                                                                                           "
	L" TIMA: 00    LYC: 00    STAT:                                                                                           "
	L"  TMA: 00     WY: 00   TIMER:                                                                                           "
	L"  TAC: 00     WX: 00   SRIAL:                                                                                           "
	L" LCDC: 00    SCY: 00    JOYP:                                                                                           "
	L" STAT: 00    SCX: 00                                                                                                    "
	L"                                                                                                                        "
};

void debug_init(HANDLE* hConsole)
{
	// Create debugger console
	AllocConsole();
	DWORD id = GetCurrentProcessId();
	AttachConsole(id);

	// Initialize console buffer
	*hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(*hConsole);
	WriteConsoleOutputCharacter(*hConsole, buffer, CONSOLE_WIDTH * CONSOLE_HEIGHT, (COORD){ 0,0 }, &bytesWritten);
}

void debug_deinit()
{
	FreeConsole();
}

void disassemble(struct GB* gb, wchar_t instrBuf[25], uint16_t addr)
{
	static const wchar_t* lookup[256] = {
		L"NOP",
		L"LD BC,####",
		L"LD (BC),A",
		L"INC BC",
		L"INC B",
		L"DEC B",
		L"LD B,##",
		L"RLC A",
		L"LD (####),SP",
		L"ADD HL,BC",
		L"LD A,(BC)",
		L"DEC BC",
		L"INC C",
		L"DEC C",
		L"LD C,##",
		L"RRC A",
		L"STOP",
		L"LD DE,####",
		L"LD (DE),A",
		L"INC DE",
		L"INC D",
		L"DEC D",
		L"LD D,##",
		L"RL A",
		L"JR ##",
		L"ADD HL,DE",
		L"LD A,(DE)",
		L"DEC DE",
		L"INC E",
		L"DEC E",
		L"LD E,##",
		L"RR A",
		L"JR NZ,##",
		L"LD HL,####",
		L"LDI (HL),A",
		L"INC HL",
		L"INC H",
		L"DEC H",
		L"LD H,##",
		L"DAA",
		L"JR Z,##",
		L"ADD HL,HL",
		L"LDI A,(HL)",
		L"DEC HL",
		L"INC L",
		L"DEC L",
		L"LD L,##",
		L"CPL",
		L"JR NC,##",
		L"LD SP,####",
		L"LDD (HL),A",
		L"INC SP",
		L"INC (HL)",
		L"DEC (HL)",
		L"LD (HL),##",
		L"SCF",
		L"JR C,##",
		L"ADD HL,SP",
		L"LDD A,(HL)",
		L"DEC SP",
		L"INC A",
		L"DEC A",
		L"LD A,##",
		L"CCF",
		L"LD B,B",
		L"LD B,C",
		L"LD B,D",
		L"LD B,E",
		L"LD B,H",
		L"LD B,L",
		L"LD B,(HL)",
		L"LD B,A",
		L"LD C,B",
		L"LD C,C",
		L"LD C,D",
		L"LD C,E",
		L"LD C,H",
		L"LD C,L",
		L"LD C,(HL)",
		L"LD C,A",
		L"LD D,B",
		L"LD D,C",
		L"LD D,D",
		L"LD D,E",
		L"LD D,H",
		L"LD D,L",
		L"LD D,(HL)",
		L"LD D,A",
		L"LD E,B",
		L"LD E,C",
		L"LD E,D",
		L"LD E,E",
		L"LD E,H",
		L"LD E,L",
		L"LD E,(HL)",
		L"LD E,A",
		L"LD H,B",
		L"LD H,C",
		L"LD H,D",
		L"LD H,E",
		L"LD H,H",
		L"LD H,L",
		L"LD H,(HL)",
		L"LD H,A",
		L"LD L,B",
		L"LD L,C",
		L"LD L,D",
		L"LD L,E",
		L"LD L,H",
		L"LD L,L",
		L"LD L,(HL)",
		L"LD L,A",
		L"LD (HL),B",
		L"LD (HL),C",
		L"LD (HL),D",
		L"LD (HL),E",
		L"LD (HL),H",
		L"LD (HL),L",
		L"HALT",
		L"LD (HL),A",
		L"LD A,B",
		L"LD A,C",
		L"LD A,D",
		L"LD A,E",
		L"LD A,H",
		L"LD A,L",
		L"LD A,(HL)",
		L"LD A,A",
		L"ADD A,B",
		L"ADD A,C",
		L"ADD A,D",
		L"ADD A,E",
		L"ADD A,H",
		L"ADD A,L",
		L"ADD A,(HL)",
		L"ADD A,A",
		L"ADC A,B",
		L"ADC A,C",
		L"ADC A,D",
		L"ADC A,E",
		L"ADC A,H",
		L"ADC A,L",
		L"ADC A,(HL)",
		L"ADC A,A",
		L"SUB A,B",
		L"SUB A,C",
		L"SUB A,D",
		L"SUB A,E",
		L"SUB A,H",
		L"SUB A,L",
		L"SUB A,(HL)",
		L"SUB A,A",
		L"SBC A,B",
		L"SBC A,C",
		L"SBC A,D",
		L"SBC A,E",
		L"SBC A,H",
		L"SBC A,L",
		L"SBC A,(HL)",
		L"SBC A,A",
		L"AND B",
		L"AND C",
		L"AND D",
		L"AND E",
		L"AND H",
		L"AND L",
		L"AND (HL)",
		L"AND A",
		L"XOR B",
		L"XOR C",
		L"XOR D",
		L"XOR E",
		L"XOR H",
		L"XOR L",
		L"XOR (HL)",
		L"XOR A",
		L"OR B",
		L"OR C",
		L"OR D",
		L"OR E",
		L"OR H",
		L"OR L",
		L"OR (HL)",
		L"OR A",
		L"CP B",
		L"CP C",
		L"CP D",
		L"CP E",
		L"CP H",
		L"CP L",
		L"CP (HL)",
		L"CP A",
		L"RET NZ",
		L"POP BC",
		L"JP NZ,####",
		L"JP ####",
		L"CALL NZ,####",
		L"PUSH BC",
		L"ADD A,##",
		L"RST 0",
		L"RET Z",
		L"RET",
		L"JP Z,####",
		L"Ext ops",
		L"CALL Z,####",
		L"CALL ####",
		L"ADC A,##",
		L"RST 8",
		L"RET NC",
		L"POP DE",
		L"JP NC,####",
		L"XX",
		L"CALL NC,####",
		L"PUSH DE",
		L"SUB A,##",
		L"RST 10",
		L"RET C",
		L"RETI",
		L"JP C,####",
		L"XX",
		L"CALL C,####",
		L"XX",
		L"SBC A,##",
		L"RST 18",
		L"LDH (##),A",
		L"POP HL",
		L"LDH (C),A",
		L"XX",
		L"XX",
		L"PUSH HL",
		L"AND ##",
		L"RST 20",
		L"ADD SP,d",
		L"JP (HL)",
		L"LD (####),A",
		L"XX",
		L"XX",
		L"XX",
		L"XOR ##",
		L"RST 28",
		L"LDH A,(##)",
		L"POP AF",
		L"XX",
		L"DI",
		L"XX",
		L"PUSH AF",
		L"OR ##",
		L"RST 30",
		L"LDHL SP,d",
		L"LD SP,HL",
		L"LD A,(####)",
		L"EI",
		L"XX",
		L"XX",
		L"CP ##",
		L"RST 38"
	};
	static const wchar_t* lookup_CB[256] = {
		L"RLC B",
		L"RLC C",
		L"RLC D",
		L"RLC E",
		L"RLC H",
		L"RLC L",
		L"RLC (HL)",
		L"RLC A",
		L"RRC B",
		L"RRC C",
		L"RRC D",
		L"RRC E",
		L"RRC H",
		L"RRC L",
		L"RRC (HL)",
		L"RRC A",
		L"RL B",
		L"RL C",
		L"RL D",
		L"RL E",
		L"RL H",
		L"RL L",
		L"RL (HL)",
		L"RL A",
		L"RR B",
		L"RR C",
		L"RR D",
		L"RR E",
		L"RR H",
		L"RR L",
		L"RR (HL)",
		L"RR A",
		L"SLA B",
		L"SLA C",
		L"SLA D",
		L"SLA E",
		L"SLA H",
		L"SLA L",
		L"SLA (HL)",
		L"SLA A",
		L"SRA B",
		L"SRA C",
		L"SRA D",
		L"SRA E",
		L"SRA H",
		L"SRA L",
		L"SRA (HL)",
		L"SRA A",
		L"SWAP B",
		L"SWAP C",
		L"SWAP D",
		L"SWAP E",
		L"SWAP H",
		L"SWAP L",
		L"SWAP (HL)",
		L"SWAP A",
		L"SRL B",
		L"SRL C",
		L"SRL D",
		L"SRL E",
		L"SRL H",
		L"SRL L",
		L"SRL (HL)",
		L"SRL A",
		L"BIT 0,B",
		L"BIT 0,C",
		L"BIT 0,D",
		L"BIT 0,E",
		L"BIT 0,H",
		L"BIT 0,L",
		L"BIT 0,(HL)",
		L"BIT 0,A",
		L"BIT 1,B",
		L"BIT 1,C",
		L"BIT 1,D",
		L"BIT 1,E",
		L"BIT 1,H",
		L"BIT 1,L",
		L"BIT 1,(HL)",
		L"BIT 1,A",
		L"BIT 2,B",
		L"BIT 2,C",
		L"BIT 2,D",
		L"BIT 2,E",
		L"BIT 2,H",
		L"BIT 2,L",
		L"BIT 2,(HL)",
		L"BIT 2,A",
		L"BIT 3,B",
		L"BIT 3,C",
		L"BIT 3,D",
		L"BIT 3,E",
		L"BIT 3,H",
		L"BIT 3,L",
		L"BIT 3,(HL)",
		L"BIT 3,A",
		L"BIT 4,B",
		L"BIT 4,C",
		L"BIT 4,D",
		L"BIT 4,E",
		L"BIT 4,H",
		L"BIT 4,L",
		L"BIT 4,(HL)",
		L"BIT 4,A",
		L"BIT 5,B",
		L"BIT 5,C",
		L"BIT 5,D",
		L"BIT 5,E",
		L"BIT 5,H",
		L"BIT 5,L",
		L"BIT 5,(HL)",
		L"BIT 5,A",
		L"BIT 6,B",
		L"BIT 6,C",
		L"BIT 6,D",
		L"BIT 6,E",
		L"BIT 6,H",
		L"BIT 6,L",
		L"BIT 6,(HL)",
		L"BIT 6,A",
		L"BIT 7,B",
		L"BIT 7,C",
		L"BIT 7,D",
		L"BIT 7,E",
		L"BIT 7,H",
		L"BIT 7,L",
		L"BIT 7,(HL)",
		L"BIT 7,A",
		L"RES 0,B",
		L"RES 0,C",
		L"RES 0,D",
		L"RES 0,E",
		L"RES 0,H",
		L"RES 0,L",
		L"RES 0,(HL)",
		L"RES 0,A",
		L"RES 1,B",
		L"RES 1,C",
		L"RES 1,D",
		L"RES 1,E",
		L"RES 1,H",
		L"RES 1,L",
		L"RES 1,(HL)",
		L"RES 1,A",
		L"RES 2,B",
		L"RES 2,C",
		L"RES 2,D",
		L"RES 2,E",
		L"RES 2,H",
		L"RES 2,L",
		L"RES 2,(HL)",
		L"RES 2,A",
		L"RES 3,B",
		L"RES 3,C",
		L"RES 3,D",
		L"RES 3,E",
		L"RES 3,H",
		L"RES 3,L",
		L"RES 3,(HL)",
		L"RES 3,A",
		L"RES 4,B",
		L"RES 4,C",
		L"RES 4,D",
		L"RES 4,E",
		L"RES 4,H",
		L"RES 4,L",
		L"RES 4,(HL)",
		L"RES 4,A",
		L"RES 5,B",
		L"RES 5,C",
		L"RES 5,D",
		L"RES 5,E",
		L"RES 5,H",
		L"RES 5,L",
		L"RES 5,(HL)",
		L"RES 5,A",
		L"RES 6,B",
		L"RES 6,C",
		L"RES 6,D",
		L"RES 6,E",
		L"RES 6,H",
		L"RES 6,L",
		L"RES 6,(HL)",
		L"RES 6,A",
		L"RES 7,B",
		L"RES 7,C",
		L"RES 7,D",
		L"RES 7,E",
		L"RES 7,H",
		L"RES 7,L",
		L"RES 7,(HL)",
		L"RES 7,A",
		L"SET 0,B",
		L"SET 0,C",
		L"SET 0,D",
		L"SET 0,E",
		L"SET 0,H",
		L"SET 0,L",
		L"SET 0,(HL)",
		L"SET 0,A",
		L"SET 1,B",
		L"SET 1,C",
		L"SET 1,D",
		L"SET 1,E",
		L"SET 1,H",
		L"SET 1,L",
		L"SET 1,(HL)",
		L"SET 1,A",
		L"SET 2,B",
		L"SET 2,C",
		L"SET 2,D",
		L"SET 2,E",
		L"SET 2,H",
		L"SET 2,L",
		L"SET 2,(HL)",
		L"SET 2,A",
		L"SET 3,B",
		L"SET 3,C",
		L"SET 3,D",
		L"SET 3,E",
		L"SET 3,H",
		L"SET 3,L",
		L"SET 3,(HL)",
		L"SET 3,A",
		L"SET 4,B",
		L"SET 4,C",
		L"SET 4,D",
		L"SET 4,E",
		L"SET 4,H",
		L"SET 4,L",
		L"SET 4,(HL)",
		L"SET 4,A",
		L"SET 5,B",
		L"SET 5,C",
		L"SET 5,D",
		L"SET 5,E",
		L"SET 5,H",
		L"SET 5,L",
		L"SET 5,(HL)",
		L"SET 5,A",
		L"SET 6,B",
		L"SET 6,C",
		L"SET 6,D",
		L"SET 6,E",
		L"SET 6,H",
		L"SET 6,L",
		L"SET 6,(HL)",
		L"SET 6,A",
		L"SET 7,B",
		L"SET 7,C",
		L"SET 7,D",
		L"SET 7,E",
		L"SET 7,H",
		L"SET 7,L",
		L"SET 7,(HL)",
		L"SET 7,A"
	};

	wmemset(instrBuf, ' ', 25);
	uint8_t opcode = RB(gb, addr & 0xFFFF, 0);

	if (opcode == 0xCB)
	{
		opcode = RB(gb, ++addr & 0xFFFF, 0);
		wcscpy(instrBuf, lookup_CB[opcode]);
	}
	else 
	{
		// This is disgusting but it works for now
		wcscpy(instrBuf, lookup[opcode]);
		wchar_t* imm = wcsstr(instrBuf, L"####");
		if (imm != NULL) {
			wchar_t temp[5];
			uint16_t u16 = RB(gb, addr+1, 0) | (RB(gb, addr+2, 0) << 8);
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
	}
}

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
	disassemble(gb, buffer + 175, gb->cpu.PC);

	// Write results to console buffer
	WriteConsoleOutputCharacter(*hConsole, buffer, CONSOLE_WIDTH * CONSOLE_HEIGHT, (COORD) { 0, 0 }, & bytesWritten);
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

void debug_break(struct GB* gb, HANDLE* hConsole, HWND window, HDC hdc)
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
		}
	}
}

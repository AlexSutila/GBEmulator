#include "instr.h"
#include "types.h"
#include "mem.h"
#include "cpu.h"

/*
		8 bit loads
*/

#define ld_x_y(x, y) \
struct instrTimingInfo ld_##x##_##y##(struct GB* gb) { \
	gb->cpu.x = gb->cpu.y; \
	return (struct instrTimingInfo) { 4, 4 }; \
}
ld_x_y(B, B) ld_x_y(B, C) ld_x_y(B, D) ld_x_y(B, E) ld_x_y(B, H) ld_x_y(B, L) ld_x_y(B, A)
ld_x_y(C, B) ld_x_y(C, C) ld_x_y(C, D) ld_x_y(C, E) ld_x_y(C, H) ld_x_y(C, L) ld_x_y(C, A)
ld_x_y(D, B) ld_x_y(D, C) ld_x_y(D, D) ld_x_y(D, E) ld_x_y(D, H) ld_x_y(D, L) ld_x_y(D, A)
ld_x_y(E, B) ld_x_y(E, C) ld_x_y(E, D) ld_x_y(E, E) ld_x_y(E, H) ld_x_y(E, L) ld_x_y(E, A)
ld_x_y(H, B) ld_x_y(H, C) ld_x_y(H, D) ld_x_y(H, E) ld_x_y(H, H) ld_x_y(H, L) ld_x_y(H, A)
ld_x_y(L, B) ld_x_y(L, C) ld_x_y(L, D) ld_x_y(L, E) ld_x_y(L, H) ld_x_y(L, L) ld_x_y(L, A)
ld_x_y(A, B) ld_x_y(A, C) ld_x_y(A, D) ld_x_y(A, E) ld_x_y(A, H) ld_x_y(A, L) ld_x_y(A, A)

#define ld_x_n(x) \
struct instrTimingInfo ld_##x##_n(struct GB* gb) { \
	gb->cpu.x = RB(gb, gb->cpu.PC++, 0); \
	return (struct instrTimingInfo) { 8, 8 }; \
}
ld_x_n(B) ld_x_n(C) ld_x_n(D) ld_x_n(E) 
ld_x_n(H) ld_x_n(L)	          ld_x_n(A)

#define ld_x_HL(x) \
struct instrTimingInfo ld_##x##_HL(struct GB* gb) { \
	gb->cpu.x = RB(gb, gb->cpu.HL, 0); \
	return (struct instrTimingInfo) { 8, 8 }; \
}
ld_x_HL(B) ld_x_HL(C) ld_x_HL(D) ld_x_HL(E)
ld_x_HL(H) ld_x_HL(L)            ld_x_HL(A)

#define ld_HL_x(x) \
struct instrTimingInfo ld_HL_##x##(struct GB* gb) { \
	WB(gb, gb->cpu.HL, gb->cpu.x, 0); \
	return (struct instrTimingInfo) { 8, 8 }; \
}
ld_HL_x(B) ld_HL_x(C) ld_HL_x(D) ld_HL_x(E)
ld_HL_x(H) ld_HL_x(L)            ld_HL_x(A)

struct instrTimingInfo ld_HL_n(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.PC++, 0);
	WB(gb, gb->cpu.HL, n, 4);
	return (struct instrTimingInfo) { 12, 8 };
}

#define ld_A_xx(xx) \
struct instrTimingInfo ld_A_##xx##(struct GB* gb) { \
	gb->cpu.A = RB(gb, gb->cpu.xx, 0); \
	return (struct instrTimingInfo) { 8, 8 }; \
}
ld_A_xx(BC) ld_A_xx(DE)

struct instrTimingInfo ld_A_nn(struct GB* gb) {
	uint8_t lsb = RB(gb, gb->cpu.PC++, 0);
	uint8_t msb = RB(gb, gb->cpu.PC++, 0);
	gb->cpu.A = RB(gb, (msb << 8) | lsb, 8);
	return (struct instrTimingInfo) { 16, 16 };
}

#define ld_xx_A(xx) \
struct instrTimingInfo ld_##xx##_A(struct GB* gb) { \
	WB(gb, gb->cpu.xx, gb->cpu.A, 0); \
	return (struct instrTimingInfo) { 8, 8 }; \
}
ld_xx_A(BC) ld_xx_A(DE)

struct instrTimingInfo ld_nn_A(struct GB* gb) {
	uint8_t lsb = RB(gb, gb->cpu.PC++, 0);
	uint8_t msb = RB(gb, gb->cpu.PC++, 0);
	WB(gb, (msb << 8) | lsb, gb->cpu.A, 8);
	return (struct instrTimingInfo) { 16, 8 };
}

struct instrTimingInfo ldh_A_n(struct GB* gb) {
	uint16_t n = 0xFF00 | RB(gb, gb->cpu.PC++, 0);
	gb->cpu.A = RB(gb, n, 4);
	return (struct instrTimingInfo) { 12, 12 };
}

struct instrTimingInfo ldh_n_A(struct GB* gb) {
	uint16_t n = 0xFF00 | RB(gb, gb->cpu.PC++, 0);
	WB(gb, n, gb->cpu.A, 4);
	return (struct instrTimingInfo) { 12, 8 };
}

struct instrTimingInfo ldh_A_C(struct GB* gb) {
	gb->cpu.A = RB(gb, 0xFF00 | gb->cpu.C, 0);
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo ldh_C_A(struct GB* gb) {
	WB(gb, 0xFF00 | gb->cpu.C, gb->cpu.A, 0);
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo ldi_HL_A(struct GB* gb) {
	WB(gb, gb->cpu.HL++, gb->cpu.A, 0);
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo ldi_A_HL(struct GB* gb) {
	gb->cpu.A = RB(gb, gb->cpu.HL++, 0);
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo ldd_HL_A(struct GB* gb) {
	WB(gb, gb->cpu.HL--, gb->cpu.A, 0);
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo ldd_A_HL(struct GB* gb) {
	gb->cpu.A = RB(gb, gb->cpu.HL--, 0);
	return (struct instrTimingInfo) { 8, 8 };
}

/*
		16 bit loads
*/

#define ld_xx_nn(xx) \
struct instrTimingInfo ld_##xx##_nn(struct GB* gb) { \
	uint16_t nn = RB(gb, gb->cpu.PC++, 0); \
	nn |= ((uint16_t)RB(gb, gb->cpu.PC++, 0)) << 8; \
	gb->cpu.xx = nn; \
	return (struct instrTimingInfo) { 12, 12 }; \
}
ld_xx_nn(BC) ld_xx_nn(DE) ld_xx_nn(HL) ld_xx_nn(SP)

struct instrTimingInfo ld_nn_SP(struct GB* gb) {
	uint16_t nn = RB(gb, gb->cpu.PC++, 0);
	nn |= ((uint16_t)RB(gb, gb->cpu.PC++, 0)) << 8;
	WB(gb, nn, (uint8_t)(gb->cpu.SP & 0xFF), 8);
	WB(gb, nn + 1, (uint8_t)(gb->cpu.SP >> 8), 4);
	return (struct instrTimingInfo) { 20, 8 };
}

struct instrTimingInfo ld_SP_HL(struct GB* gb) {
	gb->cpu.SP = gb->cpu.HL;
	return (struct instrTimingInfo) { 8, 8 };
}

#define push_xx(xx) \
struct instrTimingInfo push_##xx##(struct GB* gb) { \
	WB(gb, --gb->cpu.SP, (gb->cpu.xx >> 8), 0); \
	WB(gb, --gb->cpu.SP, (gb->cpu.xx & 0xFF), 0); \
	return (struct instrTimingInfo) { 16, 16 }; \
}
push_xx(BC) push_xx(DE) push_xx(HL) push_xx(AF)

#define pop_xx(xx) \
struct instrTimingInfo pop_##xx##(struct GB* gb) { \
	uint16_t val = RB(gb, gb->cpu.SP++, 0); \
	val |= ((uint16_t)RB(gb, gb->cpu.SP++, 0)) << 8; \
	gb->cpu.xx = val; \
	gb->cpu.F &= 0xF0;  \
	return (struct instrTimingInfo) { 12, 12 }; \
}
pop_xx(BC) pop_xx(DE) pop_xx(HL) pop_xx(AF)

/*
		8 bit arithmetic and logic
*/
#define add_A_x(x) \
struct instrTimingInfo add_A_##x##(struct GB* gb) { \
	gb->cpu.F = (((gb->cpu.x & 0xF) + (gb->cpu.A & 0xF)) & 0x10) << 1; \
	gb->cpu.F |= ((gb->cpu.x + gb->cpu.A) & 0x100) >> 4; \
	gb->cpu.A += gb->cpu.x; \
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 4, 4 }; \
}
add_A_x(B) add_A_x(C) add_A_x(D) add_A_x(E) 
add_A_x(H) add_A_x(L)            add_A_x(A)

struct instrTimingInfo add_A_n(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.PC++, 0);
	gb->cpu.F = (((n & 0xF) + (gb->cpu.A & 0xF)) & 0x10) << 1;
	gb->cpu.F |= ((n + gb->cpu.A) & 0x100) >> 4;
	gb->cpu.A += n;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo add_A_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 0);
	gb->cpu.F = (((n & 0xF) + (gb->cpu.A & 0xF)) & 0x10) << 1;
	gb->cpu.F |= ((n + gb->cpu.A) & 0x100) >> 4;
	gb->cpu.A += n;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

#define adc_A_x(x) \
struct instrTimingInfo adc_A_##x##(struct GB* gb) { \
	uint8_t c = (gb->cpu.F & 0x10) >> 4; \
	gb->cpu.F = (((gb->cpu.x & 0xF) + (gb->cpu.A & 0xF) + c) & 0x10) << 1; \
	gb->cpu.F |= ((gb->cpu.x + gb->cpu.A + c) & 0x100) >> 4; \
	gb->cpu.A += gb->cpu.x + c; \
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 4, 4 }; \
}
adc_A_x(B) adc_A_x(C) adc_A_x(D) adc_A_x(E) 
adc_A_x(H) adc_A_x(L)            adc_A_x(A)

struct instrTimingInfo adc_A_n(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.PC++, 0);
	uint8_t c = (gb->cpu.F & 0x10) >> 4;
	gb->cpu.F = (((n & 0xF) + (gb->cpu.A & 0xF) + c) & 0x10) << 1;
	gb->cpu.F |= ((n + gb->cpu.A + c) & 0x100) >> 4;
	gb->cpu.A += n + c;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo adc_A_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 0);
	uint8_t c = (gb->cpu.F & 0x10) >> 4;
	gb->cpu.F = (((n & 0xF) + (gb->cpu.A & 0xF) + c) & 0x10) << 1;
	gb->cpu.F |= ((n + gb->cpu.A + c) & 0x100) >> 4;
	gb->cpu.A += n + c;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

#define sub_A_x(x) \
struct instrTimingInfo sub_A_##x##(struct GB* gb) { \
	gb->cpu.F = 0x40; \
	gb->cpu.F |= (uint8_t)((gb->cpu.x & 0xF) > (gb->cpu.A & 0xF)) << flagBitH; \
	gb->cpu.F |= ((uint8_t)(gb->cpu.x > gb->cpu.A) << flagBitC); \
	gb->cpu.A -= gb->cpu.x; \
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 4, 4 }; \
}
sub_A_x(B) sub_A_x(C) sub_A_x(D) sub_A_x(E) 
sub_A_x(H) sub_A_x(L)            sub_A_x(A)

struct instrTimingInfo sub_A_n(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.PC++, 0);
	gb->cpu.F = 0x40;
	gb->cpu.F |= (uint8_t)((n & 0xF) > (gb->cpu.A & 0xF)) << flagBitH;
	gb->cpu.F |= ((uint8_t)(n > gb->cpu.A) << flagBitC);
	gb->cpu.A -= n;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo sub_A_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 0);
	gb->cpu.F = 0x40;
	gb->cpu.F |= (uint8_t)((n & 0xF) > (gb->cpu.A & 0xF)) << flagBitH;
	gb->cpu.F |= ((uint8_t)(n > gb->cpu.A) << flagBitC);
	gb->cpu.A -= n;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

#define sbc_A_x(x) \
struct instrTimingInfo sbc_A_##x##(struct GB* gb) { \
	uint8_t c = ((gb->cpu.F & 0x10) >> 4); \
	gb->cpu.F = 0x40; \
	gb->cpu.F |= (uint8_t)((gb->cpu.x & 0xF) + c > (gb->cpu.A & 0xF)) << flagBitH; \
	gb->cpu.F |= ((uint8_t)(((uint16_t)gb->cpu.x) + c > gb->cpu.A) << flagBitC); \
	gb->cpu.A -= gb->cpu.x + c; \
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 4, 4 }; \
}
sbc_A_x(B) sbc_A_x(C) sbc_A_x(D) sbc_A_x(E)
sbc_A_x(H) sbc_A_x(L)            sbc_A_x(A)

struct instrTimingInfo sbc_A_n(struct GB* gb) {
	uint16_t n = RB(gb, gb->cpu.PC++, 0);
	uint8_t c = ((gb->cpu.F & 0x10) >> 4);
	gb->cpu.F = 0x40;
	gb->cpu.F |= (uint8_t)(((n & 0xF) + c) > (gb->cpu.A & 0xF)) << flagBitH;
	gb->cpu.F |= ((uint8_t)((n + c) > gb->cpu.A) << flagBitC);
	gb->cpu.A -= n + c;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo sbc_A_HL(struct GB* gb) {
	uint16_t n = RB(gb, gb->cpu.HL, 0);
	uint8_t c = ((gb->cpu.F & 0x10) >> 4);
	gb->cpu.F = 0x40;
	gb->cpu.F |= (uint8_t)(((n & 0xF) + c) > (gb->cpu.A & 0xF)) << flagBitH;
	gb->cpu.F |= ((uint8_t)((n + c) > gb->cpu.A) << flagBitC);
	gb->cpu.A -= n + c;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

#define and_A_x(x) \
struct instrTimingInfo and_A_##x##(struct GB* gb) { \
	gb->cpu.F = 0x20; \
	gb->cpu.A &= gb->cpu.x; \
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 4, 4 }; \
}
and_A_x(B) and_A_x(C) and_A_x(D) and_A_x(E) 
and_A_x(H) and_A_x(L)            and_A_x(A)

struct instrTimingInfo and_A_n(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.PC++, 0);
	gb->cpu.F = 0x20;
	gb->cpu.A &= n;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo and_A_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 0);
	gb->cpu.F = 0x20;
	gb->cpu.A &= n;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

#define xor_A_x(x) \
struct instrTimingInfo xor_A_##x##(struct GB* gb) { \
	gb->cpu.F = 0x00; \
	gb->cpu.A ^= gb->cpu.x; \
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 4, 4 }; \
}
xor_A_x(B) xor_A_x(C) xor_A_x(D) xor_A_x(E)
xor_A_x(H) xor_A_x(L)            xor_A_x(A)

struct instrTimingInfo xor_A_n(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.PC++, 0);
	gb->cpu.F = 0x00;
	gb->cpu.A ^= n;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo xor_A_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 0);
	gb->cpu.F = 0x00;
	gb->cpu.A ^= n;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

#define or_A_x(x) \
struct instrTimingInfo or_A_##x##(struct GB* gb) { \
	gb->cpu.F = 0x00; \
	gb->cpu.A |= gb->cpu.x; \
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 4, 4 }; \
}
or_A_x(B) or_A_x(C) or_A_x(D) or_A_x(E)
or_A_x(H) or_A_x(L)           or_A_x(A)

struct instrTimingInfo or_A_n(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.PC++, 0);
	gb->cpu.F = 0x00;
	gb->cpu.A |= n;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo or_A_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 0);
	gb->cpu.F = 0x00;
	gb->cpu.A |= n;
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

#define cp_A_x(x) \
struct instrTimingInfo cp_A_##x##(struct GB* gb) { \
	gb->cpu.F = 0x40; \
	gb->cpu.F |= (uint8_t)((gb->cpu.x & 0xF) > (gb->cpu.A & 0xF)) << flagBitH; \
	gb->cpu.F |= ((uint8_t)(gb->cpu.x > gb->cpu.A) << flagBitC); \
	gb->cpu.F |= (uint8_t)(gb->cpu.A - gb->cpu.x == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 4, 4 }; \
}
cp_A_x(B) cp_A_x(C) cp_A_x(D) cp_A_x(E)
cp_A_x(H) cp_A_x(L)           cp_A_x(A)

struct instrTimingInfo cp_A_n(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.PC++, 0);
	gb->cpu.F = 0x40;
	gb->cpu.F |= (uint8_t)((n & 0xF) > (gb->cpu.A & 0xF)) << flagBitH;
	gb->cpu.F |= ((uint8_t)(n > gb->cpu.A) << flagBitC);
	gb->cpu.F |= (uint8_t)(gb->cpu.A - n == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

struct instrTimingInfo cp_A_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 0);
	gb->cpu.F = 0x40;
	gb->cpu.F |= (uint8_t)((n & 0xF) > (gb->cpu.A & 0xF)) << flagBitH;
	gb->cpu.F |= ((uint8_t)(n > gb->cpu.A) << flagBitC);
	gb->cpu.F |= (uint8_t)(gb->cpu.A - n == 0) << flagBitZ;
	return (struct instrTimingInfo) { 8, 8 };
}

#define inc_A_x(x) \
struct instrTimingInfo inc_A_##x##(struct GB* gb) { \
	gb->cpu.F &= ~(0b11100000); \
	gb->cpu.F |= (((gb->cpu.x & 0xF) + 1) & 0x10) << 1; \
	gb->cpu.x++; \
	gb->cpu.F |= (uint8_t)(gb->cpu.x == 0) << 7; \
	return (struct instrTimingInfo) { 4, 4 }; \
}
inc_A_x(B) inc_A_x(C) inc_A_x(D) inc_A_x(E)
inc_A_x(H) inc_A_x(L)            inc_A_x(A)

struct instrTimingInfo inc_A_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 0);
	gb->cpu.F &= ~(0b11100000);
	gb->cpu.F |= (((n & 0xF) + 1) & 0x10) << 1;
	gb->cpu.F |= (uint8_t)((uint8_t)(n + 1) == 0) << 7;
	WB(gb, gb->cpu.HL, n + 1, 4);
	return (struct instrTimingInfo) { 12, 8 };
}

#define dec_A_x(x) \
struct instrTimingInfo dec_A_##x##(struct GB* gb) { \
	gb->cpu.F &= ~(0b11100000); \
	gb->cpu.F |= (uint8_t)((gb->cpu.x & 0xF) < 1) << flagBitH; \
	gb->cpu.F |= ((uint8_t)((gb->cpu.x - 1) == 0) << flagBitZ) | (1 << flagBitN); \
	gb->cpu.x--; \
	return (struct instrTimingInfo) { 4, 4 }; \
}
dec_A_x(B) dec_A_x(C) dec_A_x(D) dec_A_x(E)
dec_A_x(H) dec_A_x(L)            dec_A_x(A)

struct instrTimingInfo dec_A_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 0);
	gb->cpu.F &= ~(0b11100000);
	gb->cpu.F |= (uint8_t)((n & 0xF) < 1) << flagBitH;
	gb->cpu.F |= ((uint8_t)((n - 1) == 0) << flagBitZ) | (1 << flagBitN);
	WB(gb, gb->cpu.HL, n - 1, 4);
	return (struct instrTimingInfo) { 12, 8 };
}

struct instrTimingInfo daa(struct GB* gb) {
	if ((gb->cpu.F & 0x40) == 0) {
		if ((gb->cpu.F & 0x10) != 0 || gb->cpu.A > 0x99) {
			gb->cpu.A += 0x60;
			gb->cpu.F |= (1 << flagBitC);
		}
		if ((gb->cpu.F & 0x20) != 0 || (gb->cpu.A & 0x0F) > 0x9) {
			gb->cpu.A += 0x6;
		}
	} else {
		if ((gb->cpu.F & 0x10) != 0) gb->cpu.A -= 0x60;
		if ((gb->cpu.F & 0x20) != 0) gb->cpu.A -= 0x06;
	}
	gb->cpu.F &= ~(0b10100000);
	gb->cpu.F |= (uint8_t)(gb->cpu.A == 0) << flagBitZ;
	return (struct instrTimingInfo) { 4, 4 };
}

struct instrTimingInfo cpl(struct GB* gb) {
	gb->cpu.A = ~gb->cpu.A;
	gb->cpu.F |= 0b01100000;
	return (struct instrTimingInfo) { 4, 4 };
}

/*
		16 bit arithmetic and logic
*/
#define add_HL_xx(xx) \
struct instrTimingInfo add_HL_##xx##(struct GB* gb) { \
	gb->cpu.F &= ~(0b01110000); \
	gb->cpu.F |= ((((uint32_t)gb->cpu.xx) + ((uint32_t)gb->cpu.HL)) & 0x10000) >> 12; \
	gb->cpu.F |= (((gb->cpu.xx & 0xFFF) + (gb->cpu.HL & 0xFFF)) & 0x1000) >> 7; \
	gb->cpu.HL += gb->cpu.xx; \
	return (struct instrTimingInfo) { 8, 8 }; \
}
add_HL_xx(BC) add_HL_xx(DE) add_HL_xx(HL) add_HL_xx(SP)

#define inc_xx(xx) \
struct instrTimingInfo inc_##xx##(struct GB* gb) { \
	gb->cpu.xx++; \
	return (struct instrTimingInfo) { 8, 8 }; \
}
inc_xx(BC) inc_xx(DE) inc_xx(HL) inc_xx(SP)

#define dec_xx(xx) \
struct instrTimingInfo dec_##xx##(struct GB* gb) { \
	gb->cpu.xx--; \
	return (struct instrTimingInfo) { 8, 8 }; \
}
dec_xx(BC) dec_xx(DE) dec_xx(HL) dec_xx(SP)

struct instrTimingInfo add_SP_dd(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.PC++, 0);
	uint16_t nn = ((n & 0x80) != 0) ? (0xFF00 | n) : n;
	gb->cpu.F = (((nn & 0xF) + (gb->cpu.SP & 0xF)) & 0x10) << 1;
	gb->cpu.F |= (((nn & 0xFF) + (gb->cpu.SP & 0xFF)) & 0x100) >> 4;
	gb->cpu.SP += nn;
	return (struct instrTimingInfo) { 16, 16 };
}

struct instrTimingInfo ld_HP_SP_dd(struct GB* gb) {
	uint8_t e = RB(gb, gb->cpu.PC++, 0);
	uint16_t nn = ((e & 0x80) != 0) ? (0xFF00 | e) : e;
	gb->cpu.F = 0x00;
	gb->cpu.F |= (((nn & 0xF) + (gb->cpu.SP & 0xF)) & 0x10) << 1;
	gb->cpu.F |= (((nn & 0xFF) + (gb->cpu.SP & 0xFF)) & 0x100) >> 4;
	gb->cpu.HL = gb->cpu.SP + nn;
	return (struct instrTimingInfo) { 12, 12 };
}

/*
		Rotate and Shift Instructions
*/
struct instrTimingInfo rlca(struct GB* gb) {
	gb->cpu.F = (gb->cpu.A & 0x80) >> 3;
	gb->cpu.A = (gb->cpu.A << 1) | ((gb->cpu.F & 0x10) >> 4);
	return (struct instrTimingInfo) { 4, 4 };
}

struct instrTimingInfo rla(struct GB* gb) {
	uint8_t c = gb->cpu.F;
	gb->cpu.F = (gb->cpu.A & 0x80) >> 3;
	gb->cpu.A = (gb->cpu.A << 1) | ((c & 0x10) >> 4);
	return (struct instrTimingInfo) { 4, 4 };
}

struct instrTimingInfo rrca(struct GB* gb) {
	gb->cpu.F = (gb->cpu.A & 0x1) << 4;
	gb->cpu.A = (gb->cpu.A >> 1) | ((gb->cpu.F & 0x10) << 3);
	return (struct instrTimingInfo) { 4, 4 };
}

struct instrTimingInfo rra(struct GB* gb) {
	uint8_t c = gb->cpu.F;
	gb->cpu.F = (gb->cpu.A & 0x1) << 4;
	gb->cpu.A = (gb->cpu.A >> 1) | ((c & 0x10) << 3);
	return (struct instrTimingInfo) { 4, 4 };
}

#define rlc_x(x) \
struct instrTimingInfo rlc_##x##(struct GB* gb) { \
	gb->cpu.F = (gb->cpu.x & 0x80) >> 3; \
	gb->cpu.x = (gb->cpu.x << 1) | ((gb->cpu.F & 0x10) >> 4); \
	gb->cpu.F |= (uint8_t)(gb->cpu.x == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 8, 8 }; \
}
rlc_x(B) rlc_x(C) rlc_x(D) rlc_x(E) 
rlc_x(H) rlc_x(L)          rlc_x(A)

struct instrTimingInfo rlc_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 4);
	gb->cpu.F = 0x00;
	gb->cpu.F |= (n & 0x80) >> 3;
	n = (n << 1) | ((gb->cpu.F & 0x10) >> 4);
	gb->cpu.F |= (uint8_t)(n == 0) << flagBitZ;
	WB(gb, gb->cpu.HL, n, 8);
	return (struct instrTimingInfo) { 16, 8 };
}

#define rl_x(x) \
struct instrTimingInfo rl_##x##(struct GB* gb) { \
	uint8_t c = gb->cpu.F; \
	gb->cpu.F = (gb->cpu.x & 0x80) >> 3; \
	gb->cpu.x = (gb->cpu.x << 1) | ((c & 0x10) >> 4); \
	gb->cpu.F |= (uint8_t)(gb->cpu.x == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 8, 8 }; \
}
rl_x(B) rl_x(C) rl_x(D) rl_x(E)
rl_x(H) rl_x(L)         rl_x(A)

struct instrTimingInfo rl_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 4);
	uint8_t c = gb->cpu.F;
	gb->cpu.F = 0x00;
	gb->cpu.F |= (n & 0x80) >> 3;
	n = (n << 1) | ((c & 0x10) >> 4);
	gb->cpu.F |= (uint8_t)(n == 0) << flagBitZ;
	WB(gb, gb->cpu.HL, n, 8);
	return (struct instrTimingInfo) { 16, 8 };
}

#define rrc_x(x) \
struct instrTimingInfo rrc_##x##(struct GB* gb) { \
	gb->cpu.F = (gb->cpu.x & 0x1) << 4; \
	gb->cpu.x = (gb->cpu.x >> 1) | ((gb->cpu.F & 0x10) << 3); \
	gb->cpu.F |= (uint8_t)(gb->cpu.x == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 8, 8 }; \
}
rrc_x(B) rrc_x(C) rrc_x(D) rrc_x(E)
rrc_x(H) rrc_x(L)          rrc_x(A)

struct instrTimingInfo rrc_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 4);
	gb->cpu.F = (n & 0x1) << 4;
	n = (n >> 1) | ((gb->cpu.F & 0x10) << 3);
	gb->cpu.F |= (uint8_t)(n == 0) << flagBitZ;
	WB(gb, gb->cpu.HL, n, 8);
	return (struct instrTimingInfo) { 16, 8 };
}

#define rr_x(x) \
struct instrTimingInfo rr_##x##(struct GB* gb) { \
	uint8_t c = gb->cpu.F; \
	gb->cpu.F = (gb->cpu.x & 0x1) << 4; \
	gb->cpu.x = (gb->cpu.x >> 1) | ((c & 0x10) << 3); \
	gb->cpu.F |= (uint8_t)(gb->cpu.x == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 8, 8 }; \
}
rr_x(B) rr_x(C) rr_x(D) rr_x(E) 
rr_x(H) rr_x(L)         rr_x(A)

struct instrTimingInfo rr_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 4);
	uint8_t c = gb->cpu.F;
	gb->cpu.F = 0x00;
	gb->cpu.F |= (n & 0x1) << 4;
	n = (n >> 1) | ((c & 0x10) << 3);
	gb->cpu.F |= (uint8_t)(n == 0) << flagBitZ;
	WB(gb, gb->cpu.HL, n, 8);
	return (struct instrTimingInfo) { 16, 8 };
}

#define sla_x(x) \
struct instrTimingInfo sla_##x##(struct GB* gb) { \
	gb->cpu.F = (gb->cpu.x & 0x80) >> 3; \
	gb->cpu.x = gb->cpu.x << 1; \
	gb->cpu.F |= (uint8_t)(gb->cpu.x == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 8, 8 }; \
}
sla_x(B) sla_x(C) sla_x(D) sla_x(E)
sla_x(H) sla_x(L)          sla_x(A)

struct instrTimingInfo sla_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 4);
	gb->cpu.F = 0x00;
	gb->cpu.F |= (n & 0x80) >> 3;
	n = n << 1;
	gb->cpu.F |= (uint8_t)(n == 0) << flagBitZ;
	WB(gb, gb->cpu.HL, n, 8);
	return (struct instrTimingInfo) { 16, 8 };
}

#define swap_x(x) \
struct instrTimingInfo swap_##x##(struct GB* gb) { \
	gb->cpu.F = 0x00; \
	gb->cpu.x = ((gb->cpu.x & 0xF0) >> 4) | ((gb->cpu.x & 0x0F) << 4); \
	gb->cpu.F |= (uint8_t)(gb->cpu.x == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 8, 8 }; \
}
swap_x(B) swap_x(C) swap_x(D) swap_x(E)
swap_x(H) swap_x(L)           swap_x(A)

struct instrTimingInfo swap_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 4);
	gb->cpu.F = 0x00;
	n = ((n & 0xF0) >> 4) | ((n & 0x0F) << 4);
	gb->cpu.F |= (uint8_t)(n == 0) << flagBitZ;
	WB(gb, gb->cpu.HL, n, 8);
	return (struct instrTimingInfo) { 16, 8 };
}

#define sra_x(x) \
struct instrTimingInfo sra_##x##(struct GB* gb) { \
	gb->cpu.F = (gb->cpu.x & 0x1) << flagBitC; \
	gb->cpu.x = (gb->cpu.x & 0x80) | (gb->cpu.x >> 1); \
	gb->cpu.F |= (uint8_t)(gb->cpu.x == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 8, 8 }; \
}
sra_x(B) sra_x(C) sra_x(D) sra_x(E)
sra_x(H) sra_x(L)          sra_x(A)

struct instrTimingInfo sra_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 4);
	gb->cpu.F = (n & 0x1) << flagBitC;
	n = (n & 0x80) | (n >> 1);
	gb->cpu.F |= (uint8_t)(n == 0) << flagBitZ;
	WB(gb, gb->cpu.HL, n, 8);
	return (struct instrTimingInfo) { 16, 8 };
}

#define srl_x(x) \
struct instrTimingInfo srl_##x##(struct GB* gb) { \
	gb->cpu.F = (gb->cpu.x & 0x1) << flagBitC; \
	gb->cpu.x = gb->cpu.x >> 1; \
	gb->cpu.F |= (uint8_t)(gb->cpu.x == 0) << flagBitZ; \
	return (struct instrTimingInfo) { 8, 8 }; \
}
srl_x(B) srl_x(C) srl_x(D) srl_x(E)
srl_x(H) srl_x(L)          srl_x(A)

struct instrTimingInfo srl_HL(struct GB* gb) {
	uint8_t n = RB(gb, gb->cpu.HL, 4);
	gb->cpu.F = (n & 0x1) << flagBitC;
	n = n >> 1;
	gb->cpu.F |= (uint8_t)(n == 0) << flagBitZ;
	WB(gb, gb->cpu.HL, n, 8);
	return (struct instrTimingInfo) { 16, 8 };
}

#define bit_n_x(n, x) \
struct instrTimingInfo bit_##n##_##x##(struct GB* gb) { \
	gb->cpu.F &= ~(0b11100000); \
	gb->cpu.F |= (((~gb->cpu.x) & (1 << n)) << (7 - n)) | (1 << 5); \
	return (struct instrTimingInfo) { 8, 8 }; \
}
bit_n_x(0, B) bit_n_x(1, B) bit_n_x(2, B) bit_n_x(3, B)	bit_n_x(4, B) bit_n_x(5, B) bit_n_x(6, B) bit_n_x(7, B)
bit_n_x(0, C) bit_n_x(1, C) bit_n_x(2, C) bit_n_x(3, C)	bit_n_x(4, C) bit_n_x(5, C)	bit_n_x(6, C) bit_n_x(7, C)
bit_n_x(0, D) bit_n_x(1, D) bit_n_x(2, D) bit_n_x(3, D)	bit_n_x(4, D) bit_n_x(5, D)	bit_n_x(6, D) bit_n_x(7, D)
bit_n_x(0, E) bit_n_x(1, E) bit_n_x(2, E) bit_n_x(3, E)	bit_n_x(4, E) bit_n_x(5, E)	bit_n_x(6, E) bit_n_x(7, E)
bit_n_x(0, H) bit_n_x(1, H) bit_n_x(2, H) bit_n_x(3, H)	bit_n_x(4, H) bit_n_x(5, H)	bit_n_x(6, H) bit_n_x(7, H)
bit_n_x(0, L) bit_n_x(1, L) bit_n_x(2, L) bit_n_x(3, L)	bit_n_x(4, L) bit_n_x(5, L)	bit_n_x(6, L) bit_n_x(7, L)
bit_n_x(0, A) bit_n_x(1, A) bit_n_x(2, A) bit_n_x(3, A)	bit_n_x(4, A) bit_n_x(5, A)	bit_n_x(6, A) bit_n_x(7, A)

#define bit_n_HL(n) \
struct instrTimingInfo bit_##n##_HL(struct GB* gb) { \
	uint8_t u8 = RB(gb, gb->cpu.HL, 4); \
	gb->cpu.F &= ~(0b11100000); \
	gb->cpu.F |= (((~u8) & (1 << n)) << (7 - n)) | (1 << 5); \
	return (struct instrTimingInfo) { 12, 12 }; \
}
bit_n_HL(0) bit_n_HL(1) bit_n_HL(2) bit_n_HL(3) 
bit_n_HL(4) bit_n_HL(5) bit_n_HL(6) bit_n_HL(7)

#define set_n_x(n, x) \
struct instrTimingInfo set_##n##_##x##(struct GB* gb) { \
	gb->cpu.x |= (1 << n); \
	return (struct instrTimingInfo) { 8, 8 }; \
}
set_n_x(0, B) set_n_x(1, B) set_n_x(2, B) set_n_x(3, B)	set_n_x(4, B) set_n_x(5, B) set_n_x(6, B) set_n_x(7, B)
set_n_x(0, C) set_n_x(1, C) set_n_x(2, C) set_n_x(3, C)	set_n_x(4, C) set_n_x(5, C)	set_n_x(6, C) set_n_x(7, C)
set_n_x(0, D) set_n_x(1, D) set_n_x(2, D) set_n_x(3, D)	set_n_x(4, D) set_n_x(5, D)	set_n_x(6, D) set_n_x(7, D)
set_n_x(0, E) set_n_x(1, E) set_n_x(2, E) set_n_x(3, E)	set_n_x(4, E) set_n_x(5, E)	set_n_x(6, E) set_n_x(7, E)
set_n_x(0, H) set_n_x(1, H) set_n_x(2, H) set_n_x(3, H)	set_n_x(4, H) set_n_x(5, H)	set_n_x(6, H) set_n_x(7, H)
set_n_x(0, L) set_n_x(1, L) set_n_x(2, L) set_n_x(3, L)	set_n_x(4, L) set_n_x(5, L)	set_n_x(6, L) set_n_x(7, L)
set_n_x(0, A) set_n_x(1, A) set_n_x(2, A) set_n_x(3, A)	set_n_x(4, A) set_n_x(5, A)	set_n_x(6, A) set_n_x(7, A)

#define set_n_HL(n) \
struct instrTimingInfo set_##n##_HL(struct GB* gb) { \
	uint8_t u8 = RB(gb, gb->cpu.HL, 4); \
	u8 |= (1 << n); \
	WB(gb, gb->cpu.HL, u8, 8); \
	return (struct instrTimingInfo) { 16, 8 }; \
}
set_n_HL(0) set_n_HL(1) set_n_HL(2) set_n_HL(3)
set_n_HL(4) set_n_HL(5) set_n_HL(6) set_n_HL(7)

#define res_n_x(n, x) \
struct instrTimingInfo res_##n##_##x##(struct GB* gb) { \
	gb->cpu.x &= ~(1 << n); \
	return (struct instrTimingInfo) { 8, 8 }; \
}
res_n_x(0, B) res_n_x(1, B) res_n_x(2, B) res_n_x(3, B)	res_n_x(4, B) res_n_x(5, B) res_n_x(6, B) res_n_x(7, B)
res_n_x(0, C) res_n_x(1, C) res_n_x(2, C) res_n_x(3, C)	res_n_x(4, C) res_n_x(5, C)	res_n_x(6, C) res_n_x(7, C)
res_n_x(0, D) res_n_x(1, D) res_n_x(2, D) res_n_x(3, D)	res_n_x(4, D) res_n_x(5, D)	res_n_x(6, D) res_n_x(7, D)
res_n_x(0, E) res_n_x(1, E) res_n_x(2, E) res_n_x(3, E)	res_n_x(4, E) res_n_x(5, E)	res_n_x(6, E) res_n_x(7, E)
res_n_x(0, H) res_n_x(1, H) res_n_x(2, H) res_n_x(3, H)	res_n_x(4, H) res_n_x(5, H)	res_n_x(6, H) res_n_x(7, H)
res_n_x(0, L) res_n_x(1, L) res_n_x(2, L) res_n_x(3, L)	res_n_x(4, L) res_n_x(5, L)	res_n_x(6, L) res_n_x(7, L)
res_n_x(0, A) res_n_x(1, A) res_n_x(2, A) res_n_x(3, A)	res_n_x(4, A) res_n_x(5, A)	res_n_x(6, A) res_n_x(7, A)

#define res_n_HL(n) \
struct instrTimingInfo res_##n##_HL(struct GB* gb) { \
	uint8_t u8 = RB(gb, gb->cpu.HL, 4); \
	u8 &= ~(1 << n); \
	WB(gb, gb->cpu.HL, u8, 8); \
	return (struct instrTimingInfo) { 16, 8 }; \
}
res_n_HL(0) res_n_HL(1) res_n_HL(2) res_n_HL(3)
res_n_HL(4) res_n_HL(5) res_n_HL(6) res_n_HL(7)

/*
		CPU control instructions
*/
struct instrTimingInfo ccf(struct GB* gb) {
	gb->cpu.F ^= (1 << flagBitC);
	gb->cpu.F &= ~(0b01100000);
	return (struct instrTimingInfo) { 4, 4 };
}

struct instrTimingInfo scf(struct GB* gb) {
	gb->cpu.F &= ~(0b01110000);
	gb->cpu.F |= 0b00010000;
	return (struct instrTimingInfo) { 4, 4 };
}

struct instrTimingInfo nop(struct GB* gb) {
	/* Literally do nothing for 4 clock cycles */
	return (struct instrTimingInfo) { 4, 4 };
}

struct instrTimingInfo halt(struct GB* gb) {
	gb->cpu.halt = 1;
	return (struct instrTimingInfo) { 4, 4 };
}

struct instrTimingInfo stop(struct GB* gb) {
	// TODO
	return (struct instrTimingInfo) { 4, 4 };
}

struct instrTimingInfo di(struct GB* gb) {
	gb->cpu.IME_scheduler |= 0;
	gb->cpu.IME = 0;
	return (struct instrTimingInfo) { 4, 4 };
}

struct instrTimingInfo ei(struct GB* gb) {
	gb->cpu.IME_scheduler |= 0x8000;
	return (struct instrTimingInfo) { 4, 4 };
}

/*
		Control flow instructions
*/
struct instrTimingInfo jp_nn(struct GB* gb) {
	uint16_t nn = RB(gb, gb->cpu.PC++, 0);
	nn |= RB(gb, gb->cpu.PC++, 0) << 8;
	gb->cpu.PC = nn;
	return (struct instrTimingInfo) { 16, 16 };
}

struct instrTimingInfo jp_HL(struct GB* gb) {
	gb->cpu.PC = gb->cpu.HL;
	return (struct instrTimingInfo) { 4, 4 };
}

#define jp_f_nn(cond, mask, val) \
struct instrTimingInfo jp_##cond##_nn(struct GB* gb) { \
	if ((gb->cpu.F & mask) == val) { \
		uint16_t nn = RB(gb, gb->cpu.PC++, 0);  \
		nn |= RB(gb, gb->cpu.PC++, 0) << 8; \
		gb->cpu.PC = nn; \
		return (struct instrTimingInfo) { 16, 16 }; \
	} else { \
		gb->cpu.PC += 2; \
		return (struct instrTimingInfo) { 12, 12 }; \
	} \
}
jp_f_nn(NZ, 0x80, 0x00) jp_f_nn(NC, 0x10, 0x00) 
jp_f_nn(Z,  0x80, 0x80) jp_f_nn(C,  0x10, 0x10)

struct instrTimingInfo jp_PC_dd(struct GB* gb) {
	int8_t e = RB(gb, gb->cpu.PC++, 0);
	gb->cpu.PC += (int16_t)e;
	return (struct instrTimingInfo) { 12, 12 };
}

#define jp_f_PC_dd(cond, mask, val) \
struct instrTimingInfo jp_##cond##_PC_dd(struct GB* gb) { \
	if ((gb->cpu.F & mask) == val) { \
		int8_t e = RB(gb, gb->cpu.PC++, 0); \
		gb->cpu.PC += (int16_t)e; \
		return (struct instrTimingInfo) { 12, 12 }; \
	} else { \
		gb->cpu.PC++; \
		return (struct instrTimingInfo) { 8, 8 }; \
	} \
}
jp_f_PC_dd(NZ, 0x80, 0x00) jp_f_PC_dd(NC, 0x10, 0x00)
 jp_f_PC_dd(Z, 0x80, 0x80)  jp_f_PC_dd(C, 0x10, 0x10)

struct instrTimingInfo call_nn(struct GB* gb) {
	uint16_t nn = RB(gb, gb->cpu.PC++, 0);
	nn |= RB(gb, gb->cpu.PC++, 0) << 8;
	WB(gb, --gb->cpu.SP, (uint8_t)(gb->cpu.PC >> 8), 0);
	WB(gb, --gb->cpu.SP, (uint8_t)(gb->cpu.PC & 0xFF), 0);
	gb->cpu.PC = nn;
	return (struct instrTimingInfo) { 24, 24 };
}

#define call_f_nn(cond, mask, val) \
struct instrTimingInfo call_##cond##_nn(struct GB* gb) { \
	if ((gb->cpu.F & mask) == val) { \
		uint16_t nn = RB(gb, gb->cpu.PC++, 0); \
		nn |= RB(gb, gb->cpu.PC++, 0) << 8; \
		WB(gb, --gb->cpu.SP, (uint8_t)(gb->cpu.PC >> 8), 0); \
		WB(gb, --gb->cpu.SP, (uint8_t)(gb->cpu.PC & 0xFF), 0); \
		gb->cpu.PC = nn; \
		return (struct instrTimingInfo) { 24, 24 }; \
	} else { \
		gb->cpu.PC += 2; \
		return (struct instrTimingInfo) { 12, 12 }; \
	} \
}

call_f_nn(NZ, 0x80, 0x00) call_f_nn(NC, 0x10, 0x00)
 call_f_nn(Z, 0x80, 0x80)  call_f_nn(C, 0x10, 0x10)

struct instrTimingInfo ret(struct GB* gb) {
	gb->cpu.PC = RB(gb, gb->cpu.SP++, 0);
	gb->cpu.PC |= RB(gb, gb->cpu.SP++, 0) << 8;
	return (struct instrTimingInfo) { 16, 16 };
}

#define ret_f(cond, mask, val) \
struct instrTimingInfo ret_##cond##(struct GB* gb) { \
	if ((gb->cpu.F & mask) == val) { \
		gb->cpu.PC = RB(gb, gb->cpu.SP++, 0); \
		gb->cpu.PC |= RB(gb, gb->cpu.SP++, 0) << 8; \
		return (struct instrTimingInfo) { 20, 20 }; \
	} else { \
		return (struct instrTimingInfo) { 8, 8 }; \
	} \
}

ret_f(NZ, 0x80, 0x00) ret_f(NC, 0x10, 0x00)
 ret_f(Z, 0x80, 0x80)  ret_f(C, 0x10, 0x10)

struct instrTimingInfo reti(struct GB* gb) {
	gb->cpu.IME_scheduler |= 0x8000;
	gb->cpu.PC = RB(gb, gb->cpu.SP++, 0);
	gb->cpu.PC |= RB(gb, gb->cpu.SP++, 0) << 8;
	return (struct instrTimingInfo) { 16, 16 };
}

#define rst_n(n) \
struct instrTimingInfo rst_##n##(struct GB* gb) { \
	WB(gb, --gb->cpu.SP, (uint8_t)(gb->cpu.PC >> 8), 0); \
	WB(gb, --gb->cpu.SP, (uint8_t)(gb->cpu.PC & 0xFF), 0); \
	gb->cpu.PC = n; \
	return (struct instrTimingInfo) { 16, 16 }; \
}
rst_n(0x00) rst_n(0x08) rst_n(0x10) rst_n(0x18)
rst_n(0x20) rst_n(0x28) rst_n(0x30) rst_n(0x38)

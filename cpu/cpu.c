#include <stdio.h>
#include "cpu.h"
#include "mem.h"
#include "instr.h"

// Interrupt bit defines
#define INTBIT_VBLANK	0
#define INTBIT_STAT		1
#define INTBIT_TIMER	2
#define INTBIT_SERIAL	3
#define INTBIT_JOYPAD	4

// Interrupt bit mask defines
#define INTMASK_VBLANK	0b00000001
#define INTMASK_STAT	0b00000010
#define INTMASK_TIMER	0b00000100
#define INTMASK_SERIAL	0b00001000
#define INTMASK_JOYPAD	0b00010000

void cpu_init(struct sharp_SM83* cpu_ptr)
{
	// Initialize interrupt stuff
	cpu_ptr->IME = 0;
	cpu_ptr->IME_scheduler = 0;
	cpu_ptr->halt = 0;

	// Initialize registers to boot vals
	cpu_ptr->AF = 0x0000;
	cpu_ptr->BC = 0x0000;
	cpu_ptr->DE = 0x0000;
	cpu_ptr->HL = 0x0000;
	cpu_ptr->SP = 0x0000;
	cpu_ptr->PC = 0x0000;
}

// To address the extended CB opcode table, which is basically an entirely seperate
//		opcode table that can be used by using a CB opcode
struct instrTimingInfo goto_cb_prefix_table(struct GB* gb) 
{
	// Extended opcode and operand lookup tables
	static const struct instrTimingInfo(*instr_lookup[256])(struct GB* gb) = {
		/*0x00*/ &rlc_B,
		/*0x01*/ &rlc_C,
		/*0x02*/ &rlc_D,
		/*0x03*/ &rlc_E,
		/*0x04*/ &rlc_H,
		/*0x05*/ &rlc_L,
		/*0x06*/ &rlc_HL,
		/*0x07*/ &rlc_A,
		/*0x08*/ &rrc_B,
		/*0x09*/ &rrc_C,
		/*0x0A*/ &rrc_D,
		/*0x0B*/ &rrc_E,
		/*0x0C*/ &rrc_H,
		/*0x0D*/ &rrc_L,
		/*0x0E*/ &rrc_HL,
		/*0x0F*/ &rrc_A,

		/*0x10*/ &rl_B,
		/*0x11*/ &rl_C,
		/*0x12*/ &rl_D,
		/*0x13*/ &rl_E,
		/*0x14*/ &rl_H,
		/*0x15*/ &rl_L,
		/*0x16*/ &rl_HL,
		/*0x17*/ &rl_A,
		/*0x18*/ &rr_B,
		/*0x19*/ &rr_C,
		/*0x1A*/ &rr_D,
		/*0x1B*/ &rr_E,
		/*0x1C*/ &rr_H,
		/*0x1D*/ &rr_L,
		/*0x1E*/ &rr_HL,
		/*0x1F*/ &rr_A,

		/*0x20*/ &sla_B,
		/*0x21*/ &sla_C,
		/*0x22*/ &sla_D,
		/*0x23*/ &sla_E,
		/*0x24*/ &sla_H,
		/*0x25*/ &sla_L,
		/*0x26*/ &sla_HL,
		/*0x27*/ &sla_A,
		/*0x28*/ &sra_B,
		/*0x29*/ &sra_C,
		/*0x2A*/ &sra_D,
		/*0x2B*/ &sra_E,
		/*0x2C*/ &sra_H,
		/*0x2D*/ &sra_L,
		/*0x2E*/ &sra_HL,
		/*0x2F*/ &sra_A,

		/*0x30*/ &swap_B,
		/*0x31*/ &swap_C,
		/*0x32*/ &swap_D,
		/*0x33*/ &swap_E,
		/*0x34*/ &swap_H,
		/*0x35*/ &swap_L,
		/*0x36*/ &swap_HL,
		/*0x37*/ &swap_A,
		/*0x38*/ &srl_B,
		/*0x39*/ &srl_C,
		/*0x3A*/ &srl_D,
		/*0x3B*/ &srl_E,
		/*0x3C*/ &srl_H,
		/*0x3D*/ &srl_L,
		/*0x3E*/ &srl_HL,
		/*0x3F*/ &srl_A,

		/*0x40*/ &bit_0_B,
		/*0x41*/ &bit_0_C,
		/*0x42*/ &bit_0_D,
		/*0x43*/ &bit_0_E,
		/*0x44*/ &bit_0_H,
		/*0x45*/ &bit_0_L,
		/*0x46*/ &bit_0_HL,
		/*0x47*/ &bit_0_A,
		/*0x48*/ &bit_1_B,
		/*0x49*/ &bit_1_C,
		/*0x4A*/ &bit_1_D,
		/*0x4B*/ &bit_1_E,
		/*0x4C*/ &bit_1_H,
		/*0x4D*/ &bit_1_L,
		/*0x4E*/ &bit_1_HL,
		/*0x4F*/ &bit_1_A,

		/*0x50*/ &bit_2_B,
		/*0x51*/ &bit_2_C,
		/*0x52*/ &bit_2_D,
		/*0x53*/ &bit_2_E,
		/*0x54*/ &bit_2_H,
		/*0x55*/ &bit_2_L,
		/*0x56*/ &bit_2_HL,
		/*0x57*/ &bit_2_A,
		/*0x58*/ &bit_3_B,
		/*0x59*/ &bit_3_C,
		/*0x5A*/ &bit_3_D,
		/*0x5B*/ &bit_3_E,
		/*0x5C*/ &bit_3_H,
		/*0x5D*/ &bit_3_L,
		/*0x5E*/ &bit_3_HL,
		/*0x5F*/ &bit_3_A,

		/*0x60*/ &bit_4_B,
		/*0x61*/ &bit_4_C,
		/*0x62*/ &bit_4_D,
		/*0x63*/ &bit_4_E,
		/*0x64*/ &bit_4_H,
		/*0x65*/ &bit_4_L,
		/*0x66*/ &bit_4_HL,
		/*0x67*/ &bit_4_A,
		/*0x68*/ &bit_5_B,
		/*0x69*/ &bit_5_C,
		/*0x6A*/ &bit_5_D,
		/*0x6B*/ &bit_5_E,
		/*0x6C*/ &bit_5_H,
		/*0x6D*/ &bit_5_L,
		/*0x6E*/ &bit_5_HL,
		/*0x6F*/ &bit_5_A,

		/*0x70*/ &bit_6_B,
		/*0x71*/ &bit_6_C,
		/*0x72*/ &bit_6_D,
		/*0x73*/ &bit_6_E,
		/*0x74*/ &bit_6_H,
		/*0x75*/ &bit_6_L,
		/*0x76*/ &bit_6_HL,
		/*0x77*/ &bit_6_A,
		/*0x78*/ &bit_7_B,
		/*0x79*/ &bit_7_C,
		/*0x7A*/ &bit_7_D,
		/*0x7B*/ &bit_7_E,
		/*0x7C*/ &bit_7_H,
		/*0x7D*/ &bit_7_L,
		/*0x7E*/ &bit_7_HL,
		/*0x7F*/ &bit_7_A,

		/*0x80*/ &res_0_B,
		/*0x81*/ &res_0_C,
		/*0x82*/ &res_0_D,
		/*0x83*/ &res_0_E,
		/*0x84*/ &res_0_H,
		/*0x85*/ &res_0_L,
		/*0x86*/ &res_0_HL,
		/*0x87*/ &res_0_A,
		/*0x88*/ &res_1_B,
		/*0x89*/ &res_1_C,
		/*0x8A*/ &res_1_D,
		/*0x8B*/ &res_1_E,
		/*0x8C*/ &res_1_H,
		/*0x8D*/ &res_1_L,
		/*0x8E*/ &res_1_HL,
		/*0x8F*/ &res_1_A,

		/*0x90*/ &res_2_B,
		/*0x91*/ &res_2_C,
		/*0x92*/ &res_2_D,
		/*0x93*/ &res_2_E,
		/*0x94*/ &res_2_H,
		/*0x95*/ &res_2_L,
		/*0x96*/ &res_2_HL,
		/*0x97*/ &res_2_A,
		/*0x98*/ &res_3_B,
		/*0x99*/ &res_3_C,
		/*0x9A*/ &res_3_D,
		/*0x9B*/ &res_3_E,
		/*0x9C*/ &res_3_H,
		/*0x9D*/ &res_3_L,
		/*0x9E*/ &res_3_HL,
		/*0x9F*/ &res_3_A,

		/*0xA0*/ &res_4_B,
		/*0xA1*/ &res_4_C,
		/*0xA2*/ &res_4_D,
		/*0xA3*/ &res_4_E,
		/*0xA4*/ &res_4_H,
		/*0xA5*/ &res_4_L,
		/*0xA6*/ &res_4_HL,
		/*0xA7*/ &res_4_A,
		/*0xA8*/ &res_5_B,
		/*0xA9*/ &res_5_C,
		/*0xAA*/ &res_5_D,
		/*0xAB*/ &res_5_E,
		/*0xAC*/ &res_5_H,
		/*0xAD*/ &res_5_L,
		/*0xAE*/ &res_5_HL,
		/*0xAF*/ &res_5_A,

		/*0xB0*/ &res_6_B,
		/*0xB1*/ &res_6_C,
		/*0xB2*/ &res_6_D,
		/*0xB3*/ &res_6_E,
		/*0xB4*/ &res_6_H,
		/*0xB5*/ &res_6_L,
		/*0xB6*/ &res_6_HL,
		/*0xB7*/ &res_6_A,
		/*0xB8*/ &res_7_B,
		/*0xB9*/ &res_7_C,
		/*0xBA*/ &res_7_D,
		/*0xBB*/ &res_7_E,
		/*0xBC*/ &res_7_H,
		/*0xBD*/ &res_7_L,
		/*0xBE*/ &res_7_HL,
		/*0xBF*/ &res_7_A,

		/*0xC0*/ &set_0_B,
		/*0xC1*/ &set_0_C,
		/*0xC2*/ &set_0_D,
		/*0xC3*/ &set_0_E,
		/*0xC4*/ &set_0_H,
		/*0xC5*/ &set_0_L,
		/*0xC6*/ &set_0_HL,
		/*0xC7*/ &set_0_A,
		/*0xC8*/ &set_1_B,
		/*0xC9*/ &set_1_C,
		/*0xCA*/ &set_1_D,
		/*0xCB*/ &set_1_E,
		/*0xCC*/ &set_1_H,
		/*0xCD*/ &set_1_L,
		/*0xCE*/ &set_1_HL,
		/*0xCF*/ &set_1_A,

		/*0xD0*/ &set_2_B,
		/*0xD1*/ &set_2_C,
		/*0xD2*/ &set_2_D,
		/*0xD3*/ &set_2_E,
		/*0xD4*/ &set_2_H,
		/*0xD5*/ &set_2_L,
		/*0xD6*/ &set_2_HL,
		/*0xD7*/ &set_2_A,
		/*0xD8*/ &set_3_B,
		/*0xD9*/ &set_3_C,
		/*0xDA*/ &set_3_D,
		/*0xDB*/ &set_3_E,
		/*0xDC*/ &set_3_H,
		/*0xDD*/ &set_3_L,
		/*0xDE*/ &set_3_HL,
		/*0xDF*/ &set_3_A,

		/*0xE0*/ &set_4_B,
		/*0xE1*/ &set_4_C,
		/*0xE2*/ &set_4_D,
		/*0xE3*/ &set_4_E,
		/*0xE4*/ &set_4_H,
		/*0xE5*/ &set_4_L,
		/*0xE6*/ &set_4_HL,
		/*0xE7*/ &set_4_A,
		/*0xE8*/ &set_5_B,
		/*0xE9*/ &set_5_C,
		/*0xEA*/ &set_5_D,
		/*0xEB*/ &set_5_E,
		/*0xEC*/ &set_5_H,
		/*0xED*/ &set_5_L,
		/*0xEE*/ &set_5_HL,
		/*0xEF*/ &set_5_A,

		/*0xF0*/ &set_6_B,
		/*0xF1*/ &set_6_C,
		/*0xF2*/ &set_6_D,
		/*0xF3*/ &set_6_E,
		/*0xF4*/ &set_6_H,
		/*0xF5*/ &set_6_L,
		/*0xF6*/ &set_6_HL,
		/*0xF7*/ &set_6_A,
		/*0xF8*/ &set_7_B,
		/*0xF9*/ &set_7_C,
		/*0xFA*/ &set_7_D,
		/*0xFB*/ &set_7_E,
		/*0xFC*/ &set_7_H,
		/*0xFD*/ &set_7_L,
		/*0xFE*/ &set_7_HL,
		/*0xFF*/ &set_7_A,
	};
	// Fetch opcode
	uint8_t opcode = RB(gb, gb->cpu.PC++, 0);
	// Execute instruction
	return (*instr_lookup[opcode])(gb);
}

// To address all regular opcodes
struct instrTimingInfo cpu_execute(struct GB* gb) 
{
	// Opcode and operand lookup tables
	static const struct instrTimingInfo(*instr_lookup[256])(struct GB* gb) = {
		/*0x00*/ &nop,
		/*0x01*/ &ld_BC_nn,
		/*0x02*/ &ld_BC_A,
		/*0x03*/ &inc_BC,
		/*0x04*/ &inc_A_B,
		/*0x05*/ &dec_A_B,
		/*0x06*/ &ld_B_n,
		/*0x07*/ &rlca,
		/*0x08*/ &ld_nn_SP,
		/*0x09*/ &add_HL_BC,
		/*0x0A*/ &ld_A_BC,
		/*0x0B*/ &dec_BC,
		/*0x0C*/ &inc_A_C,
		/*0x0D*/ &dec_A_C,
		/*0x0E*/ &ld_C_n,
		/*0x0F*/ &rrca,

		/*0x10*/ &stop,
		/*0x11*/ &ld_DE_nn,
		/*0x12*/ &ld_DE_A,
		/*0x13*/ &inc_DE,
		/*0x14*/ &inc_A_D,
		/*0x15*/ &dec_A_D,
		/*0x16*/ &ld_D_n,
		/*0x17*/ &rla,
		/*0x18*/ &jp_PC_dd,
		/*0x19*/ &add_HL_DE,
		/*0x1A*/ &ld_A_DE,
		/*0x1B*/ &dec_DE,
		/*0x1C*/ &inc_A_E,
		/*0x1D*/ &dec_A_E,
		/*0x1E*/ &ld_E_n,
		/*0x1F*/ &rra,

		/*0x20*/ &jp_NZ_PC_dd,
		/*0x21*/ &ld_HL_nn,
		/*0x22*/ &ldi_HL_A,
		/*0x23*/ &inc_HL,
		/*0x24*/ &inc_A_H,
		/*0x25*/ &dec_A_H,
		/*0x26*/ &ld_H_n,
		/*0x27*/ &daa,
		/*0x28*/ &jp_Z_PC_dd,
		/*0x29*/ &add_HL_HL,
		/*0x2A*/ &ldi_A_HL,
		/*0x2B*/ &dec_HL,
		/*0x2C*/ &inc_A_L,
		/*0x2D*/ &dec_A_L,
		/*0x2E*/ &ld_L_n,
		/*0x2F*/ &cpl,

		/*0x30*/ &jp_NC_PC_dd,
		/*0x31*/ &ld_SP_nn,
		/*0x32*/ &ldd_HL_A,
		/*0x33*/ &inc_SP,
		/*0x34*/ &inc_A_HL,
		/*0x35*/ &dec_A_HL,
		/*0x36*/ &ld_HL_n,
		/*0x37*/ &scf,
		/*0x38*/ &jp_C_PC_dd,
		/*0x39*/ &add_HL_SP,
		/*0x3A*/ &ldd_A_HL,
		/*0x3B*/ &dec_SP,
		/*0x3C*/ &inc_A_A,
		/*0x3D*/ &dec_A_A,
		/*0x3E*/ &ld_A_n,
		/*0x3F*/ &ccf,

		/*0x40*/ &ld_B_B,
		/*0x41*/ &ld_B_C,
		/*0x42*/ &ld_B_D,
		/*0x43*/ &ld_B_E,
		/*0x44*/ &ld_B_H,
		/*0x45*/ &ld_B_L,
		/*0x46*/ &ld_B_HL,
		/*0x47*/ &ld_B_A,
		/*0x48*/ &ld_C_B,
		/*0x49*/ &ld_C_C,
		/*0x4A*/ &ld_C_D,
		/*0x4B*/ &ld_C_E,
		/*0x4C*/ &ld_C_H,
		/*0x4D*/ &ld_C_L,
		/*0x4E*/ &ld_C_HL,
		/*0x4F*/ &ld_C_A,

		/*0x50*/ &ld_D_B,
		/*0x51*/ &ld_D_C,
		/*0x52*/ &ld_D_D,
		/*0x53*/ &ld_D_E,
		/*0x54*/ &ld_D_H,
		/*0x55*/ &ld_D_L,
		/*0x56*/ &ld_D_HL,
		/*0x57*/ &ld_D_A,
		/*0x58*/ &ld_E_B,
		/*0x59*/ &ld_E_C,
		/*0x5A*/ &ld_E_D,
		/*0x5B*/ &ld_E_E,
		/*0x5C*/ &ld_E_H,
		/*0x5D*/ &ld_E_L,
		/*0x5E*/ &ld_E_HL,
		/*0x5F*/ &ld_E_A,

		/*0x60*/ &ld_H_B,
		/*0x61*/ &ld_H_C,
		/*0x62*/ &ld_H_D,
		/*0x63*/ &ld_H_E,
		/*0x64*/ &ld_H_H,
		/*0x65*/ &ld_H_L,
		/*0x66*/ &ld_H_HL,
		/*0x67*/ &ld_H_A,
		/*0x68*/ &ld_L_B,
		/*0x69*/ &ld_L_C,
		/*0x6A*/ &ld_L_D,
		/*0x6B*/ &ld_L_E,
		/*0x6C*/ &ld_L_H,
		/*0x6D*/ &ld_L_L,
		/*0x6E*/ &ld_L_HL,
		/*0x6F*/ &ld_L_A,

		/*0x70*/ &ld_HL_B,
		/*0x71*/ &ld_HL_C,
		/*0x72*/ &ld_HL_D,
		/*0x73*/ &ld_HL_E,
		/*0x74*/ &ld_HL_H,
		/*0x75*/ &ld_HL_L,
		/*0x76*/ &halt,
		/*0x77*/ &ld_HL_A,
		/*0x78*/ &ld_A_B,
		/*0x79*/ &ld_A_C,
		/*0x7A*/ &ld_A_D,
		/*0x7B*/ &ld_A_E,
		/*0x7C*/ &ld_A_H,
		/*0x7D*/ &ld_A_L,
		/*0x7E*/ &ld_A_HL,
		/*0x7F*/ &ld_A_A,

		/*0x80*/ &add_A_B,
		/*0x81*/ &add_A_C,
		/*0x82*/ &add_A_D,
		/*0x83*/ &add_A_E,
		/*0x84*/ &add_A_H,
		/*0x85*/ &add_A_L,
		/*0x86*/ &add_A_HL,
		/*0x87*/ &add_A_A,
		/*0x88*/ &adc_A_B,
		/*0x89*/ &adc_A_C,
		/*0x8A*/ &adc_A_D,
		/*0x8B*/ &adc_A_E,
		/*0x8C*/ &adc_A_H,
		/*0x8D*/ &adc_A_L,
		/*0x8E*/ &adc_A_HL,
		/*0x8F*/ &adc_A_A,

		/*0x90*/ &sub_A_B,
		/*0x91*/ &sub_A_C,
		/*0x92*/ &sub_A_D,
		/*0x93*/ &sub_A_E,
		/*0x94*/ &sub_A_H,
		/*0x95*/ &sub_A_L,
		/*0x96*/ &sub_A_HL,
		/*0x97*/ &sub_A_A,
		/*0x98*/ &sbc_A_B,
		/*0x99*/ &sbc_A_C,
		/*0x9A*/ &sbc_A_D,
		/*0x9B*/ &sbc_A_E,
		/*0x9C*/ &sbc_A_H,
		/*0x9D*/ &sbc_A_L,
		/*0x9E*/ &sbc_A_HL,
		/*0x9F*/ &sbc_A_A,

		/*0xA0*/ &and_A_B,
		/*0xA1*/ &and_A_C,
		/*0xA2*/ &and_A_D,
		/*0xA3*/ &and_A_E,
		/*0xA4*/ &and_A_H,
		/*0xA5*/ &and_A_L,
		/*0xA6*/ &and_A_HL,
		/*0xA7*/ &and_A_A,
		/*0xA8*/ &xor_A_B,
		/*0xA9*/ &xor_A_C,
		/*0xAA*/ &xor_A_D,
		/*0xAB*/ &xor_A_E,
		/*0xAC*/ &xor_A_H,
		/*0xAD*/ &xor_A_L,
		/*0xAE*/ &xor_A_HL,
		/*0xAF*/ &xor_A_A,

		/*0xB0*/ &or_A_B,
		/*0xB1*/ &or_A_C,
		/*0xB2*/ &or_A_D,
		/*0xB3*/ &or_A_E,
		/*0xB4*/ &or_A_H,
		/*0xB5*/ &or_A_L,
		/*0xB6*/ &or_A_HL,
		/*0xB7*/ &or_A_A,
		/*0xB8*/ &cp_A_B,
		/*0xB9*/ &cp_A_C,
		/*0xBA*/ &cp_A_D,
		/*0xBB*/ &cp_A_E,
		/*0xBC*/ &cp_A_H,
		/*0xBD*/ &cp_A_L,
		/*0xBE*/ &cp_A_HL,
		/*0xBF*/ &cp_A_A,

		/*0xC0*/ &ret_NZ,
		/*0xC1*/ &pop_BC,
		/*0xC2*/ &jp_NZ_nn,
		/*0xC3*/ &jp_nn,
		/*0xC4*/ &call_NZ_nn,
		/*0xC5*/ &push_BC,
		/*0xC6*/ &add_A_n,
		/*0xC7*/ &rst_0x00,
		/*0xC8*/ &ret_Z,
		/*0xC9*/ &ret,
		/*0xCA*/ &jp_Z_nn,
		/*0xCB*/ &goto_cb_prefix_table,
		/*0xCC*/ &call_Z_nn,
		/*0xCD*/ &call_nn,
		/*0xCE*/ &adc_A_n,
		/*0xCF*/ &rst_0x08,

		/*0xD0*/ &ret_NC,
		/*0xD1*/ &pop_DE,
		/*0xD2*/ &jp_NC_nn,
		/*0xD3*/ &nop, // Undefined
		/*0xD4*/ &call_NC_nn,
		/*0xD5*/ &push_DE,
		/*0xD6*/ &sub_A_n,
		/*0xD7*/ &rst_0x10,
		/*0xD8*/ &ret_C,
		/*0xD9*/ &reti,
		/*0xDA*/ &jp_C_nn,
		/*0xDB*/ &nop, // Undefined
		/*0xDC*/ &call_C_nn,
		/*0xDD*/ &nop, // Undefined
		/*0xDE*/ &sbc_A_n,
		/*0xDF*/ &rst_0x18,

		/*0xE0*/ &ldh_n_A,
		/*0xE1*/ &pop_HL,
		/*0xE2*/ &ldh_C_A,
		/*0xE3*/ &nop, // Undefined
		/*0xE4*/ &nop, // Undefined
		/*0xE5*/ &push_HL,
		/*0xE6*/ &and_A_n,
		/*0xE7*/ &rst_0x20,
		/*0xE8*/ &add_SP_dd,
		/*0xE9*/ &jp_HL,
		/*0xEA*/ &ld_nn_A,
		/*0xEB*/ &nop, // Undefined
		/*0xEC*/ &nop, // Undefined
		/*0xED*/ &nop, // Undefined
		/*0xEE*/ &xor_A_n,
		/*0xEF*/ &rst_0x28,

		/*0xF0*/ &ldh_A_n,
		/*0xF1*/ &pop_AF,
		/*0xF2*/ &ldh_A_C,
		/*0xF3*/ &di,
		/*0xF4*/ &nop, // Undefined
		/*0xF5*/ &push_AF,
		/*0xF6*/ &or_A_n,
		/*0xF7*/ &rst_0x30,
		/*0xF8*/ &ld_HP_SP_dd,
		/*0xF9*/ &ld_SP_HL,
		/*0xFA*/ &ld_A_nn,
		/*0xFB*/ &ei,
		/*0xFC*/ &nop, // Undefined
		/*0xFD*/ &nop, // Undefined
		/*0xFE*/ &cp_A_n,
		/*0xFF*/ &rst_0x38,
	};
	// Fetch opcode
	uint8_t opcode = RB(gb, gb->cpu.PC++, 0);
	// Execute instruction
	return (*instr_lookup[opcode])(gb);
}

int int_request(struct GB* gb, int cycles) 
{
	/*		Effectively 2 NOPs occur, the PC is pushed to the stack (2 cycles since
				2 one byte pushes), and PC register is set to the address of the
				corresponding interrupt handler at one of the following vectors:
					- IF Bit 0: VBlank   (INT $40)
					- IF Bit 1: LCD STAT (INT $48)
					- IF Bit 2: Timer    (INT $50)
					- IF Bit 3: Serial   (INT $58)
					- IF Bit 4: Joypad   (INT $60)
			When interrupt is triggered, it's corresponding bit in IF register
				is set high (this is what triggered this interrupt)
			IF register only requests the interrupt be executed. Interrupt only
				triggers if its corresponding IE bit is also high.
			No interrupts occur regardless of IE or IF bits if the Interrupt 
				Master Enable is low, which can only be set through specific
				instructions EI and DI.
			The earlier described instructions might take not actually enable the
				IME until the next instruction, the 'IME_scheduler' is my solution
				to this. No such IME_scheduler is described in any documentation
	*/

	// Lookup tables for interrupt vectors and bit masks to mask the necessary IF 
	//		bits as an interrupt is serviced
	static const uint16_t   bitmasks[5] = { ~INTMASK_VBLANK, ~INTMASK_STAT, ~INTMASK_TIMER, ~INTMASK_SERIAL, ~INTMASK_JOYPAD };
	static const uint16_t intVectors[5] = { 0x0040, 0x0048, 0x0050, 0x0058, 0x0060 };

	// Update scheduler, setting IME if needed
	uint8_t IF = gb->ioregs[0x0F], IE = gb->reg_IE;
	gb->cpu.IME_scheduler >>= (cycles >> 2);
	if (gb->cpu.IME_scheduler & 0x3FFF) gb->cpu.IME = 1;

	// If the CPU is in halt mode, 4 extra cycles are used on top of the typical 20
	//		it uses to service a requested interrupt
	int serviceCycles = gb->cpu.halt ? 24 : 20;

	// Lower bit interrupts are highest priority
	for (int i = 0; i < 5; i++) 
	{
		if (IF & IE & (1 << i)) 
		{   // Required in this specific order to bring the CPU out of 
			//		halt mode if IME is zero prior to entering halt mode
			gb->cpu.halt = 0;
			if (!gb->cpu.IME) break;

			// Update CPU state
			gb->cpu.IME_scheduler = 0;
			gb->cpu.IME = 0;

			// Push return value to stack
			WB(gb, --gb->cpu.SP, (gb->cpu.PC >> 8), 0);
			WB(gb, --gb->cpu.SP, (gb->cpu.PC & 0x00FF), 0);

			// Clear flag bits and update PC
			gb->ioregs[0x0F] &= bitmasks[i];
			gb->cpu.PC = intVectors[i];

			// Return cycles used to service the interrupt
			return serviceCycles;
		}
	}

	// No interrupts requested, no additional cycles needed 
	return 0;
}



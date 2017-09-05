#include "../Component/Instruction.h"
#include "../Component/CPU.h"


/**
	struct instructions {
    	char name[25];
    	unsigned char size;
    	unsigned char length;
    	unsigned char ticks;
    	uint8_t (CPU::*function)();
	};

	ticks = 0 means it is a conditional instruction and we must the 
	status bits in the F register to decide how many ticks the instruction
	would need. 
	Every index of the array stands for the opcode for the instruction.

	Opcode table based on: http://pastraiser.com/cpu/gameboy/gameboy_opcodes.html



	d8  means immediate 8 bit data
	d16 means immediate 16 bit data
	a8  means 8 bit unsigned data, which are added to $FF00 in certain instructions (replacement for missing IN and OUT instructions)
	a16 means 16 bit address
	r8  means 8 bit signed data, which are added to program counter

**/


const instructions instruction[256] = {
	{ "NOP",				1, 1, 4,  &CPU::nop},						// 0x00
	{ "LD BC, 0x%04X",		3, 3, 12, &CPU::ld_bc_d16 },				// 0x01
	{ "LD (BC), A",			1, 1, 8,  &CPU::ld_bcp_a },					// 0x02
	{ "INC BC",				1, 1, 8,  &CPU::inc_bc },					// 0x03
	{ "INC B",				1, 1, 4,  &CPU::inc_b },					// 0x04
	{ "DEC B",			 	1, 1, 4,  &CPU::dec_b },					// 0x05
	{ "LD B, 0x%02X",		2, 2, 8,  &CPU::ld_b_d8 },					// 0x06
	{ "RLCA",				1, 1, 4,  &CPU::rlca },						// 0x07
	{ "LD (0x%04X), SP", 	3, 3, 20, &CPU::ld_a16p_sp },				// 0x08
	{ "ADD HL, BC",			1, 1, 8,  &CPU::add_hl_bc },				// 0x09
	{ "LD A, (BC)",			1, 1, 8,  &CPU::ld_a_bcp },					// 0x0a
	{ "DEC BC",				1, 1, 8,  &CPU::dec_bc },					// 0x0b
	{ "INC C",				1, 1, 4,  &CPU::inc_c },					// 0x0c
	{ "DEC C",				1, 1, 4,  &CPU::dec_c },					// 0x0d
	{ "LD C, 0x%02X",		2, 2, 8,  &CPU::ld_c_d8 },					// 0x0e
	{ "RRCA",				1, 1, 4,  &CPU::rrca },						// 0x0f
	{ "STOP",				2, 2, 4,  &CPU::stop },						// 0x10
	{ "LD DE, 0x%04X",		3, 3, 12, &CPU::ld_de_d16 },				// 0x11
	{ "LD (DE), A",			1, 1, 8,  &CPU::ld_dep_a },					// 0x12
	{ "INC DE",				1, 1, 8,  &CPU::inc_de },					// 0x13
	{ "INC D",				1, 1, 4,  &CPU::inc_d },					// 0x14
	{ "DEC D",				1, 1, 4,  &CPU::dec_d },					// 0x15
	{ "LD D, 0x%02X",		2, 2, 8,  &CPU::ld_d_d8 },					// 0x16
	{ "RLA",				1, 1, 4,  &CPU::rla },						// 0x17
	{ "JR 0x%02X",			2, 2, 12, &CPU::jr_r8 },					// 0x18
	{ "ADD HL, DE",			1, 1, 8,  &CPU::add_hl_de },				// 0x19
	{ "LD A, (DE)",			1, 1, 8,  &CPU::ld_a_dep },					// 0x1a
	{ "DEC DE",				1, 1, 8,  &CPU::dec_de },					// 0x1b
	{ "INC E",				1, 1, 4,  &CPU::inc_e },					// 0x1c
	{ "DEC E",				1, 1, 4,  &CPU::dec_e },					// 0x1d
	{ "LD E, 0x%02X",		2, 2, 8,  &CPU::ld_e_d8 },					// 0x1e
	{ "RRA",				1, 1, 4,  &CPU::rra },						// 0x1f
	{ "JR NZ, 0x%02X",		2, 2, 0,  &CPU::jr_nz_r8 },					// 0x20
	{ "LD HL, 0x%04X",		3, 3, 12, &CPU::ld_hl_d16 },				// 0x21
	{ "LDI (HL), A",		1, 1, 8,  &CPU::ldi_hlp_a },				// 0x22
	{ "INC HL",				1, 1, 8,  &CPU::inc_hl },					// 0x23
	{ "INC H",				1, 1, 4,  &CPU::inc_h },					// 0x24
	{ "DEC H",				1, 1, 4,  &CPU::dec_h },					// 0x25
	{ "LD H, 0x%02X",		2, 2, 8,  &CPU::ld_h_d8 },					// 0x26
	{ "DAA",				1, 1, 4,  &CPU::daa },						// 0x27
	{ "JR Z, 0x%02X",		2, 2, 0,  &CPU::jr_z_r8 },					// 0x28
	{ "ADD HL, HL",			1, 1, 8,  &CPU::add_hl_hl },				// 0x29
	{ "LDI A, (HL)",		1, 1, 8,  &CPU::ldi_a_hlp },				// 0x2a
	{ "DEC HL",				1, 1, 8,  &CPU::dec_hl },					// 0x2b
	{ "INC L",				1, 1, 4,  &CPU::inc_l },					// 0x2c
	{ "DEC L",				1, 1, 4,  &CPU::dec_l },					// 0x2d
	{ "LD L, 0x%02X",		2, 2, 8,  &CPU::ld_l_d8 },					// 0x2e
	{ "CPL",				1, 1, 4,  &CPU::cpl },						// 0x2f
	{ "JR NC, 0x%02X",		2, 2, 0,  &CPU::jr_nc_r8 },					// 0x30
	{ "LD SP, 0x%04X",		3, 3, 12, &CPU::ld_sp_d16 },				// 0x31
	{ "LDD (HL), A",		1, 1, 8,  &CPU::ldd_hlp_a },				// 0x32
	{ "INC SP",				1, 1, 8,  &CPU::inc_sp },					// 0x33
	{ "INC (HL)",			1, 1, 12, &CPU::inc_hlp },					// 0x34
	{ "DEC (HL)",			1, 1, 12, &CPU::dec_hlp },					// 0x35
	{ "LD (HL), 0x%02X",	2, 2, 12, &CPU::ld_hlp_d8 },				// 0x36
	{ "SCF",				1, 1, 4,  &CPU::scf },						// 0x37
	{ "JR C, 0x%02X",		2, 2, 0,  &CPU::jr_c_r8 },					// 0x38
	{ "ADD HL, SP",			1, 1, 8,  &CPU::add_hl_sp },				// 0x39
	{ "LDD A, (HL)",		1, 1, 8,  &CPU::ldd_a_hlp },				// 0x3a
	{ "DEC SP",				1, 1, 8,  &CPU::dec_sp },					// 0x3b
	{ "INC A",				1, 1, 4,  &CPU::inc_a },					// 0x3c
	{ "DEC A",				1, 1, 4,  &CPU::dec_a },					// 0x3d
	{ "LD A, 0x%02X",		2, 2, 8,  &CPU::ld_a_d8 },					// 0x3e
	{ "CCF",				1, 1, 4,  &CPU::ccf },						// 0x3f
	{ "LD B, B",			1, 1, 4,  &CPU::nop },						// 0x40
	{ "LD B, C",			1, 1, 4,  &CPU::ld_b_c },					// 0x41
	{ "LD B, D",			1, 1, 4,  &CPU::ld_b_d },					// 0x42
	{ "LD B, E",			1, 1, 4,  &CPU::ld_b_e },					// 0x43
	{ "LD B, H",			1, 1, 4,  &CPU::ld_b_h },					// 0x44
	{ "LD B, L",			1, 1, 4,  &CPU::ld_b_l },					// 0x45
	{ "LD B, (HL)",			1, 1, 8,  &CPU::ld_b_hlp },					// 0x46
	{ "LD B, A",			1, 1, 4,  &CPU::ld_b_a },					// 0x47
	{ "LD C, B",			1, 1, 4,  &CPU::ld_c_b },					// 0x48
	{ "LD C, C",			1, 1, 4,  &CPU::nop },						// 0x49
	{ "LD C, D",			1, 1, 4,  &CPU::ld_c_d },					// 0x4a
	{ "LD C, E",			1, 1, 4,  &CPU::ld_c_e },					// 0x4b
	{ "LD C, H",			1, 1, 4,  &CPU::ld_c_h },					// 0x4c
	{ "LD C, L", 			1, 1, 4,  &CPU::ld_c_l },					// 0x4d
	{ "LD C, (HL)",			1, 1, 8,  &CPU::ld_c_hlp },					// 0x4e
	{ "LD C, A",			1, 1, 4,  &CPU::ld_c_a },					// 0x4f
	{ "LD D, B",			1, 1, 4,  &CPU::ld_d_b },					// 0x50
	{ "LD D, C",			1, 1, 4,  &CPU::ld_d_c },					// 0x51
	{ "LD D, D",			1, 1, 4,  &CPU::nop },						// 0x52
	{ "LD D, E",			1, 1, 4,  &CPU::ld_d_e },					// 0x53
	{ "LD D, H",			1, 1, 4,  &CPU::ld_d_h },					// 0x54
	{ "LD D, L",			1, 1, 4,  &CPU::ld_d_l },					// 0x55
	{ "LD D, (HL)",			1, 1, 8,  &CPU::ld_d_hlp },					// 0x56
	{ "LD D, A", 			1, 1, 4,  &CPU::ld_d_a },					// 0x57
	{ "LD E, B",			1, 1, 4,  &CPU::ld_e_b },					// 0x58
	{ "LD E, C",			1, 1, 4,  &CPU::ld_e_c },					// 0x59
	{ "LD E, D",			1, 1, 4,  &CPU::ld_e_d },					// 0x5a
	{ "LD E, E",			1, 1, 4,  &CPU::nop },						// 0x5b
	{ "LD E, H",			1, 1, 4,  &CPU::ld_e_h },					// 0x5c
	{ "LD E, L",			1, 1, 4,  &CPU::ld_e_l },					// 0x5d
	{ "LD E, (HL)",			1, 1, 8,  &CPU::ld_e_hlp },					// 0x5e
	{ "LD E, A",			1, 1, 4,  &CPU::ld_e_a },					// 0x5f
	{ "LD H, B",			1, 1, 4,  &CPU::ld_h_b },					// 0x60
	{ "LD H, C",			1, 1, 4,  &CPU::ld_h_c },					// 0x61
	{ "LD H, D",			1, 1, 4,  &CPU::ld_h_d },					// 0x62
	{ "LD H, E",			1, 1, 4,  &CPU::ld_h_e },					// 0x63
	{ "LD H, H",			1, 1, 4,  &CPU::nop },						// 0x64
	{ "LD H, L",			1, 1, 4,  &CPU::ld_h_l },					// 0x65
	{ "LD H, (HL)",			1, 1, 8,  &CPU::ld_h_hlp },					// 0x66
	{ "LD H, A",			1, 1, 4,  &CPU::ld_h_a },					// 0x67
	{ "LD L, B",			1, 1, 4,  &CPU::ld_l_b },					// 0x68
	{ "LD L, C",			1, 1, 4,  &CPU::ld_l_c },					// 0x69
	{ "LD L, D",			1, 1, 4,  &CPU::ld_l_d },					// 0x6a
	{ "LD L, E",			1, 1, 4,  &CPU::ld_l_e },					// 0x6b
	{ "LD L, H",			1, 1, 4,  &CPU::ld_l_h },					// 0x6c
	{ "LD L, L",			1, 1, 4,  &CPU::nop },						// 0x6d
	{ "LD L, (HL)",			1, 1, 8,  &CPU::ld_l_hlp },					// 0x6e
	{ "LD L, A",			1, 1, 4,  &CPU::ld_l_a },					// 0x6f
	{ "LD (HL), B",			1, 1, 8,  &CPU::ld_hlp_b },					// 0x70
	{ "LD (HL), C",			1, 1, 8,  &CPU::ld_hlp_c },					// 0x71
	{ "LD (HL), D",			1, 1, 8,  &CPU::ld_hlp_d },					// 0x72
	{ "LD (HL), E",			1, 1, 8,  &CPU::ld_hlp_e },					// 0x73
	{ "LD (HL), H",			1, 1, 8,  &CPU::ld_hlp_h },					// 0x74
	{ "LD (HL), L",			1, 1, 8,  &CPU::ld_hlp_l },					// 0x75
	{ "HALT",				1, 1, 4,  &CPU::halt },						// 0x76
	{ "LD (HL), A",			1, 1, 8,  &CPU::ld_hlp_a },					// 0x77
	{ "LD A, B",			1, 1, 4,  &CPU::ld_a_b },					// 0x78
	{ "LD A, C",			1, 1, 4,  &CPU::ld_a_c },					// 0x79
	{ "LD A, D",			1, 1, 4,  &CPU::ld_a_d },					// 0x7a
	{ "LD A, E",			1, 1, 4,  &CPU::ld_a_e },					// 0x7b
	{ "LD A, H",			1, 1, 4,  &CPU::ld_a_h },					// 0x7c
	{ "LD A, L",			1, 1, 4,  &CPU::ld_a_l },					// 0x7d
	{ "LD A, (HL)",			1, 1, 8,  &CPU::ld_a_hlp },					// 0x7e
	{ "LD A, A",			1, 1, 4,  &CPU::nop },						// 0x7f
	{ "ADD A, B",			1, 1, 4,  &CPU::add_a_b },					// 0x80
	{ "ADD A, C",			1, 1, 4,  &CPU::add_a_c },					// 0x81
	{ "ADD A, D",			1, 1, 4,  &CPU::add_a_d },					// 0x82
	{ "ADD A, E", 			1, 1, 4,  &CPU::add_a_e },					// 0x83
	{ "ADD A, H",			1, 1, 4,  &CPU::add_a_h },					// 0x84
	{ "ADD A, L",			1, 1, 4,  &CPU::add_a_l },					// 0x85
	{ "ADD A, (HL)",		1, 1, 8,  &CPU::add_a_hlp },				// 0x86
	{ "ADD A",				1, 1, 4,  &CPU::add_a_a },					// 0x87
	{ "ADC B",				1, 1, 4,  &CPU::adc_a_b },					// 0x88
	{ "ADC C",				1, 1, 4,  &CPU::adc_a_c },					// 0x89
	{ "ADC D", 				1, 1, 4,  &CPU::adc_a_d },					// 0x8a
	{ "ADC E", 				1, 1, 4,  &CPU::adc_a_e },					// 0x8b
	{ "ADC H", 				1, 1, 4,  &CPU::adc_a_h },					// 0x8c
	{ "ADC L", 				1, 1, 4,  &CPU::adc_a_l },					// 0x8d
	{ "ADC (HL)", 			1, 1, 8,  &CPU::adc_a_hlp },				// 0x8e
	{ "ADC A", 				1, 1, 4,  &CPU::adc_a_a },					// 0x8f
	{ "SUB B", 				1, 1, 4,  &CPU::sub_a_b },					// 0x90
	{ "SUB C", 				1, 1, 4,  &CPU::sub_a_c },					// 0x91
	{ "SUB D", 				1, 1, 4,  &CPU::sub_a_d },					// 0x92
	{ "SUB E", 				1, 1, 4,  &CPU::sub_a_e },					// 0x93
	{ "SUB H", 				1, 1, 4,  &CPU::sub_a_h },					// 0x94
	{ "SUB L",				1, 1, 4,  &CPU::sub_a_l },					// 0x95
	{ "SUB (HL)", 			1, 1, 8,  &CPU::sub_a_hlp },				// 0x96
	{ "SUB A", 				1, 1, 4,  &CPU::sub_a_a },					// 0x97
	{ "SBC B", 				1, 1, 4,  &CPU::sbc_a_b },					// 0x98
	{ "SBC C", 				1, 1, 4,  &CPU::sbc_a_c },					// 0x99
	{ "SBC D", 				1, 1, 4,  &CPU::sbc_a_d },					// 0x9a
	{ "SBC E", 				1, 1, 4,  &CPU::sbc_a_e },					// 0x9b
	{ "SBC H", 				1, 1, 4,  &CPU::sbc_a_h },					// 0x9c
	{ "SBC L", 				1, 1, 4,  &CPU::sbc_a_l },					// 0x9d
	{ "SBC (HL)", 			1, 1, 8,  &CPU::sbc_a_hlp },				// 0x9e
	{ "SBC A", 				1, 1, 4,  &CPU::sbc_a_a },					// 0x9f
	{ "AND B", 				1, 1, 4,  &CPU::and_a_b },					// 0xa0
	{ "AND C", 				1, 1, 4,  &CPU::and_a_c },					// 0xa1
	{ "AND D", 				1, 1, 4,  &CPU::and_a_d },					// 0xa2
	{ "AND E", 				1, 1, 4,  &CPU::and_a_e },					// 0xa3
	{ "AND H", 				1, 1, 4,  &CPU::and_a_h },					// 0xa4
	{ "AND L", 				1, 1, 4,  &CPU::and_a_l },					// 0xa5
	{ "AND (HL)", 			1, 1, 8,  &CPU::and_a_hlp },				// 0xa6
	{ "AND A", 				1, 1, 4,  &CPU::and_a_a },					// 0xa7
	{ "XOR B", 				1, 1, 4,  &CPU::xor_a_b },					// 0xa8
	{ "XOR C", 				1, 1, 4,  &CPU::xor_a_c },					// 0xa9
	{ "XOR D", 				1, 1, 4,  &CPU::xor_a_d },					// 0xaa
	{ "XOR E", 				1, 1, 4,  &CPU::xor_a_e },					// 0xab
	{ "XOR H", 				1, 1, 4,  &CPU::xor_a_h },					// 0xac
	{ "XOR L", 				1, 1, 4,  &CPU::xor_a_l },					// 0xad
	{ "XOR (HL)", 			1, 1, 8,  &CPU::xor_a_hlp },				// 0xae
	{ "XOR A",				1, 1, 4,  &CPU::xor_a_a },					// 0xaf
	{ "OR B", 				1, 1, 4,  &CPU::or_a_b },					// 0xb0
	{ "OR C", 				1, 1, 4,  &CPU::or_a_c },					// 0xb1
	{ "OR D", 				1, 1, 4,  &CPU::or_a_d },					// 0xb2
	{ "OR E", 				1, 1, 4,  &CPU::or_a_e },					// 0xb3
	{ "OR H", 				1, 1, 4,  &CPU::or_a_h },					// 0xb4
	{ "OR L", 				1, 1, 4,  &CPU::or_a_l },					// 0xb5
	{ "OR (HL)", 			1, 1, 8,  &CPU::or_a_hlp },					// 0xb6
	{ "OR A", 				1, 1, 4,  &CPU::or_a_a },					// 0xb7
	{ "CP B", 				1, 1, 4,  &CPU::cp_a_b },					// 0xb8
	{ "CP C", 				1, 1, 4,  &CPU::cp_a_c },					// 0xb9
	{ "CP D", 				1, 1, 4,  &CPU::cp_a_d },					// 0xba
	{ "CP E", 				1, 1, 4,  &CPU::cp_a_e },					// 0xbb
	{ "CP H", 				1, 1, 4,  &CPU::cp_a_h },					// 0xbc
	{ "CP L",				1, 1, 4,  &CPU::cp_a_l },					// 0xbd
	{ "CP (HL)",			1, 1, 8,  &CPU::cp_hlp },					// 0xbe
	{ "CP A", 				1, 1, 4,  &CPU::cp_a_a },					// 0xbf
	{ "RET NZ",				1, 0, 0,  &CPU::ret_nz },					// 0xc0
	{ "POP BC", 			1, 1, 12, &CPU::pop_bc },					// 0xc1
	{ "JP NZ, 0x%04X", 		3, 0, 0,  &CPU::jp_nz_a16 },				// 0xc2
	{ "JP 0x%04X", 			3, 0, 16, &CPU::jp_a16 },					// 0xc3
	{ "CALL NZ, 0x%04X",	3, 0, 0,  &CPU::call_nz_a16 },				// 0xc4
	{ "PUSH BC", 			1, 1, 16, &CPU::push_bc },					// 0xc5
	{ "ADD A, 0x%02X", 		2, 2, 8,  &CPU::add_a_d8 }, 				// 0xc6
	{ "RST 0x00", 			1, 0, 16, &CPU::rst_00h },					// 0xc7
	{ "RET Z",				1, 0, 0,  &CPU::ret_z },					// 0xc8
	{ "RET",				1, 0, 16, &CPU::ret },						// 0xc9
	{ "JP Z, 0x%04X",		3, 0, 0,  &CPU::jp_z_a16 },					// 0xca
	{ "PREFIX CB",			1, 1, 4,  &CPU::cb },						// 0xcb
	{ "CALL Z, 0x%04X", 	3, 0, 0,  &CPU::call_z_a16 },				// 0xcc
	{ "CALL 0x%04X", 		3, 0, 24, &CPU::call_a16 },					// 0xcd
	{ "ADC 0x%02X", 		2, 2, 8,  &CPU::adc_d8 },					// 0xce
	{ "RST 0x08", 			1, 0, 16, &CPU::rst_08h },					// 0xcf
	{ "RET NC", 			1, 0, 0,  &CPU::ret_nc },					// 0xd0
	{ "POP DE", 			1, 1, 12, &CPU::pop_de },					// 0xd1
	{ "JP NC, 0x%04X", 		3, 0, 0,  &CPU::jp_nc_a16 },				// 0xd2
	{ "UNKNOWN",			1, 1, 0,  &CPU::undefined },				// 0xd3
	{ "CALL NC, 0x%04X",	3, 0, 0,  &CPU::call_nc_a16 },				// 0xd4
	{ "PUSH DE", 			1, 1, 16, &CPU::push_de },					// 0xd5
	{ "SUB 0x%02X", 		2, 2, 8,  &CPU::sub_a_d8 },					// 0xd6
	{ "RST 0x10", 			1, 0, 16, &CPU::rst_10h },					// 0xd7
	{ "RET C", 				1, 0, 0,  &CPU::ret_c },					// 0xd8
	{ "RETI", 				1, 0, 16, &CPU::reti },						// 0xd9
	{ "JP C, 0x%04X", 		3, 0, 0,  &CPU::jp_c_a16 },					// 0xda
	{ "UNKNOWN", 			1, 1, 0,  &CPU::undefined },				// 0xdb
	{ "CALL C, 0x%04X", 	3, 0, 0,  &CPU::call_c_a16 },				// 0xdc
	{ "UNKNOWN", 			1, 1, 0,  &CPU::undefined },				// 0xdd
	{ "SBC 0x%02X", 		2, 2, 8,  &CPU::sbc_a_d8 },					// 0xde
	{ "RST 0x18", 			1, 0, 16, &CPU::rst_18h },					// 0xdf
	{ "LDH (0xFF00 + 0x%02X), A", 2, 2, 12, &CPU::ldh_a8_ap },			// 0xe0
	{ "POP HL", 			1, 1, 12, &CPU::pop_hl },					// 0xe1
	{ "LDH (0xFF00 + C), A", 1, 1, 8, &CPU::ldh_cp_a },					// 0xe2
	{ "UNKNOWN", 			1, 1, 0,  &CPU::undefined },				// 0xe3
	{ "UNKNOWN",			1, 1, 0,  &CPU::undefined },				// 0xe4
	{ "PUSH HL", 			1, 1, 16, &CPU::push_hl },					// 0xe5
	{ "AND 0x%02X",     	2, 2, 8,  &CPU::and_a_d8 },					// 0xe6
	{ "RST 0x20", 			1, 0, 16, &CPU::rst_20h },					// 0xe7
	{ "ADD SP,0x%02X", 		2, 2, 16, &CPU::add_sp_r8 },				// 0xe8
	{ "JP HL", 				1, 0, 4,  &CPU::jp_hlp },					// 0xe9
	{ "LD (0x%04X), A", 	3, 3, 16, &CPU::ld_a16p_a },				// 0xea
	{ "UNKNOWN", 			1, 1, 0,  &CPU::undefined },				// 0xeb
	{ "UNKNOWN", 			1, 1, 0,  &CPU::undefined },				// 0xec
	{ "UNKNOWN", 			1, 1, 0,  &CPU::undefined },				// 0xed
	{ "XOR 0x%02X", 		2, 2, 8,  &CPU::xor_a_d8 },					// 0xee
	{ "RST 0x28", 			1, 0, 16, &CPU::rst_28h },					// 0xef
	{ "LD A, (0xFF00 + 0x%02X)", 2, 2, 12, &CPU::ldh_a_a8p },			// 0xf0
	{ "POP AF", 			1, 1, 12, &CPU::pop_af },					// 0xf1
	{ "LD A, (0xFF00 + C)", 1, 1, 8, &CPU::ldh_a_cp },					// 0xf2
	{ "DI", 				1, 1, 4,  &CPU::di },						// 0xf3
	{ "UNKNOWN", 			1, 1, 0,  &CPU::undefined },				// 0xf4
	{ "PUSH AF", 			1, 1, 16, &CPU::push_af },					// 0xf5
	{ "OR 0x%02X", 			2, 2, 8,  &CPU::or_a_d8 },					// 0xf6
	{ "RST 0x30", 			1, 0, 16, &CPU::rst_30h },					// 0xf7
	{ "LD HL, SP+0x%02X", 	2, 2, 12,&CPU::ld_hl_sp_r8 },				// 0xf8
	{ "LD SP, HL", 			1, 1, 8,  &CPU::ld_sp_hl },					// 0xf9
	{ "LD A, (0x%04X)", 	3, 3, 16, &CPU::ld_a_a16p },				// 0xfa
	{ "EI", 				1, 1, 4,  &CPU::ei },						// 0xfb
	{ "UNKNOWN", 			1, 1, 0,  &CPU::undefined },				// 0xfc
	{ "UNKNOWN", 			1, 1, 0,  &CPU::undefined },				// 0xfd
	{ "CP 0x%02X", 			2, 2, 8,  &CPU::cp_d8 },					// 0xfe
	{ "RST 0x38", 			1, 0, 16, &CPU::rst_38h },					// 0xff
};
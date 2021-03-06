#include "../Component/ExtInstruction.h"
#include "../Component/CPU.h"


/**
	struct instructions {
		char name[20];
		int operandLength in Bytes;
		int CPU ticks;
		function pointer to C function 
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

    These are the extended Instructions. They start with the prefix 0xCB

**/


const instructions ext_instruction[256] = {

    { "RLC B",			1, 1, 4, &CPU::rlc_b },				// 0x00
	{ "RLC C",			1, 1, 4, &CPU::rlc_c },				// 0x01
	{ "RLC D",			1, 1, 4, &CPU::rlc_d },				// 0x02
	{ "RLC E",			1, 1, 4, &CPU::rlc_e },				// 0x03
	{ "RLC H",			1, 1, 4, &CPU::rlc_h },				// 0x04
	{ "RLC L",			1, 1, 4, &CPU::rlc_l },				// 0x05
	{ "RLC (HL)",		1, 1, 12, &CPU::rlc_hlp },			// 0x06
	{ "RLC A",			1, 1, 4,  &CPU::rlc_a },			// 0x07
	{ "RRC B",			1, 1, 4, &CPU::rrc_b },				// 0x08
	{ "RRC C",			1, 1, 4, &CPU::rrc_c },				// 0x09
	{ "RRC D",			1, 1, 4, &CPU::rrc_d },				// 0x0a
	{ "RRC E",			1, 1, 4, &CPU::rrc_e },				// 0x0b
	{ "RRC H",			1, 1, 4, &CPU::rrc_h },				// 0x0c
	{ "RRC L",			1, 1, 4, &CPU::rrc_l },				// 0x0d
	{ "RRC (HL)",		1, 1, 12, &CPU::rrc_hlp },			// 0x0e
	{ "RRC A",			1, 1, 4, &CPU::rrc_a },				// 0x0f
	{ "RL B",			1, 1, 4, &CPU::rl_b },				// 0x10
	{ "RL C",			1, 1, 4, &CPU::rl_c },				// 0x11
	{ "RL D",			1, 1, 4, &CPU::rl_d },				// 0x12
	{ "RL E",			1, 1, 4, &CPU::rl_e },				// 0x13
	{ "RL H",			1, 1, 4, &CPU::rl_h },				// 0x14
	{ "RL L",			1, 1, 4, &CPU::rl_l },				// 0x15
	{ "RL (HL)",		1, 1, 12, &CPU::rl_hlp },			// 0x16
	{ "RL A",			1, 1, 4, &CPU::rl_a },				// 0x17
	{ "RR B",			1, 1, 4, &CPU::rr_b },				// 0x18
	{ "RR C",			1, 1, 4, &CPU::rr_c },				// 0x19
	{ "RR D",			1, 1, 4, &CPU::rr_d },				// 0x1a
	{ "RR E",			1, 1, 4, &CPU::rr_e },				// 0x1b
	{ "RR H",			1, 1, 4, &CPU::rr_h },				// 0x1c
	{ "RR L",			1, 1, 4, &CPU::rr_l },				// 0x1d
	{ "RR (HL)",		1, 1, 12, &CPU::rr_hlp },			// 0x1e
	{ "RR A",			1, 1, 4, &CPU::rr_a },				// 0x1f
	{ "SLA B",			1, 1, 4, &CPU::sla_b },				// 0x20
	{ "SLA C",			1, 1, 4, &CPU::sla_c },				// 0x21
	{ "SLA D",			1, 1, 4, &CPU::sla_d },				// 0x22
	{ "SLA E",			1, 1, 4, &CPU::sla_e },				// 0x23
	{ "SLA H",			1, 1, 4, &CPU::sla_h },				// 0x24
	{ "SLA L",			1, 1, 4, &CPU::sla_l },				// 0x25
	{ "SLA (HL)",		1, 1, 12,  &CPU::sla_hlp },			// 0x26
	{ "SLA A",			1, 1, 4, &CPU::sla_a },				// 0x27
	{ "SRA B",			1, 1, 4, &CPU::sra_b },				// 0x28
	{ "SRA C",			1, 1, 4, &CPU::sra_c },				// 0x29
	{ "SRA D",			1, 1, 4, &CPU::sra_d },				// 0x2a
	{ "SRA E",			1, 1, 4, &CPU::sra_e },				// 0x2b
	{ "SRA H",			1, 1, 4, &CPU::sra_h },				// 0x2c
	{ "SRA L",			1, 1, 4, &CPU::sra_l },				// 0x2d
	{ "SRA (HL)",		1, 1, 12, &CPU::sra_hlp },			// 0x2e
	{ "SRA A",			1, 1, 4, &CPU::sra_a },				// 0x2f
	{ "SWAP B",			1, 1, 4, &CPU::swap_b },			// 0x30
	{ "SWAP C",			1, 1, 4, &CPU::swap_c },			// 0x31
	{ "SWAP D",			1, 1, 4, &CPU::swap_d },			// 0x32
	{ "SWAP E",			1, 1, 4, &CPU::swap_e },			// 0x33
	{ "SWAP H",			1, 1, 4, &CPU::swap_h },			// 0x34
	{ "SWAP L",			1, 1, 4, &CPU::swap_l },			// 0x35
	{ "SWAP (HL)",		1, 1, 12, &CPU::swap_hlp },			// 0x36
	{ "SWAP A",			1, 1, 4, &CPU::swap_a },			// 0x37
	{ "SRL B",			1, 1, 4, &CPU::srl_b },				// 0x38
	{ "SRL C",			1, 1, 4, &CPU::srl_c },				// 0x39
	{ "SRL D",			1, 1, 4, &CPU::srl_d },				// 0x3a
	{ "SRL E",			1, 1, 4, &CPU::srl_e },				// 0x3b
	{ "SRL H",			1, 1, 4, &CPU::srl_h },				// 0x3c
	{ "SRL L",			1, 1, 4, &CPU::srl_l },				// 0x3d
	{ "SRL (HL)",		1, 1, 12, &CPU::srl_hlp },			// 0x3e
	{ "SRL A",			1, 1, 4, &CPU::srl_a },				// 0x3f
	{ "BIT 0, B",		1, 1, 4, &CPU::bit_0_b },			// 0x40
	{ "BIT 0, C",		1, 1, 4, &CPU::bit_0_c },			// 0x41
	{ "BIT 0, D",		1, 1, 4, &CPU::bit_0_d },			// 0x42
	{ "BIT 0, E",		1, 1, 4, &CPU::bit_0_e },			// 0x43
	{ "BIT 0, H",		1, 1, 4, &CPU::bit_0_h },			// 0x44
	{ "BIT 0, L",		1, 1, 4, &CPU::bit_0_l },			// 0x45
	{ "BIT 0, (HL)",	1, 1, 12, &CPU::bit_0_hlp },		// 0x46
	{ "BIT 0, A",		1, 1, 4, &CPU::bit_0_a },			// 0x47
	{ "BIT 1, B",		1, 1, 4, &CPU::bit_1_b },			// 0x48
	{ "BIT 1, C",		1, 1, 4, &CPU::bit_1_c },			// 0x49
	{ "BIT 1, D",		1, 1, 4, &CPU::bit_1_d },			// 0x4a
	{ "BIT 1, E",		1, 1, 4, &CPU::bit_1_e },			// 0x4b
	{ "BIT 1, H",		1, 1, 4, &CPU::bit_1_h },			// 0x4c
	{ "BIT 1, L",		1, 1, 4, &CPU::bit_1_l },			// 0x4d
	{ "BIT 1, (HL)",	1, 1, 4, &CPU::bit_1_hlp },			// 0x4e
	{ "BIT 1, A",		1, 1, 4, &CPU::bit_1_a },			// 0x4f
	{ "BIT 2, B",		1, 1, 4, &CPU::bit_2_b },			// 0x50
	{ "BIT 2, C",		1, 1, 4, &CPU::bit_2_c },			// 0x51
	{ "BIT 2, D",		1, 1, 4, &CPU::bit_2_d },			// 0x52
	{ "BIT 2, E",		1, 1, 4,  &CPU::bit_2_e },			// 0x53
	{ "BIT 2, H",		1, 1, 4, &CPU::bit_2_h },			// 0x54
	{ "BIT 2, L",		1, 1, 4, &CPU::bit_2_l },			// 0x55
	{ "BIT 2, (HL)",	1, 1, 12, &CPU::bit_2_hlp },		// 0x56
	{ "BIT 2, A",		1, 1, 4, &CPU::bit_2_a },			// 0x57
	{ "BIT 3, B",		1, 1, 4, &CPU::bit_3_b },			// 0x58
	{ "BIT 3, C",		1, 1, 4, &CPU::bit_3_c },			// 0x59
	{ "BIT 3, D",		1, 1, 4, &CPU::bit_3_d },			// 0x5a
	{ "BIT 3, E",		1, 1, 4, &CPU::bit_3_e },			// 0x5b
	{ "BIT 3, H",		1, 1, 4, &CPU::bit_3_h },			// 0x5c
	{ "BIT 3, L",		1, 1, 4, &CPU::bit_3_l },			// 0x5d
	{ "BIT 3, (HL)",	1, 1, 12, &CPU::bit_3_hlp },		// 0x5e
	{ "BIT 3, A",		1, 1, 4, &CPU::bit_3_a },			// 0x5f
	{ "BIT 4, B",		1, 1, 4, &CPU::bit_4_b },			// 0x60
	{ "BIT 4, C",		1, 1, 4, &CPU::bit_4_c },			// 0x61
	{ "BIT 4, D",		1, 1, 4, &CPU::bit_4_d },			// 0x62
	{ "BIT 4, E",		1, 1, 4, &CPU::bit_4_e },			// 0x63
	{ "BIT 4, H",		1, 1, 4, &CPU::bit_4_h },			// 0x64
	{ "BIT 4, L",		1, 1, 4, &CPU::bit_4_l },			// 0x65
	{ "BIT 4, (HL)",	1, 1, 12, &CPU::bit_4_hlp },		// 0x66
	{ "BIT 4, A",		1, 1, 4, &CPU::bit_4_a },			// 0x67
	{ "BIT 5, B",		1, 1, 4, &CPU::bit_5_b },			// 0x68
	{ "BIT 5, C",		1, 1, 4, &CPU::bit_5_c },			// 0x69
	{ "BIT 5, D",		1, 1, 4, &CPU::bit_5_d },			// 0x6a
	{ "BIT 5, E",		1, 1, 4, &CPU::bit_5_e },			// 0x6b
	{ "BIT 6, H",		1, 1, 4, &CPU::bit_5_h },			// 0x6c
	{ "BIT 6, L",		1, 1, 4, &CPU::bit_5_l },			// 0x6d
	{ "BIT 5, (HL)",	1, 1, 12, &CPU::bit_5_hlp },		// 0x6e
	{ "BIT 5, A",		1, 1, 4, &CPU::bit_5_a },			// 0x6f
	{ "BIT 6, B",		1, 1, 4, &CPU::bit_6_b },			// 0x70
	{ "BIT 6, C",		1, 1, 4, &CPU::bit_6_c },			// 0x71
	{ "BIT 6, D",		1, 1, 4, &CPU::bit_6_d },			// 0x72
	{ "BIT 6, E",		1, 1, 4, &CPU::bit_6_e },			// 0x73
	{ "BIT 6, H",		1, 1, 4, &CPU::bit_6_h },			// 0x74
	{ "BIT 6, L",		1, 1, 4, &CPU::bit_6_l },			// 0x75
	{ "BIT 6, (HL)",	1, 1, 12, &CPU::bit_6_hlp },		// 0x76
	{ "BIT 6, A",		1, 1, 4, &CPU::bit_6_a },			// 0x77
	{ "BIT 7, B",		1, 1, 4, &CPU::bit_7_b },			// 0x78
	{ "BIT 7, C",		1, 1, 4, &CPU::bit_7_c },			// 0x79
	{ "BIT 7, D",		1, 1, 4, &CPU::bit_7_d },			// 0x7a
	{ "BIT 7, E",		1, 1, 4, &CPU::bit_7_e },			// 0x7b
	{ "BIT 7, H",		1, 1, 4, &CPU::bit_7_h },			// 0x7c
	{ "BIT 7, L",		1, 1, 4, &CPU::bit_7_l },			// 0x7d
	{ "BIT 7, (HL)",	1, 1, 12,&CPU::bit_7_hlp },			// 0x7e
	{ "BIT 7, A",		1, 1, 4,  &CPU::bit_7_a },			// 0x7f
	{ "RES 0, B",		1, 1, 4,  &CPU::res_0_b },			// 0x80
	{ "RES 0, C",		1, 1, 4,  &CPU::res_0_c },			// 0x81
	{ "RES 0, D",		1, 1, 4,  &CPU::res_0_d },			// 0x82
	{ "RES 0, E",		1, 1, 4,  &CPU::res_0_e },			// 0x83
	{ "RES 0, H",		1, 1, 4,  &CPU::res_0_h },			// 0x84
	{ "RES 0, L",		1, 1, 4, &CPU::res_0_l },			// 0x85
	{ "RES 0, (HL)",	1, 1, 12, &CPU::res_0_hlp },		// 0x86
	{ "RES 0, A",		1, 1, 4, &CPU::res_0_a },			// 0x87
	{ "RES 1, B",		1, 1, 4, &CPU::res_1_b },			// 0x88
	{ "RES 1, C",		1, 1, 4, &CPU::res_1_c },			// 0x89
	{ "RES 1, D",		1, 1, 4, &CPU::res_1_d },			// 0x8a
	{ "RES 1, E",		1, 1, 4, &CPU::res_1_e },			// 0x8b
	{ "RES 1, H",		1, 1, 4, &CPU::res_1_h },			// 0x8c
	{ "RES 1, L",		1, 1, 4, &CPU::res_1_l },			// 0x8d
	{ "RES 1, (HL)",	1, 1, 12, &CPU::res_1_hlp },		// 0x8e
	{ "RES 1, A",		1, 1, 4, &CPU::res_1_a },			// 0x8f
	{ "RES 2, B",		1, 1, 4, &CPU::res_2_b },			// 0x90
	{ "RES 2, C",		1, 1, 4, &CPU::res_2_c },			// 0x91
	{ "RES 2, D",		1, 1, 4, &CPU::res_2_d },			// 0x92
	{ "RES 2, E",		1, 1, 4, &CPU::res_2_e },			// 0x93
	{ "RES 2, H",		1, 1, 4, &CPU::res_2_h },			// 0x94
	{ "RES 2, L",		1, 1, 4, &CPU::res_2_l },			// 0x95
	{ "RES 2, (HL)",	1, 1, 12, &CPU::res_2_hlp },		// 0x96
	{ "RES 2, A",		1, 1, 4, &CPU::res_2_a },			// 0x97
	{ "RES 3, B",		1, 1, 4, &CPU::res_3_b },			// 0x98
	{ "RES 3, C",		1, 1, 4, &CPU::res_3_c },			// 0x99
	{ "RES 3, D",		1, 1, 4, &CPU::res_3_d },			// 0x9a
	{ "RES 3, E",		1, 1, 4, &CPU::res_3_e },			// 0x9b
	{ "RES 3, H",		1, 1, 4, &CPU::res_3_h },			// 0x9c
	{ "RES 3, L",		1, 1, 4, &CPU::res_3_l },			// 0x9d
	{ "RES 3, (HL)",	1, 1, 12,  &CPU::res_3_hlp },		// 0x9e
	{ "RES 3, A",		1, 1, 4, &CPU::res_3_a },			// 0x9f
	{ "RES 4, B",		1, 1, 4, &CPU::res_4_b },			// 0xa0
	{ "RES 4, C",		1, 1, 4, &CPU::res_4_c },			// 0xa1
	{ "RES 4, D",		1, 1, 4, &CPU::res_4_d },			// 0xa2
	{ "RES 4, E",		1, 1, 4, &CPU::res_4_e },			// 0xa3
	{ "RES 4, H",		1, 1, 4, &CPU::res_4_h },			// 0xa4
	{ "RES 4, L",		1, 1, 4, &CPU::res_4_l },			// 0xa5
	{ "RES 4, (HL)",	1, 1, 12, &CPU::res_4_hlp },		// 0xa6
	{ "RES 4, A",		1, 1, 4, &CPU::res_4_a },			// 0xa7
	{ "RES 5, B",		1, 1, 4, &CPU::res_5_b },			// 0xa8
	{ "RES 5, C",		1, 1, 4, &CPU::res_5_c },			// 0xa9
	{ "RES 5, D",		1, 1, 4, &CPU::res_5_d },			// 0xaa
	{ "RES 5, E",		1, 1, 4, &CPU::res_5_e },			// 0xab
	{ "RES 5, H",		1, 1, 4, &CPU::res_5_h },			// 0xac
	{ "RES 5, L",		1, 1, 4, &CPU::res_5_l },			// 0xad
	{ "RES 5, (HL)",	1, 1, 12, &CPU::res_5_hlp },		// 0xae
	{ "RES 5, A",		1, 1, 4, &CPU::res_5_a },			// 0xaf
	{ "RES 6, B",		1, 1, 4, &CPU::res_6_b },			// 0xb0
	{ "RES 6, C",		1, 1, 4, &CPU::res_6_c },			// 0xb1
	{ "RES 6, D",		1, 1, 4, &CPU::res_6_d },			// 0xb2
	{ "RES 6, E",		1, 1, 4, &CPU::res_6_e },			// 0xb3
	{ "RES 6, H",		1, 1, 4, &CPU::res_6_h },			// 0xb4
	{ "RES 6, L",		1, 1, 4, &CPU::res_6_l },			// 0xb5
	{ "RES 6, (HL)",	1, 1, 12, &CPU::res_6_hlp },		// 0xb6
	{ "RES 6, A",		1, 1, 4, &CPU::res_6_a },			// 0xb7
	{ "RES 7, B",		1, 1, 4, &CPU::res_7_b },			// 0xb8
	{ "RES 7, C",		1, 1, 4, &CPU::res_7_c },			// 0xb9
	{ "RES 7, D",		1, 1, 4, &CPU::res_7_d },			// 0xba
	{ "RES 7, E",		1, 1, 4, &CPU::res_7_e },			// 0xbb
	{ "RES 7, H",		1, 1, 4, &CPU::res_7_h },			// 0xbc
	{ "RES 7, L",		1, 1, 4, &CPU::res_7_l },			// 0xbd
	{ "RES 7, (HL)",	1, 1, 12, &CPU::res_7_hlp },		// 0xbe
	{ "RES 7, A",		1, 1, 4, &CPU::res_7_a },			// 0xbf
	{ "SET 0, B",		1, 1, 4, &CPU::set_0_b },			// 0xc0
	{ "SET 0, C",		1, 1, 4, &CPU::set_0_c },			// 0xc1
	{ "SET 0, D",		1, 1, 4, &CPU::set_0_d },			// 0xc2
	{ "SET 0, E",		1, 1, 4, &CPU::set_0_e },			// 0xc3
	{ "SET 0, H",		1, 1, 4, &CPU::set_0_h },			// 0xc4
	{ "SET 0, L",		1, 1, 4, &CPU::set_0_l },			// 0xc5
	{ "SET 0, (HL)",	1, 1, 12, &CPU::set_0_hlp },		// 0xc6
	{ "SET 0, A",		1, 1, 4, &CPU::set_0_a },			// 0xc7
	{ "SET 1, B",		1, 1, 4, &CPU::set_1_b },			// 0xc8
	{ "SET 1, C",		1, 1, 4, &CPU::set_1_c },			// 0xc9
	{ "SET 1, D",		1, 1, 4, &CPU::set_1_d },			// 0xca
	{ "SET 1, E",		1, 1, 4, &CPU::set_1_e },			// 0xcb
	{ "SET 1, H",		1, 1, 4, &CPU::set_1_h },			// 0xcc
	{ "SET 1, L",		1, 1, 4, &CPU::set_1_l },			// 0xcd
	{ "SET 1, (HL)",	1, 1, 12, &CPU::set_1_hlp },		// 0xce
	{ "SET 1, A",		1, 1, 4, &CPU::set_1_a },			// 0xcf
	{ "SET 2, B",		1, 1, 4, &CPU::set_2_b },			// 0xd0
	{ "SET 2, C",		1, 1, 4, &CPU::set_2_c },			// 0xd1
	{ "SET 2, D",		1, 1, 4, &CPU::set_2_d },			// 0xd2
	{ "SET 2, E",		1, 1, 4, &CPU::set_2_e },			// 0xd3
	{ "SET 2, H",		1, 1, 4, &CPU::set_2_h },			// 0xd4
	{ "SET 2, L",		1, 1, 4, &CPU::set_2_l },			// 0xd5
	{ "SET 2, (HL)",	1, 1, 12, &CPU::set_2_hlp },		// 0xd6
	{ "SET 2, A",		1, 1, 4, &CPU::set_2_a },			// 0xd7
	{ "SET 3, B",		1, 1, 4, &CPU::set_3_b },			// 0xd8
	{ "SET 3, C",		1, 1, 4, &CPU::set_3_c },			// 0xd9
	{ "SET 3, D",		1, 1, 4, &CPU::set_3_d },			// 0xda
	{ "SET 3, E",		1, 1, 4, &CPU::set_3_e },			// 0xdb
	{ "SET 3, H",		1, 1, 4, &CPU::set_3_h },			// 0xdc
	{ "SET 3, L",		1, 1, 4, &CPU::set_3_l },			// 0xdd
	{ "SET 3, (HL)",	1, 1, 12, &CPU::set_3_hlp },		// 0xde
	{ "SET 3, A",		1, 1, 4, &CPU::set_3_a },			// 0xdf
	{ "SET 4, B",		1, 1, 4, &CPU::set_4_b },			// 0xe0
	{ "SET 4, C",		1, 1, 4, &CPU::set_4_c },			// 0xe1
	{ "SET 4, D",		1, 1, 4, &CPU::set_4_d },			// 0xe2
	{ "SET 4, E",		1, 1, 4, &CPU::set_4_e },			// 0xe3
	{ "SET 4, H",		1, 1, 4, &CPU::set_4_h },			// 0xe4
	{ "SET 4, L",		1, 1, 4, &CPU::set_4_l },			// 0xe5
	{ "SET 4, (HL)",	1, 1, 12, &CPU::set_4_hlp },		// 0xe6
	{ "SET 4, A",		1, 1, 4, &CPU::set_4_a },			// 0xe7
	{ "SET 5, B",		1, 1, 4, &CPU::set_5_b },			// 0xe8
	{ "SET 5, C",		1, 1, 4, &CPU::set_5_c },			// 0xe9
	{ "SET 5, D",		1, 1, 4, &CPU::set_5_d },			// 0xea
	{ "SET 5, E",		1, 1, 4, &CPU::set_5_e },			// 0xeb
	{ "SET 5, H",		1, 1, 4, &CPU::set_5_h },			// 0xec
	{ "SET 5, L",		1, 1, 4, &CPU::set_5_l },			// 0xed
	{ "SET 5, (HL)",	1, 1, 12, &CPU::set_5_hlp },		// 0xee
	{ "SET 5, A",		1, 1, 4, &CPU::set_5_a },			// 0xef
	{ "SET 6, B",		1, 1, 4, &CPU::set_6_b },			// 0xf0
	{ "SET 6, C",		1, 1, 4, &CPU::set_6_c },			// 0xf1
	{ "SET 6, D",		1, 1, 4, &CPU::set_6_d },			// 0xf2
	{ "SET 6, E",		1, 1, 4, &CPU::set_6_e },			// 0xf3
	{ "SET 6, H",		1, 1, 4, &CPU::set_6_h },			// 0xf4
	{ "SET 6, L",		1, 1, 4, &CPU::set_6_l },			// 0xf5
	{ "SET 6, (HL)",	1, 1, 12,  &CPU::set_6_hlp },		// 0xf6
	{ "SET 6, A",		1, 1, 4, &CPU::set_6_a },			// 0xf7
	{ "SET 7, B",		1, 1, 4, &CPU::set_7_b },			// 0xf8
	{ "SET 7, C",		1, 1, 4, &CPU::set_7_c },			// 0xf9
	{ "SET 7, D",		1, 1, 4, &CPU::set_7_d },			// 0xfa
	{ "SET 7, E",		1, 1, 4, &CPU::set_7_e },			// 0xfb
	{ "SET 7, H",		1, 1, 4, &CPU::set_7_h },			// 0xfc
	{ "SET 7, L",		1, 1, 4, &CPU::set_7_l },			// 0xfd
	{ "SET 7, (HL)",	1, 1, 12, &CPU::set_7_hlp },		// 0xfe
	{ "SET 7, A",		1, 1, 4, &CPU::set_7_a }			// 0xff
};
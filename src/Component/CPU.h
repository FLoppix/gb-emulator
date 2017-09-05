#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <time.h>
#include <chrono>

/* For testing/debuging */
#include <bitset>

#include "Register.h"
#include "Instruction.h"
#include "ExtInstruction.h"
#include "Memory.h"
#include "Timer.h"
#include "GPU.h"

#define CLOCK_RATE 4194304

using std::string;

class CPU {
private:
	/* Attributes */
	registers reg;
	Memory* mem;
	Timer* timer;
	GPU* gpu;
	uint8_t ext;
	bool interruptsEnabled;
	bool isHalt;

	std::chrono::steady_clock::time_point start;
	unsigned long globalTicks;


public:
	/* Constrcutor */
	CPU(Memory* m);

	/* Main Methods */
	void run();
	void disassemble();
	void readRomHeader();
	string getRomName();

/* Util Methods */

	void exec();
	void wait();

	/* Interrupt Methods */
	void enableInterrupts();
	void disableInterrupts();
	void enableRequestInterrupt(uint8_t type);
	void disableRequestInterrupt(uint8_t type);

	void triggerRequestInterrupt(uint8_t type);
	void resetRequestInterrupt(uint8_t type);


	bool isGlobalInterrupt();
	bool isRequestInterrupt(uint8_t type);
	bool isRequestInterruptEnabled(uint8_t type);
	bool isRequestInterruptTriggered(uint8_t type);
	bool isAnyInterrupt();
	bool isAnyInterruptTriggered();


	void handleInterrupts();

	void manageMemory();

	/* Register Methods */
	void setZeroFlag(bool zeroFlag);	
	void setSubtractFlag(bool subtractFlag);
	void setHalfCarryFlag(bool halfCarryFlag);
	void setCarryFlag(bool carryFlag);
	void clearZeroFlag();
	void clearSubtractFlag();
	void clearCarryFlag();
	void clearHalfCarryFlag();

	bool isZeroFlag();
	bool isSubstractFlag();
	bool isHalfCarryFlag();
	bool isCarryFlag();


	/* Debug Methods */
	void debug();
	void dumpInstr(uint16_t opcode);
	void dump();
	const char* printRequestInterrupt(uint8_t type);
	void memdump(uint16_t start, uint16_t end);
	void memdumpr(uint16_t value, string name);
	void memdumpc(uint32_t start, uint32_t end);
	void stackdump();

	/* Test Methods */
	void testEndianness();
	void testStatusFlags();

	/* Instruction Methods */
	uint8_t nop();
	uint8_t ld_bc_d16();
	uint8_t ld_bcp_a();
	uint8_t inc_bc();
	uint8_t inc_b();
	uint8_t dec_b();
	uint8_t ld_b_d8();
	uint8_t rlca();
	uint8_t ld_a16p_sp();
	uint8_t add_hl_bc();
	uint8_t ld_a_bcp();
	uint8_t dec_bc();
	uint8_t inc_c();
	uint8_t dec_c();
	uint8_t ld_c_d8();
	uint8_t rrca();
	uint8_t stop();
	uint8_t ld_de_d16();
	uint8_t ld_dep_a();
	uint8_t inc_de();
	uint8_t inc_d();
	uint8_t dec_d();
	uint8_t ld_d_d8();
	uint8_t rla();
	uint8_t jr_r8();
	uint8_t add_hl_de();
	uint8_t ld_a_dep();
	uint8_t dec_de();
	uint8_t inc_e();
	uint8_t dec_e();
	uint8_t ld_e_d8();
	uint8_t rra();
	uint8_t jr_nz_r8();
	uint8_t ld_hl_d16();
	uint8_t ldi_hlp_a();
	uint8_t inc_hl();
	uint8_t inc_h();
	uint8_t dec_h();
	uint8_t ld_h_d8();
	uint8_t daa();
	uint8_t jr_z_r8();
	uint8_t add_hl_hl();
	uint8_t ldi_a_hlp();
	uint8_t dec_hl();
	uint8_t inc_l();
	uint8_t dec_l();
	uint8_t ld_l_d8();
	uint8_t cpl();
	uint8_t jr_nc_r8();
	uint8_t ld_sp_d16();
	uint8_t ldd_hlp_a();
	uint8_t inc_sp();
	uint8_t inc_hlp();
	uint8_t dec_hlp();
	uint8_t ld_hlp_d8();
	uint8_t scf();
	uint8_t jr_c_r8();
	uint8_t add_hl_sp();
	uint8_t ldd_a_hlp();
	uint8_t dec_sp();
	uint8_t inc_a();
	uint8_t dec_a();
	uint8_t ld_a_d8();
	uint8_t ccf();
	uint8_t ld_b_c();
	uint8_t ld_b_d();
	uint8_t ld_b_e();
	uint8_t ld_b_h();
	uint8_t ld_b_l();
	uint8_t ld_b_hlp();
	uint8_t ld_b_a();
	uint8_t ld_c_b();
	uint8_t ld_c_d();
	uint8_t ld_c_e();
	uint8_t ld_c_h();
	uint8_t ld_c_l();
	uint8_t ld_c_hlp();
	uint8_t ld_c_a();
	uint8_t ld_d_b();
	uint8_t ld_d_c();
	uint8_t ld_d_e();
	uint8_t ld_d_h();
	uint8_t ld_d_l();
	uint8_t ld_d_hlp();
	uint8_t ld_d_a();
	uint8_t ld_e_b();
	uint8_t ld_e_c();
	uint8_t ld_e_d();
	uint8_t ld_e_h();
	uint8_t ld_e_l();
	uint8_t ld_e_hlp();
	uint8_t ld_e_a();
	uint8_t ld_h_b();
	uint8_t ld_h_c();
	uint8_t ld_h_d();
	uint8_t ld_h_e();
	uint8_t ld_h_l();
	uint8_t ld_h_hlp();
	uint8_t ld_h_a();
	uint8_t ld_l_b();
	uint8_t ld_l_c();
	uint8_t ld_l_d();
	uint8_t ld_l_e();
	uint8_t ld_l_h();
	uint8_t ld_l_hlp();
	uint8_t ld_l_a();
	uint8_t ld_hlp_b();
	uint8_t ld_hlp_c();
	uint8_t ld_hlp_d();
	uint8_t ld_hlp_e();
	uint8_t ld_hlp_h();
	uint8_t ld_hlp_l();
	uint8_t halt();
	uint8_t ld_hlp_a();
	uint8_t ld_a_b();
	uint8_t ld_a_c();
	uint8_t ld_a_d();
	uint8_t ld_a_e();
	uint8_t ld_a_h();
	uint8_t ld_a_l();
	uint8_t ld_a_hlp();
	uint8_t add_a_b();
	uint8_t add_a_c();
	uint8_t add_a_d();
	uint8_t add_a_e();
	uint8_t add_a_h();
	uint8_t add_a_l();
	uint8_t add_a_hlp();
	uint8_t add_a_a();
	uint8_t adc_a_b();
	uint8_t adc_a_c();
	uint8_t adc_a_d();
	uint8_t adc_a_e();
	uint8_t adc_a_h();
	uint8_t adc_a_l();
	uint8_t adc_a_hlp();
	uint8_t adc_a_a();
	uint8_t sub_a_b();
	uint8_t sub_a_c();
	uint8_t sub_a_d();
	uint8_t sub_a_e();
	uint8_t sub_a_h();
	uint8_t sub_a_l();
	uint8_t sub_a_hlp();
	uint8_t sub_a_a();
	uint8_t sbc_a_b();
	uint8_t sbc_a_c();
	uint8_t sbc_a_d();
	uint8_t sbc_a_e();
	uint8_t sbc_a_h();
	uint8_t sbc_a_l();
	uint8_t sbc_a_hlp();
	uint8_t sbc_a_a();
	uint8_t and_a_b();
	uint8_t and_a_c();
	uint8_t and_a_d();
	uint8_t and_a_e();
	uint8_t and_a_h();
	uint8_t and_a_l();
	uint8_t and_a_hlp();
	uint8_t and_a_a();
	uint8_t xor_a_b();
	uint8_t xor_a_c();
	uint8_t xor_a_d();
	uint8_t xor_a_e();
	uint8_t xor_a_h();
	uint8_t xor_a_l();
	uint8_t xor_a_hlp();
	uint8_t xor_a_a();
	uint8_t or_a_b();
	uint8_t or_a_c();
	uint8_t or_a_d();
	uint8_t or_a_e();
	uint8_t or_a_h();
	uint8_t or_a_l();
	uint8_t or_a_hlp();
	uint8_t or_a_a();
	uint8_t cp_a_b();
	uint8_t cp_a_c();
	uint8_t cp_a_d();
	uint8_t cp_a_e();
	uint8_t cp_a_h();
	uint8_t cp_a_l();
	uint8_t cp_hlp();
	uint8_t cp_a_a();
	uint8_t ret_nz();
	uint8_t pop_bc();
	uint8_t jp_nz_a16();
	uint8_t jp_a16();
	uint8_t call_nz_a16();
	uint8_t push_bc();
	uint8_t add_a_d8();
	uint8_t rst_00h();
	uint8_t ret_z();
	uint8_t ret();
	uint8_t jp_z_a16();
	uint8_t cb();
	uint8_t call_z_a16();
	uint8_t call_a16();
	uint8_t adc_d8();
	uint8_t rst_08h();
	uint8_t ret_nc();
	uint8_t pop_de();
	uint8_t jp_nc_a16();
	uint8_t undefined();
	uint8_t call_nc_a16();
	uint8_t push_de();
	uint8_t sub_a_d8();
	uint8_t rst_10h();
	uint8_t ret_c();
	uint8_t reti();
	uint8_t jp_c_a16();
	uint8_t call_c_a16();
	uint8_t sbc_a_d8();
	uint8_t rst_18h();
	uint8_t ldh_a8_ap();
	uint8_t pop_hl();
	uint8_t ldh_cp_a();
	uint8_t push_hl();
	uint8_t and_a_d8();
	uint8_t rst_20h();
	uint8_t add_sp_r8();
	uint8_t jp_hlp();
	uint8_t ld_a16p_a();
	uint8_t xor_a_d8();
	uint8_t rst_28h();
	uint8_t ldh_a_a8p();
	uint8_t pop_af();
	uint8_t ldh_a_cp();
	uint8_t di();
	uint8_t push_af();
	uint8_t or_a_d8();
	uint8_t rst_30h();
	uint8_t ld_hl_sp_r8();
	uint8_t ld_sp_hl();
	uint8_t ld_a_a16p();
	uint8_t ei();
	uint8_t cp_d8();
	uint8_t rst_38h();

	/* Extended Instruction Methods */
	uint8_t rlc_b();
	uint8_t rlc_c();
	uint8_t rlc_d();
	uint8_t rlc_e();
	uint8_t rlc_h(); 
	uint8_t rlc_l(); 
	uint8_t rlc_hlp();
	uint8_t rlc_a();
	uint8_t rrc_b();
	uint8_t rrc_c();
	uint8_t rrc_d(); 
	uint8_t rrc_e();
	uint8_t rrc_h();
	uint8_t rrc_l();
	uint8_t rrc_hlp();	
	uint8_t rrc_a(); 
	uint8_t rl_b();
	uint8_t rl_c(); 
	uint8_t rl_d();
	uint8_t rl_e(); 
	uint8_t rl_h();
	uint8_t rl_l();
	uint8_t rl_hlp();
	uint8_t rl_a();
	uint8_t rr_b();
	uint8_t rr_c();
	uint8_t rr_d(); 
	uint8_t rr_e();
	uint8_t rr_h();
	uint8_t rr_l();
	uint8_t rr_hlp(); 
	uint8_t rr_a();
	uint8_t sla_b();
	uint8_t sla_c();
	uint8_t sla_d();
	uint8_t sla_e();
	uint8_t sla_h();
	uint8_t sla_l();
	uint8_t sla_hlp();
	uint8_t sla_a();
	uint8_t sra_b(); 
	uint8_t sra_c();
	uint8_t sra_d();
	uint8_t sra_e(); 
	uint8_t sra_h(); 
	uint8_t sra_l();
	uint8_t sra_hlp();
	uint8_t sra_a(); 
	uint8_t swap_b();
	uint8_t swap_c();
	uint8_t swap_d();
	uint8_t swap_e();
	uint8_t swap_h();
	uint8_t swap_l();
	uint8_t swap_hlp();
	uint8_t swap_a();
	uint8_t srl_b();
	uint8_t srl_c();
	uint8_t srl_d(); 
	uint8_t srl_e();
	uint8_t srl_h();
	uint8_t srl_l(); 
	uint8_t srl_hlp();
	uint8_t srl_a();
	uint8_t bit_0_b();
	uint8_t bit_0_c();
	uint8_t bit_0_d();
	uint8_t bit_0_e(); 
	uint8_t bit_0_h();
	uint8_t bit_0_l();
	uint8_t bit_0_hlp();
	uint8_t bit_0_a();
	uint8_t bit_1_b();
	uint8_t bit_1_c();
	uint8_t bit_1_d();
	uint8_t bit_1_e();
	uint8_t bit_1_h();
	uint8_t bit_1_l();
	uint8_t bit_1_hlp();
	uint8_t bit_1_a();
	uint8_t bit_2_b();
	uint8_t bit_2_c();
	uint8_t bit_2_d();
	uint8_t bit_2_e();
	uint8_t bit_2_h();
	uint8_t bit_2_l();
	uint8_t bit_2_hlp();
	uint8_t bit_2_a();
	uint8_t bit_3_b();
	uint8_t bit_3_c();
	uint8_t bit_3_d();
	uint8_t bit_3_e();
	uint8_t bit_3_h();
	uint8_t bit_3_l();
	uint8_t bit_3_hlp();
	uint8_t bit_3_a();
	uint8_t bit_4_b();
	uint8_t bit_4_c();
	uint8_t bit_4_d();
	uint8_t bit_4_e();
	uint8_t bit_4_h();
	uint8_t bit_4_l();
	uint8_t bit_4_hlp();
	uint8_t bit_4_a();
	uint8_t bit_5_b();
	uint8_t bit_5_c();
	uint8_t bit_5_d();
	uint8_t bit_5_e();
	uint8_t bit_5_h();
	uint8_t bit_5_l();
	uint8_t bit_5_hlp();
	uint8_t bit_5_a();
	uint8_t bit_6_b();
	uint8_t bit_6_c();
	uint8_t bit_6_d();
	uint8_t bit_6_e();
	uint8_t bit_6_h();
	uint8_t bit_6_l();
	uint8_t bit_6_hlp();
	uint8_t bit_6_a();
	uint8_t bit_7_b();
	uint8_t bit_7_c();
	uint8_t bit_7_d();
	uint8_t bit_7_e();
	uint8_t bit_7_h();
	uint8_t bit_7_l();
	uint8_t bit_7_hlp();
	uint8_t bit_7_a();
	uint8_t res_0_b();
	uint8_t res_0_c();
	uint8_t res_0_d();
	uint8_t res_0_e();
	uint8_t res_0_h();
	uint8_t res_0_l();
	uint8_t res_0_hlp();
	uint8_t res_0_a();
	uint8_t res_1_b();
	uint8_t res_1_c();
	uint8_t res_1_d();
	uint8_t res_1_e();
	uint8_t res_1_h();
	uint8_t res_1_l();
	uint8_t res_1_hlp();
	uint8_t res_1_a();
	uint8_t res_2_b();
	uint8_t res_2_c();
	uint8_t res_2_d();
	uint8_t res_2_e();
	uint8_t res_2_h();
	uint8_t res_2_l();
	uint8_t res_2_hlp();
	uint8_t res_2_a();
	uint8_t res_3_b();
	uint8_t res_3_c();
	uint8_t res_3_d();
	uint8_t res_3_e();
	uint8_t res_3_h();
	uint8_t res_3_l();
	uint8_t res_3_hlp();
	uint8_t res_3_a();
	uint8_t res_4_b();
	uint8_t res_4_c();
	uint8_t res_4_d();
	uint8_t res_4_e();
	uint8_t res_4_h();
	uint8_t res_4_l();
	uint8_t res_4_hlp();
	uint8_t res_4_a();
	uint8_t res_5_b();
	uint8_t res_5_c();
	uint8_t res_5_d();
	uint8_t res_5_e();
	uint8_t res_5_h();
	uint8_t res_5_l();
	uint8_t res_5_hlp();
	uint8_t res_5_a();
	uint8_t res_6_b();
	uint8_t res_6_c();
	uint8_t res_6_d();
	uint8_t res_6_e();
	uint8_t res_6_h();
	uint8_t res_6_l();
	uint8_t res_6_hlp();
	uint8_t res_6_a();
	uint8_t res_7_b();
	uint8_t res_7_c();
	uint8_t res_7_d();
	uint8_t res_7_e();
	uint8_t res_7_h();
	uint8_t res_7_l();
	uint8_t res_7_hlp();
	uint8_t res_7_a();
	uint8_t set_0_b();
	uint8_t set_0_c();
	uint8_t set_0_d();
	uint8_t set_0_e();
	uint8_t set_0_h();
	uint8_t set_0_l();
	uint8_t set_0_hlp();
	uint8_t set_0_a();
	uint8_t set_1_b();
	uint8_t set_1_c();
	uint8_t set_1_d();
	uint8_t set_1_e();
	uint8_t set_1_h();
	uint8_t set_1_l();
	uint8_t set_1_hlp();
	uint8_t set_1_a();
	uint8_t set_2_b();
	uint8_t set_2_c();
	uint8_t set_2_d();
	uint8_t set_2_e();
	uint8_t set_2_h();
	uint8_t set_2_l();
	uint8_t set_2_hlp();
	uint8_t set_2_a();
	uint8_t set_3_b();
	uint8_t set_3_c();
	uint8_t set_3_d();
	uint8_t set_3_e();
	uint8_t set_3_h();
	uint8_t set_3_l();
	uint8_t set_3_hlp();
	uint8_t set_3_a();
	uint8_t set_4_b();
	uint8_t set_4_c();
	uint8_t set_4_d();
	uint8_t set_4_e();
	uint8_t set_4_h();
	uint8_t set_4_l();
	uint8_t set_4_hlp();
	uint8_t set_4_a();
	uint8_t set_5_b();
	uint8_t set_5_c();
	uint8_t set_5_d();
	uint8_t set_5_e();
	uint8_t set_5_h();
	uint8_t set_5_l();
	uint8_t set_5_hlp();
	uint8_t set_5_a();
	uint8_t set_6_b();
	uint8_t set_6_c();
	uint8_t set_6_d();
	uint8_t set_6_e();
	uint8_t set_6_h();
	uint8_t set_6_l();
	uint8_t set_6_hlp();
	uint8_t set_6_a();
	uint8_t set_7_b();
	uint8_t set_7_c();
	uint8_t set_7_d();
	uint8_t set_7_e();
	uint8_t set_7_h();
	uint8_t set_7_l();
	uint8_t set_7_hlp();
	uint8_t set_7_a();

	/* CPU Arithmetic Helper Functions  */
	void inc8(uint8_t* r);
	void inc16(uint16_t* r);
	void dec8(uint8_t* r);
	void dec16(uint16_t* r);
	void add8(uint8_t value);
	void adc8(uint8_t value);
	void sub8(uint8_t value);
	void sbc8(uint8_t value);
	void cp8(uint8_t value);
	void add16(uint16_t value);

	/* CPU Bit Helper Functions  */
	uint8_t swap(uint8_t value);
	void and8(uint8_t value);
	void or8(uint8_t value);
	void xor8(uint8_t value);
	void bit(uint8_t bit, uint8_t value);
	void set(uint8_t bit, uint8_t* r);
	void set(uint8_t bit, uint16_t addr);
	void res(uint8_t bit, uint8_t* r);
	void res(uint8_t bit, uint16_t addr);

	/* CPU Rotate and Shift Helper Functions */
	void rl(uint8_t* r);
	void rl(uint16_t addr);
	void rlc(uint8_t* r);
	void rlc(uint16_t addr);
	void rr(uint8_t* r);
	void rr(uint16_t addr);
	void rrc(uint8_t* r);
	void rrc(uint16_t addr);
	void sla(uint8_t* r);
	void sla(uint16_t addr);
	void sra(uint8_t* r);
	void sra(uint16_t addr);
	void srl(uint8_t* r);
	void srl(uint16_t addr);

	/* CPU Stack Helper Functions  */
	void push(uint16_t value);
	uint16_t pop();


};

#endif /* CPU_H */
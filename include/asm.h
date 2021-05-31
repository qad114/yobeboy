#ifndef ASM_H
#define ASM_H

#include <stdint.h>
#include "cpu.h"
#include "memory.h"

void ASM_ADC_A_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_ADC_A_n(CPU* cpu, uint8_t* reg);
void ASM_ADD_A_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_ADD_A_n(CPU* cpu, uint8_t* reg);
void ASM_ADD_HL_n(CPU* cpu, uint16_t* reg);
void ASM_ADD_SP_n(CPU* cpu, Memory* mem);
void ASM_AND_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_AND_n(CPU* cpu, uint8_t* reg);
void ASM_AND_n_byVal(CPU* cpu, Memory* mem);
void ASM_BIT_b_m(CPU* cpu, Memory* mem, int bit, uint16_t address);
void ASM_BIT_b_r(CPU* cpu, int bit, uint8_t* reg);
int  ASM_CALL_cc_nn(CPU* cpu, Memory* mem, int ccCode);
void ASM_CALL_nn(CPU* cpu, Memory* mem);
void ASM_CCF(CPU* cpu);
void ASM_CP_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_CP_n(CPU* cpu, uint8_t* reg);
void ASM_CP_n_byVal(CPU* cpu, Memory* mem);
void ASM_CPL(CPU* cpu);
void ASM_DAA(CPU* cpu);
void ASM_DEC_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_DEC_n(CPU* cpu, uint8_t* reg);
void ASM_DEC_nn(CPU* cpu, uint16_t* reg);
void ASM_INC_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_INC_n(CPU* cpu, uint8_t* reg);
void ASM_INC_nn(CPU* cpu, uint16_t* reg);
int  ASM_JP_cc_nn(CPU* cpu, Memory* mem, int ccCode);
void ASM_JP_HL(CPU* cpu);
void ASM_JP_nn(CPU* cpu, Memory* mem);
int  ASM_JR_cc_n(CPU* cpu, Memory* mem, int ccCode);
void ASM_JR_n(CPU* cpu, Memory* mem);
void ASM_LD_A_n(CPU* cpu, uint8_t* reg);
void ASM_LD_A_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_LD_C_A(CPU* cpu, Memory* mem);
void ASM_LD_n_A(CPU* cpu, uint8_t* reg);
void ASM_LD_m_A(CPU* cpu, Memory* mem, uint16_t address);
void ASM_LD_m1_m2(CPU* cpu, Memory* mem, uint16_t address1, uint16_t address2);
void ASM_LD_nn_n(CPU* cpu, Memory* mem, uint8_t* reg);
void ASM_LD_nn_SP(CPU* cpu, Memory* mem);
void ASM_LD_n_nn(CPU* cpu, Memory* mem, uint16_t* reg);
void ASM_LD_r1_r2(CPU* cpu, uint8_t* reg1, uint8_t* reg2);
void ASM_LD_r1_m(CPU* cpu, Memory* mem, uint8_t* reg1, uint16_t address);
void ASM_LD_m_r2(CPU* cpu, Memory* mem, uint16_t address, uint8_t* reg2);
void ASM_LD_SP_HL(CPU* cpu);
void ASM_LDD_HL_A(CPU* cpu, Memory* mem);
void ASM_LDH_A_n(CPU* cpu, Memory* mem);
void ASM_LDH_n_A(CPU* cpu, Memory* mem);
void ASM_LDHL_SP_n(CPU* cpu, Memory* mem);
void ASM_LDI_A_HL(CPU* cpu, Memory* mem);
void ASM_LDI_HL_A(CPU* cpu, Memory* mem);
void ASM_NOP(CPU* cpu);
void ASM_OR_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_OR_n(CPU* cpu, uint8_t* reg);
void ASM_POP_nn(CPU* cpu, Memory* mem, uint16_t* reg);
void ASM_PUSH_nn(CPU* cpu, Memory* mem, uint16_t* reg);
void ASM_RES_b_m(CPU* cpu, Memory* mem, int bit, uint16_t address);
void ASM_RES_b_r(CPU* cpu, int bit, uint8_t* reg);
void ASM_RET(CPU* cpu, Memory* mem);
int  ASM_RET_cc(CPU* cpu, Memory* mem, int ccCode);
void ASM_RETI(CPU* cpu, Memory* mem);
void ASM_RL_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_RL_n(CPU* cpu, uint8_t* reg);
void ASM_RLA(CPU* cpu);
void ASM_RLC_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_RLC_n(CPU* cpu, uint8_t* reg);
void ASM_RLCA(CPU* cpu);
void ASM_RR_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_RR_n(CPU* cpu, uint8_t* reg);
void ASM_RRA(CPU* cpu);
void ASM_RRC_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_RRC_n(CPU* cpu, uint8_t* reg);
void ASM_RRCA(CPU* cpu);
void ASM_RST_n(CPU* cpu, Memory* mem, uint8_t address);
void ASM_SBC_A_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_SBC_A_n(CPU* cpu, uint8_t* reg);
void ASM_SCF(CPU* cpu);
void ASM_SLA_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_SLA_n(CPU* cpu, uint8_t* reg);
void ASM_SET_b_m(CPU* cpu, Memory* mem, int bit, uint16_t address);
void ASM_SET_b_r(CPU* cpu, int bit, uint8_t* reg);
void ASM_SRA_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_SRA_n(CPU* cpu, uint8_t* reg);
void ASM_SRL_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_SRL_n(CPU* cpu, uint8_t* reg);
void ASM_SUB_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_SUB_n(CPU* cpu, uint8_t* reg);
void ASM_SWAP_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_SWAP_n(CPU* cpu, uint8_t* reg);
void ASM_XOR_m(CPU* cpu, Memory* mem, uint16_t address);
void ASM_XOR_n(CPU* cpu, uint8_t* reg);

#endif
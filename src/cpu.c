#include <stdio.h>
#include <stdlib.h>

#include "common/bitwise.h"
#include "asm.h"
#include "constants.h"
#include "cpu.h"
#include "gpu.h"
#include "joypad.h"
#include "memory.h"
#include "timer.h"

int mCycleTimer = 0;

int CPU_getFlagZ(CPU* cpu) { return getBit(cpu->F, 7); }
int CPU_getFlagN(CPU* cpu) { return getBit(cpu->F, 6); }
int CPU_getFlagH(CPU* cpu) { return getBit(cpu->F, 5); }
int CPU_getFlagC(CPU* cpu) { return getBit(cpu->F, 4); }

void CPU_setFlagZ(CPU* cpu, int value) { cpu->F = setBit(cpu->F, 7, value); }
void CPU_setFlagN(CPU* cpu, int value) { cpu->F = setBit(cpu->F, 6, value); }
void CPU_setFlagH(CPU* cpu, int value) { cpu->F = setBit(cpu->F, 5, value); }
void CPU_setFlagC(CPU* cpu, int value) { cpu->F = setBit(cpu->F, 4, value); }

void CPU_init(CPU* cpu) {
    // Init everything
    cpu->A = 0x01; cpu->F = 0xB0;
    cpu->B = 0x00; cpu->C = 0x13;
    cpu->D = 0x00; cpu->E = 0xD8;
    cpu->H = 0x01; cpu->L = 0x4D;
    /*cpu->A = 0x11; cpu->F = 0x80;
    cpu->B = 0x00; cpu->C = 0x00;
    cpu->D = 0xFF; cpu->E = 0x56;
    cpu->H = 0x00; cpu->L = 0x0D;*/
    cpu->SP = 0xFFFE;
    cpu->PC = 0x100; // address where code starts
    cpu->IME = 0;
    cpu->opcode = 0;
}

void CPU_destroy(CPU* cpu) {
    free(cpu);
    cpu = NULL;
}

int CPU_emulateCycle(CPU* cpu, GPU* gpu, Memory* mem, Timer* timer, Joypad* joy) {
    // Fetch the next byte/opcode
    cpu->opcode = MEM_getByte(mem, cpu->PC);
    #ifdef DISABLE_GRAPHICS
    if (mCycleTimer == 0) {
        printf("%04x: %02x - %d %d %d %d - ", cpu->PC, cpu->opcode, CPU_getFlagZ(cpu), CPU_getFlagN(cpu), CPU_getFlagH(cpu), CPU_getFlagC(cpu));
        printf("%02x%02x %02x%02x %02x%02x %02x%02x %04x %02x %02x %02x %02x %d ", cpu->A, cpu->F, cpu->B, cpu->C, cpu->D, cpu->E, cpu->H, cpu->L, cpu->SP, MEM_getByte(mem, REG_DIV), MEM_getByte(mem, REG_TIMA), MEM_getByte(mem, REG_TMA), MEM_getByte(mem, REG_TAC), timer->timaCounter);
        for (uint16_t i = 0xA000; i <= 0xA00F; ++i) printf("%02x", MEM_getByte(mem, i)); printf("\n");
    }
    #endif

    if (mCycleTimer > 0) {
        --mCycleTimer;
    } else {
        switch (cpu->opcode) {
            case 0x00: // NOP (4)
                ASM_NOP(cpu);
                break;

            case 0x01: // LD BC, nn (12)
                ASM_LD_n_nn(cpu, mem, &(cpu->BC));
                mCycleTimer = 2;
                break;

            case 0x02: // LD (BC), A (8)
                ASM_LD_m_A(cpu, mem, cpu->BC);
                mCycleTimer = 1;
                break;

            case 0x03: // INC BC (8)
                ASM_INC_nn(cpu, &(cpu->BC));
                mCycleTimer = 1;
                break;

            case 0x04: // INC B (4)
                ASM_INC_n(cpu, &(cpu->B));
                break;

            case 0x05: // DEC B (4)
                ASM_DEC_n(cpu, &(cpu->B));
                break;

            case 0x06: // LD B, n (8)
                ASM_LD_nn_n(cpu, mem, &(cpu->B));
                mCycleTimer = 1;
                break;

            case 0x07: // RLCA (4)
                ASM_RLCA(cpu);
                break;

            case 0x08: // LD (nn), SP (20)
                ASM_LD_nn_SP(cpu, mem);
                mCycleTimer = 4;
                break;

            case 0x09: // ADD HL, BC (8)
                ASM_ADD_HL_n(cpu, &(cpu->BC));
                mCycleTimer = 1;
                break;

            case 0x0A: // LD A, (BC) (8)
                ASM_LD_A_m(cpu, mem, cpu->BC);
                mCycleTimer = 1;
                break;

            case 0x0B: // DEC BC (8)
                ASM_DEC_nn(cpu, &(cpu->BC));
                mCycleTimer = 1;
                break;

            case 0x0C: // INC C (4)
                ASM_INC_n(cpu, &(cpu->C));
                break;

            case 0x0D: // DEC C (4)
                ASM_DEC_n(cpu, &(cpu->C));
                break;

            case 0xD2: // JP NC, nn (12/16)
                if (ASM_JP_cc_nn(cpu, mem, PARAM_CC_NC)) {
                    mCycleTimer = 3;
                } else {
                    mCycleTimer = 2;
                }
                break;

            case 0x0E: // LD C, n (8)
                ASM_LD_nn_n(cpu, mem, &(cpu->C));
                mCycleTimer = 1;
                break;

            case 0x0F: // RRCA (4)
                ASM_RRCA(cpu);
                break;

            case 0x10: // STOP (4)
                //printf("%x %x\n", MEM_getByte(mem, cpu->PC), MEM_getByte(mem, cpu->PC + 1));
                cpu->PC += 1;
                break;

            case 0x11: // LD DE, nn (12)
                ASM_LD_n_nn(cpu, mem, &(cpu->DE));
                mCycleTimer = 2;
                break;

            case 0x12: // LD (DE), A (8)
                ASM_LD_m_A(cpu, mem, cpu->DE);
                mCycleTimer = 1;
                break;

            case 0x13: // INC DE (8)
                ASM_INC_nn(cpu, &(cpu->DE));
                mCycleTimer = 1;
                break;

            case 0x14: // INC D (4)
                ASM_INC_n(cpu, &(cpu->D));
                break;

            case 0x15: // DEC D (4)
                ASM_DEC_n(cpu, &(cpu->D));
                break;

            case 0x16: // LD D, n (8)
                ASM_LD_nn_n(cpu, mem, &(cpu->D));
                mCycleTimer = 1;
                break;

            case 0x17: // RLA (4)
                ASM_RLA(cpu);
                break;

            case 0x18: // JR n (12)
                ASM_JR_n(cpu, mem);
                mCycleTimer = 2;
                break;

            case 0x19: // ADD HL, DE (8)
                ASM_ADD_HL_n(cpu, &(cpu->DE));
                mCycleTimer = 1;
                break;

            case 0x1A: // LD A, (DE) (8)
                ASM_LD_A_m(cpu, mem, cpu->DE);
                mCycleTimer = 1;
                break;

            case 0x1B: // DEC DE (8)
                ASM_DEC_nn(cpu, &(cpu->DE));
                mCycleTimer = 1;
                break;

            case 0x1C: // INC E (4)
                ASM_INC_n(cpu, &(cpu->E));
                break;

            case 0x1D: // DEC E (4)
                ASM_DEC_n(cpu, &(cpu->E));
                break;

            case 0x1E: // LD E, n (8)
                ASM_LD_nn_n(cpu, mem, &(cpu->E));
                mCycleTimer = 1;
                break;

            case 0x1F: // RRA (4)
                ASM_RRA(cpu);
                break;

            case 0x20: // JR NZ, n (8/12)
                if (ASM_JR_cc_n(cpu, mem, PARAM_CC_NZ)) {
                    mCycleTimer = 2;
                } else {
                    mCycleTimer = 1;
                }
                //ASM_JR_cc_n(cpu, mem, PARAM_CC_NZ);
                //mCycleTimer = 1;
                break;

            case 0x21: // LD HL, nn (12)
                ASM_LD_n_nn(cpu, mem, &(cpu->HL));
                mCycleTimer = 2;
                break;

            case 0x22: // LDI (HL), A (8)
                ASM_LDI_HL_A(cpu, mem);
                mCycleTimer = 1;
                break;

            case 0x23: // INC HL (8)
                ASM_INC_nn(cpu, &(cpu->HL));
                mCycleTimer = 1;
                break;

            case 0x24: // INC H (4)
                ASM_INC_n(cpu, &(cpu->H));
                break;

            case 0x25: // DEC H (4)
                ASM_DEC_n(cpu, &(cpu->H));
                break;

            case 0x26: // LD H, n (8)
                ASM_LD_r1_m(cpu, mem, &(cpu->H), cpu->PC + 1);
                cpu->PC += 1;
                mCycleTimer = 1;
                break;

            case 0x27: // DAA (4)
                ASM_DAA(cpu);
                break;

            case 0x28: // JR Z, n (8/12)
                if (ASM_JR_cc_n(cpu, mem, PARAM_CC_Z)) {
                    mCycleTimer = 2;
                } else {
                    mCycleTimer = 1;
                }
                break;

            case 0x29: // ADD HL, HL (8)
                ASM_ADD_HL_n(cpu, &(cpu->HL));
                mCycleTimer = 1;
                break;

            case 0x2A: // LDI A, (HL) (8)
                ASM_LDI_A_HL(cpu, mem);
                mCycleTimer = 1;
                break;

            case 0x2B: // DEC HL (8)
                ASM_DEC_nn(cpu, &(cpu->HL));
                mCycleTimer = 1;
                break;

            case 0x2C: // INC L (4)
                ASM_INC_n(cpu, &(cpu->L));
                break;

            case 0x2D: // DEC L
                ASM_DEC_n(cpu, &(cpu->L));
                break;

            case 0x2E: // LD L, n (8)
                ASM_LD_nn_n(cpu, mem, &(cpu->L));
                mCycleTimer = 1;
                break;

            case 0x2F: // CPL (4)
                ASM_CPL(cpu);
                break;

            case 0x30: // JR NC, n (8/12)
                if (ASM_JR_cc_n(cpu, mem, PARAM_CC_NC)) {
                    mCycleTimer = 2;
                } else {
                    mCycleTimer = 1;
                }
                break;

            case 0x31: // LD SP, nn (12)
                ASM_LD_n_nn(cpu, mem, &(cpu->SP));
                mCycleTimer = 2;
                break;

            case 0x32: // LDD (HL), A (8)
                ASM_LDD_HL_A(cpu, mem);
                mCycleTimer = 1;
                break;

            case 0x33: // INC SP (8)
                ASM_INC_nn(cpu, &(cpu->SP));
                mCycleTimer = 1;
                break;

            case 0x34: // INC (HL) (12)
                ASM_INC_m(cpu, mem, cpu->HL);
                mCycleTimer = 2;
                break;

            case 0x35: // DEC (HL) (12)
                ASM_DEC_m(cpu, mem, cpu->HL);
                mCycleTimer = 2;
                break;

            case 0x36: // LD (HL), n (12)
                ASM_LD_m1_m2(cpu, mem, cpu->HL, cpu->PC + 1);
                cpu->PC += 1;
                mCycleTimer = 2;
                break;

            case 0x37: // SCF (4)
                ASM_SCF(cpu);
                break;

            case 0x38: // JR C, n (8/12)
                if (ASM_JR_cc_n(cpu, mem, PARAM_CC_C)) {
                    mCycleTimer = 2;
                } else {
                    mCycleTimer = 1;
                }
                //ASM_JR_cc_n(cpu, mem, PARAM_CC_C);
                //mCycleTimer = 1;
                break;

            case 0x39: // ADD HL, SP (8)
                ASM_ADD_HL_n(cpu, &(cpu->SP));
                mCycleTimer = 1;
                break;

            case 0x3A: // LDD A, (HL) (8)
                ASM_LD_A_m(cpu, mem, cpu->HL);
                ASM_DEC_nn(cpu, &(cpu->HL));
                cpu->PC -= 1; // TODO: Stop being lazy
                mCycleTimer = 1;
                break;

            case 0x3B: // DEC SP (8)
                ASM_DEC_nn(cpu, &(cpu->SP));
                mCycleTimer = 1;
                break;

            case 0x3C: // INC A (4)
                ASM_INC_n(cpu, &(cpu->A));
                break;

            case 0x3D: // DEC A (4)
                ASM_DEC_n(cpu, &(cpu->A));
                break;

            case 0x3E: // LD A, # (8)
                ASM_LD_A_m(cpu, mem, cpu->PC + 1);
                cpu->PC += 1;
                mCycleTimer = 1;
                break;

            case 0x3F: // CCF (4)
                ASM_CCF(cpu);
                break;

            case 0x40: // LD B, B (4)
                ASM_LD_r1_r2(cpu, &(cpu->B), &(cpu->B));
                break;

            case 0x41: // LD B, C (4)
                ASM_LD_r1_r2(cpu, &(cpu->B), &(cpu->C));
                break;

            case 0x42: // LD B, D (4)
                ASM_LD_r1_r2(cpu, &(cpu->B), &(cpu->D));
                break;

            case 0x43: // LD B, E (4)
                ASM_LD_r1_r2(cpu, &(cpu->B), &(cpu->E));
                break;

            case 0x44: // LD B, H (4)
                ASM_LD_r1_r2(cpu, &(cpu->B), &(cpu->H));
                break;

            case 0x45: // LD B, L (4)
                ASM_LD_r1_r2(cpu, &(cpu->B), &(cpu->L));
                break;

            case 0x46: // LD B, (HL) (8)
                ASM_LD_r1_m(cpu, mem, &(cpu->B), cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x47: // LD B, A (4)
                ASM_LD_n_A(cpu, &(cpu->B));
                break;

            case 0x48: // LD C, B (4)
                ASM_LD_r1_r2(cpu, &(cpu->C), &(cpu->B));
                break;

            case 0x49: // LD C, C (4)
                ASM_LD_r1_r2(cpu, &(cpu->C), &(cpu->C));
                break;

            case 0x4A: // LD C, D (4)
                ASM_LD_r1_r2(cpu, &(cpu->C), &(cpu->D));
                break;

            case 0x4B: // LD C, E (4)
                ASM_LD_r1_r2(cpu, &(cpu->C), &(cpu->E));
                break;

            case 0x4C: // LD C, H (4)
                ASM_LD_r1_r2(cpu, &(cpu->C), &(cpu->H));
                break;

            case 0x4D: // LD C, L (4)
                ASM_LD_r1_r2(cpu, &(cpu->C), &(cpu->L));
                break;

            case 0x4E: // LD C, (HL) (8)
                ASM_LD_r1_m(cpu, mem, &(cpu->C), cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x4F: // LD C, A (4)
                ASM_LD_n_A(cpu, &(cpu->C));
                break;

            case 0x50: // LD D, B (4)
                ASM_LD_r1_r2(cpu, &(cpu->D), &(cpu->B));
                break;

            case 0x51: // LD D, C (4)
                ASM_LD_r1_r2(cpu, &(cpu->D), &(cpu->C));
                break;

            case 0x52: // LD D, D (4)
                ASM_LD_r1_r2(cpu, &(cpu->D), &(cpu->D));
                break;

            case 0x53: // LD D, E (4)
                ASM_LD_r1_r2(cpu, &(cpu->D), &(cpu->E));
                break;

            case 0x54: // LD D, H (4)
                ASM_LD_r1_r2(cpu, &(cpu->D), &(cpu->H));
                break;

            case 0x55: // LD D, L (4)
                ASM_LD_r1_r2(cpu, &(cpu->D), &(cpu->L));
                break;

            case 0x56: // LD D, (HL) (8)
                ASM_LD_r1_m(cpu, mem, &(cpu->D), cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x57: // LD D, A (4)
                ASM_LD_n_A(cpu, &(cpu->D));
                break;
            
            case 0x58: // LD E, B (4)
                ASM_LD_r1_r2(cpu, &(cpu->E), &(cpu->B));
                break;

            case 0x59: // LD E, C (4)
                ASM_LD_r1_r2(cpu, &(cpu->E), &(cpu->C));
                break;

            case 0x5A: // LD E, D (4)
                ASM_LD_r1_r2(cpu, &(cpu->E), &(cpu->D));
                break;

            case 0x5B: // LD E, E (4)
                ASM_LD_r1_r2(cpu, &(cpu->E), &(cpu->E));
                break;

            case 0x5C: // LD E, H (4)
                ASM_LD_r1_r2(cpu, &(cpu->E), &(cpu->H));
                break;

            case 0x5D: // LD E, L (4)
                ASM_LD_r1_r2(cpu, &(cpu->E), &(cpu->L));
                break;

            case 0x5E: // LD E, (HL) (8)
                ASM_LD_r1_m(cpu, mem, &(cpu->E), cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x5F: // LD E, A (4)
                ASM_LD_n_A(cpu, &(cpu->E));
                break;

            case 0x60: // LD H, B (4)
                ASM_LD_r1_r2(cpu, &(cpu->H), &(cpu->B));
                break;

            case 0x61: // LD H, C (4)
                ASM_LD_r1_r2(cpu, &(cpu->H), &(cpu->C));
                break;

            case 0x62: // LD H, D (4)
                ASM_LD_r1_r2(cpu, &(cpu->H), &(cpu->D));
                break;

            case 0x63: // LD H, E (4)
                ASM_LD_r1_r2(cpu, &(cpu->H), &(cpu->E));
                break;

            case 0x64: // LD H, H (4)
                ASM_LD_r1_r2(cpu, &(cpu->H), &(cpu->H));
                break;

            case 0x65: // LD H, L (4)
                ASM_LD_r1_r2(cpu, &(cpu->H), &(cpu->L));
                break;

            case 0x66: // LD H, (HL) (8)
                ASM_LD_r1_m(cpu, mem, &(cpu->H), cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x67: // LD H, A (4)
                ASM_LD_n_A(cpu, &(cpu->H));
                break;

            case 0x68: // LD L, B (4)
                ASM_LD_r1_r2(cpu, &(cpu->L), &(cpu->B));
                break;

            case 0x69: // LD L, C (4)
                ASM_LD_r1_r2(cpu, &(cpu->L), &(cpu->C));
                break;

            case 0x6A: // LD L, D (4)
                ASM_LD_r1_r2(cpu, &(cpu->L), &(cpu->D));
                break;

            case 0x6B: // LD L, E (4)
                ASM_LD_r1_r2(cpu, &(cpu->L), &(cpu->E));
                break;

            case 0x6C: // LD L, H (4)
                ASM_LD_r1_r2(cpu, &(cpu->L), &(cpu->H));
                break;

            case 0x6D: // LD L, L (4)
                ASM_LD_r1_r2(cpu, &(cpu->L), &(cpu->L));
                break;

            case 0x6E: // LD L, (HL) (8)
                ASM_LD_r1_m(cpu, mem, &(cpu->L), cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x6F: // LD L, A (4)
                ASM_LD_n_A(cpu, &(cpu->L));
                break;

            case 0x70: // LD (HL), B (8)
                ASM_LD_m_r2(cpu, mem, cpu->HL, &(cpu->B));
                mCycleTimer = 1;
                break;

            case 0x71: // LD (HL), C (8)
                ASM_LD_m_r2(cpu, mem, cpu->HL, &(cpu->C));
                mCycleTimer = 1;
                break;

            case 0x72: // LD (HL), D (8)
                ASM_LD_m_r2(cpu, mem, cpu->HL, &(cpu->D));
                mCycleTimer = 1;
                break;

            case 0x73: // LD (HL), E (8)
                ASM_LD_m_r2(cpu, mem, cpu->HL, &(cpu->E));
                mCycleTimer = 1;
                break;

            case 0x74: // LD (HL), H (8)
                ASM_LD_m_r2(cpu, mem, cpu->HL, &(cpu->H));
                mCycleTimer = 1;
                break;

            case 0x75: // LD (HL), L (8)
                ASM_LD_m_r2(cpu, mem, cpu->HL, &(cpu->L));
                mCycleTimer = 1;
                break;

            case 0x76: // HALT (4)
                if ((MEM_getByte(mem, REG_IF) & MEM_getByte(mem, REG_IE)) != 0) {
                    cpu->PC += 1;
                }
                break;

            case 0x77: // LD (HL), A (8)
                ASM_LD_m_A(cpu, mem, cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x78: // LD A, B (4)
                ASM_LD_r1_r2(cpu, &(cpu->A), &(cpu->B));
                break;

            case 0x79: // LD A, C (4)
                ASM_LD_A_n(cpu, &(cpu->C));
                break;

            case 0x7A: // LD A, D (4)
                ASM_LD_A_n(cpu, &(cpu->D));
                break;

            case 0x7B: // LD A, E (4)
                ASM_LD_A_n(cpu, &(cpu->E));
                break;

            case 0x7C: // LD A, H (4)
                ASM_LD_A_n(cpu, &(cpu->H));
                break;

            case 0x7D: // LD A, L (4)
                ASM_LD_A_n(cpu, &(cpu->L));
                break;

            case 0x7E: // LD A, (HL) (8)
                ASM_LD_A_m(cpu, mem, cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x7F: // LD A, A (4)
                ASM_LD_A_n(cpu, &(cpu->A));
                break;

            case 0x80: // ADD A, B (4)
                ASM_ADD_A_n(cpu, &(cpu->B));
                break;

            case 0x81: // ADD A, C (4)
                ASM_ADD_A_n(cpu, &(cpu->C));
                break;

            case 0x82: // ADD A, D (4)
                ASM_ADD_A_n(cpu, &(cpu->D));
                break;

            case 0x83: // ADD A, E (4)
                ASM_ADD_A_n(cpu, &(cpu->E));
                break;

            case 0x84: // ADD A, H (4)
                ASM_ADD_A_n(cpu, &(cpu->H));
                break;

            case 0x85: // ADD A, L (4)
                ASM_ADD_A_n(cpu, &(cpu->L));
                break;

            case 0x86: // ADD A, (HL) (8)
                ASM_ADD_A_m(cpu, mem, cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x87: // ADD A, A (4)
                ASM_ADD_A_n(cpu, &(cpu->A));
                break;

            case 0x88: // ADC A, B (4)
                ASM_ADC_A_n(cpu, &(cpu->B));
                break;

            case 0x89: // ADC A, C (4)
                ASM_ADC_A_n(cpu, &(cpu->C));
                break;

            case 0x8A: // ADC A, D (4)
                ASM_ADC_A_n(cpu, &(cpu->D));
                break;

            case 0x8B: // ADC A, E (4)
                ASM_ADC_A_n(cpu, &(cpu->E));
                break;

            case 0x8C: // ADC A, H (4)
                ASM_ADC_A_n(cpu, &(cpu->H));
                break;

            case 0x8D: // ADC A, L (4)
                ASM_ADC_A_n(cpu, &(cpu->L));
                break;

            case 0x8E: // ADC A, (HL) (8)
                ASM_ADC_A_m(cpu, mem, cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x8F: // ADC A, A (4)
                ASM_ADC_A_n(cpu, &(cpu->A));
                break;

            case 0x90: // SUB B (4)
                ASM_SUB_n(cpu, &(cpu->B));
                break;

            case 0x91: // SUB C (4)
                ASM_SUB_n(cpu, &(cpu->C));
                break;

            case 0x92: // SUB D (4)
                ASM_SUB_n(cpu, &(cpu->D));
                break;

            case 0x93: // SUB E (4)
                ASM_SUB_n(cpu, &(cpu->E));
                break;

            case 0x94: // SUB H (4)
                ASM_SUB_n(cpu, &(cpu->H));
                break;

            case 0x95: // SUB L (4)
                ASM_SUB_n(cpu, &(cpu->L));
                break;

            case 0x96: // SUB (HL) (8)
                ASM_SUB_m(cpu, mem, cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x97: // SUB A (4)
                ASM_SUB_n(cpu, &(cpu->A));
                break;

            case 0x98: // SBC A, B (4)
                ASM_SBC_A_n(cpu, &(cpu->B));
                break;

            case 0x99: // SBC A, C (4)
                ASM_SBC_A_n(cpu, &(cpu->C));
                break;

            case 0x9A: // SBC A, D (4)
                ASM_SBC_A_n(cpu, &(cpu->D));
                break;

            case 0x9B: // SBC A, E (4)
                ASM_SBC_A_n(cpu, &(cpu->E));
                break;

            case 0x9C: // SBC A, H (4)
                ASM_SBC_A_n(cpu, &(cpu->H));
                break;

            case 0x9D: // SBC A, L (4)
                ASM_SBC_A_n(cpu, &(cpu->L));
                break;

            case 0x9E: // SBC A, (HL) (8)
                ASM_SBC_A_m(cpu, mem, cpu->HL);
                mCycleTimer = 1;
                break;

            case 0x9F: // SBC A, A (4)
                ASM_SBC_A_n(cpu, &(cpu->A));
                break;

            case 0xA0: // AND B (4)
                ASM_AND_n(cpu, &(cpu->B));
                break;

            case 0xA1: // AND C (4)
                ASM_AND_n(cpu, &(cpu->C));
                break;

            case 0xA2: // AND D (4)
                ASM_AND_n(cpu, &(cpu->D));
                break;

            case 0xA3: // AND E (4)
                ASM_AND_n(cpu, &(cpu->E));
                break;

            case 0xA4: // AND H (4)
                ASM_AND_n(cpu, &(cpu->H));
                break;

            case 0xA5: // AND L (4)
                ASM_AND_n(cpu, &(cpu->L));
                break;

            case 0xA6: // AND (HL) (8)
                ASM_AND_m(cpu, mem, cpu->HL);
                mCycleTimer = 1;
                break;

            case 0xA7: // AND A (4)
                ASM_AND_n(cpu, &(cpu->A));
                break;

            case 0xA8: // XOR B (4)
                ASM_XOR_n(cpu, &(cpu->B));
                break;

            case 0xA9: // XOR C (4)
                ASM_XOR_n(cpu, &(cpu->C));
                break;

            case 0xAA: // XOR D (4)
                ASM_XOR_n(cpu, &(cpu->D));
                break;

            case 0xAB: // XOR E (4)
                ASM_XOR_n(cpu, &(cpu->E));
                break;

            case 0xAC: // XOR H (4)
                ASM_XOR_n(cpu, &(cpu->H));
                break;

            case 0xAD: // XOR L (4)
                ASM_XOR_n(cpu, &(cpu->L));
                break;

            case 0xAE: // XOR (HL) (8)
                ASM_XOR_m(cpu, mem, cpu->HL);
                mCycleTimer = 1;
                break;

            case 0xAF: // XOR A (4)
                ASM_XOR_n(cpu, &(cpu->A));
                break;

            case 0xB0: // OR B (4)
                ASM_OR_n(cpu, &(cpu->B));
                break;

            case 0xB1: // OR C (4)
                ASM_OR_n(cpu, &(cpu->C));
                break;

            case 0xB2: // OR D (4)
                ASM_OR_n(cpu, &(cpu->D));
                break;

            case 0xB3: // OR E (4)
                ASM_OR_n(cpu, &(cpu->E));
                break;

            case 0xB4: // OR H (4)
                ASM_OR_n(cpu, &(cpu->H));
                break;

            case 0xB5: // OR L (4)
                ASM_OR_n(cpu, &(cpu->L));
                break;

            case 0xB6: // OR (HL) (8)
                ASM_OR_m(cpu, mem, cpu->HL);
                mCycleTimer = 1;
                break;

            case 0xB7: // OR A (4)
                ASM_OR_n(cpu, &(cpu->A));
                break;

            case 0xB8: // CP B (4)
                ASM_CP_n(cpu, &(cpu->B));
                break;

            case 0xB9: // CP C (4)
                ASM_CP_n(cpu, &(cpu->C));
                break;

            case 0xBA: // CP D (4)
                ASM_CP_n(cpu, &(cpu->D));
                break;

            case 0xBB: // CP E (4)
                ASM_CP_n(cpu, &(cpu->E));
                break;

            case 0xBC: // CP H (4)
                ASM_CP_n(cpu, &(cpu->H));
                break;

            case 0xBD: // CP L (4)
                ASM_CP_n(cpu, &(cpu->L));
                break;

            case 0xBE: // CP (HL) (8)
                ASM_CP_m(cpu, mem, cpu->HL);
                mCycleTimer = 1;
                break;

            case 0xBF: // CP A (4)
                ASM_CP_n(cpu, &(cpu->A));
                break;

            case 0xC0: // RET NZ (8/20)
                if (ASM_RET_cc(cpu, mem, PARAM_CC_NZ)) {
                    mCycleTimer = 4;
                } else {
                    mCycleTimer = 1;
                }
                break;

            case 0xC1: // POP BC (12)
                ASM_POP_nn(cpu, mem, &(cpu->BC));
                mCycleTimer = 2;
                break;

            case 0xC2: // JP NZ, nn (12/16)
                if (ASM_JP_cc_nn(cpu, mem, PARAM_CC_NZ)) {
                    mCycleTimer = 3;
                } else {
                    mCycleTimer = 2;
                }
                break;

            case 0xC3: // JP nn (16)
                ASM_JP_nn(cpu, mem);
                mCycleTimer = 3;
                break;

            case 0xC4: // CALL NZ, nn (12/24)
                if (ASM_CALL_cc_nn(cpu, mem, PARAM_CC_NZ)) {
                    mCycleTimer = 5;
                } else {
                    mCycleTimer = 2;
                }
                break;

            case 0xC5: // PUSH BC (16)
                ASM_PUSH_nn(cpu, mem, &(cpu->BC));
                mCycleTimer = 3;
                break;

            case 0xC6: // ADD A, # (8)
                ASM_ADD_A_m(cpu, mem, cpu->PC + 1);
                cpu->PC += 1;
                mCycleTimer = 1;
                break;

            case 0xC7: // RST 00H (16)
                ASM_RST_n(cpu, mem, 0x00);
                mCycleTimer = 3;
                break;

            case 0xC8: // RET Z (8/20)
                if (ASM_RET_cc(cpu, mem, PARAM_CC_Z)) {
                    mCycleTimer = 4;
                } else {
                    mCycleTimer = 1;
                }
                break;

            case 0xC9: // RET (16)
                ASM_RET(cpu, mem);
                mCycleTimer = 3;
                break;

            case 0xCA: // JP Z, nn (12/16)
                if (ASM_JP_cc_nn(cpu, mem, PARAM_CC_Z)) {
                    mCycleTimer = 3;
                } else {
                    mCycleTimer = 2;
                }
                break;

            case 0xCC: // CALL Z, nn (12/24)
                if (ASM_CALL_cc_nn(cpu, mem, PARAM_CC_Z)) {
                    mCycleTimer = 5;
                } else {
                    mCycleTimer = 2;
                }
                break;

            case 0xCD: // CALL nn (24)
                ASM_CALL_nn(cpu, mem);
                mCycleTimer = 5;
                break;

            case 0xCE: // ADC A, # (8)
                ASM_ADC_A_m(cpu, mem, cpu->PC + 1);
                cpu->PC += 1;
                mCycleTimer = 1;
                break;

            case 0xCF: // RST 08H (16)
                ASM_RST_n(cpu, mem, 0x08);
                mCycleTimer = 3;
                break;

            case 0xD0: // RET NC (8/20)
                if (ASM_RET_cc(cpu, mem, PARAM_CC_NC)) {
                    mCycleTimer = 4;
                } else {
                    mCycleTimer = 1;
                }
                break;

            case 0xD1: // POP DE (12)
                ASM_POP_nn(cpu, mem, &(cpu->DE));
                mCycleTimer = 2;
                break;

            case 0xD4: // CALL NC, nn (12/24)
                if (ASM_CALL_cc_nn(cpu, mem, PARAM_CC_NC)) {
                    mCycleTimer = 5;
                } else {
                    mCycleTimer = 2;
                }
                break;

            case 0xD5: // PUSH DE (16)
                ASM_PUSH_nn(cpu, mem, &(cpu->DE));
                mCycleTimer = 3;
                break;

            case 0xD6: // SUB # (8)
                ASM_SUB_m(cpu, mem, cpu->PC + 1);
                cpu->PC += 1;
                mCycleTimer = 1;
                break;

            case 0xD7: // RST 10H (16)
                ASM_RST_n(cpu, mem, 0x10);
                mCycleTimer = 3;
                break;

            case 0xD8: // RET C (8/20)
                if (ASM_RET_cc(cpu, mem, PARAM_CC_C)) {
                    mCycleTimer = 4;
                } else {
                    mCycleTimer = 1;
                }
                break;

            case 0xD9: // RETI (16)
                //printf("EXITED INTERRUPT\n");
                ASM_RETI(cpu, mem);
                mCycleTimer = 3;
                break;

            case 0xDA: // JP C, nn (12/16)
                if (ASM_JP_cc_nn(cpu, mem, PARAM_CC_C)) {
                    mCycleTimer = 3;
                } else {
                    mCycleTimer = 2;
                }
                break;

            case 0xDC: // CALL C, nn (12/24)
                if (ASM_CALL_cc_nn(cpu, mem, PARAM_CC_C)) {
                    mCycleTimer = 5;
                } else {
                    mCycleTimer = 2;
                }
                break;

            case 0xDE: // SBC A, # (8)
                ASM_SBC_A_m(cpu, mem, cpu->PC + 1);
                cpu->PC += 1;
                mCycleTimer = 1;
                break;

            case 0xDF: // RST 18H (16)
                ASM_RST_n(cpu, mem, 0x18);
                mCycleTimer = 3;
                break;

            case 0xE0: // LDH (n), A (12)
                ASM_LDH_n_A(cpu, mem);
                mCycleTimer = 2;
                break;

            case 0xE1: // POP HL (12)
                ASM_POP_nn(cpu, mem, &(cpu->HL));
                mCycleTimer = 2;
                break;

            case 0xE2: // LD (C), A (8)
                ASM_LD_C_A(cpu, mem);
                mCycleTimer = 1;
                break;

            case 0xE5: // PUSH HL (16)
                ASM_PUSH_nn(cpu, mem, &(cpu->HL));
                mCycleTimer = 3;
                break;

            case 0xE6: // AND #n (8)
                ASM_AND_m(cpu, mem, cpu->PC + 1);
                cpu->PC += 1;
                mCycleTimer = 1;
                break;

            case 0xE7: // RST 20H (16)
                ASM_RST_n(cpu, mem, 0x20);
                mCycleTimer = 3;
                break;

            case 0xE8: // ADD SP, n (16)
                ASM_ADD_SP_n(cpu, mem);
                mCycleTimer = 3;
                break;

            case 0xE9: // JP (HL) (4)
                ASM_JP_HL(cpu);
                break;

            case 0xEA: // LD (nn), A (16)
                ASM_LD_m_A(cpu, mem, (MEM_getByte(mem, cpu->PC + 2) << 8) | MEM_getByte(mem, cpu->PC + 1));
                cpu->PC += 2;
                mCycleTimer = 3;
                break;

            case 0xEE: // XOR # (8)
                ASM_XOR_m(cpu, mem, cpu->PC + 1);
                cpu->PC += 1;
                mCycleTimer = 1;
                break;

            case 0xEF: // RST 28H (16)
                ASM_RST_n(cpu, mem, 0x28);
                mCycleTimer = 3;
                break;

            case 0xF0: // LDH A, (n) (12)
                ASM_LDH_A_n(cpu, mem);
                mCycleTimer = 2;
                break;

            case 0xF1: // POP AF (12)
                ASM_POP_nn(cpu, mem, &(cpu->AF));
                mCycleTimer = 2;
                break;

            case 0xF2: // LD A, (FF00 + C) (8)
                ASM_LD_A_m(cpu, mem, 0xFF00 + cpu->C);
                mCycleTimer = 1;
                break;

            case 0xF3: // DI (4)
                cpu->IME = 0;
                cpu->PC += 1;
                break;

            case 0xF5: // PUSH AF (16)
                ASM_PUSH_nn(cpu, mem, &(cpu->AF));
                mCycleTimer = 3;
                break;

            case 0xF6: // OR # (8)
                ASM_OR_m(cpu, mem, cpu->PC + 1);
                cpu->PC += 1;
                mCycleTimer = 1;
                break;

            case 0xF7: // RST 30H (16)
                ASM_RST_n(cpu, mem, 0x30);
                mCycleTimer = 3;
                break;

            case 0xF8: // LDHL SP, n (12)
                ASM_LDHL_SP_n(cpu, mem);
                mCycleTimer = 2;
                break;

            case 0xF9: // LD SP, HL (8)
                ASM_LD_SP_HL(cpu);
                mCycleTimer = 1;
                break;

            case 0xFA: // LD A, (nn) (16)
                ASM_LD_A_m(cpu, mem, (MEM_getByte(mem, cpu->PC + 2) << 8) | MEM_getByte(mem, cpu->PC + 1));
                cpu->PC += 2;
                mCycleTimer = 3;
                break;

            case 0xFB: // EI (4)
                cpu->IME = 1;
                cpu->PC += 1;
                break;

            case 0xFE: // CP #n (8)
                ASM_CP_m(cpu, mem, cpu->PC + 1);
                cpu->PC += 1;
                mCycleTimer = 1;
                break;

            case 0xFF: // RST 38H (16)
                ASM_RST_n(cpu, mem, 0x38);
                mCycleTimer = 3;
                break;

            case 0xCB: // this is a 16 bit opcode, let's decode the next byte
                switch (MEM_getByte(mem, cpu->PC + 1)) {
                    case 0x00: // RLC B (8)
                        ASM_RLC_n(cpu, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x01: // RLC C (8)
                        ASM_RLC_n(cpu, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x02: // RLC D (8)
                        ASM_RLC_n(cpu, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x03: // RLC E (8)
                        ASM_RLC_n(cpu, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x04: // RLC H (8)
                        ASM_RLC_n(cpu, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x05: // RLC L (8)
                        ASM_RLC_n(cpu, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x06: // RLC (HL) (16)
                        ASM_RLC_m(cpu, mem, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x07: // RLC A (8)
                        ASM_RLC_n(cpu, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x08: // RRC B (8)
                        ASM_RRC_n(cpu, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x09: // RRC C (8)
                        ASM_RRC_n(cpu, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x0A: // RRC D (8)
                        ASM_RRC_n(cpu, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x0B: // RRC E (8)
                        ASM_RRC_n(cpu, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x0C: // RRC H (8)
                        ASM_RRC_n(cpu, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x0D: // RRC L (8)
                        ASM_RRC_n(cpu, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x0E: // RRC (HL) (16)
                        ASM_RRC_m(cpu, mem, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x0F: // RRC A (8)
                        ASM_RRC_n(cpu, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x10: // RL B (8)
                        ASM_RL_n(cpu, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x11: // RL C (8)
                        ASM_RL_n(cpu, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x12: // RL D (8)
                        ASM_RL_n(cpu, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x13: // RL E (8)
                        ASM_RL_n(cpu, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x14: // RL H (8)
                        ASM_RL_n(cpu, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x15: // RL L (8)
                        ASM_RL_n(cpu, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x16: // RL (HL) (16)
                        ASM_RL_m(cpu, mem, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x17: // RL A (8)
                        ASM_RL_n(cpu, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x18: // RR B (8)
                        ASM_RR_n(cpu, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x19: // RR C (8)
                        ASM_RR_n(cpu, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x1A: // RR D (8)
                        ASM_RR_n(cpu, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x1B: // RR E (8)
                        ASM_RR_n(cpu, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x1C: // RR H (8)
                        ASM_RR_n(cpu, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x1D: // RR L (8)
                        ASM_RR_n(cpu, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x1E: // RR (HL) (16)
                        ASM_RR_m(cpu, mem, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x1F: // RR A (8)
                        ASM_RR_n(cpu, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x20: // SLA B (8)
                        ASM_SLA_n(cpu, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x21: // SLA C (8)
                        ASM_SLA_n(cpu, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x22: // SLA D (8)
                        ASM_SLA_n(cpu, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x23: // SLA E (8)
                        ASM_SLA_n(cpu, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x24: // SLA H (8)
                        ASM_SLA_n(cpu, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x25: // SLA L (8)
                        ASM_SLA_n(cpu, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x26: // SLA (HL) (16)
                        ASM_SLA_m(cpu, mem, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x27: // SLA A (8)
                        ASM_SLA_n(cpu, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x28: // SRA B (8)
                        ASM_SRA_n(cpu, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x29: // SRA C (8)
                        ASM_SRA_n(cpu, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x2A: // SRA D (8)
                        ASM_SRA_n(cpu, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x2B: // SRA E (8)
                        ASM_SRA_n(cpu, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x2C: // SRA H (8)
                        ASM_SRA_n(cpu, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x2D: // SRA L (8)
                        ASM_SRA_n(cpu, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x2E: // SRA (HL) (16)
                        ASM_SRA_m(cpu, mem, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x2F: // SRA A (8)
                        ASM_SRA_n(cpu, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x30: // SWAP B (8)
                        ASM_SWAP_n(cpu, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x31: // SWAP C (8)
                        ASM_SWAP_n(cpu, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x32: // SWAP D (8)
                        ASM_SWAP_n(cpu, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x33: // SWAP E (8)
                        ASM_SWAP_n(cpu, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x34: // SWAP H (8)
                        ASM_SWAP_n(cpu, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x35: // SWAP L (8)
                        ASM_SWAP_n(cpu, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x36: // SWAP (HL) (16)
                        ASM_SWAP_m(cpu, mem, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x37: // SWAP A (8)
                        ASM_SWAP_n(cpu, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x38: // SRL B (8)
                        ASM_SRL_n(cpu, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x39: // SRL C (8)
                        ASM_SRL_n(cpu, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x3A: // SRL D (8)
                        ASM_SRL_n(cpu, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x3B: // SRL E (8)
                        ASM_SRL_n(cpu, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x3C: // SRL H (8)
                        ASM_SRL_n(cpu, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x3D: // SRL L (8)
                        ASM_SRL_n(cpu, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x3E: // SRL (HL) (8)
                        ASM_SRL_m(cpu, mem, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x3F: // SRL A (8)
                        ASM_SRL_n(cpu, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x40: // BIT 0, B (8)
                        ASM_BIT_b_r(cpu, 0, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x41: // BIT 0, C (8)
                        ASM_BIT_b_r(cpu, 0, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x42: // BIT 0, D (8)
                        ASM_BIT_b_r(cpu, 0, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x43: // BIT 0, E (8)
                        ASM_BIT_b_r(cpu, 0, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x44: // BIT 0, H (8)
                        ASM_BIT_b_r(cpu, 0, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x45: // BIT 0, L (8)
                        ASM_BIT_b_r(cpu, 0, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x46: // BIT 0, (HL) (12)
                        ASM_BIT_b_m(cpu, mem, 0, cpu->HL);
                        mCycleTimer = 2;
                        break;

                    case 0x47: // BIT 0, A (8)
                        ASM_BIT_b_r(cpu, 0, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x48: // BIT 1, B (8)
                        ASM_BIT_b_r(cpu, 1, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x49: // BIT 1, C (8)
                        ASM_BIT_b_r(cpu, 1, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x4A: // BIT 1, D (8)
                        ASM_BIT_b_r(cpu, 1, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x4B: // BIT 1, E (8)
                        ASM_BIT_b_r(cpu, 1, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x4C: // BIT 1, H (8)
                        ASM_BIT_b_r(cpu, 1, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x4D: // BIT 1, L (8)
                        ASM_BIT_b_r(cpu, 1, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x4E: // BIT 1, (HL) (12)
                        ASM_BIT_b_m(cpu, mem, 1, cpu->HL);
                        mCycleTimer = 2;
                        break;

                    case 0x4F: // BIT 1, A (8)
                        ASM_BIT_b_r(cpu, 1, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x50: // BIT 2, B (8)
                        ASM_BIT_b_r(cpu, 2, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x51: // BIT 2, C (8)
                        ASM_BIT_b_r(cpu, 2, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x52: // BIT 2, D (8)
                        ASM_BIT_b_r(cpu, 2, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x53: // BIT 2, E (8)
                        ASM_BIT_b_r(cpu, 2, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x54: // BIT 2, H (8)
                        ASM_BIT_b_r(cpu, 2, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x55: // BIT 2, L (8)
                        ASM_BIT_b_r(cpu, 2, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x56: // BIT 2, (HL) (12)
                        ASM_BIT_b_m(cpu, mem, 2, cpu->HL);
                        mCycleTimer = 2;
                        break;

                    case 0x57: // BIT 2, A (8)
                        ASM_BIT_b_r(cpu, 2, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x58: // BIT 3, B (8)
                        ASM_BIT_b_r(cpu, 3, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x59: // BIT 3, C (8)
                        ASM_BIT_b_r(cpu, 3, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x5A: // BIT 3, D (8)
                        ASM_BIT_b_r(cpu, 3, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x5B: // BIT 3, E (8)
                        ASM_BIT_b_r(cpu, 3, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x5C: // BIT 3, H (8)
                        ASM_BIT_b_r(cpu, 3, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x5D: // BIT 3, L (8)
                        ASM_BIT_b_r(cpu, 3, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x5E: // BIT 3, (HL) (12)
                        ASM_BIT_b_m(cpu, mem, 3, cpu->HL);
                        mCycleTimer = 2;
                        break;

                    case 0x5F: // BIT 3, A (8)
                        ASM_BIT_b_r(cpu, 3, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x60: // BIT 4, B (8)
                        ASM_BIT_b_r(cpu, 4, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x61: // BIT 4, C (8)
                        ASM_BIT_b_r(cpu, 4, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x62: // BIT 4, D (8)
                        ASM_BIT_b_r(cpu, 4, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x63: // BIT 4, E (8)
                        ASM_BIT_b_r(cpu, 4, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x64: // BIT 4, H (8)
                        ASM_BIT_b_r(cpu, 4, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x65: // BIT 4, L (8)
                        ASM_BIT_b_r(cpu, 4, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x66: // BIT 4, (HL) (12)
                        ASM_BIT_b_m(cpu, mem, 4, cpu->HL);
                        mCycleTimer = 2;
                        break;

                    case 0x67: // BIT 4, A (8)
                        ASM_BIT_b_r(cpu, 4, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x68: // BIT 5, B (8)
                        ASM_BIT_b_r(cpu, 5, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x69: // BIT 5, C (8)
                        ASM_BIT_b_r(cpu, 5, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x6A: // BIT 5, D (8)
                        ASM_BIT_b_r(cpu, 5, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x6B: // BIT 5, E (8)
                        ASM_BIT_b_r(cpu, 5, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x6C: // BIT 5, H (8)
                        ASM_BIT_b_r(cpu, 5, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x6D: // BIT 5, L (8)
                        ASM_BIT_b_r(cpu, 5, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x6E: // BIT 5, (HL) (12)
                        ASM_BIT_b_m(cpu, mem, 5, cpu->HL);
                        mCycleTimer = 2;
                        break;

                    case 0x6F: // BIT 5, A (8)
                        ASM_BIT_b_r(cpu, 5, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x70: // BIT 6, B (8)
                        ASM_BIT_b_r(cpu, 6, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x71: // BIT 6, C (8)
                        ASM_BIT_b_r(cpu, 6, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x72: // BIT 6, D (8)
                        ASM_BIT_b_r(cpu, 6, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x73: // BIT 6, E (8)
                        ASM_BIT_b_r(cpu, 6, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x74: // BIT 6, H (8)
                        ASM_BIT_b_r(cpu, 6, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x75: // BIT 6, L (8)
                        ASM_BIT_b_r(cpu, 6, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x76: // BIT 6, (HL) (12)
                        ASM_BIT_b_m(cpu, mem, 6, cpu->HL);
                        mCycleTimer = 2;
                        break;

                    case 0x77: // BIT 6, A (8)
                        ASM_BIT_b_r(cpu, 6, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x78: // BIT 7, B (8)
                        ASM_BIT_b_r(cpu, 7, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x79: // BIT 7, C (8)
                        ASM_BIT_b_r(cpu, 7, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x7A: // BIT 7, D (8)
                        ASM_BIT_b_r(cpu, 7, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x7B: // BIT 7, E (8)
                        ASM_BIT_b_r(cpu, 7, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x7C: // BIT 7, H (8)
                        ASM_BIT_b_r(cpu, 7, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x7D: // BIT 7, L (8)
                        ASM_BIT_b_r(cpu, 7, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x7E: // BIT 7, (HL) (12)
                        ASM_BIT_b_m(cpu, mem, 7, cpu->HL);
                        mCycleTimer = 2;
                        break;

                    case 0x7F: // BIT 7, A (8)
                        ASM_BIT_b_r(cpu, 7, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x80: // RES 0, B (8)
                        ASM_RES_b_r(cpu, 0, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x81: // RES 0, C (8)
                        ASM_RES_b_r(cpu, 0, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x82: // RES 0, D (8)
                        ASM_RES_b_r(cpu, 0, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x83: // RES 0, E (8)
                        ASM_RES_b_r(cpu, 0, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x84: // RES 0, H (8)
                        ASM_RES_b_r(cpu, 0, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x85: // RES 0, L (8)
                        ASM_RES_b_r(cpu, 0, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x86: // RES 0, (HL) (16)
                        ASM_RES_b_m(cpu, mem, 0, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x87: // RES 0, A (8)
                        ASM_RES_b_r(cpu, 0, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x88: // RES 1, B (8)
                        ASM_RES_b_r(cpu, 1, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x89: // RES 1, C (8)
                        ASM_RES_b_r(cpu, 1, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x8A: // RES 1, D (8)
                        ASM_RES_b_r(cpu, 1, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x8B: // RES 1, E (8)
                        ASM_RES_b_r(cpu, 1, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x8C: // RES 1, H (8)
                        ASM_RES_b_r(cpu, 1, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x8D: // RES 1, L (8)
                        ASM_RES_b_r(cpu, 1, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x8E: // RES 1, (HL) (16)
                        ASM_RES_b_m(cpu, mem, 1, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x8F: // RES 1, A (8)
                        ASM_RES_b_r(cpu, 1, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x90: // RES 2, B (8)
                        ASM_RES_b_r(cpu, 2, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x91: // RES 2, C (8)
                        ASM_RES_b_r(cpu, 2, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x92: // RES 2, D (8)
                        ASM_RES_b_r(cpu, 2, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x93: // RES 2, E (8)
                        ASM_RES_b_r(cpu, 2, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x94: // RES 2, H (8)
                        ASM_RES_b_r(cpu, 2, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x95: // RES 2, L (8)
                        ASM_RES_b_r(cpu, 2, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x96: // RES 2, (HL) (16)
                        ASM_RES_b_m(cpu, mem, 2, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x97: // RES 2, A (8)
                        ASM_RES_b_r(cpu, 2, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0x98: // RES 3, B (8)
                        ASM_RES_b_r(cpu, 3, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0x99: // RES 3, C (8)
                        ASM_RES_b_r(cpu, 3, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0x9A: // RES 3, D (8)
                        ASM_RES_b_r(cpu, 3, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0x9B: // RES 3, E (8)
                        ASM_RES_b_r(cpu, 3, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0x9C: // RES 3, H (8)
                        ASM_RES_b_r(cpu, 3, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0x9D: // RES 3, L (8)
                        ASM_RES_b_r(cpu, 3, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0x9E: // RES 3, (HL) (16)
                        ASM_RES_b_m(cpu, mem, 3, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0x9F: // RES 3, A (8)
                        ASM_RES_b_r(cpu, 3, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xA0: // RES 4, B (8)
                        ASM_RES_b_r(cpu, 4, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xA1: // RES 4, C (8)
                        ASM_RES_b_r(cpu, 4, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xA2: // RES 4, D (8)
                        ASM_RES_b_r(cpu, 4, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xA3: // RES 4, E (8)
                        ASM_RES_b_r(cpu, 4, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xA4: // RES 4, H (8)
                        ASM_RES_b_r(cpu, 4, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xA5: // RES 4, L (8)
                        ASM_RES_b_r(cpu, 4, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xA6: // RES 4, (HL) (16)
                        ASM_RES_b_m(cpu, mem, 4, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xA7: // RES 4, A (8)
                        ASM_RES_b_r(cpu, 4, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xA8: // RES 5, B (8)
                        ASM_RES_b_r(cpu, 5, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xA9: // RES 5, C (8)
                        ASM_RES_b_r(cpu, 5, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xAA: // RES 5, D (8)
                        ASM_RES_b_r(cpu, 5, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xAB: // RES 5, E (8)
                        ASM_RES_b_r(cpu, 5, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xAC: // RES 5, H (8)
                        ASM_RES_b_r(cpu, 5, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xAD: // RES 5, L (8)
                        ASM_RES_b_r(cpu, 5, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xAE: // RES 5, (HL) (16)
                        ASM_RES_b_m(cpu, mem, 5, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xAF: // RES 5, A (8)
                        ASM_RES_b_r(cpu, 5, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xB0: // RES 6, B (8)
                        ASM_RES_b_r(cpu, 6, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xB1: // RES 6, C (8)
                        ASM_RES_b_r(cpu, 6, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xB2: // RES 6, D (8)
                        ASM_RES_b_r(cpu, 6, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xB3: // RES 6, E (8)
                        ASM_RES_b_r(cpu, 6, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xB4: // RES 6, H (8)
                        ASM_RES_b_r(cpu, 6, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xB5: // RES 6, L (8)
                        ASM_RES_b_r(cpu, 6, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xB6: // RES 6, (HL) (16)
                        ASM_RES_b_m(cpu, mem, 6, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xB7: // RES 6, A (8)
                        ASM_RES_b_r(cpu, 6, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xB8: // RES 7, B (8)
                        ASM_RES_b_r(cpu, 7, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xB9: // RES 7, C (8)
                        ASM_RES_b_r(cpu, 7, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xBA: // RES 7, D (8)
                        ASM_RES_b_r(cpu, 7, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xBB: // RES 7, E (8)
                        ASM_RES_b_r(cpu, 7, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xBC: // RES 7, H (8)
                        ASM_RES_b_r(cpu, 7, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xBD: // RES 7, L (8)
                        ASM_RES_b_r(cpu, 7, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xBE: // RES 7, (HL) (16)
                        ASM_RES_b_m(cpu, mem, 7, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xBF: // RES 7, A (8)
                        ASM_RES_b_r(cpu, 7, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xC0: // SET 0, B (8)
                        ASM_SET_b_r(cpu, 0, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xC1: // SET 0, C (8)
                        ASM_SET_b_r(cpu, 0, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xC2: // SET 0, D (8)
                        ASM_SET_b_r(cpu, 0, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xC3: // SET 0, E (8)
                        ASM_SET_b_r(cpu, 0, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xC4: // SET 0, H (8)
                        ASM_SET_b_r(cpu, 0, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xC5: // SET 0, L (8)
                        ASM_SET_b_r(cpu, 0, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xC6: // SET 0, (HL) (16)
                        ASM_SET_b_m(cpu, mem, 0, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xC7: // SET 0, A (8)
                        ASM_SET_b_r(cpu, 0, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xC8: // SET 1, B (8)
                        ASM_SET_b_r(cpu, 1, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xC9: // SET 1, C (8)
                        ASM_SET_b_r(cpu, 1, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xCA: // SET 1, D (8)
                        ASM_SET_b_r(cpu, 1, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xCB: // SET 1, E (8)
                        ASM_SET_b_r(cpu, 1, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xCC: // SET 1, H (8)
                        ASM_SET_b_r(cpu, 1, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xCD: // SET 1, L (8)
                        ASM_SET_b_r(cpu, 1, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xCE: // SET 1, (HL) (16)
                        ASM_SET_b_m(cpu, mem, 1, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xCF: // SET 1, A (8)
                        ASM_SET_b_r(cpu, 1, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xD0: // SET 2, B (8)
                        ASM_SET_b_r(cpu, 2, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xD1: // SET 2, C (8)
                        ASM_SET_b_r(cpu, 2, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xD2: // SET 2, D (8)
                        ASM_SET_b_r(cpu, 2, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xD3: // SET 2, E (8)
                        ASM_SET_b_r(cpu, 2, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xD4: // SET 2, H (8)
                        ASM_SET_b_r(cpu, 2, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xD5: // SET 2, L (8)
                        ASM_SET_b_r(cpu, 2, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xD6: // SET 2, (HL) (16)
                        ASM_SET_b_m(cpu, mem, 2, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xD7: // SET 2, A (8)
                        ASM_SET_b_r(cpu, 2, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xD8: // SET 3, B (8)
                        ASM_SET_b_r(cpu, 3, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xD9: // SET 3, C (8)
                        ASM_SET_b_r(cpu, 3, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xDA: // SET 3, D (8)
                        ASM_SET_b_r(cpu, 3, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xDB: // SET 3, E (8)
                        ASM_SET_b_r(cpu, 3, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xDC: // SET 3, H (8)
                        ASM_SET_b_r(cpu, 3, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xDD: // SET 3, L (8)
                        ASM_SET_b_r(cpu, 3, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xDE: // SET 3, (HL) (16)
                        ASM_SET_b_m(cpu, mem, 3, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xDF: // SET 3, A (8)
                        ASM_SET_b_r(cpu, 3, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xE0: // SET 4, B (8)
                        ASM_SET_b_r(cpu, 4, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xE1: // SET 4, C (8)
                        ASM_SET_b_r(cpu, 4, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xE2: // SET 4, D (8)
                        ASM_SET_b_r(cpu, 4, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xE3: // SET 4, E (8)
                        ASM_SET_b_r(cpu, 4, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xE4: // SET 4, H (8)
                        ASM_SET_b_r(cpu, 4, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xE5: // SET 4, L (8)
                        ASM_SET_b_r(cpu, 4, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xE6: // SET 4, (HL) (16)
                        ASM_SET_b_m(cpu, mem, 4, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xE7: // SET 4, A (8)
                        ASM_SET_b_r(cpu, 4, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xE8: // SET 5, B (8)
                        ASM_SET_b_r(cpu, 5, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xE9: // SET 5, C (8)
                        ASM_SET_b_r(cpu, 5, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xEA: // SET 5, D (8)
                        ASM_SET_b_r(cpu, 5, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xEB: // SET 5, E (8)
                        ASM_SET_b_r(cpu, 5, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xEC: // SET 5, H (8)
                        ASM_SET_b_r(cpu, 5, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xED: // SET 5, L (8)
                        ASM_SET_b_r(cpu, 5, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xEE: // SET 5, (HL) (16)
                        ASM_SET_b_m(cpu, mem, 5, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xEF: // SET 5, A (8)
                        ASM_SET_b_r(cpu, 5, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xF0: // SET 6, B (8)
                        ASM_SET_b_r(cpu, 6, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xF1: // SET 6, C (8)
                        ASM_SET_b_r(cpu, 6, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xF2: // SET 6, D (8)
                        ASM_SET_b_r(cpu, 6, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xF3: // SET 6, E (8)
                        ASM_SET_b_r(cpu, 6, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xF4: // SET 6, H (8)
                        ASM_SET_b_r(cpu, 6, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xF5: // SET 6, L (8)
                        ASM_SET_b_r(cpu, 6, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xF6: // SET 6, (HL) (16)
                        ASM_SET_b_m(cpu, mem, 6, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xF7: // SET 6, A (8)
                        ASM_SET_b_r(cpu, 6, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    case 0xF8: // SET 7, B (8)
                        ASM_SET_b_r(cpu, 7, &(cpu->B));
                        mCycleTimer = 1;
                        break;

                    case 0xF9: // SET 7, C (8)
                        ASM_SET_b_r(cpu, 7, &(cpu->C));
                        mCycleTimer = 1;
                        break;

                    case 0xFA: // SET 7, D (8)
                        ASM_SET_b_r(cpu, 7, &(cpu->D));
                        mCycleTimer = 1;
                        break;

                    case 0xFB: // SET 7, E (8)
                        ASM_SET_b_r(cpu, 7, &(cpu->E));
                        mCycleTimer = 1;
                        break;

                    case 0xFC: // SET 7, H (8)
                        ASM_SET_b_r(cpu, 7, &(cpu->H));
                        mCycleTimer = 1;
                        break;

                    case 0xFD: // SET 7, L (8)
                        ASM_SET_b_r(cpu, 7, &(cpu->L));
                        mCycleTimer = 1;
                        break;

                    case 0xFE: // SET 7, (HL) (16)
                        ASM_SET_b_m(cpu, mem, 7, cpu->HL);
                        mCycleTimer = 3;
                        break;

                    case 0xFF: // SET 7, A (8)
                        ASM_SET_b_r(cpu, 7, &(cpu->A));
                        mCycleTimer = 1;
                        break;

                    default:
                        printf("Unimplemented 16-bit opcode: cb %02x\n", MEM_getByte(mem, cpu->PC + 1));
                        printf("Address: %04x\n", cpu->PC);
                        return 0; // Failure
                }
                break;

            default:
                printf("Unimplemented opcode: %02x\n", cpu->opcode);
                printf("Address: %04x\n", cpu->PC);
                printf("%04x: %02x - %d %d %d %d - ", cpu->PC, cpu->opcode, CPU_getFlagZ(cpu), CPU_getFlagN(cpu), CPU_getFlagH(cpu), CPU_getFlagC(cpu));
                printf("%02x%02x %02x%02x %02x%02x %02x%02x %02x %02x \n", cpu->A, cpu->F, cpu->B, cpu->C, cpu->D, cpu->E, cpu->H, cpu->L, cpu->SP, MEM_getByte(mem, REG_LY));
                
                /*for (int i = 0x8000; i <= 0x97FF; ++i) {
                    printf("%02x", MEM_getByte(mem, i));
                    if ((i - 0x8000 + 1) % 16 == 0) printf("\n");
                }*/

                return 0; // Failure
        }
    }

    // Handle interrupts and then reset them
    uint8_t IE = mem->logicalMemory[REG_IE];
    uint8_t* IF = &(mem->logicalMemory[REG_IF]);
    if (cpu->IME) {
        if (getBit(IE, 0) && getBit(*IF, 0)) {
            // V-Blank
            cpu->IME = 0;
            *IF = setBit(*IF, 0, 0);
            MEM_pushToStack(mem, &(cpu->SP), cpu->PC);
            cpu->PC = 0x0040;

        } else if (getBit(IE, 1) && getBit(*IF, 1)) {
            // LCD STAT
            cpu->IME = 0;
            *IF = setBit(*IF, 1, 0);
            MEM_pushToStack(mem, &(cpu->SP), cpu->PC);
            cpu->PC = 0x0048;

        } else if (getBit(IE, 2) && getBit(*IF, 2)) {
            // Timer
            cpu->IME = 0;
            *IF = setBit(*IF, 2, 0);
            MEM_pushToStack(mem, &(cpu->SP), cpu->PC);
            cpu->PC = 0x0050;

        } else if (getBit(IE, 3) && getBit(*IF, 3)) {
            // Serial
            cpu->IME = 0;
            *IF = setBit(*IF, 3, 0);
            MEM_pushToStack(mem, &(cpu->SP), cpu->PC);
            cpu->PC = 0x0058;

        } else if (getBit(IE, 4) && getBit(*IF, 4)) {
            // Joypad
            cpu->IME = 0;
            *IF = setBit(*IF, 4, 0);
            MEM_pushToStack(mem, &(cpu->SP), cpu->PC);
            cpu->PC = 0x0060;
        }
    }

    return 1; // success
}
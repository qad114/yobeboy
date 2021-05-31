#include <stdint.h>
#include <stdio.h>
#include "asm.h"
#include "constants.h"
#include "memory.h"


// ADC A, m: Same as ADC A, n but with memory address m
void ASM_ADC_A_m(CPU* cpu, Memory* mem, uint16_t address) {
    int carry = CPU_getFlagC(cpu);
    uint16_t result = cpu->A + MEM_getByte(mem, address) + carry;
    CPU_setFlagZ(cpu, ((uint8_t) result) == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, (((cpu->A & 0b1111) + (MEM_getByte(mem, address) & 0b1111) + (carry & 0b1111)) & 0b10000) == 0b10000);
    CPU_setFlagC(cpu, result > 0xFF);
    cpu->A = (uint8_t) result;
    cpu->PC += 1;
}

// ADC A, n: Add n + carry flag to A
void ASM_ADC_A_n(CPU* cpu, uint8_t* reg) {
    int carry = CPU_getFlagC(cpu);
    uint16_t result = cpu->A + *reg + carry;
    CPU_setFlagZ(cpu, ((uint8_t) result) == 0);
    CPU_setFlagN(cpu, 0);
    //CPU_setFlagH(cpu, (((cpu->A & 0b1111) + ((*reg + carry) & 0b1111)) & 0b10000) == 0b10000);
    CPU_setFlagH(cpu, (((cpu->A & 0b1111) + (*reg & 0b1111) + (carry & 0b1111)) & 0b10000) == 0b10000);
    CPU_setFlagC(cpu, result > 0xFF);
    cpu->A = (uint8_t) result;
    cpu->PC += 1;
}

// ADD A, m: Same as ADD A, n but with memory address m
void ASM_ADD_A_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint16_t result = cpu->A + MEM_getByte(mem, address);
    CPU_setFlagZ(cpu, ((uint8_t) result) == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, (((cpu->A & 0b1111) + (MEM_getByte(mem, address) & 0b1111)) & 0b10000) == 0b10000);
    CPU_setFlagC(cpu, result > 0xFF);
    cpu->A = (uint8_t) result;
    cpu->PC += 1;
}

// ADD A, n: Add n to A
void ASM_ADD_A_n(CPU* cpu, uint8_t* reg) {
    uint16_t result = cpu->A + *reg;
    CPU_setFlagZ(cpu, ((uint8_t) result) == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, (((cpu->A & 0b1111) + (*reg & 0b1111)) & 0b10000) == 0b10000);
    CPU_setFlagC(cpu, result > 0xFF);
    cpu->A = (uint8_t) result;
    cpu->PC += 1;
}

// ADD HL, n: Add n to HL
void ASM_ADD_HL_n(CPU* cpu, uint16_t* reg) {
    uint16_t result = cpu->HL + *reg;
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, ((cpu->HL ^ *reg ^ (result & 0xFFFF)) & 0x1000) != 0);
    CPU_setFlagC(cpu, result < cpu->HL);
    cpu->HL = result;
    cpu->PC += 1;
}

// ADD SP, n: Add next signed byte to SP
void ASM_ADD_SP_n(CPU* cpu, Memory* mem) {
    uint8_t nextByte = MEM_getByte(mem, cpu->PC + 1);
    uint16_t result = cpu->SP + (int8_t) nextByte;
    uint8_t lowResult = (cpu->SP & 0xFF) + nextByte;
    CPU_setFlagZ(cpu, 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, ((((cpu->SP & 0xFF) & 0b1111) + (nextByte & 0b1111)) & 0b10000) == 0b10000);
    CPU_setFlagC(cpu, lowResult < nextByte);
    cpu->SP = result;
    cpu->PC += 2;
}

// AND m: Same as AND n but with memory address m
void ASM_AND_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t result = cpu->A & MEM_getByte(mem, address);
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 1);
    CPU_setFlagC(cpu, 0);
    cpu->A = result;
    cpu->PC += 1;
}

void ASM_AND_n(CPU* cpu, uint8_t* reg) {
    uint8_t result = cpu->A & *reg;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 1);
    CPU_setFlagC(cpu, 0);
    cpu->A = result;
    cpu->PC += 1;
}

// AND #n: Logically AND next byte with A and store in A
void ASM_AND_n_byVal(CPU* cpu, Memory* mem) {
    uint8_t result = cpu->A & MEM_getByte(mem, cpu->PC + 1);
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 1);
    CPU_setFlagC(cpu, 0);
    cpu->PC += 2;
}

// BIT b, m: Same as BIT b, r but with memory address m
void ASM_BIT_b_m(CPU* cpu, Memory* mem, int bit, uint16_t address) {
    CPU_setFlagZ(cpu, !((MEM_getByte(mem, address) & (1 << bit)) >> bit));
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 1);
    cpu->PC += 2;
}

// BIT b, r: Copy complement of bit b in register to the zero flag
void ASM_BIT_b_r(CPU* cpu, int bit, uint8_t* reg) {
    CPU_setFlagZ(cpu, !((*reg & (1 << bit)) >> bit));
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 1);
    cpu->PC += 2;
}

// CALL cc, nn: Push address of next instruction to stack and jump to nn, if condition met
int ASM_CALL_cc_nn(CPU* cpu, Memory* mem, int ccCode) {
    int cond = 0;
    switch (ccCode) {
        case PARAM_CC_Z:
            cond = CPU_getFlagZ(cpu);
            break;
        case PARAM_CC_NZ:
            cond = !CPU_getFlagZ(cpu);
            break;
        case PARAM_CC_C:
            cond = CPU_getFlagC(cpu);
            break;
        case PARAM_CC_NC:
            cond = !CPU_getFlagC(cpu);
            break;
    }
    if (cond) {
        uint16_t jumpAddress = (MEM_getByte(mem, cpu->PC + 2) << 8) | MEM_getByte(mem, cpu->PC + 1);
        uint16_t nextAddress = cpu->PC + 3;
        MEM_pushToStack(mem, &(cpu->SP), nextAddress);
        cpu->PC = jumpAddress;
        return 1;
    } else {
        cpu->PC += 3;
        return 0;
    }
}

// CALL nn: Push address of next instruction onto stack, then jump to nn
void ASM_CALL_nn(CPU* cpu, Memory* mem) {
    uint16_t jumpAddress = (MEM_getByte(mem, cpu->PC + 2) << 8) | MEM_getByte(mem, cpu->PC + 1);
    uint16_t nextAddress = cpu->PC + 3;
    MEM_pushToStack(mem, &(cpu->SP), nextAddress);
    cpu->PC = jumpAddress;
}

// CCF: Complement carry flag
void ASM_CCF(CPU* cpu) {
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, !CPU_getFlagC(cpu));
    cpu->PC += 1;
}

// CPL: Flip all bits of A
void ASM_CPL(CPU* cpu) {
    cpu->A = ~(cpu->A);
    CPU_setFlagN(cpu, 1);
    CPU_setFlagH(cpu, 1);
    cpu->PC += 1;
}

// CP m: Same as CP n but with memory address m
void ASM_CP_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t result = cpu->A - MEM_getByte(mem, address);
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 1);
    CPU_setFlagH(cpu, (((cpu->A & 0xF) - (MEM_getByte(mem, address) & 0xF)) & 0x10) != 0);
    CPU_setFlagC(cpu, result > cpu->A);
    cpu->PC += 1;
}

// CP n: Compare A with n
void ASM_CP_n(CPU* cpu, uint8_t* reg) {
    //CPU_setFlagZ(cpu, cpu->A == *reg);
    //CPU_setFlagN(cpu, 1);
    //CPU_setFlagH(cpu, ((cpu->A & 0b1111) - (*reg & 0b1111)) & 0b10000 == 0b10000);
    //CPU_setFlagC(cpu, cpu->A < *reg);
    uint8_t result = cpu->A - *reg;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 1);
    CPU_setFlagH(cpu, (((cpu->A & 0xF) - (*reg & 0xF)) & 0x10) != 0);
    CPU_setFlagC(cpu, result > cpu->A);
    cpu->PC += 1;
}

// CP #n: Compare A with next byte
void ASM_CP_n_byVal(CPU* cpu, Memory* mem) {
    uint8_t nextByte = MEM_getByte(mem, cpu->PC + 1);
    CPU_setFlagZ(cpu, cpu->A == nextByte);
    CPU_setFlagN(cpu, 1);
    CPU_setFlagH(cpu, (((cpu->A & 0b1111) - (nextByte & 0b1111)) & 0b10000) == 0b10000);
    CPU_setFlagC(cpu, cpu->A < nextByte);
    cpu->PC += 2;
}

// DAA: Decimal adjust A (BCD form)
void ASM_DAA(CPU* cpu) {
    if (!CPU_getFlagN(cpu)) {  // after an addition, adjust if (half-)carry occurred or if result is out of bounds
        if (CPU_getFlagC(cpu) || cpu->A > 0x99) { cpu->A += 0x60; CPU_setFlagC(cpu, 1); }
        if (CPU_getFlagH(cpu) || (cpu->A & 0x0f) > 0x09) { cpu->A += 0x6; }
    } else {  // after a subtraction, only adjust if (half-)carry occurred
        if (CPU_getFlagC(cpu)) { cpu->A -= 0x60; }
        if (CPU_getFlagH(cpu)) { cpu->A -= 0x6; }
    }
    CPU_setFlagZ(cpu, cpu->A == 0);
    CPU_setFlagH(cpu, 0);
    cpu->PC += 1;
}

// DEC m: Same as DEC n but with memory address m
void ASM_DEC_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t result = MEM_getByte(mem, address) - 1;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 1);
    CPU_setFlagH(cpu, (MEM_getByte(mem, address) & 0b10000) != (result & 0b10000));
    MEM_setByte(mem, address, result);
    cpu->PC += 1;
}

// DEC n: Decrement a register
void ASM_DEC_n(CPU* cpu, uint8_t* reg) {
    uint8_t result = *reg - 1;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 1);
    CPU_setFlagH(cpu, (*reg & 0b10000) != (result & 0b10000));
    *reg = result;
    cpu->PC += 1;
}

// DEC nn: Decrement a 16bit register
void ASM_DEC_nn(CPU* cpu, uint16_t* reg) {
    --*reg;
    if (reg == &(cpu->AF)) cpu->F &= 0xF0;
    cpu->PC += 1;
}

// INC m: Same as INC n but with memory address m
void ASM_INC_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t result = MEM_getByte(mem, address) + 1;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    //CPU_setFlagH(cpu, *reg & 0b10000 == 0b10000);
    CPU_setFlagH(cpu, (MEM_getByte(mem, address) & 0b10000) != (result & 0b10000));
    MEM_setByte(mem, address, result);
    cpu->PC += 1;
}

// INC n: Increment a register
void ASM_INC_n(CPU* cpu, uint8_t* reg) {
    uint8_t result = *reg + 1;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    //CPU_setFlagH(cpu, *reg & 0b10000 == 0b10000);
    CPU_setFlagH(cpu, (*reg & 0b10000) != (result & 0b10000));
    *reg = result;
    cpu->PC += 1;
}

// INC nn: Increment a 16bit register
void ASM_INC_nn(CPU* cpu, uint16_t* reg) {
    ++*reg;
    if (reg == &(cpu->AF)) cpu->F &= 0xF0;
    cpu->PC += 1;
}

// JP cc, nn: Jump to address in next 2 bytes if condition met
int ASM_JP_cc_nn(CPU* cpu, Memory* mem, int ccCode) {
    int cond = 0;
    switch (ccCode) {
        case PARAM_CC_Z:
            cond = CPU_getFlagZ(cpu);
            break;
        case PARAM_CC_NZ:
            cond = !CPU_getFlagZ(cpu);
            break;
        case PARAM_CC_C:
            cond = CPU_getFlagC(cpu);
            break;
        case PARAM_CC_NC:
            cond = !CPU_getFlagC(cpu);
            break;
    }
    if (cond) {
        cpu->PC = (MEM_getByte(mem, cpu->PC + 2) << 8) | MEM_getByte(mem, cpu->PC + 1);
        return 1;
    } else {
        cpu->PC += 3;
        return 0;
    }
}

// JP (HL): Jump to address in HL
void ASM_JP_HL(CPU* cpu) {
    cpu->PC = (cpu->H << 8) | cpu->L;
}

// JP nn: Jump to address in next 2 bytes
void ASM_JP_nn(CPU* cpu, Memory* mem) {
    cpu->PC = (MEM_getByte(mem, cpu->PC + 2) << 8) | MEM_getByte(mem, cpu->PC + 1);
}

// JR cc, n: Add next byte to PC if cc condition met
int ASM_JR_cc_n(CPU* cpu, Memory* mem, int ccCode) {
    int cond = 0;
    switch (ccCode) {
        case PARAM_CC_Z:
            cond = CPU_getFlagZ(cpu);
            break;
        case PARAM_CC_NZ:
            cond = !CPU_getFlagZ(cpu);
            break;
        case PARAM_CC_C:
            cond = CPU_getFlagC(cpu);
            break;
        case PARAM_CC_NC:
            cond = !CPU_getFlagC(cpu);
            break;
    }
    if (cond) {
        cpu->PC += ((int8_t) MEM_getByte(mem, cpu->PC + 1)) + 2;
        return 1;
    } else {
        cpu->PC += 2;
        return 0;
    }
}

// JR cc, n: Add next byte to PC
void ASM_JR_n(CPU* cpu, Memory* mem) {
    cpu->PC += ((int8_t) MEM_getByte(mem, cpu->PC + 1)) + 2;
}

// LD A, n: Put value of n into A
void ASM_LD_A_n(CPU* cpu, uint8_t* reg) {
    cpu->A = *reg;
    cpu->PC += 1;
}

// LD A, m: Same as LD A, n but with memory address m
void ASM_LD_A_m(CPU* cpu, Memory* mem, uint16_t address) {
    cpu->A = MEM_getByte(mem, address);
    cpu->PC += 1;
}

// LD (C), A: Put A into address (FF00 + C)
void ASM_LD_C_A(CPU* cpu, Memory* mem) {
    uint16_t address = 0xFF00 + cpu->C;
    MEM_setByte(mem, address, cpu->A);
    cpu->PC += 1;
}

// LD n, A: Put value of A into n
void ASM_LD_n_A(CPU* cpu, uint8_t* reg) {
    *reg = cpu->A;
    cpu->PC += 1;
}

// LD m, A: Same as LD n, A but with memory address m
void ASM_LD_m_A(CPU* cpu, Memory* mem, uint16_t address) {
    MEM_setByte(mem, address, cpu->A);
    cpu->PC += 1;
}

// LD m1, m2: Load value at address m2 into address m1
void ASM_LD_m1_m2(CPU* cpu, Memory* mem, uint16_t address1, uint16_t address2) {
    MEM_setByte(mem, address1, MEM_getByte(mem, address2));
    cpu->PC += 1;
}

// LD nn, n: Load next byte into register
void ASM_LD_nn_n(CPU* cpu, Memory* mem, uint8_t* reg) {
    *reg = MEM_getByte(mem, cpu->PC + 1);
    cpu->PC += 2;
}

// LD (nn), SP: Load SP into address contained in next 2 bytes
void ASM_LD_nn_SP(CPU* cpu, Memory* mem) {
    //cpu->SP = (MEM_getByte(mem, cpu->PC + 2) << 8) | MEM_getByte(mem, cpu->PC + 1);
    uint16_t address = (MEM_getByte(mem, cpu->PC + 2) << 8) | MEM_getByte(mem, cpu->PC + 1);
    MEM_setByte(mem, address, cpu->SP & 0xFF);
    MEM_setByte(mem, address + 1, (cpu->SP & 0xFF00) >> 8);
    cpu->PC += 3;
}

// LD n, nn: Load next 2 bytes into 16-bit register
void ASM_LD_n_nn(CPU* cpu, Memory* mem, uint16_t* reg) {
    *reg = (MEM_getByte(mem, cpu->PC + 2) << 8) | MEM_getByte(mem, cpu->PC + 1);
    if (reg == &(cpu->AF)) cpu->F &= 0xF0;
    cpu->PC += 3;
}

// LD r1, r2: Load value of r2 into r1
void ASM_LD_r1_r2(CPU* cpu, uint8_t* reg1, uint8_t* reg2) {
    *reg1 = *reg2;
    cpu->PC += 1;
}

// LD r1, m: Load value at address m into r1
void ASM_LD_r1_m(CPU* cpu, Memory* mem, uint8_t* reg1, uint16_t address) {
    *reg1 = MEM_getByte(mem, address);
    cpu->PC += 1;
}

// LD m, r2: Load value in r2 into address m
void ASM_LD_m_r2(CPU* cpu, Memory* mem, uint16_t address, uint8_t* reg2) {
    MEM_setByte(mem, address, *reg2);
    cpu->PC += 1;
}

// LD SP, HL: Load HL into SP
void ASM_LD_SP_HL(CPU* cpu) {
    cpu->SP = (cpu->H << 8) | cpu->L;
    cpu->PC += 1;
}

// LDD (HL), A: Put contents of A into address in HL, and decrement HL
void ASM_LDD_HL_A(CPU* cpu, Memory* mem) {
    uint16_t address = (cpu->H << 8) | cpu->L;
    MEM_setByte(mem, address, cpu->A);
    --address;
    cpu->H = (address & 0xFF00) >> 8;
    cpu->L = address & 0x00FF;
    cpu->PC += 1;
}

// LDH A, (n): Put #(FF00 + next byte) into A
void ASM_LDH_A_n(CPU* cpu, Memory* mem) {
    cpu->A = MEM_getByte(mem, 0xFF00 + MEM_getByte(mem, cpu->PC + 1));
    cpu->PC += 2;
}

// LDH (n), A: Put contents of A into address (FF00 + n)
void ASM_LDH_n_A(CPU* cpu, Memory* mem) {
    MEM_setByte(mem, 0xFF00 + MEM_getByte(mem, cpu->PC + 1), cpu->A);
    cpu->PC += 2;
}

// LDHL SP, n: Load address (SP + next byte) into HL
void ASM_LDHL_SP_n(CPU* cpu, Memory* mem) {
    // Add SP and next byte, and set flags
    uint8_t nextByte = MEM_getByte(mem, cpu->PC + 1);
    uint16_t result = cpu->SP + (int8_t) nextByte;
    uint8_t lowResult = (cpu->SP & 0xFF) + nextByte;
    CPU_setFlagZ(cpu, 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, ((((cpu->SP & 0xFF) & 0b1111) + (nextByte & 0b1111)) & 0b10000) == 0b10000);
    CPU_setFlagC(cpu, lowResult < nextByte);

    // Load into HL
    cpu->H = (result & 0xFF00) >> 8;
    cpu->L = result & 0xFF;
    cpu->PC += 2;
}

// LDI A, (HL): Put value at address HL into A, and increment HL
void ASM_LDI_A_HL(CPU* cpu, Memory* mem) {
    uint16_t address = (cpu->H << 8) | cpu->L;
    cpu->A = MEM_getByte(mem, address);
    ++address;
    cpu->H = (address & 0xFF00) >> 8;
    cpu->L = address & 0x00FF;
    cpu->PC += 1;
}

// LDI (HL), A: Put value in A into address in HL, and increment HL
void ASM_LDI_HL_A(CPU* cpu, Memory* mem) {
    uint16_t address = (cpu->H << 8) | cpu->L;
    MEM_setByte(mem, address, cpu->A);
    ++address;
    cpu->H = (address & 0xFF00) >> 8;
    cpu->L = address & 0x00FF;
    cpu->PC += 1;
}

// NOP: Skip to next instruction
void ASM_NOP(CPU* cpu) {
    cpu->PC += 1;
}

// OR m: Same as OR n but with memory address m
void ASM_OR_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t result = MEM_getByte(mem, address) | cpu->A;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, 0);
    cpu->A = result;
    cpu->PC += 1;
}

// OR n: OR n with A and store result in A
void ASM_OR_n(CPU* cpu, uint8_t* reg) {
    uint8_t result = *reg | cpu->A;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, 0);
    cpu->A = result;
    cpu->PC += 1;
}

// POP nn: Pop 2 bytes off stack into register, then increment SP twice
void ASM_POP_nn(CPU* cpu, Memory* mem, uint16_t* reg) {
    *reg = MEM_popFromStack(mem, &(cpu->SP));
    if (reg == &(cpu->AF)) cpu->F &= 0xF0;
    cpu->PC += 1;
}

// PUSH nn: Push 16bit register nn onto stack, then decrement SP twice
void ASM_PUSH_nn(CPU* cpu, Memory* mem, uint16_t* reg) {
    MEM_pushToStack(mem, &(cpu->SP), *reg);
    cpu->PC += 1;
}

// RES b, m: Same as RES b, r but with memory address m
void ASM_RES_b_m(CPU* cpu, Memory* mem, int bit, uint16_t address) {
    //*reg &= ~(1 << bit);
    MEM_setByte(mem, address, MEM_getByte(mem, address) & ~(1 << bit));
    cpu->PC += 2;
}

// RES b, r: Reset bit 'b' in register 'r'
void ASM_RES_b_r(CPU* cpu, int bit, uint8_t* reg) {
    *reg &= ~(1 << bit);
    cpu->PC += 2;
}

// RET: Pop 2 bytes from stack and jump to that address
void ASM_RET(CPU* cpu, Memory* mem) {
    cpu->PC = MEM_popFromStack(mem, &(cpu->SP));
}

// RET cc: Return if condition met
int ASM_RET_cc(CPU* cpu, Memory* mem, int ccCode) {
    int cond = 0;
    switch (ccCode) {
        case PARAM_CC_Z:
            cond = CPU_getFlagZ(cpu);
            break;
        case PARAM_CC_NZ:
            cond = !CPU_getFlagZ(cpu);
            break;
        case PARAM_CC_C:
            cond = CPU_getFlagC(cpu);
            break;
        case PARAM_CC_NC:
            cond = !CPU_getFlagC(cpu);
            break;
    }
    if (cond) {
        cpu->PC = MEM_popFromStack(mem, &(cpu->SP));
        return 1;
    } else {
        cpu->PC += 1;
        return 0;
    }
}

// RETI: Pop 2 bytes from stack and jump to that address, then enable interrupts
void ASM_RETI(CPU* cpu, Memory* mem) {
    cpu->PC = MEM_popFromStack(mem, &(cpu->SP));
    cpu->IME = 1;
}

// RL m: Same as RL n but with memory address m
void ASM_RL_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t bit7 = (MEM_getByte(mem, address) & 0b10000000) >> 7;
    MEM_setByte(mem, address, (MEM_getByte(mem, address) << 1) | (uint8_t) CPU_getFlagC(cpu));
    CPU_setFlagZ(cpu, MEM_getByte(mem, address) == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit7);
    cpu->PC += 2;
}

// RL n: Rotate register n left through carry flag
void ASM_RL_n(CPU* cpu, uint8_t* reg) {
    uint8_t bit7 = (*reg & 0b10000000) >> 7;
    *reg = (*reg << 1) | (uint8_t) CPU_getFlagC(cpu);
    CPU_setFlagZ(cpu, *reg == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit7);
    cpu->PC += 2;
}

// RLA: Rotate A left through carry flag
void ASM_RLA(CPU* cpu) {
    uint8_t bit7 = (cpu->A & 0b10000000) >> 7;
    cpu->A = (cpu->A << 1) | (uint8_t) CPU_getFlagC(cpu);
    CPU_setFlagZ(cpu, 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit7);
    cpu->PC += 1;
}

// RLC m: Same as RLC n but with memory address m
void ASM_RLC_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t bit7 = (MEM_getByte(mem, address) & 0b10000000) >> 7;
    MEM_setByte(mem, address, (MEM_getByte(mem, address) << 1) | bit7);
    CPU_setFlagZ(cpu, MEM_getByte(mem, address) == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit7);
    cpu->PC += 2;
}

// RLC n: Rotate N left, and set old bit 7 to carry flag
void ASM_RLC_n(CPU* cpu, uint8_t* reg) {
    uint8_t bit7 = (*reg & 0b10000000) >> 7;
    *reg = (*reg << 1) | bit7;
    CPU_setFlagZ(cpu, *reg == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit7);
    cpu->PC += 2;
}

// RLCA: Rotate A left
void ASM_RLCA(CPU* cpu) {
    uint8_t bit7 = (cpu->A & 0b10000000) >> 7;
    cpu->A = (cpu->A << 1) | bit7;
    CPU_setFlagZ(cpu, 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit7);
    cpu->PC += 1;
}

// RR m: Same as RR n but with memory address m
void ASM_RR_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t bit0 = MEM_getByte(mem, address) & 1;
    MEM_setByte(mem, address, (MEM_getByte(mem, address) >> 1) | (CPU_getFlagC(cpu) << 7));
    CPU_setFlagZ(cpu, MEM_getByte(mem, address) == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit0);
    cpu->PC += 2;
}

// RR n: Rotate n right through carry flag
void ASM_RR_n(CPU* cpu, uint8_t* reg) {
    uint8_t bit0 = *reg & 1;
    *reg = (*reg >> 1) | (CPU_getFlagC(cpu) << 7);
    CPU_setFlagZ(cpu, *reg == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit0);
    cpu->PC += 2;
}

// RRA: Rotate A right through carry flag
void ASM_RRA(CPU* cpu) {
    uint8_t bit0 = cpu->A & 1;
    cpu->A = (cpu->A >> 1) | (CPU_getFlagC(cpu) << 7);
    CPU_setFlagZ(cpu, 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit0);
    cpu->PC += 1;
}

// RRC m: Same as RRC n but with memory address m
void ASM_RRC_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t bit0 = MEM_getByte(mem, address) & 1;
    MEM_setByte(mem, address, (MEM_getByte(mem, address) >> 1) | (bit0 << 7));
    CPU_setFlagZ(cpu, MEM_getByte(mem, address) == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit0);
    cpu->PC += 2;
}

// RRC n: Rotate n right, and set old bit 0 to carry flag
void ASM_RRC_n(CPU* cpu, uint8_t* reg) {
    uint8_t bit0 = *reg & 1;
    *reg = (*reg >> 1) | (bit0 << 7);
    CPU_setFlagZ(cpu, *reg == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit0);
    cpu->PC += 2;
}

// RRCA: Rotate A right, and set old bit 0 to carry flag
void ASM_RRCA(CPU* cpu) {
    uint8_t bit0 = cpu->A & 1;
    cpu->A = (cpu->A >> 1) | (bit0 << 7);
    CPU_setFlagZ(cpu, 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (int) bit0);
    cpu->PC += 1;
}

// RST n: Push current address onto stack, then jump to address (0000 + n)
void ASM_RST_n(CPU* cpu, Memory* mem, uint8_t address) {
    MEM_pushToStack(mem, &(cpu->SP), cpu->PC + 1);
    cpu->PC = 0x0000 + address;
}

// SBC A, m: Same as SBC A, n but with memory address m
void ASM_SBC_A_m(CPU* cpu, Memory* mem, uint16_t address) {
    int carry = CPU_getFlagC(cpu);
    int result = cpu->A - MEM_getByte(mem, address) - carry;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 1);
    CPU_setFlagH(cpu, ((cpu->A & 0x0F) - (MEM_getByte(mem, address) & 0x0F) - carry) < 0);
    CPU_setFlagC(cpu, result < 0);
    cpu->A = (uint8_t) result;
    cpu->PC += 1;
}

// SBC A, n: Subtract (n + carry flag) from A
void ASM_SBC_A_n(CPU* cpu, uint8_t* reg) {
    int carry = CPU_getFlagC(cpu);
    int result = cpu->A - *reg - carry;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 1);
    CPU_setFlagH(cpu, ((cpu->A & 0x0F) - (*reg & 0x0F) - carry) < 0);
    CPU_setFlagC(cpu, result < 0);
    cpu->A = (uint8_t) result;
    cpu->PC += 1;
}

// SCF: Set carry flag
void ASM_SCF(CPU* cpu) {
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, 1);
    cpu->PC += 1;
}

// SLA m: Same as SLA n but with memory address m
void ASM_SLA_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t result = MEM_getByte(mem, address) << 1;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (MEM_getByte(mem, address) & 0b10000000) >> 7);
    MEM_setByte(mem, address, result);
    cpu->PC += 2;
}

// SLA n: Shift n left into carry flag
void ASM_SLA_n(CPU* cpu, uint8_t* reg) {
    uint8_t result = *reg << 1;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, (*reg & 0b10000000) >> 7);
    *reg = result;
    cpu->PC += 2;
}

// SET b, m: Same as SET b, r but with memory address m
void ASM_SET_b_m(CPU* cpu, Memory* mem, int bit, uint16_t address) {
    MEM_setByte(mem, address, MEM_getByte(mem, address) | (1 << bit));
    cpu->PC += 2;
}

// SET b, r: Set bit 7 in register
void ASM_SET_b_r(CPU* cpu, int bit, uint8_t* reg) {
    *reg |= (1 << bit);
    cpu->PC += 2;
}

// SRA m: Same as SRA n but with memory address m
void ASM_SRA_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t bit7mask = MEM_getByte(mem, address) & 0b10000000;
    uint8_t result = (MEM_getByte(mem, address) >> 1) | bit7mask;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, MEM_getByte(mem, address) & 1);
    MEM_setByte(mem, address, result);
    cpu->PC += 2;
}

// SRA n: Shift n right to carry flag, and leave MSB unchanged
void ASM_SRA_n(CPU* cpu, uint8_t* reg) {
    uint8_t bit7mask = *reg & 0b10000000;
    uint8_t result = (*reg >> 1) | bit7mask;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, *reg & 1);
    *reg = result;
    cpu->PC += 2;
}

// SRL m: Same as SRL n but with memory address m
void ASM_SRL_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t result = MEM_getByte(mem, address) >> 1;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, MEM_getByte(mem, address) & 1);
    MEM_setByte(mem, address, result);
    cpu->PC += 2;
}

// SRL n: Shift n right into carry and set MSB to 0
void ASM_SRL_n(CPU* cpu, uint8_t* reg) {
    uint8_t result = *reg >> 1;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, *reg & 1);
    *reg = result;
    cpu->PC += 2;
}

// SUB m: Same as SUB n but with memory address m
void ASM_SUB_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t result = cpu->A - MEM_getByte(mem, address);
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 1);
    CPU_setFlagH(cpu, (cpu->A & 0x0F) < (MEM_getByte(mem, address) & 0x0F));
    CPU_setFlagC(cpu, result > cpu->A);
    cpu->A = result;
    cpu->PC += 1;
}

// SUB n: Subtract n from A
void ASM_SUB_n(CPU* cpu, uint8_t* reg) {
    uint8_t result = cpu->A - *reg;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 1);
    //CPU_setFlagH(cpu, ((cpu->A & 0b1111) - (*reg & 0b1111)) & 0b10000 == 0b10000);
    //CPU_setFlagH(cpu, (result & 0xF0) != (cpu->A & 0xF0));
    CPU_setFlagH(cpu, (cpu->A & 0x0F) < (*reg & 0x0F));
    CPU_setFlagC(cpu, result > cpu->A);
    cpu->A = result;
    cpu->PC += 1;
}

// SWAP m: Same as SWAP n but with memory address m
void ASM_SWAP_m(CPU* cpu, Memory* mem, uint16_t address) {
    MEM_setByte(mem, address, (MEM_getByte(mem, address) << 4) | (MEM_getByte(mem, address) & 0xF0) >> 4);
    CPU_setFlagZ(cpu, MEM_getByte(mem, address) == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, 0);
    cpu->PC += 2;
}

// SWAP n: Swap upper and lower nibbles of register
void ASM_SWAP_n(CPU* cpu, uint8_t* reg) {
    *reg = (*reg << 4) | (*reg & 0xF0) >> 4;
    CPU_setFlagZ(cpu, *reg == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, 0);
    cpu->PC += 2;
}

// XOR n: Same as XOR n but with memory address m
void ASM_XOR_m(CPU* cpu, Memory* mem, uint16_t address) {
    uint8_t result = MEM_getByte(mem, address) ^ cpu->A;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, 0);
    cpu->A = result;
    cpu->PC += 1;
}

// XOR n: Bitwise XOR register n with A, put result in A
void ASM_XOR_n(CPU* cpu, uint8_t* reg) {
    uint8_t result = *reg ^ cpu->A;
    CPU_setFlagZ(cpu, result == 0);
    CPU_setFlagN(cpu, 0);
    CPU_setFlagH(cpu, 0);
    CPU_setFlagC(cpu, 0);
    cpu->A = result;
    cpu->PC += 1;
}
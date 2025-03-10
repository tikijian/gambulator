#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../types.h"
#include "../routines.h"
#include "cpu.h"
#include "memory.h"
#include "cpu_helpers.h"

byte_t cpu_F() {
    return cpu.FZ << FLAG_ZERO_BIT | cpu.FN << FLAG_SUB_BIT | cpu.FH << FLAG_HALF_CARRY_BIT | cpu.FC << FLAG_CARRY_BIT;
}
word_t cpu_AF() {
    return bytes_to_word(cpu.A, cpu_F());
}
word_t cpu_HL() {
    return bytes_to_word(cpu.H, cpu.L);
}
word_t cpu_BC() {
    return bytes_to_word(cpu.B, cpu.C);
}
word_t cpu_DE() {
    return bytes_to_word(cpu.D, cpu.E);
}

void cpu_set_BC(word_t value) {
    cpu.B = value >> 8;
    cpu.C = value & 0x00FF;
}
void cpu_set_DE(word_t value) {
    cpu.D = value >> 8;
    cpu.E = value & 0x00FF;
}
void cpu_set_HL(word_t value) {
    cpu.H = value >> 8;
    cpu.L = value & 0x00FF;
}

byte_t cpu_get_reg_by_code(opcode_t code) {
    switch(code) {
        case 0xC6: case 0xD6: case 0xE6: case 0xF6:
        case 0xCE: case 0xDE: case 0xEE: case 0xFE:
            byte_t value = mem_read(cpu.PC);
            cpu.PC++;
            return value;
    }

    switch (first_bit(code))
    {
        case 0x0: return cpu.B;
        case 0x1: return cpu.C;
        case 0x2: return cpu.D;
        case 0x3: return cpu.E;
        case 0x4: return cpu.H;
        case 0x5: return cpu.L;
        case 0x6: return mem_read(cpu_HL());
        case 0x7: return cpu.A;
        case 0x8: return cpu.B;
        case 0x9: return cpu.C;
        case 0xA: return cpu.D;
        case 0xB: return cpu.E;
        case 0xC: return cpu.H;
        case 0xD: return cpu.L;
        case 0xE: return mem_read(cpu_HL());
        case 0xF: return cpu.A;
    }
    printf("cpu_get_reg_by_code: unknown case 0x%02x\n", code);
    exit(-1);
}

void cpu_set_reg_by_code(opcode_t code, byte_t value) {
    switch (code) {
        case 0x40 ... 0x47:
            cpu.B = value;
            return;
        case 0x48 ... 0x4F:
            cpu.C = value;
            return;
        case 0x50 ... 0x57:
            cpu.D = value;
            return;
        case 0x58 ... 0x5F:
            cpu.E = value;
            return;
        case 0x60 ... 0x67:
            cpu.H = value;
            return;
        case 0x68 ... 0x6F:
            cpu.L = value;
            return;
        case 0x78 ... 0x7F:
            cpu.A = value;
            return;
        default:
            printf("cpu_set_reg_by_code: unknown case 0x%02x\n", code);
            exit(-1);
    }
}

word_t cpu_stack_target_by_code(opcode_t code) {
    switch (last_bit(code)) {
        case 0xC0: return cpu_BC();
        case 0xD0: return cpu_DE();
        case 0xE0: return cpu_HL();
        case 0xF0: return cpu_AF();
        default: {
            printf("stack_target_by_code: unknown case 0x%02x\n", code);
            exit(-1);
        }
    }
}

/*
    Universal Cpu Flag register updater.
*/
void cpu_update_flags(byte_t a, byte_t b, word_t result, char* flag_operations){
    // Z flag
    if (flag_operations[0] != '-') {
        if (flag_operations[0] == 'Z') {
            cpu.FZ = CHECK_ZERO(result) ? 1 : 0;
        } else {
            cpu.FZ = flag_operations[0] == '1' ? 1 : 0;
        }
    }

    // SUB flag (N)
    if (flag_operations[1] != '-') {
        cpu.FN = flag_operations[1] == '1' ? 1 : 0;
    }

    // Half-Carry flag
    if (flag_operations[2] != '-') {
        if (flag_operations[2] == 'H') {
            cpu.FH = CHECK_HALF_CARRY(a, b, result);
        } else {
            cpu.FH = flag_operations[2] == '1' ? 1 : 0;
        }
    }
    
    // Carry flag
    if (flag_operations[3] != '-') {
        if (flag_operations[3] == 'C') {
            cpu.FC = CHECK_UNDERFLOW(result) || CHECK_OVERFLOW(result);   
        } else {
            cpu.FC = flag_operations[3] == '1' ? 1 : 0;
        }
    }
}

void cpu_push_pc()
{
    cpu.SP--;
    mem_write(cpu.SP, MS_BYTE(cpu.PC));
    cpu.SP--;
    mem_write(cpu.SP, LS_BYTE(cpu.PC));
}

word_t cpu_stack_pop()
{
    byte_t lsb = mem_read(cpu.SP);
    cpu.SP++;
    byte_t msb = mem_read(cpu.SP);
    cpu.SP++;
    return bytes_to_word(msb, lsb);
}

void log_cpu_full(opcode_t opcode) {
    printf("PC: 0x%04X, OP: 0x%02X, SP: %04X, A: %02X, B: %02X, C: %02X, D: %02X, E: %02X, H: %02X, L: %02X, Z: %i\n", cpu.PC, opcode, cpu.SP, cpu.A, cpu.B, cpu.C, cpu.D, cpu.E, cpu.H, cpu.L, cpu.FZ);
}

void log_cpu_full_16(opcode_t opcode) {
    printf("PC: 0x%04X, OP: %02X, SP: %04X, AF: %04X, BC: %04X, DE: %04X, HL: %04X, Z: %i\n", cpu.PC, opcode, cpu.SP, cpu_AF(), cpu_BC(), cpu_DE(), cpu_HL(), cpu.FZ);
}

/*
    CB-prefixed Opcodes and helpers
*/

// Rotate right through Carry
byte_t RR(byte_t target) {
    byte_t result = R_ROTATE_THROUGH_CARRY(target);
    cpu.FZ = result == 0;
    cpu.FC = target & 0x01;
    cpu.FH = cpu.FN = 0;
    return result;
}

// Shift right into Carry
byte_t SRL(byte_t target) {
    byte_t result = target >> 1;
    cpu.FZ = result == 0;
    cpu.FC = target & 0x01;
    cpu.FH = cpu.FN = 0;
    return result;
}

byte_t SWAP(byte_t target) {
    byte_t result = LS_BYTE(target) | MS_BYTE(target);
    cpu.FZ = result == 0;
    cpu.FC = cpu.FH = cpu.FN = 0;
    return result;
}

cycle_t cpu_exec_CB_opcode(opcode_t opcode) {
    byte_t val;
    byte_t should_update = 1;
    switch (first_bit(opcode))
    {
        case 0x0:
        case 0x8:
            val = cpu.B; break;
        case 0x1:
        case 0x9:
            val = cpu.C; break;
        case 0x2:
        case 0xA:
            val = cpu.D; break;
        case 0x3:
        case 0xB:
            val = cpu.E; break;
        case 0x4:
        case 0xC:
            val = cpu.H; break;
        case 0x5:
        case 0xD:
            val = cpu.L; break;
        case 0x6:
        case 0xE:
            val = mem_read(cpu_HL()); break;
        case 0x7:
        case 0xF:
            val = cpu.A; break;
        default:
            printf("exec_CB_opcde get target value: unknown case 0x%02x\n", opcode);
            exit(-1);
    }

    byte_t result;
    switch(opcode) {
        case 0x18 ... 0x1F:
            result = RR(val); break;
        case 0x30 ... 0x37:
            result = SWAP(val); break;
        case 0x38 ... 0x3F:
            result = SRL(val); break;
        default:
            printf("exec_CB_opcde unknown operation: 0x%02x\n", opcode);
            exit(-1);
    }

    if (!should_update) {
        return 0;
    }

    switch (first_bit(opcode))
    {
        case 0x0:
        case 0x8:
            cpu.B = result; break;
        case 0x1:
        case 0x9:
            cpu.C = result; break;
        case 0x2:
        case 0xA:
            cpu.D = result; break;
        case 0x3:
        case 0xB:
            cpu.E = result; break;
        case 0x4:
        case 0xC:
            cpu.H = result; break;
        case 0x5:
        case 0xD:
            cpu.L = result; break;
        case 0x6:
        case 0xE:
            mem_write(cpu_HL(), result); break;
        case 0x7:
        case 0xF:
            cpu.A = result; break;
        default:
            printf("exec_CB_opcde get target value: unknown case 0x%02x\n", opcode);
            exit(-1);
    }
    return 0;
}
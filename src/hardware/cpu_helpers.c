#include <stdio.h>
#include <stdarg.h>

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
    mem_write_byte(cpu.SP, MS_BYTE(cpu.PC));
    cpu.SP--;
    mem_write_byte(cpu.SP, LS_BYTE(cpu.PC));
}

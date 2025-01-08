#include <stdio.h>
#include <stdarg.h>

#include "../types.h"
#include "../routines.h"
#include "cpu.h"
#include "memory.h"
#include "cpu_helpers.h"


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

/*
    Universal Cpu Flag register updater.
    @param byte_t a - first operand
*/
void cpu_update_flags(byte_t a, byte_t b, byte_t result, char* flag_operations){
    byte_t flags = cpu.F;
    
    // Z flag
    if (flag_operations[0] != '-') {
        if (flag_operations[0] == 'Z') {
            flags = FLAG_SET_ZERO(result == 0 ? 1 : 0, flags);
        } else {
            byte_t val = flag_operations[0] == '1' ? 1 : 0;
            flags = FLAG_SET_ZERO(val, flags);
        }
    }

    // SUB flag (N)
    if (flag_operations[1] != '-') {
        byte_t val = flag_operations[1] == '1' ? 1 : 0;
        flags = FLAG_SET_SUB(val, flags);
    }

    // Half-Carry flag
    if (flag_operations[2] != '-') {
        if (flag_operations[2] == 'H') {
            if (FLAG_ADD_HALF_CARRY(a, b, result)) {
                flags = FLAG_SET_HALF_CARRY(1, flags);   
            } else {
                flags = FLAG_SET_HALF_CARRY(0, flags);   
            }
        } else {
            byte_t val = flag_operations[2] == '1' ? 1 : 0;
            flags = FLAG_SET_HALF_CARRY(val, flags);
        }
    }
    
    // Half-Carry flag
    // TODO: check for 00 - 10 = -10
    if (flag_operations[3] != '-') {
        if (flag_operations[3] == 'C') {
            flags = FLAG_SET_CARRY(result > 0xFF ? 1 : 0, flags);   
        } else {
            byte_t val = flag_operations[3] == '1' ? 1 : 0;
            flags = FLAG_SET_CARRY(val, flags);
        }
    }

    cpu.F = flags;
}

void cpu_push_pc()
{
    cpu.SP--;
    mem_write_byte(cpu.SP, MS_BYTE(cpu.PC));
    cpu.SP--;
    mem_write_byte(cpu.SP, LS_BYTE(cpu.PC));
}

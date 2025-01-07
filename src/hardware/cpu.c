#include <stdlib.h>
#include "stdio.h"

#include "../constants.h"
#include "../routines.h"
#include "cpu.h"
#include "memory.h"
#include "cpu_helpers.h"


opcode_handler_t opcodes[0xFF];

struct CPU cpu = {
        .A = 0x01,
        .B = 0x0,
        .C = 0x13,
        .D = 0x0,
        .E = 0xd8,
        .H = 0x01,
        .L = 0x4d,
        .F = 0xb0,
        
        .SP = 0xfffe,
        .PC = 0x0100,
};

void cpu_exec(opcode_t opcode, void* mem) {
    if (opcode == OPCODE_PREFIX) {
        // read next instruction after prefix-opcode
        // TODO: check if should rewrite opcode variable or make new
        opcode = mem_read(cpu.PC + 1);
    }

    printf("CPU-PC: 0x%04X, OP: 0x%02X\n", cpu.PC, opcode);
    // NOP
    if (opcode == 0x00) {
        cpu.PC++;
        return;
    }
    
    if (!opcodes[opcode]) {
        printf("CPU: Unknown OP-code!\n");
        exit(-1);
    }

    cpu.PC++;
    opcodes[opcode](opcode);
}


/* Control Flow-s */
void JP_nn(opcode_t) {
    cpu.PC = bytes_to_word(mem_read(cpu.PC + 1), mem_read(cpu.PC));
}
/* -------------- */

/* 8-bit loads */
void LD_mem_from_reg(opcode_t current_opcode) {
    word_t addr = cpu_HL();
    byte_t data = cpu_get_reg_by_code(current_opcode);
    mem_write_byte(cpu_HL(), data);
}
/* -------------- */

/* 16-bit arithmetics */
void INC_16(opcode_t current_opcode) {
    switch (last_bit(current_opcode)) {
        case 0x00: {
            word_t value = bytes_to_word(cpu.B, cpu.C);
            value++;
            cpu_set_BC(value);
            break;
        };
        case 0x10: {
            word_t value = bytes_to_word(cpu.D, cpu.E);
            value++;
            cpu_set_DE(value);
            break;
        };
        case 0x20: {
            word_t value = bytes_to_word(cpu.H, cpu.L);
            value++;
            cpu_set_HL(value);
            break;
        };
        case 0x30: {
            cpu.SP++;
            break;
        };
        default: {
            printf("INC_16: unknown case 0x%02x\n", current_opcode);
            exit(-1);
        }
    }
}

void DEC_16(opcode_t current_opcode) {
    printf("BC: %04x\n", bytes_to_word(cpu.B, cpu.C));
    switch (last_bit(current_opcode)) {
        case 0x00: {
            word_t value = bytes_to_word(cpu.B, cpu.C);
            value--;
            cpu_set_BC(value);
            break;
        };
        case 0x10: {
            word_t value = bytes_to_word(cpu.D, cpu.E);
            value--;
            cpu_set_DE(value);
            break;
        };
        case 0x20: {
            word_t value = bytes_to_word(cpu.H, cpu.L);
            value--;
            cpu_set_HL(value);
            break;
        };
        case 0x30: {
            cpu.SP--;
            break;
        };
        default: {
            printf("DEC_16: unknown case 0x%02x\n", current_opcode);
            exit(-1);
        }
    }
    printf("BC: %04x\n", bytes_to_word(cpu.B, cpu.C));
}
/* -------------- */

/* 8-Bit Arithmetics */
void ADD_reg_to_A(opcode_t current_opcode) {
    byte_t value = cpu_get_reg_by_code(current_opcode);
    byte_t result = cpu.A + value;
    cpu_update_flags(cpu.A, value, result, "Z0HC");

    // should this happen before flags update??
    if (result > 0xFF) {
        result = result - 256;
    }

    printf("ADD A: a: %i, val: %i, result %i\n", cpu.A, value, result);
    cpu.A = result;    
}
/* -------------- */

opcode_handler_t opcodes[0xFF] = {
    [0x03] = INC_16,
    [0x13] = INC_16,
    [0x23] = INC_16,
    [0x33] = INC_16,

    [0x0b] = DEC_16,
    [0x1b] = DEC_16,
    [0x2b] = DEC_16,
    [0x3b] = DEC_16,

    [0x70] = LD_mem_from_reg,
    [0x71] = LD_mem_from_reg,
    [0x72] = LD_mem_from_reg,
    [0x73] = LD_mem_from_reg,
    [0x74] = LD_mem_from_reg,
    [0x75] = LD_mem_from_reg,
    [0x76] = NULL, // TODO: HALT
    [0x77] = LD_mem_from_reg,

    [0x80] = ADD_reg_to_A,
    [0x81] = ADD_reg_to_A,
    [0x82] = ADD_reg_to_A,
    [0x83] = ADD_reg_to_A,
    [0x84] = ADD_reg_to_A,
    [0x85] = ADD_reg_to_A,
    [0x86] = ADD_reg_to_A,
    [0x87] = ADD_reg_to_A,

    [0xc3] = JP_nn,
};

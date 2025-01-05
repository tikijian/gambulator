#include "stdio.h"

#include "../constants.h"
#include "../routines.h"
#include "cpu.h"
#include "memory.h"


opcode_handler_t opcodes[512];

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

void
cpu_exec(opcode_t opcode, void* mem) {
    if (opcode == OPCODE_PREFIX) {
        // read next instruction after prefix-opcode
        // TODO: check if should rewrite opcode variable or make new
        opcode = mem_read(cpu.PC + 1);
    }

    printf("CPU-PC: 0x%04X, OP: 0x%02X\n", cpu.PC, opcode);
    
    if (!opcodes[opcode]) {
        printf("CPU: Unknown OP-code!\n");
        return;
    }

    cpu.PC++;
    opcodes[opcode](opcode);
}

/* Control Flow-s */
void JP_nn() {
    cpu.PC = bytes_to_word(mem_read(cpu.PC), mem_read(cpu.PC + 1));
    printf("jump to %04x\n", cpu.PC);
}
/* -------------- */

/* 16-bit arithmetics */
void INC_16reg(opcode_t current_opcode) {
    printf("currrent opcde in INC_16 %04x\n", current_opcode);
}

/* -------------- */


opcode_handler_t opcodes[512] = {
    [0x03] = INC_16reg,
    [0x13] = INC_16reg,
    [0x23] = INC_16reg,
    [0x33] = INC_16reg,
    [0xc3] = JP_nn,
};

#include "stdio.h"

#include "../constants.h"
#include "cpu.h"
#include "memory.h"

struct CPU cpu = {
        .A = 0x01,
        .B = 0x0,
        .C = 0x13,
        .D = 0x0,
        .E = 0xd8,
        .H = 0x01,
        .L = 0x4d,
        
        .SP = 0xfffe,
        .PC = 0x0100,

        .flags = {
            .C = 1,
            .H = 1,
            .N = 0,
            .Z = 1,
        }
};

void
cpu_exec(opcode_t opcode, void* mem) {
    if (opcode == OPCODE_PREFIX) {
        // read next instruction after prefix-opcode
        // TODO: check if should rewrite opcode variable or make new
        opcode = mem_read(cpu.PC + 1);
    }


    printf("CPU-PC: %02X, current opcode: %02X\n", cpu.PC, opcode);
    
    switch (opcode) {
        case 0xc3: { 
            cpu.PC = 1;
            break;
        }
        default: {
            cpu.PC++;
        }
    }
}

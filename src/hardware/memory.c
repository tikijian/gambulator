#include <stdlib.h>
#include <stdio.h>

#include "../types.h"
#include "../constants.h"
#include "../routines.h"
#include "memory.h"


byte_t* mem;

void*
mem_init() {
    mem = calloc(MEM_TOTAL + 1, sizeof(byte_t));
    return (byte_t*)mem;
}

byte_t
mem_read(word_t address) {
    if (address > 0xFFFF) {
        printf("MEM: unable to read address 0x%04X\n", address);
        exit(-1);
    }
    return mem[address];
}

void
mem_free()
{
    free(mem);
}

void mem_write(word_t address, byte_t value)
{
    if (address > 0xFFFF) {
        printf("MEM: unable to write at address 0x%04X\n", address);
        exit(-1);
    }

    // custom writes
    switch (address) {
        case REG_DIV:
            mem[address] = 0x00;
            return;
        default:
            // regular write
            mem[address] = value;
    }
}

word_t mem_read_word(word_t current_pc) {
    return bytes_to_word(mem_read(current_pc + 1), mem_read(current_pc));
}

void reg_increment_TIMA() {
    byte_t tima = mem_read(REG_TIMA);
    // When the value overflows (exceeds $FF) it is reset to the value specified in TMA (FF06)
    // and an interrupt is requested, as described below.
    if (tima == 0xFF) {
        tima = mem_read(REG_TMA);
        // TODO: request interrupt
    } else {
        mem_write(REG_TIMA, tima + 1);
    }
}

#include <stdlib.h>
#include <stdio.h>

#include "../types.h"
#include "../constants.h"
#include "../routines.h"
#include "memory.h"

byte_t* memory = NULL;

void*
mem_init() {
    printf("mem addr is (at memory.c) %p\n", memory);
    memory = calloc(MEM_TOTAL + 1, sizeof(byte_t));
    printf("mem addr is (at memory.c after allocation) %p\n", memory);
    return (byte_t*)memory;
}

byte_t
mem_read(word_t address) {
    if (address > 0xFFFF) {
        printf("MEM: unable to read address 0x%04X\n", address);
        exit(-1);
    }
    return memory[address];
}

void
mem_free()
{
    free(memory);
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
            memory[address] = 0x00;
            return;
        default:
            // regular write
            memory[address] = value;
    }
}

word_t mem_read_word(word_t current_pc) {
    return bytes_to_word(mem_read(current_pc + 1), mem_read(current_pc));
}

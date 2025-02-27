#include <stdlib.h>
#include <stdio.h>

#include "../types.h"
#include "../constants.h"
#include "../routines.h"
#include "memory.h"
#include "clock.h"

byte_t* memory = NULL;

void*
mem_init() {
    memory = calloc(MEM_TOTAL + 1, sizeof(byte_t));
    mem_set_value(REG_DIV, 0xAB);
    return (byte_t*)memory;
}

byte_t
mem_read(word_t address) {
    if (address > 0xFFFF) {
        printf("MEM: unable to read address 0x%04X\n", address);
        exit(-1);
    }
    clock.cycles++;
    return memory[address];
}

byte_t mem_get_value(word_t address)
{
    return memory[address];
}
byte_t mem_set_value(word_t address, byte_t value)
{
    return memory[address] = value;
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

    memory[address] = value;
    clock.cycles++;
}

word_t mem_read_word(word_t current_pc) {
    return bytes_to_word(mem_read(current_pc + 1), mem_read(current_pc));
}

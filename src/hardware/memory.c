#include <stdlib.h>

#include "../types.h"
#include "../constants.h"


byte_t* mem;

void*
mem_init() {
    mem = calloc(MEM_TOTAL, sizeof(byte_t));
    return (byte_t*)mem;
}

byte_t
mem_read(word_t address) {
    return mem[address];
}

void
mem_free()
{
    free(mem);
}

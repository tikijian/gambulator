#include "../types.h"

#ifndef _MEMORY_H
# define _MEMORY_H

extern byte_t* memory;

void*  mem_init();
byte_t mem_read(word_t address);
byte_t mem_get_value(word_t address);
byte_t mem_set_value(word_t address, byte_t value);
word_t mem_read_word(word_t address);
void   mem_free();
void   mem_write(word_t address, byte_t value);

#endif
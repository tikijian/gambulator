#include "../types.h"

#ifndef _MEMORY_H
# define _MEMORY_H

void*  mem_init();
byte_t mem_read(word_t address);
word_t mem_read_word(word_t address);
void   mem_free();
void   mem_write(word_t address, byte_t value);

void   reg_increment_TIMA();

#endif
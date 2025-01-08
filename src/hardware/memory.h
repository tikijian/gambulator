#include "../types.h"

void*  mem_init();
byte_t mem_read(word_t address);
word_t mem_read_word(word_t address);
void   mem_free();
void   mem_write_byte(word_t address, byte_t value);
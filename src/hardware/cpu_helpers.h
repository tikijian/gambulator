#include "../types.h"

word_t cpu_HL();

word_t cpu_BC();

word_t cpu_DE();

void cpu_set_BC(word_t value);

void cpu_set_DE(word_t value);

void cpu_set_HL(word_t value);

byte_t cpu_get_reg_by_code(opcode_t code);

void cpu_update_flags(byte_t a, byte_t b, byte_t result, char* flags_operations);
#include "../types.h"

#ifndef _CPU_HELPERS_H
# define _CPU_HELPERS_H

byte_t cpu_F();
word_t cpu_AF();
word_t cpu_HL();
word_t cpu_BC();
word_t cpu_DE();

void cpu_set_BC(word_t value);
void cpu_set_DE(word_t value);
void cpu_set_HL(word_t value);

byte_t cpu_get_reg_by_code(opcode_t code);
void   cpu_set_reg_by_code(opcode_t code, byte_t value);

void cpu_update_flags(byte_t a, byte_t b, word_t result, char* flags_operations);

void cpu_push_pc();

cycle_t cpu_exec_CB_opcode(opcode_t opcode);

word_t cpu_stack_pop();

void log_cpu_full(opcode_t opcode);

void log_cpu_full_16(opcode_t opcode);

#endif
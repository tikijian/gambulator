#include "../types.h"

#ifndef _CPU_H
# define _CPU_H

// main CPU struct
struct CPU {
    byte_t A, B, C, D, E, H, L, F;
    // Stack Pointer
    word_t SP;
    // Prgram Counter
    word_t PC;

    byte_t FZ;
    byte_t FN;
    byte_t FH;
    byte_t FC;

    byte_t IME;
};

// I know that globals are bad :D
extern struct CPU cpu;

void cpu_exec(opcode_t, void* memory);

#endif
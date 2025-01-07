#include "../types.h"

// main CPU struct
struct CPU {
    byte_t A, B, C, D, E, H, L, F;
    // Stack Pointer
    word_t SP;
    // Prgram Counter
    word_t PC;
};

// I know that globals are bad :D
extern struct CPU cpu;

void cpu_exec(opcode_t, void* memory);

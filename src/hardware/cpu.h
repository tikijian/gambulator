#include "../types.h"

// main CPU struct
struct CPU {
    // word_t AF;
    // Accumulator
    byte_t A, B, C, D, E, H, L;
    // Flag register
    struct {
        // zero flag
        byte_t Z;
        // Add/Sub flag (BCD)
        byte_t N;
        // Half Carry flag (BCD)
        byte_t H;
        // Carry flag
        byte_t C;
    } flags;
    // Stack Pointer
    word_t SP;
    // Prgram Counter
    word_t PC;

    // possible joined implementation
    // word_t BC;
    // word_t DE;
    // word_t HL;
};

struct CPU cpu_init();
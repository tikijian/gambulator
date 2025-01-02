#include "cpu.h"

struct CPU cpu_init() {
    struct CPU cpu = {
        .A = 0x01,
        .B = 0x0,
        .C = 0x13,
        .D = 0x0,
        .E = 0xd8,
        .H = 0x01,
        .L = 0x4d,
        
        .SP = 0xfffe,
        .PC = 0x0100,

        .flags = {
            .C = 1,
            .H = 1,
            .N = 0,
            .Z = 1,
        }
    };

    return cpu;
}

#include "../constants.h"
#include "clock.h"

struct Clock clock = {
    .cycles = 0,
    .m_cycles = 0,
};

static unsigned int diff = 0;

void clock_tick(int passed_cycles) {
    diff += passed_cycles;

    // TODO: double check
    // if (diff >= CPU_CYCLES_PER_M_CYCLE) {
    //     diff = CPU_CYCLES_PER_M_CYCLE - diff;     
        
    //     clock.m_cycles++;
    // }

    clock.cycles += passed_cycles;
}
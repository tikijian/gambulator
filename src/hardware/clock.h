#include "../types.h"

#ifndef _CLOCK_H
# define _CLOCK_H

struct Clock {
    cycle_t cycles;
    cycle_t m_cycles;
};

extern struct Clock clock;
#endif
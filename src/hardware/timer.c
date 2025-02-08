#include <stdio.h>
#include "memory.h"
#include "timer.h"


void timer_update_TIMA()
{
    
    printf("mem addr is (at timer.c) %p\n", memory);
    byte_t tima = mem_read(REG_TIMA);
    // When the value overflows (exceeds $FF) it is reset to the value specified in TMA (FF06)
    // and an interrupt is requested, as described below.
    if (tima == 0xFF) {
        tima = mem_read(REG_TMA);
        // TODO: request interrupt
    } else {
        mem_write(REG_TIMA, tima + 1);
    }
}
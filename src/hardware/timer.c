#include <stdio.h>
#include "memory.h"
#include "timer.h"


static void timer_update_TIMA()
{
    byte_t tima = mem_read(REG_TIMA);
    // When the value overflows (exceeds $FF) it is reset to the value specified in TMA (FF06)
    // and an interrupt is requested, as described below.
    if (tima == 0xFF) {
        mem_write(REG_TIMA, mem_read(REG_TMA));
        // TODO: request interrupt
    } else {
        mem_write(REG_TIMA, tima + 1);
    }
}

static void timer_inc_DIV()
{
    mem_write(REG_DIV, mem_read(REG_DIV) + 1);
}

void timer_update() {
    byte_t tac = mem_read(REG_TAC);
    if (CHECK_BIT(tac, REG_TAC_ENABLE_BIT)) {
        // TODO: check M-cycles https://gbdev.io/pandocs/Timer_and_Divider_Registers.html#ff07--tac-timer-control
        timer_update_TIMA();
    }

    timer_inc_DIV();
}

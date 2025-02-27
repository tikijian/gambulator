#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "timer.h"
#include "clock.h"
#include "cpu.h"

#define UPDATE_DIV_AFTER_CYCLES 64 // TODO: check on 256


static void timer_update_TIMA(cycle_t passed_cycles)
{
    static int tima_counter = 0;

    byte_t tac  = mem_get_value((REG_TAC));
    
    if (!CHECK_BIT(tac, REG_TAC_ENABLE_BIT)) {
        return;
    }

    int rate = 0;
    switch(tac & REG_TAC_CLOCK_BIT_MASK) {
        case 0:
            rate = 256; break;
        case 0x01:
            rate = 4;   break;
        case 0x02:
            rate = 16;  break;
        case 0x03:
            rate = 64;  break;
        default:
            printf("timer_update_TIMA: unknown TAC value 0x%02x\n", tac);
            exit(-1);
    }

    tima_counter += passed_cycles;

    byte_t tima = mem_get_value(REG_TIMA);
    if (tima_counter > rate) {
        tima_counter -= rate;
        tima++;
        if (tima == 0) {
            // TODO: request Interrupt
            mem_set_value(REG_TIMA, mem_get_value(REG_TMA));
        } else {
            mem_set_value(REG_TIMA, tima);
        }
    }
}

static void timer_inc_DIV(cycle_t passed_cycles)
{
    static int div_counter = 0;
    div_counter += passed_cycles;

    if (div_counter > UPDATE_DIV_AFTER_CYCLES) {
        byte_t val = mem_get_value(REG_DIV);
        val = val == 0xFF ? 0 : val + 1;
        
        mem_set_value(REG_DIV, val);
        div_counter -= UPDATE_DIV_AFTER_CYCLES;
    }
}

void timer_update(cycle_t passed_cycles) {
    timer_update_TIMA(passed_cycles);

    if (cpu.stopped) {
        return;
    }

    timer_inc_DIV(passed_cycles);
}

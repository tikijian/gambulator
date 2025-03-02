#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "types.h"
#include "hardware/cpu.h"
#include "hardware/cpu_helpers.h"
#include "hardware/memory.h"
#include "hardware/timer.h"
#include "hardware/clock.h"
#include "hardware/interrupts.h"
#include "software/utils.h"
#include "routines.h"

int main()
{
    byte_t* memory_ptr = mem_init();
    
    read_rom("roms/individual/07-jr,jp,call,ret,rst.gb", memory_ptr);

    // for (int i = 0; i < 50; i++) {
    //     cpu_exec(mem_read(cpu.PC), memory);
    // }

    // printf("0x%02X 0x%02X\n", memory[0xff0f], memory[0xff07]);

    do {
        cycle_t initial_cycles = clock.cycles;

        ir_handle();
         
        clock.cycles += cpu_exec();

        cycle_t passed_cycles = clock.cycles - initial_cycles;
        
        timer_update(passed_cycles);
    } while (1);

    mem_free();
    return 0;
}

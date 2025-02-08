#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "types.h"
#include "hardware/cpu.h"
#include "hardware/cpu_helpers.h"
#include "hardware/memory.h"
#include "hardware/timer.h"
#include "software/utils.h"
#include "routines.h"

int main(int argc, char const *argv[])
{
    printf("mem addr is (at start of main.c) %p\n", memory);
    byte_t* memory_ptr = mem_init();
    printf("mem addr is (at main.c after alloc) %p, %p\n", memory_ptr, memory);
    
    read_rom("roms/individual/07-jr,jp,call,ret,rst.gb", memory_ptr);

    // for (int i = 0; i < 50; i++) {
    //     cpu_exec(mem_read(cpu.PC), memory);
    // }

    // printf("0x%02X 0x%02X\n", memory[0xff0f], memory[0xff07]);

    // do {
    //     // TODO: verify, if it should be before or after cpu_exec()
    //     reg_increment_TIMA();

    //     cpu_exec(mem_read(cpu.PC), memory);
    // } while (1);

    timer_update_TIMA();
    printf("mem addr is (at of main.c) %p\n", memory);
    mem_free();
    return 0;
}

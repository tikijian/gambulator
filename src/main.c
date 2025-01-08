#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "types.h"
#include "hardware/cpu.h"
#include "hardware/cpu_helpers.h"
#include "hardware/memory.h"
#include "software/utils.h"
#include "routines.h"

int main(int argc, char const *argv[])
{
    byte_t* memory = mem_init();
    
    read_rom("roms/tests/05-op rp.gb", memory);

    // for (int i = 0; i < 20; i++) {
    //     cpu_exec(mem_read(cpu.PC), memory);
    // }

    do {
        cpu_exec(mem_read(cpu.PC), memory);
    } while (1);

    mem_free();
    return 0;
}

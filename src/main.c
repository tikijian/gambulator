#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "types.h"
#include "hardware/cpu.h"
#include "hardware/memory.h"
#include "software/utils.h"
#include "routines.h"

int main(int argc, char const *argv[])
{
    byte_t* memory = mem_init();
    
    read_rom("roms/Tetris.gb", memory);

    for (int i = 0; i < 20; i++) {
        cpu_exec(mem_read(cpu.PC), memory);
    }
    
    mem_free();
    return 0;
}

#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "types.h"
#include "hardware/cpu.h"
#include "software/utils.h"

int main(int argc, char const *argv[])
{
    struct CPU cpu = cpu_init();
    byte_t* memory = calloc(0xFFFF, sizeof(byte_t));
    read_rom("roms/Tetris.gb", memory);

    for (int i = 0; i < 40; i++) {

        printf("%04X\n", memory[i + ADDR_ROM_START]);
    }
    
    free(memory);
    return 0;
}

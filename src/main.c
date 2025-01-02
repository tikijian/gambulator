#include <stdio.h>

#include "constants.h"
#include "types.h"
#include "hardware/cpu.h"

int main(int argc, char const *argv[])
{
    struct CPU cpu = cpu_init();
    printf("%x\n", cpu.A);
    return 0;
}

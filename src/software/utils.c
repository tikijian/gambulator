#include <stdio.h>
#include <stdlib.h>

#include "../constants.h"
#include "utils.h"

void read_rom(const char* rom_name, void* memory) {
    FILE* fp = fopen(rom_name, "rb");
    if (!fp) {
        perror("File opening failed");
        exit(-1);
    }

    fseek(fp, 0L, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    printf("Reading rom: %s, size %ld bytes...\n", rom_name, file_size);

    if (fread(memory + ADDR_ROM_START, sizeof(char), file_size, fp) != file_size) {
        perror("Error reading file!\n");

        fclose(fp);
        exit(-1);
    }

    fclose(fp);
}
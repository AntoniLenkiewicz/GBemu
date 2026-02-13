#include "memory.h"

/* 0 - 3FFF - Cartridge
 * 4000 - 7FFF - Cartridge swappable
 * 8000 - 9FFF - VRAM
 * A000 - BFFF - external ram
 * C000 - CFFF - Work Ram
 * D000 - DFFF - Work Ram
 * E000 - FDFF - mirror of C000–DDFF - Don't use
 * FE00 - FE9F - Object Attribute memory
 * FEA0 - FEFF - Prohibited
 * FF00 - FF7F - IO mapped
 * FF80 - FFFE -HRAM
 * FFFF - Interuppt */

static uint8_t memory[MEM_SIZE];

void write_mem(uint16_t address, uint8_t byte) {
    memory[address] = byte;
}

uint8_t read_mem(uint16_t address) {
    if (address == 0xff44) {
        return 0x94;
    }
    uint8_t byte;
    byte = memory[address];
    return byte;
}
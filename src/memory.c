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

uint8_t *returnMemoryPtr() {
    return memory;
}

void write_mem(uint16_t address, uint8_t byte) {
    if (address >= 0x0000 && address <= 0x7FFF) {
        // Switching commands, ignore writes to the rom
        return;
    }
    //Write to mirror and then work ram
    if (address >= 0xE000 && address <= 0xFDFF) {
        memory[address] = byte;
        memory[address - 0x2000] = byte;
        return;
    }

    //write to work ram and then to mirror
    if (address >= 0xC000 && address <= 0xDFFF) {
        memory[address] = byte;
        memory[address + 0x2000] = byte;
        return;
    }
    memory[address] = byte;
}

void write_rom(uint16_t address, uint8_t byte) {
    memory[address] = byte;
}

uint8_t read_mem(uint16_t address) {
    if (address == 0xff44) {
        return 0x90;
    }
    uint8_t byte;
    byte = memory[address];
    return byte;
}
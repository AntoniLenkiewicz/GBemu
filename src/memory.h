#ifndef GBEMU_MEMORY_H
#define GBEMU_MEMORY_H

#include <stdint.h>


#define MEM_SIZE 0x10000

void write_mem(uint16_t address, uint8_t byte);
void write_rom(uint16_t address, uint8_t byte);
uint8_t read_mem(uint16_t address);
uint8_t *returnMemoryPtr();


#endif //GBEMU_MEMORY_H
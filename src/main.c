#include "cart.h"
#include "cpu.h"
#include "memory.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("No file given or too many files provided\n");
        return 1;
    }
    FILE *fp;
    if (fp = openCart(argv[1])) {
        uint8_t *cp;
        if (cp = readCart(fp)) {
            //Load cartridge into ram, for now both bank 0 and switchable bank 1
            //Add switching logic later
            for (uint16_t i = 0; i < 0x8000; i ++) {
                write_rom(i, cp[i]);
            }
            FILE *fptr;
            fptr = fopen("log.txt", "w");
            fprintf(fptr, "");
            fclose(fptr);
            uint8_t *memPtr = returnMemoryPtr();
            while (parse_instruction(memPtr))
                ;
        }
    }
    return 0;
}
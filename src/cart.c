#include "cart.h"

FILE *openCart(char *cart) {
    FILE *fp;
    if (fp = fopen(cart, "rb")) {
        return fp;
    }
    return fp;
}

uint8_t *readCart(FILE *fp) {
    size_t size;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *cart = malloc(size);

    fread(cart, 1, size, fp);
    fclose(fp);

    for (int i = 0; i < 4; i++) {
        printf("%.2x ", *(cart+0x0100 + i));
    }
    printf("\n00 c3 50 01 - Instruction check \n");

    return cart;
}
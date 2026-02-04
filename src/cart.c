#include "cart.h"

FILE *openCart(char *cart) {
    FILE *fp;
    if (fp = fopen(cart, "rb")) {
        return fp;
    }
    return fp;
}

int readCart(FILE *fp) {
    size_t size;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *cart = malloc(size);

    fread(cart, 1, size, fp);
    fclose(fp);

    printf("%u\n", *(cart + 0x147));

    return 0;
}


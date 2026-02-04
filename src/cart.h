#ifndef GBEMU_CART_H
#define GBEMU_CART_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

FILE *openCart(char *cart);
int readCart(FILE *fp);

#endif //GBEMU_CART_H
#include "cart.h"
#include "cpu.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("No file given or too many files provided\n");
        return 1;
    } else {
        FILE *fp;
        if (fp = openCart(argv[1])) {
            readCart(fp);
        }
    }
    return 0;
}
#include "prelexer.h"

#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc == 3) {
        FILE *in = fopen(argv[1], "r");

        char const * const temp_out = "prelexer_temp_out";
        FILE *prelex_out = fopen(temp_out, "w+");

        prelex(fopen("test.txt", "r"), prelex_out);
        fclose(in);
        remove(temp_out);
    } else {
        fprintf(stderr, "WRONG AMOUNT OF INPUT ARGUMENTS!\n");
        exit(EXIT_FAILURE);
    }
}

#include "prelexer.h"

#include <stdlib.h>

extern void parse(FILE *in, FILE *out);

int main(int argc, char *argv[]) {
    if (argc == 3) {
        FILE *in = fopen(argv[1], "r");
        if (in == NULL) {
            fprintf(stderr, "Couldn't open file: %s!\n", argv[1]);
            exit(EXIT_FAILURE);
        }

        char const * const temp_out = "prelexer_temp_out";
        FILE *prelex_out = fopen(temp_out, "w+");
        if (prelex_out == NULL) {
            fprintf(stderr, "Couldn't create temporary files for compiler: %s!\n", temp_out);
            exit(EXIT_FAILURE);
        }

        FILE *compiler_out = fopen(argv[2], "w+");
        if (compiler_out == NULL) {
            fprintf(stderr, "Couldn't create OUTPUT FILE of compiler: %s!\n", argv[2]);
            exit(EXIT_FAILURE);
        }

        prelex(in, prelex_out);
        fclose(in);

        parse(prelex_out, compiler_out);
        fclose(prelex_out);
        remove(temp_out);
        fclose(compiler_out);
    } else {
        fprintf(stderr, "WRONG AMOUNT OF INPUT ARGUMENTS!\n");
        exit(EXIT_FAILURE);
    }
}

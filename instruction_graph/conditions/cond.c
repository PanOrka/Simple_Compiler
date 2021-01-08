#include "cond.h"

#include <stdio.h>

bool cond_val_from_const(mpz_t src_1, mpz_t src_2, expr_type type) {
    int res = mpz_cmp(src_1, src_2);
    switch (type) {
        case cond_IS_EQUAL:
            return res == 0;
            break;
        case cond_IS_N_EQUAL:
            return res != 0;
            break;
        case cond_LESS:
            return res < 0;
            break;
        case cond_GREATER:
            return res > 0;
            break;
        case cond_LESS_EQ:
            return res <= 0;
            break;
        case cond_GREATER_EQ:
            return res >= 0;
            break;
        default:
            fprintf(stderr, "[COND]: Wrong value of cond_type!\n");
            exit(EXIT_FAILURE);
    }
}
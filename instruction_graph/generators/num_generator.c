#include "num_generator.h"

void generate_value(reg *r, addr_t cur_val, addr_t val, FILE *file, bool reset) {
    if (reset) {
        fprintf(file, "RESET %c\n", r->id);
    }

    bool inc_once = false;
    for (int32_t i=0; i<8*sizeof(addr_t); ++i) {
        if (inc_once) {
            fprintf(file, "SHL %c\n", r->id);
        }

        if (val & ADDR_T_MSB) {
            fprintf(file, "INC %c\n", r->id);
            inc_once = true;
        }
        val <<= 1;
    }
}

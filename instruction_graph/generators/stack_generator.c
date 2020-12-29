#include "stack_generator.h"

static addr_t stack_value;

static void generate_value(reg *stack_ptr, addr_t addr, FILE *file) {
    fprintf(file, "RESET %c\n", stack_ptr->id);
    bool inc_once = false;
    for (int32_t i=0; i<8*sizeof(addr_t); ++i) {
        if (inc_once) {
            fprintf(file, "SHL %c\n", stack_ptr->id);
        }

        if (addr & ADDR_T_MSB) {
            fprintf(file, "INC %c\n", stack_ptr->id);
            inc_once = true;
        }
        addr <<= 1;
    }
}

void stack_ptr_generate(addr_t addr, FILE *file) {
    reg_set *r_set = get_reg_set();
    generate_value(&(r_set->stack_ptr), addr, file);
}

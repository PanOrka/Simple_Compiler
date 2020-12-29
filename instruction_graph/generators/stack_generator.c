#include "stack_generator.h"

static addr_t stack_value;

static void generate_value(reg *stack_ptr, addr_t addr, FILE *file) {
    fprintf(file, "RESET %c\n", stack_ptr->id);
    bool inc_once = false;
    for (int32_t i=0; i<8*sizeof(addr_t); ++i) {
        if (addr & ADDR_T_MSB) {
            fprintf(file, "INC %c\n", stack_ptr->id);
            inc_once = true;
        }

        if (inc_once) {
            fprintf(file, "SHL %c\n", stack_ptr->id);
        }
    }
}

reg * stack_ptr_generate(addr_t addr, FILE *file) {
    reg_set *r_set = get_reg_set();
    reg * const stack_ptr = r_set->stack_ptr;
    generate_value(stack_ptr, addr, file);
}

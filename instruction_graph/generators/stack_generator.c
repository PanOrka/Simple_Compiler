#include "stack_generator.h"
#include "num_generator.h"

static addr_t stack_value;
static bool stack_initialized = false;

void stack_ptr_generate(addr_t addr, FILE *file) {
    reg_set *r_set = get_reg_set();
    generate_value(&(r_set->stack_ptr), stack_value, addr, file, !stack_initialized);
    stack_initialized = true;
}

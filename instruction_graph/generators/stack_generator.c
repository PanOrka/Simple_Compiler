#include "stack_generator.h"
#include "num_generator.h"

static num_t stack_value = 0;
static bool stack_initialized = false;

void stack_ptr_generate(addr_t addr, FILE *file) {
    reg_set *r_set = get_reg_set();
    generate_value(&(r_set->stack_ptr), stack_value, addr, file, !stack_initialized);
    stack_value = addr;
    stack_initialized = true;
}

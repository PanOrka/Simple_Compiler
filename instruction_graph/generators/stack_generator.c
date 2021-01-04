#include "stack_generator.h"
#include "num_generator.h"

static num_t stack_value = 0;
static bool stack_initialized = false;

void stack_ptr_generate(addr_t addr) {
    reg_set *r_set = get_reg_set();
    generate_value(&(r_set->stack_ptr), stack_value, addr, !stack_initialized);
    stack_value = addr;
    stack_initialized = true;
}

void stack_ptr_clear() {
    stack_initialized = false;
}

bool stack_ptr_initialized() {
    return stack_initialized;
}

bool stack_ptr_is_null() {
    return (stack_value == 0);
}

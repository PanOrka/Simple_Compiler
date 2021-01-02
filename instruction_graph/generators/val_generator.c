#include "val_generator.h"
#include "stack_generator.h"
#include "num_generator.h"

static num_t current_val = 0;

reg * val_generate(num_t target_value, FILE *file) {
    reg_set *r_set = get_reg_set();
    reg_allocator r_alloc = oper_load_variable(VAL_GEN_ADDR, file);

    bool reset = false;
    if (!r_alloc.was_allocated) {
        current_val = 0;
        reset = true;
    }
    generate_value(r_alloc.r, current_val, target_value, file, reset);
    current_val = target_value;

    return r_alloc.r;
}
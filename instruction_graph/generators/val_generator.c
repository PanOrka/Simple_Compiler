#include "val_generator.h"
#include "stack_generator.h"
#include "num_generator.h"

static num_t current_val = 0;

reg * val_generate(num_t target_value, FILE *file) {
    reg_set *r_set = get_reg_set();
    reg_allocator r_alloc = reg_m_get(r_set, VAL_GEN_ADDR, true);
    bool val_reset = false;
    if (!r_alloc.was_allocated) {
        if (r_alloc.r->flags & REG_MODIFIED) {
            stack_ptr_generate(r_alloc.r->addr, file);
            fprintf(file, "STORE %c %c\n", r_alloc.r->id, r_set->stack_ptr.id);
        }
        current_val = 0;
        val_reset = true;

        r_alloc.r->addr = VAL_GEN_ADDR;
        r_alloc.r->flags = REG_NO_FLAGS;
    }
    generate_value(r_alloc.r, current_val, target_value, file, val_reset);

    return r_alloc.r;
}
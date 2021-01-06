#include "val_generator.h"
#include "stack_generator.h"
#include "num_generator.h"

static mpz_t current_val;
static bool mpz_initialized = false;

reg * val_generate(mpz_t target_value) {
    if (!mpz_initialized) {
        mpz_init(current_val);
        mpz_initialized = true;
    }

    reg_set *r_set = get_reg_set();
    reg_allocator r_alloc = oper_get_reg_for_variable(VAL_GEN_ADDR);

    bool reset = false;
    if (!r_alloc.was_allocated) {
        mpz_set_si(current_val, 0);
        reset = true;
    }

    generate_value(r_alloc.r, current_val, target_value, reset);
    mpz_set(current_val, target_value);
    r_alloc.r->addr = VAL_GEN_ADDR;

    return r_alloc.r;
}
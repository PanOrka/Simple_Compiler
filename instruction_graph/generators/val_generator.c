#include "val_generator.h"
#include "stack_generator.h"
#include "num_generator.h"

static mpz_t current_val;
static bool mpz_initialized = false;

reg * val_generate(uint64_t target_value) {
    mpz_t temp_target_value;
    mpz_init_set_ui(temp_target_value, target_value);

    reg * ret_reg = val_generate_from_mpz(temp_target_value);
    mpz_clear(temp_target_value);

    return ret_reg;
}

reg * val_generate_from_mpz(mpz_t target_value) {
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
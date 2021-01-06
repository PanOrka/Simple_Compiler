#include "stack_generator.h"
#include "num_generator.h"

static mpz_t stack_value;
static bool stack_initialized = false;
static bool mpz_initialized = false;

void stack_ptr_generate(addr_t addr) {
    if (!mpz_initialized) {
        mpz_init(stack_value);
        mpz_initialized = true;
    }

    reg_set *r_set = get_reg_set();

    mpz_t addr_mpz;
    mpz_init_set_ui(addr_mpz, addr);
    generate_value(&(r_set->stack_ptr), stack_value, addr_mpz, !stack_initialized);

    mpz_set(stack_value, addr_mpz);
    mpz_clear(addr_mpz);
    stack_initialized = true;
}

void stack_ptr_clear() {
    stack_initialized = false;
}

bool stack_ptr_initialized() {
    return stack_initialized;
}

bool stack_ptr_is_null() {
    return mpz_cmp_si(stack_value, 0) == 0;
}

#include "stack_generator.h"
#include "num_generator.h"

static mpz_t stack_value;
static bool stack_initialized = false;
static bool mpz_initialized = false;

void stack_ptr_generate(addr_t addr) {
    mpz_t addr_mpz;
    mpz_init_set_ui(addr_mpz, addr);

    stack_ptr_generate_from_mpz(addr_mpz);
    mpz_clear(addr_mpz);
}

void stack_ptr_generate_from_mpz(mpz_t addr) {
    if (!mpz_initialized) {
        mpz_init(stack_value);
        mpz_initialized = true;
    }
    reg_set *r_set = get_reg_set();

    generate_value(&(r_set->stack_ptr), stack_value, addr, !stack_initialized);
    mpz_set(stack_value, addr);
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

void stack_ptr_set_mpz_to_current_value(mpz_t dest) {
    if (mpz_initialized) {
        mpz_set(dest, stack_value);
    } else {
        fprintf(stderr, "[STACK_PTR]: Stack pointer is not initialized!\n");
        exit(EXIT_FAILURE);
    }
}

void stack_ptr_set_mpz(mpz_t src) {
    if (mpz_initialized) {
        mpz_set(stack_value, src);
    } else {
        fprintf(stderr, "[STACK_PTR]: Stack pointer is not initialized!\n");
        exit(EXIT_FAILURE);
    }
}

bool stack_ptr_mpz_initialized() {
    return mpz_initialized;
}

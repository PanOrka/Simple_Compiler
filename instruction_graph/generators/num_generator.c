#include "num_generator.h"
#include "../instructions/asm_fprintf.h"

#ifndef ABS
#define ABS(x) (x >= 0 ? x : -x)
#endif

static uint64_t generate_from_reset_cost(mpz_t target_val) {
    uint64_t cost = 0;

    bool inc_once = false;
    size_t size = mpz_sizeinbase(target_val, 2);
    for (size_t i=size; i>0; --i) {
        if (inc_once) {
            ++cost;
        }

        if (mpz_tstbit(target_val, i-1)) {
            ++cost;
            inc_once = true;
        }
    }

    return cost + 1;
}

static void generate_from_reset(reg *r, mpz_t target_val) {
    mpz_t test;
    mpz_init(test);

    bool inc_once = false;
    size_t size = mpz_sizeinbase(target_val, 2);
    for (size_t i=size; i>0; --i) {
        if (inc_once) {
            SHL(r);
            mpz_mul_si(test, test, 2);
        }

        if (mpz_tstbit(target_val, i-1)) {
            INC(r);
            inc_once = true;
            mpz_add_ui(test, test, 1);
        }
    }

    if (mpz_cmp(test, target_val) != 0) {
        fprintf(stderr, "[NUMBER_GENERATOR]: Generated number is wrong! (RESET_GEN)\n");
        exit(EXIT_FAILURE);
    }

    mpz_clear(test);
}

static uint64_t generate_from_current_div_cost(mpz_t curr_val, mpz_t target_val) {
    uint64_t cost = 0;

    mpz_t diff;
    mpz_init(diff);
    mpz_sub(diff, target_val, curr_val);

    mpz_t alternative;
    mpz_init(alternative);

    while (mpz_cmp(curr_val, diff) < 0) {
        mpz_mul_si(curr_val, curr_val, 2);
        mpz_sub(diff, target_val, curr_val);
        ++cost;
    }

    while (mpz_cmpabs_ui(diff, 2) > 0) {
        mpz_sub(alternative, diff, curr_val);
        if (mpz_cmpabs(alternative, diff) < 0) {
            mpz_mul_si(curr_val, curr_val, 2);
            mpz_set(diff, alternative);
            ++cost;
        } else {
            mpz_tdiv_r_ui(alternative, diff, 2);
            int32_t reminder = mpz_get_si(alternative);
            mpz_tdiv_r_ui(alternative, curr_val, 2);
            reminder += mpz_get_si(alternative);

            mpz_tdiv_q_ui(diff, diff, 2);
            mpz_tdiv_q_ui(curr_val, curr_val, 2);

            cost += 2;
            if (reminder != 0) {
                ++cost;
            }
        }
    }

    int64_t diff_val = mpz_get_si(diff);

    mpz_clear(diff);
    mpz_clear(alternative);

    return cost + (uint64_t)ABS(diff_val);
}

#include "../../vector/vector.h"

static void generate_from_current_div(reg *r, mpz_t curr_val, mpz_t target_val) {
    mpz_t diff;
    mpz_init(diff);
    mpz_sub(diff, target_val, curr_val);

    mpz_t alternative;
    mpz_init(alternative);
    /*
    // if diff >= 0 it can't be greater than MAX_uint64_t
    */
    while (mpz_cmp(curr_val, diff) < 0) {
        mpz_mul_si(curr_val, curr_val, 2);
        mpz_sub(diff, target_val, curr_val);
        SHL(r);
    }

    vector v = vector_create(sizeof(int32_t), alignof(int32_t), 64);

    while (mpz_cmpabs_ui(diff, 2) > 0) {
        mpz_sub(alternative, diff, curr_val);
        if (mpz_cmpabs(alternative, diff) < 0) {
            mpz_mul_si(curr_val, curr_val, 2);
            mpz_set(diff, alternative);
            SHL(r);
        } else {
            mpz_tdiv_r_ui(alternative, diff, 2);
            int32_t reminder = mpz_get_si(alternative);
            mpz_tdiv_r_ui(alternative, curr_val, 2);
            reminder += mpz_get_si(alternative);

            mpz_tdiv_q_ui(diff, diff, 2);
            mpz_tdiv_q_ui(curr_val, curr_val, 2);
            SHR(r);
            VECTOR_ADD(v, reminder);
        }
    }

    mpz_t test;
    mpz_init_set(test, curr_val);

    uint32_t temp_diff = mpz_get_si(diff);
    switch (temp_diff) {
        case 2:
            INC(r);
            INC(r);
            mpz_add_ui(test, test, 2);
            break;
        case 1:
            INC(r);
            mpz_add_ui(test, test, 1);
            break;
        case 0:
            break;
        case -1:
            DEC(r);
            mpz_sub_ui(test, test, 1);
            break;
        case -2:
            DEC(r);
            DEC(r);
            mpz_sub_ui(test, test, 2);
            break;
        default:
            fprintf(stderr, "[NUMBER_GENERATOR]: Got wrong reminder on diff!\n");
            exit(EXIT_FAILURE);
    }

    for (int32_t i=v.used_size; i>0; --i) {
        int32_t reminders = *((int32_t *)VECTOR_POP(v, POP));
        switch (reminders) {
            case 2:
                INC(r);
                SHL(r);
                mpz_add_ui(test, test, 1);
                mpz_mul_si(test, test, 2);
                break;
            case 1:
                SHL(r);
                INC(r);
                mpz_mul_si(test, test, 2);
                mpz_add_ui(test, test, 1);
                break;
            case 0:
                SHL(r);
                mpz_mul_si(test, test, 2);
                break;
            case -1:
                SHL(r);
                DEC(r);
                mpz_mul_si(test, test, 2);
                mpz_sub_ui(test, test, 1);
                break;
            default:
                fprintf(stderr, "[NUMBER_GENERATOR]: Got wrong reminder: %d!\n", reminders);
                exit(EXIT_FAILURE);
        }
    }
    free(v._mem_ptr);

    if (mpz_cmp(test, target_val) != 0) {
        fprintf(stderr, "[NUMBER_GENERATOR]: Generated number is wrong! (DIV_GEN)\n");
        exit(EXIT_FAILURE);
    }

    mpz_clear(diff);
    mpz_clear(alternative);
    mpz_clear(test);
}

static uint64_t generate_from_current_inc_cost(mpz_t curr_val, mpz_t target_val) {
    mpz_t diff;
    mpz_init(diff);
    mpz_sub(diff, target_val, curr_val);

    int64_t diff_val = mpz_get_si(diff);
    mpz_clear(diff);

    return ABS(diff_val);
}

static void generate_from_current_inc(reg *r, mpz_t curr_val, mpz_t target_val) {
    mpz_t diff;
    mpz_init(diff);
    mpz_sub(diff, target_val, curr_val);

    mpz_t test;
    mpz_init_set(test, curr_val);

    while (mpz_cmp_si(diff, 0) > 0) {
        INC(r);
        mpz_sub_ui(diff, diff, 1);
        mpz_add_ui(test, test, 1);
    }

    while (mpz_cmp_si(diff, 0) < 0) {
        DEC(r);
        mpz_add_ui(diff, diff, 1);
        mpz_sub_ui(test, test, 1);
    }

    if (mpz_cmp(test, target_val) != 0) {
        fprintf(stderr, "[NUMBER_GENERATOR]: Generated number is wrong! (INC_GEN)\n");
        exit(EXIT_FAILURE);
    }

    mpz_clear(diff);
    mpz_clear(test);
}

void generate_value(reg *r, mpz_t curr_val, mpz_t target_val, bool reset) {
    if (curr_val != 0 && !reset) {
        uint64_t reset_cost = generate_from_reset_cost(target_val);

        mpz_t temp_curr_val;
        mpz_init_set(temp_curr_val, curr_val);
        uint64_t div_cost = generate_from_current_div_cost(temp_curr_val, target_val);

        uint64_t inc_cost = generate_from_current_inc_cost(curr_val, target_val);

        if (reset_cost <= div_cost && reset_cost < inc_cost) {
            RESET(r);
            generate_from_reset(r, target_val);
        } else if (div_cost < inc_cost) {
            mpz_set(temp_curr_val, curr_val);
            generate_from_current_div(r, temp_curr_val, target_val);
        } else {
            generate_from_current_inc(r, curr_val, target_val);
        }

        mpz_clear(temp_curr_val);
    } else {
        if (reset) {
            RESET(r);
        }

        generate_from_reset(r, target_val);
    }
}

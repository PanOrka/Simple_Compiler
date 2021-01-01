#include "num_generator.h"

#ifndef ABS
#define ABS(x) (x >= 0 ? x : -x)
#endif

/**
 * 
 * NUM_T MSB
 * 
*/
const num_t num_t_msb = ((num_t)1) << 127;

static uint64_t generate_from_reset_cost(num_t target_val) {
    uint64_t cost = 0;

    bool inc_once = false;
    for (int32_t i=0; i<8*sizeof(target_val); ++i) {
        if (inc_once) {
            ++cost;
        }

        if (target_val & num_t_msb) {
            ++cost;
            inc_once = true;
        }
        target_val <<= 1;
    }

    return cost + 1;
}

static void generate_from_reset(reg *r, num_t target_val, FILE *file) {
    bool inc_once = false;

    num_t val = target_val;
    num_t test = 0;
    for (int32_t i=0; i<8*sizeof(val); ++i) {
        if (inc_once) {
            fprintf(file, "SHL %c\n", r->id);
            test *= 2;
        }

        if (val & num_t_msb) {
            fprintf(file, "INC %c\n", r->id);
            inc_once = true;
            ++test;
        }
        val <<= 1;
    }

    if (test != target_val) {
        fprintf(stderr, "[NUMBER_GENERATOR]: Generated number is wrong! (RESET_GEN)\n");
        exit(EXIT_FAILURE);
    }
}

static uint64_t generate_from_current_div_cost(num_t curr_val, num_t target_val) {
    uint64_t cost = 0;

    num_t diff = target_val - curr_val;
    /*
    // if diff >= 0 it can't be greater than MAX_uint64_t
    */
    while (curr_val < diff) {
        curr_val *= 2;
        diff = target_val - curr_val;
        ++cost;
    }

    while (diff > 2 || diff < -2) {
        const num_t alternative = diff - curr_val;
        if (ABS(alternative) < ABS(diff)) {
            curr_val *= 2;
            diff = alternative;
            ++cost;
        } else {
            int32_t reminder = (curr_val % 2) + (diff % 2);
            diff /= 2;
            curr_val /= 2;

            cost += 2;
            if (reminder != 0) {
                ++cost;
            }
        }
    }

    return cost + (uint64_t)ABS(diff);
}

#include "../../vector/vector.h"

static void generate_from_current_div(reg *r, num_t curr_val, num_t target_val, FILE *file) {
    num_t diff = target_val - curr_val;
    /*
    // if diff >= 0 it can't be greater than MAX_uint64_t
    */
    while (curr_val < diff) {
        curr_val *= 2;
        diff = target_val - curr_val;
        fprintf(file, "SHL %c\n", r->id);
    }

    vector v = vector_create(sizeof(int32_t), alignof(int32_t), 64);

    while (diff > 2 || diff < -2) {
        const num_t alternative = diff - curr_val;
        if (ABS(alternative) < ABS(diff)) {
            curr_val *= 2;
            diff = alternative;
            fprintf(file, "SHL %c\n", r->id);
        } else {
            int32_t reminder = (curr_val % 2) + (diff % 2);
            diff /= 2;
            curr_val /= 2;
            fprintf(file, "SHR %c\n", r->id);
            VECTOR_ADD(v, reminder);
        }
    }

    num_t test = curr_val;
    switch (diff) {
        case 2:
            fprintf(file, "INC %c\n", r->id);
            fprintf(file, "INC %c\n", r->id);
            test += 2;
            break;
        case 1:
            fprintf(file, "INC %c\n", r->id);
            ++test;
            break;
        case 0:
            test += 0;
            break;
        case -1:
            fprintf(file, "DEC %c\n", r->id);
            --test;
            break;
        case -2:
            fprintf(file, "DEC %c\n", r->id);
            fprintf(file, "DEC %c\n", r->id);
            test -= 2;
            break;
        default:
            fprintf(stderr, "[NUMBER_GENERATOR]: Got wrong reminder on diff!\n");
            exit(EXIT_FAILURE);
    }

    for (int32_t i=v.used_size; i>0; --i) {
        int32_t reminders = *((int32_t *)VECTOR_POP(v, POP));
        switch (reminders) {
            case 2:
                fprintf(file, "INC %c\n", r->id);
                fprintf(file, "SHL %c\n", r->id);
                test = 2*(++test);
                break;
            case 1:
                fprintf(file, "SHL %c\n", r->id);
                fprintf(file, "INC %c\n", r->id);
                test *= 2;
                ++test;
                break;
            case 0:
                fprintf(file, "SHL %c\n", r->id);
                test *= 2;
                break;
            case -1:
                fprintf(file, "SHL %c\n", r->id);
                fprintf(file, "DEC %c\n", r->id);
                test *= 2;
                --test;
                break;
            default:
                fprintf(stderr, "[NUMBER_GENERATOR]: Got wrong reminder: %d!\n", reminders);
                exit(EXIT_FAILURE);
        }
    }
    free(v._mem_ptr);

    if (test != target_val) {
        fprintf(stderr, "[NUMBER_GENERATOR]: Generated number is wrong! (DIV_GEN)\n");
        exit(EXIT_FAILURE);
    }
}

static uint64_t generate_from_current_inc_cost(num_t curr_val, num_t target_val) {
    num_t diff = target_val - curr_val;
    return ABS(diff);
}

static void generate_from_current_inc(reg *r, num_t curr_val, num_t target_val, FILE *file) {
    num_t diff = target_val - curr_val;
    num_t test = curr_val;

    while (diff > 0) {
        fprintf(file, "INC %c\n", r->id);
        --diff;
        ++test;
    }

    while (diff < 0) {
        fprintf(file, "DEC %c\n", r->id);
        ++diff;
        --test;
    }

    if (test != target_val) {
        fprintf(stderr, "[NUMBER_GENERATOR]: Generated number is wrong! (INC_GEN)\n");
        exit(EXIT_FAILURE);
    }
}

void generate_value(reg *r, num_t curr_val, num_t target_val, FILE *file, bool reset) {
    if (curr_val != 0 && !reset) {
        uint64_t reset_cost = generate_from_reset_cost(target_val);
        uint64_t div_cost = generate_from_current_div_cost(curr_val, target_val);
        uint64_t inc_cost = generate_from_current_inc_cost(curr_val, target_val);

        if (reset_cost <= div_cost && reset_cost < inc_cost) {
            fprintf(file, "RESET %c\n", r->id);
            generate_from_reset(r, target_val, file);
        } else if (div_cost < inc_cost) {
            generate_from_current_div(r, curr_val, target_val, file);
        } else {
            generate_from_current_inc(r, curr_val, target_val, file);
        }
    } else {
        if (reset) {
            fprintf(file, "RESET %c\n", r->id);
        }

        generate_from_reset(r, target_val, file);
    }
}

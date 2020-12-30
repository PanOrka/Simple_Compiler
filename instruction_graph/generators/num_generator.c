#include "num_generator.h"

#ifndef ABS
#define ABS(x) (x >= 0 ? x : -x)
#endif

static uint64_t generate_from_reset_cost(addr_t val) {
    uint64_t cost = 0;

    bool inc_once = false;
    for (int32_t i=0; i<8*sizeof(addr_t); ++i) {
        if (inc_once) {
            ++cost;
        }

        if (val & ADDR_T_MSB) {
            ++cost;
            inc_once = true;
        }
        val <<= 1;
    }

    return cost + 1;
}

static void generate_from_reset(reg *r, addr_t val, FILE *file) {
    bool inc_once = false;
    for (int32_t i=0; i<8*sizeof(addr_t); ++i) {
        if (inc_once) {
            fprintf(file, "SHL %c\n", r->id);
        }

        if (val & ADDR_T_MSB) {
            fprintf(file, "INC %c\n", r->id);
            inc_once = true;
        }
        val <<= 1;
    }
}

static uint64_t generate_from_current_div_cost(addr_t curr_val, addr_t target_val) {
    uint64_t cost = 0;

    int64_t diff = target_val - curr_val;
    while (diff >= 0 && curr_val < (uint64_t)diff) {
        curr_val *= 2;
        diff = target_val - curr_val;
        ++cost;
    }

    while (diff > 2 || diff < -2) {
        const int64_t alternative = diff - curr_val;
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

    return cost + ABS(diff);
}

static void generate_from_current_div(reg *r, addr_t curr_val, addr_t target_val, FILE *file) {
    int64_t diff = target_val - curr_val;
    while (diff >= 0 && curr_val < (uint64_t)diff) {
        curr_val *= 2;
        diff = target_val - curr_val;
        //++cost;
    }

    while (diff > 2 || diff < -2) {
        const int64_t alternative = diff - curr_val;
        if (ABS(alternative) < ABS(diff)) {
            curr_val *= 2;
            diff = alternative;
            //++cost;
        } else {
            int32_t reminder = (curr_val % 2) + (diff % 2);
            diff /= 2;
            curr_val /= 2;

            //cost += 2;
            if (reminder != 0) {
                //++cost;
            }
        }
    }

    //return cost + ABS(diff);
}

static uint64_t generate_from_current_inc_cost(addr_t curr_val, addr_t target_val) {
    return ABS(target_val - curr_val);
}

static uint64_t generate_from_current_inc(reg *r, addr_t curr_val, addr_t target_val, FILE *file) {
    int64_t diff = target_val - curr_val;
    while (diff != 0) {
        if (diff > 0) {
            fprintf(file, "INC %c\n", r->id);
            --diff;
        } else {
            fprintf(file, "DEC %c\n", r->id);
            ++diff;
        }
    }
}

void generate_value(reg *r, addr_t curr_val, addr_t target_val, FILE *file, bool reset) {
    if (curr_val != 0 && !reset) {
        uint64_t reset_cost = generate_from_reset_cost(target_val);
        uint64_t div_cost = generate_from_current_div_cost(curr_val, target_val);
        uint64_t inc_cost = generate_from_current_inc_cost(curr_val, target_val);

        generate_from_current_inc(r, curr_val, target_val, file);
    } else {
        if (reset) {
            fprintf(file, "RESET %c\n", r->id);
        }

        generate_from_reset(r, target_val, file);
    }
}

#include "std_oper.h"
#include "../generators/stack_generator.h"
#include "../generators/val_generator.h"
#include "../../parser_func/getters.h"

reg_allocator oper_get_reg_for_variable(addr_t addr, FILE *file) {
    reg_set *r_set = get_reg_set();
    reg_allocator r_alloc;
    if (addr == ADDR_UNDEF) {
        r_alloc = reg_m_LRU(r_set, false);
    } else {
        r_alloc = reg_m_get(r_set, addr, true);
    }

    if (!r_alloc.was_allocated) {
        if (r_alloc.r->flags & REG_MODIFIED) {
            stack_ptr_generate(r_alloc.r->addr, file);
            fprintf(file, "STORE %c %c\n", r_alloc.r->id, r_set->stack_ptr.id);
        }

        r_alloc.r->addr = ADDR_UNDEF;
        r_alloc.r->flags = REG_NO_FLAGS;
    }

    return r_alloc;
}

void oper_load_variable_to_reg(reg *r, addr_t addr, FILE *file) {
    reg_set *r_set = get_reg_set();
    stack_ptr_generate(addr, file);
    fprintf(file, "LOAD %c %c\n", r->id, r_set->stack_ptr.id);
    r->addr = addr;
}

static void add_sub_val_gen(reg *stack_ptr, addr_t arr_addr, reg *var_idx, uint64_t start_idx, FILE *file) {
    stack_ptr_generate(arr_addr, file);
    fprintf(file, "ADD %c %c\n", stack_ptr->id, var_idx->id);
    stack_ptr_clear();
    reg *s_idx = val_generate(start_idx, file);
    fprintf(file, "SUB %c %c\n", stack_ptr->id, s_idx->id);
}

static void dec_val_gen(reg *stack_ptr, addr_t arr_addr, reg *var_idx, num_t eff_addr, FILE *file) {
    stack_ptr_generate(arr_addr, file);
    fprintf(file, "ADD %c %c\n", stack_ptr->id, var_idx->id);
    stack_ptr_clear();
    for (int i=eff_addr; i < 0; ++i) {
        fprintf(file, "DEC %c\n", stack_ptr->id);
    }
}

void oper_set_stack_ptr_addr_arr(addr_t var_idx_addr, addr_t arr_addr, uint64_t start_idx, FILE *file) {
    reg_set *r_set = get_reg_set();

    reg_allocator var_idx = oper_get_reg_for_variable(var_idx_addr, file);
    if (!var_idx.was_allocated) {
        oper_load_variable_to_reg(var_idx.r, var_idx_addr, file);
    }

    num_t const eff_addr = (num_t)arr_addr - (num_t)start_idx;
    if (eff_addr >= 0) {
        stack_ptr_generate(eff_addr, file);
        fprintf(file, "ADD %c %c\n", r_set->stack_ptr.id, var_idx.r->id);
        stack_ptr_clear();
    } else if (eff_addr >= -5) {
        dec_val_gen(&(r_set->stack_ptr), arr_addr, var_idx.r, eff_addr, file);
    } else if (reg_m_get(r_set, VAL_GEN_ADDR, false).was_allocated) {
        add_sub_val_gen(&(r_set->stack_ptr), arr_addr, var_idx.r, start_idx, file);
    } else if (!(reg_m_get(r_set, VAL_GEN_ADDR, false).r->flags & REG_MODIFIED)) {
        add_sub_val_gen(&(r_set->stack_ptr), arr_addr, var_idx.r, start_idx, file);
    } else if (eff_addr >= -20) {
        dec_val_gen(&(r_set->stack_ptr), arr_addr, var_idx.r, eff_addr, file);
    } else {
        add_sub_val_gen(&(r_set->stack_ptr), arr_addr, var_idx.r, start_idx, file);
    }
}

void oper_reg_swap(reg *r1, reg *r2, FILE *file) {
    fprintf(file, "RESET %c\n", r1->id);
    fprintf(file, "ADD %c %c\n", r1->id, r2->id);
}

void oper_store_array(const addr_t addr[2], FILE *file) {
    reg_set *r_set = get_reg_set();

    for (int32_t i=0; i<REG_SIZE; ++i) {
        reg * const r = r_set->r[i];
        if (r->addr >= addr[0] && r->addr < addr[1]) {
            stack_ptr_generate(r->addr, file);
            fprintf(file, "STORE %c %c\n", r->id, r_set->stack_ptr.id);
            reg_m_drop_addr(r_set, r->addr);
        }
    }
}

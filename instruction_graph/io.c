#include "expr_checker.h"
#include "i_graph.h"
#include "../parser_func/getters.h"
#include "generators/stack_generator.h"
#include "generators/num_generator.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_READ(expression_t *expr) {
    eval_check_0(expr);
    add_to_list(expr, i_READ);
}

void eval_READ(i_graph **i_current, FILE *file) {
    expression_t const * const expr_curr = (*i_current)->payload;
    reg_set *r_set = get_reg_set();

    if (expr_curr->var_1[0].var->flags & SYMBOL_IS_ARRAY) {
        if (!(expr_curr->mask & ASSIGN_SYM2_NUM)) {
            
        } else {
            stack_ptr_generate(expr_curr->var_1[0].var->addr[0] + (addr_t)expr_curr->var_2[0].num, file);
            fprintf(file, "GET %c\n", r_set->stack_ptr.id);
        }
    } else {
        stack_ptr_generate(expr_curr->var_1[0].var->addr[0], file);
        fprintf(file, "GET %c\n", r_set->stack_ptr.id);
    }
}

void add_WRITE(expression_t *expr) {
    eval_check_1(expr);
    add_to_list(expr, i_WRITE);
}

void eval_WRITE(i_graph **i_current, FILE *file) {
    expression_t const * const expr_curr = (*i_current)->payload;
    reg_set *r_set = get_reg_set();

    if (!(expr_curr->mask & LEFT_SYM1_NUM)) {
        if (expr_curr->var_1[1].var->flags & SYMBOL_IS_ARRAY) {
            if (!(expr_curr->mask & LEFT_SYM2_NUM)) {
            
            } else {
                stack_ptr_generate(expr_curr->var_1[1].var->addr[0] + (addr_t)expr_curr->var_2[1].num, file);
                fprintf(file, "PUT %c\n", r_set->stack_ptr.id);
            }
        } else {
            stack_ptr_generate(expr_curr->var_1[1].var->addr[0], file);
            fprintf(file, "PUT %c\n", r_set->stack_ptr.id);
        }
    } else {
        symbol_table *s_table = get_symbol_table();
        symbol *sym = symbol_table_add(s_table, NULL, (add_info){ .start_idx = 0 }, 1, SYMBOL_NO_FLAGS);

        reg_allocator r_alloc = reg_m_LRU(r_set, false);
        if (r_alloc.r->flags & REG_MODIFIED) {
            stack_ptr_generate(r_alloc.r->addr, file);
            fprintf(file, "STORE %c %c\n", r_alloc.r->id, r_set->stack_ptr.id);
        }

        generate_value(r_alloc.r, 0, (addr_t)expr_curr->var_1[1].num, file, true);
        stack_ptr_generate(sym->addr[0], file);
        fprintf(file, "STORE %c %c\n", r_alloc.r->id, r_set->stack_ptr.id);
        r_alloc.r->addr = ADDR_UNDEF;
        r_alloc.r->flags = REG_NO_FLAGS;

        fprintf(file, "PUT %c\n", r_set->stack_ptr.id);

        symbol_table_pop(s_table);
    }
}

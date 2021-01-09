#include "../parser_func/expressions.h"
#include "i_level.h"
#include "expr_checker.h"
#include "std_oper/std_oper.h"
#include "conditions/cond.h"

extern void add_to_list(void *payload, instruction_type i_type);

void add_IF(expression_t *expr) {
    eval_check_1(expr);
    eval_check_2(expr);

    i_level_add(i_IF);
    add_to_list(expr, i_IF);
}

void eval_IF(i_graph **i_current) {
    expression_t const * const expr = (*i_current)->payload;
    val assign_val_1 = oper_get_assign_val_1(expr);
    val assign_val_2 = oper_get_assign_val_2(expr);

    if (i_level_is_empty()) {
        if (!(assign_val_1.is_reg || assign_val_2.is_reg)) { // both constants
            bool cond = cond_val_from_const(assign_val_1.constant, assign_val_2.constant, expr->type);
            mpz_clear(assign_val_1.constant);
            mpz_clear(assign_val_2.constant);
            i_graph_clear_if(cond, i_current);
        } else {
            i_graph_analyze_if(i_current);
        }
    } else {

    }
}

void add_ELSE() {
    if (!i_level_is_empty()) {
        branch_type b_type = i_level_pop(i_NOPOP);
        if (b_type == i_IF) {
            add_to_list(NULL, i_ELSE);
            return;
        }
    }

    fprintf(stderr, "[IF]: There is no matching IF for ELSE!\n");
    exit(EXIT_FAILURE);
}

void eval_ELSE(i_graph **i_current) {
    
}

void add_ENDIF() {
    if (!i_level_is_empty()) {
        branch_type b_type = i_level_pop(i_POP);
        if (b_type == i_IF) {
            add_to_list(NULL, i_ENDIF);
            return;
        }
    }

    fprintf(stderr, "[IF]: There is no matching IF for ENDIF!\n");
    exit(EXIT_FAILURE);
}

void eval_ENDIF(i_graph **i_current) {
    
}

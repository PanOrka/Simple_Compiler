#include "i_graph.h"
#include "instructions/asm_fprintf.h"

#include "../parser_func/expressions.h"

#include "../parser_func/loops.h"
#include "../definitions.h"
#include "i_level.h"

#include "../parser_func/getters.h"

static i_graph *start = NULL;
static i_graph *end = NULL;

void add_to_list(void *payload, instruction_type i_type) {
    i_graph *new_element = malloc(sizeof(i_graph));
    if (new_element) {
        new_element->payload = payload;
        new_element->i_type = i_type;

        if (start) {
            end->next = new_element;

            new_element->next = NULL;
            new_element->prev = end;
        } else {
            new_element->prev = NULL;
            new_element->next = NULL;
            start = new_element;
        }

        end = new_element;
    } else {
        fprintf(stderr, "[I_GRAPH]: Couldn't allocate memory for I_Graph node!\n");
        exit(EXIT_FAILURE);
    }
}


extern void add_EXPR(expression_t *expr);

extern void add_IF(expression_t *expr);
extern void add_ELSE();
extern void add_ENDIF();

extern void add_WHILE(expression_t *expr);
extern void add_ENDWHILE();

extern void add_REPEAT();
extern void add_UNTIL(expression_t *expr);

extern void add_FOR(for_loop_t *loop);
extern void add_ENDFOR();

extern void add_READ(expression_t *expr);
extern void add_WRITE(expression_t *expr);


void i_graph_add_instruction(void *payload, instruction_type i_type) {
    switch (i_type) {
        case i_EXPR:
            add_EXPR(payload);
            break;
        case i_IF:
            add_IF(payload);
            break;
        case i_ELSE:
            add_ELSE();
            break;
        case i_ENDIF:
            add_ENDIF();
            break;
        case i_WHILE:
            add_WHILE(payload);
            break;
        case i_ENDWHILE:
            add_ENDWHILE();
            break;
        case i_REPEAT:
            add_REPEAT();
            break;
        case i_UNTIL:
            add_UNTIL(payload);
            break;
        case i_FOR:
            add_FOR(payload);
            break;
        case i_ENDFOR:
            add_ENDFOR();
            break;
        case i_READ:
            add_READ(payload);
            break;
        case i_WRITE:
            add_WRITE(payload);
            break;
        default:
            fprintf(stderr, "Unknown type of instruction: %d!\n", i_type);
            exit(EXIT_FAILURE);
    }
}


void eval_EXPR(i_graph **i_current);

void eval_IF(i_graph **i_current);
void eval_ELSE(i_graph **i_current);
void eval_ENDIF(i_graph **i_current);

void eval_WHILE(i_graph **i_current);
void eval_ENDWHILE(i_graph **i_current);

void eval_REPEAT(i_graph **i_current);
void eval_UNTIL(i_graph **i_current);

void eval_FOR(i_graph **i_current);
void eval_ENDFOR(i_graph **i_current);

void eval_READ(i_graph **i_current);
void eval_WRITE(i_graph **i_current);


void i_graph_execute(FILE *file) {
    if (!i_level_is_empty()) {
        fprintf(stderr, "[I_GRAPH]: Conditional instructions stack non empty!\n");
        exit(EXIT_FAILURE);
    }

    asm_fprintf_set_file(file);
    while (start) {
        switch (start->i_type) {
            case i_EXPR:
                eval_EXPR(&start);
                break;
            case i_IF:
                eval_IF(&start);
                break;
            case i_ELSE:
                eval_ELSE(&start);
                break;
            case i_ENDIF:
                eval_ENDIF(&start);
                break;
            case i_WHILE:
                eval_WHILE(&start);
                break;
            case i_ENDWHILE:
                eval_ENDWHILE(&start);
                break;
            case i_REPEAT:
                eval_REPEAT(&start);
                break;
            case i_UNTIL:
                eval_UNTIL(&start);
                break;
            case i_FOR:
                eval_FOR(&start);
                break;
            case i_ENDFOR:
                eval_ENDFOR(&start);
                break;
            case i_READ:
                eval_READ(&start);
                break;
            case i_WRITE:
                eval_WRITE(&start);
                break;
            default:
                fprintf(stderr, "Unknown type of instruction: %d!\n", start->i_type);
                exit(EXIT_FAILURE);
        }

        start = start->next;
    }

    HALT();

    while (end) {
        i_graph *to_free = end;
        end = end->prev;
        free(to_free->payload);
        free(to_free);
    }

    free_symbol_table();
    free_reg_set();
}

static void i_delete_node(i_graph *to_free) {
    if (to_free->prev) {
        to_free->prev->next = to_free->next;
    }

    if (to_free->next) {
        to_free->next->prev = to_free->prev;
    } else {
        end = to_free->prev;
    }
    free(to_free->payload);
    free(to_free);
}

/**
 * 
 * Frees everything except start_ptr
 * 
*/
static void i_clear(i_graph *start_ptr, i_graph *end_ptr) {
    while (end_ptr != start_ptr) {
        if (end_ptr == NULL) {
            fprintf(stderr, "[I_GRAPH]: NULL ptr on i_clear - WRONG ITERATORS!\n");
            exit(EXIT_FAILURE);
        }

        i_graph *to_free = end_ptr;
        end_ptr = end_ptr->prev;
        i_delete_node(to_free);
    }
}

static void i_graph_if_find(i_graph *i_if, i_graph **i_else, i_graph **i_endif) {
    i_level_add(i_IF);
    i_graph *ptr = i_if->next;

    while (!i_level_is_empty()) {
        if (ptr == NULL) { // not possible but let's check it
            fprintf(stderr, "[I_GRAPH]: NULL ptr on if-find!\n");
            exit(EXIT_FAILURE);
        }

        switch (ptr->i_type) {
            case i_IF:
                i_level_add(i_FOR);
                break;
            case i_ELSE:
                if (i_level_pop(i_NOPOP) == i_IF) {
                    *i_else = ptr;
                }
                break;
            case i_ENDIF:
                if (i_level_pop(i_POP) == i_IF) {
                    *i_endif = ptr;
                }
                break;
            case i_WHILE:
                i_level_add(i_FOR);
                break;
            case i_ENDWHILE:
                i_level_pop(i_POP);
                break;
            case i_REPEAT:
                i_level_add(i_FOR);
                break;
            case i_UNTIL:
                i_level_pop(i_POP);
                break;
            case i_FOR:
                i_level_add(i_FOR);
                break;
            case i_ENDFOR:
                i_level_pop(i_POP);
                break;
        }

        ptr = ptr->next;
    }
}

void i_graph_clear_if(bool cond, i_graph **i_current) {
    if (i_level_is_empty()) {
        i_graph *i_if = *i_current;
        i_graph *i_else = NULL;
        i_graph *i_endif = NULL;

        i_graph_if_find(i_if, &i_else, &i_endif);

        if (cond) {
            if (i_else) {
                i_clear(i_else, i_endif);
                i_delete_node(i_else);
                *i_current = i_if;
            } else {
                i_delete_node(i_endif);
                *i_current = i_if;
            }
        } else {
            if (i_else) {
                i_clear(i_if, i_else);
                i_delete_node(i_endif);
                *i_current = i_if;
            } else {
                i_clear(i_if, i_endif);
                *i_current = i_if;
            }
        }

        return ;
    }

    fprintf(stderr, "[I_GRAPH]: Clear of non empty i_level stack!\n");
    exit(EXIT_FAILURE);
}

static void i_graph_mark(i_graph *start_ptr, i_graph *end_ptr) {
    symbol_table *s_table = get_symbol_table();

    while (start_ptr != end_ptr) {
        if (start_ptr->i_type == i_EXPR || start_ptr->i_type == i_READ) {
            expression_t *expr = start_ptr->payload;
            symbol *var_1 = symbol_table_find_by_idx(s_table, expr->var_1[0].sym_idx);
            var_1->flags |= SYMBOL_MARK_STORE;
        }

        start_ptr = start_ptr->next;
    }
}

#include "std_oper/std_oper.h"
#include "generators/val_generator.h"
#include "generators/stack_generator.h"

static void i_graph_store_marked() {
    symbol_table *s_table = get_symbol_table();
    reg_set *r_set = get_reg_set();

    for (size_t i=0; i<s_table->v.used_size; ++i) {
        symbol *sym = symbol_table_find_by_idx(s_table, i);
        if (sym->flags & SYMBOL_MARK_STORE) {
            sym->flags &= ~SYMBOL_MARK_STORE;
            const bool sym_const = sym->flags & SYMBOL_IS_CONSTANT;

            if (sym_const && sym->flags & SYMBOL_IS_ARRAY) {
                oper_flush_array_to_mem(sym);
                oper_arr_set_non_constant(sym);
            } else if (sym_const) {
                sym->flags &= ~SYMBOL_IS_CONSTANT;
                reg *val_reg = val_generate_from_mpz(sym->consts.value);
                mpz_set_si(sym->consts.value, 0);
                stack_ptr_generate(sym->addr[0]);
                STORE(val_reg, &(r_set->stack_ptr));
                sym->symbol_in_memory = true;
            }
        }
    }
}

void i_graph_analyze_if(i_graph **i_current) {
    if (i_level_is_empty_eval()) {
        i_graph *i_if = *i_current;
        i_graph *i_else = NULL;
        i_graph *i_endif = NULL;

        i_graph_if_find(i_if, &i_else, &i_endif);
        i_graph_mark(i_if, i_endif);
        i_graph_store_marked();

        return ;
    }

    fprintf(stderr, "[I_GRAPH]: Analyze of non empty i_level stack!\n");
    exit(EXIT_FAILURE);
}

static void i_graph_while_find(i_graph *i_while, i_graph **i_endwhile) {
    i_level_add(i_WHILE);
    i_graph *ptr = i_while->next;

    while (!i_level_is_empty()) {
        if (ptr == NULL) { // not possible but let's check it
            fprintf(stderr, "[I_GRAPH]: NULL ptr on while-find!\n");
            exit(EXIT_FAILURE);
        }

        switch (ptr->i_type) {
            case i_IF:
                i_level_add(i_FOR);
                break;
            case i_ELSE:
                i_level_pop(i_NOPOP);
                break;
            case i_ENDIF:
                i_level_pop(i_POP);
                break;
            case i_WHILE:
                i_level_add(i_FOR);
                break;
            case i_ENDWHILE:
                if (i_level_pop(i_POP) == i_WHILE) {
                    *i_endwhile = ptr;
                }
                break;
            case i_REPEAT:
                i_level_add(i_FOR);
                break;
            case i_UNTIL:
                i_level_pop(i_POP);
                break;
            case i_FOR:
                i_level_add(i_FOR);
                break;
            case i_ENDFOR:
                i_level_pop(i_POP);
                break;
        }

        ptr = ptr->next;
    }
}

void i_graph_clear_while(bool cond, i_graph **i_current) {
    if (i_level_is_empty()) {
        i_graph *i_while = *i_current;
        i_graph *i_endwhile = NULL;

        i_graph_while_find(i_while, &i_endwhile);

        if (cond) {
            fprintf(stderr, "[I_GRAPH]: Endless loop in code!\n");
            print_expression(i_while->payload, stderr);
            fprintf(stderr, "\n");
            exit(EXIT_FAILURE);
        } else {
            i_clear(i_while, i_endwhile);
            *i_current = i_while;
        }

        return ;
    }

    fprintf(stderr, "[I_GRAPH]: Clear of non empty i_level stack!\n");
    exit(EXIT_FAILURE);
}

void i_graph_analyze_while(i_graph **i_current) {
    if (i_level_is_empty_eval()) {
        i_graph *i_while = *i_current;
        i_graph *i_endwhile = NULL;

        i_graph_while_find(i_while, &i_endwhile);
        i_graph_mark(i_while, i_endwhile);
        i_graph_store_marked();

        return ;
    }

    fprintf(stderr, "[I_GRAPH]: Analyze of non empty i_level stack!\n");
    exit(EXIT_FAILURE);
}

static void i_graph_for_find(i_graph *i_for, i_graph **i_endfor) {
    i_level_add(i_FOR);
    i_graph *ptr = i_for->next;

    while (!i_level_is_empty()) {
        if (ptr == NULL) { // not possible but let's check it
            fprintf(stderr, "[I_GRAPH]: NULL ptr on for-find!\n");
            exit(EXIT_FAILURE);
        }

        switch (ptr->i_type) {
            case i_IF:
                i_level_add(i_IF);
                break;
            case i_ELSE:
                i_level_pop(i_NOPOP);
                break;
            case i_ENDIF:
                i_level_pop(i_POP);
                break;
            case i_WHILE:
                i_level_add(i_IF);
                break;
            case i_ENDWHILE:
                i_level_pop(i_POP);
                break;
            case i_REPEAT:
                i_level_add(i_IF);
                break;
            case i_UNTIL:
                i_level_pop(i_POP);
                break;
            case i_FOR:
                i_level_add(i_IF);
                break;
            case i_ENDFOR:
                if (i_level_pop(i_POP) == i_FOR) {
                    *i_endfor = ptr;
                }
                break;
        }

        ptr = ptr->next;
    }
}

void i_graph_analyze_for(i_graph **i_current) {
    if (i_level_is_empty_eval()) {
        i_graph *i_for = *i_current;
        i_graph *i_endfor = NULL;

        i_graph_for_find(i_for, &i_endfor);
        i_graph_mark(i_for, i_endfor);
        i_graph_store_marked();

        return ;
    }

    fprintf(stderr, "[I_GRAPH]: Analyze of non empty i_level stack!\n");
    exit(EXIT_FAILURE);
}

static void i_graph_repeat_until_find(i_graph *i_repeat, i_graph **i_until) {
    i_level_add(i_REPEAT);
    i_graph *ptr = i_repeat->next;

    while (!i_level_is_empty()) {
        if (ptr == NULL) { // not possible but let's check it
            fprintf(stderr, "[I_GRAPH]: NULL ptr on repeat_until-find!\n");
            exit(EXIT_FAILURE);
        }

        switch (ptr->i_type) {
            case i_IF:
                i_level_add(i_FOR);
                break;
            case i_ELSE:
                i_level_pop(i_NOPOP);
                break;
            case i_ENDIF:
                i_level_pop(i_POP);
                break;
            case i_WHILE:
                i_level_add(i_FOR);
                break;
            case i_ENDWHILE:
                i_level_pop(i_POP);
                break;
            case i_REPEAT:
                i_level_add(i_FOR);
                break;
            case i_UNTIL:
                if (i_level_pop(i_POP) == i_REPEAT) {
                    *i_until = ptr;
                }
                break;
            case i_FOR:
                i_level_add(i_FOR);
                break;
            case i_ENDFOR:
                i_level_pop(i_POP);
                break;
        }

        ptr = ptr->next;
    }
}

void i_graph_analyze_repeat_until(i_graph **i_current) {
    if (i_level_is_empty_eval()) {
        i_graph *i_repeat = *i_current;
        i_graph *i_until = NULL;

        i_graph_repeat_until_find(i_repeat, &i_until);
        i_graph_mark(i_repeat, i_until);
        i_graph_store_marked();

        return ;
    }

    fprintf(stderr, "[I_GRAPH]: Analyze of non empty i_level stack!\n");
    exit(EXIT_FAILURE);
}

#include "i_graph.h"

#include "../parser_func/expressions.h"
#include "../parser_func/getters.h"
#include "../definitions.h"

static i_graph *start = NULL;
static i_graph *end = NULL;

static void add_to_list(void *payload, instruction_type i_type) {
    i_graph *new_element = malloc(sizeof(i_graph));
    if (new_element) {
        new_element->payload = payload;
        new_element->i_type = i_type;

        if (start) {
            end->next = new_element;

            new_element->next = NULL;
            new_element->prev = end;

            end = new_element;
        } else {
            new_element->prev = NULL;
            new_element->next = NULL;
            start = new_element;
            end = new_element;
        }
    } else {
        fprintf(stderr, "[I_GRAPH]: Couldn't allocate memory for I_Graph node!\n");
        exit(EXIT_FAILURE);
    }
}

static void add_EXPR(expression_t *expr) {
    if (expr->var_1[0].var->flags & SYMBOL_IS_ARRAY) {
        if (!(expr->mask & ASSIGN_SYM2_NUM) && !(expr->var_2[0].var->flags & SYMBOL_INITIALIZED)) {
            fprintf(stderr, "[I_GRAPH]: Symbol %s is not initialized!\n", expr->var_2[0].var->identifier);
            fprintf(stderr, "[EXPRESSION]:\n");
            print_expression(expr);
            exit(EXIT_FAILURE);
        }
    } else {
        expr->var_1[0].var->flags |= SYMBOL_INITIALIZED;
    }

    if (!(expr->mask & LEFT_SYM1_NUM)) {
        if (expr->var_1[1].var->flags & SYMBOL_IS_ARRAY) {
            if (!(expr->mask & LEFT_SYM2_NUM) && !(expr->var_2[1].var->flags & SYMBOL_INITIALIZED)) {
                fprintf(stderr, "[I_GRAPH]: Symbol %s is not initialized!\n", expr->var_2[1].var->identifier);
                fprintf(stderr, "[EXPRESSION]:\n");
                print_expression(expr);
                exit(EXIT_FAILURE);
            }
        } else if (!(expr->var_1[1].var->flags & SYMBOL_INITIALIZED)) {
            fprintf(stderr, "[I_GRAPH]: Symbol %s is not initialized!\n", expr->var_1[1].var->identifier);
            fprintf(stderr, "[EXPRESSION]:\n");
            print_expression(expr);
            exit(EXIT_FAILURE);
        }
    }

    if (expr->type != expr_VALUE) {
        if (!(expr->mask & RIGHT_SYM1_NUM)) {
            if (expr->var_1[2].var->flags & SYMBOL_IS_ARRAY) {
                if (!(expr->mask & RIGHT_SYM2_NUM) && !(expr->var_2[2].var->flags & SYMBOL_INITIALIZED)) {
                    fprintf(stderr, "[I_GRAPH]: Symbol %s is not initialized!\n", expr->var_2[2].var->identifier);
                    fprintf(stderr, "[EXPRESSION]:\n");
                    print_expression(expr);
                    exit(EXIT_FAILURE);
                }
            } else if (!(expr->var_1[2].var->flags & SYMBOL_INITIALIZED)) {
                fprintf(stderr, "[I_GRAPH]: Symbol %s is not initialized!\n", expr->var_1[2].var->identifier);
                fprintf(stderr, "[EXPRESSION]:\n");
                print_expression(expr);
                exit(EXIT_FAILURE);
            }
        }
    }

    add_to_list(expr, EXPR);
}

void i_graph_add_instruction(void *payload, instruction_type i_type) {
    switch (i_type) {
        case EXPR:
            add_EXPR(payload);
            break;
        default:
            fprintf(stderr, "Unknown type of instruction: %d!\n", i_type);
            exit(EXIT_FAILURE);
    }
}

static addr_t stack_ptr;

// JUST FOR TESTS
/////////////////////////////////// TESTING ZONE
static void generate_stack_ptr(addr_t addr, FILE *file, reg *r) {
    if (addr > stack_ptr) {
        while (addr != stack_ptr) {
            fprintf(file, "INC %c\n", r->id);
            ++stack_ptr;
        }
    } else if (addr < stack_ptr) {
        while (addr != stack_ptr) {
            fprintf(file, "DEC %c\n", r->id);
            --stack_ptr;
        }
    }
}

void i_graph_execute(FILE *file) {
    reg_set *regs = get_reg_set();

    i_graph *idx = start;
    // now it's just expressions
    while (idx) {
        expression_t *expr = idx->payload;
        reg_allocator r = reg_m_get(regs, STACK_PTR);
        if (!r.was_allocated) {
            fprintf(file, "RESET %c\n", r.r->id);
            stack_ptr = 0;
        }

        const addr_t address = expr->var_1[0].var->addr[0];
        reg_allocator r2 = reg_m_get(regs, address);
        if (!r2.was_allocated) {
            if (r2.r->addr != ADDR_UNDEF) {
                generate_stack_ptr(r2.r->addr, file, r.r);
                fprintf(file, "STORE %c %c\n", r2.r->id, r.r->id);
            }
            r2.r->addr = address;
            generate_stack_ptr(r2.r->addr, file, r.r);
            fprintf(file, "LOAD %c %c\n", r2.r->id, r.r->id);
        }

        idx = idx->next;
    }
}
///////////////////////////////////

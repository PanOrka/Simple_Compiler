#include "i_graph.h"

#include "../parser_func/expressions.h"
#include "../parser_func/getters.h"
#include "../parser_func/loops.h"
#include "../definitions.h"

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
        print_expression(expr, file);
        fprintf(file, "\n");
        reg_allocator r = reg_m_get(regs, STACK_PTR);
        if (!r.was_allocated) {
            fprintf(file, "RESET %c\n", r.r->id);
            r.r->addr = STACK_PTR;
            stack_ptr = 0;
        }

        addr_t const address = expr->var_1[0].var->addr[0];
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

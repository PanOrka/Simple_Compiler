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


void eval_EXPR(i_graph **i_current, FILE *file);

void eval_IF(i_graph **i_current, FILE *file);
void eval_ELSE(i_graph **i_current, FILE *file);
void eval_ENDIF(i_graph **i_current, FILE *file);

void eval_WHILE(i_graph **i_current, FILE *file);
void eval_ENDWHILE(i_graph **i_current, FILE *file);

void eval_REPEAT(i_graph **i_current, FILE *file);
void eval_UNTIL(i_graph **i_current, FILE *file);

void eval_FOR(i_graph **i_current, FILE *file);
void eval_ENDFOR(i_graph **i_current, FILE *file);

void eval_READ(i_graph **i_current, FILE *file);
void eval_WRITE(i_graph **i_current, FILE *file);


void i_graph_execute(FILE *file) {
    while (start) {
        switch (start->i_type) {
            case i_EXPR:
                eval_EXPR(&start, file);
                break;
            case i_IF:
                eval_IF(&start, file);
                break;
            case i_ELSE:
                eval_ELSE(&start, file);
                break;
            case i_ENDIF:
                eval_ENDIF(&start, file);
                break;
            case i_WHILE:
                eval_WHILE(&start, file);
                break;
            case i_ENDWHILE:
                eval_ENDWHILE(&start, file);
                break;
            case i_REPEAT:
                eval_REPEAT(&start, file);
                break;
            case i_UNTIL:
                eval_UNTIL(&start, file);
                break;
            case i_FOR:
                eval_FOR(&start, file);
                break;
            case i_ENDFOR:
                eval_ENDFOR(&start, file);
                break;
            case i_READ:
                eval_READ(&start, file);
                break;
            case i_WRITE:
                eval_WRITE(&start, file);
                break;
            default:
                fprintf(stderr, "Unknown type of instruction: %d!\n", start->i_type);
                exit(EXIT_FAILURE);
        }

        start = start->next;
    }

    while (end) {
        i_graph *to_free = end;
        end = end->prev;
        free(to_free->payload);
        free(to_free);
    }
}

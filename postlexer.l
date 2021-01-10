%option noyywrap

%{
#include "postlexer.h"
#include "vector/vector.h"
#include <stdint.h>
#include <stdlib.h>

int pplex();

static FILE *postlexer_out;
static vector jump_vec;

static size_t jump_vec_idx;

void get_jump_vec();
%}

%%
"X" {
    int64_t jump = *(int64_t *)vector_get(&jump_vec, jump_vec_idx);
    fprintf(postlexer_out, "%ld", jump);
    ++jump_vec_idx;
}

[\t \n] {
    fprintf(postlexer_out, "%s", pptext);
}

. {
    fprintf(postlexer_out, "%s", pptext);
}
%%

void postlex(FILE *in, FILE *out) {
    postlexer_out = out;
    ppin = in;
    get_jump_vec(&jump_vec);
    pplex();
    free(jump_vec._mem_ptr);
}

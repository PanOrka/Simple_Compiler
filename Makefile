all:
	bison -o compiler_parser.c -d compiler_parser.y && \
	flex -o compiler_lexer.c compiler_lexer.l && \
	flex -P zz -o prelexer.c prelexer.l && \
	gcc -lfl compiler.c compiler_parser.c compiler_lexer.c prelexer.c \
	vector/vector.c symbol_table/symbol_table.c register_machine/reg_m.c \
	parser_func/declarations.c parser_func/expressions.c parser_func/getters.c \
	instruction_graph/i_graph.c instruction_graph/expr.c parser_func/loops.c \
	instruction_graph/expr_checker.c instruction_graph/if.c instruction_graph/i_level.c \
	instruction_graph/while.c instruction_graph/repeat_until.c instruction_graph/io.c \
	instruction_graph/for.c instruction_graph/generators/stack_generator.c \
	instruction_graph/generators/num_generator.c instruction_graph/generators/val_generator.c -o compiler

clean:
	rm -rf compiler_parser.c compiler_parser.h compiler_lexer.c prelexer.c

cleanall:
	rm -rf compiler && \
	make clean

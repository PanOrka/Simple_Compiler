all:
	bison -o compiler_parser.c -d compiler_parser.y && \
	flex -o compiler_lexer.c compiler_lexer.l && \
	flex -P zz -o prelexer.c prelexer.l && \
	gcc -lfl compiler.c compiler_parser.c compiler_lexer.c prelexer.c \
	vector/vector.c symbol_table/symbol_table.c register_machine/reg_m.c \
	parser_func/declarations.c parser_func/expressions.c parser_func/getters.c \
	instruction_graph/i_graph.c -o compiler

clean:
	rm -rf compiler_parser.c compiler_parser.h compiler_lexer.c prelexer.c

cleanall:
	rm -rf compiler && \
	make clean

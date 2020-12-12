all:
	bison -o compiler_parser.c -d compiler_parser.y && \
	flex -o compiler_lexer.c compiler_lexer.l && \
	flex -P zz -o prelexer.c prelexer.l && \
	gcc -lfl compiler_parser.c compiler_lexer.c prelexer.c -o compiler

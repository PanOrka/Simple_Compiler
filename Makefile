all:
	bison -o compiler_parser.c -d compiler_parser.y && \
	flex -o compiler_lexer.c compiler_lexer.l && \
	flex -P zz -o prelexer.c prelexer.l && \
	gcc -lfl compiler.c compiler_parser.c compiler_lexer.c prelexer.c -o compiler

clean:
	rm -rf compiler_parser.c compiler_parser.h compiler_lexer.c prelexer.c

cleanall:
	rm -rf compiler && \
	make clean

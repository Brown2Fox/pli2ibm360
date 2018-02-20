pli2asm: pli2asm.parser.c pli2asm.lexer.c
	clang -Wno-everything pli2asm.parser.c pli2asm.lexer.c -o pli2asm
	rm *.c *.h *.output
	mv pli2asm ../build/

pli2asm.lexer.c: pli2asm.l
	flex -o pli2asm.lexer.c pli2asm.l

pli2asm.parser.c: pli2asm.y
	bison -d pli2asm.y -o pli2asm.parser.c 
	
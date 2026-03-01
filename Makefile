#
# Makefile that builds the jc object file and jc executable for testing
#

jc: jc.o token.o asm_gen.o stack.o
	clang-15 -g3 -gdwarf-4 -Wall -o jc jc.o token.o asm_gen.o stack.o

jc.o: jc.c token.h asm_gen.h stack.h
	clang-15 -g3 -gdwarf-4 -Wall -c jc.c

token.o: token.c token.h
	clang-15 -g3 -gdwarf-4 -Wall -c token.c

asm_gen.o: asm_gen.c token.h asm_gen.h
	clang-15 -g3 -gdwarf-4 -Wall -c asm_gen.c

stack.o: stack.c stack.h
	clang-15 -g3 -gdwarf-4 -Wall -c stack.c

clean:
	rm -f *.o jc *~

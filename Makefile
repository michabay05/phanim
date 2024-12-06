CFLAGS=-I./raylib/include/
S_LIBS=-L./raylib/lib/ -l:libraylib.a -lm
D_LIBS=-L./raylib/lib/ -lraylib -lm -ldl

main: src/main.c src/phanim.c
	gcc $(CFLAGS) -o build/main src/main.c src/phanim.c $(S_LIBS)

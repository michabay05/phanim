COMP=gcc
COMP_FLAGS=-Wall -Wextra -pedantic -ggdb
RL_CFLAGS=$(COMP_FLAGS) -I./vendor/raylib/include/
RL_SLIBS=-L./vendor/raylib/lib/ -l:libraylib.a -lm
RL_DLIBS=-L./vendor/raylib/lib/ -lraylib -lm -ldl
RESVG_INC=-I./vendor/resvg/
RESVG_SLIB=-L./vendor/resvg/ -l:libresvg.a -lm

all: main textest render_video resvg_test

main: src/main.c src/phanim.c
	$(COMP) $(RL_CFLAGS) -o build/main src/main.c src/phanim.c $(RL_SLIBS)

textest: src/textest.c
	$(COMP) $(COMP_FLAGS) -o build/textest src/textest.c

render_video: src/render_video.c
	$(COMP) $(RL_CFLAGS) -o build/render_video src/ffmpeg_linux.c src/render_video.c $(RL_SLIBS)

resvg_test: src/resvg_test.c
	$(COMP) $(COMP_FLAGS) $(RESVG_INC) -o build/resvg_test src/resvg_test.c $(RESVG_SLIB)

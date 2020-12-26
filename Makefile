OBJS=net.o vpl.o drfb0.o

CFLAGS=-O3 -fomit-frame-pointer

vpl: $(OBJS)
	gcc -o vpl $(OBJS)

sdl-win: sdl-win.c
	$(CC) -o $@ $< `sdl2-config --cflags --libs`

clean:
	-rm -f $(OBJS) vpl


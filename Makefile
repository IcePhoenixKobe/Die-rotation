CC=g++
CFLAGS=-Wall -g `pkg-config gtkmm-3.0 --cflags --libs`
OBJS=main.o kernel.o file_analyst.o chip.o windowApp.o gtk_cairo.o

all:die_rotation

die_rotation: $(OBJS)
	$(CC) $(OBJS) -o $@ $(CFLAGS)

main.o: main.cc main.h windowApp.h
	$(CC) -c -o $@ $< $(CFLAGS)

kernel.o: kernel.cc kernel.h windowApp.h
	$(CC) -c -o $@ $< $(CFLAGS)

file_analyst.o: file_analyst.cc file_analyst.h
	$(CC) -c -o $@ $< $(CFLAGS)

windowApp.o: windowApp.cc windowApp.h
	$(CC) -c -o $@ $< $(CFLAGS)

gtk_cairo.o: gtk_cairo.cc gtk_cairo.h
	$(CC) -c -o $@ $< $(CFLAGS)

chip.o: chip.cc chip.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf die_rotation $(OBJS)

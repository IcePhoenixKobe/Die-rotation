CC=g++
CFLAGS=-Wall -g -O3
OBJS=main.o chip.o ball.o die.o

all:die_rotation

die_rotation: $(OBJS)
	$(CC) $(OBJS) -o $@ $(CFLAGS)

main.o: main.cc main.h chip.h
	$(CC) -c -o $@ $< $(CFLAGS)

chip.o: chip.cc chip.h
	$(CC) -c -o $@ $< $(CFLAGS)

ball.o: ball.cc ball.h
	$(CC) -c -o $@ $< $(CFLAGS)

die.o: die.cc die.h
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf die_rotation $(OBJS)

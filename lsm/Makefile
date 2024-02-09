CC=gcc
CFLAGS=-I. -Wall -g
DEPS=memtable.h lsm.h test.h
OBJ=main.o memtable.o lsm.o test.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

lsm: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean run

clean:
	rm -f *.o lsm

run: lsm
	./lsm

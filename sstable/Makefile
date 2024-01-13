CC=gcc
CFLAGS=-I. -Wall -g
DEPS=memtable.h sstable.h test.h
OBJ=main.o memtable.o sstable.o test.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sstable: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean run

clean:
	rm -f *.o sstable

run: sstable
	./sstable

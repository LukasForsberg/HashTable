CC=g++ -std=gnu++11
CFLAGS=-I
DEPS = HashTable.h
OBJ = HashTable.o SingleThreadTest.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

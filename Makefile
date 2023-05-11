.PHONY: all clean

CC = gcc

CCFLAGS = -c -Wall -Wextra -Wvla -Werror -g -std=c99

all: libhashmap.a libhashmap_tests.a

clean:
	rm *.o *.a

libhashmap.a: hashmap.o vector.o pair.o
	ar rcs $@ $^

libhashmap_tests.a: test_suite.o
	ar rcs $@ $^

hashmap.o: hashmap.c hashmap.h vector.h pair.h
	$(CC) $(CCFLAGS) -c $<

vector.o: vector.c vector.h
	$(CC) $(CCFLAGS) -c $<

test_suite.o: test_suite.c test_suite.h test_pairs.h hash_funcs.h
	$(CC) $(CCFLAGS) -c $<

pair.o: pair.c pair.h
	$(CC) $(CCFLAGS) -c $<
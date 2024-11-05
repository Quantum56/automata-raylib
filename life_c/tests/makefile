all: set
	cc test_set.c ../include/libset.a -o test_set -std=c11 -Wall -O3 -I../include/

set:
	cc ../include/set.c -c -o ../include/set.o
	ar rcs ../include/libset.a ../include/set.o

clean:
	rm -rf test_set
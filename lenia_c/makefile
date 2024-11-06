lenia: set bmp
	cc lenia.c ./include/libset.a -o ./bin/lenia -Wall -Wextra -I~/raylib/src -lm -lraylib -I./include/ -O3 -fopenmp

set:
	cc ./include/set.c -c -o ./include/set.o
	ar rcs ./include/libset.a ./include/set.o

bmp:
	cc ./include/bmpfile.c -c -o ./include/bmpfile.o
	ar rcs ./include/bmpfile.a ./include/bmpfile.o

queue:
	cc ./include/queue.c -c -o ./include/queue.o
	ar rcs ./include/libqueue.a ./include/queue.o

test:
	cc test.c -o test -Wall -Wextra -I./include/ -lgsl -lgslcblas

clean:
	rm -rf ./bin/*

all: lenia test
	echo "made all\n"

life_q: set
	cc life_q.c ./include/libset.a -o ./bin/life_q -Wall -I~/raylib/src -lraylib -lm -I./include/

life_qt: set bmp
	cc life_q_torus.c ./include/libset.a ./include/bmpfile.a -o ./bin/life_qt -Wall -I~/raylib/src -lraylib -lm -I./include/

set:
	cc ./include/set.c -c -o ./include/set.o
	ar rcs ./include/libset.a ./include/set.o

bmp:
	cc ./include/bmpfile.c -c -o ./include/bmpfile.o
	ar rcs ./include/bmpfile.a ./include/bmpfile.o

queue:
	cc ./include/queue.c -c -o ./include/queue.o
	ar rcs ./include/libqueue.a ./include/queue.o

life:
	cc life.c -o ./bin/life -Wall -I~/raylib/src -lraylib -lm

tests:
	cc test.c -o ./bin/test -Wall -I~/raylib/src -lraylib -lm
	cc test_raylib.c -o ./bin/test_raylib -Wall -I~/raylib/src -lraylib -lm
	cc test2.c -o ./bin/test2 -Wall -I~/raylib/src -lraylib -lm

clean:
	rm -rf "test"

all: life life_q life_qt
	echo "made all!"

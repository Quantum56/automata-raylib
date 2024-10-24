test:
	cc test.c -o test -I~/raylib/src -lraylib -lm && ./test

clean:
	rm -rf "test"

all:
	echo "made all!"

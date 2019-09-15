CFLAGS=-std=c11 -g -stati

xcc: xcc.c

test: xcc
	./test.sh

clean:
	rm -f xcc *.o *~ tmp*

.PHONY: test clean

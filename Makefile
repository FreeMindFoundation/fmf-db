all:
	cc -m32 -O2 -Wall -std=c99 -L./lib/ -L./libal/lib/ -lal -c filedb.c sha256.c
	ar -cvq ./lib/libfiledb.a filedb.o sha256.o

clean:
	rm *.o ./lib/libfiledb.a

CC=g++
CFLAGS=-c -Wall -std=c++11

all: mandelbrot

mandelbrot: main.o
	$(CC) main.o -lpthread -o mandelbrot

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	rm *o mandelbrot

run: all
	time ./mandelbrot 1500 1500 20 5
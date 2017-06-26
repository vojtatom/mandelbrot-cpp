CC=g++
CFLAGS=-c -Wall -std=c++11

all: mandelbrot

mandelbrot: main.o
	$(CC) main.o -o mandelbrot

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	rm *o mandelbrot

run: all
	./mandelbrot
	gwenview mandel.ppm
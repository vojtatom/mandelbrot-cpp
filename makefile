CC=g++
CFLAGS=-c -Wall

all: mandelbrot

mandelbrot: main.o
	$(CC) main.o -o mandelbrot

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	rm *o mandelbrot
all: program
program: help.o thread.o
	mpicc -lpthread -Wall help.o thread.o mainLoop.c main.c -o program.exe

help.o: help.c
	mpicc -c -Wall help.c -o help.o

thread.o: thread.c
	mpicc -c -Wall -lpthread thread.c -o thread.o

clean:
	rm -f *.o program.exe

# compiler config
CC=g++
CFLAGS=-Wall -g -std=c++11

# compiling
paging:
	$(CC) $(CFLAGS) -o paging paging.cpp

clean:
	rm -f paging paging.o

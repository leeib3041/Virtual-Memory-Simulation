# Hyeop Lee
# CPSC 3220, Section 001
# Dr. Mark Smotherman
# Project 4
# Due: Tuesday, June 18

# compiler config
CC=g++
CFLAGS=-Wall -g -std=c++11

# compiling
paging:
	$(CC) $(CFLAGS) -o paging paging.cpp

clean:
	rm -f paging paging.o

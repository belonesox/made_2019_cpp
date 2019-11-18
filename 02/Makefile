CC=g++  -std=c++14 

all: test

test: run

factor: factor.cpp catch.hpp
	$(CC) -o factor factor.cpp

testalloc: testalloc.cpp catch.hpp Makefile
	$(CC) -g -o testalloc testalloc.cpp

run: testalloc
	./testalloc 



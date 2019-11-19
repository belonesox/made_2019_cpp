CC=g++  -std=c++14 

all: test
test: run

matrix: matrix.cpp catch.hpp Makefile
	$(CC) -g -o matrix matrix.cpp

run: matrix 
	./matrix  



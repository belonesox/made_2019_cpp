CC=g++ 
FLAGS=-std=c++14 
CANONICAL_EXPR="2 + 3 * 4 -2"

all: calc test 
test: calc
	python test.py

calc: calc.cpp bigint.h
	$(CC) -o calc calc.cpp

run: calc
	./calc ${CANONICAL_EXPR}



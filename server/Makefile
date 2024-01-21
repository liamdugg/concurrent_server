CC = gcc
CFLAGS = -g -c -Wall
SRC_FILES = $(filter-out src/test.c, $(wildcard src/*.c))

TARGET	= server

all: build run

run:
	clear
	./server

build:
	clear
	gcc -g -Wall $(SRC_FILES) -o $(TARGET)

test:
	clear
	gcc -g -Wall src/test.c src/HTTPRequest.c -o test
	sudo ./test

testclr:
	rm test

clean:
	rm -rf $(TARGET)
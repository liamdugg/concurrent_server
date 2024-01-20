CC		= gcc
CFLAGS	= -g -c -Wall
SOURCE	= $(wildcard src/*.c)
OBJ		= $(SOURCE:.c=.o)
TARGET	= server

all: build run

run:
	clear
	./server

build:
	clear
	gcc -g -Wall src/server.c src/queue.c src/HTTPRequest.c -o server

test:
	clear
	gcc -g -Wall src/test.c src/HTTPRequest.c -o test
	sudo ./test

testclr:
	rm test

clean:
	rm -rf $(OBJ) $(TARGET)
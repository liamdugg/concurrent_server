CC = gcc
CFLAGS = -g -c -Wall
SRC_FILES = $(filter-out src/test.c, $(wildcard src/*.c))

TARGET = server

all: build run

run:
	clear
	sudo ./server

build:
	clear
	gcc -g -Wall $(SRC_FILES) -o $(TARGET) -lpthread

clean:
	rm -rf $(TARGET)

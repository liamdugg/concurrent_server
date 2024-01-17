CC		= gcc
CFLAGS	= -g -c -Wall
SOURCE	= $(wildcard src/*.c)
OBJ		= $(SOURCE:.c=.o)
TARGET	= server

all: $(SOURCE) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

run:
	./server

clean:
	rm -rf $(OBJ) $(TARGET)
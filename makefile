CC      := gcc
CFLAGS  := -Iinclude -Wall -Wextra -O2 $(shell pkg-config --cflags json-c libcurl raylib)
LDFLAGS := $(shell pkg-config --libs raylib json-c libcurl) -lm

SRC     := main.c categories.c pergunta.c
OBJ     := $(SRC:.c=.o)

.PHONY: all clean

all: gira

gira: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f gira $(OBJ)


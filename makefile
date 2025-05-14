CC      := gcc
CFLAGS  := -Iinclude -Wall -Wextra -O2 $(shell pkg-config --cflags raylib json-c)
LDFLAGS := $(shell pkg-config --libs    raylib json-c)

SRCS    := main.c           # <-- retire ui_raylib.c daqui!
OBJS    := $(SRCS:.c=.o)
TARGET  := gira

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

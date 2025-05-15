CC      := gcc
CFLAGS  := -Iinclude -Wall -Wextra -O2 $(shell pkg-config --cflags json-c libcurl)
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib)
LDFLAGS_GIRA  := $(shell pkg-config --libs raylib json-c)
LDFLAGS_PERG  := $(shell pkg-config --libs json-c libcurl)

SRC_GAME   := main.c
SRC_PERG   := pergunta.c

.PHONY: all clean

all: gerar_pergunta gira

# gera o utilitário que faz a chamada à API e escreve pergunta.json
gerar_pergunta: $(SRC_PERG)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS_PERG)

# gera o jogo, que pressupõe que gerar_pergunta exista e tenha rodado antes
gira: $(SRC_GAME)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) $< -o $@ $(LDFLAGS_GIRA)

clean:
	rm -f gerar_pergunta gira

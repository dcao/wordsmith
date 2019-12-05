LFLAGS         += `pkg-config libhs --cflags --libs`

.DEFAULT: all
.PHONY: all

all: main.c
	$(CC) -o wordsmith main.c $(LFLAGS) -g -Wall -pedantic

LFLAGS         += `pkg-config libhs --cflags --libs`


.DEFAULT: all
.PHONY: all

all: main.c
	$(CC) -o wordsmith main.c $(LFLAGS) -Wall -pedantic

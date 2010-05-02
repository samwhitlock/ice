# Makefile

CFLAGS = -O2 -pg -g -march=core2 -Wall -std=c99
CC = gcc

SOURCES = ice.c pbm.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE=ice

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY : clean
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)


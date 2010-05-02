# Makefile

CFLAGS = -O2 -g -march=core2 -Wall
CC = gcc

CFLAGS := $(CFLAGS) -I. -std=c99

SOURCES = main.c ice.c pbm.c
OBJECTS = $(SOURCES:.c=.o)

CLEAN_FILES = $(OBJECTS) ice

ice: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: all
all: ice

.PHONY: clean
clean:
	rm -f $(CLEAN_FILES)

include tests.mk


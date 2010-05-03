# Makefile

CFLAGS = -O2 -march=core2 -Wall
CC = gcc

EXTRA_CFLAGS = -fopenmp -std=c99 -I.
EXTRA_LDFLAGS = -fopenmp

SOURCES = main.c ice.c pbm.c
OBJECTS = $(SOURCES:.c=.o)

CLEAN_FILES = $(OBJECTS) ice

ice: $(OBJECTS)
	$(CC) $(EXTRA_LDFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.c Makefile
	$(CC) $(EXTRA_CFLAGS) $(CFLAGS) -c $< -o $@

.deps/%.d: %.c $(global_deps)
	@set -e; rm -f $@; mkdir -p $$(dirname $@) ; \
	$(CC) -M $(CPPFLAGS) $(FINAL_CFLAGS) $< > $@.$$$$ 2>/dev/null ; \
	sed 's,'$$(basename $*)'\.o[ :]*,$*.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

DEPS := $(SOURCES:%.c=.deps/%.d)
-include $(DEPS)

.PHONY: all
all: ice

.PHONY: clean
clean:
	rm -f $(CLEAN_FILES)

include tests.mk


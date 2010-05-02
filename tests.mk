# tests.mk

TEST_SOURCES = \
	tests/test_configurations_equal.c

TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TESTS = $(TEST_SOURCES:.c=)

CLEAN_FILES := $(CLEAN_FILES) $(TESTS) $(TEST_OBJECTS)

tests/test_%.o: tests/test_%.c
	$(CC) -c $(CFLAGS) $< -o $@

tests/test_%: $(filter-out main.o,$(OBJECTS)) tests/test_%.o
	$(CC) $(CFLAGS) $+ -o $@

.PHONY: check
check: all $(TESTS)
	tests/run_tests.bash $(TESTS)


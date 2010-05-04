# tests.mk

TEST_SOURCES = \
	tests/test_states_equal.c \
	tests/test_queue.c \
	tests/test_move.c

TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TESTS = $(TEST_SOURCES:.c=)

CLEAN_FILES := $(CLEAN_FILES) $(TESTS) $(TEST_OBJECTS)

tests/test_%: $(filter-out main.o,$(OBJECTS)) tests/test_%.o
	$(CC) $(EXTRA_LDFLAGS) $+ -o $@

.PHONY: check
check: all $(TESTS)
	tests/run_tests.bash $(TESTS)


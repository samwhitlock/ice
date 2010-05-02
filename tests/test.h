/* tests/test_solutions.c
 *
 * Copyright (c) 2010 Michael Forney
 */

int successes = 0;
int failures = 0;

#define RUN_TEST(test)              \
    if (test())                     \
    {                               \
        ++successes;                \
        puts(#test " succeeded");   \
    }                               \
    else                            \
    {                               \
        ++failures;                 \
        puts(#test " failed");      \
    }

#define EXIT_STATUS (failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE)

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


/* tests/test_states_equal.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "test.h"
#include "ice.h"

static bool test_sorted()
{
    struct position first[]     = { { 0, 0 }, { 1, 0 }, { 5, 0 }, { 3, 2 }, { 1, 7 } };
    struct position second[]    = { { 0, 0 }, { 1, 0 }, { 5, 0 }, { 3, 2 }, { 1, 7 } };

    return states_equal(first, second);
}

static bool test_unsorted()
{
    struct position first[]     = { { 0, 0 }, { 1, 0 }, { 5, 0 }, { 3, 2 }, { 1, 7 } };
    struct position second[]    = { { 3, 2 }, { 5, 0 }, { 1, 0 }, { 1, 7 }, { 0, 0 } };

    return states_equal(first, second);
}

static bool test_one_different()
{
    struct position first[]     = { { 0, 0 }, { 1, 0 }, { 5, 0 }, { 3, 2 }, { 1, 7 } };
    struct position second[]    = { { 0, 0 }, { 4, 0 }, { 5, 0 }, { 3, 2 }, { 1, 7 } };

    return !states_equal(first, second);
}

int main(int argc, char * argv[])
{
    state_length = 5;

    RUN_TEST(test_sorted);
    RUN_TEST(test_unsorted);
    RUN_TEST(test_one_different);

    return EXIT_STATUS;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


/* tests/test_states_equal.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "test.h"
#include "ice.h"

static bool test_equal()
{
    uint32_t first[]    = { 0xdead1234, 0xbeef5678 };
    uint32_t second[]   = { 0xdead1234, 0xbeef5678 };

    return states_equal(first, second);
}

static bool test_one_different()
{
    uint32_t first[]    = { 0xdead1234, 0xbeef5678 };
    uint32_t second[]   = { 0xdfad1234, 0xbeef5678 };

    return !states_equal(first, second);
}

int main(int argc, char * argv[])
{
    state_size = 8;

    RUN_TEST(test_equal);
    RUN_TEST(test_one_different);

    return EXIT_STATUS;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


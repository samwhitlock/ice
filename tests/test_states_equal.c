/* tests/test_states_equal.c
 *
 * This file is part of ice.
 *
 * ice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ice.  If not, see <http://www.gnu.org/licenses/>.
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


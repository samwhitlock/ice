/* tests/test.h
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

int successes = 0;
int failures = 0;

#define RUN_TEST(test) process_result(test(), #test)

void process_result(bool result, const char * test_name)
{
    if (result)
    {
        ++successes;
        printf("%s succeeded\n", test_name);
    }
    else
    {
        ++failures;
        printf("%s failed\n", test_name);
    }
}

#define EXIT_STATUS (failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE)

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


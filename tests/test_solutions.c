/* tests/test_solutions.c
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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "ice.h"
#include "pbm.h"
#include "test.h"

struct test_case
{
    const char * pbm;
    bool expected_result;
};

struct test_case test_cases[] = {
    { "simple",     true },
    { "spiral_4",   true },
    { "61c",        true }
};
int test_cases_length = sizeof(test_cases) / sizeof(struct test_case);

static bool validate_solution(uint32_t * state, struct position * end_state,
    struct move * moves, int moves_length)
{
    int move_index;

    for (move_index = 0; move_index < moves_length; ++move_index)
    {
        if (!move(moves[move_index].direction, &moves[move_index].position, state, state))
        {
            return false;
        }
    }

    return states_equal(state, end_state);
}

int main(int argc, char * argv[])
{
    uint32_t * start_state;
    uint32_t * end_state;
    int start_width, start_height, start_ones, end_ones, end_width, end_height;

    int index;

    for (index = 0; index < test_cases_length; ++index)
    {
        char pbm_path[256];
        clock_t start, end;
        double processor_time;

        start = clock();

        /* Read the PBMs */
        sprintf(pbm_path, "tests/pbm/%s_start.pbm", test_cases[index].pbm);
        read_pbm(pbm_path, &start_state, &start_width, &start_height, &start_ones);
        sprintf(pbm_path, "tests/pbm/%s_end.pbm", test_cases[index].pbm);
        read_pbm(pbm_path, &end_state, &end_width, &end_height, &end_ones);

        if (start_ones != end_ones || start_width != end_width || start_height != end_height)
        {
            process_result(!test_cases[index].expected_result, test_cases[index].pbm);
        }
        else
        {
            state_width = start_width;
            state_height = start_height;
            state_ones = start_ones;

            if (find_path(start_state, end_state))
            {
                process_result(validate_solution(start_state, end_state, moves, moves_length) ==
                    test_cases[index].expected_result, test_cases[index].pbm);
            }
            else
            {
                process_result(!test_cases[index].expected_result, test_cases[index].pbm);
            }
        }

        end = clock();

        processor_time = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("\e[1;36m>>\e[0m Running time: %g\n", processor_time);
    }

    return EXIT_STATUS;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


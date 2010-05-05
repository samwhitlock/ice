/* tests/test_solutions.c
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
    { "cs61c",      true }
};
int test_cases_length = sizeof(test_cases) / sizeof(struct test_case);

static bool validate_solution(struct position * configuration, struct position * end_configuration,
    struct move * moves, int moves_length)
{
    int move_index;
    int position_index;
    struct position * position;

    for (move_index = 0; move_index < moves_length; ++move_index)
    {
        for(position_index = 0; position_index < configuration_length; ++position_index)
        {
            position = &configuration[position_index];

            if (position->x == moves[move_index].position.x
                && position->y == moves[move_index].position.y)
            {
                if (move(configuration, moves[move_index].direction, position, position))
                {
                    break;
                }
                else
                {
                    return false;
                }
            }
        }

        if (position_index == configuration_length)
        {
            return false;
        }
    }

    return configurations_equal(configuration, end_configuration);
}

int main(int argc, char * argv[])
{
    struct position * start_configuration;
    struct position * end_configuration;
    int end_configuration_length;
    int start_configuration_length;
    int index;

    for (index = 0; index < test_cases_length; ++index)
    {
        char pbm_path[256];
        clock_t start, end;
        double processor_time;

        start = clock();

        /* Read the PBMs */
        sprintf(pbm_path, "tests/pbm/%s_start.pbm", test_cases[index].pbm);
        read_pbm(pbm_path, &start_configuration, &start_configuration_length);
        sprintf(pbm_path, "tests/pbm/%s_end.pbm", test_cases[index].pbm);
        read_pbm(pbm_path, &end_configuration, &end_configuration_length);

        if (start_configuration_length != end_configuration_length)
        {
            process_result(!test_cases[index].expected_result, test_cases[index].pbm);
        }
        else
        {
            configuration_length = start_configuration_length;

            initialize_past_configurations();

            if (find_path(start_configuration, end_configuration, 0))
            {
                process_result(validate_solution(start_configuration, end_configuration, moves, moves_length) ==
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


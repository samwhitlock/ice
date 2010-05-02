/* tests/test_solutions.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include <stdbool.h>

#include "ice.h"

struct test_case
{
	char* init_config_path;
	char* end_config_path;
};

struct test_case test_cases[] = {
	{ "pbm/simple_start.pbm", "pbm/simple_end.pbm" }
};

const char test_cases_length = 1;

static bool validate_solution(struct position * configuration, struct move * moves, int num_moves, struct position * end_configuration)
{
	struct position * position;
	for( int i = 0, j; i < num_moves; ++i )
	{
		for( j=0; j < configuration_length; ++j )
		{
			position = configuration[j];
			
			if( position->x == moves[i].position.x && position->y == moves[i].position.y )
			{
				if( move(configuration, moves[i].direction, position, position) )
					continue;
				else
					return false;
			}
		}
		
		if ( j == configuration_length )
			return false;
	}
	
	return configurations_equal(configuration, end_configuration);
}

bool test_solutions(char* init_file_path, char* end_file_path)//do file IO stuff
{
	struct position * start_configuration;
    struct position * end_configuration;
    int end_configuration_length;
    int start_configuration_length;
	
    /* Read the PBMs */
    read_pbm(init_file_path, &start_configuration, &start_configuration_length);
    read_pbm(end_file_path, &end_configuration, &end_configuration_length);
	
    if (start_configuration_length != end_configuration_length)
    {
        return FALSE;
    }
	
    configuration_length = start_configuration_length;
	
	initialize_past_configurations();
	
    if(find_path(start_configuration, end_configuration, 0) && validate_solution(start_configuration, moves, moves_length, end_configuration))
    {
		finalize_past_configurations();
		return TRUE;
    }
    else
    {
		finalize_past_configurations();
		return FALSE;
    }
}

int main(int argc, char * argv[])
{
	for( int i = 0; i < test_cases_length; ++i )
	{
		printf("Test %d: %s\n", i, test_solution(test_cases[i].init_config_path, test_cases[i].end_config_path) ? "SUCCESS" : "FAILURE");
	}
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


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

int main(int argc, char * argv[])
{
	struct position * start_configuration;
    struct position * end_configuration;
    int end_configuration_length;
    int start_configuration_length;
	
    if (argc != 3)
    {
        printf(help_text, argv[0]);
        return 1;
    }
	
    /* Read the PBMs */
    read_pbm(argv[1], &start_configuration, &start_configuration_length);
    read_pbm(argv[2], &end_configuration, &end_configuration_length);
	
    if (start_configuration_length != end_configuration_length)
    {
        puts("IMPOSSIBLE");
        return 0;
    }
	
    configuration_length = start_configuration_length;
	
    if(find_path(start_configuration, end_configuration, 0) && validate_solution(start_configuration, <#struct move *moves#>, <#int num_moves#>, <#struct position *end_configuration#>))
    {
		
    }
    else
    {
        puts("IMPOSSIBLE");
    }
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


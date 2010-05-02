/* tests/test_solutions.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include <stdbool.h>

#include "ice.h"

static bool validate solution(struct position * configuration, struct move * moves, int num_moves, struct position * end_configuration)
{
	struct position * position;
	for( int i = 0, j; i < num_moves; ++i )
	{
		for( j=0; j < configuration_length; ++j )
		{
			position = configuration[j];
			
			if( *position == moves[i].position )//maybe FIXME; is this comparison accurate?
			{
				if( move(configuration, moves[i].direction, position, position) )
					continue;
				else
					return false;
			}
		}
		
		if ( j== configuration_length )
			return false;
	}
	
	return configurations_equal(configuration, end_configuration);
}

int main(int argc, char * argv[])
{
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


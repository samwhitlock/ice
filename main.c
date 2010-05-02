/* main.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include <stdio.h>

#include "ice.h"
#include "pbm.h"

const char help_text[] =
    "Usage: %s start.pbm end.pbm\n"
    "\tThis program will output a series of valid moves to get to the end\n"
    "\tconfiguration from the start configuration. If this is impossible, the\n"
    "\tprogram will simply output \"IMPOSSIBLE\".\n";

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

    if(find_path(start_configuration, end_configuration, 0))
    {
        int index;

        for (index = 0; index < moves_length; ++index)
        {
            printf("%u %u %c\n", moves[index].position.x, moves[index].position.y,
                direction_char[moves[index].direction]);
        }
    }
    else
    {
        puts("IMPOSSIBLE");
    }
}


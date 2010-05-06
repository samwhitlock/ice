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
    uint32_t * start_state;
    uint32_t * end_state;
    int start_width, start_height, start_ones, end_ones, end_width, end_height;

    if (argc != 3)
    {
        printf(help_text, argv[0]);
        return 1;
    }

    /* Read the PBMs */
    read_pbm(argv[1], &start_state, &start_width, &start_height, &start_ones);
    read_pbm(argv[2], &end_state, &end_width, &end_height, &end_ones);

    if (start_ones != end_ones || start_width != end_width || start_height != end_height)
    {
        puts("IMPOSSIBLE");
        return 0;
    }

    state_width = start_width;
    state_height = start_height;
    state_ones = start_ones;

    if(find_path(start_state, end_state))
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

    free(start_state);
    free(end_state);

    return 0;
}


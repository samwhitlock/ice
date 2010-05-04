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
    struct position * start_state;
    struct position * end_state;
    int start_state_length;
    int end_state_length;

    if (argc != 3)
    {
        printf(help_text, argv[0]);
        return 1;
    }

    /* Read the PBMs */
    read_pbm(argv[1], &start_state, &start_state_length);
    read_pbm(argv[2], &end_state, &end_state_length);

    if (start_state_length != end_state_length)
    {
        puts("IMPOSSIBLE");
        return 0;
    }

    state_length = start_state_length;

    if(find_path(start_state, end_state))
    {
    }
    else
    {
        puts("IMPOSSIBLE");
    }
}


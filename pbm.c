/* pbm.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include <stdio.h>
#include <stdlib.h>

#include "pbm.h"

extern int state_width, state_height;

void read_pbm(const char const * filename, struct position ** configuration, int * length)
{
    FILE * file;
    int x, y, index;

    file = fopen(filename, "r");

    fscanf(file, "P1 %d %d\n", &state_width, &state_height);

    /* There are at most width * height set bits */
    *configuration = malloc(state_width * state_height * sizeof(struct position));
    index = 0;

    for (y = 0; y < state_height; ++y)
    {
        for (x = 0; x < state_width; ++x)
        {
            if (getc(file) == '1')
            {
                (*configuration)[index++] = (struct position) { x, y };
            }

            getc(file);
        }
    }

    fclose(file);

    *length = index;
    *configuration = realloc(*configuration, index * sizeof(struct position));
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


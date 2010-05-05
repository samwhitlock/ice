/* pbm.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include <stdio.h>
#include <stdlib.h>

#include "pbm.h"

void read_pbm(const char const * filename, uint32_t ** state, int * width, int * height)
{
    FILE * file;
    int x, y;

    int ints_per_row;
    int ints_per_state;

    file = fopen(filename, "r");

    fscanf(file, "P1 %d %d\n", width, height);

    ints_per_row = (*width) / 32 + (*width) % 32 == 0 ? 0 : 1;
    ints_per_state = ints_per_row * (*height);

    *state = calloc(ints_per_state, 4);

    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            if (getc(file) == '1')
            {
                (*state)[y * ints_per_row + x / 32] &= 1 << (x % 32);
            }

            getc(file);
        }
    }

    fclose(file);
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


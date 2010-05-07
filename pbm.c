/* pbm.c
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2010 Sam Whitlock
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "pbm.h"

static inline int bit_index(int x, int y, int width)
{
    return (y * width + x) % 32;
}

static inline int bitset_index(int x, int y, int width)
{
    return (y * width + x) / 32;
}

#define read_pbm_prefetch_locality 3
void read_pbm(const char const * filename, uint32_t ** state, int * width, int * height, int * ones)
{
    FILE * file;
    int x, y, w, h;


    file = fopen(filename, "r");

    if(ones != NULL)
    {
        *ones = 0;
    }

    fscanf(file, "P1 %d %d\n", &w, &h);
    assert(file);

    *width = w;
    *height = h;

    int ints_per_state = (w * h / 32) + ((w * h % 32) == 0 ? 0 : 1);

    *state = calloc(ints_per_state, 4);

    for (y = 0; y < h; ++y)
    {
        __builtin_prefetch(state+bitset_index(x,y,w), 1, read_pbm_prefetch_locality);
        for (x = 0; x < w; ++x)
        {
            __builtin_prefetch(state+bitset_index(x+1,y,w), 1, read_pbm_prefetch_locality);
            if (getc(file) == '1')
            {
                (*state)[bitset_index(x, y, w)] |= 1 << bit_index(x, y, w);

                if(ones != NULL)
                {
                    ++(*ones);
                }
            }

            getc(file);
        }
    }

    fclose(file);
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


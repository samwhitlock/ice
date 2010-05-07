/* pbm.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include <stdio.h>
#include <stdlib.h>

#include "pbm.h"

static inline uint32_t mask(int x, int y, int width)
{
    return 1 << (x+(y*width))%32;
}

static inline int offset(int x, int y, int width)
{
    return ((y*width)+x)/32;
}

void read_pbm(const char const * filename, uint32_t ** state, int * width, int * height, int * ones)
{
    FILE * file;
    int x, y, w, h;


    file = fopen(filename, "r");
    if(ones != NULL)
        *ones = 0;

    fscanf(file, "P1 %d %d\n", width, height);

    w = *width;
    h = *height;
    
    int ints_per_state = ((w*h)/32) + ((w*h)%32) > 0 ? 1 : 0;
    
    printf("Ints/state read in: %d\n", ints_per_state);
    
    *state = calloc(ints_per_state, 4);

    for (y = 0; y < *height; ++y)
    {
        for (x = 0; x < *width; ++x)
        {
            if (getc(file) == '1')
            {
                 (*state)[offset(x,y,w)] |= mask(x,y,w);
                if(ones != NULL)
                    ++(*ones);
            }

            getc(file);
        }
    }

    fclose(file);
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


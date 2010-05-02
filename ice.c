/* ice.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ice.h"
#include "pbm.h"

char direction_char[] = {
    [NORTH] = 'N',
    [SOUTH] = 'S',
    [EAST] = 'E',
    [WEST] = 'W'
};

/* The number of positions that compose a configuration of the bits */
int configuration_length;

struct position * past_configurations = NULL;
int past_configurations_length = 0;
int past_configurations_capacity = 0;

static void __attribute__((constructor)) initialize_past_configurations()
{
    /* Initialize past configuration array */
    past_configurations_capacity = 256;
    past_configurations = malloc(past_configurations_capacity *
        configuration_length * sizeof(struct position));
}

static void __attribute__((destructor)) finalize_past_configurations()
{
    free(past_configurations);
}

bool move(const struct position * configuration, enum direction direction,
    const struct position * position, struct position * next_position)
{
    const struct position * other_position;
    const struct position * blocking_position = NULL;

    /* If any of the other bits have a {higher,lower} {x,y} coordinate, this
     * move is valid */
    for (other_position = configuration; other_position < configuration + configuration_length; ++other_position)
    {
        /* Skip over the bit we are currently trying to move */
        if (other_position == position) continue;

        if (direction == NORTH)
        {
            if (position->x == other_position->x && position->y > other_position->y + 1 &&
                (!blocking_position || blocking_position->y < other_position->y))
            {
                blocking_position = other_position;
            }
        }
        else if (direction == SOUTH)
        {
            if (position->x == other_position->x && position->y < other_position->y - 1 &&
                (!blocking_position || blocking_position->y > other_position->y))
            {
                blocking_position = other_position;
            }
        }
        else if (direction == WEST)
        {
            if (position->y == other_position->y && position->x > other_position->x + 1 &&
                (!blocking_position || blocking_position->x < other_position->x))
            {
                blocking_position = other_position;
            }
        }
        else
        {
            if (position->y == other_position->y && position->x < other_position->x - 1 &&
                (!blocking_position || blocking_position->x > other_position->x))
            {
                blocking_position = other_position;
            }
        }
    }

    if (blocking_position)
    {
        *next_position = *blocking_position;

        if (direction == NORTH)         ++next_position->y;
        else if (direction == SOUTH)    --next_position->y;
        else if (direction == WEST)     ++next_position->x;
        else                            --next_position->x;

        return true;
    }
    else
    {
        return false;
    }
}

bool configurations_equal(struct position * first_configuration, struct position * second_configuration)
{
    struct position * first;
    struct position * second;
    bool found;

    for (first = first_configuration; first < first_configuration + configuration_length; ++first)
    {
        found = false;

        for (second = second_configuration; second < second_configuration + configuration_length; ++second)
        {
            if (first->x == second->x && first->y == second->y)
            {
                found = true;
            }
        }

        if (!found)
        {
            return false;
        }
    }

    return true;
}

inline struct position * past_configuration(int index)
{
    return &past_configurations[index * past_configurations_length];
}

bool is_past_configuration(struct position * configuration)
{
    int index;

    for (index = 0; index < past_configurations_length; ++index)
    {
        if (configurations_equal(configuration, past_configuration(index)))
        {
            return true;
        }
    }

    return false;
}

void add_past_configuration(struct position * configuration)
{
    past_configurations_length += configuration_length;

    while (past_configurations_length > past_configurations_capacity)
    {
        past_configurations_capacity *= 2;
        past_configurations = realloc(past_configurations,
            past_configurations_capacity * configuration_length * sizeof(struct position));
    }

    memcpy(past_configuration(past_configurations_length - 1), configuration,
        configuration_length * sizeof(struct position));
}

bool find_path(struct position * configuration, struct position * end_configuration,
    struct move_tree * parent_move, int depth)
{
    if (configurations_equal(configuration, end_configuration))
    {
        /* That's a BINGO! */
        puts("found solution");
        return true;
    }
    else
    {
        int index = 0;
        enum direction direction;
        struct position next_configuration[configuration_length];

        add_past_configuration(configuration);

        for (index = 0; index < configuration_length; ++index)
        {
            memcpy(next_configuration, configuration, configuration_length * sizeof(struct position));

            for (direction = NORTH; direction <= WEST; ++direction)
            {
                if(move(configuration, direction, &configuration[index], &next_configuration[index]))
                {
                    for (int i = 0; i < depth; ++i) putchar(' ');
                    printf("%u %u %c - ", configuration[index].x, configuration[index].y,
                        direction_char[direction]);

                    if (!is_past_configuration(next_configuration))
                    {
                        puts("okay");

                        if (find_path(next_configuration, end_configuration, NULL, depth + 1))
                        {
                            return true;
                        }
                    }
                    else
                    {
                        puts("already been here");
                    }
                }
            }
        }

        for (int i = 0; i < depth; ++i) putchar(' ');
        puts("dead end");
        return false;
    }
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


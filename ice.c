/* ice.c
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2010 Sam Whitlock
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>

#include "ice.h"

char direction_char[] = {
    [NORTH] = 'N',
    [SOUTH] = 'S',
    [EAST] = 'E',
    [WEST] = 'W'
};

/* The number of positions that compose a configuration of the bits */
int configuration_length;

struct position * past_configurations = NULL;
struct position * next_configurations = NULL;
struct position * current_configurations = NULL;

int past_configurations_length = 0;
int past_configurations_capacity = 0;
int next_configurations_length = 0;


static void initialize_past_configurations()
{
    /* Initialize past configuration array */
    past_configurations_length = 0;
    past_configurations_capacity = 256;
    past_configurations = malloc(past_configurations_capacity *
        configuration_length * sizeof(struct position));
}

static void finalize_past_configurations()
{
    free(past_configurations);
}

bool move(enum direction direction, int move_index,
    const struct position * current, struct position * next)
{
    int index;
    const struct position * other_position;
    const struct position * current_position;
    const struct position * blocking_position = NULL;

    current_position = &current[move_index];

    /* If any of the other bits have a {higher,lower} {x,y} coordinate, this
     * move is valid */
    for (index = 0; index < configuration_length; ++index)
    {
        /* Skip over the bit we are currently trying to move */
        if (index == move_index) continue;

        other_position = &current[index];

        if ((direction == NORTH &&
                current_position->x == other_position->x && current_position->y > other_position->y + 1 &&
                (!blocking_position || blocking_position->y < current_position->y)) ||
            (direction == SOUTH &&
                current_position->x == other_position->x && current_position->y < other_position->y - 1 &&
                (!blocking_position || blocking_position->y > current_position->y)) ||
            (direction == WEST &&
                current_position->y == other_position->y && current_position->x > other_position->x + 1 &&
                (!blocking_position || blocking_position->x < current_position->x)) ||
            (direction == EAST &&
                current_position->y == other_position->y && current_position->x < other_position->x - 1 &&
                (!blocking_position || blocking_position->x > current_position->x)))
        {
            blocking_position = other_position;
        }
    }

    if (blocking_position)
    {
        next[move_index] = *blocking_position;

        if (direction == NORTH)         ++next[move_index].y;
        else if (direction == SOUTH)    --next[move_index].y;
        else if (direction == WEST)     ++next[move_index].x;
        else                            --next[move_index].x;

        return true;
    }

    return false;
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

static inline struct position * past_configuration(int index)
{
    return &past_configurations[index * configuration_length];
}

bool is_past_configuration(struct position * current)
{
    struct position * past;

    for (past = past_configurations;
        past < past_configurations + past_configurations_length;
        past += configuration_length)
    {
        if (configurations_equal(current, past))
        {
            return true;
        }
    }

    return false;
}

void add_past_configuration(struct position * configuration)
{
    #pragma omp critical//TODO: Make me thread safe!
    {
        ++past_configurations_length;

        if (past_configurations_length > past_configurations_capacity)
        {
            past_configurations_capacity *= 2;

            past_configurations = realloc(past_configurations,
                past_configurations_capacity * configuration_length * sizeof(struct position));
        }

        memcpy(past_configuration(past_configurations_length - 1), configuration,
            configuration_length * sizeof(struct position));
    }
}

void print_configuration(struct position * configuration)
{
    int index = 0;

    printf("{ ");
    for (index = 0; index < configuration_length; ++index)
    {
        printf("(%u, %u) ", configuration[index].x, configuration[index].y);
    }
    printf("}\n");
}

bool find_path(struct position * start, struct position * end)
{
    struct position * current;
    struct position * position;
    struct position next[configuration_length];
    enum direction direction;
    bool found = false;

    initialize_past_configurations();

    add_past_configuration(start);

    current_configurations = past_configurations;
    next_configurations = past_configurations + configuration_length;

    while (true)
    {
        next_configurations_length = 0;

        for (current = current_configurations;
            current < next_configurations;
            current += configuration_length)
        {
            print_configuration(current);
            for (position = current; position < current + configuration_length; ++position)
            {
                memcpy(next, current, configuration_length * sizeof(struct position));

                for (direction = NORTH; direction <= WEST; ++direction)
                {
                    printf("%u %u %c - ", position->x, position->y, direction_char[direction]);

                    if (move(direction, position - current, current, next))
                    {
                        if (!is_past_configuration(next))
                        {
                            printf("okay, next: ");
                            print_configuration(next);

                            if (configurations_equal(next, end))
                            {
                                puts("found solution");
                                found = true;
                                break;
                            }

                            ++next_configurations_length;
                            add_past_configuration(next);
                        }
                        else
                        {
                            puts("already been here");
                        }
                    }
                    else
                    {
                        puts("invalid move");
                    }
                }

                if (found) break;
            }

            if (found) break;
            puts("---");
        }

        if (found) break;
        puts("===");

        if (next_configurations_length == 0)
        {
            break;
        }

        current_configurations = next_configurations;
        next_configurations += configuration_length * next_configurations_length;
    }

    finalize_past_configurations();

    return found;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


/* ice.h
 *
 * Copyright (c) 2010 Michael Forney
 */

#ifndef ICE_H
#define ICE_H 1

#include <stdbool.h>

/* Type definitions */
enum direction
{
    NORTH,
    SOUTH,
    EAST,
    WEST
};

struct position
{
    int x;
    int y;
};

struct move
{
    struct position position;
    enum direction direction;
};

extern int configuration_length;

bool find_path(struct position * configuration, struct position * end_configuration,
    struct move_tree * parent_move, int depth);

bool configurations_equal(struct position * first_configuration, struct position * second_configuration);

#endif

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


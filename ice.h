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
    int index;
    enum direction direction;
};

struct move_tree
{
    const struct move_tree * parent;
    struct move move;
};

extern char direction_char[];

extern int configuration_length;

extern int moves_length;
extern struct move * moves;

bool find_path(struct position * start, struct position * end);

bool configurations_equal(struct position * first, struct position * second);

bool move(enum direction direction, int move_index,
    const struct position * current, struct position * next);

#endif

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


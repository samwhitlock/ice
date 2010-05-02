/* ice.h
 *
 * Copyright (c) 2010 Michael Forney
 */

#ifndef ICE_H
#define ICE_H 1

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

struct move_tree
{
    struct position position;
    enum direction direction;

    struct move_tree * parent;
};

#endif

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


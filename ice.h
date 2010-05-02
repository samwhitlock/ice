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

extern char direction_char[];

extern int configuration_length;

extern int moves_length;
extern struct move * moves;

void initialize_past_configurations();
void finalize_past_configurations();

bool find_path(struct position * configuration, struct position * end_configuration, int depth);

bool configurations_equal(struct position * first_configuration, struct position * second_configuration);

bool move(const struct position * configuration, enum direction direction, const struct position * position, struct position * next_position);

#endif

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


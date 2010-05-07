/* ice.h
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2010 Sam Whitlock
 */

#ifndef ICE_H
#define ICE_H 1

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

/********** Type Definitions **********/

/**
 * A direction is the possible directions a bit can move.
 */
enum direction
{
    NORTH,
    SOUTH,
    EAST,
    WEST
};

/**
 * A position is the coordinates on the board of a bit.
 */
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

struct move_index
{
    unsigned short hash;
    int index;
};

struct move_tree
{
    struct move move;
    struct move_index parent;
    int depth;
    uint32_t state[];
};

/********** Variable Declarations **********/
extern char direction_char[];
extern int state_height, state_width, state_ones, ints_per_state;
extern size_t state_size;
extern struct move * moves;
extern int moves_length;

/********** Function Declarations **********/

/**
 * Attempts to find a series of moves to get from start to end.
 *
 * @return True if a solution exists, false otherwise.
 */
bool find_path(const uint32_t * start, const uint32_t * end);

/**
 * Compares two states.
 *
 * @return True if the states are equal, false otherwise.
 */
bool states_equal(const uint32_t * first, const uint32_t * second);

/**
 * Calculates the score for a set of positions.
 *
 * A high score indicates that this set is far off from the correct answer, a
 * low score indicates that that it is close, with a score of 0 indicating that
 * this is the correct solution.
 *
 * @return The calculated score for a set of positions.
 */
unsigned int calculate_score(const uint32_t * first_state, const uint32_t * second_state);

bool move(enum direction direction, const struct position * position,
    const struct move_index move_index, uint32_t * next_state);

void print_state(const uint32_t * state);

#endif

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


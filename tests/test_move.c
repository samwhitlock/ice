/* tests/test_move.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include <stdio.h>

#include "ice.h"
#include "test.h"

/* Directions:
 *
 *   N
 * W-|-E
 *   S
 */

/********** NORTH {{{ **********/
static bool test_invalid_edge_north()
{
    /* State:
     *
     * |LSB       MSB|
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 1 0 1 0 1 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   North
     * Expected result:     Fail (slide off edge)
     */

    const uint32_t state[] = {
        0x00,
        0x00,
        0x00,
        0x2a,
        0x00,
        0x08,
        0x00,
        0x00
    };

    uint32_t next_state[ints_per_state];

    struct position position = { 3, 3 };

    return !move(NORTH, &position, state, next_state);
}

static bool test_invalid_block_north()
{
    /* State:
     *
     * |LSB       MSB|
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 1 0 1 0 1 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   North
     * Expected result:     Fail (Blocked by (3, 2))
     */

    const uint32_t state[] = {
        0x00,
        0x00,
        0x08,
        0x2a,
        0x00,
        0x08,
        0x00,
        0x00
    };

    uint32_t next_state[ints_per_state];

    struct position position = { 3, 3 };

    return !move(NORTH, &position, state, next_state);
}

static bool test_move_north()
{
    /* State:
     *
     * |LSB       MSB|      |LSB       MSB|
     * 0 0 0 1 0 0 0 0      0 0 0 1 0 0 0 0
     * 0 1 0 0 0 1 0 0      0 1 0 1 0 1 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 1 0 1 0 1 0 0      0 1 0 0 0 1 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 1 0 0 0 1 0 0      0 1 0 0 0 1 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   North
     * Expected result:     Move to (3, 1)
     */

    const uint32_t state[] = {
        0x08,
        0x22,
        0x00,
        0x2a,
        0x00,
        0x22,
        0x00,
        0x00
    };

    uint32_t next_state[ints_per_state];

    const uint32_t expected_next_state[] = {
        0x08,
        0x2a,
        0x00,
        0x22,
        0x00,
        0x22,
        0x00,
        0x00
    };

    struct position position = { 3, 3 };

    if (!move(NORTH, &position, state, next_state))
    {
        return false;
    }

    return states_equal(expected_next_state, next_state);
}
/********** NORTH }}} **********/

/********** SOUTH {{{ **********/
static bool test_invalid_edge_south()
{
    /* State:
     *
     * |LSB       MSB|
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 1 0 1 0 1 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   South
     * Expected result:     Fail (slide off edge)
     */

    const uint32_t state[] = {
        0x00,
        0x08,
        0x00,
        0x2a,
        0x00,
        0x00,
        0x00,
        0x00
    };

    uint32_t next_state[ints_per_state];

    struct position position = { 3, 3 };

    return !move(SOUTH, &position, state, next_state);
}

static bool test_invalid_block_south()
{
    /* State:
     *
     * |LSB       MSB|
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 1 0 1 0 1 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   South
     * Expected result:     Fail (Blocked by (3, 4))
     */

    const uint32_t state[] = {
        0x00,
        0x08,
        0x00,
        0x2a,
        0x08,
        0x00,
        0x00,
        0x00
    };

    uint32_t next_state[ints_per_state];

    struct position position = { 3, 3 };

    return !move(SOUTH, &position, state, next_state);
}

static bool test_move_south()
{
    /* State:
     *
     * |LSB       MSB|      |LSB       MSB|
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 1 0 0 0 1 0 0      0 1 0 0 0 1 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 1 0 1 0 1 0 0      0 1 0 0 0 1 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 1 0 0 0 1 0 0      0 1 0 1 0 1 0 0
     * 0 0 0 1 0 0 0 0      0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   South
     * Expected result:     Move to (3, 5)
     */

    const uint32_t state[] = {
        0x00,
        0x22,
        0x00,
        0x2a,
        0x00,
        0x22,
        0x08,
        0x00
    };

    uint32_t next_state[ints_per_state];

    const uint32_t expected_next_state[] = {
        0x00,
        0x22,
        0x00,
        0x22,
        0x00,
        0x2a,
        0x08,
        0x00
    };

    struct position position = { 3, 3 };

    if (!move(SOUTH, &position, state, next_state))
    {
        return false;
    }

    return states_equal(expected_next_state, next_state);
}
/********** SOUTH }}} **********/

/********** EAST {{{ **********/
static bool test_invalid_edge_east()
{
    /* State:
     *
     * |LSB       MSB|
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 1 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   East
     * Expected result:     Fail (slide off edge)
     */

    const uint32_t state[] = {
        0x00,
        0x08,
        0x00,
        0x0a,
        0x00,
        0x08,
        0x00,
        0x00
    };

    uint32_t next_state[ints_per_state];

    struct position position = { 3, 3 };

    return !move(EAST, &position, state, next_state);
}

static bool test_invalid_block_east()
{
    /* State:
     *
     * |LSB       MSB|
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 1 0 1 1 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   East
     * Expected result:     Fail (Blocked by (4, 3))
     */

    const uint32_t state[] = {
        0x00,
        0x08,
        0x00,
        0x1a,
        0x00,
        0x08,
        0x00,
        0x00
    };

    uint32_t next_state[ints_per_state];

    struct position position = { 3, 3 };

    return !move(EAST, &position, state, next_state);
}

static bool test_move_east()
{
    /* State:
     *
     * |LSB       MSB|      |LSB       MSB|
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 1 0 1 0 1 0 0      0 1 0 1 0 1 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 1 0 1 0 0 1 0      0 1 0 0 0 1 1 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 1 0 1 0 1 0 0      0 1 0 1 0 1 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   East
     * Expected result:     Move to (3, 1)
     */

    const uint32_t state[] = {
        0x00,
        0x2a,
        0x00,
        0x4a,
        0x00,
        0x2a,
        0x00,
        0x00
    };

    uint32_t next_state[ints_per_state];

    const uint32_t expected_next_state[] = {
        0x00,
        0x2a,
        0x00,
        0x62,
        0x00,
        0x2a,
        0x00,
        0x00
    };

    struct position position = { 3, 3 };

    if (!move(EAST, &position, state, next_state))
    {
        return false;
    }

    return states_equal(expected_next_state, next_state);
}
/********** EAST }}} **********/

/********** WEST {{{ **********/
static bool test_invalid_edge_west()
{
    /* State:
     *
     * |LSB       MSB|
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 1 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   West
     * Expected result:     Fail (slide off edge)
     */

    const uint32_t state[] = {
        0x00,
        0x08,
        0x00,
        0x28,
        0x00,
        0x08,
        0x00,
        0x00
    };

    uint32_t next_state[ints_per_state];

    struct position position = { 3, 3 };

    return !move(WEST, &position, state, next_state);
}

static bool test_invalid_block_west()
{
    /* State:
     *
     * |LSB       MSB|
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 1 1 0 1 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 1 0 0 0 0
     * 0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   West
     * Expected result:     Fail (Blocked by (2, 3))
     */

    const uint32_t state[] = {
        0x00,
        0x08,
        0x00,
        0x2c,
        0x00,
        0x08,
        0x00,
        0x00
    };

    struct position position = { 3, 3 };

    uint32_t next_state[ints_per_state];

    return !move(WEST, &position, state, next_state);
}

static bool test_move_west()
{
    /* State:
     *
     * |LSB       MSB|      |LSB       MSB|
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 1 0 1 0 1 0 0      0 1 0 1 0 1 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 1 0 0 1 0 1 0 0      1 1 0 0 0 1 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 1 0 1 0 1 0 0      0 1 0 1 0 1 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     * 0 0 0 0 0 0 0 0      0 0 0 0 0 0 0 0
     *
     * Bit to move:         (3, 3)
     * Direction to move:   West
     * Expected result:     Move to (1, 3)
     */

    const uint32_t state[] = {
        0x00,
        0x2a,
        0x00,
        0x29,
        0x00,
        0x2a,
        0x00,
        0x00
    };

    uint32_t next_state[ints_per_state];

    const uint32_t expected_next_state[] = {
        0x00,
        0x2a,
        0x00,
        0x23,
        0x00,
        0x2a,
        0x00,
        0x00
    };

    struct position position = { 3, 3 };

    if (!move(WEST, &position, state, next_state))
    {
        return false;
    }

    return states_equal(expected_next_state, next_state);
}
/********** WEST }}} **********/

int main(int argc, char * argv[])
{
    state_height = 8;
    state_width = 8;
    ints_per_row = 1;
    ints_per_state = 8;
    state_size = 64;

    /* North */
    RUN_TEST(test_invalid_edge_north);
    RUN_TEST(test_invalid_block_north);
    RUN_TEST(test_move_north);

    /* South */
    RUN_TEST(test_invalid_edge_south);
    RUN_TEST(test_invalid_block_south);
    RUN_TEST(test_move_south);

    /* East */
    RUN_TEST(test_invalid_edge_east);
    RUN_TEST(test_invalid_block_east);
    RUN_TEST(test_move_east);

    /* West */
    RUN_TEST(test_invalid_edge_west);
    RUN_TEST(test_invalid_block_west);
    RUN_TEST(test_move_west);

    return EXIT_STATUS;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=marker


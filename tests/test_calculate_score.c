/* tests/test_calculate_score.c
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2010 Sam Whitlock
 */

#include "ice.h"
#include "test.h"

bool test_same()
{
    /* State:
     *
     * |LSB       MSB|
     * 0 1 0 1 0 1 0 1
     * 1 0 1 0 1 0 1 0
     * 0 1 0 1 0 1 0 1
     * 1 0 1 0 1 0 1 0
     * 0 1 0 1 0 1 0 1
     * 1 0 1 0 1 0 1 0
     * 0 1 0 1 0 1 0 1
     * 1 0 1 0 1 0 1 0
     */

    const uint32_t state[] = {
        0xaa55aa55,
        0xaa55aa55
    };

    return calculate_score(state, state) == 0;
}

bool test_different()
{
    /* State:
     *
     * |LSB       MSB|      |LSB       MSB|
     * 0 1 0 1 0 1 0 1      1 0 1 0 1 0 1 0
     * 1 0 1 0 1 0 1 0      0 1 0 1 0 1 0 1
     * 0 1 0 1 0 1 0 1      1 0 1 0 1 0 1 0
     * 1 0 1 0 1 0 1 0      0 1 0 1 0 1 0 1
     * 0 1 0 1 0 1 0 1      1 0 1 0 1 0 1 0
     * 1 0 1 0 1 0 1 0      0 1 0 1 0 1 0 1
     * 0 1 0 1 0 1 0 1      1 0 1 0 1 0 1 0
     * 1 0 1 0 1 0 1 0      0 1 0 1 0 1 0 1
     */

    const uint32_t state[] = {
        0xaa55aa55,
        0xaa55aa55
    };

    const uint32_t end_state[] = {
        0x55aa55aa,
        0x55aa55aa
    };

    return calculate_score(state, end_state) == 64;
}

int main(int argc, char * argv[])
{
    state_height = 8;
    state_width = 8;
    ints_per_state = 2;
    state_size = 8;

    RUN_TEST(test_same);
    RUN_TEST(test_different);
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


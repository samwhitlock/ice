/* tests/test_read_pbm.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include "ice.h"
#include "pbm.h"
#include "test.h"

bool test_61c()
{
    /* State:
     *
     * |LSB                         MSB|
     * 1 1 1 0 1 1 1 0 1 0 0 0 1 0 1 1 1
     * 1 0 0 0 1 0 0 0 1 0 0 0 1 0 1 0 0
     * 1 0 0 0 1 1 1 0 1 1 1 0 1 0 1 0 0
     * 1 0 0 0 0 0 1 0 1 0 1 0 1 0 1 0 0
     * 1 1 1 0 1 1 1 0 1 1 1 0 1 0 1 1 1
     */

    uint32_t * state;

    const uint32_t expected_state[] = {
        0x1d177,
        0x05111,
        0x05771,
        0x05541,
        0x1d777
    };

    read_pbm("tests/pbm/61c_end.pbm", &state, &state_width, &state_height, NULL);

    state_size = state_height * 4;

    if (state_width != 17 && state_height != 5)
    {
        printf("width: %u, expected_width: %u\n", state_width, 17);
        printf("height: %u, expected_height: %u\n", state_height, 5);

        return false;
    }
    else if (!states_equal(state, expected_state))
    {
        puts("state read:");
        print_state(state);

        puts("state expected:");
        print_state(expected_state);

        return false;
    }

    return true;
}

bool test_spiral_8()
{
    /* State:
     *
     * |LSB       MSB|
     * 1 1 1 1 1 1 1 1
     * 1 0 0 0 0 0 0 1
     * 1 1 1 1 1 1 0 1
     * 1 0 0 0 0 1 0 1
     * 1 0 1 1 0 1 0 1
     * 1 0 1 0 0 1 0 1
     * 1 0 1 1 1 1 0 1
     * 1 0 0 0 0 0 0 1
     * 1 1 1 1 1 1 1 1
     */

    uint32_t * state;

    const uint32_t expected_state[] = {
        0xff,
        0x81,
        0xbf,
        0xa1,
        0xad,
        0xa5,
        0xbd,
        0x81,
        0xff
    };

    read_pbm("tests/pbm/spiral_8_start.pbm", &state, &state_width, &state_height, NULL);

    state_size = state_height * 4;

    if (state_width != 8 && state_height != 9)
    {
        printf("width: %u, expected_width: %u\n", state_width, 8);
        printf("height: %u, expected_height: %u\n", state_height, 9);

        return false;
    }
    else if (!states_equal(state, expected_state))
    {
        puts("state read:");
        print_state(state);

        puts("state expected:");
        print_state(expected_state);

        return false;
    }

    return true;
}

bool test_simple()
{
    /* State:
     *
     * |LSB MSB|
     * 1 0 0 1 0
     * 0 0 0 0 1
     * 0 1 0 0 0
     */

    uint32_t * state;

    const uint32_t expected_state[] = {
        0x09,
        0x10,
        0x02
    };

    read_pbm("tests/pbm/simple_start.pbm", &state, &state_width, &state_height, NULL);

    state_size = state_height * 4;

    if (state_width != 5 && state_height != 3)
    {
        printf("width: %u, expected_width: %u\n", state_width, 8);
        printf("height: %u, expected_height: %u\n", state_height, 9);

        return false;
    }
    if (!states_equal(state, expected_state))
    {
        puts("state read:");
        print_state(state);

        puts("state expected:");
        print_state(expected_state);

        return false;
    }

    return true;
}

int main(int argc, char * argv[])
{
    state_size = 1;

    RUN_TEST(test_61c);
    RUN_TEST(test_spiral_8);
    RUN_TEST(test_simple);

    return EXIT_STATUS;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


/* tests/test_read_pbm.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include "ice.h"
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

    int width, height;

    uint32_t * state;

    const uint32_t expected_state[] = {
        0x771d1,
        0x11150,
        0x17750,
        0x14550,
        0x777d1
    };

    state = read_pbm("tests/pbm/61c_end.pbm", &state, &width, &height);

    if (!states_equal(state, expected_state))
    {
        puts("state read:");
        print_state(state);

        puts("state expected:");
        print_state(expected_state);

        return false;
    }
    else if (width != 17 && height != 5)
    {
        printf("width: %u, expected_width: %u\n", width, 17);
        printf("height: %u, expected_height: %u\n", height, 5);

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

    int width, height;

    uint32_t * state;

    const uint32_t expected_state[] = {
        0xff,
        0x18,
        0xfb,
        0x1a,
        0xda,
        0x5a,
        0xdb,
        0x18,
        0xff
    };

    read_pbm("tests/pbm/spiral_8_start.pbm", &state, &width, &height);

    if (!states_equal(state, expected_state))
    {
        puts("state read:");
        print_state(state);

        puts("state expected:");
        print_state(expected_state);

        return false;
    }
    else if (width != 8 && height != 9)
    {
        printf("width: %u, expected_width: %u\n", width, 8);
        printf("height: %u, expected_height: %u\n", height, 9);

        return false;
    }

    return true;
}

int main(int argc, char * argv[])
{
    RUN_TEST(test_61c);
    RUN_TEST(test_spiral_8);

    return EXIT_STATUS;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


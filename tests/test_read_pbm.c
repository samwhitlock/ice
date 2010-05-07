/* tests/test_read_pbm.c
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2010 Sam Whitlock
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
/* This is to make it easier to put into hex
1110 1110 1000 1011 1100 0100 0100 0101
0010 0011 1011 1010 1001 0000 0101 0101
0100 1110 1110 1110 1011 1
 */
    uint32_t * state;

    const uint32_t expected_state[] = {
        0xa223d177,
        0xaa095dc4,
        0x1d7772
    };

    read_pbm("tests/pbm/61c_end.pbm", &state, &state_width, &state_height, NULL);

    ints_per_state = ((state_width * state_height) / 32) + ((state_width * state_height)%32) > 0 ? 1 : 0;
    state_size = ints_per_state * 4;

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
    
    /* Easier to put into hex
     *LSB                                       MSB
     * 1111 1111 1000 0001 1111 1101 1000 0101
     * 1011 0101 1010 0101 1011 1101 1000 0001
     * 1111 1111
     */

    uint32_t * state;

    const uint32_t expected_state[] = {
        0xa1bf81ff,
        0x81bda5ad,
        0xff
    };

    read_pbm("tests/pbm/spiral_8_start.pbm", &state, &state_width, &state_height, NULL);

    ints_per_state = ((state_width * state_height) / 32) + ((state_width * state_height)%32) > 0 ? 1 : 0;
    state_size = ints_per_state * 4;

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

    /* State:
     * Easier to put into hex
     * |LSB            MSB|
     *  1001 0000 0101 000
     */
    
    uint32_t * state;

    const uint32_t expected_state[] = {
        0xa09
    };

    read_pbm("tests/pbm/simple_start.pbm", &state, &state_width, &state_height, NULL);

    ints_per_state = ((state_width * state_height) / 32) + ((state_width * state_height)%32) > 0 ? 1 : 0;
    state_size = ints_per_state * 4;

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
    RUN_TEST(test_61c);
    RUN_TEST(test_spiral_8);
    RUN_TEST(test_simple);

    return EXIT_STATUS;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


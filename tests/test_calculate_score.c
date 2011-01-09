/* tests/test_calculate_score.c
 *
 * This file is part of ice.
 *
 * ice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ice.  If not, see <http://www.gnu.org/licenses/>.
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


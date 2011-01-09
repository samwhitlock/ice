/* tests/test_queue.c
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "test.h"
#include "queue.h"

static bool test_queue()
{
    struct queue queue;

    queue_initialize(&queue);

    queue_insert(&queue, 5, (void *) 5);
    queue_insert(&queue, 2, (void *) 2);
    queue_insert(&queue, 9, (void *) 9);
    queue_insert(&queue, 7, (void *) 7);

    if (queue_pop(&queue) != (void *) 2) return false;
    if (queue_pop(&queue) != (void *) 5) return false;
    if (queue_pop(&queue) != (void *) 7) return false;
    if (queue_pop(&queue) != (void *) 9) return false;

    if (!(queue.size == 0)) return false;

    return true;
}

int main(int argc, char * argv[])
{
    RUN_TEST(test_queue);

    return EXIT_STATUS;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


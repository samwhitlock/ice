/* tests/test_queue.c
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

    return true;
}

int main(int argc, char * argv[])
{
    RUN_TEST(test_queue);

    return EXIT_STATUS;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


/* queue.h
 *
 * Copyright (c) 2010 Michael Forney
 */

#ifndef MOVE_NODE_QUEUE
#define MOVE_NODE_QUEUE 1

#include <omp.h>

#include "ice.h"

struct queue_node
{
    unsigned int score;

    struct position * state;
};

struct queue
{
    int size;
    int capacity;
    struct queue_node * nodes;
    omp_lock_t lock;
};

void queue_initialize(struct queue * queue);

struct position * queue_pop(struct queue * queue);

void queue_insert(struct queue * queue, unsigned int score, struct position * positions);

#endif

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


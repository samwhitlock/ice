/* queue.h
 *
 * Copyright (c) 2010 Michael Forney
 */

#ifndef MOVE_NODE_QUEUE
#define MOVE_NODE_QUEUE 1

#include "ice.h"

struct queue_node
{
    unsigned int score;

    const struct move_tree * move_node;
};

struct queue
{
    int size;
    int capacity;
    struct queue_node * nodes;
};

void queue_initialize(struct queue * queue);
void queue_finalize(struct queue * queue);

const struct move_tree * queue_pop(struct queue * queue);

void queue_insert(struct queue * queue, unsigned int score, const struct move_tree * move_node);

#endif

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


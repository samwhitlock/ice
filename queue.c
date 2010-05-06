/* queue.c
 *
 * Copyright (c) 2010 Michael Forney
 */

#include <omp.h>

#include "queue.h"

static inline int left_child(size_t index)
{
    return 2 * index + 1;
}

static inline int right_child(size_t index)
{
    return 2 * index + 2;
}

static inline int parent(size_t index)
{
    return (index - 1) / 2;
}

void queue_initialize(struct queue * queue)
{
    queue->capacity = 1024 * 1024;
    queue->size = 0;
    queue->nodes = malloc(queue->capacity * sizeof(struct queue_node));

    omp_init_lock(&queue->lock);
}

struct move_tree * queue_pop(struct queue * queue)
{
    struct move_tree * move_node;

    omp_set_lock(&queue->lock);

    move_node = queue->nodes[0].move_node;

    if (--queue->size > 0)
    {
        unsigned int score = queue->nodes[queue->size].score;
        struct move_tree * move_node = queue->nodes[queue->size].move_node;

        int index;
        int min_child_index;

        for (index = 0; left_child(index) < queue->size; index = min_child_index)
        {
            min_child_index = left_child(index);

            if (min_child_index + 1 < queue->size &&
                queue->nodes[min_child_index + 1].score < queue->nodes[min_child_index].score)
            {
                ++min_child_index;
            }

            if (score > queue->nodes[min_child_index].score)
            {
                queue->nodes[index].score = queue->nodes[min_child_index].score;
                queue->nodes[index].move_node = queue->nodes[min_child_index].move_node;
            }
            else break;
        }

        queue->nodes[index].score = score;
        queue->nodes[index].move_node = move_node;
    }

    omp_unset_lock(&queue->lock);

    return move_node;
}

void queue_insert(struct queue * queue, unsigned int score, struct move_tree * move_node)
{
    int parent_index;
    int index = queue->size++;

    omp_set_lock(&queue->lock);

    if (queue->size > queue->capacity)
    {
        queue->capacity *= 2;
        queue->nodes = realloc(queue->nodes, queue->capacity * sizeof(struct queue_node));
    }

    queue->nodes[index].score = score;

    for (parent_index = parent(index);
        (index != 0) && score < queue->nodes[parent_index].score;
        index = parent_index, parent_index = parent(index))
    {
        queue->nodes[index].score = queue->nodes[parent_index].score;
        queue->nodes[index].move_node = queue->nodes[parent_index].move_node;
    }

    queue->nodes[index].score = score;
    queue->nodes[index].move_node = move_node;

    omp_unset_lock(&queue->lock);
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


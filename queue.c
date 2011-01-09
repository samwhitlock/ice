/* queue.c
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
}

void queue_finalize(struct queue * queue)
{
    free(queue->nodes);
}

struct move_index queue_pop(struct queue * queue)
{
    struct move_index move_index;
    __builtin_prefetch(&queue->nodes[queue->size], 0, 1);//TODO: optimize the locality (the second number)
    move_index = queue->nodes[0].move_index;

    if (--queue->size > 0)
    {
        unsigned int score = queue->nodes[queue->size].score;
        const struct move_index move_index = queue->nodes[queue->size].move_index;

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
                queue->nodes[index].move_index = queue->nodes[min_child_index].move_index;
            }
            else break;
        }

        queue->nodes[index].score = score;
        queue->nodes[index].move_index = move_index;
    }

    return move_index;
}

void queue_insert(struct queue * queue, unsigned int score, const struct move_index move_index)
{
    int parent_index;
    int index = queue->size++;

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
        __builtin_prefetch(&queue->nodes[parent(index)], 0, 0);
        queue->nodes[index].score = queue->nodes[parent_index].score;
        queue->nodes[index].move_index = queue->nodes[parent_index].move_index;
    }

    queue->nodes[index].score = score;
    queue->nodes[index].move_index = move_index;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


/* queue.h
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

#ifndef MOVE_NODE_QUEUE
#define MOVE_NODE_QUEUE 1

#include "ice.h"

struct queue_node
{
    unsigned int score;

    struct move_index move_index;
};

struct queue
{
    int size;
    int capacity;
    struct queue_node * nodes;
};

void queue_initialize(struct queue * queue);
void queue_finalize(struct queue * queue);

struct move_index queue_pop(struct queue * queue);

void queue_insert(struct queue * queue, unsigned int score, const struct move_index move_index);

#endif

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


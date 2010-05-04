/* ice.c
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2010 Sam Whitlock
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <omp.h>

#include "ice.h"
#include "queue.h"

#ifdef DEBUG
#define DEBUG_PRINT(format, args...) printf(format, ## args)
#else
#define DEBUG_PRINT(format, args...)
#endif

char direction_char[] = {
    [NORTH] = 'N',
    [SOUTH] = 'S',
    [EAST] = 'E',
    [WEST] = 'W'
};

/* The number of positions that compose a state of the bits */
int state_length;

struct position * past_states = NULL;
struct move_tree * move_tree = NULL;

int branches_length = 0;
int branches_capacity = 0;

static void initialize_branches()
{
    /* Initialize past states array */
    branches_length = 0;
    branches_capacity = 1024 * 1024;
    past_states = malloc(branches_capacity * state_length * sizeof(struct position));
    move_tree = malloc(branches_capacity * sizeof(struct move_tree));
}

static void finalize_branches()
{
    free(past_states);
    free(move_tree);
}

bool move(enum direction direction, int move_index,
    const struct position * current, struct position * next)
{
    int index;
    const struct position * other_position;
    const struct position * current_position;
    const struct position * blocking_position = NULL;

    current_position = &current[move_index];

    /* If any of the other bits have a {higher,lower} {x,y} coordinate, this
     * move is valid */
    for (index = 0; index < state_length; ++index)
    {
        /* Skip over the bit we are currently trying to move */
        if (index == move_index) continue;

        other_position = &current[index];

        if ((direction == NORTH &&
                current_position->x == other_position->x && current_position->y > other_position->y + 1 &&
                (!blocking_position || blocking_position->y < current_position->y)) ||
            (direction == SOUTH &&
                current_position->x == other_position->x && current_position->y < other_position->y - 1 &&
                (!blocking_position || blocking_position->y > current_position->y)) ||
            (direction == WEST &&
                current_position->y == other_position->y && current_position->x > other_position->x + 1 &&
                (!blocking_position || blocking_position->x < current_position->x)) ||
            (direction == EAST &&
                current_position->y == other_position->y && current_position->x < other_position->x - 1 &&
                (!blocking_position || blocking_position->x > current_position->x)))
        {
            blocking_position = other_position;
        }
    }

    if (blocking_position)
    {
        next[move_index] = *blocking_position;

        if (direction == NORTH)         ++next[move_index].y;
        else if (direction == SOUTH)    --next[move_index].y;
        else if (direction == WEST)     ++next[move_index].x;
        else                            --next[move_index].x;

        return true;
    }

    return false;
}

bool states_equal(struct position * first_state, struct position * second_state)
{
    struct position * first;
    struct position * second;
    bool found;

    for (first = first_state; first < first_state + state_length; ++first)
    {
        found = false;

        for (second = second_state; second < second_state + state_length; ++second)
        {
            if (first->x == second->x && first->y == second->y)
            {
                found = true;
            }
        }

        if (!found)
        {
            return false;
        }
    }

    return true;
}

unsigned int calculate_score(struct position * first_state, struct position * second_state)
{
    if (states_equal(first_state, second_state))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

static inline struct position * past_state(int index)
{
    return &past_states[index * state_length];
}

static bool is_past_state(struct position * state)
{
    struct position * past;

    for (past = past_states;
        past < past_states + state_length * branches_length;
        past += state_length)
    {
        if (states_equal(state, past))
        {
            return true;
        }
    }

    return false;
}

static int add_branch(struct position * state, struct position * parent, int index, enum direction direction)
{
    ++branches_length;

    if (branches_length > branches_capacity)
    {
        branches_capacity *= 2;

        past_states = realloc(past_states,
            branches_capacity * state_length * sizeof(struct position));
        move_tree = realloc(move_tree, branches_capacity * sizeof(struct move_tree));
    }

    memcpy(past_state(branches_length - 1), state, state_length * sizeof(struct position));
    move_tree[branches_length - 1].parent = &move_tree[parent - past_states];
    move_tree[branches_length - 1].direction = direction;
    move_tree[branches_length - 1].position = &past_state(branches_length - 1)[index];

    return branches_length - 1;
}

void print_state(struct position * state)
{
    int index = 0;

    DEBUG_PRINT("0x%x: { ", state);

    for (index = 0; index < state_length; ++index)
    {
        DEBUG_PRINT("(%u, %u) ", state[index].x, state[index].y);
    }

    DEBUG_PRINT("} (%u)\n", omp_get_thread_num());
}

bool find_path(struct position * start_state, struct position * end_state)
{
    struct queue * queues;
    bool found = false;
    bool done = false;

    initialize_branches();

    add_branch(start_state, NULL, 0, 0);

    #pragma omp parallel shared(found, done)
    {
        struct position * state;
        struct position * position;
        struct position next_state[state_length];
        enum direction direction;
        int queue_index;
        int past_index;
        struct queue * smallest_queue;
        unsigned int score;

        char message[256];

        #pragma omp single
        {
            /* Initialize the queues */
            queues = alloca(omp_get_num_threads() * sizeof(struct queue));

            for (int queue_index = 0; queue_index < omp_get_num_threads(); ++queue_index)
            {
                queue_initialize(&queues[queue_index]);
            }

            queue_insert(&queues[0], calculate_score(past_state(0), end_state), past_state(0));
        }

        while (!done)
        {
            /* Wait until we have something to do */
            while (queues[omp_get_thread_num()].size == 0)
            {
                #pragma omp flush(queues)
            }

            state = queue_pop(&queues[omp_get_thread_num()]);

            #pragma omp critical
            print_state(state);

            for (position = state; position < state + state_length && !done; ++position)
            {
                memcpy(next_state, state, state_length * sizeof(struct position));

                for (direction = NORTH; direction <= WEST && !done; ++direction)
                {
                    if (move(direction, position - state, state, next_state))
                    {
                        if (!is_past_state(next_state))
                        {
                            #pragma omp critical
                            DEBUG_PRINT("%u %u %c - okay (%u)\n", position->x, position->y, direction_char[direction],
                                omp_get_thread_num());

                            score = calculate_score(next_state, end_state);

                            if (score == 0)
                            {
                                /* Huzzah! We found it! */
                                puts("found solution");
                                done = true;
                                found = true;
                            }
                            else
                            {
                                past_index = add_branch(next_state, state, position - state, direction);
                                smallest_queue = NULL;

                                for (queue_index = 0; queue_index < omp_get_num_threads(); ++queue_index)
                                {
                                    if (!smallest_queue || queues[queue_index].size < smallest_queue->size)
                                    {
                                        smallest_queue = &queues[queue_index];

                                        if (smallest_queue->size == 0) break;
                                    }
                                }

                                #pragma omp critical
                                {
                                    DEBUG_PRINT("inserting:\n\t");
                                    DEBUG_PRINT("    to queue %u (%u)\n", smallest_queue - queues, omp_get_thread_num());
                                }

                                queue_insert(smallest_queue, score, past_state(past_index));
                                #pragma omp flush(queues)

                                #pragma omp critical
                                DEBUG_PRINT("queue size: %u (%u)\n", smallest_queue->size, omp_get_thread_num());
                            }
                        }
                    }
                }
            }
        }
    }

    finalize_branches();

    return found;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


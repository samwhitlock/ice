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
#include "bit_twiddle.h"

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

enum flip
{
    OFF,
    ON
};

/* The number of positions that compose a state of the bits */
int state_length, state_height, state_width;

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

#define leading_zeros __builtin_clz
#define trailing_zeros __builtin_ctz

bool horizontal_seek(enum direction, const uint32_t * state, const struct position * current_position, struct position * block_position)
{
    bool first = true;
    
    if( direction == WEST )
    {
        unsigned int lz;
        for (int i = horiz_offset(current_position->x), offset = 0; i >= 0; --i)//FIXME: write this inline function
        {
            //some sort of prefetch can be added
            if(first)
            {
               lz = leading_zeros(state[i] << (32 - (current_position->x % 32)));
                
                if (lz < 32)
                {
                    block_position->y=current_position->y;
                    block_position->x=current_position->x - (lz + 1);
                    return true;
                } else
                {   
                    offset += leading_zeros(state[i]);
                    first = false;
                }
            } else
            { 
                lz = leading_zeros(state[i]);
                if (lz < 32)
                {
                    //it's found
                    block_position->y=current_position->y;
                    block_position->x=current_position->x - (offset+lz+1);
                    return true;
                }
                
                offset += 32;
            }
        }
    } else //direction == EAST
    {
        unsigned int tz;
        for (int i = horiz_offset(current_position->x), offset = 0; i < ints_per_row; ++i)//FIXME: write this inline function
        {
            //some sort of prefetch can be added
            if(first)
            {
                tz = trailing_zeros(state[i] >> (current_position->y % 32));
                
                if (tz < 32)
                {
                    block_position->y=current_position->y;
                    block_position->x=current_position->x + (tz + 1);
                    return true;
                } else
                {
                    offset += trailing_zeros(state[i]);
                    first = false;
                }
            } else
            {
                tz = trailing_zeros(state[i]);
                if (tz < 32)
                {
                    //it's found!!!
                    block_position->y=current_position->y;
                    block_position->x=current_position->x + (offset+tz+1);
                    return true;
                }
                
                offset += 32;
            }
        }
    }
    
    return false;
}

static inline int offset(const struct * position)
{
    return (ints_per_row * position->y) + (position->x / 32);
}

static inline int horizontal_offset(int x)
{
    return x / 32;
}

static inline int get_bit(const uint32_t * bit_str, int offset)
{
    return (*bit_str << offset & 1);
}

static inline void move_bit(uint32_t * state, const struct * initial_position, const struct * final_position)
{
    int initial_offset = offset(initial_position), final_offset = offset(final_position);
    *state[initial_offset] = *state[initial_offset] & mask(initial_position, OFF);
    *state[final_offset] = *state[final_offset] | mask(final_position, ON);
}

static inline uint32_t mask(const struct * position, enum flip)
{
    uint32_t ret_num = 0x80000000 >> (position->x % 32);
    return flip == ON ? ret_num : ~ret_num;
}



bool vertical_seek(enum direction, const uint32_t * state, const struct position * current_position, struct position * block_position)
{
    int mod_cp = current_position->x % 32;
    if (direction == NORTH)
    {
        for (int j = offset(current_position) - ints_per_row, y = current_position->y; j >= 0; j -= ints_per_row, --y)//FIXME: write this inline function
        {
            if (get_bit(state[j], mod_cp))//FIXME: write this function
            {
                block_position->x = current_position->x;
                block_position->y = y;
                return true;
            }
        }
    } else //direction == SOUTH
    {
        for (int j = offset(current_position) + ints_per_row, y = current_position->y; j < ints_per_state; j += ints_per_row, ++y)//FIXME: write this inline function
        {
            if (get_bit(state[j], mod_cp))//FIXME: write this function!
            {
                block_position->x = current_position->x;
                block_position->y = y;
                return true;
            }
        }
    }
    
    return false;
}

bool move(enum direction direction, struct position * position,
    const uint32_t * current_state, uint32_t * next_state)
{
    struct position * set_position = (position*) alloca(sizeof(position));
    
    if (direction == NORTH || direction == SOUTH)
    {
        if (vertical_seek(direction, current_state, position, set_position))
        {
            memcpy(next_state, current_state, state_size);
            move_bit(next_state, position, set_position);//FIXME: write this function
            return true;
        }
    } else //direction == WEST || direction == EAST
    {
        if (horizontal_seek(direction, current_state, position, set_position))
        {
            memcpy(next_state, current_state, state_size);
            move_bit(next_state, position, set_position);//FIXME: write this function
            return true;
        }
    }
    
    return false;
}

bool states_equal(uint32_t * first, uint32_t * second)
{
    return memcmp(first, second, state_size);
}

unsigned int calculate_score(uint32_t * first_state, uint32_t * second_state)
{
    unsigned int num_bits_set = 0;
    uint32_t temp;
    for (int i = 0; i < state_size / 4; ++i) {//tiny optimization: maybe declare a global state_size_int = state_size / 4 if you use it other places
        temp = first_state[i] ^ second_state[i];
        num_bits_sets += bits_set(temp);
        //builtin prefetch the next ones here
    }
    
    return num_bits_set;
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


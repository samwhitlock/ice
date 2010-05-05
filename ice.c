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

/* Some helpful definitions */
#define leading_zeros __builtin_clz
#define trailing_zeros __builtin_ctz
#define set_ones __builtin_popcount
#define first_one __builtin_ffs

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
int state_height;
int state_width;
size_t state_size;

int ints_per_row;
int ints_per_state;

struct move_tree * move_tree = NULL;
int move_tree_length;
int move_tree_capacity;

static void initialize_move_tree()
{
    /* Initialize past states array */
    move_tree_length = 0;
    move_tree_capacity = 1024 * 1024;
    move_tree = malloc(move_tree_capacity * (sizeof(struct move_tree) + state_size));
}

static void finalize_move_tree()
{
    free(move_tree);
}

static inline int offset(const struct position * position)
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

static inline uint32_t mask(const struct position * position, enum flip flip)
{
    uint32_t ret_num = 0x80000000 >> (position->x % 32);
    return flip == ON ? ret_num : ~ret_num;
}

static inline void move_bit(uint32_t * state, const struct position * initial_position, const struct position * final_position)
{
    int initial_offset = offset(initial_position), final_offset = offset(final_position);
    state[initial_offset] = state[initial_offset] & mask(initial_position, OFF);
    state[final_offset] = state[final_offset] | mask(final_position, ON);
}

bool horizontal_seek(enum direction direction, const uint32_t * state, const struct position * current_position, struct position * block_position)
{
    bool first = true;
    
    if( direction == WEST )
    {
        unsigned int lz;
        for (int i = horizontal_offset(current_position->x), offset = 0; i >= 0; --i)//FIXME: write this inline function
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
        for (int i = horizontal_offset(current_position->x), offset = 0; i < ints_per_row; ++i)//FIXME: write this inline function
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

bool vertical_seek(enum direction direction, const uint32_t * state, const struct position * current_position, struct position * block_position)
{
    int mod_cp = current_position->x % 32;
    if (direction == NORTH)
    {
        for (int j = offset(current_position) - ints_per_row, y = current_position->y; j >= 0; j -= ints_per_row, --y)//FIXME: write this inline function
        {
            if (get_bit(&state[j], mod_cp))//FIXME: write this function
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
            if (get_bit(&state[j], mod_cp))//FIXME: write this function!
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
    struct position set_position;
    
    if (direction == NORTH || direction == SOUTH)
    {
        if (vertical_seek(direction, current_state, position, &set_position))
        {
            memcpy(next_state, current_state, state_size);
            move_bit(next_state, position, &set_position);//FIXME: write this function
            return true;
        }
    } else //direction == WEST || direction == EAST
    {
        if (horizontal_seek(direction, current_state, position, &set_position))
        {
            memcpy(next_state, current_state, state_size);
            move_bit(next_state, position, &set_position);//FIXME: write this function
            return true;
        }
    }
    
    return false;
}

bool states_equal(const uint32_t * first, const uint32_t * second)
{
    return memcmp(first, second, state_size);
}

unsigned int calculate_score(const uint32_t * first_state, const uint32_t * second_state)
{
    unsigned int score = 0;

    int index;

    for (index = 0; index < ints_per_state; ++index)
    {
        score += set_ones(first_state[index] ^ second_state[index]);
    }

    return score;
}

static inline struct move_tree * past_move(int index)
{
    return ((void *) &move_tree[index]) + index * state_size;
}

static bool is_past_state(const uint32_t * state)
{
    int index;

    for (index = 0; index < move_tree_capacity; ++index)
    {
        if (states_equal(past_move(index)->state, state))
        {
            return true;
        }
    }

    return false;
}

static uint32_t * add_move(const uint32_t * state, const uint32_t * parent_state,
    const struct position * position, enum direction direction)
{
    struct move_tree * move_node;

    move_node = past_move(move_tree_length++);

    if (move_tree_length > move_tree_capacity)
    {
        move_tree_capacity *= 2;
        move_tree = realloc(move_tree, move_tree_capacity * state_size);
    }

    if (position) move_node->position = *position;
    move_node->direction = direction;
    move_node->parent = ((void *) parent_state) - ((long) &((struct move_tree *) NULL)->state);

    memcpy(move_node->state, state, state_size);

    return move_node->state;
}

bool find_path(const uint32_t * start_state, const uint32_t * end_state)
{
    struct queue * queues;
    bool found = false;
    bool done = false;
    unsigned int jobs = 0;
    unsigned int threads_waiting = 0;
    int queue_index;

    ints_per_row = state_width / 32 + state_width % 32 == 0 ? 0 : 1;
    ints_per_state = state_height * ints_per_row;
    state_size = ints_per_state * 4;

    initialize_move_tree();

    add_move(start_state, NULL, NULL, 0);

    #pragma omp parallel shared(found, done)
    {
        uint32_t * state;
        uint32_t next_state[ints_per_state];

        uint32_t bit_set;
        int bit_set_index;
        char bit_index;

        struct position position;
        enum direction direction;
        unsigned int score;

        #pragma omp single
        {
            /* Initialize the queues */
            queues = alloca(omp_get_num_threads() * sizeof(struct queue));

            for (int queue_index = 0; queue_index < omp_get_num_threads(); ++queue_index)
            {
                queue_initialize(&queues[queue_index]);
            }

            queue_insert(&queues[0], 0, past_move(0)->state);
        }

        threads_waiting = omp_get_num_threads();

        while (!done)
        {
            /* Wait until we have something to do */
            while (queues[omp_get_thread_num()].size == 0)
            {
                #pragma omp flush(queues)

                if (threads_waiting == omp_get_num_threads())
                {
                    #pragma omp single
                    for (queue_index = 0; queue_index < omp_get_num_threads(); ++queue_index)
                    {
                        if (queues[queue_index].size > 0) break;
                    }

                    if (queue_index == omp_get_num_threads())
                    {
                        done = true;
                    }
                }
            }

            #pragma omp atomic
            --threads_waiting;

            state = queue_pop(&queues[omp_get_thread_num()]);

            for (bit_set_index = 0; bit_set_index < ints_per_state && !done; ++bit_set_index)
            {
                bit_set = state[bit_set_index];

                while (bit_set && !done)
                {
                    bit_index = first_one(bit_set) - 1;

                    position.x = bit_set_index % ints_per_row + bit_index;
                    position.y = bit_set_index / ints_per_row;

                    for (direction = NORTH; direction <= WEST && !done; ++direction)
                    {
                        if (move(direction, &position, state, next_state))
                        {
                            if (!is_past_state(next_state))
                            {
                                #pragma omp critical
                                DEBUG_PRINT("%u %u %c - okay (%u)\n", position.x, position.y, direction_char[direction],
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
                                    add_move(next_state, state, &position, direction);

                                    queue_insert(&queues[jobs++ % omp_get_num_threads()], score,
                                        add_move(next_state, state, &position, direction));
                                }
                            }
                        }
                    }

                    bit_set &= ~(1 << bit_index);
                }
            }

            #pragma omp atomic
            ++threads_waiting;
        }
    }

    finalize_move_tree();

    return found;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


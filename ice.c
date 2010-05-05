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

/* Some helpful definitions */
#define set_ones __builtin_popcount
#define first_one __builtin_ffs
#define leading_zeros __builtin_clz
#define trailing_zeros __builtin_ctz
#define ONES_THRESHOLD 6//fool with this later

char direction_char[] = {
    [NORTH] = 'N',
    [SOUTH] = 'S',
    [EAST] = 'E',
    [WEST] = 'W'
};

enum flip
{
    OFF = 0,
    ON
};

/* The number of positions that compose a state of the bits */
int state_height, state_width, state_ones;
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

static inline uint32_t * state_bitset(const uint32_t * state, int x, int y)
{
    return &state[y * ints_per_row + x / 32];
}

static inline bool state_bit(const uint32_t * state, int x, int y)
{
    return *state_bitset(state, x, y) & (1 << (x % 32));
}

static inline void state_set_bit(const uint32_t * state, int x, int y)
{
    *state_bitset(state, x, y) |= 1 << (x % 32);
}

static inline void state_clear_bit(const uint32_t * state, int x, int y)
{
    *state_bitset(state, x, y) &= ~(1 << (x % 32));
}

static inline void state_move_bit(const uint32_t * state, int from_x, int from_y, int to_x, int to_y)
{
    printf("moving (%u, %u) to (%u, %u)\n", from_x, from_y, to_x, to_y);
    state_clear_bit(state, from_x, from_y);
    state_set_bit(state, to_x, to_y);
}

static inline int offset(const struct position * position)
{
    return (ints_per_row * position->y) + ((position->x) / 32);
}

static inline int block_index_offset(const struct position * position)
{
    return (position->x) % 32;
}

bool move(enum direction direction, const struct position * position,
    const uint32_t * state, uint32_t * next_state)
{
    printf("MOVE (%u, %u) %c\n", position->x, position->y, direction_char[direction]);

    if (direction == NORTH)
    {
        int y;

        if (position->y == 0 || state_bit(state, position->x, position->y - 1))
        {
            return false;
        }

        for (y = position->y - 2; y >= 0; --y)
        {
            if (state_bit(state, position->x, y))
            {
                memcpy(next_state, state, state_size);
                state_set_bit(next_state, position->x, y + 1);
                state_clear_bit(next_state, position->x, position->y);

                return true;
            }
        }
    }
    else if (direction == SOUTH)
    {
        int y;

        if (position->y == state_height - 1 || state_bit(state, position->x, position->y + 1))
        {
            return false;
        }

        for (y = position->y + 2; y < state_height; ++y)
        {
            if (state_bit(state, position->x, y))
            {
                memcpy(next_state, state, state_size);
                state_set_bit(next_state, position->x, y - 1);
                state_clear_bit(next_state, position->x, position->y);

                return true;
            }
        }
    }
    else
    {
        int state_offset = offset(position), bit_offset = position->x, init_index = block_index_offset(position);
        uint32_t bitSet;
        
        if (direction == EAST)
        {
            //first stuff
            bitSet = state[state_offset] >> (init_index + 1);

            if (bitSet != 0)//their are blocking bits to the EAST
            {
                //found!
                if( trailing_zeros(bitSet) == 0 )
                {
                    return false;
                } else
                {
                    bit_offset += trailing_zeros(bitSet);
                    memcpy(next_state, state, state_size);
                    state_move_bit(next_state, position->x, position->y, bit_offset, position->y);
                    return true;
                }
            } else {
                //add the number of zeros to the EAST to the offset
                bit_offset += 32 - (init_index+1);
            }
            
            for (++state_offset; state_offset % ints_per_row > 0; ++state_offset)
            {
                bitSet = state[state_offset];
                
                if(bitSet != 0)
                {
                    //found!
                    bit_offset += trailing_zeros(bitSet);
                    memcpy(next_state, state, state_size);
                    state_move_bit(next_state, position->x, position->y, bit_offset, position->y);
                    return true;
                } else {
                    bit_offset += 32;                    
                }
            }
        }
        else//directino == WEST
        {
            //first stuff
            bitSet = state[state_offset] << (32 - init_index);
            
            if (bitSet != 0)//their are blocking bits to the WEST
            {
                //found!
                if( leading_zeros(bitSet) == 0 )
                {
                    return false;
                } else 
                {

                    bit_offset -= leading_zeros(bitSet);
                    memcpy(next_state, state, state_size);
                    state_move_bit(next_state, position->x, position->y, bit_offset, position->y);
                    return true;
                }
            } else {
                //add the number of zeros to the WEST to the offset
                bit_offset += init_index;
            }
            
            for (--state_offset; state_offset % ints_per_row < ints_per_row - 1; --state_offset)
            {
                bitSet = state[state_offset];
                
                if(bitSet != 0)
                {
                    //found!
                    bit_offset -= leading_zeros(bitSet);
                    memcpy(next_state, state, state_size);
                    state_move_bit(next_state, position->x, position->y, bit_offset, position->y);
                    return true;
                } else {
                    bit_offset -= 32;
                }
            }
        }
    } 
    return false;
}

void print_state(const uint32_t * state)
{
    int x, y;

    for (y = 0; y < state_height; ++y)
    {
        for (x = 0; x < state_width; ++x)
        {
            putchar(((state[y * ints_per_row + x / 32] >> (x % 32)) & 1) + '0');
        }

        putchar('\n');
    }

    putchar('\n');
}

bool states_equal(const uint32_t * first, const uint32_t * second)
{
    return memcmp(first, second, state_size) == 0;
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

    #pragma omp parallel if(state_ones>ONES_THRESHOLD) shared(found, done)
    {
        uint32_t * state;
        uint32_t * past_state;
        uint32_t next_state[ints_per_state];

        uint32_t bitset;
        int bitset_index;
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
            #pragma omp critical
            printf("starting (%u)\n", omp_get_thread_num());

            /* Wait until we have something to do */
            while (queues[omp_get_thread_num()].size == 0 && !done)
            {
                #pragma omp flush(queues)

/*
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
*/
            }

            #pragma omp atomic
            --threads_waiting;

            state = queue_pop(&queues[omp_get_thread_num()]);

            #pragma omp critical
            {
                printf("processing state: 0x%x (%u)\n", state, omp_get_thread_num());
                print_state(state);
            }

            for (bitset_index = 0; bitset_index < ints_per_state && !done; ++bitset_index)
            {
                bitset = state[bitset_index];

                while (bitset && !done)
                {
                    bit_index = first_one(bitset) - 1;

                    position.x = bitset_index % ints_per_row + bit_index;
                    position.y = bitset_index / ints_per_row;

                    for (direction = NORTH; direction <= WEST && !done; ++direction)
                    {
                        if (move(direction, &position, state, next_state))
                        {
                            puts("before:");
                            print_state(state);
                            puts("after:");
                            print_state(next_state);
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
                                    past_state = add_move(next_state, state, &position, direction);
                                    printf("adding job to queue %u: 0x%x (%u)\n", jobs % omp_get_num_threads(),
                                        past_state, omp_get_thread_num());
                                    queue_insert(&queues[jobs++ % omp_get_num_threads()], score, past_state);
                                }
                            }
                        }
                    }

                    bitset &= ~(1 << bit_index);
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


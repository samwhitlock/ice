/* ice.c
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2010 Sam Whitlock
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <pthread.h>

#ifdef __linux
#include <unistd.h>
#else /* Assume Mac */
#include <sys/sysctl.h>
#endif

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
#define atomic_increment(variable) __sync_fetch_and_add(&variable, 1)
#define atomic_decrement(variable) __sync_fetch_and_sub(&variable, 1)

#define ONES_THRESHOLD -1//fool with this later

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
int state_height, state_width, state_ones, ints_per_state;
size_t state_size;

struct move_tree * move_tree = NULL;
int move_tree_length, move_tree_capacity;

struct move * moves;
int moves_length;

const uint32_t * end_state;

/* Thread Variables */
int thread_count;
unsigned int jobs;
bool found;
unsigned int threads_waiting;

pthread_t * threads;
pthread_cond_t * queue_conditions;
pthread_mutex_t * queue_mutexes;
struct queue * queues;

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

static inline uint32_t total_bit_offset(int x, int y)
{
    return (uint32_t) x+(y*state_height);//FIXME: generate num_columns
}

static inline uint32_t state_index(int x, int y)
{
    return total_bit_offset(x, y) / 32;    
}

static inline uint32_t bit_offset(int x, int y)
{
    return total_bit_offset(x, y) % 32;
}

//Note: assumes offset < 32
static inline uint32_t mask(uint32_t offset, enum flip flp)
{
    uint32_t num = 1 << offset;
    if (flp == ON)
    {
        return num;
    } else
    {
        return ~num;
    }
}

static inline void set_bit(int x, int y, uint32_t * state, enum flip flp)
{
    uint32_t index = state_index(x, y), offset = bit_offset(x, y);
    if (flp == ON)
    {
        state[index] = state[index] | mask(offset,flp);
    } else
    {
        state[index] = state[index] & mask(offset,flp);
    }
}

//Note: This assumes there is a bit at FROM and not one at TO. This is just an abstraction with no checks, which should be done elsewhere
static inline void move_bit(uint32_t * state, int from_x, int from_y, int to_x, int to_y)
{
    set_bit(from_x, from_y, state, OFF);
    set_bit(to_x, to_y, state, ON);
}

static inline uint32_t get_bit(int x, int y, const uint32_t * state)
{
    return (1 & (state[state_index(x,y)] >> bit_offset(x,y)));
}

bool move(enum direction direction, const struct position * position,
    const uint32_t * state, uint32_t * next_state)
{
    //TODO: add prefetch optimizations for loops to prepare to get_bit and set_bit
    bool first = true;
    if (direction == NORTH)
    {
        for (int x = position->x, y = position->y-1; y >= 0; --y, first = false)
        {
            if (get_bit(x, y, state) != 0)
            {
                if (first)
                    return false;
                else
                {
                    memcpy(next_state, state, state_size);
                    move_bit(next_state, position->x, position->y, x, y+1);
                    return true;
                }
            }
        }
    } else if (direction == SOUTH)
    {
        for (int x = position->x, y = position->y+1; y < state_height; ++y, first = false)//FIXME: generate num_rows somewhere
        {
            if (get_bit(x, y, state) != 0)
            {
                if (first)
                    return false;
                else
                {
                    memcpy(next_state, state, state_size);
                    move_bit(next_state, position->x, position->y, x, y-1);
                    return true;
                }
            }
        }
    } else if (direction == EAST)
    {
        for (int x = position->x+1, y = position->y; x < state_width; ++x, first = false)
        {
            if (get_bit(x, y, state) != 0)
            {
                if (first)
                    return false;
                else
                {
                    memcpy(next_state, state, state_size);
                    move_bit(next_state, position->x, position->y, x-1, y);
                    return true;
                }
            }
        } 
    } else //if (direction == WEST)
    {
        for (int x = position->x-1, y = position->y; x >= 0; --x, first = false)
        {
            if (get_bit(x, y, state) != 0)
            {
                if (first)
                    return false;
                else
                {
                    memcpy(next_state, state, state_size);
                    move_bit(next_state, position->x, position->y, x+1, y);
                    return true;
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
            putchar(get_bit(x, y, state) + '0');
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

    for (index = 0; index < state_size; ++index)
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

static struct move_tree * add_move(const uint32_t * state, const struct move_tree * parent,
    const struct position * position, enum direction direction)
{
    struct move_tree * move_node;

    move_node = past_move(move_tree_length++);

    if (move_tree_length > move_tree_capacity)
    {
        move_tree_capacity *= 2;
        move_tree = realloc(move_tree, move_tree_capacity * state_size);
    }

    if (position) move_node->move.position = *position;
    move_node->move.direction = direction;
    move_node->parent = parent;
    move_node->depth = parent ? parent->depth + 1 : 0;

    memcpy(move_node->state, state, state_size);

    return move_node;
}

void build_move_list(const struct move_tree * move_node)
{
    moves_length = move_node->depth;
    moves = malloc(moves_length * sizeof(struct move));

    for (; move_node->depth > 0; move_node = move_node->parent)
    {
        moves[move_node->depth - 1] = move_node->move;
    }
}

static inline int x_position(int bitset_index, int bit_index)
{
    return ((bitset_index * 32) % state_width) + bit_index;
}

static inline int y_position(int bitset_index, int bit_index)
{
    return ((bitset_index * 32) + bit_index) / 32;
}

static void * process_jobs(void * generic_thread_id)
{
    int thread_id = (int) generic_thread_id;

    int queue_index;

    uint32_t * state;
    struct move_tree * move_node;
    struct move_tree * next_move_node;
    uint32_t next_state[state_size];

    uint32_t bitset;
    int bitset_index;
    char bit_index;

    int bitset_type;

    struct position position;
    enum direction direction;
    unsigned int score;

    while (true)
    {
        atomic_increment(threads_waiting);

        while (queues[thread_id].size == 0)
        {
            pthread_cond_wait(&queue_conditions[thread_id], &queue_mutexes[thread_id]);
        }

        atomic_decrement(threads_waiting);

        move_node = queue_pop(&queues[thread_id]);
        state = move_node->state;

        pthread_mutex_unlock(&queue_mutexes[thread_id]);

        // FIXME: ints_per row is no longer used
        for (bitset_index = 0; bitset_index < state_size; ++bitset_index)
        {
            bitset = state[bitset_index];

            while (bitset)
            {
                bit_index = first_one(bitset) - 1;

                position.x = x_position(bitset_index, bit_index);
                position.y = y_position(bitset_index, bit_index);

                for (direction = NORTH; direction <= WEST; ++direction)
                {
                    if (move(direction, &position, state, next_state))
                    {
                        puts("before:");
                        print_state(state);
                        puts("after:");
                        print_state(next_state);

                        if (!is_past_state(next_state))
                        {
                            score = calculate_score(next_state, end_state);
                            next_move_node = add_move(next_state, move_node, &position, direction);

                            if (score == 0)
                            {
                                int id;

                                /* Huzzah! We found it! */
                                puts("found solution");
                                found = true;

                                for (id = 0; id < thread_count; ++id)
                                {
                                    if (id == thread_id) continue;

                                    pthread_cancel(threads[id]);
                                }

                                build_move_list(next_move_node);

                                return NULL;
                            }
                            else
                            {
                                queue_index = atomic_increment(jobs) % thread_count;

                                printf("adding job to queue %u: 0x%x (%u)\n", queue_index,
                                    next_move_node, thread_id);

                                pthread_mutex_lock(&queue_mutexes[queue_index]);
                                queue_insert(&queues[queue_index], score, next_move_node);
                                pthread_cond_signal(&queue_conditions[queue_index]);
                                pthread_mutex_unlock(&queue_mutexes[queue_index]);
                            }
                        }
                    }
                }

                bitset &= ~(1 << bit_index);
            }
        }
    }

    return NULL;
}

bool find_path(const uint32_t * start, const uint32_t * end)
{
    int id;

    found = false;
    jobs = 0;
    threads_waiting = 0;

    #ifdef __linux
    thread_count = sysconf(_SC_NPROCESSORS_ONLN);
    #else /* Assume Mac */
    size_t length = sizeof(thread_count);
    int mib[] = { CTL_HW, HW_AVAILCPU };
    sysctl(mib, 2, &thread_count, &length, NULL, 0);
    #endif

    printf("starting %u threads\n", thread_count);

    ints_per_state = state_height * state_width / 32;
    state_size = ints_per_state * 4;

    threads = alloca(thread_count * sizeof(pthread_t));
    queue_mutexes = alloca(thread_count * sizeof(pthread_mutex_t));
    queue_conditions = alloca(thread_count * sizeof(pthread_cond_t));
    queues = alloca(thread_count * sizeof(struct queue));

    for (id = 0; id < thread_count; ++id)
    {
        pthread_mutex_init(&queue_mutexes[id], NULL);
        pthread_cond_init(&queue_conditions[id], NULL);
        queue_initialize(&queues[id]);
    }

    initialize_move_tree();

    end_state = end;
    add_move(start, NULL, NULL, 0);
    queue_insert(&queues[0], 0, past_move(0));

    for (id = 0; id < thread_count; ++id)
    {
        pthread_create(&threads[id], NULL, &process_jobs, (void *) id);
    }

    for (id = 0; id < thread_count; ++id)
    {
        pthread_join(threads[id], NULL);
    }

    finalize_move_tree();

    return found;
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


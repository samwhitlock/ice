/* ice.c
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2010 Sam Whitlock
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>
#include <signal.h>

#define __USE_XOPEN2K
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

pthread_rwlock_t * move_tree_lock;

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
    pthread_rwlock_rdlock(move_tree_lock);
    if (direction == NORTH)
    {
        int y;

        if (position->y == 0 || state_bit(state, position->x, position->y - 1))
        {
            pthread_rwlock_unlock(move_tree_lock);
            return false;
        }

        for (y = position->y - 2; y >= 0; --y)
        {
            if (state_bit(state, position->x, y))
            {
                memcpy(next_state, state, state_size);
                state_set_bit(next_state, position->x, y + 1);
                state_clear_bit(next_state, position->x, position->y);

                pthread_rwlock_unlock(move_tree_lock);
                return true;
            }
        }
    }
    else if (direction == SOUTH)
    {
        int y;

        if (position->y == state_height - 1 || state_bit(state, position->x, position->y + 1))
        {
            pthread_rwlock_unlock(move_tree_lock);
            return false;
        }

        for (y = position->y + 2; y < state_height; ++y)
        {
            if (state_bit(state, position->x, y))
            {
                memcpy(next_state, state, state_size);
                state_set_bit(next_state, position->x, y - 1);
                state_clear_bit(next_state, position->x, position->y);

                pthread_rwlock_unlock(move_tree_lock);
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
            bitSet =  init_index == 31 ? 0 : state[state_offset] >> (init_index + 1);
            if (bitSet != 0)//their are blocking bits to the EAST
            {
                //found!
                if (trailing_zeros(bitSet) == 0)
                {
                    pthread_rwlock_unlock(move_tree_lock);
                    return false;
                }
                else
                {
                    bit_offset += trailing_zeros(bitSet);
                    memcpy(next_state, state, state_size);
                    state_move_bit(next_state, position->x, position->y, bit_offset, position->y);
                    pthread_rwlock_unlock(move_tree_lock);
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
                    if (trailing_zeros(bitSet) == 0 && bit_offset == position->x)
                    {
                        //this is where the error is happening
                        pthread_rwlock_unlock(move_tree_lock);
                        return false;
                    }
                    else
                    {
                        //found!
                        bit_offset += trailing_zeros(bitSet);
                        memcpy(next_state, state, state_size);
                        state_move_bit(next_state, position->x, position->y, bit_offset, position->y);
                        pthread_rwlock_unlock(move_tree_lock);
                        return true;
                    }
                }
                else
                {
                    bit_offset += 32;
                }
            }
        }
        else//direction == WEST
        {
            //first stuff
            bitSet = init_index == 0 ? 0 : state[state_offset] << (32 - init_index);

            if (bitSet != 0)//their are blocking bits to the WEST
            {
                //found!
                if (leading_zeros(bitSet) == 0)
                {
                    pthread_rwlock_unlock(move_tree_lock);
                    return false;
                }
                else
                {
                    bit_offset -= leading_zeros(bitSet);
                    memcpy(next_state, state, state_size);
                    state_move_bit(next_state, position->x, position->y, bit_offset, position->y);
                    pthread_rwlock_unlock(move_tree_lock);
                    return true;
                }
            } else {
                //add the number of zeros to the WEST to the offset
                bit_offset -= init_index;
            }

            for (--state_offset; state_offset % ints_per_row < ints_per_row - 1; --state_offset)
            {
                bitSet = state[state_offset];

                if (bitSet != 0)
                {
                    //found!
                    if (leading_zeros(bitSet)==0 && bit_offset == position->x)
                    {
                        //problem is here
                        pthread_rwlock_unlock(move_tree_lock);
                        return false;
                    }
                    else
                    {
                        bit_offset -= leading_zeros(bitSet);
                        memcpy(next_state, state, state_size);
                        state_move_bit(next_state, position->x, position->y, bit_offset, position->y);
                        pthread_rwlock_unlock(move_tree_lock);
                        return true;
                    }
                }
                else
                {
                    bit_offset -= 32;
                }
            }
        }
    }

    pthread_rwlock_unlock(move_tree_lock);
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

static struct move_tree * add_move(const uint32_t * state, const struct move_tree * parent,
    const struct position * position, enum direction direction)
{
    struct move_tree * move_node;

    int move_index = atomic_increment(move_tree_length);

    if (move_tree_length > move_tree_capacity)
    {
        pthread_rwlock_wrlock(move_tree_lock);
        move_tree_capacity *= 2;
        move_tree = realloc(move_tree, move_tree_capacity * state_size);
        pthread_rwlock_unlock(move_tree_lock);
    }

    pthread_rwlock_rdlock(move_tree_lock);
    move_node = past_move(move_index);

    if (position) move_node->move.position = *position;
    move_node->move.direction = direction;
    move_node->parent = parent;
    move_node->depth = parent ? parent->depth + 1 : 0;

    memcpy(move_node->state, state, state_size);
    pthread_rwlock_unlock(move_tree_lock);

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

static void terminate_thread(int signal)
{
    if (signal == SIGTERM)
    {
        pthread_exit(NULL);
    }
}

static void * process_jobs(void * generic_thread_id)
{
    int thread_id = (int) generic_thread_id;

    int queue_index;

    uint32_t * state;
    struct move_tree * move_node;
    struct move_tree * next_move_node;
    uint32_t next_state[ints_per_state];

    uint32_t bitset;
    int bitset_index;
    char bit_index;

    int bitset_type;

    struct position position;
    enum direction direction;
    unsigned int score;

    bool processed_all;

    signal(SIGTERM, &terminate_thread);

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

        printf("processing 0x%x\n", state);

        for (bitset_index = 0; bitset_index < ints_per_state; ++bitset_index)
        {
            pthread_rwlock_rdlock(move_tree_lock);
            bitset = state[bitset_index] & ~end_state[bitset_index];
            pthread_rwlock_unlock(move_tree_lock);
            processed_all = false;

            while (bitset || !processed_all)
            {
                bit_index = first_one(bitset) - 1;

                position.x = bitset_index % ints_per_row + bit_index;
                position.y = bitset_index / ints_per_row;

                for (direction = NORTH; direction <= WEST; ++direction)
                {
                    if (move(direction, &position, state, next_state))
                    {
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

                                    pthread_kill(threads[id], SIGTERM);
                                }

                                build_move_list(next_move_node);

                                return NULL;
                            }
                            else
                            {
                                queue_index = atomic_increment(jobs) % thread_count;

                                pthread_mutex_lock(&queue_mutexes[queue_index]);
                                queue_insert(&queues[queue_index], score, next_move_node);
                                pthread_cond_signal(&queue_conditions[queue_index]);
                                pthread_mutex_unlock(&queue_mutexes[queue_index]);
                            }
                        }
                    }
                }

                bitset &= ~(1 << bit_index);

                if (!bitset && !processed_all)
                {
                    pthread_rwlock_rdlock(move_tree_lock);
                    bitset = state[bitset_index] & end_state[bitset_index];
                    pthread_rwlock_unlock(move_tree_lock);
                    processed_all = true;
                }
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

    ints_per_row = state_width / 32 + state_width % 32 == 0 ? 0 : 1;
    ints_per_state = state_height * ints_per_row;
    state_size = ints_per_state * 4;

    threads = alloca(thread_count * sizeof(pthread_t));
    queue_mutexes = alloca(thread_count * sizeof(pthread_mutex_t));
    queue_conditions = alloca(thread_count * sizeof(pthread_cond_t));
    queues = alloca(thread_count * sizeof(struct queue));
    move_tree_lock = alloca(sizeof(pthread_rwlock_t));

    pthread_rwlock_init(move_tree_lock, NULL);

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


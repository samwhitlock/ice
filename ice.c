/* ice.c
 *
 * Copyright (c) 2010 Michael Forney
 * Copyright (c) 2010 Sam Whitlock
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alloca.h>

#define __USE_XOPEN2K
#define __USE_GNU

#include <xlocale.h>
#include <pthread.h>

#ifdef __linux
#include <unistd.h>
#else /* Assume Mac */
#include <sys/sysctl.h>
#endif

#include "ice.h"
#include "queue.h"

/* Some helpful definitions */
#define set_ones __builtin_popcount
#define first_one __builtin_ffs
#define leading_zeros __builtin_clz
#define trailing_zeros __builtin_ctz
#define atomic_increment(variable) __sync_fetch_and_add(&variable, 1)
#define atomic_decrement(variable) __sync_fetch_and_sub(&variable, 1)
#define prefetch __builtin_prefetch

#define ONES_THRESHOLD -1//fool with this later
#define HASH_MAX 1024

char direction_char[] = {
    [NORTH] = 'N',
    [SOUTH] = 'S',
    [EAST] = 'E',
    [WEST] = 'W'
};

/* The number of positions that compose a state of the bits */
int state_height, state_width, state_ones, ints_per_state;
size_t state_size;

struct move_tree * move_tree = NULL;
int move_tree_capacity;

int move_tree_hash_write_length[HASH_MAX];
int move_tree_hash_length[HASH_MAX];

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

pthread_mutex_t terminate_lock;
pthread_rwlock_t move_tree_lock;

static void initialize_move_tree()
{
    /* Initialize past states array */
    move_tree_capacity = 1024 * 8;
    move_tree = malloc(move_tree_capacity * HASH_MAX * (sizeof(struct move_tree) + state_size));
    memset(move_tree_hash_length, 0, sizeof(move_tree_hash_length));
    memset(move_tree_hash_write_length, 0, sizeof(move_tree_hash_write_length));
}

static void finalize_move_tree()
{
    free(move_tree);
}

/**
 * Returns the index of the bitset that contains the specified x and y
 * coordinates.
 */
static inline int bitset_index(int x, int y)
{
    return (y * state_width + x) / 32;
}

/**
 * Returns the index of the bit within the bitset that contains the specified x
 * and y coordinates.
 */
static inline int bit_index(int x, int y)
{
    return (y * state_width + x) % 32;
}

/**
 * Sets the bit located at the given x and y coordinates of the given state.
 */
static inline void state_set_bit(uint32_t * state, int x, int y)
{
    state[bitset_index(x, y)] |= 1 << bit_index(x, y);
}

/**
 * Clears the bit located at the given x and y coordinates of the given state.
 */
static inline void state_clear_bit(uint32_t * state, int x, int y)
{
    state[bitset_index(x, y)] &= ~(1 << bit_index(x, y));
}

/**
 * Moves the bit from one position to another in a given state.
 */
static inline void state_move_bit(uint32_t * state, int from_x, int from_y, int to_x, int to_y)
{
    state_clear_bit(state, from_x, from_y);
    state_set_bit(state, to_x, to_y);
}

/**
 * Gets the bit at the specified position in a given state.
 */
static inline bool state_bit(const uint32_t * state, int x, int y)
{
    return state[bitset_index(x, y)] & (1 << bit_index(x, y));
}

static inline struct move_tree * past_move(const struct move_index move_index)
{
    return ((void *) &move_tree[move_index.index * HASH_MAX + move_index.hash]) +
        (move_index.index * HASH_MAX + move_index.hash) * state_size;
}

/**
 * Move a bit in a desired position in the given direction. Updates
 * the next_state if move is valid.
 * @return true if move is valid; false otherwise
 */
#define move_read_locality 2//This specifies the levels of cache (i.e. level of locality). Valid 
bool move(enum direction direction, const struct position * position,
    const struct move_index move_index, uint32_t * next_state)
{
    int x = position->x;
    int y = position->y;

    pthread_rwlock_rdlock(&move_tree_lock);
    const uint32_t * state = past_move(move_index)->state;

    if (direction == NORTH)
    {
        if (y == 0 || state_bit(state, x, y - 1)) goto invalid;

        for (y = position->y - 2; y >= 0; --y)
        {
            if (state_bit(state, x, y))
            {
                ++y;
                goto valid;
            }
            prefetch(state +(((y-1) * state_width + x) / 32), 0, move_read_locality);
        }
    }
    else if (direction == SOUTH)
    {
        if (y == state_height - 1 || state_bit(state, x, y + 1)) goto invalid;

        for (y = position->y + 2; y < state_height; ++y)
        {
            if (state_bit(state, x, y))
            {
                --y;
                goto valid;
            }
            prefetch(state +(((y+1) * state_width + x) / 32), 0, move_read_locality);
        }
    }
    else if (direction == EAST)
    {
        if (x == state_width - 1 || state_bit(state, x + 1, y)) goto invalid;

        for (x = position->x + 2; x < state_width; ++x)
        {
            if (state_bit(state, x, y))
            {
                --x;
                goto valid;
            }
            prefetch(state +((y * state_width + (x+1)) / 32), 0, move_read_locality);
        }
    }
    else /* direction == WEST */
    {
        if (x == 0 || state_bit(state, x - 1, y)) goto invalid;

        for (x = position->x - 2; x >= 0; --x)
        {
            if (state_bit(state, x, y))
            {
                ++x;
                goto valid;
            }
            prefetch(state +((y * state_width + (x-1)) / 32), 0, move_read_locality);
        }
    }

invalid:
    pthread_rwlock_unlock(&move_tree_lock);
    return false;

valid:
    memcpy(next_state, state, state_size);
    pthread_rwlock_unlock(&move_tree_lock);
    state_move_bit(next_state, position->x, position->y, x, y);
    return true;
}

void print_state(const uint32_t * state)
{
    int x, y;

    for (y = 0; y < state_height; ++y)
    {
        for (x = 0; x < state_width; ++x)
        {
            putchar(state_bit(state, x, y) + '0');
        }

        putchar('\n');
    }

    putchar('\n');
}

bool states_equal(const uint32_t * first, const uint32_t * second)
{
    return memcmp(first, second, state_size) == 0;
}

#define calculate_score_prefetch_locality 0
unsigned int calculate_score(const uint32_t * first_state, const uint32_t * second_state)
{
    if(states_equal(first_state, second_state)) return 0;

    unsigned int score = 0;
    int index;

    for (index = 0; index < ints_per_state; ++index)
    {
        prefetch(first_state+(index+1), 0, calculate_score_prefetch_locality);
        prefetch(second_state+(index+1), 0, calculate_score_prefetch_locality);
        score += set_ones(first_state[index] ^ second_state[index]);
    }

    return score;
}

/*
 * Implementation of the One-at-a-Time hash.
 */
#define calculate_hash_prefetch_locality 0
unsigned short calculate_hash(const uint32_t * state)
{
    uint32_t hash = 0;

    for (int i = 0; i < ints_per_state; ++i)
    {
        hash += state[i];
        prefetch(&state[i+1], 0, calculate_hash_prefetch_locality);
        hash += ( hash << 10 );
        hash ^= ( hash >> 6 );
    }

    hash += ( hash << 3 );
    hash ^= ( hash >> 11 );
    hash += ( hash << 15 );
 
    return (unsigned short) hash % HASH_MAX;
}

static bool is_past_state(unsigned short hash, const uint32_t * state)
{
    struct move_index move_index;

    move_index.hash = hash;

    pthread_rwlock_rdlock(&move_tree_lock);

    for (move_index.index = 0; move_index.index < move_tree_hash_length[hash]; ++move_index.index)
    {
        if (states_equal(past_move(move_index)->state, state))
        {
            pthread_rwlock_unlock(&move_tree_lock);
            return true;
        }
    }

    pthread_rwlock_unlock(&move_tree_lock);
    return false;
}

static struct move_index add_move(const uint32_t * state, unsigned short hash,
    const struct move_index parent, const struct position * position, enum direction direction)
{
    struct move_index move_index;
    struct move_tree * move_node;

    move_index.index = atomic_increment(move_tree_hash_write_length[hash]);
    move_index.hash = hash;

    if (move_index.index > move_tree_capacity)
    {
        pthread_rwlock_wrlock(&move_tree_lock);
        move_tree_capacity *= 2;
        move_tree = realloc(move_tree, move_tree_capacity *
            (sizeof(struct move_tree) + state_size) * HASH_MAX);
        pthread_rwlock_unlock(&move_tree_lock);
    }

    pthread_rwlock_rdlock(&move_tree_lock);
    move_node = past_move(move_index);

    if (position) move_node->move.position = *position;
    move_node->move.direction = direction;
    move_node->parent = parent;
    move_node->depth = (parent.index >= 0) ? past_move(parent)->depth + 1 : 0;

    memcpy(move_node->state, state, state_size);
    pthread_rwlock_unlock(&move_tree_lock);

    atomic_increment(move_tree_hash_length[hash]);

    return move_index;
}

#define build_move_list_prefetch_locality 0
void build_move_list(const struct move_tree * move_node)
{
    moves_length = move_node->depth;
    moves = malloc(moves_length * sizeof(struct move));

    for (; move_node->depth > 0; move_node = past_move(move_node->parent))
    {
        prefetch(&moves[past_move(move_node->parent)->depth - 1], 0, build_move_list_prefetch_locality);

        moves[move_node->depth - 1] = move_node->move;
    }
}

static inline int x_position(int bitset_index, int bit_index)
{
    return (bitset_index * 32 + bit_index) % state_width;
}

static inline int y_position(int bitset_index, int bit_index)
{
    return (bitset_index * 32 + bit_index) / state_width;
}

static void * process_jobs(void * generic_thread_id)
{
    int thread_id = (int) generic_thread_id;

    int queue_index;

    struct move_index move_index;
    struct move_index next_move_index;
    uint32_t next_state[ints_per_state];

    uint32_t bitset;
    int bitset_index;
    char bit_index;

    struct position position;
    enum direction direction;
    unsigned int score;
    unsigned short hash;

    while (true)
    {
        atomic_increment(threads_waiting);

        pthread_mutex_lock(&queue_mutexes[thread_id]);

        while (queues[thread_id].size == 0)
        {
            if (threads_waiting == thread_count)
            {
                pthread_mutex_lock(&terminate_lock);

                for (queue_index = 0; queue_index < thread_count; ++queue_index)
                {
                    if (queues[queue_index].size > 0) break;
                }

                if (queue_index == thread_count)
                {
                    puts("IMPOSSIBLE");
                    exit(0);
                }
                else
                {
                    pthread_mutex_unlock(&terminate_lock);
                }
            }

            pthread_cond_wait(&queue_conditions[thread_id], &queue_mutexes[thread_id]);
        }

        atomic_decrement(threads_waiting);

        move_index = queue_pop(&queues[thread_id]);

        pthread_mutex_unlock(&queue_mutexes[thread_id]);

        for (bitset_index = 0; bitset_index < ints_per_state; ++bitset_index)
        {
            pthread_rwlock_rdlock(&move_tree_lock);
            bitset = past_move(move_index)->state[bitset_index];
            pthread_rwlock_unlock(&move_tree_lock);

            while (bitset)
            {
                bit_index = first_one(bitset) - 1;

                if (bit_index < 0) break;

                position.x = x_position(bitset_index, bit_index);
                position.y = y_position(bitset_index, bit_index);

                for (direction = NORTH; direction <= WEST; ++direction)
                {
                    if (move(direction, &position, move_index, next_state))
                    {
                        prefetch(next_state, 0, calculate_hash_prefetch_locality);//prefetch for the hash function
                        hash = calculate_hash(next_state);

                        if (!is_past_state(hash, next_state))
                        {
                            prefetch(next_state, 0, calculate_score_prefetch_locality);//prefetches for score calculations
                            prefetch(end_state, 0, calculate_score_prefetch_locality);

                            score = calculate_score(next_state, end_state);
                            next_move_index = add_move(next_state, hash, move_index, &position, direction);

                            if (score == 0)
                            {
                                int index;

                                /* Huzzah! We found it! */
                                pthread_mutex_lock(&terminate_lock);

                                found = true;

                                build_move_list(past_move(next_move_index));

                                for (index = 0; index < moves_length; ++index)
                                {
                                    printf("%u %u %c\n", moves[index].position.x, moves[index].position.y,
                                        direction_char[moves[index].direction]);
                                }

                                exit(0);
                            }
                            else
                            {
                                queue_index = atomic_increment(jobs) % thread_count;

                                pthread_mutex_lock(&queue_mutexes[queue_index]);
                                queue_insert(&queues[queue_index], score, next_move_index);
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

void find_path(const uint32_t * start, const uint32_t * end)
{
    int id;
    pthread_attr_t attributes;
    struct sched_param param = { 99 };

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

    --thread_count;

    ints_per_state = state_height * state_width / 32 +
        ((state_height * state_width % 32 == 0) ? 0 : 1);
    state_size = ints_per_state * 4;

    if (states_equal(start, end)) return;

    threads = alloca(thread_count * sizeof(pthread_t));
    queue_mutexes = alloca(thread_count * sizeof(pthread_mutex_t));
    queue_conditions = alloca(thread_count * sizeof(pthread_cond_t));
    queues = alloca(thread_count * sizeof(struct queue));

    pthread_rwlock_init(&move_tree_lock, NULL);
    pthread_mutex_init(&terminate_lock, NULL);

    for (id = 0; id < thread_count; ++id)
    {
        pthread_mutex_init(&queue_mutexes[id], NULL);
        pthread_cond_init(&queue_conditions[id], NULL);
        queue_initialize(&queues[id]);
    }

    initialize_move_tree();

    end_state = end;
    add_move(start, 0, (struct move_index) { 0, -1 }, NULL, 0);
    queue_insert(&queues[0], 0, (struct move_index) { 0, 0 });

    pthread_attr_init(&attributes);

    pthread_attr_setschedparam(&attributes, &param);
    pthread_attr_setschedpolicy(&attributes, SCHED_RR);

    for (id = 1; id < thread_count; ++id)
    {
        pthread_create(&threads[id], &attributes, &process_jobs, (void *) id);
    }

    process_jobs((void *) 0);
}

// vim: et sts=4 ts=8 sw=4 fo=croql fdm=syntax


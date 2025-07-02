#ifndef MALLOC_H
#define MALLOC_H

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>

/* Zone sizes - optimized for performance vs memory usage */
#define TINY_ZONE_SIZE (getpagesize() * 16)   /* 64KB zones */
#define SMALL_ZONE_SIZE (getpagesize() * 128) /* 512KB zones */
#define TINY_MAX_SIZE 128                     /* 1 to 128 bytes */
#define SMALL_MAX_SIZE 1024                   /* 129 to 1024 bytes */

/* Ensure at least 100 allocations per zone */
#define MIN_ALLOCS_PER_ZONE 100

/* Block header structure */
typedef struct s_block
{
    size_t size;
    int is_free;
    struct s_block *next;
    struct s_block *prev;
} t_block;

/* Zone structure */
typedef struct s_zone
{
    void *start;
    size_t size;
    size_t used;
    t_block *blocks;
    struct s_zone *next;
} t_zone;

/* Memory manager structure */
typedef struct s_malloc_state
{
    t_zone *tiny_zones;
    t_zone *small_zones;
    t_zone *large_zones;
    pthread_mutex_t mutex;
} t_malloc_state;

/* Global state */
extern t_malloc_state g_malloc_state;

/* Function prototypes */
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void show_alloc_mem(void);

/* Bonus functions */
void show_alloc_mem_ex(void);
void defragment_memory(void);
void malloc_stats(void);

/* Internal functions */
void init_malloc_state(void);
t_zone *create_zone(size_t zone_size);
t_block *find_free_block(t_zone *zone, size_t size);
t_block *split_block(t_block *block, size_t size);
void merge_free_blocks(t_zone *zone);
t_zone *find_zone_for_ptr(void *ptr);
t_block *get_block_from_ptr(void *ptr);
size_t align_size(size_t size);

#endif

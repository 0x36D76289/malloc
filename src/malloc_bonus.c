#include "malloc.h"
#include <stdio.h>

static int g_debug_mode = 0;
static int g_debug_initialized = 0;

void init_debug_mode(void)
{
    if (g_debug_initialized)
        return;

    char *debug_env = getenv("MALLOC_DEBUG");
    if (debug_env && *debug_env == '1')
    {
        g_debug_mode = 1;
        fprintf(stderr, "MALLOC_DEBUG: Debug mode enabled\n");
    }
    g_debug_initialized = 1;
}

void show_alloc_mem_ex(void)
{
    init_debug_mode();

    pthread_mutex_lock(&g_malloc_state.mutex);

    size_t total = 0;
    size_t total_zones = 0;
    size_t total_blocks = 0;
    t_zone *zone;

    printf("=== Enhanced Memory Allocation Report ===\n");
    printf("HOSTTYPE: %s\n", getenv("HOSTTYPE") ? getenv("HOSTTYPE") : "undefined");
    printf("Page size: %d bytes\n", getpagesize());

    // Show TINY zones
    zone = g_malloc_state.tiny_zones;
    if (zone)
        printf("\n--- TINY ZONES (≤ %d bytes) ---\n", TINY_MAX_SIZE);
    while (zone)
    {
        total_zones++;
        printf("Zone %p: size=%zu, used=%zu (%.1f%% utilization)\n",
               zone->start, zone->size, zone->used,
               zone->size > 0 ? (double)zone->used / zone->size * 100 : 0);

        t_block *block = zone->blocks;
        int block_count = 0;
        while (block)
        {
            if (!block->is_free)
            {
                void *start = (char *)block + sizeof(t_block);
                void *end = (char *)start + block->size - 1;
                printf("  %p - %p : %zu bytes", start, end, block->size);

                if (g_debug_mode && block->size <= 64)
                {
                    printf(" [");
                    unsigned char *data = (unsigned char *)start;
                    for (size_t i = 0; i < (block->size > 16 ? 16 : block->size); i++)
                    {
                        printf("%02x ", data[i]);
                    }
                    if (block->size > 16)
                        printf("...");
                    printf("]");
                }
                printf("\n");

                total += block->size;
                total_blocks++;
                block_count++;
            }
            block = block->next;
        }
        printf("  Active blocks in zone: %d\n", block_count);
        zone = zone->next;
    }

    // Show SMALL zones
    zone = g_malloc_state.small_zones;
    if (zone)
        printf("\n--- SMALL ZONES (%d - %d bytes) ---\n", TINY_MAX_SIZE + 1, SMALL_MAX_SIZE);
    while (zone)
    {
        total_zones++;
        printf("Zone %p: size=%zu, used=%zu (%.1f%% utilization)\n",
               zone->start, zone->size, zone->used,
               zone->size > 0 ? (double)zone->used / zone->size * 100 : 0);

        t_block *block = zone->blocks;
        int block_count = 0;
        while (block)
        {
            if (!block->is_free)
            {
                void *start = (char *)block + sizeof(t_block);
                void *end = (char *)start + block->size - 1;
                printf("  %p - %p : %zu bytes\n", start, end, block->size);
                total += block->size;
                total_blocks++;
                block_count++;
            }
            block = block->next;
        }
        printf("  Active blocks in zone: %d\n", block_count);
        zone = zone->next;
    }

    // Show LARGE zones
    zone = g_malloc_state.large_zones;
    if (zone)
        printf("\n--- LARGE ZONES (> %d bytes) ---\n", SMALL_MAX_SIZE);
    while (zone)
    {
        total_zones++;
        printf("Zone %p: size=%zu, used=%zu (%.1f%% utilization)\n",
               zone->start, zone->size, zone->used,
               zone->size > 0 ? (double)zone->used / zone->size * 100 : 0);

        t_block *block = zone->blocks;
        int block_count = 0;
        while (block)
        {
            if (!block->is_free)
            {
                void *start = (char *)block + sizeof(t_block);
                void *end = (char *)start + block->size - 1;
                printf("  %p - %p : %zu bytes\n", start, end, block->size);
                total += block->size;
                total_blocks++;
                block_count++;
            }
            block = block->next;
        }
        printf("  Active blocks in zone: %d\n", block_count);
        zone = zone->next;
    }

    printf("\n=== Summary ===\n");
    printf("Total allocated: %zu bytes\n", total);
    printf("Total zones: %zu\n", total_zones);
    printf("Total blocks: %zu\n", total_blocks);
    printf("Average block size: %.1f bytes\n",
           total_blocks > 0 ? (double)total / total_blocks : 0);

    pthread_mutex_unlock(&g_malloc_state.mutex);
}

void defragment_memory(void)
{
    pthread_mutex_lock(&g_malloc_state.mutex);

    t_zone *zone = g_malloc_state.tiny_zones;
    while (zone)
    {
        merge_free_blocks(zone);
        zone = zone->next;
    }

    zone = g_malloc_state.small_zones;
    while (zone)
    {
        merge_free_blocks(zone);
        zone = zone->next;
    }

    t_zone **current = &g_malloc_state.large_zones;
    while (*current)
    {
        zone = *current;
        if (zone->used == 0)
        {
            *current = zone->next;
            munmap(zone, sizeof(t_zone) + zone->size);
        }
        else
        {
            current = &zone->next;
        }
    }

    pthread_mutex_unlock(&g_malloc_state.mutex);
}

void malloc_stats(void)
{
    pthread_mutex_lock(&g_malloc_state.mutex);

    printf("=== Malloc Statistics ===\n");

    size_t tiny_total = 0, tiny_used = 0, tiny_zones = 0;
    size_t small_total = 0, small_used = 0, small_zones = 0;
    size_t large_total = 0, large_used = 0, large_zones = 0;

    t_zone *zone = g_malloc_state.tiny_zones;
    while (zone)
    {
        tiny_total += zone->size;
        tiny_used += zone->used;
        tiny_zones++;
        zone = zone->next;
    }

    zone = g_malloc_state.small_zones;
    while (zone)
    {
        small_total += zone->size;
        small_used += zone->used;
        small_zones++;
        zone = zone->next;
    }

    zone = g_malloc_state.large_zones;
    while (zone)
    {
        large_total += zone->size;
        large_used += zone->used;
        large_zones++;
        zone = zone->next;
    }

    printf("TINY:  %zu zones, %zu/%zu bytes (%.1f%% utilization)\n",
           tiny_zones, tiny_used, tiny_total,
           tiny_total > 0 ? (double)tiny_used / tiny_total * 100 : 0);
    printf("SMALL: %zu zones, %zu/%zu bytes (%.1f%% utilization)\n",
           small_zones, small_used, small_total,
           small_total > 0 ? (double)small_used / small_total * 100 : 0);
    printf("LARGE: %zu zones, %zu/%zu bytes (%.1f%% utilization)\n",
           large_zones, large_used, large_total,
           large_total > 0 ? (double)large_used / large_total * 100 : 0);

    size_t total_allocated = tiny_used + small_used + large_used;
    size_t total_mapped = tiny_total + small_total + large_total;

    printf("TOTAL: %zu allocated, %zu mapped (%.1f%% efficiency)\n",
           total_allocated, total_mapped,
           total_mapped > 0 ? (double)total_allocated / total_mapped * 100 : 0);

    pthread_mutex_unlock(&g_malloc_state.mutex);
}

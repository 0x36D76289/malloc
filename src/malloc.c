#include "malloc.h"

t_malloc_state g_malloc_state = {0};
static pthread_once_t g_init_once = PTHREAD_ONCE_INIT;

void init_malloc_state(void)
{
    pthread_mutex_init(&g_malloc_state.mutex, NULL);
    g_malloc_state.tiny_zones = NULL;
    g_malloc_state.small_zones = NULL;
    g_malloc_state.large_zones = NULL;
}

size_t align_size(size_t size)
{
    return (size + 7) & ~7;
}

t_zone *create_zone(size_t zone_size)
{
    void *zone_memory = mmap(NULL, zone_size, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (zone_memory == MAP_FAILED)
        return NULL;

    t_zone *zone = (t_zone *)zone_memory;
    zone->start = (char *)zone_memory + sizeof(t_zone);
    zone->size = zone_size - sizeof(t_zone);
    zone->used = 0;
    zone->blocks = NULL;
    zone->next = NULL;

    /* Create initial free block */
    t_block *initial_block = (t_block *)zone->start;
    initial_block->size = zone->size - sizeof(t_block);
    initial_block->is_free = 1;
    initial_block->next = NULL;
    initial_block->prev = NULL;
    zone->blocks = initial_block;

    return zone;
}

t_block *find_free_block(t_zone *zone, size_t size)
{
    t_block *current = zone->blocks;

    while (current)
    {
        if (current->is_free && current->size >= size)
            return current;
        current = current->next;
    }
    return NULL;
}

t_block *split_block(t_block *block, size_t size)
{
    if (block->size <= size + sizeof(t_block) + 8)
        return block;

    t_block *new_block = (t_block *)((char *)block + sizeof(t_block) + size);
    new_block->size = block->size - size - sizeof(t_block);
    new_block->is_free = 1;
    new_block->next = block->next;
    new_block->prev = block;

    if (block->next)
        block->next->prev = new_block;

    block->next = new_block;
    block->size = size;

    return block;
}

void merge_free_blocks(t_zone *zone)
{
    t_block *current = zone->blocks;

    while (current && current->next)
    {
        if (current->is_free && current->next->is_free)
        {
            t_block *next_block = current->next;
            current->size += sizeof(t_block) + next_block->size;
            current->next = next_block->next;
            if (next_block->next)
                next_block->next->prev = current;
        }
        else
        {
            current = current->next;
        }
    }
}

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;

    if (size > (SIZE_MAX / 2))
        return NULL;

    pthread_once(&g_init_once, init_malloc_state);
    pthread_mutex_lock(&g_malloc_state.mutex);

    size = align_size(size);
    t_zone **zone_list;
    size_t zone_size;

    if (size <= TINY_MAX_SIZE)
    {
        zone_list = &g_malloc_state.tiny_zones;
        zone_size = TINY_ZONE_SIZE;
    }
    else if (size <= SMALL_MAX_SIZE)
    {
        zone_list = &g_malloc_state.small_zones;
        zone_size = SMALL_ZONE_SIZE;
    }
    else
    {
        zone_list = &g_malloc_state.large_zones;
        zone_size = sizeof(t_zone) + sizeof(t_block) + size;
        zone_size = (zone_size + getpagesize() - 1) & ~(getpagesize() - 1);
    }

    t_zone *zone = *zone_list;
    t_block *block = NULL;

    while (zone)
    {
        block = find_free_block(zone, size);
        if (block)
            break;
        zone = zone->next;
    }

    if (!block)
    {
        zone = create_zone(zone_size);
        if (!zone)
        {
            pthread_mutex_unlock(&g_malloc_state.mutex);
            return NULL;
        }
        zone->next = *zone_list;
        *zone_list = zone;
        block = find_free_block(zone, size);
    }

    if (!block)
    {
        pthread_mutex_unlock(&g_malloc_state.mutex);
        return NULL;
    }

    block = split_block(block, size);
    block->is_free = 0;
    zone->used += block->size;

    pthread_mutex_unlock(&g_malloc_state.mutex);
    return (char *)block + sizeof(t_block);
}

t_zone *find_zone_for_ptr(void *ptr)
{
    t_zone *zone;
    char *char_ptr = (char *)ptr;

    // Check tiny zones
    zone = g_malloc_state.tiny_zones;
    while (zone)
    {
        char *zone_start = (char *)zone->start;
        char *zone_end = zone_start + zone->size;
        if (char_ptr >= zone_start && char_ptr < zone_end)
            return zone;
        zone = zone->next;
    }

    // Check small zones
    zone = g_malloc_state.small_zones;
    while (zone)
    {
        char *zone_start = (char *)zone->start;
        char *zone_end = zone_start + zone->size;
        if (char_ptr >= zone_start && char_ptr < zone_end)
            return zone;
        zone = zone->next;
    }

    // Check large zones
    zone = g_malloc_state.large_zones;
    while (zone)
    {
        char *zone_start = (char *)zone->start;
        char *zone_end = zone_start + zone->size;
        if (char_ptr >= zone_start && char_ptr < zone_end)
            return zone;
        zone = zone->next;
    }

    return NULL;
}

t_block *get_block_from_ptr(void *ptr)
{
    return (t_block *)((char *)ptr - sizeof(t_block));
}

void free(void *ptr)
{
    if (!ptr)
        return;

    pthread_mutex_lock(&g_malloc_state.mutex);

    t_zone *zone = find_zone_for_ptr(ptr);
    if (!zone)
    {
        pthread_mutex_unlock(&g_malloc_state.mutex);
        return;
    }

    t_block *block = get_block_from_ptr(ptr);
    if (block->is_free)
    {
        pthread_mutex_unlock(&g_malloc_state.mutex);
        return;
    }

    block->is_free = 1;
    zone->used -= block->size;

    merge_free_blocks(zone);

    if (zone->used == 0 && zone == g_malloc_state.large_zones)
    {
        g_malloc_state.large_zones = zone->next;
        munmap(zone, sizeof(t_zone) + zone->size);
    }

    pthread_mutex_unlock(&g_malloc_state.mutex);
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr)
        return malloc(size);

    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    pthread_mutex_lock(&g_malloc_state.mutex);

    t_zone *zone = find_zone_for_ptr(ptr);
    if (!zone)
    {
        pthread_mutex_unlock(&g_malloc_state.mutex);
        return NULL;
    }

    t_block *block = get_block_from_ptr(ptr);
    size_t old_size = block->size;
    size = align_size(size);

    if (size <= old_size)
    {
        if (old_size - size > sizeof(t_block) + 8)
        {
            split_block(block, size);
            zone->used -= (old_size - size);
        }
        pthread_mutex_unlock(&g_malloc_state.mutex);
        return ptr;
    }

    pthread_mutex_unlock(&g_malloc_state.mutex);

    void *new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;

    memcpy(new_ptr, ptr, old_size);
    free(ptr);
    return new_ptr;
}

void show_alloc_mem(void)
{
    pthread_mutex_lock(&g_malloc_state.mutex);

    size_t total = 0;
    t_zone *zone;

    zone = g_malloc_state.tiny_zones;
    while (zone)
    {
        printf("TINY : %p\n", zone->start);
        t_block *block = zone->blocks;
        while (block)
        {
            if (!block->is_free)
            {
                void *start = (char *)block + sizeof(t_block);
                void *end = (char *)start + block->size - 1;
                printf("%p - %p : %zu bytes\n", start, end, block->size);
                total += block->size;
            }
            block = block->next;
        }
        zone = zone->next;
    }

    zone = g_malloc_state.small_zones;
    while (zone)
    {
        printf("SMALL : %p\n", zone->start);
        t_block *block = zone->blocks;
        while (block)
        {
            if (!block->is_free)
            {
                void *start = (char *)block + sizeof(t_block);
                void *end = (char *)start + block->size - 1;
                printf("%p - %p : %zu bytes\n", start, end, block->size);
                total += block->size;
            }
            block = block->next;
        }
        zone = zone->next;
    }

    zone = g_malloc_state.large_zones;
    while (zone)
    {
        printf("LARGE : %p\n", zone->start);
        t_block *block = zone->blocks;
        while (block)
        {
            if (!block->is_free)
            {
                void *start = (char *)block + sizeof(t_block);
                void *end = (char *)start + block->size - 1;
                printf("%p - %p : %zu bytes\n", start, end, block->size);
                total += block->size;
            }
            block = block->next;
        }
        zone = zone->next;
    }

    printf("Total : %zu bytes\n", total);
    pthread_mutex_unlock(&g_malloc_state.mutex);
}

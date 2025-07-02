#include "libft.h"

void *ft_memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *dest_ptr;
    const unsigned char *src_ptr;

    if (!dest && !src)
        return (NULL);
    dest_ptr = (unsigned char *)dest;
    src_ptr = (const unsigned char *)src;
    while (n--)
        *dest_ptr++ = *src_ptr++;
    return (dest);
}

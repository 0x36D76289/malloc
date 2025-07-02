#include "libft.h"

void *ft_memmove(void *dest, const void *src, size_t n)
{
    unsigned char *dest_ptr;
    const unsigned char *src_ptr;

    if (!dest && !src)
        return (NULL);
    dest_ptr = (unsigned char *)dest;
    src_ptr = (const unsigned char *)src;
    if (dest_ptr < src_ptr)
    {
        while (n--)
            *dest_ptr++ = *src_ptr++;
    }
    else
    {
        dest_ptr += n;
        src_ptr += n;
        while (n--)
            *(--dest_ptr) = *(--src_ptr);
    }
    return (dest);
}
#include "libft.h"

char *ft_strncpy(char *dest, const char *src, size_t n)
{
    char *ptr = dest;

    while (n-- && *src)
        *ptr++ = *src++;
    while (n-- > 0)
        *ptr++ = '\0';
    return dest;
}

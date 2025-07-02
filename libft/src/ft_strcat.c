#include "libft.h"

char *ft_strcat(char *dest, const char *src)
{
    char *ptr = dest;

    while (*ptr)
        ptr++;
    while (*src)
        *ptr++ = *src++;
    *ptr = '\0';
    return dest;
}
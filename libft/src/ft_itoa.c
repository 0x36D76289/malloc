#include "libft.h"

static int get_num_len(long n)
{
    int len;

    len = 0;
    if (n <= 0)
        len = 1;
    while (n)
    {
        n /= 10;
        len++;
    }
    return (len);
}

char *ft_itoa(long n)
{
    char *str;
    int len;
    int negative;

    negative = 0;
    if (n < 0)
    {
        negative = 1;
        n = -n;
    }
    len = get_num_len(n) + negative;
    str = malloc(sizeof(char) * (len + 1));
    if (!str)
        return (NULL);
    str[len] = '\0';
    if (n == 0)
        str[0] = '0';
    while (n > 0)
    {
        str[--len] = (n % 10) + '0';
        n /= 10;
    }
    if (negative)
        str[0] = '-';
    return (str);
}

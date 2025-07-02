#include "libft.h"

void ft_putnbr_fd(int n, int fd)
{
    char str[12];
    int i;
    int sign;

    if (n == 0)
    {
        write(fd, "0", 1);
        return;
    }
    sign = (n < 0) ? -1 : 1;
    n *= sign;
    i = 0;
    while (n > 0)
    {
        str[i++] = (n % 10) + '0';
        n /= 10;
    }
    if (sign < 0)
        str[i++] = '-';
    while (--i >= 0)
        write(fd, &str[i], 1);
}

#include "libft.h"

static size_t count_words(const char *s, char c)
{
    size_t count = 0;
    size_t i = 0;

    while (s[i])
    {
        while (s[i] == c)
            i++;
        if (s[i] && s[i] != c)
        {
            count++;
            while (s[i] && s[i] != c)
                i++;
        }
    }
    return (count);
}

static void free_split(char **split)
{
    size_t i = 0;

    if (!split)
        return;
    while (split[i])
    {
        free(split[i]);
        i++;
    }
    free(split);
}

char **ft_split(const char *s, char c)
{
    char **result;
    size_t i;
    size_t j;
    size_t k;

    if (!s || !(result = (char **)malloc(sizeof(char *) * (count_words(s, c) + 1))))
        return (NULL);
    i = 0;
    j = 0;
    while (s[i])
    {
        if (s[i] != c)
        {
            k = 0;
            while (s[i + k] && s[i + k] != c)
                k++;
            if (!(result[j++] = ft_substr(s, i, k)))
            {
                free_split(result);
                return (NULL);
            }
            i += k;
        }
        else
            i++;
    }
    result[j] = NULL;
    return (result);
}
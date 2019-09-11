#include "util.h"

int char_to_int(const char *s, int len)
{
    int n = 0;
    for (int i = 0; i < len; i++)
    {
        n *= 10;
        n += *(s + i) - '0';
    }
    return n;
}

int contains_char(char *s, char c)
{
    for (char *p = s; *p; p++)
    {
        if (*p == c)
            return 1;
    }
    return 0;
}
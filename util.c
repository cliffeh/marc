#include "util.h"

int atoin(const char *p, int n)
{
    int r = 0;
    for (int i = 0; i < n; i++)
    {
        r *= 10;
        r += *(p + i) - '0';
    }
    return r;
}

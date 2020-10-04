#include <stdio.h>
#include "../marc.h"

const char *specific_usage = "print marc leaders";

void print_result(FILE *out, FILE *in, marcrec *rec, const char *filename)
{
    char buf[99999];
    while (marcrec_read(rec, buf, in) != 0)
    {
        fprintf(out, "%.*s\n", 24, rec->raw);
    }
}

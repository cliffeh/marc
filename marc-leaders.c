#include <stdio.h>
#include "marc.h"

const char *specific_usage = "print marc leaders";

extern int __marc_main_limit;

void print_result(FILE *out, marcrec *rec, int current_file, FILE *in)
{
    int count = 0;
    while (marcrec_read(rec, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        fprintf(out, "%.*s\n", 24, rec->data);
    }
}

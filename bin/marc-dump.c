#include <stdio.h>
#include "../marc.h"

const char *specific_usage = "dump marc records in marc format";

extern int __marc_main_limit;

void print_result(FILE *out, marcrec *rec, const char *filename, FILE *in)
{
    int count = 0;
    while (marcrec_read(rec, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        marcrec_write(out, rec);
    }
}

#include <stdio.h>
#include "marc.h"

const char *specific_usage = "dump marc records in marc format";

extern int __marc_main_limit;
extern int __marc_main_fieldspec_count;

void print_result(FILE *out, marcrec *rec, int current_file, FILE *in)
{
    if(__marc_main_fieldspec_count != 0)
        fprintf(stderr, "warning: --field flag has been provided and is unused by this command\n");

    int count = 0;
    while (marcrec_read(rec, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        marcrec_write(out, rec);
    }
}

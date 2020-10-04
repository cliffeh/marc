#include <stdio.h>
#include "../marc.h"

const char *specific_usage = "print marc leaders";

extern int __marc_main_limit;
extern fieldspec __marc_main_fieldspec;

void print_result(FILE *out, FILE *in, marcrec *rec, const char *filename)
{
    int count = 0;
    char buf[99999];
    while (marcrec_read(rec, buf, in) != 0 && (count++ < __marc_main_limit))
    {
        fprintf(out, "%.*s\n", 24, rec->raw);
    }
}

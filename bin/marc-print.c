#include <stdio.h>
#include "../marc.h"

const char *specific_usage = "print marc records/fields in a human-readable format";

extern int __marc_main_limit;
extern int __marc_main_fieldspec_count;
extern const char **__marc_main_fieldspec;

void print_result(FILE *out, marcrec *rec, const char *filename, FILE *in)
{
    int count = 0;
    while (marcrec_read(rec, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        // TODO pass in fields
        marcrec_print(out, rec, __marc_main_fieldspec_count ? __marc_main_fieldspec : 0);
    }
}

#include <stdio.h>
#include "../marc.h"
#include "../fields.h"

const char *specific_usage = "print marc records/fields in a human-readable format";

extern int __marc_main_limit;
extern int __marc_main_fieldspec_count;
extern fieldspec **__marc_main_fieldspec;

void print_result(FILE *out, FILE *in, marcrec *rec, const char *filename)
{
    int count = 0;
    char buf[99999];
    while (marcrec_read(rec, buf, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        // TODO pass in fields
        marcrec_print(out, rec, 0);
    }
}

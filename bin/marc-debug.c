#include <stdio.h>
#include "../marc.h"

const char *specific_usage = "print debug information and exit";

extern int __marc_main_limit;
extern int __marc_main_fieldspec_count;
extern char **__marc_main_fieldspec;

void print_result(FILE *out, marcrec *rec, const char *filename, FILE *in)
{
    fprintf(out, "filename: %s\n", filename);
    fprintf(stdout, "limit: %i\n", __marc_main_limit);
    fprintf(stdout, "fieldspec count: %i\n", __marc_main_fieldspec_count);
    for(int i = 0; i < __marc_main_fieldspec_count; i++) {
        fprintf(out, "  fieldspec[%i]: %s\n", i, __marc_main_fieldspec[i]);
    }
}

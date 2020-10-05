#include <stdio.h>
#include "../marc.h"

const char *specific_usage = "print debug information and exit";

extern int __marc_main_limit;
extern fieldspec __marc_main_fieldspec;

void print_result(FILE *out, FILE *in, marcrec *rec, const char *filename)
{
    fprintf(out, "filename: %s\n", filename);
    fprintf(stdout, "limit: %i\n", __marc_main_limit);
    fprintf(stdout, "fieldspec length: %i\n", __marc_main_fieldspec.len);
    for(int i = 0; i < __marc_main_fieldspec.len; i++) {
        fprintf(out, "  fieldspec[%i]: %s\n", i, __marc_main_fieldspec.spec[i]);
    }
}

#include <stdio.h>
#include "marc.h"

const char *specific_usage = "validate marc records";

extern int __marc_main_limit;
extern int __marc_main_fieldspec_count;
extern int __marc_main_infile_count;
extern const char **__marc_main_infiles;

void print_result(FILE *out, marcrec *rec, int current_file, gzFile in)
{
    if(__marc_main_fieldspec_count != 0)
        fprintf(stderr, "warning: --field flag has been provided and is unused by this command\n");

    int count = 0, valid = 0;
    while (marcrec_read(rec, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        if (marcrec_validate(rec) == 0)
            valid++;
    }
    fprintf(out, "%s: %i of %i valid\n", __marc_main_infiles[current_file], valid, count);
}

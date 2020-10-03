#include <stdio.h>
#include "marc.h"

void print_result(FILE *out, FILE *in, marcrec *rec, const char *filename)
{
    char buf[99999];
    int count = 0, valid = 0;
    while(marcrec_read(rec, buf, in) != 0) {
        count++;
        if(marcrec_validate(rec) == 0) valid++;
    }
    fprintf(out, "%s: %i of %i valid\n", filename, valid, count);
}

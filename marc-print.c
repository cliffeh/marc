#include <stdio.h>
#include "marc.h"

void print_result(FILE *out, FILE *in, marcrec *rec, const char *arg)
{
    char buf[99999];
    while(marcrec_read(rec, buf, in) != 0) {
        marcrec_write(out, rec, 1);
    }
}

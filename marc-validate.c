#include <stdlib.h>
#include <stdio.h>
#include "marc.h"

static void print_validate(FILE *out, FILE *in, const char *filename)
{
    char buf[99999];
    marcrec *rec = malloc(sizeof(marcrec));
    // 1000 seems like a reasonable upper bound
    rec->fields = malloc(sizeof(marcfield)*1000);
    
    int count = 0, valid = 0;
    while(marcrec_read(rec, buf, in) != 0) {
        count++;
        if(marcrec_validate(rec) == 0) valid++;
    }
    fprintf(out, "%s: %i of %i valid\n", filename, valid, count);

    free(rec->fields);
    free(rec);
}

int main(int argc, char *argv[])
{
    if(argc == 1) { // read from stdin
        print_validate(stdout, stdin, "-");    
    } else {
        for(int i = 1; i < argc; i++) {
            FILE *f = fopen(argv[i], "r");
            print_validate(stdout, f, argv[i]);
            fclose(f);
        }
    }
}

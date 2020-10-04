#include <stdlib.h>
#include <stdio.h>
#include "../marc.h"

extern void print_result(FILE *out, FILE *in, marcrec *rec, const char *arg);

int main(int argc, char *argv[])
{
    marcrec *rec = malloc(sizeof(marcrec));
    // 1000 seems like a reasonable upper bound
    rec->fields = malloc(sizeof(marcfield)*1000);

    if(argc == 1) { // read from stdin
        print_result(stdout, stdin, rec, "-");
    } else {
        for(int i = 1; i < argc; i++) {
            FILE *f = fopen(argv[i], "r");
            print_result(stdout, f, rec, argv[i]);
            fclose(f);
        }
    }

    free(rec->fields);
    free(rec);
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../marc.h"

int __marc_main_limit = -1;      // process all records by default
fieldspec __marc_main_fieldspec; // process all fields by default

extern void print_result(FILE *out, FILE *in, marcrec *rec, const char *filename);
extern const char *specific_usage;

#define USAGE                                                                                 \
    "usage: %s [options] [files]\n"                                                           \
    "\n"                                                                                      \
    "%s\n"                                                                                    \
    "\n"                                                                                      \
    "options:\n"                                                                              \
    "  -h, --help     print a brief help message and exit\n"                                  \
    "  -l, --limit N  limit processing to the first N records per file (default: no limit)\n" \
    "\n"                                                                                      \
    "note: if no files are provided this program will read from stdin\n"

int main(int argc, char *argv[])
{
    marcrec *rec = malloc(sizeof(marcrec));
    // 1000 seems like a reasonable upper bound
    rec->fields = malloc(sizeof(marcfield) * 1000);

    // process command line args
    char **filenames = calloc(argc, sizeof(char *));
    int file_count = 0;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0)
        {
            fprintf(stdout, USAGE, argv[0], specific_usage);
            exit(0);
        }
        /*
        else if (strcmp("--field", argv[i]) == 0 || strcmp("-f", argv[i]) == 0)
        {
            if(i+1 >= argc) {
                fprintf(stderr, "error: %s flag requires an argument\n", argv[i]);
                exit(1);
            }
        }*/
        else if (strcmp("--limit", argv[i]) == 0 || strcmp("-l", argv[i]) == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "error: %s flag requires an argument\n", argv[i]);
                exit(1);
            }
            __marc_main_limit = atoi(argv[++i]);
        }
        else
        { // we'll assume it's a filename
            filenames[file_count++] = argv[i];
        }
    }

    if (file_count == 0)
    { // read from stdin
        print_result(stdout, stdin, rec, "-");
    }
    else
    {
        for (int i = 0; i < file_count; i++)
        {
            FILE *f = fopen(filenames[i], "r");
            print_result(stdout, f, rec, filenames[i]);
            fclose(f);
        }
    }

    free(rec->fields);
    free(rec);
}

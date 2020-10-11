#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../marc.h"

/* process all records by default */
int __marc_main_limit = -1;

/* process all fields by default */
int __marc_main_fieldspec_count;
const char **__marc_main_fieldspec;

/* output file (stdout if unspecified) */
const char *__marc_main_outfile;

extern void print_result(FILE *out, marcrec *rec, const char *filename, FILE *in);
extern const char *specific_usage;

#define USAGE                                                                                     \
    "usage: %s [OPTIONS] [FILES]\n"                                                               \
    "\n"                                                                                          \
    "%s\n"                                                                                        \
    "\n"                                                                                          \
    "OPTIONS:\n"                                                                                  \
    "  -h, --help         print a brief help message and exit\n"                                  \
    "  -f, --field SPEC   only output fields adhering to SPEC\n"                                  \
    "  -l, --limit N      limit processing to the first N records per file (default: no limit)\n" \
    "  -o, --output FILE  output to FILE (default: stdout)\n"                                     \
    "\n"                                                                                          \
    "Note: if no files are provided this program will read from stdin\n"

int main(int argc, char *argv[])
{
    marcrec rec;
    // 99999 is the max possible size
    char buf[100000];
    // 1000 seems like a reasonable upper bound
    marcfield fields[1000];
    rec.data = buf;
    rec.fields = fields;

    int file_count = 0;
    char **filenames = calloc(argc, sizeof(char *)); // more than we need, but not worth optimizing

    __marc_main_fieldspec_count = 0;
    __marc_main_fieldspec = calloc(argc, sizeof(char *)); // more than we need, but not worth optimizing

    // process command line args
    for (int i = 1; i < argc; i++)
    {
        if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0)
        {
            fprintf(stdout, USAGE, argv[0], specific_usage);
            exit(0);
        }
        else if (strcmp("--field", argv[i]) == 0 || strcmp("-f", argv[i]) == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "error: %s flag requires an argument\n", argv[i]);
                exit(1);
            }
            i++;
            if (strlen(argv[i]) < 3 || !isdigit(argv[i][0]) || !isdigit(argv[i][1]) || !isdigit(argv[i][2]))
            {
                fprintf(stderr, "error: '%s' is an invalid field specifier\n", argv[i]);
                exit(1);
            }
            __marc_main_fieldspec[__marc_main_fieldspec_count++] = argv[i];
        }
        else if (strcmp("--limit", argv[i]) == 0 || strcmp("-l", argv[i]) == 0)
        {
            if (i + 1 >= argc)
            {
                fprintf(stderr, "error: %s flag requires an argument\n", argv[i]);
                exit(1);
            }
            i++;
            __marc_main_limit = atoi(argv[i]);
        }
        else if (strcmp("--output", argv[i]) == 0 || strcmp("-o", argv[i]) == 0)
        {
            if(__marc_main_outfile) {
                fprintf(stderr, "error: more than one output file specified\n");
                exit(1);
            }
            if (i + 1 >= argc)
            {
                fprintf(stderr, "error: %s flag requires an argument\n", argv[i]);
                exit(1);
            }
            i++;
            __marc_main_outfile = argv[i];
        }
        else // we'll assume it's a filename
        {
            filenames[file_count++] = argv[i];
        }
    }

    FILE *out = (__marc_main_outfile) ? fopen(__marc_main_outfile, "w") : stdout;
    if (file_count == 0)
    { // read from stdin
        print_result(out, &rec, "-", stdin);
    }
    else
    {
        for (int i = 0; i < file_count; i++)
        {
            FILE *in = fopen(filenames[i], "r");
            print_result(out, &rec, filenames[i], in);
            fclose(in);
        }
    }

    fclose(out);
    free(filenames);
    free(__marc_main_fieldspec);
}

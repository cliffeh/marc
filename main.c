#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "marc.h"
#include "config.h"

/* process all records by default */
int __marc_main_limit = -1;

/* process all fields by default */
int __marc_main_fieldspec_count;
fieldspec *__marc_main_fieldspecs;

/** input files (stdout if count == 0) */
int __marc_main_infile_count;
const char **__marc_main_infiles;

/* output file (stdout if unspecified) */
const char *__marc_main_outfile;

/* can be set to indicate error status */
int __marc_main_return_code = 0;

#define USAGE                                                                     \
    "usage: marc COMMAND [OPTIONS] [FILES]\n"                                     \
    "\n"                                                                          \
    "COMMANDS:\n"                                                                 \
    "  dump      dump records in marc format\n"                                   \
    "  help      print a brief help message and exit\n"                           \
    "  leaders   print marc leaders\n"                                            \
    "  print     print marc records/fields in a human-readable format\n"          \
    "  validate  validate marc records\n"                                         \
    "  xml       print marc records in XML format\n"                              \
    "\n"                                                                          \
    "OPTIONS:\n"                                                                  \
    "  -h, --help         print a brief help message and exit\n"                  \
    "  -f, --field SPEC   only output fields adhering to SPEC (note: this flag\n" \
    "                     is only used by marc-print)\n"                          \
    "  -l, --limit N      limit processing to the first N records per file\n"     \
    "                     (default: no limit)\n"                                  \
    "  -o, --output FILE  output to FILE (default: stdout)\n"                     \
    "  -V, --version      output version and exit\n"                              \
    "\n"                                                                          \
    "Note: if no input files are provided this program will read from stdin\n"

static void marc_dump(FILE *out, marcrec *rec, int current_file, gzFile in)
{
    if (__marc_main_fieldspec_count != 0)
        fprintf(stderr, "warning: --field flag has been provided and is unused by this command\n");

    int count = 0;
    while (marcrec_read(rec, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        marcrec_write(out, rec);
    }
}

void marc_leaders(FILE *out, marcrec *rec, int current_file, gzFile in)
{
    if (__marc_main_fieldspec_count != 0)
        fprintf(stderr, "warning: --field flag has been provided and is unused by this command\n");

    int count = 0;
    while (marcrec_read(rec, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        fprintf(out, "%.*s\n", 24, rec->data);
    }
}

void marc_print(FILE *out, marcrec *rec, int current_file, gzFile in)
{
    int count = 0;
    while (marcrec_read(rec, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        marcrec_print(out, rec, __marc_main_fieldspec_count ? __marc_main_fieldspecs : 0);
    }
}

void marc_validate(FILE *out, marcrec *rec, int current_file, gzFile in)
{
    if (__marc_main_fieldspec_count != 0)
        fprintf(stderr, "warning: --field flag has been provided and is unused by this command\n");

    int __marc_validate_valid_count = 0;
    int __marc_validate_total_count = 0;
    int count = 0, valid = 0;
    while (marcrec_read(rec, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        if (marcrec_validate(rec) == 0)
            valid++;
        else
            __marc_main_return_code = 1;
    }
    __marc_validate_valid_count += valid;
    __marc_validate_total_count += count;
    fprintf(out, "%s: %i of %i valid (total: %i/%i)\n",
            __marc_main_infiles[current_file], valid, count,
            __marc_validate_valid_count, __marc_validate_total_count);
}

void marc_xml(FILE *out, marcrec *rec, int current_file, gzFile in)
{
    if (__marc_main_fieldspec_count != 0)
        fprintf(stderr, "warning: --field flag has been provided and is unused by this command\n");

    if (current_file == 0)
    {
        fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                     "<collection\n"
                     "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
                     "  xsi:schemaLocation=\"http://www.loc.gov/MARC21/slim http://www.loc.gov/standards/marcxml/schema/MARC21slim.xsd\"\n"
                     "  xmlns=\"http://www.loc.gov/MARC21/slim\">\n");
    }
    int count = 0;
    while (marcrec_read(rec, in) != 0 && (__marc_main_limit - count) != 0)
    {
        count++;
        marcrec_xml(out, rec);
    }
    if (current_file + 1 == __marc_main_infile_count)
    {
        fprintf(out, "</collection>");
    }
}

int main(int argc, char *argv[])
{
    marcrec rec;
    // 99999 is the max possible size
    char buf[100000];
    // 10000 seems like a reasonable upper bound
    marcfield fields[10000];
    rec.data = buf;
    rec.fields = fields;

    void (*action)(FILE *, marcrec *, int, gzFile);

    if (strcmp("dump", argv[1]) == 0)
    {
        action = marc_dump;
    }
    else if (strcmp("help", argv[1]) == 0 || strcmp("--help", argv[1]) == 0 || strcmp("-h", argv[1]) == 0)
    {
        fprintf(stdout, USAGE);
        exit(0);
    }
    else if (strcmp("leaders", argv[1]) == 0)
    {
        action = marc_leaders;
    }
    else if (strcmp("print", argv[1]) == 0)
    {
        action = marc_print;
    }
    else if (strcmp("validate", argv[1]) == 0)
    {
        action = marc_validate;
    }
    else if (strcmp("xml", argv[1]) == 0)
    {
        action = marc_xml;
    }
    else
    {
        fprintf(stderr, "error: unknown action '%s'\n", argv[1]);
        exit(1);
    }

    __marc_main_infile_count = 0;
    __marc_main_infiles = calloc(argc, sizeof(char *)); // more than we need, but not worth optimizing

    __marc_main_fieldspec_count = 0;
    __marc_main_fieldspecs = calloc(argc, sizeof(fieldspec *)); // more than we need, but not worth optimizing

    // process command line args
    for (int i = 2; i < argc; i++)
    {
        if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0)
        {
            fprintf(stdout, USAGE);
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
            __marc_main_fieldspecs[__marc_main_fieldspec_count].tag =
                (argv[i][0] - '0') * 100 + (argv[i][1] - '0') * 10 + argv[i][2] - '0';
            __marc_main_fieldspecs[__marc_main_fieldspec_count++].subfields = argv[i] + 3;
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
            if (__marc_main_outfile)
            {
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
        else if (strcmp("--version", argv[i]) == 0 || strcmp("-V", argv[i]) == 0)
        {
            fprintf(stdout, PACKAGE_STRING "\n");
            exit(0);
        }
        else // we'll assume it's a filename
        {
            __marc_main_infiles[__marc_main_infile_count++] = argv[i];
        }
    }

    FILE *out = (__marc_main_outfile) ? fopen(__marc_main_outfile, "w") : stdout;
    if (__marc_main_infile_count == 0)
    { // read from stdin
        __marc_main_infiles[__marc_main_infile_count++] = "-";
        gzFile in = gzdopen(fileno(stdin), "r");
        action(out, &rec, 0, in);
        gzclose(in);
    }
    else
    {
        for (int i = 0; i < __marc_main_infile_count; i++)
        {
            gzFile in = gzopen(__marc_main_infiles[i], "r");
            action(out, &rec, i, in);
            gzclose(in);
        }
    }

    fclose(out);
    free(__marc_main_infiles);
    free(__marc_main_fieldspecs);
}

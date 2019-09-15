#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <errno.h>
#include "marc.h"

#define DEFAULT_NTHREADS 8

#define USAGE                                                              \
    "usage: marc ACTION [OPTIONS] [FILES] | marc [-h|--help]\n"            \
    "\n"                                                                   \
    "ACTIONS:\n"                                                           \
    "  help      print a brief help message and exit\n"                    \
    "  dump      dump records in their original format\n"                  \
    "  print     print marc records in a human-readable format\n"          \
    "  validate  validate marc records for correctness (default action)\n" \
    "\n"                                                                   \
    "OPTIONS:\n"                                                           \
    "  -h, --help         print a brief help message and exit\n"           \
    "  -f, --field SPEC   only output fields according to SPEC\n"          \
    "  -o, --output FILE  write output to FILE (default: stdout)\n"        \
    "  -t, --threads NUM  run using NUM threads (default: %i)\n"           \
    "Examples:\n\n"                                                        \
    "  # validate records\n"                                               \
    "  marc validate foo.marc\n\n"                                         \
    "  # print all records\n"                                              \
    "  marc print foo.marc\n\n"                                            \
    "  # print out the full 245 field of all records\n"                    \
    "  marc print --field 245 foo.marc\n\n"                                \
    "  # print out the 245a subfield of all records\n"                     \
    "  marc print --field 245a foo.marc\n\n"                               \
    "  # print out the 245 field (subfields a and b, space-delimited)\n"   \
    "  marc print --field 245ab foo.marc\n"

typedef struct arglist
{
    char **infiles, *outfile, *fieldSpec;
    int nThreads, infilePos;
    void *(*action)(marcrec *, int);
} arglist;

/* global state */
pthread_mutex_t infilePos_lock, outfile_lock, stderr_lock;
char **infiles, *fieldSpec;
int infilePos, infileLen;
FILE *outfile;
int **validateCounts;

int get_file_position()
{
    int pos = 0;
    pthread_mutex_lock(&infilePos_lock);
    pos = infilePos++;
    pthread_mutex_unlock(&infilePos_lock);
    return pos;
}

void *action_many_files(void *vargp)
{
    void *(*action)(marcrec *, int) = vargp;
    int pos;
    while ((pos = get_file_position()) < infileLen)
    {
        FILE *in = (strcmp("-", infiles[pos]) == 0) ? stdin : fopen(infiles[pos], "rb");
        if (!in)
        {
            pthread_mutex_lock(&stderr_lock);
            fprintf(stderr, "warning: couldn't open file '%s': %s\n", infiles[pos], strerror(errno));
            pthread_mutex_unlock(&stderr_lock);
        }
        else
        {
            marcrec rec;
            marcfield fields[10000];
            while (marcrec_read(&rec, fields, in) != 0)
            {
                action(&rec, pos);
            }
            fclose(in);
        }
    }

    return 0;
}

void *action_dump(marcrec *rec, int pos)
{
    pthread_mutex_lock(&outfile_lock);
    marcrec_dump(rec, outfile);
    pthread_mutex_unlock(&outfile_lock);

    return 0;
}

void *action_validate(marcrec *rec, int pos)
{

    if (marcrec_validate(rec) == 0)
    {
        validateCounts[pos][0]++;
    }
    validateCounts[pos][1]++;

    return 0;
}

void *action_print(marcrec *rec, int pos)
{
    if (!fieldSpec)
    {
        pthread_mutex_lock(&outfile_lock);
        marcrec_print(rec, outfile);
        pthread_mutex_unlock(&outfile_lock);
    }
    else
    {
        char buf[99999];
        for (int i = 0; i < rec->field_count; i++)
        {
            if (marcfield_match_field(&rec->fields[i], fieldSpec, buf) != 0)
            {
                pthread_mutex_lock(&outfile_lock);
                fprintf(outfile, "%s\n", buf);
                pthread_mutex_unlock(&outfile_lock);
            }
        }
    }

    return 0;
}

void usage_and_exit(int code, const char *fmt, ...)
{
    if (code)
    {
        fprintf(stderr, "error: ");
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, USAGE, DEFAULT_NTHREADS);
    exit(code);
}

int main(int argc, char *argv[])
{
    arglist args;

    // maximum possible number of input files is argc-2
    args.infiles = calloc(argc - 2, sizeof(char *));
    args.infilePos = 0;
    args.infiles[0] = "-";
    args.outfile = 0;
    args.nThreads = DEFAULT_NTHREADS;
    args.action = 0;
    args.fieldSpec = 0;

    if (argc == 1)
    {
        usage_and_exit(1, "no action specified\n");
    }

    // parse action
    if (strcmp("help", argv[1]) == 0)
    {
        usage_and_exit(0, 0);
    }
    else if (strcmp("dump", argv[1]) == 0)
    {
        args.action = action_dump;
    }
    else if (strcmp("print", argv[1]) == 0)
    {
        args.action = action_print;
    }
    else if (strcmp("validate", argv[1]) == 0)
    {
        args.action = action_validate;
    }
    else
    {
        usage_and_exit(1, "unrecognized/unimplemented action '%s'", argv[1]);
    }

    // parse args
    for (int i = 2; i < argc; i++)
    {
        if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0)
        {
            usage_and_exit(0, 0);
        }
        else if (strcmp("-f", argv[i]) == 0 || strcmp("--field", argv[i]) == 0)
        {
            if (i + 1 >= argc)
                usage_and_exit(1, "%s requires an argument", argv[i]);
            if (args.fieldSpec)
                usage_and_exit(1, "%s can only be specified once", argv[i]);
            if (strlen(argv[i + 1]) < 3)
                usage_and_exit(1, "invalid field specified: %s", argv[i + 1]);
            args.fieldSpec = argv[++i];
        }
        else if (strcmp("-o", argv[i]) == 0 || strcmp("--output", argv[i]) == 0)
        {
            if (i + 1 >= argc)
                usage_and_exit(1, "%s requires an argument", argv[i]);
            if (args.outfile)
                usage_and_exit(1, "%s can only be specified once", argv[i]);
            args.outfile = argv[++i];
        }
        else if (strcmp("-t", argv[i]) == 0 || strcmp("--threads", argv[i]) == 0)
        {
            if (i + 1 >= argc)
                usage_and_exit(1, "%s requires an argument", argv[i]);
            args.nThreads = atoi(argv[++i]);
            if (args.nThreads < 1)
                usage_and_exit(1, "%s requires an argument greater than 0", argv[i - 1]);
        }
        else
        {
            args.infiles[args.infilePos++] = argv[i];
        }
    }

    infiles = args.infiles;
    infilePos = 0;
    infileLen = (args.infilePos == 0) ? 1 : args.infilePos;
    outfile = (!args.outfile || (strcmp("-", args.outfile) == 0)) ? stdout : fopen(args.outfile, "w");
    fieldSpec = args.fieldSpec;

    // initialize threads & locks
    pthread_t *threads = calloc(args.nThreads, sizeof(pthread_t));
    pthread_mutex_init(&infilePos_lock, 0);
    pthread_mutex_init(&outfile_lock, 0);

    // initialize counters
    validateCounts = calloc(infileLen, sizeof(int *));
    for (int i = 0; i < infileLen; i++)
    {
        validateCounts[i] = calloc(2, sizeof(int));
    }

    // start 'em up
    for (int i = 0; i < args.nThreads; i++)
    {
        pthread_create(&threads[i], 0, action_many_files, args.action);
    }

    // wait for 'em to finish
    for (int i = 0; i < args.nThreads; i++)
    {
        pthread_join(threads[i], 0);
    }

    // cleanup
    for (int i = 0; i < infileLen; i++)
    {
        free(validateCounts[i]);
    }
    free(validateCounts);
    free(threads);
    free(args.infiles);

    return 0;
}

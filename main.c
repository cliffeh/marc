#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include "marc.h"

#define DEFAULT_NTHREADS 8

#define ACTION_PRINT 1
#define ACTION_VALIDATE 2

#define USAGE                                                                                       \
    "usage: marc ACTION [OPTIONS] | marc [-h|--help]\n"                                             \
    "\n"                                                                                            \
    "ACTIONS:\n"                                                                                    \
    "  help      print a brief help message and exit\n"                                             \
    "  print     pretty-print marc records\n"                                                       \
    "  validate  validate marc records for correctness (default action)\n"                          \
    "\n"                                                                                            \
    "OPTIONS:\n"                                                                                    \
    "  -h, --help         print a brief help message and exit\n"                                    \
    "  -i, --input  FILE  read input from FILE; can be specified multiple times (default: stdin)\n" \
    "  -o, --output FILE  write output to FILE (default: stdout)\n"                                 \
    "  -t, --threads NUM  run using NUM threads (default: %i)\n"

typedef struct arglist
{
    char **infiles, *outfile;
    int action, nThreads, infilePos;
} arglist;

/* global state */
pthread_mutex_t infilePos_lock, outfile_lock;
char **infiles;
int infilePos, infileLen;
FILE *outfile;
int **validateCounts;

void *action_validate(void *vargp)
{
    while(1) {
        pthread_mutex_lock(&infilePos_lock);
        if(infilePos == infileLen) {
            pthread_mutex_unlock(&infilePos_lock);
            return 0;
        }

        int pos = infilePos;

        // fprintf(stderr, "opening file: %s\n", infiles[infilePos]);
        FILE *in = fopen(infiles[infilePos++], "rb");
        pthread_mutex_unlock(&infilePos_lock);

        marcrec rec;
        while(marcrec_read(&rec, in) != 0) {
            if (marcrec_validate(&rec) == 0)
            {
                validateCounts[pos][0]++;
            }
            validateCounts[pos][1]++;
        }

        fclose(in);

        pthread_mutex_lock(&outfile_lock);
        fprintf(outfile, "%s: %i/%i valid records\n", infiles[pos], validateCounts[pos][0], validateCounts[pos][1]);
        pthread_mutex_unlock(&outfile_lock);

    }

    return 0;
}

void *action_print(void *vargp)
{

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

    // maximum possible number of input files is effectively argc/2
    args.infiles = calloc(argc / 2, sizeof(char *));
    args.infilePos = 0;
    args.infiles[0] = "-";
    args.outfile = 0;
    args.nThreads = DEFAULT_NTHREADS;
    args.action = 0;

    // fprintf(stderr, USAGE, DEFAULT_NTHREADS);
    // int nThreads = DEFAULT_NTHREADS, action = 0;
    // parse args
    for (int i = 1; i < argc; i++)
    {
        if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0)
        {
            usage_and_exit(0, 0);
        }
        else if (strcmp("-i", argv[i]) == 0 || strcmp("--input", argv[i]) == 0)
        {
            if (i + 1 >= argc)
                usage_and_exit(1, "%s requires an argument", argv[i]);
            args.infiles[args.infilePos++] = argv[++i];
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
        { // actions!
            if (strcmp("help", argv[i]) == 0)
            {
                usage_and_exit(0, 0);
            }
            else if (strcmp("print", argv[i]) == 0)
            {
                if (args.action)
                    usage_and_exit(1, "multiple actions specified", argv[i - 1]);
                args.action = ACTION_PRINT;
            }
            else if (strcmp("validate", argv[i]) == 0)
            {
                if (args.action)
                    usage_and_exit(1, "multiple actions specified", argv[i - 1]);
                args.action = ACTION_VALIDATE;
            }
        }
    }

    // TODO maybe have a default action?
    if (!args.action)
        usage_and_exit(1, "no action specified");

    infiles = args.infiles;
    infilePos = 0;
    infileLen = (args.infilePos == 0) ? 1 : args.infilePos;
    outfile = (!args.outfile || (strcmp("-", args.outfile) == 0)) ? stdout : fopen(args.outfile, "w");

    pthread_t *threads = calloc(args.nThreads, sizeof(pthread_t));
    pthread_mutex_init(&infilePos_lock, 0);
    pthread_mutex_init(&outfile_lock, 0);

    switch (args.action)
    {
    case ACTION_PRINT:
    {
        fprintf(stderr, "unimplemented\n");
    }
    break;
    case ACTION_VALIDATE:
    {
        validateCounts = calloc(infileLen, sizeof(int *));
        for (int i = 0; i < infileLen; i++)
        {
            validateCounts[i] = calloc(2, sizeof(int));
        }

        for(int i = 0; i < args.nThreads; i++) {
            pthread_create(&threads[i], 0, action_validate, 0);
        }

        for(int i = 0; i < args.nThreads; i++) {
            // fprintf(stderr, "joining thread %i\n", i);
            pthread_join(threads[i], 0);
        }

        int totals[2] = { 0, 0 };
        for(int i = 0; i < infileLen; i++) {
            totals[0] += validateCounts[i][0];
            totals[1] += validateCounts[i][1];
        }
        fprintf(outfile, "TOTAL: %i/%i valid records\n", totals[0], totals[1]);
        
        for (int i = 0; i < infileLen; i++)
        {
            free(validateCounts[i]);
        }
        free(validateCounts);
    }
    break;
    default:
    {
        // this shouldn't happen...
        usage_and_exit(1, "invalid action specified");
    }
    }

    // cleanup
    free(threads);

    return 0;
}
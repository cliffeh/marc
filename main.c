#include "config.h"
#include "marc.h"
#include "popt/popt.h"
#include "util.h"
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_TYPE_NONE 0
#define OUTPUT_TYPE_HUMAN 1
#define OUTPUT_TYPE_MARC 2
#define OUTPUT_TYPE_XML 3

int
main (int argc, const char *argv[])
{
  int rc, limit = -1, output_type = OUTPUT_TYPE_HUMAN, stdin_already_used = 0,
          validate = 0, verbose = 0;
  const char *defaultArgs[2] = { "-", 0 }, **args, *arg;
  char *format = "human", *outfile = "-", *logfile = 0;
  FILE *out = stdout, *log = stderr;

  poptContext optCon;

  struct poptOption options[] = {
    /* longName, shortName, argInfo, arg, val, descrip, argDescript */
    { "format", 'F', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &format, 'F',
      "output format; can be one of: n[one], h[uman], m[arc], x[ml]", 0 },
    { "logfile", 'l', POPT_ARG_STRING, &logfile, 'l',
      "log to FILE (default: stderr)", "FILE" },
    { "limit", 'L', POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &limit, 'L',
      "maximum number of records to process; -1 means process all "
      "available records",
      0 },
    { "output", 'o', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &outfile,
      'o', "output to FILE", "FILE" },
    { "validate", 'V', POPT_ARG_NONE, &validate, 'V',
      "log record validation statistics", 0 },
    { "verbose", 'v', POPT_ARG_NONE, &verbose, 'v', "enable verbose logging",
      0 },
    { "version", 0, POPT_ARG_NONE, 0, 'Z', "show version information and exit",
      0 },
    POPT_AUTOHELP POPT_TABLEEND
  };

  optCon = poptGetContext (0, argc, argv, options, 0);
  poptSetOtherOptionHelp (optCon, "[OPTION...] [FILE...]\nWill read from "
                                  "stdin if no FILEs are provided.\nOptions:");

  while ((rc = poptGetNextOpt (optCon)) > 0)
    {
      switch (rc)
        {
        case 'Z':
          {
            printf (PACKAGE_STRING);
            poptFreeContext (optCon);
            exit (0);
          }
        }
    }

  if (rc != -1)
    {
      fprintf (stderr, "error: %s: %s\n",
               poptBadOption (optCon, POPT_BADOPTION_NOALIAS),
               poptStrerror (rc));
      poptPrintHelp (optCon, stderr, 0);
      poptFreeContext (optCon);
      exit (1);
    }

  if (strcmp ("-", outfile) != 0)
    {
      if (!(out = fopen (outfile, "w")))
        {
          fprintf (stderr, "fatal: couldn't open output file '%s' (%s)\n", arg,
                   strerror (errno));
          // TODO cleanup
          exit (1);
        }
    }

  if (logfile)
    {
      if (strcmp (outfile, logfile) == 0)
        {
          fprintf (stderr,
                   "warning: output and logs being written to the same file "
                   "'%s'!\n",
                   outfile);
        }

      if (strcmp ("-", logfile) == 0)
        {
          log = stdout;
        }
      else
        {
          if (!(log = fopen (logfile, "w")))
            {
              fprintf (stderr, "fatal: couldn't open log file '%s' (%s)\n",
                       arg, strerror (errno));
              // TODO cleanup
              exit (1);
            }
        }
    }

  if ((strcmp ("h", format) == 0) || (strcmp ("human", format) == 0))
    {
      output_type = OUTPUT_TYPE_HUMAN;
    }
  else if ((strcmp ("m", format) == 0) || (strcmp ("marc", format) == 0))
    {
      output_type = OUTPUT_TYPE_MARC;
    }
  else if ((strcmp ("n", format) == 0) || (strcmp ("none", format) == 0))
    {
      output_type = OUTPUT_TYPE_NONE;
    }
  else if ((strcmp ("x", format) == 0) || (strcmp ("xml", format) == 0))
    {
      output_type = OUTPUT_TYPE_XML;
    }
  else
    {
      fprintf (stderr, "error: unknown format type '%s'\n", format);
      poptPrintHelp (optCon, stderr, 0);
      poptFreeContext (optCon);
      exit (1);
    }

  if (verbose)
    {
      fprintf (log, "verbose logging enabled\n");
      fprintf (log, "output file: %s\n", outfile);
      fprintf (log, "limit: %i\n", limit);
      fprintf (log, "validate: %s\n", validate ? "yes" : "no");
    }

  if (!(args = poptGetArgs (optCon)))
    args = defaultArgs;

  if (output_type == OUTPUT_TYPE_XML)
    fprintf (out, "%s\n", MARC_XML_PREAMBLE);

  for (; arg = *args; args++)
    {
      marcfile *in;

      // max record size is 99999, and 10000 seems like a conservative upper
      // bound for the number of fields any given record is likely to to
      // contain
      marcrec *rec = marcrec_alloc (100000, 10000);
      int current_count, total_count = 0, valid_records;

      rc = 0;

      if (verbose)
        fprintf (log, "processing file: %s\n", arg);

      // protect against trying to read from stdin more than once
      if (strcmp ("-", arg) == 0)
        {
          if (stdin_already_used)
            {
              if (verbose)
                fprintf (log, "skipping %s (stdin already read)\n", arg);
              continue;
            }
          else
            {
              in = marcfile_from_FILE (stdin);
              stdin_already_used = 1;
            }
        }
      else
        {
          if (!(in = marcfile_open (arg)))
            {
              fprintf (log,
                       "error: couldn't open file '%s' (%s); skipping...\n",
                       arg, strerror (errno));

              continue;
            }
        }

      current_count = 0;
      valid_records = 0;
      while (marcrec_read (rec, in) != 0 && (limit - total_count) != 0)
        {
          current_count++;
          total_count++;

          if (validate && rec->vflags == 0)
            {
              valid_records++;
            }

          switch (output_type)
            {
            case OUTPUT_TYPE_HUMAN:
              {
                // TODO include fieldspec (when available)
                marcrec_print (out, rec, 0);
              }
              break;

            case OUTPUT_TYPE_MARC:
              {
                marcrec_write (out, rec);
              }
              break;

            case OUTPUT_TYPE_XML:
              {
                marcrec_xml (out, rec);
              }
              break;

            case OUTPUT_TYPE_NONE: // no-op
            }
        }

      if (validate)
        {
          fprintf (log, "%s: %i/%i valid records\n", arg, valid_records,
                   current_count);
        }

      marcfile_close (in);
    }

  if (output_type == OUTPUT_TYPE_XML)
    fprintf (out, "%s\n", MARC_XML_POSTAMBLE);

  // clean up and exit
  fclose (out);
  poptFreeContext (optCon);

  // TODO maybe some better logic around specific return codes
  return rc;
}

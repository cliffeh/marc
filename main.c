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

#ifndef MAX_FIELDSPECS
#define MAX_FIELDSPECS 256
#endif

int
marcrec_print_noop (FILE *out, const marcrec *rec, const char *specs[])
{
  return 0;
}

int
main (int argc, const char *argv[])
{
  int rc, field_count = 0, limit = -1, stdin_already_used = 0, validate = 0,
          verbose = 0;
  const char *defaultArgs[2] = { "-", 0 }, *specs[MAX_FIELDSPECS + 1] = { 0 },
             **infiles, *infile;
  char *field, *format = "human", *outfile = "-", *logfile = 0;
  FILE *out = stdout, *log = stderr;

  int (*print_action) (FILE *, const marcrec *, const char **)
      = &marcrec_print;

  poptContext optCon;

  struct poptOption options[] = {
    /* longName, shortName, argInfo, arg, val, descrip, argDescript */
    { "field", 'f', POPT_ARG_STRING, &field, 'f',
      "only print fields adhering to FIELDSPEC (requires human-readable "
      "output format)",
      "FIELDSPEC" },
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
        case 'f':
          {
            if (field_count >= MAX_FIELDSPECS)
              {
                fprintf (log,
                         "warning: maximum number of fields (%i) specified; "
                         "dropping %s\n",
                         MAX_FIELDSPECS, field);
                free (field);
                continue;
              }

            if (!MARC_VALID_FIELDSPEC (field))
              {
                fprintf (stderr, "error: invalid fieldspec: %s\n", field);
                exit (1);
              }
            specs[field_count++] = field;
          }
          break;
        case 'F':
          {
            if ((strcasecmp ("h", format) == 0)
                || (strcasecmp ("human", format) == 0))
              {
                print_action = &marcrec_print;
              }
            else if ((strcasecmp ("m", format) == 0)
                     || (strcasecmp ("marc", format) == 0))
              {
                print_action = &marcrec_write;
              }
            else if ((strcasecmp ("n", format) == 0)
                     || (strcasecmp ("none", format) == 0))
              {
                print_action = &marcrec_print_noop;
              }
            else if ((strcasecmp ("x", format) == 0)
                     || (strcasecmp ("xml", format) == 0))
              {
                print_action = &marcrec_xml;
              }
            else
              {
                fprintf (stderr, "error: unknown format type '%s'\n", format);
                poptPrintHelp (optCon, stderr, 0);
                poptFreeContext (optCon);
                exit (1);
              }
            free (format);
          }
          break;
        case 'l':
          {
            if (strcmp ("-", logfile) == 0)
              {
                log = stdout;
              }
            else
              {
                if (!(log = fopen (logfile, "w")))
                  {
                    fprintf (stderr,
                             "fatal: couldn't open log file '%s' (%s)\n",
                             logfile, strerror (errno));
                    exit (1);
                  }
              }
            free (logfile);
          }
          break;
        case 'o':
          {
            if (strcmp ("-", outfile) == 0)
              {
                out = stdout;
              }
            else
              {
                if (!(out = fopen (outfile, "w")))
                  {
                    fprintf (stderr,
                             "fatal: couldn't open output file '%s' (%s)\n",
                             outfile, strerror (errno));
                    exit (1);
                  }
              }
            free (outfile);
          }
          break;
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

  // ensure null termination (paranoia?)
  specs[field_count] = 0;

  if (verbose)
    {
      fprintf (log, "verbose logging enabled\n");
      fprintf (log, "output file: %s\n", outfile);
      fprintf (log, "limit: %i\n", limit);
      fprintf (log, "validate: %s\n", validate ? "yes" : "no");
      for (int i = 0; i < field_count; i++)
        {
          fprintf (log, "desired field: %s\n", specs[i]);
        }
    }

  if (!(infiles = poptGetArgs (optCon)))
    infiles = defaultArgs;

  if (print_action == &marcrec_xml)
    fprintf (out, "%s\n", MARC_XML_PREAMBLE);

  if (field_count > 0 && print_action != &marcrec_print)
    {
      fprintf (log, "warning: fields specified with non-human-readable output "
                    "format; will be ignored...\n");
    }

  // max record size is 99999, and 10000 seems like a conservative upper
  // bound for the number of fields any given record is likely to to
  // contain
  marcrec *rec = marcrec_alloc (100000, 10000);
  for (; infile = *infiles; infiles++)
    {
      marcfile *in;
      int current_count, total_count = 0, valid_records;

      rc = 0;

      if (verbose)
        fprintf (log, "processing file: %s\n", infile);

      // protect against trying to read from stdin more than once
      if (strcmp ("-", infile) == 0)
        {
          if (stdin_already_used)
            {
              if (verbose)
                fprintf (log, "skipping %s (stdin already read)\n", infile);
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
          if (!(in = marcfile_open (infile)))
            {
              fprintf (log,
                       "error: couldn't open file '%s' (%s); skipping...\n",
                       infile, strerror (errno));

              continue;
            }
        }

      current_count = 0;
      valid_records = 0;
      while (marcrec_read (rec, in) != 0 && (limit - total_count) != 0)
        {
          current_count++;
          total_count++;

          if (validate && marcrec_validate (rec) == 0)
            {
              valid_records++;
            }

          print_action (out, rec, specs);
        }

      if (validate)
        {
          fprintf (log, "%s: %i/%i valid records\n", infile, valid_records,
                   current_count);
        }

      marcfile_close (in);
    }

  if (print_action == &marcrec_xml)
    fprintf (out, "%s\n", MARC_XML_POSTAMBLE);

  // clean up and exit
  fclose (out);
  fclose (log);
  marcrec_free (rec);
  poptFreeContext (optCon);

  // TODO maybe some better logic around specific return codes
  exit (rc);
}

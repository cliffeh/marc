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

static int
marc_dump (FILE *out, marcrec *rec, fieldspec specs[])
{
  marcrec_write (out, rec);
  return 1;
}

static int
marc_leaders (FILE *out, marcrec *rec, fieldspec specs[])
{
  fprintf (out, "%.*s\n", 24, rec->data);
  return 1;
}

static int
marc_print (FILE *out, marcrec *rec, fieldspec specs[])
{
  marcrec_print (out, rec, specs);
  return 1;
}

static int
marc_validate (FILE *out, marcrec *rec, fieldspec specs[])
{
  // TODO have main pass in the verbose file, or make it a global var
  return (marcrec_validate (rec) == 0) ? 1 : 0;
}

int
main (int argc, const char *argv[])
{
  int rc, limit = -1, output_type = OUTPUT_TYPE_HUMAN, stdin_already_used = 0;
  const char *defaultArgs[2] = { "-", 0 }, **args, *arg;
  char *format = "human", *outfile = "-", *logfile = 0;
  FILE *out = stdout, *log = stderr;

  poptContext optCon;

  struct poptOption options[] = {
    /* longName, shortName, argInfo, arg, val, descrip, argDescript */
    { "format", 'F', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &format, 'F',
      "output format; can be one of: n[one], h[uman], m[arc], x[ml]", 0 },
    { "limit", 'L', POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &limit, 'L',
      "maximum number of records to process; -1 means process all "
      "available records",
      0 },
    { "logfile", 'l', POPT_ARG_STRING, &logfile, 'l',
      "log to FILE (default: stderr)", "FILE" },
    { "output", 'o', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &outfile,
      'o', "output to FILE", "FILE" },
    POPT_AUTOHELP POPT_TABLEEND
  };

  optCon = poptGetContext (0, argc, argv, options, 0);
  poptSetOtherOptionHelp (optCon, "[OPTION...] [FILE...]\nWill read from "
                                  "stdin if no FILEs are provided.\nOptions:");

  while ((rc = poptGetNextOpt (optCon)) > 0)
    {
      // printf ("opt: %c\n", (char)rc);
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
      int current_count, total_count = 0;

      rc = 0;

      // protect against trying to read from stdin more than once
      if (strcmp ("-", arg) == 0)
        {
          if (stdin_already_used)
            {
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
              // TODO log file?
              fprintf (stderr,
                       "error: couldn't open file '%s' (%s); skipping...\n",
                       arg, strerror (errno));

              continue;
            }
        }

      current_count = 0;
      while (marcrec_read (rec, in) != 0 && (limit - total_count) != 0)
        {
          current_count++;
          total_count++;

          // TODO implement validation

          switch (output_type)
            {
            case OUTPUT_TYPE_HUMAN:
              {
                // TODO allow specifying an output file!
                marc_print (out, rec, 0);
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

      marcfile_close (in);
    }

  if (output_type == OUTPUT_TYPE_XML)
    fprintf (out, "%s\n", MARC_XML_POSTAMBLE);

  /*
  void (*preamble) (FILE *) = 0;
  int (*action) (FILE *, marcrec *, fieldspec *) = 0;
  void (*postamble) (FILE *) = 0;

  if (argc == 1)
    {
      fprintf (stderr, USAGE);
      exit (1);
    }

  if (strcmp ("dump", argv[1]) == 0)
    {
      action = marc_dump;
    }
  else if (strcmp ("help", argv[1]) == 0 || strcmp ("--help", argv[1]) == 0
           || strcmp ("-h", argv[1]) == 0)
    {
      fprintf (stdout, USAGE);
      exit (0);
    }
  else if (strcmp ("leaders", argv[1]) == 0)
    {
      action = marc_leaders;
    }
  else if (strcmp ("print", argv[1]) == 0)
    {
      action = marc_print;
    }
  else if (strcmp ("validate", argv[1]) == 0)
    {
      action = marc_validate;
    }
  else if (strcmp ("version", argv[1]) == 0
           || strcmp ("--version", argv[1]) == 0
           || strcmp ("-V", argv[1]) == 0)
    {
      fprintf (stdout, PACKAGE_STRING "\n");
      exit (0);
    }
  else if (strcmp ("xml", argv[1]) == 0)
    {
      preamble = xml_preamble;
      action = marc_xml;
      postamble = xml_postamble;
    }
  else
    {
      fprintf (stderr, "error: unknown action '%s'\n", argv[1]);
      exit (1);
    }

  int already_using_stdin = 0, limit = -1, infile_count = 0,
      fieldspec_count = 0, verbose = 0;
  FILE *out = 0, *log = 0;


  // max record size is 99999, and 10000 seems like a conservative upper
  bound
  // for the number of fields any given record is likely to to contain
  marcrec *rec = marcrec_alloc (100000, 10000);

  if (fieldspec_count == 0)
    {
      free (specs);
      specs = 0;
    }
  else if (action != marc_print)
    {
      fprintf (stderr, "warning: field specifiers are ignored by actions "
                       "other than print\n");
    }

  if (!out)
    out = stdout;
  if (!log)
    log = stderr;
  if (infile_count == 0)
    {
      infiles[infile_count++] = "-";
    }

  if (preamble)
    preamble (out);

  int total_valid = 0, total_count = 0, rc = 0;
  for (int i = 0; i < infile_count; i++)
    {
      marcfile *in = (strcmp ("-", infiles[i]) == 0)
                         ? marcfile_from_FILE (stdin)
                         : marcfile_open (infiles[i]);
      if (!in)
        {
          fprintf (stderr, "error: could not open '%s': %s\n", infiles[i],
                   strerror (errno));
        }
      else
        {
          int valid = 0, count = 0;
          while (marcrec_read (rec, in) != 0 && (limit - count) != 0)
            {
              count++;
              valid += action (out, rec, specs);
            }
          char errbuf[1024];
          if (marcfile_error (in, errbuf))
            {
              rc = 1;
              fprintf (stderr, "error: %s\n", errbuf);
            }

          marcfile_close (in);

          total_valid += valid;
          total_count += count;

          if (verbose)
            {
              fprintf (log, "%s: %i/%i (total: %i/%i)\n", infiles[i],
  valid, count, total_valid, total_count);
            }
        }
    }

  if (postamble)
    postamble (out);

  fclose (out);
  free (infiles);
  if (fieldspec_count > 0)
    free (specs);
  marcrec_free (rec);

  return rc ? rc : ((total_valid == total_count) ? 0 : 1);
  */

  // clean up and exit
  fclose (out);
  poptFreeContext (optCon);

  // TODO maybe some better logic around specific return codes
  return rc;
}

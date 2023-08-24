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

#define USAGE                                                                  \
  "usage: marc COMMAND [OPTIONS] [FILES]\n"                                    \
  "\n"                                                                         \
  "COMMANDS:\n"                                                                \
  "  dump      dump records in marc format\n"                                  \
  "  help      print a brief help message and exit\n"                          \
  "  leaders   print marc leaders\n"                                           \
  "  print     print marc records/fields in a human-readable format\n"         \
  "  validate  validate marc records\n"                                        \
  "  xml       print marc records in XML format\n"                             \
  "\n"                                                                         \
  "OPTIONS:\n"                                                                 \
  "  -h, --help         print a brief help message and exit\n"                 \
  "  -f, --field SPEC   only output fields adhering to SPEC (note: this "      \
  "flag\n"                                                                     \
  "                     is only used by `marc print`)\n"                       \
  "  -l, --limit N      limit processing to the first N records per file\n"    \
  "                     (default: no limit)\n"                                 \
  "  -o, --output FILE  output to FILE (default: stdout)\n"                    \
  "  -v, --verbose      turn on verbose logging\n"                             \
  "  -V, --version      output version and exit\n"                             \
  "\n"                                                                         \
  "Note: if no input files are provided this program will read from stdin\n"

static int marc_dump(FILE *out, marcrec *rec, fieldspec specs[]) {
  marcrec_write(out, rec);
  return 1;
}

static int marc_leaders(FILE *out, marcrec *rec, fieldspec specs[]) {
  fprintf(out, "%.*s\n", 24, rec->data);
  return 1;
}

static int marc_print(FILE *out, marcrec *rec, fieldspec specs[]) {
  marcrec_print(out, rec, specs);
  return 1;
}

static int marc_validate(FILE *out, marcrec *rec, fieldspec specs[]) {
  // TODO have main pass in the verbose file, or make it a global var
  return (marcrec_validate(rec) == 0) ? 1 : 0;
}

static void xml_preamble(FILE *out) {
  fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
               "<collection\n"
               "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
               "  xsi:schemaLocation=\"http://www.loc.gov/MARC21/slim "
               "http://www.loc.gov/standards/marcxml/schema/MARC21slim.xsd\"\n"
               "  xmlns=\"http://www.loc.gov/MARC21/slim\">\n");
}

static int marc_xml(FILE *out, marcrec *rec, fieldspec specs[]) {
  marcrec_xml(out, rec);
  return 1;
}

static void xml_postamble(FILE *out) { fprintf(out, "</collection>"); }

int main(int argc, char *argv[]) {
  void (*preamble)(FILE *) = 0;
  int (*action)(FILE *, marcrec *, fieldspec *) = 0;
  void (*postamble)(FILE *) = 0;

  if (argc == 1) {
    fprintf(stderr, USAGE);
    exit(1);
  }

  if (strcmp("dump", argv[1]) == 0) {
    action = marc_dump;
  } else if (strcmp("help", argv[1]) == 0 || strcmp("--help", argv[1]) == 0 ||
             strcmp("-h", argv[1]) == 0) {
    fprintf(stdout, USAGE);
    exit(0);
  } else if (strcmp("leaders", argv[1]) == 0) {
    action = marc_leaders;
  } else if (strcmp("print", argv[1]) == 0) {
    action = marc_print;
  } else if (strcmp("validate", argv[1]) == 0) {
    action = marc_validate;
  } else if (strcmp("version", argv[1]) == 0 ||
             strcmp("--version", argv[1]) == 0 || strcmp("-V", argv[1]) == 0) {
    fprintf(stdout, PACKAGE_STRING "\n");
    exit(0);
  } else if (strcmp("xml", argv[1]) == 0) {
    preamble = xml_preamble;
    action = marc_xml;
    postamble = xml_postamble;
  } else {
    fprintf(stderr, "error: unknown action '%s'\n", argv[1]);
    exit(1);
  }

  int already_using_stdin = 0, limit = -1, infile_count = 0,
      fieldspec_count = 0, verbose = 0;
  FILE *out = 0, *log = 0;
  char **infiles = calloc(
      argc, sizeof(char *)); // more than we need, but not worth optimizing
  fieldspec *specs = calloc(
      argc, sizeof(fieldspec *)); // more than we need, but not worth optimizing

  // process command line args
  for (int i = 2; i < argc; i++) {
    if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
      fprintf(stdout, USAGE);
      exit(0);
    } else if (strcmp("--field", argv[i]) == 0 || strcmp("-f", argv[i]) == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "error: %s flag requires an argument\n", argv[i]);
        exit(1);
      }
      i++;
      if (strlen(argv[i]) < 3 || !isdigit(argv[i][0]) || !isdigit(argv[i][1]) ||
          !isdigit(argv[i][2])) {
        fprintf(stderr, "error: '%s' is an invalid field specifier\n", argv[i]);
        exit(1);
      }
      specs[fieldspec_count].tag = atoin(argv[i], 3);
      specs[fieldspec_count++].subfields = argv[i] + 3;
    } else if (strcmp("--limit", argv[i]) == 0 || strcmp("-l", argv[i]) == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "error: %s flag requires an argument\n", argv[i]);
        exit(1);
      }
      i++;
      limit = atoi(argv[i]);
    } else if (strcmp("--output", argv[i]) == 0 || strcmp("-o", argv[i]) == 0) {
      if (out) {
        fprintf(stderr, "error: more than one output file specified\n");
        exit(1);
      }
      if (i + 1 >= argc) {
        fprintf(stderr, "error: %s flag requires an argument\n", argv[i]);
        exit(1);
      }
      i++;
      out = fopen(argv[i], "w");
    } else if (strcmp("--verbose", argv[i]) == 0 ||
               strcmp("-v", argv[i]) == 0) {
      verbose = 1;
    } else if (strcmp("--version", argv[i]) == 0 ||
               strcmp("-V", argv[i]) == 0) {
      fprintf(stdout, PACKAGE_STRING "\n");
      exit(0);
    } else // we'll assume it's a filename
    {
      // protect against multiple uses of stdin
      if (strcmp("-", argv[i]) == 0) {
        if (already_using_stdin) {
          continue;
        } else {
          already_using_stdin = 1;
        }
      }
      infiles[infile_count++] = argv[i];
    }
  }

  // max record size is 99999, and 10000 seems like a conservative upper bound
  // for the number of fields any given record is likely to to contain
  marcrec *rec = marcrec_alloc(100000, 10000);

  if (fieldspec_count == 0) {
    free(specs);
    specs = 0;
  } else if (action != marc_print) {
    fprintf(
        stderr,
        "warning: field specifiers are ignored by actions other than print\n");
  }

  if (!out)
    out = stdout;
  if (!log)
    log = stderr;
  if (infile_count == 0) {
    infiles[infile_count++] = "-";
  }

  if (preamble)
    preamble(out);

  int total_valid = 0, total_count = 0, rc = 0;
  for (int i = 0; i < infile_count; i++) {
    marcfile *in = (strcmp("-", infiles[i]) == 0) ? marcfile_from_FILE(stdin)
                                                  : marcfile_open(infiles[i]);
    if (!in) {
      fprintf(stderr, "error: could not open '%s': %s\n", infiles[i],
              strerror(errno));
    } else {
      int valid = 0, count = 0;
      while (marcrec_read(rec, in) != 0 && (limit - count) != 0) {
        count++;
        valid += action(out, rec, specs);
      }
      char errbuf[1024];
      if (marcfile_error(in, errbuf)) {
        rc = 1;
        fprintf(stderr, "error: %s\n", errbuf);
      }

      marcfile_close(in);

      total_valid += valid;
      total_count += count;

      if (verbose) {
        fprintf(log, "%s: %i/%i (total: %i/%i)\n", infiles[i], valid, count,
                total_valid, total_count);
      }
    }
  }

  if (postamble)
    postamble(out);

  fclose(out);
  free(infiles);
  if (fieldspec_count > 0)
    free(specs);
  marcrec_free(rec);

  return rc ? rc : ((total_valid == total_count) ? 0 : 1);
}

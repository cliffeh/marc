#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include "marc.h"
#include "util.h"

marcrec *marcrec_read(marcrec *rec, FILE *in)
{
  if (fread(rec->raw, sizeof(char), 24, in) == 0)
    return 0;

  // total length of record
  rec->length = char_to_int(rec->raw, 5);

  // length of leader + directory
  rec->base_address = char_to_int(rec->raw + 12, 5);

  // read the remainder of the record
  fread(rec->raw + 24, sizeof(char), rec->length - 24, in);

  return rec;
}

void marcrec_walk_fields(const marcrec *rec, void (*f)(const char *, const char *, int, void *), void *arg)
{
  for (int i = 24; i < rec->base_address - 1; i += 12)
  {
    int len = char_to_int(rec->raw + i + 3, 4);
    int pos = char_to_int(rec->raw + i + 7, 5);

    f(rec->raw + i, rec->raw + rec->base_address + pos, len, arg);
  }
}

int marcrec_validate(const marcrec *rec)
{
  int r = 0;
  if (rec->raw[rec->length - 1] != RECORD_TERMINATOR)
    r |= MISSING_RECORD_TERMINATOR;
  if (rec->raw[rec->base_address - 1] != FIELD_TERMINATOR)
    r |= MISSING_FIELD_TERMINATOR;

  marcrec_walk_fields(rec, marc_validate_field, (void *)&r);
  return r;
}

void marc_validate_field(const char *dir_entry, const char *data, int nbytes, void *retPtr)
{
  int *r = (int *)retPtr;
  if (data[nbytes - 1] != FIELD_TERMINATOR)
    *r |= MISSING_FIELD_TERMINATOR;
}

void marcrec_print(const marcrec *rec, FILE *out)
{
  fprintf(out, "length: %05i | status: %c | type: %c | bibliographic level: %c | type of control: %c\n",
          rec->length, rec->raw[5], rec->raw[6], rec->raw[7], rec->raw[9]);
  fprintf(out, "character coding scheme: %c | indicator count: %c | subfield code count: %c\n",
          rec->raw[10], rec->raw[11], rec->raw[12]);
  fprintf(out, "base address of data: %05i | encoding level: %c | descriptive cataloging form: %c\n",
          rec->base_address, rec->raw[17], rec->raw[18]);
  fprintf(out, "multipart resource record level: %c | length of the length-of-field portion: %c\n",
          rec->raw[19], rec->raw[20]);
  fprintf(out, "length of the staring-character-position portion: %c | length of the implementation-defined portion: %c\n",
          rec->raw[21], rec->raw[22]);

  fprintf(out, "variable-length fields:\n");
  marcrec_walk_fields(rec, marc_print_field, (void *)out);
}

void marc_print_field(const char *dir_entry, const char *data, int nbytes, void *outPtr)
{
  FILE *out = (FILE *)outPtr;

  // print the tag
  fprintf(out, "  %.*s: ", 3, dir_entry);

  // 4-digit field length means a max size of 9999
  char buf[10000], *p = buf;

  for (int i = 0; i < nbytes; i++)
  {
    switch (data[i])
    {
    // replace subfield delimiters with a human-readable format
    case SUBFIELD_DELIMITER:
    {
      *p++ = ' ';
      *p++ = '$';
      *p++ = data[++i];
      *p++ = ':';
      *p++ = ' ';
    }
    break;
    case FIELD_TERMINATOR:
    { // do not print
    }
    break;
    default:
      *p++ = data[i];
    }
  }

  *p = 0;

  fprintf(out, "%s\n", buf);
}

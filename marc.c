#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include "marc.h"
#include "util.h"

static marcfield *marcrec_process_fields(marcrec *rec, marcfield *fields)
{
  for (int i = 0; i < rec->field_count; i++)
  {
    fields[i].directory_entry = rec->raw + 24 + i * 12;
    fields[i].len = char_to_int(fields[i].directory_entry + 3, 4);
    fields[i].data = rec->raw + rec->base_address + char_to_int(fields[i].directory_entry + 7, 5);
  }
  return fields;
}

marcrec *marcrec_read(marcrec *rec, marcfield *fields, FILE *in)
{
  if (fread(rec->raw, sizeof(char), 24, in) == 0)
    return 0;

  // total length of record
  rec->length = char_to_int(rec->raw, 5);

  // length of leader + directory
  rec->base_address = char_to_int(rec->raw + 12, 5);

  // compute the number of fields based on directory size
  rec->field_count = (rec->base_address - 24 - 1) / 12;

  // read the remainder of the record
  fread(rec->raw + 24, sizeof(char), rec->length - 24, in);

  // if we've been given storage for fields, assume that we want them to be processed
  rec->fields = fields ? marcrec_process_fields(rec, fields) : 0;

  return rec;
}

void marcrec_dump(const marcrec *rec, FILE *out)
{
  fwrite(rec->raw, sizeof(char), rec->length, out);
}

void marcrec_walk_fields(marcrec *rec, void (*f)(const marcfield *, void *), void *arg)
{
  // 1000 fields seems like a reasonable upper bound
  marcfield buf[1000], *fields = rec->fields ? rec->fields : marcrec_process_fields(rec, buf);
  for (int i = 0; i < rec->field_count; i++)
  {
    f(&fields[i], arg);
  }
}

int marcrec_validate(marcrec *rec)
{
  int r = 0;
  if (rec->raw[rec->length - 1] != RECORD_TERMINATOR)
    r |= MISSING_RECORD_TERMINATOR;
  if (rec->raw[rec->base_address - 1] != FIELD_TERMINATOR)
    r |= MISSING_FIELD_TERMINATOR;

  marcrec_walk_fields(rec, marc_validate_field, (void *)&r);
  return r;
}

void marc_validate_field(const marcfield *field, void *retPtr)
{
  int *r = (int *)retPtr;
  if (field->data[field->len - 1] != FIELD_TERMINATOR)
    *r |= MISSING_FIELD_TERMINATOR;
}

void marcrec_print(marcrec *rec, FILE *out)
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

void marcfield_humanize(const marcfield *field, char *dest)
{
  char *p = dest;
  for (int i = 0; i < field->len; i++)
  {
    switch (field->data[i])
    {
    // replace subfield delimiters with a human-readable format
    case SUBFIELD_DELIMITER:
    {
      *p++ = ' ';
      *p++ = '$';
      *p++ = field->data[++i];
      *p++ = ':';
      *p++ = ' ';
    }
    break;
    case FIELD_TERMINATOR:
    { // do not print
    }
    break;
    default:
      *p++ = field->data[i];
    }
  }

  *p = 0;
}

void marc_print_field(const marcfield *field, void *outPtr)
{
  FILE *out = (FILE *)outPtr;

  // print the tag
  fprintf(out, "  %.*s: ", 3, field->directory_entry);

  // 4-digit field length means a max size of 9999
  char buf[10000];
  marcfield_humanize(field, buf);

  fprintf(out, "%s\n", buf);
}

int marcfield_match_field(marcfield *field, char *fieldSpec, char *dest)
{
  char *de = field->directory_entry, *fs = fieldSpec, *d = dest, *p;

  // check to see whether the tag matches
  if (*de++ != *fs++ || *de++ != *fs++ || *de++ != *fs++)
    return 0;

  // no subfields specified; we want the whole shebang
  if (!*fs)
  {
    marcfield_humanize(field, dest);
    return 1;
  }

  // we're looking for specific subfield(s)
  int write = 0, count = 0;
  for (p = field->data; (p - field->data) < field->len; p++)
  {
    if (*p == SUBFIELD_DELIMITER)
    { // state change
      if (contains_char(fs, *(++p)) == 0)
      {
        write = 0;
      }
      else
      {
        if (count++)
          *d++ = ' '; // space-delimit if we get multiple matches
        p++; // skip over the subfield code
        write = 1;
      }
    }
    if (write)
      *d++ = *p;
  }

  *d = 0;

  return count;
}
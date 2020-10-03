#include <stdio.h>
#include "marc.h"
#include "util.h"

static void marcrec_process_fields(marcrec *rec)
{
  for (int i = 0; i < rec->field_count; i++)
  {
    rec->fields[i].directory_entry = rec->raw + 24 + i * 12;
    rec->fields[i].len = char_to_int(rec->fields[i].directory_entry + 3, 4);
    rec->fields[i].data = rec->raw + rec->base_address + char_to_int(rec->fields[i].directory_entry + 7, 5);
  }
}

static void marcfield_pretty_print(FILE *out, const marcfield *field)
{
  // print the tag
  fprintf(out, "  %.*s: ", 3, field->directory_entry);

  // 4-digit field length means a max size of 9999
  char buf[10000];
  marcfield_humanize(buf, field);

  fprintf(out, "%s\n", buf);
}

static void marcrec_pretty_print(FILE *out, const marcrec *rec)
{
  fprintf(out, "length: %05i | status: %c | type: %c | bibliographic level: %c | type of control: %c\n",
          rec->len, rec->raw[5], rec->raw[6], rec->raw[7], rec->raw[9]);
  fprintf(out, "character coding scheme: %c | indicator count: %c | subfield code count: %c\n",
          rec->raw[10], rec->raw[11], rec->raw[12]);
  fprintf(out, "base address of data: %05i | encoding level: %c | descriptive cataloging form: %c\n",
          rec->base_address, rec->raw[17], rec->raw[18]);
  fprintf(out, "multipart resource record level: %c | length of the length-of-field portion: %c\n",
          rec->raw[19], rec->raw[20]);
  fprintf(out, "length of the staring-character-position portion: %c | length of the implementation-defined portion: %c\n",
          rec->raw[21], rec->raw[22]);

  for(int i = 0; i < rec->field_count; i++) {
    marcfield_pretty_print(out, &rec->fields[i]);
  }
}

int marcrec_from_buffer(marcrec *rec, const char *buf, int len)
{
  // you're mine now!
  rec->raw = buf;

  // total length of record
  rec->len = (len == 0) ? char_to_int(buf, 5) : len;

  // length of leader + directory
  rec->base_address = char_to_int(buf + 12, 5);

  // compute the number of fields based on directory size
  rec->field_count = (rec->base_address - 24 - 1) / 12;

  // if we've been given storage for fields, assume that we want them to be processed
  if(rec->fields) marcrec_process_fields(rec);

  // return a pointer to the "rest" of the buffer (if any)
  return rec->len;
}

int marcrec_read(marcrec *rec, char *buf, FILE *in)
{
  int n = fread(buf, sizeof(char), 24, in);
  if(n == 0) return 0;
  if(n < 24) return -1;
  
  // total length of record
  int len = char_to_int(buf, 5);

  // read the remainder of the record
  n = fread(buf + 24, sizeof(char), len - 24, in);
  if(n < (len-24)) return -1;

  // process the rest of the record
  return marcrec_from_buffer(rec, buf, len);
}

void marcrec_write(FILE *out, const marcrec *rec, int pretty)
{
  if(pretty == 0)
    fwrite(rec->raw, sizeof(char), rec->len, out);
  else
    marcrec_pretty_print(out, rec);
}

static int marcfield_validate(const marcfield *field)
{
  return (field->data[field->len - 1] == FIELD_TERMINATOR) ? 0 : MISSING_FIELD_TERMINATOR;
}

int marcrec_validate(const marcrec *rec)
{
  int r = 0;
  if (rec->raw[rec->len - 1] != RECORD_TERMINATOR)
    r |= MISSING_RECORD_TERMINATOR;
  if (rec->raw[rec->base_address - 1] != FIELD_TERMINATOR)
    r |= MISSING_FIELD_TERMINATOR;

  for(int i = 0; i < rec->field_count; i++) {
    r |= marcfield_validate(&rec->fields[i]);
  }

  return r;
}

void marcfield_humanize(char *dest, const marcfield *field)
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



int marcfield_match_field(char *dest, const marcfield *field, const char *fieldSpec)
{
  const char *de = field->directory_entry, *fs = fieldSpec, *p;
  char *d = dest;

  // check to see whether the tag matches
  if (*de++ != *fs++ || *de++ != *fs++ || *de++ != *fs++)
    return 0;

  // no subfields specified; we want the whole shebang
  if (!*fs)
  {
    marcfield_humanize(dest, field);
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

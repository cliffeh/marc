#include <stdio.h>
#include <string.h>
#include "marc.h"

#define atoi1(p) (*(p) - '0')
#define atoi2(p) (atoi1(p) * 10 + atoi1(p + 1))
#define atoi3(p) (atoi1(p) * 100 + atoi2(p + 1))
#define atoi4(p) (atoi1(p) * 1000 + atoi3(p + 1))
#define atoi5(p) (atoi1(p) * 10000 + atoi4(p + 1))

#define MATCH_FIELD(spec, direntry) \
  (*(spec) && (*(spec) == *(direntry))) && (*((spec) + 1) && (*((spec) + 1) == *((direntry) + 1))) && (*((spec) + 2) && (*((spec) + 2) == *((direntry) + 2)))

static void marcfield_print_subfields(FILE *out, const marcfield *field, const char *spec)
{
  // if we have no spec we're going to print the whole thing
  int printing = (!spec || !*spec) ? 1 : 0;
  int i = 0;

  if (*(field->data + 2) == SUBFIELD_DELIMITER)
  { // print indicators
    fprintf(out, " %.2s", field->data);
    i = 2;
  }
  else
  { // prettify fixed-length fields
    fprintf(out, "    ");
  }

  while (i < field->length)
  {
    switch (field->data[i])
    {
    case FIELD_TERMINATOR:
    {
      i++; // do not print
    }
    break;
    case SUBFIELD_DELIMITER: // state change; re-assess whether we should be printing
    {
      i++;
      printing = (!spec || !*spec) ? 1 : 0;

      if (!printing)
      {
        for (int j = strlen(spec) - 1; j >= 0; j--)
        {
          if (spec[j] == field->data[i])
          {
            printing = 1;
            break;
          }
        }
      }

      if (printing)
      {
        fprintf(out, " $%c: ", field->data[i++]);
      }
      else
      {
        // skip bytes we're not supposed to be printing until the next subfield
        for (; i < field->length && field->data[i] != SUBFIELD_DELIMITER; i++)
          ;
      }
    }
    break;
    default: // if we're not skipping bytes then we're supposed to be printing
      fprintf(out, "%c", field->data[i++]);
    }
  }
}

int marcfield_print(FILE *out, const marcfield *field, const char **specs)
{
  int n = 0;
  if (!specs)
  {
    // indent (assuming we're pretty-printing entire record)
    fprintf(out, "\t%.3s", field->directory_entry);
    marcfield_print_subfields(out, field, 0);
    fprintf(out, "\n");
    n = 1;
  }
  else // only print if we match the spec
  {
    for (int i = 0; specs[i]; i++)
    {
      if (MATCH_FIELD(specs[i], field->directory_entry))
      {
        // no indent since we're only printing specific fields
        fprintf(out, "%.3s", field->directory_entry);
        marcfield_print_subfields(out, field, specs[i] + 3);
        fprintf(out, "\n");
        n = 1;
      }
    }
  }
  return n;
}

int marcrec_print(FILE *out, const marcrec *rec, const char **specs)
{
  if (!specs) // TODO update spec to include leader fields?
  {
    fprintf(out, "length: %.5s | status: %c | type: %c | bibliographic level: %c | type of control: %c\n",
            rec->data, rec->data[5], rec->data[6], rec->data[7], rec->data[9]);
    fprintf(out, "character coding scheme: %c | indicator count: %c | subfield code count: %c\n",
            rec->data[10], rec->data[11], rec->data[12]);
    fprintf(out, "base address of data: %.5s | encoding level: %c | descriptive cataloging form: %c\n",
            rec->data + 12, rec->data[17], rec->data[18]);
    fprintf(out, "multipart resource record level: %c | length of the length-of-field portion: %c\n",
            rec->data[19], rec->data[20]);
    fprintf(out, "length of the staring-character-position portion: %c | length of the implementation-defined portion: %c\n",
            rec->data[21], rec->data[22]);

    // DEBUG
    // fprintf(out, "fields (%i)\n", rec->field_count);
  }

  int n = 0;
  for (int i = 0; i < rec->field_count; i++)
  {
    n += marcfield_print(out, &rec->fields[i], specs);
  }
  return n;
}

int marcrec_from_buffer(marcrec *rec, char *buf, int length)
{
  // you're mine now!
  rec->data = buf;

  // total length of record
  rec->length = (length == 0) ? atoi5(buf) : length;

  // length of leader + directory
  rec->base_address = atoi5(buf + 12);

  // compute the number of fields based on directory size
  rec->field_count = (rec->base_address - 24 - 1) / 12;

  // if we've been given storage for fields, assume that we want to process the directory
  if (rec->fields)
  {
    for (int i = 0; i < rec->field_count; i++)
    {
      rec->fields[i].directory_entry = rec->data + 24 + i * 12;
      rec->fields[i].length = atoi4(rec->fields[i].directory_entry + 3);
      rec->fields[i].data = rec->data + rec->base_address + atoi5(rec->fields[i].directory_entry + 7);
    }
  }

  // return a pointer to the "rest" of the buffer (if any)
  return rec->length;
}

int marcrec_read(marcrec *rec, FILE *in)
{
  // read the leader
  int n = fread(rec->data, sizeof(char), 24, in);
  if (n == 0)
    return 0;
  if (n < 24)
    return -1;

  // total length of record
  int length = atoi5(rec->data);

  // read the remainder of the record
  n = fread(rec->data + 24, sizeof(char), length - 24, in);
  if (n < (length - 24))
    return -1;

  // process the rest of the record
  return marcrec_from_buffer(rec, rec->data, length);
}

int marcrec_write(FILE *out, const marcrec *rec)
{
  return fwrite(rec->data, sizeof(char), rec->length, out);
}

static int marcfield_validate(const marcfield *field)
{
  return (field->data[field->length - 1] == FIELD_TERMINATOR) ? 0 : MISSING_FIELD_TERMINATOR;
}

int marcrec_validate(const marcrec *rec)
{
  int r = 0;
  if (rec->data[rec->length - 1] != RECORD_TERMINATOR)
    r |= MISSING_RECORD_TERMINATOR;
  if (rec->data[rec->base_address - 1] != FIELD_TERMINATOR)
    r |= MISSING_FIELD_TERMINATOR;

  for (int i = 0; i < rec->field_count; i++)
  {
    r |= marcfield_validate(&rec->fields[i]);
  }

  return r;
}

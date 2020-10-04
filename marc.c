#include <stdio.h>
#include "marc.h"

#define atoi1(p) (*(p)-'0')
#define atoi2(p) (atoi1(p)*10+atoi1(p+1))
#define atoi3(p) (atoi1(p)*100+atoi2(p+1))
#define atoi4(p) (atoi1(p)*1000+atoi3(p+1))
#define atoi5(p) (atoi1(p)*10000+atoi4(p+1))

static void marcrec_process_fields(marcrec *rec)
{
  for (int i = 0; i < rec->field_count; i++)
  {
    rec->fields[i].directory_entry = rec->raw + 24 + i * 12;
    rec->fields[i].len = atoi4(rec->fields[i].directory_entry + 3);
    rec->fields[i].data = rec->raw + rec->base_address + atoi5(rec->fields[i].directory_entry + 7);
  }
}

static void marcfield_pretty_print(FILE *out, const marcfield *field)
{
  // print the tag
  fprintf(out, "\t%.*s\t", 3, field->directory_entry);

  for (int i = 0; i < field->len; i++)
  {
    switch (field->data[i])
    {
      case FIELD_TERMINATOR:
      { // do not print
      } break;
      case SUBFIELD_DELIMITER:
      {
        fprintf(out, "$");
      } break;
      default:
        fprintf(out, "%c", field->data[i]);
    }
  }
  fprintf(out, "\n");
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
  rec->len = (len == 0) ? atoi5(buf) : len;

  // length of leader + directory
  rec->base_address = atoi5(buf + 12);

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
  int len = atoi5(buf);

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

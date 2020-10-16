#include "marc.h"

#define atoi1(p) (*(p) - '0')
#define atoi2(p) (atoi1(p) * 10 + atoi1(p + 1))
#define atoi3(p) (atoi1(p) * 100 + atoi2(p + 1))
#define atoi4(p) (atoi1(p) * 1000 + atoi3(p + 1))
#define atoi5(p) (atoi1(p) * 10000 + atoi4(p + 1))

#define MATCH_FIELD(spec, direntry) \
  (*(spec) && (*(spec) == *(direntry))) && (*((spec) + 1) && (*((spec) + 1) == *((direntry) + 1))) && (*((spec) + 2) && (*((spec) + 2) == *((direntry) + 2)))

#define IS_CONTROL_FIELD(tag) ((*(tag) == '0') && (*(tag + 1) == '0'))

static void marcfield_print_subfields(FILE *out, const marcfield *field, const char *spec)
{

  // we don't need to do any additional matching to tease out subfields;
  // let's just print and get out of here
  if (IS_CONTROL_FIELD(field->directory_entry))
  {
    fprintf(out, "    %.*s", field->length - 1, field->data);
    return;
  }

  // if we have no spec we're going to print the whole thing
  int printing = (!spec || !*spec) ? 1 : 0;

  // print indicators
  fprintf(out, " %.2s", field->data);

  int i = 2;
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

int marcrec_read(marcrec *rec, gzFile in)
{
  // read the leader
  int n = gzread(in, rec->data, 24);
  if (n == 0)
    return 0;
  if (n < 24)
    return -1;

  // total length of record
  int length = atoi5(rec->data);

  // read the remainder of the record
  n = gzread(in, rec->data + 24, length - 24);
  if (n < (length - 24))
    return -1;

  // process the rest of the record
  return marcrec_from_buffer(rec, rec->data, length);
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
    if (rec->fields[i].data[rec->fields[i].length - 1] != FIELD_TERMINATOR)
    {
      r |= MISSING_FIELD_TERMINATOR;
      // no sense continuing...
      return r;
    }
  }

  return r;
}

int marcrec_write(FILE *out, const marcrec *rec)
{
  return fwrite(rec->data, sizeof(char), rec->length, out);
}

static void marcfield_xml_escape_char(FILE *out, char c)
{
  switch (c)
  {
  // case '"':
  // {
  //   fprintf(out, "&quot;");
  // }
  // break;
  case '&':
  {
    fprintf(out, "&amp;");
  }
  break;
  // case '\'':
  // {
  //   fprintf(out, "&apos;");
  // }
  // break;
  case '<':
  {
    fprintf(out, "&lt;");
  }
  break;
  case '>':
  {
    fprintf(out, "&gt;");
  }
  break;
  default:
    fprintf(out, "%c", c);
  }
}

static void marcfield_xml_escape(FILE *out, const char *text, int len)
{
  for (int i = 0; i < len; i++)
  {
    marcfield_xml_escape_char(out, text[i]);
  }
}

static int marcfield_xml_subfield(FILE *out, const marcfield *field, int pos)
{
  // quick sanity check
  if (field->data[pos] != SUBFIELD_DELIMITER)
  {
    fprintf(stderr, "error: unrecognized character position; bailing...\n");
    return field->length;
  }

  int i = pos + 1;
  fprintf(out, "    <subfield code=\"%c\">", field->data[i++]);

  while (i < field->length)
  {
    switch (field->data[i])
    {
    case FIELD_TERMINATOR:
      i = field->length; // we're done here
    case SUBFIELD_DELIMITER:
    {
      fprintf(out, "</subfield>\n");
      return i;
    }
    default:
      marcfield_xml_escape_char(out, field->data[i++]);
    }
  }

  fprintf(out, "</subfield>\n");
  return i;
}

static void marcfield_xml(FILE *out, const marcfield *field)
{
  if (IS_CONTROL_FIELD(field->directory_entry))
  {
    // TODO make sure we're escaping XML characters!
    fprintf(out, "  <controlfield tag=\"%.3s\">", field->directory_entry);
    marcfield_xml_escape(out, field->data, field->length - 1);
    fprintf(out, "</controlfield>\n");
  }
  else
  {
    fprintf(out, "  <datafield tag=\"%.3s\" ind1=\"%c\" ind2=\"%c\">\n", field->directory_entry, field->data[0], field->data[1]);
    int pos = 2;
    while (pos < field->length)
    {
      pos = marcfield_xml_subfield(out, field, pos);
    }
    fprintf(out, "  </datafield>\n");
  }
}

int marcrec_xml(FILE *out, const marcrec *rec)
{
  fprintf(out, "<record>\n");
  fprintf(out, "  <leader>%.24s</leader>\n", rec->data);
  for (int i = 0; i < rec->field_count; i++)
  {
    marcfield_xml(out, &rec->fields[i]);
  }
  fprintf(out, "</record>\n");

  return 1;
}

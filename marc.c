#include "config.h"
#include "marc.h"
#include "util.h"

#define IS_CONTROL_FIELD(tag) ((*(tag) == '0') && (*(tag + 1) == '0'))

static void marcfield_print_subfields(FILE *out, const marcfield *field, const char *subfields)
{
  // we don't need to do any additional matching to tease out subfields;
  // let's just print and get out of here
  if (IS_CONTROL_FIELD(field->directory_entry))
  {
    fprintf(out, "    %.*s", field->length - 1, field->data);
    return;
  }

  // if we have no spec we're going to print the whole thing
  int printing = (!subfields || !*subfields) ? 1 : 0, i = 2;

  // print indicators
  fprintf(out, " %.2s", field->data);

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
      printing = (!subfields || !*subfields) ? 1 : 0;

      if (!printing)
      {
        for (int j = 0; subfields[j]; j++)
        {
          if (subfields[j] == field->data[i])
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

marcrec *marcrec_alloc(int nBytes, int nFields)
{
  marcrec *rec = calloc(1, sizeof(marcrec));
  if (nBytes)
    rec->data = calloc(nBytes, sizeof(char));
  if (nFields)
    rec->fields = calloc(nFields, sizeof(marcfield));
  return rec;
}

void marcrec_free(marcrec *rec)
{
  if (!rec) // just in case
    return;
  if (rec->fields)
    free(rec->fields);
  if (rec->data)
    free(rec->data);
  free(rec);
}

int marcfield_print(FILE *out, const marcfield *field, const fieldspec specs[])
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
    for (int i = 0; specs[i].tag != 0; i++)
    {
      if (specs[i].tag == field->tag)
      {
        // no indent since we're only printing specific fields
        fprintf(out, "%.3s", field->directory_entry);
        marcfield_print_subfields(out, field, specs[i].subfields);
        fprintf(out, "\n");
        n = 1;
      }
    }
  }
  return n;
}

int marcrec_print(FILE *out, const marcrec *rec, const fieldspec specs[])
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
  }

  int n = 0;
  for (int i = 0; i < rec->field_count; i++)
  {
    n += marcfield_print(out, &rec->fields[i], specs);
  }
  return n;
}

marcrec *marcrec_from_buffer(marcrec *rec, char *buf, int nBytes)
{
  // no data left
  if (!buf || !(*buf))
    return 0;

  // total length of record
  int length = nBytes ? nBytes : atoin(buf, 5);
  // length of leader + directory
  int base_address = atoin(buf + 12, 5);
  // compute the number of fields based on directory size
  int field_count = (base_address - 24 - 1) / 12;

  // we already have a buffer, need to allocaterecords and fields
  rec = rec ? rec : marcrec_alloc(0, field_count);
  rec->data = buf;
  rec->length = length;
  rec->base_address = base_address;
  rec->field_count = field_count;

  // process the directory
  for (int i = 0; i < rec->field_count; i++)
  {
    rec->fields[i].directory_entry = rec->data + 24 + i * 12;
    rec->fields[i].tag = atoin(rec->fields[i].directory_entry, 3);
    rec->fields[i].length = atoin(rec->fields[i].directory_entry + 3, 4);
    rec->fields[i].data = rec->data + rec->base_address + atoin(rec->fields[i].directory_entry + 7, 5);
  }

  // return a pointer to the newly-populated marcrec
  return rec;
}

marcrec *marcrec_read(marcrec *rec, marcfile *in)
{
// if we've reached end of file let's get out of here
#ifdef USE_ZLIB
  if (gzeof(in->gzf))
    return 0;
#else
  if (feof(in->f))
    return 0;
#endif

  int n, nBytes;
  // allocate a buffer if we haven't been given one
  char *p = rec ? rec->data : malloc(24);
#ifdef USE_ZLIB
  n = gzread(in->gzf, p, 24);
#else
  n = fread(p, sizeof(char), 24, in->f);
#endif
  // TODO set error flag if n < 24?
  if (n < 24)
  {
    if (!rec)
      free(p);
    return 0;
  }
  nBytes = atoin(p, 5);

  // grow the buffer if we need to
  if (!rec)
    p = realloc(p, nBytes);

    //read the rest of the record
#ifdef USE_ZLIB
  n = gzread(in->gzf, p + 24, nBytes - 24);
#else
  n = fread(p + 24, sizeof(char), nBytes - 24, in->f);
#endif
  // TODO set error flag if n < (length - 24)?
  if (n < (nBytes - 24))
  {
    if (!rec)
      free(p);
    return 0;
  }

  return marcrec_from_buffer(rec, p, nBytes);
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

marcfile *marcfile_open(const char *filename, const char *mode)
{
  marcfile *r = calloc(1, sizeof(marcfile));
#ifdef USE_ZLIB
  r->gzf = gzopen(filename, mode);
#else
  r->f = fopen(filename, mode);
#endif
  return r;
}

marcfile *marcfile_from_fd(int fd, char *mode)
{
  marcfile *r = malloc(sizeof(marcfield));
#ifdef USE_ZLIB
  r->gzf = gzdopen(fd, mode);
#else
  r->f = fdopen(fd, mode);
#endif
  return r;
}

marcfile *marcfile_from_FILE(FILE *file, char *mode)
{
  return marcfile_from_fd(fileno(file), mode);
}

void marcfile_close(marcfile *mf)
{
#ifdef USE_ZLIB
  gzclose(mf->gzf);
#else
  fclose(mf->f);
#endif
  free(mf);
}

// ensure config.h is always read first!
// clang-format off
#include "config.h"
// clang-format on
#include "marc.h"
#include "util.h"

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

struct marcfile
{
  int errnum;
#ifdef USE_ZLIB
  gzFile gzf;
#else
  FILE *f;
#endif
};

struct marcfield
{
  char *directory_entry, *data;
  int length;
};

struct marcrec
{
  int length, base_address, field_count, vflags;
  char *data;
  marcfield *fields;
};

/**
 * @brief determine whether a given tag represents a control field (00X)
 *
 */
#define MARC_CONTROL_FIELD(tag) ((*(tag) == '0') && (*(tag + 1) == '0'))
/**
 * @brief determine whether 2 tags match
 *
 */
#define MARC_MATCH_TAG(t1, t2)                                                \
  ((*(t1) == *(t2)) && (*(t1 + 1) == *(t2 + 1)) && (*(t1 + 2) == *(t2 + 2)))

marcrec *
marcrec_alloc (int nBytes, int nFields)
{
  marcrec *rec = calloc (1, sizeof (marcrec));
  if (nBytes)
    rec->data = calloc (nBytes, sizeof (char));
  if (nFields)
    rec->fields = calloc (nFields, sizeof (marcfield));
  return rec;
}

void
marcrec_free (marcrec *rec)
{
  if (!rec) // just in case
    return;
  if (rec->fields)
    free (rec->fields);
  if (rec->data)
    free (rec->data);
  free (rec);
}

int
marcfield_print (FILE *out, const marcfield *field, const char *subfields)
{
  int n = 0;
  fprintf (out, "%.3s", field->directory_entry);

  // we don't need to do any additional matching to tease out subfields;
  // let's just print and get out of here
  if (MARC_CONTROL_FIELD (field->directory_entry))
    {
      fprintf (out, "    %.*s", field->length - 1, field->data);
      return 1;
    }

  // if we have no spec we're going to print the whole thing
  int printing = (!subfields || !*subfields) ? 1 : 0, i = 2;

  // print indicators
  fprintf (out, " %.2s", field->data);

  while (i < field->length)
    {
      switch (field->data[i])
        {
        case FIELD_TERMINATOR:
          {
            i++; // do not print
          }
          break;
        case SUBFIELD_DELIMITER: // state change; re-assess whether we should
                                 // be printing
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
                        n++;
                        break;
                      }
                  }
              }

            if (printing)
              {
                fprintf (out, " $%c: ", field->data[i++]);
              }
            else
              {
                // skip bytes we're not supposed to be printing until the next
                // subfield
                for (;
                     i < field->length && field->data[i] != SUBFIELD_DELIMITER;
                     i++)
                  ;
              }
          }
          break;
        default: // if we're not skipping bytes then we're supposed to be
                 // printing
          fprintf (out, "%c", field->data[i++]);
        }
    }

  return n;
}

int
marcrec_print (FILE *out, const marcrec *rec, const char *filters[])
{
  int n = 0;
  if (!filters || !(*filters))
    { // we're printing the whole shebang
      fprintf (out, "%.24s\n", rec->data);

      for (int i = 0; i < rec->field_count; i++)
        {
          fprintf (out, "\t");
          marcfield_print (out, &rec->fields[i], 0);
          fprintf (out, "\n");
          n++;
        }
    }
  else
    { // we're only printing specific fields
      for (int i = 0; filters[i]; i++)
        {
          if (strcasecmp ("leader", filters[i]) == 0)
            {
              fprintf (out, "%.24s\n", rec->data);
            }
          else
            {
              for (int j = 0; j < rec->field_count; j++)
                {
                  if (MARC_MATCH_TAG (filters[i],
                                      rec->fields[j].directory_entry))
                    {
                      marcfield_print (out, &rec->fields[j], filters[i] + 3);
                      fprintf (out, "\n");
                      n++;
                    }
                }
            }
        }
    }

  return n;
}

marcrec *
marcrec_from_buffer (marcrec *rec, char *buf, int nBytes)
{
  // no data left
  if (!buf || !(*buf))
    return 0;

  // total length of record
  int length = nBytes ? nBytes : atoin (buf, 5);
  if (length < 0)
    {
      // TODO capture error information somewhere?
      return 0;
    }
  // length of leader + directory
  int base_address = atoin (buf + 12, 5);
  if (base_address < 0)
    {
      // TODO capture error information somewhere?
      return 0;
    }
  // compute the number of fields based on directory size
  int field_count = (base_address - 24 - 1) / 12;

  // we already have a buffer, need to allocate records and fields
  rec = rec ? rec : marcrec_alloc (0, field_count);
  rec->vflags = 0;

  rec->data = buf;
  rec->length = length;
  if (rec->data[rec->length - 1] != RECORD_TERMINATOR)
    {
      rec->vflags |= MISSING_RECORD_TERMINATOR;
    }

  rec->base_address = base_address;
  if (rec->data[rec->base_address - 1] != FIELD_TERMINATOR)
    {
      rec->vflags |= MISSING_FIELD_TERMINATOR;
    }

  rec->field_count = field_count;

  // process the directory
  for (int i = 0; i < rec->field_count; i++)
    {
      // TODO handle bad directory data
      rec->fields[i].directory_entry = rec->data + 24 + i * 12;
      rec->fields[i].length = atoin (rec->fields[i].directory_entry + 3, 4);
      rec->fields[i].data = rec->data + rec->base_address
                            + atoin (rec->fields[i].directory_entry + 7, 5);
      if (rec->fields[i].data[rec->fields[i].length - 1] != FIELD_TERMINATOR)
        {
          rec->vflags |= MISSING_FIELD_TERMINATOR;
        }
    }

  // return a pointer to the newly-populated marcrec
  return rec;
}

marcrec *
marcrec_read (marcrec *rec, marcfile *in)
{
// if we've reached end of file let's get out of here
#ifdef USE_ZLIB
  if (gzeof (in->gzf))
    return 0;
#else
  if (feof (in->f))
    return 0;
#endif

  int n, nBytes;
  // allocate a buffer if we haven't been given one
  char *p = rec ? rec->data : malloc (24);
#ifdef USE_ZLIB
  n = gzread (in->gzf, p, 24);
#else
  n = fread (p, sizeof (char), 24, in->f);
#endif
  if (n < 24)
    {
      if (n != 0) // eof doesn't catch this?
        in->errnum = LEADER_NOT_ENOUGH_BYTES;
      if (!rec)
        {
          free (p);
        }
      return 0;
    }
  nBytes = atoin (p, 5);
  if (nBytes < 0)
    {
      in->errnum = INVALID_MARC_RECORD_LENGTH;
      return 0;
    }

  // grow the buffer if we need to
  if (!rec)
    {
      p = realloc (p, nBytes);
    }

    // read the rest of the record
#ifdef USE_ZLIB
  n = gzread (in->gzf, p + 24, nBytes - 24);
#else
  n = fread (p + 24, sizeof (char), nBytes - 24, in->f);
#endif
  // TODO set error flag if n < (length - 24)?
  if (n < (nBytes - 24))
    {
      in->errnum = RECORD_NOT_ENOUGH_BYTES;
      if (!rec)
        {
          free (p);
        }
      return 0;
    }

  in->errnum = 0;

  return marcrec_from_buffer (rec, p, nBytes);
}

int
marcrec_write (FILE *out, const marcrec *rec, const char *filters[])
{
  return fwrite (rec->data, sizeof (char), rec->length, out);
}

static void
marcfield_xml_escape_char (FILE *out, char c)
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
        fprintf (out, "&amp;");
      }
      break;
    // case '\'':
    // {
    //   fprintf(out, "&apos;");
    // }
    // break;
    case '<':
      {
        fprintf (out, "&lt;");
      }
      break;
    case '>':
      {
        fprintf (out, "&gt;");
      }
      break;
    default:
      fprintf (out, "%c", c);
    }
}

static void
marcfield_xml_escape (FILE *out, const char *text, int len)
{
  for (int i = 0; i < len; i++)
    {
      marcfield_xml_escape_char (out, text[i]);
    }
}

static int
marcfield_xml_subfield (FILE *out, const marcfield *field, int pos)
{
  // quick sanity check
  if (field->data[pos] != SUBFIELD_DELIMITER)
    {
      fprintf (stderr, "error: unrecognized character position; bailing...\n");
      return field->length;
    }

  int i = pos + 1;
  fprintf (out, "    <subfield code=\"%c\">", field->data[i++]);

  while (i < field->length)
    {
      switch (field->data[i])
        {
        case FIELD_TERMINATOR:
          i = field->length; // we're done here
        case SUBFIELD_DELIMITER:
          {
            fprintf (out, "</subfield>\n");
            return i;
          }
        default:
          marcfield_xml_escape_char (out, field->data[i++]);
        }
    }

  fprintf (out, "</subfield>\n");
  return i;
}

static void
marcfield_xml (FILE *out, const marcfield *field, const char *filters[])
{
  if (MARC_CONTROL_FIELD (field->directory_entry))
    {
      fprintf (out, "  <controlfield tag=\"%.3s\">", field->directory_entry);
      marcfield_xml_escape (out, field->data, field->length - 1);
      fprintf (out, "</controlfield>\n");
    }
  else
    {
      fprintf (out, "  <datafield tag=\"%.3s\" ind1=\"%c\" ind2=\"%c\">\n",
               field->directory_entry, field->data[0], field->data[1]);
      int pos = 2;
      while (pos < field->length)
        {
          pos = marcfield_xml_subfield (out, field, pos);
        }
      fprintf (out, "  </datafield>\n");
    }
}

int
marcrec_xml (FILE *out, const marcrec *rec, const char *filters[])
{
  fprintf (out, "<record>\n");
  fprintf (out, "  <leader>%.24s</leader>\n", rec->data);
  for (int i = 0; i < rec->field_count; i++)
    {
      marcfield_xml (out, &rec->fields[i], filters);
    }
  fprintf (out, "</record>\n");

  return 1;
}

marcfile *
marcfile_open (const char *filename)
{
  marcfile *mf = calloc (1, sizeof (marcfile));
#ifdef USE_ZLIB
  if (!(mf->gzf = gzopen (filename, "r")))
    {
      free (mf);
      mf = 0;
    }
#else
  if (!(mf->f = fopen (filename, "r")))
    {
      free (mf);
      mf = 0;
    }
#endif
  return mf;
}

marcfile *
marcfile_from_fd (int fd)
{
  marcfile *mf = calloc (1, sizeof (marcfile));
#ifdef USE_ZLIB
  if (!(mf->gzf = gzdopen (fd, "r")))
    {
      free (mf);
      mf = 0;
    }
#else
  if (!(mf->f = fdopen (fd, "r")))
    {
      free (mf);
      mf = 0;
    }
#endif
  return mf;
}

marcfile *
marcfile_from_FILE (FILE *file)
{
  return marcfile_from_fd (fileno (file));
}

void
marcfile_close (marcfile *mf)
{
#ifdef USE_ZLIB
  gzclose (mf->gzf);
#else
  fclose (mf->f);
#endif
  free (mf);
}

int
marcfile_error (marcfile *mf, char *msg)
{

  if (!msg || !mf->errnum)
    return mf->errnum;

  int nBytes, num;
  switch (mf->errnum)
    {
    case LEADER_NOT_ENOUGH_BYTES:
      nBytes = sprintf (msg, "leader had fewer than 24 bytes");
      break;
    case RECORD_NOT_ENOUGH_BYTES:
      nBytes = sprintf (msg, "input had fewer bytes than record length");
      break;
    case INVALID_MARC_RECORD_LENGTH:
      nBytes = sprintf (msg, "invalid record length in leader");
      break;
    }
#ifdef USE_ZLIB
  const char *tmp = gzerror (mf->gzf, &num);
  // if there was a related read error on the underlying file then append it
  if (num)
    sprintf (msg + nBytes, ": %s", tmp);
#else
  num = ferror (mf->f);
#endif

  return mf->errnum;
}

int
marcrec_validate (marcrec *rec)
{
  return rec->vflags;
}
#ifndef __MARC_H
#define __MARC_H 1

// terminator constants
#define FIELD_TERMINATOR   0x1e
#define RECORD_TERMINATOR  0x1d
#define SUBFIELD_DELIMITER 0x1f

// error constants
#define MISSING_FIELD_TERMINATOR  (1<<0)
#define MISSING_RECORD_TERMINATOR (1<<1)

typedef struct marcfield {
  const char *directory_entry, *data;
  int len;
} marcfield;

typedef struct marcrec {
  int len, base_address, field_count;
  const char *raw;
  marcfield *fields;
} marcrec;

/**
 * @brief read a marcrec from a file
 *
 * if rec->fields is non-null this function will assume you want fields processed;
 * otherwise it will only read the raw record
 *
 * @param rec a pointer to an allocated marcrec object to be populated
 * @param buf a pointer to an allocated buffer to read into
 * @param in a pointer to an open FILE to read from
 * @return int the number of bytes read (-1 if the number of bytes was fewer than expected)
 */
int marcrec_read(marcrec *rec, char *buf, FILE *in);

/**
 * @brief read a marcrec from a buffer
 *
 * if rec->fields is non-null this function will assume you want fields processed;
 * otherwise it will only read the raw record
 *
 * @param rec a pointer to an allocated marcrec object to be populated
 * @param buf a pointer to a buffer containing a marc record
 * @param len the number of bytes in the marc record, or 0 if the length is unknown and should be computed
 * @return int the number of bytes processed
 */
int marcrec_from_buffer(marcrec *rec, const char *buf, int len);

/**
 * @brief write a marc record to a file
 *
 * @param out the file to write to
 * @param rec the marc record to write
 * @param pretty if 0, write in marc binary format; else write in a human-readable format
 */
void marcrec_write(FILE *out, const marcrec *rec, int pretty);

/**
 * @brief validate a marc record
 *
 * @param rec the record to validate
 * @return int 0 if the marc record has all the appropriate field/record terminators; otherwise non-zero
 */
int marcrec_validate(const marcrec *rec);

void marcfield_humanize(char *dest, const marcfield *field);
int marcfield_match_field(char *dest, const marcfield *field, const char *fieldSpec);


#endif

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

void marcrec_dump(const marcrec *rec, FILE *out);
int marcrec_validate(marcrec *rec);
void marcrec_walk_fields(marcrec *rec, void (*callback)(const marcfield *, void *), void *arg);
void marcrec_print(marcrec *rec, FILE *out);
void marcfield_humanize(const marcfield *field, char *dest);
int marcfield_match_field(char *dest, const marcfield *field, const char *fieldSpec);

// marcrec_walk_fields callbacks
void marc_print_field(const marcfield *field, void *outPtr);
void marc_validate_field(const marcfield *field, void *retPtr);

#endif

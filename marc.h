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
  char *directory_entry, *data;
  int len;
} marcfield;

typedef struct marcrec {
  int len, base_address, field_count;
  char *raw;
  marcfield *fields;
} marcrec;

/**
 * @brief read a marcrec from a file
 *
 * @param rec a pointer to an allocated marcrec object
 * @param buf a pointer to an allocated buffer to read into
 * @param in a pointer to an open FILE to read from
 * @return int the number of bytes read (-1 if the number of bytes was fewer than expected)
 */
int marcrec_read(marcrec *rec, char *buf, FILE *in);
int marcrec_from_buffer(marcrec *rec, char *buf, int len);

void marcrec_dump(const marcrec *rec, FILE *out);
int marcrec_validate(marcrec *rec);
void marcrec_walk_fields(marcrec *rec, void (*callback)(const marcfield *, void *), void *arg);
void marcrec_print(marcrec *rec, FILE *out);
void marcfield_humanize(const marcfield *field, char *dest);
int marcfield_match_field(marcfield *field, char *fieldSpec, char *dest);

// marcrec_walk_fields callbacks
void marc_print_field(const marcfield *field, void *outPtr);
void marc_validate_field(const marcfield *field, void *retPtr);

#endif

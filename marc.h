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
  int length, base_address, field_count;
  // MARC record length is represented as 5 digits, with 
  // 99999 being the maximum possible length
  char raw[99999];
  marcfield *fields;
} marcrec;

marcrec *marcrec_read(marcrec *rec, marcfield *fields, FILE *in);
int marcrec_validate(marcrec *rec);
void marcrec_walk_fields(marcrec *rec, void (*f)(const marcfield *, void *), void *arg);
void marcrec_print(marcrec *rec, FILE *out);

// marcrec_walk_fields callbacks
void marc_print_field(const marcfield *, void *outPtr);
void marc_validate_field(const marcfield *, void *retPtr);

#endif

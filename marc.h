#ifndef __MARC_H
#define __MARC_H 1

#include <zlib.h>
#include <stdio.h> // FILE is defined here
#include <string.h>

// terminator constants
#define FIELD_TERMINATOR 0x1e
#define RECORD_TERMINATOR 0x1d
#define SUBFIELD_DELIMITER 0x1f

// error constants
#define MISSING_FIELD_TERMINATOR (1 << 0)
#define MISSING_RECORD_TERMINATOR (1 << 1)

typedef struct marcfield
{
  char *directory_entry, *data;
  int tag, length;
} marcfield;

typedef struct marcrec
{
  int length, base_address, field_count;
  char *data;
  marcfield *fields;
} marcrec;

typedef struct fieldspec
{
  int tag;
  char *subfields;
} fieldspec;

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
int marcrec_from_buffer(marcrec *rec, char *buf, int len);

/**
 * @brief print a marc record in human-readable format
 *
 * @param out the file to write to
 * @param rec the marc record to write
 * @param specs specification of the desired fields to print; if null, print the entire record
 * @return int the number of fields written
 */
int marcrec_print(FILE *out, const marcrec *rec, const fieldspec specs[]);

/**
 * @brief read a marcrec from a file
 *
 * if rec->fields is non-null this function will assume you want fields processed;
 * otherwise it will only read the raw record
 *
 * @param rec a pointer to an allocated marcrec object to be populated
 * @param in a pointer to an open FILE to read from
 * @return int the number of bytes read (-1 if the number of bytes was fewer than expected)
 */
int marcrec_read(marcrec *rec, gzFile in);

/**
 * @brief validate a marc record
 *
 * @param rec the record to validate
 * @return int 0 if the marc record has all the appropriate field/record terminators; otherwise non-zero
 */
int marcrec_validate(const marcrec *rec);

/**
 * @brief write a marc record to a file
 *
 * @param out the file to write to
 * @param rec the marc record to write
 * @return int the number of bytes written
 */
int marcrec_write(FILE *out, const marcrec *rec);

/**
 * @brief print a marc record in XML format
 *
 * @param out the file to write to
 * @param rec the marc record to write
 * @return int the number of records written
 */
int marcrec_xml(FILE *out, const marcrec *rec);

/**
 * @brief print a marc field in human-readable format
 *
 * @param out the file to write to
 * @param field the marc field to write
 * @param specs specification of the desired subfields to print; if null, print the entire field
 * @return int 0 if the field wasn't printed (i.e., didn't match any of the specs); 1 otherwise
 */
int marcfield_print(FILE *out, const marcfield *field, const fieldspec specs[]);

#endif

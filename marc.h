#ifndef __MARC_H
#define __MARC_H 1

#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

// terminator constants
#define FIELD_TERMINATOR 0x1e
#define RECORD_TERMINATOR 0x1d
#define SUBFIELD_DELIMITER 0x1f

// error constants
#define MISSING_FIELD_TERMINATOR (1 << 0)
#define MISSING_RECORD_TERMINATOR (1 << 1)
#define INVALID_LEADER_CHARACTER (1 << 2)

typedef struct marcfile
{
#ifdef USE_ZLIB
  gzFile gzf;
#else
  FILE *f;
#endif
} marcfile;

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
 * @brief open a file for reading marc records
 *
 * @param filename the name of the file to open
 * @param mode mode to open the file in (must be "r")
 * @return marcfile* a pointer to the open marcfile
 */
marcfile *marcfile_open(const char *filename, const char *mode);

/**
 * @brief convert a raw file descriptor into a marcfile
 *
 * @param fd the file descriptor to open
 * @return marcfile* a pointer to the open marcfile
 */
marcfile *marcfile_from_fd(int fd, char *mode);

/**
 * @brief convert an stdio FILE into a marcfile
 *
 * @param file the FILE to open
 * @return marcfile* a pointer to the open marcfile
 */
marcfile *marcfile_from_FILE(FILE *file, char *mode);

/**
 * @brief close the marcfile
 *
 * @param file the file to be closed
 */
void marcfile_close(marcfile *mf);

/**
 * @brief dynamically allocate a new marcrec
 *
 * @param nBytes the number of bytes to allocate for marc data
 * @param nFields the number of fields to allocate
 * @return marcrec* the newly-allocated marcrec
 */
marcrec *marcrec_alloc(int nBytes, int nFields);

/**
 * @brief free a marcrec
 * 
 * note that this will free the marcrec as well as rec->fields and rec->data; if
 * you don't want this you may set either (or both) field to 0 before calling
 * marcrec_free
 *
 * @param rec the record to free
 */
void marcrec_free(marcrec *rec);

/**
 * @brief read a marcrec from a buffer
 *
 * note that this function will consume the input buffer - i.e., will result in
 * a marcrec with pointers to `buf`
 *
 * @param rec a pointer to an allocated marcrec object to be populated; if null,
 *            and there is data remaining in buf, a new marcrec will be
 *            allocated that the caller is responsible for freeing
 * @param buf a pointer to a buffer containing a marc record
 * @param nBytes the number of bytes in the marc record, or 0 if the length is
 *               unknown and should be computed
 * @return marcrec* a pointer to the marcrec that was read, or 0 if buf had no
 *                  available bytes
 */
marcrec *marcrec_from_buffer(marcrec *rec, char *buf, int nBytes);

/**
 * @brief print a marc record in human-readable format
 *
 * @param out the file to write to
 * @param rec the marc record to write
 * @param specs specification of the desired fields to print; if null, print the
 *              entire record
 * @return int the number of fields written
 */
int marcrec_print(FILE *out, const marcrec *rec, const fieldspec specs[]);

/**
 * @brief read a marcrec from a file
 *
 * @param rec a pointer to an allocated marcrec object to be populated; if null,
 *            and there is data to be read, a new marcrec will be allocated that
 *            the caller is responsible for freeing
 * @param in a pointer to an open FILE to read from
 * @return marcrec* a pointer to the marcrec that was read, or 0 if in had no
 *                  available data
 */
marcrec *marcrec_read(marcrec *rec, marcfile *in);

/**
 * @brief validate a marc record
 *
 * @param rec the record to validate
 * @return int 0 if the marc record has all the appropriate field/record
 *             terminators; otherwise non-zero
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
 * @param specs specification of the desired subfields to print; if null, print
 *              the entire field
 * @return int 0 if the field wasn't printed (i.e., didn't match any of the
 *             specs); 1 otherwise
 */
int marcfield_print(FILE *out, const marcfield *field, const fieldspec specs[]);

#endif

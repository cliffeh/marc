#ifndef __MARC_H
#define __MARC_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// terminator constants
#define FIELD_TERMINATOR 0x1e
#define RECORD_TERMINATOR 0x1d
#define SUBFIELD_DELIMITER 0x1f

// validation constants
#define MISSING_FIELD_TERMINATOR (1 << 0)
#define MISSING_RECORD_TERMINATOR (1 << 1)
#define INVALID_LEADER_CHARACTER (1 << 2)

// error constants
#define LEADER_NOT_ENOUGH_BYTES 1
#define RECORD_NOT_ENOUGH_BYTES 2
#define INVALID_MARC_RECORD_LENGTH 3

// TODO consider using an XML library
// XML constants
#define MARC_XML_PREAMBLE                                                     \
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"                              \
  "<collection"                                                               \
  " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""                  \
  " xmlns=\"http://www.loc.gov/MARC21/slim\""                                 \
  " xsi:schemaLocation=\"http://www.loc.gov/MARC21/slim"                      \
  " http://www.loc.gov/standards/marcxml/schema/MARC21slim.xsd\">"

#define MARC_XML_POSTAMBLE "</collection>"

/**
 * @brief validate a marc filter
 *
 * a valid filter either consists of the single word "leader" (case
 * insensitive), OR three digits representing a desired field tag (possibly
 * followed by an arbitrary number of subfield delimiters)
 *
 * in a regex: leader|\d\d\d.*
 *
 */
#define MARC_VALID_FILTER(spec)                                               \
  ((strcasecmp ("leader", spec) == 0)                                         \
   || ((strlen (spec) >= 3) && isdigit (*spec) && isdigit (*(spec + 1))       \
       && isdigit (*(spec + 2))))

typedef struct marcfile marcfile;

typedef struct marcfield marcfield;

typedef struct marcrec marcrec;

/**
 * @brief open a file for reading marc records
 *
 * @param filename the name of the file to open
 * @return marcfile* a pointer to an open marcfile, or null on error
 */
marcfile *marcfile_open (const char *filename);

/**
 * @brief convert a raw file descriptor into a marcfile
 *
 * @param fd the file descriptor to open
 * @return marcfile* a pointer to an open marcfile, or null on error
 */
marcfile *marcfile_from_fd (int fd);

/**
 * @brief convert a FILE into a marcfile
 *
 * @param file the FILE to open
 * @return marcfile* a pointer to an open marcfile, or null on error
 */
marcfile *marcfile_from_FILE (FILE *file);

/**
 * @brief close the marcfile
 *
 * @param file the marcfile to be closed
 */
void marcfile_close (marcfile *mf);

/**
 * @brief checks if the error indicator for the underlying stream has been set
 *
 * @param mf the marcfile to check for errors
 * @param msg a buffer to put a message describing the error into; if null this
 *            parameter will be ignored
 * @return int the error indicator that was set, or 0 if no error
 */
int marcfile_error (marcfile *mf, char *msg);

/**
 * @brief dynamically allocate a new marcrec
 *
 * @param nBytes the number of bytes to allocate for marc data
 * @param nFields the number of fields to allocate
 * @return marcrec* the newly-allocated marcrec
 */
marcrec *marcrec_alloc (int nBytes, int nFields);

/**
 * @brief free a marcrec
 *
 * note that this will free the marcrec as well as rec->fields and rec->data;
 * if you don't want this you may set either (or both) field to 0 before
 * calling marcrec_free
 *
 * @param rec the record to free
 */
void marcrec_free (marcrec *rec);

/**
 * @brief read a marcrec from a buffer
 *
 * note that this function will consume the input buffer - i.e., will result in
 * a marcrec with pointers to `buf`
 *
 * @param rec a pointer to an allocated marcrec object to be populated; if
 *            null, and there is data remaining in buf, a new marcrec will be
 *            allocated that the caller is responsible for freeing
 * @param buf a pointer to a buffer containing a marc record
 * @param nBytes the number of bytes in the marc record, or 0 if the length is
 *               unknown and should be computed
 * @return marcrec* a pointer to the marcrec that was read, or 0 if buf had no
 *                  available bytes
 */
marcrec *marcrec_from_buffer (marcrec *rec, char *buf, int nBytes);

/**
 * @brief print a marc record in human-readable format
 *
 * @param out the file to write to
 * @param rec the marc record to write
 * @param specs specification of the desired fields to print; if null, print
 *              the entire record
 * @return int the number of fields written
 */
int marcrec_print (FILE *out, const marcrec *rec, const char *filters[]);

/**
 * @brief read a marcrec from a file
 *
 * @param rec a pointer to an allocated marcrec object to be populated; if null
 *            and there is data to be read, a new marcrec will be allocated
 * that the caller is responsible for freeing
 * @param in a pointer to an open marcrec to read from
 * @return marcrec* a pointer to the marcrec that was read, or 0 if either eof
 *                  was reached or there was an error
 */
marcrec *marcrec_read (marcrec *rec, marcfile *in);

/**
 * @brief write a marc record to a file
 *
 * @param out the file to write to
 * @param rec the marc record to write
 * @param specs ignored; may be used in future, but is currently just here to
 *              match the function signature of the other print functions
 * @return int the number of bytes written
 */
int marcrec_write (FILE *out, const marcrec *rec, const char *filters[]);

/**
 * @brief print a marc record in XML format
 *
 * @param out the file to write to
 * @param rec the marc record to write
 * @param specs ignored; may be used in future, but is currently just here to
 *              match the function signature of the other print functions
 * @return int the number of records written
 */
int marcrec_xml (FILE *out, const marcrec *rec, const char *filters[]);

/**
 * @brief print a marc field in human-readable format
 *
 * @param out the file to write to
 * @param field the marc field to write
 * @param specs specification of the desired subfields to print; if null, print
 *              the entire field
 * @return int the number of subfields printed
 */
int marcfield_print (FILE *out, const marcfield *field, const char *subfields);

/**
 * @brief determines whether a marc record is valid
 *
 * @param rec the record to validate
 * @return int 0 if the record is valid, non-zero if the record has issues
 * (e.g., missing terminator(s), invalid characters, etc.)
 */
int marcrec_validate (marcrec *rec);

#endif

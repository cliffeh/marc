#ifndef __MARC_FIELDS_H
#define __MARC_FIELDS_H 1

typedef struct fieldspec
{
    int len;
    const char *spec;
} fieldspec;

/**
 * @brief validates a fieldspec
 * 
 * a valid fieldspec consists of three numeric digits followed by zero or more characters representing subfield codes
 *
 * examples:
 *   245
 *   245a
 *   245abc
 *
 * @param fs the fieldspec to validate
 * @param len the length of the fieldspec
 * @return int 0 iff the fieldspec is valid
 */
int fieldspec_validate(fieldspec *fs);

#endif

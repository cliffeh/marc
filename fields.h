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
 * @param fs the fieldspec to validate
 * @param len the length of the fieldspec
 * @return int 0 iff the fieldspec is valid
 */
int validate_fieldspec(fieldspec *fs);

#endif

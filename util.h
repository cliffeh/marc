#ifndef __MARC_UTIL_H
#define __MARC_UTIL_H 1

/**
 * @brief convert a sequence of digits into a positive integer
 *
 * @param p pointer to the beginning of the sequence
 * @param n length of the sequence
 * @return int the number represented by the sequence, or -1 if a non-digit
 *             character is encountered
 */
int atoin(const char *p, int n);

#endif

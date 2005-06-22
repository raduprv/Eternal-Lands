#ifndef MKEYSORT_H
#define MKEYSORT_H

/*!
 * \ingroup misc
 * \brief sorts an array of DISTINCT strings.
 *        Sorts an array of DISTINCT strings. 
 *
 * \param A a pointer to the array.
 * \param n the size of the array.
 */
void mkeysort(char * A[], int n);

/*!
 * \ingroup misc
 * \brief searches a sorted array of DISTINCT strings for a string
 *        Searches a sorted array of DISTINCT strings for a string.
 *
 * \param A   a pointer to the array.
 * \param n   the number of elements in the array.
 * \param key the string to search.
 * \return The index of the key string, or -1 if it is not found.
 */
int mkeyfind(char * A[], int n, const char * key);

#endif

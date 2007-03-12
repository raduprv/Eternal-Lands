/*!
 * \file
 * \brief generic multi-key sorting and related functions
 */

#ifndef __SORT_H__
#define __SORT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*! \name callback to get a key */
typedef const char * (* gen_mkey_retrieve_func)(void * pdata, int i) ;
/*! \name callback to swap two entries */
typedef void (* gen_mkey_swap_func)(void * pdata, int i, int j);
/*! \name callback to copy an entry */
typedef void (* gen_mkey_copy_func)(void * psrc, int i, void * pdst, int j);

/*!
 * \ingroup misc
 * \brief sorts an array of DISTINCT strings.
 *        Sorts an array of DISTINCT strings. 
 *
 * \param pdata a pointer to be passed to the callbacks.
 * \param get   a callback that retrieves the key of the i-th entry
 * \param swap  a callback that exchanges entries i and j
 * \param n     the size of the array.
 */
void gen_mkeysort(void * pdata, gen_mkey_retrieve_func get, gen_mkey_swap_func 
		swap, int n);

/*!
 * \ingroup misc
 * \brief searches a sorted array of DISTINCT strings for a string
 *        Searches a sorted array of DISTINCT strings for a string.
 *
 * \param pdata a pointer to be passed to the callback.
 * \param get   a callback that retrieves the key of the i-th entry
 * \param n     the number of elements in the array.
 * \param key   the string to search.
 * \return The index of the key string, or -1 if it is not found.
 */
int gen_mkeyfind(void * pdata, gen_mkey_retrieve_func get, int n, const char * 
		key);

/*! \ingroup misc
 * \brief merges two contiguous sorted arrays of distinct strings
 *        Merges two contiguous sorted arrays of distinct strings.
 *
 * \param psrc a pointer to the source data.
 * \param pdst a pointer to the destination data. Don't let the function 
 *             overwrite its own input, otherwise results are undefined.
 * \param n the number of elements in the first part
 * \param m the number of elements in the second part
 */
void gen_mkeymerge(void * psrc, void * pdst, gen_mkey_retrieve_func get, 
		gen_mkey_copy_func put, int n, int m);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ndef __GEN_MKEYSORT_H__

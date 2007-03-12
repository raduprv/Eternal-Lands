/*!
 * \file
 * \brief misc symbol table
 */

#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name symbol table data
 */
typedef union {
	int    num;
	void * ptr;
} st_data;

/*! 
 * \name symbol table entry
 */
typedef struct {
	const char * symbol; /*< symbol */
	st_data      data;   /*< data associated with \a symbol */
} st_entry;

/*!
 * symbol table structure
 */
typedef struct {
	int added,         /*!< the number of uncommitted entries */
			capacity,      /*!< the total capacity of the table */
			committed;     /*!< the number of committed entries */
	st_entry *entries, /*!< the array with the entries */
			*shadow;       /*!< shadow array used for commits */
} symbol_table;

/*!
 * \ingroup misc
 * \brief creates a new symbol table
 *        Creates a new symbol table.
 *
 * \param capacity the number of entries the table shall be able to hold
 * \retval         the newly created table
 */
symbol_table * st_create(int capacity);

/*!
 * \ingroup misc
 * \brief adds a numeric value to the table
 *        Adds a numeric value to the table. O(1)
 *
 * \param st     the symbol table to add to
 * \param symbol the symbol to add
 * \param data   the number to associate with the symbol
 */
void st_addnum(symbol_table * st, const char * symbol, int data);

/*!
 * \ingroup misc
 * \brief adds a pointer to the table
 *        Adds a pointer to the table. O(1)
 *
 * \param st     the symbol table to add to
 * \param symbol the symbol to add
 * \param data   the pointer to associate with the symbol
 */
void st_addptr(symbol_table * st, const char * symbol, void * data);

/*!
 * \ingroup misc
 * \brief commits all added symbols to the symbol table
 *        Commits all added symbols to the symbol table. O(n log n)
 *
 * \param st the symbol table to commmit
 */
void st_commit(symbol_table * st);

/*!
 * \ingroup misc
 * \brief looks up a symbol in the table
 *        Looks up a symbol in the table. Only committed symbols a searched. 
 *        O(log n)
 *
 * \param st     the symbol table to look in
 * \param symbol the symbol to look up
 * \retval       a pointer to the data associated with the symbol in a
 *               \see st_data union. If the symbol is not found, NULL is 
 *               returned.
 */
st_data * st_lookup(symbol_table * st, const char * symbol);

/*!
 * \ingroup misc
 * \brief destroys a symbol table
 *        Destroys a symbol table.
 *
 * \param st the table to destroy
 */
void st_destroy(symbol_table * st);

/*!
 * \ingroup misc
 * \brief destroys a symbol table and frees user data
 *        Destroys a symbol table and frees user data.
 *
 * \param st       the table to destroy
 * \param freedata callback to destroy user data
 */
void st_destroyExt(symbol_table * st, void (* freedata)(void *));

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ndef __SYMBOL_TABLE_H__

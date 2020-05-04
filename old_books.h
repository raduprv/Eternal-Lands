/*!
 * \file
 * \ingroup misc
 * \brief Used for the implementation of books in EL
 */
#ifndef __BOOKS_H__
#define __BOOKS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*! The book window*/
extern int book_win;

/*! The paper window*/
extern int paper_win;

/*! The ID of the book currently open*/
extern int book_opened;

/*!
 * \ingroup	books_window
 * \brief	Frees the memory allocated for books
 *
 * 		Frees the memory allocated for books
 */
void free_books(void);

/*!
 * \ingroup	books_window
 * \brief	Closes the book window with the given id
 *
 * 		Closes the book window with the given id, if it's opened
 *
 * \param	book_id The unique book ID
 */
void close_book(int book_id);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

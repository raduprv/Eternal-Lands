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
 * \ingroup	network_books
 * \brief	Opens a local book.
 *
 * 		Opens a local book - if it resides in memory already it will not be reloaded, but just redisplayed.
 *
 * \param	data The data from the server
 * \param	len The length of the data
 *
 * \callgraph
 */
void read_local_book (const char * data, int len);

/*!
 * \ingroup	books_window
 * \brief	Reads some books that will not be asked for server-side
 *
 * 		Reads a few local books that we will not ask the server about...
 */
void init_books();

/*!
 * \ingroup	books_window
 * \brief	Frees the memory allocated for books
 *
 * 		Frees the memory allocated for books
 */
void free_books();

/*!
 * \ingroup	network_books
 * \brief	Selects the parser for the book send from the server
 *
 * 		When the server sends a book to the client the first byte will be used to specify the parser that's going to be used - whether the book is local and uses xml or if it's server-side and uses the network data parser.
 *
 * \param	data The network data
 * \param	len The length of the data
 *
 * \callgraph
 */
void read_network_book (const char * data, int len);

/*!
 * \ingroup	network_books
 * \brief	Opens the book with the given ID
 *
 * 		Opens the book with the given ID - if the book isnt found it will send a SEND_BOOK followed by the ID to the server.
 *
 * \param	id The book ID
 *
 * \callgraph
 */
void open_book(int id);

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

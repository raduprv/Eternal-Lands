/*!
 * \file
 * \brief Used for the implementation of books in EL
 * \ingroup misc
 */
#ifndef __BOOKS_H__
#define __BOOKS_H__

/*!
 * The image structure - holds the filename, u/v-coordinates and the texture ID
 */
typedef struct {
	char file[200];/*!< The filename of the image for future references*/
	
	/*! \{ */
	int x;/*! < The x offset */
	int y;/*! < The y offset */
	/*! \} */
	/*! \{ */
	int w;/*! < The width of the image*/
	int h;/*! < The height of the image */
	/*! \} */
	
	int texture; /*!< The texture ID in the texture_cache - NOT the GL-texture ID*/
	/*! \{ */
	int u[2]; /*!< The start and end U-coordinates*/
	int v[2]; /*!< The start and end V-coordinates*/
	/*! \} */
} _image;

/*!
 * The page structure - holds the lines in a char ** array, 1 image and the page number (written in the bottom of the page).
 */
typedef struct {
	char ** lines; /*!< The lines on one page. The char ** is ended by a NULL-pointer (lines[i]==NULL)*/
	_image * image;/*!< The page can so far only have 1 image - this should be sufficient...*/
	int page_no; /*!< The page number */
} page;

/*!
 * The book structure, containing the pages, title and other variables telling the book type etc.
 */
struct _book {
	char title[35];	 /*!< The book title*/
	int id;		 /*!< The books unique ID (each book will have to have a different ID)*/
	
	int type;	 /*!< The type of book (currently the only available types are paper or book)*/
	
	int no_pages; /*!< The number of pages*/
	page ** pages; /*!< The pages - the last page is NULL*/
	int max_width;/*!< The page width in characters*/
	int max_lines; /*!< The max. number of lines*/
	
	int active_page; /*!< The currently active page*/

	struct _book * next; /*!< The next book*/
};
/*! The typedef for the _book structure - just for own convinience...*/
typedef struct _book book;

/*! The book texture*/
extern int book1_text;

/*!
 * \ingroup	books
 * \brief	Creates a book of the given type.
 *
 * 		Creates a book of the given type, with the given title and ID and adds it to the book collection.
 *
 * \param	title The title of the book
 * \param	type The type of book (1==Paper, 2==Book)
 * \param	id The unique ID of the book
 * \return	A pointer to the created book
 */
book * create_book(char * title, int type, int id);

/*!
 * \ingroup	books
 * \brief	Adds a new page to the given book
 *
 * 		Adds a new page to the given book using realloc. Always sets the page size to 1 more than currently used, and makes sure that the last pointer is pointing to NULL.
 *
 * \param	b A pointer to the book you wish to add a page to.
 * \return	A pointer to the newly created page.
 */
page * add_page(book * b);

/*!
 * \ingroup	books
 * \brief	Creates a new image to be used for a book.
 *
 * 		Loads the file and creates a texture from the given image. If the texture is not found it will free the memory allocated and return NULL. 
 * 		After generating the texture (as a colour-key) it sets the x, y, w, h, u, v coordinates in the _image structure.
 *
 * \param	file The filename of the texture
 * \param	x The x position
 * \param	y The y position
 * \param	w The width
 * \param	h The height
 * \param	u_start The start u texture coordinate
 * \param	v_start The start v texture coordinate
 * \param	u_end The end u texture coordinate
 * \param	v_end The end v texture coordinate
 * \return	Returns a pointer to the image created.
 */
_image *create_image(char * file, int x, int y, int w, int h, float u_start, float v_start, float u_end, float v_end);

/*!
 * \ingroup	books
 * \brief	Frees the memory allocated by a page
 * 
 * 		Frees the memory allocated by a page (including lines)
 *
 * \param	p The page which memory you wish to free
 * \return	None
 */
void free_page(page * p);

/*!
 * \ingroup	books
 * \brief	Frees the memory allocated by a book
 *
 * 		Goes through the book and frees the pages (calls free_page), then frees the memory allocated by the book
 *
 * \param	b The book which memory you wish to free.
 * \return	None
 */
void free_book(book * b);

/*!
 * \ingroup	books
 * \brief	Frees all memory allocated by books
 * 
 * 		Frees all memory allocated by books - parses through the \c books variable and frees every loaded book. (books->next)
 *
 * \param	None
 * \return	None
 */
void free_books(void);

/*!
 * \ingroup	books
 * \brief	Checks if we have the book with the given ID
 *
 * 		Checks if we have the book with the given ID
 *
 * \param	id The ID of the book
 * \return	1 if the book is found, 0 if not.
 */
int have_book(int id);

/*!
 * \ingroup	books
 * \brief	Finds a book with the given ID and returns a pointer to it
 *
 * 		Finds a book with the given ID and returns a pointer to it.
 *
 * \param	id The ID of the book
 * \return	A pointer to the book if found. If it does not exist, it returns NULL.
 */
book * get_book(int id);

/*!
 * \ingroup	books
 * \brief	Adds a book to the \c books variable.
 *
 * 		Adds a book to the \c books variable.
 */
void add_book(book *bs);

/*!
 * \ingroup	books
 * \brief	"Wraps" some text around an image
 *
 * 		Wraps the text pointed to by \c last to the image with the given width and x position and puts it in \c line.
 * 		It will only wrap words that are seperated by a space - if the word is too long it will not be wrapped (hence the image will not get any text next to it).
 *
 * \param	line The line you wish to wrap the text to
 * \param	w The width of the image
 * \param	x The x position
 * \param	max_width The maximum width of the text in \c line
 * \param	last The text you wish to wrap into the line
 * \return	Returns a pointer to the next character in last that has not been wrapped
 */
char * wrap_line_around_image(char * line, int w, int x, int max_width, char * last);

/*!
 * \ingroup	books
 * \brief	Adds an image to the given page and wraps the text around it
 *
 * 		Adds an image to the given page and wraps the text around it - calls wrap_line_around_image. Also adds an extra page if the picture or ext cannot be in the current page.
 *
 * \param	text The image text
 * \param	img The image you wish to add to the page
 * \param	b The book the page belongs to
 * \param	p The page you wish to add the image and text to
 * \return	Returns a pointer to the active page or NULL on failure.
 * \callgraph
 * \sa 		wrap_line_around_image
 */
page * add_image_to_page(char * text, _image *img, book * b, page *p);

/*!
 * \ingroup	books
 * \brief	Adds a string to the given page
 * 
 * 		Adds a string to the given page and wraps the text to newlines when it's needed. Also adds new pages if the text cannot be in the active page.
 *
 * \param	str The text to add
 * \param	type The type of text (_TITLE, _AUTHOR, _TEXT)
 * \param	b The active book
 * \param	p The page you wish to add the text to
 * \return	Returns a pointer to the active page or NULL on failure.
 */
page * add_str_to_page(char * str, int type, book *b, page *p);

/*!
 * \ingroup	books
 * \brief	Used by the XML book parser to add an image
 *
 * 		Used by the XML book parser to add an image
 *		Here's an example:
 *		
 * \code
 * 		<image x="50" y="120" w="120" h="120" src="./maps/seridia.bmp">Seridia is the first continent in EL</image>
 * \endcode
 *		
 *		The above would add an image at 50,120 (px) in the book with the width and height 120px. Furthermore it'd add the text "Seridia is the first continent in EL" to the next of the image - at least as much of the text as the wrapper allows.
 *
 * \param	cur The current xmlNode
 * \param	b The current book
 * \param	p The current page
 * \return	None
 * \callgraph
 */
void add_xml_image_to_page(xmlNode * cur, book * b, page *p);

/*!
 * \ingroup	books
 * \brief	Used by the XML book parser to add text to a page
 * 
 * 		Used by the XML book parser to add text to a page
 *		Here's an example:
 * 		
 * \code
 * 		<text>This text will show up as normal dark-brown text...</text>
 * 		<author>Whilst this text will be highlighted in yellow/brown colour</author>
 * 		<title>And finally this text will be highlighted in a red/brown colour</title>
 * \endcode
 *
 *		The parser understands newlines (both unix and windows) and will automatically add a new line if such is found.
 *
 *		If the UTF8Toisolat1 fails converting the text into iso-8859-1, it will log an error telling in which tag and at what line the error occured.
 *
 * \param	cur The current xmlNode
 * \param	in The text you are adding
 * \param	type The type of text
 * \param	b The current book
 * \param	p The current page
 * \return	None
 * \callgraph
 */
void add_xml_str_to_page(xmlNode * cur, int type, book * b, page *p);

/*!
 * \ingroup	books
 * \brief	Used by the XML book parser to add a new page to the book
 * 		
 * 		Used by the XML book parser to add a new page to the book. It forces a new page to be created.
 *
 * \param	cur The current xmlNode
 * \param	b The book we're parsing
 * \return	None
 */
void add_xml_page(xmlNode *cur, book * b);

/*!
 * \ingroup	books
 * \brief	Parses an XML-defined book
 *
 * 		Parses an XML-defined book. First it creates the book of \c type with the given \c title and \c id , then adds pages to it
 *
 * \param	in The root node
 * \param	title The title of the book
 * \param	type The type of book (Paper==1, Book==2)
 * \param	id The book's ID
 * \return	Returns a pointer to the created book
 */
book * parse_book(xmlNode *in, char * title, int type, int id);

/*!
 * \ingroup	books
 * \brief	Initial parse of an XML-defined book
 *
 * 		The function does the initial parse of an xml-defined book and makes sure it has the right format. If not it will refuse to parse the book.
 * 		An example book would be the following:
 *
 * \code
 * <?xml version="1.0" encoding="utf8"?>
 * <book title="The rise and fall of Sedicolis" id="1" type="2">
 *	 <page>
 *	 	<title>A small sheep</title>
 *		<author>       Unknown</author>
 *		<image x="50" y="120" w="120" h="120" src="./maps/seridia.bmp"></image>
 *	</page>
 *	<page>
 *		Once upon a time there was a country of milk and honey...
 *	</page>
 * </book>	                                                                
 * \endcode
 * 
 * \param	file The file you wish to open
 * \return	Returns a pointer to the book loaded or NULL on error
 * \callgraph
 */
book * read_book(char * file);

/*!
 * \ingroup	network_books
 * \brief	Opens a local book.
 *
 * 		Opens a local book - if it resides in memory already it will not be reloaded, but just redisplayed.
 *
 * \param	data The data from the server
 * \param	len The length of the data
 * \return	None
 */
void read_local_book(char * data, int len);

/*!
 * \ingroup	network_books
 * \brief	Parses the image-data from the server
 *
 * 		Parses the image data from the server and adds the image to the book.
 *
 * \param	data The data from the server
 * \param	b The book being parsed
 * \param	p The current page
 * \return 	Returns the active page
 * \todo 	Finish this function - it is supposed to get the image data send from the server and add it to the given page.
 */
page * add_image_from_server(char *data, book *b, page *p);

/*!
 * \ingroup	network_books
 * \brief	Parses the book from the server
 *
 * 		When the server sends a book to the client, that is not supposed to be stored in a local XML-book repository, this function will parse and display the book.
 *
 * \param	data The network data
 * \param	len The length of the data
 * \return	None
 * \todo 	Finish this function.
 */
void add_book_from_server(char * data, int len);

/*!
 * \ingroup	books_win
 * \brief	Displays an image
 *
 * 		The function displays the image given by i. Called by display_page
 *
 * \param	i The image you want displayed
 * \return	None
 * \sa 		display_page
 */
void display_image(_image *i);

/*!
 * \ingroup	books_win
 * \brief	Displays a page in the book
 * 
 * 		The function displays a page in the book and it's image.
 *
 * \param	b The book you're reading
 * \param	p The page you want to display currently
 * \return	None
 * \sa		display_book
 */
void display_page(book * b, page * p);

/*!
 * \ingroup	books_win
 * \brief	Displays the book of the given type.
 *
 *		Displays the book of the given type.
 * 
 * \param	b The current book
 * \param	type The type of book
 * \return	None
 * \callgraph
 */
void display_book(book * b, int type);

/*!
 * \ingroup	books_win
 * \brief	Handles initial displayal of the book
 *
 * 		Handles initial displayal of the book
 *
 * \param	win The window that's being drawn
 * \return	Returns true
 */
int display_book_handler(window_info *win);

/*!
 * \ingroup	books_win
 * \brief	Handles mouse-clicks in the book window
 *
 * 		Handles mouse-clicks in the book window (switches pages and such)
 *
 * \param	win The window that's being drawn
 * \param	mx The mouse x position in the window
 * \param	my The mouse y position in the window
 * \param	flags The flags
 * \return	Returns true
 */
int click_book_handler(window_info *win, int mx, int my, Uint32 flags);

/*!
 * \ingroup	books_win
 * \brief	Displays a book window (handles both paper and books)
 *
 * 		Displays a book window (handles both paper and books)
 *
 * \param	b A pointer to the book you wish to display
 * \return	None
 */
void display_book_window(book *b);

#endif

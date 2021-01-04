/*!
 * \file books.h
 * \ingroup books_window
 * \brief Classes and functions dealing with in-game books
 */

#ifndef BOOKS_H
#define BOOKS_H

#include "elwindows.h"

#ifdef __cplusplus

namespace eternal_lands
{

/*!
 * \ingroup books_window
 * \brief Enumeration for book content.
 *
 * This enumeration describes the different kinds of possible content in the
 * books.
 */
enum ContentType
{
	//! The book title
	TITLE,
	//! The name of the book's author
	AUTHOR,
	//! Regular text content
	TEXT,
	//! An image
	IMAGE,
	//! A caption text belonging to an image
	CAPTION,
	//! A page number
	PAGE_NR
};


/*!
 * \ingroup books_window
 * \brief A class for images in books
 *
 * Class BookImage represents an image that appears in an EL book.
 */
class BookImage
{
public:
	/*!
	 * \brief Create a new empty image
	 *
	 * Create a new image without associated data. This constructor is in fact
	 * only used to create a BookItem which does not contain an image.
	 * Use of \c std::optional might prevent the need for that, but this
	 * feature was introduced in C++17, which we are not using yet.
	 */
	BookImage(): _file_name(), _x(0), _y(0), _width(0), _height(0),
		_texture(Uint32(-1)), _u_start(0.0), _v_start(0.0), _u_end(0.0), _v_end(0.0) {}
	/*!
	 * \brief Create a new image
	 *
	 * Create a new image that displays the (\a u_start, \a v_start) - (\a u_end, \a v_end)
	 * part of the image in file \a file name, on a page in the book at position
	 * \a x, \a y with width \a width and height \a height.
	 * \note The positions and widths are scaled with the UI scale factor in
	 * the actual display code.
	 * \param file_name The file name of the image
	 * \param x         The x coordinate of the image on the page
	 * \param y         The y coordinate of the image on the page
	 * \param width     The (unscaled) width in pixels of the image when drawn
	 * \param height    The (unscaled) height in pixels of the image when drawn
	 * \param u_start   The start x-coordinate of the image part to display
	 * \param v_start   The start y-coordinate of the image part to display
	 * \param u_end     The end x-coordinate of the image part to display
	 * \param v_end     The end y-coordinate of the image part to display
	 */
	BookImage(const std::string& file_name, int x, int y, int width, int height,
		float u_start, float v_start, float u_end, float v_end);

	//! Return the x-coordinate of the image position
	int x() const { return _x; }
	//! Return the y-coordinate of the image position
	int y() const { return _y; }
	//! Return the width of the image
	int width() const { return _width; }
	//! Return the heght of the image
	int height() const { return _height; }

	/*!
	 * \brief Display the picture
	 *
	 * Draw the picture on the book page.
	 */
	void display() const;

	/*!
	 * \brief Return a scaled image
	 *
	 * Return a scaled version of this image, where all coordinates and
	 * dimensions are scaled by a factor \a zoom. This is used in the layout
	 * phase, when the book content is scaled with the UI scale factor,
	 * and the actual dimensions need to be known.
	 * \param zoom The scale factor for the image
	 * \return A scaled copy of this image
	 */
	BookImage scaled(float zoom) const
	{
		return BookImage(_file_name, _x * zoom, _y * zoom, _width * zoom, _height * zoom,
			_u_start, _v_start, _u_end, _v_end);
	}

private:
	//! The file name of the image
	std::string _file_name;
	//! The x-coordinate of the image position
	int _x;
	//! The y-coordinate of the image position
	int _y;
	//! The width of the drawn image
	int _width;
	//! The height of the drawn image
	int _height;
	//! The identifier in the texture cache for this image
	Uint32 _texture;
	//! The start x-coordinate of the part of the image to show
	float _u_start;
	//! The start y-coordinate of the part of the image to show
	float _v_start;
	//! The end x-coordinate of the part of the image to show
	float _u_end;
	//! The end y-coordinate of the part of the image to show
	float _v_end;
};


/*!
 * \ingroup books_window
 * \brief Class for a block of text
 *
 * Class TextBlock holds the data for a block of text in the book. It contains
 * the actual text itself, as well as information on how and where to draw it
 * ont he page.
 */
class TextBlock
{
public:
	/*!
	 * \brief Create a new TextBlock
	 *
	 * Create a new TextBlock with content \a text of type \a type, starting
	 * at position \a x, \a y on the page with dimensions \a width and \a height,
	 * to be drawn with scale factor \a zoom. It is assumed that the text
	 * has been formatted to fit into the specified dimensions, no measures are
	 * taken here to ensure this. The parameter \a line_spacing is a scale factor
	 * for the line spacing between consecutive lines of text in a book. Typically,
	 * the line spacing is books is somewhat smaller than the default, with
	 * \a line_spacing = 0.9.
	 * \param text         The actual text to draw
	 * \param type         The kind of text in the block
	 * \param x            The x coordinate of the left side of the text box
	 * \param y            The y coordinate of the top of the text box
	 * \param width        The width of the box
	 * \param height       The height of the box
	 * \param zoom         The scale factor for the text
	 * \param line_spacing The scale factor for the spacing between consecutive lines of text
	 */
	TextBlock(const ustring& text, ContentType type, int x, int y,
		int width, int height, float zoom, float line_spacing=1.0):
		_text(text), _type(type), _x(x), _y(y), _width(width), _height(height),
		_zoom(zoom), _line_spacing(line_spacing) {}

	//! Return the x-coordinate of the left side of the box
	int x() const { return _x; }
	//! Return the y-coordinate of the top of the box
	int y() const { return _y; }
	//! Return the width of the box
	int width() const { return _width; }
	//! Return the height of the box
	int height() const { return _height; }

	/*!
	 * \brief Display the text box
	 *
	 * Draw the contents of this text box on the page
	 */
	void display() const;

private:
	//! The actual text
	ustring _text;
	//! The type of text to draw
	ContentType _type;
	//! The x coordinate of the left side of the text box
	int _x;
	//! The y coordinate of the top of the text box
	int _y;
	//! The width of the box
	int _width;
	//! The height of the box
	int _height;
	//! The scale factor for the text
	float _zoom;
	//! Scale factor for sacing between two lines
	float _line_spacing;
};


/*!
 * \ingroup books_window
 * \brief Class for a page in a book
 *
 * Class Page hold the contents for a single page in a book, and defines
 * a number of functions to determine where new contents can be placed.
 */
class Page
{
public:
	/*!
	 * \brief Create a new Page
	 *
	 * Create a new, empty Page of height \a page_height for a book.
	 */
	Page(int page_height): _texts(), _images(),
		_free_ranges(1, std::make_pair(0, page_height)) {}

	/*!
	 * \brief Check if a specific portion of the page is free
	 *
	 * Check if the part of the page between \a y_begin and \a y_end is
	 * currently empty, and can be used for new content.
	 *
	 * \param y_begin The top y coordinate of the range to check
	 * \param y_end   The bottom y coordinate of the range to check
	 * \a return \c true if the range is free, \c false otherwise
	 */
	bool has_free_range(int y_begin, int y_end) const
	{
		for (const auto& range: _free_ranges)
		{
			if (range.first <= y_begin && range.second >= y_end)
				return true;
		}
		return false;
	}
	/*!
	 * \brief Search for a free range on the page
	 *
	 * Find the first position on the page where a block of \a min_height
	 *  pixels high that starts at a multiple of \a min_height pixels is free.
	 *
	 * \param min_height The minimum height of the free block
	 * \return The top and bottom coordinates of the free block. If no
	 * 	free block can be found on this page, (-1, -1) is returned.
	 */
	std::pair<int, int> find_free_range_aligned(int min_height) const
	{
		for (const auto& range: _free_ranges)
		{
			if (range.second - range.first >= min_height)
			{
				// Align to whole block of min_height pixels
				int y_begin = ((range.first + min_height - 1) / min_height) * min_height;
				if (range.second - y_begin >= min_height)
					return std::make_pair(y_begin, range.second);
			}
		}
		return std::make_pair(-1, -1);
	}

	/*!
	 * \brief Add a block of text
	 *
	 * Add the block of text \a block to this page, and mark the range
	 * between \a y_begin and \a y_end as occupied.
	 * \note The range to occupy may be larger than the actual block contents,
	 * for instance to prevent new content from being inserted before this
	 * text block, or to introduce margins.
	 *
	 * \param block   The text block to add to the page
	 * \param y_begin The top coordinate of the range to occupy
	 * \param y_end   The bottom coordinate of the rang to occupy
	 */
	void add_text_block(TextBlock &&block, int y_begin, int y_end)
	{
		_texts.push_back(block);
		occupy_range(y_begin, y_end);
	}
	/*!
	 * \brief Add an image
	 *
	 * Add the image \a image to this page, and mark the range between
	 * \a y_begin and \a y_end as occupied.
	 * \note The range to occupy may be larger than the actual block contents,
	 * for instance to prevent new content from being inserted before this
	 * text block, or to introduce margins.
	 *
	 * \param image   The image to add to the page
	 * \param y_begin The top coordinate of the range to occupy
	 * \param y_end   The bottom coordinate of the rang to occupy
	 */
	void add_image(BookImage &&image, int y_begin, int y_end)
	{
		_images.push_back(image);
		occupy_range(y_begin, y_end);
	}

	/*!
	 * \brief Display ths page
	 *
	 * Draw the contents of this page on the screen.
	 */
	void display() const
	{
		for (const auto& image: _images)
			image.display();
		for (const auto& text: _texts)
			text.display();
	}

private:
	//! The block of text on this page
	std::vector<TextBlock> _texts;
	//! The images on this page
	std::vector<BookImage> _images;
	//! y-coordinates of the unoccupied portions of this page
	std::vector<std::pair<int, int>> _free_ranges;

	/*!
	 * \brief Mark a range as occupied
	 *
	 * Mark the part of the page between \a y_begin and \a y_end as occupied,
	 * and unavailable for new content.
	 */
	void occupy_range(int y_begin, int y_end);
};


/*!
 * \ingroup books_window
 * \brief Class for an item of content in a book
 *
 * Class BookItem holds a single item of content for the book. This can be
 * either a piece of text, or an image with an associated caption. This class
 * is used to store the book's content only, without formatting. The contents
 * stored int object of class BookItem are laid out in a later phase, and
 * a single object may span multiple pages in the formatted book.
 */
class BookItem
{
public:
	/*!
	 * \brief Create a new text item
	 *
	 * Create a new text item of type \a type, with text contents \a text.
	 * \param type The kind of text to draw
	 * \param text The actual text itself
	 */
	BookItem(ContentType type, const ustring& text):
		_type(type), _text(text), _image() {}
	/*!
	 * \brief Create a new image item
	 *
	 * Create a new image item for image \a image, with (possibly empty) caption
	 * text \a caption.
	 * \param image   The image itself
	 * \param caption The caption text associated with the image
	 */
	BookItem(BookImage&& image, const ustring& caption):
		_type(IMAGE), _text(caption), _image(image) {}

	//! Return the type of the stored item
	ContentType type() const { return _type; }
	//! Return the text associated with this item
	const ustring& text() const { return _text; }
	//! Return the image associated with this item
	const BookImage& image() const { return _image; }

private:
	//! The type of content in this item
	ContentType _type;
	//! The text (or caption) for this item
	ustring _text;
	//! The image for this item. Empty image if this item does not decsribe an image.
	BookImage _image;
};


/*!
 * \ingroup books_window
 * \brief A class for books
 *
 * Class Book holds the data and methods for displaying the contents of a book
 * in Eternal Lands.
 */
class Book
{
public:
	//! Enumeration for the type of background on which a book is drawn
	enum PaperType
	{
		//! Single sheet of paper
		PAPER,
		//! Bound book type
		BOOK
	};

	/*!
	 * \brief Create a new book.
	 *
	 * Create a new book with title \a title and identification number \a id,
	 * to be drawn on a background of type \a paper_type.
	 * \param paper_type The type of background on which the book is drawn
	 * \param title      The title of the book
	 * \param id         The ientification number of the book
	 */
	Book(PaperType paper_type, const std::string& title, int id):
		_items(), _paper_type(paper_type), _title(title), _id(id), _laid_out(false),
		_pages(), _nr_server_pages_total(0), _nr_server_pages_obtained(0),
		_waiting_on_server_page(false), _active_text_page(0), _active_page(0) {}

	//! Return the background paper for this book
	PaperType paper_type() const { return _paper_type; }
	//! Return the title of this book
	const std::string& title() const { return _title; }
	//! Return the identification number of this book
	int id() const { return _id; }
	//! Return whether the layout of the contents is up to date
	bool is_laid_out() const { return _laid_out; }
	//! Return the number of pages in this book
	int nr_pages() const { return _pages.size(); }
	//! Return the number of pages to skip when turning a page
	int page_delta() const { return _paper_type == BOOK ? 2 : 1; }
	/*!
	 * \brief Return the page currently being displayed
	 *
	 * \note The page number returned is the offset in the _pages array,
	 * and starts at zero. Thus, it corresponds to page \a nr + 1 on screen.
	 */
	int active_page_nr() const { return _active_page; }
	//! Return whether the last page in the book is currently visible
	bool last_page_visible() const { return _active_page + page_delta() >= nr_pages(); }
	//! Check if more server side book content should be available
	bool server_book_incomplete() const
	{
		return _nr_server_pages_total > 0 && _nr_server_pages_obtained < _nr_server_pages_total;
	}

	/*!
	 * \brief Add server-side book content
	 *
	 * Add content stored in \a len bytes of data in \a data for a book stored
	 * on the server to this book. Server-side books are sent page-by-page,
	 * although the server's view of a page does not necessarily correspond
	 * to a book as formatted on the client.
	 *
	 * \param data The content data sent byt the server
	 * \param len  The number of bytes in \a data
	 */
	void add_server_content(const unsigned char* data, size_t len);
	/*!
	 * \brief Update the number of downloaded server pages.
	 *
	 * Set the total number of server pages for this book, and increase the
	 * number of pages downloaded by one.
	 *
	 * \param total The total number of server pages available
	 */
	void add_server_page(int total)
	{
		_nr_server_pages_total = total;
		++_nr_server_pages_obtained;
		_waiting_on_server_page = false;
	}

	/*!
	 * \brief Mark this book as needing layout.
	 *
	 * Mark this book as needing a reformatting. This is done in response to
	 * font changes, or when new server data has come in.
	 */
	void renew_layout() { _laid_out = false; }
	/*!
	 * \brief Format this book
	 *
	 * Format the contents of this book onto pages of size \a page_width by
	 * \a page_height, using a scale factor \a zoom for all contents.
	 *
	 * \param page_width  The width of a page's contents
	 * \param page_height The height of a page's contents
	 * \param zoom        Scale factor for the text and images in the book
	 */
	void layout(int page_width, int page_height, float zoom);

	/*!
	 * \brief Display the current page
	 *
	 * Draw the currently visible page onto the screen.
	 *
	 * \param zoom The scale factor for drawing the book's contents
	 */
	void display(float zoom) const;

	/*!
	 * \brief Change the current page
	 *
	 * Switch to viewing page \a nr.
	 * \note The page number \a nr here is the offset in the _pages array,
	 * and starts at zero. Thus, it corresponds to page \a nr + 1 on screen.
	 *
	 * \param nr The page to turn to.
	 */
	void turn_to_page(int nr);

	/*!
	 * \brief Read a book from file.
	 *
	 * Read a book to be drawn on paper type \a paper_type from XML file
	 * \a file_name, and give it identification number \a id.
	 *
	 * \param file_name The name of the file from which to read the book
	 * \param type      The type of background for the book
	 * \param id        The identification number of the new book
	 * \return The new Book object n succes. On failure, throws an ExtendedException.
	 */
	static Book read_book(const std::string& file_name, PaperType type, int id);

private:
	//! The (unscaled) number of pixels to keep free around an image
	static const int image_margin = 10;
	//! Reduced line spacing for regular text
	static constexpr const float line_spacing = 0.9;

	//! The unformatted contents of this book
	std::vector<BookItem> _items;
	//! The background paper type for this book
	PaperType _paper_type;
	//! The title of the book
	std::string _title;
	//! The identification number
	int _id;
	//! Whether the layout of the book is up to date
	bool _laid_out;
	//! The formatted pages of the book
	std::vector<Page> _pages;
	//! For server books, the total number of server pages
	int _nr_server_pages_total;
	//! For server books, the number of server pages downloaded already
	int _nr_server_pages_obtained;
	//! Whether we are currently waiting on a response to a request for a server book
	bool _waiting_on_server_page;
	//! Page index where we will next try to store text data while formatting
	int _active_text_page;
	//! Currently visible page
	int _active_page;

	/*!
	 * \brief Return the height of a single line of text
	 *
	 * Return the height of a single line of text when drawing with the current
	 * book font settings at zoom level \a zoom.
	 *
	 * \param zoom    The scale factor for the text.
	 * \param spacing The scale factor for spacing between two lines
	 * \return The height of a line of text, in pixels.
	 */
	static int line_height(float zoom, float spacing=1.0)
	{
		return FontManager::get_instance().line_height(BOOK_FONT, zoom * spacing);
	}

	//! Add a new, empty page to the book
	Page* add_page(int page_width, int page_height, float zoom);
	//! Return a pointer to the current page for writing new text on
	Page* text_page(int page_width, int page_height, float zoom)
	{
		if (size_t(_active_text_page) >= _pages.size())
			return add_page(page_width, page_height, zoom);
		return &_pages[_active_text_page];
	}
	/*!
	 * \brief Return a new page for writing new text on
	 *
	 * Return a pointer to a new page for writing text on. This can be a page
	 * that already contains contents (such as an image that was floated forwards),
	 * or a new blank page.
	 */
	Page* next_text_page(int page_width, int page_height, float zoom)
	{
		if (size_t(++_active_text_page) >= _pages.size())
			return add_page(page_width, page_height, zoom);
		return &_pages[_active_text_page];
	}
	/*!
	 * \brief Return the last page in the book
	 *
	 * Return a pointer to the last page in the book. If there is none, add a
	 * new page and return that.
	 */
	Page* last_page(int page_width, int page_height, float zoom)
	{
		if (_pages.empty())
			return add_page(page_width, page_height, zoom);
		return &_pages.back();
	}

	/*!
	 * \brief Lay out a text item.
	 *
	 * Format the text in \a text of content type \a type for drawing with scale
	 * factor \a zoom on pages of size \a page_width by \a page_height. Titles
	 * author names, and page numbers are centered, normal text is left aligned.
	 */
	void layout_text(ContentType content_type, const ustring& text,
		int page_width, int page_height, float zoom);
	/*!
	 * \brief Format a caption text
	 *
	 * Break up the caption text \a text associated with image \a image into
	 * separate blocks of text that can be placed on page of size \a page_width
	 * by \a page_height when drawn with scale factor \a zoom. The blocks may
	 * be placed to the left and/or right of the image, underneath it, and if
	 * the caption is long enough, on the next page.
	 *
	 * \param image       The image with which the caption is associated
	 * \param text        The text of the caption
	 * \param page_width  The width of a page in pixels
	 * \param page_height The height of a page in pixels
	 * \param zoom        The scale factor for the image and text
	 * \return array of tuples of text blocks and booleans, where the second
	 * 	member of the tuple indicates whether the block is to be placed on a new page.
	 */
	std::vector<std::pair<TextBlock, bool>> caption_text(const BookImage& image,
		const ustring& text, int page_width,  int page_height, float zoom) const;
	/*!
	 * \brief Layout an image
	 *
	 * Place image \a image onto a page in this book, and format the caption
	 * text in \a caption around it. The caption is drawn in a smaller size
	 * than the regular text. If not enough space is available next to the image,
	 * the caption is placed underneath it.
	 */
	void layout_image(const BookImage &image, const ustring &caption,
		int page_width, int page_height, float zoom);

	/*!
	 * \brief Add a text item to the book
	 *
	 * Add a text item \a text of type a type to the unformatted contents of
	 * this book.
	 *
	 * \param type The kind of text item
	 * \param text The text to add
	 */
	void add_item(ContentType type, const ustring& text)
	{
		_items.emplace_back(type, text);
	}
	/*!
	 * \brief Add an image item to the book
	 *
	 * Add an image \a image with (possibly empty) caption \a caption to the
	 * unformatted contents of this book.
	 *
	 * \param image   The image to add
	 * \param caption The caption associated with the image
	 */
	void add_item(BookImage&& image, const ustring& caption)
	{
		_items.emplace_back(std::move(image), caption);
	}

	//! Parse an image item from its XML description and add it to this book
	void add_xml_image(const xmlNode *node);
	//! Parse a text item from its XML description and add it to this book
	void add_xml_text(ContentType type, const xmlNode *node);
	//! Parse a page in an XML book file, and add the contents to this book
	void add_xml_page(const xmlNode *node);
	//! Parse book in XML format, and add the contents to this book
	void add_xml_content(const xmlNode *node);

	/*!
	 * \brief Parse an image from the server
	 *
	 * Parse the description of an image from \a len bytes of server data in
	 * \a data, and add it to this books contents.
	 *
	 * \param data The image descrition sent by the server
	 * \param len  The number of bytes in \a data
	 */
	void add_server_image(const unsigned char* data, size_t len);
};


/*!
 * \ingroup books_window
 * \brief Class for navigation buttons
 *
 * Class TextLink is used for the navigation buttons that are used to page
 * through the books. They consist of text labels, that when clicked, turn
 * to book to a target page.
 */
class TextLink
{
public:
	/*!
	 * \brief Create a new TextLink
	 *
	 * Create a new TextLink with label \a text, that turns the book to page
	 * \a target when clicked on, and position it a coordinates \a x, \a y
	 * in the book window. The label is drawn with scale factor \a zoom,
	 * and aligned to the specified coordinates using the method specified
	 * by \a alignment.
	 *
	 * \param target    The target page for this button
	 * \param text      The text of this button
	 * \param x         The x coordinate of the button's position
	 * \param y         The y coordinate of the button's position
	 * \param zoom      The scale factor with which the button is drawn
	 * \param alignment The alignment of the button text with respect to the position
	 */
	TextLink(int target, const char* text, int x, int y, float zoom,
		TextDrawOptions::Alignment alignment = TextDrawOptions::Alignment::LEFT);

	//! Return the x coordinate of the left side of the button
	int x_begin() const { return _x_begin; }
	//! Return the x coordinate of the right side of the button
	int x_end() const { return _x_end; }
	//! Return the target page number of the button
	int target() const { return _target; }
	//! Return whether position \a mx, \a my is inside the button
	bool is_under(int mx, int my) const
	{
		return my >= _y_begin && my < _y_end && mx >= _x_begin && mx < _x_end;
	}

	//! Draw the button on the screen
	void display() const;
	//! Handle a mouseover event for this button (this changes its color)
	void mouseover(int mx, int my)
	{
		_under_mouse = is_under(mx, my);
	}

private:
	//! The target page to turn to when clicked
	int _target;
	//! The label shown to the user
	ustring _text;
	//! Left side of the button's bounding box
	int _x_begin;
	//! Right side of the button's bounding box
	int _x_end;
	//! Top of the button's bounding box
	int _y_begin;
	//! Bottom of the button's bounding box
	int _y_end;
	//! Zoom level for the button
	float _zoom;
	//! Whether the mouse is currently over the button
	bool _under_mouse;
};


/*!
 * \ingroup books_window
 * \brief Class for the book window
 *
 * Class BookWindow handles the book window in Eternal Lands. It actually
 * contains two separate windows, for the two different kinds of backgrounds
 * that a book can have, only one of which is shown at a time.
 */
class BookWindow
{
public:
	//! Unscaled width of the book background
	static const int book_width = 530;
	//! Unscaled height of the book background
	static const int book_height = 300;
	//! Unscaled width of the paper background
	static const int paper_width = 330;
	//! Unscaled height of the paper background
	static const int paper_height = 400;

	//! Unscaled width of a single page of contents in a book
	static const int page_width_book;
	//! Unscaled height of a single page of contents in a book
	static const int page_height_book;
	//! Unscaled width of a single page of contents on paper
	static const int page_width_paper;
	//! Unscaled height of a single page of contents on paper
	static const int page_height_paper;

	//! Unscaled horizontal offset at which to start drawing on a book
	static const int x_offset_book;
	//! Unscaled vertical offset at which to start drawing on a book
	static const int y_offset_book;
	//! Unscaled distance between left an right pages in a book
	static const int x_half_book;
	//! Unscaled horizontal offset at which to start drawing on paper
	static const int x_offset_paper;
	//! Unscaled vertical offset at which to start drawing on paper
	static const int y_offset_paper;

	//! Create a new hidden book window without contents
	BookWindow(): _book_win(-1), _paper_win(-1),
		_book_texture(Uint32(-1)), _paper_texture(Uint32(-1)),
		_book_id(-1), _ui_margin(0), _ui_font_height(0) {}

	//! Return whether the book window is currently being displayed
	bool is_open() const { return get_show_window(_book_win) || get_show_window(_paper_win); }
	/*!
	 * \brief Check if the book window is showing a book
	 *
	 * Check whether the book window is currently open, and displaying the book
	 * with identifier \a id.
	 *
	 * \param id The identification number of the book to check for.
	 * \return \c true if the window is currently showing the book, \c false otherwise.
	 */
	bool book_is_open(int id) const { return _book_id == id && is_open(); }

	/*!
	 * \brief Start displaying a book
	 *
	 * Open the appropriate book window, and display the contents of book
	 * \a book in it.
	 *
	 * \param book The book to display
	 */
	void display(Book& book);
	//! Bring the book window to the front
	void select() const
	{
		if (get_show_window(_book_win))
			select_window(_book_win);
		else if (get_show_window(_paper_win))
			select_window(_paper_win);
	}
	//! Close the book window
	void close() const
	{
		hide_window(_book_win);
		hide_window(_paper_win);
	}
	//! Close the book window if it currently displays the book identified by \a id
	void close_book(int id) const
	{
		if (_book_id == id)
			close();
	}

private:
	//! The initial x coordinate of the position of the book window
	static const int window_x = 100;
	//! The initial y coordinate of the position of the book window
	static const int window_y = 100;

	//! Unscaled vertical margin for the navigation buttons
	static const int ui_margin = 2;
	//! Unscaled horizontal margin for the navigation buttons
	static const int x_margin_button = 10;

	//! ID of the book window
	int _book_win;
	//! ID of the paper window
	int _paper_win;
	//! Texture cache ID for the book background
	Uint32 _book_texture;
	//! Texture cache ID for the paper background
	Uint32 _paper_texture;
	//! ID of the book currently on display
	int _book_id;
	//! Scaled vertical margin for the navigation buttons
	int _ui_margin;
	//! Scaled font height for the navigation buttons
	int _ui_font_height;
	//! List of navigation buttons
	std::vector<TextLink> _links;

	//! Return a pointer to the book currently being displayed by the book window
	Book* get_book();

	//! Add the navigation buttons for paging through the books
	void add_links(window_info *win);

	//! Handler for mouseover events
	int mouseover_handler(int mx, int my);
	//! Static handler for mouseover events, calls mouseover_handler()
	static int static_mouseover_handler(window_info *win, int mx, int my);
	//! Handler for mouse click events
	int click_handler(window_info *win, int mx, int my, Uint32 flags);
	//! Static handler for mouse click events, calls click_handler()
	static int static_click_handler(window_info *win, int mx, int my, Uint32 flags);
	//! Handler for displaying the window
	int display_handler(window_info *win);
	//! Static handler for displaying the window, calls display_handler()
	static int static_display_handler(window_info *win);
	//! Handler for user interface scaling events
	int ui_scale_handler(window_info *win);
	//! Static handler for user interface scaling events, calls ui_scale_handler()
	static int static_ui_scale_handler(window_info *win);
	//! Handler for font change events
	int font_change_handler(window_info *win, FontManager::Category cat);
	//! Static handler for font change events, calls font_change_handler()
	static int static_font_change_handler(window_info *win, FontManager::Category cat);
};


/*!
 * \ingroup books_window
 * \brief Class for the collection of books
 *
 * Class BookCollection holds the collection of all books that have been opened
 * so far this session. It is a singleton class, all access to the object of this
 * class should be done through the get_instance() method.
 */
class BookCollection
{
public:
	//! Enumeration for the source of a book
	enum BookSource
	{
		//! Book is stored locally, in client game data
		LOCAL,
		//! Book is stored on the server
		SERVER
	};

	//! Return the singleton instance of this BookCollection
	static BookCollection& get_instance()
	{
		static BookCollection collection;
		return collection;
	}

	/*!
	 * \brief Retrieve a book
	 *
	 * Return the book with identification number \a id.
	 *
	 * \param id The identification number of the book
	 * \return A reference to the book. If the book is not found, an
	 * 	ExtendedException is thrown.
	 */
	Book& get_book(int id);

	//! Initialize the book collection, reading the default (race and knowledge) books
	void initialize();
	/*!
	 * \brief Open a book
	 *
	 * Open the book with identification number \a id in the book window, If
	 * the book is currently not present in the collection, it is assumed to
	 * be a server book, and a request for the book is sent to the server
	 * instead.
	 *
	 * \param id The identification number of the book to open.
	 */
	void open_book(int id);
	/*!
	 * \brief Close a book
	 *
	 * Close the book window if it is currently showing the book with
	 * identification number \a id.
	 *
	 * \param id The identification number of the book to check for.
	 */
	void close_book(int id) const { _window.close_book(id); }
	//! Return whether the book window is open and showing the book with ID \a id
	bool book_is_open(int id) const { return _window.book_is_open(id); }
	//! Return whether the book window is currently open
	bool window_is_open() const { return _window.is_open(); }
	//! Bring the book window to the foreground
	void select_window() const { _window.select(); }
	//! Close the book window uncondtionally
	void close_window() const { _window.close(); }

	/*!
	 * \brief Parse book data from the server
	 *
	 * Parse the book specification in \a len bytes of server data in \a data, and
	 * add the contents to the appropriate book.
	 *
	 * \param data The server-side book contents, or local book specification
	 * \param len  The number of bytes in \a data
	 * \sa Book::add_server_content(), read_local_book(), read_server_book()
	 */
	void read_network_book(const unsigned char* data, size_t len);

	/*!
	 * \brief Request book data from the server
	 *
	 * Send a request for server-side page \a page in book \a id to the game
	 * server.
	 *
	 * \param id   The identfier for the book
	 * \param page The number of the page to retrieve
	 */
	static void request_server_page(int id, int page);

private:
	//! Offset to add to IDs of knowledge books
	static const int knowledge_book_offset = 10000;

	//! The collection of books itself
	std::unordered_map<int, Book> _books;
	//! The book window for displaying books
	BookWindow _window;

	//! Create a new and empty book collection
	BookCollection(): _books(), _window() {}
	//! Prevent a BookCollection from being copied
	BookCollection(const BookCollection&) = delete;
	//! Prevent a BookCollection from being copied
	BookCollection& operator=(const BookCollection&) = delete;

	//! Add book \a book to this collection
	void add_book(Book&& book);

	/*!
	 * \brief Parse a knowledge item
	 *
	 * Parse a single knowledge item from the knowledge XML file. On success,
	 * read the book it references, and add it to this collection.
	 */
	void parse_knowledge_item(const xmlNode *node);
	//! Parse the knowledge XML file, and add all books referenced therein to this collection.
	void read_knowledge_book_index();

	/*!
	 * \brief Read a local book
	 *
	 * Read a book stored on the client's computer, that is specified by the
	 * server data \a data of \a len bytes, and add it to this collection.
	 * The data should contain the identifier and file name of the book to read.
	 *
	 * \param data The book identification as sent by the server.
	 * \param len  The number of bytes in \a data
	 */
	void read_local_book(const unsigned char* data, size_t len);
	/*!
	 * \brief Read a local book
	 *
	 * Read a page of book data sent by the server, and store it in the
	 * appropriate book.
	 *
	 * \param data The book contents as sent by the server.
	 * \param len  The number of bytes in \a data
	 */
	void read_server_book(const unsigned char* data, size_t len);
};

} // namespace eternal_lands

#endif // __cplusplus

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/*!
 * \ingroup books_window
 * \brief Reads client-side books.
 *
 * Reads the client-side books that will not be downloaded from the server.
 */
void init_books(void);

/*!
 * \ingroup	books
 * \brief Opens the book with the given ID
 *
 * Opens the book with the given ID; if the book is not found it will be
 * requested from the server.
 *
 * \param id The identifier of the book to open.
 */
void open_book(int id);
/*!
 * \ingroup books_window
 * \brief Closes the book with the given id
 *
 * Close the book window if it currently contains the book with identifier \a id.
 *
 * \param id The identifier for the book to close.
 */
void close_book(int id);
/*!
 * \ingroup books_window
 * \brief Check if a specific book is currently open
 *
 * Check is the book window is currently open and showing the book with identfier \a id.
 *
 * \param id The identifier of the book to check for.
 * \return 1 if the book window is currently showing the book, 0 otherwise.
 */
int book_is_open(int id);
/*!
 * \ingroup books_window
 * \brief Check if the book window is open
 * \return 1 if the book window is currently open, 0 otherwise
 */
int book_window_is_open();
/*!
 * \ingroup books_window
 * \brief Bring the book window to the front.
 */
void select_book_window();
/*!
 * \ingroup books_window
 * \brief Close the book window.
 */
void close_book_window();

/*!
 * \ingroup books_window
 * \brief Read a book from server data
 *
 * Read a book specified by the \a len bytes of data in \a data sent by the
 * server. This can be either the file name of a client side book, or a
 * book stored on the server transferred over the network.
 *
 * \param	data The network data
 * \param	len  The number of bytes in \a data
 */
void read_network_book(const unsigned char* data, size_t len);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // BOOKS_H

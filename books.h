#ifndef BOOKS_H
#define BOOKS_H

#include "old_books.h"
#include "elwindows.h"

#ifdef __cplusplus

namespace eternal_lands
{

enum ContentType
{
	TITLE,
	AUTHOR,
	TEXT,
	IMAGE,
	CAPTION,
	PAGE_NR
};


class Image
{
public:
	Image(): _file_name(), _x(0), _y(0), _width(0), _height(0), _texture(Uint32(-1)),
		_u_start(0.0), _v_start(0.), _u_end(0.0), _v_end(0.0) {}
	Image(const std::string& file_name, int x, int y, int width, int height,
		float u_start, float v_start, float u_end, float v_end);

	int x() const { return _x; }
	int y() const { return _y; }
	int width() const { return _width; }
	int height() const { return _height; }

	void display() const;

	Image scaled(float zoom) const
	{
		return Image(_file_name, _x * zoom, _y * zoom, _width * zoom, _height * zoom,
			_u_start, _v_start, _u_end, _v_end);
	}

private:
	std::string _file_name;
	int _x;
	int _y;
	int _width;
	int _height;
	Uint32 _texture;
	float _u_start;
	float _v_start;
	float _u_end;
	float _v_end;
};


class TextBlock
{
public:
	TextBlock(const ustring& text, ContentType type, int x, int y,
		int width, int height, float zoom):
		_text(text), _type(type), _x(x), _y(y), _width(width), _height(height),
		_zoom(zoom) {}

	int x() const { return _x; }
	int y() const { return _y; }
	int width() const { return _width; }
	int height() const { return _height; }

	void display() const;

private:
	ustring _text;
	ContentType _type;
	int _x;
	int _y;
	int _width;
	int _height;
	float _zoom;
};


class Page
{
public:
	Page(int page_height): _texts(), _images(),
		_free_ranges(1, std::make_pair(0, page_height)) {}

	bool has_image() const { return !_images.empty(); }
	bool has_free_range(int y_begin, int y_end) const
	{
		for (const auto& range: _free_ranges)
		{
			if (range.first <= y_begin && range.second >= y_end)
				return true;
		}
		return false;
	}
	std::pair<int, int> find_free_range(int min_height) const
	{
		for (const auto& range: _free_ranges)
		{
			if (range.second - range.first >= min_height)
				return range;
		}
		return std::make_pair(-1, -1);
	}

	void add_text_block(TextBlock &&block, int y_begin, int y_end)
	{
		_texts.push_back(block);
		occupy_range(y_begin, y_end);
	}
	void add_image(Image &&image, int y_begin, int y_end)
	{
		_images.push_back(image);
		occupy_range(y_begin, y_end);
	}

	void display() const
	{
		for (const auto& image: _images)
			image.display();
		for (const auto& text: _texts)
			text.display();
	}

private:
	std::vector<TextBlock> _texts;
	std::vector<Image> _images;
	std::vector<std::pair<int, int>> _free_ranges;

	void occupy_range(int y_begin, int y_end);
};


class BookItem
{
public:
	BookItem(ContentType type, const ustring& text):
		_type(type), _text(text), _image() {}
	BookItem(Image&& img, const ustring& caption):
		_type(IMAGE), _text(caption), _image(img) {}

	ContentType type() const { return _type; }
	const ustring& text() const { return _text; }
	const Image& image() const { return _image; }

private:
	ContentType _type;
	ustring _text;
	Image _image;
};


class Book
{
public:
	enum PaperType
	{
		PAPER,
		BOOK
	};

	Book(PaperType paper_type, const std::string& title, int id):
		_items(), _paper_type(paper_type), _title(title), _id(id), _laid_out(false),
		_pages(), _nr_server_pages_total(0), _nr_server_pages_obtained(0),
		_waiting_on_server_page(false), _active_text_page(0), _active_page(0) {}

	PaperType paper_type() const { return _paper_type; }
	const std::string& title() const { return _title; }
	int id() const { return _id; }
	bool is_laid_out() const { return _laid_out; }
	int nr_pages() const { return _pages.size(); }
	int page_delta() const { return _paper_type == BOOK ? 2 : 1; }
	int active_page_nr() const { return _active_page; }
	bool last_page_visible() const { return _active_page + page_delta() >= nr_pages(); }
	bool server_book_incomplete() const
	{
		return _nr_server_pages_total > 0 && _nr_server_pages_obtained < _nr_server_pages_total;
	}

	void add_server_content(const unsigned char* data, size_t len);
	void add_server_page(int total)
	{
		_nr_server_pages_total = total;
		++_nr_server_pages_obtained;
		_waiting_on_server_page = false;
	}

	void layout(int page_width, int page_height, float zoom);

	void display(float zoom) const;

	void turn_to_page(int nr);

	static Book read_book(const std::string& file_name, PaperType type, int id);

private:
	static const int image_margin = 10;

	std::vector<BookItem> _items;
	PaperType _paper_type;
	std::string _title;
	int _id;
	bool _laid_out;
	std::vector<Page> _pages;
	int _nr_server_pages_total;
	int _nr_server_pages_obtained;
	bool _waiting_on_server_page;
	int _active_text_page;
	int _active_page;

	int line_height(float zoom) const
	{
		return FontManager::get_instance().line_height(BOOK_FONT, zoom);
	}

	Page* add_page(int page_width, int page_height, float zoom);
	Page* text_page(int page_width, int page_height, float zoom)
	{
		if (size_t(_active_text_page) >= _pages.size())
			return add_page(page_width, page_height, zoom);
		return &_pages[_active_text_page];
	}
	Page* next_text_page(int page_width, int page_height, float zoom)
	{
		if (size_t(++_active_text_page) >= _pages.size())
			return add_page(page_width, page_height, zoom);
		return &_pages[_active_text_page];
	}
	Page* last_page(int page_width, int page_height, float zoom)
	{
		if (_pages.empty())
			return add_page(page_width, page_height, zoom);
		return &_pages.back();
	}

	void layout_text(ContentType content_type, const ustring& text,
		int page_width, int page_height, float zoom);
	void layout_image(const Image &img, const ustring &text,
		int page_width, int page_height, float zoom);

	void add_item(ContentType type, const ustring& text)
	{
		_items.emplace_back(type, text);
	}
	void add_item(Image&& image, const ustring& caption)
	{
		_items.emplace_back(std::move(image), caption);
	}

	std::vector<std::pair<TextBlock, bool>> caption_text(const Image& image,
		const ustring& text, int page_width,  int page_height, float zoom) const;
	void add_xml_image(const xmlNode *node);
	void add_xml_text(ContentType type, const xmlNode *node);
	void add_xml_page(const xmlNode *node);
	void add_xml_content(const xmlNode *node);
	void add_server_image(const unsigned char* data, size_t len);
};


class TextLink
{
public:
	TextLink(int target, const char* text, int x, int y, float zoom,
		TextDrawOptions::Alignment alignment = TextDrawOptions::LEFT);

	int x_begin() const { return _x_begin; }
	int x_end() const { return _x_end; }
	int target() const { return _target; }
	bool is_under(int mx, int my) const
	{
		return my >= _y_begin && my < _y_end && mx >= _x_begin && mx < _x_end;
	}

	void display() const;
	void mouseover(int mx, int my)
	{
		_under_mouse = is_under(mx, my);
	}

private:
	int _target;
	ustring _text;
	int _x_begin;
	int _x_end;
	int _y_begin;
	int _y_end;
	float _zoom;
	bool _under_mouse;
};


class BookWindow
{
public:
	static const int book_width = 530;
	static const int book_height = 320;
	static const int paper_width = 330;
	static const int paper_height = 400;

	static const int page_width_book = std::round(0.38 * book_width);
	static const int page_height_book = std::round(0.77 * book_height);
	static const int page_width_paper = std::round(0.80 * paper_width);
	static const int page_height_paper = std::round(0.80 * book_width);

	static const int x_offset_book = std::round(0.078 * book_width);
	static const int y_offset_book = std::round(0.098 * book_height);
	static const int x_half_book = std::round(0.459 * book_width);
	static const int x_offset_paper = std::round(0.098 * paper_width);
	static const int y_offset_paper = std::round(0.098 * paper_height);

	BookWindow(): _book_win(-1), _paper_win(-1),
		_book_texture(Uint32(-1)), _paper_texture(Uint32(-1)),
		_book_id(-1), _ui_margin(0), _ui_font_height(0) {}

	void display(Book& book);
	Book* get_book();

private:
	static const int window_x = 100;
	static const int window_y = 100;

	static const int ui_margin = 2;
	static const int x_margin_button = 10;

	int _book_win;
	int _paper_win;
	Uint32 _book_texture;
	Uint32 _paper_texture;
	int _book_id;
	int _ui_margin;
	int _ui_font_height;
	std::vector<TextLink> _links;

	void add_links(window_info *win);

	int mouseover_handler(int mx, int my);
	static int static_mouseover_handler(window_info *win, int mx, int my);
	int click_handler(window_info *win, int mx, int my, Uint32 flags);
	static int static_click_handler(window_info *win, int mx, int my, Uint32 flags);
	int display_handler(window_info *win);
	static int static_display_handler(window_info *win);
	void ui_scale_handler(window_info *win);
	static int static_ui_scale_handler(window_info *win);
};


class BookCollection
{
public:
	enum BookSource
	{
		LOCAL,
		SERVER
	};

	static BookCollection& get_instance()
	{
		static BookCollection collection;
		return collection;
	}

	Book& get_book(int id);

	void initialize();
	void open_book(int id);
	void read_network_book(const unsigned char* data, size_t len);

	static void request_server_page(int id, int page);

private:
	static const int knowledge_book_offset = 10000;

	std::unordered_map<int, Book> _books;
	BookWindow _window;

	BookCollection(): _books(), _window() {}
	BookCollection(const BookCollection&) = delete;
	BookCollection& operator=(const BookCollection&) = delete;

	void add_book(Book&& book);

	void parse_knowledge_item(const xmlNode *node);
	void read_knowledge_book_index();

	void read_local_book(const unsigned char* data, size_t len);
	void read_server_book(const unsigned char* data, size_t len);
};

} // namespace eternal_lands

#endif // __cplusplus

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/*!
 * \ingroup books
 * \brief Reads client-side books.
 *
 * Reads the client-side books that will not be downloaded from the server.
 */
void init_books(void);
/*!
 * \ingroup	books
 * \brief Opens the book with the given ID
 *
 * Opens the book with the given ID - if the book is not found it will be
 * requested from the server.
 *
 * \param id The IF of the book to open.
 *
 * \callgraph
 */
void open_book(int id);
/*!
 * \ingroup network_books
 * \brief Read a book from server data
 *
 * Read a book specified by the \a len bytes of data in \a data sent by the
 * server. This can be either the file name of a client side book, or a
 * book stored on the server transferred over the network.
 *
 * \param	data The network data
 * \param	len  The number of bytes in \a data
 *
 * \callgraph
 */
void read_network_book(const unsigned char* data, size_t len);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // BOOKS_H

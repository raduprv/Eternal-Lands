//FIXME: TODO: server books

#include <string>
#include <unordered_map>
#include <vector>
#include "asc.h"
#include "books.h"
#include "elconfig.h"
#include "elloggingwrapper.h"
#include "exceptions/extendedexception.hpp"
#include "font.h"
#include "knowledge.h"
#include "multiplayer.h"
#include "new_character.h"
#include "textures.h"

// FIXME: book_opened

namespace eternal_lands
{

size_t find_line(const ustring& text, size_t n, size_t pos=0)
{
	static const ustring nl(reinterpret_cast<const unsigned char*>("\r\n"));

	for ( ; n > 0; --n)
	{
		pos = text.find_first_of(nl, pos);
		if (pos == ustring::npos)
			break;
		++pos;
	}

	return pos;
}

Image::Image(const std::string& file_name, int x, int y, int width, int height,
	float u_start, float v_start, float u_end, float v_end):
	_file_name(file_name), _x(x), _y(y), _width(width), _height(height),
	_texture(load_texture_cached(_file_name.c_str(), tt_image)),
	_u_start(u_start), _v_start(v_start), _u_end(u_end), _v_end(v_end) {}

void Image::display() const
{
	glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
	bind_texture(_texture);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05f);
	glBegin(GL_QUADS);
	glTexCoord2f(_u_start, _v_end);   glVertex2i(_x,          _y+_height);
	glTexCoord2f(_u_end,   _v_end);   glVertex2i(_x + _width, _y + _height);
	glTexCoord2f(_u_end,   _v_start); glVertex2i(_x + _width, _y);
	glTexCoord2f(_u_start, _v_start); glVertex2i(_x,          _y);
	glEnd();
	glDisable(GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


void TextBlock::display() const
{
	// XXX FIXME: use fontmanager/textoptions directly
	TextDrawOptions options = TextDrawOptions().set_zoom(_zoom);
	switch (_type)
	{
		case TITLE:
		case AUTHOR:
			options.set_alignment(TextDrawOptions::CENTER);
			break;
		case PAGE_NR:
			options.set_foreground(0.385f, 0.285f, 0.19f);
			break;
		case CAPTION:
			options.set_zoom(_zoom * DEFAULT_SMALL_RATIO);
			// fallthrough
		default:
			options.set_foreground(0.34f, 0.25f, 0.16f);
			break;
	}
	FontManager::get_instance().draw(BOOK_FONT, _text.c_str(), _text.length(),
		_x, _y, options);
}


void Page::occupy_range(int y_begin, int y_end)
{
	if (y_begin >= y_end)
		return;

	auto it = std::upper_bound(_free_ranges.begin(), _free_ranges.end(),
		y_begin, [](int y, auto range) { return y < range.second; });
	while (it != _free_ranges.end())
	{
		if (it->first >= y_end)
			break;
		if (it->first < y_begin)
		{
			if (it->second <= y_end)
			{
				it->second = y_begin;
				++it;
			}
			else
			{
				it = _free_ranges.insert(it, std::make_pair(it->first, y_begin));
				++it;
				it->first = y_end;
				++it;
			}
		}
		else
		{
			if (it->second <= y_end)
			{
				it = _free_ranges.erase(it);
			}
			else
			{
				it->first = y_end;
				++it;
			}
		}
	}
}


Page* Book::add_page(int page_width, int page_height, float zoom)
{
	_pages.push_back(Page(page_height));
	Page *page = &_pages.back();

	std::ostringstream os;
	os << _pages.size();
	std::string tmp = os.str();
	ustring page_nr(tmp.begin(), tmp.end());
	page->add_text_block(TextBlock(page_nr, PAGE_NR, page_width / 2, page_height + 2,
		page_width, line_height(zoom), zoom), 0, 0);

	return page;
}

void Book::layout_text(ContentType content_type, const ustring& text,
	int page_width, int page_height, float zoom)
{
	if (text.empty())
		return;

	int line_h = line_height(zoom);
	if (line_h > page_height)
		// Line is higher than a full page. Give up.
		return;

	ustring lines;
	int nr_lines;
	int x = 0;
	std::tie(lines, nr_lines) = FontManager::get_instance().reset_soft_breaks(
		BOOK_FONT, text.data(), text.capacity(), text.length(), zoom,
		page_width, 0, 0);
	if (content_type == AUTHOR)
	{
		lines = to_color_char(c_orange3) + lines + static_cast<unsigned char>('\n');
		++nr_lines;
		x = page_width / 2;
	}
	else if (content_type == TITLE)
	{
		lines = to_color_char(c_orange4) + lines + static_cast<unsigned char>('\n');
		++nr_lines;
		x = page_width / 2;
	}

	Page* page = _pages.empty() ? add_page(page_width, page_height, zoom) : &_pages.back();
	size_t offset = 0;
	while (offset < lines.length())
	{
		int y_begin, y_end;
		std::tie(y_begin, y_end) = page->find_free_range(line_h);
		if (y_begin < 0)
		{
			page = add_page(page_width, page_height, zoom);
			std::tie(y_begin, y_end) = page->find_free_range(line_h);
		}

		int nr_lines_this_page = (y_end - y_begin) / line_h;
		size_t end;
		if (nr_lines <= nr_lines_this_page)
		{
			nr_lines_this_page = nr_lines;
			end = lines.length() + 1;
		}
		else
		{
			end = find_line(lines, nr_lines_this_page, offset);
			if (end == ustring::npos)
				end = lines.length() + 1;
		}

		int height = nr_lines_this_page * line_h;
		TextBlock block(lines.substr(offset, end - 1 - offset), content_type,
			x, y_begin, page_width, height, zoom);
		page->add_text_block(std::move(block), y_begin, y_begin + height);

		offset = end;
		nr_lines -= nr_lines_this_page;
	}
}

std::vector<std::pair<TextBlock, bool>> Book::caption_text(const Image& image,
	const ustring& text, int page_width, int page_height, float zoom) const
{
	std::vector<std::pair<TextBlock, bool>> res;
	if (text.empty())
		return res;

	bool do_left, do_right;
	if (image.y() + image.height() > page_height
		|| image.x() + image.width() > page_width)
	{
		// Image is too big for the page. Don't show it, but do show the caption.
		// There is need to flow around the image though, so:
		do_left = do_right = false;
	}
	else
	{
		// Only show text beside the image if there is at least one third of
		// the page width free.
		do_left = image.x() > page_width / 3;
		do_right = image.x() + image.width() < 2 * page_width / 3;
	}

	float caption_zoom = zoom * DEFAULT_SMALL_RATIO;
	int caption_line_height = FontManager::get_instance().line_height(BOOK_FONT, caption_zoom);
	ustring lines;
	int nr_lines;
	int nr_lines_side = (image.height() + caption_line_height - 1) / caption_line_height;
	if (image.y() + nr_lines_side * caption_line_height > page_height)
		--nr_lines_side;
	if (do_left && do_right)
	{
		ustring left_str, right_str;
		int x_left = 0;
		int x_right = image.x() + image.width();
		int left_width = image.x() - image_margin;
		int right_width = page_width - x_right;

		lines = text;
		for (int i = 0; i < nr_lines_side; ++i)
		{
			std::tie(lines, nr_lines) = FontManager::get_instance().reset_soft_breaks(
				BOOK_FONT, lines.data(), lines.capacity(), lines.length(), caption_zoom,
				left_width, 0, 0);
			size_t off = find_line(lines, 1);
			left_str.append(lines.substr(0, off));
			lines = lines.substr(off);
			if (lines.empty())
				break;

			std::tie(lines, nr_lines) = FontManager::get_instance().reset_soft_breaks(
				BOOK_FONT, lines.data(), lines.capacity(), lines.length(), caption_zoom,
				right_width, 0, 0);
			off = find_line(lines, 1);
			right_str.append(lines.substr(0, off));
			lines = lines.substr(off);
			if (lines.empty())
				break;
		}

		TextBlock left_block(left_str, CAPTION, x_left, image.y(), left_width,
			nr_lines_side * caption_line_height, zoom);
		res.push_back(std::make_pair(left_block, false));
		TextBlock right_block(right_str, CAPTION, x_right, image.y(), right_width,
				nr_lines_side * caption_line_height, zoom);
		res.push_back(std::make_pair(right_block, false));
	}
	else if (do_left)
	{
		int x = 0;
		int side_width = image.x() - image_margin;
		std::tie(lines, nr_lines) = FontManager::get_instance().reset_soft_breaks(
			BOOK_FONT, text.data(), text.capacity(), text.length(), caption_zoom,
			side_width, 0, 0);
		if (nr_lines <= nr_lines_side)
		{
			TextBlock block(lines, CAPTION, x, image.y(), side_width,
				nr_lines_side * caption_line_height, zoom);
			res.push_back(std::make_pair(block, false));
			lines.clear();
		}
		else
		{
			size_t off = find_line(lines, nr_lines_side);
			TextBlock block(lines.substr(0, off-1), CAPTION, x, image.y(), side_width,
				nr_lines * caption_line_height, zoom);
			res.push_back(std::make_pair(block, false));
			lines = lines.substr(off);
		}
	}
	else if (do_right)
	{
		int x = image.x() + image.width() + image_margin;
		int side_width = page_width - x;
		std::tie(lines, nr_lines) = FontManager::get_instance().reset_soft_breaks(
			BOOK_FONT, text.data(), text.capacity(), text.length(), caption_zoom,
			side_width, 0, 0);
		if (nr_lines <= nr_lines_side)
		{
			TextBlock block(lines, CAPTION, x, image.y(), side_width,
				nr_lines * caption_line_height, zoom);
			res.push_back(std::make_pair(block, false));
			lines.clear();
		}
		else
		{
			size_t off = find_line(lines, nr_lines_side);
			TextBlock block(lines.substr(0, off-1), CAPTION, x, image.y(), side_width,
				nr_lines_side * caption_line_height, zoom);
			res.push_back(std::make_pair(block, false));
			lines = lines.substr(off);
		}
	}
	else
	{
		lines = text;
	}

	int y = image.y() + nr_lines_side * caption_line_height;
	bool new_page = false;
	while (!lines.empty())
	{
		int max_nr_lines = (page_height - y) / caption_line_height;
		std::tie(lines, nr_lines) = FontManager::get_instance().reset_soft_breaks(
			BOOK_FONT, lines.data(), lines.capacity(), lines.length(), caption_zoom,
			page_width, 0, 0);
		if (nr_lines <= max_nr_lines)
		{
			TextBlock block(lines, CAPTION, 0, y, page_width,
				nr_lines * caption_line_height, zoom);
			res.push_back(std::make_pair(block, new_page));
			break;
		}
		else
		{
			size_t off = find_line(lines, nr_lines_side);
			TextBlock block(lines.substr(0, off-1), CAPTION, 0, y, page_width,
				nr_lines_side * caption_line_height, zoom);
			res.push_back(std::make_pair(block, new_page));

			lines = lines.substr(off);
			y = 0;
			new_page = true;
		}
	}

	return res;
}

void Book::layout_image(const Image &image, const ustring& caption,
	int page_width, int page_height, float zoom)
{
	Image scaled_img = image.scaled(zoom);
	bool image_fits = (scaled_img.y() + scaled_img.height() <= page_height
		&& scaled_img.x() + scaled_img.width() <= page_width);
	if (!image_fits && caption.empty())
		return;

	int margin = std::round(image_margin * zoom);
	int y_begin = std::max(scaled_img.y() - margin, 0);
	int y_end = image_fits ? scaled_img.y() + scaled_img.height() : scaled_img.y();
	std::vector<std::pair<TextBlock, bool>> blocks
		= caption_text(scaled_img, caption, page_width, page_height, zoom);
	for (const auto& tup: blocks)
	{
		if (!tup.second)
			// Text block goes on same page as image
			y_end = std::max(y_end, tup.first.y() + tup.first.height());
	}
	y_end += margin;

	Page *page = _pages.empty() ? add_page(page_width, page_height, zoom) : &_pages.back();
	if (!page->has_free_range(y_begin, y_end))
		page = add_page(page_width, page_height, zoom);

	page->add_image(std::move(scaled_img), y_begin, y_end);
	for (auto &tup: blocks)
	{
		if (tup.second)
			page = add_page(page_width, page_height, zoom);
		TextBlock &block = tup.first;
		page->add_text_block(std::move(block), block.y() - margin,
			block.y() + block.height() + margin);
	}
}

void Book::layout(int page_width, int page_height, float zoom)
{
	_pages.clear();
	for (const BookItem& item: _items)
	{
		if (item.type() == IMAGE)
			layout_image(item.image(), item.text(), page_width, page_height, zoom);
		else
			layout_text(item.type(), item.text(), page_width, page_height, zoom);
	}
	_laid_out = true;
}

Book Book::read_book(const std::string& file_name, PaperType paper_type, int id)
{
printf("reading %s\n", file_name.c_str());
	std::string path = std::string("languages") + '/' + lang + '/' + file_name;
	xmlDoc *doc = xmlReadFile(path.c_str(), 0, 0);
	if (!doc)
	{
		// Fall back on English if the book is not available in the user's language
		path = std::string("languages") + "/en/" + file_name;
		doc = xmlReadFile(path.c_str(), 0, 0);
		if (!doc)
		{
			char err[200];
			safe_snprintf(err, sizeof(err), book_open_err_str, path.c_str());
			LOG_TO_CONSOLE(c_red1, err);
			EXTENDED_EXCEPTION(ExtendedException::ec_file_not_found, err);
		}
	}

	xmlNode *root = xmlDocGetRootElement(doc);
	if (!root)
	{
		xmlFreeDoc(doc);
		EXTENDED_EXCEPTION(ExtendedException::ec_invalid_parameter,
			"Error while parsing: " << path);
	}
	if (xmlStrcasecmp(root->name, reinterpret_cast<const xmlChar*>("book")))
	{
		xmlFreeDoc(doc);
		EXTENDED_EXCEPTION(ExtendedException::ec_invalid_parameter,
			"Root element in " << path << " is not <book>");
	}

	xmlChar* title = xmlGetProp(root, reinterpret_cast<const xmlChar*>("title"));
	if (!title)
	{
		xmlFreeDoc(doc);
		EXTENDED_EXCEPTION(ExtendedException::ec_invalid_parameter,
			"Root element in " << path << " does not contain a title=\"<short title>\" property.");
	}

	Book book(paper_type, reinterpret_cast<const char*>(title), id);
	book.add_xml_content(root->children);

	xmlFree(title);
	xmlFreeDoc(doc);

	return book;
}

void Book::add_xml_image(const xmlNode *node)
{
	char *file_name = reinterpret_cast<char*>(xmlGetProp(node,
		reinterpret_cast<const xmlChar*>("src")));
	if (!file_name)
		return;

	int x = xmlGetInt(node, "x");
	int y = xmlGetInt(node, "y");
	int width = xmlGetInt(node, "w");
	int height = xmlGetInt(node, "h");

	float u_start = xmlGetFloat(node, "u_start", 0.0);
	float u_end = xmlGetFloat(node, "u_end", 1.0);
	float v_start = xmlGetFloat(node, "v_start", 1.0);
	float v_end = xmlGetFloat(node, "v_end", 0.0);

	Image image(file_name, x, y, width, height, u_start, v_start, u_end, v_end);

	xmlFree(file_name);

	ustring text;
	if (node->children && node->children->content)
	{
		char *text_ptr = 0;
		MY_XMLSTRCPY(&text_ptr, reinterpret_cast<const char*>(node->children->content));
		text.assign(reinterpret_cast<const unsigned char*>(text_ptr));
		free(text_ptr);
	}

	add_item(std::move(image), text);
}

void Book::add_xml_text(ContentType type, const xmlNode *node)
{
	if (node->children && node->children->content)
	{
		char* text = 0;
		if (MY_XMLSTRCPY(&text, reinterpret_cast<const char*>(node->children->content)) != -1)
		{
			add_item(type, reinterpret_cast<const unsigned char*>(text));
		}
		else
		{
#ifndef OSX
			LOG_ERROR("An error occured when parsing the content of the <%s>-tag on line %d - Check it for letters that cannot be translated into iso8859-1\n",
				node->name, node->line);
#else
			LOG_ERROR("An error occured when parsing the content of the <%s>-tag - Check it for letters that cannot be translated into iso8859-1\n",
				node->name);
#endif
		}
		free(text);
	}
}

void Book::add_xml_page(const xmlNode *node)
{
	for ( ; node; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE)
			continue;

		if (!xmlStrcasecmp(node->name, reinterpret_cast<const xmlChar*>("title")))
			add_xml_text(TITLE, node);
		else if (!xmlStrcasecmp(node->name, reinterpret_cast<const xmlChar*>("author")))
			add_xml_text(AUTHOR, node);
		else if (!xmlStrcasecmp(node->name, reinterpret_cast<const xmlChar*>("text")))
			add_xml_text(TEXT, node);
		else if (!xmlStrcasecmp(node->name, reinterpret_cast<const xmlChar*>("image")))
			add_xml_image(node);
	}
}

void Book::add_xml_content(const xmlNode *node)
{
	for (const xmlNode *cur = node; cur; cur = cur->next)
	{
		if (cur->type == XML_ELEMENT_NODE
			&& !xmlStrcasecmp(cur->name, reinterpret_cast<const xmlChar*>("page")))
		{
			add_xml_page(cur->children);
		}
	}
}

void Book::display(float zoom) const
{
	if (_active_page < 0 || size_t(_active_page) >= _pages.size())
		return;

	if (_paper_type == BOOK)
	{
		_pages[_active_page].display();
		if (size_t(_active_page + 1) < _pages.size())
		{
			glPushMatrix();
			glTranslatef(std::round(zoom * BookWindow::x_half_book), 0.0, 0.0);
			_pages[_active_page+1].display();
			glPopMatrix();
		}
	}
	else
	{
		_pages[_active_page].display();
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


TextLink::TextLink(int target, const char* text, int x, int y, float zoom,
	TextDrawOptions::Alignment alignment):
	_target(target), _text(reinterpret_cast<const unsigned char*>(text)),
	_y_begin(y), _y_end(y + FontManager::get_instance().line_height(UI_FONT, zoom)),
	_zoom(zoom)
{
	int width = FontManager::get_instance().line_width(UI_FONT, _text.c_str(),
		_text.length(), zoom);
	switch (alignment)
	{
		case TextDrawOptions::LEFT:
			_x_begin = x;
			break;
		case TextDrawOptions::CENTER:
			_x_begin = x - width / 2;
			break;
		case TextDrawOptions::RIGHT:
			_x_begin = x - width;
			break;
	}
	_x_end = _x_begin + width;
}

void TextLink::display() const
{
	TextDrawOptions options = TextDrawOptions().set_zoom(_zoom);
	if (_under_mouse)
		options.set_foreground(0.95f, 0.76f, 0.52f);
	else
		options.set_foreground(0.77f, 0.59f, 0.38f);
	FontManager::get_instance().draw(UI_FONT, _text.c_str(), _text.length(),
		_x_begin, _y_begin, options);
}


Book* BookWindow::get_book()
{
	try
	{
		return &BookCollection::get_instance().get_book(_book_id);
	}
	catch (const ExtendedException&)
	{
		return nullptr;
	}
}

void BookWindow::add_links(window_info *win)
{
	Book *book = get_book();

	_links.clear();

	float zoom = win->current_scale;
	int x, y_buttons = win->len_y - _ui_margin - _ui_font_height;
	int target = book->active_page_nr() - book->page_delta();
	if (book && target >= 0)
	{
		x = std::round(zoom * x_margin_button);
		_links.emplace_back(target, "<-", x, y_buttons, zoom);
	}

	x = win->len_x / 2;
	_links.emplace_back(-1, "[X]", x, y_buttons, zoom, TextDrawOptions::CENTER);

	target = book->active_page_nr() + book->page_delta();
	if (book && target < book->nr_pages())
	{
		x = std::round(win->len_x - zoom * x_margin_button);
		_links.emplace_back(target, "->", x, y_buttons, zoom, TextDrawOptions::RIGHT);
	}

	int button_width = FontManager::get_instance().line_width(UI_FONT,
		reinterpret_cast<const unsigned char*>("<-"), 2, zoom);
	int dx = std::round((win->len_x - 2 * zoom * x_margin_button - 2 * button_width) / 10);
	for (int j = -4; j < 5; ++j)
	{
		if (j == 0)
			continue;
		target = book->active_page_nr() + j * book->page_delta();
		if (target >= 0 && target < book->nr_pages())
		{
			std::string lbl = std::to_string(target + 1);
			x = win->len_x / 2 + j * dx;
			_links.emplace_back(target, lbl.c_str(), x, y_buttons, zoom, TextDrawOptions::CENTER);
		}
	}
}

int BookWindow::display_handler(window_info *win)
{
	Book *book = get_book();
	if (!book)
	{
		hide_window(win->window_id);
		return 0;
	}

	float zoom = win->current_scale;
	if (!book->is_laid_out())
	{
		int page_width = book->paper_type() == Book::BOOK
			? std::round(zoom * page_width_book)
			: std::round(zoom * page_width_paper);
		int page_height = book->paper_type() == Book::BOOK
			? std::round(zoom * page_height_book)
			: std::round(zoom * page_height_paper);
		book->layout(page_width, page_height, zoom);
		add_links(win);
	}

	int y_buttons = win->len_y - _ui_margin - _ui_font_height;
	int y_bottom = y_buttons - _ui_margin;
	bind_texture(book->paper_type() == Book::BOOK ? _book_texture : _paper_texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex2i(0,          0);
	glTexCoord2f(0.0f, 0.0f); glVertex2i(0,          y_bottom);
	glTexCoord2f(1.0f, 0.0f); glVertex2i(win->len_x, y_bottom);
	glTexCoord2f(1.0f, 1.0f); glVertex2i(win->len_x, 0);
	glEnd();

	float dx = std::round(
		zoom * (book->paper_type() == Book::BOOK ? x_offset_book : x_offset_paper)
	);
	float dy = std::round(
		zoom * (book->paper_type() == Book::BOOK ? y_offset_book : y_offset_paper)
	);

	glPushMatrix();
	glTranslatef(dx, dy, 0.0);
	book->display(zoom);
	glPopMatrix();

	for (const auto& link: _links)
		link.display();

	return 1;

// 	if(b->type==1) {
// 		int x_off[4] = {50 * win->current_scale, 100 * win->current_scale, win->len_x - 120 * win->current_scale, win->len_x - 70 * win->current_scale};
// 		int p_inc[4] = {-5, -2, 2, 5};
// 		int i;
// 		for (i=0; i<4; i++)
// 		{
// 			int p = b->active_page + p_inc[i];
// 			if(p >= 0 && p < b->no_pages)
// 			{
// 				safe_snprintf(str,sizeof(str),"%d",p+1);
// 				if (book_mouse_y > 0
// 					&& book_mouse_y < win->default_font_len_y && book_mouse_x > x_off[i]
// 					&& book_mouse_x < x_off[i] + get_string_width_ui((unsigned char*)str, text_zoom))
// 					glColor3f(0.95f, 0.76f, 0.52f);
// 				else
// 					glColor3f(0.77f,0.59f, 0.38f);
// 				draw_string_zoomed(x_off[i], 0, (unsigned char*)str, 0, win->current_scale);
// 			}
// 		}
// 	} else if(b->type==2) {
// 		int x_off[2] = { win->len_x / 2 - (int)(0.5 + win->current_scale * 60), win->len_x / 2 + (int)(0.5 + win->current_scale * 50)};
// 		int num_gap = (int)(0.5 + win->current_scale * 40);
// 		int sign[2] = {-1, 1};
// 		int i,j;
// 		for (j=0; j<2; j++) {
// 			for(i=1; i<5; i++) {
// 				int p = b->active_page + sign[j] * i * b->type;
// 				if (p >= 0 && p < b->no_pages) {
// 					safe_snprintf(str,sizeof(str),"%d",p+1);
// 					if (book_mouse_y > 0
// 						&& book_mouse_y < win->default_font_len_y && book_mouse_x > x_off[j]
// 						&& book_mouse_x < x_off[j] + get_string_width_ui((unsigned char*)str, text_zoom))
// 						glColor3f(0.95f, 0.76f, 0.52f);
// 					else
// 						glColor3f(0.77f,0.59f, 0.38f);
// 					draw_string_zoomed(x_off[j], 0, (unsigned char*)str, 0, win->current_scale);
// 				}
// 				x_off[j] += sign[j] * num_gap;
// 			}
// 		}
// 	}
}

int BookWindow::static_display_handler(window_info *win)
{
	BookWindow *window = reinterpret_cast<BookWindow*>(win->data);
	if (!window)
		return 0;
	return window->display_handler(win);
}

int BookWindow::mouseover_handler(int mx, int my)
{
	for (auto& link: _links)
		link.mouseover(mx, my);
	return 1;
}

int BookWindow::static_mouseover_handler(window_info *win, int mx, int my)
{
	BookWindow *window = reinterpret_cast<BookWindow*>(win->data);
	if (!window)
		return 0;
	return window->mouseover_handler(mx, my);
}

int BookWindow::click_handler(window_info *win, int mx, int my, Uint32 flags)
{
	if ((flags & ELW_MOUSE_BUTTON) == 0)
		return 0;

	Book *book = get_book();
	if (!book)
		return 0;

	for (const auto& link: _links)
	{
		if (link.is_under(mx, my))
		{
			if (link.target() < 0)
			{
				hide_window(win->window_id);
			}
			else
			{
				book->turn_to_page(link.target());
				add_links(win);
			}

			return 1;
		}
	}

	return 0;
}

int BookWindow::static_click_handler(window_info *win, int mx, int my, Uint32 flags)
{
	BookWindow *window = reinterpret_cast<BookWindow*>(win->data);
	if (!window)
		return 0;
	return window->click_handler(win, mx, my, flags);
}

void BookWindow::ui_scale_handler(window_info *win)
{
	_ui_margin = std::round(win->current_scale * ui_margin);
	_ui_font_height = FontManager::get_instance().line_height(UI_FONT, win->current_scale);
	int ui_height = 2 * _ui_margin + _ui_font_height;
	if (_book_win != -1)
	{
		int width = std::round(win->current_scale*book_width);
		int height = std::round(win->current_scale*book_height + ui_height);
		resize_window(win->window_id, width, height);
	}
	if (_paper_win != -1)
	{
		int width = std::round(win->current_scale*paper_width);
		int height = std::round(win->current_scale*paper_height + ui_height);
		resize_window(win->window_id, width, height);
	}
}

int BookWindow::static_ui_scale_handler(window_info *win)
{
	BookWindow *window = reinterpret_cast<BookWindow*>(win->data);
	if (!window)
		return 0;
	window->ui_scale_handler(win);
	return 1;
}

void BookWindow::display(Book &book)
{
	if (_book_texture == Uint32(-1))
		_book_texture = load_texture_cached("textures/book1.dds", tt_image);
	if (_paper_texture == Uint32(-1))
		_paper_texture = load_texture_cached("textures/paper1.dds", tt_image);

	int &win_id = book.paper_type() == Book::BOOK ? _book_win : _paper_win;
	if (win_id >= 0)
	{
		if (_book_id != book.id())
		{
			window_info *win = &windows_list.window[win_id];
			safe_strncpy(win->window_name, book.title().c_str(), sizeof(win->window_name));
			_book_id = book.id();
			add_links(win);
		}
		show_window(win_id);
		select_window(win_id);
	}
	else
	{
		_book_id = book.id();
		if (book.paper_type() == Book::BOOK)
		{
			win_id = create_window(book.title().c_str(), -1, 0, window_x, window_y,
				0, 0, (ELW_USE_UISCALE|ELW_WIN_DEFAULT)^ELW_CLOSE_BOX);
		}
		else
		{
			win_id = create_window(book.title().c_str(), -1, 0, window_x, window_y,
				0, 0, (ELW_USE_UISCALE|ELW_WIN_DEFAULT)^ELW_CLOSE_BOX);
		}

		if (win_id >= 0 && win_id < windows_list.num_windows)
		{
			window_info *win = &windows_list.window[win_id];
			win->data = reinterpret_cast<void*>(this);
			ui_scale_handler(win);
		}

		set_window_handler(win_id, ELW_HANDLER_DISPLAY, (int (*)())&static_display_handler);
		set_window_handler(win_id, ELW_HANDLER_MOUSEOVER, (int (*)())&static_mouseover_handler);
		set_window_handler(win_id, ELW_HANDLER_CLICK, (int (*)())&static_click_handler);
		set_window_handler(win_id, ELW_HANDLER_UI_SCALE, (int (*)())&static_ui_scale_handler);
	}
}


void BookCollection::initialize()
{
	static const std::array<std::pair<std::string, int>, 6> race_books = {
		std::make_pair("books/races/human.xml", book_human),
		std::make_pair("books/races/dwarf.xml", book_dwarf),
		std::make_pair("books/races/elf.xml", book_elf),
		std::make_pair("books/races/gnome.xml", book_gnome),
		std::make_pair("books/races/orchan.xml", book_orchan),
		std::make_pair("books/races/draegoni.xml", book_draegoni)
	};

    for (const auto& tup: race_books)
	{
		try
		{
			add_book(Book::read_book(tup.first, Book::BOOK, tup.second));
		}
		CATCH_AND_LOG_EXCEPTIONS
	}

	try
	{
		read_knowledge_book_index();
	}
	CATCH_AND_LOG_EXCEPTIONS
}

Book& BookCollection::get_book(int id)
{
	try
	{
		return _books.at(id);
	}
	catch (std::out_of_range&)
	{
		std::ostringstream os;
		os << "Book with id " << id << " not found";
		EXTENDED_EXCEPTION(ExtendedException::ec_item_not_found, os.str());
	}
}

void BookCollection::add_book(Book&& book)
{
	int id = book.id();
	auto res = _books.emplace(id, book);
	if (!res.second)
	{
		LOG_ERROR("A book with ID %d was already present", id);
	}
}

void BookCollection::parse_knowledge_item(const xmlNode *node)
{
	for (const xmlNode *cur = node; cur; cur = cur->next)
	{
		if (cur->type != XML_ELEMENT_NODE
			|| xmlStrcasecmp(cur->name, reinterpret_cast<const xmlChar*>("Knowledge"))
			|| !cur->children || !cur->children->content)
			continue;

		xmlChar* id_str = xmlGetProp(cur, reinterpret_cast<const xmlChar*>("ID"));
		if (!id_str)
		{
			LOG_ERROR("Knowledge Item does not contain an ID property.");
			continue;
		}

		char* fname = 0;
		if (MY_XMLSTRCPY(&fname, reinterpret_cast<const char*>(cur->children->content)) != -1)
		{
			int id = atoi(reinterpret_cast<const char*>(id_str));
			try
			{
				add_book(Book::read_book(fname, Book::BOOK, id + knowledge_book_offset));
				knowledge_list[id].has_book = 1;
			}
			catch (const ExtendedException& e)
			{
				LOG_ERROR("%s(): %s", __func__, e.what());
			}
			free(fname);
		}
		else
		{
#ifndef OSX
			LOG_ERROR("An error occured when parsing the content of the <%s>-tag on line %d - Check it for letters that cannot be translated into iso8859-1\n",
					cur->name, cur->line);
#else
			LOG_ERROR("An error occured when parsing the content of the <%s>-tag - Check it for letters that cannot be translated into iso8859-1\n",
					cur->name);
#endif
		}

		xmlFree(id_str);
	}
}

void BookCollection::read_knowledge_book_index()
{
	static const std::string path("knowledge.xml");

	xmlDoc *doc = xmlReadFile(path.c_str(), NULL, 0);
	if (!doc)
	{
		static const std::string err = "Can't open knowledge book index";
		LOG_TO_CONSOLE(c_red1, err.c_str());
		EXTENDED_EXCEPTION(ExtendedException::ec_file_not_found, err);
	}

	xmlNode *root = xmlDocGetRootElement(doc);
	if (!root)
	{
		xmlFreeDoc(doc);
		EXTENDED_EXCEPTION(ExtendedException::ec_invalid_parameter,
			"Error while parsing: " << path);
	}
	if (xmlStrcasecmp(root->name, reinterpret_cast<const xmlChar*>("Knowledge_Books")))
	{
		xmlFreeDoc(doc);
		EXTENDED_EXCEPTION(ExtendedException::ec_file_not_found,
			"Root element in " << path << " is not <Knowledge_Books>");
	}

	parse_knowledge_item(root->children);

	xmlFreeDoc(doc);
}

void BookCollection::open_book(int id)
{
	try
	{
		Book& book = get_book(id);
		_window.display(book);
	}
	catch (const ExtendedException&)
	{
		unsigned char msg[] = { SEND_BOOK, static_cast<unsigned char>(id & 0xff),
			static_cast<unsigned char>((id >> 8) & 0xff), 0, 0 };
		my_tcp_send(my_socket, msg, 5);
	}
}

void BookCollection::read_local_book(const unsigned char* data, size_t len)
{
	if (len < 3)
	{
		LOG_ERROR("Incomplete local book description from the server");
		return;
	}

	int id = int(data[1]) | int(data[2]) << 8;
	try
	{
		Book& book = get_book(id);
		_window.display(book);
	}
	catch (const ExtendedException&)
	{
		Book::PaperType paper_type = data[1] == 1 ? Book::PAPER : Book::BOOK;
		std::string file_name(data + 3, data + len);
		try
		{
			add_book(Book::read_book(file_name, paper_type, id));
			Book& book = get_book(id);
			_window.display(book);
		}
		CATCH_AND_LOG_EXCEPTIONS
	}
}

void BookCollection::read_server_book(const unsigned char* data, size_t len)
{
	printf("server!\n");
}

void BookCollection::read_network_book(const unsigned char* data, size_t len)
{
	if (len < 1)
		return;

	switch (*data)
	{
		case LOCAL:
			read_local_book(data + 1, len - 1);
			break;
		case SERVER:
			read_server_book(data + 1, len - 1);
			break;
		default:
			LOG_ERROR("Unknown book source ID %d", int(*data));
	}
}

} // namespace eternal_lands

extern "C"
{

using namespace eternal_lands;

void init_books(void)
{
	BookCollection::get_instance().initialize();
}

void open_book(int id)
{
	BookCollection::get_instance().open_book(id);
}

void read_network_book(const unsigned char* data, size_t len)
{
	BookCollection::get_instance().read_network_book(data, len);
}


} // extern "C"

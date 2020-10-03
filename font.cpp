#include <cmath>
#include <numeric>
#include <sstream>
#ifdef TTF
#ifndef WINDOWS
#include <glob.h>
#endif
#include <SDL_ttf.h>
#endif // TTF

#include "font.h"
#include "asc.h"
#include "chat.h"
#include "colors.h"
#include "elconfig.h"
#include "elloggingwrapper.h"
#include "exceptions/extendedexception.hpp"
#include "io/elpathwrapper.h"
#include "gl_init.h"
#include "textures.h"

namespace
{

#ifdef TTF
int next_power_of_two(int n)
{
	int res = 1;
	while (res < n)
		res *= 2;
	return res;
}
#endif

size_t memcspn(const unsigned char* text, size_t len,
	const unsigned char* reject, size_t len_reject)
{
	size_t i;
	bool rej[256] = { false };
	for (i = 0; i < len_reject; ++i)
		rej[reject[i]] = true;
	for (i = 0; i < len && !rej[text[i]]; ++i) /* do nothing */;
	return i;
}

bool filter_messages(const text_message *msgs, size_t msgs_size, Uint8 filter,
	size_t msg_start, size_t &imsg, size_t &ichar)
{
#ifndef MAP_EDITOR2
	if (filter != FILTER_ALL)
	{
		// skip all messages of the wrong channel
		while (skip_message(&msgs[imsg], filter))
		{
			if (++imsg >= msgs_size)
				imsg = 0;
			ichar = 0;
			if (imsg == msg_start || !msgs[imsg].data || imsg == msg_start)
				return true;
		}
	}
#endif
	return !msgs[imsg].data || msgs[imsg].deleted;
}

bool pos_selected(const select_info *sel, size_t imsg, size_t ichar)
{
	if (!sel || TEXT_FIELD_SELECTION_EMPTY(sel))
		return false;

	std::pair<size_t, size_t> start = std::make_pair(size_t(sel->sm), size_t(sel->sc));
	std::pair<size_t, size_t> end   = std::make_pair(size_t(sel->em), size_t(sel->ec));
	if (end < start)
		std::swap(start, end);

	return (imsg > start.first || (imsg == start.first && ichar >= start.second))
		&& (imsg < end.first   || (imsg == end.first   && ichar <= end.second));
}

#ifdef TTF
TTF_Font* open_font(const std::string& file_name, int point_size)
{
	// First try to interpret ttf_file_name as a path relative to ttf_directory. If that fails,
	// try as an absolute path.
	std::string path = ttf_directory + file_name;
	TTF_Font *font = TTF_OpenFont(path.c_str(), point_size);
	if (!font)
		font = TTF_OpenFont(file_name.c_str(), point_size);
	return font;
}
#endif

} // namespace

namespace eternal_lands
{

FontOption::FontOption(size_t font_nr): _font_nr(font_nr), _file_name(), _font_name(),
#ifdef TTF
	_is_ttf(false),
#endif
	_fixed_width(), _failed(false)
{
	static const std::array<const char*, 7> file_names = { {
		"textures/font.dds",
		"textures/font.dds",
		"textures/font2.dds",
		"textures/font3.dds",
		"textures/font5.dds",
		"textures/font6.dds",
		"textures/font7.dds"
	} };

	std::ostringstream os;
	if (font_nr > file_names.size())
	{
		_failed = true;
		LOG_ERROR("Invalid font number %zu", font_nr);
	}
	else if (font_nr == 0)
	{
		_file_name = file_names[font_nr];
		_font_name = "Type 1 (fixed)";
	}
	else
	{
		_file_name = file_names[font_nr];
		size_t begin = _file_name.find_last_of('/') + 1;
		size_t end = _file_name.find_last_of('.');
		os << "Type " << font_nr << " - " << _file_name.substr(begin, end);
		_font_name = os.str();
	}

	if (!el_file_exists(_file_name.c_str()))
	{
		LOG_ERROR("Unable to find font file '%s'", _file_name.c_str());
		_failed = true;
	}

	_fixed_width = (font_nr != 1 && font_nr != 2);
}

#ifdef TTF
FontOption::FontOption(const std::string& file_name): _font_nr(std::numeric_limits<size_t>::max()),
	_file_name(file_name), _font_name(), _is_ttf(true), _fixed_width(), _failed(false)
{
	TTF_Font *font = open_font(file_name.c_str(), 40);
	if (!font)
	{
		LOG_ERROR("Failed to open TTF font file '%s'", file_name.c_str());
		_failed = true;
		return;
	}

	// Quick check to see if the font is useful
	if (!TTF_GlyphIsProvided(font, 'A') || !TTF_GlyphIsProvided(font, '!'))
	{
		// Nope, can't render in this font
		TTF_CloseFont(font);
		LOG_ERROR("Unable to render text with TTF font file '%s'", file_name.c_str());
		_failed = true;
		return;
	}

	std::string name = TTF_FontFaceFamilyName(font);
	std::string style = TTF_FontFaceStyleName(font);

	_font_name = name + ' ' + style;
	_fixed_width = (TTF_FontFaceIsFixedWidth(font) != 0);

	TTF_CloseFont(font);
}
#endif // TTF

void FontOption::add_select_options(bool add_button) const
{
	add_multi_option_with_id("ui_font",    _font_name.c_str(), _file_name.c_str(), add_button);
	add_multi_option_with_id("chat_font",  _font_name.c_str(), _file_name.c_str(), add_button);
	add_multi_option_with_id("name_font",  _font_name.c_str(), _file_name.c_str(), add_button);
	add_multi_option_with_id("book_font",  _font_name.c_str(), _file_name.c_str(), add_button);
	add_multi_option_with_id("note_font",  _font_name.c_str(), _file_name.c_str(), add_button);
	add_multi_option_with_id("rules_font", _font_name.c_str(), _file_name.c_str(), add_button);
	if (is_fixed_width())
	{
		add_multi_option_with_id("encyclopedia_font", _font_name.c_str(), _file_name.c_str(),
			add_button);
	}
}

const std::array<float, 3> TextDrawOptions::default_foreground_color = { 1.0f, 1.0f, 1.0f };
const std::array<float, 3> TextDrawOptions::default_background_color = { 0.0f, 0.0f, 0.0f };
const std::array<float, 3> TextDrawOptions::default_selection_color = { 1.0f, 0.635f, 0.0f };

TextDrawOptions::TextDrawOptions(): _max_width(0), _max_lines(0), _zoom(1.0),
	_line_spacing(1.0), _alignment(LEFT), _vertical_alignment(TOP_LINE), _flags(0),
	_fg_color{{-1.0, -1.0, -1.0}}, _bg_color{{-1.0, -1.0, -1.0}},
	_sel_color(default_selection_color) {}

void TextDrawOptions::use_background_color() const
{
	if (has_background_color())
		glColor3fv(_bg_color.data());
	else
		glColor3fv(default_background_color.data());
}

void TextDrawOptions::use_foreground_color() const
{
	if (has_foreground_color())
		glColor3fv(_fg_color.data());
	else
		glColor3fv(default_foreground_color.data());
}

void TextDrawOptions::use_selection_color() const
{
	glColor3fv(_sel_color.data());
}

// Relative letter frequencies for English text, based on the counts in
//
// M.N. Jones and D.J.K. Mewhort,
// Behavior Research Methods, Instruments, & Computers 36, pp. 388-396 (2004)
//
// These are used to compute an "average" character width for a font. When
// using this, please note that:
// 1) This is based on English text, so these frequencies may not be applicable
//    to text in to other languages,
// 2) It is based on text from the New York Times, and so may not be
//    representative for text used in EL,
// 3) Only ASCII characters were counted (so accented characters don't contribute),
//    and a few ASCII characters are missing as well ('[', '\', ']', '^', '_',
//    and '`').
// 4) The occurance of the space character was guesstimated from the reported total
//    number of words in the corpus.
//
// When you need to be sure a text will fit in a certain space, use the maximum
// character width or use a monospaced font.
const std::array<int, Font::nr_glyphs> Font::letter_freqs = {
	1647,    0,   33,    0,    6,    0,    1,   24,    6,    6,    2,    0,  116,   30,
	 111,    1,   64,   54,   39,   22,   23,   44,   18,   14,   21,   33,    6,    4,
	   0,    0,    0,    1,    0,   33,   20,   27,   15,   16,   12,   11,   15,   26,
	   9,    5,   13,   31,   24,   12,   17,    1,   17,   36,   38,    7,    4,   13,
	   1,   11,    1,    0,    0,    0,    0,    0,    0,  619,  102,  231,  279,  911,
	 153,  142,  348,  533,    8,   54,  300,  173,  534,  556,  148,    6,  487,  492,
	 648,  190,   77,  119,   15,  125,    8,    0,    0,    0,    0
};
const ustring Font::ellipsis = reinterpret_cast<const unsigned char*>("...");

Font::Font(Font&& font): _font_name(font.font_name()), _file_name(font.file_name()),
	_flags(font._flags), _texture_width(font._texture_width),
	_texture_height(font._texture_height), _metrics(font._metrics), _block_width(font._block_width),
	_line_height(font._line_height),
	_vertical_advance(font._vertical_advance), _font_top_offset(font._font_top_offset),
	_digit_center_offset(font._digit_center_offset), _password_center_offset(font._password_center_offset),
	_max_advance(font._max_advance), _max_digit_advance(font._max_digit_advance),
	_avg_advance(font._avg_advance), _spacing(font._spacing),
	_scale_x(font._scale_x), _scale_y(font._scale_y),
#ifdef TTF
	_point_size(font._point_size), _outline(font._outline),
#endif
	_texture_id(font._texture_id)
{
	font._flags &= ~HAS_TEXTURE;
}

Font::Font(const FontOption& option): _font_name(option.font_name()),
	_file_name(option.file_name()), _flags(0), _texture_width(256), _texture_height(256), _metrics(),
	_block_width(font_block_width), _line_height(default_vertical_advance + 1),
	_vertical_advance(default_vertical_advance), _font_top_offset(0),
	_digit_center_offset(option.font_number() == 2 ? 10 : 9), _password_center_offset(0),
	_max_advance(12), _max_digit_advance(12), _avg_advance(12), _spacing(0),
	_scale_x(11.0 / 12), _scale_y(1.0),
#ifdef TTF
	_point_size(0), _outline(0),
#endif
	_texture_id()
{
	static const std::array<int, nr_glyphs> top_1 = {
		15,  2,  2,  2,  1,  2,  3,  2,  2,  2,  2,  6, 11,  8,
		11,  0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  6,  6,
		 6,  7,  6,  2,  3,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		 2,  2,  2,  1,  0,  1, -1,  4,  1,  5,  2,  5,  2,  5,
		 2,  5,  2,  1,  1,  2,  2,  5,  5,  5,  5,  5,  5,  5,
		 3,  5,  5,  5,  5,  5,  5,  2,  2,  2,  8,  2,  1,  1,
		 1,  6,  1,  2,  1,  2,  1,  1,  2,  2,  2, -1, -1, -1,
		 2,  5,  5,  1,  2,  2, -1,  2,  0,  1,  0,  0,  1,  0,
		 1,  0,  1,  0,  1,  1
	};
	static const std::array<int, nr_glyphs> bottom_1 = {
		15, 15,  9, 17, 18, 15, 15,  9, 17, 17, 12, 15, 18, 11,
		15, 17, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 17,
		15, 13, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 18, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 17, 17, 17,  6, 17,  6, 15, 15, 15, 15, 15,
		15, 18, 15, 15, 18, 15, 15, 15, 15, 15, 18, 18, 15, 15,
		15, 15, 15, 15, 15, 18, 15, 19, 19, 19, 12, 15, 15, 15,
		15, 19, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16,
		15, 15, 15, 15, 15, 15, 16, 15, 15, 15, 16, 16, 15, 16,
		15, 16, 15, 16, 15, 15
	};
	static const std::array<int, 7> asterisk_centers = { 7, 7, 7, 7, 6, 5, 6 };

	size_t font_nr = option.font_number();
	std::array<int, nr_glyphs> char_widths;
	if (font_nr == 1)
	{
		char_widths = { {
			 4,  2,  7, 11,  8, 12, 12,  2,  7,  7,  9, 10,  3,  8,
			 2, 10, 10, 10,  8,  8, 10,  7,  9,  9,  9,  9,  3,  3,
			10, 10, 10,  9, 12, 12,  9, 10, 10,  9,  9, 10,  9,  8,
			 7, 11,  8, 11, 10, 11,  9, 11, 11,  9, 10,  9, 12, 12,
			12, 12, 10,  6, 10,  6, 10, 12,  3, 11,  9,  9,  9,  9,
			 8,  9,  9,  4,  6, 10,  4, 11,  9, 10,  9,  9,  8,  8,
			 8,  9, 10, 12, 10, 10,  9,  8,  2,  8, 10,  8, 12, 12,
			12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
			12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
			12, 12, 12, 12, 12, 12
		} };
		_max_digit_advance = *std::max_element(char_widths.begin() + 16, char_widths.begin() + 26);
		_spacing = 4;
	}
	else if (font_nr == 2)
	{
		char_widths = { {
			 8,  8,  8, 10,  8, 10, 10,  8,  8,  8,  8, 10,  8,  8,
			 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
			10, 10, 10,  8, 12, 10, 10, 10, 10, 10, 10, 10, 10, 10,
			10, 10, 10, 10, 10, 10, 10, 10,  8, 10, 10, 10, 10, 10,
			10, 10, 10, 10, 10, 10, 10,  8,  8,  8,  8,  8,  8,  8,
			10,  8,  8,  8,  8,  8,  8, 10,  8,  8,  8,  8,  8,  8,
			 8,  8,  8, 10,  8,  8,  8, 10,  8, 10, 10,  8, 10,  8,
			 8,  8, 10, 10, 10,  8, 10, 10,  8,  8,  8, 12, 12, 12,
			10, 10, 12, 10, 12, 12
		} };
		_max_digit_advance = *std::max_element(char_widths.begin() + 16, char_widths.begin() + 26);
		_spacing = 2;
	}
	else
	{
		std::fill(char_widths.begin(), char_widths.end(), 12);
		_flags |= Flags::FIXED_WIDTH;
	}

	for (size_t pos = 0; pos < nr_glyphs; ++pos)
	{
		int row = pos / font_chars_per_line;
		int col = pos % font_chars_per_line;

		int cw = char_widths[pos] + _spacing;
		int skip = (12 - cw) / 2;

		_metrics[pos].width = char_widths[pos];
		_metrics[pos].advance = char_widths[pos];
		_metrics[pos].top = font_nr < 2 ? top_1[pos] : 0;
		_metrics[pos].bottom = font_nr < 2 ? bottom_1[pos] : _line_height;
		_metrics[pos].u_start = float(col * font_block_width + skip) / 256;
		_metrics[pos].v_start = float(row * font_block_height) / 256;
		_metrics[pos].u_end = float((col+1) * font_block_width - 7 - skip) / 256;
		_metrics[pos].v_end = float((row+1) * font_block_height - 2) / 256;
	}
	_avg_advance = calc_average_advance();
	_password_center_offset = asterisk_centers[font_nr];
}

#ifdef TTF
Font::Font(const FontOption& option, int height): _font_name(option.font_name()),
	_file_name(option.file_name()), _flags(IS_TTF), _texture_width(0), _texture_height(0),
	_metrics(), _block_width(0), _line_height(0), _vertical_advance(0), _font_top_offset(0),
	_digit_center_offset(0), _password_center_offset(0), _max_advance(0), _max_digit_advance(0),
	_avg_advance(0), _spacing(0), _scale_x(1.0), _scale_y(1.0), _point_size(),
	_outline(std::round(float(height) / (font_block_height - 2))), _texture_id()
{
	_point_size = find_point_size(height);
	if (option.is_fixed_width())
		_flags |= Flags::FIXED_WIDTH;
}
#endif

Font::~Font()
{
#ifdef TTF
	if (is_ttf() && has_texture())
		glDeleteTextures(1, &_texture_id.gl_id);
#endif
}

int Font::get_position(unsigned char c)
{
	static const int pos_table[256] = {
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
		16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
		32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
		48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
		64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
		80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  -1,
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		-1, 122,  -1,  -1, 109, 118, 116,  -1,  -1, 123,  -1,  -1,  -1, 125,  -1,  -1,
		-1, 120,  -1, 127,  -1,  -1, 110,  -1, 117,  -1, 129,  -1, 111,  -1,  -1, 112,
		98, 121,  97,  -1, 106, 115, 113,  99, 102,  96, 100, 101, 124, 124,  -1, 103,
		-1, 119, 126, 126, 104,  -1, 107,  -1, 114, 105, 128,  -1, 108,  -1,  -1,  -1
	};

	return pos_table[c];
}

int Font::width_pos(int pos, float zoom) const
{
	if (pos < 0)
		return 0;
	return std::round(_metrics[pos].advance * _scale_x * zoom);
}

int Font::width_spacing_pos(int pos, float zoom) const
{
	if (pos < 0)
		return 0;
	// return width of character + spacing between chars (supports variable width fonts)
	return std::round((_metrics[pos].advance + _spacing) * _scale_x * zoom);
}

int Font::max_width_spacing(float zoom) const
{
	return std::round((_max_advance + _spacing) * _scale_x * zoom);
}

int Font::average_width_spacing(float zoom) const
{
	return std::round((_avg_advance + _spacing) * _scale_x * zoom);
}

int Font::max_digit_width_spacing(float zoom) const
{
	return std::round((_max_digit_advance + _spacing) * _scale_x * zoom);
}

int Font::line_width(const unsigned char* str, size_t len, float zoom) const
{
	int cur_width = 0;
	int last_pos = -1;
	for (size_t i = 0; i < len; ++i)
	{
		int pos = get_position(str[i]);
		if (pos >= 0)
		{
			cur_width += width_spacing_pos(pos, zoom);
			last_pos = pos;
		}
	}
	cur_width -= width_spacing_pos(last_pos, zoom) - width_pos(last_pos, zoom);

	return cur_width;
}

int Font::line_width_spacing(const unsigned char* str, size_t len, float zoom) const
{
	int cur_width = 0;
	for (size_t i = 0; i < len; ++i)
		cur_width += width_spacing(str[i], zoom);

	return cur_width;
}

int Font::height(float zoom) const
{
	return std::round(_line_height * _scale_y * zoom);
}

int Font::vertical_advance(float zoom, float line_spacing) const
{
	return std::round(_vertical_advance * _scale_y * zoom * line_spacing);
}

int Font::max_nr_lines(int max_height, float zoom, float line_spacing) const
{
	int line_height = height(zoom);
	if (max_height < line_height)
		return 0;

	int line_skip = vertical_advance(zoom, line_spacing);
	return 1 + (max_height - line_height) / line_skip;
}

int Font::text_height(int nr_lines, float zoom, float line_spacing)
{
	if (nr_lines <= 0)
		return 0;
	return height(zoom) + (nr_lines-1) * vertical_advance(zoom, line_spacing);
}

std::pair<int, int> Font::dimensions(const unsigned char* str, size_t len, float zoom,
	float line_spacing) const
{
	if (len == 0)
		return std::make_pair(0, 0);

	size_t end = memcspn(str, len, reinterpret_cast<const unsigned char*>("\r\n"), 2);
	int w = line_width(str, end, zoom);
	int h = height(zoom);
	size_t off = end + 1;

	int line_skip = vertical_advance(zoom, line_spacing);
	while (off < len)
	{
		end = memcspn(str + off, len - off, reinterpret_cast<const unsigned char*>("\r\n"), 2);
		w = std::max(w, line_width(str + off, end, zoom));
		h += line_skip;
		off += end + 1;
	}
	return std::make_pair(w, h);
}

std::pair<int, int> Font::top_bottom_unscaled(const unsigned char* text, size_t len)
{
	int top = _line_height, bottom = 0;
	for (size_t i = 0; i < len; ++i)
	{
		int pos = get_position(text[i]);
		if (pos >= 0)
		{
			top = std::min(top, _metrics[pos].top);
			bottom = std::max(bottom, _metrics[pos].bottom);
		}
	}

	return std::make_pair(top, bottom);
}

std::pair<int, int> Font::top_bottom(const unsigned char* text, size_t len, float zoom)
{
	int top, bottom;
	std::tie(top, bottom) = top_bottom_unscaled(text, len);
	top = std::round(_scale_y * zoom * top);
	bottom = std::round(_scale_y * zoom * bottom);

	return std::make_pair(top, bottom);
}

int Font::center_offset(const unsigned char* text, size_t len, float zoom)
{
	int top, bottom;
	std::tie(top, bottom) = top_bottom_unscaled(text, len);
	if (top >= bottom)
		return 0;

	return std::round(_scale_y * zoom * 0.5 * (bottom + top - _line_height));
}

// Rules for wrapping text:
// 1) Soft line breaks take up no space, the cursor should never be on a soft line break.
// 2) The cursor can be on a hard line break character, though. Inserting at that position will
//    insert a new character at that position. So a hard break takes up the width of a cursor,
//    even though it is invisible.
// 3) When possible, lines are broken after the last space. If no space is found on the current
//    line, the word is broken after the last character that fits on the line. Since the cursor
//    should never be on a soft break, we can fill the entire line.
// 4) We must be able to place the cursor at the end of the text. Therefore, if the last line cannot
//    fit the cursor, an extra soft break is inserted.
std::tuple<ustring, int, int> Font::reset_soft_breaks(const unsigned char *text,
	size_t text_len, const TextDrawOptions& options, ssize_t cursor, float *max_line_width)
{
	int block_width = std::ceil(_block_width * _scale_x * options.zoom());
	int cursor_width = width_spacing('_', options.zoom());

	if (!text || options.max_width() < block_width)
		return std::make_tuple(ustring(), 0, 0);

	std::basic_string<unsigned char> wrapped_text;
	size_t start = 0, end, end_last_space = 0, last_start = 0;
	int nr_lines = 0;
	int diff_cursor = 0;
	while (start < text_len)
	{
		int cur_width = 0;
		for (end = start; end < text_len; ++end)
		{
			unsigned char c = text[end];
			if (c == '\r')
			{
				continue;
			}

			// We need to be able to place a cursor at this position, so use the maximum of the
			// cursor width and character width to determine if the character fits.
			int chr_width = width_spacing(c, options.zoom());
			if (cur_width + block_width <= options.max_width()
				|| cur_width + std::max(chr_width, cursor_width) <= options.max_width())
			{
				cur_width += chr_width;
				if (c == ' ')
				{
					end_last_space = end + 1;
				}
				else if (c == '\n')
				{
					++end;
					break;
				}
			}
			else
			{
				// Character won't fit. Split line after the last space.
				// If not found, break in the middle of the word.
				if (end_last_space > start)
					end = end_last_space;
				break;
			}
		}

		for (size_t i = start; i < end; ++i)
		{
			if (text[i] == '\r')
			{
				if (ssize_t(i) < cursor)
					--diff_cursor;
			}
			else
			{
				wrapped_text.push_back(text[i]);
			}
		}

		++nr_lines;
		if (end < text_len && (wrapped_text.empty() || wrapped_text.back() != '\n'))
		{
			wrapped_text.push_back('\r');
			if (ssize_t(end) <= cursor)
				++diff_cursor;
		}

		if (max_line_width)
		{
			cur_width = line_width(text + start, end - start, options.zoom());
			*max_line_width = std::max(*max_line_width, float(cur_width));
		}

		last_start = start;
		start = end;
	}

	// If the message ends on a newline, an extra empty line is printed, but it has not been counted yet.
	if (!wrapped_text.empty() && wrapped_text.back() == '\n')
		++nr_lines;

	// If a cursor will not fit behind the last line, add an extra line break.
	int last_line_width = line_width(text + last_start, text_len - last_start, options.zoom());
	if (last_line_width + cursor_width > options.max_width())
	{
		wrapped_text.push_back('\r');
		++nr_lines;
	}

	return std::make_tuple(wrapped_text, nr_lines, diff_cursor);
}

bool Font::load_texture()
{
#ifdef TTF
	if (is_ttf())
	{
		return build_texture_atlas();
	}
	else
	{
		_texture_id.cache_id = ::load_texture_cached(_file_name.c_str(), tt_font);
		_flags |= Flags::HAS_TEXTURE;
		return true;
	}
#else
	_texture_id = ::load_texture_cached(_file_name.c_str(), tt_font);
	_flags |= Flags::HAS_TEXTURE;
	return true;
#endif
}

void Font::bind_texture() const
{
#ifdef TTF
	if (is_ttf())
	{
		::bind_texture_id(_texture_id.gl_id);
	}
	else
	{
		::bind_texture(_texture_id.cache_id);
	}
#else // TTF
	::bind_texture(_texture_id);
#endif // TTF
}

void Font::get_texture_coordinates(int pos,
	float &u_start, float &u_end, float &v_start, float &v_end) const
{
	u_start = _metrics[pos].u_start;
	v_start = _metrics[pos].v_start;
	u_end   = _metrics[pos].u_end;
	v_end   = _metrics[pos].v_end;
}

void Font::set_color(int color)
{
	float r = static_cast<float>(colors_list[color].r1) / 255;
	float g = static_cast<float>(colors_list[color].g1) / 255;
	float b = static_cast<float>(colors_list[color].b1) / 255;
	//This fixes missing letters in the font on some clients
	//No idea why going from 3f to 4f helps, but it does
	glColor4f(r, g, b, 1.0);
}

int Font::draw_char(unsigned char c, int x, int y, float zoom, bool ignore_color) const
{
	if (is_color(c))
	{
		if (!ignore_color)
			set_color(from_color_char(c));
		return 0;
	}

	int pos = get_position(c);
	if (pos < 0) // watch for illegal/non-display characters
	{
		return 0;
	}

	// There are two widths here: char_width and advance. The first is the width
	// of the character as drawn on the screen, the second the number of pixels
	// the pen should advance for drawing the next character. Interestingly,
	// char_width can be larger than advance, epsecially for bold fonts. For
	// size calculations, the only relevant quantity is advance, though.
	int char_width = std::round((_metrics[pos].width + _spacing) * _scale_x * zoom);
	int advance = width_spacing_pos(pos, zoom);
	int char_height = height(zoom);

	float u_start, u_end, v_start, v_end;
	get_texture_coordinates(pos, u_start, u_end, v_start, v_end);

	// and place the text from the graphics on the map
	glTexCoord2f(u_start, v_start); glVertex3i(x, y, 0);
	glTexCoord2f(u_start, v_end);   glVertex3i(x, y + char_height, 0);
	glTexCoord2f(u_end,   v_end);   glVertex3i(x + char_width, y + char_height, 0);
	glTexCoord2f(u_end,   v_start); glVertex3i(x + char_width, y, 0);

	return advance;
}

void Font::draw_help_background(int x, int y, int width, int height) const
{
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	glBegin(GL_QUADS);
	glVertex3i(x - 1,     y + height, 0);
	glVertex3i(x - 1,     y,          0);
	glVertex3i(x + width, y,          0);
	glVertex3i(x + width, y + height, 0);
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
}

void Font::draw_line(const unsigned char* text, size_t len, int x, int y,
	const TextDrawOptions &options, size_t sel_begin, size_t sel_end) const
{
	if (options.shadow())
	{
		TextDrawOptions new_options = options;
		new_options.set_shadow(false);

		int delta = std::round(options.zoom());
		if (delta > 0)
		{
			new_options.set_ignore_color();

			new_options.use_background_color();
			draw_line(text, len, x-delta, y-delta, new_options, sel_begin, sel_end);
			draw_line(text, len, x-delta, y,       new_options, sel_begin, sel_end);
			draw_line(text, len, x-delta, y+delta, new_options, sel_begin, sel_end);
			draw_line(text, len, x,       y+delta, new_options, sel_begin, sel_end);
			draw_line(text, len, x+delta, y+delta, new_options, sel_begin, sel_end);
			draw_line(text, len, x+delta, y,       new_options, sel_begin, sel_end);
			draw_line(text, len, x+delta, y-delta, new_options, sel_begin, sel_end);
			draw_line(text, len, x,       y-delta, new_options, sel_begin, sel_end);
		}

		new_options.set_ignore_color(false);
		draw_line(text, len, x, y, new_options, sel_begin, sel_end);
	}
	else if (sel_end <= sel_begin || sel_begin >= len || options.ignore_color())
	{
		if (!options.ignore_color() && options.has_foreground_color())
			options.use_foreground_color();

		int cur_x = x;
		for (size_t i = 0; i < len; ++i)
		{
			cur_x += draw_char(text[i], cur_x, y, options.zoom(), options.ignore_color());
		}
	}
	else
	{
		int cur_x = x;
		int last_color_char = 0;
		if (sel_begin > 0)
		{
			if (options.has_foreground_color())
				options.use_foreground_color();

			for (size_t i = 0; i < sel_begin; ++i)
			{
				if (is_color(text[i]))
					last_color_char = text[i];
				cur_x += draw_char(text[i], cur_x, y, options.zoom(), false);
			}
		}

		options.use_selection_color();

		for (size_t i = sel_begin; i < std::min(sel_end, len); ++i)
		{
			if (is_color(text[i]))
				last_color_char = text[i];
			cur_x += draw_char(text[i], cur_x, y, options.zoom(), false);
		}

		if (len > sel_end)
		{
			if (last_color_char)
				set_color(from_color_char(last_color_char));
			else
				options.use_foreground_color();

			for (size_t i = sel_end; i < len; ++i)
			{
				cur_x += draw_char(text[i], cur_x, y, options.zoom(), false);
			}
		}
	}
}

std::pair<size_t, size_t> Font::clip_line(const unsigned char *text, size_t len,
	const TextDrawOptions &options, unsigned char &before_color, unsigned char &after_color,
	int &width) const
{
	int max_width = options.max_width() > 0 ? options.max_width() : window_width;

	before_color = 0;
	after_color = 0;
	width = line_width_spacing(text, len, options.zoom());
	if (width <= max_width)
		// Entire string fits
		return std::make_pair(0, len);

	int trunc_width = max_width;
	int ellipsis_width = 0;
	if (options.ellipsis())
	{
		ellipsis_width = line_width_spacing(ellipsis, options.zoom());
		if (options.alignment() == TextDrawOptions::Alignment::CENTER)
			ellipsis_width *= 2;
		trunc_width -= ellipsis_width;
	}

	size_t start = 0, end = len;
	switch (options.alignment())
	{
		case TextDrawOptions::Alignment::LEFT:
		{
			while (end > 0 && width > trunc_width)
			{
				unsigned char ch = text[--end];
				if (is_color(ch) && !after_color)
					after_color = ch;
				else
					width -= width_spacing(ch, options.zoom());
			}
			break;
		}
		case TextDrawOptions::Alignment::RIGHT:
		{
			while (start < len && width > trunc_width)
			{
				unsigned char ch = text[start++];
				if (is_color(ch))
					before_color = ch;
				else
					width -= width_spacing(ch, options.zoom());
			}
			break;
		}
		case TextDrawOptions::Alignment::CENTER:
		{
			int d_left = 0, d_right = 0;
			while (start < end && width - d_left - d_right > trunc_width)
			{
				if (d_left < d_right)
				{
					unsigned char ch = text[start++];
					if (is_color(ch))
						before_color = ch;
					else
						d_left += width_spacing(ch, options.zoom());
				}
				else
				{
					unsigned char ch = text[--end];
					if (is_color(ch) && !after_color)
						after_color = ch;
					else
						d_right += width_spacing(ch, options.zoom());
				}
			}
			width -= d_left + d_right;
			break;
		}
	}

	width = max_width;
	return std::make_pair(start, end-start);
}

void Font::draw(const unsigned char* text, size_t len, int x, int y,
	const TextDrawOptions &options, size_t sel_begin, size_t sel_end) const
{
	if (options.shrink_to_fit() && options.max_width() > 0)
	{
		int width = dimensions(text, len, options.zoom()).first;
		if (width > options.max_width())
		{
			TextDrawOptions new_options = TextDrawOptions(options).set_max_width(0)
				.set_zoom(options.zoom() * float(options.max_width()) / width)
				.set_shrink_to_fit(false);
			draw(text, len, x, y, new_options, sel_begin, sel_end);
			return;
		}
	}

	int tot_width = 0, tot_height = 0;
	if (options.is_help()
		|| options.vertical_alignment() == TextDrawOptions::VerticalAlignment::BOTTOM_LINE
		|| options.vertical_alignment() == TextDrawOptions::VerticalAlignment::CENTER_LINE)
	{
		std::tie(tot_width, tot_height) = dimensions(text, len, options.zoom(), options.line_spacing());
	}

	switch (options.vertical_alignment())
	{
		case TextDrawOptions::VerticalAlignment::TOP_LINE:
			break;
		case TextDrawOptions::VerticalAlignment::TOP_FONT:
			y -= std::round(options.zoom() * _scale_y * _font_top_offset);
			break;
		case TextDrawOptions::VerticalAlignment::BOTTOM_LINE:
			y -= tot_height;
			break;
		case TextDrawOptions::VerticalAlignment::CENTER_LINE:
			y -= tot_height / 2;
			break;
		case CENTER_DIGITS:
			y -= std::round(options.zoom() * _scale_y * _digit_center_offset);
			break;
		case CENTER_PASSWORD:
			y -= std::round(options.zoom() * _scale_y * _password_center_offset);
			break;
	}

	if (options.is_help())
	{
		int x_left = 0;
		switch (options.alignment())
		{
			case TextDrawOptions::Alignment::LEFT:   x_left = x; break;
			case TextDrawOptions::Alignment::CENTER: x_left = x - tot_width / 2; break;
			case TextDrawOptions::Alignment::RIGHT:  x_left = x - tot_width; break;
		}
		draw_help_background(x_left, y, tot_width, tot_height);
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glEnable(GL_ALPHA_TEST); // enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER, 0.1f);
	bind_texture();
	glBegin(GL_QUADS);

	int line_skip = vertical_advance(options.zoom(), options.line_spacing());
	int nr_lines_drawn = 0;
	size_t start = 0;
	while (start < len)
	{
		size_t line_len = memcspn(text + start, len-start,
			reinterpret_cast<const unsigned char*>("\r\n"), 2);
		size_t clipped_off, clipped_line_len;
		int line_width;
		unsigned char before_color, after_color;
		std::tie(clipped_off, clipped_line_len) = clip_line(text + start, line_len,
			options, before_color, after_color, line_width);
		bool draw_ellipsis = options.ellipsis() && clipped_line_len < line_len;
		int ellipsis_width = draw_ellipsis ? line_width_spacing(ellipsis, options.zoom()) : 0;

		int x_left = 0;
		switch (options.alignment())
		{
			case TextDrawOptions::Alignment::LEFT:   x_left = x; break;
			case TextDrawOptions::Alignment::CENTER: x_left = x - line_width / 2; break;
			case TextDrawOptions::Alignment::RIGHT:  x_left = x - line_width; break;
		}

		if (before_color)
			set_color(from_color_char(before_color));

		int x_draw = x_left;
		if (draw_ellipsis && options.alignment() != TextDrawOptions::Alignment::LEFT)
		{
			draw_line(ellipsis.data(), ellipsis.length(), x_draw, y, options);
			x_draw += ellipsis_width;
		}
		draw_line(text + start + clipped_off, clipped_line_len, x_draw, y, options,
			sel_begin < start ? 0 : sel_begin - start,
			sel_end < start ? 0 : sel_end - start);
		if (draw_ellipsis && options.alignment() != TextDrawOptions::Alignment::RIGHT)
		{
			x_draw = x_left + line_width - ellipsis_width;
			draw_line(ellipsis.data(), ellipsis.length(), x_draw, y, options);
		}

		if (after_color)
			set_color(from_color_char(after_color));

		++nr_lines_drawn;
		if (options.max_lines() > 0 && nr_lines_drawn >= options.max_lines())
			break;

		y += line_skip;
		start += line_len + 1;
	}

	glEnd();
	glDisable(GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void Font::draw_messages(const text_message *msgs, size_t msgs_size, int x, int y,
	Uint8 filter, size_t msg_start, size_t offset_start,
	const TextDrawOptions &options, ssize_t cursor, select_info* select) const
{
	int block_width = std::ceil(_block_width * _scale_x * options.zoom());
	int line_skip = vertical_advance(options.zoom(), options.line_spacing());
	int cursor_width = width_spacing('_', options.zoom());

	if (options.max_width() < block_width || options.max_lines() < 1)
		// no point in trying
		return;

	size_t imsg = msg_start;
	size_t ichar = offset_start;
	if (filter_messages(msgs, msgs_size, filter, msg_start, imsg, ichar))
		// nothing to draw
		return;

	unsigned char ch = msgs[imsg].data[ichar];
	unsigned char last_color_char = 0;
	if (!is_color(ch))
	{
		ssize_t i;
		// search backwards for the last color
		for (i = ssize_t(ichar) - 1; i >= 0; --i)
		{
			ch = msgs[imsg].data[i];
			if (is_color(ch))
			{
				set_color(from_color_char(ch));
				last_color_char = ch;
				break;
			}
		}

		if (i < 0 && msgs[imsg].r >= 0)
			// no color character found, try the message color
			glColor3f(msgs[imsg].r, msgs[imsg].g, msgs[imsg].b);
	}

 	glEnable(GL_ALPHA_TEST);	// enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER, 0.1f);
	bind_texture();

	int cur_x = x, cur_y = y, cur_line = 0;
	int cursor_x = x - 1, cursor_y = y - 1;
	size_t i_total = 0;
	bool in_select = false;
	glBegin(GL_QUADS);
	while (true)
	{
		ch = msgs[imsg].data[ichar];
		// watch for special characters
		if (ch == '\0')
		{
			// end of message
			if (++imsg >= msgs_size)
				imsg = 0;
			ichar = 0;
			if (imsg == msg_start
				|| filter_messages(msgs, msgs_size, filter, msg_start, imsg, ichar))
				break;

			last_color_char = 0;
		}

		if (select && select->lines && select->lines[cur_line].msg == -1)
		{
			select->lines[cur_line].msg = imsg;
			select->lines[cur_line].chr = ichar;
		}

		if (ch == '\n' || ch == '\r' || ch == '\0')
		{
			if (i_total == cursor)
			{
				// Cursor is on the newline character
				cursor_x = cur_x;
				cursor_y = cur_y;
			}

			// newline
			if (++cur_line >= options.max_lines())
				break;
			cur_y += line_skip;
			cur_x = x;
			if (ch != '\0')
				++ichar;
			++i_total;
			continue;
		}

		if (pos_selected(select, imsg, ichar))
		{
			if (!in_select)
			{
				glColor3fv(TextDrawOptions::default_selection_color.data());
				in_select = true;
			}
		}
		else if (in_select)
		{
			if (last_color_char)
				set_color(from_color_char(last_color_char));
			else if (msgs[imsg].r >= 0)
				glColor3f(msgs[imsg].r, msgs[imsg].g, msgs[imsg].b);
			else
				set_color(c_grey1);

			in_select = false;
		}

		if (is_color(ch))
			last_color_char = ch;

		int chr_width = width_spacing(ch, options.zoom());
		if (cur_x - x + chr_width <= options.max_width())
		{
			if (i_total == cursor)
			{
				cursor_x = cur_x;
				cursor_y = cur_y;
			}
			cur_x += draw_char(ch, cur_x, cur_y, options.zoom(), in_select);
			++ichar;
			++i_total;
		}
		else
		{
			// ignore rest of this line, but keep track of
			// color characters
			++ichar;
			++i_total;
			while (true)
			{
				ch = msgs[imsg].data[ichar];
				if (ch == '\0' || ch == '\n' || ch == '\r')
					break;
				if (is_color(ch))
					last_color_char = ch;
				++ichar;
				++i_total;
			}
		}
	}

	if (i_total == cursor)
	{
		if (cur_x - x + cursor_width <= options.max_width())
		{
			cursor_x = cur_x;
			cursor_y = cur_y;
		}
		else if (cur_line + 1 < options.max_lines())
		{
			cursor_x = x;
			cursor_y = cur_y + line_skip;
		}
	}

	if (cursor_x >= x && cursor_y >= y)
	{
		draw_char('_', cursor_x, cursor_y, options.zoom(), true);
	}

	glEnd();
	glDisable(GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void Font::draw_console_separator(int x_space, int y, const TextDrawOptions& options) const
{
	int pos = get_position('^');
	int char_width = width_pos(pos, options.zoom());
	int char_height = height(options.zoom());
	int dx = width_spacing_pos(pos, options.zoom());

	float u_start, u_end, v_start, v_end;
	get_texture_coordinates(pos, u_start, u_end, v_start, v_end);

	glEnable(GL_ALPHA_TEST); // enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER, 0.1f);
	bind_texture();

	glBegin(GL_QUADS);
	int x = x_space;
	while (x + char_width <= options.max_width())
	{
		glTexCoord2f(u_start, v_start); glVertex3i(x, y, 0);
		glTexCoord2f(u_start, v_end);   glVertex3i(x, y + char_height, 0);
		glTexCoord2f(u_end,   v_end);   glVertex3i(x + char_width, y + char_height, 0);
		glTexCoord2f(u_end,   v_start); glVertex3i(x + char_width, y, 0);

		x += dx;
		if (x + char_width > options.max_width())
			break;

		glTexCoord2f(u_start, v_start); glVertex3i(x, y, 0);
		glTexCoord2f(u_start, v_end);   glVertex3i(x, y + char_height, 0);
		glTexCoord2f(u_end,   v_end);   glVertex3i(x + char_width, y + char_height, 0);
		glTexCoord2f(u_end,   v_start); glVertex3i(x + char_width, y, 0);

		x += 2 * dx;
	}
	glEnd();

	glDisable(GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

#ifdef ELC
#ifndef MAP_EDITOR2
void Font::draw_ortho_ingame_string(const unsigned char* text, size_t len,
	float x, float y, float z, int max_lines, float zoom_x, float zoom_y) const
{
	int char_height = height(zoom_y);
	int line_skip = vertical_advance(zoom_y);
	float cur_x = x;
	float cur_y = y;
	int cur_line = 0;

	glEnable(GL_ALPHA_TEST); // enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER, 0.1f);
	bind_texture();
	glBegin(GL_QUADS);

	for (size_t i = 0; i < len; ++i)
	{
		unsigned char ch = text[i];
		if (ch == '\n')
		{
			cur_y += line_skip;
			cur_x = x;
			if (++cur_line >= max_lines)
				break;
		}
		else if (is_color(ch))
		{
			glEnd(); // Ooops - NV bug fix!!
			set_color(from_color_char(ch));
			glBegin(GL_QUADS);
		}
		else
		{
			int pos = get_position(ch);
			if (pos >= 0)
			{
				int char_width = width_pos(pos, zoom_x);
				float u_start, u_end, v_start, v_end;
				get_texture_coordinates(pos, u_start, u_end, v_start, v_end);

				glTexCoord2f(u_start, v_start); glVertex3f(cur_x,            cur_y+char_height, z);
				glTexCoord2f(u_start, v_end);   glVertex3f(cur_x,            cur_y,             z);
				glTexCoord2f(u_end,   v_end);   glVertex3f(cur_x+char_width, cur_y,             z);
				glTexCoord2f(u_end,   v_start); glVertex3f(cur_x+char_width, cur_y+char_height, z);

				cur_x += width_spacing_pos(pos, zoom_x);
			}
		}
	}

	glEnd();
	glDisable(GL_ALPHA_TEST);
}

void Font::draw_ingame_string(const unsigned char* text, size_t len,
	float x, float y, int max_lines, float zoom_x, float zoom_y) const
{
	float dy = zoom_y * zoom_level / 3.0;
	float cur_x = x;
	float cur_y = y;
	int cur_line = 0;

	glEnable(GL_ALPHA_TEST); // enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER,0.1f);
	bind_texture();
	glBegin(GL_QUADS);
	for (size_t i = 0; i < len; ++i)
	{
		unsigned char ch = text[i];
		if (ch == '\n')
		{
			cur_y += dy;
			cur_x = x;
			if (++cur_line >= max_lines)
				break;
		}
		else if (is_color(ch))
		{
			glEnd(); // Ooops - NV bug fix!!
			set_color(from_color_char(ch));
			glBegin(GL_QUADS); // Ooops - NV bug fix!!
		}
		else
		{
			int pos = get_position(ch);
			if (pos >= 0)
			{
				float dx = width_pos(pos, 1.0) * zoom_x * zoom_level / (3.0 * 12.0);
				float u_start, u_end, v_start, v_end;
				get_texture_coordinates(pos, u_start, u_end, v_start, v_end);

				glTexCoord2f(u_start, v_start); glVertex3f(cur_x,    0, cur_y+dy);
				glTexCoord2f(u_start, v_end);   glVertex3f(cur_x,    0, cur_y);
				glTexCoord2f(u_end,   v_end);   glVertex3f(cur_x+dx, 0, cur_y);
				glTexCoord2f(u_end, v_start);   glVertex3f(cur_x+dx, 0, cur_y+dy);

				cur_x += width_spacing_pos(pos, 1.0) * zoom_x * zoom_level / (3.0 * 12.0);
			}
		}
	}
	glEnd();
	glDisable(GL_ALPHA_TEST);
}
#endif // !MAP_EDITOR_2
#endif // ELC

#ifdef TTF
bool Font::render_glyph(size_t i_glyph, int size, int y_delta, int outline_size, TTF_Font *font,
	SDL_Surface *surface)
{
	static const Uint16 glyphs[nr_glyphs] = {
		' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-',
		'.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';',
		'<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
		'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
		'X', 'Y', 'Z', '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e',
		'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
		't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 252, 233, 226, // üéâ'
		224, 231, 234, 235, 232, 239, 244, 249, 228, 246, 252, 196, 214, 220, // àçêëèïôùäöüÄÖÜ
		223, 230, 248, 229, 198, 216, 197, 241, 209, 225, 193, 201, 237, 205, // ßæøåÆØÅñÑáÁÉíÍ
		243, 211, 250, 218, 251, 238                                          // óÓúÚûî

	};
	static const SDL_Color white = { .r = 0xff, .g = 0xff, .b = 0xff, .a = 0xff };
	static const SDL_Color black = { .r = 0x00, .g = 0x00, .b = 0x00, .a = 0x30 };

	Uint16 glyph = glyphs[i_glyph];
	if (!TTF_GlyphIsProvided(font, glyph))
	{
		LOG_ERROR("Font '%s' does not provide glyph for code point '%d': %s",
			_font_name.c_str(), glyph, TTF_GetError());
		return false;
	}

	SDL_Surface* glyph_surface;
	if (outline_size > 0)
	{
		TTF_SetFontOutline(font, outline_size);
		glyph_surface = TTF_RenderGlyph_Blended(font, glyph, black);
		if (!glyph_surface)
		{
			LOG_ERROR("Failed to render TTF glyph outline: %s", TTF_GetError());
			return false;
		}

		TTF_SetFontOutline(font, 0);
		SDL_Surface *fg_surface = TTF_RenderGlyph_Blended(font, glyph, white);
		if (!fg_surface)
		{
			LOG_ERROR("Failed to render TTF glyph: %s", TTF_GetError());
			return false;
		}

		SDL_Rect rect = {outline_size, outline_size, fg_surface->w, fg_surface->h};
		SDL_SetSurfaceBlendMode(glyph_surface, SDL_BLENDMODE_BLEND);
		SDL_BlitSurface(fg_surface, NULL, glyph_surface, &rect);
		SDL_FreeSurface(fg_surface);
	}
	else
	{
		glyph_surface = TTF_RenderGlyph_Solid(font, glyph, white);
		if (!glyph_surface)
		{
			LOG_ERROR("Failed to render TTF glyph: %s", TTF_GetError());
			return false;
		}
	}

	int width = glyph_surface->w;
	int height = glyph_surface->h;

	int row = i_glyph / font_chars_per_line;
	int col = i_glyph % font_chars_per_line;

	SDL_Rect area;
	area.x = col*size;
	area.y = row*(size+2) + 1 + y_delta - outline_size;
	area.w = width;
	area.h = height;

	SDL_SetSurfaceAlphaMod(glyph_surface, 0xFF);
	SDL_SetSurfaceBlendMode(glyph_surface, SDL_BLENDMODE_NONE);
	int err = SDL_BlitSurface(glyph_surface, NULL, surface, &area);
	SDL_FreeSurface(glyph_surface);
	if (err)
	{
		LOG_ERROR("Failed to write glyph to surface: %s", SDL_GetError());
		return false;
	}

	int y_min, y_max;
	_metrics[i_glyph].width = width;
	TTF_GlyphMetrics(font, glyph, nullptr, nullptr, &y_min, &y_max, &_metrics[i_glyph].advance);
	_metrics[i_glyph].top = y_delta + TTF_FontAscent(font) - y_max;
	_metrics[i_glyph].bottom = y_delta + TTF_FontAscent(font) - y_min;
	_metrics[i_glyph].u_start = float(col * size) / surface->w;
	_metrics[i_glyph].v_start = float(row * (size+2) + 1) / surface->h;
	_metrics[i_glyph].u_end = float(col * size + width) / surface->w;
	_metrics[i_glyph].v_end = float(row * (size+2) + size + 1) / surface->h;

	return true;
}

int Font::find_point_size(int height)
{
	int min = 0, max = 2 * height;
	while (max > min + 1)
	{
		int mid = (min + max) / 2;
		TTF_Font *font = open_font(_file_name.c_str(), mid);
		if (!font)
			return 0;

		int size = TTF_FontLineSkip(font);
		TTF_CloseFont(font);

		if (size == height)
			return mid;
		else if (size < height)
			min = mid;
		else
			max = mid;
	}
	return max;
}

bool Font::build_texture_atlas()
{
	static const int nr_rows = (nr_glyphs + font_chars_per_line-1) / font_chars_per_line;

	if (!is_ttf())
	{
		LOG_ERROR("Font '%s' is not a TrueType font, no need to build a texture",
			_font_name.c_str());
		return false;
	}

	// First try to interpret ttf_file_name as a path relative to ttf_directory. If that fails,
	// try as an absolute path.
	int point_size = _point_size ? _point_size : ttf_point_size;
	TTF_Font *font = open_font(_file_name.c_str(), point_size);
	if (!font)
	{
		LOG_ERROR("Failed to open TrueType font %s: %s", _file_name.c_str(), TTF_GetError());
		_flags |= Flags::FAILED;
		return false;
	}

	// SDL_ttf versions < 2.0.15 don't take transparency into account, so don't draw shadows
	// if the run-time version of this library version is too old.
	const SDL_version *link_version = TTF_Linked_Version();
	bool draw_shadow = link_version->major > 2
		|| (link_version->major == 2 && link_version->minor > 0)
		|| (link_version->major == 2 && link_version->minor == 0 && link_version->patch >= 15);
	int outline_size = draw_shadow ? _outline : 0;

	int size = TTF_FontLineSkip(font);
	int width = next_power_of_two(font_chars_per_line * size);
	// Keep two rows of empty pixels (one top, one bottom) around the glyphs for systems with an
	// alternative view on texture coordinates (i.e. off by one pixel). Perhaps they may lose a
	// single pixel row of a really high character, but at least we won't draw part of the character
	// bwlow or above it.
	int height = next_power_of_two(nr_rows * (size + 2));
	SDL_Surface *image = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000
#else
		0xFF000000,
		0x00FF0000,
		0x0000FF00,
		0x000000FF
#endif
	);
	if (!image)
	{
		LOG_ERROR("Failed to create surface for TTF texture atlas: %s", TTF_GetError());
		TTF_CloseFont(font);
		_flags |= Flags::FAILED;
		return false;
	}

	int y_delta = (size - TTF_FontHeight(font)) / 2;
	for (size_t i_glyph = 0; i_glyph < nr_glyphs; ++i_glyph)
	{
		if (!render_glyph(i_glyph, size, y_delta, outline_size, font, image))
		{
			SDL_FreeSurface(image);
			TTF_CloseFont(font);
			_flags |= Flags::FAILED;
			return false;
		}
	}

	GLuint texture_id;
	glGenTextures(1, &texture_id);
	::bind_texture_id(texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		image->pixels);

	SDL_FreeSurface(image);
	_texture_id.gl_id = texture_id;
	_texture_width = width;
	_texture_height = height;
	_block_width = size;
	_line_height = _vertical_advance = size;
	_font_top_offset = y_delta;
	int digit_top = std::min_element(_metrics.begin() + 16, _metrics.begin() + 26,
		[](const Metrics& m0, const Metrics& m1) { return m0.top < m1.top; })->top;
	int digit_bottom = std::max_element(_metrics.begin() + 16, _metrics.begin() + 26,
		[](const Metrics& m0, const Metrics& m1) { return m0.bottom < m1.bottom; })->bottom;
	_digit_center_offset = (digit_top + digit_bottom) / 2;
	int pos = get_position('*');
	_password_center_offset = (_metrics[pos].top + _metrics[pos].bottom) / 2;
	_max_advance = std::max_element(_metrics.begin(), _metrics.end(),
		[](const Metrics& m0, const Metrics& m1) { return m0.advance < m1.advance; })->advance;
	_max_digit_advance = std::max_element(_metrics.begin() + 16, _metrics.begin() + 26,
		[](const Metrics& m0, const Metrics& m1) { return m0.advance < m1.advance; })->advance;
	_avg_advance = calc_average_advance();
	_scale_x = _scale_y = float(font_block_height - 2) / size;
	_flags |= Flags::HAS_TEXTURE;

	TTF_CloseFont(font);

	return true;
}

#endif // TTF

int Font::calc_average_advance()
{
	int total = std::accumulate(letter_freqs.begin(), letter_freqs.end(), 0);
	int dot = std::inner_product(letter_freqs.begin(), letter_freqs.end(), _metrics.begin(), 0,
		std::plus<int>(), [](int f, const Metrics& m) { return f * m.advance; });
	return std::round(float(dot) / total);
}

const std::array<size_t, NR_FONT_CATS> FontManager::_default_font_idxs
	= { 0, 0, 0, 2, 0, 3, 0, 0 };
std::array<size_t, NR_FONT_CATS> FontManager::font_idxs = { 0, 0, 0, 2, 0, 3, 0, 0, 0 };
std::array<float, NR_FONT_CATS> FontManager::font_scales
	= { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.3, 1.0 };

bool FontManager::initialize()
{
    _options.clear();
	_fonts.clear();

	for (size_t i = 0; i < _nr_bundled_fonts; ++i)
	{
		FontOption option(i);
		if (!option.failed())
			_options.push_back(std::move(option));
	}
	if (_options.empty() || _options[0].failed())
		return false;
#ifdef TTF
	initialize_ttf();
#endif
	add_select_options();

	_fonts.insert(std::make_pair(0, Font(_options[0])));

	return true;
}

#ifdef TTF
void FontManager::initialize_ttf()
{
	static const char* patterns[] = { "*.ttf", "*.ttc", "*.otf" };

	if (!use_ttf)
		return;

	if (!TTF_WasInit() && TTF_Init() == -1)
	{
		LOG_ERROR("Failed to initialize True Type fonts: %s", TTF_GetError());
		use_ttf = 0;
		return;
	}

	size_t nr_existing_fonts = _options.size();
	for (const char* pattern: patterns)
	{
		search_files_and_apply(ttf_directory, pattern,
			[](const char *fname_ptr) {
				size_t dir_name_len = strlen(ttf_directory);
				std::string fname;
				if (!strncmp(fname_ptr, ttf_directory, dir_name_len))
					fname = fname_ptr + dir_name_len;
				else
					fname = fname_ptr;
				FontOption option(fname);
				if (!option.failed())
					FontManager::get_instance()._options.push_back(std::move(option));
			}, 1);
	}

	// Sort TTF fonts by font name, but keep them after EL bundled fonts
	std::sort(_options.begin() + nr_existing_fonts, _options.end(),
		[](const FontOption& f0, const FontOption& f1) { return f0.font_name() < f1.font_name(); });
}
#endif // TTF

void FontManager::add_select_options(bool add_button)
{
	clear_multiselect_var("ui_font");
	clear_multiselect_var("chat_font");
	clear_multiselect_var("name_font");
	clear_multiselect_var("book_font");
	clear_multiselect_var("note_font");
	clear_multiselect_var("rules_font");
	clear_multiselect_var("encyclopedia_font");

	_fixed_width_idxs.clear();
	for (size_t i = 0; i < _options.size(); ++i)
	{
		const FontOption& option = _options[i];
		if (option.is_fixed_width())
			_fixed_width_idxs.push_back(i);
		option.add_select_options(add_button);
	}

	if (add_button)
	{
		set_multiselect_var("ui_font", font_idxs[UI_FONT], true);
		set_multiselect_var("chat_font", font_idxs[CHAT_FONT], true);
		set_multiselect_var("name_font", font_idxs[NAME_FONT], true);
		set_multiselect_var("book_font", font_idxs[BOOK_FONT], true);
		set_multiselect_var("note_font", font_idxs[NOTE_FONT], true);
		set_multiselect_var("rules_font", font_idxs[RULES_FONT], true);
		auto it = std::find(_fixed_width_idxs.begin(), _fixed_width_idxs.end(),
			font_idxs[ENCYCLOPEDIA_FONT]);
		if (it != _fixed_width_idxs.end())
			set_multiselect_var("encyclopedia_font", it - _fixed_width_idxs.begin(), true);
	}
}

Font& FontManager::get(Category cat, float text_zoom)
{
	size_t idx = font_idxs[cat];
	if (idx > _options.size())
	{
#ifdef TTF
		// Invalid font number
		if (cat == CONFIG_FONT)
		{
			// Probably TTF was disabled, and the settings window was using a TTF font. Check if
			// it is still available
			int height = std::round((Font::font_block_height - 2) * text_zoom * font_scales[cat]);
			uint32_t key = ((height & 0xffff) << 16) | 0xffff;
			auto it = _fonts.find(key);
			if (it != _fonts.end())
				return it->second;
			else
{
printf("huh. not found\n");
				font_idxs[cat] = idx = 0;
}
		}
		else
		{
			font_idxs[cat] = idx = 0;
		}
#else // TTF
		font_idxs[cat] = idx = 0;
#endif // TTF
	}

#ifdef TTF
	// Bundled fonts come in only one size, so always use 0 for the line height and rely on scaling
	// to display the correct size.
	int height = _options[idx].is_ttf()
		? std::round((Font::font_block_height - 2) * text_zoom * font_scales[cat])
		: 0;
	// It is unlikely that there will be more than 64k fonts, or that the line height wil be more
	// than 64k pixels, so combine the two values into a single key 32-bit key.
	uint32_t key = ((height & 0xffff) << 16) | (idx & 0xffff);
	auto it = _fonts.find(key);
	if (it == _fonts.end())
	{
		// The font has not been loaded yet
		Font font = _options[idx].is_ttf() ? Font(_options[idx], height) : Font(_options[idx]);
		it = _fonts.insert(std::make_pair(key, std::move(font))).first;
	}
#else // TTF
	uint32_t key = idx;
	auto it = _fonts.find(idx);
	if (it == _fonts.end())
	{
		// The font has not been loaded yet
		Font font = Font(_options[idx]);
		it = _fonts.insert(std::make_pair(key, font)).first;
	}
#endif // TTF

	Font *result = &it->second;
	if (result->failed())
	{
		// Failed to load previously, switch to fixed
		font_idxs[cat] = 0;
		return _fonts.at(0);
	}

	if (!result->has_texture() && !result->load_texture())
	{
		// Failed to load or generate texture, switch to fixed
		font_idxs[cat] = 0;
		return _fonts.at(0);
	}

	return *result;
}

#ifdef TTF
void FontManager::disable_ttf()
{
	// If not yet initialized, do nothing
	if (!is_initialized())
		return;

	size_t config_font_idx = font_idxs[CONFIG_FONT];

	// Save the file names of the fonts currently in use
	for (size_t i = 0; i < NR_FONT_CATS; ++i)
	{
		size_t font_num = font_idxs[i];
		if (font_num >= _options.size())
			// Invalid index, possibly configuration font after disabling TTF previously
			font_num = 0;

		_saved_font_files[i] = _options[font_num].file_name();
		if (_options[font_num].is_ttf())
		{
			// Revert to default bundled font
			font_idxs[i] = (i == CONFIG_FONT ? 0xffff : _default_font_idxs[i]);
		}
	}

	// Remove the font options
	auto first_ttf = std::find_if(_options.begin(), _options.end(),
		[](const FontOption& opt) { return opt.is_ttf(); });
	_options.erase(first_ttf, _options.end());

	// Remove the fonts themselves
	std::unordered_map<uint32_t, Font> config_fonts;
	for (auto it = _fonts.begin(); it != _fonts.end(); )
	{
		if (it->second.is_ttf())
		{
			if ((it->first & 0xffff) == config_font_idx)
			{
				config_fonts.insert(std::make_pair((it->first & 0xffff0000) | 0xffff, std::move(it->second)));
			}
			it = _fonts.erase(it);
		}
		else
		{
			++it;
		}
	}
	// Reinstate config font with invalid index
	for (auto it = config_fonts.begin(); it != config_fonts.end(); ++it)
		_fonts.insert(std::make_pair(it->first, std::move(it->second)));

	// Remove the TTF font options from the selection
	add_select_options(true);
}

void FontManager::enable_ttf()
{
	// If not yet initialized, do nothing
	if (!is_initialized())
		return;

	// Add TrueType fonts to the font list
	initialize_ttf();

	// Check if we saved font file names when we disabled TTF, and if so,
	// restore them. Only restore TTF fonts, keeping any changes in bundled
	// fonts.
	for (size_t i = 0; i < NR_FONT_CATS; ++i)
	{
		// Font for the settings window should not change, it will be updated and set to the
		// UI font when the window is closed and reopened
		if (i == CONFIG_FONT)
			continue;

		const std::string& fname = _saved_font_files[i];
		if (!fname.empty())
		{
			std::vector<FontOption>::const_iterator it = std::find_if(_options.begin(), _options.end(),
				[fname](const FontOption& opt) { return opt.file_name() == fname; });
			if (it != _options.end() && it->is_ttf())
				font_idxs[i] = it - _options.begin();
		}
	}

	// Add selection options for the new fonts
	add_select_options(true);
}
#endif // TTF

} // namespace eternal_lands


extern "C"
{

using namespace eternal_lands;

size_t *font_idxs = FontManager::font_idxs.data();
float *font_scales = FontManager::font_scales.data();
#ifdef TTF
int use_ttf = 0;
#ifdef LINUX
char ttf_directory[TTF_DIR_SIZE] = "/usr/share/fonts/TTF";
#elif defined WINDOWS
char ttf_directory[TTF_DIR_SIZE] = "C:/Windows/Fonts";
#else
char ttf_directory[TTF_DIR_SIZE];
#endif //
#endif // TTF

int initialize_fonts()
{
	return FontManager::get_instance().initialize();
}

size_t get_fixed_width_font_number(size_t idx)
{
	return FontManager::get_instance().fixed_width_font_number(idx);
}

int get_char_width_zoom(unsigned char c, font_cat cat, float zoom)
{
	return FontManager::get_instance().width_spacing(cat, c, zoom);
}
int get_max_char_width_zoom(font_cat cat, float zoom)
{
	return FontManager::get_instance().max_width_spacing(cat, zoom);
}
int get_avg_char_width_zoom(font_cat cat, float zoom)
{
	return FontManager::get_instance().average_width_spacing(cat, zoom);
}
int get_max_digit_width_zoom(font_cat cat, float zoom)
{
	return FontManager::get_instance().max_digit_width_spacing(cat, zoom);
}
int get_buf_width_zoom(const unsigned char* str, size_t len, font_cat cat, float zoom)
{
	return FontManager::get_instance().line_width(static_cast<FontManager::Category>(cat),
		str, len, zoom);
}
int get_line_height(font_cat cat, float text_zoom)
{
	return FontManager::get_instance().line_height(cat, text_zoom);
}
int get_line_skip(font_cat cat, float text_zoom)
{
	return FontManager::get_instance().vertical_advance(cat, text_zoom);
}
int get_max_nr_lines(int height, font_cat cat, float zoom)
{
	return FontManager::get_instance().max_nr_lines(cat, height, zoom);
}
int get_text_height(int nr_lines, font_cat cat, float zoom)
{
	return FontManager::get_instance().text_height(cat, nr_lines, zoom);
}
void get_buf_dimensions(const unsigned char* str, size_t len, font_cat cat, float text_zoom,
	int *width, int *height)
{
	auto dims = FontManager::get_instance().dimensions(cat, str, len, text_zoom);
	*width = dims.first;
	*height = dims.second;
}

int get_center_offset(const unsigned char* text, size_t len, font_cat cat, float zoom)
{
	return FontManager::get_instance().center_offset(cat, text, len, zoom);
}

void get_top_bottom(const unsigned char* text, size_t len, font_cat cat, float zoom,
	int *top, int *bottom)
{
	auto coords = FontManager::get_instance().top_bottom(cat, text, len, zoom);
	*top = coords.first;
	*bottom = coords.second;
}

int reset_soft_breaks(unsigned char *text, int len, int size, font_cat cat,
	float text_zoom, int width, int *cursor_ptr, float *max_line_width)
{
	int cursor = cursor_ptr ? *cursor_ptr : -1;
	TextDrawOptions options = TextDrawOptions().set_max_width(width).set_zoom(text_zoom);
	ustring wrapped_text;
	int nr_lines, cursor_diff;
	std::tie(wrapped_text, nr_lines, cursor_diff) = FontManager::get_instance()
		.reset_soft_breaks(cat, text, len, options, cursor, max_line_width);

	size_t new_len = wrapped_text.copy(text, size_t(size)-1);
	text[new_len] = '\0';
	if (cursor_ptr)
		*cursor_ptr = std::min(cursor + cursor_diff, size - 1);

	return nr_lines;
}
void put_small_colored_text_in_box_zoomed(int color,
	const unsigned char* text, int len, int width,
	unsigned char* buffer, float text_zoom)
{
	TextDrawOptions options = TextDrawOptions().set_max_width(width)
		.set_zoom(text_zoom * DEFAULT_SMALL_RATIO);
	ustring wrapped_text;
	int nr_lines, cursor_diff;
	std::tie(wrapped_text, nr_lines, cursor_diff) = FontManager::get_instance()
		.reset_soft_breaks(FontManager::Category::UI_FONT, text, len, options, -1, nullptr);
	// If we don't end on a newline, add one.
	// TODO: check if that's necessary.
	if (wrapped_text.back() != '\n')
		wrapped_text.push_back('\n');

	size_t new_len = 0;
	if (!is_color(wrapped_text.front()))
		buffer[new_len++] = to_color_char(color);
	// FIXME: no size specified for either buffer. Pass unlimited size,
	// and hope for the best...
	new_len += wrapped_text.copy(buffer + new_len, ustring::npos);
	buffer[new_len] = '\0';
}

void vdraw_text(int x, int y, const unsigned char* text, size_t len, font_cat cat, va_list options)
{
	TextDrawOptions tdo;
	text_draw_option_sel sel;
	int sel_begin = 0, sel_end = 0;
	bool end_reached = false;
	do
	{
		sel = text_draw_option_sel(va_arg(options, int));
		switch (sel)
		{
			case TDO_MAX_WIDTH:
				tdo.set_max_width(va_arg(options, int));
				break;
			case TDO_MAX_LINES:
				tdo.set_max_lines(va_arg(options, int));
				break;
			case TDO_ZOOM:
				tdo.set_zoom(va_arg(options, double));
				break;
			case TDO_LINE_SPACING:
				tdo.set_line_spacing(va_arg(options, double));
				break;
			case TDO_ALIGNMENT:
				tdo.set_alignment(TextDrawOptions::Alignment(va_arg(options, int)));
				break;
			case TDO_VERTICAL_ALIGNMENT:
				tdo.set_vertical_alignment(TextDrawOptions::VerticalAlignment(va_arg(options, int)));
				break;
			case TDO_SHADOW:
				tdo.set_shadow(va_arg(options, int));
				break;
			case TDO_FOREGROUND:
			{
				float r = va_arg(options, double);
				float g = va_arg(options, double);
				float b = va_arg(options, double);
				tdo.set_foreground(r, g, b);
				break;
			}
			case TDO_BACKGROUND:
			{
				float r = va_arg(options, double);
				float g = va_arg(options, double);
				float b = va_arg(options, double);
				tdo.set_background(r, g, b);
				break;
			}
			case TDO_SELECTION:
			{
				float r = va_arg(options, double);
				float g = va_arg(options, double);
				float b = va_arg(options, double);
				tdo.set_selection(r, g, b);
				break;
			}
			case TDO_IGNORE_COLOR:
				tdo.set_ignore_color(va_arg(options, int));
				break;
			case TDO_HELP:
				tdo.set_help(va_arg(options, int));
				break;
			case TDO_ELLIPSIS:
				tdo.set_ellipsis(va_arg(options, int));
				break;
			case TDO_SHRINK_TO_FIT:
				tdo.set_shrink_to_fit(va_arg(options, int));
				break;
			case TDO_SEL_BEGIN:
				sel_begin = va_arg(options, int);
				break;
			case TDO_SEL_END:
				sel_end = va_arg(options, int);
				break;
			case TDO_END:
				end_reached = true;
				break;
			default:
				LOG_ERROR("Unknown text draw option selector %d", sel);
				end_reached = true;
				break;
		}
	} while (!end_reached);

	FontManager::get_instance().draw(cat, text, len, x, y, tdo, sel_begin, sel_end);
}

void draw_string_zoomed_centered_around(int x, int y,
	const unsigned char *text, int center_idx, float text_zoom)
{
	size_t len = strlen(reinterpret_cast<const char*>(text));
	size_t idx = center_idx;
	TextDrawOptions options = TextDrawOptions().set_zoom(text_zoom)
		.set_alignment(TextDrawOptions::Alignment::RIGHT);
	FontManager::get_instance().draw(FontManager::Category::UI_FONT, text,
		std::min(len, idx), x, y, options);
	if (idx < len)
	{
		options.set_alignment(TextDrawOptions::Alignment::LEFT);
		FontManager::get_instance().draw(FontManager::Category::UI_FONT, text + idx,
			len - idx, x, y, options);
	}
}

void draw_messages(int x, int y, text_message *msgs, int msgs_size, Uint8 filter,
	int msg_start, int offset_start, int cursor, int width, int height,
	font_cat cat, float text_zoom, select_info* select)
{
	int max_nr_lines = FontManager::get_instance().max_nr_lines(cat, height, text_zoom);
	TextDrawOptions options = TextDrawOptions().set_max_width(width)
		.set_max_lines(max_nr_lines)
		.set_zoom(text_zoom);
	FontManager::get_instance().draw_messages(cat, msgs, msgs_size, x, y, filter,
		msg_start, offset_start, options, cursor, select);
}

void draw_console_separator(int x_space, int y, int width, float text_zoom)
{
	TextDrawOptions options = TextDrawOptions().set_max_width(width).set_zoom(text_zoom);
	FontManager::get_instance().draw_console_separator(CHAT_FONT, x_space, y, options);
}

#ifdef ELC
#ifndef MAP_EDITOR2
void draw_ortho_ingame_string(float x, float y, float z,
	const unsigned char *text, int max_lines, float zoom_x, float zoom_y)
{
	FontManager::get_instance().draw_ortho_ingame_string(text,
		strlen(reinterpret_cast<const char*>(text)),
		x, y, z, max_lines, zoom_x, zoom_y);
}
void draw_ingame_string(float x, float y, const unsigned char *text,
	int max_lines, float zoom_x, float zoom_y)
{
	FontManager::get_instance().draw_ingame_string(text,
			strlen(reinterpret_cast<const char*>(text)),
			x, y, max_lines, zoom_x, zoom_y);
}
#endif // !MAP_EDITOR2
#endif // ELC

void set_config_font(void)
{
	FontManager::get_instance().set_config_font();
}

int has_glyph(unsigned char c)
{
	return Font::has_glyph(c);
}

#ifdef TTF
void disable_ttf(void)
{
	FontManager::get_instance().disable_ttf();
}

void enable_ttf(void)
{
	FontManager::get_instance().enable_ttf();
}
#endif // TTF

} //extern "C"

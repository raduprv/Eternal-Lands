#include <cmath>
#include <sstream>
#ifdef TTF
#include <glob.h>
#include <SDL_ttf.h>
#endif

#include "font.h"
#include "asc.h"
#include "chat.h"
#include "colors.h"
#include "elconfig.h"
#include "elloggingwrapper.h"
#include "exceptions/extendedexception.hpp"
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

} // namespace

namespace eternal_lands
{

size_t FontManager::font_idxs[NR_FONT_CATS] = { 0, 0, 0, 2, 0, 3 };
float FontManager::font_scales[NR_FONT_CATS] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

TextDrawOptions::TextDrawOptions(): _max_width(window_width), _max_lines(0),
	_zoom(1.0), _line_spacing(1.0), _alignment(LEFT), _shadow(false),
	_fg_r(-1.0), _fg_g(-1.0), _fg_b(-1.0), _bg_r(-1.0), _bg_g(-1.0), _bg_b(-1.0),
	_ignore_color(false) {}

void TextDrawOptions::use_background_color() const
{
	if (has_background_color())
		glColor3f(_bg_r, _bg_g, _bg_b);
	else
		glColor3f(0.0, 0.0, 0.0);
}

void TextDrawOptions::use_foreground_color() const
{
	if (has_foreground_color())
		glColor3f(_fg_r, _fg_g, _fg_b);
	else
		glColor3f(1.0, 1.0, 1.0);
}


Font::Font(size_t i): _font_name(), _file_name(), _flags(0),
	_texture_width(256), _texture_height(256),
	_char_widths(), _texture_coordinates(),
	_block_width(font_block_width), _block_height(font_block_height),
	_line_height(std::round(12.0 * default_line_height / 11)), _spacing(0),
	_scale(11.0 / 12)
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
	if (i > file_names.size())
	{
		// Invalid number. Set font name so that the button can be shown, but
		// set FAILED flag so that the font is never actually used.
		os << "Type " << (i + 1);
		_font_name = os.str();
		_flags |= Flags::FAILED;

		LOG_ERROR("Invalid font number %zu", i);
	}
	else if (i == 0)
	{
		_file_name = file_names[i];
		_font_name = "Type 1 (fixed)";
	}
	else
	{
		_file_name = file_names[i];
		size_t begin = _file_name.find_last_of('/') + 1;
		size_t end = _file_name.find_last_of('.');
		os << "Type " << i << " - " << _file_name.substr(begin, end);
		_font_name = os.str();
	}

	if (!el_file_exists(_file_name.c_str()))
	{
		LOG_ERROR("Unable to find font file '%s'", _file_name.c_str());
		_flags |= Flags::FAILED;
	}

	if (i == 1)
	{
		_char_widths = { {
			 4,  2,  7, 11,  8, 12, 12,  2,  7,  7,  9, 10,  3,  8,
			 2, 10, 10, 10,  8,  8, 10,  7,  9,  9,  9,  9,  3,  3,
			10, 10, 10,  9, 12, 12,  9, 10, 10,  9,  9, 10,  9,  8,
			 7, 11,  8, 11, 10, 11,  9, 11, 11,  9, 10,  9, 12, 12,
			12, 12, 10,  6, 10,  6, 10, 12,  3, 11,  9,  9,  9,  9,
			 8,  9,  9,  4,  6, 10,  4, 11,  9, 10,  9,  9,  8,  8,
			 8,  9, 10, 12, 10, 10,  9,  8,  2,  8, 10,  8, 12, 12,
			12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
			12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
			12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
		} };
		_spacing = 4;
	}
	else if (i == 2)
	{
		_char_widths = { {
			 8,  8,  8, 10,  8, 10, 10,  8,  8,  8,  8, 10,  8,  8,
			 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
			10, 10, 10,  8, 12, 10, 10, 10, 10, 10, 10, 10, 10, 10,
			10, 10, 10, 10, 10, 10, 10, 10,  8, 10, 10, 10, 10, 10,
			10, 10, 10, 10, 10, 10, 10,  8,  8,  8,  8,  8,  8,  8,
			10,  8,  8,  8,  8,  8,  8, 10,  8,  8,  8,  8,  8,  8,
			 8,  8,  8, 10,  8,  8,  8, 10,  8, 10, 10,  8, 10,  8,
			 8,  8, 10, 10, 10,  8, 10, 10,  8,  8,  8, 12, 12, 12,
			10, 10, 12, 10, 12, 12, 12,
		} };
		_spacing = 2;
	}
	else
	{
		std::fill(_char_widths.begin(), _char_widths.end(), 12);
	}

	for (size_t pos = 0; pos < font_nr_lines * font_chars_per_line; ++pos)
	{
		int row = pos / font_chars_per_line;
		int col = pos % font_chars_per_line;

		int cw = _char_widths[pos] + _spacing;
		int skip = (12 - cw) / 2;

		_texture_coordinates[pos][0] = float(col * font_block_width + skip) / 256;
		_texture_coordinates[pos][1] = float(row * font_block_height + 1) / 256;
		_texture_coordinates[pos][2] = float((col+1) * font_block_width - 7 - skip) / 256;
		_texture_coordinates[pos][3] = float((row+1) * font_block_height - 1) / 256;
	}
}

#ifdef TTF
Font::Font(const std::string& ttf_file_name): _font_name(), _file_name(), _flags(0),
	_texture_width(0), _texture_height(0),
	_char_widths(), _texture_coordinates(),
	_block_width(0), _block_height(0), _spacing(0), _scale(1.0)
{
	TTF_Font *font = TTF_OpenFont(ttf_file_name.c_str(), ttf_point_size);
	if (!font)
	{
		EXTENDED_EXCEPTION(ExtendedException::ec_file_not_found,
			"Failed to open TTF font file '" << ttf_file_name << "'");
	}

	// Quick check to see if the font is useful
	if (!TTF_GlyphIsProvided(font, ' '))
	{
		// Nope, can't render in this font
		EXTENDED_EXCEPTION(ExtendedException::ec_item_not_found,
			"Unable to render text with TTF font file '" << ttf_file_name << "'");
	}

	std::string name = TTF_FontFaceFamilyName(font);
	std::string style = TTF_FontFaceStyleName(font);

	TTF_CloseFont(font);

	_font_name = name + ' ' + style;
	_file_name = ttf_file_name;
	_flags |= Flags::IS_TTF;
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
	return std::round(_char_widths[pos] * _scale * zoom);
}

int Font::width_spacing_pos(int pos, float zoom) const
{
	if (pos < 0)
		return 0;
	// return width of character + spacing between chars (supports variable width fonts)
	return std::round((_char_widths[pos] + _spacing) * _scale * zoom);
}

int Font::height(float zoom) const
{
	return std::round(_line_height * _scale * zoom);
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

std::pair<int, int> Font::dimensions(const unsigned char* str, size_t len, float zoom) const
{
	int line_height = height(zoom);
	int w = 0, h = 0;
	size_t off = 0;
	while (off < len)
	{
		size_t end = memcspn(str + off, len - off,
			reinterpret_cast<const unsigned char*>("\r\n"), 2);
		w = std::max(w, line_width(str + off, end, zoom));
		h += line_height;
		off += end + 1;
	}
	return std::make_pair(w, h);
}

std::pair<ustring, int> Font::reset_soft_breaks(const unsigned char *text,
	size_t text_size, size_t text_len, float zoom, int max_width, int *cursor,
	float *max_line_width)
{
	int block_width = std::ceil(_block_width * _scale * zoom);
	if (!text || text_size == 0 || max_width < block_width)
		return std::make_pair(ustring(), 0);

	std::basic_string<unsigned char> wrapped_text;
	size_t start = 0, end, last_space = 0;
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
			if (c == '\n')
			{
				++end;
				break;
			}

			if (c == ' ')
				last_space = end;

			// Check here if the character fits using block_width instead of the
			// actual glyph width. This is consistent with draw_messages(),
			// which can't easily check the actual character width because it
			// also has to take the cursor into account. Perhaps this can at
			// some point be improved, but for now, rather be pessimistic and
			// don't lose the last character in the line.
			if (cur_width + block_width <= max_width)
			{
				cur_width += width_spacing(c, zoom);
			}
			else
			{
				// Character won't fit. Split line after the last space.
				// If not found, break in the middle of the word.
				if (last_space > start)
					end = last_space + 1;
				break;
			}
		}

		for (size_t i = start; i < end; ++i)
		{
			if (text[i] == '\r')
			{
				if (cursor && i < size_t(*cursor))
					--*cursor;
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
			if (cursor && end <= size_t(*cursor))
				++diff_cursor;
		}

		if (max_line_width)
		{
			cur_width = line_width(text + start, end - start, zoom);
			*max_line_width = std::max(*max_line_width, float(cur_width));
		}

		start = end;
	}

	if (cursor)
	{
		*cursor = std::min(*cursor + diff_cursor, int(text_size) - 1);
	}

	return std::make_pair(wrapped_text, nr_lines);
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
	u_start = _texture_coordinates[pos][0];
	v_start = _texture_coordinates[pos][1];
	u_end   = _texture_coordinates[pos][2];
	v_end   = _texture_coordinates[pos][3];
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

	int char_width = width_spacing_pos(pos, zoom);
// if (_font_name.substr(0,6) == "Type 2") printf("%c: %d+%d %d @ %f\n", c, _char_widths[pos], _spacing, char_width, zoom);
	int char_height = height(zoom);

	float u_start, u_end, v_start, v_end;
	get_texture_coordinates(pos, u_start, u_end, v_start, v_end);

	// and place the text from the graphics on the map
	glTexCoord2f(u_start, v_start); glVertex3i(x, y, 0);
	glTexCoord2f(u_start, v_end);   glVertex3i(x, y + char_height, 0);
	glTexCoord2f(u_end,   v_end);   glVertex3i(x + char_width, y + char_height, 0);
	glTexCoord2f(u_end,   v_start); glVertex3i(x + char_width, y, 0);

	return char_width;
}

void Font::draw_line(const unsigned char* text, size_t len, int x, int y,
	const TextDrawOptions &options) const
{
	if (options.shadow())
	{
		TextDrawOptions new_options = options;
		new_options.set_shadow(false).set_ignore_color();

		new_options.use_background_color();
		draw_line(text, len, x-1, y-1, new_options);
		draw_line(text, len, x-1, y,   new_options);
		draw_line(text, len, x-1, y+1, new_options);
		draw_line(text, len, x,   y+1, new_options);
		draw_line(text, len, x+1, y+1, new_options);
		draw_line(text, len, x+1, y,   new_options);
		draw_line(text, len, x+1, y-1, new_options);
		draw_line(text, len, x,   y-1, new_options);

		new_options.set_ignore_color(false);
		draw_line(text, len, x,   y,   new_options);
	}

	if (!options.ignore_color() && options.has_foreground_color())
		options.use_foreground_color();

	int cur_x = x;
	for (size_t i = 0; i < len; ++i)
	{
		cur_x += draw_char(text[i], cur_x, y, options.zoom(), options.ignore_color());
	}
}

std::pair<size_t, size_t> Font::clip_line(const unsigned char *text, size_t len,
	const TextDrawOptions &options, unsigned char &before_color, unsigned char &after_color,
	int &width) const
{
	before_color = 0;
	after_color = 0;
	width = line_width_spacing(text, len, options.zoom());

	if (width <= options.max_width())
		return std::make_pair(0, len);

	size_t start = 0, end = len;
	switch (options.alignment())
	{
		case TextDrawOptions::Alignment::LEFT:
		{
			while (end > 0 && width > options.max_width())
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
			while (start < len && width > options.max_width())
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
			size_t start = 0, end = len;
			while (start < end && width - d_left - d_right > options.max_width())
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

	return std::make_pair(start, end);
}

void Font::draw(const unsigned char* text, size_t len, int x, int y,
	const TextDrawOptions &options) const
{
	if (options.max_width() < std::ceil(_block_width * _scale * options.zoom()))
		// There's really no point in trying
		return;

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glEnable(GL_ALPHA_TEST); // enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER, 0.1f);
	bind_texture();

	size_t start = 0;
	int line_width, line_height = height(options.zoom() * options.line_spacing());
	int nr_lines = 0;
	unsigned char before_color, after_color;

	glBegin(GL_QUADS);
	while (start < len)
	{
		size_t line_len = memcspn(text + start, len-start,
			reinterpret_cast<const unsigned char*>("\r\n"), 2);
		std::pair<size_t, size_t> range = clip_line(text + start, line_len, options,
			before_color, after_color, line_width);

		const unsigned char* line = text + start + range.first;
		size_t clipped_line_len = range.second - range.first;

		if (before_color)
			set_color(from_color_char(before_color));
		switch (options.alignment())
		{
			case TextDrawOptions::Alignment::LEFT:
				draw_line(line, clipped_line_len, x, y, options);
				break;
			case TextDrawOptions::Alignment::CENTER:
				draw_line(line, clipped_line_len, x - line_width/2, y, options);
				break;
			case TextDrawOptions::Alignment::RIGHT:
				draw_line(line, clipped_line_len, x - line_width, y, options);
				break;
		}
		if (after_color)
			set_color(from_color_char(after_color));

		++nr_lines;
		if (options.max_lines() > 0 && nr_lines >= options.max_lines())
			break;

		y += line_height;
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
	const TextDrawOptions &options, size_t cursor, select_info* select) const
{
	static const float selection_red = 255 / 255.0f;
	static const float selection_green = 162 / 255.0f;
	static const float selection_blue = 0.0f;
	int block_width = std::ceil(_block_width * _scale * options.zoom());
	int block_height = height(options.zoom() * options.line_spacing());
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
		if (i_total == cursor)
		{
			if (cur_x - x + block_width <= options.max_width())
			{
				cursor_x = cur_x;
				cursor_y = cur_y;
			}
			else if (cur_line + 1 < options.max_lines())
			{
				cursor_x = x;
				cursor_y = cur_y + block_height;
			}
			// otherwise, no space to put the cursor
		}

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

			// Grum 2020-04-07: why do we rewrap here? And not on the first message?
// 			rewrap_message(&msgs[imsg], cat, options.zoom(), options.max_width(), NULL);
			last_color_char = 0;
		}

		if (select && select->lines && select->lines[cur_line].msg == -1)
		{
			select->lines[cur_line].msg = imsg;
			select->lines[cur_line].chr = ichar;
		}

		if (ch == '\n' || ch == '\r' || ch == '\0')
		{
			// newline
			if (++cur_line >= options.max_lines())
				break;
			cur_y += block_height;
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
				glColor3f(selection_red, selection_green, selection_blue);
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

		cur_x += draw_char(ch, cur_x, cur_y, options.zoom(), in_select);

		++ichar;
		++i_total;
		if (cur_x - x + block_width > options.max_width())
		{
			// ignore rest of this line, but keep track of
			// color characters
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

void Font::draw_console_separator(int x_space, int y,
	const TextDrawOptions& options) const
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
	float dy = height(zoom_y);
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
			cur_y += ch;
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
				float dx = width_pos(pos, zoom_x);
				float u_start, u_end, v_start, v_end;
				get_texture_coordinates(pos, u_start, u_end, v_start, v_end);

				glTexCoord2f(u_start, v_start); glVertex3f(cur_x,    cur_y+dy, z);
				glTexCoord2f(u_start, v_end);   glVertex3f(cur_x,    cur_y,    z);
				glTexCoord2f(u_end,   v_end);   glVertex3f(cur_x+dx, cur_y,    z);
				glTexCoord2f(u_end,   v_start); glVertex3f(cur_x+dx, cur_y+dy, z);

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
			if (ch >= 0)
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
int Font::render_glyph(Uint16 glyph, int i, int j, int size,
	TTF_Font *font, SDL_Surface *surface)
{
	static const SDL_Color white = { r: 0xff, g: 0xff, b: 0xff, a: 0xff };
	static const SDL_Color black = { r: 0x00, g: 0x00, b: 0x00, a: 0x10 };

	if (!TTF_GlyphIsProvided(font, glyph))
	{
		LOG_ERROR("Font file does not provide glyph for code point '%d': %s", glyph,
			TTF_GetError());
		return 0;
	}

	SDL_Surface* glyph_surface = TTF_RenderGlyph_Shaded(font, glyph, white, black);
	if (!glyph_surface)
	{
		LOG_ERROR("Failed to render TTF glyph: %s", TTF_GetError());
		return 0;
	}

	int w = glyph_surface->w;
	int h = glyph_surface->h;

	SDL_Rect area, glyph_area;
	glyph_area.x = 0;
	glyph_area.y = 0;
	glyph_area.w = w;
	glyph_area.h = h;
	area.x = j*size;
	area.y = i*size;
	area.w = w;
	area.h = h;

	SDL_SetSurfaceAlphaMod(glyph_surface, 0xFF);
	SDL_SetSurfaceBlendMode(glyph_surface, SDL_BLENDMODE_NONE);
	int err = SDL_BlitSurface(glyph_surface, &glyph_area, surface, &area);
	SDL_FreeSurface(glyph_surface);
	if (err)
	{
		LOG_ERROR("Failed to write glyph to surface: %s", SDL_GetError());
		return 0;
	}

	return w;
}

bool Font::build_texture_atlas()
{
	static const Uint16 glyphs[] = {
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
	static const int nr_glyphs = sizeof(glyphs) / sizeof(glyphs[0]);
	static const int nr_rows = (nr_glyphs + font_chars_per_line-1) / font_chars_per_line;

	if (!is_ttf())
	{
		LOG_ERROR("Font '%s' is not a TrueType font, no need to build a texture",
			_font_name.c_str());
		return false;
	}

	TTF_Font *font = TTF_OpenFont(_file_name.c_str(), ttf_point_size);
	if (!font)
	{
		LOG_ERROR("Failed to open TrueType font %s: %s", _file_name.c_str(), TTF_GetError());
		_flags |= Flags::FAILED;
		return false;
	}

	int size = TTF_FontLineSkip(font);
	int width = next_power_of_two(font_chars_per_line * size);
	int height = next_power_of_two(nr_rows * size);
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

	for (int i_glyph = 0; i_glyph < nr_glyphs; ++i_glyph)
	{
		int i = i_glyph / font_chars_per_line;
		int j = i_glyph % font_chars_per_line;
		int w = render_glyph(glyphs[i_glyph], i, j, size, font, image);
		if (w == 0)
		{
			SDL_FreeSurface(image);
			TTF_CloseFont(font);
			_flags |= Flags::FAILED;
			return false;
		}

		_char_widths[i_glyph] = w;
		_texture_coordinates[i_glyph][0] = float(j * size) / width;
		_texture_coordinates[i_glyph][1] = float(i * size) / height;
		_texture_coordinates[i_glyph][2] = float(j * size + w) / width;
		_texture_coordinates[i_glyph][3] = float(i * size + size) / height;
	}

	GLuint texture_id;
	glGenTextures(1, &texture_id);
	::bind_texture_id(texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		image->pixels);

	SDL_FreeSurface(image);
	_texture_id.gl_id = texture_id;
	_texture_width = width;
	_texture_height = height;
	_block_width = size;
	_block_height = size;
	_line_height = _block_height;
	_scale = float(font_block_height) / size;
	_flags |= Flags::HAS_TEXTURE;

	TTF_CloseFont(font);

	return true;
}

#endif // TTF

void Font::add_select_options() const
{
	add_multi_option_with_id("ui_font",    _font_name.c_str(), _file_name.c_str());
	add_multi_option_with_id("chat_font",  _font_name.c_str(), _file_name.c_str());
	add_multi_option_with_id("name_font",  _font_name.c_str(), _file_name.c_str());
	add_multi_option_with_id("book_font",  _font_name.c_str(), _file_name.c_str());
	add_multi_option_with_id("note_font",  _font_name.c_str(), _file_name.c_str());
	add_multi_option_with_id("rules_font", _font_name.c_str(), _file_name.c_str());
}


bool FontManager::initialize()
{
	_fonts.clear();

	for (size_t i = 0; i < 6; ++i)
		_fonts.push_back(Font(i));

#ifdef TTF
	if (use_ttf)
	{
		if (!TTF_WasInit() && TTF_Init() == -1)
		{
			LOG_ERROR("Failed to initialize True Type fonts: %s", TTF_GetError());
			use_ttf = 0;
		}
		else
		{
			std::string pattern = std::string(ttf_directory) + "/*.ttf";
#ifdef WINDOWS
			struct _finddata_t c_file;
			long hFile;
			if ((hFile = _findfirst(pattern.c_str(), &c_file)) != -1L)
			{
				do
				{
					try
					{
						_fonts.push_back(Font(c_file));
					}
					CATCH_AND_LOG_EXCEPTIONS
				}
				while (_findnext(hFile, &c_file) == 0);
			}
			_findclose(hFile);
#else // WINDOWS
			glob_t glob_res;
			if (glob(pattern.c_str(), 0, NULL, &glob_res) == 0)
			{
				for (size_t i = 0; i < glob_res.gl_pathc; i++)
				{
					try
					{
						_fonts.push_back(Font(glob_res.gl_pathv[i]));
					}
					CATCH_AND_LOG_EXCEPTIONS
				}
			}
			globfree(&glob_res);
#endif // WINDOWS
		}
	}
#endif // TTF

	for (const Font& font: _fonts)
	{
		font.add_select_options();
	}

	return !_fonts[0].failed();
}

Font& FontManager::get(Category cat)
{
	size_t font_num = font_idxs[cat];

	if (font_num < 0 || font_num >= _fonts.size())
	{
		// Invalid font number
		font_idxs[cat] = font_num = 0;
	}

	if (_fonts[font_num].failed())
	{
		// Failed to load previously, switch to fixed
		font_idxs[cat] = font_num = 0;
	}

	if (!_fonts[font_num].has_texture() && !_fonts[font_num].load_texture())
	{
		// Failed to load or generate texture, switch to fixed
		font_idxs[cat] = font_num = 0;
	}

	return _fonts[font_num];
}

} // namespace eternal_lands


extern "C"
{

using namespace eternal_lands;

size_t *font_idxs = FontManager::font_idxs;
float *font_scales = FontManager::font_scales;
#ifdef TTF
int use_ttf = 0;
char ttf_directory[TTF_DIR_SIZE];
#endif

int initialize_fonts()
{
	return FontManager::get_instance().initialize();
}

int get_char_width_zoom(unsigned char c, font_cat cat, float zoom)
{
	return FontManager::get_instance().width_spacing(cat, c, zoom);
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
void get_buf_dimensions(const unsigned char* str, size_t len, font_cat cat, float text_zoom,
	int *width, int *height)
{
	auto dims = FontManager::get_instance().dimensions(cat, str, len, text_zoom);
	*width = dims.first;
	*height = dims.second;
}

int reset_soft_breaks(unsigned char *text, int len, int size, font_cat cat,
	float text_zoom, int width, int *cursor, float *max_line_width)
{
	auto res = FontManager::get_instance().reset_soft_breaks(
		static_cast<FontManager::Category>(cat), text, size, len, text_zoom, width,
		cursor, max_line_width);

	size_t new_len = res.first.copy(text, size_t(size)-1);
	text[new_len] = '\0';

	return res.second;
}
void put_small_colored_text_in_box_zoomed(unsigned char color,
	const unsigned char* text, int len, int width,
	unsigned char* buffer, float text_zoom)
{
	// FIXME: no size specified for either buffer. Pass unlimited size,
	// and hope for the best...
	size_t size = ustring::npos;
	auto res = FontManager::get_instance().reset_soft_breaks(FontManager::Category::UI_FONT,
		text, size, len, text_zoom * DEFAULT_SMALL_RATIO, width, 0, 0);
	// If we don't end on a newline, add one.
	// TODO: check if that's necessary.
	if (res.first.back() != '\n')
		res.first.push_back('\n');

	size_t new_len = 0;
	if (!is_color(res.first.front()))
		buffer[new_len++] = color;
	new_len += res.first.copy(buffer + new_len, size);
	buffer[new_len] = '\0';
}

void draw_string_zoomed_width_font(int x, int y, const unsigned char *text,
	int max_width, int max_lines, font_cat cat, float zoom)
{
	TextDrawOptions options = TextDrawOptions().set_max_width(max_width)
		.set_max_lines(max_lines).set_zoom(zoom);
	FontManager::get_instance().draw(cat, text, strlen(reinterpret_cast<const char*>(text)),
		x, y, options);
}
void draw_string_zoomed_width_font_right(int x, int y, const unsigned char *text,
	int max_width, int max_lines, font_cat cat, float zoom)
{
	TextDrawOptions options = TextDrawOptions().set_max_width(max_width)
		.set_max_lines(max_lines).set_zoom(zoom)
		.set_alignment(TextDrawOptions::Alignment::RIGHT);
	FontManager::get_instance().draw(cat, text, strlen(reinterpret_cast<const char*>(text)),
		x, y, options);
}
void draw_string_zoomed_width_font_centered(int x, int y, const unsigned char *text,
	int max_width, int max_lines, font_cat cat, float zoom)
{
	TextDrawOptions options = TextDrawOptions().set_max_width(max_width)
		.set_max_lines(max_lines).set_zoom(zoom)
		.set_alignment(TextDrawOptions::Alignment::CENTER);
	FontManager::get_instance().draw(cat, text, strlen(reinterpret_cast<const char*>(text)),
		x, y, options);
}
void draw_string_zoomed_centered_around(int x, int y,
	const unsigned char *text, int center_idx, float zoom)
{
	size_t len = strlen(reinterpret_cast<const char*>(text));
	size_t idx = center_idx;
	TextDrawOptions options = TextDrawOptions().set_zoom(zoom)
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
void draw_string_shadowed_zoomed(int x, int y, const unsigned char* text,
	int max_lines, float fr, float fg, float fb, float br, float bg, float bb,
	float zoom)
{
	TextDrawOptions options = TextDrawOptions().set_max_lines(max_lines)
		.set_foreground(fr, fg, fb).set_background(br, bg, bb).set_zoom(zoom);
	FontManager::get_instance().draw(FontManager::Category::UI_FONT, text,
		strlen(reinterpret_cast<const char*>(text)), x, y, options);
}
void draw_string_shadowed_width(int x, int y, const unsigned char* text,
	int max_width, int max_lines, float fr, float fg, float fb,
	float br, float bg, float bb)
{
	int text_width = FontManager::get_instance().line_width(FontManager::Category::UI_FONT,
		text, strlen(reinterpret_cast<const char*>(text)));
	TextDrawOptions options = TextDrawOptions().set_max_width(max_width)
		.set_max_lines(max_lines).set_foreground(fr, fg, fb)
		.set_background(br, bg, bb)
		.set_zoom(float(max_width) / text_width);
	FontManager::get_instance().draw(FontManager::Category::UI_FONT, text,
		strlen(reinterpret_cast<const char*>(text)), x, y, options);
}

void draw_messages(int x, int y, text_message *msgs, int msgs_size, Uint8 filter,
	int msg_start, int offset_start, int cursor, int width, int height,
	font_cat cat, float text_zoom, select_info* select)
{
	int line_height = FontManager::get_instance().line_height(cat, text_zoom);
	TextDrawOptions options = TextDrawOptions().set_max_width(width)
		.set_max_lines(height / line_height)
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

int has_glyph(unsigned char c)
{
	return Font::has_glyph(c);
}

} //extern "C"

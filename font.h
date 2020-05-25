#ifndef NEW_FONT_H
#define NEW_FONT_H

#define DEFAULT_FIXED_FONT_WIDTH  11
#define DEFAULT_FIXED_FONT_HEIGHT 18
#define SMALL_FIXED_FONT_WIDTH    8
#define DEFAULT_SMALL_RATIO       (8.0f / 11.0f)

#define DEFAULT_FONT_Y_LEN      18.0f
#define SMALL_FONT_Y_LEN        15.0f
#define INGAME_FONT_X_LEN       0.17f
#define SMALL_INGAME_FONT_X_LEN 0.12f
#define SMALL_INGAME_FONT_Y_LEN 0.17f
#define ALT_INGAME_FONT_X_LEN   0.10f
#define ALT_INGAME_FONT_Y_LEN   0.15f

/*!
 * \ingroup text_font
 *
 * Enumeration for font categories
 *
 * This enumeration defines the different categories of fonts used in the
 * game. The actual enum values correspond to indices in the font_idxs array,
 * which holds the actual font numbers.
 */
typedef enum
{
	//! Index for the font used for drawing text in the user interface
	UI_FONT,
	//! Index for the font used for drawing text messages
	CHAT_FONT,
	//! Index for the font used for drawing names above the characters
	NAME_FONT,
	//! Index for the font used for drawing text in books
	BOOK_FONT,
	//! INdex for the font used to draw user notes
	NOTE_FONT,
	//! Index for the font used to draw the rules
	RULES_FONT,
	//! Index for the font used to draw the encyclopedia and help texts
	ENCYCLOPEDIA_FONT,
	//! Index used for the font used to draw the options window contents
	CONFIG_FONT,
	//! Number of font categories
	NR_FONT_CATS
} font_cat;

#include "gl_init.h"
#include "text.h"
#include "widgets.h"

#ifdef __cplusplus

#include <array>
#include <string>
#include <vector>
#include <SDL_types.h>
#ifdef TTF
#include <SDL_ttf.h>
#endif // TTF
#include "platform.h"

namespace eternal_lands
{

/*!
 * Type alias for a byte string, as used in many places in EL
 */
typedef std::basic_string<unsigned char> ustring;

class TextDrawOptions
{
public:
	//! Default foreground color for text
	static const std::array<float, 3> default_foreground_color;
	//! Default background color for text
	static const std::array<float, 3> default_background_color;
	//! Default color for selected text
	static const std::array<float, 3> default_selection_color;

	enum Alignment
	{
		LEFT,
		CENTER,
		RIGHT
	};
	//! Bit flags controlling the look of the text
	enum Flags
	{
		//! If set, draw a shadow around the text
		SHADOW = 1 << 0,
		//! If set, ignore color characters in the text
		IGNORE_COLOR = 1 << 1,
		//! If set, draw a semi-transparent background behind the text
		HELP = 1 << 2,
		//! If set, draw ellipsis (...) after truncated strings
		ELLIPSIS = 1 << 3
	};

	/*!
	 * Constructor
	 *
	 * Create a new TextDrawOptions object, with default settings.
	 */
	TextDrawOptions();

	int max_width() const { return _max_width; }
	int max_lines() const { return _max_lines; }
	float zoom() const { return _zoom; }
	float line_spacing() const { return _line_spacing; }
	Alignment alignment() const { return _alignment; }
	bool shadow() const { return _flags & SHADOW; }
	bool ignore_color() const { return _flags & IGNORE_COLOR; }
	bool is_help() const { return _flags & HELP; }
	bool ellipsis() const { return _flags & ELLIPSIS; }

	bool has_foreground_color() const { return _fg_color[0] >= 0.0; }
	bool has_background_color() const { return _bg_color[0] >= 0.0; }

	TextDrawOptions& set_max_width(int width)
	{
		_max_width = width;
		return *this;
	}

	TextDrawOptions& set_max_lines(int lines)
	{
		_max_lines = lines;
		return *this;
	}

	TextDrawOptions& set_zoom(float zoom)
	{
		_zoom = zoom;
		return *this;
	}

	TextDrawOptions& scale_zoom(float scale)
	{
		_zoom *= scale;
		return *this;
	}

	TextDrawOptions& set_line_spacing(float scale)
	{
		_line_spacing = scale;
		return *this;
	}

	TextDrawOptions& set_alignment(Alignment alignment)
	{
		_alignment = alignment;
		return *this;
	}

	TextDrawOptions& set_shadow(bool shadow=true)
	{
		if (shadow)
			_flags |= SHADOW;
		else
			_flags &= ~SHADOW;
		return *this;
	}

	TextDrawOptions& set_foreground(float r, float g, float b)
	{
		_fg_color[0] = r;
		_fg_color[1] = g;
		_fg_color[2] = b;
		return *this;
	}

	TextDrawOptions& set_background(float r, float g, float b)
	{
		_bg_color[0] = r;
		_bg_color[1] = g;
		_bg_color[2] = b;
		return *this;
	}

	TextDrawOptions& set_ignore_color(bool ignore=true)
	{
		if (ignore)
			_flags |= IGNORE_COLOR;
		else
			_flags &= ~IGNORE_COLOR;
		return *this;
	}

	TextDrawOptions& set_help(bool is_help=true)
	{
		if (is_help)
			_flags |= HELP;
		else
			_flags &= ~HELP;
		return *this;
	}

	TextDrawOptions& set_ellipsis(bool ellipsis=true)
	{
		if (ellipsis)
			_flags |= ELLIPSIS;
		else
			_flags &= ~ELLIPSIS;
		return *this;
	}

	//! Set the color for selected text to (\a r, \a g, \a b).
	TextDrawOptions& set_selection(float r, float g, float b)
	{
		_sel_color[0] = r;
		_sel_color[1] = g;
		_sel_color[2] = b;
		return *this;
	}

	//! Set the current draw color to the foreground color in these options
	void use_foreground_color() const;
	//! Set the current draw color to the background color in these options
	void use_background_color() const;
	//! Set the current draw color to the selection color in these options
	void use_selection_color() const;

private:
	int _max_width;
	int _max_lines;
	float _zoom;
	float _line_spacing;
	Alignment _alignment;
	Uint32 _flags;
	std::array<float, 3> _fg_color;
	std::array<float, 3> _bg_color;
	std::array<float, 3> _sel_color;
};

class Font
{
public:
	/*!
	 * Create a new font
	 *
	 * Initialize a new internal font. This sets the parameters for the \a i'th
	 * font bundled with EL, but does not yet load the font texture because it
	 * is called before OpenGL is initialized. There are 6 fonts available,
	 * they are:
	 * 0: textures/font.dds
	 * 1: textures/font2.dds
	 * 2: textures/font3.dds
	 * 3: textures/font5.dds
	 * 4: textures/font6.dds
	 * 5: textures/font7.dds
	 *
	 * \param i The number of the EL bundled font
	 */
	Font(size_t i);
#ifdef TTF
	/*!
	 * Create a new font.
	 *
	 * Create a new font description from the TTF font file specified by
	 * \a ttf_file_name. This only loads the font description, and does not yet
	 * generate a texture.
	 *
	 * \param ttf_file_name The file name of the TTF font to load.
	 */
	Font(const std::string& ttf_file_name);
#endif
	//! Destructor
	~Font();

	//! Return the name of the font
	const std::string& font_name() const { return _font_name; }
#ifdef TTF
	//! Check if this font is a TTF font
	bool is_ttf() const { return _flags & Flags::IS_TTF; }
#endif
	//! Check if the font is fixed width
	bool is_fixed_width() const { return _flags & Flags::FIXED_WIDTH; }
	//! Check if a texture has been generated for this font
	bool has_texture() const { return _flags & Flags::HAS_TEXTURE; }
	//! Check if this font failed to load
	bool failed() const { return _flags & Flags::FAILED; }

	/*!
	 * \ingroup text_font
	 *
	 * Check if a character has a glyph.
	 *
	 * Check if a glyph is defined for a character \a c, i.e. if it can be printed.
	 *
	 * \param c The character to check
	 * \return \c true if the character has a glyph, \c false otherwise.
	 */
	static bool has_glyph(unsigned char c)
	{
		return get_position(c) >= 0;
	}

	/*!
	 * Get the width of a character
	 *
	 * Get the width of character \a c when drawn in this font at zoom level \a zoom.
	 *
	 * \param c    The character of which to determine the width
	 * \param zoom The zoom factor for drawing the glyph
	 * \return The width of \a c, in pixels.
	 */
	int width(unsigned char c, float zoom=1.0) const
	{
		return width_pos(get_position(c), zoom);
	}
	/*!
	 * Get the width of a character plus spacing
	 *
	 * Get the width of character \a c when drawn in this font at zoom level \a zoom,
	 * plus the spacing between two characters.
	 *
	 * \param c    The character of which to determine the width
	 * \param zoom The zoom factor for drawing the glyph
	 * \return The width of \a c and spacing, in pixels.
	 */
	int width_spacing(unsigned char c, float zoom=1.0) const
	{
		return width_spacing_pos(get_position(c), zoom);
	}
	/*!
	 * Get the maximum character width, plus spacing
	 *
	 * Get the maximum width of a single character in this font when drawn at zoom
	 * level \a zoom, and include the spacing between characters.
	 *
	 * \param zoom The zoom factor for drawing the character.
	 * \return The maximum width of a character including spacing, in pixels.
	 */
	int max_width_spacing(float zoom=1.0) const;
	/*!
	 * Get the average character width, plus spacing
	 *
	 * Get an approximation to the average width of a character in this font when
	 * drawn at zoom level \a zoom, including the space between characters,
	 * for some definition of "average". The average is calculated by assigning
	 * a weight to each possible character, and taking the weighted average
	 * of all character widths. The weights are based on a study on normal
	 * English text, so use this function with the following caveats:
	 *
	 * 1) Relative frequencies are based on English text, and so may not be
	 *    applicable to text in to other languages,
	 * 2) Furthermore, weights are based on text from the New York Times, and so
	 *    may not be representative even for English text in EL,
	 * 3) Only ASCII characters were counted in the stdy (so accented characters
	 *    don't contribute), and a few ASCII characters are missing as well
	 *    ('[', '\', ']', '^', '_', and '`' to be exact).
     * 4) The occurance of the space character was guesstimated from the reported
	 *    total number of words in the corpus.
	 *
	 * Proceed to use with caution, do not use this function when you need to be
	 * sure a string will fit within a certain space, use max_width_spacing()
	 * or a monospaced font instead.
	 *
	 * \param zoom The zoom factor for drawing the character.
	 * \return The maximum width of a character including spacing, in pixels.
	 */
	int average_width_spacing(float zoom=1.0) const;
	/*!
	 * Get the maximum width of a digit, plus spacing
	 *
	 * Get the maximum width of a single digit character (0-9) in this font when
	 * drawn at zoom level \a zoom, and include the spacing between characters.
	 *
	 * \param zoom The zoom factor for drawing the character.
	 * \return The maximum width of a digit character including spacing, in pixels.
	 */
	int max_digit_width_spacing(float zoom=1.0) const;
	/*!
	 * Calculate the width of a string
	 *
	 * Calculate the width in pixels of the string \a text of length \a len
	 * when drawn in this font with scale factor zoom.
	 * NOTE: this function assumes the string is a single line. Newline
	 * characters are ignored, and having them in the middle of string will
	 * result in a too large value for the width.
	 *
	 * \param text The string for which to calculate the length
	 * \param len  The number of bytes in \a text
	 * \param zoom The scale factor for the text
	 * \return The width of the text in pixels
	 */
	int line_width(const unsigned char* text, size_t len, float zoom) const;
	/*!
	 * Calculate the width of a string with final spacing
	 *
	 * Calculate the width in pixels of the string \a text of length \a len,
	 * including the spacing after the final character, when drawn in this font
	 * with scale factor \a zoom.
	 * NOTE: this function assumes the string is a single line. Newline
	 * characters are ignored, and having them in the middle of string will
	 * result in a too large value for the width.
	 *
	 * \param text The string for which to calculate the length
	 * \param len  The number of bytes in \a text
	 * \param zoom The scale factor for the text
	 * \return The width of the text in pixels
	 */
	int line_width_spacing(const unsigned char* text, size_t len, float zoom) const;
	/*!
	 * Calculate the width of a string with final spacing
	 *
	 * Calculate the width in pixels of the string \a text including the spacing
	 * after the final character, when drawn in this font with scale factor \a zoom.
	 * NOTE: this function assumes the string is a single line. Newline
	 * characters are ignored, and having them in the middle of string will
	 * result in a too large value for the width.
	 *
	 * \param text The string for which to calculate the length
	 * \param zoom The scale factor for the text
	 * \return The width of the text in pixels
	 */
	int line_width_spacing(const ustring& text, float zoom) const
	{
		return line_width_spacing(text.data(), text.length(), zoom);
	}
	/*!
	 * Calculate the dimensions of a block of text
	 *
	 * Calculate the width and height of string \a text of length \a len bytes
	 * when drawn in this font with scale factor \a zoom. The string may contain
	 * multiple lines, the width returned is then the width of the widest line.
	 *
	 * \param text The string for which to compute the dimensions
	 * \param len  The number of bytes in \a text
	 * \param zoom The scale factor for the text
	 * \return The width and height of the text
	 */
	std::pair<int, int> dimensions(const unsigned char* text, size_t len, float zoom) const;

	/*!
	 * \ingroup text_font
	 * \brief   Recompute where the line breaks in a string should occur
	 *
	 * Recomputes the positions in string \a text where line breaks should be
	 * placed so that the string fits into a window. This creates a new string
	 * from the the contents of \a text and inserts '\r' characters at the
	 * positions where the line should be broken. The parameter \a options
	 * specifies how the text will be drawn, currently only the \c zoom and
	 * \c max_width field are used int this function. The optional parameter \a cursor
	 * contains the offset of the cursor position in the text; it will be updated
	 * as new line breaks are inserted. The optional parameter \a max_line_width
	 * can be used to retrieve the largest width in pixels of the lines in \a text
	 * after rewrapping.
	 *
	 * \param text       the string
	 * \param text_len   the actual length of the string
	 * \param options    the options defining the layout of the text
	 * \param cursor     pointer to the cursor position, or NULL if not used
	 * \param max_line_width pointer the maximum line length after wrapping, or NULL if not used
	 *
	 * \return The wrapped text, and the new number of lines in the text
	 */
	std::pair<ustring, int> reset_soft_breaks(const unsigned char *text,
		size_t text_len, const TextDrawOptions& options,
		int *cursor = 0, float *max_line_width = 0);
	/*!
	 * \ingroup text_font
	 * \brief   Recompute where the line breaks in a string should occur
	 *
	 * Recomputes the positions in string \a text where line breaks should be
	 * placed so that the string fits into a window. This creates a new string
	 * from the the contents of \a text and inserts '\r' characters at the
	 * positions where the line should be broken. The parameter \a options
	 * specifies how the text will be drawn, currently only the \c zoom and
	 * \c max_width field are used int this function.
	 *
	 * \param text       the string
	 * \param options    the options defining the layout of the text
	 *
	 * \return The wrapped text, and the new number of lines in the text
	 */
	std::pair<ustring, int> reset_soft_breaks(const ustring& text,
		const TextDrawOptions& options)
	{
		return reset_soft_breaks(text.data(), text.length(), options, nullptr, nullptr);
	}

	/*!
	 * Draw a text string
	 *
	 * Draw the text in the first \a len bytes of \a text, starting at position
	 * \a x, \a y, using the drawing option in \a options.
	 *
	 * \param text      The text to draw
	 * \param len       The number of bytes in \a text
	 * \param x         The left coordinate of the drawn text
	 * \param y         The top coordinate of the drawn text
	 * \param options   Options defining the layout of the text
	 * \param sel_begin Start index of selected text
	 * \param sel_end   End index of selected text (one past last selected character)
	 */
	void draw(const unsigned char* text, size_t len, int x, int y,
		const TextDrawOptions &options, size_t sel_begin=0, size_t sel_end=0) const;
	/*!
	 * \ingroup text_font
	 * Draws messages in a buffer to the screen
	 *
	 * Draws the messages in buffer \a msgs to the screen, starting with character
	 * \a offset_start in message number \a msg_start.
	 * NOTE: The messages are rewrapped if necessary.
	 *
	 * \param msgs         the message buffer
	 * \param msgs_size    the total number of messages that \a msgs can hold
	 * \param x            x coordinate of the position to start drawing
	 * \param y            y coordinate of the position to start drawing
	 * \param filter       draw only messages in channel \a filter. Choose
	 * 	FILTER_ALL for displaying all messages
	 * \param msg_start	  index of the first message to display
	 * \param offset_start the first character in message \a msg_start to display
	 * \param options      Options defining the layout of the text
	 * \param cursor       if >= 0, the position at which to draw the cursor
	 * \param[in,out] select information about current selection. draw_messages()
	 *	 fills the select->lines array.
	 *
	 * \callgraph
	 */
	void draw_messages(const text_message *msgs, size_t msgs_size, int x, int y,
		Uint8 filter, size_t msg_start, size_t offset_start,
		const TextDrawOptions &options, size_t cursor, select_info* select) const;
	void draw_console_separator(int x_space, int y, const TextDrawOptions& options) const;
#ifdef ELC
#ifndef MAP_EDITOR2
	void draw_ortho_ingame_string(const unsigned char* text, size_t len,
		float x, float y, float z, int max_lines, float zoom_x, float zoom_y) const;
	void draw_ingame_string(const unsigned char* text, size_t len,
		float x, float y, int max_lines, float zoom_x, float zoom_y) const;
#endif // ! MAP_EDITOR2
#endif // ELC

private:
	static const size_t font_nr_lines = 10;
	static const size_t font_chars_per_line = 14;
	//! The number of different glyphs we recognise
	static const size_t nr_glyphs = 9*14 + 6;
	static const int font_block_width = 18;
	static const int font_block_height = 21;
	static const int default_line_height = 18;
	static const int ttf_point_size = 32;
	static const std::array<int, nr_glyphs> letter_freqs;
	static const ustring ellipsis;

	//! Flags indicating the status and properties of a font
	enum Flags
	{
#ifdef TTF
		//! Set if the font is a TTF Font
		IS_TTF      = 1 << 0,
#endif
		//! Set if the font is fixed width, i.e. all characters have the same width
		FIXED_WIDTH = 1 << 1,
		//! Set if the texture for the font is loaded or generated
		HAS_TEXTURE = 1 << 2,
		//! Set if loading the font failed, and a fallback dhould be used
		FAILED      = 1 << 3
	};

	//! Name of this font. This will be shown on the multi-select button.
	std::string _font_name;
	//! Name of the files containing the font texture or TTF font description.
	std::string _file_name;
	//! Flags indicating the type and status of this font.
	Uint32 _flags;
	//! Width of the font texture in pixels.
	int _texture_width;
	//! Height of the font texture in pixels.
	int _texture_height;
	//! Width of each character in the texture, in pixels
	std::array<int, nr_glyphs> _char_widths;
	//! How far to advance the pen position after a character, in pixels
	std::array<int, nr_glyphs> _texture_char_widths;
	//! Texture coordinates for each character in the texture
	std::array<float[4], nr_glyphs> _texture_coordinates;
	//! Width reserved for a character in the texture
	int _block_width;
	//! Height reserved for a character in the texture
	int _block_height;
	//! Height of a single character in pixels
	int _line_height;
	//! Maximum width of a glyph
	int _max_char_width;
	//! Maximum width of a digit 0-9
	int _max_digit_width;
	//! "Typical" character width for English text
	int _avg_char_width;
	//! Distance between characters when drawn (at default zoom level)
	int _spacing;
	//! Scale factor that scales texture to default height
	float _scale;
#ifdef TTF
	union
	{
		//! ID of the texture for this font in the texture cache.
		Uint32 cache_id;
		//! Open GL texture ID for a generated texture.
		GLuint gl_id;
	} _texture_id;
#else
	//! ID of the texture for this font in the texture cache
	Uint32 _texture_id;
#endif

	friend class FontManager;

	/*!
	 * Get the position of a glyph in the texture
	 *
	 * \param c The byte for which to get the position
	 * \return The position in the font texture, of -1 if \a c is not a valid glyph.
	 */
	static int get_position(unsigned char c);
	/*!
	 * Get the width of a character
	 *
	 * Get the width of the glyph at position \a pos in the texture when drawn
	 * in this font at zoom level \a zoom.
	 *
	 * \param pos  The position of the glyph in the texture
	 * \param zoom The zoom factor for drawing the glyph
	 */
	int width_pos(int pos, float zoom=1.0) const;
	/*!
	 * Get the width of a character plus spacing
	 *
	 * Get the width of the glyph at position \a pos in the texture when drawn
	 * in this font at zoom level \a zoom, plus the spacing between two
	 * characters.
	 *
	 * \param pos  The position of the glyph in the texture
	 * \param zoom The zoom factor for drawing the glyph
	 */
	int width_spacing_pos(int pos, float zoom=1.0) const;
	/*!
	 * Get the height of a line.
	 *
	 * Get the height of a line when drawing text at zoom level \a zoom.
	 *
	 * \param zoom The zoom factor for drawing the text
	 */
	int height(float zoom=1.0) const;


	/*!
	 * Load or generate the texture for this font.
	 *
	 * Load the texture into the texture cache for normal fonts, or generate
	 * a texture atlas for TTF fonts.
	 *
	 * \return \c true on success, \c false on failure.
	 */
	bool load_texture();
	//! Bind the font texture for this font
	void bind_texture() const;
	/*!
	 * Get texture coordinates for a character
	 *
	 * Get the texture coordinates in the font texture for the character at
	 * position \a pos.
	 *
	 * \param pos     The position of the character within the texture
	 * \param u_start On exit, the left coordinate
	 * \param u_end   On exit, the right coordinate
	 * \param v_start On exit, the top coordinate
	 * \param v_end   On exit, the bottom coordinate
	 */
	void get_texture_coordinates(int pos,
		float &u_start, float &u_end, float &v_start, float &v_end) const;

	/*!
	 * Set the current draw color.
	 *
	 * Set the drawing color for the text to the color define in \see colors_list
	 * at index \a color.
	 *
	 * \param color The index of the text color in the \see colors_list.
	 */
	static void set_color(int color);

	/*!
	 * Draw a single character
	 *
	 * Draw a single character \a c at position \a x, \a y with its sized scaled
	 * by a factor \a zoom. This function only adds the quad with the correct
	 * coordinates and texture coordinates; calls to this functions should be
	 * done inside a glBegin(GL_QUADS)/glEnd() pair. If \a c is a color character,
	 * the color is set and nothing is drawn.
	 *
	 * \param c            The character to draw
	 * \param x            The x coordinate of the left side of the drawn character
	 * \param y            The x coordinate of the top of the drawn character
	 * \param zoom         The scale factor for the character
	 * \param ignore_color If \c true, color characters are ignored and the
	 * 	drawing color is left unchanged.
	 *
	 * \return The width of the drawn character, or 0 for color characters or
	 * 	invalid characters
	 */
	int draw_char(unsigned char c, int x, int y, float zoom, bool ignore_color) const;
	/*!
	 * Draw background for help texts
	 *
	 * Draw a semi-transparent background box for tooltips.
	 *
	 * \param x      The x coordinate of the left side of the box
	 * \param y      The y coordinate of the top of the box
	 * \param width  The width of the box in pixels
	 * \param height The height of the box in pixels
	 */
	void draw_help_background(int x, int y, int width, int height) const;
	/*!
	 * Draw a single line of text
	 *
	 * Draw a single line of text in \a text of length \a len bytes, starting
	 * at position \a x, \a y and drawing to the left.
	 * NOTE: This function is for drawing a single line. Any newlines in \a text
	 * are ignored.
	 * NOTE: The alignment option in \a options has no effect for this function,
	 * text is always drawn left to right.
	 *
	 * \param text      The line of text to draw
	 * \param len       The number of bytes in text to draw
	 * \param x         The x coordinate of the left of the line
	 * \param y         The y coordinate of the top of the line
	 * \param options   Drawing options for the text
	 * \param sel_begin Start index of selected text
	 * \param sel_end   End index of selected text (one past last selected character)
	 *
	 */
	void draw_line(const unsigned char* text, size_t len, int x, int y,
		const TextDrawOptions &options, size_t sel_begin=0, size_t sel_end=0) const;
	/*!
	 * Clip a line of text
	 *
	 * Clip a line of text such that the remainder fits into \a max_width pixels.
	 * This function takes alignment into account: left aligned text is clipped
	 * on the right, right-aligned text on the left, centered text on both sides
	 * equally. In case color characters are clipped, the last color character
	 * clipped on the left and right are returned through \a before_color and
	 * \a after_color.
	 *
	 * \param text         The text to clip
	 * \param len          The number of bytes in \a text
	 * \param options      Drawing options for the text
	 * \param before_color On exit, the last color character clipped before the
	 * 	remaining string, or 0 if there was none.
	 * \param after_color  On exit, the last color character clipped after the
	 * 	remaining string, or 0 if there was none.
	 * \param width        On exit, the remaining width of the clipped string.
	 * \return Start index and length of the clipped substring
	 */
	std::pair<size_t, size_t> clip_line(const unsigned char* text, size_t len,
		const TextDrawOptions &options, unsigned char &before_color, unsigned char &after_color,
		int &width) const;

#ifdef TTF
	/*!
	 * Render a single glyph
	 *
	 * Render the glyph \a glyph wit size \a size at row \a i and column \a j
	 * in the font atlas \a surface, using font \a font. The vertical offset
	 * parameter \a y_delta is used ti try and center the glyphs vertically,
	 * making vertical alignment of text easier.
	 *
	 * \param glyph   The Unicode code point of the glyph to render
	 * \param i       The row in the atlas at which to place the glyph
	 * \param j       The column in the atlas at which to place the glyph
	 * \param size    The size of the rendered glyph
	 * \param y_delta Vertical offset for placing the glyph in the texture
	 * \param font    The font with which to render the glyph
	 * \param surface The surface on which the glyph is rendered
	 * \return a pair of integers, containing the width of the character in the`
	 * 	texture, and how far to advance the pen to draw the next texture
	 */
	static std::pair<int, int> render_glyph(Uint16 glyph, int i, int j, int size,
		int y_delta, TTF_Font *font, SDL_Surface *surface);
	/*!
	 * Build a texture for a TTF font
	 *
	 * Build a texture containing all supported glyphs from the TrueType font
	 * associated with this font.
	 *
	 * \return \c true on succes, \c false on failure.
	 */
	bool build_texture_atlas();
#endif // TTF

	/*!
	 * Add this font t the selections
	 *
	 * Add this font as an option to the various font selections in the
	 * settings window.
	 */
	void add_select_options() const;
};

class FontManager
{
public:
	// Use font category enumeration shared with C code.
	typedef ::font_cat Category;

	//! The font numbers for each font category
	static size_t font_idxs[NR_FONT_CATS];
	//! The zoom factor for each font category
	static float font_scales[NR_FONT_CATS];

	static FontManager& get_instance()
	{
		static FontManager manager;
		return manager;
	}

	/*!
	 * Initialize the font managaer.
	 *
	 * This adds the standard fonts bundled with EL to the font manager, and
	 * if TTF fonts are enabled, scans the TTF path for True Type fonts
	 * to add as well.
	 */
	bool initialize();

	/*!
	 * Return the font number of a fixed width font
	 *
	 * Return the font number for the \a idx'th fixed width font.
	 *
	 * \param idx The number of the font in the list of fixed width fonts only
	 * \return The index of the font in the list of all fonts
	 */
	size_t fixed_width_font_number(size_t idx)
	{
		try
		{
			return _fixed_width_idxs.at(idx);
		}
		catch (const std::out_of_range&)
		{
			// Return standard fixed width font on invalid index
			return 0;
		}
	}

	/*!
	 * The width of a single character
	 *
	 * Return the width of a single character including spacing, when drawn
	 * in the font for category \a cat, at zoom level \a text_zoom.
	 *
	 * \param cat       The font category for the font used
	 * \param c         The character for which to get the width
	 * \param text_zoom The scale factor for the text
	 * \return The width of the character and spacing, in pixels.
	 */
	int width_spacing(Category cat, unsigned char c, float text_zoom=1.0)
	{
		return get(cat).width_spacing(c, text_zoom * font_scales[cat]);
	}
	/*!
	 * The maximum width of a single character
	 *
	 * Return the maximum a single character can occupy when drawn in the font
	 * for category \a cat at zoom level \a text_zoom, and include the spacing
	 * between characters.
	 *
	 * \param cat       The font category for the font used
	 * \param text_zoom The scale factor for the text
	 * \return The maximum character width incuding spacing, in pixels.
	 */
	int max_width_spacing(Category cat, float text_zoom=1.0)
	{
		return get(cat).max_width_spacing(text_zoom * font_scales[cat]);
	}
	/*!
	 * Get the average character width, plus spacing
	 *
	 * Get an approximation to the average width of a character in the font for
	 * category \a cat when drawn at zoom level \a zoom, including the space
	 * between characters, for some definition of "average". See the note
	 * in Font::average_width_spacing() for some caveats when using this function.
	 *
	 * \param cat       The font category for the font used
	 * \param zoom The zoom factor for drawing the character.
	 * \return The maximum width of a character including spacing, in pixels.
	 */
	int average_width_spacing(Category cat, float text_zoom=1.0)
	{
		return get(cat).average_width_spacing(text_zoom * font_scales[cat]);
	}
	/*!
	 * The maximum width of a single digit character
	 *
	 * Return the maximum a single digit character (0-9) can occupy when drawn
	 * in the font for category \a cat at zoom level \a text_zoom, and include
	 * the spacing between characters.
	 *
	 * \param cat       The font category for the font used
	 * \param text_zoom The scale factor for the text
	 * \return The maximum digit character width incuding spacing, in pixels.
	 */
	int max_digit_width_spacing(Category cat, float text_zoom=1.0)
	{
		return get(cat).max_digit_width_spacing(text_zoom * font_scales[cat]);
	}
	/*!
	 * Calculate the width of a string
	 *
	 * Calculate the width in pixels of the string \a text of length \a len
	 * when drawn in the font for category \a cat.
	 * NOTE: this function assumes the string is a single line. Newline
	 * characters are ignored, and having them in the middle of string will
	 * result in a too large value for the width.
	 *
	 * \param cat       The font category for the font used
	 * \param text      The string for which to calculate the length
	 * \param len       The number of bytes in \a text
	 * \param text_zoom The scale factor for the text
	 * \return The width of the text in pixels
	 */
	int line_width(Category cat, const unsigned char* text, size_t len,
		float text_zoom=1.0)
	{
		return get(cat).line_width(text, len, text_zoom * font_scales[cat]);
	}
	/*!
	 * The height of a text line
	 *
	 * Return the height of a line of text when drawn in the font for category
	 * \a cat with scale factor \a text_zoom.
	 *
	 * \param cat       The font category for the font used
	 * \param text_zoom The scale factor for the text
	 * \return The height of the text in pixels
	 */
	int line_height(Category cat, float text_zoom=1.0)
	{
		return get(cat).height(text_zoom * font_scales[cat]);
	}
	/*!
	 * Calculate the dimensions of a block of text
	 *
	 * Calculate the width and height of string \a text of length \a len bytes
	 * when drawn in the font for category \a cat, with scale factor \a zoom.
	 * The string may contain multiple lines, the width returned is then the
	 * width of the widest line.
	 *
	 * \param cat       The font category for the font used
	 * \param text      The string for which to compute the dimensions
	 * \param len       The number of bytes in \a text
	 * \param text_zoom The scale factor for the text
	 * \return The width and height of the text
	 */
	std::pair<int, int> dimensions(Category cat, const unsigned char* text, size_t len,
		float text_zoom)
	{
		return get(cat).dimensions(text, len, text_zoom * font_scales[cat]);
	}

	/*!
	 * \ingroup text_font
	 * \brief   Recompute where the line breaks in a string should occur
	 *
	 * Recomputes the positions in string \a text where line breaks should be
	 * placed so that the string fits into a window. This creates a new string
	 * from the the contents of \a text and inserts '\r' characters at the
	 * positions where the line should be broken. The parameter \a options
	 * specifies how the text will be drawn, currently only the \c zoom and
	 * \c max_width field are used int this function. The optional parameter \a cursor
	 * contains the offset of the cursor position in the text; it will be updated
	 * as new line breaks are inserted. The optional parameter \a max_line_width
	 * can be used to retrieve the largest width in pixels of the lines in \a text
	 * after rewrapping.
	 *
	 * \param cat        the font category in which the string is to be drawn
	 * \param text       the string
	 * \param text_len   the actual length of the string
	 * \param options    the options defining the layout of the text
	 * \param cursor     pointer to the cursor position, or NULL if not used
	 * \param max_line_width pointer the maximum line length after wrapping, or NULL if not used
	 *
	 * \return The wrapped text, and the new number of lines in the text
	 */
	std::pair<ustring, int> reset_soft_breaks(Category cat, const unsigned char *text,
		size_t text_len, const TextDrawOptions& options,
		int *cursor = 0, float *max_line_width = 0)
	{
		TextDrawOptions cat_options = TextDrawOptions(options).scale_zoom(font_scales[cat]);
		return get(cat).reset_soft_breaks(text, text_len, cat_options, cursor, max_line_width);
	}
	/*!
	 * \ingroup text_font
	 * \brief   Recompute where the line breaks in a string should occur
	 *
	 * Recomputes the positions in string \a text where line breaks should be
	 * placed so that the string fits into a window. This creates a new string
	 * from the the contents of \a text and inserts '\r' characters at the
	 * positions where the line should be broken. The parameter \a options
	 * specifies how the text will be drawn, currently only the \c zoom and
	 * \c max_width field are used int this function.
	 *
	 * \param cat        the font category in which the string is to be drawn
	 * \param text       the string
	 * \param options    the options defining the layout of the text
	 *
	 * \return The wrapped text, and the new number of lines in the text
	 */
	std::pair<ustring, int> reset_soft_breaks(Category cat, const ustring& text,
		const TextDrawOptions& options)
	{
		TextDrawOptions cat_options = TextDrawOptions(options).scale_zoom(font_scales[cat]);
		return get(cat).reset_soft_breaks(text, cat_options);
	}

	/*!
	 * Draw a text string
	 *
	 * Draw the text in the first \a len bytes of \a text, starting at position
	 * \a x, \a y, using the drawing option in \a options, using the font for
	 * category \a cat.
	 *
	 * \param cat       The font category for the text
	 * \param text      The text to draw
	 * \param len       The number of bytes in \a text
	 * \param x         The left coordinate of the drawn text
	 * \param y         The top coordinate of the drawn text
	 * \param options   Options defining the layout of the text
	 * \param sel_begin Start index of selected text
	 * \param sel_end   End index of selected text (one past last selected character)
	 */
	void draw(Category cat, const unsigned char* text, size_t len, int x, int y,
		const TextDrawOptions &options, size_t sel_begin=0, size_t sel_end=0)
	{
		TextDrawOptions cat_options = TextDrawOptions(options).scale_zoom(font_scales[cat]);
		get(cat).draw(text, len, x, y, cat_options, sel_begin, sel_end);
	}
	/*!
	 * \ingroup text_font
	 * Draws messages in a buffer to the screen
	 *
	 * Draws the messages in buffer \a msgs to the screen using the font for
	 * category \a cat, starting with character \a offset_start in message
	 * number \a msg_start.
	 * NOTE: The messages are rewrapped if necessary.
	 *
	 * \param cat          The font category for the text
	 * \param msgs         the message buffer
	 * \param msgs_size    the total number of messages that \a msgs can hold
	 * \param x            x coordinate of the position to start drawing
	 * \param y            y coordinate of the position to start drawing
	 * \param filter       draw only messages in channel \a filter. Choose
	 * 	FILTER_ALL for displaying all messages
	 * \param msg_start	  index of the first message to display
	 * \param offset_start the first character in message \a msg_start to display
	 * \param options      Options defining the layout of the text
	 * \param cursor       if >= 0, the position at which to draw the cursor
	 * \param[in,out] select information about current selection. draw_messages()
	 *	 fills the select->lines array.
	 *
	 * \callgraph
	 */
	void draw_messages(Category cat, const text_message *msgs, size_t msgs_size,
		int x, int y, Uint8 filter, size_t msg_start, size_t offset_start,
		const TextDrawOptions &options, size_t cursor, select_info* select)
	{
		TextDrawOptions cat_options = TextDrawOptions(options).scale_zoom(font_scales[cat]);
		get(cat).draw_messages(msgs, msgs_size, x, y, filter, msg_start, offset_start,
			cat_options, cursor, select);
	}
	void draw_console_separator(Category cat, int x_space, int y,
		const TextDrawOptions& options)
	{
		TextDrawOptions cat_options = TextDrawOptions(options).scale_zoom(font_scales[cat]);
		get(cat).draw_console_separator(x_space, y, cat_options);
	}
#ifdef ELC
#ifndef MAP_EDITOR2
	void draw_ortho_ingame_string(const unsigned char* text, size_t len,
		float x, float y, float z, int max_lines, float zoom_x, float zoom_y)
	{
		get(NAME_FONT).draw_ortho_ingame_string(text, len, x, y, z, max_lines,
			zoom_x * font_scales[NAME_FONT], zoom_y * font_scales[NAME_FONT]);
	}
	void draw_ingame_string(const unsigned char* text, size_t len,
		float x, float y, int max_lines, float zoom_x, float zoom_y)
	{
		get(CHAT_FONT).draw_ingame_string(text, len, x, y, max_lines,
			zoom_x * font_scales[CHAT_FONT], zoom_y * font_scales[CHAT_FONT]);
	}
#endif // !MAP_EDITOR_2
#endif // ELC

	/*!
	 * Set the config font
	 *
	 * Copy the current font index and scale of the UI_FONT category to that
	 * of the CONFIG_FONT category. This allows us to change the UI font
	 * without changing the way the options window is drawn, and hopefully
	 * stops the user from then being unable to change the font back.
	 */
	void set_config_font()
	{
		font_idxs[CONFIG_FONT] = font_idxs[UI_FONT];
		font_scales[CONFIG_FONT] = font_scales[UI_FONT];
	}

private:
	//! The list of known fonts
	std::vector<Font> _fonts;
	//! Lookup table mapping indices of fixed width fonts to indices in the _fonts array
	std::vector<size_t> _fixed_width_idxs;

	/*!
	 * Create a new font manager, without fonts to manage so far.
	 */
	FontManager(): _fonts(), _fixed_width_idxs() {}
	// Disallow copying, since this is a singleton class
	FontManager(const FontManager&) = delete;
	FontManager& operator=(const FontManager&) = delete;

	/*!
	 * Load a font.
	 *
	 * Get the font for font category \a cat. If this font fails to load,
	 * switch to fixed font 0;
	 *
	 * \param cat The font category for which to load a font.
	 * \return Reference to the font itself.
	 */
	Font& get(Category cat);
};

} // namespace eternal_lands

#endif // __cplusplus

#ifdef __cplusplus
extern "C"
{
#endif

//! The font numbers for each font category
extern size_t *font_idxs;
//! The zoom factor for each font category
extern float *font_scales;

#ifdef TTF
#define TTF_DIR_SIZE 256

//! Flag indicating whether to use TTF fonts or not
extern int use_ttf;
//! The path to search for TrueType fonts
extern char ttf_directory[TTF_DIR_SIZE];
#endif // TTF

int initialize_fonts();

size_t get_fixed_width_font_number(size_t idx);

int get_char_width_zoom(unsigned char c, font_cat cat, float zoom);
int get_max_char_width_zoom(font_cat cat, float zoom);
int get_avg_char_width_zoom(font_cat cat, float zoom);
int get_max_digit_width_zoom(font_cat cat, float zoom);
int get_buf_width_zoom(const unsigned char* str, size_t len, font_cat cat, float text_zoom);
static __inline__ int get_string_width_zoom(const unsigned char* str, font_cat cat,
	float text_zoom)
{
	return get_buf_width_zoom(str, strlen((const char*)str), cat, text_zoom);
}
int get_line_height(font_cat cat, float text_zoom);
void get_buf_dimensions(const unsigned char* str, size_t len, font_cat cat, float text_zoom,
	int *width, int *height);

int reset_soft_breaks(unsigned char *text, int len, int size, font_cat cat,
	float text_zoom, int width, int *cursor, float *max_line_width);
void put_small_colored_text_in_box_zoomed(unsigned char color,
	const unsigned char* text, int len, int width,
	unsigned char* buffer, float text_zoom);
static __inline__ void put_small_text_in_box_zoomed (const unsigned char* text,
	int len, int width, unsigned char* buffer, float text_zoom)
{
	put_small_colored_text_in_box_zoomed(c_grey1, text, len, width, buffer, text_zoom);
}

void draw_buf_zoomed_width_font_select(int x, int y, const unsigned char *text, size_t len,
	int max_width, int max_lines, float r, float g, float b, font_cat cat, float text_zoom,
	int sel_begin, int sel_end);
void draw_buf_zoomed_width_font(int x, int y, const unsigned char *text, size_t len,
	int max_width, int max_lines, font_cat cat, float text_zoom);
static __inline__ void draw_string_zoomed_width_font(int x, int y, const unsigned char *text,
	int max_width, int max_lines, font_cat cat, float text_zoom)
{
	draw_buf_zoomed_width_font(x, y, text, strlen((const char*)text),
		max_width, max_lines, cat, text_zoom);
}
void draw_string_zoomed_width_font_right(int x, int y, const unsigned char *text,
	int max_width, int max_lines, font_cat cat, float text_zoom);
void draw_string_zoomed_width_font_centered(int x, int y, const unsigned char *text,
	int max_width, int max_lines, font_cat cat, float text_zoom);
void draw_string_zoomed_centered_around(int x, int y,
	const unsigned char *text, int center_idx, float zoom);
static __inline__ void draw_string_zoomed(int x, int y, const unsigned char* text,
	int max_lines, float zoom)
{
	draw_string_zoomed_width_font(x, y, text, window_width, max_lines, UI_FONT, zoom);
}
static __inline__ void draw_string_zoomed_right(int x, int y, const unsigned char *text,
	int max_lines, float text_zoom)
{
	draw_string_zoomed_width_font_right(x, y, text, window_width, max_lines,
		UI_FONT, text_zoom);
}
static __inline__ void draw_string_zoomed_centered(int x, int y, const unsigned char *text,
	int max_lines, float text_zoom)
{
	draw_string_zoomed_width_font_centered(x, y, text, window_width, max_lines,
		UI_FONT, text_zoom);
}
static __inline__ void draw_string_zoomed_width(int x, int y, const unsigned char* text,
	int max_width, int max_lines, float zoom)
{
	draw_string_zoomed_width_font(x, y, text, max_width, max_lines, UI_FONT, zoom);
}
void draw_string_shadowed_zoomed(int x, int y, const unsigned char* text,
	int max_lines, float fr, float fg, float fb, float br, float bg, float bb,
	float text_zoom);
void draw_string_shadowed_zoomed_centered(int x, int y, const unsigned char* text,
	int max_lines, float fr, float fg, float fb, float br, float bg, float bb,
	float text_zoom);
void draw_string_shadowed_zoomed_right(int x, int y, const unsigned char* text,
	int max_lines, float fr, float fg, float fb, float br, float bg, float bb,
	float text_zoom);
void draw_string_shadowed_width(int x, int y, const unsigned char* text,
	int max_width, int max_lines, float fr, float fg, float fb,
	float br, float bg, float bb);
void draw_string_zoomed_ellipsis_font(int x, int y, const unsigned char *text,
	int max_width, int max_lines, font_cat cat, float text_zoom);

static __inline__ void draw_string_small_zoomed(int x, int y,
	const unsigned char* text, int max_lines, float zoom)
{
	draw_string_zoomed_width_font(x, y, text, window_width, max_lines,
		UI_FONT, zoom * DEFAULT_SMALL_RATIO);
}
static __inline__ void draw_string_small_zoomed_right(int x, int y,
	const unsigned char* text, int max_lines, float zoom)
{
	draw_string_zoomed_width_font_right(x, y, text, window_width, max_lines,
		UI_FONT, zoom * DEFAULT_SMALL_RATIO);
}
static __inline__ void draw_string_small_zoomed_centered(int x, int y,
	const unsigned char* text, int max_lines, float zoom)
{
	draw_string_zoomed_width_font_centered(x, y, text, window_width, max_lines,
		UI_FONT, zoom * DEFAULT_SMALL_RATIO);
}
static __inline__ void draw_string_small_zoomed_centered_around(int x, int y,
	const unsigned char *text, int center_idx, float zoom)
{
	draw_string_zoomed_centered_around(x, y, text, center_idx,
		zoom * DEFAULT_SMALL_RATIO);
}
static __inline__ void draw_string_small_shadowed_zoomed(int x, int y,
	const unsigned char* text, int max_lines, float fr, float fg, float fb,
	float br, float bg, float bb, float zoom)
{
	draw_string_shadowed_zoomed(x, y, text, max_lines, fr, fg, fb, br, bg, bb,
		zoom * DEFAULT_SMALL_RATIO);
}
static __inline__ void draw_string_small_shadowed_zoomed_right(int x, int y,
	const unsigned char* text, int max_lines, float fr, float fg, float fb,
	float br, float bg, float bb, float zoom)
{
	draw_string_shadowed_zoomed_right(x, y, text, max_lines,
		fr, fg, fb, br, bg, bb, zoom * DEFAULT_SMALL_RATIO);
}
static __inline__ void draw_string_small_shadowed_zoomed_centered(int x, int y,
	const unsigned char* text, int max_lines, float fr, float fg, float fb,
	float br, float bg, float bb, float zoom)
{
	draw_string_shadowed_zoomed_centered(x, y, text, max_lines,
		fr, fg, fb, br, bg, bb, zoom * DEFAULT_SMALL_RATIO);
}

void show_help_colored_scaled(const unsigned char *text, int x, int y,
	float r, float g, float b, float text_zoom);
void show_help_colored_scaled_centered(const unsigned char *text, int x, int y,
	float r, float g, float b, float text_zoom);
void show_help_colored_scaled_right(const unsigned char *text, int x, int y,
	float r, float g, float b, float text_zoom);
/*!
 * \ingroup font
 * \brief Shows a help message in small font
 *
 * Shows the help message \a text at position (\a x, \a y) using a small font
 * size. The message is drawn on a semi-transparent background.
 *
 * \param text   the help message to show
 * \param x      the x coordinate of the left of the help message
 * \param y      the y coordinate of the top of the help message
 * \param scale  the scale for the text size
 *
 * \callgraph
 */
static __inline__ void show_help(const char *text, int x, int y, float scale)
{
	show_help_colored_scaled((const unsigned char*)text, x, y, 1.0f, 1.0f, 1.0f,
		scale * DEFAULT_SMALL_RATIO);
}
/*!
 * \ingroup font
 * \brief Shows a help message in normal font
 *
 * Shows the help message \a text at position (\a x, \a y) using the normal
 * font size. The message is drawn on a semi-transparent background.
 *
 * \param text   the help message to show
 * \param x      the x coordinate of the left of the help message
 * \param y      the y coordinate of top of the help message
 * \param scale  the scale for the text size
 *
 * \callgraph
 */
static __inline__ void show_help_big(const char *text, int x, int y, float scale)
{
	show_help_colored_scaled((const unsigned char*)text, x, y, 1.0f, 1.0f, 1.0f, scale);
}

void draw_messages(int x, int y, text_message *msgs, int msgs_size, Uint8 filter,
	int msg_start, int offset_start, int cursor, int width, int height,
	font_cat cat, float text_zoom, select_info* select);
void draw_console_separator(int x_space, int y, int width, float zoom);
#ifdef ELC
#ifndef MAP_EDITOR2
void draw_ortho_ingame_string(float x, float y, float z,
	const unsigned char *text, int max_lines, float zoom_x, float zoom_y);
void draw_ingame_string(float x, float y, const unsigned char *text,
	int max_lines, float zoom_x, float zoom_y);
#endif // !MAP_EDITOR2
#endif // ELC

/*!
 * Set the config font
 *
 * Copy the current font index and scale of the UI_FONT category to that
 * of the CONFIG_FONT category. This allows us to change the UI font
 * without changing the way the options window is drawn, and hopefully
 * stops the user from then being unable to change the font back.
 */
void set_config_font();

int has_glyph(unsigned char c);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NEW_FONT_H

/*!
 * \file
 * \ingroup text_font
 * \brief Classes and functions for rendering text in different fonts
 */

#ifndef NEW_FONT_H
#define NEW_FONT_H

#define DEFAULT_FIXED_FONT_WIDTH  11
#define DEFAULT_FIXED_FONT_HEIGHT 18
#define SMALL_FIXED_FONT_WIDTH    8
#define SMALL_FIXED_FONT_HEIGHT   15
// This value of DEFAULT_SMALL_RATIO is a compromise between 8/11 = 0.7272 and 16/19=0.8421,
// which are the ratios of the widths and heights respectively between small and normal characters
// in the fixed font in earlier font handling.
#define DEFAULT_SMALL_RATIO       0.785f

#define INGAME_FONT_X_LEN       0.17f
#define SMALL_INGAME_FONT_X_LEN 0.12f
#define SMALL_INGAME_FONT_Y_LEN 0.17f
#define ALT_INGAME_FONT_X_LEN   0.10f
#define ALT_INGAME_FONT_Y_LEN   0.15f

/*!
 * \ingroup text_font
 * \brief Enumeration for font categories
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
	//! Index used for the font used to draw map marks
	MAPMARK_FONT,
	//! Index used for the font used to draw the options window contents
	CONFIG_FONT,
	//! Number of font categories
	NR_FONT_CATS
} font_cat;

//! Enumeration for horizontal text alignment
typedef enum
{
	//! Align text left to the given position
	LEFT,
	//! Center text around the given position
	CENTER,
	//! Align text right to the given position
	RIGHT
} hor_alignment;

//! Enumeration for vertical text alignment
typedef enum
{
	//! Align top of the line to the given position
	TOP_LINE,
	//! Align top of the font to the given position
	TOP_FONT,
	//! Align bottom of the line to the given position
	BOTTOM_LINE,
	//! Center text around the given position, works with multiple lines
	CENTER_LINE,
	//! Center first line around given position, assuming it consists of digits
	CENTER_DIGITS,
	//! Center first line around given position, assuming it consists of password asterisks
	CENTER_PASSWORD
} ver_alignment;

#include "gl_init.h"
#include "text.h"
#include "widgets.h"

#ifdef __cplusplus

#include <array>
#include <string>
#include <unordered_map>
#include <vector>
#include <SDL_types.h>
#ifdef TTF
#include <SDL_ttf.h>
#endif // TTF
#include "platform.h"

namespace eternal_lands
{

/*!
 * \ingroup text_font
 * \brief Type alias for a byte string, as used in many places in EL
 */
typedef std::basic_string<unsigned char> ustring;

/*!
 * \ingroup text_font
 * \brief Class for font options.
 *
 * Class FontOption holds the information necessary to display a font option to the user: the font
 * name, its file name, the type of font and whether it is a fixed width font.
 */
class FontOption
{
public:
	/*!
	 * \brief Constructor
	 *
	 * Create a new font option for the EL bundled font with index \a font_nr. Currently there are
	 * 7 such fonts available:
	 * - 0: textures/font.dds (fixed width)
	 * - 1: textures/font.dds (variable width)
	 * - 2: textures/font2.dds
	 * - 3: textures/font3.dds
	 * - 4: textures/font5.dds
	 * - 5: textures/font6.dds
	 * - 6: textures/font7.dds
	 *
	 * \param font_nr The number of the bundled font.
	 */
	FontOption(size_t font_nr);
#ifdef TTF
	/*!
	 * \brief Constructor
	 *
	 * Create a new font option for the TrueType font in file \a file_name.
	 *
	 * \param file_name The name of the file of the TrueType font.
	 */
    FontOption(const std::string& file_name);
#endif

	//! Return the font number of a bundled font
	size_t font_number() const { return _font_nr; }
	//! Return the file name of the font
    const std::string& file_name() const { return _file_name; }
    //! Return the display name of the font
    const std::string& font_name() const { return _font_name; }
#ifdef TTF
	//! Return whether the font is a TrueType font
    bool is_ttf() const { return _is_ttf; }
#endif
	//! Return whether the font is fixed width
    bool is_fixed_width() const { return _fixed_width; }
    //! Return \c true if the font file cannot be found, or the font is otherwise invalid
    bool failed() const { return _failed; }

	/*!
	 * \brief Add this font to the multi-selects
	 *
	 * Add this font option to the various font selections in the settings window. If the optional
	 * parameter \a add_button is \c true, a button for the font is immediately added to the
	 * corresponding widget.
	 */
	void add_select_options(bool add_button=false) const;

private:
	// Font number, only for bundled fonts
	size_t _font_nr;
	//! File name of the font file or texture image
    std::string _file_name;
	//! Return the display name of the font
    std::string _font_name;
#ifdef TTF
	//! \c true if this a a True Type font
    bool _is_ttf;
#endif
	//! \c true if this ais a monospaced font
    bool _fixed_width;
	//! \c true if the font file does not exist, or the font is otherwise invalid
    bool _failed;
};

/*!
 * \ingroup text_font
 * \brief Class for text drawing options.
 *
 * Class TextDrawOptions can hold options that specify how a text will be
 * displayed on the screen. Things that can be specified through TextDrawOptions
 * include:
 * * The size and color of the text to display
 * * The maximum width the text can occupy, and the maximum number of lines to draw,
 * * Alignment of the text: left, right, or centered around the given x position
 * * Whether or not to draw a semi-transparent background behind the text
 * * Whether to use ellipsis to indicate too wide text
 */
class TextDrawOptions
{
public:
	//! Default foreground color for text
	static const std::array<float, 3> default_foreground_color;
	//! Default background color for text
	static const std::array<float, 3> default_background_color;
	//! Default color for selected text
	static const std::array<float, 3> default_selection_color;

	typedef ::hor_alignment Alignment;
	typedef ::ver_alignment VerticalAlignment;

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
		ELLIPSIS = 1 << 3,
		//! If set, scale text down if it is too wide
		SHRINK_TO_FIT = 1 << 4
	};

	/*!
	 * \brief Constructor
	 *
	 * Create a new TextDrawOptions object, with default settings.
	 */
	TextDrawOptions();

	//! Return the maximum width the text can occupy
	int max_width() const { return _max_width; }
	//! Return the maximum number of text lines to draw
	int max_lines() const { return _max_lines; }
	//! Return the scale factor for the text
	float zoom() const { return _zoom; }
	//! Return the scale factor for the spacing between two lines
	float line_spacing() const { return _line_spacing; }
	//! Return the horizontal alignment of the text
	Alignment alignment() const { return _alignment; }
	//! Return the vertical alignment of the text
	VerticalAlignment vertical_alignment() const { return _vertical_alignment; }
	//! Return whether the text is drawn with a shadow background
	bool shadow() const { return _flags & SHADOW; }
	//! Return whether color characters in the text are ignored
	bool ignore_color() const { return _flags & IGNORE_COLOR; }
	//! Return whether this is a help text, with semi-transparent background
	bool is_help() const { return _flags & HELP; }
	//! Return whether to truncate wide text with ellipsis (...)
	bool ellipsis() const { return _flags & ELLIPSIS; }
	//! Return whether to shrink the text if it does not fit
	bool shrink_to_fit() const { return _flags & SHRINK_TO_FIT; }

	//! Return whether a valid foreground color was set for this text
	bool has_foreground_color() const { return _fg_color[0] >= 0.0; }
	//! Return whether a valid background color was set for this text
	bool has_background_color() const { return _bg_color[0] >= 0.0; }

	//! Set the maximum width (in pixels) the text can occupy to \a width
	TextDrawOptions& set_max_width(int width)
	{
		_max_width = width;
		return *this;
	}

	//! Set the maximum number of lines to draw to \a lines
	TextDrawOptions& set_max_lines(int lines)
	{
		_max_lines = lines;
		return *this;
	}

	//! Set the scale factor for the text to \a zoom
	TextDrawOptions& set_zoom(float zoom)
	{
		_zoom = zoom;
		return *this;
	}

	//! Multiply the current scale factor by \a scale
	TextDrawOptions& scale_zoom(float scale)
	{
		_zoom *= scale;
		return *this;
	}

	//! Set the scale factor for the line spacing to \a scale
	TextDrawOptions& set_line_spacing(float scale)
	{
		_line_spacing = scale;
		return *this;
	}

	//! Set the horizontal text alignment to \a alignment
	TextDrawOptions& set_alignment(Alignment alignment)
	{
		_alignment = alignment;
		return *this;
	}

	//! Set the vertical text alignment to \a alignment
	TextDrawOptions& set_vertical_alignment(VerticalAlignment alignment)
	{
		_vertical_alignment = alignment;
		return *this;
	}

	/*!
	 * \brief Set whether to draw a shadow in the background color around the text
	 * \sa set_background
	 */
	TextDrawOptions& set_shadow(bool shadow=true)
	{
		if (shadow)
			_flags |= SHADOW;
		else
			_flags &= ~SHADOW;
		return *this;
	}

	/*!
	 * \brief Set the foreground color of the text to (\a r, \a g, \a b).
	 *
	 * If not set, text is drawn in white, unless ignore_color is set, in which
	 * case the current OpenGL color is used.
	 * \sa set_ignore_color
	 */
	TextDrawOptions& set_foreground(float r, float g, float b)
	{
		_fg_color[0] = r;
		_fg_color[1] = g;
		_fg_color[2] = b;
		return *this;
	}

	/*!
	 * \brief  Set the background (shadow) color of the text to (\a r, \a g, \a b).
	 *
	 * If not set, shadows are drawn in black.
	 *
	 * \sa set_shadow
	 */
	TextDrawOptions& set_background(float r, float g, float b)
	{
		_bg_color[0] = r;
		_bg_color[1] = g;
		_bg_color[2] = b;
		return *this;
	}

	//! Set whether to ignore color characters in the text
	TextDrawOptions& set_ignore_color(bool ignore=true)
	{
		if (ignore)
			_flags |= IGNORE_COLOR;
		else
			_flags &= ~IGNORE_COLOR;
		return *this;
	}

	/*!
	 * \brief Set whether this is a help text.
	 *
	 * Help texts are drawn with a semi-transparent background behind the text,
	 * as if they are on a window.
	 */
	TextDrawOptions& set_help(bool is_help=true)
	{
		if (is_help)
			_flags |= HELP;
		else
			_flags &= ~HELP;
		return *this;
	}

	//! Set whether to ellipsis (...) should be added to clipped text
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

	//! Set whether to shrink the text if it is too wide
	TextDrawOptions& set_shrink_to_fit(bool shrink=true)
	{
		if (shrink)
			_flags |= SHRINK_TO_FIT;
		else
			_flags &= ~SHRINK_TO_FIT;
		return *this;
	}

	//! Set the current draw color to the foreground color in these options
	void use_foreground_color() const;
	//! Set the current draw color to the background color in these options
	void use_background_color() const;
	//! Set the current draw color to the selection color in these options
	void use_selection_color() const;

private:
	//! The maximum width of the text
	int _max_width;
	//! The maximum number of lines to draw
	int _max_lines;
	//! The scale factor for the text size
	float _zoom;
	//! The scale factor for the spacing between lines
	float _line_spacing;
	//! The horizontal alignment of the text
	Alignment _alignment;
	//! The vertical alignment of the text
	VerticalAlignment _vertical_alignment;
	//! Bit flags for further options
	Uint32 _flags;
	//! The foreground color of the text
	std::array<float, 3> _fg_color;
	//! The shadow color of the text, if shadows are drawn
	std::array<float, 3> _bg_color;
	//! The color for selected text
	std::array<float, 3> _sel_color;
};

/*!
 * \ingroup text_font
 * \brief A class for a text font
 *
 * Class Font holds information about text fonts, and provides functions to
 * draw text in a certain font. A font can be either one of the fonts bundled
 * with the game, or if the TTF compiled option is enabled, a TrueType font
 * on the user's system.
 */
class Font
{
public:
	/*!
	 * Move constructor
	 *
	 * Create a new font as a copy of \a font.
	 * \note This constructor does takes ownership of the font texture; the texture in \a font
	 * is invalidated.
	 *
	 * \param font The font to move into this font
	 */
	Font(Font&& font);
	/*!
	 * \brief Create a new font
	 *
	 * Initialize a new internal font. This sets the parameters for the bundled font with the font
	 * number stored in \a option, but does not yet load the font texture because it is called
	 * before OpenGL is initialized.
	 *
	 * \param option  The font option for the font to load, holding font number, file and font name
	 */
	Font(const FontOption& option);
#ifdef TTF
	/*!
	 * \brief Create a new font.
	 *
	 * Create a new font for the TTF font option in \a option, for text with line height \a height
	 * pixels. This only copies the font description, and determines the point size. It does not
	 * yet generate a texture.
	 *
	 * \param option The font option for the font, holding file and font name
	 * \param height The line height of the opened font
	 */
	Font(const FontOption& option, int height);
#endif
	//! Destructor
	~Font();

	//! Return the name of the font
	const std::string& font_name() const { return _font_name; }
	//! Return the file name from which the font was loaded
	const std::string& file_name() const { return _file_name; }
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
	 * \brief Check if a character has a glyph.
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
	 * \brief Get the width of a character
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
	 * \brief Get the width of a character plus spacing
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
	 * \brief Get the maximum character width, plus spacing
	 *
	 * Get the maximum width of a single character in this font when drawn at zoom
	 * level \a zoom, and include the spacing between characters.
	 *
	 * \param zoom The zoom factor for drawing the character.
	 * \return The maximum width of a character including spacing, in pixels.
	 */
	int max_width_spacing(float zoom=1.0) const;
	/*!
	 * \brief Get the average character width, plus spacing
	 *
	 * Get an approximation to the average width of a character in this font when
	 * drawn at zoom level \a zoom, including the space between characters,
	 * for some definition of "average". The average is calculated by assigning
	 * a weight to each possible character, and taking the weighted average
	 * of all character widths. The weights are based on a study on normal
	 * English text, so use this function with the following caveats:
	 * 1. Relative frequencies are based on English text, and so may not be
	 *    applicable to text in to other languages,
	 * 2. Furthermore, weights are based on text from the New York Times, and so
	 *    may not be representative even for English text in EL,
	 * 3. Only ASCII characters were counted in the study (so accented characters
	 *    don't contribute), and a few ASCII characters are missing as well
	 *    (\c '[', \c '\\', \c ']', \c '^', \c '_', and \c '`' to be exact),
	 * 4. The occurance of the space character was guesstimated from the reported
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
	 * \brief Get the maximum width of a digit, plus spacing
	 *
	 * Get the maximum width of a single digit character (0-9) in this font when
	 * drawn at zoom level \a zoom, and include the spacing between characters.
	 *
	 * \param zoom The zoom factor for drawing the character.
	 * \return The maximum width of a digit character including spacing, in pixels.
	 */
	int max_digit_width_spacing(float zoom=1.0) const;
	/*!
	 * \brief Calculate the width of a string
	 *
	 * Calculate the width in pixels of the string \a text of length \a len
	 * when drawn in this font with scale factor zoom.
	 * \note This function assumes the string is a single line. Newline
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
	 * \brief Calculate the width of a string with final spacing
	 *
	 * Calculate the width in pixels of the string \a text of length \a len,
	 * including the spacing after the final character, when drawn in this font
	 * with scale factor \a zoom.
	 * \note This function assumes the string is a single line. Newline
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
	 * \brief Calculate the width of a string with final spacing
	 *
	 * Calculate the width in pixels of the string \a text including the spacing
	 * after the final character, when drawn in this font with scale factor \a zoom.
	 * \note This function assumes the string is a single line. Newline
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
	 * \brief Get the height of a line.
	 *
	 * Get the height of a single line of text when drawing text at zoom level \a zoom.
	 * \note It is not necessarily the case that two consecutive lines of text consume twice
	 * the line height, the use of the bundled fonts (where the line height is greater than the
	 * spacing) or a custom line spacing may prevent that. Use dimensions() or vertical_advance()
	 * if you need to know the height of a text consiting of multiple lines.
	 *
	 * \param zoom The zoom factor for drawing the text
	 */
	int height(float zoom=1.0) const;
	/*!
	 * \brief Get the vertical advancement after a line.
	 *
	 * Get the number of pixels the pen is advanced in the vertical direction after drawing a
	 * line of text at zoom level \a zoom, with line spacing scale factor \a line_spacing.
	 *
	 * \param zoom         The zoom factor for drawing the text
	 * \param line_spacing The additional scale factor for the spacing between lines
	 */
	int vertical_advance(float zoom=1.0, float line_spacing=1.0) const;
	/*!
	 * \brief Calculate the number of lines that fit.
	 *
	 * Return the maximum number of lines that fit in a window of \a max_height pixels high, when
	 * drawing text in this font at zoom level \a zoom, and line spacing scale factor
	 * \a line_spacing.
	 *
	 * \param max_height   The maximum height of the text, in pixels
	 * \param zoom         The zoom factor for drawing the text
	 * \param line_spacing The additional scale factor for the spacing between lines
	 */
	int max_nr_lines(int max_height, float zoom, float line_spacing=1.0) const;
	/*!
	 * \brief Compute the height of a block of text
	 *
	 * Compute the height of a block of text of \a nr_lines lines, when drawn in this font
	 * at zoom level \a zoom, with line spacing scale factor \a line_spacing.
	 *
	 * \param nr_lines     The number of lines in the text
	 * \param text_zoom    The zoom factor for drawing the text
	 * \param line_spacing The additional scale factor for the spacing between lines
	 * \return The height of the text in pixels
	 */
	int text_height(int nr_lines, float zoom, float line_spacing=1.0);
	/*!
	 * \brief Calculate the dimensions of a block of text
	 *
	 * Calculate the width and height of string \a text of length \a len bytes when drawn in this
	 * font with scale factor \a zoom and with line spacing scale factor \a line_spacing. The
	 * string may contain multiple lines, the width returned is then the width of the widest line.
	 *
	 * \param text         The string for which to compute the dimensions
	 * \param len          The number of bytes in \a text
	 * \param zoom         The scale factor for the text
	 * \param line_spacing The additional scale factor for the spacing between lines
	 * \return The width and height of the text
	 */
	std::pair<int, int> dimensions(const unsigned char* text, size_t len, float zoom,
		float line_spacing=1.0) const;

	/*!
	 * \brief Calculate vertical offset of center
	 *
	 * Calculate the offset of the center of the characters in \a text with respect to the center
	 * of the line, when drawn at zoom level \a zoom.
	 *
	 * \param text The string for which to compute center offset
	 * \param len  The number of bytes in \a text
	 * \param zoom The scale factor for the text
	 * \return The number of pixels between the center of the line and the center of \a text.
	 */
	int center_offset(const unsigned char* text, size_t len, float zoom);
	/*!
	 * \brief Get vertical coordinates
	 *
	 * Get the minimum and maximum offsets of the characters in \a text with respect to the top of
	 * the text line, when drawing the text at zoom level \a zoom.
	 *
	 * \param text The text to get the vertical offsets for
	 * \param len  The number of characters in \a text
	 * \param zoom The scale factor for the text
	 * \return The offsets of the top and bottom of the text
	 */
	std::pair<int, int> top_bottom(const unsigned char* text, size_t len, float zoom);

	/*!
	 * \brief Recompute where the line breaks in a string should occur
	 *
	 * Recomputes the positions in string \a text where line breaks should be placed so that the
	 * string fits into a window. This creates a new string from the the contents of \a text and
	 * inserts \c '\\r' characters at the positions where the line should be broken. The parameter
	 * \a options specifies how the text will be drawn, currently only the \c zoom and \c max_width
	 * fields are used in this function. The optional parameter \a cursor contains the offset of
	 * the cursor position in the text; if it is non-negative the difference in cursor position
	 * will be returned through the third value in the result tuple. The optional parameter
	 * \a max_line_width can be used to retrieve the largest width in pixels of the lines in \a text
	 * after rewrapping.
	 *
	 * \param text       the string
	 * \param text_len   the actual length of the string
	 * \param options    the options defining the layout of the text
	 * \param cursor     the cursor position, or a negative number if not used
	 * \param max_line_width pointer the maximum line length after wrapping, or NULL if not used
	 *
	 * \return The wrapped text, the new number of lines in the text, and the difference in
	 * 	cursor position
	 */
	std::tuple<ustring, int, int> reset_soft_breaks(const unsigned char *text, size_t text_len,
		const TextDrawOptions& options, ssize_t cursor = -1, float *max_line_width = 0);
	/*!
	 * \brief Recompute where the line breaks in a string should occur
	 *
	 * Recomputes the positions in string \a text where line breaks should be
	 * placed so that the string fits into a window. This creates a new string
	 * from the the contents of \a text and inserts \c '\\r' characters at the
	 * positions where the line should be broken. The parameter \a options
	 * specifies how the text will be drawn, currently only the \c zoom and
	 * \c max_width field are used int this function.
	 *
	 * \param text       the string
	 * \param options    the options defining the layout of the text
	 *
	 * \return The wrapped text and the new number of lines in the text.
	 */
	std::pair<ustring, int> reset_soft_breaks(const ustring& text, const TextDrawOptions& options)
	{
		auto res = reset_soft_breaks(text.data(), text.length(), options, -1, nullptr);
		return std::make_pair(std::get<0>(res), std::get<1>(res));
	}

	/*!
	 * \brief Draw a text string
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
	 * \brief Draws messages in a buffer to the screen
	 *
	 * Draws the messages in buffer \a msgs to the screen, starting with character
	 * \a offset_start in message number \a msg_start.
	 *
	 * \param msgs         the message buffer
	 * \param msgs_size    the total number of messages that \a msgs can hold
	 * \param x            x coordinate of the position to start drawing
	 * \param y            y coordinate of the position to start drawing
	 * \param filter       draw only messages in channel \a filter. Choose
	 * 	FILTER_ALL for displaying all messages
	 * \param msg_start    index of the first message to display
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
		const TextDrawOptions &options, ssize_t cursor, select_info* select) const;
	/*!
	 * \brief Draw the console separator
	 *
	 * When there are more messages than can fit on a single screen, the user can
	 * scroll up to see the previous messages. When they do, a separator is
	 * drawn by this function to indicate that more messages follow.
	 *
	 * \param x_space Horizontal space tokeep free on either side
	 * \param y       Vertical position at which the line should be drawn
	 * \param options Text options for drawing the separator
	 */
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
	//! Structure for glyph metrics
	struct Metrics
	{
		//! Actual width of the character in the font texture, in pixels
		int width;
		//! How far to advance the pen after drawing this glyph
		int advance;
		//! Offset from top of line to top of glyph
		int top;
		//! Offset from top of line to bottom of glyph
		int bottom;
		//! Left side of glyph in the texture
		float u_start;
		//! Top of glyph in the texture
		float v_start;
		//! Right side of glyph in the texture
		float u_end;
		//! Bottom of glyph in the texture
		float v_end;

		//! Default constructor
		Metrics(): width(0), advance(0), top(0), bottom(0), u_start(0.0), v_start(0.0),
			u_end(0.0), v_end(0.0) {}
	};

	//! The number of lines in a font texture
	static const size_t font_nr_lines = 10;
	//! The number of glyphs on a single line in a font texture
	static const size_t font_chars_per_line = 14;
	//! The total number of different glyphs we recognise
	static const size_t nr_glyphs = 9*14 + 6;
	//! Horizontal space in the texture for a single glyph in a bundled font
	static const int font_block_width = 18;
	//! Vertical space in the texture for a single glyph in a bundled font
	static const int font_block_height = 21;
	//! Normal line height for unscaled text in a bundled font
	static const int default_vertical_advance = 18;
#ifdef TTF
	//! Point size with which TrueType fonts are opened
	static const int ttf_point_size = 40;
#endif
	//! Relative frequencies for characters in normal English text
	static const std::array<int, nr_glyphs> letter_freqs;
	//! Ellipsis string for clipped text
	static const ustring ellipsis;

	//! Flags indicating the status and properties of a font
	enum Flags
	{
#ifdef TTF
		//! Set if the font is a TTF Font
		IS_TTF       = 1 << 0,
#endif
		//! Set if the font is fixed width, i.e. all characters have the same width
		FIXED_WIDTH  = 1 << 1,
		//! Set if the texture for the font is loaded or generated
		HAS_TEXTURE  = 1 << 2,
		//! Set if loading the font failed, and a fallback should be used
		FAILED       = 1 << 3
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
	//! Glyph metrics for each supported character
	std::array<Metrics, nr_glyphs> _metrics;
	//! Width reserved for a character in the texture
	int _block_width;
	//! Height of a single character in pixels
	int _line_height;
	//! How far to advance the pen vertically when moving to the next line
	int _vertical_advance;
	//! Distance from top of line to top of font
	int _font_top_offset;
	//! Distance from top of line to center of digits
	int _digit_center_offset;
	//! Distance from top of line to center of the asterisk
	int _password_center_offset;
	//! Maximum width of a glyph
	int _max_advance;
	//! Maximum width of a digit 0-9
	int _max_digit_advance;
	//! "Typical" character width for English text
	int _avg_advance;
	//! Distance between characters when drawn (at default zoom level)
	int _spacing;
	//! Scale factor in the horizontal direction
	float _scale_x;
	//! Scale factor in the vertical direction
	float _scale_y;
#ifdef TTF
	//! Point size to open the font file with
	int _point_size;
	//! Outline size in pixels
	int _outline;
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
	 * \brief Get the position of a glyph in the texture
	 *
	 * \param c The byte for which to get the position
	 * \return The position in the font texture, of -1 if \a c is not a valid glyph.
	 */
	static int get_position(unsigned char c);
	/*!
	 * \brief Get the width of a character
	 *
	 * Get the width of the glyph at position \a pos in the texture when drawn
	 * in this font at zoom level \a zoom.
	 *
	 * \param pos  The position of the glyph in the texture
	 * \param zoom The zoom factor for drawing the glyph
	 */
	int width_pos(int pos, float zoom=1.0) const;
	/*!
	 * \brief Get the width of a character plus spacing
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
	 * \brief Get vertical coordinates
	 *
	 * Get the minimum and maximum offsets of the characters in \a text with respect to the top of
	 * the text line, before any scaling.
	 *
	 * \param text The text to get the vertical offsets for
	 * \param len  The number of characters in \a text
	 * \return The offsets of the top and bottom of the text
	 */
	std::pair<int, int> top_bottom_unscaled(const unsigned char* text, size_t len);

	/*!
	 * \brief Load or generate the texture for this font.
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
	 * \brief Get texture coordinates for a character
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
	 * \brief Set the current draw color.
	 *
	 * Set the drawing color for the text to the color define in colors_list
	 * at index \a color.
	 *
	 * \param color The index of the text color in the \see colors_list.
	 */
	static void set_color(int color);

	/*!
	 * \brief Draw a single character
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
	 * \brief Draw background for help texts
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
	 * \brief Draw a single line of text
	 *
	 * Draw a single line of text in \a text of length \a len bytes, starting
	 * at position \a x, \a y and drawing to the left.
	 * \note This function is for drawing a single line. Any newlines in \a text
	 * are ignored.
	 * \note The alignment option in \a options has no effect for this function,
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
	 * \brief Clip a line of text
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
	 * \brief Render a single glyph
	 *
	 * Render the glyph for position \a pos with size \a size in the font atlas \a surface, using
	 * font \a font. The vertical offset parameter \a y_delta is used to try and center the glyphs
	 * vertically, making vertical alignment of text easier.
	 *
	 * \param pos          The position of the glyph to draw
	 * \param size         The size of the rendered glyph
	 * \param y_delta      Vertical offset for placing the glyph in the texture
	 * \param font         The font with which to render the glyph
	 * \param outline_size If positive, the width of a semi-transparent outline around the glyph
	 * \param surface      The surface on which the glyph is rendered
	 * \return \c true if the glyph was successfully rendered, \c false otherwise.
	 */
	bool render_glyph(size_t i_glyph, int size, int y_delta, int outline_size, TTF_Font *font,
		SDL_Surface *surface);
	/*!
	 * \brief Find an appropriate font size
	 *
	 * Find a point size for this font such that the line height is \a height pixels.
	 *
	 * \param height The line height to target
	 * \return A point size for this font which gives the correct line height, or 0 on failure
	 */
	int find_point_size(int height);
	/*!
	 * \brief Build a texture for a TTF font
	 *
	 * Build a texture containing all supported glyphs from the TrueType font
	 * associated with this font.
	 *
	 * \return \c true on succes, \c false on failure.
	 */
	bool build_texture_atlas();
#endif // TTF

	/*!
	 * \brief Calculate the average character width.
	 *
	 * Calculate a weighted average character width based on the widths of the characters and
	 * estimated letter frequencies in English text.
	 *
	 * \return The average width
	 */
	int calc_average_advance();
};

/*!
 * \brief Class for managing fonts
 *
 * Class FontManager is used as an entry point to font handling. It holds a list
 * of all known fonts, as well as a font number and scale factor for a number
 * of font categories. The font categories are used for drawing different kinds
 * of texts; by changing the font for a certain category all windows and widgets
 * using this category are updated automatically. Class FontManager is a singleton
 * class: all access to this class must go through get_instance().
 * \sa font_cat
 */
class FontManager
{
public:
	// Use font category enumeration shared with C code.
	typedef ::font_cat Category;

	//! The font numbers for each font category
	static std::array<size_t, NR_FONT_CATS> font_idxs;
	//! The zoom factor for each font category
	static std::array<float, NR_FONT_CATS> font_scales;

	//! Get the singleton FontManager instance
	static FontManager& get_instance()
	{
		static FontManager manager;
		return manager;
	}

	//! Check if this font manager has been initialized
	bool is_initialized() const { return !_options.empty(); }
	/*!
	 * \brief Initialize the font manager.
	 *
	 * This adds the standard fonts bundled with EL to the font manager, and
	 * if TTF fonts are enabled, scans the TTF path for True Type fonts
	 * to add as well.
	 */
	bool initialize();

	/*!
	 * \brief Return the font number of a fixed width font
	 *
	 * Return the font number for the \a idx'th fixed width font.
	 *
	 * \param idx The number of the font in the list of fixed width fonts only
	 * \return The index of the font in the list of all fonts
	 */
	size_t fixed_width_font_number(size_t idx) const
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
	 * \brief The width of a single character
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
		return get(cat, text_zoom).width_spacing(c, text_zoom * font_scales[cat]);
	}
	/*!
	 * \brief The maximum width of a single character
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
		return get(cat, text_zoom).max_width_spacing(text_zoom * font_scales[cat]);
	}
	/*!
	 * \brief Get the average character width, plus spacing
	 *
	 * Get an approximation to the average width of a character in the font for
	 * category \a cat when drawn at zoom level \a text_zoom, including the space
	 * between characters, for some definition of "average". See the note
	 * in Font::average_width_spacing() for some caveats when using this function.
	 *
	 * \param cat       The font category for the font used
	 * \param text_zoom The zoom factor for drawing the character.
	 * \return The maximum width of a character including spacing, in pixels.
	 */
	int average_width_spacing(Category cat, float text_zoom=1.0)
	{
		return get(cat, text_zoom).average_width_spacing(text_zoom * font_scales[cat]);
	}
	/*!
	 * \brief The maximum width of a single digit character
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
		return get(cat, text_zoom).max_digit_width_spacing(text_zoom * font_scales[cat]);
	}
	/*!
	 * \brief Calculate the width of a string
	 *
	 * Calculate the width in pixels of the string \a text of length \a len
	 * when drawn in the font for category \a cat.
	 * \note This function assumes the string is a single line. Newline
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
		return get(cat, text_zoom).line_width(text, len, text_zoom * font_scales[cat]);
	}
	/*!
	 * \brief The height of a text line
	 *
	 * Return the height of a line of text when drawn in the font for category
	 * \a cat with scale factor \a text_zoom.
	 * \note It is not necessarily the case that two consecutive lines of text consume twice
	 * the line height, the use of the bundled fonts (where the line height is greater than the
	 * spacing) or a custom line spacing may prevent that. Use dimensions() or vertical_advance()
	 * if you need to know the height of a text consiting of multiple lines.
	 *
	 * \param cat       The font category for the font used
	 * \param text_zoom The scale factor for the text
	 * \return The height of the text in pixels
	 */
	int line_height(Category cat, float text_zoom=1.0)
	{
		return get(cat, text_zoom).height(text_zoom * font_scales[cat]);
	}
	/*!
	 * \brief Get the vertical advancement after a line.
	 *
	 * Get the number of pixels the pen is advanced in the vertical direction after drawing a
	 * line of text at zoom level \a text_zoom, with line spacing scale factor \a line_spacing.
	 *
	 * \param cat          The font category for the font used
	 * \param text_zoom    The zoom factor for drawing the text
	 * \param line_spacing The additional scale factor for the spacing between lines
	 * \return The number of pixels the pen is shifted down after a line
	 */
	int vertical_advance(Category cat, float text_zoom=1.0, float line_spacing=1.0)
	{
		return get(cat, text_zoom).vertical_advance(text_zoom * font_scales[cat], line_spacing);
	}
	/*!
	 * \brief Calculate the number of lines that fit.
	 *
	 * Return the maximum number of lines that fit in a window of \a max_height pixels high, when
	 * drawing text in the font for category \a cat at zoom level \a text_zoom, with line spacing
	 * scale factor \a line_spacing.
	 *
	 * \param cat          The font category for the font used
	 * \param max_height   The maximum height for the text, in pixels
	 * \param text_zoom    The zoom factor for drawing the text
	 * \param line_spacing The additional scale factor for the spacing between lines
	 * \return The maximum number of lines that fit
	 */
	int max_nr_lines(Category cat, int max_height, float text_zoom, float line_spacing=1.0)
	{
		return get(cat, text_zoom).max_nr_lines(max_height, text_zoom * font_scales[cat], line_spacing);
	}
	/*!
	 * \brief Compute the height of a block of text
	 *
	 * Compute the height of a block of text of \a nr_lines lines, when drawn in the font for
	 * category \a cat at zoom level \a zoom, with line spacing scale factor \a line_spacing.
	 *
	 * \param cat          The font category for the font used
	 * \param nr_lines     The number of lines in the text
	 * \param text_zoom    The zoom factor for drawing the text
	 * \param line_spacing The additional scale factor for the spacing between lines
	 * \return The height of the text in pixels
	 */
	int text_height(font_cat cat, int nr_lines, float text_zoom, float line_spacing=1.0)
	{
		return get(cat, text_zoom).text_height(nr_lines, text_zoom * font_scales[cat], line_spacing);
	}
	/*!
	 * \brief Calculate the dimensions of a block of text
	 *
	 * Calculate the width and height of string \a text of length \a len bytes when drawn in the
	 * font for category \a cat, with scale factor \a zoom and line spacing scale factor
	 * \a line_spacing. The string may contain multiple lines, the width returned is then the
	 * width of the widest line.
	 *
	 * \param cat          The font category for the font used
	 * \param text         The string for which to compute the dimensions
	 * \param len          The number of bytes in \a text
	 * \param text_zoom    The scale factor for the text
	 * \param line_spacing The additional scale factor for the spacing between lines
	 * \return The width and height of the text
	 */
	std::pair<int, int> dimensions(Category cat, const unsigned char* text, size_t len,
		float text_zoom, float line_spacing=1.0)
	{
		return get(cat, text_zoom).dimensions(text, len, text_zoom * font_scales[cat], line_spacing);
	}

	/*!
	 * \brief Calculate vertical offset of center
	 *
	 * Calculate the offset of the center of the characters in \a text with respect to the center
	 * of the line, when drawn in the font for category \a cat at zoom level \a zoom.
	 *
	 * \param cat       The font category for the font used
	 * \param text      The string for which to compute center offset
	 * \param len       The number of bytes in \a text
	 * \param text_zoom The scale factor for the text
	 * \return The number of pixels between the center of the line and the center of \a text.
	 */
	int center_offset(Category cat, const unsigned char* text, size_t len, float text_zoom)
	{
		return get(cat, text_zoom).center_offset(text, len, text_zoom * font_scales[cat]);
	}
	/*!
	 * \brief Get vertical coordinates
	 *
	 * Get the minimum and maximum offsets of the characters in \a text with respect to the top of
	 * the text line, when drawing the text in the font for category \a cat at zoom level \a zoom.
	 *
	 * \param cat       The font category for the font used
	 * \param text      The text to get the vertical offsets for
	 * \param len       The number of characters in \a text
	 * \param text_zoom The scale factor for the text
	 * \return The offsets of the top and bottom of the text
	 */
	std::pair<int, int> top_bottom(Category cat, const unsigned char* text, size_t len, float text_zoom)
	{
		return get(cat, text_zoom).top_bottom(text, len, text_zoom * font_scales[cat]);
	}


	/*!
	 * \brief Recompute where the line breaks in a string should occur
	 *
	 * Recomputes the positions in string \a text where line breaks should be placed so that the
	 * string fits into a window. This creates a new string from the the contents of \a text and
	 * inserts \c '\\r' characters at the positions where the line should be broken. The parameter
	 * \a options specifies how the text will be drawn, currently only the \c zoom and \c max_width
	 * fields are used in this function. The optional parameter \a cursor contains the offset of
	 * the cursor position in the text; if it is non-negative the difference in cursor position
	 * will be returned through the third value in the result tuple. The optional parameter
	 * \a max_line_width can be used to retrieve the largest width in pixels of the lines in \a text
	 * after rewrapping.
	 *
	 * \param cat        the font category in which the string is to be drawn
	 * \param text       the string
	 * \param text_len   the actual length of the string
	 * \param options    the options defining the layout of the text
	 * \param cursor     pointer to the cursor position, or NULL if not used
	 * \param max_line_width pointer the maximum line length after wrapping, or NULL if not used
	 *
	 * \return The wrapped text, the new number of lines in the text, and the difference in
	 * 	cursor position
	 */
	std::tuple<ustring, int, int> reset_soft_breaks(Category cat, const unsigned char *text,
		size_t text_len, const TextDrawOptions& options, int cursor = -1, float *max_line_width = nullptr)
	{
		TextDrawOptions cat_options = TextDrawOptions(options).scale_zoom(font_scales[cat]);
		return get(cat, options.zoom())
			.reset_soft_breaks(text, text_len, cat_options, cursor, max_line_width);
	}
	/*!
	 * \brief Recompute where the line breaks in a string should occur
	 *
	 * Recomputes the positions in string \a text where line breaks should be
	 * placed so that the string fits into a window. This creates a new string
	 * from the the contents of \a text and inserts \c '\\r' characters at the
	 * positions where the line should be broken. The parameter \a options
	 * specifies how the text will be drawn, currently only the \c zoom and
	 * \c max_width field are used int this function.
	 *
	 * \param cat        the font category in which the string is to be drawn
	 * \param text       the string
	 * \param options    the options defining the layout of the text
	 *
	 * \return The wrapped text, the new number of lines in the text, and the difference in
	 * 	cursor position
	 */
	std::pair<ustring, int> reset_soft_breaks(Category cat, const ustring& text,
		const TextDrawOptions& options)
	{
		TextDrawOptions cat_options = TextDrawOptions(options).scale_zoom(font_scales[cat]);
		return get(cat, options.zoom()).reset_soft_breaks(text, cat_options);
	}

	/*!
	 * \brief Draw a text string
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
		get(cat, options.zoom()).draw(text, len, x, y, cat_options, sel_begin, sel_end);
	}
	/*!
	 * \brief Draws messages in a buffer to the screen
	 *
	 * Draws the messages in buffer \a msgs to the screen using the font for
	 * category \a cat, starting with character \a offset_start in message
	 * number \a msg_start.
	 *
	 * \param cat          The font category for the text
	 * \param msgs         the message buffer
	 * \param msgs_size    the total number of messages that \a msgs can hold
	 * \param x            x coordinate of the position to start drawing
	 * \param y            y coordinate of the position to start drawing
	 * \param filter       draw only messages in channel \a filter. Choose
	 * 	FILTER_ALL for displaying all messages
	 * \param msg_start    index of the first message to display
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
		const TextDrawOptions &options, ssize_t cursor, select_info* select)
	{
		TextDrawOptions cat_options = TextDrawOptions(options).scale_zoom(font_scales[cat]);
		get(cat, options.zoom()).draw_messages(msgs, msgs_size, x, y, filter, msg_start, offset_start,
			cat_options, cursor, select);
	}
	/*!
	 * \brief Draw the console separator
	 *
	 * When there are more messages than can fit on a single screen, the user can
	 * scroll up to see the previous messages. When they do, a separator is
	 * drawn by this function to indicate that more messages follow. The font
	 * category should normally be CHAT_FONT, as it is only used in the console
	 * window.
	 *
	 * \param cat     The font category for the separator
	 * \param x_space Horizontal space to keep free on either side
	 * \param y       Vertical position at which the line should be drawn
	 * \param options Text options for drawing the separator
	 */
	void draw_console_separator(Category cat, int x_space, int y,
		const TextDrawOptions& options)
	{
		TextDrawOptions cat_options = TextDrawOptions(options).scale_zoom(font_scales[cat]);
		get(cat, options.zoom()).draw_console_separator(x_space, y, cat_options);
	}
#ifdef ELC
#ifndef MAP_EDITOR2
	void draw_ortho_ingame_string(const unsigned char* text, size_t len,
		float x, float y, float z, int max_lines, float zoom_x, float zoom_y)
	{
		get(NAME_FONT, zoom_y).draw_ortho_ingame_string(text, len, x, y, z, max_lines,
			zoom_x * font_scales[NAME_FONT], zoom_y * font_scales[NAME_FONT]);
	}
	void draw_ingame_string(const unsigned char* text, size_t len,
		float x, float y, int max_lines, float zoom_x, float zoom_y)
	{
		get(CHAT_FONT, zoom_y).draw_ingame_string(text, len, x, y, max_lines,
			zoom_x * font_scales[CHAT_FONT], zoom_y * font_scales[CHAT_FONT]);
	}
#endif // !MAP_EDITOR_2
#endif // ELC

	/*!
	 * \brief Set the config font
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

#ifdef TTF
	//! Disable TrueType fonts, allowing bundled fonts only.
	void disable_ttf();
	//! Enable TrueTyoe fonts, restoring previous font indices if possible
	void enable_ttf();
#endif

private:
	//! The number of fonts bundled with EL itself
	static const size_t _nr_bundled_fonts = 7;
	//! The fonts to use by default for each category
	static const std::array<size_t, NR_FONT_CATS> _default_font_idxs;

	//! The list of known font options
	std::vector<FontOption> _options;
	//! Map from font number and line height to actual font
	std::unordered_map<uint32_t, Font> _fonts;
	//! Lookup table mapping indices of fixed width fonts to indices in the _fonts array
	std::vector<size_t> _fixed_width_idxs;
#ifdef TTF
	/*!
	 * When disabling TrueType fonts, the current font file names are stored in this
	 * array. If TTF support is enabled again in the same session, they can be
	 * restored again without the user having to search the fonts in the options
	 * window.
	 */
	std::array<std::string, NR_FONT_CATS> _saved_font_files;
#endif

	/*!
	 * \brief Constructor
	 *
	 * Create a new font manager, without fonts to manage so far.
	 */
	FontManager(): _options(), _fonts(), _fixed_width_idxs()
#ifdef TTF
		, _saved_font_files()
#endif
		{}
	// Disallow copying, since this is a singleton class
	FontManager(const FontManager&) = delete;
	FontManager& operator=(const FontManager&) = delete;

	/*!
	 * \brief Initialize TrueType fonts
	 *
	 * Scan the TTF font directory for TrueType fonts, and add them to the
	 * current list of fonts.
	 */
	void initialize_ttf();
	/*!
	 * \brief Scan for TrueType fonts
	 *
	 * Scan for files matching the glob pattern \a pattern, and if they contain
	 * a TTF font that supports our character set, add it to our list of known
	 * fonts.
	 *
	 * \param pattern A glob pattern defining the search path
	 */
	 void add_ttf_from_pattern(const std::string& pattern);

	/*!
	 * \brief Add options to the configuration
	 *
	 * Add options for all fonts to the multi-select variables in the
	 * configuration window. If the optional parameter \a add_button i \c true,
	 * buttons for the fonts will immediately be added to the corresponding
	 * widget.
	 */
	void add_select_options(bool add_button=false);

	/*!
	 * \brief Load a font.
	 *
	 * Get the font for font category \a cat and text scale factor \a zoom. If this font fails to
	 * load, switch to fixed font 1.
	 *
	 * \param cat       The font category for which to load a font.
	 * \param text_zoom The scale factor for the text
	 * \return Reference to the font itself.
	 */
	Font& get(Category cat, float text_zoom);
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

/*!
 * \ingroup text_font
 * \brief Initialize the font manager.
 *
 * This adds the standard fonts bundled with EL to the font manager, and
 * if TTF fonts are enabled, scans the TTF path for True Type fonts
 * to add as well.
 */
int initialize_fonts();

/*!
 * \ingroup text_font
 * \brief Return the font number of a fixed width font
 *
 * Return the font number for the \a idx'th fixed width font.
 *
 * \param idx The number of the font in the list of fixed width fonts only
 * \return The index of the font in the list of all fonts
 */
size_t get_fixed_width_font_number(size_t idx);

/*!
 * \ingroup text_font
 * \brief The width of a single character
 *
 * Return the width of character \a c, including spacing, when drawn
 * in the font for category \a cat, at zoom level \a text_zoom.
 *
 * \param c         The character for which to get the width
 * \param cat       The font category for the font used
 * \param text_zoom The scale factor for the text
 * \return The width of the character and spacing, in pixels.
 */
int get_char_width_zoom(unsigned char c, font_cat cat, float text_zoom);
/*!
 * \ingroup text_font
 * \brief The maximum width of a single character
 *
 * Return the maximum a single character can occupy when drawn in the font
 * for category \a cat at zoom level \a text_zoom, and include the spacing
 * between characters.
 *
 * \param cat       The font category for the font used
 * \param text_zoom The scale factor for the text
 * \return The maximum character width incuding spacing, in pixels.
 */
int get_max_char_width_zoom(font_cat cat, float text_zoom);
/*!
 * \ingroup text_font
 * \brief Get the average character width, plus spacing
 *
 * Get an approximation to the average width of a character in the font for
 * category \a cat when drawn at zoom level \a text_zoom, including the space
 * between characters, for some definition of "average". See the note
 * in eternal_lands::Font::average_width_spacing() for some caveats when using
 * this function.
 *
 * \param cat       The font category for the font used
 * \param text_zoom The zoom factor for drawing the character.
 * \return The maximum width of a character including spacing, in pixels.
 */
int get_avg_char_width_zoom(font_cat cat, float text_zoom);
/*!
 * \ingroup text_font
 * \brief The maximum width of a single digit character
 *
 * Return the maximum a single digit character (0-9) can occupy when drawn
 * in the font for category \a cat at zoom level \a text_zoom, and include
 * the spacing between characters.
 *
 * \param cat       The font category for the font used
 * \param text_zoom The scale factor for the text
 * \return The maximum digit character width incuding spacing, in pixels.
 */
int get_max_digit_width_zoom(font_cat cat, float text_zoom);
/*!
 * \ingroup text_font
 * \brief Calculate the width of a string
 *
 * Calculate the width in pixels of the string \a text of length \a len
 * when drawn in the font for category \a cat at zoom level \a text_zoom.
 * \note This function assumes the string is a single line. Newline
 * characters are ignored, and having them in the middle of string will
 * result in a too large value for the width.
 *
 * \param cat       The font category for the font used
 * \param text      The string for which to calculate the length
 * \param len       The number of bytes in \a text
 * \param text_zoom The scale factor for the text
 * \return The width of the text in pixels
 */
int get_buf_width_zoom(const unsigned char* text, size_t len, font_cat cat, float text_zoom);
/*!
 * \ingroup text_font
 * \brief Calculate the width of a string
 *
 * Calculate the width in pixels of the nul-terminated string \a text
 * when drawn in the font for category \a cat at zoom level \a text_zoom.
 * \note This function assumes the string is a single line. Newline
 * characters are ignored, and having them in the middle of string will
 * result in a too large value for the width.
 *
 * \param cat       The font category for the font used
 * \param text      The string for which to calculate the length
 * \param text_zoom The scale factor for the text
 * \return The width of the text in pixels
 */
static __inline__ int get_string_width_zoom(const unsigned char* str, font_cat cat,
	float text_zoom)
{
	return get_buf_width_zoom(str, strlen((const char*)str), cat, text_zoom);
}
/*!
 * \ingroup text_font
 * \brief The height of a text line
 *
 * Return the height of a line of text when drawn in the font for category
 * \a cat with scale factor \a text_zoom.
 *
 * \param cat       The font category for the font used
 * \param text_zoom The scale factor for the text
 * \return The height of the text in pixels
 */
int get_line_height(font_cat cat, float text_zoom);
/*!
 * \ingroup text_font
 * \brief The distance between two lines
 *
 * Return the distance between two lines of text when drawn in the font for category
 * \a cat with scale factor \a text_zoom. This is not necessarily the same as the line
 * height.
 *
 * \param cat       The font category for the font used
 * \param text_zoom The scale factor for the text
 * \return The line distance
 */
int get_line_skip(font_cat cat, float text_zoom);
/*!
 * \brief Calculate the number of lines that fit.
 *
 * Return the maximum number of lines that fit in a window of \a height pixels high, when drawing
 * text in the font for category \a cat at zoom level \a zoom.
 *
 * \param max_height The maximum height of the text in pixels
 * \param cat        The font category for the font used
 * \param zoom       The zoom factor for drawing the text
 * \return The maximum number of lines that fit
 */
int get_max_nr_lines(int max_height, font_cat cat, float zoom);
/*!
 * \brief Compute the height of a block of text
 *
 * Compute the height of a block of text of \a nr_lines lines, when drawn in the font for category
 * \a cat at zoom level \a zoom.
 *
 * \param nr_lines The number of lines in the text
 * \param cat      The font category for the font used
 * \param zoom     The zoom factor for drawing the text
 * \return The height of the text in pixels
 */
int get_text_height(int nr_lines, font_cat cat, float zoom);
/*!
 * \ingroup text_font
 * \brief Calculate the dimensions of a block of text
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
 * \param width     Place to store the calculated width
 * \param height    Place to store the calculated height
 */
void get_buf_dimensions(const unsigned char* text, size_t len, font_cat cat,
	float text_zoom, int *width, int *height);

/*!
 * \brief Calculate vertical offset of center
 *
 * Calculate the offset of the center of the characters in \a text with respect to the center
 * of the line, when drawn in the font for category \a cat at zoom level \a text_zoom. Subtract the
 * result of this function from the \c y coordinate you wish to center around, to center the
 * content of \a text around this position.
 *
 * \param cat       The font category for the font used
 * \param text      The string for which to compute center offset
 * \param len       The number of bytes in \a text
 * \param text_zoom The scale factor for the text
 * \return The number of pixels between the center of the line and the center of \a text.
 */
int get_center_offset(const unsigned char* text, size_t len, font_cat cat, float text_zoom);
/*!
 * \brief Get vertical coordinates
 *
 * Get the minimum and maximum offsets of the characters in \a text with respect to the top of
 * the text line, when drawing the text in the font for category \a cat at zoom level \a zoom.
 *
 * \param text      The text to get the vertical offsets for
 * \param len       The number of characters in \a text
 * \param cat       The font category for the font used
 * \param text_zoom The scale factor for the text
 * \return The offsets of the top and bottom of the text
 */
void get_top_bottom(const unsigned char* text, size_t len, font_cat cat, float text_zoom,
	int *top, int *bottom);

/*!
 * \ingroup text_font
 * \brief Recompute where the line breaks in a string should occur
 *
 * Recomputes the positions in string \a text where line breaks should be placed,
 * when drawn in the font for category \a cat at zoom level \a text_zoom such
 * that the string fits into a window of width \a width pixels. This inserts
 * \c '\\r' characters at the positions in \a text where the line should be broken.
 * Parameters \a len and \a size are the current and maximum number of bytes in \a text,
 * respectively. If \a cursor is not \c NULL, it should point to the offset of the
 * cursor position in the text; it will be updated as new line breaks are inserted.
 * If \a max_line_width is not \c NULL, it can be used to retrieve the largest width
 * in pixels of the lines in \a text after rewrapping.
 *
 * \param text           the string
 * \param len            the actual length of the string
 * \param size           the maximum number of bytes in \a text
 * \param cat            the font category in which the string is to be drawn
 * \param text_zoom      the scale factor for the text
 * \param width          the allowed width of a line of text
 * \param cursor         pointer to the cursor position, or NULL if not used
 * \param max_line_width pointer the maximum line length after wrapping, or NULL if not used
 *
 * \return The wrapped text, and the new number of lines in the text
 */
int reset_soft_breaks(unsigned char *text, int len, int size, font_cat cat,
	float text_zoom, int width, int *cursor, float *max_line_width);
/*!
 * \ingroup text_font
 * \brief Wrap a text so that it fits into a window
 *
 * Create a version of \a text with newline characters inserted so that the
 * line length does not exceeed \a width pixels when drawn in a small font for
 * the UI_FONT category, at zoom level \a zoom, and store it in \a buffer. The
 * parameter \a color is an index in colors_list; it will be converted to a
 * color character which is inserted in front of the text in \a buffer.
 * \warning No check is made if \a buffer is large enough to hold the output
 * text, the caller needs to ensure this.
 *
 * \param color     The color with which the text should be drawn
 * \param text      The text to wrap
 * \param len       The number of bytes in text
 * \param width     The maximum allowed width in pixels of a line of text
 * \param buffer    Output buffer in which the result is stored
 * \param text_zoom Zoom factor for the text (on top of using small text)
 */
void put_small_colored_text_in_box_zoomed(int color,
	const unsigned char* text, int len, int width,
	unsigned char* buffer, float text_zoom);
/*!
 * \ingroup text_font
 * \brief Wrap a text so that it fits into a window
 *
 * Create a version of \a text with newline characters inserted so that the
 * line length does not exceeed \a width pixels when drawn in a small font for
 * the UI_FONT category, at zoom level \a zoom, and store it in \a buffer. The
 * text will be drawn in white.
 * \warning No check is made if \a buffer is large enough to hold the output
 * text, the caller needs to ensure this.
 *
 * \param text      The text to wrap
 * \param len       The number of bytes in text
 * \param width     The maximum allowed width in pixels of a line of text
 * \param buffer    Output buffer in which the result is stored
 * \param text_zoom Zoom factor for the text (on top of using small text)
 */
static __inline__ void put_small_text_in_box_zoomed (const unsigned char* text,
	int len, int width, unsigned char* buffer, float text_zoom)
{
	put_small_colored_text_in_box_zoomed(c_grey1, text, len, width, buffer, text_zoom);
}

/*!
 * \ingroup text_font
 * \brief Enumeration for text drawing options
 *
 * Various text drawing options can be passed to \ref draw text, specified by a selector from this
 * enumeration followed by one or more arguments.
 */
typedef enum
{
	//! Set the maximum width, followed by \c int
	TDO_MAX_WIDTH,
	//! Set the maximum number of lines, followed by \c int
	TDO_MAX_LINES,
	//! Scale factor for the text, followed by \c float
	TDO_ZOOM,
	//! Scale factor for the spacing between lines, followed by \c float
	TDO_LINE_SPACING,
	//! Horizontal alignment, followed by a \c hor_alignment
	TDO_ALIGNMENT,
	//! Vertical alignment, followed by a \c ver_alignment
	TDO_VERTICAL_ALIGNMENT,
	//! Draw the text with a shadow in the background color, followed by \c bool
	TDO_SHADOW,
	//! Foreground color of the text, followed by 3 \c floats
	TDO_FOREGROUND,
	//! Background color of the text, followed by 3 \c floats
	TDO_BACKGROUND,
	//! Selection color of the text, followed by 3 \c floats
	TDO_SELECTION,
	//! Ignor color characters in the text, followed by \c bool
	TDO_IGNORE_COLOR,
	//! Draw the string as help text, with a semi-transparent background, followed by \c bool
	TDO_HELP,
	//! Indicate clipped text with ellipsis, followed by \c bool
	TDO_ELLIPSIS,
	//! Shrink text if it is too wide, followed by \a bool
	TDO_SHRINK_TO_FIT,
	//! Start index of the selected text, followed by \c int
	TDO_SEL_BEGIN,
	//! End index of the selected text, followed by \c int
	TDO_SEL_END,
	//! End of text draw options, should always be last
	TDO_END
} text_draw_option_sel;

/*!
 * \ingroup text_font
 * \brief Draw a text string
 *
 * Draw the text in the first \a len bytes of \a text, starting at position \a x, \a y on the
 * screen, using the font for category \a cat, with formatting options \a options. The options
 * should be a (possbily empty) list of option selectors of type \ref text_draw_option_sel, each
 * followed by the arguments for that particular selector, and should end with \ref TDO_END.
 *
 * \param x         The left coordinate of the drawn text
 * \param y         The top coordinate of the drawn text
 * \param text      The text to draw
 * \param len       The number of bytes in \a text
 * \param cat       The font category for the text
 * \param options   Formatting options for the text
 */
void vdraw_text(int x, int y, const unsigned char* text, size_t len, font_cat cat, va_list options);
/*!
 * \ingroup text_font
 * \brief Draw a text string
 *
 * Draw the text in the first \a len bytes of \a text, starting at position \a x, \a y on the
 * screen, using the font for category \a cat, with formatting options \a options. Formatting
 * options for the text can be given after \a cat; the list of formatting options should always
 * end with \ref TDO_END.
 * \note Even when no formatting options are given, \ref TDO_END should be passed as the only
 * extra parameter to indicate the end of the options list.
 *
 * \param x         The left coordinate of the drawn text
 * \param y         The top coordinate of the drawn text
 * \param text      The text to draw
 * \param len       The number of bytes in \a text
 * \param cat       The font category for the text
 * \param options   Formatting options for the text
 * \note The type of \a cat is \c int here instead of \c font_cat. This is on purpose, as using a
 * type that undergoes default argument promotion as the last argument before the variable
 * argument list results in undefined behaviour.
 */
static __inline__ void draw_text(int x, int y, const unsigned char* text, size_t len, int cat, ...)
{
	va_list ap;
	va_start(ap, cat);
	vdraw_text(x, y, text, len, (font_cat)cat, ap);
	va_end(ap);
}

/*!
 * \ingroup text_font
 * \brief Draw a text string
 *
 * Draw the text in the nul-terminated buffer \a text, starting at position
 * \a x, \a y on the screen, using the font for category \a cat. Options \a max_width and
 * \a max_lines specify the maximum width of the text and maximum number of lines
 * drawn respectively. Text outside these boundaries will not be drawn. The text
 * will be drawn in the default foreground color.
 *
 * \param x         The left coordinate of the drawn text
 * \param y         The top coordinate of the drawn text
 * \param text      The text to draw
 * \param max_width The maximum width in pixels of the text
 * \param max_lines The maximum number of lines to draw, or 0 for no limit
 * \param cat       The font category for the text
 * \param text_zoom Scale factor for the text size
 */
static __inline__ void draw_string_zoomed_width_font(int x, int y, const unsigned char *text,
	int max_width, int max_lines, font_cat cat, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), cat, TDO_MAX_WIDTH, max_width,
		TDO_MAX_LINES, max_lines, TDO_ZOOM, text_zoom, TDO_END);
}
/*!
 * \ingroup text_font
 * \brief Draw a text string
 *
 * Draw the text in the nul-terminated buffer \a text, centered around position
 * \a x, \a y on the screen, using the font for the UI_FONT category. The centering is
 * done such that the bytes up to \a center_idx are drawn left of \a x, and the
 * bytes from \a center_idx onward are drawn to the right of \a x.
 *
 * \param x          The x coordinate of the center of drawn text
 * \param y          The top coordinate of the drawn text
 * \param text       The text to draw
 * \param center_idx The index of the character around which the centering is done
 * \param text_zoom  Scale factor for the text size
 */
void draw_string_zoomed_centered_around(int x, int y, const unsigned char *text, int center_idx,
	float text_zoom);
/*!
 * \ingroup text_font
 * \brief Draw a text string
 *
 * Draw the text in the nul-terminated buffer \a text, starting at position
 * \a x, \a y on the screen, using the font for the UI_FONT category. The option
 * \a max_lines specifies the maximum number of lines drawn, any text after
 * this line will not be drawn. The text will be drawn in the default foreground
 * color.
 *
 * \param x         The left coordinate of the drawn text
 * \param y         The top coordinate of the drawn text
 * \param text      The text to draw
 * \param max_lines The maximum number of lines to draw, or 0 for no limit
 * \param text_zoom Scale factor for the text size
 */
static __inline__ void draw_string_zoomed(int x, int y, const unsigned char* text,
	int max_lines, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), UI_FONT, TDO_MAX_LINES, max_lines,
		TDO_ZOOM, text_zoom, TDO_END);
}
/*!
 * \ingroup text_font
 * \brief Draw a text string
 *
 * Draw the text in the nul-terminated buffer \a text, centered around position
 * \a x, \a y on the screen, using the font for the UI_FONT category. The option
 * \a max_lines specifies the maximum number of lines drawn, any text after
 * this line will not be drawn. The text will be drawn in the default foreground
 * color.
 *
 * \param x         The x coordinate of the center of drawn text
 * \param y         The top coordinate of the drawn text
 * \param text      The text to draw
 * \param max_lines The maximum number of lines to draw, or 0 for no limit
 * \param text_zoom Scale factor for the text size
 */
static __inline__ void draw_string_zoomed_centered(int x, int y, const unsigned char *text,
	int max_lines, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), UI_FONT, TDO_MAX_LINES, max_lines,
		TDO_ZOOM, text_zoom, TDO_ALIGNMENT, CENTER, TDO_END);
}
/*!
 * \ingroup text_font
 * \brief Draw a text string
 *
 * Draw the text in the nul-terminated buffer \a text, starting at position
 * \a x, \a y on the screen, using the font for the UI_FONT category. The string
 * is drawn in the color (\a fr, \a fg, \a fb), with a background shadow in
 * the color (\a br, \a bg, \a bb). The option \a max_lines specifies the maximum
 * number of lines drawn, any text after this line will not be drawn.
 *
 *
 * \param x         The left coordinate of the drawn text
 * \param y         The top coordinate of the drawn text
 * \param text      The text to draw
 * \param max_lines The maximum number of lines to draw, or 0 for no limit
 * \param fr        Red component of the foreground color
 * \param fg        Green component of the foreground color
 * \param fb        Blue component of the foreground color
 * \param br        Red component of the background color
 * \param bg        Green component of the background color
 * \param bb        Blue component of the background color
 * \param text_zoom Scale factor for the text size
 */
static __inline__ void draw_string_shadowed_zoomed(int x, int y, const unsigned char* text,
	int max_lines, float fr, float fg, float fb, float br, float bg, float bb, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), UI_FONT, TDO_MAX_LINES, max_lines,
		TDO_SHADOW, 1, TDO_FOREGROUND, fr, fg, fb, TDO_BACKGROUND, br, bg, bb, TDO_ZOOM, text_zoom,
		TDO_END);
}
/*!
 * \ingroup text_font
 * \brief Draw a text string
 *
 * Draw the text in the nul-terminated buffer \a text, centered around position
 * \a x, \a y on the screen, using the font for the UI_FONT category. The string
 * is drawn in the color (\a fr, \a fg, \a fb), with a background shadow in
 * the color (\a br, \a bg, \a bb). The option \a max_lines specifies the maximum
 * number of lines drawn, any text after this line will not be drawn.
 *
 *
 * \param x         The x coordinate of the center of drawn text
 * \param y         The top coordinate of the drawn text
 * \param text      The text to draw
 * \param max_lines The maximum number of lines to draw, or 0 for no limit
 * \param fr        Red component of the foreground color
 * \param fg        Green component of the foreground color
 * \param fb        Blue component of the foreground color
 * \param br        Red component of the background color
 * \param bg        Green component of the background color
 * \param bb        Blue component of the background color
 * \param text_zoom Scale factor for the text size
 */
static __inline__ void draw_string_shadowed_zoomed_centered(int x, int y, const unsigned char* text,
	int max_lines, float fr, float fg, float fb, float br, float bg, float bb, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), UI_FONT, TDO_MAX_LINES, max_lines,
		TDO_SHADOW, 1, TDO_FOREGROUND, fr, fg, fb, TDO_BACKGROUND, br, bg, bb, TDO_ZOOM, text_zoom,
		TDO_ALIGNMENT, CENTER, TDO_END);
}
/*!
 * \ingroup text_font
 * \brief Draw a text string
 *
 * Draw the text in the nul-terminated buffer \a text, starting at position
 * \a x, \a y on the screen, using the font for category \a cat. If a line
 * cannot fit within the maximum width \a max_width, it will be truncated and
 * an ellipsis is added. The option \a max_lines specifies the maximum
 * number of lines drawn, any text after this line will not be drawn.
 *
 * \param x         The left coordinate of the drawn text
 * \param y         The top coordinate of the drawn text
 * \param text      The text to draw
 * \param max_width The maximum width in pixels of the text
 * \param max_lines The maximum number of lines to draw, or 0 for no limit
 * \param cat       The font category for the text
 * \param text_zoom Scale factor for the text size
 */
static __inline__ void draw_string_zoomed_ellipsis_font(int x, int y, const unsigned char *text,
	int max_width, int max_lines, font_cat cat, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), cat, TDO_MAX_WIDTH, max_width,
		TDO_MAX_LINES, max_lines, TDO_ZOOM, text_zoom, TDO_ELLIPSIS, 1, TDO_END);
}

//! Analogue of draw_string_zoomed(), for small text size
static __inline__ void draw_string_small_zoomed(int x, int y,
	const unsigned char* text, int max_lines, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), UI_FONT, TDO_MAX_LINES, max_lines,
		TDO_ZOOM, text_zoom * DEFAULT_SMALL_RATIO, TDO_END);
}
/*!
 * \ingroup text_font
 * \brief Draw a text string
 *
 * Draw the text in the nul-terminated buffer \a text, right aligned to position
 * \a x, \a y on the screen, using the small font for the UI_FONT category. The option
 * \a max_lines specifies the maximum number of lines drawn, any text after
 * this line will not be drawn. The text will be drawn in the default foreground
 * color.
 *
 * \param x         The right coordinate of the drawn text
 * \param y         The top coordinate of the drawn text
 * \param text      The text to draw
 * \param max_lines The maximum number of lines to draw, or 0 for no limit
 * \param text_zoom Scale factor for the text size
 */
static __inline__ void draw_string_small_zoomed_right(int x, int y,
	const unsigned char* text, int max_lines, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), UI_FONT, TDO_MAX_LINES, max_lines,
		TDO_ZOOM, text_zoom * DEFAULT_SMALL_RATIO, TDO_ALIGNMENT, RIGHT, TDO_END);
}
//! Analogue of draw_string_zoomed_centered(), for small text size
static __inline__ void draw_string_small_zoomed_centered(int x, int y,
	const unsigned char* text, int max_lines, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), UI_FONT, TDO_MAX_LINES, max_lines,
		TDO_ZOOM, text_zoom * DEFAULT_SMALL_RATIO, TDO_ALIGNMENT, CENTER, TDO_END);
}
//! Analogue of draw_string_zoomed_centered_around(), for small text size
static __inline__ void draw_string_small_zoomed_centered_around(int x, int y,
	const unsigned char *text, int center_idx, float text_zoom)
{
	draw_string_zoomed_centered_around(x, y, text, center_idx, text_zoom * DEFAULT_SMALL_RATIO);
}
//! Analogue of draw_string_shadowed_zoomed(), for small text size
static __inline__ void draw_string_small_shadowed_zoomed(int x, int y,
	const unsigned char* text, int max_lines, float fr, float fg, float fb,
	float br, float bg, float bb, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), UI_FONT, TDO_MAX_LINES, max_lines,
		TDO_SHADOW, 1, TDO_FOREGROUND, fr, fg, fb, TDO_BACKGROUND, br, bg, bb,
		TDO_ZOOM, text_zoom * DEFAULT_SMALL_RATIO, TDO_END);
}
/*!
 * \ingroup text_font
 * \brief Draw a text string
 *
 * Draw the text in the nul-terminated buffer \a text, right aligned to position
 * \a x, \a y on the screen, using the small font for the UI_FONT category. The string
 * is drawn in the color (\a fr, \a fg, \a fb), with a background shadow in
 * the color (\a br, \a bg, \a bb). The option \a max_lines specifies the maximum
 * number of lines drawn, any text after this line will not be drawn.
 *
 *
 * \param x         The right coordinate of the drawn text
 * \param y         The top coordinate of the drawn text
 * \param text      The text to draw
 * \param max_lines The maximum number of lines to draw, or 0 for no limit
 * \param fr        Red component of the foreground color
 * \param fg        Green component of the foreground color
 * \param fb        Blue component of the foreground color
 * \param br        Red component of the background color
 * \param bg        Green component of the background color
 * \param bb        Blue component of the background color
 * \param text_zoom Scale factor for the text size
 */
static __inline__ void draw_string_small_shadowed_zoomed_right(int x, int y,
	const unsigned char* text, int max_lines, float fr, float fg, float fb,
	float br, float bg, float bb, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), UI_FONT, TDO_MAX_LINES, max_lines,
		TDO_SHADOW, 1, TDO_FOREGROUND, fr, fg, fb, TDO_BACKGROUND, br, bg, bb,
		TDO_ZOOM, text_zoom * DEFAULT_SMALL_RATIO, TDO_ALIGNMENT, RIGHT, TDO_END);
}

/*!
 * \ingroup text_font
 * \brief Draw a help message
 *
 * Shows the help message \a text centered horizontally around position
 * (\a x, \a y) in text color (\a r, \a g, \a b), with text scale factor
 * \a text_zoom. The message is drawn on a semi-transparent background.
 *
 * \param text      The help text to display
 * \param x         The x-coordinate of the center of the text
 * \param y         The y-coordinate of the top of text text
 * \param r         The red component of the text color
 * \param g         The green component of the text color
 * \param b         The blue component of the text color
 * \param text_zoom The scale factor for the text size
 */
static __inline__ void show_help_colored_scaled_centered(const unsigned char *text, int x, int y,
	float r, float g, float b, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), UI_FONT, TDO_MAX_WIDTH, window_width - 80,
		TDO_HELP, 1, TDO_FOREGROUND, r, g, b, TDO_ZOOM, text_zoom, TDO_ALIGNMENT, CENTER, TDO_END);
}
/*!
 * \ingroup text_font
 * \brief Draw a help message
 *
 * Shows the help message \a text right-aligned to position (\a x, \a y)
 * in text color (\a r, \a g, \a b), with text scale factor \a text_zoom.
 * The message is drawn on a semi-transparent background.
 *
 * \param text      The help text to display
 * \param x         The x-coordinate of the right side of the text
 * \param y         The y-coordinate of the top of text text
 * \param r         The red component of the text color
 * \param g         The green component of the text color
 * \param b         The blue component of the text color
 * \param text_zoom The scale factor for the text size
 */
static __inline__ void show_help_colored_scaled_right(const unsigned char *text, int x, int y,
	float r, float g, float b, float text_zoom)
{
	draw_text(x, y, text, strlen((const char*)text), UI_FONT, TDO_MAX_WIDTH, window_width - 80,
		TDO_HELP, 1, TDO_FOREGROUND, r, g, b, TDO_ZOOM, text_zoom, TDO_ALIGNMENT, RIGHT, TDO_END);
}
/*!
 * \ingroup text_font
 * \brief Shows a help message in small font
 *
 * Shows the help message \a text at position (\a x, \a y) using a small font
 * size. The message is drawn on a semi-transparent background.
 *
 * \param text      the help message to show
 * \param x         the x coordinate of the left of the help message
 * \param y         the y coordinate of the top of the help message
 * \param text_zoom the scale for the text size
 */
static __inline__ void show_help(const char *text, int x, int y, float text_zoom)
{
	draw_text(x, y, (const unsigned char*)text, strlen(text), UI_FONT,
		TDO_MAX_WIDTH, window_width - 80, TDO_HELP, 1, TDO_FOREGROUND, 1.0, 1.0, 1.0,
		TDO_ZOOM, text_zoom * DEFAULT_SMALL_RATIO, TDO_END);
}
/*!
 * \ingroup text_font
 * \brief Shows a help message in normal font
 *
 * Shows the help message \a text at position (\a x, \a y) using the normal
 * font size. The message is drawn on a semi-transparent background.
 *
 * \param text      the help message to show
 * \param x         the x coordinate of the left of the help message
 * \param y         the y coordinate of top of the help message
 * \param text_zoom the scale for the text size
 */
static __inline__ void show_help_big(const char *text, int x, int y, float text_zoom)
{
	draw_text(x, y, (const unsigned char*)text, strlen(text), UI_FONT,
		TDO_MAX_WIDTH, window_width - 80,TDO_HELP, 1, TDO_FOREGROUND, 1.0, 1.0, 1.0,
		TDO_ZOOM, text_zoom, TDO_END);
}

/*!
 * \ingroup text_font
 * \brief Draw messages in a buffer to the screen
 *
 * Draws the messages in buffer \a msgs to the screen, starting with character
 * \a offset_start in message number \a msg_start.
 *
 * \param x            x coordinate of the position to start drawing
 * \param y            y coordinate of the position to start drawing
 * \param msgs         the message buffer
 * \param msgs_size    the total number of messages that \a msgs can hold
 * \param filter       draw only messages in channel \a filter. Choose
 * 	FILTER_ALL for displaying all messages
 * \param msg_start    index of the first message to display
 * \param offset_start the first character in message \a msg_start to display
 * \param cursor       if >= 0, the position at which to draw the cursor
 * \param width        the width of the output window
 * \param height       the height of the output window
 * \param cat          the font category for the font used to draw the messages
 * \param text_zoom    the scale factor for the text size
 * \param[in,out] select information about current selection. draw_messages()
 *	 fills the select->lines array.
 */
void draw_messages(int x, int y, text_message *msgs, int msgs_size, Uint8 filter,
	int msg_start, int offset_start, int cursor, int width, int height,
	font_cat cat, float text_zoom, select_info* select);
/*!
 * \ingroup text_font
 * \brief Draw the console separator
 *
 * When there are more messages than can fit on a single screen, the user can
 * scroll up to see the previous messages. When they do, a separator is
 * drawn by this function to indicate that more messages follow.
 *
 * \param x_space   Horizontal space tokeep free on either side
 * \param y         Vertical position at which the line should be drawn
 * \param width     The width of the console window
 * \param text_zoom The zoom level of the console text
 */
void draw_console_separator(int x_space, int y, int width, float text_zoom);
#ifdef ELC
#ifndef MAP_EDITOR2
void draw_ortho_ingame_string(float x, float y, float z,
	const unsigned char *text, int max_lines, float zoom_x, float zoom_y);
void draw_ingame_string(float x, float y, const unsigned char *text,
	int max_lines, float zoom_x, float zoom_y);
#endif // !MAP_EDITOR2
#endif // ELC

/*!
 * \ingroup text_font
 * \brief Set the config font
 *
 * Copy the current font index and scale of the UI_FONT category to that
 * of the CONFIG_FONT category. This allows us to change the UI font
 * without changing the way the options window is drawn, and hopefully
 * stops the user from then being unable to change the font back.
 */
void set_config_font();

/*!
 * \ingroup text_font
 * \brief whether a glyph is defined for character \a c, i.e. if it is printable
 */
int has_glyph(unsigned char c);

#ifdef TTF
//! Disable TrueType fonts, allowing bundled fonts only.
void disable_ttf(void);
//! Enable TrueTyoe fonts, restoring previous font indices if possible
void enable_ttf(void);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // NEW_FONT_H

/*!
 * \file
 * \ingroup text_font
 * \brief the font structure and related functions.
 */
#ifndef __FONT_H__
#define __FONT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup text_font
 * \brief  Enumeration forfont categories
 *
 *      This enumeration defines the different categories of fonts used in the
 *      game. The actual enum values correspond to indices in the font_idxs array,
 *      which holds the actual font numbers.
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
	//! Index used for the font used to draw the rules
	RULES_FONT,
	//! Number of font categories
	NR_FONT_CATS
} font_cat;

#ifdef __cplusplus
} // extern "C"
#endif

#include "gl_init.h"
#include "text.h"
#if !defined(MAP_EDITOR)
#include "widgets.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name constant sizes
 */
/*! @{ */
#define	INGAME_FONT_X_LEN 0.17
#define	INGAME_FONT_Y_LEN 0.25
#define SMALL_INGAME_FONT_X_LEN 0.12
#define SMALL_INGAME_FONT_Y_LEN 0.17
#define ALT_INGAME_FONT_X_LEN 0.10
#define ALT_INGAME_FONT_Y_LEN 0.15
#define DEFAULT_FONT_X_LEN 11.0f
#define DEFAULT_FONT_Y_LEN 18.0f
#define SMALL_FONT_X_LEN 8.0f
#define SMALL_FONT_Y_LEN 15.0f
#define DEFAULT_SMALL_RATIO (SMALL_FONT_X_LEN/DEFAULT_FONT_X_LEN)
/*! @} */

#ifdef TTF
#define TTF_DIR_SIZE 256

extern int use_ttf;    /*!< Whether to use True Type fonts for font rendering */
extern char ttf_directory[TTF_DIR_SIZE];
#endif

//! The font numbers for each font category
extern int font_idxs[NR_FONT_CATS];
//! The zoom factor for each font category
extern float font_scales[NR_FONT_CATS];

#if !defined(MAP_EDITOR)
/*!
 * \ingroup text_font
 * \brief  draws messages in a buffer to the screen
 *
 *      Draws the messages in buffer \a msgs to the screen, starting with character \a offset_start in message number \a nr_start.
 *      NOTE: The messages are rewrapped if necessary!
 *
 * \param x		x coordinate of the position to start drawing
 * \param y		y coordinate of the position to start drawing
 * \param msgs		the message buffer
 * \param msgs_size	the total number of messages that \a msgs can hold
 * \param filter	draw only message in channel \a filter. Choose FILTER_ALL for displaying all messages
 * \param nr_start	the first message to display
 * \param offset_start	the first character in message \a nr_start to display
 * \param cursor	if >= 0, the position at which to draw the cursor
 * \param width		the width of the draw area
 * \param height	the height of the draw area
 * \param font		the category of the font in which the text is drawn
 * \param text_zoom	the size of the text
 * \param[in,out]       select information about current selection. draw_messages fills select->lines array.
 *
 * \callgraph
 */
void draw_messages(int x, int y, text_message *msgs, int msgs_size, Uint8 filter,
	int nr_start, int offset_start, int cursor, int width, int height,
	font_cat font, float text_zoom, select_info* select);
void draw_console_separator(int x_space, int y, int width, float zoom);
#endif

/*!
 * \ingroup text_font
 * \brief   returns the width of \a cur_char
 *
 *      Returns the width of char \a cur_char
 *
 * \param c         the char to get the width for
 * \param cat       the context in which the character is used
 * \param text_zoom caller supplies zoom level
 * \retval int
 * \callgraph
 */
int get_char_width_zoom(unsigned char c, font_cat cat, float zoom);
static __inline__ int get_char_width_ui(unsigned char c, float text_zoom)
{
	return get_char_width_zoom(c, UI_FONT, text_zoom);
}

/*!
 * \ingroup text_font
 * \brief   returns the width of the string \a str.
 *
 *      Returns the width of the string \a str
 *
 * \param str       the string which width to return
 * \param cat       the context in which the character is used
 * \param text_zoom caller supplies zoom level
 * \retval int
 * \sa get_nstring_width
 * \callgraph
 */
int get_string_width_zoom(const unsigned char* str, font_cat cat, float text_zoom);
static __inline__ int get_string_width_ui(const unsigned char* str, float text_zoom)
{
	return get_string_width_zoom(str, UI_FONT, text_zoom);
}

/*!
 * \ingroup text_font
 * \brief   draws the given string \a our_string at the desired position (\a x, \a y) with a zoom factor of \a text_zoom.
 *
 *      Draws the given string \a our_string at the desired position (\a x, \a y) with a zoom factor of \a text_zoom.
 *
 * \param x             x coordinate of the position to draw the string
 * \param y             y coordinate of the position to draw the string
 * \param our_string    the string to draw
 * \param max_lines
 * \param font_cat      the category of the font in which the text is drawn
 * \param text_zoom     the zoom factor to use
 *
 * \callgraph
 */
int draw_string_zoomed_width_font(int x, int y, const unsigned char *our_string,
	int max_width, int max_lines, font_cat font, float text_zoom);
static __inline__ int draw_string_zoomed(int x, int y,
	const unsigned char* our_string, int max_lines, float text_zoom)
{
	return draw_string_zoomed_width_font(x, y, our_string, window_width, max_lines,
		UI_FONT, text_zoom);
}
static __inline__ int draw_string_zoomed_width(int x, int y,
	const unsigned char* our_string, int max_width, int max_lines, float text_zoom)
{
	return draw_string_zoomed_width_font(x, y, our_string, max_width, max_lines,
		UI_FONT, text_zoom);
}
int draw_string_shadowed_zoomed_font(int x, int y, const unsigned char* our_string,
	int max_lines, float fr, float fg, float fb, float br, float bg, float bb,
	font_cat font, float text_zoom);
static __inline__ int draw_string_shadowed_zoomed(int x, int y,
	const unsigned char* our_string, int max_lines, float fr, float fg, float fb,
	float br, float bg, float bb, float text_zoom)
{
	return draw_string_shadowed_zoomed_font(x, y, our_string, max_lines, fr, fg, fb,
		br, bg, bb, UI_FONT, text_zoom);
}
static __inline__ int draw_string_shadowed_width(int x, int y,
	const unsigned char* our_string, int max_width, int max_lines,
	float fr, float fg, float fb, float br, float bg, float bb)
{
	float text_zoom = (float)max_width / get_string_width_ui(our_string, 1.0);
	return draw_string_shadowed_zoomed_font(x, y, our_string, max_lines, fr, fg, fb,
		br, bg, bb, UI_FONT, text_zoom);
}

/*!
 * \ingroup text_font
 * \brief   recompute where the line breaks in a string should occur
 *
 *      Recomputes the positions in the string where line breaks should be placed so that the string fits into a window.
 *
 * \param str		the string
 * \param len		the length of the string
 * \param size		the size of the string buffer
 * \param font_num	the number of the font in which the text is to be rendered
 * \param text_zoom	the scale factor for the text
 * \param width		the width of the text window
 * \param cursor	pointer to the cursor position, or NULL if not used
 * \param max_line_width	pointer the maximum line length after wrapping, or NULL if not used
 *
 * \retval int the number of window lines the string will use
 * \callgraph
 */
int reset_soft_breaks (char *str, int len, int size, font_cat font, float text_zoom,
	int width, int *cursor, float *max_line_width);

/*!
 * \ingroup text_font
 * \brief   draws the given string \a our_string at the desired position (\a x, \a y) using a small font.
 *
 *      Draws the given string \a our_string at the desired position (\a x, \a y) using a small font.
 *
 * \param x             x coordinate of the position to draw the string
 * \param y             y coordinate of the position to draw the string
 * \param our_string    the string to draw
 * \param max_lines
 * \param text_zoom     the zoom factor to use
 * \param font_cat      the category of the font in which the text is drawn
 *
 * \callgraph
 */
static __inline__ void draw_string_small_zoomed(int x, int y,
	const unsigned char* our_string, int max_lines, float text_zoom)
{
	draw_string_zoomed_width_font(x, y, our_string, window_width, max_lines,
		UI_FONT, text_zoom * DEFAULT_SMALL_RATIO);
}
static __inline__ void draw_string_small_shadowed_zoomed(int x, int y,
	const unsigned char* our_string, int max_lines, float fr, float fg, float fb,
	float br, float bg, float bb, float text_zoom)
{
	draw_string_shadowed_zoomed_font(x, y, our_string, max_lines, fr, fg, fb,
		br, bg, bb, UI_FONT, text_zoom * DEFAULT_SMALL_RATIO);
}


#ifdef	ELC
/*!
 * \ingroup text_font
 * \brief   draws the string \a our_string in-game at the desired position (\a x, \a y), with the specified font scalings.
 *
 *      Draws the string \a our_string in-game at the desired position (\a x, \a y), with the specified font scalings.
 *
 * \param x             x coordinate of the position to draw the string
 * \param y             y coordinate of the position to draw the string
 * \param our_string    the string to draw
 * \param max_lines
 * \param font          the category of the font in which to draw the string
 * \param font_x_scale  font scaling factor in x direction
 * \param font_y_scale  font scaling factor in y direction
 *
 * \callgraph
 */
void draw_ingame_string(float x, float y, const unsigned char * our_string,
	int max_lines, font_cat font, float font_x_scale, float font_y_scale);
void draw_ortho_ingame_string(float x, float y, float z, const unsigned char* our_string,
	int max_lines, font_cat font, float font_x_scale, float font_y_scale);
#endif	//ELC

/*!
 * \ingroup text_font
 * \brief   sets the current font to the one given in \a cat.
 *
 *      Sets the current font to the one for category \a cat.
 *
 * \param cat  The font category to use
 * \retval int
 */
int	set_font(font_cat cat);

/*!
 * \ingroup other
 * \brief   intializes the font engine
 *
 *      Initializes the font engine
 *
 * \sa init_stuff
 * \retval int 0 on failure, 1 on success
 * \callgraph
 */
int init_fonts();

/*!
 * \ingroup other
 * \brief   loads the font textures
 *
 *      loads the font textures
 *
 * \sa init_fonts
 * \sa init_stuff
 * \retval int 0 on failure, 1 on success
 * \callgraph
 */
int load_font_textures ();

/*!
 * \ingroup text_font
 *
 * \brief Check if a character \a c has a glyph.
 *
 * Check if a glyph is defined for a character \a c, i.e. if it can be printed.
 *
 * \param c The character to check
 * \return 1 if the character is printable, 0 otherwise
 */
int has_glyph(unsigned char c);

void cleanup_fonts(void);

int get_line_height(font_cat cat, float zoom);

#ifdef TTF
int add_all_ttf_files(const char* dir_name);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif

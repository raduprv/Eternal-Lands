/*!
 * \file
 * \ingroup text_font
 * \brief the font structure and related functions.
 */
#ifndef __FONT_H__
#define __FONT_H__

#include "text.h"
#include "widgets.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \name contstant sizes
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

extern int	chat_font; /*!< font size used for chat font */
extern int	name_font; /*!< font size used for name font */

void recolour_message(text_message *msg);

void recolour_messages(text_message *msgs);


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
 * \param text_zoom	the size of the text
 * \param[in,out]       select information about current selection. draw_messages fills select->lines array.
 *
 * \callgraph
 */
void draw_messages (int x, int y, text_message *msgs, int msgs_size, Uint8 filter, int nr_start, int offset_start, int cursor, int width, int height, float text_zoom, select_info* select);

/*!
 * \ingroup text_font
 * \brief   draws the given string \a our_string at the desired position (\a x, \a y).
 *
 *      Draws the given string \a our_string at the desired position (\a x, \a y).
 *
 * \param x             x coordinate of the position to draw the string
 * \param y             y coordinate of the position to draw the string
 * \param our_string    the string to draw
 * \param max_lines
 *
 * \callgraph
 */
int draw_string(int x, int y, const unsigned char * our_string, int max_lines);
int draw_string_shadowed (int x, int y, const unsigned char * our_string, int max_lines, float fr,float fg,float fb, float br,float bg,float bb);
int draw_string_shadowed_width (int x, int y, const unsigned char * our_string, int max_width, int max_lines, float fr,float fg,float fb, float br,float bg,float bb);
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
 * \param text_zoom     the zoom factor to use
 *
 * \callgraph
 */
int draw_string_zoomed (int x, int y,const unsigned char * our_string,int max_lines, float text_zoom);

int draw_string_zoomed_width (int x, int y, const unsigned char * our_string, int max_width, int max_lines, float text_zoom);
int draw_string_width(int x, int y, const unsigned char * our_string, int max_width, int max_lines);
/*!
 * \ingroup text_font
 * \brief   draws the given string \a our_string at the desired position (\a x, \a y).
 *
 *      Draws the given string \a our_string at the desired position (\a x, \a y), clipping characters that would be drawn to the right of \a width or below \a height.
 *
 * \param x             x coordinate of the position to draw the string
 * \param y             y coordinate of the position to draw the string
 * \param our_string    the string to draw
 * \param width		the maximum width
 * \param height	the maximum height
 *
 * \callgraph
 */
void draw_string_clipped(int x, int y, const unsigned char * our_string, int width, int height);

/*!
 * \ingroup text_font
 * \brief   draws the given string \a our_string at the desired position (\a x, \a y) with a zoom factor of \a text_zoom.
 *
 *      Draws the given string \a our_string at the desired position (\a x, \a y) with a zoom factor of \a text_zoom, clipping characters that would be drawn to the right of \a width or below \a height.
 *
 * \param x             x coordinate of the position to draw the string
 * \param y             y coordinate of the position to draw the string
 * \param our_string    the string to draw
 * \param cursor_pos    if true, the string will be drawn at the current cursor position
 * \param width		the maximum width
 * \param height	the maximum height
 * \param text_zoom     the zoom factor to use
 *
 * \callgraph
 */
void draw_string_zoomed_clipped(int x, int y, const unsigned char * our_string, int cursor_pos, int width, int height, float text_zoom);

/*
void draw_window_string (int x, int y, const unsigned char *str, int width, int height, float text_zoom);
*/

/*!
 * \ingroup text_font
 * \brief   recompute where the line breaks in a string should occur
 *
 *      Recomputes the positions in the string where line breaks should be placed so that the string fits into a window.
 *
 * \param str		the string
 * \param len		the length of the string
 * \param size		the size of the string buffer
 * \param zoom		the scale factor for the text
 * \param width		the width of the text window
 * \param cursor	pointer to the cursor position, or NULL if not used
 * \param max_line_width	pointer the maximum line length after wrapping, or NULL if not used
 *
 * \retval int the number of window lines the string will use
 * \callgraph
 */
int reset_soft_breaks (char *str, int len, int size, float zoom, int width, int *cursor, float *max_line_width);

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
 *
 * \callgraph
 */
void draw_string_small(int x, int y,const unsigned char * our_string,int max_lines);
void draw_string_small_shadowed(int x, int y,const unsigned char * our_string,int max_lines, float fr, float fg, float fb, float br, float bg, float bb);

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
 * \param font_x_scale  font scaling factor in x direction
 * \param font_y_scale  font scaling factor in y direction
 *
 * \callgraph
 */
void draw_ingame_string(float x, float y, const unsigned char * our_string, int max_lines, float font_x_scale, float font_y_scale);
void draw_ortho_ingame_string(float x, float y, float z, const unsigned char * our_string, int max_lines, float font_x_scale, float font_y_scale);
#endif	//ELC

/*!
 * \ingroup text_font
 * \brief   returns the width of \a cur_char
 *
 *      Returns the width of char \a cur_char
 *
 * \param cur_char  the char to get the width for
 * \retval int
 * \callgraph
 */
int get_char_width(unsigned char cur_char);

/*!
 * \ingroup text_font
 * \brief   returns the width of the string \a str.
 *
 *      Returns the width of the string \a str
 *
 * \param str   the string which width to return
 * \retval int
 * \sa get_nstring_width
 * \callgraph
 */
int get_string_width(const unsigned char *str);

/*!
 * \ingroup text_font
 * \brief   sets the current font to the one given in \a num.
 *
 *      Sets the current font to the one given in \a num.
 *
 * \param num       the index in the \see fonts variable that defines the font to be used.
 * \retval int
 */
int	set_font(int num);

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

#ifndef	NEW_TEXTURES
/*!
 * \ingroup other
 * \brief Reloads the font textures
 *
 * 	Reloads the font textures (call this when changing resolution)
 *
 * \sa load_font_textures;
 */
void reload_fonts();
#endif	/* NEW_TEXTURES */

void cleanup_fonts(void);

int get_font_char(unsigned char cur_char);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

/*!
 * \file
 * \ingroup text_font
 * \brief the font structure and related functions.
 */
#ifndef __FONT_H__
#define __FONT_H__

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in font.c, no need to declare them here.
 */
//*!
// * \name constants for fonts
// */
//*! @{ */
//#define FONT_START_CHAR	32	/*!< first character in font.bmp */
//#define FONT_CHARS_PER_LINE	14	/*!< how manu chars per line in font.bmp? */
//#define	FONT_X_SPACING	18	/*!< X spacing of font in font.bmp */
//#define	FONT_Y_SPACING	21	/*!< Y spacing of font in font.bmp */
//*! @} */

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
/*! @} */

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in font.c, no need to declare it here.
 */
//*!
// * font info structure
// */
//typedef struct	{
//	int	spacing;
//	int texture_id; /*!< id of the texture used for the font */
//	int	widths[10*FONT_CHARS_PER_LINE];
//	char name[32]; /*!< name of the font */
//} font_info;

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in font.c, no need to declare them here.
 */
//extern int	cur_font_num; /*!< font index of currently used font */
//extern int	max_fonts; /*!< max. number of fonts in \see fonts */
//extern	font_info	*fonts[]; /*!< an array of pointers to \see font_info structs. Contains max. \see max_fonts fonts. */

extern int	chat_font; /*!< font size used for chat font */
extern int	name_font; /*!< font size used for name font */


void draw_messages (int x, int y, const text_message *msgs, int msgs_size, int nr_start, int offset_start, int cursor, int width, int height, float text_zoom);

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
 * \param str	the string
 * \param zoom	the scale factor for the text
 * \param width	the width of the text window
 *
 * \retval int the number of window lines the string will use
 * \callgraph
 */
int reset_soft_breaks (char *str, float zoom, int width);

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

/*
 * OBSOLETE: Queued for removal from this file.
 * Only used in actors.c, no need to declare them here. They are declared in actors.c however.
 */
//*!
// * \name ingame text drawing macros
// */
//*! @{ */
//#define DRAW_INGAME_NORMAL(x, y, our_string, max_lines)	draw_ingame_string(x, y, our_string, max_lines, INGAME_FONT_X_LEN, INGAME_FONT_Y_LEN)
//#define DRAW_INGAME_SMALL(x, y, our_string, max_lines)	draw_ingame_string(x, y, our_string, max_lines, SMALL_INGAME_FONT_X_LEN, SMALL_INGAME_FONT_Y_LEN)
//#define DRAW_INGAME_ALT(x, y, our_string, max_lines)	draw_ingame_string(x, y, our_string, max_lines, ALT_INGAME_FONT_X_LEN, ALT_INGAME_FONT_Y_LEN)
//*! @} */
#endif	//ELC

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup text_font
// * \brief   displays a single char \a cur_char at the desired position (\a cur_x, \a cur_y) with the specified font scalings.
// *
// *      Draws a single char \a cur_char at the desired position (\a cur_x, \a cur_y) with the specified font scalings.
// *
// * \param cur_char                  the char to draw
// * \param cur_x                     x coordinate of the position to draw the string
// * \param cur_y                     y coordinate of the position to draw the string
// * \param displayed_font_x_size     the size of the font in x direction as displayed
// * \param displayed_font_y_size     the size of the font in y direction as displayed
// * \retval int
// * \callgraph
// */
//int	draw_char_scaled(unsigned char cur_char, int cur_x, int cur_y, float displayed_font_x_size, float displayed_font_y_size);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup text_font
// * \brief   returns the index of the font used by \a cur_char
// *
// *      Returns the index of the font used by \a cur_char
// *
// * \param cur_char  the char to get the font info for.
// * \retval int
// */
//int get_font_char(unsigned char cur_char);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup text_font
// * \brief   search for the font of \a cur_char and returns the index if found.
// *
// *      Search for the font of \a cur_char and returns the index if found
// *
// * \param cur_char  the char to search for
// * \retval int
// *
// * \sa get_font_char
// */
//int find_font_char(unsigned char cur_char);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup text_font
// * \brief   returns the font width of \a cur_char
// *
// *      Returns the font width of \a cur_char
// *
// * \param cur_char  the char to get the font width for
// * \retval int
// */
//int get_font_width(int cur_char);

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

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup text_font
// * \brief   returns the width of the first \a len bytes of \a str.
// *
// *      Returns the width of the first \a len bytes of \a str
// *
// * \param str       the string for which the width should returned
// * \param len       the length until which \a str should be used
// * \retval int
// * \sa get_string_width
// * \callgraph
// */
//int get_nstring_width(const unsigned char *str, int len);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup text_font
// * \brief   sets parameters for font \a num
// *
// *      sets parameters for font \a num, but does not load the texture yet
// *
// * \param num       the number of the font
// * \retval int -1 on failure, \a num on success
// * \callgraph
// */
//int set_font_parameters (int num);

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

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup text_font
// * \brief   removes the font with the given index \a num from the \see fonts variable.
// *
// *      Removes the font with the given index \a num from the \see fonts variable.
// *
// * \param num       index of the font to remove.
// */
//void remove_font(int num);

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

#endif

/*!
 * \file
 * \ingroup text_font
 * \brief   Text handling
 */
#ifndef __TEXT_H__
#define __TEXT_H__

#include <SDL_types.h>

#define DISPLAY_TEXT_BUFFER_SIZE 5000 /*!< maximum number of lines in the text buffer */

#define CHAT_ALL	((Uint8) -1)
#define CHAT_NONE	((Uint8) -2)

#define FILTER_LOCAL	CHAT_LOCAL
#define FILTER_PERSONAL	CHAT_PERSONAL
#define FILTER_GM	CHAT_GM
#define FILTER_SERVER	CHAT_SERVER
#define FILTER_MOD	CHAT_MOD
#define FILTER_CHANNEL1	CHAT_CHANNEL1
#define FILTER_CHANNEL2	CHAT_CHANNEL2
#define FILTER_CHANNEL3	CHAT_CHANNEL3
#define FILTER_ALL	CHAT_ALL
#define FILTER_NONE	CHAT_NONE

typedef struct
{
	Uint8 chan_idx;
	Uint32 channel;
	Uint16 len, size;
	Uint8 *data;
	Uint16 wrap_width;
	float wrap_zoom;
	Uint8 wrap_lines;
	char deleted;
} text_message;

extern text_message display_text_buffer[DISPLAY_TEXT_BUFFER_SIZE];
extern int last_message;
extern int buffer_full;
extern int total_nr_lines; /*!< The number of lines in the text buffer */
extern Uint8 current_filter;

extern float chat_zoom; /*!< zoom factor for chat text */

extern text_message input_text_line; /*!< user input text */

extern int display_text_buffer_first;
extern int display_text_buffer_last;

extern char last_pm_from[32]; /*!< actor name from whom the last pm arrived */

extern Uint32 last_server_message_time; /*!< timestamp of the last server message */
extern int lines_to_show; /*!< number of lines to show at once */
extern int max_lines_no;

extern char not_from_the_end_console;

extern int log_chat; /*!< flag stating whether to log server messages or not */

extern int current_text_width; /*!< Current wrapping width for text buffers */

/*!
 * \ingroup text_font
 * \brief   Initializes the text buffers
 *
 *      Initializes the text buffers.
 */
void init_text_buffers ();

/*!
 * \ingroup text_font
 * \brief   Writes the given data up to a length of len to the logfile.
 *
 *      Writes the given data up to a length of len to the logfile.
 *
 * \param data  The data to write to the logfile
 * \param len   The length of data.
 */
void write_to_log(Uint8 * data,int len);

/*!
 * \ingroup text_font
 * \brief   Sends the current input text to the server
 *
 *      Sends the current text in input_text_line to the server.
 *
 * \param line The text to send
 * \param len The length of the text
 */
void send_input_text_line (char *line, int len);

/*!
 * \ingroup text_font
 * \brief Adds the string in text_to_add up to the specified length to the filtered text.
 *
 *      Adds the string in text_to_add up to the specified length to the filtered text.
 *
 * \param text_to_add   the string to add
 * \param len           the length of text_to_add
 * \retval int
 * \callgraph
 */
int filter_or_ignore_text(unsigned char *text_to_add, int len, int size);

/*!
 * \ingroup text_font
 * \brief   Puts a character in a buffer
 *
 *      Puts the given character \a ch into text message \a buf at position \a pos
 *
 * \param buf	(pointer to) the message
 * \param ch    the character to add
 * \param pos	the position at which the character is to be placed
 * \retval int	1 if a character is inserted, 0 otherwise
 * \callgraph
 */
int put_char_in_buffer (text_message *buf, Uint8 ch, int pos);

/*!
 * \ingroup text_font
 * \brief    Inserts a string in a buffer
 *
 *      Inserts the given string \a str into the text buffer at position \a pos
 *
 * \param buf	(pointer to) the message
 * \param str	the string to add
 * \param pos	the position at which the string is to be placed
 * \retval int	the number of characters inserted
 * \callgraph
 */
int put_string_in_buffer (text_message *buf, const Uint8 *str, int pos);

/*!
 * \ingroup text_font
 * \brief Adds the string in text_to_add up to the specified length in the text buffer.
 *
 *      Adds the string in text_to_add up to the specified length to the text buffer. If x_chars_limit is !=0 then words in the filter list will get filtered and replaced.
 *
 * \param channel	the channel index of the message
 * \param text_to_add   the string to add to the buffer
 * \param len           the length of text_to_add
 *
 * \callgraph
 */
void put_text_in_buffer (Uint8 channel, const Uint8 *text_to_add, int len);

/*!
 * \ingroup text_font
 * \brief Works like \ref put_text_in_buffer, but the text will be in the specified color.
 *
 *      Works like \ref put_text_in_buffer, but the text will be in the specified color.
 *
 * \param color         the color of the text
 * \param channel	the channel index of the message
 * \param text_to_add   the string to add to the buffer
 * \param len           the length of text_to_add
 *
 * \callgraph
 */
void put_colored_text_in_buffer (Uint8 color, Uint8 channel, const Uint8 *text_to_add, int len);

/*!
 * \ingroup text_font
 * \brief put_small_text_in_box
 *
 *      put_small_text_in_box(unsigned char*,int,int,char*)
 *
 * \param text_to_add   the string to add
 * \param len           the length of text_to_add
 * \param pixels_limit
 * \param buffer
 *
 * \callgraph
 *
 * \todo Fix documentation
 */
void put_small_text_in_box (const Uint8 *text_to_add, int len, int pixels_limit, char *buffer);

/*!
 * \ingroup text_font
 * \brief put_small_colored_text_in_box
 *
 *      put_small_colored_text_in_box(unsigned char*,int,int,char*)
 *
 * \param color	the color
 * \param text_to_add   the string to add
 * \param len           the length of text_to_add
 * \param pixels_limit
 * \param buffer
 *
 * \callgraph
 *
 * \todo Fix documentation
 */
void put_small_colored_text_in_box (Uint8 color, const Uint8 *text_to_add, int len, int pixels_limit, char *buffer);

/*!
 * \ingroup text_font
 * \brief find_last_lines_time
 *
 *      find_last_lines_time(int *, int *, Uint8 filter, int width)
 *
 * \retval int
 *
 * \todo Fix documentation
 */
int find_last_lines_time (int *msg, int *offset, Uint8 filter, int width);

/*!
 * \ingroup text_font
 * \brief Finds the position of the beginning of a line
 *
 *	finds the position of the beginning of a line in the text message buffer
 *
 * \param nr_lines The total number of lines
 * \param line     The number of the line to be found
 * \param msg      The message in which the lines is located
 * \param channel  the channel in which to search for the line, or CHANNEL_ALL to search in all channels
 * \param offset   The offset in the message at which the line starts
 * \param zoom     the text zoom that is to be used for wrapping
 * \param width    the text width that is to be used for wrapping
 * \retval The position of the beginning of the line
 */
int find_line_nr (int nr_lines, int line, Uint8 filter, int *msg, int *offset, float zoom, int width);

/*!
 * \ingroup interface_console
 * \brief Moves the screen up one line in console mode.
 *
 *      Moves the screen in console mode one line up.
 *
 * \sa console_move_down, \sa console_move_page_down, \sa console_move_page_up
 */
void console_move_up();

/*!
 * \ingroup interface_console
 * \brief Moves the screen in console mode one line down
 *
 *      Moves the screen down one line in console mode.
 *
 * \sa console_move_up, \sa console_move_page_down, \sa console_move_page_up
 */
void console_move_down();

/*!
 * \ingroup interface_console
 * \brief moves the screen down one page in console mode
 *
 *      Moves the screen in console mode down one page.
 *
 * \sa console_move_page_up, \sa console_move_down, \sa console_move_up
 * \callgraph
 */
void console_move_page_down();

/*!
 * \ingroup interface_console
 * \brief Moves the screen up one page in console mode
 *
 *      Moves the screen in console mode up one page.
 *
 * \sa console_move_page_down, \sa console_move_up, \sa console_move_down
 * \callgraph
 */
void console_move_page_up();

// XXX FIXME (Grum): obsolete
//*!
// * \ingroup interface_console
// * \brief displays the console text
// *
// *      Switches to console mode and displays the txt.
// *
// * \callgraph
// */
//void display_console_text();


/*!
 * \ingroup text_font
 * \brief Clears the text buffer
 *
 *      clears the text buffer
 *
 * \callgraph
 */
void clear_display_text_buffer ();

/*!
 * \ingroup text_font
 * \brief   Rewraps the a text buffer.
 *
 *      Rewraps a text buffer.
 *
 * \param msg    pointer to the message to rewrap
 * \param zoom   the text zoom to use for wrapping
 * \param width  the max width of a line
 * \param cursor cursor passed to \sa reset_soft_breaks
 * \retval       the number of lines after wrapping
 */
int rewrap_message(text_message * buf, float zoom, int width, int * cursor);


void cleanup_text_buffers(void);

#define LOG_TO_CONSOLE(color,buffer)	put_colored_text_in_buffer(color,CHAT_SERVER,(const Uint8*)buffer,-1) /*!< logs the text in buffer with the specified color to the console. */

#endif

/*!
 * \file
 * \ingroup text_font
 * \brief   Text handling
 */
#ifndef __TEXT_H__
#define __TEXT_H__

#include <stdlib.h>
#include <SDL_types.h>
#include "platform.h"
#include "eye_candy_wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DISPLAY_TEXT_BUFFER_SIZE 5000 /*!< maximum number of lines in the text buffer */
#define DISPLAY_TEXT_BUFFER_DEL 100 /*!< number of lines to discard when buffer is full */

#define CHAT_ALL	((Uint8) -1)
#define CHAT_NONE	((Uint8) -2)
#define CHAT_LIST	((Uint8) -3)
#define CHAT_HIST	((Uint8) -4)

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

#define LOG_NONE				0
#define LOG_CHAT				1
#define LOG_SERVER				2
#define LOG_SERVER_SEPERATE		3

typedef struct
{
	Uint8 chan_idx;
	Uint32 channel;
	Uint16 len, size;
	char *data;
	Uint16 wrap_width;
	float wrap_zoom;
	Uint8 wrap_lines;
	Uint8 deleted;
	float max_line_width;
	float r, g, b;
} text_message;

extern text_message display_text_buffer[DISPLAY_TEXT_BUFFER_SIZE];
extern int last_message;
extern Uint8 current_filter;

extern float chat_zoom; /*!< zoom factor for chat text */

extern text_message input_text_line; /*!< user input text */

extern char last_pm_from[32]; /*!< actor name from whom the last pm arrived */

extern Uint32 last_server_message_time; /*!< timestamp of the last server message */
extern int lines_to_show; /*!< number of lines to show at once */

extern int show_timestamp;

extern int dark_channeltext;

extern int log_chat; /*!< flag stating whether to log server messages or not */

extern ec_reference harvesting_effect_reference;

extern int emote_filter; //used to ignore text lines of emotes only

extern int summoning_filter; //used to ignore text lines of summoning messages

/*!
 * \brief Allocate the character buffer for a text_message
 *
 *	Allocates memory for the character buffer of text_message \a msg, 
 *	and initializes its size and used length.
 *
 * \param msg  The text_message for which to allocate a buffer
 * \param size The desired size of the buffer
 */
void alloc_text_message_data (text_message *msg, int size);

/*!
 * \brief Resize the character buffer of a text_message
 *
 *	If necessary, resize the character buffer of text_message \a msg, 
 *	so that it can hold at least a (zero-terminated) string of \a len 
 *	characters. If \a len is less than the current allocated size,
 *	nothing is done, otherwise the array is doubled in size until
 *	it is large enough. The existing data in the text_message's 
 *	buffer is preserved.
 *
 * \param msg The text_message that should be resized
 * \param len The new minimum size of the character string
 */
void resize_text_message_data (text_message *msg, int len);

/*!
 * \brief Clear the text string of a text_message
 *
 *	Set the text of text_message \a msg to an empty string.
 *
 * \param msg The text message to be cleared
 */
static __inline__ void clear_text_message_data (text_message *msg)
{
	msg->len = 0;
	if (msg->size > 0)
		msg->data[0] = '\0';
}

/*!
 * \brief Copy a string into a text_message
 *
 *	Copy string \a data into the character buffer of text_message 
 *	\a msg. Note that if the current character buffer is too small 
 *	to hold all of \a data, only the first part is copied.
 *
 * \param msg  The text_message to be updated
 * \patam data The string to be copied
 * \sa resize_text_message_data()
 */
void set_text_message_data (text_message *msg, const char* data);

/*!
 * \brief Free a text_message's character buffer
 *
 *	Free the memory allocated for the character buffer in 
 *	text_message \a msg.
 *
 * \param msg The text_message whose character buffer will be freed
 */
static __inline__ void free_text_message_data (text_message *msg)
{
	if (msg->size > 0)
	{
		free (msg->data);
		msg->data = NULL;
		msg->len = msg->size = 0;
	}
}

/*!
 * \brief Set the color of a text_message
 *
 *	Set the color in which the text of text_message \a msg should 
 *	be drawn. Note that this color is inly used until the first
 *	color character in the text is encountered.
 *
 * \param msg The text message for which to set the color
 * \param r   The red component of the new text color 
 * \param g   The green component of the new text color 
 * \param b   The blue component of the new text color 
 */
static __inline__ void set_text_message_color (text_message *msg, float r, float g, float b)
{
	msg->r = r;
	msg->g = g;
	msg->b = b;
}

/*!
 * \brief Initialize a text_message
 *
 *	Initialize text_message \a msg by allocating a character buffer
 *	of \a size bytes, and setting the other fields to default 
 *	values.
 *
 * \param msg  The text_message to be initialized
 * \param size The initial size of the \a msg's characetr buffer
 */
static __inline__ void init_text_message (text_message *msg, Uint16 size)
{
	msg->chan_idx = CHAT_NONE;
	msg->channel = 0;
	alloc_text_message_data (msg, size);		
	msg->wrap_width = 0;
	msg->wrap_zoom = 1.0f;
	msg->wrap_lines = 0;
	msg->deleted = 0;
	msg->max_line_width = 0.0f;
	set_text_message_color (msg, -1.0f, -1.0f, -1.0f);
}

/*!
 * \brief Whether text message \a msg is empty
 */
static __inline__ int text_message_is_empty (const text_message *msg)
{
	return msg->len == 0;
}

/*!
 * \ingroup text_font
 * \brief   Initializes the text buffers
 *
 *      Initializes the text buffers.
 */
void init_text_buffers ();

/*!
 * \ingroup text_font
 * \brief   Writes a timestamp to the logfile.
 *
 *      Writes a timestamp to the logfile.
 *
 */
void timestamp_chat_log();

/*!
 * \ingroup text_font
 * \brief   Writes the given data up to a length of \a len to a logfile.
 *
 *      Writes the given data up to a length of \a len to a logfile. 
 *
 * \param channel The channel index of the message
 * \param data    The data to write to the logfile
 * \param len     The length of data.
 */
void write_to_log (Uint8 channel, const Uint8* const data, int len);

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
int filter_or_ignore_text(char *text_to_add, int len, int size, Uint8 channel);

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
///*!
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



/*!
 * \ingroup text_font
 * \brief Start or stop the harvesting effect
 *
 *       Start or stop the harvesting eye candy effect dependent on the
 * 	state of the \sa harvesting flag
 *
 * \callgraph
 */
void check_harvesting_effect(void);


#define LOG_TO_CONSOLE(color,buffer)	put_colored_text_in_buffer(color,CHAT_SERVER,(const Uint8*)buffer,-1) /*!< logs the text in buffer with the specified color to the console. */

#ifdef __cplusplus
} // extern "C"
#endif

#endif

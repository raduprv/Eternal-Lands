/*!
 * \file
 * \ingroup text_font
 * \brief   Text handling
 */
#ifndef __TEXT_H__
#define __TEXT_H__

#define MAX_DISPLAY_TEXT_BUFFER_LENGTH 264128 /*!< max. buffer length for displayable text */
#define DISPLAY_TEXT_BUFFER_SIZE 5000 /*!< maximum number of lines in the text buffer */

#define CHANNEL_LOCAL	-1	/*!< local chat */
#define CHANNEL_GM	-2	/*!< guild messages */
#define CHANNEL_ALL	-3	/*!< personal and mod messages */

typedef struct
{
	int chan_nr;
	Uint16 len, size;
	Uint8 *data;
} text_message;

extern text_message display_text_buffer[DISPLAY_TEXT_BUFFER_SIZE];

extern float chat_zoom; /*!< zoom factor for chat text */

extern text_message input_text_line; /*!< user input text */
extern int total_nr_lines; /*!< The number of lines in the text buffer */

extern int display_text_buffer_first;
extern int display_text_buffer_last;

/*
 * OBSOLETE: Queued for removal from this file.
 * Unused variable.
 */
//extern int display_console_text_buffer_last;

extern char last_pm_from[32]; /*!< actor name from whom the last pm arrived */

extern Uint32 last_server_message_time; /*!< timestamp of the last server message */
extern int lines_to_show; /*!< number of lines to show at once */
extern int max_lines_no;

extern char not_from_the_end_console;

extern int log_server; /*!< flag stating whether to log server messages or not */

/*!
 * \ingroup text_font
 * \brief   Initializes the text buffers
 *
 *      Initializes the text buffers.
 */
void init_text_buffers ();

/*!
 * \ingroup text_font
 * \brief   Adjusts the soft line breaks in the text buffers
 *
 *      Adjusts the soft line breaks in the text buffers
 *
 * \param new_width the new text width in pixels
 */
void adjust_line_breaks (int new_width);

/*!
 * \ingroup text_font
 * \brief   Writes the given data up to a length of len to the logfile.
 *
 *      Writes the given data up to a length of len to the logfile.
 *
 * \param data  The data to write to the logfile
 * \param len   The lenght of data.
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
 * \brief adds the string in text_to_add up to the specified length to the filtered text.
 *
 *      Adds the string in text_to_add up to the specified length to the filtered text.
 *
 * \param text_to_add   the string to add
 * \param len           the length of text_to_add
 * \retval int
 * \callgraph
 */
int filter_or_ignore_text(unsigned char *text_to_add, int len);

/*!
 * \ingroup text_font
 * \brief   Puts a character in a buffer
 *
 *      Puts the given character \a ch into the text buffer at position \a pos
 *
 * \param ch    the character to add
 * \param pos	the position at which the character is to be placed
 * \retval int	1 if a character is inserted, 0 otherwise
 * \callgraph
 */
int put_char_in_buffer (Uint8 ch, int pos);

/*!
 * \ingroup text_font
 * \brief    Inserts a string in a buffer
 *
 *      Inserts the given string \a str into the text buffer at position \a pos
 *
 * \param str	the string to add
 * \param pos	the position at which the string is to be placed
 * \retval int	the number of characters inserted
 * \callgraph
 */
int put_string_in_buffer (const Uint8 *str, int pos);

/*!
 * \ingroup text_font
 * \brief adds the string in text_to_add up to the specified length in the text buffer.
 *
 *      Adds the string in text_to_add up to the specified length to the text buffer. If x_chars_limit is !=0 then words in the filter list will get filtered and replaced.
 *
 * \param text_to_add   the string to add to the buffer
 * \param len           the length of text_to_add
 * \param x_chars_limit flag indicating whether the text should be filtered (!=0) or not (==0).
 *
 * \callgraph
 */
void put_text_in_buffer(unsigned char *text_to_add, int len, int x_chars_limit);

/*!
 * \ingroup text_font
 * \brief works like put_text_in_buffer, but the text will be in the specified color.
 *
 *      Works like put_text_in_buffer, but the text will be in the specified color.
 *
 * \param color         the color of the text
 * \param text_to_add   the string to add to the buffer
 * \param len           the length of text_to_add
 * \param x_chars_limit flag indicating whether the text should be filtered (!=0) or not (==0).
 *
 * \sa put_text_in_buffer
 * \callgraph
 */
void put_colored_text_in_buffer(Uint8 color,unsigned char *text_to_add, int len, 
								int x_chars_limit);

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
 */
void put_small_text_in_box(unsigned char *text_to_add, int len, int pixels_limit, 
						   char *buffer);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup text_font
// * \brief works the same as put_small_text_in_box, but the text will be added in the specified color.
// *
// *      Works the same like put_small_text_in_box, but the text will be added in the specified color.
// *
// * \param color         the color of the text
// * \param text_to_add   the string to add
// * \param len           the length of text_to_add
// * \param pixels_limit
// * \param buffer
// *
// * \sa put_small_text_in_box
// */
//void put_small_colored_text_in_box(Uint8 color,unsigned char *text_to_add, int len, 
//								   int pixels_limit, char *buffer);

/*!
 * \ingroup text_font
 * \brief find_last_lines_time
 *
 *      find_last_lines_time(int *, int *)
 *
 * \retval int
 */
int find_last_lines_time (int *msg, int *offset);

/* OBSOLETE declaration: queued for removal from this header file */
//*!
// * \ingroup interface_console
// * \brief find_last_console_lines
// *
// *      find_last_console_lines(int)
// *
// * \param lines_no
// * \retval int
// */
//int find_last_console_lines(int lines_no);

/*!
 * \ingroup text_font
 * \brief finds the position of the beginning of a line
 *
 *	finds the position of the beginning of a line in the text buffer
 *
 * \param nr_lines The total number of lines
 * \param line The number of the line to be found
 * \param msg The message in which the lines is located
 * \param offset The offset in the message at which the line starts
 * \retval The position of the beginning of the line
 */
int find_line_nr (int nr_lines, int line, int *msg, int *offset);

/*!
 * \ingroup interface_console
 * \brief moves the screen up one line in console mode.
 *
 *      Moves the screen in console mode one line up.
 *
 * \sa console_move_down, \sa console_move_page_down, \sa console_move_page_up
 */
void console_move_up();

/*!
 * \ingroup interface_console
 * \brief moves the screen in console mode one line down
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
 * \brief moves the screen up one page in console mode
 *
 *      Moves the screen in console mode up one page.
 *
 * \sa console_move_page_down, \sa console_move_up, \sa console_move_down
 * \callgraph
 */
void console_move_page_up();

/*!
 * \ingroup interface_console
 * \brief displays the console text
 *
 *      Switches to console mode and displays the txt.
 *
 * \callgraph
 */
void display_console_text();

/*!
 * \ingroup text_font
 * \brief clears the text buffer
 *
 *      clears the text buffer
 *
 * \callgraph
 */
void clear_display_text_buffer ();

#define LOG_TO_CONSOLE(color,buffer)	put_colored_text_in_buffer(color,buffer,-1,0) /*!< logs the text in buffer with the specified color to the console. */

#endif

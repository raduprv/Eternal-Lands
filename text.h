/*!
 * \file
 * \ingroup text_font
 * \brief   Text handling
 */
#ifndef __TEXT_H__
#define __TEXT_H__

#define max_display_text_buffer_lenght 264128 /*!< max. buffer length for displayable text */

extern float chat_zoom; /*!< zoom factor for chat text */

extern char input_text_line[257]; /*!< user input text */
extern int input_text_lenght; /*!< actual length of \see input_text_line */
extern int input_text_lines; /*!< number of lines used by \see input_text_line. \todo check this desc. */
extern char display_text_buffer[max_display_text_buffer_lenght]; /*!< buffer to hold the text to display */
#ifndef OLD_EVENT_HANDLER
extern int nr_text_buffer_lines; /*!< The number of lines in the text buffer */
#endif

extern int display_text_buffer_first;
extern int display_text_buffer_last;

extern int display_console_text_buffer_first;
extern int display_console_text_buffer_last;
extern char last_pm_from[32]; /*!< actor name from whom the last pm arrived */

extern Uint32 last_server_message_time; /*!< timestamp of the last server message */
extern int lines_to_show; /*!< number of lines to show at once */
extern int max_lines_no;

extern char console_mode; /*!< flag that indicates whether we are in console mode or not */
extern char not_from_the_end_console;

extern int log_server; /*!< flag stating whether to log server messages or not */

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
 */
void send_input_text_line();

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
 * \brief   Puts the character ch in a buffer
 *
 *      Puts the given character into the text buffer
 *
 * \param ch    the character to add
 *
 * \callgraph
 */
void put_char_in_buffer(unsigned char ch);

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

/*!
 * \ingroup text_font
 * \brief works the same as put_small_text_in_box, but the text will be added in the specified color.
 *
 *      Works the same like put_small_text_in_box, but the text will be added in the specified color.
 *
 * \param color         the color of the text
 * \param text_to_add   the string to add
 * \param len           the length of text_to_add
 * \param pixels_limit
 * \param buffer
 *
 * \sa put_small_text_in_box
 */
void put_small_colored_text_in_box(Uint8 color,unsigned char *text_to_add, int len, 
								   int pixels_limit, char *buffer);

/*!
 * \ingroup text_font
 * \brief find_last_lines_time
 *
 *      find_last_lines_time()
 *
 * \retval int
 */
int find_last_lines_time();

/*!
 * \ingroup interface_console
 * \brief find_last_console_lines
 *
 *      find_last_console_lines(int)
 *
 * \param lines_no
 * \retval int
 */
int find_last_console_lines(int lines_no);

#ifndef OLD_EVENT_HANDLER
/*!
 * \ingroup text_font
 * \brief finds the position of the beginning of a line
 *
 *	finds the position of the beginning of a line in the text buffer
 *
 * \param line The number of the line to be found
 * \retval The position of the beginning of the line
 */
int find_line_nr (int line);
#endif

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

#define log_to_console(color,buffer)	put_colored_text_in_buffer(color,buffer,-1,0) /*!< logs the text in buffer with the specified color to the console. */

#endif

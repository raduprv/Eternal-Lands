/*!
 * \file
 * \brief   Text handling
 * \ingroup text_font
 */
#ifndef __TEXT_H__
#define __TEXT_H__

#define max_display_text_buffer_lenght 264128 /*!< max. buffer length for displayable text */

extern float chat_zoom; /*!< zoom factor for chat text */

extern char input_text_line[257]; /*!< user input text */
extern int input_text_lenght; /*!< actual length of \see input_text_line */
extern int input_text_lines; /*!< number of lines used by \see input_text_line. \todo check this desc. */
extern char display_text_buffer[max_display_text_buffer_lenght]; /*!< buffer to hold the text to display */

extern int display_text_buffer_first;
extern int display_text_buffer_last;

extern int display_console_text_buffer_first;
extern int display_console_text_buffer_last;
extern char last_pm_from[32]; /*!< actor name from whom the last pm arrived */

extern Uint32 last_server_message_time; /*!< timestamp of the last server message */
extern int lines_to_show; /*!< number of lines to show at once */
extern int max_lines_no;

extern char console_mode;
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
 *      Sends the current text in \see input_text_line to the server.
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
 * \return int
 */
int filter_or_ignore_text(unsigned char *text_to_add, int len);

/*!
 * \ingroup text_font
 * \brief   Puts the character ch in a buffer
 *
 *      Puts the given character into the text buffer
 *
 * \param ch    the character to add
 * \return None
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
 * \return None
 */
void put_text_in_buffer(unsigned char *text_to_add, int len, int x_chars_limit);

/*!
 * \ingroup text_font
 * \brief works like \see put_text_in_buffer, but the text will be in the specified color.
 *
 *      Works like \see put_text_in_buffer, but the text will be in the specified color.
 *
 * \param color         the color of the text
 * \param text_to_add   the string to add to the buffer
 * \param len           the length of text_to_add
 * \param x_chars_limit flag indicating whether the text should be filtered (!=0) or not (==0).
 * \return None
 *
 * \sa put_text_in_buffer
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
 * \return None
 */
void put_small_text_in_box(unsigned char *text_to_add, int len, int pixels_limit, 
						   char *buffer);

/*!
 * \ingroup text_font
 * \brief works the same as \see put_small_text_in_box, but the text will be added in the specified color.
 *
 *      Works the same like \see put_small_text_in_box, but the text will be added in the specified color.
 *
 * \param color         the color of the text
 * \param text_to_add   the string to add
 * \param len           the length of text_to_add
 * \param pixels_limit
 * \param buffer
 * \return None
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
 * \return int
 */
int find_last_lines_time();

/*!
 * \ingroup interface_console
 * \brief find_last_console_lines
 *
 *      find_last_console_lines(int)
 *
 * \param lines_no
 * \return None
 */
int find_last_console_lines(int lines_no);

/*!
 * \ingroup interface_console
 * \brief moves the screen up one line in console mode.
 *
 *      Moves the screen in console mode one line up.
 *
 * \return None
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
 * \return None
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
 * \return None
 *
 * \sa console_move_page_up, \sa console_move_down, \sa console_move_up
 */
void console_move_page_down();

/*!
 * \ingroup interface_console
 * \brief moves the screen up one page in console mode
 *
 *      Moves the screen in console mode up one page.
 *
 * \return None
 *
 * \sa console_move_page_down, \sa console_move_up, \sa console_move_down
 */
void console_move_page_up();

/*!
 * \ingroup interface_console
 * \brief displays the console text
 *
 *      Switches to console mode and displays the txt.
 *
 * \return None
 */
void display_console_text();

#define log_to_console(color,buffer)	put_colored_text_in_buffer(color,buffer,-1,0) /*!< logs the text in buffer with the specified color to the console. */

#endif


#ifndef __TEXT_H__
#define __TEXT_H__

#define max_display_text_buffer_lenght 264128

extern float chat_zoom;

extern char input_text_line[257];
extern int input_text_lenght;
extern int input_text_lines;
extern char display_text_buffer[max_display_text_buffer_lenght];

extern int display_text_buffer_first;
extern int display_text_buffer_last;

extern int display_console_text_buffer_first;
extern int display_console_text_buffer_last;
extern char last_pm_from[32];

extern Uint32 last_server_message_time;
extern int lines_to_show;
extern int max_lines_no;

extern char console_mode;
extern char not_from_the_end_console;

extern int log_server;

void write_to_log(Uint8 * data,int len);
void send_input_text_line();
int filter_or_ignore_text(unsigned char *text_to_add, int len);
void put_char_in_buffer(unsigned char ch);
void put_text_in_buffer(unsigned char *text_to_add, int len, int x_chars_limit);
void put_colored_text_in_buffer(Uint8 color,unsigned char *text_to_add, int len, 
								int x_chars_limit);
void put_small_text_in_box(unsigned char *text_to_add, int len, int pixels_limit, 
						   char *buffer);
void put_small_colored_text_in_box(Uint8 color,unsigned char *text_to_add, int len, 
								   int pixels_limit, char *buffer);
int find_last_lines_time();
int find_last_console_lines(int lines_no);
void console_move_up();
void console_move_down();
void console_move_page_down();
void console_move_page_up();
void display_console_text();
#define log_to_console(color,buffer)	put_colored_text_in_buffer(color,buffer,-1,0)

#endif

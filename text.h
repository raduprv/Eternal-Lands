#ifndef __TEXT_H__
#define __TEXT_H__

#define max_display_text_buffer_lenght 264128

#define SMALL_INGAME_FONT_X_LEN 0.12
#define SMALL_INGAME_FONT_Y_LEN 0.17

extern char input_text_line[170];
extern int input_text_lenght;
extern char display_text_buffer[max_display_text_buffer_lenght];

extern int display_text_buffer_first;
extern int display_text_buffer_last;

extern int display_console_text_buffer_first;
extern int display_console_text_buffer_last;


extern Uint32 last_server_message_time;
extern int lines_to_show;
extern int max_lines_no;

extern char console_mode;
extern char not_from_the_end_console;

int filter_or_ignore_text(unsigned char *text_to_add, int len);
void put_text_in_buffer(unsigned char *text_to_add, int len, int x_chars_limit);

#endif

#ifndef __NOTEPAD_H__
#define __NOTEPAD_H__

#ifdef NOTEPAD
unsigned int get_edit_pos(unsigned short x, unsigned short y, char *str, unsigned int maxchar, float text_zoom);
int put_char_in_custom(text_message *buf, Uint8 ch, int pos);
int put_string_in_custom(text_message *buf, const Uint8 *str, int pos);
int click_input_handler(widget_list *w, int mx, int my);
int keypress_input_handler(widget_list *w, int mx, int my, Uint32 key, Uint32 unikey);
int keypress_input_window_handler(window_info *win, int mx, int my, Uint32 key, Uint32 unikey);
void display_popup_win(char* label, int maxlen, void (*OnReturn)());
void display_notepad();
#endif

#endif

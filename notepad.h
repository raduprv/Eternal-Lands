#ifndef __NOTEPAD_H__
#define __NOTEPAD_H__

#ifdef NOTEPAD
extern int notepad_loaded;

unsigned int get_edit_pos(unsigned short x, unsigned short y, char *str, unsigned int maxchar, float text_zoom);
void display_popup_win(char* label, int maxlen);
void display_notepad();
int notepadSaveFile();
#endif

#endif

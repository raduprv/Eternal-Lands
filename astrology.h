#ifndef __ASTROLOGY_H__
#define __ASTROLOGY_H__

#include "items.h"
#include "elwindows.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int astrology_win_x;
extern int astrology_win_y;
extern int astrology_win_x_len;
extern int astrology_win_y_len;

extern int astrology_win;

void display_astrology_window();
int is_astrology_message (const char * RawText);

extern float load_bar_colors[12];

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__ASTROLOGY_H__

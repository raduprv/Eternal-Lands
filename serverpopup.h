#ifndef __POPUPWIN_H__
#define __POPUPWIN_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int server_popup_win;
extern int server_popup_win_x;
extern int server_popup_win_y;
extern int use_server_pop_win;
extern int server_pop_chan;

void display_server_popup_win(const char * const message);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

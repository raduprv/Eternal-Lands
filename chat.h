#ifndef __CHAT_H__
#define __CHAT_H__

#ifdef WINDOW_CHAT

#define CHAT_WIN_TEXT_WIDTH   500
#define CHAT_WIN_TEXT_HEIGHT  (18*10) // 10 lines in normal font size
#define CHAT_WIN_SCROLL_WIDTH 20

extern int use_windowed_chat;

extern int chat_win_text_width;

void update_chat_scrollbar ();

void display_chat ();

#endif // WINDOW_CHAT

#endif // def __CHAT_H__

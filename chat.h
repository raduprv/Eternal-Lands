#ifndef __CHAT_H__
#define __CHAT_H__

#define CHAT_WIN_TEXT_WIDTH   500
#define CHAT_WIN_TEXT_HEIGHT  (18*10) // 10 lines in normal font size
#define CHAT_WIN_SCROLL_WIDTH 20

extern int use_windowed_chat;

void update_chat_scrollbar ();

void display_chat ();

#endif // def __CHAT_H__

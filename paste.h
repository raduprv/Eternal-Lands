#ifndef __PASTE_H__
#define __PASTE_H__

void do_paste(Uint8 * buffer);

#ifndef WINDOWS
#include <X11/Xlib.h>
void startpaste();
void finishpaste(XSelectionEvent event);
#else
void windows_paste();
#endif

#endif

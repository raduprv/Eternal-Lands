
#ifndef __HIGHLIGHT_H__
#define __HIGHLIGHT_H__

#define HIGHLIGHT_TYPE_WALKING_DESTINATION 	1

extern int highlighting_enabled;

void add_highlight(short x, short y, int type);
void display_highlight_markers();

#endif

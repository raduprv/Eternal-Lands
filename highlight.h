
#ifndef __HIGHLIGHT_H__
#define __HIGHLIGHT_H__

#define TILESIZE_X					(0.5f)
#define TILESIZE_Y					(0.5f)
#define HIGHLIGHT_TYPE_WALKING_DESTINATION 	1

extern int highlighting_enabled;

void add_highlight(short x, short y, int type);
void display_highlight_markers();

#endif


#ifndef __HIGHLIGHT_H__
#define __HIGHLIGHT_H__

#define HIGHLIGHT_TYPE_WALKING_DESTINATION 	1
#ifdef SFX
#define HIGHLIGHT_TYPE_SMITE_SUMMONINGS		4
#define HIGHLIGHT_TYPE_HEAL_SUMMONED		5
#endif

extern int highlighting_enabled;

void add_highlight(short x, short y, int type);
void display_highlight_markers();

#endif

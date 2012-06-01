
#ifndef __HIGHLIGHT_H__
#define __HIGHLIGHT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TILESIZE_X					(0.5f)
#define TILESIZE_Y					(0.5f)
#define HIGHLIGHT_TYPE_WALKING_DESTINATION 	1
#define HIGHLIGHT_TYPE_SPELL_TARGET 	2
#define HIGHLIGHT_TYPE_ATTACK_TARGET 	3
#define HIGHLIGHT_TYPE_LOCK 			4

extern int highlighting_enabled;

void add_highlight(short x, short y, int type);
void display_highlight_markers();

#ifdef __cplusplus
} // extern "C"
#endif

#endif

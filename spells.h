#ifndef __SPELLS_H__
#define __SPELLS_H__

#define SIGILS_NO 50

typedef struct
{
	int sigil_img;
	char name[32];
	char description[64];
	int have_sigil;
}sigil_def;

sigil_def sigils_list[SIGILS_NO];

extern Sint8 active_spells[10];

extern int sigil_menu_x;
extern int sigil_menu_y;
extern int sigil_menu_x_len;
extern int sigil_menu_y_len;
extern int sigil_menu_dragged;

extern int sigils_text;
extern Uint8 spell_text[256];
extern int view_sigils_menu;
extern int sigils_we_have;
extern int have_error_message;
#endif


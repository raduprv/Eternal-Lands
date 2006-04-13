#ifndef __EXT_KILLS_H__
#define __EXT_KILLS_H__

extern int kills_win;

void load_kills();
void on_actor_killed(actor *a);
int command_kills(char *text, int len);
void fill_kills_win();

#endif

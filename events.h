#ifndef __EVENTS_H__
#define __EVENTS_H__

extern int shift_on;
extern int alt_on;
extern int ctrl_on;

void	quick_use(int use_id);
int HandleEvent(SDL_Event *event);

#endif	// __EVENTS_H__

#ifndef __EVENTS_H__
#define __EVENTS_H__

extern int shift_on;
extern int alt_on;
extern int ctrl_on;

int HandleEvent(SDL_Event *event);
#endif

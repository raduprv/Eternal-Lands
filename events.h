#ifndef __EVENTS_H__
#define __EVENTS_H__

enum { EVENT_MOVEMENT_TIMER,EVENT_UPDATE_CAMERA,EVENT_ANIMATE_ACTORS };

extern int shift_on;
extern int alt_on;
extern int ctrl_on;
extern SDL_TimerID event_timer_clock;

void	quick_use(int use_id);
Uint32 event_timer(Uint32 interval, void * data);
int HandleEvent(SDL_Event *event);

typedef struct
{
    int x ;
    int y;
    char text[512];
}marking;

#endif	// __EVENTS_H__

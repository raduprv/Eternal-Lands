#ifndef __EVENTS_H__
#define __EVENTS_H__

extern int shift_on;
extern int alt_on;
extern int ctrl_on;

#define SHIFT (1<<31)
#define CTRL (1<<30)
#define ALT (1<<29)

extern Uint32 K_CAMERAUP;
extern Uint32 K_CAMERADOWN;
extern Uint32 K_ZOOMOUT;
extern Uint32 K_ZOOMIN;
extern Uint32 K_TURNLEFT;
extern Uint32 K_TURNRIGHT;
extern Uint32 K_ADVANCE;
extern Uint32 K_HEALTHBAR;
extern Uint32 K_VIEWNAMES;
extern Uint32 K_STATS;
extern Uint32 K_WALK;
extern Uint32 K_LOOK;
extern Uint32 K_USE;
extern Uint32 K_OPTIONS;
extern Uint32 K_REPEATSPELL;
extern Uint32 K_SIGILS;
extern Uint32 K_MANUFACTURE;
extern Uint32 K_ITEMS;
extern Uint32 K_MAP;
extern Uint32 K_ROTATELEFT;
extern Uint32 K_ROTATERIGHT;
extern Uint32 K_FROTATELEFT;
extern Uint32 K_FROTATERIGHT;
extern Uint32 K_BROWSER;
extern Uint32 K_ESCAPE;
extern Uint32 K_CONSOLE;
extern Uint32 K_SHADOWS;
extern Uint32 K_KNOWLEDGE;
extern Uint32 K_ENCYCLOPEDIA;
extern Uint32 K_ITEM1;
extern Uint32 K_ITEM2;
extern Uint32 K_ITEM3;
extern Uint32 K_ITEM4;
extern Uint32 K_ITEM5;
extern Uint32 K_ITEM6;
extern Uint32 K_VIEWTEXTASOVERTEXT;


int HandleEvent(SDL_Event *event);
#endif

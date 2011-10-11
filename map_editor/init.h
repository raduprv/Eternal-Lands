#ifndef __INIT_H__
#define __INIT_H__

#ifndef DATA_DIR
#define DATA_DIR "./"
#endif

extern char lang[10];
extern char datadir[256];
extern char configdir[256];
extern int use_clipboard; /*!< whether to use CLIPBOARD or PRIMARY for pasting */

void init_stuff();
void window_resize();
#endif

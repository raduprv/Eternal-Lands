#ifndef __INIT_H__
#define __INIT_H__

#include "global.h"

#ifndef DATA_DIR
#define DATA_DIR "./"
#endif

char lang[10];
extern char datadir[256];

void init_stuff();
void resize_window();
#endif

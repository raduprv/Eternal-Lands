#ifndef __FILTER_H__
#define __FILTER_H__

#include	"ignore.h"	/* just in case it hasn't been included */

#define max_filters 1000
typedef struct
  {
	Uint8 name[16];
	int len;
  }filter_slot;

extern filter_slot filter_list[max_filters];
extern int filtered_so_far;
extern int save_filters;
extern int use_global_filters;
extern char text_filter_replace[];

#endif

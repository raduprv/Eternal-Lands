#ifndef __IGNORE_H__
#define __IGNORE_H__

#define max_ignores 1000
typedef struct
  {
	Uint8 name[16];
	char used;
  }ignore_slot;

extern ignore_slot ignore_list[max_ignores];
extern int ignored_so_far;
extern int save_ignores;
extern int use_global_ignores;

#endif

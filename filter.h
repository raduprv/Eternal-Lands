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
extern int caps_filter;
extern char storage_filter[128];

int add_to_filter_list(Uint8 *name, char save_name);
int remove_from_filter_list(Uint8 *name);
int check_if_filtered(Uint8 *name);
int filter_text(Uint8 * input_text, int len);
void load_filters_list(char * file_name);
void clear_filter_list();
void load_filters();
void list_filters();

#endif

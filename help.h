#ifndef __HELP_H__
#define __HELP_H__

#define MAX_HELP_ENTRIES 50

typedef struct
{
	Uint8 topic_name[32];
	Uint8 topic_info[1024];
}help_entry;

extern help_entry help_list[MAX_HELP_ENTRIES];

#endif


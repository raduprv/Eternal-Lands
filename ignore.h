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

int add_to_ignore_list(Uint8 *name, char save_name);
int remove_from_ignore_list(Uint8 *name);
int check_if_ignored(Uint8 *name);
int pre_check_if_ignored(Uint8 * input_text, int type);
void load_ignores_list(char * file_name);
void clear_ignore_list();
void load_ignores();
void list_ignores();

#endif

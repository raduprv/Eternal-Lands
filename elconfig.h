#ifndef __ELCONFIG_H__
#define __ELCONFIG_H__

typedef struct
{
	int 	type;
	char	name[52];
	int 	nlen;
	char 	shortname[12];
	int 	snlen;
	void 	(*func)();
	void 	*var;
	int 	len;
//	char 	*message; //In case you want a message to be written when a setting is changed
} var_struct;

struct variables
{
	int no;
	var_struct * var[100];
};

extern struct variables our_vars;

int check_var(char * str, int type);
void init_vars();
void free_vars();

#endif

#include <stdlib.h>
#include <string.h>
#include "global.h"

ignore_slot ignore_list[max_ignores];
int ignored_so_far=0;
int save_ignores=1;
int use_global_ignores=1;

//returns -1 if the name is already ignored, 1 on sucess, -2 if no more ignore slots
int add_to_ignore_list(Uint8 *name, char save_name)
{
	int i;
	//see if this name is already on the list
	for(i=0;i<max_ignores;i++)
		{
			if(ignore_list[i].used)
				if(my_strcompare(ignore_list[i].name,name))return -1;//already in the list
		}

	//ok, find a free spot
	for(i=0;i<max_ignores;i++)
		{
			if(!ignore_list[i].used)
				{
					//excellent, a free spot
					my_strcp(ignore_list[i].name,name);
					//add to the global ignore file, if the case
					if(save_name)
						{
							FILE *f = NULL;
							char local_ignores[256];
							strcpy(local_ignores, configdir);
							strcat(local_ignores, "local_ignores.txt");
							f=fopen(local_ignores, "a");
							fwrite(name, strlen(name), 1, f);
							fwrite("\n", 1, 1, f);
							fclose(f);
						}
					ignore_list[i].used=1;//mark as used
					ignored_so_far++;
					return 1;
				}
		}

	return -2;//if we are here, it means the ignores list is full
}

//returns -1 if the name is already ignored, 1 on sucess
int remove_from_ignore_list(Uint8 *name)
{
	int i;
	int found = 0;
	FILE *f = NULL;
	//see if this name is on the list
	for(i=0;i<max_ignores;i++)
		{
			if(!found && ignore_list[i].used)
				if(my_strcompare(ignore_list[i].name,name))
					{
						ignore_list[i].used=0;
						found = 1;
						ignored_so_far--;
					}
		}
	if(found)
		{
			char local_ignores[256];
			strcpy(local_ignores, configdir);
			strcat(local_ignores, "local_ignores.txt");
			f=fopen(local_ignores, "w");
			for(i=0;i<max_ignores;i++)
				{
					if(ignore_list[i].used)
						{
							fwrite(ignore_list[i].name, strlen(ignore_list[i].name), 1, f);
							fwrite("\n", 1, 1, f);	
						}
				}
			fclose(f);
			return 1;
		}
	else
		return -1;
}


//returns 1 if ignored, 0 if not ignored
int check_if_ignored(Uint8 *name)
{
	int i;
	for(i=0;i<max_ignores;i++)
		{
			if(ignore_list[i].used)
				if(my_strcompare(ignore_list[i].name,name))return 1;//yep, ignored
		}
	return 0;//nope
}


//returns 1 if ignored, 0 if not ignored
int pre_check_if_ignored(Uint8 * input_text, int type)
{
	int i=0;
	Uint8 name[16];
	Uint8 ch;
	if(type)
		{
			//now find the name portion
			input_text+=10;
			for(i=0;i<15;i++)
				{
					ch=input_text[i];	//skip over the prefix
					if(ch==':' || ch==' ')break;
					name[i]=ch;
				}
			name[i]=0;
			if(check_if_ignored(name))return 1;
			//memorize this players name
			my_strcp(last_pm_from,name);
			return 0;
		}

	if(input_text[1] == '[') input_text++;
	for(i=0;i<15;i++)
		{
			ch=input_text[i+1];
			if(ch==':' || ch==' ' || ch==']')break;
			name[i]=ch;
		}
	name[i]=0;
	if(check_if_ignored(name))return 1;
	else return 0;

}


void load_ignores_list(char * file_name)
{
	int f_size;
	FILE *f = NULL;
	Uint8 * ignore_list_mem;
	int i,j;
	Uint8 name[64];
	Uint8 ch;

	f = fopen(file_name, "rb");
	if(!f)return;
	fseek(f,0,SEEK_END);
	f_size = ftell(f);

	//ok, allocate memory for it
	ignore_list_mem=(Uint8 *)calloc(f_size, 1);
	fseek (f, 0, SEEK_SET);
	fread (ignore_list_mem, 1, f_size, f);
	fclose (f);

	j=0;
	i=0;
	while(i<f_size)
		{
			ch=ignore_list_mem[i];
			if(ch=='\n' || ch=='\r')
				{
					if(j)if(add_to_ignore_list(name,0)==-1)return;//ignore list full
					j=0;
					i++;
					continue;
				}
			else
				{
					name[j]=ch;
				}
			name[j+1]=0;
			j++;
			i++;
		}


	free(ignore_list_mem);
}


void clear_ignore_list()
{
	int i;
	//see if this name is already on the list
	for(i=0;i<max_ignores;i++)
		ignore_list[i].used=0;
}


void load_ignores()
{
	char local_ignores[256];
	strcpy(local_ignores, configdir);
	strcat(local_ignores, "local_ignores.txt");
	clear_ignore_list();
	load_ignores_list(local_ignores);
	if(use_global_ignores)load_ignores_list("global_ignores.txt");
}

void list_ignores()
{
	int i;
	Uint8 str[max_ignores*19];

	if(!ignored_so_far)
		{
			log_to_console(c_grey1,no_ignores_str);
			return;
		}
	sprintf(str,"%s:\n",ignores_str);
	for(i=0;i<max_ignores;i++)
		{
			if(ignore_list[i].used)
				{
					my_strcp(&str[strlen(str)],ignore_list[i].name);
					my_strcp(&str[strlen(str)],", ");
				}
		}

	str[strlen(str)-2]=0;//get rid of the last ", " thingy

	log_to_console(c_grey1,str);

}


#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "global.h"

ignore_slot ignore_list[MAX_IGNORES];
int ignored_so_far=0;
int save_ignores=1;
int use_global_ignores=1;

//returns -1 if the name is already ignored, 1 on sucess, -2 if no more ignore slots
int add_to_ignore_list(Uint8 *name, char save_name)
{
	int i;
	
	// never ignore uobeyuok, the rule bot
	if(!strcasecmp(name, "uobeyuok")){
		return(-1);
	}
	//see if this name is already on the list
	for(i=0;i<MAX_IGNORES;i++)
		{
			if(ignore_list[i].used)
				if(my_strcompare(ignore_list[i].name,name))return -1;//already in the list
		}

	//ok, find a free spot
	for(i=0;i<MAX_IGNORES;i++)
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
							snprintf(local_ignores, sizeof(local_ignores), "%slocal_ignores.txt", configdir);
							f=my_fopen(local_ignores, "a");
							if (f != NULL)
							{
								fwrite(name, strlen(name), 1, f);
								fwrite("\n", 1, 1, f);
								fclose(f);
							}
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
	for(i=0;i<MAX_IGNORES;i++)
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
			snprintf(local_ignores, sizeof(local_ignores), "%slocal_ignores.txt", configdir);
			f=my_fopen(local_ignores, "w");
			if (f != NULL)
			{
				for(i=0;i<MAX_IGNORES;i++)
				{
					if(ignore_list[i].used)
						{
							fwrite(ignore_list[i].name, strlen(ignore_list[i].name), 1, f);
							fwrite("\n", 1, 1, f);
						}
				}
				fclose(f);
			}
			return 1;
		}
	else
		return -1;
}


//returns 1 if ignored, 0 if not ignored
int check_if_ignored (const Uint8 *name)
{
	int i;

	for (i = 0; i < MAX_IGNORES; i++)
	{
		if (ignore_list[i].used && my_strcompare(ignore_list[i].name, name))
			return 1;	// yep, ignored
	}
	return 0;	// nope
}

int name_is_valid(const Uint8 *name)
{
	int i, len;
	for(i = 0, len = strlen(name); i < len; i++) {
		if(!isalnum(name[i]) && name[i] != '_')
		{
			return 0;
		}
	}
	return 1;
}
//returns 1 if ignored, 0 if not ignored
int pre_check_if_ignored (const Uint8 *input_text, int len, Uint8 channel)
{
	int i, offset;
	Uint8 name[16] = {0};
	Uint8 ch;

	switch(channel)
	{
		case CHAT_PERSONAL:
		case CHAT_MODPM:
			offset = (channel == CHAT_MODPM ? strlen(mod_pm_from_str) : strlen(pm_from_str))+2;
			for (i = 0; i < 15 && ((i+offset) < len); i++)
			{
				ch = input_text[i+offset];	//skip over the prefix
				if (ch == ':' || ch == ' ')
				{
					break;
				}
				name[i] = ch;
			}
			name[i] = '\0';
			if(channel == CHAT_MODPM)
			{
				return 0;
			}
			else
			{
				break;
			}
		case CHAT_LOCAL:
			offset = 0;
			while (IS_COLOR (input_text[offset]))
			{
				offset++;
			}
			for (i = 0; i < 15 && i+offset < len; i++)
			{
				ch = input_text[i+offset];
				if (ch == ':' || ch == ' ' || IS_COLOR(ch))
				{
					break;
				}
				name[i] = ch;
			}
			name[i] = '\0';
			if(!name_is_valid(name)) {
				//This can be a lot. Let's separate them based on the color for now.
				switch(*input_text) {
					case 127+c_grey1:
						//Check for summoning messages
						//(*) NAME summoned a %s
						if(strcmp(name, "(*)") == 0) {
							offset += i;
							while(offset < len && isspace(input_text[offset])) {
								offset++;
							}
							for (i = 0; i < 15 && i+offset < len; i++)
							{
								ch = input_text[i+offset];
								if (isspace(ch))
								{
									break;
								}
								name[i] = ch;
							}
							name[i] = '\0';
						}
						break;
				}
			}
		break;
		case CHAT_CHANNEL1:
		case CHAT_CHANNEL2:
		case CHAT_CHANNEL3:
			for(offset = 0; IS_COLOR (input_text[offset]); offset++);
			if (input_text[offset] == '[')
			{
				offset++;
			}
			for (i = 0; i < 15 && i+offset < len; i++)
			{
				ch = input_text[i+offset];
				if (ch == ':' || ch == ' ' || ch == ']')
				{
					break;
				}
				name[i] = ch;
			}
			name[i] = '\0';
		break;
		case CHAT_GM:
			for(offset = 0; IS_COLOR(input_text[offset]); offset++);
			if(strncasecmp(input_text+offset, gm_from_str, strlen(gm_from_str)) == 0)
			{
				offset = strlen(gm_from_str)+2;
				for (i = 0; i < 15 && i+offset < len; i++)
				{
					ch = input_text[i+offset];	//skip the prefix
					if (ch == ':' || ch == ' ')
					{
						break;
					}
					name[i] = ch;
				}
				name[i] = '\0';
			}
			break;
	}
	if(*name && name_is_valid(name)) {
		add_name_to_tablist(name);
	} else {
		for(offset = 0; IS_COLOR(input_text[offset]); offset++);
		for (i = 0; i < 15 && i+offset < len; i++) {
			ch = input_text[i+offset];
			if (ch == ' ' || ch == ':' || IS_COLOR(ch)) {
				break;
			}
			name[i] = ch;
		}
		name[i] = '\0';
	}
	if(!check_if_ignored (name)){
		if(channel == CHAT_PERSONAL || channel == CHAT_MODPM){
			//memorise the name
			my_strcp (last_pm_from, name);
		}
		return 0;
	}else{
		return 1;
	}
}


void load_ignores_list(char * file_name)
{
	int f_size;
	FILE *f = NULL;
	Uint8 * ignore_list_mem;
	int i,j;
	Uint8 name[64];
	Uint8 ch;

	// don't use my_fopen, absence of ignores is not an error
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
					if(j && add_to_ignore_list(name,0) == -1) {
						return;//ignore list full
					}
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
	for(i=0;i<MAX_IGNORES;i++)
		ignore_list[i].used=0;
}


void load_ignores()
{
	char local_ignores[256];
	snprintf(local_ignores, sizeof(local_ignores), "%slocal_ignores.txt", configdir);
	clear_ignore_list();
	load_ignores_list(local_ignores);
	if(use_global_ignores)load_ignores_list("global_ignores.txt");
}

int list_ignores()
{
	int i;
	Uint8 str[MAX_IGNORES*19];
	
	if(!ignored_so_far)
		{
			LOG_TO_CONSOLE(c_grey1,no_ignores_str);
			return 1;
		}
	snprintf(str,sizeof(str),"%s:\n",ignores_str);
	for(i=0;i<MAX_IGNORES;i++)
		{
			if(ignore_list[i].used)
				{
					strncat(str, ignore_list[i].name,sizeof(str)-1);
					strncat(str, ", ",sizeof(str)-1);
				}
		}

	str[strlen(str)-2]=0;//get rid of the last ", " thingy

	LOG_TO_CONSOLE(c_grey1,str);
	return 1;
}

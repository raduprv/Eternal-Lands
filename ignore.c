#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "ignore.h"
#include "asc.h"
#include "console.h"
#include "init.h"
#include "text.h"
#include "translate.h"
#include "errors.h"
#include "io/elpathwrapper.h"

ignore_slot ignore_list[MAX_IGNORES];
int ignored_so_far=0;
int save_ignores=1;
int use_global_ignores=1;

//returns -1 if the name is already ignored, 1 on sucess, -2 if no more ignore slots
int add_to_ignore_list(char *name, char save_name)
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
							FILE * f=open_file_config("local_ignores.txt", "a");
							if (f == NULL){
								LOG_ERROR("%s: %s \"local_ignores.txt\": %s\n", reg_error_str, cant_open_file, strerror(errno));
							} else {
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
int remove_from_ignore_list(char *name)
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
			f=open_file_config("local_ignores.txt", "w");
			if (f == NULL){
				LOG_ERROR("%s: %s \"local_ignores.txt\": %s\n", reg_error_str, cant_open_file, strerror(errno));
			} else {
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
int check_if_ignored (const char *name)
{
	int i;

	for (i = 0; i < MAX_IGNORES; i++)
	{
		if (ignore_list[i].used && my_strcompare(ignore_list[i].name, name))
			return 1;	// yep, ignored
	}
	return 0;	// nope
}

int name_is_valid(const char *name)
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


// Searches for terminating characters based on type, from "input_text", adding them to "name"
int get_name_from_text(const char * input_text, int len, int type, int offset, char * name)
{
	int i, do_break;
	Uint8 ch;
	
	for (i = 0; i < MAX_USERNAME_LENGTH - 1 && (i + offset) < len; i++)
	{
		ch = input_text[i+offset];	//skip over the prefix
		do_break = 0;
		switch (type)
		{
			case 0:
				if (ch == ':' || ch == ' ')
					do_break = 1;
				break;
			case 1:
				if (ch == ':' || ch == ' ' || is_color (ch))
					do_break = 1;
				break;
			case 2:
				if (isspace(ch))
					do_break = 1;
				break;
			case 3:
				if (ch == ':' || ch == ' ' || ch == ']')
					do_break = 1;
				break;
			case 4:
				if (ch == ':' || ch == '-' || ch == ' ')
					do_break = 1;
				break;
		}
		if (do_break == 1)
			break;
		name[i] = ch;
	}
	name[i] = '\0';
	return i;
}

// Returns 1 if ignored, 0 if not ignored
int pre_check_if_ignored (const char *input_text, int len, Uint8 channel)
{
	int offset;
	char name[MAX_USERNAME_LENGTH] = {0};

	if (channel == CHAT_MODPM)
	{
		return 0;		// Don't ever ignore MOD PM's
	}
	
	switch(channel)
	{
		case CHAT_PERSONAL:
			offset = strlen(pm_from_str) + 2;
			get_name_from_text(input_text, len, 0, offset, name);		// Type 0 = ":" or " "
			break;
		case CHAT_LOCAL:
			offset = 0;
			while (is_color (input_text[offset]))
			{
				offset++;
			}
			offset += get_name_from_text(input_text, len, 1, offset, name);		// Type 1 = ":", " " or is_color
			if (!name_is_valid(name))
			{
				//This can be a lot. Let's separate them based on the color for now.
				switch (from_color_char (*input_text))
				{
					case c_grey1:
						//Check for summoning messages
						//(*) NAME summoned a %s
						if (strcmp(name, "(*)") == 0)
						{
							while (offset < len && isspace(input_text[offset]))
							{
								offset++;
							}
							get_name_from_text(input_text, len, 2, offset, name);		// Type 2 = isspace
						}
						break;
				}
			}
			break;
		case CHAT_CHANNEL1:
		case CHAT_CHANNEL2:
		case CHAT_CHANNEL3:
			for (offset = 0; is_color (input_text[offset]); offset++);		// Ignore colours
			if (input_text[offset] == '[')
			{
				offset++;
			}
			get_name_from_text(input_text, len, 3, offset, name);		// Type 3 = ":", " " or "]"
			break;
		case CHAT_GM:
			for (offset = 0; is_color (input_text[offset]); offset++);		// Ignore colours
			if(strncasecmp(input_text+offset, gm_from_str, strlen(gm_from_str)) == 0)
			{
				offset = strlen(gm_from_str)+2;
				get_name_from_text(input_text, len, 0, offset, name);		// Type 0 = ":" or " "
			}
			else if(strncasecmp(input_text+offset, ig_from_str, strlen(ig_from_str)) == 0)
			{
				offset = strlen(ig_from_str)+1;
				get_name_from_text(input_text, len, 4, offset, name);		// Type 4 = ":", "-" or " "
			}
			break;
		case CHAT_MOD:
			for (offset = 0; is_color (input_text[offset]); offset++);		// Ignore colours
			if(strncasecmp(input_text+offset, mc_from_str, strlen(mc_from_str)) == 0)
			{
				offset = strlen(mc_from_str)+2;
				get_name_from_text(input_text, len, 0, offset, name);		// Type 0 = ":" or " "
			}
			break;
	}
	if (*name && name_is_valid(name))
	{
		add_name_to_tablist(name);
	}
	else
	{
		for (offset = 0; is_color (input_text[offset]); offset++);		// Ignore colours
		get_name_from_text(input_text, len, 1, offset, name);	// Type 1 = ":", " " or is_color
	}
	if (!check_if_ignored (name))
	{
		if (channel == CHAT_PERSONAL || channel == CHAT_MODPM)
		{
			//memorise the name
			my_strcp (last_pm_from, name);
		}
		return 0;
	}
	else
	{
		return 1;
	}
}


void load_ignores_list(char * file_name)
{
	int f_size;
	FILE *f = NULL;
	char * ignore_list_mem;
	int i,j;
	char name[64];
	Uint8 ch;
	size_t ret;

	f = open_file_config(file_name, "rb");
	if(f == NULL){return;}
	fseek(f,0,SEEK_END);
	f_size = ftell(f);
	if (f_size <= 0)
	{
		fclose(f);
		return;
	}

	//ok, allocate memory for it
	ignore_list_mem=(char *)calloc(f_size, 1);
	fseek (f, 0, SEEK_SET);
	ret = fread (ignore_list_mem, 1, f_size, f);
	fclose (f);
	if (ret != f_size)
	{
		free (ignore_list_mem);
		LOG_ERROR("%s() read failed for file [%s]\n", __FUNCTION__, file_name);
		return;
	}

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
	safe_snprintf(local_ignores, sizeof(local_ignores), "%slocal_ignores.txt", configdir);
	clear_ignore_list();
	load_ignores_list(local_ignores);
	if(use_global_ignores)load_ignores_list("global_ignores.txt");
}

int list_ignores()
{
	int i;
	char str[MAX_IGNORES*19];
	
	if(!ignored_so_far)
		{
			LOG_TO_CONSOLE(c_grey1,no_ignores_str);
			return 1;
		}
	safe_snprintf(str,sizeof(str),"%s:\n",ignores_str);
	for(i=0;i<MAX_IGNORES;i++)
		{
			if(ignore_list[i].used)
				{
					safe_strcat (str, ignore_list[i].name, sizeof(str));
					safe_strcat (str, ", ", sizeof(str));
				}
		}

	str[strlen(str)-2]=0;//get rid of the last ", " thingy

	LOG_TO_CONSOLE(c_grey1,str);
	return 1;
}

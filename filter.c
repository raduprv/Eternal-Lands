#include <stdlib.h>
#include <string.h>
#include "global.h"

filter_slot filter_list[max_filters];
int filtered_so_far=0;
int use_global_filters=1;
char text_filter_replace[128]="smeg";
int caps_filter=1;
char storage_filter[128];

//returns -1 if the name is already filtered, 1 on sucess, -2 if no more filter slots
int add_to_filter_list(Uint8 *name, char save_name)
{
	int i;
	//see if this name is already on the list
	for(i=0;i<max_filters;i++)
		{
			if(filter_list[i].len > 0)
				if(my_strcompare(filter_list[i].name,name))return -1;//already in the list
		}

	//ok, find a free spot
	for(i=0;i<max_filters;i++)
		{
			if(!filter_list[i].len > 0)
				{
					//excellent, a free spot
					my_strcp(filter_list[i].name,name);
					//add to the global filter file, if the case
					if(save_name)
						{
							FILE *f = NULL;
							char local_filters[256];
							strcpy(local_filters, configdir);
							strcat(local_filters, "local_filters.txt");
							f=fopen(local_filters, "a");
							fwrite(name, strlen(name), 1, f);
							fwrite("\n", 1, 1, f);
							fclose(f);
						}
					filter_list[i].len=strlen(filter_list[i].name);//memorize the length
					filtered_so_far++;
					return 1;
				}
		}

	return -2;//if we are here, it means the filters list is full
}

//returns -1 if the name is already filtered, 1 on sucess
int remove_from_filter_list(Uint8 *name)
{
	int i;
	int found = 0;
	FILE *f = NULL;
	//see if this name is on the list
	for(i=0;i<max_filters;i++)
		{
			if(!found && filter_list[i].len > 0)
				if(my_strcompare(filter_list[i].name,name))
					{
						filter_list[i].len=0;
						found = 1;
						filtered_so_far--;
					}
		}
	if(found)
		{
			char local_filters[256];
			strcpy(local_filters, configdir);
			strcat(local_filters, "local_filters.txt");
			f=fopen(local_filters, "w");
			for(i=0;i<max_filters;i++)
				{
					if(filter_list[i].len > 0)
						{
							fwrite(ignore_list[i].name, filter_list[i].len, 1, f);
							fwrite("\n", 1, 1, f);	
						}
				}
			fclose(f);
			return 1;
		}
	else
		return -1;
}


//returns length to be filtered, 0 if not filtered
int check_if_filtered(Uint8 *name)
{
	int i;
	for(i=0;i<max_filters;i++)
		{
			if(filter_list[i].len > 0)
				if(my_strncompare(filter_list[i].name,name,filter_list[i].len))return filter_list[i].len;//yep, filtered
		}
	return 0;//nope
}

// Filter the lines that contain the desired string from the inventory listing
int filter_storage_text (Uint8 * input_text, int len) {
	int istart, iline, ic, diff;
	int flen;

	flen = strlen (storage_filter);
	istart = iline = ic = 0;
	while (ic < len - flen)
        	{
			if (input_text[ic] == '\n')
                        	{
					iline = ++ic;
				}
			else if (my_strncompare (input_text+ic, storage_filter, flen))
                        	{
					diff = iline - istart;
					if (diff > 0)
                                        	{
					    		len -= diff;
					    		memmove (input_text + istart, input_text + iline, len * sizeof (Uint8));
			    				ic -= diff;
			  			}
			 		while (ic < len && input_text[ic] != '\n') ic++;
					  iline = istart = ++ic;
				}
			else
                        	{
					ic++;
				}
		}

	if (istart == 0)
        	{
			sprintf (input_text, "<none>");
			len = 6;
		} 
	else
        	{
			input_text[--istart] = '\0';
			len = istart;
		}

	storage_filter[0] = '\0';
	return len;
}  

//returns the new length of the text
int filter_text(Uint8 * input_text, int len)
{
	int i,bad_len,rep_len;
	Uint8 *rloc=input_text;
	
	// See if a search term has been added to the #storage command, and if so, 
        // only list those items with that term
	if (*storage_filter && my_strncompare (input_text+1, "Items you have in your storage:", 31))
		len = 33 + filter_storage_text (input_text+33, len-33);

	//do we need to do CAPS filtering?
	if(caps_filter)
		{ 
			int	clen=len;
			// skip any coloring
			if(*rloc >= 128) while(*rloc && *rloc >= 128 && clen > 0)
				{
					rloc++;
					clen--;
				}
			// handle PM's
			if(*rloc == '[' || rloc[1] == '[') while(*rloc && *rloc != ':' && clen > 0)
				{
					rloc++;
					clen--;
				}
			// or ignore first word
			else while(*rloc && *rloc != ' ' && *rloc != ':' && clen > 0)
				{
					rloc++;
					clen--;
				}
			while(*rloc && (*rloc == ' ' || *rloc == ':' || *rloc >= 128) && clen > 0)
				{
					rloc++;
					clen--;
				}
			// check for hitting the EOS
			if(!*rloc) rloc= input_text;
			// if we pass the upper test, entire line goes lower
			if(len-(rloc-input_text) > 4 && my_isupper(rloc, clen)) my_tolower(input_text);
			rloc= input_text;	// restore the initial value
		}
	//do we need to do any content filtering?
	if(max_filters == 0)return(len);
	// get the length of the replacement string
	rep_len=strlen(text_filter_replace);
	// scan the text for any strings
	for(i=0;i<len;i++,rloc++)
		{
			if((bad_len=check_if_filtered(rloc)) > 0)
				{
					//oops, remove this word 
					if(bad_len <= rep_len)
						{
							strncpy(rloc, text_filter_replace, bad_len);
						}
					else
						{
							strncpy(rloc, text_filter_replace, rep_len);
							memmove(rloc+rep_len, rloc+bad_len, len-(i+bad_len));
							// adjust the length
							len-=(bad_len-rep_len);
						}
				}
		}

	return(len);
}


void load_filters_list(char * file_name)
{
	int f_size;
	FILE *f = NULL;
	Uint8 * filter_list_mem;
	int i,j;
	Uint8 name[64];
	Uint8 ch;

	f = fopen(file_name, "rb");
	if(!f)return;
	fseek(f,0,SEEK_END);
	f_size = ftell(f);

	//ok, allocate memory for it
	filter_list_mem=(Uint8 *)calloc(f_size, 1);
	fseek (f, 0, SEEK_SET);
	fread (filter_list_mem, 1, f_size, f);
	fclose (f);

	j=0;
	i=0;
	while(i<f_size)
		{
			ch=filter_list_mem[i];
			if(ch=='\n' || ch=='\r')
				{
					if(j)if(add_to_filter_list(name,0)==-1)return;//filter list full
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


	free(filter_list_mem);
}


void clear_filter_list()
{
	int i;
	//see if this name is already on the list
	for(i=0;i<max_filters;i++)
		filter_list[i].len=0;
}


void load_filters()
{
	char local_filters[256];
	strcpy(local_filters, configdir);
	strcat(local_filters, "local_filters.txt");
	clear_filter_list();
	load_filters_list(local_filters);
	if(use_global_filters)load_filters_list("global_filters.txt");
}

void list_filters()
{
	int i;
	Uint8 str[max_filters*19];

	if(!filtered_so_far)
		{
			log_to_console(c_grey1,no_filters_str);
			return;
		}
	sprintf(str,"%s:\n",filters_str);
	for(i=0;i<max_filters;i++)
		{
			if(filter_list[i].len > 0)
				{
					my_strcp(&str[strlen(str)],filter_list[i].name);
					my_strcp(&str[strlen(str)],", ");
				}
		}

	str[strlen(str)-2]=0;//get rid of the last ", " thingy

	log_to_console(c_grey1,str);

}


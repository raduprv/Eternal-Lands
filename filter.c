#include <stdlib.h>
#include <string.h>
#include "global.h"
#include <ctype.h>

#define MAX_FILTERS 1000

typedef struct
{
	Uint8 name[64];
	int len;
	Uint8 replacement[64];
	int rlen;
	char wildcard_type; /* 0=none, 1=*word, 2=word*, 3=*word* */
}filter_slot;

filter_slot filter_list[MAX_FILTERS];
int filtered_so_far=0;
int use_global_filters=1;
int caps_filter=1;
char storage_filter[128];

//returns -1 if the name is already filtered, 1 on sucess, -2 if no more filter slots
int add_to_filter_list(Uint8 *name, char save_name)
{
	int i;
	char left[256];
	char right[256];
	char buff[256];
	int t;
	int l=0;

	//see if this name is already on the list
	for(i=0;i<MAX_FILTERS;i++)
		{
			if(filter_list[i].len > 0)
				if(my_strcompare(filter_list[i].name,name))return -1;//already in the list
		}

	//ok, find a free spot
	for(i=0;i<MAX_FILTERS;i++)
		{
			if(filter_list[i].len <= 0)
				{
					//excellent, a free spot
					snprintf(left, sizeof(left), name);
					for(t=0;;t++){
						if(left[t]==0){
							snprintf(right, sizeof(right), "smeg");
							break;
						}
						if(left[t]=='='){
							left[t]=0;
							snprintf(right, sizeof(right), left+t+2);
							break;
						}
					}
					//add to the global filter file, if the case
					if(save_name)
						{
							FILE *f = NULL;
							char local_filters[256];
							snprintf(local_filters, sizeof(local_filters), "%slocal_filters.txt", configdir);
							f=my_fopen(local_filters, "a");
							if (f != NULL)
							{
								snprintf(buff, sizeof(buff),"%s = %s", left, right);
								fwrite(buff, strlen(buff), 1, f);
								fwrite("\n", 1, 1, f);
								fclose(f);
							}
						}
					left[64]=0;
					right[64]=0;
					filter_list[i].wildcard_type=0;
					l=strlen(left)-2;
					if(left[0]=='*' && left[l]!='*') filter_list[i].wildcard_type=1;
					if(left[0]!='*' && left[l]=='*') filter_list[i].wildcard_type=2;
					if(left[0]=='*' && left[l]=='*') filter_list[i].wildcard_type=3;
					my_strcp(filter_list[i].name,left);
					my_strcp(filter_list[i].replacement,right);
					filter_list[i].len=strlen(filter_list[i].name);//memorize the length
					filter_list[i].rlen=strlen(filter_list[i].replacement);//memorize the length

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
	for(i=0;i<MAX_FILTERS;i++)
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
			snprintf(local_filters, sizeof(local_filters), "%slocal_filters.txt", configdir);
			f=my_fopen(local_filters, "w");
			if (f != NULL)
			{
				for(i=0;i<MAX_FILTERS;i++)
				{
					if(filter_list[i].len > 0)
						{
							fwrite(ignore_list[i].name, filter_list[i].len, 1, f);
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


//returns length to be filtered, 0 if not filtered
int check_if_filtered(Uint8 *name)
{
	int t, i, l;
	char buff[256];

	for(i=0;i<MAX_FILTERS;i++)
		{
			if(filter_list[i].len > 0)
			{
				if(filter_list[i].wildcard_type==0){  /* no wildcard, normal compare */
					if(my_strncompare(filter_list[i].name,name,filter_list[i].len)){
						if(isalpha(name[filter_list[i].len-1])==0){ /* fits with end of word? */
							return i;//yep, filtered
						} 
					}
				}
				else if(filter_list[i].wildcard_type==1){  /* *word                       */
					for(t=0;;t++){
						if(isalpha(name[t])==0) break; /* t points now at the end of the word */
					}
					strcpy(buff, filter_list[i].name);
					l=filter_list[i].len;
					if(my_strncompare(buff+1,name+t-(l-2),l-2)){
						return i;//yep, filtered
					}
				}
				else if(filter_list[i].wildcard_type==2){  /* word*                      */
					strcpy(buff, filter_list[i].name);
					l=filter_list[i].len;
					if(my_strncompare(buff,name,l-2)){
						return i;//yep, filtered
					}
				}
				else if(filter_list[i].wildcard_type==3){  /* *word*                     */
					strcpy(buff, filter_list[i].name);
					buff[strlen(buff)-1]=0;  /* remove trailing * */
					l=filter_list[i].len;
					for(t=0;;t++){
						if(isalpha(name[t])==0) break;
						if(my_strncompare(buff+1,name+t,l-3)){
							return i;//yep, filtered
						}
					}
				}
			}
		}
	return -1;//nope
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
	int i, t,bad_len,rep_len, idx;
	Uint8 *rloc=input_text;
	Uint8 buff[4096];

	// See if a search term has been added to the #storage command, and if so, 
        // only list those items with that term
	if (*storage_filter && my_strncompare (input_text+1, "Items you have in your storage:", 31))
		len = 33 + filter_storage_text (input_text+33, len-33);

	memset(buff, 0, len+3);  /* clear buffer */
	snprintf(buff+1, len, input_text); /* now we have leading and trailing spaces */
	buff[0]=' ';
	buff[len+1]=' ';

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
	if(MAX_FILTERS == 0)return(len);

	// scan the text for any strings
	for(i=0;i<len;i++)
		{                      /* remember, buff has 1 leading space!! */
			if(isalpha(buff[i])==0){  /* beginning of word? */
				if((idx=check_if_filtered(buff+i+1)) > -1) {
					//oops, remove this word 
					if(filter_list[idx].wildcard_type>0){
						bad_len=0;
						for(t=1;;t++){
							if(isalpha(buff[i+t])==0) break;
							bad_len++;
						}
					}
					else{
						bad_len=filter_list[idx].len;
					}
					rep_len=filter_list[idx].rlen;

					if(bad_len == rep_len) {
						snprintf(buff+i+1, rep_len, filter_list[idx].replacement);
					}
					else{
						if(filter_list[idx].wildcard_type > 0){
							bad_len++;
						}
						memmove(buff+i+1+rep_len+1, buff+i+1+bad_len, len-(i-1+bad_len));
						snprintf(buff+i+1, rep_len, filter_list[idx].replacement);
						buff[i+1+rep_len]=' '; /* end word with a space */
						// adjust the length
						len-=(bad_len-(rep_len+1));
					}
				}
			}
		}
	snprintf(input_text, len, buff+1);
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

	// don't use my_fopen, absence of filters is not an error
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
	for(i=0;i<MAX_FILTERS;i++)
		filter_list[i].len=0;
}


void load_filters()
{
	char local_filters[256];
	snprintf(local_filters, sizeof(local_filters), "%slocal_filters.txt", configdir);
	clear_filter_list();
	load_filters_list(local_filters);
	if(use_global_filters)load_filters_list("global_filters.txt");
}

void list_filters()
{
	int i;
	Uint8 str[MAX_FILTERS*19];

	if(!filtered_so_far)
		{
			LOG_TO_CONSOLE(c_grey1,no_filters_str);
			return;
		}
	sprintf(str,"%s:\n",filters_str);
	for(i=0;i<MAX_FILTERS;i++)
		{
			if(filter_list[i].len > 0)
				{
					my_strcp(&str[strlen(str)],filter_list[i].name);
					my_strcp(&str[strlen(str)]," = ");
					my_strcp(&str[strlen(str)],filter_list[i].replacement);
					my_strcp(&str[strlen(str)],", ");
				}
		}

	str[strlen(str)-2]=0;//get rid of the last ", " thingy

	LOG_TO_CONSOLE(c_grey1,str);

}

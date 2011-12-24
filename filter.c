#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "filter.h"
#include "asc.h"
#include "init.h"
#include "translate.h"
#include "errors.h"
#include "io/elpathwrapper.h"

#define MAX_FILTERS 1000

typedef struct
{
	char name[64];
	int len;
	char replacement[64];
	int rlen;
	char wildcard_type; /* 0=none, 1=*word, 2=word*, 3=*word* */
	char local;
}filter_slot;

filter_slot filter_list[MAX_FILTERS];
int have_storage_list = 0;
int filtered_so_far=0;
int use_global_filters=1;
int caps_filter=1;
char storage_filter[128];

unsigned char cached_storage_list[8192] = {0};

//returns -1 if the name is already filtered, 1 on sucess, -2 if no more filter slots
int add_to_filter_list (const char *name, char local, char save_name)
{
	int i, j;
	char left[256];
	char right[256];
	int t, tp;
	int l=0;

	//ok, find a free spot
	for (i = 0; i < MAX_FILTERS; i++)
	{
		if (filter_list[i].len <= 0)
		{
			// excellent, a free spot
			safe_strncpy (left, name, sizeof(left));
			for (t = 0; ; t++)
			{
				if (left[t] == '\0')
				{
					safe_strncpy (right, "smeg", sizeof(right));
					break;
				}
				if(left[t]=='=')
				{
					left[t] = '\0';
					tp = t - 1;
					for (tp = t-1; tp >= 0 && isspace (left[tp]); tp--)
					{
						left[tp] = '\0';
					}
					for (tp = t + 1; left[tp] != '\0' && !(left[tp]&0x80) && isspace(left[tp]); tp++) ;
					safe_strncpy (right, &left[tp], sizeof(right));
					break;
				}
			}
			// See if this name is already on the list
			for (j = 0; j < MAX_FILTERS; j++)
			{
				if (filter_list[j].len > 0)
				{
					if (my_strcompare (filter_list[j].name, left))
						return -1; // Already in the list
				}
			}
			// add to the local filter file, if the case
			if (save_name)
			{
				FILE *f = open_file_config("local_filters.txt", "a");
				if (f == NULL){
					LOG_ERROR("%s: %s \"local_filters.txt\": %s\n", reg_error_str, cant_open_file, strerror(errno));
				} else {
					fprintf (f, "%s = %s\n", left, right);
					fclose(f);
				}
			}
			left[sizeof(filter_list[i].name)-1] = '\0';
			right[sizeof(filter_list[i].replacement)-1] = '\0';
			filter_list[i].wildcard_type = 0;
			l = strlen (left) - 1;
			if (left[0] == '*' && left[l] != '*') filter_list[i].wildcard_type = 1;
			if (left[0] != '*' && left[l] == '*') filter_list[i].wildcard_type = 2;
			if (left[0] == '*' && left[l] == '*') filter_list[i].wildcard_type = 3;
			my_strcp (filter_list[i].name, left);
			my_strcp (filter_list[i].replacement, right);
			filter_list[i].len = strlen(filter_list[i].name);//memorize the length
			filter_list[i].rlen = strlen(filter_list[i].replacement);//memorize the length
			filter_list[i].local = local;

			filtered_so_far++;
			return 1;
		}
	}

	return -2;//if we are here, it means the filters list is full
}

//returns -1 if the name is not filtered, 1 on sucess
int remove_from_filter_list (const char *name)
{
	int i;
	int local = 0;
	FILE *f = NULL;
	
	//see if this name is on the list
	for (i = 0; i < MAX_FILTERS; i++)
	{
		if (filter_list[i].len > 0)
		{
			if (my_strcompare (filter_list[i].name, name))
			{
				local = filter_list[i].local;
				filter_list[i].len = 0;
				filtered_so_far--;
				break;
			}
		}
	}
	
	if (local)
	{
		f = open_file_config ("local_filters.txt", "w");
		if (f == NULL){
			LOG_ERROR("%s: %s \"local_filters.txt\": %s\n", reg_error_str, cant_open_file, strerror(errno));
		} else {
			for (i = 0; i < MAX_FILTERS; i++)
			{
				if (filter_list[i].len > 0 && filter_list[i].local)
					fprintf (f, "%s = %s\n", filter_list[i].name, filter_list[i].replacement);
			}
			fclose(f);
		}
		return 1;
	}

	return -1;
}

#ifdef DEBUG
void print_filter_list ()
{
	int i;
	
	printf ("filter:\n");
	for (i = 0; i < MAX_FILTERS; i++)
	{
		if (filter_list[i].len > 0)
		{
			printf ("%d: name = %.*s, len = %d, replacement = %.*s, rlen = %d, type = %d\n", i, filter_list[i].len, filter_list[i].name, filter_list[i].len, filter_list[i].rlen, filter_list[i].replacement, filter_list[i].rlen, filter_list[i].wildcard_type);
		}
	}
}
#endif

//returns length to be filtered, 0 if not filtered
int check_if_filtered (const char *name)
{
	int t, i, l;

	for (i = 0; i < MAX_FILTERS; i++)
	{
		if (filter_list[i].len > 0 && (use_global_filters || filter_list[i].local))
		{
			if (filter_list[i].wildcard_type==0)
			{
				/* no wildcard, normal compare */
				if (my_strncompare (filter_list[i].name, name, filter_list[i].len))
				{
					if (!isalpha (name[filter_list[i].len]))
					{
						/* fits with end of word? */
						return i; // yep, filtered
					}
				}
			}
			else if (filter_list[i].wildcard_type == 1)
			{
				/* *word                       */
				for (t = 0; ; t++)
				{
					if (!isalpha (name[t])) break; /* t points now at the end of the word */
				}
				l = filter_list[i].len;
				if (t >= l-1)
				{
					if (my_strncompare (&(filter_list[i].name[1]), &name[t-l], l-1))
						return i;
				}
			}
			else if (filter_list[i].wildcard_type == 2)
			{
				/* word*                      */
				if (my_strncompare (filter_list[i].name, name, filter_list[i].len-1))
					return i;
			}
			else if (filter_list[i].wildcard_type==3)
			{
				/* *word*                     */
				for (t = 0; ; t++)
				{
					if (!isalpha (name[t])) break;
					if (my_strncompare(&(filter_list[i].name[1]), &name[t], filter_list[i].len-2))
						return i;//yep, filtered
				}
			}
		}
	}
	
	return -1;//nope
}

// Filter the lines that contain the desired string from the inventory listing
int filter_storage_text (char * input_text, int len, int size) {
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
				memmove (input_text + istart, input_text + iline, len * sizeof (char));
				ic -= diff;
			}
			while (ic < len && input_text[ic] != '\n')
				ic++;
			iline = istart = ++ic;
		}
		else
		{
			ic++;
		}
	}

	if (istart == 0)
	{
		safe_snprintf (input_text, size, "<none>");
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
int filter_text (char *buff, int len, int size)
{
	int i, t, bad_len, rep_len, new_len, idx;

	if (len > 31 && my_strncompare (buff+1, "Items you have in your storage:", 31)){
		//First up, attempt to save the storage list for re-reading later
		if(size <= sizeof(cached_storage_list)){
			memcpy(cached_storage_list, buff, size);
			have_storage_list = 1;
		}
		// See if a search term has been added to the #storage command, and if so, 
		//only list those items with that term
		if (storage_filter[0] != '\0'){
			len = 33 + filter_storage_text (buff+33, len-33, size-33);
		}
	}

	//do we need to do CAPS filtering?
	if (caps_filter)
	{ 
		int idx, clen = len;

		// skip any coloring
		idx = 0;
		while (is_color (buff[idx]) && clen > 0)
		{
			idx++;
			clen--;
		}

		// handle PM's
		if (buff[idx] == '[' || buff[idx+1] == '[')
		{
			while (buff[idx] != '\0' && buff[idx] != ':' && clen > 0)
			{
				idx++;
				clen--;
			}
		}
		// or ignore first word
		else
		{
			while (buff[idx] != '\0' && buff[idx] != ' ' && buff[idx] != ':' && clen > 0)
			{
				idx++;
				clen--;
			}
		}
		
		while (buff[idx] != '\0' && (buff[idx] == ' ' || buff[idx] == ':' || is_color (buff[idx])) && clen > 0)
		{
			idx++;
			clen--;
		}
		
		// check for hitting the EOS
		if (buff[idx] == '\0') idx = 0;
		// if we pass the upper test, entire line goes lower
		if (len - idx > 4 && my_isupper (&buff[idx], clen))
		{
			// don't call my_tolower, since we're not sure if the string is NULL terminated
			for (idx = 0; idx < len; idx++)
				buff[idx] = tolower (buff[idx]);
		}
	}
	
	//do we need to do any content filtering?
	if (filtered_so_far == 0) return len;

	// scan the text for any strings
	new_len = len;
	i = 0;
	while (i < new_len)
	{
		/* skip non-alpha characters */
		while (i < new_len && !isalpha ((unsigned char)buff[i])) i++;
		if (i >= new_len) break;
		
		/* check if we need to filter this word */
		idx = check_if_filtered (&buff[i]);
		if (idx >= 0)
		{
			/* oops, remove this word */
			if (filter_list[idx].wildcard_type > 0)
			{
				bad_len = 0;
				for (t = 0; ; t++)
				{
					if (!isalpha (buff[i+t])) break;
					bad_len++;
				}
			}
			else
			{
				bad_len = filter_list[idx].len;
			}
			rep_len = filter_list[idx].rlen;

			if (bad_len == rep_len)
			{
				memcpy(buff+i, filter_list[idx].replacement, rep_len);
			}
			else if (new_len + rep_len - bad_len >= size - 1)
			{
				// not enough space for substitution
				break;
			}
			else
			{
				memmove(buff+i+rep_len, buff+i+bad_len, new_len-i-bad_len+1);
				memcpy(buff+i, filter_list[idx].replacement, rep_len);
				new_len+= rep_len - bad_len;
			}
			/* don't filter the replacement text */
			i += rep_len;
		}
		else
		{
			/* skip this word */
			while (i < new_len && isalpha (buff[i])) i++;
		}
	}
	
	return new_len;
}


void load_filters_list (const char *file_name, char local)
{
	int f_size;
	FILE *f = NULL;
	char *filter_list_mem;
	int istart, iend;
	char name[128];
	size_t ret;

	f = open_file_config (file_name, "rb");
	if (f == NULL) return;
	fseek (f, 0, SEEK_END);
	f_size = ftell (f);
	if (f_size <= 0)
	{
		fclose(f);
		return;
	}

	//ok, allocate memory for it
	filter_list_mem = (char *) calloc (f_size, 1);
	fseek (f, 0, SEEK_SET);
	ret = fread (filter_list_mem, 1, f_size, f);
	fclose (f);
	if (ret != f_size)
	{
		free (filter_list_mem);
		LOG_ERROR("%s read failed for file [%s]\n", __FUNCTION__, file_name);
		return;
	}

	istart = 0;
	while (istart < f_size)
	{
		// find end of the line
		for (iend = istart; iend < f_size; iend++)
		{
			if (filter_list_mem[iend] == '\n' || filter_list_mem[iend] == '\r')
				break;
		}

		// copy the line and process it
		if (iend > istart)
		{
			safe_strncpy2 (name, filter_list_mem+istart, sizeof (name), iend-istart);
			if (add_to_filter_list (name, local, 0) == -2)		// -1 == already exists, -2 == list full
			{
				free (filter_list_mem);
				return; // filter list full
			}
		}

		// move to next line
		istart = iend+1;
	}

	free (filter_list_mem);
}


void clear_filter_list ()
{
	int i;
	//see if this name is already on the list
	for (i = 0; i < MAX_FILTERS; i++)
		filter_list[i].len = 0;
	filtered_so_far = 0;
}


void load_filters()
{
	char local_filters[256];
	safe_snprintf (local_filters, sizeof (local_filters), "%s/local_filters.txt", configdir);
	clear_filter_list ();
	load_filters_list (local_filters, 1);
	if (use_global_filters) load_filters_list ("global_filters.txt", 0);
}

int list_filters()
{
	int i, size, minlen;
	char *str;

	if (filtered_so_far == 0)
	{
		LOG_TO_CONSOLE (c_grey1, no_filters_str);
		return 1;
	}

	size = MAX_FILTERS*20;
	while (strlen (filters_str) + 2 >= size)
		size *= 2;
	str = calloc (size, sizeof (char));

	safe_snprintf(str, size * sizeof(char), "%s:\n",filters_str);
	for (i = 0; i < MAX_FILTERS; i++)
	{
		if (filter_list[i].len > 0 && (use_global_filters || filter_list[i].local))
		{
			minlen = filter_list[i].len + filter_list[i].rlen + 5;
			if (minlen >= size)
			{
				while (minlen >= size)
					size *= 2;
				str = realloc (str, size * sizeof (char));
			}
		
			strcat (str, filter_list[i].name);
			strcat (str, " = ");
			strcat (str, filter_list[i].replacement);
			strcat (str, ", ");
		}
	}

	str[strlen(str)-2] = '\0'; // get rid of the last ", " thingy
	LOG_TO_CONSOLE (c_grey1,str);

	free (str);
	return 1;
}

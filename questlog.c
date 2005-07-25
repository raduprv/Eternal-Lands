#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"

typedef struct ld
{
	char *msg;
	struct ld *Next;
}_logdata;

int questlog_win=-1;
int questlog_menu_x=150;
int questlog_menu_y=70;
int questlog_menu_x_len=STATS_TAB_WIDTH;
int questlog_menu_y_len=STATS_TAB_HEIGHT;
_logdata logdata,*last,*current;
int	logdata_length=0;
int quest_scroll_id = 14;
int nr_screen_lines = 0;
FILE *qlf = NULL;

int questlog_y;

void add_questlog_line(char *t, int len);   /* forward declaration */

void load_questlog()
{
	FILE *f = NULL;
	char temp[1000];

#ifndef WINDOWS
	char questlog_ini[256];
	snprintf (questlog_ini, sizeof (questlog_ini), "%s/quest.log", configdir);
	// don't use my_fopen here, not everyone uses local settings
	f= fopen(questlog_ini,"rb"); //try to load local settings
	if(!f)	//use global settings
		f= my_fopen("quest.log","rb");

#else
	f= my_fopen("quest.log","rb");
#endif
	logdata.msg= NULL;
	current= last= &logdata;

	if(!f)	return;

	while(!feof(f)){//loads and adds to a list all the quest log messages
		temp[0]= 0;
		fgets(temp,999,f);
		if(temp[0] == 0)break;
		add_questlog_line(temp, strlen(temp));
	}
	current=logdata.Next;
	fclose(f);
}


void unload_questlog()
{
	//frees all the quest list
	_logdata *t= logdata.Next;
	while(t!=NULL){
		_logdata *tmp= t;
		if(t->msg)	free(t->msg);
		t= t->Next;
		free(tmp);
	}
	if(qlf!=NULL)	fclose(qlf);
}


void string_fix(char *t, int len)
{
	char *s = t;
	int maxchar = (questlog_menu_x_len - 25) / 8;	// calculate maximum amount of characters per line
	int i = 0, j = 0, lastspace = 0, c = 0;


	while (s[i] != '\0' && len >= 0)
	{
		//breaking the string in parts
		if (s[i] == ' ')
		{
			c = 0;
			lastspace = i;
		}
		if (j > maxchar)
		{
			j = c;
			t[lastspace] = '\n';
			nr_screen_lines++;
		}
		i++;
		j++;
		c++;
		len--;
	}
}


void add_questlog(char *t, int len)
{
	char *s= t;

	//write on file
	if(qlf == NULL){
		#ifndef WINDOWS
			char questlog_ini[256];
			snprintf (questlog_ini, sizeof (questlog_ini), "%s/quest.log", configdir);
			// don't use my_fopen here, not everyone uses local settings
			// Heh? Changed local quest log to open in append mode too, instead
			// of overwrite
			qlf= fopen(questlog_ini,"ab");
			if(!qlf) //use global settings
				qlf= my_fopen("quest.log","ab");
			else
				fseek(qlf,SEEK_END,0);

		#else
			qlf= my_fopen("quest.log","ab");
		#endif
			if (qlf == NULL) return;
	}
	while(*s){ //converting multiline msg in single line
		if(*s=='\n')	*s= ' ';
		s++;
	}
	if(len <= 0)	len=strlen(t);
	fwrite(t, sizeof(char), len, qlf);
	fputc(10, qlf);
	//add to list
	add_questlog_line(t, len);
}


void add_questlog_line(char *t, int len)
{
	_logdata *l;

	if(len <= 0)	len=strlen(t);
	l= (_logdata*)malloc(sizeof(_logdata));
	l->Next= NULL;
	l->msg= (char*)malloc(len+1);
	string_fix(t, len);
	snprintf(l->msg, len+1, "%s", t);
	last->Next= l;
	last= l;
	if(current==NULL)	current= l;
	// keep track of the counter
	logdata_length++;

	// update the scrollbar length in the questlog window
	if (questlog_win >= 0)
		vscrollbar_set_bar_len (questlog_win, quest_scroll_id, logdata_length);
}


void goto_questlog_entry(int ln)
{
	static int current_line = 0;
	int cnt;

	if(ln <= 0)
	{
		// at or before the start
		current= &logdata;
		current_line = 0;
	}
	else if (ln >= logdata_length)
	{
		// at or after then end
		current= last;
		current_line = logdata_length - 1;
	}
	else if (ln > current_line)
	{
		cnt = ln - current_line;
		while(current->Next && cnt-- > 0)
			current = current->Next;
		current_line = ln;
	} 
	else if (ln < current_line)
	{
		//reset to the start
		current= &logdata;
		cnt = ln;
		// loop thru all of the entries
		while(current->Next && cnt-- > 0)
			current= current->Next;
		current_line = ln;
	}
}


int draw_questlog_string(char *t)
{
	char temp[256];
	int i= 0;

	//we split the string in lines and draw it.
	while (*t != 0)
	{
		while (*t != '\n' && *t != '\0')
		{
			temp[i] = *t;
			t++;
			i++;
		}
		if (*t != '\0')
			t++;
		temp[i] = '\0';
		i = 0;
		draw_string_small (2, questlog_y, temp, 1);
		questlog_y += 15;
		if (questlog_y > questlog_menu_y_len - 15)
			return 1;
	}
	
	return 0;
}


int	display_questlog_handler(window_info *win)
{
	_logdata *t= current;
	//calc where the scroll bar goes

	// Draw all texts from list
	questlog_y= 0;
	while(t!=NULL){
		if(t->msg && draw_questlog_string(t->msg))	return 1;
		t=t->Next;
	}
	return 1;
}


int questlog_click (widget_list *widget, int mx, int my, Uint32 flags)
{
	int scroll = vscrollbar_get_pos (questlog_win, widget->id);
	goto_questlog_entry (scroll);
	return 1;
}


int questlog_drag (widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy)
{
	int scroll = vscrollbar_get_pos (questlog_win, widget->id);
	goto_questlog_entry (scroll);
	return 1;
}

void fill_questlog_win ()
{
	int boxlen = 0;

	set_window_handler(questlog_win, ELW_HANDLER_DISPLAY, &display_questlog_handler );

	quest_scroll_id = vscrollbar_add_extended (questlog_win, quest_scroll_id, NULL, questlog_menu_x_len - 20, boxlen, 20, questlog_menu_y_len - boxlen, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, logdata_length);
	
	widget_set_OnClick (questlog_win, quest_scroll_id, questlog_click);
	widget_set_OnDrag (questlog_win, quest_scroll_id, questlog_drag);
}

void display_questlog()
{
	if(questlog_win < 0)
	{
		questlog_win= create_window("Quest", game_root_win, 0, questlog_menu_x, questlog_menu_y, questlog_menu_x_len, questlog_menu_y_len, ELW_WIN_DEFAULT);

		fill_questlog_win ();
	}
	else
	{
		show_window(questlog_win);
		select_window(questlog_win);
	}
}

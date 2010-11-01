#ifndef NEW_QUESTLOG

#include <stdlib.h>
#include <string.h>
#include "dialogues.h"
#include "questlog.h"
#include "asc.h"
#include "elwindows.h"
#include "gamewin.h"
#include "init.h"
#include "interface.h"
#include "tabs.h"
#include "errors.h"
#include "translate.h"
#include "io/elpathwrapper.h"

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
_logdata logdata= {NULL,NULL}, *last = &logdata, *current = NULL;
int	logdata_length=0;
int quest_scroll_id = 14;
int nr_screen_lines = 0;
FILE *qlf = NULL;
static char filename[256] = {'\0'};

int questlog_y;

void add_questlog_line(char *t, int len, const char *npcprefix);   /* forward declaration */

void load_questlog()
{
	FILE *f = NULL;
	FILE *f2 = NULL;
	char username[16], temp[1000];
	int i;
	
	safe_strncpy(username, username_str, sizeof(username));
	for (i = 0; username[i]; i++) {
		username[i] = tolower(username[i]);
	}
	
	safe_snprintf(filename, sizeof(filename), "quest_%s.log", username);
	f = open_file_config(filename,"rb");
	if(f == NULL){
		LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, cant_open_file, filename);
		//try the old filename: quest.log
		f = open_file_config("quest.log","rb");
		if(f == NULL){
			LOG_ERROR("%s: %s \"quest.log\"\n", reg_error_str, cant_open_file);
			return;
		}
		f2 = open_file_config(filename,"ab");//try to create a new quest log
		if(f2 == NULL){
			LOG_ERROR("%s: Can't create file \"%s\"\n", reg_error_str, filename);
		}
	}

	while(!feof(f)){//loads and adds to a list all the quest log messages
		temp[0]= 0;
		fgets(temp,999,f);
		if(temp[0] == 0)break;
		if(f2)
			fputs(temp,f2);//puts the quest messages into the new quest log
		add_questlog_line(temp, strlen(temp), "");
	}
	current=logdata.Next;
	fclose(f);
	if (f2)
		fclose(f2);
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


void add_questlog (char *t, int len)
{
	int idx;
	char npcprefix[30];

	//write on file
	if(qlf == NULL){
		qlf = open_file_config(filename, "ab");
		if(qlf == NULL){
			LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, cant_open_file, filename);
			return;
		}
		fseek(qlf,SEEK_END,0);
	}
	//convert multiline msg in single line
	for (idx = 0; idx < len && t[idx] != '\0'; idx++)
	{
		if (t[idx] == '\n') t[idx] = ' ';
	}

	safe_snprintf(npcprefix, sizeof(npcprefix), "%c%s: ", (char)to_color_char(c_blue2), (char *)npc_name);
	fwrite (npcprefix, sizeof (char), strlen(npcprefix), qlf);
	fwrite (t, sizeof (char), len, qlf);
	fputc ('\n', qlf);
	fflush(qlf);
	// add to list
	add_questlog_line (t, len, npcprefix);
}


void add_questlog_line(char *t, int len, const char *npcprefix)
{
	_logdata *l;
	int npclen = strlen(npcprefix);

	if(len <= 0)	len=strlen(t);
	l= (_logdata*)malloc(sizeof(_logdata));
	l->Next= NULL;
	l->msg= (char*)malloc(len + npclen + 1);
	safe_snprintf(l->msg, len+npclen+1, "%s%s", npcprefix, t);
	string_fix(l->msg, len + npclen);
	if (last!=NULL)
		last->Next= l;
	last= l;
	if(current==NULL)	current= l;
	// keep track of the counter
	logdata_length++;

	// update the scrollbar length in the questlog window
	if (questlog_win >= 0)
		vscrollbar_set_bar_len (questlog_win, quest_scroll_id, logdata_length-1);
}


void goto_questlog_entry(int ln)
{
	static int current_line = 0;
	int cnt;

	if(ln <= 0)
	{
		// at or before the start
		current= logdata.Next;
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
		current= logdata.Next;
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
		draw_string_small (2, questlog_y, (unsigned char*)temp, 1);
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


int questlog_scroll_click (widget_list *widget, int mx, int my, Uint32 flags)
{
	int scroll = vscrollbar_get_pos (questlog_win, widget->id);
	goto_questlog_entry (scroll);
	return 1;
}


int questlog_scroll_drag (widget_list *widget, int mx, int my, Uint32 flags, int dx, int dy)
{
	int scroll = vscrollbar_get_pos (questlog_win, widget->id);
	goto_questlog_entry (scroll);
	return 1;
}

int questlog_click(window_info *win, int mx, int my, Uint32 flags)
{
	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(questlog_win, quest_scroll_id);
		goto_questlog_entry(vscrollbar_get_pos(questlog_win, quest_scroll_id));
		return 1;
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(questlog_win, quest_scroll_id);
		goto_questlog_entry(vscrollbar_get_pos(questlog_win, quest_scroll_id));
		return 1;
	} else {
		return 0;
	}
}

static int boxlen = 0;

void fill_questlog_win ()
{

	set_window_handler(questlog_win, ELW_HANDLER_DISPLAY, &display_questlog_handler);
	set_window_handler(questlog_win, ELW_HANDLER_CLICK, &questlog_click);

	quest_scroll_id = vscrollbar_add_extended (questlog_win, quest_scroll_id, NULL, questlog_menu_x_len - 20, boxlen, 20, questlog_menu_y_len - boxlen, 0, 1.0, 0.77f, 0.57f, 0.39f, logdata_length-1, 1, logdata_length-1);
	goto_questlog_entry(logdata_length-1);
	
	widget_set_OnClick (questlog_win, quest_scroll_id, questlog_scroll_click);
	widget_set_OnDrag (questlog_win, quest_scroll_id, questlog_scroll_drag);
}

void display_questlog()
{
	if(questlog_win < 0)
	{
		questlog_win= create_window(tab_questlog, game_root_win, 0, questlog_menu_x, questlog_menu_y, questlog_menu_x_len, questlog_menu_y_len, ELW_WIN_DEFAULT);
		boxlen = ELW_BOX_SIZE;
		fill_questlog_win ();
	}
	else
	{
		show_window(questlog_win);
		select_window(questlog_win);
	}
}

#endif // #ifndef NEW_QUESTLOG

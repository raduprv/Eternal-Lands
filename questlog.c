#include <string.h>
#include "global.h"
#include "elwindows.h"

int questlog_win=0;
int questlog_menu_x=150;
int questlog_menu_y=70;
int questlog_menu_x_len=550;
int questlog_menu_y_len=300;
_logdata logdata,*last,*current;
int	logdata_length=0;
int	quest_page_start=0;
int	quest_page_pos=0;
FILE *qlf = NULL;

int questlog_y;


void load_questlog()
{
	FILE *f = NULL;
	char temp[1000];

#ifndef WINDOWS
	char questlog_ini[256];
	strcpy(questlog_ini, configdir);
	strcat(questlog_ini, "quest.log");
	f= fopen(questlog_ini,"rb"); //try to load local settings
	if(!f)	//use global settings
		f= fopen("quest.log","rb");

#else
	f= fopen("quest.log","rb");
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
	char *s= t;
	int maxchar= (questlog_menu_x_len-25)/8;// calculate maximum amount of characters per line
	int i=0, j=0, lastspace=0, c=0;


	while(s[i]!=0 && len >= 0){ //breaking the string in parts
		if(s[i]==' '){
			c= 0;
			lastspace= i;
		}
		if(j>maxchar){
			j= c;
			t[lastspace]= '\n';
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
			strcpy(questlog_ini, configdir);
			strcat(questlog_ini, "quest.log");
			qlf= fopen(questlog_ini,"wb");
			if(!qlf) //use global settings
				qlf= fopen("quest.log","ab");
			else
				fseek(qlf,SEEK_END,0);

		#else
			qlf= fopen("quest.log","ab");
		#endif
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
	strncpy(l->msg, t, len);
	l->msg[len]= 0;
	last->Next= l;
	last= l;
	if(current==NULL)	current= l;
	// keep track of the counter
	logdata_length++;
}


void goto_questlog_entry(int ln)
{
	int	cnt= ln;

	if(ln <= 0)
		{
			// at or before the start
			current= &logdata;
			return;
		}
	else if (ln >= logdata_length)
		{
			// at or after then end
			current= last;
			return;
		}
	//reset to the start
	current= &logdata;
	// loop thru all of the entries
	while(current->Next && cnt-- > 0)
		{
			current= current->Next;
		}
}


int draw_questlog_string(char *t)
{
	char temp[256];
	int i= 0;

	//we split the string in lines and draw it.
	while(*t!=0){
		while(*t!=10 && *t!=0){
			temp[i]= *t;
			t++;
			i++;
		}
		if(*t!=0)	t++;
		temp[i]= 0;
		i= 0;
		draw_string_small(2,questlog_y,temp,1);
		questlog_y+= 15;
		if(questlog_y > (questlog_menu_y_len-15))
			return 1;
	}
	return 0;
}


int	display_questlog_handler(window_info *win)
{
	_logdata *t= current;
	//calc where the scroll bar goes
	int scroll= quest_page_pos;

	//glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	//scroll bar
	glVertex3i(win->len_x-20,20,0);
	glVertex3i(win->len_x-20,win->len_y,0);
	glVertex3i(win->len_x-15,30,0);
	glVertex3i(win->len_x-10,25,0);
	glVertex3i(win->len_x-10,25,0);
	glVertex3i(win->len_x-5,30,0);
	glVertex3i(win->len_x-15,win->len_y-15,0);
	glVertex3i(win->len_x-10,win->len_y-10,0);
	glVertex3i(win->len_x-10,win->len_y-10,0);
	glVertex3i(win->len_x-5,win->len_y-15,0);
	glEnd();
	glBegin(GL_QUADS);
	//scroll bar
	glVertex3i(win->len_x-13,35+scroll,0);
	glVertex3i(win->len_x-7,35+scroll,0);
	glVertex3i(win->len_x-7,55+scroll,0);
	glVertex3i(win->len_x-13,55+scroll,0);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	// Draw all texts from list
	questlog_y= 0;
	while(t!=NULL){
		if(t->msg && draw_questlog_string(t->msg))	return 1;
		t=t->Next;
	}
	return 1;
}


int click_questlog_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int x,y;

	x= mx;
	y= my;
	if(x > win->len_x-16 && y > 18 && y < 18+16)
		{
			if(quest_page_start > 0)
				{
					quest_page_start--;
					goto_questlog_entry(quest_page_start);
					quest_page_pos= ((win->len_y-20-30-20)*quest_page_start)/logdata_length;
				}
			return 1;
		}
	if(x > win->len_x-16 && y > win->len_y-15 && y < win->len_y-4)
		{
			if(current && current->Next)
				{
					current= current->Next;
					quest_page_start++;
					quest_page_pos= ((win->len_y-20-30-20)*quest_page_start)/logdata_length;
				}
			return 1;
		}
	return 0;
}


int drag_questlog_handler(window_info *win, int mx, int my, Uint32 flags, int dx, int dy)
{
	int scroll_area= win->len_y-20-30-20;	// account for the X ^ V and the scrollbar

	if(win->drag_in || (logdata_length > 0 && mx>win->len_x-20 && my>35+quest_page_pos && my<55+quest_page_pos)) {
		win->drag_in= 1;
		//if(left_click>1)
		quest_page_pos+= dy;
		// bounds checking
		if(quest_page_pos < 0) quest_page_pos= 0;
		if(quest_page_pos >= scroll_area) quest_page_pos= scroll_area-1;
		//and set which item to list first
		quest_page_start= (logdata_length*quest_page_pos)/scroll_area;
		goto_questlog_entry(quest_page_start);
		return 1;
	}

	return 0;
}


void display_questlog()
{
	if(questlog_win <= 0)
		{
			questlog_win= create_window("Quest", 0, 0, questlog_menu_x, questlog_menu_y, questlog_menu_x_len, questlog_menu_y_len, ELW_WIN_DEFAULT);
			set_window_handler(questlog_win, ELW_HANDLER_DISPLAY, &display_questlog_handler );
			set_window_handler(questlog_win, ELW_HANDLER_CLICK, &click_questlog_handler );
			set_window_handler(questlog_win, ELW_HANDLER_DRAG, &drag_questlog_handler );
		}
	else
		{
			show_window(questlog_win);
			select_window(questlog_win);
		}
}


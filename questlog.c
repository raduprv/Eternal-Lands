#include <string.h>
#include "global.h"

int view_questlog=0;
int questlog_menu_x=150;
int questlog_menu_y=70;
int questlog_menu_x_len=550;
int questlog_menu_y_len=300;
int questlog_menu_dragged=0;
_logdata logdata,*last,*current;
FILE *qlf = NULL;

int y;

void load_questlog()
{
	FILE *f = NULL;
	char temp[1000];
	_logdata *L;

#ifndef WINDOWS
	char questlog_ini[256];
	strcpy(questlog_ini, configdir);
	strcat(questlog_ini, "quest.log");
	f=fopen(questlog_ini,"rb"); //try to load local settings
	if(!f) //use global settings
		f=fopen("quest.log","rb");

#else
	f=fopen("quest.log","rb");
#endif
	L=&logdata;
	logdata.msg=NULL;
	last=&logdata;
	current=NULL;

	if(!f)return;

	while(!feof(f)){//loads and adds to a list all the quest log messages
		_logdata *l;
		int len;

		temp[0]=0;
		fgets(temp,999,f);
		if(temp[0]==0)break;
		l=(_logdata*)malloc(sizeof(_logdata));
		L->Next=l;
		L=l;
		l->Next=0;
		len=strlen(temp);
		l->msg=(char*)malloc(len+1);
		string_fix(temp);
		strcpy(l->msg,temp);
	}
	last=L;
	current=logdata.Next;
	fclose(f);

}

void unload_questlog()
{
	//frees all the quest list
	_logdata *t=logdata.Next;
	while(t!=NULL){
		_logdata *tmp=t;
		if(t->msg)free(t->msg);
		t=t->Next;
		free(tmp);
	}
	if(qlf!=NULL)fclose(qlf);
}

void string_fix(char *t)
{
	char *s=t;
	int maxchar=(questlog_menu_x_len-25)/8;// calculate maximum amount of charcters per line
	int i=0,j=0,lastspace=0,c=0;


	while(s[i]!=0){ //breaking the string in parts
		if(s[i]==' '){c=0;lastspace=i;}
		if(j>maxchar){
			j=c;
			t[lastspace]='\n';
		}
		i++;
		j++;
		c++;
	}

}

void add_questlog(char *t)
{
	_logdata *l;
	//write on file
	if(qlf==NULL){
		#ifndef WINDOWS
			char questlog_ini[256];
			strcpy(questlog_ini, configdir);
			strcat(questlog_ini, "quest.log");
			qlf=fopen(questlog_ini,"wb");
			if(!qlf) //use global settings
				qlf=fopen("quest.log","ab");
			else
				fseek(qlf,SEEK_END,0);

		#else
			qlf=fopen("quest.log","ab");
		#endif
	}
	fwrite(t,sizeof(char),strlen(t),qlf);
	fputc(10,qlf);
	//add to list
	l=(_logdata*)malloc(sizeof(_logdata));
	l->Next=NULL;
	l->msg=(char*)malloc(strlen(t)+1);
	string_fix(t);
	strcpy(l->msg,t);
	last->Next=l;
	last=l;
	if(current==NULL)current=l;

}


int draw_questlog_string(char *t)
{
	char temp[256];
	int i=0;

	//we split the string in lines and draw it.
	while(*t!=0){
		while(*t!=10 && *t!=0){
			temp[i]=*t;
			t++;
			i++;
		}
		if(*t!=0)t++;
		temp[i]=0;
		i=0;
		draw_string_small(questlog_menu_x+2,y,temp,1);
		y+=15;
		if(y>(questlog_menu_y+questlog_menu_y_len-15))
			return 1;
	}
	return 0;
}


void display_questlog()
{
	_logdata *t=current;
	//title bar
	draw_menu_title_bar(questlog_menu_x,questlog_menu_y-16,questlog_menu_x_len);
	// window drawing
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(questlog_menu_x,questlog_menu_y+questlog_menu_y_len,0);
	glVertex3i(questlog_menu_x,questlog_menu_y,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len,questlog_menu_y,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len,questlog_menu_y+questlog_menu_y_len,0);
	glEnd();
	glDisable(GL_BLEND);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	glVertex3i(questlog_menu_x,questlog_menu_y,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len,questlog_menu_y,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len,questlog_menu_y,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len,questlog_menu_y+questlog_menu_y_len,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len,questlog_menu_y+questlog_menu_y_len,0);
	glVertex3i(questlog_menu_x,questlog_menu_y+questlog_menu_y_len,0);
	glVertex3i(questlog_menu_x,questlog_menu_y+questlog_menu_y_len,0);
	glVertex3i(questlog_menu_x,questlog_menu_y,0);
	// X corner
	glVertex3i(questlog_menu_x+questlog_menu_x_len,questlog_menu_y+20,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-20,questlog_menu_y+20,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-20,questlog_menu_y+20,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-20,questlog_menu_y,0);

	//scroll bar
	glVertex3i(questlog_menu_x+questlog_menu_x_len-20,questlog_menu_y+20,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-20,questlog_menu_y+questlog_menu_y_len,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-15,questlog_menu_y+30,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-10,questlog_menu_y+25,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-10,questlog_menu_y+25,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-5,questlog_menu_y+30,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-15,questlog_menu_y+questlog_menu_y_len-15,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-10,questlog_menu_y+questlog_menu_y_len-10,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-10,questlog_menu_y+questlog_menu_y_len-10,0);
	glVertex3i(questlog_menu_x+questlog_menu_x_len-5,questlog_menu_y+questlog_menu_y_len-15,0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	// The X
	draw_string(questlog_menu_x+questlog_menu_x_len-16,questlog_menu_y+2,"X",1);

	// Draw all texts from list
	y=questlog_menu_y;
	while(t!=NULL){
		if(draw_questlog_string(t->msg))return;
		t=t->Next;
	}
}

int check_questlog_interface()
{
	int x,y;
	if(!view_questlog || mouse_x>questlog_menu_x+questlog_menu_x_len || mouse_x<questlog_menu_x
	   || mouse_y<questlog_menu_y || mouse_y>questlog_menu_y+questlog_menu_y_len)return 0;

	x=mouse_x-questlog_menu_x;
	y=mouse_y-questlog_menu_y;
	if(x > questlog_menu_x_len-16 && x < questlog_menu_x_len &&
	   y > 18 && y < 18+16)
		{
			_logdata *t;
			if(logdata.Next && current!=logdata.Next && current!=NULL){
				t=logdata.Next;
				while(t!=NULL){
					if(t->Next==current){current=t;return 1;}
					t=t->Next;
				}
			}

		}
	if(x > questlog_menu_x_len-16 && x < questlog_menu_x_len &&
	   y > questlog_menu_y_len-15 && y < questlog_menu_y_len-4)
		{
			if(current)
				if(current->Next)current=current->Next;
		}
	return 1;
}

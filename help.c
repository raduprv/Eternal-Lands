#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"

int help_win=0;
int help_menu_x=150;
int help_menu_y=70;
int help_menu_x_len=500;
int help_menu_y_len=350;

int helppage;

int display_help_handler(window_info *win)
{
	_Text *t=Page[helppage].T.Next;
	_Image *i=Page[helppage].I.Next;

	while(t){
		int ylen=(t->size)?18:15;
		int xlen=strlen(t->text)*((t->size)?11:8);

		if(t->ref)
			{
				//draw a line
				glColor3f(0.5,0.5,0.5);
				glDisable(GL_TEXTURE_2D);
				glBegin(GL_LINES);
				glVertex3i(t->x+4,t->y+ylen,0);
				glVertex3i(t->x+4+xlen-8,t->y+ylen,0);
				glEnd();
				glEnable(GL_TEXTURE_2D);
			}
		if(t->size)
			{
				if(t->ref && mouse_x>(t->x+win->cur_x) && mouse_x<(t->x+xlen+win->cur_x) && mouse_y>(t->y+win->cur_y) && mouse_y<(t->y+ylen+win->cur_y))
				glColor3f(0.3,0.6,1.0);
				else
				glColor3f(t->r,t->g,t->b);
				draw_string(t->x,t->y,t->text,1);
			}
		else
			{
				if(t->ref && mouse_x>(t->x+win->cur_x) && mouse_x<(t->x+xlen+win->cur_x) && mouse_y>(t->y+win->cur_y) && mouse_y<(t->y+ylen+win->cur_y))
				glColor3f(0.3,0.6,1.0);
				else
				glColor3f(t->r,t->g,t->b);
				draw_string_small(t->x,t->y,t->text,1);
			}
		t=t->Next;
	}

	glColor3f(1.0f,1.0f,1.0f);
	while(i){
		if(i->mouseover==1){i=i->Next;continue;}
		if(mouse_x>(i->x+win->cur_x) && mouse_x<(win->cur_x+i->xend) && mouse_y>(i->y+win->cur_y) && mouse_y<(win->cur_y+i->yend)){
			if(i->Next!=NULL){
				if(i->Next->mouseover==1)
					i=i->Next;
			}
		}
		get_and_set_texture_id(i->id);
		glBegin(GL_QUADS);
		draw_2d_thing(i->u, i->v, i->uend, i->vend,i->x, i->y,i->xend,i->yend);
		glEnd();
		i=i->Next;
	}
	return 1;
}

int click_help_handler(window_info *win, int mx, int my, Uint32 flags)
{
	_Text *t=Page[helppage].T.Next;
	while(t){
		int xlen=strlen(t->text)*((t->size)?11:8),ylen=(t->size)?18:15;
		if(t->ref && mx>(t->x) && mx<(t->x+xlen) && my>(t->y) && my<(t->y+ylen)){
				//changing page
				int i;
				for(i=0;i<numpage+1;i++){
					if(!xmlStrcasecmp(Page[i].Name,t->ref)){
						helppage=i;
						break;
					}
				}

			break;
		}
		t=t->Next;
	}
	return 1;
}


void display_help()
{
	if(help_win <= 0)
		{
			int i;
			for(i=0;i<500;i++){
				if(my_strcompare(Page[i].Name,"HelpPage"))
					break;
			}
			helppage=i;
			help_win = create_window("help", 0, 0, help_menu_x, help_menu_y, help_menu_x_len, help_menu_y_len, ELW_WIN_DEFAULT);
			set_window_handler(help_win, ELW_HANDLER_DISPLAY, &display_help_handler );
			set_window_handler(help_win, ELW_HANDLER_CLICK, &click_help_handler );
		}
	else
		{
			show_window(help_win);
			select_window(help_win);
		}
}

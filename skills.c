#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"

int skills_win=-1;
int skills_menu_x=150;
int skills_menu_y=70;
int skills_menu_x_len=HELP_TAB_WIDTH;
int skills_menu_y_len=HELP_TAB_HEIGHT;

// Pixels to Scroll
int skills_max_lines=1000;

int skillspage;

int display_skills_handler(window_info *win)
{
	_Text *t=Page[skillspage].T.Next;
	_Image *i=Page[skillspage].I.Next;
	int j;
	j=vscrollbar_get_pos(skills_win,0);

	while(t){
		int ylen=(t->size)?18:15;
		int xlen=strlen(t->text)*((t->size)?11:8);

		if((t->y-j > 0) && (t->y-j < skills_menu_y_len-20 ))
		{
			if(t->ref)
			{
				//draw a line
				glColor3f(0.5,0.5,0.5);
				glDisable(GL_TEXTURE_2D);
				glBegin(GL_LINES);
				glVertex3i(t->x+4,t->y+ylen-j,0);
				glVertex3i(t->x+4+xlen-8,t->y+ylen-j,0);
				glEnd();
				glEnable(GL_TEXTURE_2D);
			}
			if(t->size)
			{
				if(t->ref && mouse_x>(t->x+win->cur_x) && mouse_x<(t->x+xlen+win->cur_x) && mouse_y>(t->y+win->cur_y-j) && mouse_y<(t->y+ylen+win->cur_y-j))
				glColor3f(0.3,0.6,1.0);
				else
				glColor3f(t->r,t->g,t->b);
				draw_string(t->x,t->y-j,t->text,1);
			}
			else
			{
				if(t->ref && mouse_x>(t->x+win->cur_x) && mouse_x<(t->x+xlen+win->cur_x) && mouse_y>(t->y+win->cur_y-j) && mouse_y<(t->y+ylen+win->cur_y-j))
				glColor3f(0.3,0.6,1.0);
				else
				glColor3f(t->r,t->g,t->b);
				draw_string_small(t->x,t->y-j,t->text,1);
			}
		}
		t=t->Next;
	}

	glColor3f(1.0f,1.0f,1.0f);
	while(i){
		if((i->y-j > 0) && (i->yend-j < skills_menu_y_len-40 ))
		{
			if(i->mouseover==1)
			{
				i=i->Next;
				continue;
			}
			if(mouse_x>(i->x+win->cur_x) && mouse_x<(win->cur_x+i->xend) && mouse_y>(i->y+win->cur_y-j) && mouse_y<(win->cur_y+i->yend-j))
			{
				if(i->Next!=NULL)
				{
					if(i->Next->mouseover==1)
						i=i->Next;
				}
			}
			get_and_set_texture_id(i->id);
			glBegin(GL_QUADS);
			draw_2d_thing(i->u, i->v, i->uend, i->vend,i->x, i->y-j,i->xend,i->yend-j);
			glEnd();
		}
		i=i->Next;
	}
	return 1;
}

int click_skills_handler(window_info *win, int mx, int my, Uint32 flags)
{
	_Text *t=Page[skillspage].T.Next;
	int j;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	j=vscrollbar_get_pos(skills_win,0);

	while(t){
		int xlen=strlen(t->text)*((t->size)?11:8),ylen=(t->size)?18:15;
		if(t->ref && mx>(t->x) && mx<(t->x+xlen) && my>(t->y-j) && my<(t->y+ylen-j)){
				//changing page
				int i;
				for(i=0;i<numpage+1;i++){
					if(!xmlStrcasecmp(Page[i].Name,t->ref)){
						skillspage=i;
						break;
					}
				}

			break;
		}
		t=t->Next;
	}
	return 1;
}

void fill_skills_win ()
{
	int i;
	for(i=0;i<=numpage;i++)
	{
		if(my_strcompare(Page[i].Name,"newskills"))
			break;
	}
	skillspage=i;
	set_window_handler (skills_win, ELW_HANDLER_DISPLAY, &display_skills_handler);
	set_window_handler (skills_win, ELW_HANDLER_CLICK, &click_skills_handler);

	vscrollbar_add_extended(skills_win, 0, NULL, skills_menu_x_len-20, 0, 20, skills_menu_y_len, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 1, skills_max_lines);
}

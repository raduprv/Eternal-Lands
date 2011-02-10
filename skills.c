#include <stdlib.h>
#include <string.h>
#include "skills.h"
#include "asc.h"
#include "elwindows.h"
#include "encyclopedia.h"
#include "interface.h"
#include "tabs.h"
#include "textures.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

int skills_win=-1;
int skills_menu_x=150;
int skills_menu_y=70;
int skills_menu_x_len=HELP_TAB_WIDTH;
int skills_menu_y_len=HELP_TAB_HEIGHT;
int skills_menu_scroll_id = 0;

// Pixels to Scroll
int skills_max_lines=1000;

int skillspage;

int display_skills_handler(window_info *win)
{
	_Text *t=Page[skillspage].T.Next;
	_Image *i=Page[skillspage].I.Next;
	int j = vscrollbar_get_pos(skills_win, skills_menu_scroll_id);

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
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
			}
			if(t->size)
			{
				if(t->ref && mouse_x>(t->x+win->cur_x) && mouse_x<(t->x+xlen+win->cur_x) && mouse_y>(t->y+win->cur_y-j) && mouse_y<(t->y+ylen+win->cur_y-j))
				glColor3f(0.3,0.6,1.0);
				else
				glColor3f(t->r,t->g,t->b);
				draw_string(t->x,t->y-j,(unsigned char*)t->text,1);
			}
			else
			{
				if(t->ref && mouse_x>(t->x+win->cur_x) && mouse_x<(t->x+xlen+win->cur_x) && mouse_y>(t->y+win->cur_y-j) && mouse_y<(t->y+ylen+win->cur_y-j))
				glColor3f(0.3,0.6,1.0);
				else
				glColor3f(t->r,t->g,t->b);
				draw_string_small(t->x,t->y-j,(unsigned char*)t->text,1);
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
#ifdef	NEW_TEXTURES
			bind_texture(i->id);
#else	/* NEW_TEXTURES */
			get_and_set_texture_id(i->id);
#endif	/* NEW_TEXTURES */
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

	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(skills_win, skills_menu_scroll_id);
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(skills_win, skills_menu_scroll_id);
	} else {
		int j = vscrollbar_get_pos(skills_win, skills_menu_scroll_id);

		while(t){
			int xlen=strlen(t->text)*((t->size)?11:8),ylen=(t->size)?18:15;
			if(t->ref && mx>(t->x) && mx<(t->x+xlen) && my>(t->y-j) && my<(t->y+ylen-j)){
					//changing page
					int i;
					for(i = 0; i < numpage+1; i++) {
						if(!xmlStrcasecmp((xmlChar*)Page[i].Name,(xmlChar*)t->ref)){
							skillspage=i;
							vscrollbar_set_pos(skills_win, skills_menu_scroll_id, 0);
							vscrollbar_set_bar_len(skills_win, skills_menu_scroll_id, Page[skillspage].max_y);
							break;
						}
					}
		
				break;
			}
			t=t->Next;
		}
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
	skillspage = i;
	set_window_handler (skills_win, ELW_HANDLER_DISPLAY, &display_skills_handler);
	set_window_handler (skills_win, ELW_HANDLER_CLICK, &click_skills_handler);

	skills_menu_scroll_id = vscrollbar_add_extended(skills_win, skills_menu_scroll_id, NULL, skills_menu_x_len-20, 0, 20, skills_menu_y_len, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 30, Page[skillspage].max_y);
}

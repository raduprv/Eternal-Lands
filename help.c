#include <stdlib.h>
#include <string.h>
#include "help.h"
#include "asc.h"
#include "elwindows.h"
#include "encyclopedia.h"
#include "interface.h"
#include "tabs.h"
#include "textures.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif

int help_win=-1;
int help_menu_x=150;
int help_menu_y=70;
int help_menu_x_len=HELP_TAB_WIDTH;
int help_menu_y_len=HELP_TAB_HEIGHT;
int help_menu_scroll_id = 0;

// Pixels to Scroll
int help_max_lines=1000;

int helppage;

int display_help_handler(window_info *win)
{
	_Text *t=Page[helppage].T.Next;
	_Image *i=Page[helppage].I.Next;
	int j;
	j=vscrollbar_get_pos(help_win,0);

	while(t){
		int ylen=(t->size)?18:15;
		int xlen=strlen(t->text)*((t->size)?11:8);

		if((t->y-j > 0) && (t->y-j < help_menu_y_len-20 ))
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
		if((i->y-j > 0) && (i->yend-j < help_menu_y_len-40 ))
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

int click_help_handler(window_info *win, int mx, int my, Uint32 flags)
{
	_Text *t=Page[helppage].T.Next;

	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(help_win, help_menu_scroll_id);
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(help_win, help_menu_scroll_id);
	} else {
		int j = vscrollbar_get_pos(help_win, help_menu_scroll_id);
	
		while(t){
			int xlen=strlen(t->text)*((t->size)?11:8),ylen=(t->size)?18:15;
			if(t->ref && mx>(t->x) && mx<(t->x+xlen) && my>(t->y-j) && my<(t->y+ylen-j)){
					//changing page
					int i;
					for(i=0;i<numpage+1;i++){
						if(!xmlStrcasecmp((xmlChar*)Page[i].Name,(xmlChar*)t->ref)){
							helppage=i;
							vscrollbar_set_pos(help_win, help_menu_scroll_id, 0);
							vscrollbar_set_bar_len(help_win, help_menu_scroll_id, Page[helppage].max_y);
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

void fill_help_win ()
{
	int i;
	for(i=0;i<=numpage;i++)
	{
		if(my_strcompare(Page[i].Name,"HelpPage"))
			break;
	}
	helppage=i;
	set_window_handler (help_win, ELW_HANDLER_DISPLAY, &display_help_handler);
	set_window_handler (help_win, ELW_HANDLER_CLICK, &click_help_handler);

	help_menu_scroll_id = vscrollbar_add_extended(help_win, help_menu_scroll_id, NULL, help_menu_x_len-20, 0, 20, help_menu_y_len, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 30, Page[helppage].max_y);
}

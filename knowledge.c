#include <stdlib.h>
#include <string.h>
#include "global.h"

int knowledge_win= -1;
int knowledge_menu_x= 100;
int knowledge_menu_y= 20;
int knowledge_menu_x_len= STATS_TAB_WIDTH;
int knowledge_menu_y_len= STATS_TAB_HEIGHT;
int knowledge_scroll_id= 13;
int knowledge_book_image_id;
int knowledge_book_label_id;
int knowledge_book_id= 0;

knowledge knowledge_list[KNOWLEDGE_LIST_SIZE];
int knowledge_count= 0;

char knowledge_string[400]="";

int add_knowledge_book_image() {
	// Book image
	int isize, tsize, tid, picsperrow, xtile, ytile, ssize, id;
	float ftsize, u, v, uend, vend;
	
	isize=256;
	tsize=51;
	ssize=100;
	tid=21;
	picsperrow=isize/tsize;
	xtile=tid%picsperrow;
	ytile=tid/picsperrow;
	ftsize=(float)tsize/isize;
	u=ftsize*xtile;
	v=-ftsize*ytile;
	uend=u+ftsize;
	vend=v-ftsize;
	id= load_texture_cache_deferred("textures/items1.bmp", 0);
	return image_add_extended(knowledge_win, 0, NULL, 500, 215, 50, 50, 0, 1.0, 1.0, 1.0, 1.0, id, u, v, uend, vend, 0.05f); 
}

int handle_knowledge_book()
{
	open_book(knowledge_book_id + 10000);
	// Bring the new window to the front               <----- Doesn't work. Is in front for the first usage, but not after that
	select_window(book_win);
	return 1;
}

int display_knowledge_handler(window_info *win)
{
	int i,x=2,y=2;
	int progress = (125*your_info.research_completed+1)/(your_info.research_total+1);
	int scroll = vscrollbar_get_pos (knowledge_win, knowledge_scroll_id);
	char points_string[16];
	char *research_string;
	
	if(your_info.research_total && 
	   (your_info.research_completed==your_info.research_total))
		safe_snprintf(points_string, sizeof(points_string), "%s", completed_research);
	else
		safe_snprintf(points_string, sizeof(points_string), "%4i/%-4i",your_info.research_completed,your_info.research_total);
	if(your_info.researching < knowledge_count)
	{
		research_string = knowledge_list[your_info.researching].name;
	}
	else
	{
		research_string = not_researching_anything;
		points_string[0] = '\0';
		progress = 1;
	}

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	// window separators
	glVertex3i(0,200,0);
	glVertex3i(win->len_x,200,0);
	glVertex3i(0,300,0);
	glVertex3i(win->len_x,300,0);
	//progress bar
	glVertex3i(330,315,0);
	glVertex3i(455,315,0);
	glVertex3i(330,335,0);
	glVertex3i(455,335,0);
	glVertex3i(330,315,0);
	glVertex3i(330,335,0);
	glVertex3i(455,315,0);
	glVertex3i(455,335,0);
	glEnd();
	glBegin(GL_QUADS);
	//progress bar
	glColor3f(0.40f,0.40f,1.00f);
	glVertex3i(331,316,0);
	glVertex3i(330+progress,316,0);
	glColor3f(0.10f,0.10f,0.80f);
	glVertex3i(330+progress,334,0);
	glVertex3i(331,334,0);
	glColor3f(0.77f,0.57f,0.39f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	//draw text
	draw_string_small(4,210,(unsigned char*)knowledge_string,4);
	glColor3f(1.0f,1.0f,1.0f);
	draw_string_small(10,320,(unsigned char*)researching_str,1);
	draw_string_small(120,320,(unsigned char*)research_string,1);
	draw_string_small(355,320,(unsigned char*)points_string,1);
	// Draw knowledges
	for(i = 2*scroll; i < 2 * (scroll + 19); i++)
	{
		if (knowledge_list[i].mouse_over)
		{
			glColor3f (0.1f,0.1f,0.9f);
		}
		else if (knowledge_list[i].present)
		{
			glColor3f (0.9f, 0.9f, 0.9f);
		}
		else
		{
			glColor3f (0.5f, 0.5f, 0.5f);
		}

		draw_string_zoomed(x,y,(unsigned char*)knowledge_list[i].name,1,0.7);

		x += 240;
		if (i % 2 == 1)
		{
			y += 10;
			x = 2;
		}
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

int mouseover_knowledge_handler(window_info *win, int mx, int my)
{
	int	i;

	for(i=0;i<knowledge_count;i++)knowledge_list[i].mouse_over=0;
	if(mx>win->len_x-20)
		return 0;
	if(my>192)
		return 0;
	mx/=240;
	my/=10;
	knowledge_list[mx+2*(my+vscrollbar_get_pos (knowledge_win, knowledge_scroll_id))].mouse_over=1;
	return 0;
}


int click_knowledge_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int x,y,idx;
	Uint8 str[3];

	x= mx;
	y= my;
	if(x > win->len_x-20)
		return 0;
	if(y > 192)
		return 0;

	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(knowledge_win, knowledge_scroll_id);
		return 1;
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(knowledge_win, knowledge_scroll_id);
		return 1;
	} else {
		
		x/=240;
		y/=10;
		idx = x + 2 *(y + vscrollbar_get_pos (knowledge_win, knowledge_scroll_id));
		if(idx < knowledge_count)
			{
				str[0] = GET_KNOWLEDGE_INFO;
				*(Uint16 *)(str+1) = SDL_SwapLE16((short)idx);
				my_tcp_send(my_socket,str,3);
				// Check if we display the book image and label
				knowledge_book_id = idx;
				if (knowledge_list[idx].present && knowledge_list[idx].has_book) {
					widget_unset_flag(knowledge_win, knowledge_book_image_id, WIDGET_DISABLED);
					widget_unset_flag(knowledge_win, knowledge_book_label_id, WIDGET_DISABLED);
				} else {
					widget_set_flags(knowledge_win, knowledge_book_image_id, WIDGET_DISABLED);
					widget_set_flags(knowledge_win, knowledge_book_label_id, WIDGET_DISABLED);
				}
			}
	}
	return 1;
} 


void get_knowledge_list (Uint16 size, const char *list)
{
	int i;
	
	// make sure the entire knowledge list is 0 incase of short data
	for(i=0; i<KNOWLEDGE_LIST_SIZE; i++){
		knowledge_list[i].present= 0;
	}
	
	// watch for size being too large
	if(size*8 > KNOWLEDGE_LIST_SIZE){
		size= KNOWLEDGE_LIST_SIZE/8;
	}
	
	// now copy the data
	for(i=0; i<size; i++)
	{
		knowledge_list[i*8+0].present= list[i] & 0x01;
		knowledge_list[i*8+1].present= list[i] & 0x02;
		knowledge_list[i*8+2].present= list[i] & 0x04;
		knowledge_list[i*8+3].present= list[i] & 0x08;
		knowledge_list[i*8+4].present= list[i] & 0x10;
		knowledge_list[i*8+5].present= list[i] & 0x20;
		knowledge_list[i*8+6].present= list[i] & 0x40;
		knowledge_list[i*8+7].present= list[i] & 0x80;
	}
}

void get_new_knowledge(Uint16 idx)
{
	if(idx < KNOWLEDGE_LIST_SIZE){
		knowledge_list[idx].present= 1;
	}
}

void fill_knowledge_win ()
{
	set_window_handler(knowledge_win, ELW_HANDLER_DISPLAY, &display_knowledge_handler );
	set_window_handler(knowledge_win, ELW_HANDLER_CLICK, &click_knowledge_handler );
	set_window_handler(knowledge_win, ELW_HANDLER_MOUSEOVER, &mouseover_knowledge_handler );
	
	knowledge_scroll_id = vscrollbar_add_extended (knowledge_win, knowledge_scroll_id, NULL, knowledge_menu_x_len - 20,  0, 20, 200, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 10, (knowledge_count+2)/2-19);
	knowledge_book_image_id = add_knowledge_book_image();
	widget_set_OnClick(knowledge_win, knowledge_book_image_id, &handle_knowledge_book);
	widget_set_flags(knowledge_win, knowledge_book_image_id, WIDGET_DISABLED);
	knowledge_book_label_id = label_add_extended(knowledge_win, knowledge_book_image_id + 1, NULL, 485, 265, WIDGET_DISABLED, 0.8, 1.0, 1.0, 1.0, knowledge_read_book);
	widget_set_OnClick(knowledge_win, knowledge_book_label_id, &handle_knowledge_book);
}

void display_knowledge()
{
	if(knowledge_win < 0)
	{
		knowledge_win= create_window("Knowledge", game_root_win, 0, knowledge_menu_x, knowledge_menu_y, knowledge_menu_x_len, knowledge_menu_y_len, ELW_WIN_DEFAULT);		
		fill_knowledge_win ();
	} 
	else 
	{
		show_window(knowledge_win);
		select_window(knowledge_win);
	}
}

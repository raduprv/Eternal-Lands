#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "keys.h"
#include "elwindows.h"
#include "widgets.h"

windows_info	windows_list;	// the master list of windows

/*
 * The intent of the windows system is to create the window once
 * and then hide the window when you don't wantto use it.
 *
 * Window #0 is special ... it represents the entire screen
 *
 * Note: In the handlers, all cursor coordinates are relative to the window
 *
 */

// general windows manager functions
void	display_windows(int level)
{
	int	id;
	int	next_id;
	int i;

	windows_list.display_level= level;
	glColor3f(1.0f, 1.0f, 1.0f);
	// first draw everything that is last under everything
	id= -1;
	while(1)
		{
			next_id= -9999;
			for(i=1; i<windows_list.num_windows; i++){
				// only look at displayed windows
				if(windows_list.window[i].displayed > 0){
					// at this level?
					if(windows_list.window[i].order == id){
						display_window(i);
					} else if(windows_list.window[i].order < id && windows_list.window[i].order > next_id){
						// try to find the next level
						next_id= windows_list.window[i].order;
					}
				}
			}
			if(next_id <= -9999)
				{
					break;
				}
			else
				{
					id= next_id;
				}
		}
	if(level > 0)
		{
			// now display each window in the proper order
			id= 1;
			while(1)
				{
					next_id= 9999;
					for(i=1; i<windows_list.num_windows; i++){
						// only look at displayed windows
						if(windows_list.window[i].displayed > 0){
							// at this level?
							if(windows_list.window[i].order == id){
								display_window(i);
							} else if(windows_list.window[i].order > id && windows_list.window[i].order < next_id){
								// try to find the next level
								next_id= windows_list.window[i].order;
							}
						}
					}
					if(next_id >= 9999)
						{
							break;
						}
					else
						{
							id= next_id;
						}
				}
		}
}


int	click_in_windows(int mx, int my, Uint32 flags)
{
	int	done= 0;
	int	id;
	int	next_id;
	int	first_win= 0;
	int i;

	// watch for needing to convert the globals into the flags
	if(!flags){
		if(shift_on)	flags |= ELW_SHIFT;
		if(ctrl_on)		flags |= ELW_CTRL;
		if(alt_on)		flags |= ELW_ALT;
		if(right_click)	flags |= ELW_RIGHT_MOUSE;
		if(middle_click)	flags |= ELW_MID_MOUSE;
		if(left_click)	flags |= ELW_LEFT_MOUSE;
		// TODO: centralized double click handling
		// TODO: consider other ways of triggering double clieck, like middle click or shift click
		//if(double_click)	flags |= ELW_DBL_CLICK;
	}

	// check each window in the proper order
	if(windows_list.display_level > 0)
		{
			id= 9999;
			while(done <= 0)
				{
					next_id= 0;
					for(i=1; i<windows_list.num_windows; i++){
						// only look at displayed windows
						if(windows_list.window[i].displayed > 0){
							// at this level?
							if(windows_list.window[i].order == id){
								done= click_in_window(i, mx, my, flags);
								if(done > 0){
									if(windows_list.window[i].displayed > 0)	select_window(i);	// select this window to the front
									return i;
								}
								if(first_win == 0 && mouse_in_window(i, mx, my))	first_win= i;
							} else if(windows_list.window[i].order < id && windows_list.window[i].order > next_id){
								// try to find the next level
								next_id= windows_list.window[i].order;
							}
						}
					}
					if(next_id <= 0)
						{
							break;
						}
					else
						{
							id= next_id;
						}
				}
		}
	// now check the background windows in the proper order
	id= -9999;
	while(done <= 0)
		{
			next_id= 0;
			for(i=1; i<windows_list.num_windows; i++){
				// only look at displayed windows
				if(windows_list.window[i].displayed > 0){
					// at this level?
					if(windows_list.window[i].order == id){
						done= click_in_window(i, mx, my, flags);
						if(done > 0){
							//select_window(i);	// these never get selected
							return i;
						}
					} else if(windows_list.window[i].order > id && windows_list.window[i].order < next_id){
						// try to find the next level
						next_id= windows_list.window[i].order;
					}
				}
			}
			if(next_id >= 0)
				{
					break;
				}
			else
				{
					id= next_id;
				}
		}
	// nothing to click on, do a select instead
	if(first_win > 0)
		{
			select_window(first_win);
			return 1;
		}
	return 0;	// no click in a window
}

int	drag_in_windows(int mx, int my, Uint32 flags, int dx, int dy)
{
	int	done= 0;
	int	id;
	int	next_id;
	int	first_win= 0;
	int i;

	// ignore a drag of 0, but say we processed
	if(dx == 0 && dy == 0)	return 0;
	// watch for needing to convert the globals into the flags
	if(!flags){
		if(shift_on)	flags |= ELW_SHIFT;
		if(ctrl_on)		flags |= ELW_CTRL;
		if(alt_on)		flags |= ELW_ALT;
		if(right_click)	flags |= ELW_RIGHT_MOUSE;
		if(middle_click)	flags |= ELW_MID_MOUSE;
		if(left_click)	flags |= ELW_LEFT_MOUSE;
		// TODO: centralized double click handling
		// TODO: consider other ways of triggering double click, like middle click or shift click
		//if(double_click)	flags |= ELW_DBL_CLICK;
	}

	// check each window in the proper order
	if(windows_list.display_level > 0)
		{
			id= 9999;
			while(done <= 0)
				{
					next_id= 0;
					for(i=1; i<windows_list.num_windows; i++){
						// only look at displayed windows
						if(windows_list.window[i].displayed > 0){
							// at this level?
							if(windows_list.window[i].order == id){
								done= drag_in_window(i, mx, my, flags, dx, dy);
								if(done > 0){
									if(windows_list.window[i].displayed > 0)	select_window(i);	// select this window to the front
									return i;
								}
								if(first_win == 0 && mouse_in_window(i, mx, my))	first_win= i;
							} else if(windows_list.window[i].order < id && windows_list.window[i].order > next_id){
								// try to find the next level
								next_id= windows_list.window[i].order;
							}
						}
					}
					if(next_id <= 0)
						{
							break;
						}
					else
						{
							id= next_id;
						}
				}
		}
	// now check the background windows in the proper order
	id= -9999;
	while(done <= 0)
		{
			next_id= 0;
			for(i=1; i<windows_list.num_windows; i++){
				// only look at displayed windows
				if(windows_list.window[i].displayed > 0){
					// at this level?
					if(windows_list.window[i].order == id){
						done= drag_in_window(i, mx, my, flags, dx, dy);
						if(done > 0){
							//select_window(i);	// these never get selected
							return i;
						}
					} else if(windows_list.window[i].order > id && windows_list.window[i].order < next_id){
						// try to find the next level
						next_id= windows_list.window[i].order;
					}
				}
			}
			if(next_id >= 0)
				{
					break;
				}
			else
				{
					id= next_id;
				}
		}
	return 0;	// no drag in a window
}


int	drag_windows(int mx, int my, int dx, int dy)
{
	int	next_id;
	int	id, i;
	int	drag_id= 0;

	// check each window in the proper order for which one might be getting dragged
	if(windows_list.display_level > 0)
		{
			id= 9999;
			while(drag_id <= 0)
				{
					next_id= 0;
					for(i=1; i<windows_list.num_windows; i++){
						// only look at displayed windows
						if(windows_list.window[i].displayed > 0 && (windows_list.window[i].flags&ELW_DRAGGABLE)){
							// at this level?
							if(windows_list.window[i].order == id){
								// check for being actively dragging or on the top bar
								if(windows_list.window[i].dragged || (mouse_in_window(i, mx, my) && my<windows_list.window[i].cur_y) ){
									drag_id= i;
									break;
								} else if(mouse_in_window(i, mx, my)){
									// stop processing if we are inside of another window
									return 0;
								}
							} else if(windows_list.window[i].order < id && windows_list.window[i].order > next_id){
								// try to find the next level
								next_id= windows_list.window[i].order;
							}
						}
					}
					if(next_id <= 0)
						{
							break;
						}
					else
						{
							id= next_id;
						}
				}
		}

	// this section probably won't be needed, included to be complete
	// now check the background windows in the proper order for which one might be getting dragged
	id= -9999;
	while(drag_id <= 0)
		{
			next_id= 0;
			for(i=1; i<windows_list.num_windows; i++){
				// only look at displayed windows
				if(windows_list.window[i].displayed > 0 && (windows_list.window[i].flags&ELW_DRAGGABLE)){
					// at this level?
					if(windows_list.window[i].order == id){
						// check for being actively dragging or on the top bar
						if(windows_list.window[i].dragged || (mouse_in_window(i, mx, my) && my<windows_list.window[i].cur_y) ){
							drag_id= i;
							break;
						} else if(mouse_in_window(i, mx, my)){
							// stop processing if we are inside of another window
							return 0;
						}
					} else if(windows_list.window[i].order > id && windows_list.window[i].order < next_id){
						// try to find the next level
						next_id= windows_list.window[i].order;
					}
				}
			}
			if(next_id >= 0)
				{
					break;
				}
			else
				{
					id= next_id;
				}
		}

	// are we dragging a window?
	if(drag_id <= 0)	return 0;

	// dragged window is always on top
	select_window(drag_id);
	// flag we are dragging
	windows_list.window[drag_id].dragged= 1;
	if(left_click>1 && (dx != 0 || dy != 0))	// TODO: avoid globals?
		{
			// move to new location
			move_window(drag_id, windows_list.window[drag_id].pos_id, windows_list.window[drag_id].pos_loc,
					windows_list.window[drag_id].pos_x+dx, windows_list.window[drag_id].pos_y+dy);
		}

	return 1;
}


void	end_drag_windows()
{
	int	i;

	for(i= 0; i<windows_list.num_windows; i++)
		{
			windows_list.window[i].dragged= 0;
			windows_list.window[i].drag_in= 0;
		}
}


int	select_window(int win_id)
{
	int	i, old;

	if(win_id <=0 || win_id >= windows_list.num_windows)	return -1;
	if(windows_list.window[win_id].window_id != win_id)	return -1;

	// shuffle the order of the windows
	old= windows_list.window[win_id].order;
	if(old <= 0)	return 0;	// show last are never shuffled
	for(i=1; i<windows_list.num_windows; i++){
		if(windows_list.window[i].order > old){
			windows_list.window[i].order--;
		}
	}
	// and put it on top
	windows_list.window[win_id].order= windows_list.num_windows-1;

	return 1;
}



// specific windows functions
int	create_window(const Uint8 *name, int pos_id, Uint32 pos_loc, int pos_x, int pos_y, int size_x, int size_y, Uint32 property_flags)
{
	window_info *win;
	int	win_id=-1;
	int	i;

	// verify that we are setup and space allocated
	if(!windows_list.window)
		{
			// allocate the space
			windows_list.num_windows= 0;
			windows_list.max_windows= 32;
			windows_list.window=(window_info *)calloc(32, sizeof(window_info));
			windows_list.window[0].window_id= -1;	// force a rebuild of this
			windows_list.num_windows= 1;
		}
	// now, verify that the main window is correct
	if(windows_list.window[0].window_id != 0 || windows_list.window[0].len_x != window_width)
		{
			// this window is currently here only to help position things
			// this represents the entire screen
			windows_list.window[0].window_id= 0;
			windows_list.window[0].order= -1;
			windows_list.window[0].pos_id= -1;
			windows_list.window[0].pos_loc= 0;
			windows_list.window[0].pos_x= 0;
			windows_list.window[0].pos_y= 0;
			windows_list.window[0].len_x= window_width;
			windows_list.window[0].len_y= window_height;
			windows_list.window[0].cur_x= 0;
			windows_list.window[0].cur_y= 0;

			windows_list.window[0].flags=0;
			windows_list.window[0].displayed=0;
		}

	// find an empty slot
	for(i=1; i<windows_list.num_windows; i++)
		{
			if(windows_list.window[i].window_id < 0)
				{
					win_id=i;
					break;
				}
		}

	// need a new_entry?
	if(win_id < 0)
		{
			if(windows_list.num_windows < windows_list.max_windows-1)
				{
					win_id= windows_list.num_windows++;
				}
		}

	// fill in the information
	if(win_id > 0)
		{
			win= &windows_list.window[win_id];
			win->window_id= win_id;
			win->order= (property_flags&ELW_SHOW_LAST)?-win_id:win_id;

			win->flags= property_flags;
			win->displayed= (property_flags&ELW_SHOW)?1:0;
			//win->collapsed= 0;
			win->dragged= 0;
			strncpy(win->window_name, name, 32);

			win->back_color[0]= 0.0f;
			win->back_color[1]= 0.0f;
			win->back_color[2]= 0.0f;
			win->back_color[3]= 0.5f;
			win->border_color[0]= 0.77f;
			win->border_color[1]= 0.57f;
			win->border_color[2]= 0.39f;
			win->border_color[3]= 0.0f;
			win->line_color[0]= 0.77f;
			win->line_color[1]= 0.57f;
			win->line_color[2]= 0.39f;
			win->line_color[3]= 0.0f;

			win->init_handler= NULL;
			win->display_handler= NULL;
			win->click_handler= NULL;
			win->drag_handler= NULL;
			win->mouseover_handler= NULL;
			// now call the routine to place it properly
			init_window(win_id, pos_id, pos_loc, pos_x, pos_y, size_x, size_y);
		}

	return	win_id;
}


void	destroy_window(int win_id)
{
	if(win_id <=0 || win_id >= windows_list.num_windows)	return;
	if(windows_list.window[win_id].window_id != win_id)	return;
	// mark the window as unused
	windows_list.window[win_id].window_id= -1;
	windows_list.window[win_id].order= -1;
	windows_list.window[win_id].displayed= 0;
}


int		find_window(const char *name)
{
	int	win_id= -1;
	int	i;

	for(i=0; i<windows_list.num_windows; i++)
		{
			if(!strcmp(windows_list.window[win_id].window_name, name))
				{
					win_id= i;
					break;
				}
		}

	return win_id;
}


int	init_window(int win_id, int pos_id, Uint32 pos_loc, int pos_x, int pos_y, int size_x, int size_y)
{
	if(win_id <=0 || win_id >= windows_list.num_windows)	return -1;
	if(windows_list.window[win_id].window_id != win_id)	return -1;

	// memorize the size
	windows_list.window[win_id].len_x= size_x;
	windows_list.window[win_id].len_y= size_y;
	// then place the window
	move_window(win_id, pos_id, pos_loc, pos_x, pos_y);

	// finally, call any init_handler that was defined
	if(windows_list.window[win_id].init_handler)
		{
			return((*windows_list.window[win_id].init_handler)(&windows_list.window[win_id]));
		}
	return 1;
}

int	move_window(int win_id, int pos_id, Uint32 pos_loc, int pos_x, int pos_y)
{
	window_info *win;

	if(win_id <=0 || win_id >= windows_list.num_windows)	return -1;
	win= &windows_list.window[win_id];
	if(win->window_id != win_id)	return -1;

	win->pos_id= pos_id;	//NOT SUPPORTED YET
	win->pos_loc= pos_loc;	//NOT SUPPORTED YET
	win->pos_x= pos_x;
	win->pos_y= pos_y;
	win->cur_x= pos_x;	//TODO: calc based on pos_id & pos_loc
	win->cur_y= pos_y;	//TODO: calc based on pos_id & pos_loc

	// check for the window actually being on the screen, if not, move it
	if(win->cur_y < ((win->flags&ELW_TITLE_BAR)?ELW_TITLE_HEIGHT:0)) win->cur_y= (win->flags&ELW_TITLE_BAR)?ELW_TITLE_HEIGHT:0;
	if(win->cur_y >= window_height) win->cur_y= window_height;	// had -32, but do we want that?
	if(win->cur_x+win->len_x < ELW_BOX_SIZE) win->cur_x= 0-win->len_x+ELW_BOX_SIZE;
	if(win->cur_x > window_width-ELW_BOX_SIZE) win->cur_x= window_width-ELW_BOX_SIZE;

	return 1;
}

int	draw_window_title(window_info *win)
{
	float u_first_start= (float)31/255;
	float u_first_end= 0;
	float v_first_start= 1.0f-(float)160/255;
	float v_first_end= 1.0f-(float)175/255;

	float u_middle_start= (float)32/255;
	float u_middle_end= (float)63/255;
	float v_middle_start= 1.0f-(float)160/255;
	float v_middle_end= 1.0f-(float)175/255;

	float u_last_start= 0;
	float u_last_end= (float)31/255;
	float v_last_start= 1.0f-(float)160/255;
	float v_last_end= 1.0f-(float)175/255;

	if((win->flags&ELW_TITLE_BAR) == ELW_TITLE_NONE)	return 0;

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now draw that shit...

	get_and_set_texture_id(icons_text);
	glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.03f);
	glBegin(GL_QUADS);

	glTexCoord2f(u_first_end, v_first_start);
	glVertex3i(0, -ELW_TITLE_HEIGHT, 0);
	glTexCoord2f(u_first_end, v_first_end);
	glVertex3i(0, 0, 0);
	glTexCoord2f(u_first_start, v_first_end);
	glVertex3i(32, 0, 0);
	glTexCoord2f(u_first_start, v_first_start);
	glVertex3i(32, -ELW_TITLE_HEIGHT, 0);

	// draw one streched out cell to the proper size
	glTexCoord2f(u_middle_end, v_middle_start);
	glVertex3i(32, -ELW_TITLE_HEIGHT, 0);
	glTexCoord2f(u_middle_end, v_middle_end);
	glVertex3i(32, 0, 0);
	glTexCoord2f(u_middle_start, v_middle_end);
	glVertex3i(win->len_x-32, 0, 0);
	glTexCoord2f(u_middle_start, v_middle_start);
	glVertex3i(win->len_x-32, -ELW_TITLE_HEIGHT, 0);

	glTexCoord2f(u_last_end, v_last_start);
	glVertex3i(win->len_x-32, -ELW_TITLE_HEIGHT, 0);
	glTexCoord2f(u_last_end, v_last_end);
	glVertex3i(win->len_x-32, 0, 0);
	glTexCoord2f(u_last_start, v_last_end);
	glVertex3i(win->len_x, 0, 0);
	glTexCoord2f(u_last_start, v_last_start);
	glVertex3i(win->len_x, -ELW_TITLE_HEIGHT, 0);

	glEnd();
	glDisable(GL_ALPHA_TEST);

	// draw the name of the window
	if(win->flags&ELW_TITLE_NAME)
		{
			int	len;

			glEnable(GL_TEXTURE_2D);
			glColor3f(win->border_color[0],win->border_color[1],win->border_color[2]);
			// center text
			len=(get_string_width(win->window_name)*8)/12;
			draw_string_small((win->len_x-len)/2, -ELW_TITLE_HEIGHT, win->window_name, 1);
		}

	return 1;
}

int	draw_window_border(window_info *win)
{
	glDisable(GL_TEXTURE_2D);
	if(win->flags&ELW_USE_BACKGROUND)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_SRC_ALPHA);
			glColor4f(win->back_color[0],win->back_color[1],win->back_color[2],win->back_color[3]);
			glBegin(GL_QUADS);
			glVertex3i(0, win->len_y, 0);
			glVertex3i(0, 0, 0);
			glVertex3i(win->len_x, 0, 0);
			glVertex3i(win->len_x, win->len_y, 0);
			glEnd();

			glDisable(GL_BLEND);
		}

	if(win->flags&ELW_USE_BORDER)
		{
			glColor3f(win->border_color[0],win->border_color[1],win->border_color[2]);
			glBegin(GL_LINES);
			glVertex3i(0, 0, 0);
			glVertex3i(win->len_x, 0, 0);
			glVertex3i(win->len_x, 0, 0);
			glVertex3i(win->len_x, win->len_y, 0);
			glVertex3i(win->len_x, win->len_y, 0);
			glVertex3i(0, win->len_y, 0);
			glVertex3i(0, win->len_y, 0);
			glVertex3i(0, 0, 0);
			glEnd();
		}

	if(win->flags&ELW_CLOSE_BOX)
		{
			//draw the corner, with the X in
			glColor3f(win->border_color[0],win->border_color[1],win->border_color[2]);
			glBegin(GL_LINES);
			glVertex3i(win->len_x, ELW_BOX_SIZE, 0);
			glVertex3i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE, 0);

			glVertex3i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE, 0);
			glVertex3i(win->len_x-ELW_BOX_SIZE, 0, 0);
			glEnd();

			glEnable(GL_TEXTURE_2D);
			draw_string(win->len_x-(ELW_BOX_SIZE-4), 2, "X", 1);
		}
	else glEnable(GL_TEXTURE_2D);

	return 1;
}

int	draw_window(window_info *win)
{
	int	ret_val=0;
	widget_list *W = &win->widgetlist;
	if(win == NULL || win->window_id < 0)	return -1;

	if(!win->displayed)	return 0;
	// mouse over processing first
	mouseover_window(win->window_id, mouse_x, mouse_y);
	// now normal display processing
	glPushMatrix();
	glTranslatef((float)win->cur_x, (float)win->cur_y, 0.0f);

	draw_window_title(win);
	draw_window_border(win);
	glColor3f(1.0f, 1.0f, 1.0f);

	if(win->display_handler)
		{
			ret_val=(*win->display_handler)(win);
		}
	else
		{
		ret_val=1;
		}

	// widget drawing
	while(W->next != NULL){
		W = W->next;
		if(W->OnDraw != NULL)
			W->OnDraw(W);
	}

	glPopMatrix();
	
	return(ret_val);
}

void	show_window(int win_id)
{
	if(win_id <=0 || win_id >= windows_list.num_windows)	return;
	if(windows_list.window[win_id].window_id != win_id)	return;

	// pull to the top if not currently displayed
	if(!windows_list.window[win_id].displayed)	select_window(win_id);
	windows_list.window[win_id].displayed= 1;
}

void	hide_window(int win_id)
{
	if(win_id <=0 || win_id >= windows_list.num_windows)	return;
	if(windows_list.window[win_id].window_id != win_id)	return;

	windows_list.window[win_id].displayed= 0;
}

void	toggle_window(int win_id)
{
	if(win_id <=0 || win_id >= windows_list.num_windows)	return;
	if(windows_list.window[win_id].window_id != win_id)	return;

	if(!windows_list.window[win_id].displayed)
		select_window(win_id);
	windows_list.window[win_id].displayed=!windows_list.window[win_id].displayed;
}


int	get_show_window(int win_id)
{
	// unititialized windows always fail as if not shown, not an error
	if(win_id <=0 || win_id >= windows_list.num_windows)	return 0;
	if(windows_list.window[win_id].window_id != win_id)	return 0;

	return windows_list.window[win_id].displayed;
}

int	display_window(int win_id)
{
	if(win_id <=0 || win_id >= windows_list.num_windows)	return -1;
	if(windows_list.window[win_id].window_id != win_id)	return -1;
	// is it active/displayed?
	if(windows_list.window[win_id].displayed)
		{
			return(draw_window(&windows_list.window[win_id]));
		}
	return 0;
}

int	mouse_in_window(int win_id, int x, int y)
{
	// NOTE: these tests do not take depth into account, just location
	// Returns -1 on error, 0 No, 1 yes
	if(win_id <=0 || win_id >= windows_list.num_windows)	return -1;
	if(windows_list.window[win_id].window_id != win_id)	return -1;

	if(x<windows_list.window[win_id].cur_x || x>=windows_list.window[win_id].cur_x+windows_list.window[win_id].len_x)	return 0;
	if(y<windows_list.window[win_id].cur_y-((windows_list.window[win_id].flags&ELW_TITLE_BAR)?ELW_TITLE_HEIGHT:0) || y>=windows_list.window[win_id].cur_y+windows_list.window[win_id].len_y)	return 0;

	return 1;
}

int	click_in_window(int win_id, int x, int y, Uint32 flags)
{
    window_info *win;
    int	mx, my;
   	widget_list *W; 

	if(win_id <=0 || win_id >= windows_list.num_windows)	return -1;
	if(windows_list.window[win_id].window_id != win_id)	return -1;
	win= &windows_list.window[win_id];
	W = &win->widgetlist;
	if(mouse_in_window(win_id, x, y) > 0)
		{
			// watch for needing to convert the globals into the flags
			// TODO: put this in the window manager
			if(!flags){
				if(shift_on)	flags |= ELW_SHIFT;
				if(ctrl_on)		flags |= ELW_CTRL;
				if(alt_on)		flags |= ELW_ALT;
				if(right_click)	flags |= ELW_RIGHT_MOUSE;
				//if(mid_click)	flags |= ELW_MID_MOUSE;
				if(left_click)	flags |= ELW_LEFT_MOUSE;
				//if(double_click)	flags |= ELW_DBL_CLICK;
			}
			mx= x - win->cur_x;
			my= y - win->cur_y;
			//check the X for close - but hide it
			if(win->flags&ELW_CLOSE_BOX)
				{
        			if(my>0 && my<=20 && mx>(win->len_x-20) && mx<=win->len_x)
						{
							// the X was hit, hide this window
							hide_window(win_id);
							return 1;
						}				
				}
			
			// widgets
			glPushMatrix();
			glTranslatef((float)win->cur_x, (float)win->cur_y, 0.0f);
			while(W->next != NULL){
				W = W->next;
				if(mx>W->pos_x && mx<=(W->pos_x+W->len_x) && my>W->pos_y && my<=(W->pos_y+W->len_y)){
					if(W->OnClick != NULL)
					{
						W->OnClick(W,mx - W->pos_x, my - W->pos_y);
					}
				}
			}
			glPopMatrix();

			//use the handler
			if(win->click_handler != NULL){
			    int	ret_val;
			    
				glPushMatrix();
				glTranslatef((float)win->cur_x, (float)win->cur_y, 0.0f);
				ret_val= (*win->click_handler)(win, mx, my, flags);
				glPopMatrix();

				//return	ret_val;	// with click-thru
				return	1;	// no click-thru permitted
			} else {
				return 1;
			}
		}

	return 0;
}


int	drag_in_window(int win_id, int x, int y, Uint32 flags, int dx, int dy)
{
    window_info *win;
    int	mx, my;
    widget_list *W; 

	if(win_id <=0 || win_id >= windows_list.num_windows)	return -1;
	if(windows_list.window[win_id].window_id != win_id)	return -1;
	win= &windows_list.window[win_id];
	W = &win->widgetlist;
	if(win->drag_in || mouse_in_window(win_id, x, y) > 0)
		{
			//use the handler
			if(win->drag_handler != NULL){
			    int	ret_val;

				// watch for needing to convert the globals into the flags
				// TODO: put this in the window manager
				if(!flags){
					if(shift_on)	flags |= ELW_SHIFT;
					if(ctrl_on)		flags |= ELW_CTRL;
					if(alt_on)		flags |= ELW_ALT;
					if(right_click)	flags |= ELW_RIGHT_MOUSE;
					//if(mid_click)	flags |= ELW_MID_MOUSE;
					if(left_click)	flags |= ELW_LEFT_MOUSE;
					//if(double_click)	flags |= ELW_DBL_CLICK;
				}
				mx= x - win->cur_x;
				my= y - win->cur_y;
			    
				// widgets
				glPushMatrix();
				glTranslatef((float)win->cur_x, (float)win->cur_y, 0.0f);
				while(W->next != NULL){
					W = W->next;
					if(mx>W->pos_x && mx<=(W->pos_x+W->len_x) && my>W->pos_y && my<=(W->pos_y+W->len_y)){
						if(W->OnDrag != NULL)
						{
							W->OnDrag(W,dx, dy);
						}
					}
				}
				glPopMatrix();

				glPushMatrix();
				glTranslatef((float)win->cur_x, (float)win->cur_y, 0.0f);
				ret_val= (*win->drag_handler)(win, mx, my, flags, dx, dy);
				glPopMatrix();

				return	1;	// drag has been processed
			}
		}

	return 0;
}

int	mouseover_window(int win_id, int x, int y)
{
	int	mx,	my;
	int	ret_val=0;
	widget_list *W = &windows_list.window[win_id].widgetlist;

	if(mouse_in_window(win_id, x, y) > 0)
		{
			mx= x - windows_list.window[win_id].cur_x;
			my= y - windows_list.window[win_id].cur_y;

			// widgets
			glPushMatrix();
			glTranslatef((float)windows_list.window[win_id].cur_x, (float)windows_list.window[win_id].cur_y, 0.0f);
			while(W->next != NULL){
				W = W->next;
				if(mx>W->pos_x && mx<=(W->pos_x+W->len_x) && my>W->pos_y && my<=(W->pos_y+W->len_y)){
					if(W->OnMouseover != NULL)
						W->OnMouseover(W,mx,my);
				}

				
			}
			glPopMatrix();

			//use the handler if present
			if(windows_list.window[win_id].mouseover_handler){

				glPushMatrix();
				glTranslatef((float)windows_list.window[win_id].cur_x, (float)windows_list.window[win_id].cur_y, 0.0f);
				ret_val= (*windows_list.window[win_id].mouseover_handler)(&windows_list.window[win_id], mx, my);
				glPopMatrix();

			} 
#ifdef	ELC
			if(!ret_val) {
				elwin_mouse = CURSOR_ARROW;
			}
#endif	//ELC
			return 1;
		}

	return 0;
}

void	*set_window_handler(int win_id, int handler_id, int (*handler)() )
{
	void	*old_handler;

	if(win_id <=0 || win_id >= windows_list.num_windows)	return NULL;
	if(windows_list.window[win_id].window_id != win_id)	return NULL;

	// save the information
	switch(handler_id){
		case	ELW_HANDLER_INIT:
			old_handler= (void *)windows_list.window[win_id].init_handler;
			windows_list.window[win_id].init_handler=handler;
			break;
		case	ELW_HANDLER_DISPLAY:
			old_handler= (void *)windows_list.window[win_id].display_handler;
			windows_list.window[win_id].display_handler=handler;
			break;
		case	ELW_HANDLER_CLICK:
			old_handler= (void *)windows_list.window[win_id].click_handler;
			windows_list.window[win_id].click_handler=handler;
			break;
		case	ELW_HANDLER_DRAG:
			old_handler= (void *)windows_list.window[win_id].drag_handler;
			windows_list.window[win_id].drag_handler=handler;
			break;
		case	ELW_HANDLER_MOUSEOVER:
			old_handler= (void *)windows_list.window[win_id].mouseover_handler;
			windows_list.window[win_id].mouseover_handler=handler;
			break;
		default:
			old_handler=NULL;
	}

	return old_handler;
}


void	*get_window_handler(int win_id, int handler_id)
{
	void	*old_handler;

	// get the information
	switch(handler_id){
		case	ELW_HANDLER_INIT:
			old_handler= (void *)windows_list.window[win_id].init_handler;
			break;
		case	ELW_HANDLER_DISPLAY:
			old_handler= (void *)windows_list.window[win_id].display_handler;
			break;
		case	ELW_HANDLER_CLICK:
			old_handler= (void *)windows_list.window[win_id].click_handler;
			break;
		case	ELW_HANDLER_DRAG:
			old_handler= (void *)windows_list.window[win_id].drag_handler;
			break;
		case	ELW_HANDLER_MOUSEOVER:
			old_handler= (void *)windows_list.window[win_id].mouseover_handler;
			break;
		default:
			old_handler=NULL;
	}

	return old_handler;
}


int	set_window_color(int win_id, Uint32 color_id, float r, float g, float b, float a)
{
	if(win_id <=0 || win_id >= windows_list.num_windows)	return 0;
	if(windows_list.window[win_id].window_id != win_id)	return 0;

	// save the information
	switch(color_id){
		case	ELW_COLOR_BACK:
			windows_list.window[win_id].back_color[0]= r;
			windows_list.window[win_id].back_color[1]= g;
			windows_list.window[win_id].back_color[2]= b;
			windows_list.window[win_id].back_color[3]= a;
			return	1;

		case	ELW_COLOR_BORDER:
			windows_list.window[win_id].border_color[0]= r;
			windows_list.window[win_id].border_color[1]= g;
			windows_list.window[win_id].border_color[2]= b;
			windows_list.window[win_id].border_color[3]= a;
			return	1;

		case	ELW_COLOR_LINE:
			windows_list.window[win_id].line_color[0]= r;
			windows_list.window[win_id].line_color[1]= g;
			windows_list.window[win_id].line_color[2]= b;
			windows_list.window[win_id].line_color[3]= a;
			return	1;
	}
	return 0;
}

int		use_window_color(int win_id, Uint32 color_id)
{
	if(win_id <=0 || win_id >= windows_list.num_windows)	return 0;
	if(windows_list.window[win_id].window_id != win_id)	return 0;

	// save the information
	switch(color_id){
		case	ELW_COLOR_BACK:
			glColor4f(windows_list.window[win_id].back_color[0], windows_list.window[win_id].back_color[1], windows_list.window[win_id].back_color[2], windows_list.window[win_id].back_color[3]);
			return	1;

		case	ELW_COLOR_BORDER:
			glColor3f(windows_list.window[win_id].border_color[0], windows_list.window[win_id].border_color[1], windows_list.window[win_id].border_color[2]);
			return	1;

		case	ELW_COLOR_LINE:
			glColor3f(windows_list.window[win_id].line_color[0], windows_list.window[win_id].line_color[1], windows_list.window[win_id].line_color[2]);
			return	1;
	}
	return 0;
}


#include <stdlib.h>
#include <string.h>
#include "elconfig.h"
#include "context_menu.h"
#include "hud.h"
#include "init.h"
#include "translate.h"
#include "elwindows.h"
#include "alphamap.h"
#include "asc.h"
#include "cursors.h"
#include "gl_init.h"
#include "global.h"
#include "interface.h"
#include "keys.h"
#include "misc.h"
#include "multiplayer.h"
#include "storage.h"
#include "textures.h"
#include "trade.h"
#include "widgets.h"
#include "sound.h"

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * int find_window(const char*);
 * void* get_window_handler(int, int);
 */

#define ELW_WIN_MAX 128

windows_info	windows_list;	// the master list of windows

static window_info *cur_drag_window = NULL;
static widget_list *cur_drag_widget = NULL;
int top_SWITCHABLE_OPAQUE_window_drawn = -1;
int opaque_window_backgrounds = 0;
static int last_opaque_window_backgrounds = 0;

int display_window(int win_id);
int	drag_in_window(int win_id, int x, int y, Uint32 flags, int dx, int dy);
int	mouseover_window(int win_id, int x, int y);	// do mouseover processing for a window
int	keypress_in_window(int win_id, int x, int y, Uint32 key, Uint32 unikey);	// keypress in the window

/*
 * The intent of the windows system is to create the window once
 * and then hide the window when you don't wantto use it.
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
	while (1)
	{
		next_id = -9999;
		for (i=0; i < windows_list.num_windows; i++)
		{
			// only look at displayed windows
			if (windows_list.window[i].displayed > 0)
			{
				// at this level?
				if (windows_list.window[i].order == id)
				{
					display_window(i);
				} else if (windows_list.window[i].order < id && windows_list.window[i].order > next_id)
				{
					// try to find the next level
					next_id = windows_list.window[i].order;
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
	
	top_SWITCHABLE_OPAQUE_window_drawn = -1;
	if(level > 0)
	{
		// now display each window in the proper order
		id = 0;
		while (1)
		{
			next_id = 9999;
			for (i = 0; i < windows_list.num_windows; i++)
			{
				// change the opaque option for all ELW_SWITCHABLE_OPAQUE if config has changed
				if ((windows_list.window[i].flags&ELW_SWITCHABLE_OPAQUE) &&
				    (last_opaque_window_backgrounds != opaque_window_backgrounds))
				{
					windows_list.window[i].opaque = opaque_window_backgrounds;
				}
				// only look at displayed windows
				if (windows_list.window[i].displayed > 0)
				{
					// at this level?
					if (windows_list.window[i].order == id)
					{
						display_window(i);
						// remember the top window that has ELW_SWITCHABLE_OPAQUE
						if (windows_list.window[i].flags&ELW_SWITCHABLE_OPAQUE)
							top_SWITCHABLE_OPAQUE_window_drawn = i;
					} 
					else if (windows_list.window[i].order > id && windows_list.window[i].order < next_id)
					{
						// try to find the next level
						next_id = windows_list.window[i].order;
					}
				}
			}
			if (next_id >= 9999)
			{
				break;
			}
			else
			{
				id = next_id;
			}
		}
	}
	last_opaque_window_backgrounds = opaque_window_backgrounds;
}


int	click_in_windows(int mx, int my, Uint32 flags)
{
	int	done= 0;
	int	id;
	int	next_id;
	int	first_win= -1;
	int i;
	
	/* only activate context menu on unmodified right click */
	int cm_try_activate = cm_pre_show_check(flags);

	// check each window in the proper order
	if(windows_list.display_level > 0)
	{
		id= 9999;
		while(done <= 0)
		{
			next_id= 0;
			for (i=0; i<windows_list.num_windows; i++)
			{
				// only look at displayed windows
				if (windows_list.window[i].displayed > 0)
				{
					// at this level?
					if(windows_list.window[i].order == id)
					{
						if (cm_try_activate && cm_show_if_active(i))
							return 0;
						done= click_in_window(i, mx, my, flags);
						if(done > 0)
						{
							if(windows_list.window[i].displayed > 0)	select_window(i);	// select this window to the front
							cm_post_show_check(0);
							return i;
						}
						if(first_win < 0 && mouse_in_window(i, mx, my))	first_win= i;
					}
					else if(windows_list.window[i].order < id && windows_list.window[i].order > next_id)
					{
						// try to find the next level
						next_id= windows_list.window[i].order;
					}
				}
			}
			if(next_id <= 0)
				break;
			else
				id= next_id;
		}
	}
	
	// now check the background windows in the proper order
	id= -9999;
	while(done <= 0)
	{
		next_id= 0;
		for(i=0; i<windows_list.num_windows; i++)
		{
			// only look at displayed windows
			if(windows_list.window[i].displayed > 0)
			{
				// at this level?
				if(windows_list.window[i].order == id)
				{
					if (cm_try_activate && cm_show_if_active(i))
						return 0;
					done= click_in_window(i, mx, my, flags);
					if(done > 0)
					{
						//select_window(i);	// these never get selected
						cm_post_show_check(0);
						return i;
					}
				} 
				else if(windows_list.window[i].order > id && windows_list.window[i].order < next_id)
				{
					// try to find the next level
					next_id= windows_list.window[i].order;
				}
			}
		}
		if(next_id >= 0)
			break;
		else
			id= next_id;
	}
	
	cm_post_show_check(0);

	// nothing to click on, do a select instead
	if(first_win >= 0)
	{
		select_window(first_win);
		return first_win;
	}

	return -1;	// no click in a window
}

int	drag_in_windows(int mx, int my, Uint32 flags, int dx, int dy)
{
	int	done = 0;
	int	id;
	int	next_id;
	int i;
	window_info *win;

	// ignore a drag of 0, but say we processed
	if(dx == 0 && dy == 0)	return -1;
	
	if (cur_drag_window)
	{
		// a drag was started from cur_drag_window, let that window 
		// handle it, regardless of where the cursor is
		
		done = drag_in_window (cur_drag_window->window_id, mx, my, flags, dx, dy);
		if (done > 0)
		{
			if (cur_drag_window->displayed)
				// select this window to the front
				select_window (cur_drag_window->window_id); 
			return cur_drag_window->window_id;
		}
		else
		{
			// The original window didn't handle the drag, reset it
			// and continue
			cur_drag_window = NULL;
		}
	}

	// check each window in the proper order
	if(windows_list.display_level > 0)
	{
		id= 9999;
		while (done <= 0)
		{
			next_id= 0;
			for(i=0; i < windows_list.num_windows; i++)
			{
				win = &(windows_list.window[i]);				
				// only look at displayed windows
				if (win->displayed)
				{
					// at this level?
					if(win->order == id)
					{
						done = drag_in_window (i, mx, my, flags, dx, dy);
						if (done > 0)
						{
							if (win->displayed)
								select_window(i);	// select this window to the front
							cur_drag_window = win;
							return i;
						}
						else if (mouse_in_window (i, mx, my))
						{
							// drag started in this window
							return -1;
						}
					} 
					else if (win->order < id && win->order > next_id)
					{
						// try to find the next level
						next_id= win->order;
					}
				}
			}
			if(next_id <= 0)
				break;
			else
				id= next_id;
		}
	}
	
	// now check the background windows in the proper order
	id= -9999;
	while (done <= 0)
	{
		next_id= 0;
		for (i=0; i<windows_list.num_windows; i++)
		{
			win = &(windows_list.window[i]);
			// only look at displayed windows
			if(win->displayed)
			{
				// at this level?
				if(win->order == id)
				{
					done = drag_in_window (i, mx, my, flags, dx, dy);
					if(done > 0)
					{
						//select_window(i);	// these never get selected
						cur_drag_window = win;
						return i;
					}
					else if (mouse_in_window (i, mx, my))
					{
						// drag started in this window
						return -1;
					}
				} 
				else if (win->order > id && win->order < next_id)
				{
					// try to find the next level
					next_id = win->order;
				}
			}
		}
		if(next_id >= 0)
			break;
		else
			id= next_id;
	}

	return -1;	// no drag in a window
}


int drag_windows (int mx, int my, int dx, int dy)
{
	int	next_id;
	int	id, i;
	int	drag_id = -1;
	int dragable, resizeable;
	int x, y;
	window_info *win;
	
	if (cur_drag_window)
	{
		// We are currently dragging inside another window, don't
		// interrupt that by moving another window around
		return -1;
	}

	// check each window in the proper order for which one might be getting dragged
	if(windows_list.display_level > 0)
	{
		id= 9999;
		while(drag_id < 0)
		{
			next_id= 0;
			for(i=0; i<windows_list.num_windows; i++)
			{
				win = &(windows_list.window[i]);
				dragable = win->flags & ELW_DRAGGABLE;
				resizeable = win->flags & ELW_RESIZEABLE;
				// only look at displayed windows
				if (win->displayed && (dragable || resizeable) )
				{
					// at this level?
					if(win->order == id)
					{
						// position relative to window
						x = mx - win->cur_x;
						y = my - win->cur_y;
						
						// first check for being actively dragging or on the top bar
						if (win->dragged || (dragable && mouse_in_window(i, mx, my) && ((y < 0) || (win->owner_drawn_title_bar && y < ELW_TITLE_HEIGHT))) )
						{
							drag_id = i;
							win->dragged = 1;
							break;
						} 
						// check if we are resizing this window
						else if (win->resized || (resizeable && mouse_in_window(i, mx, my) && x > win->len_x - ELW_BOX_SIZE && y > win->len_y - ELW_BOX_SIZE) )
						{
							drag_id = i;
							win->resized = 1;
							break;
						}
						// check if we're dragging inside a window 
						else if(mouse_in_window(i, mx, my))
						{
							return -1;
						}
					} 
					else if (win->order < id && win->order > next_id)
					{
						// try to find the next level
						next_id = win->order;
					}
				}
			}
			if(next_id <= 0)
				break;
			else
				id= next_id;
		}
	}

	// this section probably won't be needed, included to be complete
	// now check the background windows in the proper order for which one might be getting dragged
	id = -9999;
	while (drag_id < 0)
	{
		next_id = 0;
		for (i=0; i<windows_list.num_windows; i++)
		{
			win = &(windows_list.window[i]);
			dragable = win->flags & ELW_DRAGGABLE;
			resizeable = win->flags & ELW_RESIZEABLE;
			// only look at displayed windows
			if (win->displayed && (dragable || resizeable) )
			{
				// at this level?
				if(win->order == id)
				{
					// position relative to window
					x = mx - win->cur_x;
					y = my - win->cur_y;

					// check for being actively dragging or on the top bar
					if(win->dragged || (mouse_in_window(i, mx, my) && y < 0) )
					{
						drag_id = i;
						win->dragged = 1;
						break;
					} 
					// check if we are resizing this window
					else if (resizeable && mouse_in_window(i, mx, my) && x > win->len_x - ELW_BOX_SIZE && y > win->len_y - ELW_BOX_SIZE) 
					{
						drag_id = i;
						win->resized = 1;
						break;
					} 
					// check if we're dragging inside a window 
					else if (mouse_in_window(i, mx, my))
					{
						return -1;
					}
				} 
				else if (win->order > id && win->order < next_id){
					// try to find the next level
					next_id = win->order;
				}
			}
		}
		if (next_id >= 0)
			break;
		else
			id= next_id;
	}

	// are we dragging a window?
	if (drag_id < 0) return -1;

	// dragged window is always on top
	select_window (drag_id);
	
	if(left_click>1 && (dx != 0 || dy != 0))	// TODO: avoid globals?
	{
		win = &(windows_list.window[drag_id]);
		if (win->dragged)
			// move to new location
			move_window (drag_id, win->pos_id, win->pos_loc, win->pos_x+dx, win->pos_y+dy);
		else
			// resize this window
			resize_window (drag_id, win->len_x + dx, win->len_y + dy); 
	}

	return drag_id;
}

int	keypress_in_windows(int x, int y, Uint32 key, Uint32 unikey)
{
	int	done= 0;
	int	id;
	int	next_id;
	int i;

	// check each window in the proper order
	if(windows_list.display_level > 0)
	{
		id= 9999;
		while(done <= 0)
		{
			next_id= 0;
			for (i=0; i<windows_list.num_windows; i++)
			{
				// only look at displayed windows
				if (windows_list.window[i].displayed > 0)
				{
					// at this level?
					if(windows_list.window[i].order == id)
					{
						done = keypress_in_window (i, x, y, key, unikey);
						if(done > 0)
						{
							if (windows_list.window[i].displayed > 0)
								select_window(i);	// select this window to the front
							return i;
						}
					}
					else if(windows_list.window[i].order < id && windows_list.window[i].order > next_id)
					{
						// try to find the next level
						next_id = windows_list.window[i].order;
					}
				}
			}
			if(next_id <= 0)
				break;
			else
				id= next_id;
		}
	}
	
	// now check the background windows in the proper order
	id= -9999;
	while(done <= 0)
	{
		next_id= 0;
		for(i=0; i<windows_list.num_windows; i++)
		{
			// only look at displayed windows
			if(windows_list.window[i].displayed > 0)
			{
				// at this level?
				if(windows_list.window[i].order == id)
				{
					done = keypress_in_window(i, x, y, key, unikey);
					if(done > 0)
					{
						//select_window(i);	// these never get selected
						return i;
					}
				} 
				else if(windows_list.window[i].order > id && windows_list.window[i].order < next_id)
				{
					// try to find the next level
					next_id= windows_list.window[i].order;
				}
			}
		}
		if(next_id >= 0)
			break;
		else
			id= next_id;
	}
	
	return -1;	// no keypress in a window
}

void	end_drag_windows()
{
	int	i;

	for (i=0; i<windows_list.num_windows; i++)
	{
		windows_list.window[i].dragged= 0;
		windows_list.window[i].resized= 0;
		windows_list.window[i].drag_in= 0;
	}
	
	// also reset the window we were dragging in, and the dragged widget
	cur_drag_window = NULL;
	cur_drag_widget = NULL;
}


int	select_window_with (int win_id, int raise_parent, int raise_children)
{
	int	i, old;
	
	if (win_id < 0 || win_id >= windows_list.num_windows)	return -1;
	if (windows_list.window[win_id].window_id != win_id)	return -1;
	// never select background windows
	if (windows_list.window[win_id].order < 0)		return 0;
	
	// if this is a child window, raise the parent first
	if (raise_parent && windows_list.window[win_id].pos_id >= 0)
		select_window_with (windows_list.window[win_id].pos_id, 1, 0);

	// shuffle the order of the windows
	old = windows_list.window[win_id].order;
	for (i=0; i<windows_list.num_windows; i++)
	{
		if(windows_list.window[i].order > old)
			windows_list.window[i].order--;
	}
	
	// and put it on top
	windows_list.window[win_id].order = windows_list.num_windows;
	
	// now raise all children
	if (raise_children)
	{
		for (i=0; i<windows_list.num_windows; i++)
		{
			if (windows_list.window[i].pos_id == win_id)
				select_window_with (i, 0, 1);
		}
	}

	return 1;
}

int	select_window (int win_id)
{
	return select_window_with (win_id, 1, 1);
}


int cm_title_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	extern void hide_all_windows();
	switch (option)
	{
		case 0: hide_all_windows(); break;
		case 1: break; // make sure the sound is sucess. 
	}
	return 1;
}


// specific windows functions
int	create_window(const char *name, int pos_id, Uint32 pos_loc, int pos_x, int pos_y, int size_x, int size_y, Uint32 property_flags)
{
	window_info *win;
	int	win_id=-1;
	int	i;
	int isold = 1;
	
	// verify that we are setup and space allocated
	if (windows_list.window == NULL)
	{
		// allocate the space
		windows_list.num_windows = 0;
		windows_list.max_windows = ELW_WIN_MAX;
		windows_list.window=(window_info *) calloc(ELW_WIN_MAX, sizeof(window_info));
		//windows_list.window[0].window_id = -1;	// force a rebuild of this
		//windows_list.num_windows = 1;
	}

	// find an empty slot
	for (i=1; i<windows_list.num_windows; i++)
	{
		if (windows_list.window[i].window_id < 0)
		{
			win_id = i;
			break;
		}
	}

	// need a new_entry?
	if (win_id < 0)
	{
		isold = 0;
		if (windows_list.num_windows < windows_list.max_windows - 1)
		{
			win_id = windows_list.num_windows++;
		}
	}

	// fill in the information
	if (win_id >= 0)
	{
		win = &windows_list.window[win_id];
		win->window_id = win_id;

		win->flags = property_flags;
		//win->collapsed = 0;
		win->dragged = 0;
		win->resized = 0;
		win->drag_in = 0;
		win->opaque = opaque_window_backgrounds;
		win->owner_drawn_title_bar = 0;
		if (win->flags&ELW_TITLE_BAR)
		{
			win->cm_id = cm_create(cm_title_menu_str, cm_title_handler);
			if (win->flags&ELW_SWITCHABLE_OPAQUE)
				cm_bool_line(win->cm_id, 1, &win->opaque, NULL);
			else
				cm_grey_line(win->cm_id, 1, 1);
			cm_bool_line(win->cm_id, 2, &windows_on_top, "windows_on_top");
		}
		else
			win->cm_id = CM_INIT_VALUE;
		my_strncp(win->window_name, name, sizeof (win->window_name));
		
		if (pos_id >= 0 && !windows_list.window[pos_id].displayed)
		{
			// parent is hidden
			win->reinstate = (property_flags & ELW_SHOW) ? 1 : 0;
			win->displayed = 0;
		}
		else
		{
			// no parent, or parent is shown
			win->displayed = (property_flags & ELW_SHOW) ? 1 : 0;
			win->reinstate = 0;
		}

		win->back_color[0] = 0.0f;
		win->back_color[1] = 0.0f;
		win->back_color[2] = 0.0f;
		win->back_color[3] = 0.4f;
		win->border_color[0] = 0.77f;
		win->border_color[1] = 0.57f;
		win->border_color[2] = 0.39f;
		win->border_color[3] = 0.0f;
		win->line_color[0] = 0.77f;
		win->line_color[1] = 0.57f;
		win->line_color[2] = 0.39f;
		win->line_color[3] = 0.0f;

		win->init_handler = NULL;
		win->display_handler = NULL;
		win->pre_display_handler = NULL;
		win->click_handler = NULL;
		win->drag_handler = NULL;
		win->mouseover_handler = NULL;
		win->resize_handler = NULL;
		win->keypress_handler = NULL;
		win->close_handler = NULL;
		win->destroy_handler = NULL;
		win->show_handler = NULL;
		win->after_show_handler = NULL;
		win->hide_handler = NULL;
		
		win->widgetlist = NULL;
		
		// now call the routine to place it properly
		init_window (win_id, pos_id, pos_loc, pos_x, pos_y, size_x, size_y);

		win->order = (property_flags & ELW_SHOW_LAST) ? -win_id-1 : win_id+1;
		// make sure the order is unique if this is not a background 
		// window
		if (isold && win->order > 0)
		{
			// determine the highest unused order
			int order = windows_list.num_windows;
			
			while (1)
			{
				for (i = 0; i < windows_list.num_windows; i++)
				{
					if (windows_list.window[i].order == order)
						break;
				}
				if (i < windows_list.num_windows)
					order--;
				else
					break;
			}
			
			win->order = order;
			// select the window to the foreground
			select_window (win_id);
		}
	}

	return	win_id;
}


void	destroy_window(int win_id)
{
	window_info *win;

	if(win_id < 0 || win_id >= windows_list.num_windows)	return;
	if(windows_list.window[win_id].window_id != win_id)	return;
	// mark the window as unused
	
	win = &(windows_list.window[win_id]);

	if (cm_valid(win->cm_id))
	{
		cm_destroy(win->cm_id);
		win->cm_id = CM_INIT_VALUE;
	}

	// call destruction handler        
	if (win->destroy_handler != NULL)
		win->destroy_handler (win);

	// destroy our widgets
	while (win->widgetlist != NULL)
	{
	    widget_destroy(win_id, win->widgetlist->id);
	}
	win->widgetlist = NULL;
	
	win->window_id = -1;
	win->order = -1;
	win->displayed = 0;
}

int	init_window(int win_id, int pos_id, Uint32 pos_loc, int pos_x, int pos_y, int size_x, int size_y)
{
	int pwin_x, pwin_y;
	
	if(win_id < 0 || win_id >= windows_list.num_windows)	return -1;
	if(windows_list.window[win_id].window_id != win_id)	return -1;
	if (pos_id >= 0)
	{
		if (pos_id >= windows_list.num_windows)			return -1;
		if (windows_list.window[pos_id].window_id != pos_id)	return -1;
	}
			
	// parent window position. The new window is placed relative to these
	// coordinates. If pos_id < 0, the values are taken to be absolute
	// (i.e. relative to (0, 0) )
	pwin_x = pos_id >= 0 ? windows_list.window[pos_id].cur_x : 0;
	pwin_y = pos_id >= 0 ? windows_list.window[pos_id].cur_y : 0;

	// memorize the size
	windows_list.window[win_id].len_x= size_x;
	windows_list.window[win_id].len_y= size_y;
	windows_list.window[win_id].orig_len_x= size_x; //for self-resizing windows
	windows_list.window[win_id].orig_len_y= size_y; //for self-resizing windows
	// initialize min_len_x and min_len_y to zero. 
	windows_list.window[win_id].min_len_x= 0;
	windows_list.window[win_id].min_len_y= 0;
	// then place the window
	move_window(win_id, pos_id, pos_loc, pos_x+pwin_x, pos_y+pwin_y);

	if(windows_list.window[win_id].flags&ELW_SCROLLABLE) {
		/* Add the scroll widget */
		Uint16 x = size_x-ELW_BOX_SIZE,
				y = size_y,
				width = ELW_BOX_SIZE,
				height = size_y;
		
		if(windows_list.window[win_id].flags&ELW_CLOSE_BOX) {
			/* Don't put the scrollbar behind the close box. */
			y += ELW_BOX_SIZE;
			height -= ELW_BOX_SIZE;
		}
		if(windows_list.window[win_id].flags&ELW_RESIZEABLE) {
			/* Don't put the scrollbar behind the resize box. */
			height -= ELW_BOX_SIZE;
		}
		windows_list.window[win_id].scroll_id = vscrollbar_add(win_id, NULL, x, y, width, height);
		windows_list.window[win_id].scroll_yoffset = 0;
		widget_set_color(win_id, windows_list.window[win_id].scroll_id,
						windows_list.window[win_id].border_color[0],
						windows_list.window[win_id].border_color[1],
						windows_list.window[win_id].border_color[2]);
	}
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
	int dx, dy, i;

	if(win_id < 0 || win_id >= windows_list.num_windows)
		return -1;
	if(windows_list.window[win_id].window_id != win_id)
		return -1;
	
	win= &windows_list.window[win_id];

	dx = -win->cur_x;
	dy = -win->cur_y;

	win->pos_id = pos_id;
	win->pos_loc= pos_loc;	//NOT SUPPORTED YET
	//TODO: calc win->cur_[xy] based on pos_id & pos_loc
	if(win->flags&ELW_TITLE_BAR || win->owner_drawn_title_bar){
		int xbound = (win->len_x < 50) ? win->len_x : 50;
		int ybound = (win->len_y < 50) ? win->len_y : 50;
		win->pos_x = win->cur_x = clampi(pos_x, xbound - win->len_x, window_width - xbound);
		if(win->owner_drawn_title_bar)
			win->pos_y = win->cur_y = clampi(pos_y, 0, window_height - ybound);
		else
			win->pos_y = win->cur_y = clampi(pos_y, ELW_TITLE_HEIGHT, window_height - ybound);
	} else {
		win->pos_x = win->cur_x = pos_x;
		win->pos_y = win->cur_y = pos_y;
	}

	// don't check child windows for visibility
	if (pos_id < 0 || windows_list.window[pos_id].order < 0) {
		// check for the window actually being on the screen, if not, move it
		if(win->cur_y < ((win->flags&ELW_TITLE_BAR)?ELW_TITLE_HEIGHT:0))
			win->cur_y= (win->flags&ELW_TITLE_BAR)?ELW_TITLE_HEIGHT:0;
		if(win->cur_y >= window_height)
			win->cur_y= window_height;	// had -32, but do we want that?
		if(win->cur_x+win->len_x < ELW_BOX_SIZE)
			win->cur_x= 0-win->len_x+ELW_BOX_SIZE;
		if(win->cur_x > window_width-ELW_BOX_SIZE)
			win->cur_x= window_width-ELW_BOX_SIZE;
	}

	// move child windows, if any
	dx += win->cur_x;
	dy += win->cur_y;
	for (i = 0; i < windows_list.num_windows; i++) {
		if (windows_list.window[i].pos_id == win_id) {
			move_window (i, win_id, 0, windows_list.window[i].cur_x + dx, windows_list.window[i].cur_y + dy);
		}
	}

	return 1;
}

int	draw_window_title(window_info *win)
{
#ifdef	NEW_TEXTURES
	float u_first_start = (float)31/255;
	float u_first_end = 0;
	float v_first_start = (float)160/255;
	float v_first_end = (float)175/255;

	float u_middle_start = (float)32/255;
	float u_middle_end = (float)63/255;
	float v_middle_start = (float)160/255;
	float v_middle_end = (float)175/255;

	float u_last_start = 0;
	float u_last_end = (float)31/255;
	float v_last_start = (float)160/255;
	float v_last_end = (float)175/255;
#else	/* NEW_TEXTURES */
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
#endif	/* NEW_TEXTURES */

	if((win->flags&ELW_TITLE_BAR) == ELW_TITLE_NONE)	return 0;

	/* draw the help text if the mouse is over the title bar */
	if (show_help_text && cm_valid(win->cm_id) && (cm_window_shown() == CM_INIT_VALUE) &&
		mouse_x > win->cur_x && mouse_x < win->cur_x+win->len_x &&
		mouse_y > win->cur_y-ELW_TITLE_HEIGHT && mouse_y < win->cur_y)
		show_help(cm_title_help_str, 0, win->len_y+10);
	
	glColor3f(1.0f,1.0f,1.0f);
	//ok, now draw that shit...

#ifdef	NEW_TEXTURES
	bind_texture(icons_text);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(icons_text);
#endif	/* NEW_TEXTURES */
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0.03f);
	glBegin(GL_QUADS);

	if (win->len_x > 64) 
	{
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
	}
	else
	{
		glTexCoord2f(u_first_end, v_first_start);
		glVertex3i(0, -ELW_TITLE_HEIGHT, 0);
		glTexCoord2f(u_first_end, v_first_end);
		glVertex3i(0, 0, 0);
		glTexCoord2f(u_first_start, v_first_end);
		glVertex3i(win->len_x / 2, 0, 0);
		glTexCoord2f(u_first_start, v_first_start);
		glVertex3i(win->len_x / 2, -ELW_TITLE_HEIGHT, 0);

		glTexCoord2f(u_middle_end, v_middle_start);
		glVertex3i(win->len_x / 2, -ELW_TITLE_HEIGHT, 0);
		glTexCoord2f(u_middle_end, v_middle_end);
		glVertex3i(win->len_x / 2, 0, 0);
		glTexCoord2f(u_middle_start, v_middle_end);
		glVertex3i(win->len_x / 2 + 1, 0, 0);
		glTexCoord2f(u_middle_start, v_middle_start);
		glVertex3i(win->len_x / 2 + 1, -ELW_TITLE_HEIGHT, 0);

		glTexCoord2f(u_last_end, v_last_start);
		glVertex3i(win->len_x / 2 + 1, -ELW_TITLE_HEIGHT, 0);
		glTexCoord2f(u_last_end, v_last_end);
		glVertex3i(win->len_x / 2 + 1, 0, 0);
		glTexCoord2f(u_last_start, v_last_end);
		glVertex3i(win->len_x, 0, 0);
		glTexCoord2f(u_last_start, v_last_start);
		glVertex3i(win->len_x, -ELW_TITLE_HEIGHT, 0);
	}

	glEnd();
	glDisable(GL_ALPHA_TEST);

	// draw the name of the window
	if(win->flags&ELW_TITLE_NAME)
		{
			int	len=(get_string_width((unsigned char*)win->window_name)*8)/12;
			int	x_pos=(win->len_x-len)/2;

			glColor4f(0.0f,0.0f,0.0f,1.0f);
			glBegin(GL_QUADS);
				glVertex3i(x_pos, -ELW_TITLE_HEIGHT, 0);
				glVertex3i(x_pos+len, -ELW_TITLE_HEIGHT, 0);
				glVertex3i(x_pos+len, 0, 0);
				glVertex3i(x_pos, 0, 0);
			glEnd();
			glBegin(GL_TRIANGLE_STRIP);
				glVertex3i(x_pos, -ELW_TITLE_HEIGHT, 0);
				glVertex3i(x_pos-10, -ELW_TITLE_HEIGHT/2, 0);
				glVertex3i(x_pos, 0, 0);
			glEnd();
			glBegin(GL_TRIANGLE_STRIP);
				glVertex3i(x_pos+len, -ELW_TITLE_HEIGHT, 0);
				glVertex3i(x_pos+len+10, -ELW_TITLE_HEIGHT/2, 0);
				glVertex3i(x_pos+len, 0, 0);
			glEnd();
			glEnable(GL_TEXTURE_2D);
			glColor3f(win->border_color[0],win->border_color[1],win->border_color[2]);
			// center text
			draw_string_small((win->len_x-len)/2, 1-ELW_TITLE_HEIGHT, (unsigned char*)win->window_name, 1);
		}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}

int	draw_window_border(window_info *win)
{
	glDisable(GL_TEXTURE_2D);
	if(win->flags&ELW_USE_BACKGROUND)
	{
		if (!(win->opaque && win->flags&ELW_SWITCHABLE_OPAQUE))
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_SRC_ALPHA);
		}
		glColor4f(win->back_color[0],win->back_color[1],win->back_color[2],win->back_color[3]);
		glBegin(GL_QUADS);
			glVertex3i(0, win->len_y, 0);
			glVertex3i(0, 0, 0);
			glVertex3i(win->len_x, 0, 0);
			glVertex3i(win->len_x, win->len_y, 0);
		glEnd();
		if (!(win->opaque && win->flags&ELW_SWITCHABLE_OPAQUE))
			glDisable(GL_BLEND);
	}

	if(win->flags&ELW_USE_BORDER)
	{
		if ( use_alpha_border && (win->flags & ELW_ALPHA_BORDER) )
		{
			draw_window_alphaborder (win);
		}
		else 
		{
			glColor3f(win->border_color[0],win->border_color[1],win->border_color[2]);
			glBegin(GL_LINE_LOOP);
				glVertex3i(0, 0, 0);
				glVertex3i(win->len_x, 0, 0);
				glVertex3i(win->len_x, win->len_y, 0);
				glVertex3i(0, win->len_y, 0);
			glEnd();
		}
	}

	if (win->flags&ELW_RESIZEABLE)
	{
		// draw the diagonal drag stripes
		glColor3f(win->border_color[0],win->border_color[1],win->border_color[2]);
		glBegin(GL_LINES);
			glVertex3i(win->len_x-ELW_BOX_SIZE/4, win->len_y, 0);
			glVertex3i(win->len_x, win->len_y-ELW_BOX_SIZE/4, 0);

			glVertex3i(win->len_x-ELW_BOX_SIZE/2, win->len_y, 0);
			glVertex3i(win->len_x, win->len_y-ELW_BOX_SIZE/2, 0);

			glVertex3i(win->len_x-(3*ELW_BOX_SIZE)/4, win->len_y, 0);
			glVertex3i(win->len_x, win->len_y-(3*ELW_BOX_SIZE)/4, 0);

			glVertex3i(win->len_x-ELW_BOX_SIZE, win->len_y, 0);
			glVertex3i(win->len_x, win->len_y-ELW_BOX_SIZE, 0);
		glEnd();
	}
	
	if(win->flags&ELW_CLOSE_BOX)
	{
		//draw the corner, with the X in
		glColor3f(win->border_color[0],win->border_color[1],win->border_color[2]);
		glBegin(GL_LINE_STRIP);
			glVertex3i(win->len_x, ELW_BOX_SIZE, 0);
			glVertex3i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE, 0);
			glVertex3i(win->len_x-ELW_BOX_SIZE, 0, 0);
		glEnd();

		glLineWidth(2.0f);

		glBegin(GL_LINES);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, 3);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE-3);
		
			glVertex2i(win->len_x-3, 3);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE-3);
		glEnd();

		glLineWidth(1.0f);
	}
	
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}

int	draw_window(window_info *win)
{
	int	ret_val=0;
	widget_list *W = NULL;
	
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	if(win == NULL || win->window_id < 0)
		return -1;

	if(!win->displayed)
		return 0;
	// mouse over processing first
	mouseover_window(win->window_id, mouse_x, mouse_y);
	if(win->flags&ELW_TITLE_BAR) {
		/* Only move windows with title bars, otherwise we'll get problems
		 * with tab collections, etc. Windows without title bars
		 * can't be moved anyways.
		 */
		 //if it's too far out of bounds, put it back. you do the bottom bounds first incase the window in question is larger than the game window
		if(win->cur_x + 20 > window_width) {
			move_window(win->window_id, win->pos_id, win->pos_loc, window_width-20, win->pos_y);
		}
		if(win->cur_y + 10 > window_height) {
			move_window(win->window_id, win->pos_id, win->pos_loc, win->pos_x, window_height-10);
		}
		if(win->cur_x + win->len_x < 20) {
			move_window(win->window_id, win->pos_id, win->pos_loc, 20 - win->len_x, win->pos_y);
		}
		if(win->cur_y < ELW_TITLE_HEIGHT) {
			move_window(win->window_id, win->pos_id, win->pos_loc, win->pos_x, ELW_TITLE_HEIGHT);
		}
	}
	// now normal display processing
	glPushMatrix();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glTranslatef((float)win->cur_x, (float)win->cur_y, 0.0f);
	draw_window_title(win);
	draw_window_border(win);
	glColor3f(1.0f, 1.0f, 1.0f);
	if(win->pre_display_handler)
		(*win->pre_display_handler)(win);

	if(win->flags&ELW_SCROLLABLE) {
		int pos = vscrollbar_get_pos(win->window_id, win->scroll_id);
		int offset = win->scroll_yoffset + ((win->flags&ELW_CLOSE_BOX) ? ELW_BOX_SIZE : 0);

		widget_move(win->window_id, win->scroll_id, win->len_x-ELW_BOX_SIZE, pos+offset);
		/* Cut away what we've scrolled past, */
		glEnable(GL_SCISSOR_TEST);
		glScissor(win->cur_x+gx_adjust, window_height-win->cur_y-win->len_y-gy_adjust, win->len_x+1, win->len_y+1);
		glTranslatef(0, -pos, 0);
	}
	if(win->display_handler)
	{
		ret_val=(*win->display_handler)(win);
//the window's own display handler can cause OpenGL errors
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	}
	else
	{
		ret_val=1;
	}
	// assign here in case display_handler changed the widgets - like deletes the widgets
	W = win->widgetlist;
	
	// widget drawing
	while(W != NULL)
	{
		if (!(W->Flags&WIDGET_INVISIBLE) && !(W->Flags&WIDGET_DISABLED)) {
			// Draw defaults, then secondaries
			if (W->type != NULL)
				if (W->type->draw != NULL)
					W->type->draw(W);
			if (W->OnDraw != NULL)
			{
				if(W->spec != NULL)
					W->OnDraw(W, W->spec);
				else
					W->OnDraw(W);
			}
		}
		W = W->next;
	}
	if(win->flags&ELW_SCROLLABLE) {
		glDisable(GL_SCISSOR_TEST);
	}
	glPopMatrix();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return(ret_val);
}

void show_window(int win_id)
{
	int iwin;
	int ipos;
	window_info *win;
	
	if(win_id < 0 || win_id >= windows_list.num_windows)	return;
	if(windows_list.window[win_id].window_id != win_id)	return;

	win = &windows_list.window[win_id];
	if (win->show_handler) (*win->show_handler)(win);
	
	// pull to the top if not currently displayed
	if(!windows_list.window[win_id].displayed)
		select_window(win_id);

	ipos = windows_list.window[win_id].pos_id;
	if (ipos >= 0 && !windows_list.window[ipos].displayed)
	{
		// parent is hidden, simply set the reinstated flag
		windows_list.window[win_id].reinstate = 1;
	}
	else
	{
		// display it
		windows_list.window[win_id].displayed = 1;
	}
	
	// see if child windows need to be reinstated
	for (iwin = 0; iwin < windows_list.num_windows; iwin++)
		if (windows_list.window[iwin].pos_id == win_id && windows_list.window[iwin].reinstate)
			show_window (iwin);
	
	if (win->after_show_handler) (*win->after_show_handler)(win);
}

void	hide_window(int win_id)
{
	int iwin;
	window_info * win;
	
	if(win_id < 0 || win_id >= windows_list.num_windows)	return;
	if(windows_list.window[win_id].window_id != win_id)	return;
	
	win = &windows_list.window[win_id];
	
	win->displayed = 0;
	win->reinstate = 0;
	
	// hide child windows
	for (iwin = 0; iwin < windows_list.num_windows; iwin++)
		if (windows_list.window[iwin].pos_id == win_id && windows_list.window[iwin].displayed)
		{
			hide_window (iwin);
			windows_list.window[iwin].reinstate = 1;
		}

	if (win->hide_handler) (*win->hide_handler)(win);
}

void	toggle_window(int win_id)
{
	if(win_id < 0 || win_id >= windows_list.num_windows)	return;
	if(windows_list.window[win_id].window_id != win_id)	return;

	if(!windows_list.window[win_id].displayed)
		select_window(win_id);
	//windows_list.window[win_id].displayed=!windows_list.window[win_id].displayed;

	// if we hide a window, we have to hide it's children too, so we cannot
	// simply toggle the displayed flag.
	if (windows_list.window[win_id].displayed || windows_list.window[win_id].reinstate)
		hide_window (win_id);
	else
		show_window (win_id);
}

static void resize_scrollbar(window_info *win)
{
	int sblen = win->len_y - win->scroll_yoffset;
	if (win->flags&ELW_CLOSE_BOX)
		sblen -= ELW_BOX_SIZE;
	if (win->flags&ELW_RESIZEABLE)
		sblen -= ELW_BOX_SIZE;
	widget_resize(win->window_id, win->scroll_id,
		widget_get_width(win->window_id, win->scroll_id), sblen);
}

void resize_window (int win_id, int new_width, int new_height)
{
	window_info *win;
		
	if (win_id < 0 || win_id >= windows_list.num_windows)	return;
	if (windows_list.window[win_id].window_id != win_id)	return;
	
	win = &(windows_list.window[win_id]);

	if (new_width < win->min_len_x) new_width = win->min_len_x;
	if (new_height < win->min_len_y) new_height = win->min_len_y;
	
	win->len_x = new_width;
	win->len_y = new_height;
	
	if (win->flags&ELW_SCROLLABLE)
		resize_scrollbar(win);
	
	if (win->resize_handler != NULL)
	{
		glPushMatrix ();
		glTranslatef ((float)win->cur_x, (float)win->cur_y, 0.0f);
		(*win->resize_handler) (win, new_width, new_height);
		glPopMatrix  ();
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

int	get_show_window(int win_id)
{
	// unititialized windows always fail as if not shown, not an error
	if(win_id < 0 || win_id >= windows_list.num_windows)	return 0;
	if(windows_list.window[win_id].window_id != win_id)	return 0;

	return windows_list.window[win_id].displayed;
}

int	get_window_showable(int win_id)
{
	// unititialized windows always fail as if not shown, not an error
	if(win_id < 0 || win_id >= windows_list.num_windows)	return 0;
	if(windows_list.window[win_id].window_id != win_id)	return 0;

	return (windows_list.window[win_id].displayed || windows_list.window[win_id].reinstate);
}

int	display_window(int win_id)
{
	if(win_id < 0 || win_id >= windows_list.num_windows)	return -1;
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
	if(win_id < 0 || win_id >= windows_list.num_windows)	return -1;
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
	int ret_val = 0;
	int scroll_pos = 0;

	if(win_id < 0 || win_id >= windows_list.num_windows)	return -1;
	if(windows_list.window[win_id].window_id != win_id)	return -1;
	win= &windows_list.window[win_id];
	W = win->widgetlist;
	if(mouse_in_window(win_id, x, y) > 0)
	{
		static int time=0;
		if(time+60000<cur_time){
			/*Server testing - required*/
			Uint8 str[1];
			str[0]=PING_REQUEST;
			my_tcp_send(my_socket, str, 1);
			time=cur_time;
		}

		mx = x - win->cur_x;
		my = y - win->cur_y;
		//check the X for close - but hide it
		if(win->flags&ELW_CLOSE_BOX)
		{
        		if(my>0 && my<=ELW_BOX_SIZE && mx>(win->len_x-ELW_BOX_SIZE) && mx<=win->len_x)
			{
				// the X was hit, hide this window
				// but don't close storage if trade is open
				if(win_id != storage_win || trade_win < 0 || !windows_list.window[trade_win].displayed){
					hide_window(win_id);
#ifndef	OLD_CLOSE_BAG
					if(win_id == ground_items_win){	// are we closing the ground item/bag window?
						// we need to tell the server we closed the bag
						unsigned char protocol_name;
  	 
  	                    protocol_name= S_CLOSE_BAG;
  	                    my_tcp_send(my_socket,&protocol_name,1);
					}
#endif	//OLD_CLOSE_BAG
				}
				if (win->close_handler != NULL)
					win->close_handler (win);
				do_window_close_sound();
				return 1;
			}				
		}
		if(win->flags&ELW_RESIZEABLE && mx > win->len_x-ELW_BOX_SIZE && my > win->len_y-ELW_BOX_SIZE) {
			/* Clicked on the resize-corner. */
			return 1;
		}
		if(win->flags&ELW_SCROLLABLE) {
			/* Adjust mouse y coordinates according to the scrollbar position */
			scroll_pos = vscrollbar_get_pos(win->window_id, win->scroll_id);
			my += scroll_pos;
		}
		// check the widgets
		glPushMatrix();
		glTranslatef((float)win->cur_x, (float)win->cur_y-scroll_pos, 0.0f);
		while (W != NULL)
		{
			if (!(W->Flags&WIDGET_DISABLED) && !(W->Flags&WIDGET_CLICK_TRANSPARENT) &&  !(W->Flags&WIDGET_INVISIBLE) &&
					mx > W->pos_x && mx <= W->pos_x + W->len_x && my > W->pos_y && my <= W->pos_y + W->len_y)
			{
				if ( widget_handle_click (W, mx - W->pos_x, my - W->pos_y, flags) )
				{
					// widget handled it
					glPopMatrix ();
					return 1;
				}
			}
			W = W->next;
		}
		glPopMatrix();

		// widgets don't deal with it, try the window handler
		if (win->click_handler != NULL)
		{
			glPushMatrix();
			glTranslatef((float)win->cur_x, (float)win->cur_y-scroll_pos, 0.0f);
			ret_val = (*win->click_handler)(win, mx, my - scroll_pos, flags);
			glPopMatrix();
		}
		
		if(!ret_val && win->flags&ELW_SCROLLABLE && flags &(ELW_WHEEL_UP|ELW_WHEEL_DOWN)) {
			/* Scroll, pass to our scroll widget */
			if(flags&ELW_WHEEL_UP) {
				vscrollbar_scroll_up(win->window_id, win->scroll_id);
			} else if(flags&ELW_WHEEL_DOWN) {
				vscrollbar_scroll_down(win->window_id, win->scroll_id);
			}
		} else if ( !ret_val && (win->flags & ELW_CLICK_TRANSPARENT) && my >= 0 ) {
			return 0;	// click is not handled, and the window is transparent
		}
		return	1;	// click is handled
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 0;
}


int	drag_in_window(int win_id, int x, int y, Uint32 flags, int dx, int dy)
{
	window_info *win;
	int	mx, my;
	int scroll_pos = 0;
	widget_list *W;

	if(win_id < 0 || win_id >= windows_list.num_windows)
		return -1;
	if(windows_list.window[win_id].window_id != win_id)
		return -1;

	win= &windows_list.window[win_id];
	W = win->widgetlist;

	mx = x - win->cur_x;
	my = y - win->cur_y;

	if(win->flags&ELW_SCROLLABLE) {
		/* Adjust mouse y coordinates according to the scrollbar position */
		scroll_pos = vscrollbar_get_pos(win->window_id, win->scroll_id);
		my += scroll_pos;
	}
	if (cur_drag_widget)
	{
		// Check if cur_drag_widget is indeed one of our widgets
		while (W && W != cur_drag_widget)
			W = W->next;

		if (W && !(W->Flags & WIDGET_DISABLED))
		{
			int ret_val;
			
			glPushMatrix ();
			glTranslatef ((float)win->cur_x, (float)win->cur_y-scroll_pos, 0.0f);
			ret_val = widget_handle_drag (W, mx - W->pos_x, my - W->pos_y, flags, dx, dy);
			glPopMatrix ();
			if (ret_val)
				// widget handled it
				return 1;
		}

		// If we got here, cur_drag_widget isn't in this window, or
		// is unable to handle the drag request. Reset it and continue.
		cur_drag_widget = NULL;
	}

	if(win->drag_in || mouse_in_window(win_id, x, y) > 0)
	{
		// widgets
		glPushMatrix();
		glTranslatef((float)win->cur_x, (float)win->cur_y-scroll_pos, 0.0f);
		while (W != NULL)
		{
			if (mx > W->pos_x && mx <= W->pos_x + W->len_x && my> W ->pos_y && my <= W->pos_y+W->len_y)
			{
				if (!(W->Flags&WIDGET_DISABLED)) {
					if ( widget_handle_drag (W, mx - W->pos_x, my - W->pos_y, flags, dx, dy) )
					{
						// widget handled it
						glPopMatrix ();
						cur_drag_widget = W;
						return 1;
					}
				}
			}
			W = W->next;
		}
		glPopMatrix();

		// widgets don't deal with it, try the window handler
		if (win->drag_handler != NULL)
		{
			glPushMatrix();
			glTranslatef((float)win->cur_x, (float)win->cur_y-scroll_pos, 0.0f);
			glPopMatrix();
		}
		return	1;	// drag has been processed	
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 0;
}

int	mouseover_window (int win_id, int x, int y)
{
	window_info *win;
	int	mx, my;
	int	ret_val=0;
	int scroll_pos = 0;
	widget_list *W;

	if(win_id < 0 || win_id >= windows_list.num_windows)	return -1;
	if(windows_list.window[win_id].window_id != win_id)	return -1;
	win = &windows_list.window[win_id];
	W = win->widgetlist;
	
	if (mouse_in_window (win_id, x, y) > 0)
	{
		mx = x - win->cur_x;
		my = y - win->cur_y;

		if(win->flags&ELW_SCROLLABLE) {
			/* Adjust mouse y coordinates according to the scrollbar position */
			scroll_pos = vscrollbar_get_pos(win->window_id, win->scroll_id);
			my += scroll_pos;
		} else {
			scroll_pos = 0;
		}
		// widgets
		glPushMatrix();
		glTranslatef((float)win->cur_x, (float)win->cur_y-scroll_pos, 0.0f);
		while (W != NULL)
		{
			if (mx > W->pos_x && mx <= W->pos_x + W->len_x && my > W->pos_y && my <= W->pos_y+W->len_y)
			{
				if (!(W->Flags&WIDGET_DISABLED)) {
					// don't return on mouseover. hopefully it 
					// won't destroy our window...
					widget_handle_mouseover (W, mx, my);
				}
			}
			W = W->next;
		}
		glPopMatrix();

		// use the handler if present
		if(win->mouseover_handler)
		{
			glPushMatrix();
			glTranslatef ((float)win->cur_x, (float)win->cur_y-scroll_pos, 0.0f);
			ret_val = (*win->mouseover_handler)(win, mx, my);
			glPopMatrix();

		} 
#ifdef	ELC
		if (!ret_val)
			elwin_mouse = CURSOR_ARROW;
#endif	//ELC
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

		return 1;
	}

	return 0;
}

int	keypress_in_window(int win_id, int x, int y, Uint32 key, Uint32 unikey)
{
	window_info *win;
	int	mx, my;
	int scroll_pos = 0;
   	widget_list *W;

	if(win_id < 0 || win_id >= windows_list.num_windows
	|| windows_list.window[win_id].window_id != win_id) {
		return -1;
	}
	win = &windows_list.window[win_id];
	W = win->widgetlist;

	if (mouse_in_window (win_id, x, y) > 0)
	{
		mx = x - win->cur_x;
		my = y - win->cur_y;

		if(win->flags&ELW_SCROLLABLE) {
			/* Adjust mouse y coordinates according to the scrollbar position */
			scroll_pos = vscrollbar_get_pos(win->window_id, win->scroll_id);
			my += scroll_pos;
		} else {
			scroll_pos = 0;
		}
		// widgets
		glPushMatrix();
		glTranslatef((float)win->cur_x, (float)win->cur_y-scroll_pos, 0.0f);
		while(W != NULL)
		{
			if (mx > W->pos_x && mx <= W->pos_x + W->len_x && my > W->pos_y && my <= W->pos_y+W->len_y)
			{
				if (!(W->Flags&WIDGET_DISABLED)) {
					if ( widget_handle_keypress (W, mx - W->pos_x, my - W->pos_y, key, unikey) )
					{
						// widget handled it 
						glPopMatrix ();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
						return 1;
					}
				}
			}
			W = W->next;
		}
		glPopMatrix();

		// widgets don't deal with it, try the window handler
		if(win->keypress_handler != NULL)
		{
			int ret_val;
			
			glPushMatrix();
			glTranslatef((float)win->cur_x, (float)win->cur_y, 0.0f);
			ret_val = (*win->keypress_handler) (win, mx, my, key, unikey);
			glPopMatrix();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

			return ret_val; // keypresses are fall-through
		} 
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 0;
}

void	*set_window_handler(int win_id, int handler_id, int (*handler)() )
{
	void	*old_handler;

	if(win_id < 0 || win_id >= windows_list.num_windows)	return NULL;
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
		case	ELW_HANDLER_PRE_DISPLAY:
			old_handler= (void *)windows_list.window[win_id].pre_display_handler;
			windows_list.window[win_id].pre_display_handler=handler;
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
		case	ELW_HANDLER_RESIZE:
			old_handler= (void *)windows_list.window[win_id].resize_handler;
			windows_list.window[win_id].resize_handler=handler;
			break;
		case	ELW_HANDLER_KEYPRESS:
			old_handler= (void *)windows_list.window[win_id].keypress_handler;
			windows_list.window[win_id].keypress_handler=handler;
			break;
		case	ELW_HANDLER_CLOSE:
			old_handler= (void *)windows_list.window[win_id].close_handler;
			windows_list.window[win_id].close_handler=handler;
			break;
		case	ELW_HANDLER_DESTROY:
			old_handler= (void *)windows_list.window[win_id].destroy_handler;
			windows_list.window[win_id].destroy_handler=handler;
			break;
		case	ELW_HANDLER_SHOW:
			old_handler= (void *)windows_list.window[win_id].show_handler;
			windows_list.window[win_id].show_handler=handler;
			break;
		case	ELW_HANDLER_AFTER_SHOW:
			old_handler= (void *)windows_list.window[win_id].after_show_handler;
			windows_list.window[win_id].after_show_handler=handler;
			break;
		case	ELW_HANDLER_HIDE:
			old_handler= (void *)windows_list.window[win_id].hide_handler;
			windows_list.window[win_id].hide_handler=handler;
			break;
		default:
			old_handler=NULL;
	}

	return old_handler;
}

int	set_window_color(int win_id, Uint32 color_id, float r, float g, float b, float a)
{
	if(win_id < 0 || win_id >= windows_list.num_windows)	return 0;
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
			if(windows_list.window[win_id].flags&ELW_SCROLLABLE) {
				/* Update the color of the scroll widget too */
				widget_set_color(win_id, windows_list.window[win_id].scroll_id, r, g, b);
			}
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

int	use_window_color(int win_id, Uint32 color_id)
{
	if(win_id < 0 || win_id >= windows_list.num_windows)	return 0;
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

int set_window_min_size (int win_id, int width, int height)
{
	if (win_id < 0 || win_id >= windows_list.num_windows)	return 0;
	if (windows_list.window[win_id].window_id != win_id)	return 0;
	if (width < 0 || height < 0)	return 0;

	windows_list.window[win_id].min_len_x = width;
	windows_list.window[win_id].min_len_y = height;
	
	return 1;
}

int set_window_flag (int win_id, Uint32 flag)
{
	if (win_id < 0 || win_id >= windows_list.num_windows)	return 0;
	if (windows_list.window[win_id].window_id != win_id)	return 0;
	
	windows_list.window[win_id].flags |= flag;
	return windows_list.window[win_id].flags; 
}

void set_window_scroll_len(int win_id, int bar_len)
{
	if(windows_list.window[win_id].flags&ELW_SCROLLABLE) {
		vscrollbar_set_bar_len (win_id, windows_list.window[win_id].scroll_id, bar_len);
	}
}

void set_window_scroll_yoffset(int win_id, int yoffset)
{
	if(windows_list.window[win_id].flags&ELW_SCROLLABLE)
	{
		windows_list.window[win_id].scroll_yoffset = yoffset;
		resize_scrollbar(&windows_list.window[win_id]);
	}
}

void set_window_scroll_inc(int win_id, int inc)
{
	if(windows_list.window[win_id].flags&ELW_SCROLLABLE)
		vscrollbar_set_pos_inc(win_id, windows_list.window[win_id].scroll_id, inc);
}

void set_window_scroll_pos(int win_id, int pos)
{
	if(windows_list.window[win_id].flags&ELW_SCROLLABLE)
		vscrollbar_set_pos(win_id, windows_list.window[win_id].scroll_id, pos);
}

int get_window_scroll_pos(int win_id)
{
	if(windows_list.window[win_id].flags&ELW_SCROLLABLE)
		return vscrollbar_get_pos(win_id, windows_list.window[win_id].scroll_id);
	else
		return 0;
}

/* currently UNUSED
int	find_window(const char *name)
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
		case	ELW_HANDLER_RESIZE:
			old_handler= (void *)windows_list.window[win_id].resize_handler;
			break;
		case	ELW_HANDLER_KEYPRESS:
			old_handler= (void *)windows_list.window[win_id].keypress_handler;
			break;
		case	ELW_HANDLER_DESTROY:
			old_handler= (void *)windows_list.window[win_id].destroy_handler;
			break;
		default:
			old_handler=NULL;
	}

	return old_handler;
}
*/

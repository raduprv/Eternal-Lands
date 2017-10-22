#include "asc.h"
#include "context_menu.h"
#include "cursors.h"
#include "elconfig.h"
#include "elwindows.h"
#include "font.h"
#include "gl_init.h"
#include "hud_misc_window.h"
#include "hud_quickbar_window.h"
#include "interface.h"
#include "item_info.h"
#include "items.h"
#include "multiplayer.h"
#include "sound.h"
#include "spells.h"
#include "textures.h"

#define UI_SCALED_VALUE(BASE) ((int)(0.5 + ((BASE) * get_global_scale())))

int	quickbar_win= -1;
int quickbar_x = 0;
int quickbar_y = 0;
int quickbar_draggable=0;
int quickbar_dir=VERTICAL;
int quickbar_relocatable=0;
int num_quickbar_slots = 6;
int qb_action_mode=ACTION_USE;
int cm_quickbar_enabled = 0;

static size_t cm_quickbar_id = CM_INIT_VALUE;
static int mouseover_quickbar_item_pos = -1;
static int last_num_quickbar_slots = 6;
static int DEF_QUICKBAR_X_LEN = -1;
static int DEF_QUICKBAR_Y_LEN = -1;
static int DEF_QUICKBAR_X = -1;
static int DEF_QUICKBAR_Y = -1;
static int quickbar_x_len = 0;

enum {	CMQB_RELOC=0, CMQB_DRAG, CMQB_RESET, CMQB_FLIP, CMQB_ENABLE };


/*Change flags*/
static void change_flags(int win_id, Uint32 flags)
{
	int order = windows_list.window[win_id].order;
	
	windows_list.window[win_id].flags = flags;
	if ( (order > 0 && (flags & ELW_SHOW_LAST)) || (order < 0 && !(flags & ELW_SHOW_LAST)) )
		windows_list.window[win_id].order = -order;
}


/*Return flags*/
static Uint32 get_flags(int win_id) {
	return windows_list.window[win_id].flags;
}


// return the window y len based on the number of slots
static int get_quickbar_y_len(void)
{
	return num_quickbar_slots * DEF_QUICKBAR_Y_LEN + 1;
}


/* get the base y coord of the quick bar if its in 
   it's default place, otherwise return where the top would be */
static int get_quickbar_y_base(void)
{
	if ((quickbar_draggable) || (quickbar_dir!=VERTICAL) ||
		(quickbar_x_len != DEF_QUICKBAR_X_LEN) || 
		(quickbar_x != DEF_QUICKBAR_X) || (quickbar_y != DEF_QUICKBAR_Y))
		return DEF_QUICKBAR_Y;
	else
		return DEF_QUICKBAR_Y + get_quickbar_y_len();
}


/*Enable/disable quickbar title bar and dragability*/
static void toggle_quickbar_draggable()
{
	Uint32 flags = get_flags(quickbar_win);
	if (!quickbar_draggable)
	{
		flags &= ~ELW_SHOW_LAST;
		flags |= ELW_DRAGGABLE | ELW_TITLE_BAR;
		change_flags (quickbar_win, flags);
		quickbar_draggable = 1;
	}
	else 
	{
		flags |= ELW_SHOW_LAST;
		flags &= ~(ELW_DRAGGABLE | ELW_TITLE_BAR);
		change_flags (quickbar_win, flags);
		quickbar_draggable = 0;
		quickbar_x = window_width - windows_list.window[quickbar_win].cur_x;
		quickbar_y = windows_list.window[quickbar_win].cur_y;
	}
}


/*Change the quickbar from vertical to horizontal, or vice versa*/
static void flip_quickbar() 
{
	if (quickbar_dir==VERTICAL) 
		{
			quickbar_dir=HORIZONTAL;
			init_window(quickbar_win, -1, 0, windows_list.window[quickbar_win].cur_x, windows_list.window[quickbar_win].cur_y, get_quickbar_y_len(), quickbar_x_len);
		}      
	else if (quickbar_dir==HORIZONTAL) 
		{
			quickbar_dir=VERTICAL;
			init_window(quickbar_win, -1, 0, windows_list.window[quickbar_win].cur_x, windows_list.window[quickbar_win].cur_y, quickbar_x_len, get_quickbar_y_len());
		}
}


/*Return the quickbar to it's Built-in position*/
static void reset_quickbar() 
{
	//Necessary Variables
	quickbar_x_len= DEF_QUICKBAR_X_LEN;
	quickbar_x= DEF_QUICKBAR_X;
	quickbar_y= DEF_QUICKBAR_Y;
	//Re-set to default orientation
	quickbar_dir=VERTICAL;
	quickbar_draggable=0;
	init_window(quickbar_win, -1, 0, quickbar_x, quickbar_y, quickbar_x_len, get_quickbar_y_len());
	//Re-set  Flags
	change_flags(quickbar_win, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST);
	//NEED x_offset
	move_window(quickbar_win, -1, 0, window_width-quickbar_x, DEF_QUICKBAR_Y);
	quickbar_relocatable = 0;
}


static int context_quickbar_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	switch (option)
	{
		case CMQB_DRAG: quickbar_draggable ^= 1; toggle_quickbar_draggable(); break;
		case CMQB_RESET: reset_quickbar(); break;
		case CMQB_FLIP: flip_quickbar(); break;
	}
	return 1;
}


static void quickbar_item_description_help(window_info *win, int pos, int slot)
{
	Uint16 item_id = item_list[pos].id;
	int image_id = item_list[pos].image_id;
	if (show_item_desc_text && item_info_available() && (get_item_count(item_id, image_id) == 1))
	{
		const char *str = get_item_description(item_id, image_id);
		if (str != NULL)
		{
			int xpos = 0, ypos = 0;
			int len_str = UI_SCALED_VALUE((strlen(str) + 1) * SMALL_FONT_X_LEN);
			/* vertical place right (or left) and aligned with slot */
			if (quickbar_dir==VERTICAL)
			{
				xpos = win->len_x + 5;
				if ((xpos + len_str + win->cur_x) > window_width)
					xpos = -len_str;
				ypos = slot * DEF_QUICKBAR_Y_LEN + (DEF_QUICKBAR_Y_LEN - UI_SCALED_VALUE(SMALL_FONT_Y_LEN)) / 2;
			}
			/* horizontal place right at bottom (or top) of window */
			else
			{
				xpos = 0;
				ypos = win->len_y + 5;
				if ((xpos + len_str + win->cur_x) > window_width)
					xpos = window_width - win->cur_x - len_str;
				if ((xpos + win->cur_x) < 0)
					xpos = -win->cur_x + 5;
				if ((ypos + UI_SCALED_VALUE(SMALL_FONT_Y_LEN) + win->cur_y) > window_height)
					ypos = -(5 + UI_SCALED_VALUE(SMALL_FONT_Y_LEN) + (quickbar_draggable * ELW_TITLE_HEIGHT));
			}
			show_help(str, xpos, ypos, win->current_scale);
		}
	}
}


static int mouseover_quickbar_handler(window_info *win, int mx, int my) {
	int y,i=0;
	int x_screen,y_screen;
	for(y=0;y<num_quickbar_slots;y++)
		{
			if(quickbar_dir==VERTICAL)
				{
					x_screen=0;
					y_screen=y*DEF_QUICKBAR_Y_LEN;
				}
			else
				{
					x_screen=y*DEF_QUICKBAR_Y_LEN;
					y_screen=0;
				}
			if(mx>x_screen && mx<x_screen+DEF_QUICKBAR_Y_LEN && my>y_screen && my<y_screen+DEF_QUICKBAR_Y_LEN)
				{
					for(i=0;i<ITEM_NUM_ITEMS;i++){
						if(item_list[i].quantity && item_list[i].pos==y)
							{
								if(qb_action_mode==ACTION_LOOK) {
									elwin_mouse=CURSOR_EYE;
								} else if(qb_action_mode==ACTION_USE) {
									elwin_mouse=CURSOR_USE;
								} else if(qb_action_mode==ACTION_USE_WITEM) {
									elwin_mouse=CURSOR_USE_WITEM;
								} else {
									elwin_mouse=CURSOR_PICK;
								}
								quickbar_item_description_help(win, i, y);
								mouseover_quickbar_item_pos = y;
								return 1;
							}
					}
				}
			}
	return 0;
}


static int	click_quickbar_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i,y;
	int x_screen,y_screen;
	Uint8 str[100];
	int trigger=ELW_LEFT_MOUSE|ELW_CTRL|ELW_SHIFT;//flags we'll use for the quickbar relocation handling
	int right_click = flags & ELW_RIGHT_MOUSE;
	int ctrl_on = flags & ELW_CTRL;
	int shift_on = flags & ELW_SHIFT;

	// only handle mouse button clicks, not scroll wheels moves or clicks
	if (( (flags & ELW_MOUSE_BUTTON) == 0) || ( (flags & ELW_MID_MOUSE) != 0)) return 0;

	if(right_click) {
		switch(qb_action_mode) {
		case ACTION_WALK:
			qb_action_mode=ACTION_LOOK;
			break;
		case ACTION_LOOK:
			qb_action_mode=ACTION_USE;
			break;
		case ACTION_USE:
			qb_action_mode=ACTION_USE_WITEM;
			break;
		case ACTION_USE_WITEM:
			if(use_item!=-1)
				use_item=-1;
			else
				qb_action_mode=ACTION_WALK;
			break;
		default:
			use_item=-1;
			qb_action_mode=ACTION_WALK;
		}
		if (cm_quickbar_enabled)
			cm_show_direct(cm_quickbar_id, quickbar_win, -1);
		return 1;
	}
	
	if(qb_action_mode==ACTION_USE_WITEM)	action_mode=ACTION_USE_WITEM;

	// no in window check needed, already done
	//see if we clicked on any item in the main category
	for(y=0;y<num_quickbar_slots;y++)
		{
			if(quickbar_dir==VERTICAL)
				{
					x_screen=0;
					y_screen=y*DEF_QUICKBAR_Y_LEN;
				}
			else
				{
					x_screen=y*DEF_QUICKBAR_Y_LEN;
					y_screen=0;
				}
			if(mx>x_screen && mx<x_screen+DEF_QUICKBAR_Y_LEN && my>y_screen && my<y_screen+DEF_QUICKBAR_Y_LEN)
				{
					//see if there is an empty space to drop this item over.
					if(item_dragged!=-1)//we have to drop this item
						{
							int any_item=0;
						        if(item_dragged == y) 
						        {		        
							
								 //let's try auto equip
								 int i;
								 for(i = ITEM_WEAR_START; i<ITEM_WEAR_START+8;i++)
								 {
								       if(item_list[i].quantity<1)
								       {
								              move_item(y,i);
								              break;
								       }								     
								  }								
							     
						                  item_dragged = -1;
						                  return 1;
						        }
						   	for(i=0;i<num_quickbar_slots;i++)
								{
									if(item_list[i].quantity && item_list[i].pos==y)
										{
											any_item=1;
											if(item_dragged==i)//drop the item only over itself
												item_dragged=-1;
											do_drop_item_sound();
											return 1;
										}
								}
							if(!any_item)
								{
									//send the drop info to the server
									str[0]=MOVE_INVENTORY_ITEM;
									str[1]=item_list[item_dragged].pos;
									str[2]=y;
									my_tcp_send(my_socket,str,3);
									item_dragged=-1;
									do_drag_item_sound();
									return 1;
								}
						}
					if(quickbar_relocatable>0)
						{
							if((flags&trigger)==(ELW_LEFT_MOUSE|ELW_CTRL))
							{
								//toggle draggable
								toggle_quickbar_draggable();
							}
							else if ( (flags & trigger)== (ELW_LEFT_MOUSE | ELW_SHIFT) && (get_flags (quickbar_win) & (ELW_TITLE_BAR | ELW_DRAGGABLE)) == (ELW_TITLE_BAR | ELW_DRAGGABLE) )
							{
								//toggle vertical/horisontal
								flip_quickbar();
							}
							else if (((flags&trigger)==trigger))
								{
									//reset
									reset_quickbar();
								}
						}
					//see if there is any item there
					for(i=0;i<num_quickbar_slots;i++)
						{
							//should we get the info for it?
							if(item_list[i].quantity && item_list[i].pos==y)
								{

									if(ctrl_on){
										str[0]=DROP_ITEM;
										str[1]=item_list[i].pos;
										*((Uint32 *)(str+2))=item_list[i].quantity;
										my_tcp_send(my_socket, str, 4);
										do_drop_item_sound();
										return 1;
									} else if(qb_action_mode==ACTION_LOOK)
										{
											click_time=cur_time;
											str[0]=LOOK_AT_INVENTORY_ITEM;
											str[1]=item_list[i].pos;
											my_tcp_send(my_socket,str,2);
										}
									else if(qb_action_mode==ACTION_USE)
										{
											if(item_list[i].use_with_inventory)
												{
													str[0]=USE_INVENTORY_ITEM;
													str[1]=item_list[i].pos;
													my_tcp_send(my_socket,str,2);
#ifdef NEW_SOUND
													item_list[i].action = USE_INVENTORY_ITEM;
#endif // NEW_SOUND
													return 1;
												}
											return 1;
										}
									else if(qb_action_mode==ACTION_USE_WITEM) {
										if(use_item!=-1) {
											str[0]=ITEM_ON_ITEM;
											str[1]=item_list[use_item].pos;
											str[2]=item_list[i].pos;
											my_tcp_send(my_socket,str,3);
#ifdef NEW_SOUND
											item_list[use_item].action = ITEM_ON_ITEM;
											item_list[i].action = ITEM_ON_ITEM;
#endif // NEW_SOUND
											if (!shift_on)
												use_item=-1;
										}
										else
											use_item=i;
										return 1;
									}
									else//we might test for other things first, like use or drop
										{
											if(item_dragged==-1)//we have to drag this item
												{
													item_dragged=i;
													do_drag_item_sound();
												}
										}

									return 1;
								}
						}
				}
		}
	return 1;
}


static int	display_quickbar_handler(window_info *win)
{
	char str[80];
	int y, i;
	Uint32 _cur_time = SDL_GetTicks(); /* grab a snapshot of current time */
	int ypos = -1, xpos = -1;

	// check if the number of slots has changes and adjust if needed
	if (last_num_quickbar_slots == -1)
		last_num_quickbar_slots = num_quickbar_slots;
	else if (last_num_quickbar_slots != num_quickbar_slots)
	{
		last_num_quickbar_slots = num_quickbar_slots;
		if (quickbar_dir==VERTICAL) 
			init_window(win->window_id, -1, 0, win->cur_x, win->cur_y, quickbar_x_len, get_quickbar_y_len());
		else
			init_window(win->window_id, -1, 0, win->cur_x, win->cur_y, get_quickbar_y_len(), quickbar_x_len);
	}

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=num_quickbar_slots-1;i>=0;i--)
	{
		if(item_list[i].quantity > 0)
		{
			float u_start,v_start,u_end,v_end;
			int this_texture,cur_item,cur_pos;
			int x_start,x_end,y_start,y_end, itmp;

			//get the UV coordinates.
			cur_item=item_list[i].image_id%25;
			get_item_uv(cur_item, &u_start, &v_start, &u_end,
				&v_end);

			//get the x and y
			cur_pos=item_list[i].pos;
					
			x_start= UI_SCALED_VALUE(2);
			x_end= x_start+UI_SCALED_VALUE(27);
			y_start= DEF_QUICKBAR_Y_LEN*(cur_pos%num_quickbar_slots)+UI_SCALED_VALUE(2);
			y_end= y_start+UI_SCALED_VALUE(27);

			if(quickbar_dir != VERTICAL)
			{
				itmp = x_start; x_start = y_start; y_start = itmp;
				itmp = x_end; x_end = y_end; y_end = itmp;
			}

			//get the texture this item belongs to
			this_texture=get_items_texture(item_list[i].image_id/25);

			bind_texture(this_texture);
			glBegin(GL_QUADS);
				draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();
			
			if (item_list[i].cooldown_time > _cur_time)
			{
				float cooldown = ((float)(item_list[i].cooldown_time - _cur_time)) / ((float)item_list[i].cooldown_rate);
				float x_center = (x_start + x_end)*0.5f;
				float y_center = (y_start + y_end)*0.5f;

				if (cooldown < 0.0f)
					cooldown = 0.0f;
				else if (cooldown > 1.0f)
					cooldown = 1.0f;
				
				glDisable(GL_TEXTURE_2D);
				glEnable(GL_BLEND);
				
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_TRIANGLE_FAN);
					glColor4f(0.14f, 0.35f, 0.82f, 0.50f); 

					glVertex2f(x_center, y_center);

					if (cooldown >= 0.875f) {
						float t = tan(2.0f*M_PI*(1.0f - cooldown));
						glVertex2f(t*x_end + (1.0f - t)*x_center, y_start);
						glVertex2f(x_end, y_start);
						glVertex2f(x_end, y_end);
						glVertex2f(x_start, y_end);
						glVertex2f(x_start, y_start);
					} else if (cooldown >= 0.625f) {
						float t = 0.5f + 0.5f*tan(2.0f*M_PI*(0.75f - cooldown));
						glVertex2f(x_end, t*y_end + (1.0f - t)*y_start);
						glVertex2f(x_end, y_end);
						glVertex2f(x_start, y_end);
						glVertex2f(x_start, y_start);
					} else if (cooldown >= 0.375f) {
						float t = 0.5f + 0.5f*tan(2.0f*M_PI*(0.5f - cooldown));
						glVertex2f(t*x_start + (1.0f - t)*x_end, y_end);
						glVertex2f(x_start, y_end);
						glVertex2f(x_start, y_start);
					} else if (cooldown >= 0.125f) {
						float t = 0.5f + 0.5f*tan(2.0f*M_PI*(0.25f - cooldown));
						glVertex2f(x_start, t*y_start + (1.0f - t)*y_end);
						glVertex2f(x_start, y_start);
					} else {
						float t = tan(2.0f*M_PI*(cooldown));
						glVertex2f(t*x_start + (1.0f - t)*x_center, y_start);
					}

					glVertex2f(x_center, y_start);
				glEnd();

				glDisable(GL_BLEND);
				glEnable(GL_TEXTURE_2D);
			}
			
			safe_snprintf(str,sizeof(str),"%i",item_list[i].quantity);
			if (quickbar_dir==VERTICAL)
			{
				int lenstr = strlen(str);
				lenstr *= ((mouseover_quickbar_item_pos == i) && enlarge_text()) ?DEFAULT_FONT_X_LEN :SMALL_FONT_X_LEN;
				xpos = ((x_start + lenstr + win->cur_x) > window_width) ?window_width - win->cur_x - lenstr :x_start;
				ypos = y_end-UI_SCALED_VALUE(15);
			}
			else
			{
				xpos = x_start;
				ypos = (i&1)?(y_end-UI_SCALED_VALUE(15)):(y_end-UI_SCALED_VALUE(25));
			}
			if ((mouseover_quickbar_item_pos == i) && enlarge_text())
				draw_string_shadowed_zoomed(xpos,ypos,(unsigned char*)str,1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f, win->current_scale);
			else
				draw_string_small_shadowed_zoomed(xpos,ypos,(unsigned char*)str,1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f, win->current_scale);
		}
	}
	mouseover_quickbar_item_pos = -1;
	
	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all 
	// cards
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	use_window_color(quickbar_win, ELW_COLOR_LINE);
	//draw the grid
	if(quickbar_dir==VERTICAL)
		{
			for(y=1;y<num_quickbar_slots;y++)
				{
					glVertex3i(0, y*DEF_QUICKBAR_Y_LEN+1, 0);
					glVertex3i(quickbar_x_len, y*DEF_QUICKBAR_Y_LEN+1, 0);
				}
		}
	else
		{
			for(y=1;y<num_quickbar_slots;y++)
				{
					glVertex3i(y*DEF_QUICKBAR_Y_LEN+1, 0, 0);
					glVertex3i(y*DEF_QUICKBAR_Y_LEN+1, quickbar_x_len, 0);
				}
		}
	glEnd();
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}


void init_quickbar (void)
{
	Uint32 flags = ELW_USE_UISCALE | ELW_USE_BACKGROUND | ELW_USE_BORDER;
	static int last_ui_scale = 0;

	if (last_ui_scale != get_global_scale())
	{
		DEF_QUICKBAR_X_LEN = UI_SCALED_VALUE(30);
		DEF_QUICKBAR_Y_LEN = UI_SCALED_VALUE(30);
		DEF_QUICKBAR_X = UI_SCALED_VALUE(30+4);
		DEF_QUICKBAR_Y = UI_SCALED_VALUE(64);
		quickbar_x_len = DEF_QUICKBAR_X_LEN;
		if ((!quickbar_relocatable) || (quickbar_x < 0) || (quickbar_y < 0))
		{
			quickbar_x = DEF_QUICKBAR_X;
			quickbar_y = DEF_QUICKBAR_Y;
		}
		last_ui_scale = get_global_scale();
	}

	quickbar_x_len = DEF_QUICKBAR_X_LEN;
	
	if (!quickbar_relocatable)
	{
		flags |= ELW_SHOW_LAST;
		quickbar_draggable = 0;
	}
	if (quickbar_draggable) flags |= ELW_TITLE_BAR | ELW_DRAGGABLE;	
	
	if (quickbar_win < 0)
	{
		if (quickbar_dir == VERTICAL)
			quickbar_win = create_window ("Quickbar", -1, 0, window_width - quickbar_x, quickbar_y, quickbar_x_len, get_quickbar_y_len(), flags);
		else
			quickbar_win = create_window ("Quickbar", -1, 0, window_width - quickbar_x, quickbar_y, get_quickbar_y_len(), quickbar_x_len, flags);
		last_num_quickbar_slots = num_quickbar_slots;

		set_window_handler(quickbar_win, ELW_HANDLER_DISPLAY, &display_quickbar_handler);
		set_window_handler(quickbar_win, ELW_HANDLER_CLICK, &click_quickbar_handler);
		set_window_handler(quickbar_win, ELW_HANDLER_MOUSEOVER, &mouseover_quickbar_handler );

		cm_quickbar_id = cm_create(cm_quickbar_menu_str, context_quickbar_handler);
		cm_bool_line(cm_quickbar_id, CMQB_RELOC, &quickbar_relocatable, "relocate_quickbar");
		cm_bool_line(cm_quickbar_id, CMQB_DRAG, &quickbar_draggable, NULL);
		cm_bool_line(cm_quickbar_id, CMQB_ENABLE, &cm_quickbar_enabled, NULL);
	}
	else
	{
		change_flags (quickbar_win, flags);
		if (quickbar_draggable) 
		{
			show_window (quickbar_win);
		}
		else if (quickbar_y > window_height || quickbar_x > window_width) 
		{
			move_window (quickbar_win, -1, 0, 200, 64); // Correct invalid position
		}
		else
		{
			move_window (quickbar_win, -1, 0, window_width - quickbar_x, quickbar_y);
		}
		if (quickbar_dir == VERTICAL)
			resize_window(quickbar_win, quickbar_x_len, get_quickbar_y_len());
		else
			resize_window(quickbar_win, get_quickbar_y_len(), quickbar_x_len);
	}
}


void switch_action_mode(int * mode, int id)
{
	item_action_mode=qb_action_mode=action_mode=*mode;
}


/*	Get the longest of the active quickspells and the
	quickbar (if its in default place)
*/
int get_max_quick_y(void)
{
	int quickspell_base = get_quickspell_y_base();
	int quickbar_base = get_quickbar_y_base();
	int max_quick_y = window_height;
	
	if (quickspell_base > quickbar_base)
		max_quick_y -= quickspell_base;
	else
		max_quick_y -= quickbar_base;

	return max_quick_y;
}

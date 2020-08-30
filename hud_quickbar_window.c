#include "asc.h"
#include "context_menu.h"
#include "cursors.h"
#include "elconfig.h"
#include "elwindows.h"
#include "font.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "hud_quickbar_window.h"
#include "hud_misc_window.h"
#include "interface.h"
#include "item_info.h"
#include "items.h"
#include "keys.h"
#include "multiplayer.h"
#include "sound.h"
#include "spells.h"
#include "textures.h"

int quickbar_draggable=1;
int quickbar_dir=HORIZONTAL;
// with quickbar_relocatable off, the default will be docked to right hud
int quickbar_relocatable=0;
int num_quickbar_slots = 6;
int cm_quickbar_enabled = 0;
int independant_quickbar_action_modes = 0;

static size_t cm_quickbar_id = CM_INIT_VALUE;
static int mouseover_quickbar_item_pos = -1;
static int item_quickbar_slot_size = -1;
static int default_item_quickbar_x = -1;
static int default_item_quickbar_y = -1;
static int shown_quickbar_slots = -1;
static int qb_action_mode=ACTION_USE;

enum { CMQB_ENABLE=0, CMQB_SEP1, CMQB_RELOC, CMQB_DRAG, CMQB_FLIP, CMQB_SEP2, CMQB_RESET };


/*Change flags*/
static void change_flags(int win_id, Uint32 flags)
{
	int order = windows_list.window[win_id].order;

	windows_list.window[win_id].flags = flags;
	if ( (order > 0 && (flags & ELW_SHOW_LAST)) || (order < 0 && !(flags & ELW_SHOW_LAST)) )
		windows_list.window[win_id].order = -order;
}


/*Return flags*/
static Uint32 get_flags(int win_id)
{
	return windows_list.window[win_id].flags;
}


// returns try if the window is not in the default place, false if it is, even if it can be relocated
static int is_relocated(void)
{
	int quickbar_win = get_id_MW(MW_QUICKBAR);
	window_info *win = NULL;
	if (quickbar_win < 0 || quickbar_win > windows_list.num_windows)
		return 1;
	win = &windows_list.window[quickbar_win];
	if ((quickbar_draggable) || (quickbar_dir != VERTICAL) ||
		(win->cur_x != default_item_quickbar_x) || (win->cur_y != default_item_quickbar_y))
		return 1;
	else
		return 0;
}


// return the window y len based on the number of slots
static int get_quickbar_y_len(void)
{
	return shown_quickbar_slots * item_quickbar_slot_size + 1;
}


/* get the base y coord of the quick bar if its in
   it's default place, otherwise return where the top would be */
int get_quickbar_y_base(void)
{
	if (is_relocated())
		return default_item_quickbar_y;
	else
		return default_item_quickbar_y + get_quickbar_y_len();
}


/*Enable/disable quickbar title bar and dragability*/
static void toggle_quickbar_draggable(void)
{
	int quickbar_win = get_id_MW(MW_QUICKBAR);
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
	}
}


// common function to resize window depending on orientation
static void resize_item_quickbar_window(int window_id)
{
	if (quickbar_dir==VERTICAL)
		resize_window(window_id, item_quickbar_slot_size, get_quickbar_y_len());
	else
		resize_window(window_id, get_quickbar_y_len(), item_quickbar_slot_size);
}


// return the number of shown slots, increasing to the oprions value if appropriate
static void update_shown_quickbar_slots(window_info *win)
{
	int last_shown_slots = shown_quickbar_slots;

	if (quickbar_relocatable && is_relocated())
		shown_quickbar_slots = num_quickbar_slots;
	else
	{
		int max_slots = (window_height - get_min_hud_misc_len_y() - default_item_quickbar_y - 1) / item_quickbar_slot_size;
		if (max_slots > num_quickbar_slots)
			shown_quickbar_slots = num_quickbar_slots;
		else if (max_slots < 1)
			shown_quickbar_slots = 1;
		else
			shown_quickbar_slots = max_slots;
	}

	if (last_shown_slots != shown_quickbar_slots)
		resize_item_quickbar_window(win->window_id);
}


/*Change the quickbar from vertical to horizontal, or vice versa*/
static void flip_quickbar(int window_id)
{
	if (quickbar_dir == VERTICAL)
		quickbar_dir = HORIZONTAL;
	else
		quickbar_dir = VERTICAL;
	resize_item_quickbar_window(window_id);
}


/*Return the quickbar to it's Built-in position*/
static void reset_quickbar()
{
	int quickbar_win = get_id_MW(MW_QUICKBAR);
	limit_win_scale_to_default(get_scale_WM(MW_QUICKBAR));
	quickbar_dir = VERTICAL;
	quickbar_draggable = 0;
	quickbar_relocatable = 0;
	set_var_unsaved("relocate_quickbar", INI_FILE_VAR);
	change_flags(quickbar_win, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW_LAST);
	init_window(quickbar_win, -1, 0, default_item_quickbar_x, default_item_quickbar_y, item_quickbar_slot_size, get_quickbar_y_len());
}


static void cm_quickbar_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	cm_grey_line(cm_quickbar_id, CMQB_DRAG, (quickbar_relocatable) ?0 :1);
	cm_grey_line(cm_quickbar_id, CMQB_FLIP, (quickbar_relocatable) ?0 :1);
}


static int context_quickbar_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	switch (option)
	{
		case CMQB_RELOC: if (quickbar_relocatable) toggle_quickbar_draggable(); break;
		case CMQB_DRAG: quickbar_draggable ^= 1; toggle_quickbar_draggable(); break;
		case CMQB_RESET: reset_quickbar(); break;
		case CMQB_FLIP: flip_quickbar(win->window_id); break;
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
			const int tooltip_sep = (int)(0.5 + win->current_scale * 5);
			int len_str = get_string_width_zoom((const unsigned char*)str,
				win->font_category, win->current_scale_small) + tooltip_sep;
			/* vertical place right (or left) and aligned with slot */
			if (quickbar_dir==VERTICAL)
			{
				xpos = win->len_x + 5;
				if ((xpos + len_str + win->cur_x) > window_width)
					xpos = -len_str;
				ypos = slot * item_quickbar_slot_size + (item_quickbar_slot_size - win->small_font_len_y) / 2;
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
				if ((ypos + win->small_font_len_y + win->cur_y) > window_height)
					ypos = -(5 + win->small_font_len_y + (quickbar_draggable * win->title_height));
			}
			show_help(str, xpos, ypos, win->current_scale);
		}
	}
}


static int mouseover_quickbar_handler(window_info *win, int mx, int my) {
	int y,i=0;
	int x_screen,y_screen;

	for(y=0;y<shown_quickbar_slots;y++)
		{
			if(quickbar_dir==VERTICAL)
				{
					x_screen=0;
					y_screen=y*item_quickbar_slot_size;
				}
			else
				{
					x_screen=y*item_quickbar_slot_size;
					y_screen=0;
				}
			if(mx>x_screen && mx<x_screen+item_quickbar_slot_size && my>y_screen && my<y_screen+item_quickbar_slot_size)
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
	int trigger=ELW_LEFT_MOUSE|KMOD_CTRL|KMOD_SHIFT;//flags we'll use for the quickbar relocation handling
	int right_click = flags & ELW_RIGHT_MOUSE;
	int ctrl_on = flags & KMOD_CTRL;
	int shift_on = flags & KMOD_SHIFT;

	// only handle mouse button clicks, not scroll wheels moves or clicks
	if (( (flags & ELW_MOUSE_BUTTON) == 0) || ( (flags & ELW_MID_MOUSE) != 0)) return 0;

	if(right_click) {
		switch(qb_action_mode) {
		case ACTION_WALK:
			if(item_dragged != -1)
				item_dragged = -1;
			else
				qb_action_mode = ACTION_LOOK;
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
			cm_show_direct(cm_quickbar_id, win->window_id, -1);
		return 1;
	}

	if (qb_action_mode == ACTION_USE_WITEM)
		set_gamewin_usewith_action();

	// no in window check needed, already done
	//see if we clicked on any item in the main category
	for(y=0;y<shown_quickbar_slots;y++)
		{
			if(quickbar_dir==VERTICAL)
				{
					x_screen=0;
					y_screen=y*item_quickbar_slot_size;
				}
			else
				{
					x_screen=y*item_quickbar_slot_size;
					y_screen=0;
				}
			if(mx>x_screen && mx<x_screen+item_quickbar_slot_size && my>y_screen && my<y_screen+item_quickbar_slot_size)
				{
					//see if there is an empty space to drop this item over.
					if(item_dragged!=-1)//we have to drop this item
						{
							int any_item=0;
							if(item_dragged == y)
								{
									try_auto_equip(item_dragged);
									return 1;
								}
							for(i=0;i<shown_quickbar_slots;i++)
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
							if((flags&trigger)==(ELW_LEFT_MOUSE|KMOD_CTRL))
							{
								//toggle draggable
								toggle_quickbar_draggable();
							}
							else if ( (flags & trigger)== (ELW_LEFT_MOUSE | KMOD_SHIFT) && (get_flags (win->window_id) & (ELW_TITLE_BAR | ELW_DRAGGABLE)) == (ELW_TITLE_BAR | ELW_DRAGGABLE) )
							{
								//toggle vertical/horisontal
								flip_quickbar(win->window_id);
							}
							else if (((flags&trigger)==trigger))
								{
									//reset
									reset_quickbar();
								}
						}
					//see if there is any item there
					for(i=0;i<shown_quickbar_slots;i++)
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
													used_item_counter_action_use(i);
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
											used_item_counter_action_use(use_item);
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
	unsigned char str[80];
	int y, i;
	Uint32 _cur_time = SDL_GetTicks(); /* grab a snapshot of current time */
	int xpos = -1;
	const int scaled_2 = (int)(0.5 + win->current_scale * 2);
	const int scaled_27 = (int)(0.5 + win->current_scale * 27);

	update_shown_quickbar_slots(win);
	check_for_swap_completion();

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=shown_quickbar_slots-1;i>=0;i--)
	{
		if(item_list[i].quantity > 0)
		{
			float u_start,v_start,u_end,v_end;
			int this_texture,cur_item,cur_pos;
			int x_start,x_end,y_start,y_end, itmp;
			float zoom;

			// don't display an item that is in the proces of being moved after equipment swap
			if (item_swap_in_progress(i))
				continue;

			//get the UV coordinates.
			cur_item=item_list[i].image_id%25;
			get_item_uv(cur_item, &u_start, &v_start, &u_end,
				&v_end);

			//get the x and y
			cur_pos=item_list[i].pos;

			x_start = scaled_2;
			x_end = x_start + scaled_27;
			y_start = item_quickbar_slot_size * (cur_pos % shown_quickbar_slots) + scaled_2;
			y_end = y_start + scaled_27;

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

			if ((_cur_time - item_list_extra[i].slot_busy_start) < 250)
				gray_out(x_start,y_start,item_quickbar_slot_size);

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

			safe_snprintf((char*)str, sizeof(str), "%d", item_list[i].quantity);
			xpos = x_start;
			zoom = (mouseover_quickbar_item_pos == i && enlarge_text())
				? win->current_scale : win->current_scale_small;
			if (quickbar_dir==VERTICAL)
			{
				int lenstr = get_string_width_zoom(str, win->font_category, zoom);
				xpos = min2i(xpos, window_width - win->cur_x - lenstr);
			}

			draw_text(xpos, y_end, str, strlen((const char*)str), win->font_category, TDO_SHADOW, 1,
				TDO_FOREGROUND, 1.0, 1.0, 1.0, TDO_BACKGROUND, 0.0, 0.0, 0.0, TDO_ZOOM, zoom,
				TDO_VERTICAL_ALIGNMENT, BOTTOM_LINE, TDO_END);
		}
	}
	mouseover_quickbar_item_pos = -1;

	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all
	// cards
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	use_window_color(win->window_id, ELW_COLOR_LINE);
	//draw the grid
	if(quickbar_dir==VERTICAL)
		{
			for(y=1;y<shown_quickbar_slots;y++)
				{
					glVertex3i(0, y*item_quickbar_slot_size+1, 0);
					glVertex3i(item_quickbar_slot_size, y*item_quickbar_slot_size+1, 0);
				}
		}
	else
		{
			for(y=1;y<shown_quickbar_slots;y++)
				{
					glVertex3i(y*item_quickbar_slot_size+1, 0, 0);
					glVertex3i(y*item_quickbar_slot_size+1, item_quickbar_slot_size, 0);
				}
		}
	glEnd();
	glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}


static int ui_scale_quickbar_handler(window_info *win)
{
	update_shown_quickbar_slots(win);
	item_quickbar_slot_size = (int)(0.5 + win->current_scale * 30);
	default_item_quickbar_x = window_width - item_quickbar_slot_size - 1;
	default_item_quickbar_y = get_hud_logo_size();
	if (!quickbar_relocatable)
		reset_quickbar();
	else
	{
		resize_item_quickbar_window(win->window_id);
		if (win->cur_x > window_width || win->cur_y > window_height)
			move_window(win->window_id, -1, 0, 100, 100);
	}
	update_shown_quickbar_slots(win);
	return 1;
}


void init_quickbar (void)
{
	int quickbar_win = get_id_MW(MW_QUICKBAR);
	Uint32 flags = ELW_USE_UISCALE | ELW_USE_BACKGROUND | ELW_USE_BORDER;

	if (!quickbar_relocatable)
	{
		flags |= ELW_SHOW_LAST;
		quickbar_draggable = 0;
	}
	if (quickbar_draggable)
		flags |= ELW_TITLE_BAR | ELW_DRAGGABLE;

	if (quickbar_win < 0)
	{
		quickbar_win = create_window ("Quickbar", -1, 0, get_pos_x_MW(MW_QUICKBAR), get_pos_y_MW(MW_QUICKBAR), 0, 0, flags);
		if (quickbar_win < 0 || quickbar_win >= windows_list.num_windows)
			return;
		set_id_MW(MW_QUICKBAR, quickbar_win);

		set_window_custom_scale(quickbar_win, MW_QUICKBAR);
		ui_scale_quickbar_handler(&windows_list.window[quickbar_win]);

		set_window_handler(quickbar_win, ELW_HANDLER_DISPLAY, &display_quickbar_handler);
		set_window_handler(quickbar_win, ELW_HANDLER_CLICK, &click_quickbar_handler);
		set_window_handler(quickbar_win, ELW_HANDLER_MOUSEOVER, &mouseover_quickbar_handler );
		set_window_handler(quickbar_win, ELW_HANDLER_UI_SCALE, &ui_scale_quickbar_handler );

		cm_quickbar_id = cm_create(cm_quickbar_menu_str, context_quickbar_handler);
		cm_set_pre_show_handler(cm_quickbar_id, cm_quickbar_pre_show_handler);
		cm_bool_line(cm_quickbar_id, CMQB_RELOC, &quickbar_relocatable, "relocate_quickbar");
		cm_bool_line(cm_quickbar_id, CMQB_DRAG, &quickbar_draggable, NULL);
		cm_bool_line(cm_quickbar_id, CMQB_ENABLE, &cm_quickbar_enabled, NULL);
	}
	else
	{
		change_flags (quickbar_win, flags);
		ui_scale_quickbar_handler(&windows_list.window[quickbar_win]);
		show_window (quickbar_win);
	}
}


// try to use or auto equip the item in the slot
static void quick_use(int use_id, size_t *timer)
{
	Uint8 quick_use_str[3];
	int	i;

	for(i=0; i<ITEM_NUM_ITEMS; i++)
	{
		if (item_list[i].pos==use_id)
		{
			if (item_list[i].quantity)
			{
				// its a usabale item so try to use it
				if (item_list[i].use_with_inventory)
				{
					quick_use_str[0]= USE_INVENTORY_ITEM;
					quick_use_str[1]= use_id;
					quick_use_str[2]= i;
					my_tcp_send(my_socket,quick_use_str,2);
					used_item_counter_action_use(i);
#ifdef NEW_SOUND
					item_list[i].action = USE_INVENTORY_ITEM;
#endif // NEW_SOUND
				}
				// this else catches all other item types, but is not used if we have recenly use the slot
				// if the item type is not equipable, the server will tell us like it normally does
				else if (!item_swap_in_progress(i) && ((SDL_GetTicks() - *timer) > 500))
				{
					*timer = SDL_GetTicks();
					try_auto_equip(i);
				}
				// the slot will shown as busy for sort while
				else
				{
					item_list_extra[i].slot_busy_start = SDL_GetTicks();
					do_alert1_sound();
				}
			}
			return;
		}
	}
}


// check if key is one of the item keys and use it if so.
int action_item_keys(SDL_Keycode key_code, Uint16 key_mod)
{
	size_t i;
	el_key_def keys[] = {K_ITEM1, K_ITEM2, K_ITEM3, K_ITEM4, K_ITEM5, K_ITEM6,
					 K_ITEM7, K_ITEM8, K_ITEM9, K_ITEM10, K_ITEM11, K_ITEM12 };
	static size_t timers[sizeof(keys)/sizeof(el_key_def)] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	for (i=0; i<sizeof(keys)/sizeof(el_key_def); i++)
		if(KEY_DEF_CMP(keys[i], key_code, key_mod))
		{
			quick_use (i, &timers[i]);
			return 1;
		}
	return 0;
}

// Limit external setting of the action mode: Called due to an action keypress or action icon in the icon window.
void set_quickbar_action_mode(int new_mode)
{
	// Only change the action mode if is one used by the window.
	if (!independant_quickbar_action_modes && ((new_mode == ACTION_WALK) || (new_mode == ACTION_LOOK) || (new_mode == ACTION_USE) || (new_mode == ACTION_USE_WITEM)))
		qb_action_mode = new_mode;
}

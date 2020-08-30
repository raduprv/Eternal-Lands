#include <stdlib.h>
#include "trade.h"
#include "asc.h"
#include "cursors.h"
#include "elwindows.h"
#include "gamewin.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "items.h"
#include "item_info.h"
#include "manufacture.h"
#include "multiplayer.h"
#include "spells.h"
#include "storage.h"
#include "string.h"
#include "textures.h"
#include "trade_log.h"
#include "translate.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "sound.h"

#define ITEM_INVENTORY 1
#define ITEM_ROWS 4
#define ITEM_COLS 4
#define MAX_ITEMS (ITEM_COLS * ITEM_ROWS)
#define ITEM_INFO_ROWS 4
#define ITEM_BANK 2

int trade_you_accepted=0;
int trade_other_accepted=0;

static trade_item your_trade_list[MAX_ITEMS];
static trade_item others_trade_list[MAX_ITEMS];
static char other_player_trade_name[20];
static unsigned char items_string[350]={0};
static size_t last_items_string_id = 0;
static int no_view_my_items=0;
static int trade_border = 0;
static int trade_gridsize = 0;
static int trade_grid_start_y = 0;
static int button_x_left = 0;
static int button_x_right = 0;
static int button_y_top = 0;
static int button_y_bot = 0;
static const char *tool_tip_str = NULL;
static int mouse_over_your_trade_pos = -1;
static int mouse_over_others_trade_pos = -1;
static int show_abort_help=0;
static int storage_available=0;

static int display_trade_handler(window_info *win)
{
	int x_off = 0;
	int i;
	char str[20];

	//Draw the names in the accept boxes

	if(trade_you_accepted==1){
		glColor3f(1.0f,1.0f,0.0f);
	} else if(trade_you_accepted==2){
		glColor3f(0.0f,1.0f,0.0f);
	} else {
		glColor3f(1.0f,0.0f,0.0f);
	}

	x_off = trade_border + ITEM_COLS / 2 * trade_gridsize;
	draw_string_small_zoomed_centered(x_off, button_y_bot - win->default_font_len_y * 0.9, (unsigned char*)accept_str, 1, win->current_scale);

	if(trade_other_accepted<=0){    // RED
		glColor3f(1.0f,0.0f,0.0f);
	} else if(trade_other_accepted==1){ // YELLOW
		glColor3f(1.0f,1.0f,0.0f);
	} else {    // all others default to GREEN
		glColor3f(0.0f,1.0f,0.0f);
	}

	x_off = trade_border + (ITEM_COLS + 1) * trade_gridsize + ITEM_COLS / 2 * trade_gridsize;
	draw_string_small_zoomed_centered(x_off, button_y_bot - win->default_font_len_y * 0.9, (unsigned char*)accept_str, 1, win->current_scale);

	glColor3f(0.77f,0.57f,0.39f);

	//Draw the trade session names
	x_off = trade_border + ITEM_COLS / 2 * trade_gridsize;
	draw_string_small_zoomed_centered(x_off, trade_grid_start_y - win->small_font_len_y, (unsigned char*)you_str, 1, win->current_scale);
	x_off = trade_border + (ITEM_COLS + 1) * trade_gridsize + ITEM_COLS / 2 * trade_gridsize;
	draw_string_small_zoomed_centered(x_off, trade_grid_start_y - win->small_font_len_y, (unsigned char*)other_player_trade_name, 1, win->current_scale);

	glColor3f(1.0f,1.0f,1.0f);

	//Now let's draw the goods on trade...

	for(i=MAX_ITEMS-1; i>=0; --i){
		if(your_trade_list[i].quantity){
			GLfloat u_start, v_start, u_end, v_end;
			int x_start, x_end, y_start, y_end;
			int cur_item;
			GLuint this_texture;
			int y_text;
			float zoom;
			ver_alignment valign;

			cur_item=your_trade_list[i].image_id%25;
			get_item_uv(cur_item, &u_start, &v_start, &u_end, &v_end);
			this_texture=get_items_texture(your_trade_list[i].image_id/25);
			if (this_texture != -1)
			{
				bind_texture(this_texture);
			}

			x_start = trade_border + (i % ITEM_COLS) * trade_gridsize;
			x_end = x_start + trade_gridsize - 1;
			y_start= trade_grid_start_y + (i / ITEM_ROWS) * trade_gridsize;
			y_end = y_start + trade_gridsize - 1;

			glBegin(GL_QUADS);
			draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();

			safe_snprintf(str, sizeof(str), "%i",your_trade_list[i].quantity);
			zoom = (mouse_over_your_trade_pos == i && enlarge_text())
				? win->current_scale : win->current_scale_small;
			if (i & 1)
			{
				y_text = y_start;
				valign = TOP_LINE;
			}
			else
			{
				y_text = y_end;
				valign = BOTTOM_LINE;
			}
			draw_text(x_start, y_text, (const unsigned char*)str, strlen(str), win->font_category,
				TDO_SHADOW, 1, TDO_FOREGROUND, 1.0, 1.0, 1.0, TDO_BACKGROUND, 0.0, 0.0, 0.0,
				TDO_ZOOM, zoom, TDO_VERTICAL_ALIGNMENT, valign, TDO_END);
			//by doing the images in reverse, you can't cover up the digits>4
			//also, by offsetting each one, numbers don't overwrite each other:
			//before: 123456 in one box and 56 in the other could allow
			//        1234                  56  which looks legitimate
			//now:    123456
			//            56 - the odd/even numbers are staggered
			//                 stopping potential scams
		}
	}
	mouse_over_your_trade_pos = -1;

	for(i=MAX_ITEMS-1; i>=0; --i){
		if(others_trade_list[i].quantity){
			GLfloat u_start, v_start, u_end, v_end;
			int x_start, x_end, y_start, y_end;
			int cur_item;
			GLuint this_texture;
			int y_text;
			float zoom;
			ver_alignment valign;

			cur_item=others_trade_list[i].image_id%25;
			get_item_uv(cur_item, &u_start, &v_start, &u_end, &v_end);

			this_texture=get_items_texture(others_trade_list[i].image_id/25);

			if (this_texture != -1)
			{
				bind_texture(this_texture);
			}

			x_start = trade_border + (ITEM_COLS + 1) * trade_gridsize + (i % ITEM_COLS) * trade_gridsize;
			x_end = x_start + trade_gridsize - 1;
			y_start= trade_grid_start_y + (i / ITEM_ROWS) * trade_gridsize;
			y_end = y_start + trade_gridsize - 1;

			glBegin(GL_QUADS);
			draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();

			safe_snprintf(str, sizeof(str), "%i",others_trade_list[i].quantity);
			zoom = (mouse_over_others_trade_pos == i && enlarge_text())
				? win->current_scale : win->current_scale_small;
			if (i & 1)
			{
				y_text = y_start;
				valign = TOP_LINE;
			}
			else
			{
				y_text = y_end;
				valign = BOTTOM_LINE;
			}
			draw_text(x_start, y_text, (const unsigned char*)str, strlen(str), win->font_category,
				TDO_SHADOW, 1, TDO_FOREGROUND, 1.0, 1.0, 1.0, TDO_BACKGROUND, 0.0, 0.0, 0.0,
				TDO_ZOOM, zoom, TDO_VERTICAL_ALIGNMENT, valign, TDO_END);

			if(storage_available && others_trade_list[i].type==ITEM_BANK){
				str[0]='S';
				str[1]=0;
			if (i & 1)
			{
				y_text = y_end;
				valign = BOTTOM_LINE;
			}
			else
			{
				y_text = y_start;
				valign = TOP_LINE;
			}
			draw_text(x_end - win->small_font_max_len_x/2, y_text, (const unsigned char*)str, strlen(str), win->font_category,
				TDO_SHADOW, 1, TDO_FOREGROUND, 1.0, 1.0, 1.0, TDO_BACKGROUND, 0.0, 0.0, 0.0,
				TDO_ZOOM, win->current_scale_small, TDO_VERTICAL_ALIGNMENT, valign, TDO_ALIGNMENT, RIGHT, TDO_END);
			}
		}
	}
	mouse_over_others_trade_pos = -1;

	glDisable(GL_TEXTURE_2D);

	glColor3f(0.77f,0.57f,0.39f);
	// grids for goods on trade
	rendergrid (ITEM_COLS, ITEM_ROWS, trade_border, trade_grid_start_y, trade_gridsize, trade_gridsize);
	rendergrid (ITEM_COLS, ITEM_ROWS, trade_border + (ITEM_COLS + 1) * trade_gridsize, trade_grid_start_y, trade_gridsize, trade_gridsize);

	// Accept buttons
	glBegin (GL_LINE_LOOP);
		glVertex3i(button_x_left, button_y_top, 0);
		glVertex3i(button_x_right ,button_y_top, 0);
		glVertex3i(button_x_right, button_y_bot, 0);
		glVertex3i(button_x_left, button_y_bot, 0);
	glEnd ();

	x_off = (ITEM_COLS + 1) * trade_gridsize;
	glBegin (GL_LINE_LOOP);
		glVertex3i(x_off + button_x_left, button_y_top, 0);
		glVertex3i(x_off + button_x_right ,button_y_top, 0);
		glVertex3i(x_off + button_x_right, button_y_bot, 0);
		glVertex3i(x_off + button_x_left, button_y_bot, 0);
	glEnd ();

	//Draw the help text
	if(show_help_text && show_abort_help)
	{
		show_help_colored_scaled_centered((const unsigned char*)abort_str,
			win->len_x - win->box_size / 2, win->box_size + win->box_size/5,
			1.0f, 1.0f, 1.0f, win->current_scale);
	}

	glEnable(GL_TEXTURE_2D);

	//now, draw the inventory text, if any.
	if (last_items_string_id != inventory_item_string_id)
	{
		put_small_text_in_box_zoomed((unsigned char*)inventory_item_string,
			strlen(inventory_item_string), win->len_x - trade_border, items_string,
			win->current_scale);
		last_items_string_id = inventory_item_string_id;
	}
	draw_string_small_zoomed(trade_border/2, button_y_bot + trade_border, items_string,
		ITEM_INFO_ROWS, win->current_scale);

	if (tool_tip_str != NULL)
	{
		show_help(tool_tip_str, 0, win->len_y+10, win->current_scale);
		tool_tip_str = NULL;
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}


static int click_trade_handler(window_info *win, int mx, int my, Uint32 flags)
{
	Uint8 str[256];
	int left_click = flags & ELW_LEFT_MOUSE;
	int right_click = flags & ELW_RIGHT_MOUSE;
	int trade_quantity_storage_offset = 3; /* Offset of trade quantity in packet. Can be 3 or 4 */

	if ( !(left_click || right_click) ) return 0;

	if(right_click) {
		if ((item_dragged == -1) && (storage_item_dragged == -1) && is_gamewin_look_action()) {
			clear_gamewin_look_action();
			return 1;
		}
		if ((item_dragged != -1) || (storage_item_dragged != -1)) {
			item_dragged = storage_item_dragged = -1;
			return 1;
		}
	}

	if(left_click && item_dragged!=-1){
		str[0]=PUT_OBJECT_ON_TRADE;
		str[1]=ITEM_INVENTORY;
		str[2]=item_list[item_dragged].pos;
		*((Uint32 *)(str+3))= SDL_SwapLE32(item_quantity);
		my_tcp_send(my_socket,str,7);
		do_drop_item_sound();
		return 1;
	} else if(storage_available && left_click && storage_item_dragged!=-1){
		str[0]=PUT_OBJECT_ON_TRADE;
		str[1]=ITEM_BANK;
		if ( storage_items[storage_item_dragged].pos > 255 ) {
			*((Uint16 *)(str+2))= SDL_SwapLE16(storage_items[storage_item_dragged].pos);
			trade_quantity_storage_offset++; /* Offset is 1 byte ahead now */
		} else {
			str[2]=storage_items[storage_item_dragged].pos;
		}
		*((Uint32 *)(str+trade_quantity_storage_offset))= SDL_SwapLE32(item_quantity);
		my_tcp_send(my_socket,str, 4 + trade_quantity_storage_offset );
		do_drop_item_sound();
		return 1;
	} else if(mx > trade_border && mx < trade_border + ITEM_COLS * trade_gridsize &&
				my > trade_grid_start_y && my < trade_grid_start_y + ITEM_ROWS * trade_gridsize){
		int pos=get_mouse_pos_in_grid (mx, my, ITEM_COLS, ITEM_ROWS, trade_border, trade_grid_start_y, trade_gridsize, trade_gridsize);

		if (pos >= 0 && your_trade_list[pos].quantity)
		{
			if(right_click || is_gamewin_look_action()) {
				str[0]=LOOK_AT_TRADE_ITEM;
				str[1]=pos;
				str[2]=0;//your trade
				my_tcp_send(my_socket,str,3);
			} else {
				str[0]=REMOVE_OBJECT_FROM_TRADE;
				str[1]=pos;
				*((Uint32 *)(str+2))=SDL_SwapLE32(item_quantity);
				my_tcp_send(my_socket,str,6);
				do_drag_item_sound();
			}
		}

		return 1;
	} else if(mx > trade_border + (ITEM_COLS + 1) * trade_gridsize && mx < trade_border + (ITEM_COLS * 2 + 1) * trade_gridsize &&
				trade_grid_start_y && my < trade_grid_start_y + ITEM_ROWS * trade_gridsize){
		int pos=get_mouse_pos_in_grid(mx, my, ITEM_COLS, ITEM_ROWS, trade_border + (ITEM_COLS + 1) * trade_gridsize, trade_grid_start_y,
			trade_gridsize, trade_gridsize);

		if (pos >= 0 && others_trade_list[pos].quantity)
		{
			if(right_click || is_gamewin_look_action()){
				str[0]=LOOK_AT_TRADE_ITEM;
				str[1]=pos;
				str[2]=1;//their trade
				my_tcp_send(my_socket,str,3);
			} else if (left_click && storage_available){
				if(others_trade_list[pos].type==ITEM_BANK)
					others_trade_list[pos].type=ITEM_INVENTORY;
				else
					others_trade_list[pos].type=ITEM_BANK;
			}
		}

		return 1;
	} else if(mx > button_x_left && mx < button_x_right && my > button_y_top && my < button_y_bot) {
		//check to see if we hit the Accept box
		if(trade_you_accepted==2 || right_click){
			str[0]= REJECT_TRADE;
			my_tcp_send(my_socket, str, 1);
			do_click_sound();
		} else {
			str[0]= ACCEPT_TRADE;
			if(trade_you_accepted==1){
				int i;
				for(i=0;i<MAX_ITEMS;i++){
					str[i+1]=(others_trade_list[i].quantity>0)*others_trade_list[i].type;
				}
				trade_accepted(other_player_trade_name, your_trade_list, others_trade_list, MAX_ITEMS);
			}
			my_tcp_send(my_socket, str, MAX_ITEMS + 1);
			do_click_sound();
		}

		return 1;
	}

	else if (my > button_y_bot + trade_border) {
		static Uint32 last_click = 0;
		if (safe_button_click(&last_click)) {
			set_shown_string(0,"");
			return 1;
		}
	}

	return 1;
}

static int mouseover_trade_handler(window_info *win, int mx, int my)
{
	int check_for_eye = 0;
	int pos = -1;
	trade_item *over_item = NULL;

	if(mx > win->len_x - win->box_size && my < win->box_size)
		show_abort_help = 1;
	else
		show_abort_help = 0;

	if (show_help_text && *inventory_item_string && (my > button_y_bot + trade_border))
	{
		tool_tip_str = (disable_double_click)?click_clear_str :double_click_clear_str;
		return 0;
	}

	pos = get_mouse_pos_in_grid (mx, my, ITEM_COLS, ITEM_ROWS, trade_border, trade_grid_start_y, trade_gridsize, trade_gridsize);
	if (pos >= 0 && your_trade_list[pos].quantity)
	{
		over_item = &your_trade_list[pos];
		mouse_over_your_trade_pos = pos;
		check_for_eye = 1;
	}
	else {
		pos = get_mouse_pos_in_grid(mx, my, ITEM_COLS, ITEM_ROWS, trade_border + (ITEM_COLS + 1) * trade_gridsize, trade_grid_start_y,
			trade_gridsize, trade_gridsize);
		if (pos >= 0 && others_trade_list[pos].quantity)
		{
			over_item = &others_trade_list[pos];
			mouse_over_others_trade_pos = pos;
			check_for_eye = 1;
		}
	}

	if (show_item_desc_text && (over_item != NULL) && item_info_available() && (get_item_count(over_item->id, over_item->image_id) == 1))
		tool_tip_str = get_item_description(over_item->id, over_item->image_id);

	/* if we're over an occupied slot and the eye cursor function is active, show the eye cursor */
	if (check_for_eye && is_gamewin_look_action())
	{
		elwin_mouse = CURSOR_EYE;
		return 1;
	}

	return 0;
}

void get_trade_partner_name (const Uint8 *player_name, int len)
{
	int i;

	storage_available = player_name[0];

	for (i = 0; i+1 < len; i++)
		other_player_trade_name[i] = player_name[i+1];
	other_player_trade_name[i] = '\0';
}


void get_your_trade_objects (const Uint8 *data)
{
	int i;

	//clear the items first
	for(i = 0; i < MAX_ITEMS; i++)
		your_trade_list[i].quantity = 0;
	for(i = 0; i < MAX_ITEMS; i++)
		others_trade_list[i].quantity = 0;

	no_view_my_items=1;
	get_your_items(data);

	//reset the accepted flags too
	trade_you_accepted=0;
	trade_other_accepted=0;

	//we have to close the manufacture window, otherwise bad things can happen.
	hide_window_MW(MW_MANU);
	hide_window_MW(MW_SPELLS);
	//Open the inventory window
	display_items_menu();
	view_window(MW_TRADE);
}

void put_item_on_trade (const Uint8 *data)
{
	int pos;

	pos=data[7];
	if (pos >= MAX_ITEMS)
		return;
	if(!data[8])
	{
		your_trade_list[pos].image_id=SDL_SwapLE16(*((Uint16 *)(data)));
		your_trade_list[pos].quantity+=SDL_SwapLE32(*((Uint32 *)(data+2)));
		your_trade_list[pos].type=data[6];
		if (item_uid_enabled)
			your_trade_list[pos].id=SDL_SwapLE16(*((Uint16 *)(data+9)));
		else
			your_trade_list[pos].id=unset_item_uid;
	}
	else
	{
		others_trade_list[pos].image_id=SDL_SwapLE16(*((Uint16 *)(data)));
		others_trade_list[pos].quantity+=SDL_SwapLE32(*((Uint32 *)(data+2)));
		others_trade_list[pos].type=data[6];
		if (item_uid_enabled)
			others_trade_list[pos].id=SDL_SwapLE16(*((Uint16 *)(data+9)));
		else
			others_trade_list[pos].id=unset_item_uid;
	}
}

void remove_item_from_trade (const Uint8 *data)
{
	int pos;
	int quantity;

	pos=data[4];
	if (pos >= MAX_ITEMS)
		return;
	quantity=SDL_SwapLE32(*((Uint32 *)(data)));

	if(!data[5])
	{
		your_trade_list[pos].quantity-=quantity;
	}
	else
	{
		others_trade_list[pos].quantity-=quantity;
	}
}

static int ui_scale_trade_handler(window_info *win)
{
	int len_x = 0;
	int len_y = 0;

	last_items_string_id = 0;
	trade_border = (int)(0.5 + win->current_scale * 10);
	trade_gridsize = (int)(0.5 + win->current_scale * 33);
	trade_grid_start_y = trade_border + win->box_size;

	button_x_left = trade_border + trade_gridsize;
	button_x_right = button_x_left + (ITEM_COLS - 2) * trade_gridsize;
	button_y_top = trade_grid_start_y + trade_gridsize * ITEM_ROWS + trade_border;
	button_y_bot = button_y_top + win->default_font_len_y;

	len_x = (ITEM_COLS * 2 + 1) * trade_gridsize + 2 * trade_border;
	len_y = button_y_bot + win->small_font_len_y * ITEM_INFO_ROWS + trade_border * 1.5;

	resize_window(win->window_id, len_x, len_y);

	return 1;
}

static int close_trade_handler(window_info *win)
{
	// User click the close button, abort the trade
	unsigned char msg = EXIT_TRADE;
	my_tcp_send(my_socket, &msg, 1);
	return 1;
}

void display_trade_menu()
{
	int trade_win = get_id_MW(MW_TRADE);

	if(trade_win < 0){
		trade_win = create_window(win_trade, (not_on_top_now(MW_TRADE) ?game_root_win : -1), 0,
			get_pos_x_MW(MW_TRADE), get_pos_y_MW(MW_TRADE), 0, 0, ELW_USE_UISCALE|ELW_WIN_DEFAULT);
		set_id_MW(MW_TRADE, trade_win);

		set_window_custom_scale(trade_win, MW_TRADE);
		set_window_handler(trade_win, ELW_HANDLER_DISPLAY, &display_trade_handler );
		set_window_handler(trade_win, ELW_HANDLER_CLICK, &click_trade_handler );
		set_window_handler(trade_win, ELW_HANDLER_MOUSEOVER, &mouseover_trade_handler );
		set_window_handler(trade_win, ELW_HANDLER_CLOSE, &close_trade_handler );
		set_window_handler(trade_win, ELW_HANDLER_UI_SCALE, &ui_scale_trade_handler );

		if (trade_win >= 0 && trade_win < windows_list.num_windows)
			ui_scale_trade_handler(&windows_list.window[trade_win]);
		check_proportional_move(MW_TRADE);

	} else {
		show_window(trade_win);
		select_window(trade_win);
	}
}

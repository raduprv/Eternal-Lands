#include <stdlib.h>
#include "trade.h"
#include "asc.h"
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
#define ITEM_BANK 2

int trade_win=-1;

trade_item your_trade_list[24];
trade_item others_trade_list[24];
int trade_you_accepted=0;
int trade_other_accepted=0;
char other_player_trade_name[20];
static char items_string[350]={0};
static size_t last_items_string_id = 0;
int no_view_my_items=0;

int trade_menu_x=10;
int trade_menu_y=20;
int trade_menu_x_len=9*33+20;
int trade_menu_y_len=4*33+115;
static int button_y_top = 4*33+40;
static int button_y_bot = 4*33+60;
static const char *tool_tip_str = NULL;


int show_abort_help=0;

int storage_available=0;

int display_trade_handler(window_info *win)
{
	int x=10+33;
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
	
	draw_string_small(x+33-strlen(accept_str)*4, button_y_top+2, (unsigned char*)accept_str, 1);

	if(trade_other_accepted<=0){    // RED
		glColor3f(1.0f,0.0f,0.0f);
	} else if(trade_other_accepted==1){ // YELLOW
		glColor3f(1.0f,1.0f,0.0f);
	} else {    // all others default to GREEN
		glColor3f(0.0f,1.0f,0.0f);
	}
	
	draw_string_small(x+6*33-strlen(accept_str)*4, button_y_top+2, (unsigned char*)accept_str, 1);
	
	glColor3f(0.77f,0.57f,0.39f);	
	
	//Draw the trade session names
	draw_string_small(10+2*33-strlen(you_str)*4,11,(unsigned char*)you_str,1);
	draw_string_small(10+7*33-strlen(other_player_trade_name)*4,11,(unsigned char*)other_player_trade_name,1);

	//Draw the X for aborting the trade
	draw_string(win->len_x-(ELW_BOX_SIZE-4), 2, (unsigned char*)"X", 1);
	
	glColor3f(1.0f,1.0f,1.0f);
	
	//Now let's draw the goods on trade...
	
	for(i=16; i>=0; --i){
		if(your_trade_list[i].quantity){
			GLfloat u_start, v_start, u_end, v_end;
			int x_start, x_end, y_start, y_end;
			int cur_item;
			GLuint this_texture;

			cur_item=your_trade_list[i].image_id%25;
#ifdef	NEW_TEXTURES
			get_item_uv(cur_item, &u_start, &v_start, &u_end,
				&v_end);
#else	/* NEW_TEXTURES */
			u_start=0.2f*(cur_item%5);
			u_end=u_start+(float)50/255;
			v_start=(1.0f+((float)50/255)/255.0f)-((float)50/255*(cur_item/5));
			v_end=v_start-(float)50/255;
#endif	/* NEW_TEXTURES */
		
			this_texture=get_items_texture(your_trade_list[i].image_id/25);

#ifdef	NEW_TEXTURES
			if (this_texture != -1)
			{
				bind_texture(this_texture);
			}
#else	/* NEW_TEXTURES */
			if(this_texture!=-1) get_and_set_texture_id(this_texture);
#endif	/* NEW_TEXTURES */

			x_start=(i%4)*33+10;
			x_end=x_start+32;
			y_start=(i/4)*33+30;
			y_end=y_start+32;

			glBegin(GL_QUADS);
			draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();
			
			safe_snprintf(str, sizeof(str), "%i",your_trade_list[i].quantity);
			draw_string_small_shadowed(x_start,(i&1)?(y_end-12):(y_end-22),(unsigned char*)str,1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);
			//by doing the images in reverse, you can't cover up the digits>4
			//also, by offsetting each one, numbers don't overwrite each other:
			//before: 123456 in one box and 56 in the other could allow
			//        1234                  56  which looks legitimate
			//now:    123456
			//            56 - the odd/even numbers are staggered
			//                 stopping potential scams
		}
	}

	for(i=16; i>=0; --i){
		if(others_trade_list[i].quantity){
			GLfloat u_start, v_start, u_end, v_end;
			int x_start, x_end, y_start, y_end;
			int cur_item;
			GLuint this_texture;

			cur_item=others_trade_list[i].image_id%25;
#ifdef	NEW_TEXTURES
			get_item_uv(cur_item, &u_start, &v_start, &u_end,
				&v_end);
#else	/* NEW_TEXTURES */
			u_start=0.2f*(cur_item%5);
			u_end=u_start+(float)50/255;
			v_start=(1.0f+((float)50/255)/255.0f)-((float)50/255*(cur_item/5));
			v_end=v_start-(float)50/255;
#endif	/* NEW_TEXTURES */
		
			this_texture=get_items_texture(others_trade_list[i].image_id/25);

#ifdef	NEW_TEXTURES
			if (this_texture != -1)
			{
				bind_texture(this_texture);
			}
#else	/* NEW_TEXTURES */
			if(this_texture!=-1) get_and_set_texture_id(this_texture);
#endif	/* NEW_TEXTURES */

			x_start=(i%4)*33+10+5*33;
			x_end=x_start+32;
			y_start=(i/4)*33+30;
			y_end=y_start+32;

			glBegin(GL_QUADS);
			draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();
			
			safe_snprintf(str, sizeof(str), "%i",others_trade_list[i].quantity);
			draw_string_small_shadowed(x_start,(!(i&1))?(y_end-12):(y_end-22),(unsigned char*)str,1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);

			if(storage_available && others_trade_list[i].type==ITEM_BANK){
				str[0]='s';
				str[1]='t';
				str[2]='o';
				str[3]=0;
				draw_string_small_shadowed(x_start,y_start-1,(unsigned char*)str,1,1.0f,1.0f,1.0f,0.0f,0.0f,0.0f);
			}
		}
	}
	
	glDisable(GL_TEXTURE_2D);
	
	glColor3f(0.77f,0.57f,0.39f);	
	// grids for goods on trade
	rendergrid (4, 4, 10, 30, 33, 33);
	rendergrid (4, 4, 10+5*33, 30, 33, 33);

	// Accept buttons
	x=10+33;
	
	glBegin (GL_LINE_LOOP);
		glVertex3i(x,button_y_top,0);
		glVertex3i(x+66,button_y_top,0);
		glVertex3i(x+66,button_y_bot,0);
		glVertex3i(x,button_y_bot,0);
	glEnd ();
	
	x+=5*33;

	glBegin (GL_LINE_LOOP);
		glVertex3i(x,button_y_top,0);
		glVertex3i(x+66,button_y_top,0);
		glVertex3i(x+66,button_y_bot,0);
		glVertex3i(x,button_y_bot,0);
	glEnd ();


	//Draw the border for the "X"
	glBegin(GL_LINE_STRIP);
		glVertex3i(win->len_x, ELW_BOX_SIZE, 0);
		glVertex3i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE, 0);
		glVertex3i(win->len_x-ELW_BOX_SIZE, 0, 0);
	glEnd();
	
	//Draw the help text
	if(show_help_text && show_abort_help) show_help(abort_str, win->len_x-(ELW_BOX_SIZE-4)/2-strlen(abort_str)*4, 4+ELW_BOX_SIZE);

	glEnable(GL_TEXTURE_2D);
	
	//now, draw the inventory text, if any.
	if (last_items_string_id != inventory_item_string_id)
	{		
		put_small_text_in_box((unsigned char*)inventory_item_string, strlen(inventory_item_string), win->len_x-8, items_string);
		last_items_string_id = inventory_item_string_id;
	}
	draw_string_small(4,button_y_bot+5,(unsigned char*)items_string,3);

	if (tool_tip_str != NULL)
	{
		show_help(tool_tip_str, 0, win->len_y+10);
		tool_tip_str = NULL;
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 1;
}


int click_trade_handler(window_info *win, int mx, int my, Uint32 flags)
{
	Uint8 str[256];
	int left_click = flags & ELW_LEFT_MOUSE;
	int right_click = flags & ELW_RIGHT_MOUSE;
	int trade_quantity_storage_offset = 3; /* Offset of trade quantity in packet. Can be 3 or 4 */
	
	if ( !(left_click || right_click) ) return 0;

	if(left_click && mx>win->len_x-ELW_BOX_SIZE && my<ELW_BOX_SIZE){
		str[0]=EXIT_TRADE;
		my_tcp_send(my_socket,str,1);

		hide_window(trade_win);
		return 1;
	}
	
	if(right_click) {
		item_dragged=
		storage_item_dragged=-1;
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
	} else if(mx>10 && mx<10+4*33 && my>30 && my<30+4*33){
		int pos=get_mouse_pos_in_grid (mx, my, 4, 4, 10, 30, 33, 33);
		
		if (pos >= 0 && your_trade_list[pos].quantity)
		{
			if(action_mode==ACTION_LOOK || right_click) {
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
	} else if(mx>10+5*33 && mx<10+9*33 && my>30 && my<30+4*33){
		int pos=get_mouse_pos_in_grid(mx, my, 4, 4, 10+5*33, 30, 33, 33);

		if (pos >= 0 && others_trade_list[pos].quantity)
		{
			if(action_mode==ACTION_LOOK || right_click){
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
	} else if(mx>10+33 && mx<10+33+66 && my>button_y_top && my<button_y_bot) {
		//check to see if we hit the Accept box
		if(trade_you_accepted==2 || right_click){
			str[0]= REJECT_TRADE;
			my_tcp_send(my_socket, str, 1);
			do_click_sound();
		} else {
			str[0]= ACCEPT_TRADE;
			if(trade_you_accepted==1){
				int i;
				for(i=0;i<16;i++){
					str[i+1]=(others_trade_list[i].quantity>0)*others_trade_list[i].type;
				}
				trade_accepted(other_player_trade_name, your_trade_list, others_trade_list, 16);
			}
			my_tcp_send(my_socket, str, 17);
			do_click_sound();
		}
		
		return 1;
	}

	else if (my > button_y_bot+5) {
		static Uint32 last_click = 0;
		if (safe_button_click(&last_click)) {
			set_shown_string(0,"");
			return 1;
		}
	}

	return 1;
}

int mouseover_trade_handler(window_info *win, int mx, int my) 
{
	if(mx>win->len_x-ELW_BOX_SIZE && my<ELW_BOX_SIZE) show_abort_help=1;
	else show_abort_help=0;

	if (show_help_text && *inventory_item_string && (my > button_y_bot+5))
		tool_tip_str = (disable_double_click)?click_clear_str :double_click_clear_str;

	else if (show_item_desc_text) {
		trade_item *over_item = NULL;
		int pos=get_mouse_pos_in_grid (mx, my, 4, 4, 10, 30, 33, 33);
		if (pos >= 0 && your_trade_list[pos].quantity)
			over_item = &your_trade_list[pos];
		else {
			int pos=get_mouse_pos_in_grid(mx, my, 4, 4, 10+5*33, 30, 33, 33);
			if (pos >= 0 && others_trade_list[pos].quantity)
				over_item = &others_trade_list[pos];
		}
		if ((over_item != NULL) && item_info_available() && (get_item_count(over_item->id, over_item->image_id) == 1))
			tool_tip_str = get_item_description(over_item->id, over_item->image_id);
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
	for(i=0;i<16;i++)your_trade_list[i].quantity=0;
	for(i=0;i<16;i++)others_trade_list[i].quantity=0;

	no_view_my_items=1;
	get_your_items(data);

	//reset the accepted flags too
	trade_you_accepted=0;
	trade_other_accepted=0;

	//we have to close the manufacture window, otherwise bad things can happen.
	hide_window(manufacture_win);
	hide_window(sigil_win);
	//Open the inventory window
	display_items_menu();
	view_window(&trade_win, -1);
}

void put_item_on_trade (const Uint8 *data)
{
	int pos;

	pos=data[7];
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

void display_trade_menu()
{
	if(trade_win < 0){
		int our_root_win = -1;
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		trade_win= create_window(win_trade, our_root_win, 0, trade_menu_x, trade_menu_y, trade_menu_x_len, trade_menu_y_len, (ELW_WIN_DEFAULT& ~ELW_CLOSE_BOX));

		set_window_handler(trade_win, ELW_HANDLER_DISPLAY, &display_trade_handler );
		set_window_handler(trade_win, ELW_HANDLER_CLICK, &click_trade_handler );
		set_window_handler(trade_win, ELW_HANDLER_MOUSEOVER, &mouseover_trade_handler );
	} else {
		show_window(trade_win);
		select_window(trade_win);
	}
}

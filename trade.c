#include <stdlib.h>
#include "global.h"
#include "elwindows.h"
#include "string.h"

#define ITEM_INVENTORY 1
#define ITEM_BANK 2

int trade_win=-1;

trade_item your_trade_list[24];
trade_item others_trade_list[24];
int trade_you_accepted=0;
int trade_other_accepted=0;
char other_player_trade_name[20];

int no_view_my_items=0;

int trade_menu_x=10;
int trade_menu_y=20;
int trade_menu_x_len=9*33+20;
int trade_menu_y_len=4*33+100;

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
	
	draw_string_small(x+33-strlen(accept_str)*4, win->len_y-58, accept_str, 1);

	if(trade_other_accepted<=0){    // RED
		glColor3f(1.0f,0.0f,0.0f);
	} else if(trade_other_accepted==1){ // YELLOW
		glColor3f(1.0f,1.0f,0.0f);
	} else {    // all others default to GREEN
		glColor3f(0.0f,1.0f,0.0f);
	}
	
	draw_string_small(x+6*33-strlen(accept_str)*4, win->len_y-58, accept_str, 1);
	
	glColor3f(0.77f,0.57f,0.39f);	
	
	//Draw the trade session names
	draw_string_small(10+2*33-strlen(you_str)*4,11,you_str,1);
	draw_string_small(10+7*33-strlen(other_player_trade_name)*4,11,other_player_trade_name,1);

	//Draw the X for aborting the trade
	draw_string(win->len_x-(ELW_BOX_SIZE-4), 2, "X", 1);
	
	glColor3f(1.0f,1.0f,1.0f);
	
	//Now let's draw the goods on trade...
	
	for(i=16; i>=0; --i){
		if(your_trade_list[i].quantity){
			GLfloat u_start, v_start, u_end, v_end;
			int x_start, x_end, y_start, y_end;
			int cur_item;
			GLuint this_texture;

			cur_item=your_trade_list[i].image_id%25;
			u_start=0.2f*(cur_item%5);
			u_end=u_start+(float)50/255;
			v_start=(1.0f+((float)50/255)/255.0f)-((float)50/255*(cur_item/5));
			v_end=v_start-(float)50/255;
		
			this_texture=get_items_texture(your_trade_list[i].image_id/25);

			if(this_texture!=-1) get_and_set_texture_id(this_texture);

			x_start=(i%4)*33+10;
			x_end=x_start+32;
			y_start=(i/4)*33+30;
			y_end=y_start+32;

			glBegin(GL_QUADS);
			draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();
			
			safe_snprintf(str, sizeof(str), "%i",your_trade_list[i].quantity);
			draw_string_small(x_start,(i&1)?(y_end-12):(y_end-22),str,1);
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
			u_start=0.2f*(cur_item%5);
			u_end=u_start+(float)50/255;
			v_start=(1.0f+((float)50/255)/255.0f)-((float)50/255*(cur_item/5));
			v_end=v_start-(float)50/255;
		
			this_texture=get_items_texture(others_trade_list[i].image_id/25);

			if(this_texture!=-1) get_and_set_texture_id(this_texture);

			x_start=(i%4)*33+10+5*33;
			x_end=x_start+32;
			y_start=(i/4)*33+30;
			y_end=y_start+32;

			glBegin(GL_QUADS);
			draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();
			
			safe_snprintf(str, sizeof(str), "%i",others_trade_list[i].quantity);
			draw_string_small(x_start,(!(i&1))?(y_end-12):(y_end-22),str,1);

			if(storage_available && others_trade_list[i].type==ITEM_BANK){
				str[0]='s';
				str[1]='t';
				str[2]='o';
				str[3]=0;
				draw_string_small(x_start,y_start-1,str,1);
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
		glVertex3i(x,win->len_y-60,0);
		glVertex3i(x+66,win->len_y-60,0);
		glVertex3i(x+66,win->len_y-40,0);
		glVertex3i(x,win->len_y-40,0);
	glEnd ();
	
	x+=5*33;

	glBegin (GL_LINE_LOOP);
		glVertex3i(x,win->len_y-60,0);
		glVertex3i(x+66,win->len_y-60,0);
		glVertex3i(x+66,win->len_y-40,0);
		glVertex3i(x,win->len_y-40,0);
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
	draw_string_small(4,win->len_y-35,items_string,3);

	return 1;
}


int click_trade_handler(window_info *win, int mx, int my, Uint32 flags)
{
	Uint8 str[256];
	int left_click = flags & ELW_LEFT_MOUSE;
	int right_click = flags & ELW_RIGHT_MOUSE;
	
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
		return 1;
	} else if(storage_available && left_click && storage_item_dragged!=-1){
		str[0]=PUT_OBJECT_ON_TRADE;
		str[1]=ITEM_BANK;
		str[2]=storage_items[storage_item_dragged].pos;
		*((Uint32 *)(str+3))= SDL_SwapLE32(item_quantity);
		my_tcp_send(my_socket,str,7);
		return 1;
	} else if(mx>10 && mx<10+4*33 && my>10 && my<10+4*33){
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
			}
		}

		return 1;
	} else if(mx>10+5*33 && mx<10+9*33 && my>10 && my<10+4*33){
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
	} else if(mx>10+33 && mx<10+33+66 && my>win->len_y-60 && my<win->len_y-40) {
		//check to see if we hit the Accept box
		if(trade_you_accepted==2 || right_click){
			str[0]= REJECT_TRADE;
			my_tcp_send(my_socket, str, 1);
		} else {
			str[0]= ACCEPT_TRADE;
			if(trade_you_accepted==1){
				int i;
			
				for(i=0;i<16;i++){
					str[i+1]=(others_trade_list[i].quantity>0)*others_trade_list[i].type;
				}
			}
			my_tcp_send(my_socket, str, 17);
		}
		
		return 1;
	}

	return 1;
}

int mouseover_trade_handler(window_info *win, int mx, int my) 
{
	if(mx>win->len_x-ELW_BOX_SIZE && my<ELW_BOX_SIZE) show_abort_help=1;
	else show_abort_help=0;

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
	}
	else
	{
		others_trade_list[pos].image_id=SDL_SwapLE16(*((Uint16 *)(data)));
		others_trade_list[pos].quantity+=SDL_SwapLE32(*((Uint32 *)(data+2)));
		others_trade_list[pos].type=data[6];
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
		trade_win= create_window(win_trade, game_root_win, 0, trade_menu_x, trade_menu_y, trade_menu_x_len, trade_menu_y_len, (ELW_WIN_DEFAULT& ~ELW_CLOSE_BOX));

		set_window_handler(trade_win, ELW_HANDLER_DISPLAY, &display_trade_handler );
		set_window_handler(trade_win, ELW_HANDLER_CLICK, &click_trade_handler );
		set_window_handler(trade_win, ELW_HANDLER_MOUSEOVER, &mouseover_trade_handler );
	} else {
		show_window(trade_win);
		select_window(trade_win);
	}
}

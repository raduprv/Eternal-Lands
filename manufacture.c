#include <stdlib.h>
#include "global.h"
#include "elwindows.h"

item manu_recipe[6];
item manufacture_list[ITEM_NUM_ITEMS];
int	manufacture_win= -1;

int manufacture_menu_x=10;
int manufacture_menu_y=20;
int manufacture_menu_x_len=12*33+20;
int manufacture_menu_y_len=6*33;

void build_manufacture_list()
{
	int i,j,l;

	for(i=0;i<36+6;i++)manufacture_list[i].quantity=0;

	//ok, now see which items are resources
	j=0;
	for(i=0;i<ITEM_WEAR_START;i++) {
		if(item_list[i].quantity && item_list[i].is_resource) {
			manufacture_list[j].quantity=item_list[i].quantity;
			manufacture_list[j].image_id=item_list[i].image_id;
			manufacture_list[j].pos=item_list[i].pos;
			j++;
		}
	}
	//now check for all items in the current recipe
	l=1;
	for(i=0; l>0 && i<6; i++) {
		if(manu_recipe[i].quantity > 0) {
			for(j=0; l>0 && j<36; j++) {
				if(manufacture_list[j].quantity>0 && manu_recipe[i].image_id == manufacture_list[j].image_id) {
					if(manu_recipe[i].quantity > manufacture_list[j].quantity) {
						l=0;	// can't make
					}
					break;
				}
			}
			
			// watch for the item missing
			if(j >= 36) {
				l=0;
			}
		}
	}
	
	//all there? good, put them in
	if(l>0) {
		for(i=0; i<6; i++) {
			if(manu_recipe[i].quantity > 0)	{
				for(j=0;j<36;j++){
					if(manufacture_list[j].quantity>0 && manufacture_list[j].quantity>=manu_recipe[i].quantity && manufacture_list[j].image_id==manu_recipe[i].image_id){
						//found an empty space in the "production pipe"
						manufacture_list[j].quantity-=manu_recipe[i].quantity;
						manufacture_list[i+36].quantity=manu_recipe[i].quantity;
						manufacture_list[i+36].pos=manufacture_list[j].pos;
						manufacture_list[i+36].image_id=manufacture_list[j].image_id;
						break;
					}
				}
			} else {
				manufacture_list[i+36].quantity = 0;
			}
		}
	}
}

int	display_manufacture_handler(window_info *win)
{
	Uint8 str[80];
	int i;

	glColor3f(0.77f,0.57f,0.39f);
	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<36;i++) {
		if(manufacture_list[i].quantity) {
			float u_start,v_start,u_end,v_end;
			int this_texture,cur_item,cur_pos;
			int x_start,x_end,y_start,y_end;

			//get the UV coordinates.
			cur_item=manufacture_list[i].image_id%25;
			u_start=0.2f*(cur_item%5);
			u_end=u_start+(float)50/256;
			v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
			v_end=v_start-(float)50/256;

			//get the x and y
			cur_pos=i;

			x_start=33*(cur_pos%12)+1;
			x_end=x_start+32;
			y_start=33*(cur_pos/12);
			y_end=y_start+32;

			//get the texture this item belongs to
			this_texture=get_items_texture(manufacture_list[i].image_id/25);
			get_and_set_texture_id(this_texture);
			
			glBegin(GL_QUADS);
				draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();

			sprintf(str,"%i",manufacture_list[i].quantity);
			draw_string_small(x_start,y_end-15,str,1);
		}
	}

	//ok, now let's draw the mixed objects
	for(i=36;i<36+6;i++) {
		if(manufacture_list[i].quantity){
			float u_start,v_start,u_end,v_end;
			int this_texture,cur_item,cur_pos;
			int x_start,x_end,y_start,y_end;

			//get the UV coordinates.
			cur_item=manufacture_list[i].image_id%25;
			u_start=0.2f*(cur_item%5);
			u_end=u_start+(float)50/256;
			v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
			v_end=v_start-(float)50/256;


			//get the x and y
			cur_pos=i;

			x_start=33*(cur_pos%6)+5;
			x_end=x_start+32;
			y_start=win->len_y-37;
			y_end=y_start+32;

			//get the texture this item belongs to
			this_texture=get_items_texture(manufacture_list[i].image_id/25);
			get_and_set_texture_id(this_texture);
			
			glBegin(GL_QUADS);
				draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
			glEnd();

			sprintf(str,"%i",manufacture_list[i].quantity);
			draw_string_small(x_start,y_end-15,str,1);
		}
	}
	
	//now, draw the inventory text, if any.
	draw_string_small(4,win->len_y-85,items_string,4);

	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all 
	// cards
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	
	//draw the grid
	rendergrid(12,3,0,0,33,33);
	
	//Draw the bottom grid
	rendergrid(6,1,5, win->len_y-37, 33, 33);

	glEnable(GL_TEXTURE_2D);

	return 1;
}

int click_manufacture_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int pos;
	Uint8 str[100];

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	//see if we clicked on any item in the main category
	pos=get_mouse_pos_in_grid(mx, my, 12, 3, 0, 0, 33, 33);

	if (pos >= 0 && manufacture_list[pos].quantity)
	{
		if(action_mode==ACTION_LOOK || (flags&ELW_RIGHT_MOUSE)) {
			str[0]=LOOK_AT_INVENTORY_ITEM;
			str[1]=manufacture_list[pos].pos;
			my_tcp_send(my_socket,str,2);
			return 1;
		} else	{
			int j;
			
			for(j=36;j<36+6;j++)
				if(manufacture_list[j].pos==manufacture_list[pos].pos && manufacture_list[j].quantity){
					//found an empty space in the "production pipe"
					manufacture_list[j].quantity++;
					manufacture_list[j].pos=manufacture_list[pos].pos;
					manufacture_list[j].image_id=manufacture_list[pos].image_id;
					manufacture_list[pos].quantity--;
					return 1;
				}
			

			for(j=36;j<36+6;j++)
				if(!manufacture_list[j].quantity){
					//found an empty space in the "production pipe"
					manufacture_list[j].quantity++;
					manufacture_list[j].pos=manufacture_list[pos].pos;
					manufacture_list[j].image_id=manufacture_list[pos].image_id;
					manufacture_list[pos].quantity--;
					return 1;
				}
		}
	}

	pos=get_mouse_pos_in_grid(mx, my, 6, 1, 5, win->len_y-37, 33, 33);
	
	//see if we clicked on any item from the "production pipe"
	if (pos >= 0 && manufacture_list[36+pos].quantity)
	{
		if(action_mode==ACTION_LOOK || (flags&ELW_RIGHT_MOUSE)){
			str[0]=LOOK_AT_INVENTORY_ITEM;
			str[1]=manufacture_list[36+pos].pos;
			my_tcp_send(my_socket,str,2);
			return 1;
		} else {
			int j;
			for(j=0;j<36;j++)
				if(manufacture_list[j].quantity && manufacture_list[j].pos==manufacture_list[36+pos].pos) {
					//found an empty space in the "production pipe"
					manufacture_list[j].quantity++;
					manufacture_list[j].pos=manufacture_list[36+pos].pos;
					manufacture_list[j].image_id=manufacture_list[36+pos].image_id;
					manufacture_list[36+pos].quantity--;
					return 1;
				}

			for(j=0;j<36;j++)
				if(!manufacture_list[j].quantity) {
					//found an empty space in the "production pipe"
					manufacture_list[j].quantity++;
					manufacture_list[j].pos=manufacture_list[36+pos].pos;
					manufacture_list[j].image_id=manufacture_list[36+pos].image_id;
					manufacture_list[36+pos].quantity--;
					return 1;
				}
		}
	}

	return 0;
}

int clear_handler()
{
	int i;
	
	for(i=0; i<6; i++) manu_recipe[i].quantity= manu_recipe[i].image_id= 0; // clear the recipe
	build_manufacture_list();
	return 1;
}

int mix_handler()
{
	Uint8 str[20];
	int items_no=0;
	int i;

	str[0]=MANUFACTURE_THIS;
	for(i=36;i<36+6;i++){
		if(manufacture_list[i].quantity){
			str[items_no*3+2]=manufacture_list[i].pos;
			*((Uint16 *)(str+items_no*3+2+1))=SDL_SwapLE16(manufacture_list[i].quantity);
			items_no++;
		}
	}

	str[1]=items_no;
	if(items_no){
		//don't send an empty string
		my_tcp_send(my_socket,str,items_no*3+2);
		// and copy this recipe
		for(i=36;i<36+6;i++){
			manu_recipe[i-36]=manufacture_list[i];
		}
	}
		
	return 1;
}

void display_manufacture_menu()
{
	if(manufacture_win < 0){
		static int clear_button_id=100;
		static int mix_button_id=101;
		
		manufacture_win= create_window(win_manufacture, game_root_win, 0, manufacture_menu_x, manufacture_menu_y, manufacture_menu_x_len, manufacture_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(manufacture_win, ELW_HANDLER_DISPLAY, &display_manufacture_handler );
		set_window_handler(manufacture_win, ELW_HANDLER_CLICK, &click_manufacture_handler );

		mix_button_id=button_add_extended(manufacture_win, mix_button_id, NULL, 33*6+15, manufacture_menu_y_len-36, 0, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, mix_str);
		widget_set_OnClick(manufacture_win, mix_button_id, mix_handler);
		
		clear_button_id=button_add_extended(manufacture_win, clear_button_id, NULL, 33*9+8, manufacture_menu_y_len-36, 0, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, clear_str);
		widget_set_OnClick(manufacture_win, clear_button_id, clear_handler);
	} else {
		show_window(manufacture_win);
		select_window(manufacture_win);
	}
}


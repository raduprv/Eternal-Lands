#include "global.h"

void build_manufacture_list()
{
	int i,j;

	for(i=0;i<36+6;i++)manufacture_list[i].quantity=0;

	//ok, now see which items are resources
	j=0;
	for(i=0;i<36+6;i++)
		{
			if(item_list[i].quantity)
			if(item_list[i].pos<36)
			if(item_list[i].is_resource)
				{
					manufacture_list[j].quantity=item_list[i].quantity;
					manufacture_list[j].image_id=item_list[i].image_id;
					manufacture_list[j].pos=item_list[i].pos;
					j++;
				}
		}
}

void display_manufacture_menu()
{
	Uint8 str[80];
	int x,y,i;
	//first of all, draw the actual menu.

	draw_menu_title_bar(manufacture_menu_x,manufacture_menu_y-16,manufacture_menu_x_len);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(manufacture_menu_x,manufacture_menu_y+manufacture_menu_y_len,0);
	glVertex3i(manufacture_menu_x,manufacture_menu_y,0);
	glVertex3i(manufacture_menu_x+manufacture_menu_x_len,manufacture_menu_y,0);
	glVertex3i(manufacture_menu_x+manufacture_menu_x_len,manufacture_menu_y+manufacture_menu_y_len,0);
	glEnd();

	glDisable(GL_BLEND);

	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	glVertex3i(manufacture_menu_x,manufacture_menu_y,0);
	glVertex3i(manufacture_menu_x+manufacture_menu_x_len,manufacture_menu_y,0);

	glVertex3i(manufacture_menu_x+manufacture_menu_x_len,manufacture_menu_y,0);
	glVertex3i(manufacture_menu_x+manufacture_menu_x_len,manufacture_menu_y+manufacture_menu_y_len,0);

	glVertex3i(manufacture_menu_x+manufacture_menu_x_len,manufacture_menu_y+manufacture_menu_y_len,0);
	glVertex3i(manufacture_menu_x,manufacture_menu_y+manufacture_menu_y_len,0);

	glVertex3i(manufacture_menu_x,manufacture_menu_y+manufacture_menu_y_len,0);
	glVertex3i(manufacture_menu_x,manufacture_menu_y,0);

	//draw the grid
	for(y=1;y<4;y++)
		{
			glVertex3i(manufacture_menu_x,manufacture_menu_y+y*33,0);
			glVertex3i(manufacture_menu_x+12*33,manufacture_menu_y+y*33,0);
		}
	for(x=1;x<13;x++)
		{
			glVertex3i(manufacture_menu_x+x*33,manufacture_menu_y,0);
			glVertex3i(manufacture_menu_x+x*33,manufacture_menu_y+3*33,0);
		}

	//draw the bottom grid
	for(y=1;y<2;y++)
		{
			glVertex3i(manufacture_menu_x,manufacture_menu_y+y*5*33,0);
			glVertex3i(manufacture_menu_x+6*33,manufacture_menu_y+y*5*33,0);
		}
	for(x=1;x<7;x++)
		{
			glVertex3i(manufacture_menu_x+x*33,manufacture_menu_y+5*33,0);
			glVertex3i(manufacture_menu_x+x*33,manufacture_menu_y+6*33,0);
		}

	glColor3f(0.77f,0.57f,0.39f);
	//draw the corner, with the X in
	glVertex3i(manufacture_menu_x+manufacture_menu_x_len,manufacture_menu_y+20,0);
	glVertex3i(manufacture_menu_x+manufacture_menu_x_len-20,manufacture_menu_y+20,0);
	glVertex3i(manufacture_menu_x+manufacture_menu_x_len-20,manufacture_menu_y+20,0);
	glVertex3i(manufacture_menu_x+manufacture_menu_x_len-20,manufacture_menu_y,0);


	//draw the buttons frame
	//Mix button
	glVertex3i(manufacture_menu_x+33*6+40,manufacture_menu_y+manufacture_menu_y_len-30,0);
	glVertex3i(manufacture_menu_x+33*6+40+50,manufacture_menu_y+manufacture_menu_y_len-30,0);

	glVertex3i(manufacture_menu_x+33*6+40,manufacture_menu_y+manufacture_menu_y_len-10,0);
	glVertex3i(manufacture_menu_x+33*6+40+50,manufacture_menu_y+manufacture_menu_y_len-10,0);

	glVertex3i(manufacture_menu_x+33*6+40+50,manufacture_menu_y+manufacture_menu_y_len-30,0);
	glVertex3i(manufacture_menu_x+33*6+40+50,manufacture_menu_y+manufacture_menu_y_len-9,0);

	glVertex3i(manufacture_menu_x+33*6+40,manufacture_menu_y+manufacture_menu_y_len-30,0);
	glVertex3i(manufacture_menu_x+33*6+40,manufacture_menu_y+manufacture_menu_y_len-10,0);

	//Clear button
	glVertex3i(manufacture_menu_x+33*9+40,manufacture_menu_y+manufacture_menu_y_len-30,0);
	glVertex3i(manufacture_menu_x+33*9+40+70,manufacture_menu_y+manufacture_menu_y_len-30,0);

	glVertex3i(manufacture_menu_x+33*9+40,manufacture_menu_y+manufacture_menu_y_len-10,0);
	glVertex3i(manufacture_menu_x+33*9+40+70,manufacture_menu_y+manufacture_menu_y_len-10,0);

	glVertex3i(manufacture_menu_x+33*9+40+70,manufacture_menu_y+manufacture_menu_y_len-30,0);
	glVertex3i(manufacture_menu_x+33*9+40+70,manufacture_menu_y+manufacture_menu_y_len-9,0);

	glVertex3i(manufacture_menu_x+33*9+40,manufacture_menu_y+manufacture_menu_y_len-30,0);
	glVertex3i(manufacture_menu_x+33*9+40,manufacture_menu_y+manufacture_menu_y_len-10,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);

	draw_string(manufacture_menu_x+manufacture_menu_x_len-16,manufacture_menu_y+2,"X",1);
	draw_string(manufacture_menu_x+33*6+40+8,manufacture_menu_y+manufacture_menu_y_len-30+2,"Mix",1);
	draw_string(manufacture_menu_x+33*9+40+8,manufacture_menu_y+manufacture_menu_y_len-30+2,"Clear",1);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<36;i++)
		{
			if(manufacture_list[i].quantity)
				{
					float u_start,v_start,u_end,v_end;
					int this_texture,cur_item,cur_pos;
					int x_start,x_end,y_start,y_end;

					//get the UV coordinates.
					cur_item=manufacture_list[i].image_id%25;
					u_start=0.2f*(cur_item%5);
					u_end=u_start+0.2f;
					v_start=(1.0f+2.0f/256.0f)-(0.2f*(cur_item/5));
					v_end=v_start-0.2f;

					//get the x and y
					cur_pos=i;

					x_start=manufacture_menu_x+33*(cur_pos%12)+1;
					x_end=x_start+32;
					y_start=manufacture_menu_y+33*(cur_pos/12);
					y_end=y_start+32;

					//get the texture this item belongs to
					this_texture=manufacture_list[i].image_id/25;
					if(this_texture==0)this_texture=items_text_1;
					else
					if(this_texture==1)this_texture=items_text_2;
					else
					if(this_texture==2)this_texture=items_text_3;
					else
					if(this_texture==3)this_texture=items_text_4;
					else
					if(this_texture==4)this_texture=items_text_5;
					else
					if(this_texture==5)this_texture=items_text_6;

					if(last_texture!=texture_cache[this_texture].texture_id)
						{
							glBindTexture(GL_TEXTURE_2D, texture_cache[this_texture].texture_id);
							last_texture=texture_cache[this_texture].texture_id;
						}

					glBegin(GL_QUADS);
					draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
					glEnd();

					sprintf(str,"%i",manufacture_list[i].quantity);
					draw_string_small(x_start,y_end-15,str,1);
				}

		}

	//ok, now let's draw the mixed objects
	for(i=36;i<36+6;i++)
		{
			if(manufacture_list[i].quantity)
				{
					float u_start,v_start,u_end,v_end;
					int this_texture,cur_item,cur_pos;
					int x_start,x_end,y_start,y_end;

					//get the UV coordinates.
					cur_item=manufacture_list[i].image_id%25;
					u_start=0.2f*(cur_item%5);
					u_end=u_start+0.2f;
					v_start=(1.0f+2.0f/256.0f)-(0.2f*(cur_item/5));
					v_end=v_start-0.2f;

					//get the x and y
					cur_pos=i;

					x_start=manufacture_menu_x+33*(cur_pos%6)+1;
					x_end=x_start+32;
					y_start=manufacture_menu_y+33*5;
					y_end=y_start+32;

					//get the texture this item belongs to
					this_texture=manufacture_list[i].image_id/25;
					if(this_texture==0)this_texture=items_text_1;
					else
					if(this_texture==1)this_texture=items_text_2;
					else
					if(this_texture==2)this_texture=items_text_3;
					else
					if(this_texture==3)this_texture=items_text_4;
					else
					if(this_texture==4)this_texture=items_text_5;
					else
					if(this_texture==5)this_texture=items_text_6;

					if(last_texture!=texture_cache[this_texture].texture_id)
						{
							glBindTexture(GL_TEXTURE_2D, texture_cache[this_texture].texture_id);
							last_texture=texture_cache[this_texture].texture_id;
						}

					glBegin(GL_QUADS);
					draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
					glEnd();

					sprintf(str,"%i",manufacture_list[i].quantity);
					draw_string_small(x_start,y_end-15,str,1);
				}

		}
	//now, draw the inventory text, if any.
	draw_string_small(manufacture_menu_x+4,manufacture_menu_y+manufacture_menu_y_len-85,items_string,4);
	glColor3f(1.0f,1.0f,1.0f);
}

int check_manufacture_interface()
{
	int i,x,y;
	int x_screen,y_screen;
	Uint8 str[100];

	if(!view_manufacture_menu || mouse_x>manufacture_menu_x+manufacture_menu_x_len || mouse_x<manufacture_menu_x
	|| mouse_y<manufacture_menu_y || mouse_y>manufacture_menu_y+manufacture_menu_y_len)return 0;

	if(mouse_x>manufacture_menu_x+33*9+40 && mouse_x<manufacture_menu_x+33*9+40+70 &&
	mouse_y>manufacture_menu_y+manufacture_menu_y_len-30 && mouse_y<manufacture_menu_y+manufacture_menu_y_len-10)
		{
			build_manufacture_list();
			return 1;
		}

	if(mouse_x>manufacture_menu_x+33*6+40 && mouse_x<manufacture_menu_x+33*6+40+50 &&
	mouse_y>manufacture_menu_y+manufacture_menu_y_len-30 && mouse_y<manufacture_menu_y+manufacture_menu_y_len-10)
		{
			Uint8 str[20];
			int len;
			int items_no=0;

			str[0]=MANUFACTURE_THIS;
			for(i=36;i<36+6;i++)
			if(manufacture_list[i].quantity)
				{
					str[items_no*3+2]=manufacture_list[i].pos;
					*((Uint16 *)(str+items_no*3+2+1))=manufacture_list[i].quantity;
					items_no++;
				}

			str[1]=items_no;
			if(items_no)//don't send an empty string
			my_tcp_send(my_socket,str,items_no*3+2);
			return 1;
		}

	//see if we clicked on any item in the main category
	for(y=0;y<3;y++)
	for(x=0;x<12;x++)
		{
			x_screen=manufacture_menu_x+x*33;
			y_screen=manufacture_menu_y+y*33;
			if(mouse_x>x_screen && mouse_x<x_screen+33 && mouse_y>y_screen && mouse_y<y_screen+33)
				{

					//see if there is any item there
					//should we get the info for it?
					if(manufacture_list[y*12+x].quantity)
						{

							if(action_mode==action_look || right_click)
								{
									str[0]=LOOK_AT_INVENTORY_ITEM;
									str[1]=manufacture_list[y*12+x].pos;
									my_tcp_send(my_socket,str,2);
								}
							else
								{
									int j;
									if(manufacture_list[y*12+x].quantity)
										{
											for(j=36;j<36+6;j++)
											if(manufacture_list[j].pos==manufacture_list[y*12+x].pos && manufacture_list[j].quantity)
												{
													//found an empty space in the "production pipe"
													manufacture_list[j].quantity++;
													manufacture_list[j].pos=manufacture_list[y*12+x].pos;
													manufacture_list[j].image_id=manufacture_list[y*12+x].image_id;
													manufacture_list[y*12+x].quantity--;
													return 1;
												}

											for(j=36;j<36+6;j++)
											if(!manufacture_list[j].quantity)
												{
													//found an empty space in the "production pipe"
													manufacture_list[j].quantity++;
													manufacture_list[j].pos=manufacture_list[y*12+x].pos;
													manufacture_list[j].image_id=manufacture_list[y*12+x].image_id;
													manufacture_list[y*12+x].quantity--;
													return 1;
												}
										}
								}

							return 1;
							}
				}
		}

	//see if we clicked on any item from the "production pipe"
	for(x=0;x<6;x++)
		{
			x_screen=manufacture_menu_x+x*33;
			y_screen=manufacture_menu_y+5*33;
			if(mouse_x>x_screen && mouse_x<x_screen+33 && mouse_y>y_screen && mouse_y<y_screen+33)
				{

					//see if there is any item there
					//should we get the info for it?
					if(manufacture_list[36+x].quantity)
						{

							if(action_mode==action_look || right_click)
								{
									str[0]=LOOK_AT_INVENTORY_ITEM;
									str[1]=manufacture_list[36+x].pos;
									my_tcp_send(my_socket,str,2);
								}
							else
								{
									int j;
									if(manufacture_list[36+x].quantity)
										{
											for(j=0;j<36;j++)
											if(manufacture_list[j].quantity && manufacture_list[j].pos==manufacture_list[36+x].pos)
												{
													//found an empty space in the "production pipe"
													manufacture_list[j].quantity++;
													manufacture_list[j].pos=manufacture_list[36+x].pos;
													manufacture_list[j].image_id=manufacture_list[36+x].image_id;
													manufacture_list[36+x].quantity--;
													return 1;
												}

											for(j=0;j<36;j++)
											if(!manufacture_list[j].quantity)
												{
													//found an empty space in the "production pipe"
													manufacture_list[j].quantity++;
													manufacture_list[j].pos=manufacture_list[36+x].pos;
													manufacture_list[j].image_id=manufacture_list[36+x].image_id;
													manufacture_list[36+x].quantity--;
													return 1;
												}
										}
								}

							return 1;
							}
				}
		}

	return 1;

}

#include "global.h"
#include "elwindows.h"

item manu_recipe[6];
int	manufacture_win= 0;

void build_manufacture_list()
{
	int i,j,l;

	for(i=0;i<36+6;i++)manufacture_list[i].quantity=0;

	//ok, now see which items are resources
	j=0;
	for(i=0;i<ITEM_NUM_ITEMS;i++)
		{
			if(item_list[i].quantity)
				if(item_list[i].pos<ITEM_WEAR_START)
					if(item_list[i].is_resource)
						{
							manufacture_list[j].quantity=item_list[i].quantity;
							manufacture_list[j].image_id=item_list[i].image_id;
							manufacture_list[j].pos=item_list[i].pos;
							j++;
						}
		}
	//now check for all items in the current recipe
	l=1;
	for(i=0; l>0 && i<6; i++)
		{
			if(manu_recipe[i].quantity > 0)
				{
					for(j=0; l>0 && j<36; j++)
						{
							if(manufacture_list[j].quantity>0 && manu_recipe[i].image_id == manufacture_list[j].image_id)
								{
									if(manu_recipe[i].quantity > manufacture_list[j].quantity)
										{
											l=0;	// can't make
										}
										break;
								}
						}
					// watch for the item missing
					if(j >= 36)
						{
							l=0;
						}
				}
		}
	//all there? good, put them in
	if(l>0)
		{
			for(i=0; i<6; i++)
				{
					if(manu_recipe[i].quantity > 0)
						{
							for(j=0;j<36;j++)
								{
									if(manufacture_list[j].quantity>0 && manufacture_list[j].quantity>=manu_recipe[i].quantity && manufacture_list[j].image_id==manu_recipe[i].image_id)
										{
											//found an empty space in the "production pipe"
											manufacture_list[j].quantity-=manu_recipe[i].quantity;
											manufacture_list[i+36].quantity=manu_recipe[i].quantity;
											manufacture_list[i+36].pos=manufacture_list[j].pos;
											manufacture_list[i+36].image_id=manufacture_list[j].image_id;
											break;
										}
								}
						}
					else
						{
							manufacture_list[i+36].quantity = 0;
						}
				}
		}
}

int	display_manufacture_handler(window_info *win)
{
	Uint8 str[80];
	int x,y,i;
	//first of all, draw the actual menu.

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	//draw the grid
	for(y=1;y<4;y++)
		{
			glVertex3i(0,y*33,0);
			glVertex3i(12*33,y*33,0);
		}
	for(x=1;x<13;x++)
		{
			glVertex3i(x*33,0,0);
			glVertex3i(x*33,3*33,0);
		}

	//draw the bottom grid
	for(y=1;y<2;y++)
		{
			glVertex3i(0,y*5*33,0);
			glVertex3i(6*33,y*5*33,0);
		}
	for(x=1;x<7;x++)
		{
			glVertex3i(x*33,5*33,0);
			glVertex3i(x*33,6*33,0);
		}

	//draw the buttons frame
	//Mix button
	glVertex3i(33*6+40,win->len_y-30,0);
	glVertex3i(33*6+40+50,win->len_y-30,0);

	glVertex3i(33*6+40,win->len_y-10,0);
	glVertex3i(33*6+40+50,win->len_y-10,0);

	glVertex3i(33*6+40+50,win->len_y-30,0);
	glVertex3i(33*6+40+50,win->len_y-9,0);

	glVertex3i(33*6+40,win->len_y-30,0);
	glVertex3i(33*6+40,win->len_y-10,0);

	//Clear button
	glVertex3i(33*9+40,win->len_y-30,0);
	glVertex3i(33*9+40+70,win->len_y-30,0);

	glVertex3i(33*9+40,win->len_y-10,0);
	glVertex3i(33*9+40+70,win->len_y-10,0);

	glVertex3i(33*9+40+70,win->len_y-30,0);
	glVertex3i(33*9+40+70,win->len_y-9,0);

	glVertex3i(33*9+40,win->len_y-30,0);
	glVertex3i(33*9+40,win->len_y-10,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);

	draw_string(33*6+40+8,win->len_y-30+2,mix_str,1);
	draw_string(33*9+40+8,win->len_y-30+2,clear_str,1);

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
					this_texture=manufacture_list[i].image_id/25;
					switch(this_texture) {
					case 0:
						this_texture=items_text_1;break;
					case 1:
						this_texture=items_text_2;break;
					case 2:
						this_texture=items_text_3;break;
					case 3:
						this_texture=items_text_4;break;
					case 4:
						this_texture=items_text_5;break;
					case 5:
						this_texture=items_text_6;break;
					case 6:
						this_texture=items_text_7;break;
					case 7:
						this_texture=items_text_8;break;
					case 8:
						this_texture=items_text_9;break;
					}

					get_and_set_texture_id(this_texture);
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
					u_end=u_start+(float)50/256;
					v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
					v_end=v_start-(float)50/256;


					//get the x and y
					cur_pos=i;

					x_start=33*(cur_pos%6)+1;
					x_end=x_start+32;
					y_start=33*5;
					y_end=y_start+32;

					//get the texture this item belongs to
					this_texture=manufacture_list[i].image_id/25;
					if(this_texture==0)this_texture=items_text_1;
					else if(this_texture==1)this_texture=items_text_2;
					else if(this_texture==2)this_texture=items_text_3;
					else if(this_texture==3)this_texture=items_text_4;
					else if(this_texture==4)this_texture=items_text_5;
					else if(this_texture==5)this_texture=items_text_6;
					else if(this_texture==6)this_texture=items_text_7;
					else if(this_texture==7)this_texture=items_text_8;
					else if(this_texture==8)this_texture=items_text_9;

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
	glColor3f(1.0f,1.0f,1.0f);
	return 1;
}

int click_manufacture_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i,x,y;
	int x_screen,y_screen;
	Uint8 str[100];

	//Clear
	if(mx>33*9+40 && mx<33*9+40+70 &&
	   my>win->len_y-30 && my<win->len_y-10)
		{
			for(i=0; i<6; i++) manu_recipe[i].quantity= manu_recipe[i].image_id= 0; // clear the recipe
			build_manufacture_list();
			return 1;
		}

	//Mix
	if(mx>33*6+40 && mx<33*6+40+50 &&
	   my>win->len_y-30 && my<win->len_y-10)
		{
			Uint8 str[20];
			int items_no=0;

			str[0]=MANUFACTURE_THIS;
			for(i=36;i<36+6;i++)
				{
					if(manufacture_list[i].quantity)
						{
							str[items_no*3+2]=manufacture_list[i].pos;
							*((Uint16 *)(str+items_no*3+2+1))=manufacture_list[i].quantity;
							items_no++;
						}
				}

			str[1]=items_no;
			if(items_no)
				{
					//don't send an empty string
					my_tcp_send(my_socket,str,items_no*3+2);
					// and copy this recipe
					for(i=36;i<36+6;i++)
						{
							manu_recipe[i-36]=manufacture_list[i];
						}
				}
			return 1;
		}

	//see if we clicked on any item in the main category
	for(y=0;y<3;y++)
		for(x=0;x<12;x++)
			{
				x_screen=x*33;
				y_screen=y*33;
				if(mx>x_screen && mx<x_screen+33 && my>y_screen && my<y_screen+33)
					{
						//see if there is any item there
						//should we get the info for it?
						if(manufacture_list[y*12+x].quantity)
							{

								if(action_mode==action_look || (flags&ELW_RIGHT_MOUSE))
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
			x_screen=x*33;
			y_screen=5*33;
			if(mx>x_screen && mx<x_screen+33 && my>y_screen && my<y_screen+33)
				{
					//see if there is any item there
					//should we get the info for it?
					if(manufacture_list[36+x].quantity)
						{

							if(action_mode==action_look || (flags&ELW_RIGHT_MOUSE))
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
	return 0;
}

void display_manufacture_menu()
{
	if(manufacture_win <= 0){
		manufacture_win= create_window("Manufacture", 0, 0, manufacture_menu_x, manufacture_menu_y, manufacture_menu_x_len, manufacture_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(manufacture_win, ELW_HANDLER_DISPLAY, &display_manufacture_handler );
		set_window_handler(manufacture_win, ELW_HANDLER_CLICK, &click_manufacture_handler );
	} else {
		show_window(manufacture_win);
		select_window(manufacture_win);
	}
	display_window(manufacture_win);
}


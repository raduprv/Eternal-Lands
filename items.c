#include <string.h>
#include "global.h"
#include "elwindows.h"

item item_list[ITEM_NUM_ITEMS];
item manufacture_list[ITEM_NUM_ITEMS];
ground_item ground_item_list[50];
bag bag_list[200];

item inventory_trade_list[ITEM_WEAR_START];
item your_trade_list[24];
item others_trade_list[24];
int trade_you_accepted=0;
int trade_other_accepted=0;
char other_player_trade_name[20];

void strap_word(char * in, char * out)
{
	int i = 3;
	while(i--) *out++=*in++;
	while(*in==' ')in++;
	*out++='\n';
	i=3;
	while(i--) *out++=*in++;
	*out=0;
}

int view_ground_items=0;
int no_view_my_items=0;

int items_win= 0;
int items_menu_x=10;
int items_menu_y=20;
int items_menu_x_len=6*51+110;
int items_menu_y_len=6*51+60;
//int items_menu_dragged=0;

int ground_items_win= 0;
int ground_items_menu_x=6*51+100+20;
int ground_items_menu_y=20;
int ground_items_menu_x_len=6*33;
int ground_items_menu_y_len=10*33;
//int ground_items_menu_dragged=0;

int manufacture_menu_x=10;
int manufacture_menu_y=20;
int manufacture_menu_x_len=12*33+20;
int manufacture_menu_y_len=6*33;
//int manufacture_menu_dragged=0;

int trade_menu_x=10;
int trade_menu_y=20;
int trade_menu_x_len=13*33;
int trade_menu_y_len=11*33;
//int trade_menu_dragged=0;

int options_menu_x=220;
int options_menu_y=50;
int options_menu_x_len=390;
int options_menu_y_len=300;
//int options_menu_dragged=0;

int items_text_1;
int items_text_2;
int items_text_3;
int items_text_4;
int items_text_5;
int items_text_6;
int items_text_7;
int items_text_8;
int items_text_9;

char items_string[300];
int item_dragged=-1;
int item_quantity=1;

int wear_items_x_offset=6*51+20;
int wear_items_y_offset=30;

int display_items_handler(window_info *win)
{
	Uint8 str[80];
	int x,y,i;
	int item_is_weared=0;

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);

	//draw the grid
	for(y=1;y<7;y++)
		{
			glVertex3i(0,y*51,0);
			glVertex3i(6*51,y*51,0);
		}
	for(x=1;x<7;x++)
		{
			glVertex3i(x*51,0,0);
			glVertex3i(x*51,6*51,0);
		}

	glColor3f(0.57f,0.67f,0.49f);
	//draw the small grid
	for(y=0;y<5;y++)
		{
			glVertex3i(wear_items_x_offset,wear_items_y_offset+y*33,0);
			glVertex3i(wear_items_x_offset+2*33,wear_items_y_offset+y*33,0);
		}
	for(x=0;x<3;x++)
		{
			glVertex3i(wear_items_x_offset+x*33,wear_items_y_offset,0);
			glVertex3i(wear_items_x_offset+x*33,wear_items_y_offset+4*33,0);
		}

	//now, draw the quantity boxes
	glColor3f(0.3f,0.5f,1.0f);
	for(y=0;y<6;y++)
		{
			glVertex3i(wear_items_x_offset,wear_items_y_offset+160+y*20,0);
			glVertex3i(wear_items_x_offset+2*35,wear_items_y_offset+160+y*20,0);
		}
	for(x=0;x<3;x++)
		{
			glVertex3i(wear_items_x_offset+x*35,wear_items_y_offset+160,0);
			glVertex3i(wear_items_x_offset+x*35,wear_items_y_offset+160+5*20,0);
		}

	glEnd();
	glEnable(GL_TEXTURE_2D);

	//draw the quantity string
	draw_string_small(wear_items_x_offset,wear_items_y_offset+145,"Quantity",1);
	//draw the quantity values
	if(item_quantity==1)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(wear_items_x_offset+15,wear_items_y_offset+163,"1",1);
	if(item_quantity==5)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(wear_items_x_offset+50,wear_items_y_offset+163,"5",1);
	if(item_quantity==10)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(wear_items_x_offset+10,wear_items_y_offset+183,"10",1);
	if(item_quantity==20)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(wear_items_x_offset+45,wear_items_y_offset+183,"20",1);
	if(item_quantity==50)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(wear_items_x_offset+10,wear_items_y_offset+203,"50",1);
	if(item_quantity==100)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(wear_items_x_offset+40,wear_items_y_offset+203,"100",1);
	if(item_quantity==200)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(wear_items_x_offset+5,wear_items_y_offset+223,"200",1);
	if(item_quantity==500)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(wear_items_x_offset+40,wear_items_y_offset+223,"500",1);
	if(item_quantity==1000)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(wear_items_x_offset+1,wear_items_y_offset+243,"1000",1);
	if(item_quantity==2000)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(wear_items_x_offset+36,wear_items_y_offset+243,"2000",1);
	glColor3f(0.77f,0.57f,0.39f);
	//draw_string(items_menu_x_len-16,2,"X",1);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<ITEM_NUM_ITEMS;i++)
		{
			if(item_list[i].quantity)
				{
					float u_start,v_start,u_end,v_end;
					int this_texture,cur_item,cur_pos;
					int x_start,x_end,y_start,y_end;

					//get the UV coordinates.
					cur_item=item_list[i].image_id%25;
					u_start=0.2f*(cur_item%5);
					u_end=u_start+(float)50/256;
					v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
					v_end=v_start-(float)50/256;

					//get the x and y
					cur_pos=item_list[i].pos;
					if(cur_pos>=ITEM_WEAR_START)//the items we 'wear' are smaller
						{
							cur_pos-=ITEM_WEAR_START;
							item_is_weared=1;
							x_start=wear_items_x_offset+33*(cur_pos%2)+1;
							x_end=x_start+32;
							y_start=wear_items_y_offset+33*(cur_pos/2);
							y_end=y_start+32;
						}
					else
						{
							item_is_weared=0;
							x_start=51*(cur_pos%6)+1;
							x_end=x_start+50;
							y_start=51*(cur_pos/6);
							y_end=y_start+50;
						}

					//get the texture this item belongs to
					this_texture=item_list[i].image_id/25;
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
					if(!item_is_weared)
						{
							sprintf(str,"%i",item_list[i].quantity);
							draw_string_small(x_start,y_end-15,str,1);
						}
				}
		}
	//now, draw the inventory text, if any.
	draw_string_small(4,win->len_y-59,items_string,4);

	glColor3f(1.0f,1.0f,1.0f);
	//draw the load string
	sprintf(str,"%s: %i/%i",attributes.carry_capacity.shortname,your_info.carry_capacity.cur,your_info.carry_capacity.base);
	draw_string_small(6*51+4-((strlen(str)-4)*8),6*51+44,str,1);
	return 1;
}



void get_your_items(Uint8 *data)
{
	int i,total_items;
	Uint8 flags;

	total_items=data[0];

	//clear the item string we might have left from a previous session
	items_string[0]=0;

	//clear the items first
	for(i=0;i<ITEM_NUM_ITEMS;i++)
		{
			item_list[i].quantity=0;
		}
	for(i=0;i<total_items;i++)
		{
			item_list[i].image_id=*((Uint16 *)(data+i*8+1));
			item_list[i].quantity=*((Uint32 *)(data+i*8+1+2));
			item_list[i].pos=data[i*8+1+6];
			flags=data[i*8+1+7];

			if((flags&ITEM_RESOURCE))item_list[i].is_resource=1;
			else item_list[i].is_resource=0;
			if((flags&ITEM_REAGENT))item_list[i].is_reagent=1;
			else item_list[i].is_reagent=0;
			if((flags&ITEM_INVENTORY_USABLE))item_list[i].use_with_inventory=1;
			else item_list[i].use_with_inventory=0;
		}
	build_manufacture_list();
}


int click_items_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i,x,y;
	int x_screen,y_screen;
	Uint8 str[100];

	//see if we changed the quantity
	for(y=0;y<5;y++)
		for(x=0;x<2;x++)
			{
				x_screen=wear_items_x_offset+x*35;
				y_screen=wear_items_y_offset+160+y*20;
				if(mx>x_screen && mx<x_screen+35 && my>y_screen && my<y_screen+20)
					{
						if(x==0 && y==0)item_quantity=1;
						else if(x==1 && y==0)item_quantity=5;
						else if(x==0 && y==1)item_quantity=10;
						else if(x==1 && y==1)item_quantity=20;
						else if(x==0 && y==2)item_quantity=50;
						else if(x==1 && y==2)item_quantity=100;
						else if(x==0 && y==3)item_quantity=200;
						else if(x==1 && y==3)item_quantity=500;
						else if(x==0 && y==4)item_quantity=1000;
						else if(x==1 && y==4)item_quantity=2000;
					}
			}



	//see if we clicked on any item in the main category
	for(y=0;y<6;y++)
		for(x=0;x<6;x++)
			{
				x_screen=x*51;
				y_screen=y*51;
				if(mx>x_screen && mx<x_screen+51 && my>y_screen && my<y_screen+51)
					{
						//see if there is an empty space to drop this item over.
						if(item_dragged!=-1)//we have to drop this item
							{
								int any_item=0;
								for(i=0;i<ITEM_NUM_ITEMS;i++)
									{
										if(item_list[i].quantity && item_list[i].pos==y*6+x)
											{
												any_item=1;
												if(item_dragged==i)//drop the item only over itself
													item_dragged=-1;
												return 1;
											}
									}
								if(!any_item)
									{
										//send the drop info to the server
										str[0]=MOVE_INVENTORY_ITEM;
										str[1]=item_list[item_dragged].pos;
										str[2]=y*6+x;
										my_tcp_send(my_socket,str,3);
										item_dragged=-1;
										return 1;
									}
							}

						//see if there is any item there

						for(i=0;i<ITEM_NUM_ITEMS;i++)
							{
								//should we get the info for it?
								if(item_list[i].quantity && item_list[i].pos==y*6+x)
									{
										if(ctrl_on){
											str[0]=DROP_ITEM;
											str[1]=item_list[i].pos;
											*((Uint16 *)(str+2))=item_list[i].quantity;
											my_tcp_send(my_socket, str, 4);
											return 1;
										}
										if(action_mode==action_look || right_click)
											{
												if(cur_time<(click_time+click_speed))
													if(item_list[i].use_with_inventory)
														{
															str[0]=USE_INVENTORY_ITEM;
															str[1]=item_list[i].pos;
															my_tcp_send(my_socket,str,2);
															return 1;
														}
												click_time=cur_time;
												str[0]=LOOK_AT_INVENTORY_ITEM;
												str[1]=item_list[i].pos;
												my_tcp_send(my_socket,str,2);
											}
										else if(action_mode==action_use)
											{
												if(item_list[i].use_with_inventory)
													{
														str[0]=USE_INVENTORY_ITEM;
														str[1]=item_list[i].pos;
														my_tcp_send(my_socket,str,2);
														return 1;
													}
												return 1;
											}
										else//we might test for other things first, like use or drop
											{
												if(item_dragged==-1)//we have to drag this item
													{
														item_dragged=i;
													}
											}

										return 1;
									}
							}
					}
			}

	//see if we clicked on any item in the wear category
	for(y=0;y<4;y++)
		for(x=0;x<2;x++)
			{
				x_screen=wear_items_x_offset+x*33;
				y_screen=wear_items_y_offset+y*33;
				if(mx>x_screen && mx<x_screen+33 && my>y_screen && my<y_screen+33)
					{
						//see if there is any item there
						//see if there is an empty space to drop this item over.
						if(item_dragged!=-1)//we have to drop this item
							{
								int any_item=0;
								for(i=0;i<ITEM_NUM_ITEMS;i++)
									{
										if(item_list[i].quantity && item_list[i].pos==ITEM_WEAR_START+y*2+x)
											{
												any_item=1;
												if(item_dragged==i)//drop the item only over itself
													item_dragged=-1;
												return 1;
											}
									}
								if(!any_item)
									{
										Uint8 str[20];
										//send the drop info to the server
										str[0]=MOVE_INVENTORY_ITEM;
										str[1]=item_list[item_dragged].pos;
										str[2]=ITEM_WEAR_START+y*2+x;
										my_tcp_send(my_socket,str,3);
										item_dragged=-1;
										return 1;
									}
							}

						for(i=0;i<ITEM_NUM_ITEMS;i++)
							{
								//should we get the info for it?
								if(item_list[i].quantity && item_list[i].pos==y*2+x+ITEM_WEAR_START)
									{
										if(action_mode==action_look || right_click)
											{
												str[0]=LOOK_AT_INVENTORY_ITEM;
												str[1]=item_list[i].pos;
												my_tcp_send(my_socket,str,2);
											}
										else//we might test for other things first, like use or drop
											{
												if(item_dragged==-1)//we have to drag this item
													{
														item_dragged=i;
													}
											}
										return 1;
									}
							}
					}
			}

	return 1;

}



void drag_item()
{
	float u_start,v_start,u_end,v_end;
	int cur_item,this_texture;

	cur_item=item_list[item_dragged].image_id%25;
	u_start=0.2f*(cur_item%5);
	u_end=u_start+(float)50/256;
	v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
	v_end=v_start-(float)50/256;

	//get the texture this item belongs to
	this_texture=item_list[item_dragged].image_id/25;
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
	draw_2d_thing(u_start,v_start,u_end,v_end,mouse_x-25,mouse_y-25,mouse_x+25,mouse_y+25);
	glEnd();
}


void remove_item_from_inventory(int pos)
{
	int i;
	for(i=0;i<ITEM_NUM_ITEMS;i++)
		{
			if(item_list[i].quantity)
				if(item_list[i].pos==pos)
					{
						item_list[i].quantity=0;
						build_manufacture_list();
						return;
					}
		}
}

void remove_item_from_ground(Uint8 pos)
{
	ground_item_list[pos].quantity= 0;
}

void get_new_inventory_item(Uint8 *data)
{
	int i;
	int pos;
	Uint8 flags;
	int quantity;
	int image_id;

	pos= data[6];
	flags= data[7];
	image_id=*((Uint16 *)(data));
	quantity=*((Uint32 *)(data+2));

	//first, try to see if the items already exists, and replace it
	for(i=0;i<ITEM_NUM_ITEMS;i++)
		{
			if(item_list[i].quantity)
				if(item_list[i].pos==pos)
					{
						item_list[i].image_id=image_id;
						item_list[i].quantity=quantity;
						item_list[i].pos=pos;
						build_manufacture_list();
						return;
					}
		}

	for(i=0;i<ITEM_NUM_ITEMS;i++)
		{
			if(!item_list[i].quantity)
				{
					item_list[i].image_id=image_id;
					item_list[i].quantity=quantity;
					item_list[i].pos=pos;
					if((flags&ITEM_RESOURCE))item_list[i].is_resource=1;
					else item_list[i].is_resource=0;
					if((flags&ITEM_REAGENT))item_list[i].is_reagent=1;
					else item_list[i].is_reagent=0;
					if((flags&ITEM_INVENTORY_USABLE))item_list[i].use_with_inventory=1;
					else item_list[i].use_with_inventory=0;

					build_manufacture_list();
					return;
				}
		}

}






int display_ground_items_handler(window_info *win)
{
	Uint8 str[80];
	Uint8 my_str[10];
	int x,y,i;

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	//draw the grid
	for(y=1;y<11;y++)
		{
			glVertex3i(0,y*33,0);
			glVertex3i(5*33,y*33,0);
		}
	for(x=1;x<6;x++)
		{
			glVertex3i(x*33,0,0);
			glVertex3i(x*33,10*33,0);
		}
	// draw the "get all" box
	glVertex3i(win->len_x, 20,0);
	glVertex3i(win->len_x-33, 20,0);
	glVertex3i(win->len_x-33, 20,0);
	glVertex3i(win->len_x-33, 53,0);
	glVertex3i(win->len_x-33, 53,0);
	glVertex3i(win->len_x, 53,0);
	glVertex3i(win->len_x, 53,0);
	glVertex3i(win->len_x, 20,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);

	// write "get all" in the "get all" box :)
	strap_word(get_all_str,my_str);
	draw_string_small(win->len_x-28, 23, my_str, 2);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<50;i++)
		{
			if(ground_item_list[i].quantity > 0)
				{
					float u_start,v_start,u_end,v_end;
					int this_texture,cur_item,cur_pos;
					int x_start,x_end,y_start,y_end;

					//get the UV coordinates.
					cur_item=ground_item_list[i].image_id%25;
					u_start=0.2f*(cur_item%5);
					u_end=u_start+0.2f;
					v_start=(1.0f+2.0f/256.0f)-(0.2f*(cur_item/5));
					v_end=v_start-0.2f;

					//get the x and y
					cur_pos=i;
					x_start=33*(cur_pos%5)+1;
					x_end=x_start+32;
					y_start=33*(cur_pos/5);
					y_end=y_start+32;

					//get the texture this item belongs to
					this_texture=ground_item_list[i].image_id/25;
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
					sprintf(str,"%i",ground_item_list[i].quantity);
					draw_string_small(x_start,y_end-15,str,1);
				}
		}
	glColor3f(1.0f,1.0f,1.0f);
	return 1;
}


//do the flags later on
void get_bag_item(Uint8 *data)
{
	int	pos;
	pos= data[6];

	ground_item_list[pos].image_id= *((Uint16 *)(data));
	ground_item_list[pos].quantity= *((Uint32 *)(data+2));
	ground_item_list[pos].pos= pos;
}

//put the flags later on
void get_bags_items_list(Uint8 *data)
{
	Uint16 items_no;
	int i;
	int pos;
	int my_offset;

	view_ground_items=1;
	draw_pick_up_menu();
	if(item_window_on_drop)
		{
			display_items_menu();
		}
	//clear the list
	for(i=0;i<50;i++) ground_item_list[i].quantity=0;

	items_no=data[0];
	for(i=0;i<items_no;i++)
		{
			my_offset= i*7+1;
			pos= data[my_offset+6];
			ground_item_list[pos].image_id= *((Uint16 *)(data+my_offset));
			ground_item_list[pos].quantity= *((Uint32 *)(data+my_offset+2));
			ground_item_list[pos].pos= pos;
		}
}

void put_bag_on_ground(int bag_x,int bag_y,int bag_id)
{
	float x,y,z;
	int obj_3d_id;

	//now, get the Z position
	z=-2.2f+height_map[bag_y*tile_map_size_x*6+bag_x]*0.2f;
	//convert from height values to meters
	x=(float)bag_x/2;
	y=(float)bag_y/2;
	//center the object
	x=x+0.25f;
	y=y+0.25f;
	obj_3d_id=add_e3d("./3dobjects/misc_objects/bag1.e3d",x,y,z,0,0,0,1,0,1.0f,1.0f,1.0f);

	//now, find a place into the bags list, so we can destroy the bag properly
	bag_list[bag_id].x=bag_x;
	bag_list[bag_id].y=bag_y;
	bag_list[bag_id].obj_3d_id=obj_3d_id;
}

void add_bags_from_list(Uint8 *data)
{
	Uint16 bags_no;
	int i;
	int bag_x,bag_y,my_offset; //bag_type unused?
	float x,y,z;
	int obj_3d_id, bag_id;

	bags_no=data[0];
	for(i=0;i<bags_no;i++)
		{
			my_offset=i*5+1;
			bag_x=*((Uint16 *)(data+my_offset));
			bag_y=*((Uint16 *)(data+my_offset+2));
			bag_id=*((Uint8 *)(data+my_offset+4));
			//now, get the Z position
			z=-2.2f+height_map[bag_y*tile_map_size_x*6+bag_x]*0.2f;
			//convert from height values to meters
			x=(float)bag_x/2;
			y=(float)bag_y/2;
			//center the object
			x=x+0.25f;
			y=y+0.25f;

			obj_3d_id=add_e3d("./3dobjects/misc_objects/bag1.e3d",x,y,z,0,0,0,1,0,1.0f,1.0f,1.0f);
			//now, find a place into the bags list, so we can destroy the bag properly

			bag_list[bag_id].x=bag_x;
			bag_list[bag_id].y=bag_y;
			bag_list[bag_id].obj_3d_id=obj_3d_id;
		}
}

void remove_bag(int which_bag)
{
	add_particle_sys_at_tile("./particles/bag_out.part",bag_list[which_bag].x,bag_list[which_bag].y);
	destroy_3d_object(bag_list[which_bag].obj_3d_id);
	bag_list[which_bag].obj_3d_id=-1;
}

int click_ground_items_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int x,y;
	int x_screen,y_screen;
	Uint8 str[10];

	// see if we clicked on the "Get All" box
	if(mx>(win->len_x-33) && mx<win->len_x && my>20 && my<53){
		int pos;

		for(pos = 0; pos < 50; pos++){
			if(ground_item_list[pos].quantity){
				str[0]=PICK_UP_ITEM;
				str[1]=pos;
				*((Uint16 *)(str+2))=ground_item_list[pos].quantity;
				my_tcp_send(my_socket,str,4);
			}
		}
		return 1;
	}

	//see if we clicked on any item in the wear category
	for(y=0;y<10;y++)
		for(x=0;x<5;x++)
			{
				x_screen= x*33;
				y_screen= y*33;
				if(mx>x_screen && mx<x_screen+33 && my>y_screen && my<y_screen+33)
					{
						int pos;
						pos= y*5+x;
						if(!ground_item_list[pos].quantity) {
							if (item_dragged != -1){
								Uint8 str[10];
								int quantity = item_list[item_dragged].quantity;
								if (quantity > item_quantity)
									quantity = item_quantity;
								str[0] = DROP_ITEM;
								str[1] = item_list[item_dragged].pos;
								*((Uint16 *) (str + 2)) = quantity;
								my_tcp_send(my_socket, str, 4);
								if (item_list[item_dragged].quantity - quantity <= 0)
									item_dragged = -1;
							}
							return 1;
						}
						if(action_mode==action_look || right_click)
							{
								str[0]= LOOK_AT_GROUND_ITEM;
								str[1]= pos;
								my_tcp_send(my_socket,str,2);
							}
						else
							{
								int quantity;
								quantity= ground_item_list[pos].quantity;
								if(quantity > item_quantity) quantity= item_quantity;

								str[0]= PICK_UP_ITEM;
								str[1]= pos;
								*((Uint16 *)(str+2))= quantity;
								my_tcp_send(my_socket,str,4);
							}
						return 1;
					}
			}

	return 1;
}


int mouseover_ground_items_handler(window_info *win, int mx, int my) {
	int x,y;
	int x_screen,y_screen;
	for(y=0;y<10;y++)
		for(x=0;x<5;x++)
			{
				x_screen= x*33;
				y_screen= y*33;
				if(mx>x_screen && mx<x_screen+33 && my>y_screen && my<y_screen+33) {
					int pos;
					pos= y*5+x;
					if(ground_item_list[pos].quantity) {
						if(action_mode==action_look) {
							elwin_mouse=CURSOR_EYE;
						} else {
							elwin_mouse=CURSOR_PICK;
						}
						return 1;
					}
				}
			}
	return 0;
}

int mouseover_items_handler(window_info *win, int mx, int my) {
	int x,y,i;
	int x_screen,y_screen;
	for(y=0;y<6;y++)
		for(x=0;x<6;x++)
			{
				x_screen=x*51;
				y_screen=y*51;
				if(mx>x_screen && mx<x_screen+51 && my>y_screen && my<y_screen+51)
					{
						for(i=0;i<ITEM_NUM_ITEMS;i++)
							{
								//should we get the info for it?
								if(item_list[i].quantity && item_list[i].pos==y*6+x)
									{
										if(action_mode==action_look) {
											elwin_mouse=CURSOR_EYE;
										} else if(action_mode==action_use) {
											elwin_mouse=CURSOR_USE;
										} else {
											elwin_mouse=CURSOR_PICK;
										}
										return 1;
									}
							}
					}
			}
	return 0;
}

void open_bag(int object_id)
{
	int i;
	Uint8 str[4];
	for(i=0;i<100;i++)
		{
			if(bag_list[i].obj_3d_id==object_id)
				{
					str[0]= INSPECT_BAG;
					str[1]= i;
					my_tcp_send(my_socket,str,2);
					return;
				}
		}
}


void display_items_menu()
{
	if(items_win <= 0){
		items_win= create_window("Inventory", 0, 0, items_menu_x, items_menu_y, items_menu_x_len, items_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(items_win, ELW_HANDLER_DISPLAY, &display_items_handler );
		set_window_handler(items_win, ELW_HANDLER_CLICK, &click_items_handler );
		set_window_handler(items_win, ELW_HANDLER_MOUSEOVER, &mouseover_items_handler );
	} else {
		show_window(items_win);
		select_window(items_win);
	}
}


void draw_pick_up_menu()
{
	if(ground_items_win <= 0){
		ground_items_win= create_window("Bag", 0, 0, ground_items_menu_x, ground_items_menu_y, ground_items_menu_x_len, ground_items_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(ground_items_win, ELW_HANDLER_DISPLAY, &display_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_CLICK, &click_ground_items_handler );
		set_window_handler(ground_items_win, ELW_HANDLER_MOUSEOVER, &mouseover_ground_items_handler );
	} else {
		show_window(ground_items_win);
		select_window(ground_items_win);
	}
}


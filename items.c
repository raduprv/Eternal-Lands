#include "global.h"

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

int view_my_items=0;
int view_ground_items=0;
int view_manufacture_menu=0;
int view_trade_menu=0;
int no_view_my_items=0;

int items_menu_x=10;
int items_menu_y=20;
int items_menu_x_len=6*51+110;
int items_menu_y_len=6*51+60;
int items_menu_dragged=0;

int ground_items_menu_x=6*51+100+20;
int ground_items_menu_y=20;
int ground_items_menu_x_len=6*33;
int ground_items_menu_y_len=10*33;
int ground_items_menu_dragged=0;

int manufacture_menu_x=10;
int manufacture_menu_y=20;
int manufacture_menu_x_len=12*33+20;
int manufacture_menu_y_len=6*33;
int manufacture_menu_dragged=0;

int trade_menu_x=10;
int trade_menu_y=20;
int trade_menu_x_len=13*33;
int trade_menu_y_len=11*33;
int trade_menu_dragged=0;

int options_menu_x=220;
int options_menu_y=50;
int options_menu_x_len=390;
int options_menu_y_len=260;
int options_menu_dragged=0;

int items_text_1;
int items_text_2;
int items_text_3;
int items_text_4;
int items_text_5;
int items_text_6;
int items_text_7;

char items_string[300];
int item_dragged=-1;
int item_quantity=1;

int wear_items_x_offset=6*51+20;
int wear_items_y_offset=30;

Uint32 click_time=0;

void display_items_menu()
{
	Uint8 str[80];
	int x,y,i;
	int item_is_weared=0;
	//first of all, draw the actual menu.

	draw_menu_title_bar(items_menu_x,items_menu_y-16,items_menu_x_len);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(items_menu_x,items_menu_y+items_menu_y_len,0);
	glVertex3i(items_menu_x,items_menu_y,0);
	glVertex3i(items_menu_x+items_menu_x_len,items_menu_y,0);
	glVertex3i(items_menu_x+items_menu_x_len,items_menu_y+items_menu_y_len,0);
	glEnd();

	glDisable(GL_BLEND);

	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	glVertex3i(items_menu_x,items_menu_y,0);
	glVertex3i(items_menu_x+items_menu_x_len,items_menu_y,0);

	glVertex3i(items_menu_x+items_menu_x_len,items_menu_y,0);
	glVertex3i(items_menu_x+items_menu_x_len,items_menu_y+items_menu_y_len,0);

	glVertex3i(items_menu_x+items_menu_x_len,items_menu_y+items_menu_y_len,0);
	glVertex3i(items_menu_x,items_menu_y+items_menu_y_len,0);

	glVertex3i(items_menu_x,items_menu_y+items_menu_y_len,0);
	glVertex3i(items_menu_x,items_menu_y,0);

	//draw the grid
	for(y=1;y<7;y++)
		{
			glVertex3i(items_menu_x,items_menu_y+y*51,0);
			glVertex3i(items_menu_x+6*51,items_menu_y+y*51,0);
		}
	for(x=1;x<7;x++)
		{
			glVertex3i(items_menu_x+x*51,items_menu_y,0);
			glVertex3i(items_menu_x+x*51,items_menu_y+6*51,0);
		}

	glColor3f(0.57f,0.67f,0.49f);
	//draw the small grid
	for(y=0;y<5;y++)
		{
			glVertex3i(items_menu_x+wear_items_x_offset,items_menu_y+wear_items_y_offset+y*33,0);
			glVertex3i(items_menu_x+wear_items_x_offset+2*33,items_menu_y+wear_items_y_offset+y*33,0);
		}
	for(x=0;x<3;x++)
		{
			glVertex3i(items_menu_x+wear_items_x_offset+x*33,items_menu_y+wear_items_y_offset,0);
			glVertex3i(items_menu_x+wear_items_x_offset+x*33,items_menu_y+wear_items_y_offset+4*33,0);
		}
	glColor3f(0.77f,0.57f,0.39f);
	//draw the corner, with the X in
	glVertex3i(items_menu_x+items_menu_x_len,items_menu_y+20,0);
	glVertex3i(items_menu_x+items_menu_x_len-20,items_menu_y+20,0);

	glVertex3i(items_menu_x+items_menu_x_len-20,items_menu_y+20,0);
	glVertex3i(items_menu_x+items_menu_x_len-20,items_menu_y,0);


	//now, draw the quantity boxes
	glColor3f(0.3f,0.5f,1.0f);
	for(y=0;y<6;y++)
		{
			glVertex3i(items_menu_x+wear_items_x_offset,items_menu_y+wear_items_y_offset+160+y*20,0);
			glVertex3i(items_menu_x+wear_items_x_offset+2*35,items_menu_y+wear_items_y_offset+160+y*20,0);
		}
	for(x=0;x<3;x++)
		{
			glVertex3i(items_menu_x+wear_items_x_offset+x*35,items_menu_y+wear_items_y_offset+160,0);
			glVertex3i(items_menu_x+wear_items_x_offset+x*35,items_menu_y+wear_items_y_offset+160+5*20,0);
		}

	glEnd();
	glEnable(GL_TEXTURE_2D);

	//draw the quantity string
	draw_string_small(items_menu_x+wear_items_x_offset,items_menu_y+wear_items_y_offset+145,"Quantity",1);
	//draw the quantity values
	if(item_quantity==1)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(items_menu_x+wear_items_x_offset+15,items_menu_y+wear_items_y_offset+163,"1",1);
	if(item_quantity==5)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(items_menu_x+wear_items_x_offset+50,items_menu_y+wear_items_y_offset+163,"5",1);
	if(item_quantity==10)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(items_menu_x+wear_items_x_offset+10,items_menu_y+wear_items_y_offset+183,"10",1);
	if(item_quantity==20)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(items_menu_x+wear_items_x_offset+45,items_menu_y+wear_items_y_offset+183,"20",1);
	if(item_quantity==50)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(items_menu_x+wear_items_x_offset+10,items_menu_y+wear_items_y_offset+203,"50",1);
	if(item_quantity==100)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(items_menu_x+wear_items_x_offset+40,items_menu_y+wear_items_y_offset+203,"100",1);
	if(item_quantity==200)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(items_menu_x+wear_items_x_offset+5,items_menu_y+wear_items_y_offset+223,"200",1);
	if(item_quantity==500)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(items_menu_x+wear_items_x_offset+40,items_menu_y+wear_items_y_offset+223,"500",1);
	if(item_quantity==1000)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(items_menu_x+wear_items_x_offset+1,items_menu_y+wear_items_y_offset+243,"1000",1);
	if(item_quantity==2000)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(items_menu_x+wear_items_x_offset+36,items_menu_y+wear_items_y_offset+243,"2000",1);
	glColor3f(0.77f,0.57f,0.39f);
	draw_string(items_menu_x+items_menu_x_len-16,items_menu_y+2,"X",1);

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
					u_end=u_start+0.2f;
					v_start=(1.0f+2.0f/256.0f)-(0.2f*(cur_item/5));
					v_end=v_start-0.2f;

					//get the x and y
					cur_pos=item_list[i].pos;
					if(cur_pos>=ITEM_WEAR_START)//the items we 'wear' are smaller
						{
							cur_pos-=ITEM_WEAR_START;
							item_is_weared=1;
							x_start=items_menu_x+wear_items_x_offset+33*(cur_pos%2)+1;
							x_end=x_start+32;
							y_start=items_menu_y+wear_items_y_offset+33*(cur_pos/2);
							y_end=y_start+32;
						}
					else
						{
							item_is_weared=0;
							x_start=items_menu_x+51*(cur_pos%6)+1;
							x_end=x_start+50;
							y_start=items_menu_y+51*(cur_pos/6);
							y_end=y_start+50;
						}

					//get the texture this item belongs to
					this_texture=item_list[i].image_id/25;
					if(this_texture==0)this_texture=items_text_1;
					else if(this_texture==1)this_texture=items_text_2;
					else if(this_texture==2)this_texture=items_text_3;
					else if(this_texture==3)this_texture=items_text_4;
					else if(this_texture==4)this_texture=items_text_5;
					else if(this_texture==5)this_texture=items_text_6;
					else if(this_texture==6)this_texture=items_text_7;

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
	draw_string_small(items_menu_x+4,items_menu_y+items_menu_y_len-59,items_string,4);

	glColor3f(1.0f,1.0f,1.0f);
	//draw the load string
	sprintf(str,"Load:%i/%i",your_info.carry_capacity.cur,your_info.carry_capacity.base);
	draw_string_small(items_menu_x+6*51+4,items_menu_y+6*51+44,str,1);
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


int check_items_interface()
{
	int i,x,y;
	int x_screen,y_screen;
	Uint8 str[100];

	if(!view_my_items || mouse_x>items_menu_x+items_menu_x_len || mouse_x<items_menu_x
	   || mouse_y<items_menu_y || mouse_y>items_menu_y+items_menu_y_len)return 0;

	//see if we changed the quantity
	for(y=0;y<5;y++)
		for(x=0;x<2;x++)
			{
				x_screen=items_menu_x+wear_items_x_offset+x*35;
				y_screen=items_menu_y+wear_items_y_offset+160+y*20;
				if(mouse_x>x_screen && mouse_x<x_screen+35 && mouse_y>y_screen && mouse_y<y_screen+20)
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
				x_screen=items_menu_x+x*51;
				y_screen=items_menu_y+y*51;
				if(mouse_x>x_screen && mouse_x<x_screen+51 && mouse_y>y_screen && mouse_y<y_screen+51)
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

										if(action_mode==action_look || right_click)
											{
												if(cur_time<(click_time+500))
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
										else if(action_mode==action_pick)
											{
												int quantity;
												quantity=item_list[i].quantity;
												if(quantity-item_quantity>0)quantity=item_quantity;
												str[0]=DROP_ITEM;
												str[1]=item_list[i].pos;
												*((Uint16 *)(str+2))=quantity;//quantity
												my_tcp_send(my_socket,str,4);
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
				x_screen=wear_items_x_offset+items_menu_x+x*33;
				y_screen=wear_items_y_offset+items_menu_y+y*33;
				if(mouse_x>x_screen && mouse_x<x_screen+33 && mouse_y>y_screen && mouse_y<y_screen+33)
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
	u_end=u_start+0.2f;
	v_start=(1.0f+2.0f/256.0f)-(0.2f*(cur_item/5));
	v_end=v_start-0.2f;

	//get the texture this item belongs to
	this_texture=item_list[item_dragged].image_id/25;
	if(this_texture==0)this_texture=items_text_1;
	else if(this_texture==1)this_texture=items_text_2;
	else if(this_texture==2)this_texture=items_text_3;
	else if(this_texture==3)this_texture=items_text_4;
	else if(this_texture==4)this_texture=items_text_5;
	else if(this_texture==5)this_texture=items_text_6;
	else if(this_texture==6)this_texture=items_text_7;

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
	ground_item_list[pos].quantity=0;
}

void get_new_inventory_item(Uint8 *data)
{
	int i;
	int pos;
	Uint8 flags;
	int quantity;
	int image_id;

	pos=data[6];
	flags=data[7];
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






void draw_pick_up_menu()
{
	Uint8 str[80];
	int x,y,i;
	//first of all, draw the actual menu.

	draw_menu_title_bar(ground_items_menu_x,ground_items_menu_y-16,ground_items_menu_x_len);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(ground_items_menu_x,ground_items_menu_y+ground_items_menu_y_len,0);
	glVertex3i(ground_items_menu_x,ground_items_menu_y,0);
	glVertex3i(ground_items_menu_x+ground_items_menu_x_len,ground_items_menu_y,0);
	glVertex3i(ground_items_menu_x+ground_items_menu_x_len,ground_items_menu_y+ground_items_menu_y_len,0);
	glEnd();

	glDisable(GL_BLEND);

	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	glVertex3i(ground_items_menu_x,ground_items_menu_y,0);
	glVertex3i(ground_items_menu_x+ground_items_menu_x_len,ground_items_menu_y,0);

	glVertex3i(ground_items_menu_x+ground_items_menu_x_len,ground_items_menu_y,0);
	glVertex3i(ground_items_menu_x+ground_items_menu_x_len,ground_items_menu_y+ground_items_menu_y_len,0);

	glVertex3i(ground_items_menu_x+ground_items_menu_x_len,ground_items_menu_y+ground_items_menu_y_len,0);
	glVertex3i(ground_items_menu_x,ground_items_menu_y+ground_items_menu_y_len,0);

	glVertex3i(ground_items_menu_x,ground_items_menu_y+ground_items_menu_y_len,0);
	glVertex3i(ground_items_menu_x,ground_items_menu_y,0);

	//draw the grid
	for(y=1;y<11;y++)
		{
			glVertex3i(ground_items_menu_x,ground_items_menu_y+y*33,0);
			glVertex3i(ground_items_menu_x+5*33,ground_items_menu_y+y*33,0);
		}
	for(x=1;x<6;x++)
		{
			glVertex3i(ground_items_menu_x+x*33,ground_items_menu_y,0);
			glVertex3i(ground_items_menu_x+x*33,ground_items_menu_y+10*33,0);
		}


	glColor3f(0.77f,0.57f,0.39f);
	//draw the corner, with the X in
	glVertex3i(ground_items_menu_x+ground_items_menu_x_len,ground_items_menu_y+20,0);
	glVertex3i(ground_items_menu_x+ground_items_menu_x_len-20,ground_items_menu_y+20,0);

	glVertex3i(ground_items_menu_x+ground_items_menu_x_len-20,ground_items_menu_y+20,0);
	glVertex3i(ground_items_menu_x+ground_items_menu_x_len-20,ground_items_menu_y,0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	draw_string(ground_items_menu_x+ground_items_menu_x_len-16,ground_items_menu_y+2,"X",1);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<50;i++)
		{
			if(ground_item_list[i].quantity)
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
					x_start=ground_items_menu_x+33*(cur_pos%5)+1;
					x_end=x_start+32;
					y_start=ground_items_menu_y+33*(cur_pos/5);
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

					get_and_set_texture_id(this_texture);
					glBegin(GL_QUADS);
					draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
					glEnd();
					sprintf(str,"%i",ground_item_list[i].quantity);
					draw_string_small(x_start,y_end-15,str,1);
				}
		}
	glColor3f(1.0f,1.0f,1.0f);
}


//do the flags later on
void get_bag_item(Uint8 *data)
{
	int pos;
	pos=data[6];
	ground_item_list[pos].image_id=*((Uint16 *)(data));
	ground_item_list[pos].quantity=*((Uint32 *)(data+2));
	ground_item_list[pos].pos=pos;
}

//put the flags later on
void get_bags_items_list(Uint8 *data)
{
	Uint16 items_no;
	int i;
	int my_offset;
	int pos;


	view_ground_items=1;
	view_my_items=1;
	//clear the list
	for(i=0;i<50;i++)ground_item_list[i].quantity=0;

	items_no=data[0];
	for(i=0;i<items_no;i++)
		{
			my_offset=i*7+1;
			pos=data[my_offset+6];
			ground_item_list[pos].image_id=*((Uint16 *)(data+my_offset));
			ground_item_list[pos].quantity=*((Uint32 *)(data+my_offset+2));
			ground_item_list[pos].pos=pos;
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
	bag_list[bag_id].x=x;
	bag_list[bag_id].y=y;
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

			bag_list[bag_id].x=x;
			bag_list[bag_id].y=y;
			bag_list[bag_id].obj_3d_id=obj_3d_id;


		}
}

void remove_bag(int which_bag)
{
	add_bag_out(bag_list[which_bag].x*2,bag_list[which_bag].y*2);
	destroy_3d_object(bag_list[which_bag].obj_3d_id);
	bag_list[which_bag].obj_3d_id=-1;
}

int check_ground_items_interface()
{
	int x,y;
	int x_screen,y_screen;
	Uint8 str[10];

	if(!view_ground_items || mouse_x>ground_items_menu_x+ground_items_menu_x_len || mouse_x<ground_items_menu_x
	   || mouse_y<ground_items_menu_y || mouse_y>ground_items_menu_y+ground_items_menu_y_len)return 0;

	//see if we clicked on any item in the wear category
	for(y=0;y<10;y++)
		for(x=0;x<5;x++)
			{
				x_screen=ground_items_menu_x+x*33;
				y_screen=ground_items_menu_y+y*33;
				if(mouse_x>x_screen && mouse_x<x_screen+33 && mouse_y>y_screen && mouse_y<y_screen+33)
					{
						int pos;
						pos=y*5+x;
						if(!ground_item_list[pos].quantity)return 1;

						if(/*action_mode==action_look ||*/ right_click)
							{
								str[0]=LOOK_AT_GROUND_ITEM;
								str[1]=pos;
								my_tcp_send(my_socket,str,2);
							}
						else /*if(action_mode==action_pick)*/
							{
								int quantity;
								quantity=ground_item_list[pos].quantity;
								if(quantity-item_quantity>0)quantity=item_quantity;

								str[0]=PICK_UP_ITEM;
								str[1]=pos;
								*((Uint16 *)(str+2))=quantity;
								my_tcp_send(my_socket,str,4);
							}
						return 1;
					}
			}

	return 1;

}


void open_bag(int object_id)
{
	int i;
	Uint8 str[4];
	for(i=0;i<100;i++)
		{
			if(bag_list[i].obj_3d_id==object_id)
				{
					str[0]=INSPECT_BAG;
					str[1]=i;
					my_tcp_send(my_socket,str,2);
					return;
				}
		}
}









#include "global.h"


void display_trade_menu()
{
	Uint8 str[80];
	int x,y,i,j;
	//first of all, draw the actual menu.

	draw_menu_title_bar(trade_menu_x,trade_menu_y-16,trade_menu_x_len);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(trade_menu_x,trade_menu_y+trade_menu_y_len,0);
	glVertex3i(trade_menu_x,trade_menu_y,0);
	glVertex3i(trade_menu_x+trade_menu_x_len,trade_menu_y,0);
	glVertex3i(trade_menu_x+trade_menu_x_len,trade_menu_y+trade_menu_y_len,0);
	glEnd();

	glDisable(GL_BLEND);

	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	glVertex3i(trade_menu_x,trade_menu_y,0);
	glVertex3i(trade_menu_x+trade_menu_x_len,trade_menu_y,0);

	glVertex3i(trade_menu_x+trade_menu_x_len,trade_menu_y,0);
	glVertex3i(trade_menu_x+trade_menu_x_len,trade_menu_y+trade_menu_y_len,0);

	glVertex3i(trade_menu_x+trade_menu_x_len,trade_menu_y+trade_menu_y_len,0);
	glVertex3i(trade_menu_x,trade_menu_y+trade_menu_y_len,0);

	glVertex3i(trade_menu_x,trade_menu_y+trade_menu_y_len,0);
	glVertex3i(trade_menu_x,trade_menu_y,0);

	//draw the grid
	for(y=1;y<4;y++)
		{
			glVertex3i(trade_menu_x,trade_menu_y+y*33,0);
			glVertex3i(trade_menu_x+12*33,trade_menu_y+y*33,0);
		}
	for(x=1;x<13;x++)
		{
			glVertex3i(trade_menu_x+x*33,trade_menu_y,0);
			glVertex3i(trade_menu_x+x*33,trade_menu_y+3*33,0);
		}

	glColor3f(0.57f,0.67f,0.49f);
	//draw the you have grid
	for(y=1;y<6;y++)
		{
			glVertex3i(trade_menu_x,trade_menu_y+(y+3)*33,0);
			glVertex3i(trade_menu_x+4*33,trade_menu_y+(y+3)*33,0);
		}
	for(x=0;x<5;x++)
		{
			glVertex3i(trade_menu_x+x*33,trade_menu_y+4*33,0);
			glVertex3i(trade_menu_x+x*33,trade_menu_y+8*33,0);
		}
	//draw the what the other player has
	for(y=1;y<6;y++)
		{
			glVertex3i(trade_menu_x+5*33,trade_menu_y+(y+3)*33,0);
			glVertex3i(trade_menu_x+9*33,trade_menu_y+(y+3)*33,0);
		}
	for(x=5;x<10;x++)
		{
			glVertex3i(trade_menu_x+x*33,trade_menu_y+4*33,0);
			glVertex3i(trade_menu_x+x*33,trade_menu_y+8*33,0);
		}

	glColor3f(0.77f,0.57f,0.39f);

	//draw the button frame

	//Clear button
	glVertex3i(trade_menu_x+33*5,trade_menu_y+trade_menu_y_len-30,0);
	glVertex3i(trade_menu_x+33*5+70,trade_menu_y+trade_menu_y_len-30,0);
	glVertex3i(trade_menu_x+33*5,trade_menu_y+trade_menu_y_len-10,0);
	glVertex3i(trade_menu_x+33*5+70,trade_menu_y+trade_menu_y_len-10,0);
	glVertex3i(trade_menu_x+33*5+70,trade_menu_y+trade_menu_y_len-30,0);
	glVertex3i(trade_menu_x+33*5+70,trade_menu_y+trade_menu_y_len-9,0);
	glVertex3i(trade_menu_x+33*5,trade_menu_y+trade_menu_y_len-30,0);
	glVertex3i(trade_menu_x+33*5,trade_menu_y+trade_menu_y_len-10,0);

	//the players accept boxes
	glVertex3i(trade_menu_x+5,trade_menu_y+4*33-20,0);
	glVertex3i(trade_menu_x+20,trade_menu_y+4*33-20,0);
	glVertex3i(trade_menu_x+5,trade_menu_y+4*33-5,0);
	glVertex3i(trade_menu_x+20,trade_menu_y+4*33-5,0);
	glVertex3i(trade_menu_x+20,trade_menu_y+4*33-20,0);
	glVertex3i(trade_menu_x+20,trade_menu_y+4*33-4,0);
	glVertex3i(trade_menu_x+5,trade_menu_y+4*33-20,0);
	glVertex3i(trade_menu_x+5,trade_menu_y+4*33-5,0);

	glVertex3i(trade_menu_x+5*33+5,trade_menu_y+4*33-20,0);
	glVertex3i(trade_menu_x+5*33+20,trade_menu_y+4*33-20,0);
	glVertex3i(trade_menu_x+5*33+5,trade_menu_y+4*33-5,0);
	glVertex3i(trade_menu_x+5*33+20,trade_menu_y+4*33-5,0);
	glVertex3i(trade_menu_x+5*33+20,trade_menu_y+4*33-20,0);
	glVertex3i(trade_menu_x+5*33+20,trade_menu_y+4*33-4,0);
	glVertex3i(trade_menu_x+5*33+5,trade_menu_y+4*33-20,0);
	glVertex3i(trade_menu_x+5*33+5,trade_menu_y+4*33-5,0);


	//now, draw the quantity boxes
	glColor3f(0.3f,0.5f,1.0f);
	for(y=0;y<6;y++)
		{
			glVertex3i(trade_menu_x+33*9+25,trade_menu_y+133+y*20,0);
			glVertex3i(trade_menu_x+33*9+25+2*35,trade_menu_y+133+y*20,0);
		}
	for(x=0;x<3;x++)
		{
			glVertex3i(trade_menu_x+33*9+25+x*35,trade_menu_y+133,0);
			glVertex3i(trade_menu_x+33*9+25+x*35,trade_menu_y+133+5*20,0);
		}

	glEnd();
	glEnable(GL_TEXTURE_2D);

	//draw the quantity string
	draw_string_small(trade_menu_x+33*9+25,trade_menu_y+116,"Quantity",1);
	//draw the quantity values
	if(item_quantity==1)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(trade_menu_x+33*9+25+15,trade_menu_y+136,"1",1);
	if(item_quantity==5)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(trade_menu_x+33*9+25+50,trade_menu_y+136,"5",1);
	if(item_quantity==10)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(trade_menu_x+33*9+25+10,trade_menu_y+156,"10",1);
	if(item_quantity==20)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(trade_menu_x+33*9+25+45,trade_menu_y+156,"20",1);
	if(item_quantity==50)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(trade_menu_x+33*9+25+10,trade_menu_y+176,"50",1);
	if(item_quantity==100)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(trade_menu_x+33*9+25+40,trade_menu_y+176,"100",1);
	if(item_quantity==200)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(trade_menu_x+33*9+25+5,trade_menu_y+196,"200",1);
	if(item_quantity==500)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(trade_menu_x+33*9+25+40,trade_menu_y+196,"500",1);
	if(item_quantity==1000)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(trade_menu_x+33*9+25+1,trade_menu_y+216,"1000",1);
	if(item_quantity==2000)glColor3f(0.0f,1.0f,0.3f); else glColor3f(0.3f,0.5f,1.0f);
	draw_string_small(trade_menu_x+33*9+25+36,trade_menu_y+216,"2000",1);

	glColor3f(0.77f,0.57f,0.39f);
	draw_string(trade_menu_x+33*5+8,trade_menu_y+trade_menu_y_len-30+2,"Abort",1);
	if(trade_you_accepted)draw_string_small(trade_menu_x+8,trade_menu_y+4*33-19,"X",1);
	draw_string_small(trade_menu_x+24,trade_menu_y+4*33-19,"You",1);
	if(trade_other_accepted)draw_string_small(trade_menu_x+5*33+8,trade_menu_y+4*33-19,"X",1);
	draw_string_small(trade_menu_x+5*33+24,trade_menu_y+4*33-19,other_player_trade_name,1);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	j=0;
	for(i=0;i<36+6;i++)
		{
			if(item_list[i].quantity && item_list[i].pos<36)
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
					cur_pos=j;
					j++;

					x_start=trade_menu_x+33*(cur_pos%12)+1;
					x_end=x_start+32;
					y_start=trade_menu_y+33*(cur_pos/12);
					y_end=y_start+32;

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

					sprintf(str,"%i",item_list[i].quantity);
					draw_string_small(x_start,y_end-15,str,1);
				}

		}

	//let's draw the objects we have on trade
	for(i=0;i<16;i++)
		{
			if(your_trade_list[i].quantity)
				{
					float u_start,v_start,u_end,v_end;
					int this_texture,cur_item,cur_pos;
					int x_start,x_end,y_start,y_end;

					//get the UV coordinates.
					cur_item=your_trade_list[i].image_id%25;
					u_start=0.2f*(cur_item%5);
					u_end=u_start+(float)50/256;
					v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
					v_end=v_start-(float)50/256;

					//get the x and y
					cur_pos=i;

					x_start=trade_menu_x+33*(cur_pos%4)+1;
					x_end=x_start+32;
					y_start=trade_menu_y+33*(cur_pos/4+4);
					y_end=y_start+32;

					//get the texture this item belongs to
					this_texture=your_trade_list[i].image_id/25;
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
					else
					if(this_texture==6)this_texture=items_text_7;

					get_and_set_texture_id(this_texture);
					glBegin(GL_QUADS);
					draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
					glEnd();

					sprintf(str,"%i",your_trade_list[i].quantity);
					draw_string_small(x_start,y_end-15,str,1);
				}

		}

	//let's draw the objects the other has on trade
	for(i=0;i<16;i++)
		{
			if(others_trade_list[i].quantity)
				{
					float u_start,v_start,u_end,v_end;
					int this_texture,cur_item,cur_pos;
					int x_start,x_end,y_start,y_end;

					//get the UV coordinates.
					cur_item=others_trade_list[i].image_id%25;
					u_start=0.2f*(cur_item%5);
					u_end=u_start+(float)50/256;
					v_start=(1.0f+((float)50/256)/256.0f)-((float)50/256*(cur_item/5));
					v_end=v_start-(float)50/256;


					//get the x and y
					cur_pos=i;

					x_start=trade_menu_x+33*(cur_pos%4+5)+1;
					x_end=x_start+32;
					y_start=trade_menu_y+33*(cur_pos/4+4);
					y_end=y_start+32;

					//get the texture this item belongs to
					this_texture=others_trade_list[i].image_id/25;
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
					else
					if(this_texture==6)this_texture=items_text_7;

					get_and_set_texture_id(this_texture);
					glBegin(GL_QUADS);
					draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
					glEnd();

					sprintf(str,"%i",others_trade_list[i].quantity);
					draw_string_small(x_start,y_end-15,str,1);
				}

		}


	//now, draw the inventory text, if any.
	draw_string_small(trade_menu_x+4,trade_menu_y+trade_menu_y_len-75,items_string,4);
	glColor3f(1.0f,1.0f,1.0f);
}


int check_trade_interface()
{
	int x,y;
	int x_screen,y_screen;
	Uint8 str[10];

	if(!view_trade_menu || mouse_x>trade_menu_x+trade_menu_x_len || mouse_x<trade_menu_x
	|| mouse_y<trade_menu_y || mouse_y>trade_menu_y+trade_menu_y_len)return 0;

	//see if we changed the quantity
	for(y=0;y<5;y++)
	for(x=0;x<2;x++)
		{
			x_screen=trade_menu_x+33*9+25+x*35;
			y_screen=trade_menu_y+133+y*20;
			if(mouse_x>x_screen && mouse_x<x_screen+35 && mouse_y>y_screen && mouse_y<y_screen+20)
				{
					if(x==0 && y==0)item_quantity=1;
					else
					if(x==1 && y==0)item_quantity=5;
					else
					if(x==0 && y==1)item_quantity=10;
					else
					if(x==1 && y==1)item_quantity=20;
					else
					if(x==0 && y==2)item_quantity=50;
					else
					if(x==1 && y==2)item_quantity=100;
					else
					if(x==0 && y==3)item_quantity=200;
					else
					if(x==1 && y==3)item_quantity=500;
					else
					if(x==0 && y==4)item_quantity=1000;
					else
					if(x==1 && y==4)item_quantity=2000;
				}
		}

	//check to see if we hit the Accept box
	if(mouse_x>trade_menu_x+5 && mouse_x<trade_menu_x+20 && mouse_y>trade_menu_y+4*33-20 && mouse_y<trade_menu_y+4*33-5)
		{

			if(trade_you_accepted)str[0]=REJECT_TRADE;
			else str[0]=ACCEPT_TRADE;
			my_tcp_send(my_socket,str,1);
			return 1;
		}

	//check to see if we hit the Abort button
	if(mouse_x>trade_menu_x+33*5 && mouse_x<trade_menu_x+33*5+70 &&
	mouse_y>trade_menu_y+trade_menu_y_len-30 && mouse_y<trade_menu_y+trade_menu_y_len-10)
		{

			str[0]=EXIT_TRADE;
			my_tcp_send(my_socket,str,1);
			return 1;
		}

	//see if we clicked on any item in the main category
	for(y=0;y<3;y++)
		for(x=0;x<12;x++)
			{
				x_screen=trade_menu_x+x*33;
				y_screen=trade_menu_y+y*33;
				if(mouse_x>x_screen && mouse_x<x_screen+33 && mouse_y>y_screen && mouse_y<y_screen+33)
					{
						int i,j;

						//see if there is any item there
						j=0;
						for(i=0;i<36+6;i++)
							{
							if(item_list[i].quantity && item_list[i].pos<36)
								{
									if(j==y*12+x)break;
									j++;
								}
							}
						if(i<36+6 && item_list[i].quantity)
							{
								//if(action_mode==action_look && left_click)
								if(action_mode==action_look || right_click)
									{
										str[0]=LOOK_AT_INVENTORY_ITEM;
										str[1]=item_list[i].pos;
										my_tcp_send(my_socket,str,2);
									}
								else
									{
										str[0]=PUT_OBJECT_ON_TRADE;
										str[1]=item_list[i].pos;
										*((Uint16 *)(str+2))=item_quantity;
										my_tcp_send(my_socket,str,4);
									}

								return 1;
							}
					}
		}

	//see if we clicked on any item in your trading objects category
	for(y=0;y<4;y++)
	for(x=0;x<4;x++)
		{
			x_screen=trade_menu_x+x*33;
			y_screen=trade_menu_y+(y+4)*33;
			if(mouse_x>x_screen && mouse_x<x_screen+33 && mouse_y>y_screen && mouse_y<y_screen+33)
				{

					//see if there is any item there
					//should we get the info for it?
					if(your_trade_list[y*4+x].quantity)
						{

							if(action_mode==action_look || right_click)
								{
									str[0]=LOOK_AT_TRADE_ITEM;
									str[1]=y*4+x;
									str[2]=0;//your trade
									my_tcp_send(my_socket,str,3);
								}
							else
								{
									str[0]=REMOVE_OBJECT_FROM_TRADE;
									str[1]=y*4+x;
									*((Uint16 *)(str+2))=item_quantity;
									my_tcp_send(my_socket,str,4);
								}

							return 1;
						}
				}
		}

	//see if we clicked on any item in your trading partner objects category
	for(y=0;y<4;y++)
	for(x=0;x<4;x++)
		{
			x_screen=trade_menu_x+33*5+x*33;
			y_screen=trade_menu_y+(y+4)*33;
			if(mouse_x>x_screen && mouse_x<x_screen+33 && mouse_y>y_screen && mouse_y<y_screen+33)
				{

					//see if there is any item there
					//should we get the info for it?
					if(others_trade_list[y*4+x].quantity)
						{

							if(action_mode==action_look || right_click)
								{
									str[0]=LOOK_AT_TRADE_ITEM;
									str[1]=y*4+x;
									str[2]=1;//their trade
									my_tcp_send(my_socket,str,3);
								}
							return 1;
						}
				}
		}
	return 1;
}



void get_trade_partner_name(Uint8 *player_name,int len)
{
	int i;
	for(i=0;i<len;i++)
		{
			other_player_trade_name[i]=player_name[i];
		}
	other_player_trade_name[i]=0;
}


void get_your_trade_objects(Uint8 *data)
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

	view_trade_menu=1;

	//we have to close the inventory and manufacture windows, otherwise bad things can happen.
	view_my_items=0;
	view_manufacture_menu=0;
	view_sigils_menu=0;
}

void put_item_on_trade(Uint8 *data)
{
	int pos;

	pos=data[6];
	if(!data[7])
	{
		your_trade_list[pos].image_id=*((Uint16 *)(data));
		your_trade_list[pos].quantity+=*((Uint32 *)(data+2));
	}
	else
	{
		others_trade_list[pos].image_id=*((Uint16 *)(data));
		others_trade_list[pos].quantity+=*((Uint32 *)(data+2));
	}
}

void remove_item_from_trade(Uint8 *data)
{
	int pos;
	int quantity;

	pos=data[2];
	quantity=*((Uint16 *)(data));

	if(!data[3])
	{
		your_trade_list[pos].quantity-=quantity;
	}
	else
	{
		others_trade_list[pos].quantity-=quantity;
	}
}


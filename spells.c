#include "global.h"

Sint8 on_cast[6];
Sint8 cast_cache[6];
int clear_mouseover=0;
int cast_mouseover=0;

void repeat_spell()
{
}

void make_sigils_list()
{
	int i;

	for(i=0;i<SIGILS_NO;i++)sigils_list[i].have_sigil=0;

	spell_text[0]=0;
	i=0;

	sigils_list[i].sigil_img=0;
	my_strcp(sigils_list[i].name,"Change");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=1;
	my_strcp(sigils_list[i].name,"Restore");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=2;
	my_strcp(sigils_list[i].name,"Space");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=3;
	my_strcp(sigils_list[i].name,"Increase");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=4;
	my_strcp(sigils_list[i].name,"Decrease");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=5;
	my_strcp(sigils_list[i].name,"Temporary");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=6;
	my_strcp(sigils_list[i].name,"Permanent");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=7;
	my_strcp(sigils_list[i].name,"Move");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=8;
	my_strcp(sigils_list[i].name,"Local");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=9;
	my_strcp(sigils_list[i].name,"Global");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=10;
	my_strcp(sigils_list[i].name,"Fire");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=11;
	my_strcp(sigils_list[i].name,"Water");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=12;
	my_strcp(sigils_list[i].name,"Air");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=13;
	my_strcp(sigils_list[i].name,"Earth");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=14;
	my_strcp(sigils_list[i].name,"Spirit");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=15;
	my_strcp(sigils_list[i].name,"Matter");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=16;
	my_strcp(sigils_list[i].name,"Energy");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=17;
	my_strcp(sigils_list[i].name,"Magic");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=18;
	my_strcp(sigils_list[i].name,"Destroy");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=19;
	my_strcp(sigils_list[i].name,"Create");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=20;
	my_strcp(sigils_list[i].name,"Knowledge");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=21;
	my_strcp(sigils_list[i].name,"Protection");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=22;
	my_strcp(sigils_list[i].name,"Remove");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=23;
	my_strcp(sigils_list[i].name,"Health");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=24;
	my_strcp(sigils_list[i].name,"Life");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=25;
	my_strcp(sigils_list[i].name,"Death");
	my_strcp(sigils_list[i].description,"");
	sigils_list[i].have_sigil=1;

	for(i=0;i<6;i++)on_cast[i]=-1;
	for(i=0;i<10;i++)active_spells[i]=-1;
}

void get_active_spell(int pos, int spell)
{
	active_spells[pos]=spell;
}

void remove_active_spell(int pos)
{
	active_spells[pos]=-1;
}

void get_active_spell_list(Uint8 *my_spell_list)
{
	int i;
	for(i=0;i<10;i++)active_spells[i]=my_spell_list[i];
}

void display_spells_we_have()
{
	int i;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<10;i++)
		{
			if(active_spells[i]!=-1)
				{
					float u_start,v_start,u_end,v_end;
					int cur_spell,cur_pos;
					int x_start,x_end,y_start,y_end;

					//get the UV coordinates.
					cur_spell=active_spells[i]+32;//the first 32 icons are the sigils
					u_start=0.125f*(cur_spell%8);
					u_end=u_start+0.125f;
					v_start=1.0f-(0.125f*(cur_spell/8));
					v_end=v_start-0.125f;

					//get the x and y
					cur_pos=i;

					x_start=33*cur_pos;
					x_end=x_start+32;
					y_start=window_height-64;
					y_end=y_start+32;


					if(last_texture!=texture_cache[sigils_text].texture_id)
						{
							glBindTexture(GL_TEXTURE_2D, texture_cache[sigils_text].texture_id);
							last_texture=texture_cache[sigils_text].texture_id;
						}

					glBegin(GL_QUADS);
					draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
					glEnd();

				}
		}
	//glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
}

void display_sigils_menu()
{
	int x,y,i;
	//first of all, draw the actual menu.

	draw_menu_title_bar(sigil_menu_x,sigil_menu_y-16,sigil_menu_x_len);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(sigil_menu_x,sigil_menu_y+sigil_menu_y_len,0);
	glVertex3i(sigil_menu_x,sigil_menu_y,0);
	glVertex3i(sigil_menu_x+sigil_menu_x_len,sigil_menu_y,0);
	glVertex3i(sigil_menu_x+sigil_menu_x_len,sigil_menu_y+sigil_menu_y_len,0);
	glEnd();

	glDisable(GL_BLEND);

	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	glVertex3i(sigil_menu_x,sigil_menu_y,0);
	glVertex3i(sigil_menu_x+sigil_menu_x_len,sigil_menu_y,0);

	glVertex3i(sigil_menu_x+sigil_menu_x_len,sigil_menu_y,0);
	glVertex3i(sigil_menu_x+sigil_menu_x_len,sigil_menu_y+sigil_menu_y_len,0);

	glVertex3i(sigil_menu_x+sigil_menu_x_len,sigil_menu_y+sigil_menu_y_len,0);
	glVertex3i(sigil_menu_x,sigil_menu_y+sigil_menu_y_len,0);

	glVertex3i(sigil_menu_x,sigil_menu_y+sigil_menu_y_len,0);
	glVertex3i(sigil_menu_x,sigil_menu_y,0);

	//draw the grid
	for(y=1;y<4;y++)
		{
			glVertex3i(sigil_menu_x,sigil_menu_y+y*33,0);
			glVertex3i(sigil_menu_x+12*33,sigil_menu_y+y*33,0);
		}
	for(x=1;x<13;x++)
		{
			glVertex3i(sigil_menu_x+x*33,sigil_menu_y,0);
			glVertex3i(sigil_menu_x+x*33,sigil_menu_y+3*33,0);
		}

	//draw the bottom grid
	for(y=1;y<2;y++)
		{
			glVertex3i(sigil_menu_x,sigil_menu_y+y*5*33,0);
			glVertex3i(sigil_menu_x+6*33,sigil_menu_y+y*5*33,0);
		}
	for(x=1;x<7;x++)
		{
			glVertex3i(sigil_menu_x+x*33,sigil_menu_y+5*33,0);
			glVertex3i(sigil_menu_x+x*33,sigil_menu_y+6*33,0);
		}

	glColor3f(0.77f,0.57f,0.39f);
	//draw the corner, with the X in
	glVertex3i(sigil_menu_x+sigil_menu_x_len,sigil_menu_y+20,0);
	glVertex3i(sigil_menu_x+sigil_menu_x_len-20,sigil_menu_y+20,0);
	glVertex3i(sigil_menu_x+sigil_menu_x_len-20,sigil_menu_y+20,0);
	glVertex3i(sigil_menu_x+sigil_menu_x_len-20,sigil_menu_y,0);


	//draw the buttons frame
	//Mix button
	glVertex3i(sigil_menu_x+33*6+40,sigil_menu_y+sigil_menu_y_len-30,0);
	glVertex3i(sigil_menu_x+33*6+40+50,sigil_menu_y+sigil_menu_y_len-30,0);

	glVertex3i(sigil_menu_x+33*6+40,sigil_menu_y+sigil_menu_y_len-10,0);
	glVertex3i(sigil_menu_x+33*6+40+50,sigil_menu_y+sigil_menu_y_len-10,0);

	glVertex3i(sigil_menu_x+33*6+40+50,sigil_menu_y+sigil_menu_y_len-30,0);
	glVertex3i(sigil_menu_x+33*6+40+50,sigil_menu_y+sigil_menu_y_len-9,0);

	glVertex3i(sigil_menu_x+33*6+40,sigil_menu_y+sigil_menu_y_len-30,0);
	glVertex3i(sigil_menu_x+33*6+40,sigil_menu_y+sigil_menu_y_len-10,0);

	//Clear button
	glVertex3i(sigil_menu_x+33*9+40,sigil_menu_y+sigil_menu_y_len-30,0);
	glVertex3i(sigil_menu_x+33*9+40+70,sigil_menu_y+sigil_menu_y_len-30,0);

	glVertex3i(sigil_menu_x+33*9+40,sigil_menu_y+sigil_menu_y_len-10,0);
	glVertex3i(sigil_menu_x+33*9+40+70,sigil_menu_y+sigil_menu_y_len-10,0);

	glVertex3i(sigil_menu_x+33*9+40+70,sigil_menu_y+sigil_menu_y_len-30,0);
	glVertex3i(sigil_menu_x+33*9+40+70,sigil_menu_y+sigil_menu_y_len-9,0);

	glVertex3i(sigil_menu_x+33*9+40,sigil_menu_y+sigil_menu_y_len-30,0);
	glVertex3i(sigil_menu_x+33*9+40,sigil_menu_y+sigil_menu_y_len-10,0);

	glEnd();
	glEnable(GL_TEXTURE_2D);

	draw_string(sigil_menu_x+sigil_menu_x_len-16,sigil_menu_y+2,"X",1);
	if(cast_mouseover)
		glColor3f(0.87f,0.67f,0.49f);
	else
		glColor3f(0.77f,0.57f,0.39f);
	draw_string(sigil_menu_x+33*6+40+4,sigil_menu_y+sigil_menu_y_len-30+2,"Cast",1);

	if(clear_mouseover)
		glColor3f(0.87f,0.67f,0.49f);
	else
		glColor3f(0.77f,0.57f,0.39f);
	draw_string(sigil_menu_x+33*9+40+8,sigil_menu_y+sigil_menu_y_len-30+2,"Clear",1);

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now let's draw the objects...
	for(i=0;i<SIGILS_NO;i++)
		{
			if(sigils_list[i].have_sigil)
				{
					float u_start,v_start,u_end,v_end;
					int cur_item,cur_pos;
					int x_start,x_end,y_start,y_end;

					//get the UV coordinates.
					cur_item=sigils_list[i].sigil_img;
					u_start=0.125f*(cur_item%8);
					u_end=u_start+0.125f;
					v_start=1.0f-(0.125f*(cur_item/8));
					v_end=v_start-0.125f;

					//get the x and y
					cur_pos=i;

					x_start=sigil_menu_x+33*(cur_pos%12)+1;
					x_end=x_start+32;
					y_start=sigil_menu_y+33*(cur_pos/12);
					y_end=y_start+32;


					if(last_texture!=texture_cache[sigils_text].texture_id)
						{
							glBindTexture(GL_TEXTURE_2D, texture_cache[sigils_text].texture_id);
							last_texture=texture_cache[sigils_text].texture_id;
						}

					glBegin(GL_QUADS);
					draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
					glEnd();

				}

		}

	//ok, now let's draw the sigils on the list
	for(i=0;i<6;i++)
		{
			if(on_cast[i]!=-1)
				{
					float u_start,v_start,u_end,v_end;
					int cur_item,cur_pos;
					int x_start,x_end,y_start,y_end;

					//get the UV coordinates.
					cur_item=on_cast[i];
					u_start=0.125f*(cur_item%8);
					u_end=u_start+0.125f;
					v_start=1.0f-(0.125f*(cur_item/8));
					v_end=v_start-0.125f;

					//get the x and y
					cur_pos=i;

					x_start=sigil_menu_x+33*(cur_pos%6)+1;
					x_end=x_start+32;
					y_start=sigil_menu_y+33*5;
					y_end=y_start+32;

					//get the texture this item belongs to
					if(last_texture!=texture_cache[sigils_text].texture_id)
						{
							glBindTexture(GL_TEXTURE_2D, texture_cache[sigils_text].texture_id);
							last_texture=texture_cache[sigils_text].texture_id;
						}

					glBegin(GL_QUADS);
					draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
					glEnd();

				}

		}

	//now, draw the inventory text, if any.
	draw_string_small(sigil_menu_x+4,sigil_menu_y+sigil_menu_y_len-90,spell_text,4);
	glColor3f(1.0f,1.0f,1.0f);
}


int check_sigil_interface()
{
	int i,x,y;
	int x_screen,y_screen;

	if(!view_sigils_menu || mouse_x>sigil_menu_x+sigil_menu_x_len || mouse_x<sigil_menu_x
	   || mouse_y<sigil_menu_y || mouse_y>sigil_menu_y+sigil_menu_y_len)return 0;

	//clear button pressed?
	if(mouse_x>sigil_menu_x+33*9+40 && mouse_x<sigil_menu_x+33*9+40+70 &&
	   mouse_y>sigil_menu_y+sigil_menu_y_len-30 && mouse_y<sigil_menu_y+sigil_menu_y_len-10)
		{
			for(i=0;i<6;i++)on_cast[i]=-1;
			for(i=0;i<6;i++)cast_cache[i]=-1;
			return 1;
		}

	if(mouse_x>sigil_menu_x+33*6+40 && mouse_x<sigil_menu_x+33*6+40+50 &&
	   mouse_y>sigil_menu_y+sigil_menu_y_len-30 && mouse_y<sigil_menu_y+sigil_menu_y_len-10)
		{
			//Cast?
			Uint8 str[20];
			int count=0;
			int sigils_no=0;


			for(i=0;i<6;i++)
				{
					if(on_cast[i]!=-1)
						count++;
				}

			if(count<2)
				{
					sprintf(spell_text,"%cYou need at least 2 sigils for a spell.",127+c_red2);
					have_error_message=1;
					return 1;
				}
			str[0]=CAST_SPELL;
			for(i=0;i<6;i++)
				{
					if(on_cast[i]!=-1)
						{
							str[sigils_no+2]=on_cast[i];
							sigils_no++;
						}
					cast_cache[i]=on_cast[i];	// cache this spell
				}

			str[1]=sigils_no;
			my_tcp_send(my_socket,str,sigils_no+2);
			return 1;
			//ok, send it to the server...
		}

	//see if we clicked on any sigil in the main category
	for(y=0;y<3;y++)
		for(x=0;x<12;x++)
			{
				x_screen=sigil_menu_x+x*33;
				y_screen=sigil_menu_y+y*33;
				if(mouse_x>x_screen && mouse_x<x_screen+33 && mouse_y>y_screen && mouse_y<y_screen+33)
					{

						//see if there is any sigil there
						if(sigils_list[y*12+x].have_sigil)
							{

								int j;
								int image_id=sigils_list[y*12+x].sigil_img;

								//see if it is already on the list
								for(j=0;j<6;j++)
									if(on_cast[j]==image_id)return 1;


								for(j=0;j<6;j++)
									if(on_cast[j]==-1)
										{
											on_cast[j]=image_id;
											return 1;
										}
								return 1;
							}
					}
			}

	//see if we clicked on any sigil from "on cast"
	for(x=0;x<6;x++)
		{
			x_screen=sigil_menu_x+x*33;
			y_screen=sigil_menu_y+5*33;
			if(mouse_x>x_screen && mouse_x<x_screen+33 && mouse_y>y_screen && mouse_y<y_screen+33)
				{
					on_cast[x]=-1;
					return 1;
				}
		}
	return 1;
}


void check_sigil_mouseover()
{
	int x,y;
	int x_screen,y_screen;

	if(!have_error_message)spell_text[0]=0;

	if(!view_sigils_menu || mouse_x>sigil_menu_x+sigil_menu_x_len || mouse_x<sigil_menu_x
	   || mouse_y<sigil_menu_y || mouse_y>sigil_menu_y+sigil_menu_y_len)return;

	//clear button?
	if(mouse_x>sigil_menu_x+33*9+40 && mouse_x<sigil_menu_x+33*9+40+70 &&
	   mouse_y>sigil_menu_y+sigil_menu_y_len-30 && mouse_y<sigil_menu_y+sigil_menu_y_len-10)
		clear_mouseover=1;
	else
		clear_mouseover=0;


	if(mouse_x>sigil_menu_x+33*6+40 && mouse_x<sigil_menu_x+33*6+40+50 &&
	   mouse_y>sigil_menu_y+sigil_menu_y_len-30 && mouse_y<sigil_menu_y+sigil_menu_y_len-10)
		cast_mouseover=1;
	else
		cast_mouseover=0;

	//see if we clicked on any sigil in the main category
	for(y=0;y<3;y++)
		for(x=0;x<12;x++)
			{
				x_screen=sigil_menu_x+x*33;
				y_screen=sigil_menu_y+y*33;
				if(mouse_x>x_screen && mouse_x<x_screen+33 && mouse_y>y_screen && mouse_y<y_screen+33)
					{

						//see if there is any sigil there
						if(sigils_list[y*12+x].have_sigil)
							{
								my_strcp(spell_text,sigils_list[y*12+x].name);
								have_error_message=0;
								return;
							}
					}
			}

	//see if we clicked on any sigil from "on cast"
	for(x=0;x<6;x++)
		{
			x_screen=sigil_menu_x+x*33;
			y_screen=sigil_menu_y+5*33;
			if(mouse_x>x_screen && mouse_x<x_screen+33 && mouse_y>y_screen && mouse_y<y_screen+33)
				if(on_cast[x]!=-1)
					{
						my_strcp(spell_text,sigils_list[on_cast[x]].name);
						have_error_message=0;
						return;
					}
		}
}

void get_sigils_we_have(Uint32 sigils_we_have)
{
	int i;
	int po2=1;

	for(i=0;i<32;i++)
		{
			if((sigils_we_have&po2))sigils_list[i].have_sigil=1;
			else sigils_list[i].have_sigil=0;
			po2*=2;
		}

}



#include <string.h>
#include "global.h"
#include "elwindows.h"

sigil_def sigils_list[SIGILS_NO];
int sigil_win=0;
int sigil_menu_x=10;
int sigil_menu_y=20;
int sigil_menu_x_len=12*33+20;
int sigil_menu_y_len=6*33;

int sigils_text;
Uint8 spell_text[256];
int sigils_we_have;
int have_error_message=0;
Sint8 active_spells[10];

Sint8 on_cast[6];
int clear_mouseover=0;
int cast_mouseover=0;

char	last_spell_str[20];
int		last_spell_len= 0;

void repeat_spell()
{
	if(last_spell_len > 0)
		{
			my_tcp_send(my_socket, last_spell_str, last_spell_len);
		}
}

void make_sigils_list()
{
	int i;

	for(i=0;i<SIGILS_NO;i++)sigils_list[i].have_sigil=0;

	spell_text[0]=0;
	i=0;

	sigils_list[i].sigil_img=0;
	my_strcp(sigils_list[i].name,sig_change.str);
	my_strcp(sigils_list[i].description,sig_change.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=1;
	my_strcp(sigils_list[i].name,sig_restore.str);
	my_strcp(sigils_list[i].description,sig_restore.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=2;
	my_strcp(sigils_list[i].name,sig_space.str);
	my_strcp(sigils_list[i].description,sig_space.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=3;
	my_strcp(sigils_list[i].name,sig_increase.str);
	my_strcp(sigils_list[i].description,sig_increase.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=4;
	my_strcp(sigils_list[i].name,sig_decrease.str);
	my_strcp(sigils_list[i].description,sig_decrease.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=5;
	my_strcp(sigils_list[i].name,sig_temp.str);
	my_strcp(sigils_list[i].description,sig_temp.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=6;
	my_strcp(sigils_list[i].name,sig_perm.str);
	my_strcp(sigils_list[i].description,sig_perm.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=7;
	my_strcp(sigils_list[i].name,sig_move.str);
	my_strcp(sigils_list[i].description,sig_move.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=8;
	my_strcp(sigils_list[i].name,sig_local.str);
	my_strcp(sigils_list[i].description,sig_local.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=9;
	my_strcp(sigils_list[i].name,sig_global.str);
	my_strcp(sigils_list[i].description,sig_global.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=10;
	my_strcp(sigils_list[i].name,sig_fire.str);
	my_strcp(sigils_list[i].description,sig_fire.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=11;
	my_strcp(sigils_list[i].name,sig_water.str);
	my_strcp(sigils_list[i].description,sig_water.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=12;
	my_strcp(sigils_list[i].name,sig_air.str);
	my_strcp(sigils_list[i].description,sig_air.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=13;
	my_strcp(sigils_list[i].name,sig_earth.str);
	my_strcp(sigils_list[i].description,sig_earth.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=14;
	my_strcp(sigils_list[i].name,sig_spirit.str);
	my_strcp(sigils_list[i].description,sig_spirit.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=15;
	my_strcp(sigils_list[i].name,sig_matter.str);
	my_strcp(sigils_list[i].description,sig_matter.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=16;
	my_strcp(sigils_list[i].name,sig_energy.str);
	my_strcp(sigils_list[i].description,sig_energy.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=17;
	my_strcp(sigils_list[i].name,sig_magic.str);
	my_strcp(sigils_list[i].description,sig_magic.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=18;
	my_strcp(sigils_list[i].name,sig_destroy.str);
	my_strcp(sigils_list[i].description,sig_destroy.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=19;
	my_strcp(sigils_list[i].name,sig_create.str);
	my_strcp(sigils_list[i].description,sig_create.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=20;
	my_strcp(sigils_list[i].name,sig_knowledge.str);
	my_strcp(sigils_list[i].description,sig_knowledge.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=21;
	my_strcp(sigils_list[i].name,sig_protection.str);
	my_strcp(sigils_list[i].description,sig_protection.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=22;
	my_strcp(sigils_list[i].name,sig_remove.str);
	my_strcp(sigils_list[i].description,sig_remove.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=23;
	my_strcp(sigils_list[i].name,sig_health.str);
	my_strcp(sigils_list[i].description,sig_health.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=24;
	my_strcp(sigils_list[i].name,sig_life.str);
	my_strcp(sigils_list[i].description,sig_life.desc);
	sigils_list[i].have_sigil=1;

	i++;
	sigils_list[i].sigil_img=25;
	my_strcp(sigils_list[i].name,sig_death.str);
	my_strcp(sigils_list[i].description,sig_death.desc);
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

					get_and_set_texture_id(sigils_text);
					glBegin(GL_QUADS);
					draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
					glEnd();
				}
		}
	//glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
}

int display_sigils_handler(window_info *win)
{
	int x,y,i;

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
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

	if(cast_mouseover)
		glColor3f(0.87f,0.67f,0.49f);
	else
		glColor3f(0.77f,0.57f,0.39f);
	draw_string(33*6+40+4,win->len_y-30+2,"Cast",1);

	if(clear_mouseover)
		glColor3f(0.87f,0.67f,0.49f);
	else
		glColor3f(0.77f,0.57f,0.39f);
	draw_string(33*9+40+8,win->len_y-30+2,"Clear",1);

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

					x_start=33*(cur_pos%12)+1;
					x_end=x_start+32;
					y_start=33*(cur_pos/12);
					y_end=y_start+32;

					get_and_set_texture_id(sigils_text);
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

					x_start=33*(cur_pos%6)+1;
					x_end=x_start+32;
					y_start=33*5;
					y_end=y_start+32;

					//get the texture this item belongs to
					get_and_set_texture_id(sigils_text);
					glBegin(GL_QUADS);
					draw_2d_thing(u_start,v_start,u_end,v_end,x_start,y_start,x_end,y_end);
					glEnd();
				}
		}

	//now, draw the inventory text, if any.
	draw_string_small(4,win->len_y-90,spell_text,4);
	glColor3f(1.0f,1.0f,1.0f);
	return 1;
}


int click_sigils_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i,x,y;
	int x_screen,y_screen;

	//clear button pressed?
	if(mx>33*9+40 && mx<33*9+40+70 &&
	   my>win->len_y-30 && my<win->len_y-10)
		{
			for(i=0;i<6;i++)on_cast[i]=-1;
			return 1;
		}

	if(mx>33*6+40 && mx<33*6+40+50 &&
	   my>win->len_y-30 && my<win->len_y-10)
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
					sprintf(spell_text,"%c%s",127+c_red2,sig_too_few_sigs);
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
				}

			str[1]=sigils_no;
			last_spell_len= sigils_no+2;
			my_tcp_send(my_socket, str, last_spell_len);
			memcpy(last_spell_str, str, last_spell_len);
			return 1;
			//ok, send it to the server...
		}

	//see if we clicked on any sigil in the main category
	for(y=0;y<3;y++)
		for(x=0;x<12;x++)
			{
				x_screen=x*33;
				y_screen=y*33;
				if(mx>x_screen && mx<x_screen+33 && my>y_screen && my<y_screen+33)
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
			x_screen=x*33;
			y_screen=5*33;
			if(mx>x_screen && mx<x_screen+33 && my>y_screen && my<y_screen+33)
				{
					on_cast[x]=-1;
					return 1;
				}
		}
	return 0;
}


int mouseover_sigils_handler(window_info *win, int mx, int my)
{
	int x,y;
	int x_screen,y_screen;

	if(!have_error_message)spell_text[0]=0;

	//clear button?
	if(mx>33*9+40 && mx<33*9+40+70 &&
	   my>win->len_y-30 && my<win->len_y-10)
		clear_mouseover=1;
	else
		clear_mouseover=0;


	if(mx>33*6+40 && mx<33*6+40+50 &&
	   my>win->len_y-30 && my<win->len_y-10)
		cast_mouseover=1;
	else
		cast_mouseover=0;

	//see if we clicked on any sigil in the main category
	for(y=0;y<3;y++)
		for(x=0;x<12;x++)
			{
				x_screen=x*33;
				y_screen=y*33;
				if(mx>x_screen && mx<x_screen+33 && my>y_screen && my<y_screen+33)
					{

						//see if there is any sigil there
						if(sigils_list[y*12+x].have_sigil)
							{
								my_strcp(spell_text,sigils_list[y*12+x].name);
								have_error_message=0;
								return 0;
							}
					}
			}

	//see if we clicked on any sigil from "on cast"
	for(x=0;x<6;x++)
		{
			x_screen=x*33;
			y_screen=5*33;
			if(mx>x_screen && mx<x_screen+33 && my>y_screen && my<y_screen+33)
				if(on_cast[x]!=-1)
					{
						my_strcp(spell_text,sigils_list[on_cast[x]].name);
						have_error_message=0;
						return 0;
					}
		}
	return 0;
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


void display_sigils_menu()
{
	if(sigil_win <= 0){
		sigil_win= create_window("Sigils", 0, 0, sigil_menu_x, sigil_menu_y, sigil_menu_x_len, sigil_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(sigil_win, ELW_HANDLER_DISPLAY, &display_sigils_handler );
		set_window_handler(sigil_win, ELW_HANDLER_CLICK, &click_sigils_handler );
		set_window_handler(sigil_win, ELW_HANDLER_MOUSEOVER, &mouseover_sigils_handler );
	} else {
		show_window(sigil_win);
		select_window(sigil_win);
	}
	display_window(sigil_win);
}

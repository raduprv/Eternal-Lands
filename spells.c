#include "global.h"
#include "elwindows.h"

sigil_def sigils_list[SIGILS_NO];
int sigil_win=0;
int sigil_menu_x=10;
int sigil_menu_y=20;
int sigil_menu_x_len=12*33+20;
int sigil_menu_y_len=360;

int sigils_text;
Uint8 spell_text[256];
int sigils_we_have;
int have_error_message=0;
Sint8 active_spells[10];

Sint8 on_cast[6];
Sint8 cast_cache[6];
int clear_mouseover=0;
int cast_mouseover=0;
Uint8 str[80];
int magiclevel;

int sChange = 0;
int sRestore = 1;
int sSpace = 2;
int sIncrease = 3;
int sDecrease = 4;
int sTemporary = 5;
int sPermanent = 6;
int sMove = 7;
int sLocal = 8;
int sGlobal = 9;
int sFire = 10;
int sWater = 11;
int sAir = 12;
int sEarch = 13;
int sSpirit = 14;
int sMatter = 15;
int sEnergy = 16;
int sMagic = 17;
int sDestroy = 18;
int sCreate = 19;
int sKnowledge = 20;
int sProtection = 21;
int sRemove = 22;
int sHealth = 23;
int sLife = 24;
int sDeath = 25;
void repeat_spell()
{
	//TODO:!!
}

void make_spell_list()
{
int i = 0;

	spell_list[i].spell_level=0;
	my_strcp(spell_list[i].description ,"");	//Description of spell can be put here for mouseover event
	my_strcp(spell_list[i].name  ,"Heal");
	spell_list[i].sigilcount=2;
	spell_list[i].pos_x=10;
	spell_list[i].pos_y=220;

	i++;
	spell_list[i].spell_level=3;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Remote Heal");
	spell_list[i].sigilcount=3;
	spell_list[i].pos_x=10;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;

	i++;
	spell_list[i].spell_level=6;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Magic Protection");
	spell_list[i].sigilcount=3;
	spell_list[i].pos_x=10;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;


	i++;
	spell_list[i].spell_level=9;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Shield");
	spell_list[i].sigilcount=3;
	spell_list[i].pos_x=10;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;

	i++;
	spell_list[i].spell_level=12;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Poison");
	spell_list[i].sigilcount=3;
	spell_list[i].pos_x=10;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;

	i++;
	spell_list[i].spell_level=15;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Local Teleport");
	spell_list[i].sigilcount=4;
	spell_list[i].pos_x=10;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;

	i++;
	spell_list[i].spell_level=18;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Harm");
	spell_list[i].sigilcount=2;
	spell_list[i].pos_x=10;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;

	i++;
	spell_list[i].spell_level=21;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Restoration");
	spell_list[i].sigilcount=2;
	spell_list[i].pos_x=210;
	spell_list[i].pos_y=220;

	i++;
	spell_list[i].spell_level=23;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Bones to Gold");
	spell_list[i].sigilcount=3;
	spell_list[i].pos_x=210;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;

	i++;
	spell_list[i].spell_level=24;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"PortalRoom");
	spell_list[i].sigilcount=4;
	spell_list[i].pos_x=210;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;

	i++;
	spell_list[i].spell_level=27;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Life Drain");
	spell_list[i].sigilcount=3;
	spell_list[i].pos_x=210;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;

	i++;
	spell_list[i].spell_level=30;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Magic Immunity");
	spell_list[i].sigilcount=4;
	spell_list[i].pos_x=210;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;

	i++;
	spell_list[i].spell_level=33;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Heal Summoned");
	spell_list[i].sigilcount=4;
	spell_list[i].pos_x=210;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;

	i++;
	spell_list[i].spell_level=36;
	my_strcp(spell_list[i].description ,"");
	my_strcp(spell_list[i].name  ,"Smite Summoned");
	spell_list[i].sigilcount=4;
	spell_list[i].pos_x=210;
	spell_list[i].pos_y=spell_list[i-1].pos_y+20;

	for (i==0;i<SPELL_NO;i++)
	{
		spell_list[i].have_all_sigils = check_for_sigil_available(i);
	}
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

	magiclevel = your_info.magic_skill.cur  ;	//check our magic level for the colour of the spell
	make_spell_list();
}

int check_for_sigil_available(int i)
{
	switch (i)
	{
	case 0:
		if (sigils_list[sIncrease].have_sigil ==0 || sigils_list[sHealth].have_sigil == 0 )return 0;else return 1;
		break;
	case 1:
		if (sigils_list[sGlobal].have_sigil ==0 || sigils_list[sIncrease].have_sigil == 0 || sigils_list[sHealth].have_sigil==0)return 0;else return 1;
		break;
	case 2:
		if (sigils_list[sCreate].have_sigil ==0 || sigils_list[sMagic].have_sigil == 0 || sigils_list[sProtection].have_sigil==0)return 0;else return 1;
		break;
	case 3:
		if (sigils_list[sCreate].have_sigil ==0 || sigils_list[sMatter].have_sigil == 0 || sigils_list[sProtection].have_sigil==0)return 0;else return 1;
		break;
	case 4:
		if (sigils_list[sTemporary].have_sigil ==0 || sigils_list[sDecrease].have_sigil == 0 || sigils_list[sLife].have_sigil==0)return 0;else return 1;
		break;
	case 5:
		if (sigils_list[sMove].have_sigil ==0 || sigils_list[sChange].have_sigil == 0 || sigils_list[sLocal].have_sigil==0|| sigils_list[sSpace].have_sigil==0)return 0;else return 1;
		break;
	case 6:
		if (sigils_list[sRemove].have_sigil ==0 || sigils_list[sHealth].have_sigil == 0)return 0;else return 1;
		break;
	case 7:
		if (sigils_list[sRestore].have_sigil ==0 || sigils_list[sLife].have_sigil == 0 )return 0;else return 1;
		break;
	case 8:
		if (sigils_list[sPermanent].have_sigil ==0 || sigils_list[sChange].have_sigil == 0 || sigils_list[sMatter].have_sigil==0)return 0;else return 1;
		break;
	case 9:
		if (sigils_list[sMove].have_sigil ==0 || sigils_list[sChange].have_sigil == 0 || sigils_list[sGlobal].have_sigil==0|| sigils_list[sSpace].have_sigil==0)return 0;else return 1;
		break;
	case 10:
		if (sigils_list[sMove].have_sigil ==0 || sigils_list[sChange].have_sigil == 0 || sigils_list[sLife].have_sigil==0)return 0;else return 1;
		break;
	case 11:
		if (sigils_list[sCreate].have_sigil ==0 || sigils_list[sIncrease].have_sigil == 0 || sigils_list[sMagic].have_sigil==0|| sigils_list[sProtection].have_sigil==0)return 0;else return 1;
		break;
	case 12:
		if (sigils_list[sLocal].have_sigil ==0 || sigils_list[sSpace].have_sigil == 0 || sigils_list[sHealth].have_sigil==0|| sigils_list[sSpirit].have_sigil==0)return 0;else return 1;
		break;
	case 13:
		if (sigils_list[sLocal].have_sigil ==0 || sigils_list[sSpace].have_sigil == 0 || sigils_list[sRemove].have_sigil==0|| sigils_list[sLife].have_sigil==0)return 0;else return 1;
		break;
	}
	//we should not get here
	return 0;

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
player_attribs cur_stats= your_info;
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
	glDisable(GL_BLEND);
}

int display_sigils_handler(window_info *win)
{
	int x,y,i,blabla;
	player_attribs cur_stats= your_info;
	magiclevel = your_info.magic_skill.cur ;
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
	//draw seperators between selected sigils and spell shortcuts
	glVertex3i(0,198,0);
	glVertex3i(win->len_x ,198,0);
	glVertex3i(0,210,0);
	glVertex3i(win->len_x ,210,0);
	//draw seperator between spell shortcuts left and right
	glVertex3i(200,210,0);
	glVertex3i(200 ,360,0);

	//draw the buttons frame
	//Cast button
	glVertex3i(238,168,0);	//top line
	glVertex3i(288,168,0);

	glVertex3i(238,188,0);	//bottom line
	glVertex3i(288,188,0);

	glVertex3i(238,168,0);	//left line
	glVertex3i(238,188,0);

	glVertex3i(288,168,0);	//right line
	glVertex3i(288,188,0);

	//Clear button
	glVertex3i(337,168,0);
	glVertex3i(407,168,0);

	glVertex3i(337,188,0);
	glVertex3i(407,188,0);

	glVertex3i(407,168,0);
	glVertex3i(407,188,0);

	glVertex3i(337,168,0);
	glVertex3i(337,188,0);


	glEnd();
	glEnable(GL_TEXTURE_2D);

	if(cast_mouseover)
		glColor3f(0.87f,0.67f,0.49f);
	else
		glColor3f(0.77f,0.57f,0.39f);
		draw_string(242,170,"Cast",1);
	if(clear_mouseover)
		glColor3f(0.87f,0.67f,0.49f);
	else
		glColor3f(0.77f,0.57f,0.39f);
		draw_string(345,170,"Clear",1);

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

	for(i=0;i<14;i++)
	{
		if (magiclevel >= spell_list[i].spell_level)
			{
			if (check_for_sigil_available(i)==1)
				{
					glColor3f(1.0f,1.0f,1.0f);
				}
				else 
				{
					glColor3f(0.5f,0.0f,0.0f);
				}
			}
		else 
			{	
			glColor3f(0.5f,0.5f,0.5f);
			}
		draw_string(spell_list[i].pos_x ,spell_list[i].pos_y ,spell_list[i].name ,1);
	}

	//now, draw the inventory text, if any.
	glColor3f(1.0f,0.0f,0.0f);
	draw_string_small(4,108,spell_text,4);
	glColor3f(1.0f,1.0f,1.0f);
	return 1;
}	//display_sigils_handler end

//clear spell :
int click_sigils_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i,j,x,y,sigilresult;
	int bla;//remove later
	int x_screen,y_screen;
	have_error_message=0;
	//clear button pressed?
	if (mx > 337 && mx < 407 && my > 168 && my < 188)
		{
			for(i=0;i<6;i++)on_cast[i]=-1;
			for(i=0;i<6;i++)cast_cache[i]=-1;
			return 1;
		}
	//Spell shortcut ?
	for(i=0;i<14;i++)
	{
		if(mx>spell_list[i].pos_x && mx<spell_list[i].pos_x+190 && my>spell_list[i].pos_y && my<spell_list[i].pos_y+15)
			{
				sigilresult = get_sigils_for_spell(i);
				if (sigilresult != spell_list[i].sigilcount)
					{
					sprintf(spell_text,"%c You have not all the sigils for %s",127+c_red2,spell_list[i].name);
					for(j=0;j<6;j++)on_cast[j]=-1;
					for(j=0;j<6;j++)cast_cache[j]=-1;
					}
				else
					{
					sprintf(spell_text,"%c %s",127+c_blue2,spell_list[i].name);
					}
				if (magiclevel < spell_list[i].spell_level )
					{
					sprintf(spell_text,"%c You have not all the level for %s",127+c_red2,spell_list[i].name);
					for(j=0;j<6;j++)on_cast[j]=-1;
					for(j=0;j<6;j++)cast_cache[j]=-1;
					}
				have_error_message=1;
				return 1;
			}
	}



if (mx > 238 && mx < 288 && my > 168 && my < 188)
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
//	if(mx>33*9+40 && mx<33*9+40+70 &&
//	   my>win->len_y-30 && my<win->len_y-10)
	if (mx > 337 && mx < 407 && my > 168 && my < 188)
		clear_mouseover=1;
	else
		clear_mouseover=0;

	if (mx > 238 && mx < 288 && my > 168 && my < 188)
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

int get_sigils_for_spell(int i)
{
	int j,result;
	result = 0;
	for(j=0;j<6;j++)on_cast[j]=-1;
	for(j=0;j<6;j++)cast_cache[j]=-1;
	switch (i)
	{
	case 0:
		{
		result = place_sigils(sIncrease);
		result = result + place_sigils(sHealth);
		return result;
		}
	case 1:
		{
		result = place_sigils(sGlobal);
		result = result + place_sigils(sIncrease);
		result = result + place_sigils(sHealth);
		return result;
		}
	case 2:
		{

		result = place_sigils(sCreate);
		result = result + place_sigils(sMagic);
		result = result + place_sigils(sProtection);
		return result;
		}
	case 3:
		{
		result = place_sigils(sCreate);
		result = result + place_sigils(sMatter);
		result = result + place_sigils(sProtection);
		return result;
		}
	case 4:
		{
		result = place_sigils(sTemporary);
		result = result + place_sigils(sDecrease);
		result = result + place_sigils(sLife);
		return result;
		}
	case 5:
		{
		result = place_sigils(sMove);
		result = result + place_sigils(sChange);
		result = result + place_sigils(sLocal);
		result = result + place_sigils(sSpace);
		return result;
		}
	case 6:
		{
		result = place_sigils(sRemove);
		result = result + place_sigils(sHealth);
		return result;
		}
	case 7:
		{
		result = place_sigils(sRestore);
		result = result + place_sigils(sLife);
		return result;
		}
	case 8:
		{
		result = place_sigils(sPermanent);
		result = result + place_sigils(sChange);
		result = result + place_sigils(sMatter);
		return result;
		}
	case 9:
		{
		result = place_sigils(sMove);
		result = result + place_sigils(sChange);
		result = result + place_sigils(sGlobal);
		result = result + place_sigils(sSpace);
		return result;
		}
	case 10:
		{
		result = place_sigils(sMove);
		result = result + place_sigils(sChange);
		result = result + place_sigils(sLife);
		return result;
		}
	case 11:
		{
		result = place_sigils(sCreate);
		result = result + place_sigils(sIncrease);
		result = result + place_sigils(sMagic);
		result = result + place_sigils(sProtection);
		return result;
		}
	case 12:
		{
		result = place_sigils(sLocal);
		result = result + place_sigils(sSpace);
		result = result + place_sigils(sHealth);
		result = result + place_sigils(sSpirit);
		return result;
		}
	case 13:
		{
		result = place_sigils(sLocal);
		result = result + place_sigils(sSpace);
		result = result + place_sigils(sRemove);
		result = result + place_sigils(sLife);
		return result;
		}
	}
return 0;

}
int place_sigils(a)
{
int x,y;
char str[50];
y = a/12;
x = a - (12*(a/12));

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
	else return 0;

}
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "elwindows.h"
#include "interface.h"
#include "eye_candy_wrapper.h"

#define SIGILS_NO 50
#define MAX_DATA_FILE_SIZE 560

typedef struct
{
	int sigil_img;
	char name[32];
	char description[64];
	int have_sigil;
}sigil_def;

sigil_def sigils_list[SIGILS_NO];
int sigil_win=-1;
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
int spell_result=0;

mqbdata * mqb_data[7]={NULL};//mqb_data will hold the magic quickbar name, image, pos.
int quickspells=6;
int quickspell_size=20;//size of displayed icons in pixels
int quickspell_columns=1;
int quickspell_rows=6;
int quickspell_x_len=26;
int quickspell_y_len=6*30;
int quickspell_x=60;
int quickspell_y=64;
int quickspells_loaded = 0;

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

void get_active_spell_list(const Uint8 *my_spell_list)
{
	int i;
	for (i = 0; i < 10; i++)
		active_spells[i] = my_spell_list[i];
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
			y_start=window_height-hud_y-64;
			y_end=y_start+32;

			get_and_set_texture_id(sigils_text);
			glBegin(GL_QUADS);
			draw_2d_thing (u_start, v_start, u_end, v_end, x_start, y_start, x_end, y_end);
			glEnd();
		}
	}
	glDisable(GL_BLEND);
}

int show_last_spell_help=0;

int display_sigils_handler(window_info *win)
{
	int i;

	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0f,1.0f,1.0f);
	//let's add the new spell icon if we have one
	get_and_set_texture_id(sigils_text);
	
	if(mqb_data[0] && mqb_data[0]->spell_id!=-1)
	{
		int x_start,y_start,x_end,y_end;
		float u_start,v_start,u_end,v_end;
		
		x_start=350;
		x_end=x_start+31;
		y_start=112;
		y_end=y_start+31;
		//location in window ready, now for the bitmap..
		u_start=0.125f*(mqb_data[0]->spell_image%8);//0 to 7 across
		v_start=1.0f-((float)32/256*(mqb_data[0]->spell_image/8));
		
		u_end=u_start+0.125f-(1.0f/256.0f);//32 pixels(1/8th of 256, -1/256th)
		v_end=v_start-0.125f-(1.0f/256.0f);//32 pixels(1/8th of 256, -1/256th)
		
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.05f);
		glBegin(GL_QUADS);
			draw_2d_thing(u_start,v_start,u_end,v_end, x_start,y_start,x_end,y_end);
		glEnd();
		glDisable(GL_ALPHA_TEST);
	}
	
	glBegin(GL_QUADS);
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

			draw_2d_thing (u_start, v_start, u_end, v_end, x_start, y_start, x_end, y_end);
		}
	}
	glEnd();

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

			x_start=33*(cur_pos%6)+5;
			x_end=x_start+32;
			y_start=win->len_y-37;
			y_end=y_start+32;

			//get the texture this item belongs to
			get_and_set_texture_id(sigils_text);
			glBegin(GL_QUADS);
			draw_2d_thing (u_start, v_start, u_end, v_end, x_start, y_start, x_end, y_end);
			glEnd();
		}
	}

	//now, draw the inventory text, if any.
	draw_string_small(4,win->len_y-90,spell_text,4);

	// Render the grid *after* the images. It seems impossible to code
	// it such that images are rendered exactly within the boxes on all 
	// cards
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);
	
	rendergrid (12, 3, 0, 0, 33, 33);
	rendergrid (6, 1, 5, win->len_y-37, 33, 33);
	
	glEnable(GL_TEXTURE_2D);
	
	if(show_last_spell_help && mqb_data[0] && mqb_data[0]->spell_id!=-1)show_help(mqb_data[0]->spell_name,350-8*strlen(mqb_data[0]->spell_name),120);
	show_last_spell_help=0;

	return 1;
}


int click_sigils_handler(window_info *win, int mx, int my, Uint32 flags)
{
	// only handle real clicks, not scroll wheel moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0 ) {
		return 0;
	} else if(mx>=350 && mx<=381 && my>=112 && my<=143&&mqb_data[0] && mqb_data[0]->spell_id!=-1) {
		add_spell_to_quickbar();
		return 1;
	} else if(mx>0 && mx<12*33 && my>0 && my<3*33) {
		int pos=get_mouse_pos_in_grid(mx,my, 12, 3, 0, 0, 33, 33);

		if (pos >= 0 && sigils_list[pos].have_sigil) {
			int j;
			int image_id=sigils_list[pos].sigil_img;

			//see if it is already on the list
			for(j=0;j<6;j++) {
				if(on_cast[j]==image_id) {
					return 1;
				}
			}

			for(j=0;j<6;j++) {
				if(on_cast[j]==-1) {
					on_cast[j]=image_id;
					return 1;
				}
			}
			return 1;
		}
	} else if(mx>5 && mx<6*33+5 && my>win->len_y-37 && my<win->len_y-5) {
		int pos=get_mouse_pos_in_grid(mx, my, 6, 1, 5, win->len_y-37, 33, 33);

		if (pos >= 0) {
			on_cast[pos]=-1;
		}
	}
	return 0;
}


int mouseover_sigils_handler(window_info *win, int mx, int my)
{
	if(!have_error_message) {
		spell_text[0] = 0;
	}

	if(mx>=350 && mx<=381 && my>=112 && my<=143&&mqb_data[0] &&mqb_data[0]->spell_name[0]) {
		show_last_spell_help = 1;
	}
	
	//see if we clicked on any sigil in the main category
	if(mx>0 && mx<12*33 && my>0 && my<3*33) {
		int pos=get_mouse_pos_in_grid(mx,my, 12, 3, 0, 0, 33, 33);
		
		if (pos >= 0 && sigils_list[pos].have_sigil)
		{
			my_strcp(spell_text,sigils_list[pos].name);
			have_error_message=0;
		}
		return 0;
	}

	//see if we clicked on any sigil from "on cast"
	if(mx>5 && mx<6*33+5 && my>win->len_y-37 && my<win->len_y-5) {
		int pos=get_mouse_pos_in_grid(mx, my, 6, 1, 5, win->len_y-37, 33, 33);
		
		if (pos >= 0 && on_cast[pos]!=-1){
			my_strcp(spell_text,sigils_list[on_cast[pos]].name);
			have_error_message=0;
		}
		return 0;
	}
	
	if(mx>=350 && mx<=381 && my>=112 && my<=143 && mqb_data[0] && mqb_data[0]->spell_id != -1) {
		snprintf(spell_text, sizeof(spell_text), "Click to add the spell to the quickbar");
		return 0;
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

//Quickspell I/O start
char * invalid_spell_str={"Invalid spell"};

int have_spell_name(int spell_id)
{
	int i;

	for(i=1;i<7;i++){
		if(mqb_data[i] && mqb_data[i]->spell_id==spell_id && mqb_data[i]->spell_name[0]){
			if(mqb_data[0])
				snprintf(mqb_data[0]->spell_name, sizeof(mqb_data[0]->spell_name), "%s", mqb_data[i]->spell_name);
			return 1;
		}
	}
	
	return 0;
}

void add_spell_to_quickbar()
{
	int i;

	if(!mqb_data[0])
		return;
	
	for(i=1;i<7;i++) {
		if(mqb_data[i] && mqb_data[0]->spell_id==mqb_data[i]->spell_id) {
			return;
		}
	}
	
	//Else move the other spells down the quickbar
	if(mqb_data[6]) {
		free(mqb_data[6]);
	}
			
	for(i=6; i; i--){
		mqb_data[i]=mqb_data[i-1];
	}
	
	mqb_data[0]=(mqbdata*)calloc(1,sizeof(mqbdata));
	memcpy(mqb_data[0], mqb_data[1], sizeof(mqbdata));
	save_quickspells();
}

void remove_spell_from_quickbar (int pos)
{
	int i;

	if (pos < 1 || pos > 6 || mqb_data[pos] == NULL) {
		return;
	}
	
	// remove the spell
	free (mqb_data[pos]);
	
	// move the other spells one up
	for (i = pos; i < 6; i++) {
		mqb_data[i] = mqb_data[i+1];
	}
	mqb_data[6] = NULL;
	save_quickspells();
}

void set_spell_name (int id, const char *data, int len)
{
	int i;

	if (len >= 60) return;

#ifdef COUNTERS
	counters_set_spell_name(id, (char *)data, len);
#endif
	
	for (i = 0; i < 7; i++)
	{
		if (mqb_data[i] != NULL && mqb_data[i]->spell_id==id)
		{
			snprintf (mqb_data[i]->spell_name, sizeof(mqb_data[i]->spell_name), "%.*s", len, data);
		}
	}

}

void process_network_spell (const char *data, int len)
{
	switch (data[0])
	{
		case S_INVALID:
			spell_result=0;
			LOG_TO_CONSOLE(c_red1, invalid_spell_str);
			return;
		case S_NAME:
			set_spell_name (data[1], &data[2], len-2);//Will set the spell name of the given ID
			return;;
		case S_SELECT_TARGET://spell_result==3
			spell_result=3;
			action_mode=ACTION_WAND;
			break;
		case S_SELECT_TELE_LOCATION://spell_result==2
			// we're about to teleport, don't let the pathfinder 
			// interfere with our destination
			if (pf_follow_path) pf_destroy_path ();
			spell_result=2;
			action_mode=ACTION_WAND;
			break;
		case S_SUCCES://spell_result==1
			spell_result=1;
			action_mode=ACTION_WALK;
			break;
		case S_FAILED:
			spell_result=0;
			action_mode=ACTION_WALK;
			return;
	}
	
	if(!mqb_data[0]){
		mqb_data[0]=(mqbdata*)calloc(1,sizeof(mqbdata));
		mqb_data[0]->spell_id=-1;
	}
	
	if(mqb_data[0]->spell_id!=data[1]){
		if(!have_spell_name(data[1])){
			char str[2];
			
			str[0]=SPELL_NAME;
			str[1]=data[1];
			my_tcp_send(my_socket, str, 2);
		} 
					
		mqb_data[0]->spell_id=data[1];
		mqb_data[0]->spell_image=data[2];
	}
}

void load_quickspells ()
{
	Uint8 fname[256];
	char data[MAX_DATA_FILE_SIZE];
	FILE *fp;
	Uint8 i;
#ifndef WINDOWS
	char username[20];
#endif

	// Grum: move this over here instead of at the end of the function,
	// so that quickspells are always saved when the player logs in. 
	// (We're only interested in if this function is called, not if it 
	// succeeds)
	quickspells_loaded = 1;
	
#ifndef WINDOWS
	snprintf(username, sizeof(username), "%s", username_str);
	my_tolower(username);
	snprintf (fname, sizeof (fname), "%s/spells_%s.dat", configdir, username);
	fp = my_fopen (fname, "rb"); // try local file first
	if (!fp)
#endif
	{
		//write to the data file, to ensure data integrity, we will write all the information
		sprintf(fname,"spells_%s.dat",username_str);
		my_tolower(fname);
		fp=fopen(fname,"rb");
		if(!fp)
			return;
	}

	fread (data, sizeof(*data), sizeof(data), fp);
	fclose (fp);

	memset (mqb_data, 0, sizeof (mqb_data));
	for (i = 1; i < (int)data[0]; i++)
	{
		mqb_data[i] = (mqbdata*) calloc (1, sizeof(mqbdata));
		memcpy (mqb_data[i], data+1+(i-1)*sizeof(mqbdata), sizeof(mqbdata));
	}	
}

void save_quickspells()
{
	Uint8 fname[128];
	FILE *fp;
	Uint8 i;
	char data[MAX_DATA_FILE_SIZE];
	//extern char username_str[16];
#ifndef WINDOWS
	char username[20];
#endif
	
	if (!quickspells_loaded)
		return;
	
#ifndef WINDOWS	
	snprintf(username, sizeof(username), "%s", username_str);
	my_tolower(username);
	snprintf (fname, sizeof (fname), "%s/spells_%s.dat", configdir, username);
	fp = my_fopen (fname, "wb"); // try local file first
	if (!fp)
#endif
	{
		//write to the data file, to ensure data integrity, we will write all the information
		sprintf(fname,"spells_%s.dat",username_str);
		my_tolower(fname);
		fp=fopen(fname,"wb");
		if(!fp)
			return;
	}

	for (i = 1; i < 7; i++)
	{
		if (mqb_data[i] == NULL)
			break;
		memcpy (data+1+(i-1)*sizeof(mqbdata), mqb_data[i], sizeof(mqbdata));
	}
	data[0] = i;

	fwrite(data, sizeof(*data), sizeof(data), fp);
	
	fclose(fp);
}

// Quickspell window start

int quickspell_over=-1;

int display_quickspell_handler(window_info *win)
{
	int x,y,width,height,i;
	float u_start,v_start,u_end,v_end;
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.20f);
	glColor3f(1.0f,1.0f,1.0f);

	get_and_set_texture_id(sigils_text);
	
	glBegin(GL_QUADS);
	for(i=1;i<7;i++) {
		if(mqb_data[i] && mqb_data[i]->spell_name[0]){
			x=quickspell_size/2;
			y=(i-1)*30+15;
			width=quickspell_size/2;
			height=quickspell_size/2;
			
			//location in window ready, now for the bitmap..
			u_start=0.125f*(mqb_data[i]->spell_image%8);//0 to 7 across
			v_start=1.0f-((float)32/256*(mqb_data[i]->spell_image/8));
			
			u_end=u_start+0.125f-(1.0f/256.0f);//32 pixels(1/8th of 256, -1/256th)
			v_end=v_start-0.125f-(1.0f/256.0f);//32 pixels(1/8th of 256, -1/256th)
		
			draw_2d_thing(u_start,v_start,u_end,v_end, x-width,y-height,x+width,y+height);
		}
	}
	
	glEnd();
	glDisable(GL_ALPHA_TEST);
	
	if(quickspell_over!=-1 && mqb_data[quickspell_over])
		show_help(mqb_data[quickspell_over]->spell_name,-10-strlen(mqb_data[quickspell_over]->spell_name)*8,(quickspell_over-1)*30+10);
	quickspell_over=-1;

	return 1;
}

int mouseover_quickspell_handler(window_info *win, int mx, int my)
{
	int pos;
	
	pos=my/30+1;
	if(pos<7 && pos>=1 && mqb_data[pos] && mqb_data[pos]->spell_name[0]) {
		quickspell_over=pos;
		elwin_mouse=CURSOR_WAND;
		return 1;
	}
	
	return 0;
}

int click_quickspell_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int pos;
	
	pos=my/30+1;

	if(pos<7 && pos>=1 && mqb_data[pos])
	{
		if (flags & ELW_LEFT_MOUSE && mqb_data[pos]->spell_str[0])
		{
			my_tcp_send(my_socket, mqb_data[pos]->spell_str, 12);
			return 1;
		}
		else if (flags & ELW_RIGHT_MOUSE)
		{
			remove_spell_from_quickbar (pos);
			return 1;
		}
	}
	return 0;
}

void init_quickspell()
{
	if (quickspell_win < 0){
		quickspell_win = create_window ("Quickspell", -1, 0, window_width - quickspell_x, quickspell_y, quickspell_x_len, quickspell_y_len, ELW_TITLE_NONE|ELW_SHOW_LAST);
		set_window_handler(quickspell_win, ELW_HANDLER_DISPLAY, &display_quickspell_handler);
		set_window_handler(quickspell_win, ELW_HANDLER_CLICK, &click_quickspell_handler);
		set_window_handler(quickspell_win, ELW_HANDLER_MOUSEOVER, &mouseover_quickspell_handler );
	} else {
		show_window (quickspell_win);
		move_window (quickspell_win, -1, 0, window_width - quickspell_x, quickspell_y);
	}
}

int spell_clear_handler()
{
	int i;

	for(i=0;i<6;i++) {
		on_cast[i]=-1;
	}

	return 1;
}

int cast_handler()
{
	//Cast?
	Uint8 str[20];
	int count=0;
	int sigils_no=0;
	int i;

	for(i=0;i<6;i++) {
		if(on_cast[i]!=-1) {
			count++;
		}
	}

	if(count<2) {
		sprintf(spell_text,"%c%s",127+c_red2,sig_too_few_sigs);
		have_error_message=1;
		return 1;
	}
	
	str[0]=CAST_SPELL;
	for(i=0;i<6;i++) {
		if(on_cast[i]!=-1){
			str[sigils_no+2]=on_cast[i];
			sigils_no++;
		}
	}

	str[1]=sigils_no;
	last_spell_len=sigils_no+2;
	
	if(!mqb_data[0]) {
		mqb_data[0]=(mqbdata*)calloc(1,sizeof(mqbdata));
		mqb_data[0]->spell_id=-1;
	}
	
	memcpy(mqb_data[0]->spell_str, str, last_spell_len);//Copy the last spell send to the server
	
	//ok, send it to the server...
	my_tcp_send(my_socket, str, sigils_no+2);
	memcpy(last_spell_str, str, last_spell_len);
	return 1;
}

// Quickspell window end

void display_sigils_menu()
{
	if(sigil_win < 0){
		static int cast_button_id=100;
		static int clear_button_id=101;
		int our_root_win = -1;
		if (!windows_on_top) {
			our_root_win = game_root_win;
		}
		sigil_win= create_window(win_sigils, our_root_win, 0, sigil_menu_x, sigil_menu_y, sigil_menu_x_len, sigil_menu_y_len, ELW_WIN_DEFAULT);

		set_window_handler(sigil_win, ELW_HANDLER_DISPLAY, &display_sigils_handler );
		set_window_handler(sigil_win, ELW_HANDLER_CLICK, &click_sigils_handler );
		set_window_handler(sigil_win, ELW_HANDLER_MOUSEOVER, &mouseover_sigils_handler );
		
		cast_button_id=button_add_extended(sigil_win, cast_button_id, NULL, 33*6+15, manufacture_menu_y_len-36, 0, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, cast_str);
		widget_set_OnClick(sigil_win, cast_button_id, cast_handler);
		
		clear_button_id=button_add_extended(sigil_win, clear_button_id, NULL, 33*9+8, manufacture_menu_y_len-36, 0, 0, 0, 1.0f, 0.77f, 0.57f, 0.39f, clear_str);
		widget_set_OnClick(sigil_win, clear_button_id, spell_clear_handler);
	} else {
		show_window(sigil_win);
		select_window(sigil_win);
	}
}

#include <stdio.h>
#include <string.h>
#include "options.h"
#include "global.h"

char	*opt_vid1={"640x480x16"},
	*opt_vid2={"640x480x32"},
	*opt_vid3={"800x600x16"},
	*opt_vid4={"800x600x32"},
	*opt_vid5={"1024x768x16"},
	*opt_vid6={"1024x768x32"},
	*opt_vid7={"1152x864x16"},
	*opt_vid8={"1152x864x32"},
	*opt_vid9={"1280x1024x16"},
	*opt_vid10={"1280x1024x32"},
	* no_string = {"(error - no str)"},
	opt_vid1_desc[75],
	opt_vid2_desc[75],
	opt_vid3_desc[75],
	opt_vid4_desc[75],
	opt_vid5_desc[75],
	opt_vid6_desc[75],
	opt_vid7_desc[75],
	opt_vid8_desc[75],
	opt_vid9_desc[75],
	opt_vid10_desc[75];


int options_win= 0;
int show_fps=1;

struct options_struct options = {0,{NULL}};

int always_true=1;

int you_sit=0;
int sit_lock=0;

float lit_gem_u_start=(float)224/256;
float lit_gem_v_start=1.0f-(float)112/256;
float lit_gem_u_end=(float)255/256;
float lit_gem_v_end=1.0f-(float)128/256;

float broken_gem_u_start=(float)192/256;
float broken_gem_v_start=1.0f-(float)112/256;
float broken_gem_u_end=(float)223/256;
float broken_gem_v_end=1.0f-(float)128/256;

float unlit_gem_u_start=(float)224/256;
float unlit_gem_v_start=1.0f-(float)96/256;
float unlit_gem_u_end=(float)255/256;
float unlit_gem_v_end=1.0f-(float)111/256;

int display_options_handler(window_info *win);
int click_options_handler(window_info *win, int mx, int my, Uint32 flags);
int mouseover_options_handler(window_info * win, int mx, int my);

char options_help_text[400]={0};

void display_options_menu()
{
	if(options_win <= 0){
		options_win= create_window("Options", 0, 0, options_menu_x, options_menu_y, options_menu_x_len, options_menu_y_len, ELW_WIN_DEFAULT);

		set_window_color(options_win, ELW_COLOR_BORDER, 0.0f, 1.0f, 0.0f, 0.0f);
		set_window_handler(options_win, ELW_HANDLER_DISPLAY, &display_options_handler );
		set_window_handler(options_win, ELW_HANDLER_CLICK, &click_options_handler );
		set_window_handler(options_win, ELW_HANDLER_MOUSEOVER, &mouseover_options_handler );
		init_display_options_menu();
	} else {
		show_window(options_win);
		select_window(options_win);
	}
	display_window(options_win);
}

void init_display_options_menu()
{
	int option;
	if(options.no) return;
	
	add_option(OPTION,opt_shadows.str,opt_shadows.desc,change_option,&have_stencil,&shadows_on,0);
	add_option(OPTION,opt_clouds.str,opt_clouds.desc,change_option,&have_multitexture,&clouds_shadows,0);
	add_option(OPTION,opt_reflections.str,opt_reflections.desc,change_option,&always_true,&show_reflection,0);
	add_option(OPTION,opt_show_fps.str,opt_show_fps.desc,change_option,&always_true,&show_fps,0);
	add_option(OPTION,opt_sit_lock.str,opt_sit_lock.desc,change_option,&always_true,&sit_lock,0);
	add_option(OPTION,opt_caps_filter.str,opt_caps_filter.desc,change_option,&always_true,&caps_filter,0);
	add_option(OPTION,opt_sound.str,opt_sound.desc,change_sound,&have_sound,&sound_on,0);
	add_option(OPTION,opt_music.str,opt_music.desc,change_music,&have_music,&music_on,0);
	add_option(OPTION,opt_autocam.str,opt_autocam.desc,change_option,&always_true,&auto_camera,0);
	add_option(NONE,NULL,NULL,NULL,NULL,NULL,0);//A hole :0)
	add_option(OPTION,opt_exit.str,opt_exit.desc,change_option,&always_true,&exit_now,0);

	//Video mode automatically switches side - should this be changed?
	add_option(OPTION,opt_full_screen.str,opt_full_screen.desc,move_to_full_screen,&always_true,&full_screen,1);
	option=1;
	sprintf(opt_vid1_desc,switch_video_mode,opt_vid1);
	add_option(VIDEO_MODE,opt_vid1,opt_vid1_desc,switch_video_modes,(int*)&(video_modes[0].supported),&option,1);
	option=2;
	sprintf(opt_vid2_desc,switch_video_mode,opt_vid2);
	add_option(VIDEO_MODE,opt_vid2,opt_vid2_desc,switch_video_modes,(int*)&(video_modes[1].supported),&option,1);
	option=3;
	sprintf(opt_vid3_desc,switch_video_mode,opt_vid3);
	add_option(VIDEO_MODE,opt_vid3,opt_vid3_desc,switch_video_modes,(int*)&(video_modes[2].supported),&option,1);
	option=4;
	sprintf(opt_vid4_desc,switch_video_mode,opt_vid4);
	add_option(VIDEO_MODE,opt_vid4,opt_vid4_desc,switch_video_modes,(int*)&(video_modes[3].supported),&option,1);
	option=5;
	sprintf(opt_vid5_desc,switch_video_mode,opt_vid5);
	add_option(VIDEO_MODE,opt_vid5,opt_vid5_desc,switch_video_modes,(int*)&(video_modes[4].supported),&option,1);
	option=6;
	sprintf(opt_vid6_desc,switch_video_mode,opt_vid6);
	add_option(VIDEO_MODE,opt_vid6,opt_vid6_desc,switch_video_modes,(int*)&(video_modes[5].supported),&option,1);
	option=7;
	sprintf(opt_vid7_desc,switch_video_mode,opt_vid7);
	add_option(VIDEO_MODE,opt_vid7,opt_vid7_desc,switch_video_modes,(int*)&(video_modes[6].supported),&option,1);
	option=8;
	sprintf(opt_vid8_desc,switch_video_mode,opt_vid8);
	add_option(VIDEO_MODE,opt_vid8,opt_vid8_desc,switch_video_modes,(int*)&(video_modes[7].supported),&option,1);
	option=9;
	sprintf(opt_vid9_desc,switch_video_mode,opt_vid9);
	add_option(VIDEO_MODE,opt_vid9,opt_vid9_desc,switch_video_modes,(int*)&(video_modes[8].supported),&option,1);
	option=10;
	sprintf(opt_vid10_desc,switch_video_mode,opt_vid10);
	add_option(VIDEO_MODE,opt_vid10,opt_vid10_desc,switch_video_modes,(int*)&(video_modes[9].supported),&option,1);
}

void add_option(int type, char * name, char * desc, void * func, int * data_1, int * data_2, int column)
{
	int no=options.no++;
	options.option[no]=(option_struct*)calloc(1,sizeof(option_struct));
	if(name!=NULL)options.option[no]->name=name;
	else name=no_string;
	if(desc!=NULL)options.option[no]->desc=desc;
	else desc=no_string;
	options.option[no]->type=type;
	options.option[no]->data_1=data_1;
	if(type!=VIDEO_MODE) options.option[no]->data_2=data_2;
	else 
		{
			options.option[no]->data_2=(int*)malloc(1*sizeof(int));
			*(options.option[no]->data_2)=*data_2;
		}
	options.option[no]->func=func;
	options.option[no]->column=column;
}

//Wrappers
void change_option(int * unused,int * option) {	*option = !*option; }
void move_to_full_screen(int * unused, int * unused2) { toggle_full_screen(); }
void switch_video_modes(int * unused, int * mode)
{
	if(video_mode!=*mode) set_new_video_mode(full_screen,*mode);
}
void change_sound(int * unused, int * unused2) {
	if(sound_on)
		turn_sound_off();
	else
		turn_sound_on();
}
void change_music(int * unused, int * unused2) {
	if(music_on)
		turn_music_off();
	else
		turn_music_on();
}

int display_options_handler(window_info *win)
{
	int i = 0, y[2], x[2];
	option_struct * cur;
	x[0]=8;
	x[1]=188;
	y[0]=35;
	y[1]=35;
	get_and_set_texture_id(icons_text);
	glBegin(GL_QUADS);
	for(;i<options.no;i++)
		{
			cur=options.option[i];
			if (cur->type!=NONE)
				{
					if (*(cur->data_1)>0)
						{
							if (
								(cur->type==OPTION && *(cur->data_2))
										||
								(cur->type==VIDEO_MODE && video_modes[*(cur->data_2)-1].selected)
							   )
								{
									draw_2d_thing(lit_gem_u_start, lit_gem_v_start, lit_gem_u_end, lit_gem_v_end,
											x[cur->column], y[cur->column], x[cur->column]+30, y[cur->column?1:0]+16);
								}
							else
								{
									draw_2d_thing(unlit_gem_u_start, unlit_gem_v_start, unlit_gem_u_end, unlit_gem_v_end,
											x[cur->column], y[cur->column], x[cur->column]+30, y[cur->column]+16);
								}
						}
					else
						{
							draw_2d_thing(broken_gem_u_start,broken_gem_v_start,broken_gem_u_end,broken_gem_v_end,
									x[cur->column], y[cur->column], x[cur->column]+30, y[cur->column]+16);
						}
				}
			y[cur->column]+=20;
		}
	glEnd();
	
	draw_string(55,10,opt_strings.str,1);//Options
	draw_string(225,10,opt_strings.desc,1);//Video mode

	x[0]=45;
	x[1]=225;
	y[0]=35;
	y[1]=35;
	
	for(i=0;i<options.no;i++)
		{
			cur=options.option[i];
			if(cur->type!=NONE)
				{
					draw_string(x[cur->column],y[cur->column],cur->name,1);
				}
			y[cur->column]+=20;
		}

	glColor4f(0.0f, 1.0f, 0.0f, 0.0f);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	glVertex3i(0, win->len_y-40,0);
	glVertex3i(win->len_x, win->len_y-40,0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	draw_string_small(4,win->len_y-35,options_help_text,2);
	return 1;
}

int mouseover_options_handler(window_info *win, int mx, int my)
{
	int no;
	if(mx>8 && mx < 170 && my > 35)
		{
			no=(my-35)/20;
			if (no>=0 && no < options.no && options.option[no]->type!=NONE && *(options.option[no]->data_1))
				{
					put_small_text_in_box(options.option[no]->desc,strlen(options.option[no]->desc),200*2,options_help_text);
				}
			else options_help_text[0]='\0';
		}
	else if (mx>193 && mx < 355 && my > 35)
		{
			no=(my-35)/20+11;
			if (no>=0 && no < options.no && *(options.option[no]->data_1))
				{
					put_small_text_in_box(options.option[no]->desc,strlen(options.option[no]->desc),200*2,options_help_text);
				}
			else options_help_text[0]='\0';
		}
	else options_help_text[0]='\0';
	return 0;
}

int click_options_handler(window_info *win, int mx, int my, Uint32 flags)
{
	// in the first column?
	int no;
	if(mx>8 && mx<38 && my > 35)
		{
			no=(my-35)/20;
			if (*(options.option[no]->data_1)<=0) return 1;
			if (no>=0 && no < options.no) options.option[no]->func(options.option[no]->data_1,options.option[no]->data_2);
		}
	else if(mx>193 && mx<220 && my > 35)//Second col, VIDEO_MODE
		{
			no=(my-35)/20+11;
			if (*(options.option[no]->data_1)<=0) return 1;
			if (no>=0 && no < options.no) options.option[no]->func(&full_screen,options.option[no]->data_2);
		}
	return 1;
}

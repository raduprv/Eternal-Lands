#include <math.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "elconfig.h"

#define SPECINT		0 	//Multiple ints, special func							func(int)
#define SPECCHAR	1 	//Char pointer, special func							func(char*)
#define SPEC 		2 	//A special function call, bool (is called when value != default value) 	func()
#define BOOL		3	//Change variable 								func(int*)
#define STRING		4	//Change string 								func(char*,char*)
#define FLOAT		5 	//Change float									func(float*,float)
#define INT		6	//Change int									func(int*,int)

struct variables our_vars={0,{NULL}};

void change_var(int * var)
{
	*var=!*var;
}

void change_int(int * var, int value)
{
	if(value>=0) *var=value;
}

void change_float(float * var, float * value)
{
	if(*value>=0) *var=*value;
}

void change_string(char * var, char * str, int len)
{
	while(*str && len--)*var++=*str++;
	*var=0;
}

void change_sound_level(float *var, float * value)
{
	if(*value>=0 && *value<=100)
		{
			*var=(float)*value/100.0f;
		}
}

void change_password(char * passwd)
{
	int i=0;
	char * str=password_str;
	while(*passwd)*str++=*passwd++;
	*str=0;
	if(password_str[0])//We have a password
		{
			for(;i<str-password_str;i++) display_password_str[i]='*';
			display_password_str[i]=0;
		}
}

void change_poor_man(int value)
{
	if(value>0)
		{
			show_reflection=0;
			shadows_on=0;
			clouds_shadows=1;
			poor_man=1;
		}
	else poor_man=0;
}

void change_vertex_array(int value)
{
	use_vertex_array=(value>0);
	if(use_vertex_array)
		{
			log_to_console(c_green2,enabled_vertex_arrays);
		}
}

void change_point_particles(int value)
{
	use_point_particles=(value>0);
	if(!use_point_particles)
		{
			log_to_console(c_green2,disabled_point_particles);
		}
}

void change_particles_percentage(int value)
{
	if(value>0 && value <=100) particles_percentage=value;
	else 
		{
			particles_percentage=0;
			log_to_console(c_green2,disabled_particles_str);
		}
}

void switch_vidmode(int mode)
{
	if(mode>10 || mode<=0)
		{
			//warn about this error
			log_to_console(c_red2,invalid_video_mode);
			return;
		}
	else video_mode=mode;
	if(!video_mode_set) return;
	set_new_video_mode(full_screen,video_mode);
}

void toggle_full_screen_mode(int * fs)
{
	if(!video_mode_set) 
		{
			*fs=!*fs;
			return;
		}
	toggle_full_screen();
}

void change_compass_direction(int dir)
{
	compass_direction=1-2*(dir>0);
}

void set_afk_time(int time)
{
	if(time>0)afk_time=time*60000;
	else afk_time=0;
}

int check_var(char * str, int type)
{
	int i,*p;
	char * ptr=str;
	float foo;
	for(i=0;i<our_vars.no;i++)
		{
			if(type?!strncmp(str,our_vars.var[i]->name,our_vars.var[i]->nlen):!strncmp(str,our_vars.var[i]->shortname,our_vars.var[i]->snlen))
				{
					//Allright, it's the right variable... now move ptr forward
					ptr+=type?our_vars.var[i]->nlen:our_vars.var[i]->snlen;
					while(*ptr && (*ptr==' '||*ptr=='='))ptr++;//go to the string occurence
					if(!*ptr||*ptr=='\n')return -1;//hmm, why would you do such a stupid thing?
					if(*ptr=='"')
						{
							//Accurate quoting
							char *tptr=++ptr;
							while(*tptr && *tptr!='"')
								{
									if(*tptr==0x0a||*tptr==0x0d) 
										{
											char str[200];
											snprintf(str,200,"Reached newline without an ending \" in %s",our_vars.var[i]->name);
											log_to_console(c_red2,str);
											break;
										}
									tptr++;
								}
							*tptr=0;
						}
					else
						{
							//Strip it
							char our_string[200];
							char *tptr=our_string;
							while(*ptr && *ptr!=0x0a && *ptr!=0x0d)
								{
									if(*ptr!=' ')*tptr++=*ptr++; //Strip all spaces
									else ptr++;
								}
							*tptr=0;
							ptr=our_string;
						}
					switch(our_vars.var[i]->type)
						{
							case SPECINT:
								our_vars.var[i]->func(atoi(ptr));
								return 1;
							case SPECCHAR:
								our_vars.var[i]->func(ptr);
								return 1;
							case INT:
								our_vars.var[i]->func(our_vars.var[i]->var,atoi(ptr));
								return 1;
							case SPEC:
								p=our_vars.var[i]->var;
								if(*p!=atoi(ptr))our_vars.var[i]->func();//the variable has changed
								return 1;
							case BOOL:
								p=our_vars.var[i]->var;
								if((atoi(ptr)>0)!=*p) our_vars.var[i]->func(our_vars.var[i]->var);
								return 1;
							case STRING:
								our_vars.var[i]->func(our_vars.var[i]->var,ptr,our_vars.var[i]->len);
								return 1;
							case FLOAT:
								foo=atof(ptr);
								our_vars.var[i]->func(our_vars.var[i]->var,&foo);
								return 1;
						}
				}
		}
	return -1;//no variable was found
}

void free_vars()
{
	int i=0;
	for(;i<our_vars.no;i++)
		{
			free(our_vars.var[i]);
		}
}

void add_var(int type, char * name, char * shortname, void * var, void * func, int def)
{
	int *i=var;
	float *f=var;
	int no=our_vars.no++;
	our_vars.var[no]=(var_struct*)calloc(1,sizeof(var_struct));
	switch(our_vars.var[no]->type=type)
		{
			case SPECINT:
			case INT:
			case SPEC:
			case BOOL:
				*i=def;
				break;
			case STRING:
				our_vars.var[no]->len=def;
				break;
			case FLOAT:
				*f=(float)def;
				break;
			case SPECCHAR:
				break;
		}
	our_vars.var[no]->var=var;
	our_vars.var[no]->func=func;
	strncpy(our_vars.var[no]->name,name,50);
	strncpy(our_vars.var[no]->shortname,shortname,10);
	our_vars.var[no]->nlen=strlen(our_vars.var[no]->name);
	our_vars.var[no]->snlen=strlen(our_vars.var[no]->shortname);
}

void init_vars()
{
	add_var(SPECINT,"video_mode","vid",&video_mode,switch_vidmode,4);
	add_var(BOOL,"full_screen","fs",&full_screen,toggle_full_screen_mode,0);
	add_var(BOOL,"shadows_on","shad",&shadows_on,change_var,0);
	add_var(BOOL,"use_shadow_mapping","sm",&use_shadow_mapping,change_var,0);
	add_var(INT,"max_shadow_map_size","smsize",&max_shadow_map_size,change_int,1024);
	add_var(SPECINT,"poor_man","poor",&poor_man,change_poor_man,0);
	add_var(BOOL,"show_reflections","refl",&show_reflection,change_var,1);
	add_var(BOOL,"no_adjust_shadows","noadj",&no_adjust_shadows,change_var,0);
	add_var(BOOL,"clouds_shadows","cshad",&clouds_shadows,change_var,1);
	add_var(BOOL,"show_fps","fps",&show_fps,change_var,1);
	add_var(INT,"limit_fps","lfps",&limit_fps,change_int,0);
	add_var(BOOL,"use_mipmaps","mm",&use_mipmaps,change_var,0);
	add_var(SPECINT,"use_point_particles","upp",&use_point_particles,change_point_particles,1);
	add_var(SPECINT,"particles_percentage","pp",&particles_percentage,change_particles_percentage,100);

	add_var(SPECINT,"use_vertex_array","vertex",&use_vertex_array,change_vertex_array,0);

	add_var(INT,"mouse_limit","lmouse",&mouse_limit,change_int,15);
	add_var(INT,"click_speed","cspeed",&click_speed,change_int,300);

	add_var(FLOAT,"normal_camera_rotation_speed","nrot",&normal_camera_rotation_speed,change_float,15);
	add_var(FLOAT,"fine_camera_rotation_speed","frot",&fine_camera_rotation_speed,change_float,1);
	
	add_var(FLOAT,"name_text_size","nsize",&name_zoom,change_float,1);
	add_var(INT,"name_font","nfont",&name_font,change_int,0);
	add_var(FLOAT,"chat_text_size","csize",&chat_zoom,change_float,1);
	add_var(INT,"chat_font","cfont",&chat_font,change_int,0);
	
	add_var(BOOL,"no_sound","sound",&no_sound,change_var,0);
	add_var(FLOAT,"sound_gain","sgain",&sound_gain,change_sound_level,1);
	add_var(FLOAT,"music_gain","mgain",&music_gain,change_sound_level,1);

	add_var(BOOL,"sit_lock","sl",&sit_lock,change_var,0);
	add_var(BOOL,"item_window_on_drop","itemdrop",&item_window_on_drop,change_var,1);
	add_var(BOOL,"view_digital_clock","digit",&view_digital_clock,change_var,1);
	add_var(BOOL,"show_stats_in_hud","sstats",&show_stats_in_hud,change_var,0);
	add_var(BOOL,"show_help_text","shelp",&show_help_text,change_var,1);
	add_var(BOOL,"relocate_quickbar","requick",&quickbar_relocatable,change_var,0);
	add_var(SPECINT,"compass_north","comp",&compass_direction,change_compass_direction,1);
	
	add_var(SPECINT,"auto_afk_time","afkt",&afk_time,set_afk_time,5*60000);
	add_var(STRING,"afk_message","afkm",afk_message,change_string,127);
	
	add_var(BOOL,"use_global_ignores","gign",&use_global_ignores,change_var,1);
	add_var(BOOL,"save_ignores","sign",&save_ignores,change_var,1);
	add_var(BOOL,"use_global_filters","gfil",&use_global_filters,change_var,1);
	add_var(STRING,"text_filter_replace","trepl",text_filter_replace,change_string,127);
	add_var(BOOL,"caps_filter","caps",&caps_filter,change_var,1);
	
	add_var(STRING,"server_address","sa",server_address,change_string,70);
	add_var(INT,"server_port","sp",&port,change_int,2000);
	add_var(STRING,"username","u",username_str,change_string,16);
	add_var(SPECCHAR,"password","p",password_str,change_password,16);
	add_var(INT,"log_server","log",&log_server,change_var,1);
	
	add_var(STRING,"data_dir","dir",datadir,change_string,90);//Only possible to do at startup - this could of course be changed by using SPECCHAR as the type and adding a special function for this purpose. I just don't see why you'd want to change the directory whilst running the game...
	add_var(STRING,"language","lang",lang,change_string,8);
	add_var(STRING,"browser","b",broswer_name,change_string,70);
}

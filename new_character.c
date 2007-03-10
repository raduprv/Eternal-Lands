#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "global.h"
#include "actors.h"
#include "draw_scene.h"

typedef int my_enum;//This enumeration will decrease, then wrap to top, increase and then wrap to bottom, when using the inc() and dec() functions. Special purpose though, since you have to have between 2 and 255 values in the enumeration and you have to have the same value in enum[0] as in enum[max] - otherwise we'll probably segfault...

my_enum	normal_skin_enum[]	= { SKIN_BROWN, SKIN_NORMAL, SKIN_PALE, SKIN_TAN, SKIN_BROWN };
my_enum	elf_skin_enum[]		= { SKIN_BROWN, SKIN_NORMAL, SKIN_PALE, SKIN_TAN, SKIN_DARK_BLUE, SKIN_BROWN };
my_enum	draegoni_skin_enum[]= { SKIN_BROWN, SKIN_NORMAL, SKIN_PALE, SKIN_TAN, SKIN_WHITE, SKIN_BROWN };
my_enum	normal_hair_enum[]	= { HAIR_BLACK, HAIR_BLOND, HAIR_BROWN, HAIR_GRAY, HAIR_RED, HAIR_WHITE, HAIR_DARK_BROWN, HAIR_STRAWBERRY, HAIR_LIGHT_BLOND, HAIR_DIRTY_BLOND, HAIR_BROWN_GRAY, HAIR_DARK_GRAY, HAIR_DARK_RED, HAIR_BLACK };
my_enum	draegoni_hair_enum[]= { HAIR_BLACK, HAIR_BLOND, HAIR_BROWN, HAIR_GRAY, HAIR_RED, HAIR_WHITE, HAIR_DARK_BROWN, HAIR_STRAWBERRY, HAIR_LIGHT_BLOND, HAIR_DIRTY_BLOND, HAIR_BROWN_GRAY, HAIR_DARK_GRAY, HAIR_DARK_RED, HAIR_BLUE, HAIR_GREEN, HAIR_PURPLE, HAIR_BLACK };
my_enum	male_shirt_enum[]	= { SHIRT_BLACK, SHIRT_BLUE, SHIRT_BROWN, SHIRT_GREY, SHIRT_GREEN, SHIRT_LIGHTBROWN, SHIRT_ORANGE, SHIRT_PURPLE, SHIRT_RED, SHIRT_WHITE, SHIRT_YELLOW, SHIRT_BLACK };
my_enum	normal_shirt_enum[]	= { SHIRT_BLACK, SHIRT_BLUE, SHIRT_BROWN, SHIRT_GREY, SHIRT_GREEN, SHIRT_LIGHTBROWN, SHIRT_ORANGE, SHIRT_PINK, SHIRT_PURPLE, SHIRT_RED, SHIRT_WHITE, SHIRT_YELLOW, SHIRT_BLACK };
my_enum	normal_pants_enum[]	= { PANTS_BLACK, PANTS_BLUE, PANTS_BROWN, PANTS_DARKBROWN, PANTS_GREY, PANTS_GREEN, PANTS_LIGHTBROWN, PANTS_RED, PANTS_WHITE, PANTS_BLACK };
my_enum	normal_boots_enum[]	= { BOOTS_BLACK, BOOTS_BROWN, BOOTS_DARKBROWN, BOOTS_DULLBROWN, BOOTS_LIGHTBROWN, BOOTS_ORANGE, BOOTS_BLACK };
my_enum	normal_head_enum[]	= { HEAD_1, HEAD_2, HEAD_3, HEAD_4, HEAD_1 };
my_enum	human_head_enum[]	= { HEAD_1, HEAD_2, HEAD_3, HEAD_4, HEAD_5, HEAD_1 };

struct race_def {
	int type;
	my_enum *skin;
	my_enum *hair;
	my_enum *shirts;
	my_enum *pants;
	my_enum *boots;
	my_enum *head;
	float x, y, z_rot;
} races[12] = {
	{human_female, 		normal_skin_enum, normal_hair_enum, 	normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, human_head_enum, 43.0f,	156.0f,	140.0f},
	{human_male, 		normal_skin_enum, normal_hair_enum, 	male_shirt_enum, 	normal_pants_enum, normal_boots_enum, human_head_enum, 43.0f,	156.0f,	140.0f},
	{elf_female, 		elf_skin_enum,    normal_hair_enum,	 	normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 42.0f,	92.0f,	180.0f},
	{elf_male,			elf_skin_enum,    normal_hair_enum,	 	male_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 42.0f,	92.0f,	180.0f},
	{dwarf_female,		normal_skin_enum, normal_hair_enum, 	normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 100.0f,	149.0f,	180.0f},
	{dwarf_male,		normal_skin_enum, normal_hair_enum, 	male_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 100.0f,	149.0f,	180.0f},
	{gnome_female,		normal_skin_enum, normal_hair_enum, 	normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 43.0f,	156.0f,	180.0f},
	{gnome_male,		normal_skin_enum, normal_hair_enum, 	male_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 43.0f,	156.0f,	180.0f},
	{orchan_female,		normal_skin_enum, normal_hair_enum, 	normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 42.0f,	92.0f,	180.0f},
	{orchan_male,		normal_skin_enum, normal_hair_enum, 	male_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 42.0f,	92.0f,	180.0f},
	{draegoni_female,	draegoni_skin_enum, draegoni_hair_enum, normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 100.0f,	149.0f,	180.0f},
	{draegoni_male,		draegoni_skin_enum, draegoni_hair_enum, male_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 100.0f,	149.0f,	180.0f},
};

struct char_def {
	int male;
	int race_id;//races[race_id]
	int race;
	int skin;
	int hair;
	int shirt;
	int pants;
	int boots;
	int head;
	struct race_def * def;
	actor * our_model;
} our_actor = {
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	NULL,
	NULL
};

//Enum handling

int find_pos_in_enum(my_enum * def, int val)
{
	int i;

	for(i=1;i<255;i++){
		if(def[i]==val) return i;
		else if(def[i]==def[0])return 0;
	}

	return 0;
}

int inc(my_enum * def, int val, int no_steps)
{
	my_enum * here=&def[find_pos_in_enum(def, val)];

	while(no_steps--){
		if(*here==def[0])here=&def[1];
		else here++;
	}

	return *here;
}

int dec(my_enum * def, int val, int no_steps)
{
	my_enum * top=&def[find_pos_in_enum(def, def[0])];
	my_enum * here=&def[find_pos_in_enum(def, val)];

	while(no_steps--){
		if(*here==*top)here=top;
		here--;
	}

	return *here;
}

//New char interface

int display_time=0;
	
struct input_text {
	char str[40];
	int pos;
} inputs[3] = {
	{"Player", 6},
	{"", 0},
	{"", 0}
};

int creating_char = 1;

void set_create_char_error (const char *msg, int len)
{
	char buf[512];

	if (len <= 0)
	{
		// server didn't send a message, use the default
		snprintf (create_char_error_str, sizeof(create_char_error_str), "%s: %s", reg_error_str, char_name_in_use);
	}
	else
	{
		int prelen = strlen (reg_error_str) + 2;
		int maxlen = sizeof (create_char_error_str) - prelen - 1;

		if (len > maxlen) len = maxlen;
		snprintf (create_char_error_str, sizeof(create_char_error_str), "%s: %.*s", reg_error_str, len, msg);
		create_char_error_str[len+prelen] = '\0';
		reset_soft_breaks (create_char_error_str, len+prelen, sizeof (create_char_error_str), 1.0, window_width - 20, NULL, NULL);
	}

	LOG_TO_CONSOLE(c_red1, create_char_error_str);
	
	put_small_colored_text_in_box(c_red1, create_char_error_str, strlen(create_char_error_str), 200, buf);
	snprintf(create_char_error_str, sizeof(create_char_error_str), "%s", buf);
	display_time=cur_time+6000;

	creating_char=1;
}

void change_actor ()
{
	// We only need to reload the core model, and attach all the correct mesh types.
	if (our_actor.our_model){
		if(our_actor.our_model->calmodel!=NULL)
			CalModel_Delete(our_actor.our_model->calmodel);
		
		our_actor.our_model->calmodel = CalModel_New(actors_defs[our_actor.race].coremodel);
			
		// Attach the Meshes.
		CalModel_AttachMesh(our_actor.our_model->calmodel,actors_defs[our_actor.race].head[our_actor.head].mesh_index);
		CalModel_AttachMesh(our_actor.our_model->calmodel,actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index);
		CalModel_AttachMesh(our_actor.our_model->calmodel,actors_defs[our_actor.race].legs[our_actor.pants].mesh_index);
		
		// Save which mesh is which.
		our_actor.our_model->body_parts->head_meshindex=actors_defs[our_actor.race].head[our_actor.head].mesh_index;
		our_actor.our_model->body_parts->torso_meshindex=actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index;
		our_actor.our_model->body_parts->legs_meshindex=actors_defs[our_actor.race].legs[our_actor.pants].mesh_index;
				
		// Recopy all of the textures.
		my_strncp(our_actor.our_model->body_parts->hands_tex,actors_defs[our_actor.race].skin[our_actor.skin].hands_name,sizeof(our_actor.our_model->body_parts->hands_tex));
		my_strncp(our_actor.our_model->body_parts->head_tex,actors_defs[our_actor.race].skin[our_actor.skin].head_name,sizeof(our_actor.our_model->body_parts->head_tex));
		
		my_strncp(our_actor.our_model->body_parts->hair_tex,actors_defs[our_actor.race].hair[our_actor.hair].hair_name,sizeof(our_actor.our_model->body_parts->hair_tex));
		
		my_strncp(our_actor.our_model->body_parts->arms_tex,actors_defs[our_actor.race].shirt[our_actor.shirt].arms_name,sizeof(our_actor.our_model->body_parts->arms_tex));
		my_strncp(our_actor.our_model->body_parts->torso_tex,actors_defs[our_actor.race].shirt[our_actor.shirt].torso_name,sizeof(our_actor.our_model->body_parts->torso_tex));
		
		my_strncp(our_actor.our_model->body_parts->pants_tex,actors_defs[our_actor.race].legs[our_actor.pants].legs_name,sizeof(our_actor.our_model->body_parts->pants_tex));
		
		my_strncp(our_actor.our_model->body_parts->boots_tex,actors_defs[our_actor.race].boots[our_actor.boots].boots_name,sizeof(our_actor.our_model->body_parts->boots_tex));

		glDeleteTextures(1,&our_actor.our_model->texture_id); // Free the textures
		our_actor.our_model->texture_id = load_bmp8_enhanced_actor(our_actor.our_model->body_parts, 255);	// Rebuild the textures.
		
		// Move the actor. Could be a little disorienting, though.
		our_actor.our_model->x_tile_pos = our_actor.def->x;
		our_actor.our_model->y_tile_pos = our_actor.def->y;
		our_actor.our_model->x_pos = our_actor.def->x*0.5f;
		our_actor.our_model->y_pos = our_actor.def->y*0.5f;
		our_actor.our_model->z_rot = our_actor.def->z_rot;
	}
}

//////////////////////////////////////////////////////////////////////////

// New character window code below.

int newchar_root_win = -1;
int color_race_win = -1;
int namepass_win = -1;

int display_newchar_handler (window_info *win)
{
	int any_reflection; 
	static int main_count = 0;

	//see if we have to load a model (male or female)
	if (creating_char && !our_actor.our_model){
		move_camera();//Make sure we lag a little...
		our_actor.our_model = add_actor_interface (our_actor.def->x, our_actor.def->y, our_actor.def->z_rot, 1.0f, our_actor.race, our_actor.skin, our_actor.hair, our_actor.shirt, our_actor.pants, our_actor.boots, our_actor.head);
		yourself = 0;
		your_actor = our_actor.our_model; 
	}

	if(disconnected)connect_to_server();

	if (!(main_count%10))
		read_mouse_now = 1;
	else
		read_mouse_now = 0;
	reset_under_the_mouse();
	
	//This window is a bit special since it's not fully 2D
	Leave2DMode ();
	glPushMatrix ();

	if (new_zoom_level != zoom_level) {
		zoom_level = new_zoom_level;
		resize_root_window ();
	}
	
	move_camera ();
	save_scene_matrix ();

	CalculateFrustum ();
#ifdef	NEW_FRUSTUM
	set_click_line();
#endif
	any_reflection = find_reflection ();
	CHECK_GL_ERRORS ();
	
	if (SDL_GetAppState() & SDL_APPACTIVE) {
		get_tmp_actor_data();

#ifndef NEW_WEATHER
		//now, determine the current weather light level
		get_weather_light_level ();
#endif

		draw_global_light ();
		update_scene_lights();
		draw_lights();
		CHECK_GL_ERRORS ();

		if (shadows_on && is_day) {
			render_light_view();
			CHECK_GL_ERRORS ();
		}

		if (weather_use_fog()) render_fog();
		if (any_reflection > 1) {
			draw_sky_background ();
			CHECK_GL_ERRORS ();
			if (show_reflection) display_3d_reflection ();
		}

		CHECK_GL_ERRORS ();

		if (shadows_on && is_day) {
			draw_sun_shadowed_scene (any_reflection);
		} else {
			glNormal3f (0.0f,0.0f,1.0f);
			if (any_reflection) draw_lake_tiles ();
			draw_tile_map ();
			CHECK_GL_ERRORS ();
			display_2d_objects ();
			CHECK_GL_ERRORS ();
			anything_under_the_mouse (0, UNDER_MOUSE_NOTHING);
			display_objects ();
			display_ground_objects();
			display_actors (1, 0);
			display_alpha_objects();
			display_blended_objects();
		}
		CHECK_GL_ERRORS ();
	}

	//particles should be last, we have no Z writting
	display_particles ();
	CHECK_GL_ERRORS ();
	
	Enter2DMode ();

	glColor3f(1.0f,1.0f,1.0f);

	draw_hud_frame();
	
	CHECK_GL_ERRORS ();

	{
		int msg, offset, ytext, filter;
		ytext = use_windowed_chat == 1 ? 25 : 20;
		filter = use_windowed_chat == 1 ? current_filter : FILTER_ALL;
		if ( find_last_lines_time (&msg, &offset, current_filter, console_text_width) ){
			set_font(chat_font);    // switch to the chat font
			draw_messages (10, ytext, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, filter, msg, offset, -1, win->len_x - hud_x - 20, win->len_y, chat_zoom);
			set_font (0);   // switch to fixed
		}
	}

	glColor3f(251/255.0f, 250/255.0f, 190/255.0f);
	draw_string_small(132, win->len_y-hud_y+15, zoom_in_out, 1);
	draw_string_small(132, win->len_y-hud_y+32, rotate_camera, 1);
	
	Leave2DMode ();

	glEnable (GL_LIGHTING);
	glPopMatrix (); // restore the state
	Enter2DMode ();

	main_count++;

	return 1;
}

int mouseover_newchar_handler (window_info *win, int mx, int my)
{
	return 1;
}

int click_newchar_handler (window_info *win, int mx, int my, Uint32 flags)
{
	if (flags & ELW_WHEEL_UP) {
		if (camera_zoom_dir == -1)
			camera_zoom_frames += 5;
		else
			camera_zoom_frames = 5;
		camera_zoom_dir = -1;
		return 1;
	}

	if (flags & ELW_WHEEL_DOWN) {
		if (camera_zoom_dir == 1)
			camera_zoom_frames += 5;
		else
			camera_zoom_frames = 5;
		camera_zoom_dir = 1;
		return 1;
	}

	return 1; // we captured this mouseclick
}

int keypress_newchar_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	static int last_time=0;

	if ( check_quit_or_fullscreen (key) ) {
		return 1;
	} else if(disconnected && !alt_on && !ctrl_on){
		connect_to_server();
	} else if (key == K_CAMERAUP) {
		if (rx > -60) rx -= 1.0f;
	} else if (key == K_CAMERADOWN) {
		if (rx < -45) rx += 1.0f;
	} else if (key == K_ZOOMIN) {
		if (zoom_level >= 1.50f) new_zoom_level = zoom_level - 0.25;
	} else if (key == K_ZOOMOUT) {
		if (zoom_level <= 3.50f) new_zoom_level = zoom_level + 0.25;
	} else if(key==K_OPTIONS){
		view_window(&elconfig_win, 0);
	} else if(key==K_ENCYCLOPEDIA){
		view_tab(&tab_help_win, &tab_help_collection_id, HELP_TAB_ENCYCLOPEDIA);
	} else if(key==K_HELP) {
		view_tab(&tab_help_win, &tab_help_collection_id, HELP_TAB_HELP);
	} else if (key == K_RULES) {
		view_tab(&tab_help_win, &tab_help_collection_id, HELP_TAB_RULES);
	} else if (key == K_ROTATELEFT) {
		camera_rotation_speed = normal_camera_rotation_speed / 40;
		camera_rotation_frames = 40;
	} else if (key == K_FROTATELEFT) {
		camera_rotation_speed = fine_camera_rotation_speed / 10;
		camera_rotation_frames = 10;
	} else if (key == K_ROTATERIGHT) {
		camera_rotation_speed = -normal_camera_rotation_speed / 40;
		camera_rotation_frames = 40;
	} else if (key == K_FROTATERIGHT) {
		camera_rotation_speed = -fine_camera_rotation_speed / 10;
		camera_rotation_frames = 10;
	} else if(key==K_TURNLEFT){
		if(last_time+666<cur_time){
			add_command_to_actor(0, turn_left);
			last_time=cur_time;
		}
	} else if(key==K_TURNRIGHT){
		if(last_time+666<cur_time){
			add_command_to_actor(0, turn_right);
			last_time=cur_time;
		}
	} 
	return 1;
}


int show_newchar_handler (window_info *win) {
	init_hud_interface(0);
	show_hud_windows();
	
	return 1;
}

void create_newchar_root_window ()
{
	if (newchar_root_win < 0)
	{
		our_actor.race_id=RAND(0, 5);
		our_actor.def=&races[our_actor.race_id];//6 "races" - counting women as their own race, of course ;-) We cannot include the new races in the random function since they are p2p
		our_actor.skin = inc(our_actor.def->skin, SKIN_BROWN, RAND (SKIN_BROWN, SKIN_TAN));//Increment a random # of times. 
		our_actor.hair = inc(our_actor.def->hair, HAIR_BLACK, RAND (HAIR_BLACK, our_actor.def->type >= draegoni_female ? HAIR_PURPLE:HAIR_WHITE));
		our_actor.shirt = inc(our_actor.def->shirts, SHIRT_BLACK, RAND (SHIRT_BLACK, SHIRT_YELLOW));
		our_actor.pants = inc(our_actor.def->pants, PANTS_BLACK, RAND (PANTS_BLACK, PANTS_WHITE));
		our_actor.boots = inc(our_actor.def->boots, BOOTS_BLACK, RAND (BOOTS_BLACK, BOOTS_ORANGE));
		our_actor.head = inc(our_actor.def->head, HEAD_1, RAND (HEAD_1, our_actor.def->type==human_female?HEAD_5:HEAD_4));
		our_actor.race = our_actor.def->type;
		our_actor.male = our_actor.race<gnome_female?our_actor.race%2:!(our_actor.race%2);
		
		change_map ("./maps/newcharactermap.elm");

		newchar_root_win = create_window (win_newchar, -1, -1, 0, 0, window_width, window_height, ELW_TITLE_NONE|ELW_SHOW_LAST);

		set_window_handler (newchar_root_win, ELW_HANDLER_DISPLAY, &display_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_CLICK, &click_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_KEYPRESS, &keypress_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_SHOW, &show_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_AFTER_SHOW, &update_have_display);
		set_window_handler (newchar_root_win, ELW_HANDLER_HIDE, &update_have_display);

		LOG_TO_CONSOLE(c_green1, char_help);
	} else {
		show_window(newchar_root_win);
	}
}

int active=0;
int hidden=1;

int are_you_sure=0;
int numbers_in_name=0;

char * get_pass_str(int l)
{
	static char str[20];
	
	memset(str, '*', l);
	str[l]=0;

	return str;
}

//Returns 1 if it's valid, 0 if invalid and -1 if there's too many numbers in the name
int check_character(int type, char ch)
{
	int retval=0;

	if(type==0){
		//name
		if(isdigit(ch)){
			// no more then two digits in a name
			if (numbers_in_name >= 2)
			{
				retval=-1;
			} else {
				numbers_in_name++;
				retval=1;
			}
		} else if(isalnum(ch)||ch=='_'){
			retval=1;
		} else if ((ch>= 33 && ch<=47)||(ch>=58 && ch<=64)||(ch>=91&&ch<=96)||(ch>=122 && ch<=126)){
			// not permitted in a name
			retval=-1;
		}
	} else {	// password
		if(ch>=33 && ch<126) retval=1;
	}

	return retval;
}

void add_text_to_buffer(int color, char * text, int time_to_display)
{
	put_small_colored_text_in_box(color, text, strlen(text), 200, create_char_error_str);
	display_time=cur_time+time_to_display;
}

void create_character()
{
	if(!strncasecmp(inputs[1].str, actors_list[0]->actor_name, strlen(actors_list[0]->actor_name))){
		add_text_to_buffer(c_red2, "Bad password!", 6000);
		//add_text_to_buffer(c_red2, error_bad_pass, 6000);
		return;
	} else if(strcmp(inputs[1].str, inputs[2].str)){
		add_text_to_buffer(c_red2, error_pass_no_match, 6000);
		return;
	} else if(inputs[0].pos<3){
		add_text_to_buffer(c_red2, error_username_length, 6000);
		return;
	} else if(inputs[1].pos<4){
		add_text_to_buffer(c_red2, error_password_length, 6000);
		return;
	}

	if(are_you_sure){
		creating_char=0;
		send_new_char(inputs[0].str, inputs[1].str, our_actor.skin, our_actor.hair, our_actor.shirt, our_actor.pants, our_actor.boots, our_actor.head, our_actor.race);
	} else {
		are_you_sure=1;
		
		add_text_to_buffer(c_orange3, error_confirm_create_char, 6000);
		LOG_TO_CONSOLE(c_green2, "\n");
		LOG_TO_CONSOLE(c_green2, remember_change_appearance);
		
		show_color_race_win();
	}
}

void login_from_new_char()
{
	snprintf(username_str, sizeof(username_str), "%s", inputs[0].str);
	snprintf(password_str, sizeof(password_str), "%s", inputs[1].str);
	
	// now destroy reference to ourself, otherwise we'll mess up the ID's
	destroy_all_actors();
	our_actor.our_model=NULL;

	//now send the log in info
	send_login_info();
}

int display_namepass_handler (window_info * win)
{
	glColor3f(0.77f,0.57f,0.39f);
	
	draw_string_small(20, 20, login_username_str, 1);
	draw_string_small(20, 60, login_password_str, 1);
	draw_string_small(20, 90, confirm_password, 1);
	draw_smooth_button(inputs[0].str, DEFAULT_SMALL_RATIO, 100, 16, 120, 1, 0.77f, 0.57f ,0.39f, active == 0, 0.32f, 0.23f, 0.15f, 0.5f);
	draw_smooth_button(hidden?get_pass_str(inputs[1].pos):inputs[1].str, DEFAULT_SMALL_RATIO, 100, 56, 120, 1, 0.77f, 0.57f ,0.39f, active == 1, 0.32f, 0.23f, 0.15f, 0.5f);
	draw_smooth_button(hidden?get_pass_str(inputs[2].pos):inputs[2].str, DEFAULT_SMALL_RATIO, 100, 86, 120, 1, 0.77f, 0.57f ,0.39f, active == 2, 0.32f, 0.23f, 0.15f, 0.5f);
	
	draw_smooth_button(hidden?show_password:hide_password, DEFAULT_SMALL_RATIO, 20, 120, 200, 1, 0.77f, 0.57f ,0.39f, hidden, 0.32f, 0.23f, 0.15f, 0.5f);

	draw_box(NULL, 20, 160, 220, 60, 0);
	
	draw_smooth_button(char_done, DEFAULT_SMALL_RATIO, 20, 230, 60, 1, 0.77f, 0.57f ,0.39f, are_you_sure, 0.32f, 0.23f, 0.15f, 0.5f);
	draw_smooth_button(char_back, DEFAULT_SMALL_RATIO, 160, 230, 60, 1, 0.77f, 0.57f ,0.39f, 0, 0.32f, 0.23f, 0.15f, 0.5f);

	if(display_time>cur_time){
		draw_string_small(30, 168, create_char_error_str, 3);
	}
	
	return 1;
}

int keypress_namepass_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint8 ch = key_to_char (unikey);
	int ret=0;
	struct input_text * t=&inputs[active];

	if ((ret=check_character (active > 0, ch)))
	{
		if (ret==-1)
		{
			add_text_to_buffer (c_red1, error_max_digits, 6000);
		}
		else if (t->pos >= 15)
		{
			add_text_to_buffer (c_red2, error_length, 6000);
		}
		else
		{
			// don't allow a character to start with player
			if(active == 0 && !strcasecmp(t->str, "player")) t->pos= 0;
			// add the character to the buffer
			t->str[t->pos++]=ch;
			t->str[t->pos]=0;
			ret=1;	//Reused to show that a letter has been added
		}
	} 
	else if (unikey == SDLK_TAB || unikey == SDLK_RETURN)
	{
		active++;
		if(active>2) active=0;
	}
#ifndef OSX
	else if (unikey == SDLK_BACKSPACE && t->pos>0)
#else
        else if (key == SDLK_BACKSPACE && t->pos>0)
#endif
	{
		t->pos--;
		if (isdigit (t->str[t->pos])) numbers_in_name--;
		t->str[t->pos] = 0;
		ret = 1;	// Reused to show that a letter has been removed
	}
	else
	{
		//only send error messages on non-null characters
		if(ch != 0){
			add_text_to_buffer (c_red2, error_illegal_character, 6000);
		}
	}

	if(active>0){
		//Password/confirm
		if(ret){
			if(!strncasecmp(inputs[1].str, actors_list[0]->actor_name, strlen(actors_list[0]->actor_name))){
				add_text_to_buffer(c_red2, error_bad_pass, 6000);
			} else if(strcmp(inputs[1].str, inputs[2].str)){
				add_text_to_buffer(c_red2, error_pass_no_match, 6000);
			} else {
				add_text_to_buffer(c_green1, passwords_match, 6000);
			}
		}
	} else {
		snprintf(actors_list[0]->actor_name, sizeof(actors_list[0]->actor_name), "%s", inputs[0].str);
	}
	
	return 1;
}

int click_namepass_handler(window_info * win, int mx, int my, Uint32 flags)
{   
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if(mx>100 && mx<260){
		if(my>16 && my<38){
			active=0;
		} else if(my>56 && my<78){
			active=1;
		} else if(my>86 && my<108){
			active=2;
		}
	}

	if(mx>20 && mx<220 && my>120 && my<144){
		hidden=!hidden;
	}

	if(my>230 && my<252){
		if(mx>20 && mx<100){
			create_character();
		} if(mx>160 && mx<240){
			destroy_all_actors();
			our_actor.our_model=NULL;
			hide_window (newchar_root_win);
			show_window (login_root_win);
			hide_hud_windows ();
		}
	}
	
	
	return 0;
}

void show_account_win ()
{
	if (namepass_win < 0){
	        // Create the window
	        namepass_win = create_window (win_name_pass, newchar_root_win, 0, 10, 200, 270, 260, ELW_WIN_DEFAULT);
	        set_window_handler (namepass_win, ELW_HANDLER_DISPLAY, &display_namepass_handler);
	        set_window_handler (namepass_win, ELW_HANDLER_KEYPRESS, &keypress_namepass_handler);
	        set_window_handler (namepass_win, ELW_HANDLER_CLICK, &click_namepass_handler);
    	} else {
        	show_window(namepass_win);
		select_window(namepass_win);
	}
}

//The character design window
void change_race(int new_race)
{
	if(our_actor.race_id==new_race)return;
	our_actor.race_id=new_race;
	our_actor.def=&races[new_race];
	our_actor.skin = our_actor.def->skin[find_pos_in_enum(our_actor.def->skin, our_actor.skin)];//Increment a random # of times. 
	our_actor.hair = our_actor.def->hair[find_pos_in_enum(our_actor.def->hair, our_actor.hair)];
	our_actor.shirt = our_actor.def->shirts[find_pos_in_enum(our_actor.def->shirts, our_actor.shirt)];
	our_actor.pants = our_actor.def->pants[find_pos_in_enum(our_actor.def->pants, our_actor.pants)];
	our_actor.boots = our_actor.def->boots[find_pos_in_enum(our_actor.def->boots, our_actor.boots)];
	our_actor.head = our_actor.def->head[find_pos_in_enum(our_actor.def->head, our_actor.head)];
	our_actor.race = our_actor.def->type;
	our_actor.male = our_actor.race<gnome_female?our_actor.race%2:!(our_actor.race%2);

	change_actor();
}

int race_help=0;
int book_over=-1;

int book_human=200000;
int book_dwarf=200001;
int book_elf=200002;
int book_gnome=200003;
int book_orchan=200004;
int book_draegoni=200005;

void toggle_book(int id)
{
	static int book_opened=-1;
	if(book_opened==id && book_win && windows_list.window[book_win].displayed){
		close_book(id);
		book_opened=-1;
	} else {
		open_book(id);
		book_opened=id;
	}
}

int display_color_race_handler (window_info *win)
{
	int x;

	glColor3f(1.00f,0.0f,0.f);
	// the red box around P2P races
	draw_box("P2P", 135, 70, 120, 88 , 0);

	glColor3f(0.77f,0.57f,0.39f);

	//Gender
	draw_box(gender_str, 10, 10, 250, 45, 0);
	draw_smooth_button(male_str, DEFAULT_SMALL_RATIO, 40, 22, 60, 1, 0.77f, 0.57f ,0.39f, our_actor.male, 0.32f, 0.23f, 0.15f, 0.5f);
	draw_smooth_button(female_str, DEFAULT_SMALL_RATIO, 150, 22, 60, 1, 0.77f, 0.57f ,0.39f, !our_actor.male, 0.32f, 0.23f, 0.15f, 0.5f);

	//Race
	draw_box(race_str, 10, 65, 250, 98, 0);
	draw_smooth_button(human_str, DEFAULT_SMALL_RATIO, 20, 75, 60, 1, 0.77f, 0.57f ,0.39f, our_actor.race==human_female||our_actor.race==human_male, 0.32f, 0.23f, 0.15f, 0.5f);
	draw_smooth_button(elf_str, DEFAULT_SMALL_RATIO, 20, 103, 60, 1, 0.77f, 0.57f ,0.39f, our_actor.race==elf_female||our_actor.race==elf_male, 0.32f, 0.23f, 0.15f, 0.5f);
	draw_smooth_button(dwarf_str, DEFAULT_SMALL_RATIO, 20, 131, 60, 1, 0.77f, 0.57f ,0.39f, our_actor.race==dwarf_female||our_actor.race==dwarf_male, 0.32f, 0.23f, 0.15f, 0.5f);
	draw_smooth_button(gnome_str, DEFAULT_SMALL_RATIO, 140, 75, 60, 1, 0.77f, 0.57f ,0.39f, our_actor.race==gnome_female||our_actor.race==gnome_male, 0.32f, 0.23f, 0.15f, 0.5f);
	draw_smooth_button(orchan_str, DEFAULT_SMALL_RATIO, 140, 103, 60, 1, 0.77f, 0.57f ,0.39f, our_actor.race==orchan_female||our_actor.race==orchan_male, 0.32f, 0.23f, 0.15f, 0.5f);
	draw_smooth_button(draegoni_str, DEFAULT_SMALL_RATIO, 140, 131, 60, 1, 0.77f, 0.57f ,0.39f, our_actor.race==draegoni_female||our_actor.race==draegoni_male, 0.32f, 0.23f, 0.15f, 0.5f);
	
	//Appearance
	draw_box(appearance_str, 270, 10, 120, win->len_y-17, 0);
	x=330;
	draw_string_small(x-(get_string_width(head_str)*8.0f/12.0f)/2.0f, 25, head_str, 1);
	draw_string_small(x-(get_string_width(skin_str)*8.0f/12.0f)/2.0f, 48, skin_str, 1);
	draw_string_small(x-(get_string_width(hair_str)*8.0f/12.0f)/2.0f, 71, hair_str, 1);
	draw_string_small(x-(get_string_width(shirt_str)*8.0f/12.0f)/2.0f, 94, shirt_str, 1);
	draw_string_small(x-(get_string_width(pants_str)*8.0f/12.0f)/2.0f, 117, pants_str, 1);
	draw_string_small(x-(get_string_width(boots_str)*8.0f/12.0f)/2.0f, 140, boots_str, 1);
	
	//<<
	x=280;
	draw_string_small(x, 25, "<<", 1);
	draw_string_small(x, 48, "<<", 1);
	draw_string_small(x, 71, "<<", 1);
	draw_string_small(x, 94, "<<", 1);
	draw_string_small(x, 117, "<<", 1);
	draw_string_small(x, 140, "<<", 1);
	
	//>>
	x=364;
	draw_string_small(x, 25, ">>", 1);
	draw_string_small(x, 48, ">>", 1);
	draw_string_small(x, 71, ">>", 1);
	draw_string_small(x, 94, ">>", 1);
	draw_string_small(x, 117, ">>", 1);
	draw_string_small(x, 140, ">>", 1);
	
	switch(race_help){
		case 1:
			show_help(p2p_race, 260, 80);
			break;
		case 2:
			show_help(p2p_race, 260, 110);
			break;
		case 3:
			show_help(p2p_race, 260, 140);
			break;
		case 0:
		default:
			break;
	}

	if(book_over==book_human){
		show_help(about_human, 142, 80);
	} else if(book_over==book_elf){
		show_help(about_elves, 142, 108);
	} else if(book_over==book_dwarf){
		show_help(about_dwarfs, 142, 136);
	} else if(book_over==book_gnome){
		show_help(about_gnomes, 260, 80);
	} else if(book_over==book_orchan){
		show_help(about_orchans, 262, 108);
	} else if(book_over==book_draegoni){
		show_help(about_draegoni, 262, 136);
	}
	
	glColor3f(1.0f,1.0f,1.0f);
	glEnable(GL_TEXTURE_2D);

	get_and_set_texture_id(icons_text);
	glBegin(GL_QUADS);
	if(book_opened==book_human||book_over==book_human)
		draw_2d_thing((float)32/256,1.0f-(float)64/256,(float)63/256,1.0f-(float)95/256, 110, 75, 132, 97);
	else 
		draw_2d_thing((float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256, 110, 75, 132, 97);
	if(book_opened==book_elf||book_over==book_elf)
		draw_2d_thing((float)32/256,1.0f-(float)64/256,(float)63/256,1.0f-(float)95/256, 110, 103, 132, 125);
	else 
		draw_2d_thing((float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256, 110, 103, 132, 125);
	if(book_opened==book_dwarf||book_over==book_dwarf)
		draw_2d_thing((float)32/256,1.0f-(float)64/256,(float)63/256,1.0f-(float)95/256, 110, 131, 132, 153);
	else 
		draw_2d_thing((float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256, 110, 131, 132, 153);
	if(book_opened==book_gnome||book_over==book_gnome)
		draw_2d_thing((float)32/256,1.0f-(float)64/256,(float)63/256,1.0f-(float)95/256, 230, 75, 252, 97);
	else 
		draw_2d_thing((float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256, 230, 75, 252, 97);
	if(book_opened==book_orchan||book_over==book_orchan)
		draw_2d_thing((float)32/256,1.0f-(float)64/256,(float)63/256,1.0f-(float)95/256, 230, 103, 252, 125);
	else 
		draw_2d_thing((float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256, 230, 103, 252, 125);
	if(book_opened==book_draegoni||book_over==book_draegoni)
		draw_2d_thing((float)32/256,1.0f-(float)64/256,(float)63/256,1.0f-(float)95/256, 230, 131, 252, 153);
	else 
		draw_2d_thing((float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256, 230, 131, 252, 153);
	
	glEnd();
	
	return 1;
}

int mouseover_color_race_handler (window_info *win, int mx, int my)
{
	if(mx>140 && mx<220){
		//P2P races
		if(my>75 && my<97){
			race_help=1;
		} else if(my>103 && my<125){
			race_help=2;
		} else if(my>131 && my<153){
			race_help=3;
		} else  race_help=0;
	} else race_help=0;
	
	if(mx>110 && mx<132) {
		if(my>75 && my<97){
			book_over=book_human;
		} else if(my>103 && my<125){
			book_over=book_elf;
		} else if(my>131 && my<153){
			book_over=book_dwarf;
		} else  book_over=-1;
	} else if(mx>230 && mx<252){
		if(my>75 && my<97){
			book_over=book_gnome;
		} else if(my>103 && my<125){
			book_over=book_orchan;
		} else if(my>131 && my<153){
			book_over=book_draegoni;
		} else  book_over=-1;
	} else book_over=-1;
	return 1;
}

int click_color_race_handler (window_info *win, int mx, int my, Uint32 flags)
{
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;

	if(my>22 && my<44){
		if(mx>40 && mx<120){
			change_race(our_actor.race_id+!our_actor.male);
		} else if(mx>150 && mx<210){
			change_race(our_actor.race_id-our_actor.male);
		}
	}
	
	if(mx>20 && mx<100){
		if(my>75 && my<97){
			change_race(0+our_actor.male);
		} else if(my>103 && my<125){
			change_race(2+our_actor.male);
		} else if(my>131 && my<153){
			change_race(4+our_actor.male);
		}
	} else if(mx>110 && mx<132) {
		if(my>75 && my<97){
			toggle_book(book_human);
			return 0;
		} else if(my>103 && my<125){
			toggle_book(book_elf);
			return 0;
		} else if(my>131 && my<153){
			toggle_book(book_dwarf);
			return 0;
		}
	} else if(mx>140 && mx<220) {
		if(my>75 && my<97){
			change_race(6+our_actor.male);
		} else if(my>103 && my<125){
			change_race(8+our_actor.male);
		} else if(my>131 && my<153){
			change_race(10+our_actor.male);
		} 
	} else if(mx>230 && mx<252){
		if(my>75 && my<97){
			toggle_book(book_gnome);
			return 0;
		} else if(my>103 && my<125){
			toggle_book(book_orchan);
			return 0;
		} else if(my>131 && my<153){
			toggle_book(book_draegoni);
			return 0;
		}
	} else if(mx>280 && mx<295){
		if(my>25 && my<36){
			our_actor.head=dec(our_actor.def->head, our_actor.head, 1);
			
			// Detach the old head, and reattach and save the new one.
			CalModel_DetachMesh(our_actor.our_model->calmodel,our_actor.our_model->body_parts->head_meshindex);
			CalModel_AttachMesh(our_actor.our_model->calmodel,actors_defs[our_actor.race].head[our_actor.head].mesh_index);
			our_actor.our_model->body_parts->head_meshindex=actors_defs[our_actor.race].head[our_actor.head].mesh_index;
		} else if(my>48 && my<59){
			our_actor.skin=dec(our_actor.def->skin, our_actor.skin, 1);
			// Copy the skin texture names.
			my_strncp(our_actor.our_model->body_parts->hands_tex,actors_defs[our_actor.race].skin[our_actor.skin].hands_name,sizeof(our_actor.our_model->body_parts->hands_tex));
			my_strncp(our_actor.our_model->body_parts->head_tex,actors_defs[our_actor.race].skin[our_actor.skin].head_name,sizeof(our_actor.our_model->body_parts->head_tex));
			
			glDeleteTextures(1,&our_actor.our_model->texture_id); // Free the textures
			our_actor.our_model->texture_id = load_bmp8_enhanced_actor(our_actor.our_model->body_parts, 255);	// Rebuild the actor's textures.
		} else if(my>71 && my<82){
			our_actor.hair=dec(our_actor.def->hair, our_actor.hair, 1);
			
			// Copy the hair texture name.
			my_strncp(our_actor.our_model->body_parts->hair_tex,actors_defs[our_actor.race].hair[our_actor.hair].hair_name,sizeof(our_actor.our_model->body_parts->hair_tex));
			
			glDeleteTextures(1,&our_actor.our_model->texture_id); // Free the textures
			our_actor.our_model->texture_id = load_bmp8_enhanced_actor(our_actor.our_model->body_parts, 255);	// Rebuild the actor's textures.
		} else if(my>94 && my<105){
			our_actor.shirt=dec(our_actor.def->shirts, our_actor.shirt, 1);
			
			// Copy the shirt and arms texture names.
			my_strncp(our_actor.our_model->body_parts->arms_tex,actors_defs[our_actor.race].shirt[our_actor.shirt].arms_name,sizeof(our_actor.our_model->body_parts->arms_tex));
			my_strncp(our_actor.our_model->body_parts->torso_tex,actors_defs[our_actor.race].shirt[our_actor.shirt].torso_name,sizeof(our_actor.our_model->body_parts->torso_tex));
			
			// If we need a new mesh, drop the old one and load it.
			if(actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index != our_actor.our_model->body_parts->torso_meshindex)
			{
				CalModel_DetachMesh(our_actor.our_model->calmodel,our_actor.our_model->body_parts->torso_meshindex);
				CalModel_AttachMesh(our_actor.our_model->calmodel,actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index);
				our_actor.our_model->body_parts->torso_meshindex=actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index;
			}
			
			glDeleteTextures(1,&our_actor.our_model->texture_id); // Free the textures
			our_actor.our_model->texture_id = load_bmp8_enhanced_actor(our_actor.our_model->body_parts, 255);	// Rebuild the actor's textures.
		} else if(my>117 && my<128){
			our_actor.pants=dec(our_actor.def->pants, our_actor.pants, 1);
			
			// Copy the pants texture name.
			my_strncp(our_actor.our_model->body_parts->pants_tex,actors_defs[our_actor.race].legs[our_actor.pants].legs_name,sizeof(our_actor.our_model->body_parts->pants_tex));
			
			// If we need a new mesh, drop the old one and load it.
			if(actors_defs[our_actor.race].legs[our_actor.pants].mesh_index != our_actor.our_model->body_parts->legs_meshindex)
			{
				CalModel_DetachMesh(our_actor.our_model->calmodel,our_actor.our_model->body_parts->legs_meshindex);
				CalModel_AttachMesh(our_actor.our_model->calmodel,actors_defs[our_actor.race].legs[our_actor.pants].mesh_index);
				our_actor.our_model->body_parts->legs_meshindex=actors_defs[our_actor.race].legs[our_actor.pants].mesh_index;
			}
			
			glDeleteTextures(1,&our_actor.our_model->texture_id); // Free the textures
			our_actor.our_model->texture_id = load_bmp8_enhanced_actor(our_actor.our_model->body_parts, 255);	// Rebuild the actor's textures.
		} else if(my>140 && my<151){
			our_actor.boots=dec(our_actor.def->boots, our_actor.boots, 1);
			
			// Copy the new boots texture name.
			my_strncp(our_actor.our_model->body_parts->boots_tex,actors_defs[our_actor.race].boots[our_actor.boots].boots_name,sizeof(our_actor.our_model->body_parts->boots_tex));
			
			glDeleteTextures(1,&our_actor.our_model->texture_id); // Free the textures
			our_actor.our_model->texture_id = load_bmp8_enhanced_actor(our_actor.our_model->body_parts, 255);	// Rebuild the actor's textures.
		}
	} else if(mx>364 && mx<379){
		if(my>25 && my<40){
			our_actor.head=inc(our_actor.def->head, our_actor.head, 1);
			
			// Detach the old head, and reattach and save the new one.
			CalModel_DetachMesh(our_actor.our_model->calmodel,our_actor.our_model->body_parts->head_meshindex);
			CalModel_AttachMesh(our_actor.our_model->calmodel,actors_defs[our_actor.race].head[our_actor.head].mesh_index);
			our_actor.our_model->body_parts->head_meshindex=actors_defs[our_actor.race].head[our_actor.head].mesh_index;
		} else if(my>48 && my<63){
			our_actor.skin=inc(our_actor.def->skin, our_actor.skin, 1);
			
			// Copy the skin texture names.
			my_strncp(our_actor.our_model->body_parts->hands_tex,actors_defs[our_actor.race].skin[our_actor.skin].hands_name,sizeof(our_actor.our_model->body_parts->hands_tex));
			my_strncp(our_actor.our_model->body_parts->head_tex,actors_defs[our_actor.race].skin[our_actor.skin].head_name,sizeof(our_actor.our_model->body_parts->head_tex));
			
			glDeleteTextures(1,&our_actor.our_model->texture_id); // Free the textures
			our_actor.our_model->texture_id = load_bmp8_enhanced_actor(our_actor.our_model->body_parts, 255);	// Rebuild the actor's textures.
		} else if(my>71 && my<86){
			our_actor.hair=inc(our_actor.def->hair, our_actor.hair, 1);
			
			// Copy the hair texture name.
			my_strncp(our_actor.our_model->body_parts->hair_tex,actors_defs[our_actor.race].hair[our_actor.hair].hair_name,sizeof(our_actor.our_model->body_parts->hair_tex));
			
			glDeleteTextures(1,&our_actor.our_model->texture_id); // Free the textures
			our_actor.our_model->texture_id = load_bmp8_enhanced_actor(our_actor.our_model->body_parts, 255);	// Rebuild the actor's textures.
		} else if(my>94 && my<109){
			our_actor.shirt=inc(our_actor.def->shirts, our_actor.shirt, 1);
			
			// Copy the shirt and arms texture names.
			my_strncp(our_actor.our_model->body_parts->arms_tex,actors_defs[our_actor.race].shirt[our_actor.shirt].arms_name,sizeof(our_actor.our_model->body_parts->arms_tex));
			my_strncp(our_actor.our_model->body_parts->torso_tex,actors_defs[our_actor.race].shirt[our_actor.shirt].torso_name,sizeof(our_actor.our_model->body_parts->torso_tex));
			
			// If we need a new mesh, drop the old one and load it.
			if(actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index != our_actor.our_model->body_parts->torso_meshindex)
			{
				CalModel_DetachMesh(our_actor.our_model->calmodel,our_actor.our_model->body_parts->torso_meshindex);
				CalModel_AttachMesh(our_actor.our_model->calmodel,actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index);
				our_actor.our_model->body_parts->torso_meshindex=actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index;
			}
			
			glDeleteTextures(1,&our_actor.our_model->texture_id); // Free the textures
			our_actor.our_model->texture_id = load_bmp8_enhanced_actor(our_actor.our_model->body_parts, 255);	// Rebuild the actor's textures.
		} else if(my>117 && my<132){
			our_actor.pants=inc(our_actor.def->pants, our_actor.pants, 1);
			
			// Copy the pants texture name.
			my_strncp(our_actor.our_model->body_parts->pants_tex,actors_defs[our_actor.race].legs[our_actor.pants].legs_name,sizeof(our_actor.our_model->body_parts->pants_tex));
			
			// If we need a new mesh, drop the old one and load it.
			if(actors_defs[our_actor.race].legs[our_actor.pants].mesh_index != our_actor.our_model->body_parts->legs_meshindex)
			{
				CalModel_DetachMesh(our_actor.our_model->calmodel,our_actor.our_model->body_parts->legs_meshindex);
				CalModel_AttachMesh(our_actor.our_model->calmodel,actors_defs[our_actor.race].legs[our_actor.pants].mesh_index);
				our_actor.our_model->body_parts->legs_meshindex=actors_defs[our_actor.race].legs[our_actor.pants].mesh_index;
			}
			
			glDeleteTextures(1,&our_actor.our_model->texture_id); // Free the textures
			our_actor.our_model->texture_id = load_bmp8_enhanced_actor(our_actor.our_model->body_parts, 255);	// Rebuild the actor's textures.
		} else if(my>140 && my<155){
			our_actor.boots=inc(our_actor.def->boots, our_actor.boots, 1);
			
			// Copy the new boots texture name.
			my_strncp(our_actor.our_model->body_parts->boots_tex,actors_defs[our_actor.race].boots[our_actor.boots].boots_name,sizeof(our_actor.our_model->body_parts->boots_tex));
			
			glDeleteTextures(1,&our_actor.our_model->texture_id); // Free the textures
			our_actor.our_model->texture_id = load_bmp8_enhanced_actor(our_actor.our_model->body_parts, 255);	// Rebuild the actor's textures.
		}
	}
	return 1;
}

void show_color_race_win()
{
	if(color_race_win < 0){
		color_race_win = create_window (win_design, newchar_root_win, 0, 300, 200, 420, 170, ELW_WIN_DEFAULT|ELW_CLICK_TRANSPARENT);
		set_window_handler (color_race_win, ELW_HANDLER_DISPLAY, &display_color_race_handler);
		set_window_handler (color_race_win, ELW_HANDLER_MOUSEOVER, &mouseover_color_race_handler);
		set_window_handler (color_race_win, ELW_HANDLER_CLICK, &click_color_race_handler);
    	} else {
        	show_window(color_race_win);
        	select_window(color_race_win);
    	}
}

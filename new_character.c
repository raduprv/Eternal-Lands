#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "new_character.h"
#include "2d_objects.h"
#include "3d_objects.h"
#include "actors.h"
#include "actor_scripts.h"
#include "asc.h"
#include "bbox_tree.h"
#include "books.h"
#include "cal3d_wrapper.h"
#include "chat.h"
#include "consolewin.h"
#include "cursors.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "gamewin.h"
#include "gl_init.h"
#include "global.h"
#include "hud.h"
#include "icon_window.h"
#include "init.h"
#include "interface.h"
#include "lights.h"
#include "loginwin.h"
#include "map.h"
#include "misc.h"
#include "multiplayer.h"
#include "new_actors.h"
#include "particles.h"
#include "reflection.h"
#include "shadows.h"
#include "sky.h"
#include "tabs.h"
#include "textures.h"
#include "tiles.h"
#include "translate.h"
#include "weather.h"
#include "widgets.h"
#include "actor_init.h"

static void add_text_to_buffer(int color, const char * text, int time_to_display);

typedef int my_enum;//This enumeration will decrease, then wrap to top, increase and then wrap to bottom, when using the inc() and dec() functions. Special purpose though, since you have to have between 2 and 255 values in the enumeration and you have to have the same value in enum[0] as in enum[max] - otherwise we'll probably segfault...

static my_enum	normal_skin_enum[]	= { SKIN_BROWN, SKIN_NORMAL, SKIN_PALE, SKIN_TAN, SKIN_BROWN };
static my_enum	elf_skin_enum[]		= { SKIN_BROWN, SKIN_NORMAL, SKIN_PALE, SKIN_TAN, SKIN_DARK_BLUE, SKIN_BROWN };
static my_enum	draegoni_skin_enum[]	= { SKIN_BROWN, SKIN_NORMAL, SKIN_PALE, SKIN_TAN, SKIN_WHITE, SKIN_BROWN };
static my_enum	normal_hair_enum[]	= { HAIR_BLACK, HAIR_BLOND, HAIR_BROWN, HAIR_GRAY, HAIR_RED, HAIR_WHITE, HAIR_DARK_BROWN, HAIR_STRAWBERRY, HAIR_LIGHT_BLOND, HAIR_DIRTY_BLOND, HAIR_BROWN_GRAY, HAIR_DARK_GRAY, HAIR_DARK_RED, HAIR_BLACK };
static my_enum	draegoni_hair_enum[]	= { HAIR_BLACK, HAIR_BLOND, HAIR_BROWN, HAIR_GRAY, HAIR_RED, HAIR_WHITE, HAIR_DARK_BROWN, HAIR_STRAWBERRY, HAIR_LIGHT_BLOND, HAIR_DIRTY_BLOND, HAIR_BROWN_GRAY, HAIR_DARK_GRAY, HAIR_DARK_RED, HAIR_BLUE, HAIR_GREEN, HAIR_PURPLE, HAIR_BLACK };
static my_enum	eyes_enum[]		= { EYES_BROWN, EYES_DARK_BROWN, EYES_BROWN_RED, EYES_LIGHT_BLUE, EYES_BLUE, EYES_DARK_BLUE, EYES_LIGHT_GREEN, EYES_GREEN, EYES_DARK_GREEN, EYES_LAVENDER, EYES_VIOLET, EYES_GOLD, EYES_BROWN };
static my_enum	male_shirt_enum[]	= { SHIRT_BLACK, SHIRT_BLUE, SHIRT_BROWN, SHIRT_GREY, SHIRT_GREEN, SHIRT_LIGHTBROWN, SHIRT_ORANGE, SHIRT_PURPLE, SHIRT_RED, SHIRT_WHITE, SHIRT_YELLOW, SHIRT_BLACK };
static my_enum	normal_shirt_enum[]	= { SHIRT_BLACK, SHIRT_BLUE, SHIRT_BROWN, SHIRT_GREY, SHIRT_GREEN, SHIRT_LIGHTBROWN, SHIRT_ORANGE, SHIRT_PINK, SHIRT_PURPLE, SHIRT_RED, SHIRT_WHITE, SHIRT_YELLOW, SHIRT_BLACK };
static my_enum	normal_pants_enum[]	= { PANTS_BLACK, PANTS_BLUE, PANTS_BROWN, PANTS_DARKBROWN, PANTS_GREY, PANTS_GREEN, PANTS_LIGHTBROWN, PANTS_RED, PANTS_WHITE, PANTS_BLACK };
static my_enum	normal_boots_enum[]	= { BOOTS_BLACK, BOOTS_BROWN, BOOTS_DARKBROWN, BOOTS_DULLBROWN, BOOTS_LIGHTBROWN, BOOTS_ORANGE, BOOTS_BLACK };
static my_enum	normal_head_enum[]	= { HEAD_1, HEAD_2, HEAD_3, HEAD_4, HEAD_1 };
static my_enum	human_head_enum[]	= { HEAD_1, HEAD_2, HEAD_3, HEAD_4, HEAD_5, HEAD_1 };

struct race_def {
	int type;
	my_enum *skin;
	my_enum *hair;
	my_enum *eyes;
	my_enum *shirts;
	my_enum *pants;
	my_enum *boots;
	my_enum *head;
	float x, y, z_rot;
} races[12] = {
	{human_female, 		normal_skin_enum, normal_hair_enum, 	eyes_enum,	normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, human_head_enum, 43.0f,	156.0f,	140.0f},
	{human_male, 		normal_skin_enum, normal_hair_enum, 	eyes_enum,	male_shirt_enum, 	normal_pants_enum, normal_boots_enum, human_head_enum, 43.0f,	156.0f,	140.0f},
	{elf_female, 		elf_skin_enum,    normal_hair_enum,	eyes_enum,	normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 42.0f,	92.0f,	180.0f},
	{elf_male,		elf_skin_enum,    normal_hair_enum,	eyes_enum,	male_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 42.0f,	92.0f,	180.0f},
	{dwarf_female,		normal_skin_enum, normal_hair_enum, 	eyes_enum,	normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 100.0f,	149.0f,	180.0f},
	{dwarf_male,		normal_skin_enum, normal_hair_enum, 	eyes_enum,	male_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 100.0f,	149.0f,	180.0f},
	{gnome_female,		normal_skin_enum, normal_hair_enum, 	eyes_enum,	normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 43.0f,	156.0f,	180.0f},
	{gnome_male,		normal_skin_enum, normal_hair_enum, 	eyes_enum,	male_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 43.0f,	156.0f,	180.0f},
	{orchan_female,		normal_skin_enum, normal_hair_enum, 	eyes_enum,	normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 42.0f,	92.0f,	180.0f},
	{orchan_male,		normal_skin_enum, normal_hair_enum, 	eyes_enum,	male_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 42.0f,	92.0f,	180.0f},
	{draegoni_female,	draegoni_skin_enum, draegoni_hair_enum, eyes_enum,	normal_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 100.0f,	149.0f,	180.0f},
	{draegoni_male,		draegoni_skin_enum, draegoni_hair_enum, eyes_enum,	male_shirt_enum, 	normal_pants_enum, normal_boots_enum, normal_head_enum, 100.0f,	149.0f,	180.0f},
};

struct char_def {
	int male;
	int race_id;//races[race_id]
	int race;
	int skin;
	int hair;
	int eyes;
	int shirt;
	int pants;
	int boots;
	int head;
	struct race_def * def;
	actor * our_model;
} our_actor = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	NULL,
	NULL
};

//Enum handling

static int find_pos_in_enum(my_enum * def, int val)
{
	int i;

	for(i=1;i<255;i++){
		if(def[i]==val) return i;
		else if(def[i]==def[0])return 0;
	}

	return 0;
}

static int inc(my_enum * def, int val, int no_steps)
{
	my_enum * here=&def[find_pos_in_enum(def, val)];

	while(no_steps--){
		if(*here==def[0])here=&def[1];
		else here++;
	}

	return *here;
}

static int dec(my_enum * def, int val, int no_steps)
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

#define ERR_STR_LEN 520
static char create_char_error_str[ERR_STR_LEN] = {0};
static int old_use_windowed_chat;
static int display_time=0;
static const int DEF_MESSAGE_TIMEOUT = 3000;

struct input_text {
	char str[40];
	int pos;
} inputs[3] = {
	{"Player", 6},
	{"", 0},
	{"", 0}
};

static int clear_player_name = 1;

static int creating_char = 1;

void set_create_char_error (const char *msg, int len)
{
	char buf[ERR_STR_LEN];

	if (len <= 0)
	{
		// server didn't send a message, use the default
		safe_snprintf (buf, sizeof(buf), "%s: %s", reg_error_str, char_name_in_use);
	}
	else
	{
		safe_snprintf (buf, sizeof (buf), "%s: %.*s", reg_error_str, len, msg);
		reset_soft_breaks (buf, strlen (buf), sizeof (create_char_error_str), 1.0, window_width - 20, NULL, NULL);
	}

	LOG_TO_CONSOLE(c_red1, buf);

	add_text_to_buffer(c_red1, msg, DEF_MESSAGE_TIMEOUT);

	creating_char=1;
}

static void change_actor (void)
{
	// We only need to reload the core model, and attach all the correct mesh types.
	if (our_actor.our_model){
		if(our_actor.our_model->calmodel!=NULL)
			model_delete(our_actor.our_model->calmodel);

		our_actor.our_model->calmodel = model_new(actors_defs[our_actor.race].coremodel);
		our_actor.our_model->actor_type = our_actor.race;

		// Attach the Meshes.
		model_attach_mesh(our_actor.our_model,
			actors_defs[our_actor.race].head[our_actor.head].mesh_index);
		model_attach_mesh(our_actor.our_model,
			actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index);
		model_attach_mesh(our_actor.our_model,
			actors_defs[our_actor.race].legs[our_actor.pants].mesh_index);
		model_attach_mesh(our_actor.our_model,
			actors_defs[our_actor.race].boots[our_actor.boots].mesh_index);

		// Save which mesh is which.
		our_actor.our_model->body_parts->head_meshindex =
			actors_defs[our_actor.race].head[our_actor.head].mesh_index;
		our_actor.our_model->body_parts->torso_meshindex =
			actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index;
		our_actor.our_model->body_parts->legs_meshindex =
			actors_defs[our_actor.race].legs[our_actor.pants].mesh_index;
		our_actor.our_model->body_parts->boots_meshindex =
			actors_defs[our_actor.race].boots[our_actor.boots].mesh_index;

		// Recopy all of the textures.
		my_strncp(our_actor.our_model->body_parts->hands_tex,actors_defs[our_actor.race].skin[our_actor.skin].hands_name,sizeof(our_actor.our_model->body_parts->hands_tex));
		my_strncp(our_actor.our_model->body_parts->head_tex,actors_defs[our_actor.race].skin[our_actor.skin].head_name,sizeof(our_actor.our_model->body_parts->head_tex));

		my_strncp(our_actor.our_model->body_parts->hair_tex,actors_defs[our_actor.race].hair[our_actor.hair].hair_name,sizeof(our_actor.our_model->body_parts->hair_tex));
#ifdef NEW_EYES
		my_strncp(our_actor.our_model->body_parts->eyes_tex,actors_defs[our_actor.race].eyes[our_actor.eyes].eyes_name,sizeof(our_actor.our_model->body_parts->eyes_tex));
#endif
		my_strncp(our_actor.our_model->body_parts->arms_tex,actors_defs[our_actor.race].shirt[our_actor.shirt].arms_name,sizeof(our_actor.our_model->body_parts->arms_tex));
		my_strncp(our_actor.our_model->body_parts->torso_tex,actors_defs[our_actor.race].shirt[our_actor.shirt].torso_name,sizeof(our_actor.our_model->body_parts->torso_tex));

		my_strncp(our_actor.our_model->body_parts->pants_tex,actors_defs[our_actor.race].legs[our_actor.pants].legs_name,sizeof(our_actor.our_model->body_parts->pants_tex));

		my_strncp(our_actor.our_model->body_parts->boots_tex,actors_defs[our_actor.race].boots[our_actor.boots].boots_name,sizeof(our_actor.our_model->body_parts->boots_tex));

		free_actor_texture(our_actor.our_model->texture_id);
		our_actor.our_model->texture_id = load_enhanced_actor(our_actor.our_model->body_parts, 0);	// Rebuild the actor's textures.

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
static int color_race_win = -1;
static int namepass_win = -1;
static int newchar_advice_win = -1;
static int newchar_hud_win = -1;
static int error_widget_id = -1;

//	Display the "Character creation screen" and creation step help window.
//
static int display_advice_handler (window_info *win)
{
	static int lastw = -1, lasth = -1;
	static float last_scale = 0;
	static Uint32 last_time = 0;
	static int flash = 0;
	static char *last_help = NULL;
	static char *help_str = NULL;
	int sep = 5;

	// Resize and move the window on first use and if window size changed.
	// Place centred, just down from the top of the screen.
	if ((lastw!=window_width) || (lasth!=window_height) || (last_scale != win->current_scale))
	{
		int len_x = (int)(2*sep + strlen(newchar_warning) * win->default_font_len_x);
		int len_y = (int)(2*sep + win->default_font_len_y);
		int pos_x = (int)((window_width - len_x - hud_x) / 2);
		resize_window(win->window_id, len_x, len_y);
		move_window(win->window_id, win->pos_id, win->pos_loc, pos_x, sep);
		lastw = window_width;
		lasth = window_height;
		last_scale = win->current_scale;
	}

	// Draw the warning text.
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f,1.0f,1.0f);
	draw_string_zoomed(sep, sep, (const unsigned char *)newchar_warning, 1, win->current_scale);

	// Give eye icon help, then credentials icon help then "done" help.
	if (color_race_win < 0)
		help_str = newchar_cust_help;
	else if (!get_show_window(namepass_win))
		help_str = newchar_cred_help;
	else
		help_str = newchar_done_help;

	// Remember the last help string so we can flash when it changes
	if (help_str != last_help)
	{
		flash = 16;
		last_help = help_str;
	}

	// Time flashing, flash for a few seconds then remain on.
	if (flash && (SDL_GetTicks()-last_time) > 250)
	{
		flash--;
		last_time = SDL_GetTicks();
	}

	// Either always on or 1 in 4 off.
	if (!flash || (flash & 3))
	{
		int len = strlen(help_str);
		int x = (int)((win->len_x - len * win->small_font_len_x)/2);
		int y = window_height - HUD_MARGIN_Y - 2*sep - win->small_font_len_y;
		if(x >= -win->cur_x) //Does everything fit in one line?
		{
			show_help(help_str, x, y, win->current_scale);
		}
		else
		{
			int i;
			for(i = len/2; i<=len; i++) //Find first space after the middle of the text
			{
				if(help_str[i] == ' ')
					break;
			}
			help_str[i] = 0;
			y -= 2 + win->small_font_len_y;
			x = (win->len_x - i*win->small_font_len_x)/2;
			glColor4f(0.0f,0.0f,0.0f,0.5f);
			glDisable(GL_TEXTURE_2D);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glBegin(GL_QUADS);
			glVertex3i(x - 1,y + 2 + 2*win->small_font_len_y, 0);
			glVertex3i(x - 1,y,0);
			glVertex3i(x + i*win->small_font_len_x, y, 0);
			glVertex3i(x + i*win->small_font_len_x, y + 2 + 2*win->small_font_len_y, 0);
			glEnd();
			glDisable(GL_BLEND);
			glEnable(GL_TEXTURE_2D);

			glColor3f(1.0f,1.0f,1.0f);
			draw_string_small_zoomed(x, y, (unsigned char*)help_str, 1, win->current_scale);
			draw_string_small_zoomed((win->len_x - (len - i - 1)*win->small_font_len_x)/2, y + 2 + win->small_font_len_y, (unsigned char*)(help_str + i + 1), 1, win->current_scale);
			help_str[i] = ' ';
		}
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}


static int display_newchar_handler (window_info *win)
{
	int any_reflection;
	static int main_count = 0;

	if (display_time && cur_time > display_time)
	{
		create_char_error_str[0] = '\0';
		display_time = 0;
	}

	if(disconnected)
	{
		static int nested_flag = 0;
		/* connect_to_server() calls draw_scene() so we need to prevent recursion */
		if (!nested_flag)
		{
			LOG_TO_CONSOLE(c_red2, "Connection failed, please try again");
			creating_char = 1;	/* this was clear before the failed connect so reset */
			nested_flag = 1;
			connect_to_server();
			nested_flag = 0;
		}
	}

	//see if we have to load a model (male or female)
	if (creating_char && !our_actor.our_model){
		move_camera();//Make sure we lag a little...
		our_actor.our_model = add_actor_interface (our_actor.def->x, our_actor.def->y, our_actor.def->z_rot, 1.0f, our_actor.race,
			inputs[0].str, our_actor.skin, our_actor.hair, our_actor.eyes, our_actor.shirt, our_actor.pants, our_actor.boots, our_actor.head);
		yourself = 0;
		LOCK_ACTORS_LISTS();
		set_our_actor (our_actor.our_model);
		UNLOCK_ACTORS_LISTS();
	}

	if (!(main_count%10))
		read_mouse_now = 1;
	else
		read_mouse_now = 0;

	//This window is a bit special since it's not fully 2D
	Leave2DMode ();
	glPushMatrix ();

    update_camera();

	if (new_zoom_level != zoom_level) {
		zoom_level = new_zoom_level;
		resize_root_window ();
	}

	move_camera ();

	CalculateFrustum ();
	set_click_line();
	any_reflection = find_reflection ();
	CHECK_GL_ERRORS ();

	reset_under_the_mouse();

	if (SDL_GetAppState() & SDL_APPACTIVE)
	{

		draw_global_light ();

		if (skybox_show_sky)
        {
			if (skybox_update_delay < 1)
				skybox_update_colors();
            skybox_compute_z_position();
            glPushMatrix();
            glTranslatef(0.0, 0.0, skybox_get_z_position());
			skybox_display();
            glPopMatrix();
        }

		update_scene_lights();
		draw_lights();
		CHECK_GL_ERRORS ();

		if (shadows_on && is_day) {
			render_light_view();
			CHECK_GL_ERRORS ();
		}

		if (use_fog)
			weather_render_fog();
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
			display_actors (1, DEFAULT_RENDER_PASS);
			display_alpha_objects();
			display_blended_objects();
		}

		CHECK_GL_ERRORS ();
	}

	//particles should be last, we have no Z writting
	display_particles ();
	CHECK_GL_ERRORS ();

	Enter2DMode ();

	draw_hud_interface(win);

	CHECK_GL_ERRORS ();

	{
		int msg, offset;
		if ( find_last_lines_time (&msg, &offset, current_filter, get_console_text_width()) ){
			set_font(chat_font);    // switch to the chat font
			draw_messages (10, 40, display_text_buffer, DISPLAY_TEXT_BUFFER_SIZE, FILTER_ALL, msg, offset, -1, win->len_x - hud_x - 20, win->len_y, chat_zoom, NULL);
			set_font (0);   // switch to fixed
		}
	}

	glColor3f(251/255.0f, 250/255.0f, 190/255.0f);
	{
		int y_off = win->len_y - HUD_MARGIN_Y + (HUD_MARGIN_Y - 2 * win->small_font_len_y) / 2;
		draw_string_small_zoomed(get_icons_win_active_len() + win->small_font_len_x, y_off, (unsigned char*)zoom_in_out, 1, win->current_scale);
		y_off += win->small_font_len_y;
		draw_string_small_zoomed(get_icons_win_active_len() + win->small_font_len_x, y_off, (unsigned char*)rotate_camera, 1, win->current_scale);
	}

	Leave2DMode ();

	glEnable (GL_LIGHTING);
	glPopMatrix (); // restore the state
	Enter2DMode ();

	main_count++;
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

static int mouseover_newchar_handler (window_info *win, int mx, int my)
{
	return 1;
}

static int click_newchar_handler (window_info *win, int mx, int my, Uint32 flags)
{
	if (flags & ELW_WHEEL_UP) {
		if (camera_zoom_dir == -1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = -1;
		return 1;
	}

	if (flags & ELW_WHEEL_DOWN) {
		if (camera_zoom_dir == 1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = 1;
		return 1;
	}

	return 1; // we captured this mouseclick
}

static int keypress_newchar_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	static int last_time=0;
	int alt_on = key & ELW_ALT;
	int ctrl_on = key & ELW_CTRL;

	if ( check_quit_or_fullscreen (key) ) {
		return 1;
	} else if(disconnected && !alt_on && !ctrl_on){
		connect_to_server();
	} else if (key == K_CAMERAUP) {
		camera_tilt_speed = -normal_camera_rotation_speed * 0.0005;
		camera_tilt_duration += 100;
        camera_tilt_deceleration = normal_camera_deceleration*0.5E-3;
	} else if (key == K_CAMERADOWN) {
		camera_tilt_speed = normal_camera_rotation_speed * 0.0005;
		camera_tilt_duration += 100;
        camera_tilt_deceleration = normal_camera_deceleration*0.5E-3;
	} else if (key == K_ZOOMIN) {
		if (camera_zoom_dir == -1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = -1;
	} else if (key == K_ZOOMOUT) {
		if (camera_zoom_dir == 1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = 1;
	} else if(key==K_OPTIONS){
		view_window(&elconfig_win, 0);
	} else if(key==K_ENCYCLOPEDIA){
		view_tab(&tab_help_win, &tab_help_collection_id, HELP_TAB_ENCYCLOPEDIA);
	} else if(key==K_HELP) {
		view_tab(&tab_help_win, &tab_help_collection_id, HELP_TAB_HELP);
	} else if (key == K_RULES) {
		view_tab(&tab_help_win, &tab_help_collection_id, HELP_TAB_RULES);
	} else if (key == K_ROTATELEFT) {
		camera_rotation_speed = normal_camera_rotation_speed / 800.0;
		camera_rotation_duration = 800;
        camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
	} else if (key == K_FROTATELEFT) {
		camera_rotation_speed = fine_camera_rotation_speed / 200.0;
		camera_rotation_duration = 200;
		camera_rotation_speed /= 4.0;
        camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
	} else if (key == K_ROTATERIGHT) {
		camera_rotation_speed = -normal_camera_rotation_speed / 800.0;
		camera_rotation_duration = 800;
        camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
	} else if (key == K_FROTATERIGHT) {
		camera_rotation_speed = -fine_camera_rotation_speed / 200.0;
		camera_rotation_duration = 200;
		camera_rotation_speed /= 4.0;
        camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
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


static int show_newchar_handler (window_info *win) {
	init_hud_interface (HUD_INTERFACE_NEW_CHAR);
	show_hud_windows();

	return 1;
}

static void create_newchar_hud_window(void);

static int ui_scale_newchar_handler(window_info *win)
{
	hud_x = (int)(0.5 + win->current_scale * NEW_CHARACTER_BASE_HUD_X);
	resize_newchar_hud_window();
	return 1;
}

void create_newchar_root_window (void)
{
	if (newchar_root_win < 0)
	{
		our_actor.race_id=RAND(0, 5);
		our_actor.def=&races[our_actor.race_id];//6 "races" - counting women as their own race, of course ;-) We cannot include the new races in the random function since they are p2p
		our_actor.skin = inc(our_actor.def->skin, SKIN_BROWN, RAND (SKIN_BROWN, SKIN_TAN));//Increment a random # of times.
		our_actor.hair = inc(our_actor.def->hair, HAIR_BLACK, RAND (HAIR_BLACK, our_actor.def->type >= draegoni_female ? HAIR_PURPLE:HAIR_WHITE));
		our_actor.eyes = inc(our_actor.def->eyes, EYES_BROWN, RAND (EYES_BROWN, EYES_GOLD));
		our_actor.shirt = inc(our_actor.def->shirts, SHIRT_BLACK, RAND (SHIRT_BLACK, SHIRT_YELLOW));
		our_actor.pants = inc(our_actor.def->pants, PANTS_BLACK, RAND (PANTS_BLACK, PANTS_WHITE));
		our_actor.boots = inc(our_actor.def->boots, BOOTS_BLACK, RAND (BOOTS_BLACK, BOOTS_ORANGE));
		our_actor.head = inc(our_actor.def->head, HEAD_1, RAND (HEAD_1, our_actor.def->type==human_female?HEAD_5:HEAD_4));
		our_actor.race = our_actor.def->type;
		our_actor.male = our_actor.race<gnome_female?our_actor.race%2:!(our_actor.race%2);

		game_minute = 120;	//Midday. So that it's bright and sunny.
		real_game_minute = game_minute;

		change_map ("./maps/newcharactermap.elm");

		newchar_root_win = create_window (win_newchar, -1, -1, 0, 0, window_width, window_height, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW_LAST);

		set_window_handler (newchar_root_win, ELW_HANDLER_DISPLAY, &display_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_CLICK, &click_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_KEYPRESS, &keypress_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_SHOW, &show_newchar_handler);
		set_window_handler (newchar_root_win, ELW_HANDLER_AFTER_SHOW, &update_have_display);
		set_window_handler (newchar_root_win, ELW_HANDLER_HIDE, &update_have_display);
		set_window_handler (newchar_root_win, ELW_HANDLER_UI_SCALE, &ui_scale_newchar_handler);

		newchar_advice_win = create_window ("Advice", newchar_root_win, 0, 100, 10, 200, 100, ELW_USE_UISCALE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW|ELW_ALPHA_BORDER);
		set_window_handler (newchar_advice_win, ELW_HANDLER_DISPLAY, &display_advice_handler);

		if (newchar_root_win >= 0 && newchar_root_win < windows_list.num_windows)
			ui_scale_newchar_handler(&windows_list.window[newchar_root_win]);

		create_newchar_hud_window();

		LOG_TO_CONSOLE(c_green1, char_help);
	} else {
		show_window(newchar_root_win);
		show_window(newchar_advice_win);
		show_window(newchar_hud_win);
		show_window(color_race_win);
		hide_window(namepass_win);
	}
	old_use_windowed_chat = use_windowed_chat;
	use_windowed_chat = 0;
}

static int active=0;
static int hidden=0;
static int are_you_sure=1;
static int numbers_in_name=0;

static char * get_pass_str(int l)
{
	static char str[20];

	memset(str, '*', l);
	str[l]=0;

	return str;
}

//Returns 1 if it's valid, 0 if invalid and -1 if there's too many numbers in the name
static int check_character(int type, char ch)
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
			retval=-2;
		}
	} else {	// password
		if (VALID_PASSWORD_CHAR(ch)) retval=1;
	}

	return retval;
}

static void add_text_to_buffer(int color, const char * text, int time_to_display)
{
	if (namepass_win < 0 || namepass_win >= windows_list.num_windows)
		return;
	put_small_colored_text_in_box_zoomed(color, (unsigned char*)text, strlen(text),
		widget_get_width(namepass_win, error_widget_id), create_char_error_str, windows_list.window[namepass_win].current_scale);
	display_time=cur_time+time_to_display;
}

static void create_character(void)
{
	if(inputs[0].pos<3){
		add_text_to_buffer(c_red2, error_username_length, DEF_MESSAGE_TIMEOUT);
		return;
	} else if(inputs[1].pos<4){
		add_text_to_buffer(c_red2, error_password_length, DEF_MESSAGE_TIMEOUT);
		return;
	} else if(!strncasecmp(inputs[1].str, actors_list[0]->actor_name, strlen(actors_list[0]->actor_name))){
		add_text_to_buffer(c_red2, error_bad_pass, DEF_MESSAGE_TIMEOUT);
		return;
	} else if(strcmp(inputs[1].str, inputs[2].str)){
		add_text_to_buffer(c_red2, error_pass_no_match, DEF_MESSAGE_TIMEOUT);
		return;
	}

	if(are_you_sure){
		creating_char=0;
		// Clear the error message, if necessary
		create_char_error_str[0] = '\0';
		send_new_char(inputs[0].str, inputs[1].str, our_actor.skin, our_actor.hair, our_actor.eyes, our_actor.shirt, our_actor.pants, our_actor.boots, our_actor.head, our_actor.race);
	} else {
		are_you_sure=1;

		add_text_to_buffer(c_orange3, error_confirm_create_char, DEF_MESSAGE_TIMEOUT);
		LOG_TO_CONSOLE(c_green2, "\n");
		LOG_TO_CONSOLE(c_green2, remember_change_appearance);
	}
}

void login_from_new_char(void)
{
	set_username(inputs[0].str);
	set_password(inputs[1].str);

	// now destroy reference to ourself, otherwise we'll mess up the ID's
	destroy_all_actors();
	our_actor.our_model=NULL;

	// close help and setting windows
	if (tab_help_win >= 0) hide_window (tab_help_win);
	if (elconfig_win >= 0) hide_window (elconfig_win);

	//restore use_windowed_chat
	use_windowed_chat = old_use_windowed_chat;
	hide_window(newchar_hud_win);

	//now send the log in info
	send_login_info();
}

//The character design window
static void change_race(int new_race)
{
	if(our_actor.race_id==new_race)return;
	destroy_all_actors();
	our_actor.our_model = NULL;
	our_actor.race_id=new_race;
	our_actor.def=&races[new_race];
	our_actor.skin = our_actor.def->skin[find_pos_in_enum(our_actor.def->skin, our_actor.skin)];//Increment a random # of times.
	our_actor.hair = our_actor.def->hair[find_pos_in_enum(our_actor.def->hair, our_actor.hair)];
	our_actor.eyes = our_actor.def->eyes[find_pos_in_enum(our_actor.def->eyes, our_actor.eyes)];
	our_actor.shirt = our_actor.def->shirts[find_pos_in_enum(our_actor.def->shirts, our_actor.shirt)];
	our_actor.pants = our_actor.def->pants[find_pos_in_enum(our_actor.def->pants, our_actor.pants)];
	our_actor.boots = our_actor.def->boots[find_pos_in_enum(our_actor.def->boots, our_actor.boots)];
	our_actor.head = our_actor.def->head[find_pos_in_enum(our_actor.def->head, our_actor.head)];
	our_actor.race = our_actor.def->type;
	our_actor.male = our_actor.race<gnome_female?our_actor.race%2:!(our_actor.race%2);

	change_actor();
}

int book_human=200000;
int book_dwarf=200001;
int book_elf=200002;
int book_gnome=200003;
int book_orchan=200004;
int book_draegoni=200005;

static void toggle_book(int id)
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

static int newchar_mouseover = 0; //book id if mouse is over a book 1 if mouse is over p2p race
static int newchar_mouseover_time = 0;
static char* tooltip = NULL;
static int tooltip_x = 0;
static int tooltip_y = 0;

static int click_done_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	if(w->window_id == color_race_win)
	{
		hide_window(color_race_win);
		show_window(namepass_win);
	}
	else
	{
		if(w->Flags)
		{
			create_character();
		}
		else
		{
			w->Flags = BUTTON_ACTIVE;
			add_text_to_buffer(c_orange3, error_confirm_create_char, DEF_MESSAGE_TIMEOUT);
		}
	}
	return 1;
}

static int click_back_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	if(w->window_id == color_race_win)
	{
		destroy_all_actors();
		our_actor.our_model = NULL;
		use_windowed_chat = old_use_windowed_chat; //Restore use_windowed_chat
		hide_window(newchar_hud_win);
		hide_window(newchar_root_win);
		show_window(login_root_win);
		hide_hud_windows();
	}
	else
	{
		hide_window(namepass_win);
		show_window(color_race_win);
	}
	return 1;
}

static int click_namepass_field(widget_list *w, int mx, int my, Uint32 flags)
{
	active = *(int*)w->spec;
	return 1;
}

static int errorbox_draw(widget_list *w)
{
	if (w->window_id >= 0 && w->window_id < windows_list.num_windows)
		draw_string_small_zoomed(w->pos_x, w->pos_y, (unsigned char*)create_char_error_str, 8, windows_list.window[w->window_id].current_scale);
	return 1;
}

static int name_draw(widget_list *w)
{
	draw_smooth_button((char*)w->widget_info, w->size, w->pos_x, w->pos_y, w->len_x - 2*BUTTONRADIUS*w->size, 1, w->r, w->g, w->b, active == *(int*)w->spec, 0.32f, 0.23f, 0.15f, 0.0f);
	return 1;
}

static int password_draw(widget_list *w)
{
	if(!hidden)
	{
		draw_smooth_button(get_pass_str(strlen((char*)w->widget_info)), w->size, w->pos_x, w->pos_y, w->len_x - 2*BUTTONRADIUS*w->size, 1, w->r, w->g, w->b, active == *(int*)w->spec, 0.32f, 0.23f, 0.15f, 0.0f);
	}
	else
	{
		draw_smooth_button((char*)w->widget_info, w->size, w->pos_x, w->pos_y, w->len_x - 2*BUTTONRADIUS*w->size, 1, w->r, w->g, w->b, active == *(int*)w->spec, 0.32f, 0.23f, 0.15f, 0.0f);
	}
	return 1;
}

static int keypress_namepass_handler (window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	Uint8 ch = key_to_char (unikey);
	int ret=0;
	struct input_text * t=&inputs[active];

	if (clear_player_name)
	{
		clear_player_name = 0;
		inputs[0].pos = 0;
		inputs[0].str[0] = 0;
	}

	if ((ret=check_character (active > 0, ch)))
	{
		if (ret==-1)
		{
			add_text_to_buffer (c_red2, error_max_digits, DEF_MESSAGE_TIMEOUT);
		}
		else if (ret==-2)
		{
			add_text_to_buffer (c_red2, error_illegal_character, DEF_MESSAGE_TIMEOUT);
		}
		else if (t->pos + 1 > MAX_USERNAME_LENGTH - 1)		// MAX_USERNAME_LENGTH includes the null terminator and t->pos counts from 0
		{
			add_text_to_buffer (c_red2, error_length, DEF_MESSAGE_TIMEOUT);
		}
		else
		{
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
	else if (unikey == SDLK_BACKSPACE)
#else
	else if (key == SDLK_BACKSPACE)
#endif
	{
		if (t->pos>0)
		{
			t->pos--;
			if ((active == 0) && isdigit (t->str[t->pos]))
				numbers_in_name--;
			t->str[t->pos] = 0;
			ret = 1;	// Reused to show that a letter has been removed
		}
	}
	else
	{
		//only send error messages on non-null characters
		if(ch != 0){
			add_text_to_buffer (c_red2, error_illegal_character, DEF_MESSAGE_TIMEOUT);
		}
	}

	if(active>0){
		//Password/confirm
		if((inputs[1].pos > 0) && (inputs[2].pos > 0) && ret){
			if(!strncasecmp(inputs[1].str, actors_list[0]->actor_name, strlen(actors_list[0]->actor_name))){
				add_text_to_buffer(c_red2, error_bad_pass, DEF_MESSAGE_TIMEOUT);
			} else if(strcmp(inputs[1].str, inputs[2].str)){
				add_text_to_buffer(c_red2, error_pass_no_match, DEF_MESSAGE_TIMEOUT);
			} else {
				add_text_to_buffer(c_green1, passwords_match, DEF_MESSAGE_TIMEOUT);
			}
		}
	} else {
		safe_snprintf(actors_list[0]->actor_name, sizeof(actors_list[0]->actor_name), "%s", inputs[0].str);
	}

	return 1;
}

static const struct WIDGET_TYPE name_type = {NULL, &name_draw, &click_namepass_field, NULL, NULL, NULL, NULL, NULL}; //custom widget for the name button
static const struct WIDGET_TYPE password_type = {NULL, &password_draw, &click_namepass_field, NULL, NULL, NULL, NULL, NULL}; //custom widget for the password buttons
static const struct WIDGET_TYPE errorbox_type = {NULL, &errorbox_draw, NULL, NULL, NULL, NULL, NULL, NULL}; //custom widget for displaying name/password errors
static int specs[3] = {0, 1, 2};

static int init_namepass_handler(window_info * win)
{
	float r = 0.77f, g = 0.57f, b = 0.39f; //widget color
	float very_small = DEFAULT_SMALL_RATIO  * win->current_scale; //font sizes
	float bit_small = 0.9f * win->current_scale;
	float normal = 1.0f * win->current_scale;
	int free_widget_id = 8;
	int widget_id;
	int sep = (int)(0.5 + win->current_scale * 5);
	int y = sep;
	int x_off = 3 * sep;
	int size = (int)(0.5 + win->current_scale * 15);
	int input_len_x = (MAX_USERNAME_LENGTH - 1) * very_small * DEFAULT_FONT_X_LEN + 2 * BUTTONRADIUS * very_small;
	int input_len_y = 2 * BUTTONRADIUS * very_small;
	int input_off = strlen(login_username_str);
	int center = (input_len_y - normal * DEFAULT_FONT_Y_LEN) / 2;

	if (strlen(login_password_str) > input_off) input_off = strlen(login_password_str);
	if (strlen(confirm_password) > input_off) input_off = strlen(confirm_password);
	input_off = x_off + (input_off + 1) * normal * DEFAULT_FONT_X_LEN;

	//Choose your name and password
	widget_id = label_add_extended(win->window_id, free_widget_id++, 0, (win->len_x - strlen((char*)win_name_pass)*DEFAULT_FONT_X_LEN*bit_small)/2, y, 0, bit_small, r, g, b, (char*)win_name_pass);
	y += widget_get_height(win->window_id, widget_id) + 2*sep;
	label_add_extended(win->window_id, free_widget_id++, 0, x_off, y + center, 0, normal, r, g, b, (char*)login_username_str);
	widget_id = widget_add(win->window_id, free_widget_id++, 0, input_off, y, input_len_x, input_len_y, 0, very_small, r, g, b, &name_type, inputs[0].str, (void*)&specs[0]);
	y += input_len_y + 2*sep;
	label_add_extended(win->window_id, free_widget_id++, 0, x_off, y + center, 0, normal, r, g, b, (char*)login_password_str);
	widget_id = widget_add(win->window_id, free_widget_id++, 0, input_off, y, input_len_x, input_len_y, 0, very_small, r, g, b, &password_type, inputs[1].str, (void*)&specs[1]);
	y += input_len_y + 2*sep;
	label_add_extended(win->window_id ,free_widget_id++, 0, x_off, y + center, 0, normal, r, g, b, (char*)confirm_password);
	widget_id = widget_add(win->window_id, free_widget_id++, 0, input_off, y, input_len_x, input_len_y, 0, very_small, r, g, b, &password_type, inputs[2].str, (void*)&specs[2]);
	y += input_len_y + 2*sep;

	//Show password checkbox and error label
	label_add_extended(win->window_id, free_widget_id++, 0, win->len_x - 4 * sep - size - strlen((char*)show_password)*DEFAULT_FONT_X_LEN*bit_small, y, 0, bit_small, r, g, b, (char*)show_password);
	widget_id = checkbox_add_extended(win->window_id, free_widget_id++, 0, win->len_x - 2 * sep - size, y, size, size, 0, bit_small, r, g, b, &hidden);
	y += DEFAULT_FONT_Y_LEN * bit_small + sep;
	error_widget_id = widget_add(win->window_id, free_widget_id++, 0, 2 * sep, y, win->len_x - 4 * sep, win->len_y - y, 0, very_small, r, g, b, &errorbox_type, NULL, NULL);

	//Done and Back buttons
	widget_id = button_add_extended(win->window_id, free_widget_id++, 0, 0, 0, 0, 0, 0, very_small, r, g, b, char_done);
	widget_set_OnClick(win->window_id, widget_id, &click_done_handler);
	y = win->len_y - widget_get_height(win->window_id, widget_id) - sep;
	widget_move(win->window_id, widget_id, (win->len_x / 2 - widget_get_width(win->window_id, widget_id)) / 2, y);
	widget_id = button_add_extended(win->window_id, free_widget_id++, 0, 0, 0, 0, 0, 0, very_small, r, g, b, char_back);
	widget_set_OnClick(win->window_id, widget_id, &click_back_handler);
	widget_move(win->window_id, widget_id, win->len_x / 2 + (win->len_x / 2 - widget_get_width(win->window_id, widget_id)) / 2, y);
	return 1;
}

static int click_newchar_book_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;
	image_set_uv(w->window_id, w->id, (float)32/256,1.0f-(float)64/256,(float)63/256,1.0f-(float)95/256);
	toggle_book(w->id);
	newchar_mouseover = w->id;
	return 1;
}

static int mouseover_newchar_book_handler(widget_list *w, int mx, int my)
{
	image_set_uv(w->window_id, w->id, (float)32/256,1.0f-(float)64/256,(float)63/256,1.0f-(float)95/256);
	newchar_mouseover = w->id;
	if( newchar_mouseover == book_human) tooltip = about_human;
	else if( newchar_mouseover == book_elf) tooltip = about_elves;
	else if( newchar_mouseover == book_dwarf) tooltip = about_dwarfs;
	else if( newchar_mouseover == book_gnome) tooltip = about_gnomes;
	else if( newchar_mouseover == book_orchan) tooltip = about_orchans;
	else if( newchar_mouseover == book_draegoni) tooltip = about_draegoni;
	else if( newchar_mouseover == book_human) tooltip = about_human;
	else return 1;
	newchar_mouseover_time = cur_time; //we are currently over something
	{
		window_info *win = &windows_list.window[w->window_id];
		int size = (1 + strlen(tooltip)) * win->small_font_len_x;
		tooltip_x = (mx + size <= hud_x) ? mx : hud_x - size;
		tooltip_y = my + win->small_font_len_y;
	}
	return 1;
}

static int click_newchar_gender_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	int i = multiselect_get_selected(w->window_id, w->id);
	switch(i)
	{
		case 0:
			change_race(our_actor.race_id + !our_actor.male);
		break;
		case 1:
			change_race(our_actor.race_id - our_actor.male);
	break;
	}
	return 1;
}

static int mouseover_p2p_race_handler(widget_list *w, int mx, int my)
{
	window_info *win = &windows_list.window[w->window_id];
	int size = (1 + strlen(p2p_race)) * win->small_font_len_x;
	newchar_mouseover = 1; //we are over an p2p race
	newchar_mouseover_time = cur_time; //we are currently over something
	tooltip = p2p_race;
	tooltip_x = (mx + size <= hud_x) ? mx : hud_x - size;
	tooltip_y = my + win->small_font_len_y;
	return 1;
}

static int click_newchar_race_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	int i = multiselect_get_selected(w->window_id, w->id);
	switch(i)
	{
		case 0:
			change_race(human_female + our_actor.male);
		break;
		case 1:
			change_race(elf_female + our_actor.male);
		break;
		case 2:
			change_race(dwarf_female + our_actor.male);
		break;
		case 3:
			change_race(gnome_female - 31 + our_actor.male);
		break;
		case 4:
			change_race(orchan_female - 31 + our_actor.male);
		break;
		case 5:
			change_race(draegoni_female - 31 + our_actor.male);
		break;
	}
	return 0;
}

static void update_head(void)
{
	// Detach the old head, and reattach and save the new one.
	model_detach_mesh(our_actor.our_model,
		our_actor.our_model->body_parts->head_meshindex);
	model_attach_mesh(our_actor.our_model,
		actors_defs[our_actor.race].head[our_actor.head].mesh_index);
	our_actor.our_model->body_parts->head_meshindex =
		actors_defs[our_actor.race].head[our_actor.head].mesh_index;
}

static void update_skin(void)
{
	// Copy the skin texture names.
	my_strncp(our_actor.our_model->body_parts->hands_tex,
		actors_defs[our_actor.race].skin[our_actor.skin].hands_name,
		sizeof(our_actor.our_model->body_parts->hands_tex));
	my_strncp(our_actor.our_model->body_parts->head_tex,
		actors_defs[our_actor.race].skin[our_actor.skin].head_name,
		sizeof(our_actor.our_model->body_parts->head_tex));

	change_enhanced_actor(our_actor.our_model->texture_id,
		our_actor.our_model->body_parts);
}

static void update_hair(void)
{
	// Copy the hair texture name.
	my_strncp(our_actor.our_model->body_parts->hair_tex,
		actors_defs[our_actor.race].hair[our_actor.hair].hair_name,
		sizeof(our_actor.our_model->body_parts->hair_tex));

	change_enhanced_actor(our_actor.our_model->texture_id,
		our_actor.our_model->body_parts);
}

#ifdef NEW_EYES
static void update_eyes()
{
	// Copy the eyes texture name.
	my_strncp(our_actor.our_model->body_parts->eyes_tex,
		actors_defs[our_actor.race].eyes[our_actor.eyes].eyes_name,
		sizeof(our_actor.our_model->body_parts->eyes_tex));

	change_enhanced_actor(our_actor.our_model->texture_id,
		our_actor.our_model->body_parts);
}
#endif	/* NEW_EYES */

static void update_shirt()
{
	// Copy the shirt and arms texture names.
	my_strncp(our_actor.our_model->body_parts->arms_tex,
		actors_defs[our_actor.race].shirt[our_actor.shirt].arms_name,
		sizeof(our_actor.our_model->body_parts->arms_tex));
	my_strncp(our_actor.our_model->body_parts->torso_tex,
		actors_defs[our_actor.race].shirt[our_actor.shirt].torso_name,
		sizeof(our_actor.our_model->body_parts->torso_tex));

	// If we need a new mesh, drop the old one and load it.
	if (actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index !=
		our_actor.our_model->body_parts->torso_meshindex)
	{
		model_detach_mesh(our_actor.our_model,
			our_actor.our_model->body_parts->torso_meshindex);
		model_attach_mesh(our_actor.our_model,
			actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index);
		our_actor.our_model->body_parts->torso_meshindex =
			actors_defs[our_actor.race].shirt[our_actor.shirt].mesh_index;
	}

	change_enhanced_actor(our_actor.our_model->texture_id, our_actor.our_model->body_parts);
}

static void update_pants(void)
{
	// Copy the pants texture name.
	my_strncp(our_actor.our_model->body_parts->pants_tex,
		actors_defs[our_actor.race].legs[our_actor.pants].legs_name,
		sizeof(our_actor.our_model->body_parts->pants_tex));

	// If we need a new mesh, drop the old one and load it.
	if (actors_defs[our_actor.race].legs[our_actor.pants].mesh_index !=
		our_actor.our_model->body_parts->legs_meshindex)
	{
		model_detach_mesh(our_actor.our_model,
			our_actor.our_model->body_parts->legs_meshindex);
		model_attach_mesh(our_actor.our_model,
			actors_defs[our_actor.race].legs[our_actor.pants].mesh_index);
		our_actor.our_model->body_parts->legs_meshindex =
			actors_defs[our_actor.race].legs[our_actor.pants].mesh_index;
	}

	change_enhanced_actor(our_actor.our_model->texture_id,
		our_actor.our_model->body_parts);
}

static void update_boots(void)
{
	// Copy the new boots texture name.
	my_strncp(our_actor.our_model->body_parts->boots_tex,
		actors_defs[our_actor.race].boots[our_actor.boots].boots_name,
		sizeof(our_actor.our_model->body_parts->boots_tex));

	// If we need a new mesh, drop the old one and load it.
	if (actors_defs[our_actor.race].boots[our_actor.boots].mesh_index !=
		our_actor.our_model->body_parts->boots_meshindex)
	{
		model_detach_mesh(our_actor.our_model,
			our_actor.our_model->body_parts->boots_meshindex);
		model_attach_mesh(our_actor.our_model,
			actors_defs[our_actor.race].boots[our_actor.boots].mesh_index);
		our_actor.our_model->body_parts->boots_meshindex =
			actors_defs[our_actor.race].boots[our_actor.boots].mesh_index;
	}

	change_enhanced_actor(our_actor.our_model->texture_id,
		our_actor.our_model->body_parts);
}

static int head_dec_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.head = dec(our_actor.def->head, our_actor.head, 1);

	update_head();

	return 1;
}

static int head_inc_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.head = inc(our_actor.def->head, our_actor.head, 1);

	update_head();

	return 1;
}

static int skin_dec_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.skin = dec(our_actor.def->skin, our_actor.skin, 1);

	update_skin();

	return 1;
}

static int skin_inc_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.skin = inc(our_actor.def->skin, our_actor.skin, 1);

	update_skin();

	return 1;
}

static int hair_dec_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.hair = dec(our_actor.def->hair, our_actor.hair, 1);

	update_hair();

	return 1;
}

static int hair_inc_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.hair = inc(our_actor.def->hair, our_actor.hair, 1);

	update_hair();

	return 1;
}

static int eyes_dec_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.eyes = dec(our_actor.def->eyes, our_actor.eyes, 1);
#ifdef NEW_EYES
	update_eyes();
#endif
	return 1;
}

static int eyes_inc_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.eyes = inc(our_actor.def->eyes, our_actor.eyes, 1);
#ifdef NEW_EYES
	update_eyes();
#endif
	return 1;
}

static int shirt_dec_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.shirt = dec(our_actor.def->shirts, our_actor.shirt, 1);

	update_shirt();

	return 1;
}

static int shirt_inc_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.shirt = inc(our_actor.def->shirts, our_actor.shirt, 1);

	update_shirt();

	return 1;
}

static int pants_dec_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.pants=dec(our_actor.def->pants, our_actor.pants, 1);

	update_pants();

	return 1;
}

static int pants_inc_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.pants=inc(our_actor.def->pants, our_actor.pants, 1);

	update_pants();

	return 1;
}

static int boots_dec_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.boots = dec(our_actor.def->boots, our_actor.boots, 1);

	update_boots();

	return 1;
}

static int boots_inc_handler(widget_list *w, int mx, int my, Uint32 flags)
{
	our_actor.boots = inc(our_actor.def->boots, our_actor.boots, 1);

	update_boots();

	return 1;
}

static int mouseover_color_race_handler(window_info *win, int mx, int my)
{
	if(!(newchar_mouseover == book_human)) image_set_uv(win->window_id, book_human, (float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256);
	if(!(newchar_mouseover == book_elf)) image_set_uv(win->window_id, book_elf, (float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256);
	if(!(newchar_mouseover == book_dwarf)) image_set_uv(win->window_id, book_dwarf, (float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256);
	if(!(newchar_mouseover == book_gnome)) image_set_uv(win->window_id, book_gnome, (float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256);
	if(!(newchar_mouseover == book_orchan)) image_set_uv(win->window_id, book_orchan, (float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256);
	if(!(newchar_mouseover == book_draegoni)) image_set_uv(win->window_id, book_draegoni, (float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256);
	if(!newchar_mouseover)
		newchar_mouseover_time = 0; //reset timer
	newchar_mouseover = 0;
	return 1;
}

static int box_draw(widget_list *w)
{
	glColor3f(w->r, w->g, w->b); //draw a box
	draw_box(w->widget_info, w->pos_x, w->pos_y, w->len_x, w->len_y, w->size, 0);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

static const struct WIDGET_TYPE box_type = {NULL, &box_draw, NULL, NULL, NULL, NULL, NULL, NULL}; //a custom box widget

static int init_color_race_handler(window_info * win)
{
	float r = 0.77f, g = 0.57f, b = 0.39f; //widget color
	float rh = 0.32f, gh = 0.23f, bh = 0.15f; //highlighted color
	float very_small = DEFAULT_SMALL_RATIO * win->current_scale; //font sizes
	float bit_small = 0.9f * win->current_scale;
	float normal = 1.0f * win->current_scale;
	int free_widget_id = 8; //next free widget id
	int widget_id;
	int sep = (int)(0.5 + win->current_scale * 5);
	int y = sep;

	//Design your character
	widget_id = label_add_extended(win->window_id, free_widget_id++, 0, (win->len_x - strlen((char*)win_design)*DEFAULT_FONT_X_LEN*bit_small)/2, y, 0, bit_small, r, g, b, (char*)win_design);
	y += widget_get_height(win->window_id, widget_id);

	//Gender selection
	{
		int box_label_height = very_small * DEFAULT_FONT_Y_LEN;
		int button_height = 2 * very_small * BUTTONRADIUS;
		int element_height = box_label_height + button_height;
		int button_width = (int)(0.5 + win->current_scale * 100);
		y += sep + box_label_height / 2;
		widget_add(win->window_id, free_widget_id++, 0, sep, y, win->len_x - 2 * sep, element_height, 0, very_small, r, g, b, &box_type, gender_str, NULL);
		widget_id = multiselect_add_extended(win->window_id, free_widget_id++, 0, 2 * sep, y + box_label_height / 2, win->len_x - 4 * sep, button_height, normal, r, g, b, rh, gh, bh, 2);
		multiselect_button_add_extended(win->window_id, widget_id, 0, 0, button_width, male_str, very_small, our_actor.male);
		multiselect_button_add_extended(win->window_id, widget_id, win->len_x - button_width - 4 * sep, 0, button_width, female_str, very_small, !our_actor.male);
		widget_set_OnClick(win->window_id, widget_id, &click_newchar_gender_handler);
		y += element_height;
	}

	//Races
	{
		int book_ids[6] = {book_human, book_elf, book_dwarf, book_gnome, book_orchan, book_draegoni};
		int box_label_height = very_small * DEFAULT_FONT_Y_LEN;
		int button_height = 2 * very_small * BUTTONRADIUS;
		int button_width = (int)(0.5 + win->current_scale * 80);
		int button_y_off = box_label_height;
		int button_set_height = 3 * button_height + 2 * sep;
		int button_set_width = win->len_x - 4 * sep;
		int col_two_x_off = sep + button_set_width / 2;
		int element_height = 2 * box_label_height + button_set_height;
		size_t i;
		y += sep + box_label_height / 2;

		widget_add(win->window_id, free_widget_id++, 0, sep, y, win->len_x - 2 * sep, element_height, 0, very_small, r, g, b, &box_type, race_str, NULL);

		widget_id = widget_add(win->window_id, free_widget_id++, 0, sep + col_two_x_off, y + box_label_height / 2 + 2,
			3 * sep + button_width + button_height, button_set_height + box_label_height / 2 + sep, 0, very_small, 1.0f, 0.0f, 0.0f, &box_type, "P2P", NULL);
		widget_set_OnMouseover(win->window_id, widget_id, &mouseover_p2p_race_handler);

		widget_id = multiselect_add_extended(win->window_id, free_widget_id++, 0, 2 * sep, y + button_y_off , button_set_width, button_set_height, normal, r, g, b, rh, gh, bh, 6);
		multiselect_button_add_extended(win->window_id, widget_id, 0, 0, button_width, human_str, very_small, our_actor.race==human_female||our_actor.race==human_male);
		multiselect_button_add_extended(win->window_id, widget_id, 0, button_height + sep, button_width, elf_str, very_small, our_actor.race==elf_female||our_actor.race==elf_male);
		multiselect_button_add_extended(win->window_id, widget_id, 0, 2 * (button_height + sep), button_width, dwarf_str, very_small, our_actor.race==dwarf_female||our_actor.race==dwarf_male);
		multiselect_button_add_extended(win->window_id, widget_id, col_two_x_off, 0, button_width, gnome_str, very_small, our_actor.race==dwarf_female||our_actor.race==dwarf_male);
		multiselect_button_add_extended(win->window_id, widget_id, col_two_x_off, button_height + sep, button_width, orchan_str, very_small, our_actor.race==orchan_female||our_actor.race==orchan_male);
		multiselect_button_add_extended(win->window_id, widget_id, col_two_x_off, 2 * (button_height + sep), button_width, draegoni_str, very_small, our_actor.race==draegoni_female||our_actor.race==draegoni_male);
		widget_set_OnClick(win->window_id, widget_id, &click_newchar_race_handler);

		for(i = 0; i < 3; i++)
		{
			widget_id = image_add_extended(win->window_id, book_ids[i], 0, 2 * sep + button_width + sep, y + button_y_off + (button_height + sep)*i, button_height, button_height, 0, 1.0f, 1.0f, 1.0f, 1.0f, icons_text, (float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256, -1);
			widget_set_OnClick(win->window_id, widget_id, &click_newchar_book_handler);
			widget_set_OnMouseover(win->window_id, widget_id, &mouseover_newchar_book_handler);
		}
		for(i = 0; i< 3; i++)
		{
			widget_id = image_add_extended(win->window_id, book_ids[i+3], 0, 3 * sep + col_two_x_off + button_width, y + button_y_off + (button_height + sep)*i, button_height, button_height, 0, 1.0f, 1.0f, 1.0f, 1.0f, icons_text, (float)0/256,1.0f-(float)64/256,(float)31/256,1.0f-(float)95/256, -1);
			widget_set_OnClick(win->window_id, widget_id, &click_newchar_book_handler);
			widget_set_OnMouseover(win->window_id, widget_id, &mouseover_newchar_book_handler);
		}
		y += element_height;
	}

	//Appereance
	{
		char* body_part_strs[7] = {head_str, skin_str, hair_str, eyes_str, shirt_str, pants_str, boots_str};
		void* body_handlers_dec[7] = {&head_dec_handler, &skin_dec_handler, &hair_dec_handler, &eyes_dec_handler, &shirt_dec_handler, &pants_dec_handler, &boots_dec_handler};
		void* body_handlers_inc[7] = {&head_inc_handler, &skin_inc_handler, &hair_inc_handler, &eyes_inc_handler, &shirt_inc_handler, &pants_inc_handler, &boots_inc_handler};
		int box_label_height = very_small * DEFAULT_FONT_Y_LEN;
		int changer_height = DEFAULT_FONT_Y_LEN * bit_small;
		int element_height = box_label_height + 4 * changer_height + 3 * sep;
		int size = 2 * DEFAULT_FONT_X_LEN * bit_small;
		int x;
		size_t i;
		y += sep + box_label_height / 2;
		widget_add(win->window_id, free_widget_id++, 0, sep, y, win->len_x - 2 * sep, element_height, 0, very_small, r, g, b, &box_type, appearance_str, NULL);
		y += box_label_height / 2;
		for(i = 0; i < 4; i++)//Head, Skin, Hair and Eyes
		{
			widget_id = label_add_extended(win->window_id, free_widget_id++, 0, 2 * sep, y+(DEFAULT_FONT_Y_LEN*bit_small+sep)*i, 0, bit_small, r, g, b, "<<");
			widget_set_OnClick(win->window_id, widget_id, body_handlers_dec[i]);
			x = 2 * sep + size + (win->len_x/2 - 3 * sep - 2*size - strlen((char*)body_part_strs[i])*DEFAULT_FONT_X_LEN*bit_small)/2;
			label_add_extended(win->window_id, free_widget_id++, 0, x, y+(DEFAULT_FONT_Y_LEN*bit_small+sep)*i, 0, bit_small, r, g, b, (char*)body_part_strs[i]);
			widget_id = label_add_extended(win->window_id, free_widget_id++, 0, win->len_x/2-sep-size, y+(DEFAULT_FONT_Y_LEN*bit_small+sep)*i, 0, bit_small, r, g, b, ">>");
			widget_set_OnClick(win->window_id, widget_id, body_handlers_inc[i]);
		}
		for(i = 4; i < 7; i++)//Shirt, Pants and Boots
		{
			widget_id = label_add_extended(win->window_id, free_widget_id++, 0, win->len_x/2+sep, y+(DEFAULT_FONT_Y_LEN*bit_small+sep)*(i-4), 0, bit_small, r, g, b, "<<");
			widget_set_OnClick(win->window_id, widget_id, body_handlers_dec[i]);
			x = win->len_x/2 + sep + size + (win->len_x/2 - 3 * sep - 2*size - strlen((char*)body_part_strs[i])*DEFAULT_FONT_X_LEN*bit_small)/2;
			label_add_extended(win->window_id, free_widget_id++, 0, x, y+(DEFAULT_FONT_Y_LEN*bit_small+sep)*(i-4), 0, bit_small, r, g, b, (char*)body_part_strs[i]);
			widget_id = label_add_extended(win->window_id, free_widget_id++, 0, win->len_x-2 * sep-size, y+(DEFAULT_FONT_Y_LEN*bit_small+sep)*(i-4), 0, bit_small, r, g, b, ">>");
			widget_set_OnClick(win->window_id, widget_id, body_handlers_inc[i]);
		}
	}

	//Done and Back
	widget_id = button_add_extended(win->window_id, free_widget_id++, 0, 0, 0, 0, 0, 0, very_small, r, g, b, char_done);
	widget_set_OnClick(win->window_id, widget_id, &click_done_handler);
	y = win->len_y - widget_get_height(win->window_id, widget_id) - sep;
	widget_move(win->window_id, widget_id, (win->len_x / 2 - widget_get_width(win->window_id, widget_id)) / 2, y);
	widget_id = button_add_extended(win->window_id, free_widget_id++, 0, 0, 0, 0, 0, 0, very_small, r, g, b, char_back);
	widget_set_OnClick(win->window_id, widget_id, &click_back_handler);
	widget_move(win->window_id, widget_id, win->len_x / 2 + (win->len_x / 2 - widget_get_width(win->window_id, widget_id)) / 2, y);

	return 1;
}

static int tooltip_win;

static int display_tooltip_handler(window_info * win)
{
	if(newchar_mouseover_time == cur_time) //draw a help text if currently over something
		show_help(tooltip, tooltip_x, tooltip_y, win->current_scale);
	return 1;
}

static int display_newchar_hud_handler(window_info * win)
{
	glColor3f(0.0f, 0.0f, 0.0f); //Draw a black background
	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(0, win->len_y);
	glVertex2i(win->len_x, win->len_y);
	glVertex2i(win->len_x, 0);
	glEnd();
	glColor3f(0.77f, 0.57f, 0.39f);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

static void create_newchar_hud_window(void)
{
	if(newchar_hud_win != -1) return;

	newchar_hud_win = create_window("Newchar_Hud", newchar_root_win, 0, window_width - hud_x, 0, hud_x, window_height - hud_y, ELW_USE_UISCALE|ELW_USE_BORDER|ELW_SHOW|ELW_SHOW_LAST);
	set_window_handler(newchar_hud_win, ELW_HANDLER_DISPLAY, &display_newchar_hud_handler);

	tooltip_win = create_window("Help Text", newchar_hud_win, 0, 0, 0, 0, 0, ELW_USE_UISCALE|ELW_SHOW);
	set_window_handler(tooltip_win, ELW_HANDLER_DISPLAY, &display_tooltip_handler);

	color_race_win = create_window(win_design, newchar_hud_win, 0, 0, 0, hud_x, window_height - hud_y, ELW_USE_UISCALE|ELW_SHOW|ELW_SHOW_LAST); //Design your character
	set_window_handler(color_race_win, ELW_HANDLER_INIT, &init_color_race_handler);
	set_window_handler(color_race_win, ELW_HANDLER_MOUSEOVER, &mouseover_color_race_handler);
	init_window(color_race_win, newchar_hud_win, 0, 0, 0, hud_x, window_height - hud_y);

	namepass_win = create_window(win_name_pass, newchar_hud_win, 0, 0, 0, hud_x, window_height - hud_y, ELW_USE_UISCALE|ELW_SHOW_LAST); //Choose name and password
	set_window_handler(namepass_win, ELW_HANDLER_INIT, &init_namepass_handler);
	set_window_handler(namepass_win, ELW_HANDLER_KEYPRESS, &keypress_namepass_handler);
	init_window(namepass_win, newchar_hud_win, 0, 0, 0, hud_x, window_height - hud_y);
}

void resize_newchar_hud_window(void)
{
	if(newchar_hud_win >= 0) //Simply destroy and recreate everything
	{
		destroy_window(color_race_win);
		destroy_window(namepass_win);
		destroy_window(newchar_hud_win);
		destroy_window(tooltip_win);

		newchar_hud_win = -1;
		create_newchar_hud_window();
	}
}

void destroy_new_character_interface(void)
{
	destroy_window(color_race_win);
	destroy_window(newchar_advice_win);
	destroy_window(namepass_win);
	destroy_window(newchar_hud_win);
	destroy_window(tooltip_win);
	destroy_window(newchar_root_win);
	color_race_win = newchar_advice_win = namepass_win = newchar_hud_win = tooltip_win = newchar_root_win = -1;
}

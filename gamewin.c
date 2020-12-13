#include <stdlib.h>
#include <string.h>
#include <SDL_keycode.h>
#include "gamewin.h"
#include "actor_init.h"
#include "actor_scripts.h"
#include "achievements.h"
#include "asc.h"
#include "buddy.h"
#ifdef CLUSTER_INSIDES
#include "cluster.h"
#endif // CLUSTER_INSIDES
#include "console.h"
#include "consolewin.h"
#include "context_menu.h"
#include "cursors.h"
#include "dialogues.h"
#include "elconfig.h"
#include "emotes.h"
#include "events.h"
#include "eye_candy_wrapper.h"
#include "gl_init.h"
#include "highlight.h"
#include "hud.h"
#include "hud_quickbar_window.h"
#include "hud_quickspells_window.h"
#include "interface.h"
#include "manufacture.h"
#include "main.h"
#include "map.h"
#include "minimap.h"
#include "missiles.h"
#include "multiplayer.h"
#include "paste.h"
#include "pathfinder.h"
#ifdef PAWN
#include "pawn/elpawn.h"
#endif
#include "pm_log.h"
#include "questlog.h"
#include "reflection.h"
#include "shadows.h"
#include "special_effects.h"
#include "spells.h"
#include "sky.h"
#ifdef DEBUG
#include "filter.h"
#include "sound.h"
#endif
#include "storage.h"
#include "tabs.h"
#include "textures.h"
#include "translate.h"
#include "trade.h"
#include "url.h"
#include "weather.h"

// exported
int HUD_MARGIN_X = 64;
int HUD_MARGIN_Y = 49;
float fps_average = 100.0;
int have_mouse = 0;
int game_root_win = -1;

// configuration options exported
int use_old_clicker=0;
int include_use_cursor_on_animals = 0;
int cm_banner_disabled = 0;
int auto_disable_ranging_lock = 1;
int target_close_clicked_creature = 1;
int open_close_clicked_bag = 1;
int show_fps = 0;

#ifdef  DEBUG
extern int e3d_count, e3d_total;    // LRNR:stats testing only
#endif  //DEBUG

static Uint32 next_fps_time = 0;
static int last_count = 0;
static int keep_grabbing_mouse = 0;
static int ranging_lock = 0;
#ifdef NEW_CURSOR
static int cursors_tex;
#endif // NEW_CURSOR
static int fps_center_x = 0;
static int fps_default_width = 0;
static int action_mode = ACTION_WALK;
static int last_action_mode = ACTION_WALK;

// Set the game root window action mode
void set_gamewin_action_mode(int new_mode)
{
	action_mode = new_mode;
}

// save the current mode so it can be restored later
void save_gamewin_action_mode(void)
{
	last_action_mode = action_mode;
}

// return the last saved action mode
int retrieve_gamewin_action_mode(void)
{
	return last_action_mode;
}

// Get the game root window action mode
int get_gamewin_action_mode(void)
{
	return action_mode;
}

int get_fps_default_width(void)
{
	return fps_default_width;
}

int ranging_lock_is_on(void)
{
	return ranging_lock;
}

static void toggle_ranging_lock(void)
{
	ranging_lock = !ranging_lock;
	if (ranging_lock)
		LOG_TO_CONSOLE(c_green1, ranginglock_enabled_str);
	else
		LOG_TO_CONSOLE(c_green1, ranginglock_disabled_str);
}

void check_to_auto_disable_ranging_lock(void)
{
	if(ranging_lock && auto_disable_ranging_lock)
		toggle_ranging_lock();
}

static void toggle_target_close_clicked_creature(void)
{
	toggle_OPT_BOOL_by_name("target_close_clicked_creature");
	if (target_close_clicked_creature)
		LOG_TO_CONSOLE(c_green1, close_click_targetting_on_str);
	else
		LOG_TO_CONSOLE(c_green1, close_click_targetting_off_str);
}

void draw_special_cursors(void)
{
	const float RET_WID = 4.0f;
	const float RET_LEN = 10.0f;
	float ret_x = 0.0, ret_y = 0.0;

	float ret_spin,ret_zoom, ret_alpha=0.5f;
	float ret_color[4];
	float ret_out = 7.0;

#ifdef NEW_CURSOR
	if (!have_mouse && sdl_cursors) return;
#else // NEW_CURSOR
	if (!have_mouse) return;
#endif // NEW_CURSOR

	if(!(SDL_GetWindowFlags(el_gl_window) & SDL_WINDOW_MOUSE_FOCUS)) return;

	switch (current_cursor){
	case (CURSOR_ATTACK):
		ret_zoom = 2.0f;
		ret_spin = (cur_time%2000)*360.0f/2000.0f;
		ret_color[0]=1.0f;
		ret_color[1]=0.0f;
		ret_color[2]=0.0f;
		ret_color[3]=ret_alpha;
		break;
	case (CURSOR_WAND):
		ret_spin = 0.0f;
		ret_zoom = (sin((cur_time%1000)*3.1415/1000.0)+1.0)*6.0;
		ret_color[0]=0.0f;
		ret_color[1]=0.0f;
		ret_color[2]=1.0f;
		ret_color[3]=ret_alpha;
		ret_out=15.0f;
		break;
	default:
		ret_spin = 45.0f;
		ret_zoom = 3.0f;
		ret_color[0]=0.0f;
		ret_color[1]=1.0f;
		ret_color[2]=0.0f;
		ret_color[3]=ret_alpha;
	}

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();
	glTranslatef(mouse_x, mouse_y, 0.0);
#ifdef NEW_CURSOR
	glScalef(pointer_size, pointer_size, 1.0);
#endif // NEW_CURSOR

	//printf("mouse_x=%d mouse_y=%d\n", mouse_x, mouse_y);

	if(have_mouse)
	{
#ifdef NEW_CURSOR
		glColor4f(1,1,1,1);
		bind_texture(cursors_tex);
		if(big_cursors){
			float u_start = (32.0f/256.0f) * ((current_cursor + 8) % 8) + 0.5f / 256.0f;
			float u_end = u_start + (31.0f/256.0f);
			float v_start = (32.0f/256.0f) * ((current_cursor + 8) / 8) + 0.5f / 256.0f;
			float v_end = v_start + (31.0f/256.0f);
			glBegin(GL_QUADS);
			draw_2d_thing(u_start, v_start, u_end, v_end, 0, 0, 32, 32);
			glEnd();
		} else {
			float u_start = (16.0f/256.0f) * (current_cursor % 16) + 0.5f / 256.0f;
			float u_end = u_start + (15.0f/256.0f);
			float v_start = (16.0f/256.0f) * (current_cursor / 16) + 0.5f / 256.0f;
			float v_end = v_start + (15.0f/256.0f);
			glBegin(GL_QUADS);
			draw_2d_thing(u_start, v_start, u_end, v_end, 10, 10, 26, 26);
			glEnd();
		}
#endif // NEW_CURSOR

		glRotatef(ret_spin, 0.0, 0.0, 1.0);
		glColor4fv(ret_color);
		glDisable(GL_TEXTURE_2D);

		ret_x += ret_zoom;
		glBegin(GL_TRIANGLES);

		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x+RET_LEN,ret_y-RET_WID);
		glVertex2f(ret_x+ret_out,ret_y);

		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x+RET_LEN,ret_y+RET_WID);
		glVertex2f(ret_x+ret_out,ret_y);

		ret_x -= ret_zoom*2;

		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_LEN,ret_y-RET_WID);
		glVertex2f(ret_x-ret_out,ret_y);

		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_LEN,ret_y+RET_WID);
		glVertex2f(ret_x-ret_out,ret_y);

		ret_x += ret_zoom;
		ret_y -= ret_zoom;

		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_WID,ret_y-RET_LEN);
		glVertex2f(ret_x,ret_y-ret_out);

		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x+RET_WID,ret_y-RET_LEN);
		glVertex2f(ret_x,ret_y-ret_out);

		ret_y += ret_zoom*2;

		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_WID,ret_y+RET_LEN);
		glVertex2f(ret_x,ret_y+ret_out);

		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x+RET_WID,ret_y+RET_LEN);
		glVertex2f(ret_x,ret_y+ret_out);

		glEnd();
		ret_y -= ret_zoom;

		glColor4f(0.0,0.0,0.0,ret_alpha);

		ret_x += ret_zoom;
		glBegin(GL_LINE_LOOP);
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x+RET_LEN,ret_y-RET_WID);
		glVertex2f(ret_x+ret_out,ret_y);
		glVertex2f(ret_x+RET_LEN,ret_y+RET_WID);
		glEnd();
		ret_x -= ret_zoom*2;
		glBegin(GL_LINE_LOOP);
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_LEN,ret_y-RET_WID);
		glVertex2f(ret_x-ret_out,ret_y);
		glVertex2f(ret_x-RET_LEN,ret_y+RET_WID);
		glEnd();

		ret_x += ret_zoom;
		ret_y -= ret_zoom;
		glBegin(GL_LINE_LOOP);
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_WID,ret_y-RET_LEN);
		glVertex2f(ret_x,ret_y-ret_out);
		glVertex2f(ret_x+RET_WID,ret_y-RET_LEN);
		glEnd();

		ret_y += ret_zoom*2;
		glBegin(GL_LINE_LOOP);
		glVertex2f(ret_x,ret_y);
		glVertex2f(ret_x-RET_WID,ret_y+RET_LEN);
		glVertex2f(ret_x,ret_y+7);
		glVertex2f(ret_x+RET_WID,ret_y+RET_LEN);
		glEnd();
		ret_y -= ret_zoom;
	}
#ifdef NEW_CURSOR
	else
#endif // NEW_CURSOR
#ifdef NEW_CURSOR
	{
		if (current_cursor != CURSOR_ARROW){
			glColor4fv(ret_color);
			glRotatef(-135.0,0,0,1);
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_TRIANGLES);
			glVertex2f(ret_x,ret_y);
			glVertex2f(ret_x-RET_LEN,ret_y-RET_WID);
			glVertex2f(ret_x-ret_out,ret_y);

			glVertex2f(ret_x,ret_y);
			glVertex2f(ret_x-RET_LEN,ret_y+RET_WID);
			glVertex2f(ret_x-ret_out,ret_y);
			glEnd();

			glColor4f(0.0,0.0,0.0,ret_alpha);

			glBegin(GL_LINE_LOOP);
			glVertex2f(ret_x,ret_y);
			glVertex2f(ret_x-RET_LEN,ret_y-RET_WID);
			glVertex2f(ret_x-ret_out,ret_y);
			glVertex2f(ret_x-RET_LEN,ret_y+RET_WID);
			glEnd();
			glRotatef(135.0,0,0,1);
			glEnable(GL_TEXTURE_2D);
		}

		glColor4f(1,1,1,1);
		bind_texture(cursors_tex);
		if(big_cursors){
			float u_start = (32.0f/256.0f) * ((current_cursor + 8) % 8) + 0.5f / 256.0f;
			float u_end = u_start + (31.0f/256.0f);
			float v_start = (32.0f/256.0f) * ((current_cursor + 8) / 8) + 0.5f / 256.0f;
			float v_end = v_start + (31.0f/256.0f);
			glBegin(GL_QUADS);
			draw_2d_thing(u_start, v_start, u_end, v_end, 0, 0, 32, 32);
			glEnd();
		} else {
			float u_start = (16.0f/256.0f) * (current_cursor % 16) + 0.5f / 256.0f;
			float u_end = u_start + (15.0f/256.0f);
			float v_start = (16.0f/256.0f) * (current_cursor / 16) + 0.5f / 256.0f;
			float v_end = v_start + (15.0f/256.0f);
			glBegin(GL_QUADS);
			draw_2d_thing(u_start, v_start, u_end, v_end, 0, 0, 16, 16);
			glEnd();
		}
	}
#endif // NEW_CURSOR

	glPopMatrix();
	glPopAttrib();
	reset_material();
}

void toggle_have_mouse(void)
{
	have_mouse = !have_mouse;
	if(have_mouse){
		SDL_SetWindowGrab(el_gl_window, SDL_TRUE);
#ifdef NEW_CURSOR
		if (sdl_cursors)
#endif // NEW_CURSOR
			SDL_ShowCursor(0);
		if (fol_cam) toggle_follow_cam(&fol_cam);
		LOG_TO_CONSOLE (c_red1, "Grab mode: press alt+g again to enter Normal mode.");
	} else {
		SDL_SetWindowGrab(el_gl_window, SDL_FALSE);
#ifdef NEW_CURSOR
		if (sdl_cursors)
#endif // NEW_CURSOR
			SDL_ShowCursor(1);
		LOG_TO_CONSOLE (c_red1, "Normal mode: press alt+g again to enter Grab mode.");
	}
}

static void toggle_first_person()
{
	if (first_person == 0){
		//rotate camera where actor is looking at
		actor *me = get_our_actor();
		if (me)
			rz=me->z_rot;
		rx=-90;
		first_person = 1;
		fol_cam = 0;
	} else {
		first_person = 0;
		if (rx < -90) {rx = -90;}
	}
	++adjust_view;
	resize_root_window();
	//set_all_intersect_update_needed(main_bbox_tree);
}

// This is the main part of the old check_cursor_change ()
static int mouseover_game_handler (window_info *win, int mx, int my)
{
	SDL_Keymod mod_key_status = SDL_GetModState();

	if (hud_mouse_over(win, mx, my))
		return 1;

	if(object_under_mouse == -1)
	{
		if(spell_result==2){
			elwin_mouse = CURSOR_WAND;
		} else {
			elwin_mouse = CURSOR_WALK;
		}
	}

	else if (thing_under_the_mouse==UNDER_MOUSE_3D_OBJ && objects_list[object_under_mouse])
	{
		int range_weapon_equipped;
		LOCK_ACTORS_LISTS();
		range_weapon_equipped = (your_actor &&
								 your_actor->cur_weapon >= BOW_LONG &&
								 your_actor->cur_weapon <= BOW_CROSS);
		UNLOCK_ACTORS_LISTS();
		if(action_mode==ACTION_LOOK)
		{
			elwin_mouse = CURSOR_EYE;
		}
		else if(objects_list[object_under_mouse]->flags&OBJ_3D_BAG)
		{
			elwin_mouse = CURSOR_PICK;
		}
		else if(action_mode==ACTION_USE)
		{
			elwin_mouse = CURSOR_USE;
		}
		else if(action_mode==ACTION_USE_WITEM)
		{
			elwin_mouse = CURSOR_USE_WITEM;
		}
		// allow to shoot at 3D objects
		else if (range_weapon_equipped &&
                 (action_mode == ACTION_ATTACK || ((mod_key_status & KMOD_ALT) && (mod_key_status & KMOD_CTRL))))
		{
			elwin_mouse = CURSOR_ATTACK;
		}
		//see if the object is a harvestable resource.
		else if(objects_list[object_under_mouse]->flags&OBJ_3D_HARVESTABLE)
		{
			elwin_mouse = CURSOR_HARVEST;
		}
		//see if the object is an entrable resource.
		else if(objects_list[object_under_mouse]->flags&OBJ_3D_ENTRABLE) {
			elwin_mouse = CURSOR_ENTER;
		}
		//hmm, no usefull object, so select walk....
		else
		{
			if(spell_result==2)
				elwin_mouse = CURSOR_WAND;
			else
				elwin_mouse = CURSOR_WALK;
		}
	}

	else if(thing_under_the_mouse==UNDER_MOUSE_NPC)
	{
		if(action_mode==ACTION_LOOK)
		{
			elwin_mouse = CURSOR_EYE;
		}
		else
		{
			elwin_mouse = CURSOR_TALK;
		}
	}

	else if(thing_under_the_mouse==UNDER_MOUSE_PLAYER)
	{
		if(action_mode==ACTION_USE)
		{
			elwin_mouse = CURSOR_USE;
		}
		else if(action_mode==ACTION_LOOK)
		{
			elwin_mouse = CURSOR_EYE;
		}
		else if(action_mode==ACTION_TRADE)
		{
			elwin_mouse = CURSOR_TRADE;
		}
		else if((mod_key_status & KMOD_ALT) || action_mode==ACTION_ATTACK)
		{
			elwin_mouse = CURSOR_ATTACK;
		}
		else if (spell_result==3 && action_mode==ACTION_WAND)
		{
			elwin_mouse = CURSOR_WAND;
		}
		else
		{
			elwin_mouse = CURSOR_EYE;
		}
	}

	else if(thing_under_the_mouse==UNDER_MOUSE_ANIMAL)
	{
		if(action_mode==ACTION_USE)
		{
			elwin_mouse = CURSOR_USE;
		}
		else if(action_mode==ACTION_LOOK)
		{
			elwin_mouse = CURSOR_EYE;
		}
		else if(mod_key_status & KMOD_SHIFT)
		{
			elwin_mouse = CURSOR_EYE;
		}
		else if(spell_result==3 && action_mode == ACTION_WAND)
		{
			elwin_mouse = CURSOR_WAND;
		}
		else if((mod_key_status & KMOD_ALT) || action_mode==ACTION_ATTACK || (actor_under_mouse && !actor_under_mouse->dead))
		{
			elwin_mouse = CURSOR_ATTACK;
		}
	}

	// when all fails - walk
	else
	{
		if(spell_result==2) {
			elwin_mouse = CURSOR_WAND;
		} else {
			elwin_mouse = CURSOR_WALK;
		}
	}

	return 1;
}

static void attack_someone(int who_to_attack)
{
	Uint8 str[10];
	str[0] = ATTACK_SOMEONE;
	*((int *)(str+1)) = SDL_SwapLE32(who_to_attack);
	my_tcp_send (my_socket, str, 5);
}

static void touch_player(int player_to_touch)
{
	Uint8 str[10];
	str[0] = TOUCH_PLAYER;
	*((int *)(str+1)) = SDL_SwapLE32(player_to_touch);
	my_tcp_send (my_socket, str, 5);
}

// this is the main part of the old check_mouse_click ()
static int click_game_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int flag_alt = flags & KMOD_ALT;
	int flag_ctrl = flags & KMOD_CTRL;
	int flag_right = flags & ELW_RIGHT_MOUSE;
	int force_walk = (flag_ctrl && flag_right && !flag_alt);
	int shift_on = flags & KMOD_SHIFT;
	int range_weapon_equipped;

	if ((flags & ELW_MOUSE_BUTTON_WHEEL) == ELW_MID_MOUSE)
		// Don't handle middle button clicks
		return 0;

	if (flags & ELW_WHEEL_UP)
	{
		camera_zoom_speed = (flag_ctrl) ?10 :1;
		if (camera_zoom_dir == -1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = -1;
		return 1;
	}

	if (flags & ELW_WHEEL_DOWN)
	{
		camera_zoom_speed = (flag_ctrl) ?10 :1;
		if (camera_zoom_dir == 1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = 1;
		return 1;
	}

	if (hud_click(win, mx, my, flags))
		return 1;

	LOCK_ACTORS_LISTS();
	range_weapon_equipped = (your_actor &&
							 your_actor->cur_weapon >= BOW_LONG &&
							 your_actor->cur_weapon <= BOW_CROSS);
	UNLOCK_ACTORS_LISTS();

	if (!force_walk)
	{
		if (flag_right)
		{
			if (!cm_banner_disabled)
			{
				/* show the banner control menu if right-clicked and over your actors banner */
				static Uint32 reset_cursor_time = 0;
				static int cm_activate_when_cursor_is = -1;
				extern int cm_mouse_over_banner;
				/* activate the menu once in the cursor cycle - start-cursor reset after a couple of seconds inactivity */
				if (SDL_GetTicks()-reset_cursor_time > 2000)
					cm_activate_when_cursor_is = current_cursor;
				if (cm_mouse_over_banner && (current_cursor == cm_activate_when_cursor_is))
				{
					static size_t cm_id = CM_INIT_VALUE;
					if (!cm_valid(cm_id))
					{
						/* create first time needed */
						cm_id = cm_create(cm_banner_menu_str, NULL);
						cm_bool_line(cm_id, 0, &view_names, NULL);
						cm_bool_line(cm_id, 1, &view_health_bar, NULL);
						cm_bool_line(cm_id, 2, &view_hp, NULL);
						cm_bool_line(cm_id, 3, &view_ether_bar, NULL);
						cm_bool_line(cm_id, 4, &view_ether, NULL);
						cm_bool_line(cm_id, 5, &view_mode_instance, "use_view_mode_instance");
						cm_bool_line(cm_id, 6, &view_chat_text_as_overtext, NULL);
						cm_bool_line(cm_id, 7, &use_alpha_banner, "use_alpha_banner");
						cm_bool_line(cm_id, 8, &sit_lock, "sit_lock");
						cm_bool_line(cm_id, 9, &ranging_lock, NULL);
						cm_bool_line(cm_id, 11, &cm_banner_disabled, "cm_banner_disabled");
					}
					cm_show_direct(cm_id, -1, -1);
					reset_cursor_time = SDL_GetTicks();
				}
			}
			if (item_dragged != -1 || use_item != -1 || object_under_mouse == -1
					|| storage_item_dragged != -1
					)
			{
				use_item = -1;
				item_dragged = -1;
				storage_item_dragged = -1;
				action_mode = ACTION_WALK;
				return 1;
			}
			switch (current_cursor)
			{
				case CURSOR_EYE:
					if (thing_under_the_mouse == UNDER_MOUSE_ANIMAL && spell_result==3)
						action_mode = ACTION_WAND;
					else if (thing_under_the_mouse == UNDER_MOUSE_PLAYER)
						action_mode = ACTION_TRADE;
					else if (thing_under_the_mouse == UNDER_MOUSE_3D_OBJ){
						action_mode = ACTION_USE;
					}
					else if ((thing_under_the_mouse == UNDER_MOUSE_ANIMAL) && include_use_cursor_on_animals)
						action_mode = ACTION_USE;
					else
						action_mode = ACTION_WALK;
					break;
				case CURSOR_HARVEST:
					action_mode = ACTION_LOOK;
					break;
				case CURSOR_TRADE:
					if(spell_result==3)action_mode=ACTION_WAND;
					else action_mode = ACTION_ATTACK;
					break;
				case CURSOR_USE_WITEM:
					if(use_item != -1)
						use_item = -1;
					else
						action_mode = ACTION_WALK;
					break;
				case CURSOR_WAND:
					if(thing_under_the_mouse == UNDER_MOUSE_ANIMAL||thing_under_the_mouse == UNDER_MOUSE_PLAYER)
						action_mode = ACTION_ATTACK;
					else if(thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
						action_mode = ACTION_LOOK;
					break;
				case CURSOR_ATTACK:
					if(thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
						action_mode = ACTION_LOOK;
					else
						action_mode = ACTION_WALK;
					break;
				case CURSOR_ENTER:
				case CURSOR_PICK:
				case CURSOR_WALK:
					if(thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
						action_mode = ACTION_LOOK;
					else
						action_mode = ACTION_WALK;
					break;
				case CURSOR_USE:
					if (range_weapon_equipped)
						action_mode = ACTION_ATTACK;
					else
						action_mode = ACTION_WALK;
					break;
				case CURSOR_TALK:
				case CURSOR_ARROW:
				default:
					action_mode = ACTION_WALK;
					break;
			}
			return 1;
		}
	}

	// after we test for interface clicks
	// alternative drop method...
	if (item_dragged >= 0 && item_dragged < ITEM_NUM_ITEMS)
	{
		Uint8 str[10];

		if (flag_right)
		{
			item_dragged = -1;
			return 1;
		}

		str[0] = DROP_ITEM;
		str[1] = item_list[item_dragged].pos;
		*((Uint32 *) (str + 2)) = SDL_SwapLE32(item_quantity);
		my_tcp_send(my_socket, str, 6);
		return 1;
	}

	if (storage_item_dragged != -1)
	{
		//TODO: Withdraw from storage, drop on ground...
	}

	// if we're following a path, stop now if the click was in the main window
	if (pf_follow_path && !((mx >= window_width-hud_x) || (my >= window_height-hud_y)))
	{
		pf_destroy_path();
	}

	if (force_walk)
	{
		short x,y;

		get_old_world_x_y (&x, &y);
		// check to see if the coordinates are OUTSIDE the map
		if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
			return 1;

		add_highlight(x, y, HIGHLIGHT_TYPE_WALKING_DESTINATION);

		move_to (x, y, 1);
		return 1;
	}

	switch(current_cursor)
	{
		case CURSOR_EYE:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;

			if (thing_under_the_mouse == UNDER_MOUSE_PLAYER || thing_under_the_mouse == UNDER_MOUSE_NPC || thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
			{
#ifdef DEBUG
				char log[100];

				safe_snprintf(log,sizeof(log),"Actor id: %d",object_under_mouse);
				LOG_TO_CONSOLE(c_green1, log);
#endif
				if (thing_under_the_mouse == UNDER_MOUSE_PLAYER)
					achievements_requested(mouse_x, mouse_y, flag_ctrl);
				str[0] = GET_PLAYER_INFO;
				*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
				my_tcp_send (my_socket, str, 5);
				return 1;
			}
			else if (thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
			{
#ifdef DEBUG
				char log[100];

				safe_snprintf(log,sizeof(log),"Object id: %d",object_under_mouse);
				LOG_TO_CONSOLE(c_green1, log);
#endif
				str[0] = LOOK_AT_MAP_OBJECT;
				*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
				my_tcp_send (my_socket, str, 5);
				return 1;
			}

			break;
		}

		case CURSOR_WAND:
		{
			if (spell_result==2)
			{
				short x, y;

				if(use_old_clicker)
					get_old_world_x_y (&x, &y);
				else
					get_world_x_y (&x, &y);

				// check to see if the coordinates are OUTSIDE the map
				if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
					return 1;

				move_to(x,y,0);
				return 1;
			}
			else if (spell_result==3)
			{
				if (object_under_mouse >= 0 &&
					(thing_under_the_mouse == UNDER_MOUSE_ANIMAL ||
					 thing_under_the_mouse == UNDER_MOUSE_PLAYER))
				{
					actor *this_actor = get_actor_ptr_from_id(object_under_mouse);
					if(this_actor != NULL)
					{
						add_highlight(this_actor->x_tile_pos,this_actor->y_tile_pos, HIGHLIGHT_TYPE_SPELL_TARGET);
						touch_player((int)object_under_mouse);
					}
				}
			}

			break;
		}

		case CURSOR_TRADE:
		{
			Uint8 str[10];

			if (object_under_mouse == -1)
				return 1;
			if (thing_under_the_mouse != UNDER_MOUSE_PLAYER)
				return 1;
			str[0] = TRADE_WITH;
			*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
			my_tcp_send (my_socket, str, 5);
			return 1;

			break;
		}

		case CURSOR_ATTACK:
		{
			if (object_under_mouse == -1)
				return 1;
			if (you_sit && sit_lock && !flag_ctrl){
				if(your_actor != NULL)
					add_highlight(your_actor->x_tile_pos,your_actor->y_tile_pos, HIGHLIGHT_TYPE_LOCK);
				return 1;
			}
			if (thing_under_the_mouse == UNDER_MOUSE_PLAYER || thing_under_the_mouse == UNDER_MOUSE_NPC || thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
			{
				if (object_under_mouse>=0){
					actor *this_actor = get_actor_ptr_from_id(object_under_mouse);
					if(this_actor != NULL)
						add_highlight(this_actor->x_tile_pos,this_actor->y_tile_pos, HIGHLIGHT_TYPE_ATTACK_TARGET);
				}
				attack_someone((int)object_under_mouse);
				return 1;
			}
			else if (range_weapon_equipped && thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
			{
				Uint8 str[10];
				str[0] = FIRE_MISSILE_AT_OBJECT;
				*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
				my_tcp_send(my_socket, str, 5);
			}
			break;
		}

		case CURSOR_ENTER:
		case CURSOR_USE:
		case CURSOR_USE_WITEM:
		case CURSOR_TALK:
		{
			Uint8 str[10];

			if (flag_alt && range_weapon_equipped)
				return 1;
			if (object_under_mouse == -1)
				return 1;
			if (thing_under_the_mouse == UNDER_MOUSE_PLAYER || thing_under_the_mouse == UNDER_MOUSE_NPC || thing_under_the_mouse == UNDER_MOUSE_ANIMAL)
			{
				touch_player((int)object_under_mouse);
				// clear the previous dialogue entries, so we won't have a left over from some other NPC
				if (thing_under_the_mouse == UNDER_MOUSE_NPC)
					clear_dialogue_responses();
				return 1;
			}

			str[0] = USE_MAP_OBJECT;
			*((int *)(str+1)) = SDL_SwapLE32((int)object_under_mouse);
			if (use_item != -1 && current_cursor == CURSOR_USE_WITEM)
			{
				used_item_counter_action_use(use_item);
				*((int *)(str+5)) = SDL_SwapLE32((int)item_list[use_item].pos);
				if (!shift_on)
				{
					use_item = -1;
					action_mode = ACTION_WALK;
				}
			}
			else
			{
				*((int *)(str+5)) = SDL_SwapLE32((int)-1);
			}

			my_tcp_send (my_socket, str, 9);
			return 1;

			break;
		}

		case CURSOR_PICK:
		{
			if (flag_alt && range_weapon_equipped)
				return 1;
			if (object_under_mouse == -1)
				return 1;
			if (thing_under_the_mouse == UNDER_MOUSE_3D_OBJ)
			{
				open_bag (object_under_mouse);
				return 1;
			}
			break;
		}

		case CURSOR_HARVEST:
		{
			Uint8 str[10];

			if (flag_alt && range_weapon_equipped)
				return 1;
			if (object_under_mouse == -1)
				return 1;
			str[0] = HARVEST;
			*((Uint16 *)(str+1)) = SDL_SwapLE16((Uint16)object_under_mouse);
			my_tcp_send (my_socket, str, 3);
			return 1;
			break;
		}

		case CURSOR_WALK:
		default:
		{
			int is_ranging_locked = 0;
			int is_sit_locked = 0;
			short x, y;

			/* if outside the main window, on the hud, don't walk */
			if ((mx >= window_width-hud_x) || (my >= window_height-hud_y))
				return 1;

			if (flag_alt && range_weapon_equipped)
				return 1;

			if (ranging_lock_is_on() && range_weapon_equipped)
				is_ranging_locked = 1;

			if (you_sit && sit_lock && !flag_ctrl)
				is_sit_locked = 1;

			if(use_old_clicker)
				get_old_world_x_y (&x, &y);
			else
				get_world_x_y (&x, &y);

			// check to see if the coordinates are OUTSIDE the map
			if (y < 0 || x < 0 || x >= tile_map_size_x*6 || y >= tile_map_size_y*6)
				return 1;

			if (target_close_clicked_creature)
			{
				int closest_actor = get_closest_actor(x, y, 0.8f);
				if (closest_actor != -1)
				{
					actor *this_actor = get_actor_ptr_from_id(closest_actor);
					if (this_actor != NULL)
					{
						if (spell_result == 3)
						{
							add_highlight(this_actor->x_tile_pos,this_actor->y_tile_pos, HIGHLIGHT_TYPE_SPELL_TARGET);
							touch_player(closest_actor);
							return 1;
						}
						else if (!is_ranging_locked && !is_sit_locked)
						{
							add_highlight(this_actor->x_tile_pos, this_actor->y_tile_pos, HIGHLIGHT_TYPE_ATTACK_TARGET);
							attack_someone(closest_actor);
							return 1;
						}
					}
				}
			}

			if (is_ranging_locked || is_sit_locked)
			{
				if(your_actor != NULL)
					add_highlight(your_actor->x_tile_pos,your_actor->y_tile_pos, HIGHLIGHT_TYPE_LOCK);
				return 1;
			}

			if (open_close_clicked_bag && find_and_open_closest_bag(x, y, 0.8f))
				return 1;

			add_highlight(x, y, HIGHLIGHT_TYPE_WALKING_DESTINATION);

#ifdef DEBUG // FOR DEBUG ONLY!
			if (enable_client_aiming) {
				if (flag_ctrl) {
					float target[3];

					target[0] = x * 0.5 + 0.25;
					target[1] = y * 0.5 + 0.25;
					target[2] = get_tile_height(x, y) + 1.2f;

					missiles_aim_at_xyz(yourself, target);
					add_command_to_actor(yourself, aim_mode_reload);
					missiles_fire_a_to_xyz(yourself, target);
				}
				else {
					char in_aim_mode;
					actor *cur_actor = get_actor_ptr_from_id(yourself);
					LOCK_ACTORS_LISTS();
					in_aim_mode = cur_actor->in_aim_mode;
					UNLOCK_ACTORS_LISTS();
					if (in_aim_mode == 1)
						add_command_to_actor(yourself, leave_aim_mode);
					move_to(x, y, 1);
				}
			}
			else
				move_to (x, y, 1);
#else
			move_to (x, y, 1);
#endif // DEBUG

			return 1;
		}
	}

	left_click = 2;
	right_click = 2;
	return 1;
}

// common to console and map windows
void display_handling_common(window_info *win)
{
	if(special_effects){
		display_special_effects(0);
	}

	// remember the time stamp to improve FPS quality when switching modes
	next_fps_time=cur_time+1000;
	last_count=0;

	ec_idle();

	missiles_update();
	update_camera();

	draw_delay = 20;

	if ((input_widget!= NULL) && (input_widget->window_id != win->window_id))
		input_widget_move_to_win(win->window_id);
}


// common to console and map windows
void return_to_gamewin_common(void)
{
	if (keep_grabbing_mouse)
	{
		toggle_have_mouse();
		keep_grabbing_mouse=0;
	}
	hide_window_MW(MW_TABMAP);
	hide_window_MW(MW_CONSOLE);
	show_window (game_root_win);
	show_hud_windows();
}

static void draw_ingame_interface(window_info *win)
{
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	draw_hud_interface(win);
	display_spells_we_have();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


static int display_game_handler (window_info *win)
{
	static int main_count = 0;
	static int times_FPS_below_3 = 0;
	static int fps[5]={100};
	static int shadows_were_disabled=0;
	static int eye_candy_was_disabled=0;
	unsigned char str[180];
	int i;
	int any_reflection = 0;
	int mouse_rate;

	if (!have_a_map) return 1;
	if (yourself==-1) return 1; //we don't have ourselves

	for(i=0; i<max_actors; i++)
	{
        	if(actors_list[i] && actors_list[i]->actor_id == yourself)
			break;
	}
	if(i > max_actors) return 1;//we still don't have ourselves

#ifdef CLUSTER_INSIDES
	current_cluster = get_actor_cluster();
#endif // CLUSTER_INSIDES

	main_count++;
	last_count++;

	if (fps[0] < 5)
	{
		mouse_rate = 1;
		read_mouse_now = 1;
	}
	else if (fps[0] < 10)
	{
		mouse_rate = 3;
	}
	else if (fps[0] < 20)
	{
		mouse_rate = 6;
	}
	else if (fps[0] < 30)
	{
		mouse_rate = 10;
	}
	else if (fps[0] < 40)
	{
		mouse_rate = 15;
	}
	else
	{
		mouse_rate = 20;
	}
	if(mouse_rate > mouse_limit) {
		mouse_rate = mouse_limit;
	}
	if (mouse_rate < 5)
	{
		mouse_rate = 5;
	}
	if ((main_count % mouse_rate) == 0)
	{
		read_mouse_now = 1;
	}
	else
	{
		read_mouse_now = 0;
	}

	// This window is a bit special since it's not fully 2D
	Leave2DMode ();
	glPushMatrix ();

    update_camera();

	if (new_zoom_level != zoom_level)
	{
		if (new_zoom_level > zoom_level)
			set_all_intersect_update_needed(main_bbox_tree);
		zoom_level = new_zoom_level;
		resize_root_window ();
	}

	move_camera ();
	save_scene_matrix ();

	CalculateFrustum ();
	set_click_line();
	any_reflection = find_reflection ();
	CHECK_GL_ERRORS ();

	reset_under_the_mouse();

	if (!dungeon){
		draw_global_light ();
	} else {
		draw_dungeon_light ();
	}

	if (skybox_update_delay < 1)
		skybox_update_colors();
	if (skybox_show_sky)
	{
		skybox_compute_z_position();
		glPushMatrix();
		glTranslatef(0.0, 0.0, skybox_get_z_position());
		skybox_display();
		glPopMatrix();
	}

	if (use_fog)
		weather_render_fog();

	// only draw scene lights if inside or it is night
	if (dungeon || !is_day)
	{
		update_scene_lights ();
		draw_lights ();
	}
	CHECK_GL_ERRORS ();

	if (!dungeon && shadows_on && (is_day || lightning_falling))
	{
		render_light_view();
		CHECK_GL_ERRORS ();
	}

	if (any_reflection > 1) // there are water tiles to display
	{
		draw_water_background();
		CHECK_GL_ERRORS ();
		if (show_reflection) display_3d_reflection ();
	}
	CHECK_GL_ERRORS ();
	glClear(GL_DEPTH_BUFFER_BIT);

	missiles_update();

	if (!is_day)
		weather_init_lightning_light();

	if (!dungeon && shadows_on && (is_day || lightning_falling))
	{
		glNormal3f(0.0f,0.0f,1.0f);
		if (use_fog && any_reflection) blend_reflection_fog();
		draw_sun_shadowed_scene (any_reflection);
	}
	else
	{
		glNormal3f (0.0f,0.0f,1.0f);
		if (any_reflection) {
			blend_reflection_fog();
			draw_lake_tiles ();
		}

		setup_cloud_texturing();
		draw_tile_map();
		CHECK_GL_ERRORS ();
		display_2d_objects();
		CHECK_GL_ERRORS();
		anything_under_the_mouse(0, UNDER_MOUSE_NOTHING);
		display_objects();
		display_ground_objects();
		display_actors(1, DEFAULT_RENDER_PASS);
		display_alpha_objects();
		display_blended_objects();
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(skybox_view);
	glMatrixMode(GL_MODELVIEW);

	weather_render_lightning();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	CHECK_GL_ERRORS ();

#ifdef DEBUG_TIME
	light_idle();
#endif // DEBUG_TIME

	ec_idle();

	CHECK_GL_ERRORS();

	if (show_weather){
		weather_render();
	}

	CHECK_GL_ERRORS ();
	//particles should be last, we have no Z writting
	display_particles ();

	CHECK_GL_ERRORS ();
	//we do this because we don't want the rain/particles to mess with our cursor

	ec_draw();
	CHECK_GL_ERRORS();

	missiles_draw();
	CHECK_GL_ERRORS();


	last_texture = -1;

	CHECK_GL_ERRORS ();

	animate_map_markers();
	display_map_markers();
	display_map_marks();

	Enter2DMode ();
	//get the FPS, etc

	if (next_fps_time<cur_time){
		fps[4]=fps[3];
		fps[3]=fps[2];
		fps[2]=fps[1];
		fps[1]=fps[0];
		fps[0]=last_count*1000/(cur_time-next_fps_time+1000);
		last_count=0;
		next_fps_time=cur_time+1000;
		fps_average=(fps[0]+fps[1]+fps[2]+fps[3]+fps[4])/5.0f;
	}

	if (!no_adjust_shadows)
	{
		if ((fps_average < 5.0f) || (max_fps != limit_fps))
		{
			times_FPS_below_3++;
			if (((times_FPS_below_3 > 10) || (max_fps != limit_fps)) && (shadows_on || use_eye_candy ))
			{
				if (max_fps == limit_fps)
					LOG_TO_CONSOLE(c_red1, (unsigned char*)low_framerate_str);
				times_FPS_below_3 = 0;
				if (shadows_on)
				{
					shadows_on = 0;
					shadows_were_disabled=1;
				}
				if (use_eye_candy)
				{
					use_eye_candy = 0;
					eye_candy_was_disabled = 1;
				}
			}
		}
		else
		{
			times_FPS_below_3 = 0;

			if(shadows_were_disabled){
				shadows_on = 1;
				shadows_were_disabled=0;
			}

			if (eye_candy_was_disabled){
				use_eye_candy = 1;
				eye_candy_was_disabled = 0;
			}
		}
	}
	if (show_fps)
	{
		int fps_y;
#ifdef	DEBUG
		int y_cnt = 10;
		actor *me = get_our_actor ();

		glColor3f (1.0f, 1.0f, 1.0f);
		if(me){
 			safe_snprintf((char*)str,sizeof(str),"Busy: %i",me->busy);
	 		draw_string_zoomed (0, win->len_y - hud_y - (y_cnt--) * win->default_font_len_y, str, 1, win->current_scale);
			safe_snprintf((char*)str,sizeof(str),"Command: %i",me->last_command);
 			draw_string_zoomed (0, win->len_y - hud_y - (y_cnt--) * win->default_font_len_y, str, 1, win->current_scale);
			safe_snprintf((char*)str,sizeof(str),"Coords: %-3i %3i",me->x_tile_pos, me->y_tile_pos);
 			draw_string_zoomed (0, win->len_y - hud_y - (y_cnt--) * win->default_font_len_y, str, 1, win->current_scale);
			safe_snprintf((char*)str,sizeof(str),"Coords: %.3g %.3g",me->x_pos, me->y_pos);
 			draw_string_zoomed (0, win->len_y - hud_y - (y_cnt--) * win->default_font_len_y, str, 1, win->current_scale);
		}

		safe_snprintf((char*)str, sizeof(str), "lights: ambient=(%.2f,%.2f,%.2f,%.2f) diffuse=(%.2f,%.2f,%.2f,%.2f)",
					  ambient_light[0], ambient_light[1], ambient_light[2], ambient_light[3],
					  diffuse_light[0], diffuse_light[1], diffuse_light[2], diffuse_light[3]);
		draw_string_zoomed (0, win->len_y - hud_y - (y_cnt--) * win->default_font_len_y, str, 1, win->current_scale);
		safe_snprintf((char*)str, sizeof(str), "weather: drops=%d/%d/%d/%d/%d/%d, int=%f, dty=%f, fog=%f",
					  weather_get_drops_count(1),
					  weather_get_drops_count(2),
					  weather_get_drops_count(3),
					  weather_get_drops_count(4),
					  weather_get_drops_count(5),
					  weather_get_drops_count(6),
					  weather_get_intensity(),
					  weather_get_density(),
					  skybox_fog_density);
		draw_string_zoomed (0, win->len_y - hud_y - (y_cnt--) * win->default_font_len_y, str, 1, win->current_scale);
		//LRNR: stats testing
		safe_snprintf ((char*)str, sizeof(str),"Lights: %i", show_lights);
		draw_string_zoomed (0, win->len_y - hud_y - (y_cnt--) * win->default_font_len_y, str, 1, win->current_scale);
		safe_snprintf((char*)str, sizeof(str), "E3D:%3d TOT:%3d", e3d_count, e3d_total);
		draw_string_zoomed (0, win->len_y - hud_y - (y_cnt--) * win->default_font_len_y, str, 1, win->current_scale);
		e3d_count= e3d_total= 0;
#else	//DEBUG
		glColor3f (1.0f, 1.0f, 1.0f);
#endif	//DEBUG

		if (fps_default_width == 0)
		{
			fps_default_width = get_string_width_zoom((const unsigned char*)"FPS: ",
					win->font_category, win->current_scale)
				+ 3 * get_max_digit_width_zoom(UI_FONT, win->current_scale)
				+ win->default_font_max_len_x;
			fps_center_x = win->len_x - hud_x - win->default_font_max_len_x -
				3 * get_max_digit_width_zoom(UI_FONT, win->current_scale);
		}

		fps_y = 4 * win->current_scale;
		if (max_fps != limit_fps)
			safe_snprintf ((char*)str, sizeof(str), "FPS: -");
		else
			safe_snprintf ((char*)str, sizeof(str), "FPS: %i", fps[0]);
		draw_string_zoomed_centered_around(fps_center_x, fps_y, str, 4, win->current_scale);
		safe_snprintf((char*)str, sizeof(str), "UVP: %d", use_animation_program);
		draw_string_zoomed_centered_around(fps_center_x, fps_y + win->default_font_len_y,
			str, 4, win->current_scale);
	}
	else
		fps_default_width = 0;
	draw_spell_icon_strings(win);

	CHECK_GL_ERRORS ();
	/* Draw the chat text */
	if (is_chat_shown() && (use_windowed_chat != 2))
	{
		int msg, offset, filter;
		filter = use_windowed_chat == 1 ? current_filter : FILTER_ALL;
		if (find_last_lines_time(&msg, &offset, filter, get_console_text_width()))
		{
			draw_messages(get_tab_bar_x(), get_tab_bar_y(), display_text_buffer,
				DISPLAY_TEXT_BUFFER_SIZE, filter, msg, offset, -1,
				get_console_text_width(), 1 + get_text_height(get_lines_to_show(), CHAT_FONT, 1.0),
				CHAT_FONT, 1.0, NULL);
		}
	}

	anything_under_the_mouse (0, UNDER_MOUSE_NO_CHANGE);
	CHECK_GL_ERRORS ();

	draw_ingame_interface (win);

	CHECK_GL_ERRORS ();

	Leave2DMode ();

	if(special_effects){
		display_special_effects(1);
	}
	display_highlight_markers();

	glEnable (GL_LIGHTING);

	// Return to 2D mode to draw the other windows
	glPopMatrix ();	// restore the state
	Enter2DMode ();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	if ((input_widget!= NULL) && (input_widget->window_id != win->window_id) && !get_show_window(get_id_MW(MW_CHAT)))
		input_widget_move_to_win(win->window_id);

	return 1;
}

int check_quit_or_fullscreen (SDL_Keycode key_code, Uint16 key_mod)
{
	// first, try to see if we pressed Alt+x or Ctrl+q, to quit.
	if (KEY_DEF_CMP(K_QUIT, key_code, key_mod) || KEY_DEF_CMP(K_QUIT_ALT, key_code, key_mod))
	{
		exit_now = 1;
	}
	else if ((key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) && key_mod & KMOD_ALT)
	{
		toggle_full_screen ();
	}
	else
	{
		return 0;
	}

	return 1;
}

Uint8 key_to_char (Uint32 unikey)
{
	return (Uint8)unikey;
}

int string_input(char *text, size_t maxlen, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	size_t len = strlen(text);
#ifndef OSX
	if (key_code == SDLK_BACKSPACE)
#else
	if ((key_code == SDLK_BACKSPACE) || (key_unicode == 127))
#endif
	{
		if (len > 0)
			text[len-1] = '\0';
		return 1;
	}
	if (is_printable (key_unicode))
	{
		if (len < (maxlen-1))
		{
			text[len] = key_unicode;
			text[len+1] = '\0';
		}
		return 1;
	}
	return 0;
}

void hide_all_windows()
{
	/* Note: We don't watch for if a window is otherwise closed; alt+d to reopen only cares about the last
	 * time it hid windows itself. If you alt+d to reopen windows, manually close them all, and alt+d
	 * again, it'll reopen the same ones.
	 */
	int any_open = 0;
	enum managed_window_enum i;

	// control the minimap only if it is not pinned
	if (pin_minimap)
		clear_hideable_MW(MW_MINIMAP);
	else
		set_hideable_MW(MW_MINIMAP);

	// check if any of the windows are shown, we will hide them all if so
	for (i = 0; i < MW_MAX; i++)
	{
		if (!is_hideable_MW(i))
			continue;
		if (get_show_window_MW(i) > 0)
		{
			any_open = 1;
			break;
		}
	}

	for (i = 0; i < MW_MAX; i++)
	{
		if (!is_hideable_MW(i))
			continue;
		// at least one is shown so we hide them all
		if (any_open)
		{
			// mark any that are open, so we re-open them next time
			if (get_window_showable(get_id_MW(i)) > 0)
			{
				set_was_open_MW(i); // do first so overrideable in handler
				hide_window_MW(i);
				if (i == MW_BAGS)  // too many edge case to reopen a bag so close it fully
					client_close_bag();
			}
			else
				clear_was_open_MW(i);
		}
		// non were shown so we re-open any hidden last time
		else
		{
			if (was_open_MW(i))
				show_window_MW(i);
		}
	}
}


static void toggle_sit_stand()
{
	Uint8 str[4];
	//Send message to server...
	str[0]=SIT_DOWN;
	str[1]=!you_sit;
	my_tcp_send(my_socket,str,2);
}

void switch_action_mode(int mode)
{
	action_mode = mode;
	set_quickbar_action_mode(mode);
	set_items_action_mode(mode);
}


// keypress handler common to all in-game root windows (game_root_win,
// console_root_win, and map_root_win)
int keypress_root_common (SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	Uint16 alt_on = key_mod & KMOD_ALT;
	Uint16 ctrl_on = key_mod & KMOD_CTRL;
#ifdef DEBUG
	int i;
	Uint32 _cur_time= SDL_GetTicks();
	Uint16 shift_on = key_mod & KMOD_SHIFT;
#endif

	if(check_quit_or_fullscreen(key_code, key_mod))
	{
		return 1;
	}
	else if((key_code == SDLK_UP || key_code == SDLK_DOWN) && ctrl_on)
	{
		char *line;

		if(key_code == SDLK_UP)
		{
			line = history_get_line_up();
		}
		else
		{
			line= history_get_line_down();
		}
		if(line != NULL)
		{
			put_string_in_input_field((unsigned char*)line);
		}
	}
	else if(disconnected && !alt_on && !ctrl_on && !locked_to_console)
	{
		connect_to_server();
	}
	else if (KEY_DEF_CMP(K_PASTE, key_code, key_mod))
	{
		start_paste(NULL);
	}
#ifdef DEBUG
	else if((key_code == SDLK_LEFT) && shift_on && ctrl_on && !alt_on)
	{
		for (i=0; i<ITEM_WEAR_START;i++) {
			item_list[i].cooldown_rate = 60000;
			item_list[i].cooldown_time = _cur_time + 60000;
		}
	}
	else if((key_code == SDLK_DOWN) && shift_on && ctrl_on && !alt_on)
	{
		for(i=0; i<ITEM_WEAR_START;i++) {
			item_list[i].cooldown_time -= 1000;
		}
	}
	else if((key_code == SDLK_UP) && shift_on && ctrl_on && !alt_on)
	{
		for(i=0; i<ITEM_WEAR_START;i++) {
			item_list[i].cooldown_time += 1000;
		}
	}
	else if((key_code == SDLK_t) && shift_on && ctrl_on && !alt_on)
	{
		weather_add_lightning(rand()%5, -camera_x-100+rand()%200, -camera_y-100+rand()%200);
	}
	else if((key_code == SDLK_w) && shift_on && ctrl_on && alt_on)
	{
		if(real_game_minute >= 355) real_game_minute -=355; else real_game_minute +=  5;
		new_minute();
	}
	else if((key_code == SDLK_q) && shift_on && ctrl_on && alt_on)
	{
		if(real_game_minute < 5) real_game_minute +=355; else real_game_minute -=  5;
		new_minute();
	}
	else if((key_code == SDLK_w) && !shift_on && ctrl_on && alt_on)
	{
		if(real_game_minute >= 359) real_game_minute -=359; else real_game_minute +=  1;
		new_minute();
	}
	else if((key_code == SDLK_q) && !shift_on && ctrl_on && alt_on)
	{
		if(real_game_minute < 1) real_game_minute +=359; else real_game_minute -=  1;
		new_minute();
	}
	else if((key_code == SDLK_s) && shift_on && ctrl_on && alt_on)
	{
		skybox_init_defs(NULL);
	}
	else if((key_code == SDLK_f) && shift_on && ctrl_on && alt_on)
	{
		freeze_time = !freeze_time;
		if (freeze_time)
			LOG_TO_CONSOLE(c_green2, "Time freezed!");
		else
		{
			LOG_TO_CONSOLE(c_green2, "Time unfreezed!");
			new_minute();
			new_second();
		}
	}
	else if((key_code == SDLK_HOME) && shift_on && ctrl_on && !alt_on)
	{
		weather_set_area(0, -camera_x, -camera_y, 100.0, 1, 1.0, 10);
	}
	else if ((key_code == SDLK_END) && shift_on && ctrl_on && !alt_on)
	{
		weather_set_area(1, -camera_x, -camera_y, 100.0, 2, 1.0, 10);
	}
	else if((key_code == SDLK_z) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_SMALL_MINE, (poor_man ? 6 : 10));
	}
	else if((key_code == SDLK_x) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_MEDIUM_MINE, (poor_man ? 6 : 10));
	}
	else if((key_code == SDLK_c) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_HIGH_EXPLOSIVE_MINE, (poor_man ? 6 : 10));
	}
	else if((key_code == SDLK_v) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_TRAP, (poor_man ? 6 : 10));
	}
	else if((key_code == SDLK_b) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_CALTROP, (poor_man ? 6 : 10));
	}
	else if((key_code == SDLK_n) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_POISONED_CALTROP, (poor_man ? 6 : 10));
	}
	else if((key_code == SDLK_m) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_MANA_BURNER, (poor_man ? 6 : 10));
	}
	else if((key_code == SDLK_j) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_MANA_DRAINER, (poor_man ? 6 : 10));
	}
	else if((key_code == SDLK_k) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_UNINVIZIBILIZER, (poor_man ? 6 : 10));
	}
	else if((key_code == SDLK_l) && shift_on && ctrl_on && !alt_on)
	{
		ec_create_mine_detonate(your_actor->x_pos + 0.25f, your_actor->y_pos + 0.25f, 0, MINE_TYPE_MAGIC_IMMUNITY_REMOVAL, (poor_man ? 6 : 10));
	}
#endif
#ifdef DEBUG
    // scale the current actor
	else if((key_code == SDLK_p) && shift_on && ctrl_on && !alt_on)
	{
		get_our_actor()->scale *= 1.05;
	}
	else if((key_code == SDLK_o) && shift_on && ctrl_on && !alt_on)
	{
		get_our_actor()->scale /= 1.05;
	}
	else if((key_code == SDLK_h) && shift_on && ctrl_on && !alt_on)
	{
        if (get_our_actor())
        {
            if (get_our_actor()->attached_actor < 0)
                add_actor_attachment(get_our_actor()->actor_id, 200);
            else
                remove_actor_attachment(get_our_actor()->actor_id);
        }
	}
#endif // DEBUG
	// use quickbar items & spells
	else if (action_item_keys(key_code, key_mod))
	{
	}
	else if (action_spell_keys(key_code, key_mod))
	{
	}
	else if (KEY_DEF_CMP(K_SUMMONINGMENU, key_code, key_mod))
	{
		int actor_to_touch = get_id_last_summoned();
		if (actor_to_touch >= 0)
			touch_player(actor_to_touch);
	}
	// hide all windows
	else if (KEY_DEF_CMP(K_HIDEWINS, key_code, key_mod))
	{
		hide_all_windows();
	}
	// toggle options
	else if (KEY_DEF_CMP(K_HEALTHBAR, key_code, key_mod))
	{
		view_health_bar = !view_health_bar;
	}
	else if (KEY_DEF_CMP(K_VIEWETHER, key_code, key_mod))
	{
		view_ether = !view_ether;
	}
	else if (KEY_DEF_CMP(K_ETHERBARS, key_code, key_mod))
	{
		view_ether_bar = !view_ether_bar;
	}
	else if (KEY_DEF_CMP(K_VIEWTEXTASOVERTEXT, key_code, key_mod))
	{
		view_chat_text_as_overtext = !view_chat_text_as_overtext;
	}
	else if (KEY_DEF_CMP(K_VIEWNAMES, key_code, key_mod))
	{
		view_names = !view_names;
	}
	else if (KEY_DEF_CMP(K_VIEWHP, key_code, key_mod))
	{
		view_hp = !view_hp;
	}
	else if (KEY_DEF_CMP(K_SHADOWS, key_code, key_mod))
	{
		clouds_shadows = !clouds_shadows;
	}
	else if (KEY_DEF_CMP(K_RANGINGLOCK, key_code, key_mod))
	{
		toggle_ranging_lock();
	}
	// open or close tabbed windows
	else if (KEY_DEF_CMP(K_STATS, key_code, key_mod))
	{
		view_tab(MW_STATS, tab_stats_collection_id, STATS_TAB_STATS);
	}
	else if (KEY_DEF_CMP(K_SESSION, key_code, key_mod))
	{
		view_tab(MW_STATS, tab_stats_collection_id, STATS_TAB_SESSION);
	}
	else if (KEY_DEF_CMP(K_COUNTERS, key_code, key_mod))
	{
		view_tab(MW_STATS, tab_stats_collection_id, STATS_TAB_COUNTERS);
	}
	else if (KEY_DEF_CMP(K_KNOWLEDGE, key_code, key_mod))
	{
		view_tab(MW_STATS, tab_stats_collection_id, STATS_TAB_KNOWLEDGE);
	}
	else if (KEY_DEF_CMP(K_ENCYCLOPEDIA, key_code, key_mod))
	{
		view_tab(MW_HELP, tab_help_collection_id, HELP_TAB_ENCYCLOPEDIA);
	}
	else if (KEY_DEF_CMP(K_HELP, key_code, key_mod))
	{
		view_tab(MW_HELP, tab_help_collection_id, HELP_TAB_HELP);
	}
	else if (KEY_DEF_CMP(K_HELPSKILLS, key_code, key_mod))
	{
		view_tab(MW_HELP, tab_help_collection_id, HELP_TAB_SKILLS);
	}
	else if (KEY_DEF_CMP(K_RULES, key_code, key_mod))
	{
		view_tab(MW_HELP, tab_help_collection_id, HELP_TAB_RULES);
	}
	else if (KEY_DEF_CMP(K_NOTEPAD, key_code, key_mod))
	{
		view_tab(MW_INFO, tab_info_collection_id, INFO_TAB_NOTEPAD);
	}
	else if (KEY_DEF_CMP(K_BROWSERWIN, key_code, key_mod))
	{
		view_tab(MW_INFO, tab_info_collection_id, INFO_TAB_URLWIN);
	}
	// set action modes
	else if (KEY_DEF_CMP(K_WALK, key_code, key_mod))
	{
		switch_action_mode(ACTION_WALK);
	}
	else if (KEY_DEF_CMP(K_LOOK, key_code, key_mod))
	{
		switch_action_mode(ACTION_LOOK);
	}
	else if (KEY_DEF_CMP(K_USE, key_code, key_mod))
	{
		switch_action_mode(ACTION_USE);
	}
	else if (KEY_DEF_CMP(K_AFK, key_code, key_mod))
	{
		if (!afk)
		{
			go_afk ();
			last_action_time = cur_time - afk_time;
		}
		else
		{
			go_ifk ();
		}
	}
	else if (KEY_DEF_CMP(K_TARGET_CLOSE, key_code, key_mod))
	{
		toggle_target_close_clicked_creature();
	}
	else if (KEY_DEF_CMP(K_SIT, key_code, key_mod))
	{
		toggle_sit_stand ();
	}
	else if (KEY_DEF_CMP(K_BROWSER, key_code, key_mod))
	{
		open_last_seen_url();
	}
	else if (key_code == SDLK_ESCAPE)
	{
		root_key_to_input_field (key_code, key_unicode, key_mod);
	}
	else if (KEY_DEF_CMP(K_NEXT_CHAT_TAB, key_code, key_mod))
	{
		next_channel_tab();
	}
	else if (KEY_DEF_CMP(K_PREV_CHAT_TAB, key_code, key_mod))
	{
		prev_channel_tab();
	}
	else if (KEY_DEF_CMP(K_WINDOWS_ON_TOP, key_code, key_mod))
	{
		change_windows_on_top(&windows_on_top);
	}
#ifdef PNG_SCREENSHOT
	else if (KEY_DEF_CMP(K_SCREENSHOT, key_code, key_mod))
	{
		makeScreenShot();
	}
#endif
	else if (KEY_DEF_CMP(K_OPAQUEWIN, key_code, key_mod))
	{
		if (top_SWITCHABLE_OPAQUE_window_drawn != -1)
			windows_list.window[top_SWITCHABLE_OPAQUE_window_drawn].opaque ^= 1;
	}
	else if (KEY_DEF_CMP(K_REPEATSPELL, key_code, key_mod))	// REPEAT spell command
	{
		if ( !get_show_window_MW(MW_TRADE) )
		{
			repeat_spell();
		}
	}
	else
	{
		enum managed_window_enum i;
		for (i = 0; i < MW_MAX; i++)
		{
			if (match_keydef_MW(i, key_code, key_mod))
			{
				view_window(i);
				return 1;
			}
		}
		return 0; // nothing we can handle
	}

	return 1; // we handled it
}

int text_input_handler (SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	Uint8 ch = key_to_char (key_unicode);

	if (root_key_to_input_field(key_code, key_unicode, key_mod))
	{
		return 1;
	}
	// The following should only be reached when we hit an invalid key
	// combo or for any reason we don't have a valid input_widget.
	else if (is_printable (ch) && input_text_line.len < MAX_TEXT_MESSAGE_LENGTH)
	{
		if(put_char_in_buffer (&input_text_line, ch, input_text_line.len)) {
			if(input_widget) {
				text_field *tf = input_widget->widget_info;
				tf->cursor = tf->buffer->len;
				if(input_widget->window_id == game_root_win) {
					widget_unset_flags (input_widget->window_id, input_widget->id, WIDGET_DISABLED);
				}
			}
		}
	}
#ifndef OSX
	else if (key_code == SDLK_BACKSPACE && input_text_line.len > 0)
#else
	else if (((key_code == SDLK_BACKSPACE) || (ch == 127)) && input_text_line.len > 0)
#endif
	{
		input_text_line.len--;
		if (input_text_line.data[input_text_line.len] == '\n'
		|| input_text_line.data[input_text_line.len] == '\r') {
			input_text_line.len--;
		}
		input_text_line.data[input_text_line.len] = '\0';
	}
	else if ((key_code == SDLK_RETURN || key_code == SDLK_KP_ENTER) && input_text_line.len > 0)
	{
		parse_input(input_text_line.data, input_text_line.len);
		add_line_to_history(input_text_line.data, input_text_line.len);
		clear_input_line();
	}
	else
	{
		// no clue what to do with this character
		return 0;
	}
	return 1;
}

static int keypress_game_handler (window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	// first try the keypress handler for all root windows
	if ( keypress_root_common (key_code, key_unicode, key_mod) )
	{
		return 1;
	}
	else if (KEY_DEF_CMP(K_TURNLEFT, key_code, key_mod))
	{
		//Moved delay to my_tcp_send
		Uint8 str[2];
		str[0] = TURN_LEFT;
		my_tcp_send (my_socket, str, 1);
	}
	else if (KEY_DEF_CMP(K_TURNRIGHT, key_code, key_mod))
	{
		Uint8 str[2];
		str[0] = TURN_RIGHT;
		my_tcp_send (my_socket, str, 1);
	}
	else if (KEY_DEF_CMP(K_ADVANCE, key_code, key_mod))
	{
		move_self_forward();
	}
	else if (KEY_DEF_CMP(K_ROTATELEFT, key_code, key_mod))
	{
		camera_rotation_speed = (first_person?-1:1)*normal_camera_rotation_speed / 800.0;
		camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		camera_rotation_duration = 800;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
	}
	else if (KEY_DEF_CMP(K_FROTATELEFT, key_code, key_mod))
	{
		camera_rotation_speed = (first_person?-1:1)*fine_camera_rotation_speed / 200.0;
		camera_rotation_speed /= 4.0;
		camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		camera_rotation_duration = 200;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
	}
	else if (KEY_DEF_CMP(K_ROTATERIGHT, key_code, key_mod))
	{
		camera_rotation_speed = (first_person?1:-1)*normal_camera_rotation_speed / 800.0;
		camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		camera_rotation_duration = 800;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
	}
	else if (KEY_DEF_CMP(K_FROTATERIGHT, key_code, key_mod))
	{
		camera_rotation_speed = (first_person?1:-1)*fine_camera_rotation_speed / 200.0;
		camera_rotation_speed /= 4.0;
		camera_rotation_deceleration = normal_camera_deceleration*0.5E-3;
		camera_rotation_duration = 200;
		if (fol_cam && !fol_cam_behind)
		{
			hold_camera += camera_kludge - last_kludge;
			last_kludge = camera_kludge;
		}
	}
	else if (KEY_DEF_CMP(K_CAMERAUP, key_code, key_mod))
	{
		camera_tilt_speed = -normal_camera_rotation_speed * 0.0005;
		camera_tilt_duration += 100;
		camera_tilt_deceleration = normal_camera_deceleration*0.5E-3;
	}
	else if (KEY_DEF_CMP(K_CAMERADOWN, key_code, key_mod))
	{
		camera_tilt_speed = normal_camera_rotation_speed * 0.0005;
		camera_tilt_duration += 100;
		camera_tilt_deceleration = normal_camera_deceleration*0.5E-3;
	}
	else if (KEY_DEF_CMP(K_ZOOMIN, key_code, key_mod))
	{
		if (camera_zoom_dir == -1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = -1;
	}
	if (KEY_DEF_CMP(K_ZOOMOUT, key_code, key_mod))
	{
		if (camera_zoom_dir == 1)
			camera_zoom_duration += 100;
		else
			camera_zoom_duration = 100;
		camera_zoom_dir = 1;
	}
	else if ((KEY_DEF_CMP(K_MAP, key_code, key_mod)) || (KEY_DEF_CMP(K_MARKFILTER, key_code, key_mod)))
	{
		// if K_MARKFILTER pressed, open the map window with the filter active
		if (KEY_DEF_CMP(K_MARKFILTER, key_code, key_mod))
			mark_filter_active = 1;
		if ( switch_to_game_map () )
		{
			if (have_mouse) {toggle_have_mouse(); keep_grabbing_mouse=1;}
			hide_window (game_root_win);
			show_window_MW(MW_TABMAP);
		}
	}
	else if (key_code == SDLK_F6)
	{
		if(!hud_x)
		{
			hud_x = HUD_MARGIN_X;
			hud_y = HUD_MARGIN_Y;
		}
		else
		{
			hud_x=0;
			hud_y=0;
		}
		resize_root_window ();
	}
	else if (KEY_DEF_CMP(K_FIRST_PERSON, key_code, key_mod))
	{
		toggle_first_person();
	}
	else if (KEY_DEF_CMP(K_GRAB_MOUSE, key_code, key_mod))
	{
		toggle_have_mouse();
	}
	else if (KEY_DEF_CMP(K_EXTEND_CAM, key_code, key_mod))
	{
		toggle_ext_cam(&ext_cam);
	}
#ifdef PAWN
	else if (key_code == SDLK_F8)
	{
		if (object_under_mouse != -1 && thing_under_the_mouse == UNDER_MOUSE_3D_OBJ && objects_list[object_under_mouse])
		{
			run_pawn_map_function("play_with_object_pos", "ii", object_under_mouse, key_mod & KMOD_SHIFT ? 1: 0);
		}
		else
		{
			run_pawn_server_function("pawn_test", "s", "meep!");
		}
	}
#endif
	else if (key_code == SDLK_F8)
	{
		static int ison = 0;
		if ((ison) && (weather_get_intensity() > 0.01))
		{
			weather_set_area(1, -camera_x, -camera_y, 100.0, 2, 0, 0);
			ison = 0;
		}
		else
		{
			weather_set_area(1, -camera_x, -camera_y, 100.0, 2, 1.0, 0);
			ison = 1;
		}
	}

	else if (key_code == SDLK_F9)
	{
		actor *me = get_actor_ptr_from_id (yourself);
		if (key_mod & KMOD_SHIFT)
			remove_fire_at_tile(me->x_pos * 2, me->y_pos * 2);
		else
			add_fire_at_tile(1, me->x_pos * 2, me->y_pos * 2, get_tile_height(me->x_tile_pos, me->y_tile_pos));
	}
#ifdef DEBUG
	else if (key_code == SDLK_F10)
	{
		if (key_mod & KMOD_SHIFT)
		{
#ifdef NEW_SOUND
			print_sound_types();
			print_sound_samples();
			print_sounds_list();
			print_sound_sources();
#endif //!NEW_SOUND
		}
		else if (key_mod & KMOD_ALT)
		{
			print_filter_list ();
		}
		else
		{
			int iwin;
			widget_list *l;
			for (iwin = 0; iwin < windows_list.num_windows; iwin++)
			{
				printf ("%s: id = %d, order = %d, parent = %d, pos = (%d, %d), cur_pos = (%d, %d), displayed = %d\n", windows_list.window[iwin].window_name, windows_list.window[iwin].window_id, windows_list.window[iwin].order, windows_list.window[iwin].pos_id, windows_list.window[iwin].pos_x, windows_list.window[iwin].pos_y, windows_list.window[iwin].cur_x, windows_list.window[iwin].cur_y, windows_list.window[iwin].displayed);
				for (l = windows_list.window[iwin].widgetlist; l; l = l->next)
					printf ("\t%d\n", l->id);
			}
		}
	}
	else if (key_code == SDLK_F11)
	{
		unload_texture_cache();
	}
	else if (key_code == SDLK_F12)
	{
		dump_texture_cache();
	}
#endif	/* DEBUG */
	// END OF TEST!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	else
	{
		Uint8 ch = key_to_char (key_unicode);

		if (ch == '`' || KEY_DEF_CMP(K_CONSOLE, key_code, key_mod))
		{
			if (have_mouse) {toggle_have_mouse(); keep_grabbing_mouse=1;}
			hide_window (game_root_win);
			show_window_MW(MW_CONSOLE);
		}
		// see if the common text handler can deal with it
		else if ( !text_input_handler (key_code, key_unicode, key_mod) )
		{
			// nothing we can handle
			return 0;
		}
	}

	// we handled it, return 1 to let the window manager know
	return 1;
}

void do_keypress(el_key_def key)
{
	if (game_root_win >= 0)
	{
		window_info *win = &windows_list.window[game_root_win];
		if (win != NULL)
			keypress_game_handler(win, 0, 0, key.key_code, 0, key.key_mod);
	}
}

static int show_game_handler (window_info *win) {
	init_hud_interface (HUD_INTERFACE_GAME);
	show_hud_windows();
	if (use_windowed_chat == 1)
	{
		if (is_chat_shown())
			display_tab_bar();
		else
			hide_window(tab_bar_win);
	}
	set_all_intersect_update_needed(main_bbox_tree); // redraw the scene
	return 1;
}

static int resize_game_root_handler(window_info *win, int width, int height)
{
	if (get_show_window(win->window_id))
	{
		init_hud_interface (HUD_INTERFACE_GAME);
		set_all_intersect_update_needed(main_bbox_tree); // redraw the scene
	}
	// Recalculate FPS width
	fps_default_width = 0;
	return 1;
}

static int ui_scale_game_root_handler(window_info* win)
{
	// Recalculate FPS width
	fps_default_width = 0;
	return 1;
}

static int change_game_root_font_handler(window_info *win, font_cat cat)
{
	if (cat != UI_FONT)
		return 0;
	ui_scale_game_root_handler(win);
	return 1;
}

void create_game_root_window (int width, int height)
{
	if (game_root_win < 0)
	{
		game_root_win = create_window ("Game", -1, -1, 0, 0, width, height, ELW_USE_UISCALE|ELW_TITLE_NONE|ELW_SHOW_LAST);

		set_window_handler (game_root_win, ELW_HANDLER_DISPLAY, &display_game_handler);
		set_window_handler (game_root_win, ELW_HANDLER_CLICK, &click_game_handler);
		set_window_handler (game_root_win, ELW_HANDLER_MOUSEOVER, &mouseover_game_handler);
		set_window_handler (game_root_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_game_handler);
		set_window_handler (game_root_win, ELW_HANDLER_SHOW, &show_game_handler);
		set_window_handler (game_root_win, ELW_HANDLER_AFTER_SHOW, &update_have_display);
		set_window_handler (game_root_win, ELW_HANDLER_HIDE, &update_have_display);
		set_window_handler (game_root_win, ELW_HANDLER_RESIZE, &resize_game_root_handler);
		set_window_handler (game_root_win, ELW_HANDLER_UI_SCALE, &ui_scale_game_root_handler);
		set_window_handler (game_root_win, ELW_HANDLER_FONT_CHANGE, &change_game_root_font_handler);

		if (input_widget == NULL)
		{
			int input_height = get_input_height();
			Uint32 id;

			if (dark_channeltext == 1)
				set_text_message_color (&input_text_line, 0.6f, 0.6f, 0.6f);
			else if (dark_channeltext == 2)
				set_text_message_color (&input_text_line, 0.16f, 0.16f, 0.16f);
			else
				set_text_message_color (&input_text_line, 1.0f, 1.0f, 1.0f);
			id = text_field_add_extended(game_root_win, 42, NULL,
				0, height-input_height-hud_y, width-hud_x, input_height,
				INPUT_DEFAULT_FLAGS, CHAT_FONT, 1.0,
				&input_text_line, 1, FILTER_ALL, INPUT_MARGIN, INPUT_MARGIN);
			input_widget = widget_find(game_root_win, id);
			input_widget->OnResize = input_field_resize;
		}
		widget_set_OnKey(input_widget->window_id, input_widget->id, (int (*)())chat_input_key);
		if(input_text_line.len > 0) {
			widget_unset_flags (input_widget->window_id, input_widget->id, WIDGET_DISABLED);
		}
		resize_root_window();

#ifdef NEW_CURSOR
		cursors_tex = load_texture_cached("textures/cursors2.dds", tt_gui);
		//Emajekral's hi-color & big cursor code
		if (!sdl_cursors) SDL_ShowCursor(0);
#endif // NEW_CURSOR
	}
}

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <SDL.h>
#include "actors.h"
#include "actor_scripts.h"
#include "asc.h"
#include "bbox_tree.h"
#include "buffs.h"
#include "cal.h"
#include "cursors.h"
#include "draw_scene.h"
#include "errors.h"
#include "events.h"
#include "gl_init.h"
#include "interface.h"
#include "load_gl_extensions.h"
#include "map.h"
#include "missiles.h"
#include "new_actors.h"
#include "platform.h"
#include "shadows.h"
#include "special_effects.h"
#include "textures.h"
#include "translate.h"
#include "vmath.h"
#ifdef CLUSTER_INSIDES
#include "cluster.h"
#endif
#include "eye_candy_wrapper.h"
#include "minimap.h"
#include "actor_init.h"
#ifdef	FSAA
#include "fsaa/fsaa.h"
#endif	/* FSAA */

actor *actors_list[MAX_ACTORS];
int max_actors=0;
SDL_mutex *actors_lists_mutex = NULL;	//used for locking between the timer and main threads
actor *your_actor = NULL;

actor_types actors_defs[MAX_ACTOR_DEFS];

attached_actors_types attached_actors_defs[MAX_ACTOR_DEFS];

static void draw_actor_overtext(actor* actor_ptr, double x, double y, double z); /* forward declaration */

int no_near_actors=0;
#ifdef NEW_SOUND
int no_near_enhanced_actors = 0;
float distanceSq_to_near_enhanced_actors;
#endif // NEW_SOUND
near_actor near_actors[MAX_ACTORS];

#ifdef MUTEX_DEBUG
SDL_threadID have_actors_lock = 0;
#endif

int cm_mouse_over_banner = 0;		/* use to trigger banner context menu */

int get_closest_actor(int tile_x, int tile_y, float max_distance)
{
	int i;
	int found_actor = -1;
	float x=tile_x / 2.0f;
	float y=tile_y / 2.0f;
	float distance;
	float min_distance_found = 50;

	// check if too far
	actor *me = get_our_actor();
	if (me)
	{
		distance = sqrt((me->x_pos - x) * (me->x_pos - x) + (me->y_pos - y) * (me->y_pos - y));
		if (distance > 6)
			return -1;
	}

	for (i=0; i<max_actors; i++)
		if (actors_list[i])
			if (!actors_list[i]->dead)
				if (actors_list[i]->kind_of_actor != NPC && actors_list[i]->kind_of_actor != HUMAN && actors_list[i]->kind_of_actor != COMPUTER_CONTROLLED_HUMAN)
				{
					distance = sqrt((actors_list[i]->x_pos - x) * (actors_list[i]->x_pos - x) + (actors_list[i]->y_pos - y ) * (actors_list[i]->y_pos - y));
					if (distance < max_distance)
						if (distance < min_distance_found)
						{
							found_actor = actors_list[i]->actor_id;
							min_distance_found = distance;
						}
				}

	return found_actor;
}

//Threading support for actors_lists
void init_actors_lists()
{
	int	i;

	actors_lists_mutex=SDL_CreateMutex();
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	for (i=0; i < MAX_ACTORS; i++)
		actors_list[i] = NULL;
	UNLOCK_ACTORS_LISTS();	// release now that we are done
}

//return the ID (number in the actors_list[]) of the new allocated actor
int add_actor (int actor_type, char * skin_name, float x_pos, float y_pos, float z_pos, float z_rot, float scale, char remappable, short skin_color, short hair_color, short eyes_color, short shirt_color, short pants_color, short boots_color, int actor_id)
{
	int texture_id;
	int i;
	int k;
	actor *our_actor;
#ifdef CLUSTER_INSIDES
	int x, y;
#endif

#ifdef EXTRA_DEBUG
	ERR();
#endif

	if (actors_defs[actor_type].ghost)
	{
		texture_id = load_texture_cached(skin_name, tt_mesh);
	}
	else
	{
		if (!remappable)
		{
			texture_id = load_texture_cached(skin_name, tt_mesh);
		}
		else
		{
			LOG_ERROR("remapped skin for %s", skin_name);
			exit(-1);
		}
	}

	our_actor = calloc(1, sizeof(actor));

	memset(our_actor->current_displayed_text, 0, sizeof(our_actor->current_displayed_text));
	our_actor->current_displayed_text_lines = 0;
	our_actor->current_displayed_text_time_left =  0;

	our_actor->is_enhanced_model=0;
	our_actor->remapped_colors=remappable;
	our_actor->actor_id=actor_id;
	our_actor->cur_anim_sound_cookie = 0;

	our_actor->cal_h_rot_start = 0.0;
	our_actor->cal_h_rot_end = 0.0;
	our_actor->cal_v_rot_start = 0.0;
	our_actor->cal_v_rot_end = 0.0;
	our_actor->cal_rotation_blend = -1.0;
	our_actor->cal_rotation_speed = 0.0;
	our_actor->are_bones_rotating = 0;
	our_actor->in_aim_mode = 0;
	our_actor->range_actions_count = 0;
	our_actor->delayed_item_changes_count = 0;

	our_actor->x_pos=x_pos;
	our_actor->y_pos=y_pos;
	our_actor->z_pos=z_pos;
	our_actor->scale=scale;

	our_actor->x_speed=0;
	our_actor->y_speed=0;
	our_actor->z_speed=0;

	our_actor->x_rot=0;
	our_actor->y_rot=0;
	our_actor->z_rot=z_rot;

	our_actor->last_range_attacker_id = -1;

	//reset the script related things
	our_actor->move_x_speed=0;
	our_actor->move_y_speed=0;
	our_actor->move_z_speed=0;
	our_actor->rotate_x_speed=0;
	our_actor->rotate_y_speed=0;
	our_actor->rotate_z_speed=0;
	our_actor->movement_time_left=0;
	our_actor->moving=0;
	our_actor->rotating=0;
	our_actor->busy=0;
	our_actor->last_command=nothing;
#ifdef	ANIMATION_SCALING
	our_actor->animation_scale = 1.0f;
#endif	/* ANIMATION_SCALING*/

    /* load the texture in case it's not already loaded and look if it has
     * an alpha map */
	our_actor->has_alpha = get_texture_alpha(texture_id);

	//clear the que
	for(k=0;k<MAX_CMD_QUEUE;k++)	our_actor->que[k]=nothing;
	//clear emotes
	for(k=0;k<MAX_EMOTE_QUEUE;k++)	{
		our_actor->emote_que[k].emote=NULL;
		our_actor->emote_que[k].origin=NO_EMOTE;
		our_actor->emote_que[k].create_time=0;
		}
	memset(&our_actor->cur_emote,0,sizeof(emote_anim));
	memset(&our_actor->poses,0,sizeof(emote_data*)*4);
	for(k=0;k<MAX_EMOTE_FRAME;k++) our_actor->cur_emote.frames[k].anim_index=-1;
	our_actor->cur_emote.idle.anim_index=-1;
	our_actor->cur_emote_sound_cookie=0;




	our_actor->texture_id=texture_id;
	our_actor->skin=skin_color;
	our_actor->hair=hair_color;
	our_actor->eyes=eyes_color;
	our_actor->pants=pants_color;
	our_actor->boots=boots_color;
	our_actor->shirt=shirt_color;
	our_actor->stand_idle=0;
	our_actor->sit_idle=0;

	our_actor->attached_actor = -1;
	our_actor->attachment_shift[0] = our_actor->attachment_shift[1] = our_actor->attachment_shift[2] = 0.0;

	for (i = 0; i < NUM_BUFFS; i++)
	{
		our_actor->ec_buff_reference[i] = NULL;
	}

#ifdef CLUSTER_INSIDES
	x = (int) (our_actor->x_pos / 0.5f);
	y = (int) (our_actor->y_pos / 0.5f);
	our_actor->cluster = get_cluster (x, y);
#endif

	//find a free spot, in the actors_list
	LOCK_ACTORS_LISTS();

	for(i=0;i<max_actors;i++)
		{
			if(!actors_list[i])break;
		}

	if(actor_id == yourself)
		set_our_actor (our_actor);

	actors_list[i]=our_actor;
	if(i>=max_actors)max_actors=i+1;

	//It's unlocked later

	ec_add_actor_obstruction(our_actor, 3.0);
	return i;
}

void add_actor_attachment(int actor_id, int attachment_type)
{
	int i;
	actor *parent = NULL;

	for (i = 0; i < max_actors; ++i)
		if (actors_list[i]->actor_id == actor_id)
		{
			parent = actors_list[i];
			break;
		}

	if (!parent)
		LOG_ERROR("unable to add an attached actor: actor with id %d doesn't exist!", actor_id);
	else if(attachment_type < 0 || attachment_type >= MAX_ACTOR_DEFS || (attachment_type > 0 && actors_defs[attachment_type].actor_type != attachment_type) )
		LOG_ERROR("unable to add an attached actor: illegal/missing actor definition %d", attachment_type);
	else
	{
		int id = add_actor(attachment_type, actors_defs[attachment_type].skin_name,
						   parent->x_pos, parent->y_pos, parent->z_pos, parent->z_rot, get_actor_scale(parent),
						   0, 0, 0, 0, 0, 0, 0, -1);
		actors_list[id]->attached_actor = i;
		parent->attached_actor = id;

		actors_list[id]->async_fighting = 0;
		actors_list[id]->async_x_tile_pos = parent->async_x_tile_pos;
		actors_list[id]->async_y_tile_pos = parent->async_y_tile_pos;
		actors_list[id]->async_z_rot = parent->async_z_rot;

		actors_list[id]->x_tile_pos=parent->x_tile_pos;
		actors_list[id]->y_tile_pos=parent->y_tile_pos;
		actors_list[id]->buffs=parent->buffs & BUFF_DOUBLE_SPEED; // the attachment can only have this buff
		actors_list[id]->actor_type=attachment_type;
		actors_list[id]->damage=0;
		actors_list[id]->damage_ms=0;
		actors_list[id]->sitting=0;
		actors_list[id]->fighting=0;
		//test only
		actors_list[id]->max_health=0;
		actors_list[id]->cur_health=0;
		actors_list[id]->ghost=actors_defs[attachment_type].ghost;
		actors_list[id]->dead=0;
		actors_list[id]->stop_animation=1;//helps when the actor is dead...
		actors_list[id]->kind_of_actor=0;

		if (attached_actors_defs[attachment_type].actor_type[parent->actor_type].is_holder)
			actors_list[id]->step_duration = actors_defs[attachment_type].step_duration;
		else
			actors_list[id]->step_duration = parent->step_duration;

		if (actors_list[id]->buffs & BUFF_DOUBLE_SPEED)
			actors_list[id]->step_duration /= 2;

		actors_list[id]->z_pos = get_actor_z(actors_list[id]);

		//printf("attached actor n°%d of type %d to actor n°%d with id %d\n", id, attachment_type, i, actor_id);

		if (actors_defs[attachment_type].coremodel!=NULL) {
			//Setup cal3d model
			actors_list[id]->calmodel = model_new(actors_defs[attachment_type].coremodel);
			//Attach meshes
			if(actors_list[id]->calmodel) {
				model_attach_mesh(actors_list[id], actors_defs[attachment_type].shirt[0].mesh_index);
				set_on_idle(id);

				build_actor_bounding_box(actors_list[id]);
				if (use_animation_program)
					set_transformation_buffers(actors_list[id]);
			}
		}
		else
			actors_list[id]->calmodel=NULL;

		UNLOCK_ACTORS_LISTS();
	}
}

void remove_actor_attachment(int actor_id)
{
	int i;

	LOCK_ACTORS_LISTS();

	for (i = 0; i < max_actors; ++i)
		if (actors_list[i]->actor_id == actor_id)
		{
			int att = actors_list[i]->attached_actor;
			actors_list[i]->attached_actor = -1;
			actors_list[i]->attachment_shift[0] = 0.0;
			actors_list[i]->attachment_shift[1] = 0.0;
			actors_list[i]->attachment_shift[2] = 0.0;
			free_actor_special_effect(actors_list[att]->actor_id);
			free_actor_data(att);
			free(actors_list[att]);
			actors_list[att]=NULL;
			if(att==max_actors-1)max_actors--;
			else {
				//copy the last one down and fill in the hole
				max_actors--;
				actors_list[att]=actors_list[max_actors];
				actors_list[max_actors]=NULL;
				if (actors_list[att] && actors_list[att]->attached_actor >= 0)
					actors_list[actors_list[att]->attached_actor]->attached_actor = att;
			}
			break;
		}

	UNLOCK_ACTORS_LISTS();
}

static void set_health_color(actor * actor_id, float percent, float multiplier, float a)
{
	float r,g;

	if (actor_id != NULL)
	{
		// Only a subset of actors have health bars, so the choice
		// here is limited.
		if (actor_id->actor_id == yourself)
		{
			if (!dynamic_banner_colour.yourself)
				percent = 1.0f;
		}
		else if (actor_id->is_enhanced_model)
		{
			if (!dynamic_banner_colour.other_players)
				percent = 1.0f;
		}
		else if (!dynamic_banner_colour.creatures)
			percent = 1.0f;
	}

	r=(1.0f-percent)*2.0f;
	g=(percent/1.25f)*2.0f;

	if(r<0.0f)r=0.0f;
	else if(r>1.0f)r=1.0f;
	if(g<0.0f)g=0.0f;
	else if(g>1.0f)g=1.0f;

	glColor4f(r*multiplier,g*multiplier,0.0f, a);
}

static void set_mana_color(float percent, float multiplier, float a)
{
	float c;

	if (!dynamic_banner_colour.yourself)
		percent = 1.0f;

	c=0.6f - percent*0.6f;

	if(c<0.0f)c=0.0f;
	else if(c>1.0f)c=1.0f;

	glColor4f(c,c,2.0f, a);
}


void draw_actor_banner(actor * actor_id, float offset_z)
{
	unsigned char str[60];
	GLdouble model[16],proj[16];
	GLint view[4];

	GLdouble hx,hy,hz,a_bounce;
	float name_zoom = font_scales[NAME_FONT];
	float font_scale = 1.0f/ALT_INGAME_FONT_X_LEN;
	float stat_font_size = font_scale * ALT_INGAME_FONT_X_LEN; // base it on font_scale - OK so this makes it 1.0f currently
	float name_font_size = font_scale * SMALL_INGAME_FONT_X_LEN;
	double healthbar_x=0.0f;
	double healthbar_y=0.0f;
	double healthbar_z=offset_z+0.1;
	double healthbar_x_len_converted=0;
	double healthbar_x_len_loss=0;
	double healthbar_x_loss_fade=1.0f;
	double y_top, y_bottom;

	//we use health bar variables if possible, all the extras we need for ether bar are:
	double ether_str_x_len = 0;
	double etherbar_x_len_converted=0;
	GLdouble name_bot_y, health_bot_y, ether_bot_y;

	//some general values valid for whole banner
	double bar_x_len = 0;
	double bar_y_len = get_line_height(NAME_FONT, stat_font_size);
	float banner_width = 0.0f;

	//define inner display_xxxxx variables to have more control over displaying inside this function
	//necesary to implement instance mode makes code a bit more easy to understand imho
	int display_hp = view_hp;
	int display_names = view_names;
	int display_health_bar = view_health_bar;
	int display_ether_bar = view_ether_bar;
	int display_ether = view_ether;
	int display_banner_alpha = use_alpha_banner;

	//some general info about "what's going on" - allows not to repeat complex conditions later
	int displaying_me = 0;
	int displaying_other_player = 0;
	int display_health_line = 0;
	int display_ether_line = 0;

	//if first person, dont draw banner
	actor *me = get_our_actor();
	if (me && me->actor_id==actor_id->actor_id) {
		displaying_me = 1;
	};
	if (displaying_me && first_person) return;

	//if not drawing me, can't display ether and ether bar
	if (!displaying_me) {
		display_ether_bar = 0;
		display_ether = 0;
	}

	//if instance mode enabled, overwrite default view banner view options according to it
	if (view_mode_instance) {
		//for my banner - use standard banner settings
		if (!actor_id->is_enhanced_model) {
			//creatures
			display_hp = im_creature_view_hp;
			display_names = im_creature_view_names;
			display_health_bar = im_creature_view_hp_bar;
			display_banner_alpha = im_creature_banner_bg;
		//TODO: it shows healthbar above mule & summons too - probably no way to solve this issue
		} else if (!displaying_me && actor_id->is_enhanced_model){
			//other players
			displaying_other_player = (actor_id->kind_of_actor != NPC);
			display_hp = im_other_player_view_hp;
			display_names = im_other_player_view_names;
			display_health_bar = im_other_player_view_hp_bar;
			display_banner_alpha = im_other_player_banner_bg;
		}
	}

	//Figure out where the point just above the actor's head is in the viewport
	//See if Projection and viewport can be saved elsewhere to prevent doing this so often
	//MODELVIEW is hopeless
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);
	// Input adjusted healthbar_y value to scale hy according to actor scale
	gluProject(healthbar_x, healthbar_y, healthbar_z * actor_id->scale * actors_defs[actor_id->actor_type].actor_scale + 0.02, model, proj, view, &hx, &hy, &hz);

	//Save World-view and Projection matrices to allow precise raster placement of quads
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//Don't forget that the viewport is expressed in X+W,Y+H, or point-displacement,
	//versus the Ortho projection which expects x1,x2,y1,y2, or absolute coordinates
	glOrtho(view[0],view[2]+view[0],view[1],view[3]+view[1],0.0f,-1.0f);

	glColor3f (1.0f, 0.0f, 0.0f);

	glDepthFunc(GL_ALWAYS);
	if(actor_id->damage_ms){
		if(floatingmessages_enabled){
			float a=(float)(cur_time-actor_id->last_health_loss)/2000.0f;
			if(actor_id->damage>0){
				sprintf((char*)str,"%i",actor_id->damage);
				glColor4f(1.0f, 0.1f, 0.2f, 1.0-(a*a));
			} else {
				sprintf((char*)str,"%i",-actor_id->damage);
				glColor4f(0.3f, 1.0f, 0.3f, 1.0-(a*a));
			}

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			//Make damage numbers bounce on the actor's head. Owie!
			a*=2000;
			a_bounce=0;
			if (a < 500){
				a_bounce = 50.0 - 0.0002 * powf(a,2);
			} else if ( a < 950.0){
				a_bounce = 0.09*(a-500.0) - .0002 * powf((a-500.0), 2);
			} else if ( a < 1355.0 ){
				a_bounce = 0.081*(a-950.0) - .0002 * powf((a-950.0), 2);
			} else if ( a < 1720 ){
				a_bounce = 0.0730*(a-1355.0) - .0002 * powf((a-1355.0), 2);
			} else {
				a_bounce = 0.0640*(a-1720.0) - .0002 * powf((a-1720.0), 2);
			}
			/* Schmurk: actually we never reach this code as long as there's
			 * an exit condition at the beginning of the function */
			if ((first_person)&&(actor_id->actor_id==yourself)){
				float x,y;
				x = window_width / 2.0 - 0.5f * (float)get_string_width_zoom(str, NAME_FONT,  font_scale*0.17);
				y = a_bounce + window_height/2.0-40.0;
				draw_ortho_ingame_string(x, y, 0, str, 1, NAME_FONT, font_scale*.14, font_scale*.14);
			}
			else
			{
				float font_scale2 = font_scale*powf(1.0f+((float)abs(actor_id->damage)/2.0f)/1000.0f, 4.0);
				int extra_y = (view_mode_instance && displaying_me) ?view_mode_instance_damage_height * bar_y_len : 0;
				int lines = (!(view_mode_instance && displaying_me) && (display_hp || display_health_bar) && (display_ether || display_ether_bar)) ? 3 : 2;
				draw_ortho_ingame_string(hx - 0.5f * (float)get_string_width_zoom(str, NAME_FONT, font_scale2*0.17),
					a_bounce + hy + extra_y + get_text_height(lines, NAME_FONT, name_zoom), 0, str, 1,
					NAME_FONT, font_scale2*.14, font_scale2*.14);
			}
			glDisable(GL_BLEND);
		}
		else
		{	//No floating messages
			sprintf((char*)str,"%i",actor_id->damage);
			glColor3f (1.0f, 0.3f, 0.3f);
			draw_ortho_ingame_string(-0.1f, 0.5*healthbar_z, 0.0f, str, 1, NAME_FONT,
				INGAME_FONT_X_LEN*10.0, INGAME_FONT_X_LEN*10.0);
		}
		if (view_mode_instance && im_other_player_show_banner_on_damage && displaying_other_player && !display_hp && !display_health_bar && actor_id->damage>0) {
			display_hp = 1;
			display_names = 1;
		}
	}

	glDepthFunc(GL_LESS);

	//figure out which lines should we display
	display_health_line = (actor_id->kind_of_actor != NPC && (display_hp || display_health_bar) && actor_id->cur_health > 0 && actor_id->max_health > 0);
	display_ether_line = ((display_ether || display_ether_bar) && displaying_me && your_info.ethereal_points.base > 0 );
	if (view_mode_instance && displaying_me) {
		//make your bar a bit more above everything else so you can see it good enough
		//and got no problems with attacking mobs
		hy += view_mode_instance_banner_height*bar_y_len;
	}

	// calculate the bottom y coord for the displayed info lines, keep centred around a the same point whether 3, 2 or 1 line being displays
	hy += bar_y_len;
	name_bot_y = (display_names) ?hy + (display_health_line * bar_y_len / 2) + (display_ether_line * bar_y_len / 2) :0.0;
	health_bot_y = (display_health_line) ?hy - (display_names * bar_y_len / 2) + (display_ether_line * bar_y_len / 2) :0.0;
	ether_bot_y = (display_ether_line) ?hy -(display_names * bar_y_len / 2) - (display_health_line * bar_y_len / 2) : 0.0;
	// printf("hy=%.1lf name=%.1lf health=%.1lf ether=%.1lf\n", hy, name_bot_y, health_bot_y, ether_bot_y);

	// main block that draws the name, health and ether text
	if (!((first_person)&&(actor_id->actor_id==yourself)))
	{
		if(actor_id->actor_name[0] && (display_names || display_health_line || display_ether_line))
		{
			if (display_names)
			{
				if(actor_id->kind_of_actor==NPC){
					glColor3f(0.3f,0.8f,1.0f);
				} else if(actor_id->kind_of_actor==HUMAN || actor_id->kind_of_actor==COMPUTER_CONTROLLED_HUMAN){
					if(map_type == 2){
						glColor3f(0.6f,0.9f,0.9f);
					} else {
						glColor3f(1.0f,1.0f,1.0f);
					}
				} else if(actor_id->is_enhanced_model && (actor_id->kind_of_actor==PKABLE_HUMAN || actor_id->kind_of_actor==PKABLE_COMPUTER_CONTROLLED)){
					glColor3f(1.0f,0.0f,0.0f);
				} else {
					glColor3f(1.0f,1.0f,0.0f);
				}
				banner_width = 0.5 * (float)get_string_width_zoom((unsigned char*)actor_id->actor_name, NAME_FONT, name_font_size);
				draw_ortho_ingame_string(hx - banner_width, name_bot_y, hz,
					(const unsigned char*)actor_id->actor_name, 1, NAME_FONT, name_font_size, name_font_size);
			}
			if (view_buffs)
			{
				draw_buffs(actor_id->actor_id, hx, hy, hz);
			}

			if(  (!actor_id->dead) && (actor_id->kind_of_actor != NPC) && (display_health_line || display_ether_line)){
				unsigned char hp[200];
				unsigned char mana[200];
				double health_str_x_len = 0.0;

				// make the heath bar the same length as the the health text so they are balanced
				// use the same length health bar, even if not displaying the health text
				sprintf((char*)hp,"%u/%u", actor_id->cur_health, actor_id->max_health);
				health_str_x_len = (float)get_string_width_zoom(hp, NAME_FONT, stat_font_size);
				//do the same with mana if we want to display it
				if (display_ether || display_ether_bar) {
					sprintf((char*)mana,"%d/%d", your_info.ethereal_points.cur, your_info.ethereal_points.base);
					ether_str_x_len=(float)get_string_width_zoom(mana, NAME_FONT, stat_font_size);
				}
				//set bar length to longer one (mana or health) - not really clean solution
				if (ether_str_x_len > health_str_x_len) {
					bar_x_len = ether_str_x_len;
				} else {
					bar_x_len = health_str_x_len;
				}

				if (display_hp || display_ether) {
					float hp_off=(bar_x_len - health_str_x_len)/2.0;
					float eth_off=(bar_x_len - ether_str_x_len)/2.0;
					float disp;
					disp=(bar_x_len/2.0);

					if(display_health_bar){
						hp_off+=5.0+disp;
					}
					if(display_ether_bar){
						eth_off+=5.0+disp;
					}
					if (display_hp && (disp+hp_off > banner_width)) {
						banner_width = disp + hp_off;
					}
					if (display_ether && (disp+eth_off > banner_width)) {
						banner_width = disp + eth_off;
					}

					if (display_hp) {
						//choose color for the health
						set_health_color(actor_id, (float)actor_id->cur_health/(float)actor_id->max_health, 1.0f, 1.0f);
						draw_ortho_ingame_string(hx - disp + hp_off, health_bot_y,
							hz, hp, 1, NAME_FONT, stat_font_size, stat_font_size);
					}

					if (display_ether) {
						set_mana_color((float)your_info.ethereal_points.cur / (float)your_info.ethereal_points.base, 1.0f, 1.0f);
						draw_ortho_ingame_string(hx - disp + eth_off, ether_bot_y,
							hz, mana, 1, NAME_FONT, stat_font_size, stat_font_size);
					}
				}
			}
		}
	}

	//draw the health bar
	glDisable(GL_TEXTURE_2D);

	if(display_health_bar && display_health_line && (!actor_id->dead) && (actor_id->kind_of_actor != NPC)){
		float percentage = (float)actor_id->cur_health/(float)actor_id->max_health;
		float off;
		float top_y = health_bot_y + bar_y_len / 3.0;
		float bot_y = health_bot_y + 2 * bar_y_len / 3.0;

		if(percentage>1.1f) //deal with massive bars by trimming at 110%
			percentage = 1.1f;
		if (display_hp){
			off = bar_x_len + 5.0f;
		} else {
			off = bar_x_len / 2.0f;
		}

		if(actor_id->last_health_loss && cur_time-actor_id->last_health_loss<1000){//only when using floatingmessages
			if(actor_id->damage>0){
				healthbar_x_len_converted=bar_x_len*percentage;
				healthbar_x_len_loss=bar_x_len*(float)((float)actor_id->damage/(float)actor_id->max_health);
				healthbar_x_loss_fade=1.0f-((float)(cur_time-actor_id->last_health_loss)/1000.0f);
			} else {
				healthbar_x_len_converted=bar_x_len*(float)((float)(actor_id->cur_health+actor_id->damage)/(float)actor_id->max_health);
				healthbar_x_len_loss=bar_x_len*(float)((float)(-actor_id->damage)/(float)actor_id->max_health);
				healthbar_x_loss_fade=((float)(cur_time-actor_id->last_health_loss)/1000.0f);
			}
		} else {
			healthbar_x_len_converted=bar_x_len*percentage;
			actor_id->last_health_loss=0;
		}

		if (bar_x_len / 2.0f > banner_width) {
			banner_width = bar_x_len / 2.0f;
		}

		hx-=off;

		//choose tint color
		set_health_color(actor_id, percentage, 0.5f, 1.0f);
		glBegin(GL_QUADS);
			glVertex3d(hx, top_y, hz);
			glVertex3d(hx + healthbar_x_len_converted, top_y, hz);

		set_health_color(actor_id, percentage, 1.0f, 1.0f);

			glVertex3d(hx + healthbar_x_len_converted, bot_y, hz);
			glVertex3d(hx, bot_y, hz);
		glEnd();

		if(healthbar_x_len_loss){
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			set_health_color(actor_id, percentage, 0.5f, healthbar_x_loss_fade);

			glBegin(GL_QUADS);
				glVertex3d(hx + healthbar_x_len_converted, top_y, hz);
				glVertex3d(hx + healthbar_x_len_converted + healthbar_x_len_loss, top_y, hz);

			set_health_color(actor_id, percentage, 1.0f, healthbar_x_loss_fade);

				glVertex3d(hx + healthbar_x_len_converted + healthbar_x_len_loss, bot_y, hz);
				glVertex3d(hx + healthbar_x_len_converted, bot_y, hz);
			glEnd();

			glDisable(GL_BLEND);
		}

		//draw the frame
		glDepthFunc(GL_LEQUAL);
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_LOOP);
			glVertex3f(hx - 1.0, top_y - 1.0, hz);
			glVertex3f(hx + bar_x_len + 1.0, top_y - 1.0, hz);
			glVertex3f(hx + bar_x_len + 1.0, bot_y + 1.0, hz);
			glVertex3f(hx - 1.0, bot_y + 1.0, hz);
		glEnd();

		hx+=off;
	}

	if (display_ether_bar && display_ether_line) {
		float percentage = (float)your_info.ethereal_points.cur / (float)your_info.ethereal_points.base;
		float off;
		float top_y = ether_bot_y + bar_y_len / 3.0;
		float bot_y = ether_bot_y + 2 * bar_y_len / 3.0;
		if(percentage>1.1f) // limit bar length to +10%
			percentage = 1.1f;
		if (display_ether){
			off = bar_x_len + 5.0f;
		} else {
			off = bar_x_len / 2.0f;
		}
		if (bar_x_len / 2.0f > banner_width) {
			banner_width = bar_x_len / 2.0f;
		}
		hx-=off;

		set_mana_color(percentage, 0.5f, 1.0f);
		etherbar_x_len_converted = percentage * bar_x_len;
		glBegin(GL_QUADS);
			glVertex3d(hx, top_y, hz);
			glVertex3d(hx + etherbar_x_len_converted, top_y, hz);

		set_mana_color(percentage, 1.0f, 1.0f);

			glVertex3d(hx + etherbar_x_len_converted, bot_y, hz);
			glVertex3d(hx, bot_y, hz);
		glEnd();
		set_health_color(actor_id, percentage, 1.0f, 1.0f);
		glDepthFunc(GL_LEQUAL);
		glColor3f (0.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_LOOP);
			glVertex3f (hx - 1.0, top_y - 1.0 , hz);
			glVertex3f (hx + bar_x_len + 1.0, top_y - 1.0, hz);
			glVertex3f (hx + bar_x_len + 1.0, bot_y + 1.0, hz);
			glVertex3f (hx - 1.0, bot_y + 1.0, hz);
		glEnd();
		hx+=off;
	}

	// draw the alpha background (if ness)
	y_bottom = ((ether_bot_y > 0) ?ether_bot_y :((health_bot_y > 0) ?health_bot_y : name_bot_y)) - 0.2 * bar_y_len;
	y_top = ((name_bot_y > 0) ?name_bot_y :((health_bot_y > 0) ?health_bot_y : ether_bot_y)) + 1.2 * bar_y_len;
	if (display_banner_alpha && banner_width > 0) {
		//if banner width > 0 there MUST be something displayed in the banner
		banner_width += name_zoom * 3;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_SRC_ALPHA);
		glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
		glBegin(GL_QUADS);
			glVertex3f(hx-banner_width, y_bottom, hz + 0.0001);
			glVertex3f(hx+banner_width, y_bottom, hz + 0.0001);
			glVertex3f(hx+banner_width, y_top, hz + 0.0001);
			glVertex3f(hx-banner_width, y_top, hz + 0.0001);
		glEnd();
		glDisable(GL_BLEND);
	}

	glEnable(GL_TEXTURE_2D);

	if ((actor_id->current_displayed_text_time_left>0)&&(actor_id->current_displayed_text[0] != 0)){
		draw_actor_overtext(actor_id, hx, y_top, hz);
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	if(floatingmessages_enabled)drawactor_floatingmessages(actor_id->actor_id, healthbar_z);

	/* set cm_mouse_over_banner true if the mouse is over your banner, or a box where it might be */
	if (actor_id->actor_id == yourself)
	{
		/* use the same calculation as for the alpha background but have a fallback if no banner shown */
		int xoff = (banner_width > 0) ?banner_width: 60;
		int yoff = (banner_width > 0) ?y_top - y_bottom: bar_y_len;
		if ((mouse_x > hx-xoff) && (mouse_x < hx+xoff) &&
			((window_height - mouse_y) > y_bottom) && ((window_height - mouse_y) < (y_bottom + yoff)))
			cm_mouse_over_banner = 1;
		else
			cm_mouse_over_banner = 0;
	}

	glColor3f(1,1,1);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


static void draw_bubble(float x_left, float x_right, float x_leg_left, float x_leg_right,
	float y_top, float y_bottom, float y_actor, float z, float r)
{
	const float mul = M_PI/180.0f;
	int angle;

	glEnable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
	glBlendFunc(GL_NONE, GL_SRC_ALPHA);

	glBegin(GL_POLYGON);
	for (angle = 0; angle <= 90; angle += 10)
	{
		float rad = mul * angle;
		glVertex3f(x_right - r + cos(rad)*r, y_top - r + sin(rad)*r, z);
	}

	for (angle = 90;angle <= 180; angle += 10)
	{
		float rad = mul*angle;
		glVertex3f(x_left + r + cos(rad)*r, y_top - r + sin(rad)*r, z);
	}

	for(angle = 180; angle < 270; angle += 10)
	{
		float rad = mul*angle;
		glVertex3f(x_left + r + cos(rad)*r, y_bottom + r + sin(rad)*r, z);
	}
	// Explicitly include the vertices where the leg attaches to the bubble, to prevent a gap
	// between the bubble and the leg
	glVertex3f(x_leg_left, y_bottom, z);
	glVertex3f(x_leg_right, y_bottom, z);

	for (angle = 270; angle <= 360; angle += 10)
	{
		float rad = mul*angle;
		glVertex3f(x_right - r + cos(rad)*r, y_bottom + r + sin(rad)*r, z);
	}
	glEnd(); // GL_POLYGON

	glBegin(GL_TRIANGLES);
		glVertex3f(x_leg_left, y_bottom, z);
		glVertex3f(x_leg_right, y_actor, z);
		glVertex3f(x_leg_right, y_bottom, z);
	glEnd();

	glDisable(GL_BLEND);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

//-- Logan Dugenoux [5/26/2004]
static void draw_actor_overtext(actor* actor_ptr, double x, double y, double z)
{
	int lines = min2i(actor_ptr->current_displayed_text_lines, MAX_CURRENT_DISPLAYED_TEXT_LINES);
	float font_scale = 0.14f / ALT_INGAME_FONT_X_LEN;
	float x_left, x_right, x_leg_left, x_leg_right, y_top, y_bottom;
	int text_width, line_height, text_height;
	float margin;

	actor *me = get_our_actor();
	if (me && me != actor_ptr)
	{
		const float s_rx = sin(rx * M_PI / 180);
		const float c_rx = cos(rx * M_PI / 180);
		const float s_rz = sin(rz * M_PI / 180);
		const float c_rz = cos(rz * M_PI / 180);
		float cam_x = me->x_pos + zoom_level*camera_distance * s_rx * s_rz;
		float cam_y = me->y_pos + zoom_level*camera_distance * s_rx * c_rz;
		float cam_z = me->z_pos + zoom_level*camera_distance * c_rx;
		double dx, dy, dz, actor_dsq, me_dsq;
		dx = actor_ptr->x_pos - cam_x;
		dy = actor_ptr->y_pos - cam_y;
		dz = actor_ptr->z_pos - cam_z;
		actor_dsq = dx*dx + dy*dy + dz*dz;
		me_dsq = zoom_level*zoom_level * camera_distance*camera_distance;
		font_scale *= 0.5 + 0.5 * me_dsq / actor_dsq;
	}

	get_buf_dimensions((const unsigned char*)actor_ptr->current_displayed_text,
		strlen(actor_ptr->current_displayed_text),
		CHAT_FONT, font_scale, &text_width, &text_height);
	line_height = get_line_height(CHAT_FONT, font_scale);

	margin = 0.5 * line_height;

	x_left = x - 0.5*text_width - margin;
	x_right = x + 0.5*text_width + margin;
	x_leg_left = x - margin;
	x_leg_right = x;

	y_bottom = y + 1.5*margin;
	y_top = y_bottom + text_height + 2*margin;

	glDisable(GL_TEXTURE_2D);
	draw_bubble(x_left, x_right, x_leg_left, x_leg_right, y_top, y_bottom, y, z+0.0001f, 1.5*margin);
	glEnable(GL_TEXTURE_2D);

	//---
	// Draw text

	draw_ortho_ingame_string(x - 0.5f * text_width, y_bottom + margin + text_height - line_height, z,
		(const unsigned char*)actor_ptr->current_displayed_text, lines,
		CHAT_FONT, font_scale, font_scale);

	//-- decrease display time
	actor_ptr->current_displayed_text_time_left -= (cur_time-last_time);
	if (actor_ptr->current_displayed_text_time_left <= 0)
	{	// clear if needed
		actor_ptr->current_displayed_text_time_left = 0;
		actor_ptr->current_displayed_text_lines = 0;
		actor_ptr->current_displayed_text[0] = '\0';
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_actor_without_banner(actor * actor_id, Uint32 use_lightning, Uint32 use_textures, Uint32 use_glow)
{
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	//if first person, dont draw actor
	actor *me = get_our_actor();
	if (me&&me->actor_id==actor_id->actor_id&&first_person) return;
	if (use_textures)
	{
		if (actor_id->is_enhanced_model)
		{
			if (bind_actor_texture(actor_id->texture_id, &actor_id->has_alpha) == 0)
			{
				return;
			}
		}
		else
		{
			if (!actor_id->remapped_colors)
			{
				bind_texture(actor_id->texture_id);
			}
			else
			{
				if (bind_actor_texture(actor_id->texture_id, &actor_id->has_alpha) == 0)
				{
					return;
				}
			}
		}
	}

	glPushMatrix();//we don't want to affect the rest of the scene

	x_pos = actor_id->x_pos;
	y_pos = actor_id->y_pos;
	z_pos = actor_id->z_pos;

	if (z_pos == 0.0f)
	{
		//actor is walking, as opposed to flying, get the height underneath
		z_pos = get_tile_height(actor_id->x_tile_pos, actor_id->y_tile_pos);
	}

	x_rot = actor_id->x_rot;
	y_rot = actor_id->y_rot;
	z_rot = 180 - actor_id->z_rot;

	glTranslatef(x_pos + 0.25f, y_pos + 0.25f, z_pos);

	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if (actor_id->attached_actor >= 0)
		glTranslatef(actor_id->attachment_shift[0], actor_id->attachment_shift[1], actor_id->attachment_shift[2]);

	if (use_animation_program)
	{
		cal_render_actor_shader(actor_id, use_lightning, use_textures, use_glow);
	}
	else
	{
		cal_render_actor(actor_id, use_lightning, use_textures, use_glow);
	}

	//now, draw their damage & nametag
	glPopMatrix();  // restore the matrix

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static __inline__ void draw_actor_banner_new(actor * actor_id)
{
	float x_pos, y_pos, z_pos;
	float healthbar_z;

	healthbar_z = actor_id->max_z + 0.2;

	glPushMatrix();//we don't want to affect the rest of the scene

	x_pos = actor_id->x_pos;
	y_pos = actor_id->y_pos;
	z_pos = actor_id->z_pos;

	if (z_pos == 0.0f)
	{
		//actor is walking, as opposed to flying, get the height underneath
		z_pos = get_tile_height(actor_id->x_tile_pos, actor_id->y_tile_pos);
	}

	glTranslatef(x_pos + 0.25f, y_pos + 0.25f, z_pos);

	if (actor_id->attached_actor >= 0)
	{
		glRotatef(180 - actor_id->z_rot, 0.0f, 0.0f, 1.0f);
		glTranslatef(actor_id->attachment_shift[0], actor_id->attachment_shift[1], actor_id->attachment_shift[2]);
		glRotatef(180 - actor_id->z_rot, 0.0f, 0.0f, -1.0f);
	}

	glRotatef(-rz, 0.0f, 0.0f, 1.0f);

	draw_actor_banner(actor_id, healthbar_z);

	glPopMatrix();	//we don't want to affect the rest of the scene
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static int comp_actors(const void *in_a, const void *in_b)
{
	near_actor *a, *b;
	int at, bt;

	a = (near_actor *)in_a;
	b = (near_actor *)in_b;

	at = a->type;
	bt = b->type;

	if (at < bt)
	{
		return (-1);
	}
	else
	{
		if (at == bt)
		{
			return (0);
		}
		else
		{
			return (1);
		}
	}
}

void get_actors_in_range()
{
	VECTOR3 pos;
	unsigned int i;
#ifdef NEW_SOUND
	unsigned int tmp_nr_enh_act;		// Use temp variables to stop crowd sound interference during count
	float tmp_dist_to_nr_enh_act;
#endif // NEW_SOUND
	actor *me;
	AABBOX bbox;

	me = get_our_actor ();

	if (!me) return;

	no_near_actors = 0;
#ifdef NEW_SOUND
	tmp_nr_enh_act = 0;
	tmp_dist_to_nr_enh_act = 0;
#endif // NEW_SOUND

	set_current_frustum(get_cur_intersect_type(main_bbox_tree));

	for (i = 0; i < max_actors; i++)
	{
		if(actors_list[i]
#ifdef CLUSTER_INSIDES
		   && (actors_list[i]->cluster == me->cluster || actors_list[i]->cluster == 0)
#endif
		)
		{
			// if we have an attached actor, we maybe have to modify the position of the current actor
			if (actors_list[i]->attached_actor >= 0)
			{
				actor *att = actors_list[actors_list[i]->attached_actor];
				attachment_props *att_props;
				float loc_pos[3];
				float att_pos[3];
				float loc_scale = get_actor_scale(actors_list[i]);
				float att_scale = get_actor_scale(att);
				if (actors_list[i]->actor_id < 0) // we are on a attached actor
				{
					att_props = &attached_actors_defs[actors_list[i]->actor_type].actor_type[att->actor_type];
					if (!att_props->is_holder) // the attachment is not a holder so we have to move it
					{
						cal_get_actor_bone_local_position(att, att_props->parent_bone_id, NULL, att_pos);
						cal_get_actor_bone_local_position(actors_list[i], att_props->local_bone_id, NULL, loc_pos);
						actors_list[i]->attachment_shift[0] = att_pos[0] * att_scale - (loc_pos[0] - att_props->shift[0]) * loc_scale;
						actors_list[i]->attachment_shift[1] = att_pos[1] * att_scale - (loc_pos[1] - att_props->shift[1]) * loc_scale;
						actors_list[i]->attachment_shift[2] = att_pos[2] * att_scale - (loc_pos[2] - att_props->shift[2]) * loc_scale;
					}
				}
				else if (actors_list[i]->actor_id >= 0) // we are on a standard actor
				{
					att_props = &attached_actors_defs[att->actor_type].actor_type[actors_list[i]->actor_type];
					if (att_props->is_holder) // the attachment is an holder, we have to move the current actor
					{
						cal_get_actor_bone_local_position(att, att_props->local_bone_id, NULL, att_pos);
						cal_get_actor_bone_local_position(actors_list[i], att_props->parent_bone_id, NULL, loc_pos);
						actors_list[i]->attachment_shift[0] = att_pos[0] * att_scale - (loc_pos[0] - att_props->shift[0]) * loc_scale;
						actors_list[i]->attachment_shift[1] = att_pos[1] * att_scale - (loc_pos[1] - att_props->shift[1]) * loc_scale;
						actors_list[i]->attachment_shift[2] = att_pos[2] * att_scale - (loc_pos[2] - att_props->shift[2]) * loc_scale;
					}
				}
			}
			pos[X] = actors_list[i]->x_pos + actors_list[i]->attachment_shift[X];
			pos[Y] = actors_list[i]->y_pos + actors_list[i]->attachment_shift[Y];
			pos[Z] = actors_list[i]->z_pos + actors_list[i]->attachment_shift[Z];

			if (pos[Z] == 0.0f)
			{
				//actor is walking, as opposed to flying, get the height underneath
				pos[Z] = get_tile_height(actors_list[i]->x_tile_pos, actors_list[i]->y_tile_pos);
			}

			if (actors_list[i]->calmodel == NULL) continue;

			memcpy(&bbox, &actors_list[i]->bbox, sizeof(AABBOX));
			rotate_aabb(&bbox, actors_list[i]->x_rot, actors_list[i]->y_rot, 180.0f-actors_list[i]->z_rot);

			VAddEq(bbox.bbmin, pos);
			VAddEq(bbox.bbmax, pos);

			if (aabb_in_frustum(bbox))
			{
				near_actors[no_near_actors].actor = i;
				near_actors[no_near_actors].ghost = actors_list[i]->ghost;
				near_actors[no_near_actors].buffs = actors_list[i]->buffs;
				near_actors[no_near_actors].select = 0;
				near_actors[no_near_actors].type = actors_list[i]->actor_type;
				if (actors_list[i]->ghost)
				{
					near_actors[no_near_actors].alpha = 0;
				}
				else
				{
					near_actors[no_near_actors].alpha =
						actors_list[i]->has_alpha;
				}

				actors_list[i]->max_z = actors_list[i]->bbox.bbmax[Z];

				if (read_mouse_now && (get_cur_intersect_type(main_bbox_tree) == INTERSECTION_TYPE_DEFAULT))
				{
					near_actors[no_near_actors].select = 1;
				}
				no_near_actors++;
#ifdef NEW_SOUND
				if (actors_list[i]->is_enhanced_model && actors_list[i]->actor_id != me->actor_id)
				{
					tmp_nr_enh_act++;
					tmp_dist_to_nr_enh_act += ((me->x_pos - actors_list[i]->x_pos) *
														(me->x_pos - actors_list[i]->x_pos)) +
														((me->y_pos - actors_list[i]->y_pos) *
														(me->y_pos - actors_list[i]->y_pos));
				}
#endif // NEW_SOUND
			}
		}
	}
#ifdef NEW_SOUND
	if (tmp_nr_enh_act > 0)
		tmp_dist_to_nr_enh_act = tmp_dist_to_nr_enh_act / tmp_nr_enh_act;
	no_near_enhanced_actors = tmp_nr_enh_act;
	distanceSq_to_near_enhanced_actors = tmp_dist_to_nr_enh_act;
#endif // NEW_SOUND
	qsort(near_actors, no_near_actors, sizeof(near_actor), comp_actors);
}

void display_actors(int banner, int render_pass)
{
	Sint32 i, has_alpha, has_ghosts;
	Uint32 use_lightning = 0, use_textures = 0;

	get_actors_in_range();

	glEnable(GL_CULL_FACE);

	if (use_animation_program)
	{
		set_actor_animation_program(render_pass, 0);
	}

	switch (render_pass)
	{
		case DEFAULT_RENDER_PASS:
		case SHADOW_RENDER_PASS:
		case REFLECTION_RENDER_PASS:
			use_lightning = 1;
			use_textures = 1;
			break;
		case DEPTH_RENDER_PASS:
			use_lightning = 0;
			use_textures = 1;
			break;
	}

	has_alpha = 0;
	has_ghosts = 0;

#ifdef	FSAA
	if (fsaa > 1)
	{
		glEnable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */
	for (i = 0; i < no_near_actors; i++)
	{
		if (near_actors[i].ghost || (near_actors[i].buffs & BUFF_INVISIBILITY))
		{
			if ((render_pass == DEFAULT_RENDER_PASS) ||
				(render_pass == SHADOW_RENDER_PASS))
			{
				has_ghosts = 1;
			}
		}
		else if (near_actors[i].alpha)
		{
			has_alpha = 1;
		}
		else
		{
			actor *cur_actor = actors_list[near_actors[i].actor];
			if (cur_actor)
			{
				draw_actor_without_banner(cur_actor, use_lightning, use_textures, 1);
				if (near_actors[i].select)
				{
					if (cur_actor->kind_of_actor == NPC)
					{
						anything_under_the_mouse(near_actors[i].actor, UNDER_MOUSE_NPC);
					}
					else
					{
						if ((cur_actor->kind_of_actor == HUMAN) ||
							(cur_actor->kind_of_actor == COMPUTER_CONTROLLED_HUMAN) ||
							(cur_actor->is_enhanced_model &&
							((cur_actor->kind_of_actor == PKABLE_HUMAN) ||
							(cur_actor->kind_of_actor == PKABLE_COMPUTER_CONTROLLED))))
						{
							anything_under_the_mouse(near_actors[i].actor, UNDER_MOUSE_PLAYER);
						}
						else
						{
							anything_under_the_mouse(near_actors[i].actor, UNDER_MOUSE_ANIMAL);
						}
					}
				}
			}
		}
	}
	if (has_alpha)
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.4f);
		for (i = 0; i < no_near_actors; i++)
		{

			if (near_actors[i].alpha && !(near_actors[i].ghost || (near_actors[i].buffs & BUFF_INVISIBILITY)))
			{

				actor *cur_actor = actors_list[near_actors[i].actor];
				if (cur_actor)
				{
					draw_actor_without_banner(cur_actor, use_lightning, 1, 1);

					if (near_actors[i].select)
					{
						if (cur_actor->kind_of_actor == NPC)
						{
							anything_under_the_mouse(near_actors[i].actor, UNDER_MOUSE_NPC);
						}
						else
						{
							if ((cur_actor->kind_of_actor == HUMAN) ||
								(cur_actor->kind_of_actor == COMPUTER_CONTROLLED_HUMAN) ||
								(cur_actor->is_enhanced_model &&
								 ((cur_actor->kind_of_actor == PKABLE_HUMAN) ||
								 (cur_actor->kind_of_actor == PKABLE_COMPUTER_CONTROLLED))))
							{
								anything_under_the_mouse(near_actors[i].actor, UNDER_MOUSE_PLAYER);
							}
							else
							{
								anything_under_the_mouse(near_actors[i].actor, UNDER_MOUSE_ANIMAL);
							}
						}
					}
				}
			}
		}
		glDisable(GL_ALPHA_TEST);
	}
	if (has_ghosts)
	{
		glEnable(GL_BLEND);
		glDisable(GL_LIGHTING);
		if (use_animation_program)
		{
			set_actor_animation_program(render_pass, 1);
		}

		for (i = 0; i < no_near_actors; i++)
		{

			if (near_actors[i].ghost || (near_actors[i].buffs & BUFF_INVISIBILITY))
			{

				actor *cur_actor = actors_list[near_actors[i].actor];
				if (cur_actor)
				{
					//if any ghost has a glowing weapon, we need to reset the blend function each ghost actor.
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

					if (!use_animation_program)
					{
						if ((near_actors[i].buffs & BUFF_INVISIBILITY))
						{
							glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
						}
						else
						{
							glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
						}
					}

					draw_actor_without_banner(cur_actor, use_lightning, use_textures, 1);

					if (near_actors[i].select)
					{
						if (cur_actor->kind_of_actor == NPC)
						{
							anything_under_the_mouse(near_actors[i].actor, UNDER_MOUSE_NPC);
						}
						else
						{
							if ((cur_actor->kind_of_actor == HUMAN) ||
								(cur_actor->kind_of_actor == COMPUTER_CONTROLLED_HUMAN) ||
								(cur_actor->is_enhanced_model &&
								 ((cur_actor->kind_of_actor == PKABLE_HUMAN) ||
								 (cur_actor->kind_of_actor == PKABLE_COMPUTER_CONTROLLED))))
							{
								anything_under_the_mouse(near_actors[i].actor, UNDER_MOUSE_PLAYER);
							}
							else
							{
								anything_under_the_mouse(near_actors[i].actor, UNDER_MOUSE_ANIMAL);
							}
						}
					}
				}
			}
		}
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
	}
#ifdef	FSAA
	if (fsaa > 1)
	{
		glDisable(GL_MULTISAMPLE);
	}
#endif	/* FSAA */

	if (use_animation_program)
	{
		disable_actor_animation_program();
	}

	if (banner)
	{
		if (use_shadow_mapping)
		{
			glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(shadow_unit);
			glDisable(depth_texture_target);
			disable_texgen();
			ELglActiveTextureARB(GL_TEXTURE0);
			glEnable(GL_TEXTURE_2D);
			glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}

		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);

		for (i = 0; i < no_near_actors; i++)
		{
			actor *cur_actor = actors_list[near_actors[i].actor];
			if (cur_actor
				&& cur_actor->actor_id >= 0
				)
			{
				draw_actor_banner_new(cur_actor);
			}
		}

		if (use_shadow_mapping)
		{
			last_texture = -1;
			glPopAttrib();
		}

		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


void add_actor_from_server (const char *in_data, int len)
{
	short actor_id;
	Uint32 buffs = 0;
	short x_pos;
	short y_pos;
	short z_rot;
	short max_health;
	short cur_health;
	short actor_type;
	Uint8 frame;
	int i;
	int dead=0;
	int kind_of_actor;

	double f_x_pos,f_y_pos,f_z_rot;
	float scale= 1.0f;
	emote_data *pose=NULL;
	int attachment_type = -1;

	actor_id=SDL_SwapLE16(*((short *)(in_data)));
#ifndef EL_BIG_ENDIAN
	buffs=((*((char *)(in_data+3))>>3)&0x1F) | (((*((char*)(in_data+5))>>3)&0x1F)<<5);	// Strip the last 5 bits of the X and Y coords for the buffs
	x_pos=*((short *)(in_data+2)) & 0x7FF;
	y_pos=*((short *)(in_data+4)) & 0x7FF;
#else
	buffs=((SDL_SwapLE16(*((char*)(in_data+3)))>>3)&0x1F | (SDL_SwapLE16(((*((char*)(in_data+5)))>>3)&0x1F)<<5));	// Strip the last 5 bits of the X and Y coords for the buffs
	x_pos=SDL_SwapLE16(*((short *)(in_data+2))) & 0x7FF;
	y_pos=SDL_SwapLE16(*((short *)(in_data+4))) & 0x7FF;
#endif //EL_BIG_ENDIAN
	buffs |= (SDL_SwapLE16(*((short *)(in_data+6))) & 0xFF80) << 3; // we get the 9 MSB for the buffs and leave the 7 LSB for a further use
	z_rot=SDL_SwapLE16(*((short *)(in_data+8)));
	actor_type=*(in_data+10);

	frame=*(in_data+11);
	max_health=SDL_SwapLE16(*((short *)(in_data+12)));
	cur_health=SDL_SwapLE16(*((short *)(in_data+14)));
	kind_of_actor=*(in_data+16);
	if(len > 17+(int)strlen(in_data+17)+2){
		scale=((float)SDL_SwapLE16(*((short *)(in_data+17+strlen(in_data+17)+1)))/((float)ACTOR_SCALE_BASE));

		if(len > 17+(int)strlen(in_data+17)+3)
			attachment_type = (unsigned char)in_data[17+strlen(in_data+17)+3];
	}

	if(actor_type < 0 || actor_type >= MAX_ACTOR_DEFS || (actor_type > 0 && actors_defs[actor_type].actor_type != actor_type) ){
		LOG_ERROR("Illegal/missing actor definition %d", actor_type);
	}

	//translate from tile to world
	f_x_pos=x_pos*0.5;
	f_y_pos=y_pos*0.5;
	f_z_rot=z_rot;

	//get the current frame
	switch(frame) {
	case frame_walk:
	case frame_run:
		break;
	case frame_die1:
	case frame_die2:
		dead=1;
		break;
	case frame_pain1:
	case frame_pain2:
	case frame_pick:
	case frame_drop:
	case frame_idle:
	case frame_sit_idle:
	case frame_harvest:
	case frame_cast:
	case frame_attack_up_1:
	case frame_attack_up_2:
	case frame_attack_up_3:
	case frame_attack_up_4:
	case frame_attack_up_5:
	case frame_attack_up_6:
	case frame_attack_up_7:
	case frame_attack_up_8:
	case frame_attack_up_9:
	case frame_attack_up_10:
	case frame_attack_down_1:
	case frame_attack_down_2:
	case frame_attack_down_3:
	case frame_attack_down_4:
	case frame_attack_down_5:
	case frame_attack_down_6:
	case frame_attack_down_7:
	case frame_attack_down_8:
	case frame_attack_down_9:
	case frame_attack_down_10:
	case frame_combat_idle:
		break;
	default:
		{
		if(frame>=frame_poses_start&&frame<=frame_poses_end) {
			//we have a pose, get it! (frame is the emote_id)
			hash_entry *he;
			he=hash_get(emotes,(void*)(uintptr_t)frame);
			if(!he) LOG_ERROR("unknown pose %d", frame);
			else pose = he->item;
			break;
		}

			LOG_ERROR("%s %d - %s\n", unknown_frame, frame, &in_data[17]);
		}
	}

#ifdef EXTRA_DEBUG
	ERR();
#endif

	//find out if there is another actor with that ID
	//ideally this shouldn't happen, but just in case

	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				if(actors_list[i]->actor_id==actor_id)
					{
						LOG_ERROR(duplicate_actors_str,actor_id, actors_list[i]->actor_name, &in_data[17]);
						destroy_actor(actors_list[i]->actor_id);//we don't want two actors with the same ID
						i--;// last actor was put here, he needs to be checked too
					}
		}

	i= add_actor(actor_type, actors_defs[actor_type].skin_name, f_x_pos, f_y_pos, 0.0, f_z_rot, scale, 0, 0, 0, 0, 0, 0, 0, actor_id);

	if(i==-1)
	{
		UNLOCK_ACTORS_LISTS();
		return;//A nasty error occured and we couldn't add the actor. Ignore it.
	}

	//The actors list is locked when we get here...

	actors_list[i]->async_fighting = 0;
	actors_list[i]->async_x_tile_pos = x_pos;
	actors_list[i]->async_y_tile_pos = y_pos;
	actors_list[i]->async_z_rot = z_rot;

	actors_list[i]->x_tile_pos=x_pos;
	actors_list[i]->y_tile_pos=y_pos;
	actors_list[i]->buffs=buffs;
	actors_list[i]->actor_type=actor_type;
	actors_list[i]->damage=0;
	actors_list[i]->damage_ms=0;
	actors_list[i]->sitting=0;
	actors_list[i]->fighting=0;
	//test only
	actors_list[i]->max_health=max_health;
	actors_list[i]->cur_health=cur_health;

    actors_list[i]->step_duration = actors_defs[actor_type].step_duration;
	if (actors_list[i]->buffs & BUFF_DOUBLE_SPEED)
		actors_list[i]->step_duration /= 2;

	actors_list[i]->z_pos = get_actor_z(actors_list[i]);
	if(frame==frame_sit_idle||(pose!=NULL&&pose->pose==EMOTE_SITTING)){ //sitting pose sent by the server
			actors_list[i]->poses[EMOTE_SITTING]=pose;
			actors_list[i]->sitting=1;
		}
	else if(frame==frame_stand||(pose!=NULL&&pose->pose==EMOTE_STANDING)){//standing pose sent by server
			actors_list[i]->poses[EMOTE_STANDING]=pose;
			actors_list[i]->sitting=0;
		}
	else if(frame==frame_walk||(pose!=NULL&&pose->pose==EMOTE_WALKING)){//walking pose sent by server
			actors_list[i]->poses[EMOTE_WALKING]=pose;
		}
	else if(frame==frame_run||(pose!=NULL&&pose->pose==EMOTE_RUNNING)){//running pose sent by server
			actors_list[i]->poses[EMOTE_RUNNING]=pose;
		}
	else
		{
			if(frame==frame_combat_idle)
				actors_list[i]->fighting=1;
			else if (frame == frame_ranged)
				actors_list[i]->in_aim_mode = 1;
		}
	//ghost or not?
	actors_list[i]->ghost=actors_defs[actor_type].ghost;

	actors_list[i]->dead=dead;
	actors_list[i]->stop_animation=1;//helps when the actor is dead...
	actors_list[i]->kind_of_actor=kind_of_actor;
	if(strlen(&in_data[17]) >= 30)
		{
			LOG_ERROR("%s (%d): %s/%d\n", bad_actor_name_length, actors_list[i]->actor_type,&in_data[17], (int)strlen(&in_data[17]));
		}
	else safe_strncpy(actors_list[i]->actor_name,&in_data[17],30);

	if (attachment_type >= 0)
		add_actor_attachment(actor_id, attachment_type);

	if (actors_defs[actor_type].coremodel!=NULL) {
		//Setup cal3d model
		actors_list[i]->calmodel = model_new(actors_defs[actor_type].coremodel);
		//Attach meshes
		if(actors_list[i]->calmodel){
			model_attach_mesh(actors_list[i], actors_defs[actor_type].shirt[0].mesh_index);
			if(dead){
				cal_actor_set_anim(i, actors_defs[actors_list[i]->actor_type].cal_frames[cal_actor_die1_frame]);
				actors_list[i]->stop_animation=1;
				CalModel_Update(actors_list[i]->calmodel,1000);
			}
            else {
                /* Schmurk: we explicitly go on idle here to avoid weird
                 * flickering when actors appear */
                set_on_idle(i);
                /* CalModel_Update(actors_list[i]->calmodel,0); */
            }
			build_actor_bounding_box(actors_list[i]);
			if (use_animation_program)
			{
				set_transformation_buffers(actors_list[i]);
			}
            /* lines commented by Schmurk: we've set an animation just before
             * so we don't want do screw it up */
			/* actors_list[i]->cur_anim.anim_index=-1; */
			/* actors_list[i]->cur_anim_sound_cookie=0; */
			/* actors_list[i]->IsOnIdle=0; */
		}
	}
	else
	{
		actors_list[i]->calmodel=NULL;
	}
	update_actor_buffs(actor_id, buffs);

	check_if_new_actor_last_summoned(actors_list[i]);

	UNLOCK_ACTORS_LISTS();	//unlock it
#ifdef EXTRA_DEBUG
	ERR();
#endif

}

//--- LoganDugenoux [5/25/2004]
#define MS_PER_CHAR	200
#define MINI_BUBBLE_MS	500
void add_displayed_text_to_actor(actor *actor_ptr, const char* text)
{
	char *dest = actor_ptr->current_displayed_text;
	const size_t size = sizeof(actor_ptr->current_displayed_text);
	safe_snprintf(dest, size, "%s", text);
	actor_ptr->current_displayed_text_lines = reset_soft_breaks((unsigned char*)dest, strlen(dest),
		size, CHAT_FONT, 1.0, window_width/3, NULL, NULL);
	actor_ptr->current_displayed_text_time_left = MINI_BUBBLE_MS + strlen(text) * MS_PER_CHAR;
}

//--- LoganDugenoux [5/25/2004]
actor *	get_actor_ptr_from_id( int actor_id )
{
	int i;
	for (i = 0; i < max_actors; i++)
	{
		if (actors_list[i]->actor_id == actor_id)
			return actors_list[i];
	}
	return NULL;
}

void end_actors_lists()
{
	SDL_DestroyMutex(actors_lists_mutex);
	actors_lists_mutex=NULL;
}


int on_the_move (const actor *act){
	if (act == NULL) return 0;
	return act->moving || (act->que[0] >= move_n && act->que[0] <= move_nw);
}

void get_actor_rotation_matrix(actor *in_act, float *out_rot)
{
	float tmp_rot1[9], tmp_rot2[9];

	MAT3_ROT_Z(out_rot, (180.0 - in_act->z_rot) * (M_PI / 180.0));
	MAT3_ROT_X(tmp_rot1, in_act->x_rot * (M_PI / 180.0));
	MAT3_MULT(tmp_rot2, out_rot, tmp_rot1);
	MAT3_ROT_Y(tmp_rot1, in_act->y_rot * (M_PI / 180.0));
	MAT3_MULT(out_rot, tmp_rot2, tmp_rot1);
}

void transform_actor_local_position_to_absolute(actor *in_act, float *in_local_pos, float *in_act_rot, float *out_pos)
{
	float scale = get_actor_scale(in_act);
	float rot[9];

	if (!in_act_rot)
	{
		get_actor_rotation_matrix(in_act, rot);
		in_act_rot = rot;
	}

	MAT3_VECT3_MULT(out_pos, in_act_rot, in_local_pos);

	out_pos[0] = out_pos[0] * scale + in_act->x_pos + 0.25;
	out_pos[1] = out_pos[1] * scale + in_act->y_pos + 0.25;
	out_pos[2] = out_pos[2] * scale + get_actor_z(in_act);

	if (in_act->attached_actor >= 0)
	{
		float shift[3];
		MAT3_VECT3_MULT(shift, in_act_rot, in_act->attachment_shift);
		out_pos[0] += shift[0];
		out_pos[1] += shift[1];
		out_pos[2] += shift[2];
	}
}

// Split the given string into basic name and, if any, guild parts.
// For players, the size should be at least MAX_ACTOR_NAME
// For non players, the size should be at least ACTOR_NAME_SIZE
// Name format is <name part> then optional <space><colour character><guild past>
static void split_name_and_guild(const char *full_name, char *name_part, char *guild_part, size_t parts_size)
{
		size_t i, ni=0, gi=0;
		if ((full_name == NULL) || (name_part == NULL) || (guild_part == NULL) || (parts_size < strlen(full_name)))
		{
			LOG_ERROR("invalid parameters");
			return;
		}
		name_part[0] = '\0';
		guild_part[0] = '\0';

		// get the name without any guild part
		for (i=0; i<strlen(full_name); ni++, i++)
		{
			if (is_color(full_name[i]))
			{
				name_part[ni - ((ni==0) ?0: 1)] = '\0';
				i++;
				break;
			}
			else
				name_part[ni] = full_name[i];
		}
		name_part[ni] = '\0';

		// get the guild if any
		for (; i<strlen(full_name); gi++, i++)
		{
			guild_part[gi] = full_name[i];
		}
		guild_part[gi] = '\0';
}

// A simple structure to hold state for the last summoned creature
static struct last_summoned
{
	Uint32 summoned_time;
	char summoned_name[256];
	int actor_id;
} last_summoned_var = {0, "", -1};


// Store the time and the name of the last sucessful summons by the player
void remember_new_summoned(const char *summoned_name)
{
	last_summoned_var.summoned_time = SDL_GetTicks();
	safe_strncpy2(last_summoned_var.summoned_name, summoned_name, sizeof(summoned_name), strlen(summoned_name));
	//printf("%u new summoned [%s]\n", last_summoned_var.summoned_time, last_summoned_var.summoned_name);
}

// Check if the new actor ....
// 		has been created close in time to the last sucessful summons
//		has the same name a the last sucessful summons
//		has the same guild (if any) of the player
// Must be called while we have the LOCK_ACTORS_LISTS() lock
void check_if_new_actor_last_summoned(actor *new_actor)
{
	if (SDL_GetTicks() < last_summoned_var.summoned_time + 250)
	{
		actor *me = get_our_actor();
		if (me)
		{
			char me_name_part[MAX_ACTOR_NAME] = "", me_guild_part[MAX_ACTOR_NAME] = "";
			char summoned_name_part[ACTOR_DEF_NAME_SIZE] = "", summoned_guild_part[ACTOR_DEF_NAME_SIZE] = "";

			split_name_and_guild(me->actor_name, me_name_part, me_guild_part, MAX_ACTOR_NAME);
			split_name_and_guild(new_actor->actor_name, summoned_name_part, summoned_guild_part, ACTOR_DEF_NAME_SIZE);

			if ((strcmp(last_summoned_var.summoned_name, summoned_name_part) == 0) &&
					(strcmp(summoned_guild_part, me_guild_part) == 0))
				last_summoned_var.actor_id = new_actor->actor_id;

			//printf("%u %s id=%d wanted [%s/%s] got [%s/%s]\n", SDL_GetTicks(),
			//	((last_summoned_var.actor_id == new_actor->actor_id) ?"MATCHED" : "NO MATCH"), new_actor->actor_id,
			//	last_summoned_var.summoned_name, me_guild_part, summoned_name_part, summoned_guild_part);
		}
	}
}

// Return the id of the last sucessful summoned creature, if its still present
int get_id_last_summoned(void)
{
	size_t i;
	if (last_summoned_var.actor_id < 0)
		return -1;

	// check if the actor is still present
	LOCK_ACTORS_LISTS();
	for (i=0; i<max_actors; i++)
		if (actors_list[i]->actor_id == last_summoned_var.actor_id)
			break;
	UNLOCK_ACTORS_LISTS();

	if (i == max_actors)
		last_summoned_var.actor_id = -1;
	return last_summoned_var.actor_id;
}

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "actor_scripts.h"
#include "actors_list.h"
#include "asc.h"
#include "cal.h"
#include "cal3d_wrapper.h"
#include "counters.h"
#include "cursors.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "errors.h"
#include "elloggingwrapper.h"
#include "gamewin.h"
#include "hud_statsbar_window.h"
#include "interface.h"
#include "missiles.h"
#include "new_actors.h"
#include "multiplayer.h"
#include "new_character.h"
#include "particles.h"
#include "pathfinder.h"
#include "platform.h"
#include "skeletons.h"
#include "special_effects.h"
#ifdef NEW_SOUND
#include "sound.h"
#endif // NEW_SOUND
#include "spells.h"
#include "text.h"
#include "tiles.h"
#include "timers.h"
#include "translate.h"
#include "eye_candy_wrapper.h"
#include "minimap.h"
#include "io/elfilewrapper.h"
#include "io/cal3d_io_wrapper.h"
#include "actor_init.h"
#include "textures.h"

#ifndef EXT_ACTOR_DICT
const dict_elem skin_color_dict[] =
	{ { "brown"	, SKIN_BROWN	},
	  { "normal", SKIN_NORMAL	},
	  { "pale"	, SKIN_PALE		},
	  { "tan"	, SKIN_TAN		},
	  { "darkblue", SKIN_DARK_BLUE },	// Elf's only
	  { "dark_blue", SKIN_DARK_BLUE },	// Elf's only, synonym
	  { "white" , SKIN_WHITE    },		// Draegoni only
	  { NULL	, -1			}
	};

const dict_elem glow_mode_dict[] =
	{ { "none"   , GLOW_NONE    },
	  { "fire"   , GLOW_FIRE    },
	  { "ice"    , GLOW_COLD    },
	  { "thermal", GLOW_THERMAL },
	  { "magic"  , GLOW_MAGIC   },
	  { NULL     , -1           }
	};

const dict_elem head_number_dict[] =
	{ { "1" ,  HEAD_1 },
	  { "2" ,  HEAD_2 },
	  { "3" ,  HEAD_3 },
	  { "4" ,  HEAD_4 },
	  { "5" ,  HEAD_5 },
	  { NULL, -1      }
	};
int actor_part_sizes[ACTOR_NUM_PARTS] = {10, 40, 50, 100, 100, 100, 10, 20, 40, 60, 20, 20};		// Elements according to actor_parts_enum
#else // EXT_ACTOR_DICT
#define MAX_SKIN_COLORS 7
#define MAX_GLOW_MODES 5
#define MAX_HEAD_NUMBERS 5
dict_elem skin_color_dict[MAX_SKIN_COLORS];
dict_elem head_number_dict[MAX_GLOW_MODES];
dict_elem glow_mode_dict[MAX_HEAD_NUMBERS];
int num_skin_colors = 0;
int num_head_numbers = 0;
int num_glow_modes = 0;
int actor_part_sizes[ACTOR_NUM_PARTS];
#endif // EXT_ACTOR_DICT

//Forward declarations
int cal_load_weapon_mesh (actor_types *act, const char *fn, const char *kind);
int cal_load_mesh(actor_types *act, const char *fn, const char *kind);
static void unqueue_cmd(actor *act);
#ifdef NEW_SOUND
int parse_actor_sounds(actor_types *act, const xmlNode *cfg);
#endif	//NEW_SOUND

hash_table *emote_cmds = NULL;
hash_table *emotes = NULL;
int parse_actor_frames(actor_types *act, const xmlNode *cfg, const xmlNode *defaults);


#ifdef MORE_ATTACHED_ACTORS_DEBUG
static int thecount=0;
#endif

static inline int is_actor_held(const actor *act, const actor *attached)
{
	if (!attached)
		return 0;
    return ((act->actor_id >= HORSE_ID_OFFSET // the actor is the attachment
				&& !attached_actors_defs[act->actor_type].actor_type[attached->actor_type].is_holder)
			|| (act->actor_id < HORSE_ID_OFFSET // the actor is the parent of the attachment
				&& attached_actors_defs[attached->actor_type].actor_type[act->actor_type].is_holder)
			);
}

static inline int is_actor_barehanded(actor *act, int hand){
	if(hand==EMOTE_BARE_L)
		return (act->cur_shield==SHIELD_NONE||act->cur_shield==QUIVER_ARROWS||act->cur_shield==QUIVER_BOLTS);
	else
		return (act->cur_weapon==WEAPON_NONE||act->cur_weapon==GLOVE_FUR||act->cur_weapon==GLOVE_LEATHER);
}

// static inline int get_held_actor_motion_frame(actor *act, actor *attached)
// {
// 	if (!attached)
// 		return cal_attached_walk_frame;
// 	else if (attached->buffs & BUFF_DOUBLE_SPEED)
// 		return cal_attached_run_frame;
// 	else
// 		return cal_attached_walk_frame;
// }

static void unfreeze_horse(actor *act, actor* horse)
{
	if (horse->que[0]==wait_cmd)
	{
		//printf("%i, horse out of wait\n",thecount);
		unqueue_cmd(horse);
		horse->busy = 0;
		set_on_idle(horse, act);
	}
}


static void cal_actor_set_random_idle(actor* actor)
{
	struct CalMixer *mixer;
	int i;
	int random_anim;
	int random_anim_index;

	if (actor->calmodel==NULL) return;
	//LOG_TO_CONSOLE(c_green2,"Randomizing");
	//if (actor->cur_anim.anim_index==anim.anim_index) return;
	srand( (unsigned)time( NULL ) );
	mixer=CalModel_GetMixer(actor->calmodel);
	//Stop previous animation if needed
	if (actor->IsOnIdle!=1){
		if ((actor->cur_anim.anim_index!=-1)&&(actor->cur_anim.kind==0)) {
			CalMixer_ClearCycle(mixer,actor->cur_anim.anim_index,0.05);
		}
		if ((actor->cur_anim.anim_index!=-1)&&(actor->cur_anim.kind==1)) {
			CalMixer_RemoveAction(mixer,actor->cur_anim.anim_index);
		}
	}

	for (i=0;i<actors_defs[actor->actor_type].group_count;++i) {
		random_anim=rand()%(actors_defs[actor->actor_type].idle_group[i].count+1);
		if (random_anim<actors_defs[actor->actor_type].idle_group[i].count) random_anim_index=actors_defs[actor->actor_type].idle_group[i].anim[random_anim].anim_index;
		else random_anim_index=-1;
		if (actor->IsOnIdle==1) {
			if (actor->cur_idle_anims[i].anim_index!=random_anim_index)
			CalMixer_ClearCycle(mixer,actor->cur_idle_anims[i].anim_index,2.0);
		}
		if (actor->cur_idle_anims[i].anim_index!=random_anim_index)
			if (random_anim_index>=0) CalMixer_BlendCycle(mixer,random_anim_index,0.5,0.05);
		//safe_snprintf(str, sizeof(str),"%d",random_anim);
		//LOG_TO_CONSOLE(c_green2,str);
		actor->cur_idle_anims[i].anim_index=random_anim_index;
		//anim.anim_index,1.0,0.05);else
	}

	//if (anim.kind==0) CalMixer_BlendCycle(mixer,anim.anim_index,1.0,0.05);else
	//CalMixer_ExecuteAction(mixer,anim.anim_index,0.0,0.0);
	//actor->cur_anim=anim;
	//actor->anim_time=0.0;
	CalModel_Update(actor->calmodel,0.0001);//Make changes take effect now
	build_actor_bounding_box(actor);
	if (use_animation_program)
	{
		set_transformation_buffers(actor);
	}
	actor->IsOnIdle= 1;
	actor->cur_anim.duration= 0;
	actor->anim_time= 0.0;
	actor->last_anim_update= cur_time;
	actor->cur_anim.anim_index= -1;
#ifdef NEW_SOUND
	if (check_sound_loops(actor->cur_anim_sound_cookie))
		stop_sound(actor->cur_anim_sound_cookie);
#endif // NEW_SOUND
	actor->cur_anim_sound_cookie= 0;
	//if (actor->cur_anim.anim_index==-1) actor->busy=0;
}


float unwindAngle_Degrees( float fAngle )
{
	fAngle -= 360.0f * (int)( fAngle / 360.0f );
	if( fAngle < 0.0f )
		{
			fAngle += 360.0f;
		}
	return fAngle;
}


float get_rotation_vector( float fStartAngle, float fEndAngle )
{
	float ccw = unwindAngle_Degrees( fStartAngle - fEndAngle );
	float cw = unwindAngle_Degrees( fEndAngle - fStartAngle );
	if(cw<ccw)return cw;
	else return -ccw;
}

int get_motion_vector(int move_cmd, int *dx, int *dy)
{
	int result = 1;
    switch(move_cmd) {
    case move_n:
    case run_n:
        *dx = 0;
        *dy = 1;
        break;
    case move_s:
    case run_s:
        *dx = 0;
        *dy = -1;
        break;
    case move_e:
    case run_e:
        *dx = 1;
        *dy = 0;
        break;
    case move_w:
    case run_w:
        *dx = -1;
        *dy = 0;
        break;
    case move_ne:
    case run_ne:
        *dx = 1;
        *dy = 1;
        break;
    case move_se:
    case run_se:
        *dx = 1;
        *dy = -1;
        break;
    case move_sw:
    case run_sw:
        *dx = -1;
        *dy = -1;
        break;
    case move_nw:
    case run_nw:
        *dx = -1;
        *dy = 1;
        break;

    default:
        *dx = 0;
        *dy = 0;
		result = 0;
        break;
    }
	return result;
}

#ifdef	ANIMATION_SCALING
static Uint32 update_actor_animation_speed(actor* a, const float time_diff)
{
	float scale, seconds;
	Uint32 i, animations;

	if (a == 0)
	{
		return 0;
	}

	animations = 0;

	for (i = 0; i < MAX_CMD_QUEUE; i++)
	{
		if ((a->que[i] != nothing) && (a->que[i] != wait_cmd))
		{
			animations++;
		}
	}

	scale = a->animation_scale;

	seconds = time_diff / 2000.0f;

	if (animations > 2)
	{
		animations -= 2;
	}
	else
	{
		animations = 1;
	}

	if (scale > animations)
	{
		scale = max2f(scale - seconds, animations);
	}
	else
	{
		if (scale < animations)
		{
			scale = min2f(scale + seconds, animations);
		}
	}

	a->animation_scale = scale;

	return scale * time_diff;
}
#endif	/* ANIMATION_SCALING */

static void animate_actor(actor *act, actor *attached, void* data, locked_list_ptr list)
{
	actor *horse = (attached && is_horse(attached)) ? attached : NULL;
#ifdef	ANIMATION_SCALING
	int actors_time_diff, time_diff;

	actors_time_diff = *((const int*)data);
	time_diff = update_actor_animation_speed(act, actors_time_diff);
#else	/* ANIMATION_SCALING */
    int time_diff = *((const int*)data);
#endif	/* ANIMATION_SCALING */

	if(act->moving)
	{
#ifdef	ANIMATION_SCALING
		int tmp_time_diff = min2i(act->movement_time_left + 40, time_diff);

		act->x_pos += act->move_x_speed * tmp_time_diff;
		act->y_pos += act->move_y_speed * tmp_time_diff;
		act->z_pos += act->move_z_speed * tmp_time_diff;
#else	/* ANIMATION_SCALING */
		if (time_diff <= act->movement_time_left+40)
		{
			act->x_pos += act->move_x_speed*time_diff;
			act->y_pos += act->move_y_speed*time_diff;
			act->z_pos += act->move_z_speed*time_diff;
		}
		else
		{
			act->x_pos += act->move_x_speed*act->movement_time_left;
			act->y_pos += act->move_y_speed*act->movement_time_left;
			act->z_pos += act->move_z_speed*act->movement_time_left;
		}
#endif	/* ANIMATION_SCALING */
		act->movement_time_left -= time_diff;
		if (act->movement_time_left <= 0)
		{
			// We moved all the way
			Uint8 last_command;
			int dx, dy;

			act->moving= 0;	//don't move next time, ok?
			//now, we need to update the x/y_tile_pos, and round off
			//the x/y_pos according to x/y_tile_pos
			last_command= act->last_command;
			//if(HAS_HORSE(i)) {MY_HORSE(i)->busy=0; if(act->actor_id==yourself) printf("%i, %s wakes up Horse\n",thecount, act->actor_name);}
			if (get_motion_vector(last_command, &dx, &dy))
			{
				act->x_tile_pos += dx;
				act->y_tile_pos += dy;

				act->busy = 0;
				//if(act->actor_id==yourself) printf("%i, unbusy(moved)\n", thecount);
				//if(act->actor_id<0) printf("%i, unbusy horse(moved)\n", thecount);

				if (act->que[0] >= move_n && act->que[0] <= move_nw)
				{
					next_command(list);
				}
				else
				{
					act->x_pos= act->x_tile_pos*0.5;
					act->y_pos= act->y_tile_pos*0.5;
					act->z_pos= get_actor_z(act);
				}
			}
			else
			{
				act->busy = 0;
				//if(act->actor_id==yourself) printf("%i, unbusy(moved2)\n", thecount);
				//if(act->actor_id<0) printf("%i, unbusy horse(moved2)\n", thecount);

			}
		}
	} // moving

	if (act->rotating)
	{
#ifdef	ANIMATION_SCALING
		int tmp_time_diff = min2i(act->rotate_time_left, time_diff);
		act->rotate_time_left -= time_diff;

		if (act->rotate_time_left <= 0)
		{
			// We rotated all the way
			act->rotating = 0;//don't rotate next time, ok?
		}
#else	/* ANIMATION_SCALING */
		act->rotate_time_left -= time_diff;
		if (act->rotate_time_left <= 0)
		{
			// We rotated all the way
			act->rotating= 0;//don't rotate next time, ok?
			tmp_time_diff = time_diff + act->rotate_time_left;
/*
#ifdef MORE_ATTACHED_ACTORS
			if(act->actor_id==yourself) printf("%i, rot: %i\n",thecount,act->rotating);
			if(act->actor_id<0) printf("%i, (horse) rot: %i\n",thecount,act->rotating);
#endif
*/
		}
		else
		{
			tmp_time_diff = time_diff;
		}
#endif	/* ANIMATION_SCALING */
		act->x_rot+= act->rotate_x_speed*tmp_time_diff;
		act->y_rot+= act->rotate_y_speed*tmp_time_diff;
		act->z_rot+= act->rotate_z_speed*tmp_time_diff;
		if(act->z_rot >= 360) {
			act->z_rot -= 360;
		} else if (act->z_rot <= 0) {
			act->z_rot += 360;
		}
		//if(act->actor_id==yourself) printf("%i, rotating: z_rot %f,  status %i-%i\n",thecount,act->z_rot,act->rotating,act->moving);
		//if(act->actor_id<0) printf("%i, rotating (horse): z_rot %f,  status %i-%i\n",thecount,act->z_rot,act->rotating,act->moving);
	} // rotating

#ifdef	ANIMATION_SCALING
	act->anim_time += (time_diff*act->cur_anim.duration_scale)/1000.0;
#else	/* ANIMATION_SCALING */
	act->anim_time += ((cur_time-last_update)*act->cur_anim.duration_scale)/1000.0;
#endif	/* ANIMATION_SCALING */
	/*if(act->anim_time>=act->cur_anim.duration) {
		if (HAS_HORSE(i)||IS_HORSE(i)) {
				if(MY_HORSE(i)->anim_time<MY_HORSE(i)->cur_anim.duration) {
					MY_HORSE(i)->anim_time=MY_HORSE(i)->cur_anim.duration;
					printf("%i, ANIMATION FORCED\n",thecount);
				}
		}
	}*/
#ifndef	DYNAMIC_ANIMATIONS
	if (act->calmodel!=NULL)
	{
		//check if emote animation is ended, then remove it
		handle_cur_emote(act);

#ifdef MORE_EMOTES
		if(act->startIdle != act->endIdle)
		{
			if (do_transition(act))
			{
				act->stand_idle = act->sit_idle = 0; //force set_on_idle
				set_on_idle(act, attached);
			}
		}
#endif // MORE_EMOTES
#ifdef	ANIMATION_SCALING
		CalModel_Update(act->calmodel, (time_diff * act->cur_anim.duration_scale) / 1000.0f);
#else	/* ANIMATION_SCALING */
		CalModel_Update(act->calmodel, (((cur_time-last_update)*act->cur_anim.duration_scale)/1000.0));
#endif	/* ANIMATION_SCALING */
		build_actor_bounding_box(act);
		{
			int wasbusy = act->busy;
			missiles_rotate_actor_bones(act);
			if (horse && act->busy != wasbusy)
			{
				//if(act->actor_id==yourself) printf("%i, %s is no more busy due to missiles_rotate_actor_bones!! Setting the horse free...\n",thecount, act->actor_name);
				unfreeze_horse(act, horse);
			}
		}
		if (use_animation_program)
		{
			set_transformation_buffers(act);
		}
	}
#endif // !DYNAMIC_ANIMATIONS
}

void animate_actors()
{
	static int last_update = 0;

	locked_list_ptr list;
	int time_diff = cur_time - last_update;

	// lock the actors_list so that nothing can interfere with this look
	list = get_locked_actors_list();
	for_each_actor_and_attached(list, animate_actor, &time_diff);
	release_locked_actors_list(list);

	last_update = cur_time;
}

static void unqueue_cmd(actor *act)
{
	int k;
	int max_queue=0;
	//move que down with one command
	for(k=0;k<MAX_CMD_QUEUE-1;k++)
	{
		if(k>max_queue && act->que[k]!=nothing)
			max_queue=k;
		act->que[k]=act->que[k+1];
	}
	act->que[k]=nothing;
}

#ifdef MORE_ATTACHED_ACTORS_DEBUG
static void print_queue(actor *act, actor *attached) {
	int k;

	printf("   Actor %s queue:",act->actor_name);
	printf(" -->");
	for(k=0; k<MAX_CMD_QUEUE; k++)
	{
		if(act->que[k]==enter_combat) printf("IC");
		if(act->que[k]==leave_combat) printf("LC");
		if(act->que[k]>=move_n&&act->que[k]<=move_nw) printf("M");
		if(act->que[k]>=turn_n&&act->que[k]<=turn_nw) printf("R");
		printf("%2i|",act->que[k]);
	}
	printf("\n");
	/*for(k=0; k<MAX_RANGE_ACTION_QUEUE; k++){
			printf("%2i-%2i|",act->range_actions[k].shot_type,act->range_actions[k].state);
	}
	printf("\n");
	*/

	if (attached)
	{
		printf("   Horse %s queue:",act->actor_name);
		printf(" -->");
		for(k=0; k<MAX_CMD_QUEUE; k++)
		{
			if(attached->que[k]==enter_combat) printf("IC");
			if(attached->que[k]==leave_combat) printf("LC");
			if(attached->que[k] >= move_n && attached->que[k] <= move_nw) printf("M");
			if(attached->que[k] >= turn_n && attached->que[k] <= turn_nw) printf("R");
			printf("%2i|",attached->que[k]);
		}
		printf("\n");
	}

}

static void attached_info(actor *act, actor *attached, int c)
{
	if (act->actor_id==yourself && act->que[0] != nothing)
	{
		printf("%i---------> DOING: %i -----------\n",c,act->que[0]);
		print_queue(act, attached);
	}
	else if (act->actor_id < 0 && attached && attached->actor_id == yourself
		&& act->que[0] != wait_cmd && act->que[0]!=nothing)
	{
		printf("%i---------> DOING (horse): %i ---\n",c,act->que[0]);
		print_queue(attached, act);
	}
}
#endif // MORE_ATTACHED_ACTORS_DEBUG

void flush_delayed_item_changes(actor *a)
{
	int item;
	for (item = 0; item < a->delayed_item_changes_count; ++item) {
		if (a->delayed_item_changes[item] < 0) {
			missiles_log_message("%s (%d): unwearing item type %d now\n",
								 a->actor_name,
								 a->actor_id,
								 a->delayed_item_type_changes[item]);
			unwear_item_from_actor(a->actor_id,
								   a->delayed_item_type_changes[item]);
		}
		else {
			missiles_log_message("%s (%d): wearing item type %d now\n",
								 a->actor_name,
								 a->actor_id,
								 a->delayed_item_type_changes[item]);
			actor_wear_item(a->actor_id,
							a->delayed_item_type_changes[item],
							a->delayed_item_changes[item]);
		}
	}
	a->delayed_item_changes_count = 0;
}

static void move_actor_to_next_frame(actor* act, actor *attached, void* UNUSED(data),
	locked_list_ptr UNUSED(list))
{
	actor *horse = (attached && is_horse(attached)) ? attached : NULL;

	if (act->calmodel)
	{
		if (act->stop_animation == 1 && act->anim_time >= act->cur_anim.duration)
		{
			//if(act->actor_id==yourself) printf("%i, unbusy: anim %i, anim_time %f, duration %f\n",thecount,act->cur_anim.anim_index,act->anim_time,act->cur_anim.duration);
			//if(act->actor_id<0&&MY_HORSE(i)->actor_id==yourself) printf("%i, (horse) unbusy: anim %i, anim_time %f, duration %f\n",thecount,act->cur_anim.anim_index,act->anim_time,act->cur_anim.duration);

			if (horse)
			{
				//rotations during idle animation like when server sends turn_n..turn_nw
				//need to be synchronized on the minimum remaining animation time between
				//the idle of the horse and the actor.
				if (
					/*(horse->anim_time < horse->cur_anim.duration) &&*/
					horse->cur_anim.kind==cycle)
				{
					//horse->anim_time = horse->cur_anim.duration;
					horse->busy = 0;
					horse->in_aim_mode = 0;
					set_on_idle(horse, act);
					//horse->stop_animation=0;
					//if(act->actor_id==yourself) printf("%i, %s stops Horse\n",thecount, act->actor_name);
				}
			}
			else if (is_horse(act))
			{
				if (attached && attached->anim_time < attached->cur_anim.duration)
				{
					//wait for actor
					//if(attached->actor_id==yourself) printf("%i, Horse waits for %s\n",thecount, attached->actor_name);
					return;
				}
			}

			act->busy = 0;
			if (act->in_aim_mode == 2)
			{
				// we really leave the aim mode only when the animation is finished
				act->in_aim_mode = 0;
				missiles_log_message("%s (%d): leaving range mode finished!\n",
					act->actor_name, act->actor_id);

			}
		}
	}

	if (act->in_aim_mode == 0)
	{
		if (act->is_enhanced_model != 0)
		{
			if (get_actor_texture_ready(act->texture_id))
			{
				use_ready_actor_texture(act->texture_id);
			}
		}

		if (act->delayed_item_changes_count > 0)
		{
			// we really leave the aim mode only when the animation is finished
			act->delay_texture_item_changes = 0;

			// then we do all the item changes that have been delayed
			flush_delayed_item_changes(act);

			act->delay_texture_item_changes = 1;
		}
	}

	// we change the idle animation only when the previous one is finished
	if (act->stand_idle && act->anim_time >= act->cur_anim.duration - 0.2)
	{
		if (!is_actor_held(act, attached))
		{
			set_on_idle(act, attached);
		}
	}

	if (act->cur_anim.anim_index==-1) {
		act->busy=0;
		//if(act->actor_id==yourself) printf("%i, unbusy(-1)\n", thecount);
		//if(act->actor_id<0) printf("%i, unbusy horse(-1)\n", thecount);
	}

	//first thing, decrease the damage time, so we will see the damage splash only for 2 seconds
	if(act->damage_ms) {
		act->damage_ms-=80;
		if(act->damage_ms<0)act->damage_ms=0;
	}

	//9 frames, not moving, and another command is queued farther on (based on how long we've done this action)
	if(!act->moving && !act->rotating){

		/*	act->stop_animation=1;	//force stopping, not looping
			act->busy=0;	//ok, take the next command
			LOG_TO_CONSOLE(c_green2,"FREE");
			//Idle here?
		*/
	}

	if(act->stop_animation) {

		//we are done with this guy
		//Should we go into idle here?
	}

	if (horse
		&& !act->busy
		&& act->actor_id >= 0
		&& ((act->last_command >= enter_aim_mode && act->last_command <= aim_mode_fire)
			|| (act->last_command >= enter_combat && act->last_command <= leave_combat)))
	{
		unfreeze_horse(act, horse);
	}
	if (horse && act->in_aim_mode == 3) // when no_action==1
	{
		act->in_aim_mode=0;
		unfreeze_horse(act, horse);
		//printf("%i,Unfreeze after no_action==1\n",thecount);
	}
}

void move_to_next_frame()
{
	locked_list_ptr actors_list = get_locked_actors_list();
	for_each_actor_and_attached(actors_list, move_actor_to_next_frame, NULL);
	release_locked_actors_list(actors_list);
}

struct cal_anim *get_pose(actor *a, int pose_id, int pose_type, int held) {
	hash_entry *he,*eh;
	emote_data *pose;

	eh=hash_get(emotes,(void *)(uintptr_t)pose_id);
	pose = eh->item;

	he=hash_get(actors_defs[a->actor_type].emote_frames, (void *)(uintptr_t)(pose->anims[pose_type][0][held]->ids[0]));
	if (he) return (struct cal_anim*) he->item;
	else return NULL;
}

static struct cal_anim *get_pose_frame(int actor_type, actor *act, actor *attached,
	int pose_type, int held)
{
	hash_entry *he;
	int a_type=emote_actor_type(actor_type);

	//find the pose. Pose is the first anim of the first frame
	if (act->poses[pose_type]) {
		//printf("getting pose for %s\n",a->actor_name);
		he=hash_get(actors_defs[actor_type].emote_frames, (void *)(uintptr_t)(act->poses[pose_type]->anims[a_type][0][held]->ids[0]));
		if (he) return (struct cal_anim*) he->item;
	}
	//no pose or no emote..set defaults
	if (!act->poses[pose_type]) {
		//printf("no pose for %s\n",a->actor_name);
		switch(pose_type){
			case EMOTE_SITTING:
				return &actors_defs[act->actor_type].cal_frames[cal_actor_idle_sit_frame];
			case EMOTE_STANDING:
				if(held)
				{
					attachment_props *att_props = get_attachment_props_if_held(act, attached);
					if (att_props)
						return &att_props->cal_frames[cal_attached_idle_frame];
				}
				else
				{
					// 75% chance to do idle1
					if (actors_defs[act->actor_type].cal_frames[cal_actor_idle2_frame].anim_index != -1
						&& RAND(0, 3) == 0)
					{
						return &actors_defs[act->actor_type].cal_frames[cal_actor_idle2_frame]; //idle2
					} else {
						return &actors_defs[act->actor_type].cal_frames[cal_actor_idle1_frame]; //idle1
					}
				}
				break;
			case EMOTE_RUNNING:
				if(held)
				{
					attachment_props *att_props = get_attachment_props_if_held(act, attached);
					if (att_props)
						return &att_props->cal_frames[cal_attached_run_frame /*get_held_actor_motion_frame(act, attached)*/];
				} else
					return &actors_defs[actor_type].cal_frames[cal_actor_run_frame/*get_actor_motion_frame(a)*/];
				break;
			case EMOTE_WALKING:
				if(held)
				{
					attachment_props *att_props = get_attachment_props_if_held(act, attached);
					if (att_props)
						return &att_props->cal_frames[cal_attached_walk_frame/*get_held_actor_motion_frame(a)*/];
				} else
					return &actors_defs[actor_type].cal_frames[cal_actor_walk_frame/*get_actor_motion_frame(a)*/];
				break;
			default:
				return NULL;
			break;
		}
	}
	return NULL;
}

void set_on_idle(actor *act, actor *attached)
{
	if (act->dead)
		return;

	act->stop_animation = 0;
	//we have an emote idle, ignore the rest
	if (act->cur_emote.idle.anim_index >= 0)
		return;

	if (act->fighting)
	{
		if (attached)
		{
			// Both for horses and actors
			if(is_horse(act))
				cal_actor_set_anim(act, attached,
					actors_defs[act->actor_type].cal_frames[cal_actor_combat_idle_frame]);
			else if (actor_weapon(act)->turn_horse && act->horse_rotated)
				cal_actor_set_anim(act, attached,
					actors_defs[act->actor_type].cal_frames[cal_actor_combat_idle_held_frame]);
			else
				cal_actor_set_anim(act, attached,
					actors_defs[act->actor_type].cal_frames[cal_actor_combat_idle_held_unarmed_frame]);
		}
		else
		{
			cal_actor_set_anim(act, attached,
				actors_defs[act->actor_type].cal_frames[cal_actor_combat_idle_frame]);
		}
	}
	else if (act->in_aim_mode == 1)
	{
		if (is_horse(act))
		{
			// Ranging horse
			if (act->cur_anim.anim_index != actors_defs[act->actor_type].cal_frames[cal_actor_idle1_frame].anim_index
				&& act->cur_anim.anim_index != actors_defs[act->actor_type].cal_frames[cal_actor_idle2_frame].anim_index)
			{
				//printf("%i, horse on idle from %i\n",thecount, actor->cur_anim.anim_index);
				cal_actor_set_anim(act, attached,
					*get_pose_frame(act->actor_type, act, attached, EMOTE_STANDING, 0));
			}
		}
		else if (attached)
		{
			cal_actor_set_anim(act, attached,
				actors_defs[act->actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_idle_held_frame]);
		}
		else
		{
			cal_actor_set_anim(act, attached,
				actors_defs[act->actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_idle_frame]);
		}
	}
	else if (!act->sitting)
	{
		// We are standing, see if we can activate a stand idle
		if (!act->stand_idle || act->cur_anim.anim_index < 0)
		{
			if (actors_defs[act->actor_type].group_count == 0)
			{
				//if(actor->actor_id<0) printf("%i, horse on standing idle from %i\n",thecount, actor->cur_anim.anim_index);
				attachment_props *att_props = get_attachment_props_if_held(act, attached);
				struct cal_anim *ca = get_pose_frame(act->actor_type, act, attached,
					EMOTE_STANDING, att_props != NULL);
				cal_actor_set_anim(act, attached, *ca);
				//printf("setting standing pose\n");
			}
			else
			{
				cal_actor_set_random_idle(act);
				act->IsOnIdle = 1;
			}

			act->stand_idle = 1;
		}
	}
	else
	{
		// We are sitting, see if we can activate the sit idle
		if (!act->sit_idle || act->cur_anim.anim_index < 0)
		{
			cal_actor_set_anim(act, attached,
				*get_pose_frame(act->actor_type, act, attached, EMOTE_SITTING, 0));
			//printf("setting sitting pose\n");
			act->sit_idle = 1;
		}
	}
}









void unqueue_emote(actor *act){

	int k,max_queue = 0;
	// Move queue down one command
	for (k = 0; k < MAX_EMOTE_QUEUE - 1; k++) {
		if (k > max_queue && act->emote_que[k].origin != NO_EMOTE)
			max_queue = k;
		act->emote_que[k] = act->emote_que[k+1];
	}
	act->emote_que[k].origin = NO_EMOTE;
}

static int handle_emote_command(actor *act, actor *horse, emote_command *command)
{
	struct cal_anim *pose[4];

	//check if emote is null. If so, reset emote anims
	if(!command->emote){
		//printf("reset emotes of actor %i\n",act->actor_id);
		cal_reset_emote_anims(act,1);
		unqueue_emote(act);
		return 1;
	}

	//check if emote is timed out
	//printf("Handle emote %i created at %i for actor %i\n",command->emote->id,command->create_time,act->actor_id);
	if(command->create_time+command->emote->timeout<cur_time){
		//timed out
		//printf("Emote %i timed out\n",command->emote->id);
		unqueue_emote(act);
		return 1;
	} else if(!act->cur_emote.active){
		//there is still time to do it, and no emote going on:
		//check current frame, actor_type and emote flags
		int idle=0,actor_type,held;
		//struct cal_anim *defs;
		emote_frame *frames;

		//barehanded?
		if((command->emote->barehanded&EMOTE_BARE_R)&&!is_actor_barehanded(act,EMOTE_BARE_R)){
			//printf("Remove weapon to play, waiting...\n");
			return 0;
		}
		if((command->emote->barehanded&EMOTE_BARE_L)&&!is_actor_barehanded(act,EMOTE_BARE_L)){
			//printf("Remove shield to play, waiting...\n");
			return 0;
		}


		held = is_actor_held(act, horse) ? 1 : 0;
		//printf("actor %i is held: %i\n",act_id,held);
		pose[EMOTE_STANDING] = get_pose_frame(act->actor_type, act, horse, EMOTE_STANDING, held);
		pose[EMOTE_WALKING] = get_pose_frame(act->actor_type, act, horse, EMOTE_WALKING, held);
		pose[EMOTE_SITTING] = get_pose_frame(act->actor_type, act, horse, EMOTE_SITTING, held);
		pose[EMOTE_RUNNING] = get_pose_frame(act->actor_type, act, horse, EMOTE_RUNNING, held);

#ifdef MORE_EMOTES

		if(command->emote->pose<=EMOTE_STANDING) {
			//we have a pose
			hash_entry *he;
			he=hash_get(actors_defs[actor_type].emote_frames, (void *)(uintptr_t)(command->emote->anims[act->actor_type][0][held]->ids[0]));
			
			start_transition(act,((struct cal_anim*) he->item)->anim_index,300);
			act->poses[command->emote->pose]=command->emote;
			unqueue_emote(act);
			return 0;
		}
#endif
		/*printf("STANDING --> a: %i, c: %i\n",pose[EMOTE_STANDING]->anim_index, act->cur_anim.anim_index);
		printf("WALKING --> a: %i, c: %i\n",pose[EMOTE_WALKING]->anim_index, act->cur_anim.anim_index);
		printf("SITTING --> a: %i, c: %i\n",pose[EMOTE_SITTING]->anim_index, act->cur_anim.anim_index);
		printf("RUNNING --> a: %i, c: %i\n",pose[EMOTE_RUNNING]->anim_index, act->cur_anim.anim_index);
		*/
		if(pose[EMOTE_STANDING]&&pose[EMOTE_STANDING]->anim_index==act->cur_anim.anim_index) idle=EMOTE_STANDING;
		else
		if(pose[EMOTE_WALKING]&&pose[EMOTE_WALKING]->anim_index==act->cur_anim.anim_index) idle=EMOTE_WALKING;
		else
		if(pose[EMOTE_RUNNING]&&pose[EMOTE_RUNNING]->anim_index==act->cur_anim.anim_index) idle=EMOTE_RUNNING;
		else
		if(pose[EMOTE_SITTING]&&pose[EMOTE_SITTING]->anim_index==act->cur_anim.anim_index) idle=EMOTE_SITTING;
		else if(act->cur_anim.anim_index<0) {
			//we have an emote idle. Remove it and try again
			cal_reset_emote_anims(act,1);
			set_on_idle(act, horse);
			return 0;
		} else {
			//printf("No suitable state for !held\n");
			unqueue_emote(act);
			return 1;
		}
		actor_type=emote_actor_type(act->actor_type);
		frames=command->emote->anims[actor_type][idle][held];
		
		//we have a emote and not already playing one
		//printf("we have anim...actor: %i, held: %i, frames: %p, idle: %i\n",act->actor_id, held,frames,idle);
		if (!frames) {
			//not ready yet, try later
			//printf("but not ready yet\n");
			return 0;
		} else {
			//ready! set emote and unqueue
			//printf("ready!!\n");
			cal_actor_set_emote_anim(act, frames);
			if (horse)
			{
				horse->cur_emote.active=1; //synch with horse!!
				//cal_actor_set_emote_anim(horse, frames);
			}
			//printf("unqueue\n");
			unqueue_emote(act);
			//LOG_TO_CONSOLE(c_green2, "Emote command");
			return 0;
		}
	}

	return 0;
}

static void rotate_actor_and_horse_by(actor *act, actor *horse, int mul, float angle)
{
	if (!act->rotating)
	{
		act->rotate_z_speed = (float)mul*angle/(float)HORSE_FIGHT_TIME;
		act->rotate_time_left = HORSE_FIGHT_TIME;
		act->rotating = 1;
		act->stop_animation = 1;
		act->horse_rotated = (mul<0) ? (1) : (0); //<0 enter fight, >=0 leave fight
		horse->rotate_z_speed = act->rotate_z_speed;
		horse->rotate_time_left = act->rotate_time_left;
		horse->rotating = 1;
		horse->stop_animation = 1;
	}
}

void rotate_actor_and_horse_locked(actor *act, actor *horse, int mul)
{
	rotate_actor_and_horse_by(act, horse, mul, HORSE_FIGHT_ROTATION);
}

static inline void rotate_actor_and_horse_range(actor *act, actor *horse, int mul)
{
	rotate_actor_and_horse_by(act, horse, mul, HORSE_RANGE_ROTATION);
}

//in case the actor is not busy, and has commands in it's que, execute them
static void next_actor_command(actor* act, actor *attached, void* UNUSED(data),
	locked_list_ptr list)
{
	actor *horse;

	if (act->que[0] >= emote_cmd && act->que[0] < wait_cmd)
	{
		int k, max_queue = 0;
		add_emote_to_actor(act->actor_id, act->que[0]);
		//act->stop_animation=1;
		//move que down with one command
		for(k=0;k<MAX_CMD_QUEUE-1;k++) {
			if(k>max_queue && act->que[k]!=nothing)max_queue=k;
			act->que[k]=act->que[k+1];
		}
		act->que[max_queue]=nothing;
	}

	if (act->busy) // Are we busy?
		return;

	horse = (attached && is_horse(attached)) ? attached : NULL;

	// Are we playing an emote?
	if (act->cur_emote.active
		&& !(act->que[0] >= move_n && act->que[0] <= move_nw
			&& act->last_command >= move_n && act->last_command <= move_nw)
		&& !horse)
	{
		return;
	}

	// If the que is empty, check for an emote to display
	while (act->emote_que[0].origin != NO_EMOTE)
	{
		if (!handle_emote_command(act, horse, &act->emote_que[0]))
			break;
	}

	if (act->que[0]==nothing) // Is the queue empty?
	{
		// If que is empty, set on idle
		set_on_idle(act, attached);
		act->last_command=nothing; // Prevents us from not updating the walk/run animation
	}
	else
	{
		int actor_type;
		int last_command=act->last_command;
		float z_rot=act->z_rot;
		float targeted_z_rot;
		int no_action = 0;

		act->sit_idle=0;
		act->stand_idle=0;

#ifndef DISABLE_RANGE_MODE_EXIT_BUGFIX
		if (act->is_enhanced_model && act->in_aim_mode == 1
			&& (act->que[0] < enter_aim_mode || act->que[0] > missile_critical)
			&& (act->que[0] < turn_n || act->que[0] > turn_nw))
		{
			LOG_ERROR("%s: %d: command incompatible with range mode detected: %d", __FUNCTION__, __LINE__, act->que[0]);
			missiles_log_message("%s (%d): forcing aim mode exit", act->actor_name,
				act->actor_id);
			act->cal_h_rot_start = 0.0;
			act->cal_v_rot_start = 0.0;
			act->cal_h_rot_end = 0.0;
			act->cal_v_rot_end = 0.0;
			act->cal_rotation_blend = 1.0;
			act->cal_rotation_speed = 0.0;
			act->cal_last_rotation_time = cur_time;
			act->are_bones_rotating = 1;
			act->in_aim_mode = 0;
			flush_delayed_item_changes(act);
		}
#endif // DISABLE_RANGE_MODE_EXIT_BUGFIX

		actor_type=act->actor_type;
#ifdef MORE_ATTACHED_ACTORS_DEBUG
		//just for debugging
		attached_info(act, attached, thecount);
#endif
		switch (act->que[0])
		{
			case kill_me:
				// Obsolete
				break;
			case die1:
				cal_actor_set_anim(act, attached,
					actors_defs[actor_type].cal_frames[cal_actor_die1_frame]);
				act->stop_animation=1;
				act->dead=1;
				break;
			case die2:
				cal_actor_set_anim(act, attached,
					actors_defs[actor_type].cal_frames[cal_actor_die2_frame]);
				act->stop_animation=1;
				act->dead=1;
				break;
			case pain1:
			case pain2:
			{
				int painframe = (act->que[0]==pain1) ? cal_actor_pain1_frame : cal_actor_pain2_frame;
				attachment_props *att_props = get_attachment_props_if_held(act, attached);
				if (att_props)
				{
					if (horse && !actor_weapon(act)->unarmed)
					{
						cal_actor_set_anim(act, attached,
							att_props->cal_frames[cal_attached_pain_armed_frame]);
					}
					else
					{
						cal_actor_set_anim(act, attached,
							att_props->cal_frames[cal_attached_pain_frame]);
					}
				}
				else
				{
					cal_actor_set_anim(act, attached,
						actors_defs[actor_type].cal_frames[painframe]);
				}
				act->stop_animation=1;
				break;
			}
			case pick:
				cal_actor_set_anim(act, attached,
					actors_defs[actor_type].cal_frames[cal_actor_pick_frame]);
				act->stop_animation=1;
				break;
			case drop:
				cal_actor_set_anim(act, attached,
					actors_defs[actor_type].cal_frames[cal_actor_drop_frame]);
				act->stop_animation=1;
				break;
			case harvest:
				cal_actor_set_anim(act, attached,
					actors_defs[actor_type].cal_frames[cal_actor_harvest_frame]);
				act->stop_animation=1;
				LOG_TO_CONSOLE(c_green2,"Harvesting!");
				break;
			case cast:
				cal_actor_set_anim(act, attached,
					actors_defs[actor_type].cal_frames[cal_actor_attack_cast_frame]);
				act->stop_animation=1;
				break;
			case ranged:
				cal_actor_set_anim(act, attached,
					actors_defs[actor_type].cal_frames[cal_actor_attack_ranged_frame]);
				act->stop_animation=1;
				break;
			case sit_down:
				cal_actor_set_anim(act, attached,
					actors_defs[actor_type].cal_frames[cal_actor_sit_down_frame]);
				act->stop_animation=1;
				act->sitting=1;
				if(act->actor_id==yourself)
					you_sit=1;
				break;
			case stand_up:
				//LOG_TO_CONSOLE(c_green2,"stand_up");
				cal_actor_set_anim(act, attached,
					actors_defs[actor_type].cal_frames[cal_actor_stand_up_frame]);
				act->stop_animation=1;
				act->sitting=0;
				if(act->actor_id==yourself)
					you_sit=0;
				break;
			case enter_combat:
			case leave_combat:
			{
				int fight_k = (act->que[0] == enter_combat) ? 1 : 0;
				int combat_frame = fight_k ? cal_actor_in_combat_frame : cal_actor_out_combat_frame;
				int combat_held_frame = fight_k ? cal_actor_in_combat_held_frame : cal_actor_out_combat_held_frame;
				int combat_held_unarmed_frame = fight_k ?  cal_actor_in_combat_held_unarmed_frame : cal_actor_out_combat_held_unarmed_frame;
				int mul_angle = fight_k ? -1 : 1;

				if (horse)
				{
					// Rotate horse and actor if needed
					if (actor_weapon(act)->turn_horse)
					{
						if (fight_k || act->horse_rotated)
							rotate_actor_and_horse_locked(act, horse, mul_angle);
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].cal_frames[combat_held_frame]);
					}
					else
					{
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].cal_frames[combat_held_unarmed_frame]);
					}

					// horse enter combat
					horse->fighting=fight_k;
					horse->stop_animation=1;
					cal_actor_set_anim(horse, attached,
						actors_defs[horse->actor_type].cal_frames[combat_frame]);
				}
				else
				{
					cal_actor_set_anim(act, attached,
						actors_defs[actor_type].cal_frames[combat_frame]);
				}

				act->stop_animation=1;
				act->fighting=fight_k;
				break;
			}
			case attack_up_1:
			case attack_up_2:
			case attack_up_3:
			case attack_up_4:
			case attack_up_5:
			case attack_up_6:
			case attack_up_7:
			case attack_up_8:
			case attack_up_9:
			case attack_up_10:
			case attack_down_1:
			case attack_down_2:
			case attack_down_3:
			case attack_down_4:
			case attack_down_5:
			case attack_down_6:
			case attack_down_7:
			case attack_down_8:
			case attack_down_9:
			case attack_down_10:
			{
				int index = -1;
				switch (act->que[0])
				{
					case attack_down_10:
						index++;
					// fall-through - suppress the compile warning with this comment
					case attack_down_9:
						index++;
					// fall-through
					case attack_down_8:
						index++;
					// fall-through
					case attack_down_7:
						index++;
					// fall-through
					case attack_down_6:
						index++;
					// fall-through
					case attack_down_5:
						index++;
					// fall-through
					case attack_down_4:
						index++;
					// fall-through
					case attack_down_3:
						index++;
					// fall-through
					case attack_down_2:
						index++;
					// fall-through
					case attack_down_1:
						index++;
					// fall-through
					case attack_up_10:
						index++;
					// fall-through
					case attack_up_9:
						index++;
					// fall-through
					case attack_up_8:
						index++;
					// fall-through
					case attack_up_7:
						index++;
					// fall-through
					case attack_up_6:
						index++;
					// fall-through
					case attack_up_5:
						index++;
					// fall-through
					case attack_up_4:
						index++;
					// fall-through
					case attack_up_3:
						index++;
					// fall-through
					case attack_up_2:
						index++;
					// fall-through
					case attack_up_1:
						index++;
						break;
					default:
						break;
				}
				if (act->is_enhanced_model)
				{
					if (horse)
					{
						// Actor does combat anim
						index+=cal_weapon_attack_up_1_held_frame; //30; //select held weapon animations
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[index]);
					}
					else
					{
						// Normal weapon animation
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[index]);
					}
				}
				else
				{
					// Non enhanced models
					if (horse)
					{
						// Non enhanced model with a horse
						index+=cal_actor_attack_up_1_held_frame;
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].cal_frames[index]);
					}
					else
					{
						// Select normal actor att frames
						index +=cal_actor_attack_up_1_frame;
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].cal_frames[index]);
					}
				}
				act->stop_animation=1;
				act->fighting=1;

#ifdef NEW_SOUND
				// Maybe play a battlecry sound
				add_battlecry_sound(act);
#endif // NEW_SOUND
				// Check if a horse rotation is needed
				if (horse && !act->horse_rotated && actor_weapon(act)->turn_horse)
				{
					// Horse, not rotated, to be rotated -> do rotation
					rotate_actor_and_horse_locked(act, horse, -1);
				}
				else if (horse && act->horse_rotated && !actor_weapon(act)->turn_horse)
				{
					// Horse, rotated, not to be rotated -> undo rotation
					act->horse_rotated=0;
					rotate_actor_and_horse_locked(act, horse, 1);
				}
				break;
			}
			case turn_left:
			case turn_right:
			{
				int mul= (act->que[0] == turn_left) ? 1 : -1;
				int turnframe = (act->que[0] == turn_left) ? cal_actor_turn_left_frame : cal_actor_turn_right_frame;

				//LOG_TO_CONSOLE(c_green2,"turn left");
				act->rotate_z_speed=mul*45.0/540.0;
				act->rotate_time_left=540;
				act->rotating=1;
				//generate a fake movement, so we will know when to make the actor
				//not busy
				act->move_x_speed=0;
				act->move_y_speed=0;
				act->move_z_speed=0;
				act->movement_time_left=540;
				act->moving=1;
				//test
				if (!act->fighting)
				{
					attachment_props *att_props = get_attachment_props_if_held(act, attached);
					if (att_props)
					{
						cal_actor_set_anim(act, attached,
							*get_pose_frame(act->actor_type, act, attached, EMOTE_MOTION(act), 1));
					}
					else
					{
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].cal_frames[turnframe]);
					}
				}
				act->stop_animation=0;
				break;
			}
			case enter_aim_mode:
				missiles_log_message("%s (%d): cleaning the queue from enter_aim_mode command",
					act->actor_name, act->actor_id);
				missiles_clean_range_actions_queue(act);

				if (act->in_aim_mode == 0)
				{
					missiles_log_message("%s (%d): enter in aim mode", act->actor_name,
						act->actor_id);
					//if(act->actor_id==yourself) printf("%i, enter aim 0\n",thecount);
					if (horse)
					{
						// Set the horse aim mode
						if (!act->horse_rotated)
						{
							rotate_actor_and_horse_range(act, horse, -1);
							act->horse_rotated=1;
						}
						horse->in_aim_mode=1;
						// We could start a horse_ranged_in
						set_on_idle(horse, act);
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_in_held_frame]);
					}
					else
					{
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_in_frame]);
					}

					act->cal_h_rot_start = 0.0;
					act->cal_v_rot_start = 0.0;
					if (act->range_actions_count != 1)
					{
						LOG_ERROR("%s (%d): entering in range mode with an non empty range action queue!",
							act->actor_name, act->actor_id);
					}
				}
				else
				{
					float range_rotation;
					range_action *action = &act->range_actions[0];
					//if(act->actor_id==yourself) printf("%i, enter aim %i\n",thecount,act->in_aim_mode);
					missiles_log_message("%s (%d): aiming again (time=%d)", act->actor_name,
						act->actor_id, cur_time);
					if (horse)
					{
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_idle_held_frame]);
						//if (!act->horse_rotated) {rotate_actor_and_horse(act, horse, -1); act->horse_rotated=1;}
					}
					else
					{
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_idle_frame]);
					}
					act->cal_h_rot_start = act->cal_h_rot_start * (1.0 - act->cal_rotation_blend)
						+ act->cal_h_rot_end * act->cal_rotation_blend;
					act->cal_v_rot_start = act->cal_v_rot_start * (1.0 - act->cal_rotation_blend)
						+ act->cal_v_rot_end * act->cal_rotation_blend;

					/* we look if the actor is still around and if yes,
						* we recompute it's position */
					if (action->aim_actor >= 0)
					{
						actor *aim_actor = get_actor_from_id(list, action->aim_actor);
						if (aim_actor) {
							cal_get_actor_bone_absolute_position(aim_actor, get_actor_bone_id(aim_actor, body_top_bone), NULL, action->aim_position);
						}
					}

					if (horse)
						act->z_rot+=HORSE_RANGE_ROTATION;
					range_rotation = missiles_compute_actor_rotation(&act->cal_h_rot_end,
						&act->cal_v_rot_end, act, action->aim_position);
					if (horse)
						act->z_rot-=HORSE_RANGE_ROTATION;
					act->cal_rotation_blend = 0.0;
					act->cal_rotation_speed = 1.0/360.0;
					act->cal_last_rotation_time = cur_time;
					act->are_bones_rotating = 1;
					act->stop_animation = 1;
					if (action->state == 0) action->state = 1;

					if (range_rotation != 0.0)
					{
						missiles_log_message("%s (%d): not facing its target => client side rotation needed",
							act->actor_name, act->actor_id);
						if (act->rotating)
						{
							range_rotation += act->rotate_z_speed * act->rotate_time_left;
						}
						act->rotate_z_speed = range_rotation/360.0;
						act->rotate_time_left=360;
						act->rotating=1;
						if (horse)
						{
							//printf("rotating the horse client side!\n");
							horse->rotate_z_speed=range_rotation/360.0;
							horse->rotate_time_left=360;
							horse->rotating=1;
						}
					}
				}
				break;
			case leave_aim_mode:
				if (act->in_aim_mode != 1)
				{
					if (act->cal_rotation_blend < 0.0 ||
						(act->cal_h_rot_end == 0.0 && act->cal_v_rot_end == 0.0))
					{
						LOG_ERROR("next_command: trying to leave range mode while we are not in it => aborting safely...");
						no_action = 1;
						if (act->horse_rotated)
						{
							rotate_actor_and_horse_range(act, horse, 1);
							act->horse_rotated=0;
						}
						break;
					}
					else
					{
						LOG_ERROR("next_command: trying to leave range mode while we are not in it => continuing because of a wrong actor bones rotation!");
					}
				}

				missiles_log_message("%s (%d): leaving aim mode", act->actor_name,
					act->actor_id);
				if (horse)
				{
					cal_actor_set_anim(act, attached,
						actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_out_held_frame]);
					if (act->horse_rotated)
					{
						rotate_actor_and_horse_range(act, horse, 1);
						act->horse_rotated=0;
					}
				}
				else
				{
					cal_actor_set_anim(act, attached,
						actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_out_frame]);
				}
				act->cal_h_rot_start = act->cal_h_rot_start * (1.0 - act->cal_rotation_blend)
					+ act->cal_h_rot_end * act->cal_rotation_blend;
				act->cal_v_rot_start = act->cal_v_rot_start * (1.0 - act->cal_rotation_blend)
					+ act->cal_v_rot_end * act->cal_rotation_blend;
				act->cal_h_rot_end = 0.0;
				act->cal_v_rot_end = 0.0;
				act->cal_rotation_blend = 0.0;
				act->cal_rotation_speed = 1.0/360.0;
				act->cal_last_rotation_time = cur_time;
				act->are_bones_rotating = 1;
				act->in_aim_mode = 2;
				act->stop_animation = 1;
				break;

// 				case aim_mode_reload
// 					missiles_log_message("%s (%d): reload after next fire", act->actor_name,
// 						act->actor_id);
// 					act->reload = 1;
// 					no_action = 1;
// 					break;

			case aim_mode_fire:
			{
				range_action *action = &act->range_actions[0];
				action->state = 3;

				if (act->in_aim_mode != 1)
				{
					LOG_ERROR("next_command: trying to fire an arrow out of range mode => aborting!");
					no_action = 1;
					missiles_log_message("%s (%d): cleaning the queue from aim_mode_fire command (error)",
						act->actor_name, act->actor_id);
					missiles_clean_range_actions_queue(act);
					break;
				}

				if (action->reload)
				{
					missiles_log_message("%s (%d): fire and reload", act->actor_name,
						act->actor_id);
					// launch fire and reload animation

					//if(act->actor_id==yourself) printf("%i, enter reload\n",thecount);
					if (horse)
					{
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_fire_held_frame]);
					}
					else
					{
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_fire_frame]);
					}
					act->in_aim_mode = 1;
				}
				else
				{
					missiles_log_message("%s (%d): fire and leave aim mode", act->actor_name,
						act->actor_id);
					// launch fire and leave aim mode animation

					//if (act->actor_id==yourself) printf("%i, enter fire & leave\n",thecount);
					if (horse)
					{
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_fire_out_held_frame]);
					}
					else
					{
						cal_actor_set_anim(act, attached,
							actors_defs[actor_type].weapon[act->cur_weapon].cal_frames[cal_weapon_range_fire_out_frame]);
					}
					act->in_aim_mode = 2;
				}

				act->cal_h_rot_start = act->cal_h_rot_start * (1.0 - act->cal_rotation_blend)
					+ act->cal_h_rot_end * act->cal_rotation_blend;
				act->cal_v_rot_start = act->cal_v_rot_start * (1.0 - act->cal_rotation_blend)
					+ act->cal_v_rot_end * act->cal_rotation_blend;
				act->cal_h_rot_end = 0.0;
				act->cal_v_rot_end = 0.0;
				act->cal_rotation_blend = 0.0;
				act->cal_rotation_speed = 1.0/360.0;
				act->cal_last_rotation_time = cur_time;
				act->are_bones_rotating = 1;
				act->stop_animation = 1;

				/* In case of a missed shot due to a collision with an actor,
					* the server send the position of the actor with 0.0 for the Z coordinate.
					* So we have to compute the coordinate of the ground at this position.
					*/
				if (action->shot_type == MISSED_SHOT &&
					action->fire_position[2] == 0.0)
				{
					int tile_x = (int)(action->fire_position[0]*2.0);
					int tile_y = (int)(action->fire_position[1]*2.0);
					action->fire_position[2] = get_tile_height(tile_x, tile_y);
					missiles_log_message("missed shot detected: new height computed: %f", action->fire_position[2]);
				}
				else if (action->fire_actor >= 0)
				{
					actor *fire_actor = get_actor_from_id(list, action->fire_actor);
					if (fire_actor)
					{
						cal_get_actor_bone_absolute_position(fire_actor, get_actor_bone_id(fire_actor, body_top_bone), NULL, action->fire_position);
					}
				}

#if 0 //def DEBUG
				{
					float aim_angle = atan2f(action->aim_position[1] - act->y_pos,
												action->aim_position[0] - act->x_pos);
					float fire_angle = atan2f(action->fire_position[1] - act->y_pos,
												action->fire_position[0] - act->x_pos);
					if (aim_angle < 0.0) aim_angle += 2*M_PI;
					if (fire_angle < 0.0) fire_angle += 2*M_PI;
					if (fabs(fire_angle - aim_angle) > M_PI/8.0) {
						char msg[512];
						sprintf(msg,
								"%s (%d): WARNING! Target position is too different from aim position: pos=(%f,%f,%f) aim=(%f,%f,%f) target=(%f,%f,%f) aim_angle=%f target_angle=%f",
								act->actor_name,
								act->actor_id,
								act->x_pos,
								act->y_pos,
								act->z_pos,
								action->aim_position[0],
								action->aim_position[1],
								action->aim_position[2],
								action->fire_position[0],
								action->fire_position[1],
								action->fire_position[2],
								aim_angle, fire_angle);
						LOG_TO_CONSOLE(c_red2, msg);
						missiles_log_message(msg);
					}
				}
#endif // DEBUG

				missiles_fire_arrow(act, action->fire_position, action->shot_type);
				missiles_log_message("%s (%d): cleaning the queue from aim_mode_fire command (end)",
					act->actor_name, act->actor_id);
				missiles_clean_range_actions_queue(act);

				break;
			}
// 				case missile_miss:
// 					missiles_log_message("%s (%d): will miss his target", act->actor_name,
// 						act->actor_id);
// 					if (act->shots_count < MAX_SHOTS_QUEUE)
// 						act->shot_type[act->shots_count] = MISSED_SHOT;
// 					no_action = 1;
// 					break;
// 				case missile_critical:
// 					missiles_log_message("%s (%d): will do a critical hit", act->actor_name,
// 						act->actor_id);
// 					if (act->shots_count < MAX_SHOTS_QUEUE)
// 						act->shot_type[act->shots_count] = CRITICAL_SHOT;
// 					no_action = 1;
// 					break;
// 				case unwear_bow:
// 					unwear_item_from_actor_locked(act, KIND_OF_WEAPON);
// 					no_action = 1;
// 					break;
// 				case unwear_quiver:
// 					unwear_item_from_actor_locked(act->actor_id, KIND_OF_SHIELD);
// 					no_action = 1;
// 					break;
			case wait_cmd:
				//horse only
				act->stand_idle=1;
				return;
			// Ok, now the movement, this is the tricky part
			default:
				if (act->que[0] >= move_n && act->que[0] <= move_nw)
				{
					float rotation_angle;
					int dx, dy;
					int step_duration = act->step_duration;
					struct cal_anim *walk_anim;

					attachment_props *att_props = get_attachment_props_if_held(act, attached);
					if (att_props)
					{
						walk_anim = get_pose_frame(act->actor_type, act, attached,
							EMOTE_MOTION(act), 1);
					}
					else
					{
						walk_anim = get_pose_frame(act->actor_type, act, attached,
							EMOTE_MOTION(act), 0);
					}

					act->moving=1;
					act->fighting=0;
					if (last_command < move_n || last_command > move_nw)
					{
						// Update the frame name too
						cal_actor_set_anim(act, attached, *walk_anim);
						act->stop_animation=0;
					}

					if (last_command != act->que[0])
					{
						// Calculate the rotation
						targeted_z_rot=(act->que[0]-move_n)*45.0f;
						rotation_angle=get_rotation_vector(z_rot,targeted_z_rot);
						act->rotate_z_speed=rotation_angle/360.0;

						act->rotate_time_left=360;
						act->rotating=1;
					}
					get_motion_vector(act->que[0], &dx, &dy);

					/* if other move commands are waiting in the queue,
						* we walk at a speed that is close to the server speed
						* else we walk at a slightly slower speed to wait next
						* incoming walking commands */
					if (act->que[1] >= move_n && act->que[1] <= move_nw)
					{
						if (act->que[2] >= move_n && act->que[2] <= move_nw)
						{
							if (act->que[3] >= move_n && act->que[3] <= move_nw)
								act->movement_time_left = (int)(step_duration*0.9); // 3 moves
							else
								act->movement_time_left = step_duration; // 2 moves
						}
						else
						{
							act->movement_time_left = (int)(step_duration*1.1); // 1 move
						}
					}
					else
					{
						act->movement_time_left = (int)(step_duration*1.2); // 0 move
					}
					// if we have a diagonal motion, we slow down the animation a bit
					if (dx != 0 && dy != 0)
						act->movement_time_left = (int)(act->movement_time_left*1.2+0.5);

					// we compute the moving speeds in x, y and z directions
					act->move_x_speed = 0.5*(dx+act->x_tile_pos)-act->x_pos;
					act->move_y_speed = 0.5*(dy+act->y_tile_pos)-act->y_pos;
					act->move_z_speed = get_tile_height(act->x_tile_pos+dx, act->y_tile_pos+dy)
						- act->z_pos;
					act->move_x_speed /= (float)act->movement_time_left;
					act->move_y_speed /= (float)act->movement_time_left;
					act->move_z_speed /= (float)act->movement_time_left;

					/* we change the speed of the walking animation according to the walking speed and to the size of the actor
						* we suppose here that the normal speed of the walking animation is 2 meters per second (1 tile in 250ms) */
					act->cur_anim.duration_scale = walk_anim->duration_scale;
					act->cur_anim.duration_scale *= (float)DEFAULT_STEP_DURATION/(act->movement_time_left*act->scale);
					if (actors_defs[actor_type].actor_scale != 1.0)
						act->cur_anim.duration_scale /= actors_defs[actor_type].actor_scale;
					else
						act->cur_anim.duration_scale /= actors_defs[actor_type].scale;
					if (dx != 0 && dy != 0)
						act->cur_anim.duration_scale *= 1.4142315;
				}
				else if (act->que[0] >= turn_n && act->que[0] <= turn_nw)
				{
					float rotation_angle;

					int horse_angle=0;
					if (is_horse(act))
						horse_angle=(act->fighting && actor_weapon(attached)->turn_horse) ? HORSE_FIGHT_ROTATION : 0;
					else if (horse)
						horse_angle=(act->fighting && actor_weapon(act)->turn_horse) ? HORSE_FIGHT_ROTATION : 0;

					if ((horse || is_horse(act)) && (act->in_aim_mode || attached->in_aim_mode)) {
						horse_angle=HORSE_RANGE_ROTATION;
						act->stand_idle=1;
						//if(act->actor_id==yourself) printf("%i, %s rotates\n",thecount, act->actor_name);
						//if(attached->actor_id==yourself) printf("%i, Horse %s rotates\n",thecount, attached->actor_name);
					}
					targeted_z_rot=(act->que[0] - turn_n) * 45.0f - horse_angle;
					rotation_angle=get_rotation_vector(act->z_rot,targeted_z_rot);
					act->rotate_z_speed=rotation_angle/360.0f;
					act->rotate_time_left=360;
					act->rotating=1;
					act->stop_animation=1;

					if (act->fighting && horse_angle != 0)
						act->horse_rotated=1;
					missiles_log_message("%s (%d): rotation %d requested", act->actor_name,
						act->actor_id, act->que[0] - turn_n);
				}
		} // switch (act->que[0])

		// Mark the actor as being busy
		if (!no_action)
			act->busy=1;
		else if (horse)
			act->in_aim_mode=3; //needed to unfreeze the horse in move_to_next_frame
		//if (act->actor_id==yourself) LOG_TO_CONSOLE(c_green2,"Busy");
		// Save the last command. It is especially good for run and walk
		act->last_command=act->que[0];

		/* We do the enter in aim mode in two steps in order the actor have
			* the time to do the load animation before rotating bones. This is
			* necessary in the case of cross bows where the actor need to use
			* his foot to reload. So here, we don't remove the enter aim mode
			* from the queue in order to treat it again but this time in aim mode.
			*/
		if (act->que[0] == enter_aim_mode && act->in_aim_mode == 0)
		{
			act->in_aim_mode = 1;
			act->last_command=missile_miss; //dirty hack to avoid processing enter_aim_mode twice :/
		}
		else
		{
			unqueue_cmd(act);
		}
	}
}

void next_command(locked_list_ptr list)
{
#ifdef MORE_ATTACHED_ACTORS_DEBUG
	thecount++;
#endif

	for_each_actor_and_attached(list, next_actor_command, NULL);
}

void free_actor_data(actor* act)
{
	if(act->calmodel!=NULL)
		model_delete(act->calmodel);
	if(act->remapped_colors)
	{
		free_actor_texture(act->texture_id);
	}
	if (act->is_enhanced_model)
	{
		free_actor_texture(act->texture_id);
		if (act->body_parts)
		{
			free(act->body_parts);
		}
	}
#ifdef NEW_SOUND
    stop_sound(act->cur_anim_sound_cookie);
    act->cur_anim_sound_cookie = 0;
#endif	//NEW_SOUND
    ec_actor_delete(act);
}

void update_all_actors(int log_the_update)
{
	Uint8 cmd = SEND_ME_MY_ACTORS;

	//we got a nasty error, log it
	if (log_the_update)
		LOG_TO_CONSOLE(c_red2,resync_server);

	destroy_all_actors();
	my_tcp_send(&cmd, 1);
}

int push_command_in_actor_queue(unsigned int command, actor *act)
{
	int k;
	for(k=0;k<MAX_CMD_QUEUE;k++){
		if(act->que[k]==nothing){
			//if we are SEVERLY behind, just update all the actors in range
			if(k>MAX_CMD_QUEUE-2) break;
			else if(k>MAX_CMD_QUEUE-8){
				// is the front a sit/stand spam?
				if((act->que[0]==stand_up||act->que[0]==sit_down)
				   &&(act->que[1]==stand_up||act->que[1]==sit_down)){
					int j;
					//move que down with one command
					for(j=0;j<=k;j++){
						act->que[j]=act->que[j+1];
					}
					act->que[j]=nothing;
					//backup one entry
					k--;
				}

				// is the end a sit/stand spam?
				else if((command==stand_up||command==sit_down)
						&& (act->que[k-1]==stand_up||act->que[k-1]==sit_down)) {
					act->que[k-1]=command;
					break;
				}

			}

			act->que[k]=command;
			break;
		}
	}
	return k;
}


static void sanitize_cmd_queue(actor *act){
	int k,j;
	for(k=0,j=0;k<MAX_CMD_QUEUE-1-j;k++){
		if(act->que[k]==nothing) j++;
		act->que[k]=act->que[k+j];
	}
	for(k=MAX_CMD_QUEUE-1;k>0&&j>0;k--,j--) act->que[k]=nothing;
}

void add_command_to_actor_locked(actor *act, actor *horse, unsigned char command)
{
	//int i=0;
	int k=0;
	int k2 = 0;
	//int have_actor=0;
//if ((act->actor_id==yourself)&&(command==enter_combat)) LOG_TO_CONSOLE(c_green2,"FIGHT!");
	int isme = 0;
#ifdef EXTRA_DEBUG
	ERR();
#endif

	//if (act->actor_id == yourself) printf("ADD COMMAND %i to %i\n",command,act->actor_id);

	if (command == missile_miss) {
		missiles_log_message("%s (%d): will miss his target", act->actor_name, act->actor_id);
		if (act->range_actions_count <= MAX_RANGE_ACTION_QUEUE &&
			act->range_actions_count > 0)
			act->range_actions[act->range_actions_count-1].shot_type = MISSED_SHOT;
		else
			LOG_ERROR("%s (%d): unable to add a missed shot action, the queue is empty!", act->actor_name, act->actor_id);
		return;
	}
	else if (command == missile_critical) {
		missiles_log_message("%s (%d): will do a critical hit", act->actor_name, act->actor_id);
		if (act->range_actions_count <= MAX_RANGE_ACTION_QUEUE &&
			act->range_actions_count > 0)
			act->range_actions[act->range_actions_count-1].shot_type = CRITICAL_SHOT;
		else
			LOG_ERROR("%s (%d): unable to add a critical shot action, the queue is empty!", act->actor_name, act->actor_id);
		return;
	}
	else if (command == aim_mode_reload) {
		missiles_log_message("%s (%d): reload after next fire", act->actor_name, act->actor_id);
		if (act->range_actions_count <= MAX_RANGE_ACTION_QUEUE &&
			act->range_actions_count > 0)
			act->range_actions[act->range_actions_count-1].reload = 1;
		else
			LOG_ERROR("%s (%d): unable to add a reload action, the queue is empty!", act->actor_name, act->actor_id);
		return;
	}
	else if (command == enter_aim_mode)
	{
		missiles_log_message("%s (%d): adding enter_aim_mode command",
								act->actor_name, act->actor_id);
	}
	else if (command == leave_aim_mode)
	{
		missiles_log_message("%s (%d): adding leave_aim_mode command",
								act->actor_name, act->actor_id);
	}


	if(command==leave_combat||command==enter_combat||command==die1||command==die2)
	{
		int j= 0;

		//Strip the queue for attack messages
		for(k=0; k<MAX_CMD_QUEUE; k++){
			switch(act->que[k]){
				case pain1:
				case pain2:
				case attack_up_1:
				case attack_up_2:
				case attack_up_3:
				case attack_up_4:
				case attack_down_1:
				case attack_down_2:
					act->que[k]= nothing;
					break;

				default:
					act->que[j]= act->que[k];
					j++;
					if(j<=k){
						act->que[k]= nothing;
					}
					break;
			}
		}

		if (horse) {
			//strip horse queue too
			int j=0;

			for(k=0; k<MAX_CMD_QUEUE; k++){
				switch(horse->que[k]){
					case pain1:
					case pain2:
					case attack_up_1:
					case attack_up_2:
					case attack_up_3:
					case attack_up_4:
					case attack_down_1:
					case attack_down_2:
						horse->que[k]= nothing;
						break;

					default:
						horse->que[j]= horse->que[k];
						j++;
						if(j<=k){
							horse->que[k]= nothing;
						}
						break;
				}
			}
		}



		if(act->last_command == nothing)
		{
			//We may be on idle, update the actor so we can reduce the rendering lag
			CalModel_Update(act->calmodel, 5.0f);
			build_actor_bounding_box(act);
			missiles_rotate_actor_bones(act);
			if (use_animation_program)
			{
				set_transformation_buffers(act);
			}
		}
	}

	k = push_command_in_actor_queue(command, act);

	if (horse){
		//if in aim mode, ignore turning and ranging related commands. We do it manually in next_command()
		switch(command){
			case enter_aim_mode:
			case leave_aim_mode:
			case enter_combat:
			case leave_combat:
			case aim_mode_fire:
				//insert a wait_cmd where the horse must synch with the actor
				k2 = push_command_in_actor_queue(wait_cmd, horse);
				break;
			default:
				k2 = push_command_in_actor_queue(command, horse);
		}
	}
	else
	{
		k2 = k;
	}

	isme = act->actor_id == yourself;

	//if(act->actor_id==yourself) printf("COMMAND: %i at pos %i (and %i)\n",command,k,k2);
	//if(act->actor_id==yourself) print_queue(act);
	//Reduce resync in invasions
	if(command==enter_combat) {
		// we received an enter_combat, look back in the queue
		// if a leave_combat is found and all the commands in between
		// are turning commands, just ignore the leave and the enter combat commands

		int j=k-1;
		int j2=k2-1;
		while(act->que[j]>=turn_n&&act->que[j]<=turn_nw&&j>=0) j--; //skip rotations
		if (horse)
			while(horse->que[j2]>=turn_n
				&&horse->que[j2]<=turn_nw
				&&j2>=0) j2--; //skip rotations for horse
		if(j>=0&&act->que[j]==leave_combat) {
			//remove leave_combat and enter_combat
			act->que[j]=nothing;
			act->que[k]=nothing;
			sanitize_cmd_queue(act);
			//if(act->actor_id==yourself) printf("   actor %s: skipped %i and %i\n",act->actor_name,j,k);
			if(horse && j2 >= 0 && horse->que[j2] == wait_cmd)
			{
				//remove leave_combat and enter_combat for horse
				horse->que[j2]=nothing;
				horse->que[k2]=nothing;
				sanitize_cmd_queue(horse);
				//if(act->actor_id==yourself) printf("   horse %s: skipped %i and %i\n",act->actor_name,j2,k2);
			}
		}

		//if(act->actor_id==yourself) printf("   ***Skip Done***\n");
		//if(act->actor_id==yourself) print_queue(act);
	}




	switch(command) {
	case enter_combat:
		act->async_fighting= 1;
		if (isme)
			check_to_auto_disable_ranging_lock();
		break;
	case leave_combat:
		act->async_fighting= 0;
		break;
	case move_n:
	case run_n:
		act->async_y_tile_pos++;
		act->async_z_rot= 0;
		if(isme && pf_follow_path)
		{
			if(checkvisitedlist(act->async_x_tile_pos,act->async_y_tile_pos))
				pf_destroy_path();
		}
		break;
	case move_ne:
	case run_ne:
		act->async_x_tile_pos++;
		act->async_y_tile_pos++;
		act->async_z_rot= 45;
		if(isme && pf_follow_path)
		{
			if(checkvisitedlist(act->async_x_tile_pos,act->async_y_tile_pos))
				pf_destroy_path();
		}
		break;
	case move_e:
	case run_e:
		act->async_x_tile_pos++;
		act->async_z_rot= 90;
		if(isme && pf_follow_path)
		{
			if(checkvisitedlist(act->async_x_tile_pos,act->async_y_tile_pos))
				pf_destroy_path();
		}
		break;
	case move_se:
	case run_se:
		act->async_x_tile_pos++;
		act->async_y_tile_pos--;
		act->async_z_rot= 135;
		if(isme && pf_follow_path)
		{
			if(checkvisitedlist(act->async_x_tile_pos,act->async_y_tile_pos))
				pf_destroy_path();
		}
		break;
	case move_s:
	case run_s:
		act->async_y_tile_pos--;
		act->async_z_rot= 180;
		if(isme && pf_follow_path)
		{
			if(checkvisitedlist(act->async_x_tile_pos,act->async_y_tile_pos))
				pf_destroy_path();
		}
		break;
	case move_sw:
	case run_sw:
		act->async_x_tile_pos--;
		act->async_y_tile_pos--;
		act->async_z_rot= 225;
		if(isme && pf_follow_path)
		{
			if(checkvisitedlist(act->async_x_tile_pos,act->async_y_tile_pos))
				pf_destroy_path();
		}
		break;
	case move_w:
	case run_w:
		act->async_x_tile_pos--;
		act->async_z_rot= 270;
		if(isme && pf_follow_path)
		{
			if(checkvisitedlist(act->async_x_tile_pos,act->async_y_tile_pos))
				pf_destroy_path();
		}
		break;
	case move_nw:
	case run_nw:
		act->async_x_tile_pos--;
		act->async_y_tile_pos++;
		act->async_z_rot= 315;
		if(isme && pf_follow_path)
		{
			if(checkvisitedlist(act->async_x_tile_pos,act->async_y_tile_pos))
				pf_destroy_path();
		}
		break;
	case turn_n:
	case turn_ne:
	case turn_e:
	case turn_se:
	case turn_s:
	case turn_sw:
	case turn_w:
	case turn_nw:
			act->async_z_rot= (command-turn_n)*45;
			break;
	}

	if (k != k2) {
		LOG_ERROR("Inconsistency between queues of attached actors %s (%d) and %s (%d)!",
					act->actor_name,
					act->actor_id,
					horse->actor_name,
					horse->actor_id);
	}
	else
	if(k>MAX_CMD_QUEUE-2){
		int i;
		int k;
		if (max_fps == limit_fps)
			LOG_ERROR("Too much commands in the queue for actor %d (%s) => skip emotes!",
					act->actor_id, act->actor_name);
		for(i=MAX_CMD_QUEUE-1;i>=0;i--) {
			if(act->que[i]>=emote_cmd&&act->que[i]<wait_cmd) {
				//skip this emote
				for(k=i;k<MAX_CMD_QUEUE-1;k++) {
					act->que[k]=act->que[k+1];
					if (horse)
						horse->que[k] = horse->que[k+1];
				}
				act->que[k]=nothing;
				horse->que[k]=nothing;
				add_command_to_actor_locked(act, horse, command); //RECURSIVE!!!! should be done only one time
				return;
			}
		}
		//if we are here no emotes have been skipped
		if (max_fps == limit_fps)
		{
			LOG_ERROR("Too much commands in the queue for actor %d (%s) => resync!\n",
				act->actor_id, act->actor_name);
#ifdef	ANIMATION_SCALING
			LOG_ERROR("animation_scale: %f\n", act->animation_scale);
#endif	/* ANIMATION_SCALING */
			for (i = 0; i < MAX_CMD_QUEUE; ++i)
				LOG_ERROR("%dth command in the queue: %d\n", i, (int)act->que[i]);
		}
		update_all_actors(max_fps == limit_fps);
	}
}

void add_command_to_actor(int actor_id, unsigned char command)
{
	locked_list_ptr actors_list;
	actor *act, *horse;
	actors_list = lock_and_get_actor_and_attached_from_id(actor_id, &act, &horse);
	if (!actors_list)
	{
		// Resync
		// If we got here, it means we don't have this actor, so get it from the server...
		LOG_ERROR("%s %d - %d\n", cant_add_command, command, actor_id);
	}
	else
	{
		add_command_to_actor_locked(act, horse, command);
		release_locked_actors_list_and_invalidate2(actors_list, &act, &horse);
	}
}


static void queue_emote(actor *act, emote_data *emote){
	int j,k;
	
	for (k = 0; k < MAX_EMOTE_QUEUE; k++) {
		if (act->emote_que[k].origin!=SERVER_EMOTE) {
			if (k <= MAX_EMOTE_QUEUE - 2) {
				for(j=MAX_EMOTE_QUEUE-1;j>k;j--) //throw away last client_emote if necessary
					act->emote_que[j]=act->emote_que[j-1];
				act->emote_que[k].emote = emote;
				act->emote_que[k].origin = SERVER_EMOTE;
				act->emote_que[k].create_time = cur_time;
				//printf("in queue SERVER: %p at %i in %i\n\n",emote,cur_time,k);
			}
			break;
		}
	}
	if (k > MAX_EMOTE_QUEUE - 2)
		LOG_ERROR("Too many commands in the emote queue for actor %d (%s)", act->actor_id, act->actor_name);


}

void add_emote_to_actor(int actor_id, int emote_id)
{
	locked_list_ptr actors_list;
	actor *act;
	emote_data *emote;
	hash_entry *he;

	//printf("SERVER MSG\nwe have actor %i %p\n",actor_id,act);
	if(emote_id!=0)
	{
		//dirty, but avoids warnings :P
		he=hash_get(emotes, (void *)(uintptr_t)emote_id);
		if(!he) {
			LOG_ERROR("%s (Emote) %i- NULL emote passed", cant_add_command,emote_id);
			return;
		}
		emote = he->item;
	}
	else
	{
		emote=NULL;
	}

	actors_list = lock_and_get_actor_from_id(actor_id, &act);
	if (!actors_list)
	{
		LOG_ERROR("%s (Emote) %d - NULL actor passed", cant_add_command, emote_id);
	}
	else
	{
		//printf("emote message to be added %p\n",emote);
		queue_emote(act,emote);
		release_locked_actors_list_and_invalidate(actors_list, &act);
	}
}


void get_actor_damage(int actor_id, int damage)
{
	actor *act;
	float blood_level;
	float bone_list[1024][3];
	int total_bones;
	int bone;
	float bone_x, bone_y, bone_z;
	locked_list_ptr actors_list = get_locked_actors_list();

#ifdef EXTRA_DEBUG
	ERR();
#endif

	if (!get_self(actors_list))
	{
		release_locked_actors_list(actors_list);
		return;
	}

	act = get_actor_from_id(actors_list, actor_id);
	if (!act)
	{
		//if we got here, it means we don't have this actor, so get it from the server...
		release_locked_actors_list(actors_list);
		return;
	}

	if(floatingmessages_enabled){
		act->last_health_loss=cur_time;
	}

	if (actor_id == yourself)
		set_last_damage(damage);

	act->damage=damage;
	act->damage_ms=2000;
	if(act->cur_health > (Uint16)damage)
		act->cur_health -= damage;
	else
		act->cur_health = 0;

	if (act->cur_health <= 0) {
#ifdef NEW_SOUND
		add_death_sound(act);
#endif // NEW_SOUND
		increment_death_counter(act, actors_list);
	}
	act->last_range_attacker_id = -1;

	if (use_eye_candy && enable_blood)
	{
		if (strcmp(act->actor_name, "Gargoyle") && strcmp(act->actor_name, "Skeleton") && strcmp(act->actor_name, "Phantom Warrior"))	//Ideally, we'd also check to see if it was a player or not, but since this is just cosmetic...
		{
			blood_level=(int)powf(damage / powf(act->max_health, 0.5), 0.75) + 0.5;
			total_bones = CalSkeleton_GetBonePoints(CalModel_GetSkeleton(act->calmodel), &bone_list[0][0]);
			bone = rand() % total_bones;
			bone_x = bone_list[bone][0] + act->x_pos + 0.25;
			bone_y = bone_list[bone][1] + act->y_pos + 0.25;
			bone_z = bone_list[bone][2] + ec_get_z(act);
//				printf("ec_create_impact_blood((%f %f, %f), (%f, %f, %f), %d, %f);", bone_x, bone_y, bone_z, ((float)rand()) * blood_level / RAND_MAX / 13.0, ((float)rand()) * blood_level / RAND_MAX / 13.0, ((float)rand()) * blood_level / RAND_MAX / 13.0, (poor_man ? 6 : 10), blood_level);
			ec_create_impact_blood(bone_x, bone_y, bone_z, (float)rand() * blood_level / (RAND_MAX * 13.0), (float)rand() * blood_level / (RAND_MAX * 13.0), (float)rand() * blood_level / (RAND_MAX * 13.0), (poor_man ? 6 : 10), blood_level);
		}
	}

	release_locked_actors_list_and_invalidate(actors_list, &act);
}

void get_actor_heal(int actor_id, int quantity)
{
	locked_list_ptr actors_list;
	actor *act;
#ifdef EXTRA_DEBUG
	ERR();
#endif
	actors_list = lock_and_get_actor_from_id(actor_id, &act);
	if(!actors_list)
		//if we got here, it means we don't have this actor, so get it from the server...
		return;

	if (actor_id == yourself)
		set_last_heal(quantity);

	if(floatingmessages_enabled){
		act->damage=-quantity;
		act->damage_ms=2000;
		act->last_health_loss=cur_time;
	}

	act->cur_health+=quantity;

	release_locked_actors_list_and_invalidate(actors_list, &act);
}

void get_actor_health(int actor_id, int quantity)
{
	//int i=0;
	locked_list_ptr actors_list;
	actor *act;
#ifdef EXTRA_DEBUG
	ERR();
#endif
	actors_list = lock_and_get_actor_from_id(actor_id, &act);
	if (!actors_list)
		//if we got here, it means we don't have this actor, so get it from the server...
		return;

//	if(floatingmessages_enabled){
		//act->damage=-quantity;
		//act->damage_ms=2000;
		//act->last_health_loss=cur_time;
//	}

	act->max_health=quantity;

	release_locked_actors_list_and_invalidate(actors_list, &act);
}

void move_self_forward()
{
	int x, y, rot;
	short tx,ty;
	actor *me;
	locked_list_ptr actors_list = lock_and_get_self(&me);
	if (!actors_list)
		return;

	x=me->x_tile_pos;
	y=me->y_tile_pos;
	rot=(int)rint(me->z_rot/45.0f);

	release_locked_actors_list_and_invalidate(actors_list, &me);

	if (rot < 0) rot += 8;
	switch(rot) {
		case 8: //360
		case 0: //0
			tx=x;
			ty=y+1;
			break;
		case 1: //45
			tx=x+1;
			ty=y+1;
			break;
		case 2: //90
			tx=x+1;
			ty=y;
			break;
		case 3: //135
			tx=x+1;
			ty=y-1;
			break;
		case 4: //180
			tx=x;
			ty=y-1;
			break;
		case 5: //225
			tx=x-1;
			ty=y-1;
			break;
		case 6: //270
			tx=x-1;
			ty=y;
			break;
		case 7: //315
			tx=x-1;
			ty=y+1;
			break;
		default:
			tx=x;
			ty=y;
	}

	//check to see if the coordinates are inside the map
	if (ty >= 0 && tx >= 0 && tx < tile_map_size_x*6 && ty < tile_map_size_y*6)
	{
		if (pf_follow_path) {
			pf_destroy_path();
		}

		move_to(&tx, &ty, 0);
	}
}


static inline void actor_check_string(actor_types *act, const char *section, const char *type, const char *value)
{
	if (value == NULL || *value=='\0')
	{
#ifdef DEBUG
		LOG_ERROR("Data Error in %s(%d): Missing %s.%s", act->actor_name, act->actor_type, section, type);
#endif // DEBUG
	}
}

static inline void actor_check_int(actor_types *act, const char *section, const char *type, int value)
{
	if (value < 0)
	{
#ifdef DEBUG
		LOG_ERROR("Data Error in %s(%d): Missing %s.%s", act->actor_name, act->actor_type, section, type);
#endif // DEBUG
	}
}



void add_emote_to_dict(const xmlNode *node, emote_data *emote)
{
	emote_dict *entry=NULL;
	if(!emote_cmds)
		emote_cmds= create_hash_table(EMOTE_CMDS_HASH,hash_fn_str,cmp_fn_str,free);

	entry = (emote_dict*) malloc(sizeof(emote_dict));
	get_string_value (entry->command, sizeof(entry->command), node);
	entry->emote = emote;
	hash_add(emote_cmds,(void*)entry->command,(void*)entry);
	//printf("Adding Emote Command: [%s], emote %p\n",entry->command,entry->emote);

}

void init_emote(emote_data *emote){
	int i;

	emote->timeout=EMOTE_TIMEOUT;
	emote->barehanded=0;
	for(i=0;i<EMOTE_ACTOR_TYPES;i++){
		emote->anims[i][EMOTE_SITTING][0]=emote->anims[i][EMOTE_SITTING][1]=NULL;
		emote->anims[i][EMOTE_WALKING][0]=emote->anims[i][EMOTE_WALKING][1]=NULL;
		emote->anims[i][EMOTE_RUNNING][0]=emote->anims[i][EMOTE_RUNNING][1]=NULL;
		emote->anims[i][EMOTE_STANDING][0]=emote->anims[i][EMOTE_STANDING][1]=NULL;
	}
	emote->name[0]=0;
	emote->desc[0]=0;
}


// 0=f 1=m <0=any, race 0=human, 1=elf, 2=dwarf, 3=orchan, 4=gnome, 5=draegoni, 6=monster
void calc_actor_types(int sex, int race, int *buf, int *len){

	buf[0]=emote_actor_type(human_female);
	buf[1]=emote_actor_type(human_male);
	buf[2]=emote_actor_type(elf_female);
	buf[3]=emote_actor_type(elf_male);
	buf[4]=emote_actor_type(dwarf_female);
	buf[5]=emote_actor_type(dwarf_male);
	buf[6]=emote_actor_type(orchan_female);
	buf[7]=emote_actor_type(orchan_male);
	buf[8]=emote_actor_type(gnome_female);
	buf[9]=emote_actor_type(gnome_male);
	buf[10]=emote_actor_type(draegoni_female);
	buf[11]=emote_actor_type(draegoni_male);
	buf[12]=emote_actor_type(-1); //all other mobs
	buf[13]=emote_actor_type(-1); //all other mobs
	

	if(sex<0&&race<0){
		*len=14;	
	} else if (sex<0) {
		*len=2;
		buf[0]=buf[race*2];
		buf[1]=buf[race*2+1];		
	} else if(race<0) {
		int i;
		*len=7;
		for(i=sex;i<14;i+=2) buf[i/2]=buf[i];
	} else {
		*len=1;
		buf[0]=buf[race*2+sex];
	}	
}

void set_emote_anim(emote_data *emote, emote_frame *frames, int sex, int race, int held, int idle){
	int buf[14];
	int len;
	int i,h;
	
	h=(held<=0) ? 0:1;
	calc_actor_types(sex,race,buf,&len);
	for(i=0;i<len;i++) {
		if(held<0)
		 	emote->anims[buf[i]][idle][1-h]=frames;
	 	emote->anims[buf[i]][idle][h]=frames;
	}
}


void flag_emote_frames(emote_data *emote,emote_frame *frame){
	int i,j,k;
	for(i=0;i<EMOTE_ACTOR_TYPES;i++)
		for(j=0;j<4;j++)
			for(k=0;k<2;k++)
				if(emote->anims[i][j][k]==frame)
					emote->anims[i][j][k]=NULL;
}

void free_emote_data(void *data)
{
	emote_data *emote = data;
	emote_frame *head, *frame, *tf;
	int i, j, k;
	if (!emote)
		return;

	for (i = 0; i < EMOTE_ACTOR_TYPES; i++)
	{
		for (j = 0; j < 4; j++)
		{
			for (k = 0; k < 2; k++)
			{
				head = emote->anims[i][j][k];
				if (!head)
					continue;

				flag_emote_frames(emote, head);

				for (frame = head, tf = head->next; tf; frame = tf, tf = tf->next)
					free(frame);
				free(frame);
			}
		}
	}
	free(emote);
}

emote_data *new_emote(int id){

	emote_data *emote;

	if(!emotes)
		emotes= create_hash_table(2*EMOTE_CMDS_HASH,hash_fn_int,cmp_fn_int,free_emote_data);
	emote=(emote_data*)calloc(1,sizeof(emote_data));
	init_emote(emote);
	emote->id=id;
	hash_add(emotes,(void *)(uintptr_t)id,(void*)emote);
	return emote;
}

void get_emote_props(const xmlNode *item, int *sex, int *race, int *held)
{
	const char *s_sex,*s_race,*s_held;
	*sex=-1;
	*race=-1;
	*held=-1;

	s_sex=get_string_property(item,"sex");
	s_race=get_string_property(item,"race");
	s_held=get_string_property(item,"held");

	if(!strcasecmp(s_sex,"m")) *sex=1;
	if(!strcasecmp(s_sex,"f")) *sex=0;

	if(!strcasecmp(s_race,"human")) *race=0;
	if(!strcasecmp(s_race,"elf")) *race=1;
	if(!strcasecmp(s_race,"dwarf")) *race=2;
	if(!strcasecmp(s_race,"orchan")) *race=3;
	if(!strcasecmp(s_race,"gnome")) *race=4;
	if(!strcasecmp(s_race,"draegoni")) *race=5;
	if(!strcasecmp(s_race,"monster")) *race=6;

	if(!strcasecmp(s_held,"true")||!strcasecmp(s_held,"1")) *held=1;
	if(!strcasecmp(s_held,"false")||!strcasecmp(s_held,"0")) *held=0;
}


emote_frame *get_emote_frames(const xmlNode *node)
{
	const xmlNode *item;
	emote_frame *frames=NULL,*head=NULL;
	char tmp[100];

	for (item = node->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"anim") == 0) {
				int i,j,k;
				char c;
				i=j=k=0;
				if(!head){
					head=calloc(1,sizeof(emote_frame));
					frames=head;
				} else {
					frames->next=calloc(1,sizeof(emote_frame));
					frames=frames->next;
				}
				get_string_value(tmp, sizeof(tmp), item);
				do {
					j=i;
					while(tmp[i]!='|'&&tmp[i]) i++;
					c=tmp[i];
					tmp[i]=0;
					frames->ids[k++]=atoi(&tmp[j]);
					tmp[i]=c;
					if(c)i++;
				} while(tmp[i]&&k<MAX_EMOTE_FRAME);
				frames->nframes=k;
			} else {
				LOG_ERROR("unknown emote property \"%s\"", item->name);
			}
		}
	}
	return head;

}

int parse_emote_def(emote_data *emote, const xmlNode *node)
{
	const xmlNode *item;
	int ok,s,r,h;
	const char *bare,*pose;

	if (node == NULL || node->children == NULL) return 0;


	bare = get_string_property(node,"barehanded"); //returns "" if not specified
	emote->barehanded = (bare[0]=='L' || bare[0]=='B') ? EMOTE_BARE_L:0;
	emote->barehanded |= (bare[0]=='R' || bare[0]=='B') ? EMOTE_BARE_R:0;

	pose = get_string_property(node,"pose"); //returns "" if not specified
	emote->pose=EMOTE_STANDING+1;
	if(!strcasecmp(pose,"sitting")) emote->pose = EMOTE_SITTING;
	else if(!strcasecmp(pose,"walking")) emote->pose = EMOTE_WALKING;
	else if(!strcasecmp(pose,"standing")) emote->pose = EMOTE_STANDING;
	else if(!strcasecmp(pose,"running")) emote->pose = EMOTE_RUNNING;


	ok = 1;
	for (item = node->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"command") == 0) {
				add_emote_to_dict(item,emote);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"timeout") == 0) {
				emote->timeout=get_int_value(item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"name") == 0) {
				get_string_value(emote->name, sizeof(emote->name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"desc") == 0) {
				get_string_value(emote->desc, sizeof(emote->desc), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"default") == 0) {
				emote_frame *frames=get_emote_frames(item);
				get_emote_props(item,&s,&r,&h);
				set_emote_anim(emote,frames,s,r,h,EMOTE_SITTING);
				set_emote_anim(emote,frames,s,r,h,EMOTE_WALKING);
				set_emote_anim(emote,frames,s,r,h,EMOTE_STANDING);
				set_emote_anim(emote,frames,s,r,h,EMOTE_RUNNING);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"sitting") == 0) {
				get_emote_props(item,&s,&r,&h);
				set_emote_anim(emote,get_emote_frames(item),s,r,h,EMOTE_SITTING);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"walking") == 0) {
				get_emote_props(item,&s,&r,&h);
				set_emote_anim(emote,get_emote_frames(item),s,r,h,EMOTE_WALKING);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"standing") == 0) {
				get_emote_props(item,&s,&r,&h);
				set_emote_anim(emote,get_emote_frames(item),s,r,h,EMOTE_STANDING);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"running") == 0) {
				get_emote_props(item,&s,&r,&h);
				set_emote_anim(emote,get_emote_frames(item),s,r,h,EMOTE_RUNNING);
			} else {
				LOG_ERROR("unknown emote property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	return ok;
}


int parse_emotes_defs(const xmlNode *node)
{
	const xmlNode *def;
	emote_data *emote = NULL;
	int ok = 1;

	for (def = node->children; def; def = def->next) {
		if (def->type == XML_ELEMENT_NODE)
			if (xmlStrcasecmp(def->name, (xmlChar*)"emote") == 0) {
				int id = get_int_property(def, "id");
				if (id < 0) {
					LOG_ERROR("Unable to find id property %s\n", def->name);
					ok=0;
				} else {
					emote=new_emote(id);
					ok &= parse_emote_def(emote, def);
				}
			} else if (xmlStrcasecmp(def->name, (xmlChar*)"frames") == 0) {
				int act_type = get_int_property(def, "actor_type");
				if (act_type < 0) {
					ok&=parse_actor_frames(&actors_defs[human_female], def->children, NULL);
					ok&=parse_actor_frames(&actors_defs[human_male], def->children, NULL);
					ok&=parse_actor_frames(&actors_defs[elf_female], def->children, NULL);
					ok&=parse_actor_frames(&actors_defs[elf_male], def->children, NULL);
					ok&=parse_actor_frames(&actors_defs[dwarf_female], def->children, NULL);
					ok&=parse_actor_frames(&actors_defs[dwarf_male], def->children, NULL);
					ok&=parse_actor_frames(&actors_defs[orchan_female], def->children, NULL);
					ok&=parse_actor_frames(&actors_defs[orchan_male], def->children, NULL);
					ok&=parse_actor_frames(&actors_defs[gnome_female], def->children, NULL);
					ok&=parse_actor_frames(&actors_defs[gnome_male], def->children, NULL);
					ok&=parse_actor_frames(&actors_defs[draegoni_female], def->children, NULL);
					ok&=parse_actor_frames(&actors_defs[draegoni_male], def->children, NULL);
				} else ok &= parse_actor_frames(&actors_defs[act_type], def->children, NULL);
			} else{
				LOG_ERROR("parse error: emote or include expected");
				ok = 0;
			}
		else if (def->type == XML_ENTITY_REF_NODE) {
			ok &= parse_emotes_defs(def->children);
		}
	}

	return ok;
}

int read_emotes_defs(void)
{
	const xmlNode *root;
	xmlDoc *doc;
	const char *fname = "emotes.xml";
	int ok = 1;

	doc = xmlReadFile(fname, NULL, 0);
	if (doc == NULL) {
		LOG_ERROR("Unable to read emotes definition file %s", fname);
		return 0;
	}

	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		LOG_ERROR("Unable to parse emotes definition file %s", fname);
		ok = 0;
	} else if (xmlStrcasecmp(root->name, (xmlChar*)"emotes") != 0) {
		LOG_ERROR("Unknown key \"%s\" (\"emotes\" expected).", root->name);
		ok = 0;
	} else {
		ok = parse_emotes_defs(root);
	}

	xmlFreeDoc(doc);
	return ok;
}



void free_emotes()
{
	int i;
	for(i=0;i<MAX_ACTOR_DEFS;i++)
		destroy_hash_table(actors_defs[i].emote_frames);
	destroy_hash_table(emote_cmds);
	destroy_hash_table(emotes);
}

const xmlNode *get_default_node(const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *item;
	const char *group;

	// first, check for errors
	if(defaults == NULL || cfg == NULL)
		return NULL;

	//lets find out what group to look for
	group = get_string_property(cfg, "group");

	// look for defaul entries with the same name
	for(item=defaults->children; item; item=item->next){
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, cfg->name) == 0){
				const char *item_group;

				item_group = get_string_property(item, "group");
				// either both have no group, or both groups match
				if(xmlStrcasecmp((xmlChar*)item_group, (xmlChar*)group) == 0){
					// this is the default entry we want then!
					return item;
				}
			}
		}
	}

	// if we got here, there is no default node that matches
	return NULL;
}

int parse_actor_shirt(actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *item;
	int ok, col_idx;
	shirt_part *shirt;

	if(cfg == NULL || cfg->children == NULL) return 0;

	col_idx= get_int_property(cfg, "id");
/*	if(col_idx < 0){
		col_idx= get_property(cfg, "color", "shirt color", shirt_color_dict);
	}
*/
	if(col_idx < 0 || col_idx >= actor_part_sizes[ACTOR_SHIRT_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->shirt == NULL) {
		int i;
		act->shirt = (shirt_part*)calloc(actor_part_sizes[ACTOR_SHIRT_SIZE], sizeof(shirt_part));
		for (i = actor_part_sizes[ACTOR_SHIRT_SIZE]; i--;) act->shirt[i].mesh_index= -1;
	}

	shirt= &(act->shirt[col_idx]);
	ok= 1;
	for(item=cfg->children; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp (item->name, (xmlChar*)"arms") == 0) {
				get_string_value(shirt->arms_name, sizeof(shirt->arms_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"mesh") == 0) {
				get_string_value(shirt->model_name, sizeof(shirt->model_name), item);
				shirt->mesh_index= cal_load_mesh(act, shirt->model_name, "shirt");
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"torso") == 0) {
				get_string_value(shirt->torso_name, sizeof(shirt->torso_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"armsmask") == 0) {
				get_string_value(shirt->arms_mask, sizeof(shirt->arms_mask), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"torsomask") == 0) {
				get_string_value(shirt->torso_mask, sizeof(shirt->torso_mask), item);
			} else {
				LOG_ERROR("unknown shirt property \"%s\"", item->name);
				ok= 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		const xmlNode *default_node = get_default_node(cfg, defaults);

		if(default_node){
			if (*shirt->arms_name=='\0')
				get_item_string_value(shirt->arms_name, sizeof(shirt->arms_name), default_node, (xmlChar*)"arms");
			if (*shirt->model_name=='\0'){
				get_item_string_value(shirt->model_name, sizeof(shirt->model_name), default_node, (xmlChar*)"mesh");
				shirt->mesh_index= cal_load_mesh(act, shirt->model_name, "shirt");
			}
			if (*shirt->torso_name=='\0')
				get_item_string_value(shirt->torso_name, sizeof(shirt->torso_name), default_node, (xmlChar*)"torso");
		}
	}

	// check the critical information
	actor_check_string(act, "shirt", "arms", shirt->arms_name);
	actor_check_string(act, "shirt", "model", shirt->model_name);
	actor_check_int(act, "shirt", "mesh", shirt->mesh_index);
	actor_check_string(act, "shirt", "torso", shirt->torso_name);

	return ok;
}

int parse_actor_skin (actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *item;
	int ok, col_idx;
	skin_part *skin;

	if (cfg == NULL || cfg->children == NULL) return 0;

	col_idx= get_int_property(cfg, "id");
	if(col_idx < 0){
		col_idx= get_property(cfg, "color", "skin color", skin_color_dict);
	}

	if(col_idx < 0 || col_idx >= actor_part_sizes[ACTOR_SKIN_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->skin == NULL) {
		int i;
		act->skin = (skin_part*)calloc(actor_part_sizes[ACTOR_SKIN_SIZE], sizeof(skin_part));
		for (i = actor_part_sizes[ACTOR_SKIN_SIZE]; i--;) act->skin[i].mesh_index= -1;
	}

	skin = &(act->skin[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"hands") == 0) {
				get_string_value (skin->hands_name, sizeof (skin->hands_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"head") == 0) {
				get_string_value (skin->head_name, sizeof (skin->head_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"torso") == 0) {
				get_string_value (skin->body_name, sizeof (skin->body_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"arms") == 0) {
				get_string_value (skin->arms_name, sizeof (skin->arms_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"legs") == 0) {
				get_string_value (skin->legs_name, sizeof (skin->legs_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"feet") == 0) {
				get_string_value (skin->feet_name, sizeof (skin->feet_name), item);
			} else {
				LOG_ERROR("unknown skin property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		const xmlNode *default_node= get_default_node(cfg, defaults);

		if(default_node){
			if(*skin->hands_name=='\0')
				get_item_string_value(skin->hands_name, sizeof(skin->hands_name), default_node, (xmlChar*)"hands");
			if(*skin->head_name=='\0')
				get_item_string_value(skin->head_name, sizeof(skin->head_name), default_node, (xmlChar*)"head");
		}
	}

	// check the critical information
	actor_check_string(act, "skin", "hands", skin->hands_name);
	actor_check_string(act, "skin", "head", skin->head_name);


	return ok;
}

int parse_actor_legs (actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *item;
	int ok, col_idx;
	legs_part *legs;

	if (cfg == NULL || cfg->children == NULL) return 0;

	col_idx= get_int_property(cfg, "id");
/*	if(col_idx < 0){
		col_idx= get_property(cfg, "color", "legs color", legs_color_dict);
	}
*/
	if(col_idx < 0 || col_idx >= actor_part_sizes[ACTOR_LEGS_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->legs == NULL) {
		int i;
		act->legs = (legs_part*)calloc(actor_part_sizes[ACTOR_LEGS_SIZE], sizeof(legs_part));
		for (i = actor_part_sizes[ACTOR_LEGS_SIZE]; i--;) act->legs[i].mesh_index= -1;
	}

	legs = &(act->legs[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"skin") == 0) {
				get_string_value (legs->legs_name, sizeof (legs->legs_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"mesh") == 0) {
				get_string_value (legs->model_name, sizeof (legs->model_name), item);
				legs->mesh_index = cal_load_mesh (act, legs->model_name, "legs");
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"legsmask") == 0) {
				get_string_value (legs->legs_mask, sizeof (legs->legs_mask), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"glow") == 0) {
				int mode = find_description_index (glow_mode_dict, (char*)item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				legs->glow = mode;
			} else {
				LOG_ERROR("unknown legs property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		const xmlNode *default_node = get_default_node(cfg, defaults);

		if(default_node){
			if (*legs->legs_name=='\0')
				get_item_string_value(legs->legs_name, sizeof(legs->legs_name), default_node, (xmlChar*)"skin");
			if (*legs->model_name=='\0'){
				get_item_string_value(legs->model_name, sizeof(legs->model_name), default_node, (xmlChar*)"mesh");
				legs->mesh_index= cal_load_mesh(act, legs->model_name, "legs");
			}
		}
	}

	// check the critical information
	actor_check_string(act, "legs", "skin", legs->legs_name);
	actor_check_string(act, "legs", "model", legs->model_name);
	actor_check_int(act, "legs", "mesh", legs->mesh_index);


	return ok;
}

int parse_actor_weapon_detail (actor_types *act, weapon_part *weapon, const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *item;
	char str[255];
	char name[256];
	int ok, index;

	if (cfg == NULL || cfg->children == NULL) return 0;

	ok = 1;
	for (item = cfg->children; item; item = item->next)
	{
		if (item->type == XML_ELEMENT_NODE)
		{
			safe_strncpy(name, (const char*)item->name, sizeof(name));
			my_tolower(name);

			if (!strcmp(name, "mesh")) {
				get_string_value (weapon->model_name, sizeof (weapon->model_name), item);
				weapon->mesh_index = cal_load_weapon_mesh (act, weapon->model_name, "weapon");
			} else if (!strcmp(name, "skin")) {
				get_string_value (weapon->skin_name, sizeof (weapon->skin_name), item);
			} else if (!strcmp(name, "skinmask")) {
				get_string_value (weapon->skin_mask, sizeof (weapon->skin_mask), item);
			} else if (!strcmp(name, "glow")) {
				int mode = find_description_index(glow_mode_dict, (char*)item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				weapon->glow = mode;
#ifdef NEW_SOUND
			} else if (!strcmp(name, "snd_attack_up1")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_1_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_up2")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_2_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_up3")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_3_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_up4")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_4_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_up5")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_5_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_up6")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_6_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_up7")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_7_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_up8")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_8_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_up9")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_9_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_up10")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_10_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_down1")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_1_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_down2")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_2_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_down3")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_3_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_down4")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_4_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_down5")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_5_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_down6")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_6_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_down7")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_7_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_down8")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_8_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_down9")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_9_frame], str, get_string_property(item, "sound_scale"));
			} else if (!strcmp(name, "snd_attack_down10")) {
				get_string_value (str,sizeof(str),item);
				cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_10_frame], str, get_string_property(item, "sound_scale"));
#endif	//NEW_SOUND
			} else {
				index = -1;
				if (!strcmp(name, "cal_attack_up1")) {
					index = cal_weapon_attack_up_1_frame;
				} else if (!strcmp(name, "cal_attack_up2")) {
					index = cal_weapon_attack_up_2_frame;
				} else if (!strcmp(name, "cal_attack_up3")) {
					index = cal_weapon_attack_up_3_frame;
				} else if (!strcmp(name, "cal_attack_up4")) {
					index = cal_weapon_attack_up_4_frame;
				} else if (!strcmp(name, "cal_attack_up5")) {
					index = cal_weapon_attack_up_5_frame;
				} else if (!strcmp(name, "cal_attack_up6")) {
					index = cal_weapon_attack_up_6_frame;
				} else if (!strcmp(name, "cal_attack_up7")) {
					index = cal_weapon_attack_up_7_frame;
				} else if (!strcmp(name, "cal_attack_up8")) {
					index = cal_weapon_attack_up_8_frame;
				} else if (!strcmp(name, "cal_attack_up9")) {
					index = cal_weapon_attack_up_9_frame;
				} else if (!strcmp(name, "cal_attack_up10")) {
					index = cal_weapon_attack_up_10_frame;
				} else if (!strcmp(name, "cal_attack_down1")) {
					index = cal_weapon_attack_down_1_frame;
				} else if (!strcmp(name, "cal_attack_down2")) {
					index = cal_weapon_attack_down_2_frame;
				} else if (!strcmp(name, "cal_attack_down3")) {
					index = cal_weapon_attack_down_3_frame;
				} else if (!strcmp(name, "cal_attack_down4")) {
					index = cal_weapon_attack_down_4_frame;
				} else if (!strcmp(name, "cal_attack_down5")) {
					index = cal_weapon_attack_down_5_frame;
				} else if (!strcmp(name, "cal_attack_down6")) {
					index = cal_weapon_attack_down_6_frame;
				} else if (!strcmp(name, "cal_attack_down7")) {
					index = cal_weapon_attack_down_7_frame;
				} else if (!strcmp(name, "cal_attack_down8")) {
					index = cal_weapon_attack_down_8_frame;
				} else if (!strcmp(name, "cal_attack_down9")) {
					index = cal_weapon_attack_down_9_frame;
				} else if (!strcmp(name, "cal_attack_down10")) {
					index = cal_weapon_attack_down_10_frame;
				}else if (!strcmp(name, "cal_attack_up1_held")) {
					index = cal_weapon_attack_up_1_held_frame;
				} else if (!strcmp(name, "cal_attack_up2_held")) {
					index = cal_weapon_attack_up_2_held_frame;
				} else if (!strcmp(name, "cal_attack_up3_held")) {
					index = cal_weapon_attack_up_3_held_frame;
				} else if (!strcmp(name, "cal_attack_up4_held")) {
					index = cal_weapon_attack_up_4_held_frame;
				} else if (!strcmp(name, "cal_attack_up5_held")) {
					index = cal_weapon_attack_up_5_held_frame;
				} else if (!strcmp(name, "cal_attack_up6_held")) {
					index = cal_weapon_attack_up_6_held_frame;
				} else if (!strcmp(name, "cal_attack_up7_held")) {
					index = cal_weapon_attack_up_7_held_frame;
				} else if (!strcmp(name, "cal_attack_up8_held")) {
					index = cal_weapon_attack_up_8_held_frame;
				} else if (!strcmp(name, "cal_attack_up9_held")) {
					index = cal_weapon_attack_up_9_held_frame;
				} else if (!strcmp(name, "cal_attack_up10_held")) {
					index = cal_weapon_attack_up_10_held_frame;
				} else if (!strcmp(name, "cal_attack_down1_held")) {
					index = cal_weapon_attack_down_1_held_frame;
				} else if (!strcmp(name, "cal_attack_down2_held")) {
					index = cal_weapon_attack_down_2_held_frame;
				} else if (!strcmp(name, "cal_attack_down3_held")) {
					index = cal_weapon_attack_down_3_held_frame;
				} else if (!strcmp(name, "cal_attack_down4_held")) {
					index = cal_weapon_attack_down_4_held_frame;
				} else if (!strcmp(name, "cal_attack_down5_held")) {
					index = cal_weapon_attack_down_5_held_frame;
				} else if (!strcmp(name, "cal_attack_down6_held")) {
					index = cal_weapon_attack_down_6_held_frame;
				} else if (!strcmp(name, "cal_attack_down7_held")) {
					index = cal_weapon_attack_down_7_held_frame;
				} else if (!strcmp(name, "cal_attack_down8_held")) {
					index = cal_weapon_attack_down_8_held_frame;
				} else if (!strcmp(name, "cal_attack_down9_held")) {
					index = cal_weapon_attack_down_9_held_frame;
				} else if (!strcmp(name, "cal_attack_down10_held")) {
					index = cal_weapon_attack_down_10_held_frame;
				} else if (!strcmp(name, "cal_range_fire")) {
					index = cal_weapon_range_fire_frame;
				} else if (!strcmp(name, "cal_range_fire_out")) {
					index = cal_weapon_range_fire_out_frame;
				} else if (!strcmp(name, "cal_range_idle")) {
					index = cal_weapon_range_idle_frame;
				} else if (!strcmp(name, "cal_range_in")) {
					index = cal_weapon_range_in_frame;
				} else if (!strcmp(name, "cal_range_out")) {
					index = cal_weapon_range_out_frame;
				} else if (!strcmp(name, "cal_range_fire_held")) {
					index = cal_weapon_range_fire_held_frame;
				} else if (!strcmp(name, "cal_range_fire_out_held")) {
					index = cal_weapon_range_fire_out_held_frame;
				} else if (!strcmp(name, "cal_range_idle_held")) {
					index = cal_weapon_range_idle_held_frame;
				} else if (!strcmp(name, "cal_range_in_held")) {
					index = cal_weapon_range_in_held_frame;
				} else if (!strcmp(name, "cal_range_out_held")) {
					index = cal_weapon_range_out_held_frame;
				}
				if (index > -1)
				{
					get_string_value (str,sizeof(str),item);
					weapon->cal_frames[index] = cal_load_anim(act, str
#ifdef NEW_SOUND
						, get_string_property(item, "sound")
						, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
						, get_int_property(item, "duration")
						);
				}
#ifndef NEW_SOUND
				else if (!strcmp(name, "snd_attack_up1") || !strcmp(name, "snd_attack_up2") ||
					!strcmp(name, "snd_attack_up3") || !strcmp(name, "snd_attack_up4") ||
					!strcmp(name, "snd_attack_up5") || !strcmp(name, "snd_attack_up6") ||
					!strcmp(name, "snd_attack_up7") || !strcmp(name, "snd_attack_up8") ||
					!strcmp(name, "snd_attack_up9") || !strcmp(name, "snd_attack_up10") ||
					!strcmp(name, "snd_attack_down1") || !strcmp(name, "snd_attack_down2") ||
					!strcmp(name, "snd_attack_down3") || !strcmp(name, "snd_attack_down4") ||
					!strcmp(name, "snd_attack_down5") || !strcmp(name, "snd_attack_down6") ||
					!strcmp(name, "snd_attack_down7") || !strcmp(name, "snd_attack_down8") ||
					!strcmp(name, "snd_attack_down9") || !strcmp(name, "snd_attack_down10"))
				{ /* ignore */ }
#endif
				else
				{
					LOG_ERROR("unknown weapon property \"%s\"", item->name);
					ok = 0;
				}
			}
		}
		else if (item->type == XML_ENTITY_REF_NODE)
		{
			ok &= parse_actor_weapon_detail(act, weapon, item->children, defaults);
		}
	}


	return ok;
}

int parse_actor_weapon(actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	int ok, type_idx;
	weapon_part *weapon;

	if (cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");
/*	if(type_idx < 0){
		type_idx= get_property(cfg, "type", "weapon type", weapon_type_dict);
	}
*/
	if(type_idx < 0 || type_idx >= actor_part_sizes[ACTOR_WEAPON_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->weapon == NULL) {
		int i, j;
		act->weapon = (weapon_part*)calloc(actor_part_sizes[ACTOR_WEAPON_SIZE], sizeof(weapon_part));
		for (i = actor_part_sizes[ACTOR_WEAPON_SIZE]; i--;) {
			act->weapon[i].mesh_index = -1;
			for (j = 0; j < NUM_WEAPON_FRAMES; j++) {
				act->weapon[i].cal_frames[j].anim_index = -1;
#ifdef NEW_SOUND
				act->weapon[i].cal_frames[j].sound = -1;
#endif // NEW_SOUND
			}
		}
	}

	weapon = &(act->weapon[type_idx]);
	ok= parse_actor_weapon_detail(act, weapon, cfg, defaults);
	weapon->turn_horse=get_int_property(cfg, "turn_horse");
	weapon->unarmed=(get_int_property(cfg, "unarmed")<=0) ? (0):(1);

	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		const xmlNode *default_node= get_default_node(cfg, defaults);

		if(default_node){
			if(*weapon->skin_name=='\0')
				get_item_string_value(weapon->skin_name, sizeof(weapon->skin_name), default_node, (xmlChar*)"skin");
			if(type_idx!=GLOVE_FUR && type_idx!=GLOVE_LEATHER){ // these dont have meshes
				if(*weapon->model_name=='\0'){
					get_item_string_value(weapon->model_name, sizeof(weapon->model_name), default_node, (xmlChar*)"mesh");
					weapon->mesh_index= cal_load_weapon_mesh(act, weapon->model_name, "weapon");
				}
			}
			// TODO: combat animations
		}
	}

	// check the critical information
	if(type_idx!=WEAPON_NONE){   // no weapon doesn't have a skin/model
		actor_check_string(act, "weapon", "skin", weapon->skin_name);
		if(type_idx!=GLOVE_FUR && type_idx!=GLOVE_LEATHER){ // these dont have meshes
			actor_check_string(act, "weapon", "model", weapon->model_name);
			actor_check_int(act, "weapon.mesh", weapon->model_name, weapon->mesh_index);
		}
		// TODO: check combat animations
	}


	return ok;
}

int parse_actor_body_part (actor_types *act, body_part *part, const xmlNode *cfg, const char *part_name, const xmlNode *default_node)
{
	const xmlNode *item;
	int ok = 1;

	if(cfg == NULL) return 0;

	for(item=cfg; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, (xmlChar*)"mesh") == 0) {
				get_string_value (part->model_name, sizeof (part->model_name), item);
				if(strcmp("shield",part_name)==0)
					part->mesh_index = cal_load_weapon_mesh (act, part->model_name, part_name);
				else
					part->mesh_index = cal_load_mesh (act, part->model_name, part_name);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skin") == 0) {
				get_string_value (part->skin_name, sizeof (part->skin_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skinmask") == 0) {
				get_string_value (part->skin_mask, sizeof (part->skin_mask), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"glow") == 0) {
				int mode = find_description_index (glow_mode_dict, (char*)item->children->content, "glow mode");
				if(mode < 0) mode = GLOW_NONE;
				part->glow= mode;
			} else {
				LOG_ERROR("unknown %s property \"%s\"", part_name, item->name);
				ok = 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(default_node){
		if(*part->skin_name=='\0')
			if(strcmp(part_name, "head")){ // heads don't have separate skins here
				get_item_string_value(part->skin_name, sizeof(part->skin_name), default_node, (xmlChar*)"skin");
			}
		if(*part->model_name=='\0'){
			get_item_string_value(part->model_name, sizeof(part->model_name), default_node, (xmlChar*)"mesh");
			if(strcmp("shield",part_name)==0)
				part->mesh_index= cal_load_weapon_mesh(act, part->model_name, part_name);
			else
				part->mesh_index= cal_load_mesh(act, part->model_name, part_name);
		}
	}

	// check the critical information
	if(strcmp(part_name, "head")){ // heads don't have separate skins here
		actor_check_string(act, part_name, "skin", part->skin_name);
	}
	actor_check_string(act, part_name, "model", part->model_name);
	actor_check_int(act, part_name, "mesh", part->mesh_index);


	return ok;
}

int parse_actor_helmet (actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *default_node= get_default_node(cfg, defaults);
	int type_idx;
	body_part *helmet;

	if(cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");
/*	if(type_idx < 0){
		type_idx= get_property(cfg, "type", "helmet type", helmet_type_dict);
	}
*/
	if(type_idx < 0 || type_idx >= actor_part_sizes[ACTOR_HELMET_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->helmet == NULL) {
		int i;
		act->helmet = (body_part*)calloc(actor_part_sizes[ACTOR_HELMET_SIZE], sizeof(body_part));
		for (i = actor_part_sizes[ACTOR_HELMET_SIZE]; i--;) act->helmet[i].mesh_index= -1;
	}

	helmet= &(act->helmet[type_idx]);


	return parse_actor_body_part(act,helmet, cfg->children, "helmet", default_node);
}

int parse_actor_neck (actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *default_node = get_default_node(cfg, defaults);
	int type_idx;
	body_part *neck;

	if(cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");

	if(type_idx < 0 || type_idx >= actor_part_sizes[ACTOR_NECK_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->neck == NULL) {
		int i;
		act->neck = (body_part*)calloc(actor_part_sizes[ACTOR_NECK_SIZE], sizeof(body_part));
		for (i = actor_part_sizes[ACTOR_NECK_SIZE]; i--;) act->neck[i].mesh_index= -1;
	}

	neck= &(act->neck[type_idx]);

	return parse_actor_body_part(act,neck, cfg->children, "neck", default_node);
}

#ifdef NEW_SOUND
int parse_actor_sounds(actor_types *act, const xmlNode *cfg)
{
	const xmlNode *item;
	char str[255];
	int ok;
	int i;

	if (cfg == NULL) return 0;
	if (!have_sound_config) return 1;  // not a fatal error for loading actor_defs.

	ok = 1;
	for (item = cfg; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			get_string_value (str,sizeof(str),item);
			if (xmlStrcasecmp (item->name, (xmlChar*)"walk") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_walk_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"run") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_run_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"die1") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_die1_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"die2") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_die2_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"pain1") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_pain1_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"pain2") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_pain2_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"pick") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_pick_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"drop") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_drop_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"harvest") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_harvest_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"attack_cast") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_attack_cast_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"attack_ranged") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_attack_ranged_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"sit_down") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_sit_down_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"stand_up") == 0) {
				cal_set_anim_sound(&act->cal_frames[cal_actor_stand_up_frame], str, get_string_property(item, "sound_scale"));
			// These sounds are only found in the <sounds> block as they aren't tied to an animation
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"battlecry") == 0) {
				i = get_index_for_sound_type_name(str);
				if (i == -1)
					LOG_ERROR("Unknown battlecry sound (%s) in actor def: %s", str, act->actor_name);
				else
				{
					act->battlecry.sound = i;
					safe_strncpy(str, get_string_property(item, "sound_scale"), sizeof(str));
					if (strcasecmp(str, ""))
						act->battlecry.scale = atof(str);
					else
						act->battlecry.scale = 1.0f;
				}
			} else {
				LOG_ERROR("Unknown sound \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	return ok;
}
#endif	//NEW_SOUND

int parse_actor_cape (actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *default_node = get_default_node(cfg, defaults);
	int type_idx;
	body_part *cape;

	if(cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");
/*	if(type_idx < 0){
		type_idx= get_property(cfg, "color", "cape color", cape_color_dict);
	}
*/
	if(type_idx < 0 || type_idx >= actor_part_sizes[ACTOR_CAPE_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->cape == NULL) {
		int i;
		act->cape = (body_part*)calloc(actor_part_sizes[ACTOR_CAPE_SIZE], sizeof(body_part));
		for (i = actor_part_sizes[ACTOR_CAPE_SIZE]; i--;) act->cape[i].mesh_index= -1;
	}

	cape= &(act->cape[type_idx]);


	return parse_actor_body_part(act,cape, cfg->children, "cape", default_node);
}

int parse_actor_head (actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *default_node= get_default_node(cfg, defaults);
	int type_idx;
	body_part *head;

	if(cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");
	if(type_idx < 0){
		type_idx= get_property(cfg, "number", "head number", head_number_dict);
	}

	if(type_idx < 0 || type_idx >= actor_part_sizes[ACTOR_HEAD_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->head == NULL) {
		int i;
		act->head = (body_part*)calloc(actor_part_sizes[ACTOR_HEAD_SIZE], sizeof(body_part));
		for (i = actor_part_sizes[ACTOR_HEAD_SIZE]; i--;) act->head[i].mesh_index= -1;
	}

	head= &(act->head[type_idx]);



	return parse_actor_body_part(act, head, cfg->children, "head", default_node);
}

int parse_actor_shield_part (actor_types *act, shield_part *part, const xmlNode *cfg, const xmlNode *default_node)
{
	const xmlNode *item;
	int ok = 1;

	if(cfg == NULL) return 0;

	for(item=cfg; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, (xmlChar*)"mesh") == 0) {
				get_string_value (part->model_name, sizeof (part->model_name), item);
				part->mesh_index = cal_load_weapon_mesh (act, part->model_name, "shield");
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skin") == 0) {
				get_string_value (part->skin_name, sizeof (part->skin_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skinmask") == 0) {
				get_string_value (part->skin_mask, sizeof (part->skin_mask), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"glow") == 0) {
				int mode = find_description_index (glow_mode_dict, (char*)item->children->content, "glow mode");
				if(mode < 0) mode = GLOW_NONE;
				part->glow= mode;
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"missile") == 0) {
				part->missile_type = get_int_value(item);
			} else {
				LOG_ERROR("unknown shield property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(default_node){
		if(*part->model_name=='\0'){
			get_item_string_value(part->model_name, sizeof(part->model_name), default_node, (xmlChar*)"mesh");
			part->mesh_index= cal_load_weapon_mesh(act, part->model_name, "shield");
		}
	}

	// check the critical information
	actor_check_string(act, "shield", "model", part->model_name);
	actor_check_int(act, "shield", "mesh", part->mesh_index);


	return ok;
}

int parse_actor_shield (actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *default_node = get_default_node(cfg, defaults);
	int type_idx;
	shield_part *shield;

	if(cfg == NULL || cfg->children == NULL) return 0;

	type_idx= get_int_property(cfg, "id");
/*	if(type_idx < 0){
		type_idx= get_property(cfg, "type", "shield type", shield_type_dict);
	}
*/
	if(type_idx < 0 || type_idx >= actor_part_sizes[ACTOR_SHIELD_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->shield == NULL) {
		int i;
		act->shield = (shield_part*)calloc(actor_part_sizes[ACTOR_SHIELD_SIZE], sizeof(shield_part));
		for (i = actor_part_sizes[ACTOR_SHIELD_SIZE]; i--;) {
			act->shield[i].mesh_index = -1;
			act->shield[i].missile_type = -1;
		}
	}

	shield= &(act->shield[type_idx]);


	return parse_actor_shield_part(act, shield, cfg->children, default_node);
}

int parse_actor_hair (actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	int col_idx;
	size_t len;
	char *buf;

	if(cfg == NULL || cfg->children == NULL) return 0;

	col_idx= get_int_property(cfg, "id");
/*	if(col_idx < 0){
		col_idx= get_property(cfg, "color", "hair color", hair_color_dict);
	}
*/
	if(col_idx < 0 || col_idx >= actor_part_sizes[ACTOR_HAIR_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->hair == NULL) {
		int i;
		act->hair = (hair_part*)calloc(actor_part_sizes[ACTOR_HAIR_SIZE], sizeof(hair_part));
		for (i = actor_part_sizes[ACTOR_HAIR_SIZE]; i--;) act->hair[i].mesh_index= -1;
	}

	buf= act->hair[col_idx].hair_name;
	len= sizeof (act->hair[col_idx].hair_name);
	get_string_value(buf, len, cfg);


	return 1;
}

int parse_actor_eyes (actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	int col_idx;
	size_t len;
	char *buf;

	if(cfg == NULL || cfg->children == NULL) return 0;

	col_idx= get_int_property(cfg, "id");
/*	if(col_idx < 0){
		col_idx= get_property(cfg, "color", "eyes color", eyes_color_dict);
	}
*/
	if(col_idx < 0 || col_idx >= actor_part_sizes[ACTOR_EYES_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->eyes == NULL) {
		int i;
		act->eyes = (eyes_part*)calloc(actor_part_sizes[ACTOR_EYES_SIZE], sizeof(eyes_part));
		for (i = actor_part_sizes[ACTOR_EYES_SIZE]; i--;) act->eyes[i].mesh_index= -1;
	}

	buf= act->eyes[col_idx].eyes_name;
	len= sizeof (act->eyes[col_idx].eyes_name);
	get_string_value(buf, len, cfg);


	return 1;
}

int cal_get_idle_group(actor_types *act,char *name)
{
	int i;
	int res=-1;

	for (i=0;i<act->group_count;++i) {
		if (strcmp(name,act->idle_group[i].name)==0) res=i;
	}

	if (res>=0) return res;//Found it, return

	//Create a new named group
	res=act->group_count;
	safe_strncpy(act->idle_group[res].name, name, sizeof(act->idle_group[res].name));
	++act->group_count;

	return res;
}

struct cal_anim cal_load_idle(actor_types *act, char *str)
{
	struct cal_anim res={-1,0,0,0.0f
#ifdef NEW_SOUND
	,-1
	,0.0f
#endif  //NEW_SOUND
	};
	struct CalCoreAnimation *coreanim;

	res.anim_index=CalCoreModel_ELLoadCoreAnimation(act->coremodel,str,act->scale);
	if(res.anim_index == -1) {
		LOG_ERROR("Cal3d error: %s: %s\n", str, CalError_GetLastErrorDescription());
		return res;
	}
	coreanim=CalCoreModel_GetCoreAnimation(act->coremodel,res.anim_index);

	if (coreanim) {
		res.duration=CalCoreAnimation_GetDuration(coreanim);
	} else {
		LOG_ERROR("No Anim: %s\n",str);
	}

	return res;
}

void cal_group_addanim(actor_types *act,int gindex, char *fanim)
{
	int i;

	i=act->idle_group[gindex].count;
	act->idle_group[gindex].anim[i]=cal_load_idle(act,fanim);
	//LOG_TO_CONSOLE(c_green2,fanim);
	++act->idle_group[gindex].count;
}

void parse_idle_group (actor_types *act, const char *str)
{
	char gname[255]={0};
	char fname[255]={0};
	//char temp[255];
	int gindex;

	if (sscanf (str, "%254s %254s", gname, fname) != 2) return;

	gindex=cal_get_idle_group(act,gname);
	cal_group_addanim(act,gindex,fname);
	//safe_snprintf(temp, sizeof(temp), "%d",gindex);
	//LOG_TO_CONSOLE(c_green2,gname);
	//LOG_TO_CONSOLE(c_green2,fname);
	//LOG_TO_CONSOLE(c_green2,temp);
}

int parse_actor_frames (actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *item;
	char str[255];
	int ok = 1;

	if (cfg == NULL) return 0;

	for (item = cfg; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			int index = -1;
			if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_IDLE_GROUP") == 0) {
				get_string_value (str,sizeof(str),item);
     				//act->cal_walk_frame=cal_load_anim(act,str);
				//LOG_TO_CONSOLE(c_green2,str);
				parse_idle_group(act,str);
				//Not functional!
				index = -2;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_walk") == 0) {
				index = cal_actor_walk_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_run") == 0) {
				index = cal_actor_run_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_turn_left") == 0) {
				index = cal_actor_turn_left_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_turn_right") == 0) {
				index = cal_actor_turn_right_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_die1") == 0) {
				index = cal_actor_die1_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_die2") == 0) {
				index = cal_actor_die2_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_pain1") == 0) {
				index = cal_actor_pain1_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_pain2") == 0) {
				index = cal_actor_pain2_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_pick") == 0) {
				index = cal_actor_pick_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_drop") == 0) {
				index = cal_actor_drop_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_idle") == 0) {
				index = cal_actor_idle1_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_idle2") == 0) {
				index = cal_actor_idle2_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_idle_sit") == 0) {
				index = cal_actor_idle_sit_frame;
 			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_harvest") == 0) {
				index = cal_actor_harvest_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_cast") == 0) {
				index = cal_actor_attack_cast_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_sit_down") == 0) {
				index = cal_actor_sit_down_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_stand_up") == 0) {
				index = cal_actor_stand_up_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_in_combat") == 0) {
				index = cal_actor_in_combat_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_out_combat") == 0) {
				index = cal_actor_out_combat_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_combat_idle") == 0) {
				index = cal_actor_combat_idle_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_1") == 0) {
				index = cal_actor_attack_up_1_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_2") == 0) {
				index = cal_actor_attack_up_2_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_3") == 0) {
				index = cal_actor_attack_up_3_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_4") == 0) {
				index = cal_actor_attack_up_4_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_5") == 0) {
				index = cal_actor_attack_up_5_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_6") == 0) {
				index = cal_actor_attack_up_6_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_7") == 0) {
				index = cal_actor_attack_up_7_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_8") == 0) {
				index = cal_actor_attack_up_8_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_9") == 0) {
				index = cal_actor_attack_up_9_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_10") == 0) {
				index = cal_actor_attack_up_10_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_1") == 0) {
				index = cal_actor_attack_down_1_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_2") == 0) {
				index = cal_actor_attack_down_2_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_3") == 0) {
				index = cal_actor_attack_down_3_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_4") == 0) {
				index = cal_actor_attack_down_4_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_5") == 0) {
				index = cal_actor_attack_down_5_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_6") == 0) {
				index = cal_actor_attack_down_6_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_7") == 0) {
				index = cal_actor_attack_down_7_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_8") == 0) {
				index = cal_actor_attack_down_8_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_9") == 0) {
				index = cal_actor_attack_down_9_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_10") == 0) {
				index = cal_actor_attack_down_10_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_in_combat_held") == 0) {
				index = cal_actor_in_combat_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_out_combat_held") == 0) {
				index = cal_actor_out_combat_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_combat_idle_held") == 0) {
				index = cal_actor_combat_idle_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_in_combat_held_unarmed") == 0) {
				index = cal_actor_in_combat_held_unarmed_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_out_combat_held_unarmed") == 0) {
				index = cal_actor_out_combat_held_unarmed_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_combat_idle_held_unarmed") == 0) {
				index = cal_actor_combat_idle_held_unarmed_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_1_held") == 0) {
				index = cal_actor_attack_up_1_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_2_held") == 0) {
				index = cal_actor_attack_up_2_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_3_held") == 0) {
				index = cal_actor_attack_up_3_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_4_held") == 0) {
				index = cal_actor_attack_up_4_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_5_held") == 0) {
				index = cal_actor_attack_up_5_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_6_held") == 0) {
				index = cal_actor_attack_up_6_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_7_held") == 0) {
				index = cal_actor_attack_up_7_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_8_held") == 0) {
				index = cal_actor_attack_up_8_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_9_held") == 0) {
				index = cal_actor_attack_up_9_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up_10_held") == 0) {
				index = cal_actor_attack_up_10_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_1_held") == 0) {
				index = cal_actor_attack_down_1_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_2_held") == 0) {
				index = cal_actor_attack_down_2_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_3_held") == 0) {
				index = cal_actor_attack_down_3_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_4_held") == 0) {
				index = cal_actor_attack_down_4_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_5_held") == 0) {
				index = cal_actor_attack_down_5_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_6_held") == 0) {
				index = cal_actor_attack_down_6_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_7_held") == 0) {
				index = cal_actor_attack_down_7_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_8_held") == 0) {
				index = cal_actor_attack_down_8_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_9_held") == 0) {
				index = cal_actor_attack_down_9_held_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down_10_held") == 0) {
				index = cal_actor_attack_down_10_held_frame;
			} else {
				int j;
				char *etag="CAL_emote";
				struct cal_anim *anim;
				if(!strncasecmp(etag,(const char*) item->name,9)) {
					//load emote frame
					j=get_int_property(item,"index");	
					get_string_value(str, sizeof(str), item);
					if(!act->emote_frames)
						act->emote_frames=create_hash_table(EMOTES_FRAMES,hash_fn_int,cmp_fn_int,free);
					anim=calloc(1,sizeof(struct cal_anim));
		     			*anim = cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
					, get_int_property(item, "duration")
					);
					hash_add(act->emote_frames, (void *)(uintptr_t)j, (void*)anim);
					continue;
				}
			}

			if (index >= 0)
			{
				get_string_value(str, sizeof(str), item);
     			act->cal_frames[index] = cal_load_anim(act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
					, get_int_property(item, "duration")
					);
			}
			else if (index != -2)
			{
				LOG_ERROR("unknown frame property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	return ok;
}

int parse_actor_attachment (actor_types *act, const xmlNode *cfg, int actor_type)
{
	const xmlNode *item;
	int ok = 1;
	attached_actors_types *att = &attached_actors_defs[act->actor_type];
	actor_types *held_act = NULL;
	char str[256];
	struct CalCoreSkeleton *skel;

	if (cfg == NULL || cfg->children == NULL) return 0;

	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"holder") == 0) {
				att->actor_type[actor_type].is_holder = get_bool_value(item);
				if (att->actor_type[actor_type].is_holder)
					held_act = &actors_defs[actor_type];
				else
					held_act = act;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"parent_bone") == 0) {
				get_string_value (str, sizeof (str), item);
				skel = CalCoreModel_GetCoreSkeleton(actors_defs[actor_type].coremodel);
				if (skel) {
					att->actor_type[actor_type].parent_bone_id = find_core_bone_id(skel, str);
					if (att->actor_type[actor_type].parent_bone_id < 0) {
						LOG_ERROR("bone %s was not found in skeleton of actor type %d", str, actor_type);
						ok = 0;
					}
				}
				else {
					LOG_ERROR("the skeleton for actor type %d doesn't exist!", actor_type);
					ok = 0;
				}
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"local_bone") == 0) {
				get_string_value (str, sizeof (str), item);
				skel = CalCoreModel_GetCoreSkeleton(act->coremodel);
				if (skel) {
					att->actor_type[actor_type].local_bone_id = find_core_bone_id(skel, str);
					if (att->actor_type[actor_type].local_bone_id < 0) {
						LOG_ERROR("bone %s was not found in skeleton of actor type %d", str, act->actor_type);
						ok = 0;
					}
				}
				else {
					LOG_ERROR("the skeleton for actor type %d doesn't exist!", act->actor_type);
					ok = 0;
				}
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"held_shift") == 0) {
				xmlAttr *attr;
				for (attr = item->properties; attr; attr = attr->next)
					if (attr->type == XML_ATTRIBUTE_NODE)
					{
						if (xmlStrcasecmp (attr->name, (xmlChar*)"x") == 0)
							att->actor_type[actor_type].shift[0] = atof((char*)attr->children->content);
						else if (xmlStrcasecmp (attr->name, (xmlChar*)"y") == 0)
							att->actor_type[actor_type].shift[1] = atof((char*)attr->children->content);
						else if (xmlStrcasecmp (attr->name, (xmlChar*)"z") == 0)
							att->actor_type[actor_type].shift[2] = atof((char*)attr->children->content);
						else {
							LOG_ERROR("unknown attachment shift attribute \"%s\"", attr->name);
							ok = 0;
						}
					}
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_held_walk") == 0) {
				get_string_value (str, sizeof(str), item);
     			att->actor_type[actor_type].cal_frames[cal_attached_walk_frame] = cal_load_anim(held_act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
					, get_int_property(item, "duration")
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_held_run") == 0) {
				get_string_value (str, sizeof(str), item);
     			att->actor_type[actor_type].cal_frames[cal_attached_run_frame] = cal_load_anim(held_act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
					, get_int_property(item, "duration")
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_held_idle") == 0) {
				get_string_value (str,sizeof(str),item);
     			att->actor_type[actor_type].cal_frames[cal_attached_idle_frame] = cal_load_anim(held_act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
					, get_int_property(item, "duration")
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_held_pain") == 0) {
				get_string_value (str,sizeof(str),item);
     			att->actor_type[actor_type].cal_frames[cal_attached_pain_frame] = cal_load_anim(held_act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
					, get_int_property(item, "duration")
					);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_held_armed_pain") == 0) {
				get_string_value (str,sizeof(str),item);
     			att->actor_type[actor_type].cal_frames[cal_attached_pain_armed_frame] = cal_load_anim(held_act, str
#ifdef NEW_SOUND
					, get_string_property(item, "sound")
					, get_string_property(item, "sound_scale")
#endif	//NEW_SOUND
					, get_int_property(item, "duration")
					);
			} else {
				LOG_ERROR("unknown attachment property \"%s\"", item->name);
				ok = 0;
			}
		} else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= parse_actor_attachment(act, item->children, actor_type);
		}
	}

	return ok;
}

int parse_actor_boots (actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	const xmlNode *item;
	int ok, col_idx;
	boots_part *boots;

	if (cfg == NULL || cfg->children == NULL) return 0;

	col_idx= get_int_property(cfg, "id");
/*	if(col_idx < 0){
		col_idx = get_property (cfg, "color", "boots color", boots_color_dict);
	}
*/
	if(col_idx < 0 || col_idx >= actor_part_sizes[ACTOR_BOOTS_SIZE]){
		LOG_ERROR("Unable to find id/property node %s\n", cfg->name);
		return 0;
	}

	if (act->boots == NULL) {
		int i;
		act->boots = (boots_part*)calloc(actor_part_sizes[ACTOR_BOOTS_SIZE], sizeof(boots_part));
		for (i = actor_part_sizes[ACTOR_BOOTS_SIZE]; i--;) act->boots[i].mesh_index= -1;
	}

	boots = &(act->boots[col_idx]);
	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"skin") == 0) {
				get_string_value (boots->boots_name, sizeof (boots->boots_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"mesh") == 0) {
				get_string_value (boots->model_name, sizeof (boots->model_name), item);
				boots->mesh_index = cal_load_mesh (act, boots->model_name, "boots");
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"bootsmask") == 0) {
				get_string_value (boots->boots_mask, sizeof (boots->boots_mask), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"glow") == 0) {
				int mode = find_description_index (glow_mode_dict, (char*)item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				boots->glow = mode;
			} else {
				LOG_ERROR("unknown legs property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		const xmlNode *default_node = get_default_node(cfg, defaults);

		if(default_node){
			if (*boots->boots_name=='\0')
				get_item_string_value(boots->boots_name, sizeof(boots->boots_name), default_node, (xmlChar*)"skin");
			if (*boots->model_name=='\0')
			{
				get_item_string_value(boots->model_name, sizeof(boots->model_name), default_node, (xmlChar*)"mesh");
				boots->mesh_index= cal_load_mesh(act, boots->model_name, "boots");
			}
		}
	}

	// check the critical information
	actor_check_string(act, "boots", "boots", boots->boots_name);
	actor_check_string(act, "boots", "model", boots->model_name);
	actor_check_int(act, "boots", "mesh", boots->mesh_index);

	return ok;
}

//Searches if a mesh is already loaded- TODO:MAKE THIS BETTER
int cal_search_mesh (actor_types *act, const char *fn, const char *kind)
{
	int i;

	if (kind == NULL)
	{
		return -1;
	}
	else if (act->head && strcmp (kind, "head") == 0)
	{
		for (i = 0; i < actor_part_sizes[ACTOR_HEAD_SIZE]; i++)
			if (strcmp (fn, act->head[i].model_name) == 0 && act->head[i].mesh_index != -1)
				return act->head[i].mesh_index;
	}
	else if (act->shirt && strcmp (kind, "shirt") == 0)
	{
		for (i = 0; i < actor_part_sizes[ACTOR_SHIRT_SIZE]; i++)
		{
			if (strcmp (fn, act->shirt[i].model_name) == 0 && act->shirt[i].mesh_index != -1)
				return act->shirt[i].mesh_index;
		}
	}
	else if (act->legs && strcmp (kind, "legs") == 0)
	{
		for (i = 0; i < actor_part_sizes[ACTOR_LEGS_SIZE]; i++)
		{
			if (strcmp (fn, act->legs[i].model_name) == 0 && act->legs[i].mesh_index != -1)
				return act->legs[i].mesh_index;
		}
	}
	else if (act->boots && strcmp (kind, "boots") == 0)
	{
		for (i = 0; i < actor_part_sizes[ACTOR_BOOTS_SIZE]; i++)
		{
			if (strcmp (fn, act->boots[i].model_name) == 0 && act->boots[i].mesh_index != -1)
				return act->boots[i].mesh_index;
		}
	}
	else if (act->cape && strcmp (kind, "cape") == 0)
	{
		for (i = 0; i < actor_part_sizes[ACTOR_CAPE_SIZE]; i++)
		{
			if (strcmp (fn, act->cape[i].model_name) == 0 && act->cape[i].mesh_index != -1)
				return act->cape[i].mesh_index;
		}
	}
	else if (act->helmet && strcmp (kind, "helmet") == 0)
	{
		for (i = 0; i < actor_part_sizes[ACTOR_HELMET_SIZE]; i++)
		{
			if (strcmp (fn, act->helmet[i].model_name) == 0 && act->helmet[i].mesh_index != -1)
				return act->helmet[i].mesh_index;
		}
	}
	else if (act->neck && strcmp (kind, "neck") == 0)
	{
		for (i = 0; i < actor_part_sizes[ACTOR_NECK_SIZE]; i++)
		{
			if (strcmp (fn, act->neck[i].model_name) == 0 && act->neck[i].mesh_index != -1)
				return act->neck[i].mesh_index;
		}
	}
	else if (act->shield && strcmp (kind, "shield") == 0)
	{
		for (i = 0; i < actor_part_sizes[ACTOR_SHIELD_SIZE]; i++)
		{
			if (strcmp (fn, act->shield[i].model_name) == 0 && act->shield[i].mesh_index != -1)
				return act->shield[i].mesh_index;
		}
	}
	else if (act->weapon && strcmp (kind, "weapon") == 0)
	{
		for (i = 0; i < actor_part_sizes[ACTOR_WEAPON_SIZE]; i++)
		{
			if (strcmp (fn, act->weapon[i].model_name) == 0 && act->weapon[i].mesh_index != -1)
				return act->weapon[i].mesh_index;
		}
	}

	return -1;
}

//Loads a Cal3D mesh
int cal_load_mesh(actor_types *act, const char *fn, const char *kind)
{
	int res;
	struct CalCoreMesh *mesh;

	if (fn==0) return -1;
	if (strlen(fn)==0) return -1;
	if (act->coremodel==NULL) return -1;
	if (kind != NULL)
	{
		res = cal_search_mesh (act, fn, kind);
		if (res != -1) return res;
	}

	//Load coremesh
	res=CalCoreModel_ELLoadCoreMesh(act->coremodel,fn);

	//Scale coremesh
	if (res >= 0) {
		mesh=CalCoreModel_GetCoreMesh(act->coremodel,res);
		if ((mesh)&&(act->mesh_scale!=1.0)) CalCoreMesh_Scale(mesh,act->mesh_scale);
	} else {
		LOG_ERROR("Cal3d error: %s: %s\n", fn, CalError_GetLastErrorDescription());
	}

	return res;
}

int cal_load_weapon_mesh (actor_types *act, const char *fn, const char *kind)
{
	int res;
	struct CalCoreMesh *mesh;

	if (fn==0) return -1;
	if (strlen(fn)==0) return -1;
	if (act->coremodel==NULL) return -1;

	if (kind != NULL)
	{
		res = cal_search_mesh (act, fn, kind);
		if (res!=-1) return res;
	}

	//Load coremesh
	res=CalCoreModel_ELLoadCoreMesh(act->coremodel,fn);

	//Scale coremesh
	if (res>=0) {
		mesh=CalCoreModel_GetCoreMesh(act->coremodel,res);
		if ((mesh)&&(act->skel_scale!=1.0)) CalCoreMesh_Scale(mesh,act->skel_scale);
	} else {
		LOG_ERROR("Cal3d error: %s: %s\n", fn, CalError_GetLastErrorDescription());
	}

	return res;
}

int parse_actor_nodes(actor_types *act, const xmlNode *cfg, const xmlNode *defaults)
{
	char name[256];
	const xmlNode *item;
	int ok= 1;

	for (item=cfg->children; item; item=item->next)
	{
		if(item->type == XML_ELEMENT_NODE)
		{
			safe_strncpy(name, (const char*)item->name, sizeof(name));
			my_tolower(name);

			if (!strcmp(name, "ghost")) {
				act->ghost= get_bool_value(item);
			} else if (!strcmp(name, "skin")) {
				get_string_value(act->skin_name, sizeof (act->skin_name), item);
			} else if (!strcmp(name, "mesh")) {
				get_string_value(act->file_name, sizeof (act->file_name), item);
			} else if (!strcmp(name, "actor_scale")) {
				act->actor_scale= get_float_value(item);
			} else if (!strcmp(name, "scale")) {
				act->scale= get_float_value(item);
			} else if (!strcmp(name, "mesh_scale")) {
				act->mesh_scale= get_float_value(item);
			} else if (!strcmp(name, "bone_scale")) {
				act->skel_scale= get_float_value(item);
			} else if (!strcmp(name, "skeleton")) {
				char skeleton_name[MAX_FILE_PATH];
				get_string_value(skeleton_name, sizeof(skeleton_name), item);
				act->coremodel= CalCoreModel_New("Model");
				if(!CalCoreModel_ELLoadCoreSkeleton(act->coremodel, skeleton_name)) {
					LOG_ERROR("Cal3d error: %s: %s\n", skeleton_name, CalError_GetLastErrorDescription());
					act->skeleton_type = -1;
				}
				else {
					act->skeleton_type = get_skeleton(act->coremodel, skeleton_name);
				}
			} else if (!strcmp(name, "walk_speed")) { // unused
				act->walk_speed= get_float_value(item);
			} else if (!strcmp(name, "run_speed")) { // unused
				act->run_speed= get_float_value(item);
			} else if (!strcmp(name, "step_duration")) {
				act->step_duration = get_int_value(item);
			} else if (!strcmp(name, "defaults")) {
				defaults= item;
			} else if (!strcmp(name, "frames")) {
				ok &= parse_actor_frames(act, item->children, defaults);
			} else if (!strcmp(name, "shirt")) {
				ok &= parse_actor_shirt(act, item, defaults);
			} else if (!strcmp(name, "hskin")) {
				ok &= parse_actor_skin(act, item, defaults);
			} else if (!strcmp(name, "hair")) {
				ok &= parse_actor_hair(act, item, defaults);
			} else if (!strcmp(name, "eyes")) {
				ok &= parse_actor_eyes(act, item, defaults);
			} else if (!strcmp(name, "boots")) {
				ok &= parse_actor_boots(act, item, defaults);
			} else if (!strcmp(name, "legs")) {
				ok &= parse_actor_legs(act, item, defaults);
			} else if (!strcmp(name, "cape")) {
				ok &= parse_actor_cape(act, item, defaults);
			} else if (!strcmp(name, "head")) {
				ok &= parse_actor_head(act, item, defaults);
			} else if (!strcmp(name, "shield")) {
				ok &= parse_actor_shield(act, item, defaults);
			} else if (!strcmp(name, "weapon")) {
				ok &= parse_actor_weapon(act, item, defaults);
			} else if (!strcmp(name, "helmet")) {
				ok &= parse_actor_helmet(act, item, defaults);
			} else if (!strcmp(name, "neck")) {
				ok &= parse_actor_neck(act, item, defaults);
			} else if (!strcmp(name, "sounds")) {
#ifdef NEW_SOUND
				ok &= parse_actor_sounds(act, item->children);
#endif	//NEW_SOUND
			} else if (!strcmp(name, "actor_attachment")) {
				int id = get_int_property(item, "id");
				if (id < 0 || id >= MAX_ACTOR_DEFS) {
					LOG_ERROR("Unable to find id/property node %s\n", item->name);
					ok = 0;
				}
				else
					ok &= parse_actor_attachment(act, item, id);
			} else {
				LOG_ERROR("Unknown actor attribute \"%s\"", item->name);
				ok= 0;
			}
		} else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= parse_actor_nodes(act, item->children, defaults);
		}
	}
	return ok;
}

int parse_actor_script(const xmlNode *cfg)
{
	int ok, act_idx, i;
	int j;
	actor_types *act;
	struct CalCoreSkeleton *skel;

	if(cfg == NULL || cfg->children == NULL) return 0;

	act_idx= get_int_property(cfg, "id");
/*	if(act_idx < 0){
		act_idx= get_property(cfg, "type", "actor type", actor_type_dict);
	}
*/
	if (act_idx < 0 || act_idx >= MAX_ACTOR_DEFS){
		LOG_ERROR("Data Error in %s(%d): Actor ID out of range %d",
			get_string_property(cfg, "type"), act_idx, act_idx);
		return 0;
	}

	act= &(actors_defs[act_idx]);
	// watch for loading an actor more then once
	if(act->actor_type > 0 || *act->actor_name){
		LOG_ERROR("Data Error in %s(%d): Already loaded %s(%d)",
			get_string_property(cfg, "type"), act_idx, act->actor_name, act->actor_type);
	}
	act->actor_type= act_idx;	// memorize the ID & name to help in debugging
	safe_strncpy(act->actor_name, get_string_property(cfg, "type"), sizeof(act->actor_name));
	actor_check_string(act, "actor", "name", act->actor_name);

	//Initialize Cal3D settings
	act->coremodel= NULL;
	act->actor_scale= 1.0;
	act->scale= 1.0;
	act->mesh_scale= 1.0;
	act->skel_scale= 1.0;
	act->group_count= 0;
	for (i=0; i<16; ++i) {
		safe_strncpy(act->idle_group[i].name, "", sizeof(act->idle_group[i].name));
		act->idle_group[i].count= 0;
	}

	for (i = 0; i < NUM_ACTOR_FRAMES; i++) {
		act->cal_frames[i].anim_index= -1;
#ifdef NEW_SOUND
		act->cal_frames[i].sound= -1;
#endif // NEW_SOUND
	}
#ifdef NEW_SOUND
	act->battlecry.sound = -1;
#endif // NEW_SOUND

	for (i = 0; i < MAX_ACTOR_DEFS; ++i)
	{
		for (j = 0; j < NUM_ATTACHED_ACTOR_FRAMES; j++) {
			attached_actors_defs[act_idx].actor_type[i].cal_frames[j].anim_index = -1;
#ifdef NEW_SOUND
			attached_actors_defs[act_idx].actor_type[i].cal_frames[j].sound = -1;
#endif // NEW_SOUND
		}
	}

	act->step_duration = DEFAULT_STEP_DURATION; // default value

	ok= parse_actor_nodes(act, cfg, NULL);

	// TODO: add error checking for missing actor information

	//Actor def parsed, now setup the coremodel
	if (act->coremodel!=NULL)
	{
		skel=CalCoreModel_GetCoreSkeleton(act->coremodel);
		if(skel){
			CalCoreSkeleton_Scale(skel,act->skel_scale);
		}

		// If this not an enhanced actor, load the single mesh and exit
		if(!act->head || strcmp (act->head[0].model_name, "") == 0)
		{
			act->shirt = (shirt_part*)calloc(actor_part_sizes[ACTOR_SHIRT_SIZE], sizeof(shirt_part));
			act->shirt[0].mesh_index= cal_load_mesh(act, act->file_name, NULL); //save the single meshindex as torso
		}
		if (use_animation_program)
		{
			build_buffers(act);
		}
	}

	return ok;
}

int parse_actor_defs(const xmlNode *node)
{
	const xmlNode *def;
	int ok = 1;

	for (def = node->children; def; def = def->next)
	{
		if (def->type == XML_ELEMENT_NODE)
		{
			if (xmlStrcasecmp (def->name, (xmlChar*)"actor") == 0)
			{
				ok &= parse_actor_script (def);
			}
			else
			{
				LOG_ERROR("parse error: actor or include expected");
				ok = 0;
			}
		}
		else if (def->type == XML_ENTITY_REF_NODE)
		{
			ok &= parse_actor_defs (def->children);
		}
	}

	return ok;
}

#ifdef EXT_ACTOR_DICT
int parse_skin_colours(const xmlNode *node)
{
	const xmlNode *data;
	int ok = 1, i;

	num_skin_colors = 0;
	for (i = 0; i < MAX_SKIN_COLORS; i++) {
		skin_color_dict[i].index = 0;
		strcpy(skin_color_dict[i].desc, "");
	}
	for (data = node->children; data; data = data->next) {
		if (data->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp(data->name, (xmlChar*)"skin") == 0) {
				safe_strncpy(skin_color_dict[num_skin_colors].desc, get_string_property(data, "color"), sizeof(skin_color_dict[num_skin_colors].desc));
				skin_color_dict[num_skin_colors].index = get_int_value(data);
				num_skin_colors++;
			} else {
				LOG_ERROR("parse error: skin or include expected");
				ok = 0;
			}
		} else if (data->type == XML_ENTITY_REF_NODE) {
			ok &= parse_skin_colours(data->children);
		}
	}

	return ok;
}

int parse_glow_modes(const xmlNode *node)
{
	const xmlNode *data;
	int ok = 1, i;

	num_glow_modes = 0;
	for (i = 0; i < MAX_GLOW_MODES; i++) {
		glow_mode_dict[i].index = 0;
		strcpy(glow_mode_dict[i].desc, "");
	}
	for (data = node->children; data; data = data->next) {
		if (data->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp(data->name, (xmlChar*)"glow") == 0) {
				safe_strncpy(glow_mode_dict[num_glow_modes].desc, get_string_property(data, "mode"), sizeof(glow_mode_dict[num_glow_modes].desc));
				glow_mode_dict[num_glow_modes].index = get_int_value(data);
				num_glow_modes++;
			} else {
				LOG_ERROR("parse error: glow or include expected");
				ok = 0;
			}
		} else if (data->type == XML_ENTITY_REF_NODE) {
			ok &= parse_glow_modes(data->children);
		}
	}

	return ok;
}

int parse_head_numbers(const xmlNode *node)
{
	const xmlNode *data;
	int ok = 1, i;

	num_head_numbers = 0;
	for (i = 0; i < MAX_HEAD_NUMBERS; i++) {
		head_number_dict[i].index = 0;
		strcpy(head_number_dict[i].desc, "");
	}
	for (data = node->children; data; data = data->next) {
		if (data->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp(data->name, (xmlChar*)"head") == 0) {
				safe_strncpy(head_number_dict[num_head_numbers].desc, get_string_property(data, "number"), sizeof(head_number_dict[num_head_numbers].desc));
				head_number_dict[num_head_numbers].index = get_int_value(data);
				num_head_numbers++;
			} else {
				LOG_ERROR("parse error: head or include expected");
				ok = 0;
			}
		} else if (data->type == XML_ENTITY_REF_NODE) {
			ok &= parse_head_numbers(data->children);
		}
	}

	return ok;
}

int parse_actor_dict(const xmlNode *node)
{
	const xmlNode *data;
	int ok = 1;

	for (data = node->children; data; data = data->next) {
		if (data->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp(data->name, (xmlChar*)"skin_colors") == 0) {
				ok &= parse_skin_colours(data);
			} else if (xmlStrcasecmp(data->name, (xmlChar*)"glow_modes") == 0) {
				ok &= parse_glow_modes(data);
			} else if (xmlStrcasecmp(data->name, (xmlChar*)"head_numbers") == 0) {
				ok &= parse_head_numbers(data);
			} else {
				LOG_ERROR("parse error: skin_colors, glow_modes, head_numbers or include expected");
				ok = 0;
			}
		} else if (data->type == XML_ENTITY_REF_NODE) {
			ok &= parse_actor_dict(data->children);
		}
	}

	return ok;
}

int parse_actor_part_sizes(const xmlNode *node)
{
	const xmlNode *data;
	int ok = 1;
	char str[20];

	for (data = node->children; data; data = data->next) {
		if (data->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp(data->name, (xmlChar*)"max") == 0) {
				safe_strncpy(str, get_string_property(data, "part"), sizeof(str));
				if (!strcasecmp(str, "head")) {
					actor_part_sizes[ACTOR_HEAD_SIZE] = get_int_value(data);
				} else if (!strcasecmp(str, "shield")) {
					actor_part_sizes[ACTOR_SHIELD_SIZE] = get_int_value(data);
				} else if (!strcasecmp(str, "cape")) {
					actor_part_sizes[ACTOR_CAPE_SIZE] = get_int_value(data);
				} else if (!strcasecmp(str, "helmet")) {
					actor_part_sizes[ACTOR_HELMET_SIZE] = get_int_value(data);
				} else if (!strcasecmp(str, "neck")) {
					actor_part_sizes[ACTOR_NECK_SIZE] = get_int_value(data);
				} else if (!strcasecmp(str, "weapon")) {
					actor_part_sizes[ACTOR_WEAPON_SIZE] = get_int_value(data);
				} else if (!strcasecmp(str, "shirt")) {
					actor_part_sizes[ACTOR_SHIRT_SIZE] = get_int_value(data);
				} else if (!strcasecmp(str, "skin")) {
					actor_part_sizes[ACTOR_SKIN_SIZE] = get_int_value(data);
				} else if (!strcasecmp(str, "hair")) {
					actor_part_sizes[ACTOR_HAIR_SIZE] = get_int_value(data);
				} else if (!strcasecmp(str, "eyes")) {
					actor_part_sizes[ACTOR_EYES_SIZE] = get_int_value(data);
				} else if (!strcasecmp(str, "boots")) {
					actor_part_sizes[ACTOR_BOOTS_SIZE] = get_int_value(data);
				} else if (!strcasecmp(str, "legs")) {
					actor_part_sizes[ACTOR_LEGS_SIZE] = get_int_value(data);
				}
			} else {
				LOG_ERROR("parse error: max or include expected");
				ok = 0;
			}
		} else if (data->type == XML_ENTITY_REF_NODE) {
			ok &= parse_actor_part_sizes(data->children);
		}
	}

	return ok;
}

int parse_actor_data(const xmlNode *node)
{
	const xmlNode *data;
	int ok = 1;
	static int dict = 0, parts = 0;

	for (data = node->children; data; data = data->next) {
		if (data->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp(data->name, (xmlChar*)"actors") == 0) {
				// Check we have parsed both the actor dict and part sizes already!!
				if (parts && dict)
					ok &= parse_actor_defs(data);
				else
				{
					LOG_ERROR("parse error: actor_dict (%d) and actor_part_maximums (%d) *must* be parsed before actors", dict, parts);
					ok = 0;
				}
			} else if (xmlStrcasecmp(data->name, (xmlChar*)"actor_dict") == 0) {
				ok &= dict = parse_actor_dict(data);
			} else if (xmlStrcasecmp(data->name, (xmlChar*)"actor_part_maximums") == 0) {
				ok &= parts = parse_actor_part_sizes(data);
			} else {
				LOG_ERROR("parse error: actors, actor_dict, actor_part_maximums or include expected");
				ok = 0;
			}
		} else if (data->type == XML_ENTITY_REF_NODE) {
			ok &= parse_actor_data(data->children);
		}
	}

	return ok;
}
#endif // EXT_ACTOR_DICT

int read_actor_defs (const char *dir, const char *index)
{
	const xmlNode *root;
	xmlDoc *doc;
	char fname[120];
	int ok = 1;
	xmlParserCtxtPtr ctxt;

	safe_snprintf (fname, sizeof(fname), "%s/%s", dir, index);

	// Version v2.11 of libxml introduced "Protection against entity expansion attacks"
	// Loading the actor defs, trips this protection and so v2.11 will not load the defs.
	// v2.12 adds the ability to set a high limit for the checks.
	// See https://gitlab.gnome.org/GNOME/libxml2/-/issues/581
	// Thanks to @nwellnhof
#if (LIBXML_VERSION >= 21100) && (LIBXML_VERSION < 21200)
	#error Version 2.11 of libxml will not load actor defs.
#endif
	ctxt = xmlNewParserCtxt();
#if LIBXML_VERSION >= 21200
	xmlCtxtSetMaxAmplification (ctxt, 100);
#endif
	doc = xmlCtxtReadFile (ctxt, fname, NULL, XML_PARSE_NOENT);
	if (doc == NULL) {
		LOG_ERROR("Unable to read actor definition file %s", fname);
		xmlFreeParserCtxt(ctxt);
		return 0;
	}

	root = xmlDocGetRootElement (doc);
	if (root == NULL) {
		LOG_ERROR("Unable to parse actor definition file %s", fname);
		ok = 0;
#ifndef EXT_ACTOR_DICT
	} else if (xmlStrcasecmp (root->name, (xmlChar*)"actors") != 0) {
		LOG_ERROR("Unknown key \"%s\" (\"actors\" expected).", root->name);
#else // EXT_ACTOR_DICT
	} else if (xmlStrcasecmp (root->name, (xmlChar*)"actor_data") != 0) {
		LOG_ERROR("Unknown key \"%s\" (\"actor_data\" expected).", root->name);
#endif // EXT_ACTOR_DICT
		ok = 0;
	} else {
#ifndef EXT_ACTOR_DICT
		ok = parse_actor_defs (root);
#else // EXT_ACTOR_DICT
		ok = parse_actor_data (root);
#endif // EXT_ACTOR_DICT
	}

	xmlFreeDoc (doc);
	xmlFreeParserCtxt (ctxt);
	return ok;
}

void init_actor_defs()
{
	// initialize the whole thing to zero
	memset (actors_defs, 0, sizeof (actors_defs));
	memset (attached_actors_defs, 0, sizeof (attached_actors_defs));
	set_invert_v_coord();
	if (read_actor_defs ("actor_defs", "actor_defs.xml") != 1)
	{
		FATAL_ERROR_WINDOW("Failed to read actor defs. See logs, going to exit.");
		exit(1);
	}
}

void free_actor_defs()
{
	int i;
	for (i=0; i<MAX_ACTOR_DEFS; i++)
	{
		if (actors_defs[i].head)
			free(actors_defs[i].head);
		if (actors_defs[i].shield)
			free(actors_defs[i].shield);
		if (actors_defs[i].cape)
			free(actors_defs[i].cape);
		if (actors_defs[i].helmet)
			free(actors_defs[i].helmet);
		if (actors_defs[i].neck)
			free(actors_defs[i].neck);
		if (actors_defs[i].weapon)
			free(actors_defs[i].weapon);
		if (actors_defs[i].shirt)
			free(actors_defs[i].shirt);
		if (actors_defs[i].skin)
			free(actors_defs[i].skin);
		if (actors_defs[i].hair)
			free(actors_defs[i].hair);
		if (actors_defs[i].boots)
			free(actors_defs[i].boots);
		if (actors_defs[i].legs)
			free(actors_defs[i].legs);
		if (actors_defs[i].eyes)
			free(actors_defs[i].eyes);
		if (actors_defs[i].hardware_model)
			clear_buffers(&actors_defs[i]);
		CalCoreModel_Delete(actors_defs[i].coremodel);
	}
}

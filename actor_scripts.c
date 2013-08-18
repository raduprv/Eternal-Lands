#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "actor_scripts.h"
#include "actors.h"
#include "asc.h"
#include "cal.h"
#include "cal3d_wrapper.h"
#include "counters.h"
#include "cursors.h"
#include "draw_scene.h"
#include "errors.h"
#include "global.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "missiles.h"
#include "new_actors.h"
#include "multiplayer.h"
#include "new_character.h"
#include "particles.h"
#include "pathfinder.h"
#include "platform.h"
#include "skeletons.h"
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
#ifdef	NEW_TEXTURES
#include "textures.h"
#endif	/* NEW_TEXTURES */

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
int actor_part_sizes[ACTOR_NUM_PARTS] = {10, 40, 50, 100, 100, 100, 10, 20, 40, 60, 20};		// Elements according to actor_parts_enum
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
void unqueue_cmd(int i);
#ifdef NEW_SOUND
int parse_actor_sounds(actor_types *act, const xmlNode *cfg);
#endif	//NEW_SOUND

hash_table *emote_cmds = NULL;
hash_table *emotes = NULL;
int parse_actor_frames(actor_types *act, const xmlNode *cfg, const xmlNode *defaults);


#ifdef MORE_ATTACHED_ACTORS_DEBUG
static int thecount=0;
#endif


void unfreeze_horse(int i){

			if(HAS_HORSE(i)&&MY_HORSE(i)->que[0]==wait_cmd) {
				//printf("%i, horse out of wait\n",thecount);
				unqueue_cmd(MY_HORSE_ID(i));
				MY_HORSE(i)->busy=0;
				set_on_idle(MY_HORSE_ID(i));
			}
}


void cal_actor_set_random_idle(int id)
{
	struct CalMixer *mixer;
	int i;
	int random_anim;
	int random_anim_index;

	if (actors_list[id]->calmodel==NULL) return;
	//LOG_TO_CONSOLE(c_green2,"Randomizing");
	//if (actors_list[id]->cur_anim.anim_index==anim.anim_index) return;
	srand( (unsigned)time( NULL ) );
	mixer=CalModel_GetMixer(actors_list[id]->calmodel);
	//Stop previous animation if needed
	if (actors_list[id]->IsOnIdle!=1){
		if ((actors_list[id]->cur_anim.anim_index!=-1)&&(actors_list[id]->cur_anim.kind==0)) {
			CalMixer_ClearCycle(mixer,actors_list[id]->cur_anim.anim_index,0.05);
		}
		if ((actors_list[id]->cur_anim.anim_index!=-1)&&(actors_list[id]->cur_anim.kind==1)) {
			CalMixer_RemoveAction(mixer,actors_list[id]->cur_anim.anim_index);
		}
	}

	for (i=0;i<actors_defs[actors_list[id]->actor_type].group_count;++i) {
		random_anim=rand()%(actors_defs[actors_list[id]->actor_type].idle_group[i].count+1);
		if (random_anim<actors_defs[actors_list[id]->actor_type].idle_group[i].count) random_anim_index=actors_defs[actors_list[id]->actor_type].idle_group[i].anim[random_anim].anim_index;
		else random_anim_index=-1;
		if (actors_list[id]->IsOnIdle==1) {
			if (actors_list[id]->cur_idle_anims[i].anim_index!=random_anim_index)
			CalMixer_ClearCycle(mixer,actors_list[id]->cur_idle_anims[i].anim_index,2.0);
		}
		if (actors_list[id]->cur_idle_anims[i].anim_index!=random_anim_index)
			if (random_anim_index>=0) CalMixer_BlendCycle(mixer,random_anim_index,0.5,0.05);
		//safe_snprintf(str, sizeof(str),"%d",random_anim);
		//LOG_TO_CONSOLE(c_green2,str);
		actors_list[id]->cur_idle_anims[i].anim_index=random_anim_index;
		//anim.anim_index,1.0,0.05);else
	}

	//if (anim.kind==0) CalMixer_BlendCycle(mixer,anim.anim_index,1.0,0.05);else
	//CalMixer_ExecuteAction(mixer,anim.anim_index,0.0,0.0);
	//actors_list[id]->cur_anim=anim;
	//actors_list[id]->anim_time=0.0;
	CalModel_Update(actors_list[id]->calmodel,0.0001);//Make changes take effect now
	build_actor_bounding_box(actors_list[id]);
	if (use_animation_program)
	{
		set_transformation_buffers(actors_list[id]);
	}
	actors_list[id]->IsOnIdle= 1;
	actors_list[id]->cur_anim.duration= 0;
	actors_list[id]->anim_time= 0.0;
	actors_list[id]->last_anim_update= cur_time;
	actors_list[id]->cur_anim.anim_index= -1;
#ifdef NEW_SOUND
	if (check_sound_loops(actors_list[id]->cur_anim_sound_cookie))
		stop_sound(actors_list[id]->cur_anim_sound_cookie);
#endif // NEW_SOUND
	actors_list[id]->cur_anim_sound_cookie= 0;
	//if (actors_list[id]->cur_anim.anim_index==-1) actors_list[id]->busy=0;
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

void animate_actors()
{
#ifdef	ANIMATION_SCALING
	static int last_update = 0;
	int i, actors_time_diff, time_diff, tmp_time_diff;

	actors_time_diff = cur_time - last_update;
#else	/* ANIMATION_SCALING */
	int i;
	static int last_update= 0;
    int time_diff = cur_time-last_update;
    int tmp_time_diff;
#endif	/* ANIMATION_SCALING */

	// lock the actors_list so that nothing can interere with this look
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	for(i=0; i<max_actors; i++) {
		if(actors_list[i]) {
#ifdef	ANIMATION_SCALING
			time_diff = update_actor_animation_speed(actors_list[i], actors_time_diff);
#endif	/* ANIMATION_SCALING */
			if(actors_list[i]->moving) {
#ifdef	ANIMATION_SCALING
				tmp_time_diff = min2i(actors_list[i]->movement_time_left + 40, time_diff);

				actors_list[i]->x_pos += actors_list[i]->move_x_speed * tmp_time_diff;
				actors_list[i]->y_pos += actors_list[i]->move_y_speed * tmp_time_diff;
				actors_list[i]->z_pos += actors_list[i]->move_z_speed * tmp_time_diff;
#else	/* ANIMATION_SCALING */
				if (time_diff <= actors_list[i]->movement_time_left+40) {
					actors_list[i]->x_pos += actors_list[i]->move_x_speed*time_diff;
					actors_list[i]->y_pos += actors_list[i]->move_y_speed*time_diff;
					actors_list[i]->z_pos += actors_list[i]->move_z_speed*time_diff;
				}
				else {
					actors_list[i]->x_pos += actors_list[i]->move_x_speed*actors_list[i]->movement_time_left;
					actors_list[i]->y_pos += actors_list[i]->move_y_speed*actors_list[i]->movement_time_left;
					actors_list[i]->z_pos += actors_list[i]->move_z_speed*actors_list[i]->movement_time_left;
				}
#endif	/* ANIMATION_SCALING */
				actors_list[i]->movement_time_left -= time_diff;
				if(actors_list[i]->movement_time_left <= 0){	//we moved all the way
					Uint8 last_command;
					int dx, dy;

					actors_list[i]->moving= 0;	//don't move next time, ok?
					//now, we need to update the x/y_tile_pos, and round off
					//the x/y_pos according to x/y_tile_pos
					last_command= actors_list[i]->last_command;
					//if(HAS_HORSE(i)) {MY_HORSE(i)->busy=0; if(actors_list[i]->actor_id==yourself) printf("%i, %s wakes up Horse\n",thecount, ACTOR(i)->actor_name);}
					if (get_motion_vector(last_command, &dx, &dy)) {
						actors_list[i]->x_tile_pos += dx;
						actors_list[i]->y_tile_pos += dy;

						actors_list[i]->busy = 0;
						//if(actors_list[i]->actor_id==yourself) printf("%i, unbusy(moved)\n", thecount);
						//if(actors_list[i]->actor_id<0) printf("%i, unbusy horse(moved)\n", thecount);

						if (actors_list[i]->que[0] >= move_n &&
							actors_list[i]->que[0] <= move_nw) {
							next_command();
						}
						else {
							actors_list[i]->x_pos= actors_list[i]->x_tile_pos*0.5;
							actors_list[i]->y_pos= actors_list[i]->y_tile_pos*0.5;
							actors_list[i]->z_pos= get_actor_z(actors_list[i]);
						}
					} else {
						actors_list[i]->busy = 0;
						//if(actors_list[i]->actor_id==yourself) printf("%i, unbusy(moved2)\n", thecount);
						//if(actors_list[i]->actor_id<0) printf("%i, unbusy horse(moved2)\n", thecount);

					}
				}
			} //moving

			if(actors_list[i]->rotating) {
#ifdef	ANIMATION_SCALING
				tmp_time_diff = min2i(actors_list[i]->rotate_time_left, time_diff);
				actors_list[i]->rotate_time_left -= time_diff;

				//we rotated all the way
				if (actors_list[i]->rotate_time_left <= 0)
				{
					actors_list[i]->rotating = 0;//don't rotate next time, ok?
				}
#else	/* ANIMATION_SCALING */
				actors_list[i]->rotate_time_left -= time_diff;
				if (actors_list[i]->rotate_time_left <= 0) { //we rotated all the way
					actors_list[i]->rotating= 0;//don't rotate next time, ok?
					tmp_time_diff = time_diff + actors_list[i]->rotate_time_left;
/*
#ifdef MORE_ATTACHED_ACTORS					
					if(actors_list[i]->actor_id==yourself) printf("%i, rot: %i\n",thecount,actors_list[i]->rotating);
					if(actors_list[i]->actor_id<0) printf("%i, (horse) rot: %i\n",thecount,actors_list[i]->rotating);
#endif
*/
				}
				else {
					tmp_time_diff = time_diff;
				}
#endif	/* ANIMATION_SCALING */
				actors_list[i]->x_rot+= actors_list[i]->rotate_x_speed*tmp_time_diff;
				actors_list[i]->y_rot+= actors_list[i]->rotate_y_speed*tmp_time_diff;
				actors_list[i]->z_rot+= actors_list[i]->rotate_z_speed*tmp_time_diff;
				if(actors_list[i]->z_rot >= 360) {
					actors_list[i]->z_rot -= 360;
				} else if (actors_list[i]->z_rot <= 0) {
					actors_list[i]->z_rot += 360;
				}
				//if(actors_list[i]->actor_id==yourself) printf("%i, rotating: z_rot %f,  status %i-%i\n",thecount,actors_list[i]->z_rot,actors_list[i]->rotating,actors_list[i]->moving);
				//if(actors_list[i]->actor_id<0) printf("%i, rotating (horse): z_rot %f,  status %i-%i\n",thecount,actors_list[i]->z_rot,actors_list[i]->rotating,actors_list[i]->moving);
			}//rotating

#ifdef	ANIMATION_SCALING
			actors_list[i]->anim_time += (time_diff*actors_list[i]->cur_anim.duration_scale)/1000.0;
#else	/* ANIMATION_SCALING */
			actors_list[i]->anim_time += ((cur_time-last_update)*actors_list[i]->cur_anim.duration_scale)/1000.0;
#endif	/* ANIMATION_SCALING */
			/*if(ACTOR(i)->anim_time>=ACTOR(i)->cur_anim.duration) {
				if (HAS_HORSE(i)||IS_HORSE(i)) {
						if(MY_HORSE(i)->anim_time<MY_HORSE(i)->cur_anim.duration) {
							MY_HORSE(i)->anim_time=MY_HORSE(i)->cur_anim.duration;
							printf("%i, ANIMATION FORCED\n",thecount);
						}
				} 
			}*/
#ifndef	DYNAMIC_ANIMATIONS
			if (actors_list[i]->calmodel!=NULL){
				//check if emote animation is ended, then remove it
				handle_cur_emote(actors_list[i]);

#ifdef MORE_EMOTES
				if(ACTOR(i)->startIdle!=ACTOR(i)->endIdle){
					if(do_transition(ACTOR(i))) {
						ACTOR(i)->stand_idle=ACTOR(i)->sit_idle=0; //force set_on_idle
						set_on_idle(i);
					}
				}
#endif
#ifdef	ANIMATION_SCALING
				CalModel_Update(actors_list[i]->calmodel, (time_diff * actors_list[i]->cur_anim.duration_scale) / 1000.0f);
#else	/* ANIMATION_SCALING */
				CalModel_Update(actors_list[i]->calmodel, (((cur_time-last_update)*actors_list[i]->cur_anim.duration_scale)/1000.0));
#endif	/* ANIMATION_SCALING */
				build_actor_bounding_box(actors_list[i]);
				{
				int wasbusy = ACTOR(i)->busy;
				missiles_rotate_actor_bones(actors_list[i]);
				if (ACTOR(i)->busy!=wasbusy&&HAS_HORSE(i)) {
					//if(actors_list[i]->actor_id==yourself) printf("%i, %s is no more busy due to missiles_rotate_actor_bones!! Setting the horse free...\n",thecount, ACTOR(i)->actor_name);
					unfreeze_horse(i);
				}

				}
				if (use_animation_program)
				{
					set_transformation_buffers(actors_list[i]);
				}
			}
#endif	//DYNAMIC_ANIMATIONS
		}
	}
	// unlock the actors_list since we are done now
	UNLOCK_ACTORS_LISTS();

	last_update = cur_time;
}

void unqueue_cmd(int i){
	int k;
	int max_queue=0;
	//move que down with one command
	for(k=0;k<MAX_CMD_QUEUE-1;k++) {
		if(k>max_queue && actors_list[i]->que[k]!=nothing)max_queue=k;
			actors_list[i]->que[k]=actors_list[i]->que[k+1];
		}
	actors_list[i]->que[k]=nothing;
}


void print_queue(actor *act) {
	int k;

	printf("   Actor %s queue:",act->actor_name);
	printf(" -->");
	for(k=0; k<MAX_CMD_QUEUE; k++){
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

	if (act->attached_actor >= 0) {
		printf("   Horse %s queue:",act->actor_name);
		printf(" -->");
		for(k=0; k<MAX_CMD_QUEUE; k++){
			if(actors_list[act->attached_actor]->que[k]==enter_combat) printf("IC");
			if(actors_list[act->attached_actor]->que[k]==leave_combat) printf("LC");
			if(actors_list[act->attached_actor]->que[k]>=move_n&&actors_list[act->attached_actor]->que[k]<=move_nw) printf("M");
			if(actors_list[act->attached_actor]->que[k]>=turn_n&&actors_list[act->attached_actor]->que[k]<=turn_nw) printf("R");
			printf("%2i|",actors_list[act->attached_actor]->que[k]);
		}
		printf("\n");
	}

}

void attached_info(int i, int c){
					if(actors_list[i]->actor_id==yourself&&actors_list[i]->que[0]!=nothing) {
						printf("%i---------> DOING: %i -----------\n",c,actors_list[i]->que[0]);
						print_queue(actors_list[i]);
					}
					if(actors_list[i]->actor_id<0&&MY_HORSE(i)->actor_id==yourself&&actors_list[i]->que[0]!=wait_cmd&&actors_list[i]->que[0]!=nothing){
						printf("%i---------> DOING (horse): %i ---\n",c,actors_list[i]->que[0]);
						print_queue(actors_list[actors_list[i]->attached_actor]);
					}

}

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

int coun= 0;
void move_to_next_frame()
{
	int i;
	//int numFrames=0;
	//char frame_exists;
	//struct CalMixer *mixer;
	//char str[255];

	LOCK_ACTORS_LISTS();
	for(i=0;i<max_actors;i++) {
		if(actors_list[i]!=NULL) {
			if (actors_list[i]->calmodel!=NULL) {
			if ((ACTOR(i)->stop_animation==1)&&(ACTOR(i)->anim_time>=ACTOR(i)->cur_anim.duration)){
					


					//if(actors_list[i]->actor_id==yourself) printf("%i, unbusy: anim %i, anim_time %f, duration %f\n",thecount,actors_list[i]->cur_anim.anim_index,actors_list[i]->anim_time,actors_list[i]->cur_anim.duration);
					//if(actors_list[i]->actor_id<0&&MY_HORSE(i)->actor_id==yourself) printf("%i, (horse) unbusy: anim %i, anim_time %f, duration %f\n",thecount,actors_list[i]->cur_anim.anim_index,actors_list[i]->anim_time,actors_list[i]->cur_anim.duration);

					if(HAS_HORSE(i)) {
				//rotations during idle animation like when server sends turn_n..turn_nw
				//need to be synchronized on the minimum remaining animation time between
				//the idle of the horse and the actor.
						if(
						  /*(MY_HORSE(i)->anim_time<MY_HORSE(i)->cur_anim.duration)&&*/
						  MY_HORSE(i)->cur_anim.kind==cycle){
								//MY_HORSE(i)->anim_time=MY_HORSE(i)->cur_anim.duration;
								MY_HORSE(i)->busy=0;
								MY_HORSE(i)->in_aim_mode=0;
								set_on_idle(MY_HORSE_ID(i));
								//MY_HORSE(i)->stop_animation=0;
								//if(actors_list[i]->actor_id==yourself) printf("%i, %s stops Horse\n",thecount, ACTOR(i)->actor_name);
							}
					} else if (IS_HORSE(i)) {
						if(MY_HORSE(i)->anim_time<MY_HORSE(i)->cur_anim.duration) {
							//wait for actor
							//if(MY_HORSE(i)->actor_id==yourself) printf("%i, Horse waits for %s\n",thecount, MY_HORSE(i)->actor_name);
							continue;
						}
					}
					
					

					actors_list[i]->busy=0;
					if (actors_list[i]->in_aim_mode == 2) {
						// we really leave the aim mode only when the animation is finished
						actors_list[i]->in_aim_mode = 0;
						missiles_log_message("%s (%d): leaving range mode finished!\n",
											 actors_list[i]->actor_name, actors_list[i]->actor_id);

#ifndef	NEW_TEXTURES
						// then we do all the item changes that have been delayed
						flush_delayed_item_changes(actors_list[i]);
#endif	/* NEW_TEXTURES */
					}
				}
			}
#ifdef	NEW_TEXTURES
			if (actors_list[i]->in_aim_mode == 0)
			{
				if (actors_list[i]->is_enhanced_model != 0)
				{
					if (get_actor_texture_ready(actors_list[i]->texture_id))
					{
						use_ready_actor_texture(actors_list[i]->texture_id);
					}
				}

				if (actors_list[i]->delayed_item_changes_count > 0)
				{
					// we really leave the aim mode only when the animation is finished
					actors_list[i]->delay_texture_item_changes = 0;

					// then we do all the item changes that have been delayed
					flush_delayed_item_changes(actors_list[i]);

					actors_list[i]->delay_texture_item_changes = 1;
				}
			}
#endif	/* NEW_TEXTURES */

			// we change the idle animation only when the previous one is finished
			if (actors_list[i]->stand_idle && actors_list[i]->anim_time >= actors_list[i]->cur_anim.duration - 0.2)
			{
				if (!is_actor_held(actors_list[i]))
				{
					set_on_idle(i);
				}
			}

			if (actors_list[i]->cur_anim.anim_index==-1) {
				actors_list[i]->busy=0;
				//if(actors_list[i]->actor_id==yourself) printf("%i, unbusy(-1)\n", thecount);
				//if(actors_list[i]->actor_id<0) printf("%i, unbusy horse(-1)\n", thecount);
			}

			//first thing, decrease the damage time, so we will see the damage splash only for 2 seconds
			if(actors_list[i]->damage_ms) {
				actors_list[i]->damage_ms-=80;
				if(actors_list[i]->damage_ms<0)actors_list[i]->damage_ms=0;
			}

			//9 frames, not moving, and another command is queued farther on (based on how long we've done this action)
			if(!actors_list[i]->moving && !actors_list[i]->rotating){
				
				/*	actors_list[i]->stop_animation=1;	//force stopping, not looping
					actors_list[i]->busy=0;	//ok, take the next command
					LOG_TO_CONSOLE(c_green2,"FREE");
					//Idle here?
				*/
			}

			if(actors_list[i]->stop_animation) {

				//we are done with this guy
				//Should we go into idle here?
			}
			if(!ACTOR(i)->busy){
				if(ACTOR(i)->attached_actor>=0&&ACTOR(i)->actor_id>=0&&
				(
				(ACTOR(i)->last_command>=enter_aim_mode&&ACTOR(i)->last_command<=aim_mode_fire)||
				(ACTOR(i)->last_command>=enter_combat&&ACTOR(i)->last_command<=leave_combat)
				)
				) {
					unfreeze_horse(i);
				}
			}
			if(HAS_HORSE(i)&&ACTOR(i)->in_aim_mode==3) { //when no_action==1
				ACTOR(i)->in_aim_mode=0;
				unfreeze_horse(i);
				//printf("%i,Unfreeze after no_action==1\n",thecount);
			}
		}
	}
	UNLOCK_ACTORS_LISTS();
}



struct cal_anim *get_pose(actor *a, int pose_id, int pose_type, int held) {
	hash_entry *he,*eh;
	emote_data *pose;

	eh=hash_get(emotes,(NULL+pose_id));
	pose = eh->item;

	he=hash_get(actors_defs[a->actor_type].emote_frames, (NULL+pose->anims[pose_type][0][held]->ids[0]));
	if (he) return (struct cal_anim*) he->item;
	else return NULL;
}

struct cal_anim *get_pose_frame(int actor_type, actor *a, int pose_type, int held){

	hash_entry *he;
	int a_type=emote_actor_type(actor_type);

	//find the pose. Pose is the first anim of the first frame
	if (a->poses[pose_type]) {
		//printf("getting pose for %s\n",a->actor_name);
		he=hash_get(actors_defs[actor_type].emote_frames, (NULL+a->poses[pose_type]->anims[a_type][0][held]->ids[0]));
		if (he) return (struct cal_anim*) he->item;
	}
	//no pose or no emote..set defaults
	if (!a->poses[pose_type]) {
		//printf("no pose for %s\n",a->actor_name);
		switch(pose_type){
			case EMOTE_SITTING:
				return &actors_defs[a->actor_type].cal_frames[cal_actor_idle_sit_frame];
			case EMOTE_STANDING:
			    if(held) {
				attachment_props *att_props = get_attachment_props_if_held(a);
				if (att_props)
					return &att_props->cal_frames[cal_attached_idle_frame];
			    } else
	                	// 75% chance to do idle1
	                    if (actors_defs[a->actor_type].cal_frames[cal_actor_idle2_frame].anim_index != -1 
				&& RAND(0, 3) == 0){
				return &actors_defs[a->actor_type].cal_frames[cal_actor_idle2_frame]; //idle2
	                    } else {
        	                return &actors_defs[a->actor_type].cal_frames[cal_actor_idle1_frame]; //idle1
			    }
			case EMOTE_RUNNING:
			    if(held) {
				attachment_props *att_props = get_attachment_props_if_held(a);
				if (att_props)
					return &att_props->cal_frames[cal_attached_run_frame/*get_held_actor_motion_frame(a)*/];
			    } else
				return &actors_defs[actor_type].cal_frames[cal_actor_run_frame/*get_actor_motion_frame(a)*/];
			case EMOTE_WALKING:
			    if(held) {
				attachment_props *att_props = get_attachment_props_if_held(a);
				if (att_props)
					return &att_props->cal_frames[cal_attached_walk_frame/*get_held_actor_motion_frame(a)*/];
			    } else
				return &actors_defs[actor_type].cal_frames[cal_actor_walk_frame/*get_actor_motion_frame(a)*/];
			default:
				return NULL;
			break;
		}
	}
	return NULL;
}

void set_on_idle(int actor_idx)
{
    actor *a = actors_list[actor_idx];
    if(!a->dead) {
        a->stop_animation=0;
	//we have an emote idle, ignore the rest
	if(a->cur_emote.idle.anim_index>=0) 
		return;

        if(a->fighting){
			if(a->attached_actor>=0) {
				//both for horses and actors
				if(IS_HORSE(actor_idx))
					cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].cal_frames[cal_actor_combat_idle_frame]);
				else if (ACTOR_WEAPON(actor_idx)->turn_horse&&ACTOR(actor_idx)->horse_rotated) cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].cal_frames[cal_actor_combat_idle_held_frame]);
				else cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].cal_frames[cal_actor_combat_idle_held_unarmed_frame]);
			} else
            cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].cal_frames[cal_actor_combat_idle_frame]);
        }
        else if (a->in_aim_mode == 1) {
			if(a->actor_id<0){
				//ranging horse
				if(a->cur_anim.anim_index!=actors_defs[a->actor_type].cal_frames[cal_actor_idle1_frame].anim_index&&
				   a->cur_anim.anim_index!=actors_defs[a->actor_type].cal_frames[cal_actor_idle2_frame].anim_index){
				   //printf("%i, horse on idle from %i\n",thecount, a->cur_anim.anim_index);
				   cal_actor_set_anim(actor_idx, *get_pose_frame(a->actor_type,a,EMOTE_STANDING,0));
			}
			} else if(a->attached_actor>=0) {
				cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].weapon[a->cur_weapon].cal_frames[cal_weapon_range_idle_held_frame]);
			} else
            cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].weapon[a->cur_weapon].cal_frames[cal_weapon_range_idle_frame]);
        }
        else if(!a->sitting) {
            // we are standing, see if we can activate a stand idle
            if(!a->stand_idle||a->cur_anim.anim_index<0){
                if (actors_defs[a->actor_type].group_count == 0){
                //if(a->actor_id<0) printf("%i, horse on standing idle from %i\n",thecount, a->cur_anim.anim_index);
				   
			attachment_props *att_props = get_attachment_props_if_held(a);
			if (att_props) {
				struct cal_anim *ca=get_pose_frame(a->actor_type,a,EMOTE_STANDING,1);
				cal_actor_set_anim(actor_idx, *ca);
				}
			else {
				struct cal_anim *ca=get_pose_frame(a->actor_type,a,EMOTE_STANDING,0);
				cal_actor_set_anim(actor_idx, *ca);
			}

			//printf("setting standing pose\n");
                }
                else
                {
                    cal_actor_set_random_idle(actor_idx);
                    a->IsOnIdle=1;
                }

                a->stand_idle=1;
            }
        } else	{
            // we are sitting, see if we can activate the sit idle
            if(!a->sit_idle||a->cur_anim.anim_index<0) {
		cal_actor_set_anim(actor_idx, *get_pose_frame(a->actor_type,a,EMOTE_SITTING,0));
		//printf("setting sitting pose\n");
                a->sit_idle=1;
            }
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

int handle_emote_command(int act_id, emote_command *command)
{
	actor *act=actors_list[act_id];
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


		held = (is_actor_held(act))?1:0;
		//printf("actor %i is held: %i\n",act_id,held);
		pose[EMOTE_STANDING] = get_pose_frame(act->actor_type,act,EMOTE_STANDING,held);
		pose[EMOTE_WALKING] = get_pose_frame(act->actor_type,act,EMOTE_WALKING,held);
		pose[EMOTE_SITTING] = get_pose_frame(act->actor_type,act,EMOTE_SITTING,held);
		pose[EMOTE_RUNNING] = get_pose_frame(act->actor_type,act,EMOTE_RUNNING,held);

#ifdef MORE_EMOTES

		if(command->emote->pose<=EMOTE_STANDING) {
			//we have a pose
			hash_entry *he;
			he=hash_get(actors_defs[actor_type].emote_frames, (NULL+command->emote->anims[act->actor_type][0][held]->ids[0]));
			
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
			set_on_idle(act_id);
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
			if(HAS_HORSE(act_id)) {
				MY_HORSE(act_id)->cur_emote.active=1; //synch with horse!!
				//cal_actor_set_emote_anim(MY_HORSE(act_id), frames);
			}
			//printf("unqueue\n");
			unqueue_emote(act);
			//LOG_TO_CONSOLE(c_green2, "Emote command");
			return 0;
		}

	}
	return 0;
}

void rotate_actor_and_horse_by(int id, int mul, float angle){

			//printf("%i. ACTOR %s (rotating: %i): time left -> %i, z speed -> %f\n",thecount,ACTOR(id)->actor_name,actors_list[id]->rotating,actors_list[id]->rotate_time_left,actors_list[id]->rotate_z_speed);

			if(!ACTOR(id)->rotating){
				ACTOR(id)->rotate_z_speed=(float)mul*angle/(float)HORSE_FIGHT_TIME;
				ACTOR(id)->rotate_time_left=HORSE_FIGHT_TIME;
				ACTOR(id)->rotating=1;
				ACTOR(id)->stop_animation=1;
				ACTOR(id)->horse_rotated=(mul<0) ? (1):(0); //<0 enter fight, >=0 leave fight
				MY_HORSE(id)->rotate_z_speed=(float)mul*angle/(float)HORSE_FIGHT_TIME;
				MY_HORSE(id)->rotate_time_left=HORSE_FIGHT_TIME;
				MY_HORSE(id)->rotating=1;
				MY_HORSE(id)->stop_animation=1;
			}

}

void rotate_actor_and_horse(int id, int mul){
	rotate_actor_and_horse_by(id,mul,HORSE_FIGHT_ROTATION);
}

void rotate_actor_and_horse_range(int id, int mul){
	rotate_actor_and_horse_by(id,mul,HORSE_RANGE_ROTATION);
}


//in case the actor is not busy, and has commands in it's que, execute them
void next_command()
{
	int i, index;
	int max_queue=0;


#ifdef MORE_ATTACHED_ACTORS_DEBUG
	thecount++;
#endif

	for(i=0;i<max_actors;i++){
		if(!actors_list[i])continue;//actor exists?
		if(actors_list[i]->que[0]>=emote_cmd
		&&actors_list[i]->que[0]<wait_cmd
		){
			int k;
			add_emote_to_actor(actors_list[i]->actor_id,actors_list[i]->que[0]);
			//actors_list[i]->stop_animation=1;
			//move que down with one command
			for(k=0;k<MAX_CMD_QUEUE-1;k++) {
				if(k>max_queue && actors_list[i]->que[k]!=nothing)max_queue=k;
				actors_list[i]->que[k]=actors_list[i]->que[k+1];
			}
			actors_list[i]->que[max_queue]=nothing;
		}
		if(!actors_list[i]->busy){//Are we busy?
			//are we playing an emote?
			if(actors_list[i]->cur_emote.active
			&&!(actors_list[i]->que[0]>=move_n&&actors_list[i]->que[0]<=move_nw&&actors_list[i]->last_command>=move_n&&actors_list[i]->last_command<=move_nw)
			&&!HAS_HORSE(i)
			) { continue;}
			
			// If the que is empty, check for an emote to display
			while (actors_list[i]->emote_que[0].origin != NO_EMOTE)
				if(!handle_emote_command(i, &actors_list[i]->emote_que[0])) break;
			if(actors_list[i]->que[0]==nothing){//Is the queue empty?
				//if que is empty, set on idle
				set_on_idle(i);
				//synch_attachment(i);
				actors_list[i]->last_command=nothing;//prevents us from not updating the walk/run animation
			} else {
				int actor_type;
				int last_command=actors_list[i]->last_command;
				float z_rot=actors_list[i]->z_rot;
				float targeted_z_rot;
				int no_action = 0;

				actors_list[i]->sit_idle=0;
				actors_list[i]->stand_idle=0;

#ifndef DISABLE_RANGE_MODE_EXIT_BUGFIX
				if (actors_list[i]->is_enhanced_model && actors_list[i]->in_aim_mode == 1 &&
					(actors_list[i]->que[0] < enter_aim_mode || actors_list[i]->que[0] > missile_critical) &&
					(actors_list[i]->que[0] < turn_n || actors_list[i]->que[0] > turn_nw))
				{
					actor *a = actors_list[i];

					LOG_ERROR("%s: %d: command incompatible with range mode detected: %d", __FUNCTION__, __LINE__, actors_list[i]->que[0]);
					missiles_log_message("%s (%d): forcing aim mode exit", a->actor_name, a->actor_id);
					a->cal_h_rot_start = 0.0;
					a->cal_v_rot_start = 0.0;
					a->cal_h_rot_end = 0.0;
					a->cal_v_rot_end = 0.0;
					a->cal_rotation_blend = 1.0;
					a->cal_rotation_speed = 0.0;
					a->cal_last_rotation_time = cur_time;
					a->are_bones_rotating = 1;
					a->in_aim_mode = 0;
					flush_delayed_item_changes(a);
				}
#endif // DISABLE_RANGE_MODE_EXIT_BUGFIX

				actor_type=actors_list[i]->actor_type;
#ifdef MORE_ATTACHED_ACTORS_DEBUG
				//just for debugging
				attached_info(i,thecount);
#endif
				switch(actors_list[i]->que[0]) {
					case kill_me:
/*						if(actors_list[i]->remapped_colors)
						glDeleteTextures(1,&actors_list[i]->texture_id);
						ec_actor_delete(actors_list[i]);
						free(actors_list[i]);
						actors_list[i]=0;*/ //Obsolete
						break;
					case die1:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_die1_frame]);
						actors_list[i]->stop_animation=1;
						actors_list[i]->dead=1;
						break;
					case die2:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_die2_frame]);
						actors_list[i]->stop_animation=1;
						actors_list[i]->dead=1;
						break;
					case pain1:
					case pain2: {
						int painframe = (actors_list[i]->que[0]==pain1) ? (cal_actor_pain1_frame):(cal_actor_pain2_frame);
						attachment_props *att_props = get_attachment_props_if_held(actors_list[i]);
						if (att_props) {
							if(HAS_HORSE(i)&&!ACTOR_WEAPON(i)->unarmed) {
							cal_actor_set_anim(i, att_props->cal_frames[cal_attached_pain_armed_frame]);
							} else
							cal_actor_set_anim(i, att_props->cal_frames[cal_attached_pain_frame]);
						} else

							cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[painframe]);
						actors_list[i]->stop_animation=1;
						break;
					}
/*					case pain2: {
						attachment_props *att_props = get_attachment_props_if_held(actors_list[i]);
						if (att_props)
							cal_actor_set_anim(i, att_props->cal_frames[cal_attached_pain_frame]);
						else
							cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_pain2_frame]);
						actors_list[i]->stop_animation=1;
						break;
					}
*/					case pick:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_pick_frame]);
						actors_list[i]->stop_animation=1;
						break;
					case drop:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_drop_frame]);
						actors_list[i]->stop_animation=1;
						break;
					case harvest:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_harvest_frame]);
						actors_list[i]->stop_animation=1;
						LOG_TO_CONSOLE(c_green2,"Harvesting!");
						break;
					case cast:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_attack_cast_frame]);
						actors_list[i]->stop_animation=1;
						break;
					case ranged:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_attack_ranged_frame]);
						actors_list[i]->stop_animation=1;
						break;
					case sit_down:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_sit_down_frame]);
						actors_list[i]->stop_animation=1;
						actors_list[i]->sitting=1;
						if(actors_list[i]->actor_id==yourself)
							you_sit=1;
						break;
					case stand_up:
						//LOG_TO_CONSOLE(c_green2,"stand_up");
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_stand_up_frame]);
						actors_list[i]->stop_animation=1;
						actors_list[i]->sitting=0;
						if(actors_list[i]->actor_id==yourself)
							you_sit=0;
						break;
					case enter_combat:
					case leave_combat:
						{
						int fight_k = (actors_list[i]->que[0]==enter_combat) ? (1):(0);
						int combat_frame = (fight_k) ? (cal_actor_in_combat_frame):(cal_actor_out_combat_frame);
						int combat_held_frame = (fight_k) ? (cal_actor_in_combat_held_frame):(cal_actor_out_combat_held_frame);
						int combat_held_unarmed_frame = (fight_k) ? (cal_actor_in_combat_held_unarmed_frame):(cal_actor_out_combat_held_unarmed_frame);
						int mul_angle = (fight_k) ? (-1):(1);

						if(HAS_HORSE(i)){
							//rotate horse and actor if needed
							if(ACTOR_WEAPON(i)->turn_horse) {
							if(fight_k||ACTOR(i)->horse_rotated) rotate_actor_and_horse(i,mul_angle);
							cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[combat_held_frame]);
							}
							else cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[combat_held_unarmed_frame]);
							//horse enter combat
							MY_HORSE(i)->fighting=fight_k;
							MY_HORSE(i)->stop_animation=1;
							cal_actor_set_anim(MY_HORSE_ID(i),actors_defs[MY_HORSE(i)->actor_type].cal_frames[combat_frame]);
						} else
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[combat_frame]);

						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=fight_k;
						}
						break;
/*					case leave_combat:
#ifdef MORE_ATTACHED_ACTORS
						if(HAS_HORSE(i)){
							cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_out_combat_held_frame]);
							//rotate counterclowwise horse and actor
							add_rotation_to_actor(i,HORSE_FIGHT_ROTATION,HORSE_FIGHT_TIME);
							rotate_actor(MY_HORSE_ID(i),HORSE_FIGHT_ROTATION,HORSE_FIGHT_TIME);
							
							cal_actor_set_anim(MY_HORSE_ID(i),actors_defs[MY_HORSE(i)->actor_type].cal_frames[cal_actor_out_combat_frame]);
							MY_HORSE(i)->stop_animation=1;
							MY_HORSE(i)->fighting=0;
						} else
#endif
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_out_combat_frame]);
						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=0;
						break;
*/
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
						index = -1;
						switch (actors_list[i]->que[0])
						{
							case attack_down_10:
								index++;
							case attack_down_9:
								index++;
							case attack_down_8:
								index++;
							case attack_down_7:
								index++;
							case attack_down_6:
								index++;
							case attack_down_5:
								index++;
							case attack_down_4:
								index++;
							case attack_down_3:
								index++;
							case attack_down_2:
								index++;
							case attack_down_1:
								index++;
							case attack_up_10:
								index++;
							case attack_up_9:
								index++;
							case attack_up_8:
								index++;
							case attack_up_7:
								index++;
							case attack_up_6:
								index++;
							case attack_up_5:
								index++;
							case attack_up_4:
								index++;
							case attack_up_3:
								index++;
							case attack_up_2:
								index++;
							case attack_up_1:
								index++;
								break;
							default:
								break;
						}
						if (actors_list[i]->is_enhanced_model) {
							if(HAS_HORSE(i)/*actors_list[i]->attached_actor>=0&&actors_list[i]->actor_id>=0*/) {
								//actor does combat anim
								index+=cal_weapon_attack_up_1_held_frame; //30; //select held weapon animations
								cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[index]);
							} else {
								//normal weapon animation
								cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[index]);
							}
						} else {
							//non enhanced models
						if(HAS_HORSE(i)) {
								//non enhanced model with a horse
								index+=cal_actor_attack_up_1_held_frame;
								cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[index]);
						} else
						 {
							//select normal actor att frames
							index +=cal_actor_attack_up_1_frame;
							cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[index]);
						 }
						}
						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

#ifdef NEW_SOUND
						// Maybe play a battlecry sound
						add_battlecry_sound(actors_list[i]);
#endif // NEW_SOUND
						//check if a horse rotation is needed
						if(HAS_HORSE(i)&&!ACTOR(i)->horse_rotated&&ACTOR_WEAPON(i)->turn_horse) {
							//horse, not rotated, to be rotated -> do rotation
							rotate_actor_and_horse(i,-1);
						} else if (HAS_HORSE(i)&&ACTOR(i)->horse_rotated&&!ACTOR_WEAPON(i)->turn_horse) {
							//horse, rotated, not to be rotated -> undo rotation
							ACTOR(i)->horse_rotated=0;							
							rotate_actor_and_horse(i,1);

						}							
						break;
					case turn_left:
					case turn_right: 
					{
						int mul= (actors_list[i]->que[0]==turn_left) ? (1):(-1);
						int turnframe=(actors_list[i]->que[0]==turn_left) ? (cal_actor_turn_left_frame):(cal_actor_turn_right_frame);

						//LOG_TO_CONSOLE(c_green2,"turn left");
						actors_list[i]->rotate_z_speed=mul*45.0/540.0;
						actors_list[i]->rotate_time_left=540;
						actors_list[i]->rotating=1;
						//generate a fake movement, so we will know when to make the actor
						//not busy
						actors_list[i]->move_x_speed=0;
						actors_list[i]->move_y_speed=0;
						actors_list[i]->move_z_speed=0;
						actors_list[i]->movement_time_left=540;
						actors_list[i]->moving=1;
						//test
						if(!actors_list[i]->fighting){
							attachment_props *att_props = get_attachment_props_if_held(actors_list[i]);
							if (att_props)
								cal_actor_set_anim(i, *get_pose_frame(actors_list[i]->actor_type,actors_list[i],EMOTE_MOTION(actors_list[i]),1));
							else
							cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[turnframe]);
						}
						actors_list[i]->stop_animation=0;
						break;
					}
				case enter_aim_mode:
					missiles_log_message("%s (%d): cleaning the queue from enter_aim_mode command",
										 actors_list[i]->actor_name, actors_list[i]->actor_id);
					missiles_clean_range_actions_queue(actors_list[i]);

					if (actors_list[i]->in_aim_mode == 0) {
						missiles_log_message("%s (%d): enter in aim mode", actors_list[i]->actor_name, actors_list[i]->actor_id);
					//if(actors_list[i]->actor_id==yourself) printf("%i, enter aim 0\n",thecount);
					if(actors_list[i]->attached_actor>=0){
						if (!ACTOR(i)->horse_rotated) {rotate_actor_and_horse_range(i,-1); ACTOR(i)->horse_rotated=1;}						//set the horse aim mode
						actors_list[actors_list[i]->attached_actor]->in_aim_mode=1;
						//stop_attachment(i); //add a wait
						//we could start a horse_ranged_in
						set_on_idle(actors_list[i]->attached_actor);
						cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[cal_weapon_range_in_held_frame]);
					} else
						cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[cal_weapon_range_in_frame]);
						
						actors_list[i]->cal_h_rot_start = 0.0;
						actors_list[i]->cal_v_rot_start = 0.0;
						if (actors_list[i]->range_actions_count != 1) {
							LOG_ERROR("%s (%d): entering in range mode with an non empty range action queue!",
									  actors_list[i]->actor_name, actors_list[i]->actor_id);
						}
					}
					else {
                        float range_rotation;
						range_action *action = &actors_list[i]->range_actions[0];
						//if(actors_list[i]->actor_id==yourself) printf("%i, enter aim %i\n",thecount,actors_list[i]->in_aim_mode);
						missiles_log_message("%s (%d): aiming again (time=%d)", actors_list[i]->actor_name, actors_list[i]->actor_id, cur_time);
						if(HAS_HORSE(i)) {
						cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[cal_weapon_range_idle_held_frame]);
						//if (!ACTOR(i)->horse_rotated) {rotate_actor_and_horse(i,-1); ACTOR(i)->horse_rotated=1;}
						
						}
						else
						cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[cal_weapon_range_idle_frame]);
						actors_list[i]->cal_h_rot_start = (actors_list[i]->cal_h_rot_start *
														   (1.0 - actors_list[i]->cal_rotation_blend) +
														   actors_list[i]->cal_h_rot_end *
														   actors_list[i]->cal_rotation_blend);
						actors_list[i]->cal_v_rot_start = (actors_list[i]->cal_v_rot_start *
														   (1.0 - actors_list[i]->cal_rotation_blend) +
														   actors_list[i]->cal_v_rot_end *
														   actors_list[i]->cal_rotation_blend);

						/* we look if the actor is still around and if yes,
						 * we recompute it's position */
						if (action->aim_actor >= 0) {
							actor *aim_actor = get_actor_ptr_from_id(action->aim_actor);
							if (aim_actor) {
								cal_get_actor_bone_absolute_position(aim_actor, get_actor_bone_id(aim_actor, body_top_bone), NULL, action->aim_position);
							}
						}


						if(HAS_HORSE(i)) ACTOR(i)->z_rot+=HORSE_RANGE_ROTATION;
						range_rotation = missiles_compute_actor_rotation(&actors_list[i]->cal_h_rot_end,
																		 &actors_list[i]->cal_v_rot_end,
																		 actors_list[i], action->aim_position);
						if(HAS_HORSE(i)) ACTOR(i)->z_rot-=HORSE_RANGE_ROTATION;
						actors_list[i]->cal_rotation_blend = 0.0;
						actors_list[i]->cal_rotation_speed = 1.0/360.0;
                        actors_list[i]->cal_last_rotation_time = cur_time;
						actors_list[i]->are_bones_rotating = 1;
						actors_list[i]->stop_animation = 1;
						if (action->state == 0) action->state = 1;

						if (range_rotation != 0.0) {
							missiles_log_message("%s (%d): not facing its target => client side rotation needed",
                                                 actors_list[i]->actor_name, actors_list[i]->actor_id);
							if (actors_list[i]->rotating) {
								range_rotation += actors_list[i]->rotate_z_speed * actors_list[i]->rotate_time_left;
							}
							actors_list[i]->rotate_z_speed = range_rotation/360.0;
							actors_list[i]->rotate_time_left=360;
							actors_list[i]->rotating=1;
							if(HAS_HORSE(i)){
								//printf("rotating the horse client side!\n");
								actors_list[actors_list[i]->attached_actor]->rotate_z_speed=range_rotation/360.0;
								actors_list[actors_list[i]->attached_actor]->rotate_time_left=360;
								actors_list[actors_list[i]->attached_actor]->rotating=1;								
							}
						}
					}
					break;

				case leave_aim_mode:
					if (actors_list[i]->in_aim_mode != 1) {
						if (actors_list[i]->cal_rotation_blend < 0.0 ||
							(actors_list[i]->cal_h_rot_end == 0.0 &&
							 actors_list[i]->cal_v_rot_end == 0.0)) {
							LOG_ERROR("next_command: trying to leave range mode while we are not in it => aborting safely...");
							no_action = 1;
							if (ACTOR(i)->horse_rotated) {rotate_actor_and_horse_range(i,1); ACTOR(i)->horse_rotated=0;}
							break;
						}
						else {
							LOG_ERROR("next_command: trying to leave range mode while we are not in it => continuing because of a wrong actor bones rotation!");
						}
					}

					missiles_log_message("%s (%d): leaving aim mode", actors_list[i]->actor_name, actors_list[i]->actor_id);
					if(HAS_HORSE(i)) {
					cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[cal_weapon_range_out_held_frame]);
					if (ACTOR(i)->horse_rotated) {rotate_actor_and_horse_range(i,1); ACTOR(i)->horse_rotated=0;}
					}
					else
					cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[cal_weapon_range_out_frame]);
					actors_list[i]->cal_h_rot_start = (actors_list[i]->cal_h_rot_start *
													   (1.0 - actors_list[i]->cal_rotation_blend) +
													   actors_list[i]->cal_h_rot_end *
													   actors_list[i]->cal_rotation_blend);
					actors_list[i]->cal_v_rot_start = (actors_list[i]->cal_v_rot_start *
													   (1.0 - actors_list[i]->cal_rotation_blend) +
													   actors_list[i]->cal_v_rot_end *
													   actors_list[i]->cal_rotation_blend);
					actors_list[i]->cal_h_rot_end = 0.0;
					actors_list[i]->cal_v_rot_end = 0.0;
					actors_list[i]->cal_rotation_blend = 0.0;
					actors_list[i]->cal_rotation_speed = 1.0/360.0;
                    actors_list[i]->cal_last_rotation_time = cur_time;
					actors_list[i]->are_bones_rotating = 1;
					actors_list[i]->in_aim_mode = 2;
					actors_list[i]->stop_animation = 1;
					break;

/* 				case aim_mode_reload: */
/* 					missiles_log_message("%s (%d): reload after next fire", actors_list[i]->actor_name, actors_list[i]->actor_id); */
/* 					actors_list[i]->reload = 1; */
/*  					no_action = 1; */
/* 					break; */

				case aim_mode_fire:
					{
						range_action *action = &actors_list[i]->range_actions[0];
						action->state = 3;

						if (actors_list[i]->in_aim_mode != 1) {
							LOG_ERROR("next_command: trying to fire an arrow out of range mode => aborting!");
							no_action = 1;
							missiles_log_message("%s (%d): cleaning the queue from aim_mode_fire command (error)",
												 actors_list[i]->actor_name, actors_list[i]->actor_id);
							missiles_clean_range_actions_queue(actors_list[i]);
							break;
						}

						if (action->reload) {
							missiles_log_message("%s (%d): fire and reload", actors_list[i]->actor_name, actors_list[i]->actor_id);
							// launch fire and reload animation
					
							//if(actors_list[i]->actor_id==yourself) printf("%i, enter reload\n",thecount);
							if(HAS_HORSE(i))
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[cal_weapon_range_fire_held_frame]);
							else
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[cal_weapon_range_fire_frame]);
							actors_list[i]->in_aim_mode = 1;
						}
						else {
							missiles_log_message("%s (%d): fire and leave aim mode", actors_list[i]->actor_name, actors_list[i]->actor_id);
							// launch fire and leave aim mode animation

							//if(actors_list[i]->actor_id==yourself) printf("%i, enter fire & leave\n",thecount);
							if(HAS_HORSE(i))
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[cal_weapon_range_fire_out_held_frame]);
							else
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[cal_weapon_range_fire_out_frame]);
							actors_list[i]->in_aim_mode = 2;
						}

						actors_list[i]->cal_h_rot_start = (actors_list[i]->cal_h_rot_start *
														   (1.0 - actors_list[i]->cal_rotation_blend) +
														   actors_list[i]->cal_h_rot_end *
														   actors_list[i]->cal_rotation_blend);
						actors_list[i]->cal_v_rot_start = (actors_list[i]->cal_v_rot_start *
														   (1.0 - actors_list[i]->cal_rotation_blend) +
														   actors_list[i]->cal_v_rot_end *
														   actors_list[i]->cal_rotation_blend);
						actors_list[i]->cal_h_rot_end = 0.0;
						actors_list[i]->cal_v_rot_end = 0.0;
						actors_list[i]->cal_rotation_blend = 0.0;
						actors_list[i]->cal_rotation_speed = 1.0/360.0;
						actors_list[i]->cal_last_rotation_time = cur_time;
						actors_list[i]->are_bones_rotating = 1;
						actors_list[i]->stop_animation = 1;

						/* In case of a missed shot due to a collision with an actor,
						 * the server send the position of the actor with 0.0 for the Z coordinate.
						 * So we have to compute the coordinate of the ground at this position.
						 */
						if (action->shot_type == MISSED_SHOT &&
							action->fire_position[2] == 0.0) {
							int tile_x = (int)(action->fire_position[0]*2.0);
							int tile_y = (int)(action->fire_position[1]*2.0);
							action->fire_position[2] = get_tile_height(tile_x, tile_y);
							missiles_log_message("missed shot detected: new height computed: %f", action->fire_position[2]);
						}
						else if (action->fire_actor >= 0) {
							actor *fire_actor = get_actor_ptr_from_id(action->fire_actor);
							if (fire_actor) {
								cal_get_actor_bone_absolute_position(fire_actor, get_actor_bone_id(fire_actor, body_top_bone), NULL, action->fire_position);
							}
						}

#if 0 //def DEBUG
						{
							float aim_angle = atan2f(action->aim_position[1] - actors_list[i]->y_pos,
													 action->aim_position[0] - actors_list[i]->x_pos);
							float fire_angle = atan2f(action->fire_position[1] - actors_list[i]->y_pos,
													  action->fire_position[0] - actors_list[i]->x_pos);
							if (aim_angle < 0.0) aim_angle += 2*M_PI;
							if (fire_angle < 0.0) fire_angle += 2*M_PI;
							if (fabs(fire_angle - aim_angle) > M_PI/8.0) {
								char msg[512];
								sprintf(msg,
										"%s (%d): WARNING! Target position is too different from aim position: pos=(%f,%f,%f) aim=(%f,%f,%f) target=(%f,%f,%f) aim_angle=%f target_angle=%f",
										actors_list[i]->actor_name,
										actors_list[i]->actor_id,
										actors_list[i]->x_pos,
										actors_list[i]->y_pos,
										actors_list[i]->z_pos,
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

						missiles_fire_arrow(actors_list[i], action->fire_position, action->shot_type);
						missiles_log_message("%s (%d): cleaning the queue from aim_mode_fire command (end)",
											 actors_list[i]->actor_name, actors_list[i]->actor_id);
						missiles_clean_range_actions_queue(actors_list[i]);
					}
					break;

/* 				case missile_miss: */
/* 					missiles_log_message("%s (%d): will miss his target", actors_list[i]->actor_name, actors_list[i]->actor_id); */
/* 					if (actors_list[i]->shots_count < MAX_SHOTS_QUEUE) */
/* 						actors_list[i]->shot_type[actors_list[i]->shots_count] = MISSED_SHOT; */
/*  					no_action = 1; */
/* 					break; */

/* 				case missile_critical: */
/* 					missiles_log_message("%s (%d): will do a critical hit", actors_list[i]->actor_name, actors_list[i]->actor_id); */
/* 					if (actors_list[i]->shots_count < MAX_SHOTS_QUEUE) */
/* 						actors_list[i]->shot_type[actors_list[i]->shots_count] = CRITICAL_SHOT; */
/*  					no_action = 1; */
/* 					break; */

/* 				case unwear_bow: */
/* 					unwear_item_from_actor(actors_list[i]->actor_id, KIND_OF_WEAPON); */
/*  					no_action = 1; */
/* 					break; */

/* 				case unwear_quiver: */
/* 					unwear_item_from_actor(actors_list[i]->actor_id, KIND_OF_SHIELD); */
/*  					no_action = 1; */
/* 					break; */
					case wait_cmd:
						//horse only
						ACTOR(i)->stand_idle=1;
					continue;

					//ok, now the movement, this is the tricky part
					default:
						if(actors_list[i]->que[0]>=move_n && actors_list[i]->que[0]<=move_nw) {
							float rotation_angle;
							int dx, dy;
							int step_duration = actors_list[i]->step_duration;
							struct cal_anim *walk_anim;

							attachment_props *att_props = get_attachment_props_if_held(actors_list[i]);
							if (att_props)
								walk_anim = get_pose_frame(actors_list[i]->actor_type,actors_list[i],EMOTE_MOTION(actors_list[i]),1);
							else
							walk_anim = get_pose_frame(actors_list[i]->actor_type,actors_list[i],EMOTE_MOTION(actors_list[i]),0);



							actors_list[i]->moving=1;
							actors_list[i]->fighting=0;
							if(last_command<move_n || last_command>move_nw){//update the frame name too
								cal_actor_set_anim(i,*walk_anim);
								actors_list[i]->stop_animation=0;
							}

							if(last_command!=actors_list[i]->que[0]){ //Calculate the rotation
								targeted_z_rot=(actors_list[i]->que[0]-move_n)*45.0f;
								rotation_angle=get_rotation_vector(z_rot,targeted_z_rot);
								actors_list[i]->rotate_z_speed=rotation_angle/360.0;
								if(auto_camera && actors_list[i]->actor_id==yourself){
									camera_rotation_speed=rotation_angle/1000.0;
									camera_rotation_duration=1000;
								}

								actors_list[i]->rotate_time_left=360;
								actors_list[i]->rotating=1;
							}
                            get_motion_vector(actors_list[i]->que[0], &dx, &dy);

							/* if other move commands are waiting in the queue,
							 * we walk at a speed that is close to the server speed
							 * else we walk at a slightly slower speed to wait next
							 * incoming walking commands */
                            if (actors_list[i]->que[1] >= move_n &&
                                actors_list[i]->que[1] <= move_nw) {
                                if (actors_list[i]->que[2] >= move_n &&
                                    actors_list[i]->que[2] <= move_nw) {
									if (actors_list[i]->que[3] >= move_n &&
										actors_list[i]->que[3] <= move_nw)
										actors_list[i]->movement_time_left = (int)(step_duration*0.9); // 3 moves
									else
										actors_list[i]->movement_time_left = step_duration; // 2 moves
								}
                                else
                                    actors_list[i]->movement_time_left = (int)(step_duration*1.1); // 1 move
                            }
                            else {
                                actors_list[i]->movement_time_left = (int)(step_duration*1.2); // 0 move
                            }
							// if we have a diagonal motion, we slow down the animation a bit
							if (dx != 0 && dy != 0)
								actors_list[i]->movement_time_left = (int)(actors_list[i]->movement_time_left*1.2+0.5);

                            // we compute the moving speeds in x, y and z directions
							actors_list[i]->move_x_speed = 0.5*(dx+actors_list[i]->x_tile_pos)-actors_list[i]->x_pos;
							actors_list[i]->move_y_speed = 0.5*(dy+actors_list[i]->y_tile_pos)-actors_list[i]->y_pos;
							actors_list[i]->move_z_speed = get_tile_height(actors_list[i]->x_tile_pos+dx, actors_list[i]->y_tile_pos+dy) - actors_list[i]->z_pos;
							actors_list[i]->move_x_speed /= (float)actors_list[i]->movement_time_left;
							actors_list[i]->move_y_speed /= (float)actors_list[i]->movement_time_left;
							actors_list[i]->move_z_speed /= (float)actors_list[i]->movement_time_left;

							/* we change the speed of the walking animation according to the walking speed and to the size of the actor
							 * we suppose here that the normal speed of the walking animation is 2 meters per second (1 tile in 250ms) */
							actors_list[i]->cur_anim.duration_scale = walk_anim->duration_scale;
							actors_list[i]->cur_anim.duration_scale *= (float)DEFAULT_STEP_DURATION/(actors_list[i]->movement_time_left*actors_list[i]->scale);
							if (actors_defs[actor_type].actor_scale != 1.0)
								actors_list[i]->cur_anim.duration_scale /= actors_defs[actor_type].actor_scale;
							else
								actors_list[i]->cur_anim.duration_scale /= actors_defs[actor_type].scale;
							if (dx != 0 && dy != 0)
								actors_list[i]->cur_anim.duration_scale *= 1.4142315;
						} else if(actors_list[i]->que[0]>=turn_n && actors_list[i]->que[0]<=turn_nw) {
							float rotation_angle;

							int horse_angle=0;
							if(IS_HORSE(i))
								horse_angle=(ACTOR(i)->fighting&&ACTOR_WEAPON(MY_HORSE_ID(i))->turn_horse) ? (HORSE_FIGHT_ROTATION):(0);
							else if(HAS_HORSE(i))
								horse_angle=(ACTOR(i)->fighting&&ACTOR_WEAPON(i)->turn_horse) ? (HORSE_FIGHT_ROTATION):(0);

							if((HAS_HORSE(i)||IS_HORSE(i))&&(ACTOR(i)->in_aim_mode||MY_HORSE(i)->in_aim_mode)) {
								horse_angle=HORSE_RANGE_ROTATION;
								ACTOR(i)->stand_idle=1;
								//if(actors_list[i]->actor_id==yourself) printf("%i, %s rotates\n",thecount, ACTOR(i)->actor_name);
								//if(MY_HORSE(i)->actor_id==yourself) printf("%i, Horse %s rotates\n",thecount, MY_HORSE(i)->actor_name);
							}
							targeted_z_rot=(ACTOR(i)->que[0]-turn_n)*45.0f-horse_angle;
							rotation_angle=get_rotation_vector(ACTOR(i)->z_rot,targeted_z_rot);
							ACTOR(i)->rotate_z_speed=rotation_angle/360.0f;
							ACTOR(i)->rotate_time_left=360;
							ACTOR(i)->rotating=1;
							ACTOR(i)->stop_animation=1;

							if(ACTOR(i)->fighting&&horse_angle!=0) ACTOR(i)->horse_rotated=1;
							missiles_log_message("%s (%d): rotation %d requested", actors_list[i]->actor_name, actors_list[i]->actor_id, actors_list[i]->que[0] - turn_n);
						}
					}

					//mark the actor as being busy
					if (!no_action)
						actors_list[i]->busy=1;
					else if(HAS_HORSE(i)) ACTOR(i)->in_aim_mode=3; //needed to unfreeze the horse in move_to_next_frame
					//if (actors_list[i]->actor_id==yourself) LOG_TO_CONSOLE(c_green2,"Busy");
					//save the last command. It is especially good for run and walk
					actors_list[i]->last_command=actors_list[i]->que[0];

					/* We do the enter in aim mode in two steps in order the actor have
					 * the time to do the load animation before rotating bones. This is
					 * necessary in the case of cross bows where the actor need to use
					 * his foot to reload. So here, we don't remove the enter aim mode
					 * from the queue in order to treat it again but this time in aim mode.
					 */
					if (actors_list[i]->que[0] == enter_aim_mode && actors_list[i]->in_aim_mode == 0) {
						actors_list[i]->in_aim_mode = 1;
						actors_list[i]->last_command=missile_miss; //dirty hack to avoid processing enter_aim_mode twice :/
						continue;
					}
					unqueue_cmd(i);
				}
			}
		}
}

void free_actor_data(int actor_index)
{
	actor *act = actors_list[actor_index];
    if(act->calmodel!=NULL)
        model_delete(act->calmodel);
#ifdef	NEW_TEXTURES
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
#else	/* NEW_TEXTURES */
    if(act->remapped_colors) glDeleteTextures(1,&act->texture_id);
    if(act->is_enhanced_model){
        glDeleteTextures(1,&act->texture_id);
        if(act->body_parts)free(act->body_parts);
    }
#endif	/* NEW_TEXTURES */
#ifdef NEW_SOUND
    stop_sound(act->cur_anim_sound_cookie);
    act->cur_anim_sound_cookie = 0;
#endif	//NEW_SOUND
    ec_actor_delete(act);
}

void destroy_actor(int actor_id)
{
	int i;
    int attached_actor = -1;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	for(i=0;i<max_actors;i++){
		if(actors_list[i])//The timer thread doesn't free memory
			if(actors_list[i]->actor_id==actor_id){
				LOCK_ACTORS_LISTS();
                attached_actor = actors_list[i]->attached_actor;

				if (actor_id == yourself)
					set_our_actor (NULL);
                free_actor_data(i);
				free(actors_list[i]);
				actors_list[i]=NULL;
				if(i==max_actors-1)max_actors--;
				else {
					//copy the last one down and fill in the hole
					max_actors--;
					actors_list[i]=actors_list[max_actors];
					actors_list[max_actors]=NULL;
                    if (attached_actor == max_actors) attached_actor = i;
					if (actors_list[i] && actors_list[i]->attached_actor >= 0)
						actors_list[actors_list[i]->attached_actor]->attached_actor = i;
				}

                if (attached_actor >= 0)
                {
                    free_actor_data(attached_actor);
                    free(actors_list[attached_actor]);
                    actors_list[attached_actor]=NULL;
                    if(attached_actor==max_actors-1)max_actors--;
                    else {
                        //copy the last one down and fill in the hole
                        max_actors--;
                        actors_list[attached_actor]=actors_list[max_actors];
                        actors_list[max_actors]=NULL;
						if (actors_list[attached_actor] && actors_list[attached_actor]->attached_actor >= 0)
							actors_list[actors_list[attached_actor]->attached_actor]->attached_actor = attached_actor;
                    }
                }

				actor_under_mouse = NULL;
				UNLOCK_ACTORS_LISTS();
				break;
			}
	}
}

void destroy_all_actors()
{
	int i=0;
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	set_our_actor (NULL);
	for(i=0;i<max_actors;i++) {
		if(actors_list[i]){
            free_actor_data(i);
			free(actors_list[i]);
			actors_list[i]=NULL;
		}
	}
	max_actors= 0;
	actor_under_mouse = NULL;
	my_timer_adjust= 0;
	harvesting_effect_reference = NULL;
	UNLOCK_ACTORS_LISTS();	//unlock it since we are done
}




void update_all_actors()
{
 	Uint8 str[40];

	//we got a nasty error, log it
	LOG_TO_CONSOLE(c_red2,resync_server);

	destroy_all_actors();
	str[0]=SEND_ME_MY_ACTORS;
	my_tcp_send(my_socket,str,1);
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


void sanitize_cmd_queue(actor *act){
	int k,j;
	for(k=0,j=0;k<MAX_CMD_QUEUE-1-j;k++){
		if(act->que[k]==nothing) j++;
		act->que[k]=act->que[k+j];
	}
	for(k=MAX_CMD_QUEUE-1;k>0&&j>0;k--,j--) act->que[k]=nothing;
}

void add_command_to_actor(int actor_id, unsigned char command)
{
	//int i=0;
	int k=0;
	int k2 = 0;
	//int have_actor=0;
//if ((actor_id==yourself)&&(command==enter_combat)) LOG_TO_CONSOLE(c_green2,"FIGHT!");
	actor * act;
	int isme = 0;
#ifdef EXTRA_DEBUG
	ERR();
#endif
	act= get_actor_ptr_from_id(actor_id);

	if(!act){
		//Resync
		//if we got here, it means we don't have this actor, so get it from the server...
		LOG_ERROR("%s %d - %d\n", cant_add_command, command, actor_id);
	} else {
		LOCK_ACTORS_LISTS();

		//if (get_our_actor()->actor_id==act->actor_id) printf("ADD COMMAND %i to %i\n",command,actor_id);


		if (command == missile_miss) {
			missiles_log_message("%s (%d): will miss his target", act->actor_name, actor_id);
			if (act->range_actions_count <= MAX_RANGE_ACTION_QUEUE &&
				act->range_actions_count > 0)
				act->range_actions[act->range_actions_count-1].shot_type = MISSED_SHOT;
			else
				LOG_ERROR("%s (%d): unable to add a missed shot action, the queue is empty!", act->actor_name, actor_id);
			UNLOCK_ACTORS_LISTS();
			return;
		}
		else if (command == missile_critical) {
			missiles_log_message("%s (%d): will do a critical hit", act->actor_name, actor_id);
			if (act->range_actions_count <= MAX_RANGE_ACTION_QUEUE &&
				act->range_actions_count > 0)
				act->range_actions[act->range_actions_count-1].shot_type = CRITICAL_SHOT;
			else
				LOG_ERROR("%s (%d): unable to add a critical shot action, the queue is empty!", act->actor_name, actor_id);
			UNLOCK_ACTORS_LISTS();
			return;
		}
		else if (command == aim_mode_reload) {
			missiles_log_message("%s (%d): reload after next fire", act->actor_name, actor_id);
			if (act->range_actions_count <= MAX_RANGE_ACTION_QUEUE &&
				act->range_actions_count > 0)
				act->range_actions[act->range_actions_count-1].reload = 1;
			else
				LOG_ERROR("%s (%d): unable to add a reload action, the queue is empty!", act->actor_name, actor_id);
			UNLOCK_ACTORS_LISTS();
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

			if(act->attached_actor>=0) {
				//strip horse queue too
				int j=0;
				actor *horse=actors_list[act->attached_actor];
				
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
				missiles_rotate_actor_bones(get_actor_ptr_from_id(actor_id));
				if (use_animation_program)
				{
					set_transformation_buffers(act);
				}
			}
		}

		k = push_command_in_actor_queue(command, act);

		if (act->attached_actor >= 0){
			//if in aim mode, ignore turning and ranging related commands. We do it manually in next_command()
			switch(command){
				case enter_aim_mode:
				case leave_aim_mode:
				case enter_combat:
				case leave_combat:
				case aim_mode_fire:
					//insert a wait_cmd where the horse must synch with the actor
					k2 = push_command_in_actor_queue(wait_cmd, actors_list[act->attached_actor]);
					break;
				default:
					k2 = push_command_in_actor_queue(command, actors_list[act->attached_actor]);			
			}
		}
		else
			k2 = k;
		{
			actor * me = get_our_actor();
			if (me!=NULL)
				isme = act->actor_id == me->actor_id;
		}


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
			if (act->attached_actor >= 0)
				while(actors_list[act->attached_actor]->que[j2]>=turn_n
					&&actors_list[act->attached_actor]->que[j2]<=turn_nw
					&&j2>=0) j2--; //skip rotations for horse
			if(j>=0&&act->que[j]==leave_combat) {
				//remove leave_combat and enter_combat
				act->que[j]=nothing;
				act->que[k]=nothing;
				sanitize_cmd_queue(act);
				//if(act->actor_id==yourself) printf("   actor %s: skipped %i and %i\n",act->actor_name,j,k);
				if(act->attached_actor >=0&&j2>=0&&actors_list[act->attached_actor]->que[j2]==wait_cmd) {
					//remove leave_combat and enter_combat for horse
					actors_list[act->attached_actor]->que[j2]=nothing;
					actors_list[act->attached_actor]->que[k2]=nothing;
					sanitize_cmd_queue(actors_list[act->attached_actor]);
					//if(act->actor_id==yourself) printf("   horse %s: skipped %i and %i\n",act->actor_name,j2,k2);
				}
			}
			
			//if(act->actor_id==yourself) printf("   ***Skip Done***\n");
			//if(act->actor_id==yourself) print_queue(act);
		}




		switch(command) {
		case enter_combat:
			act->async_fighting= 1;
			if(ranging_lock && auto_disable_ranging_lock)
			{
				ranging_lock= 0;
				LOG_TO_CONSOLE(c_green1, ranginglock_disabled_str);
			}
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
		UNLOCK_ACTORS_LISTS();

		if (k != k2) {
			LOG_ERROR("Inconsistency between queues of attached actors %s (%d) and %s (%d)!",
					  act->actor_name,
					  act->actor_id,
					  actors_list[act->attached_actor]->actor_name,
					  actors_list[act->attached_actor]->actor_id);
		}
		else
		if(k>MAX_CMD_QUEUE-2){
			int i;
			int k;
			LOG_ERROR("Too much commands in the queue for actor %d (%s) => skip emotes!",
					  act->actor_id, act->actor_name);
			for(i=MAX_CMD_QUEUE-1;i>=0;i--) {
				if(act->que[i]>=emote_cmd&&act->que[i]<wait_cmd) {
					//skip this emote
					for(k=i;k<MAX_CMD_QUEUE-1;k++) {
						act->que[k]=act->que[k+1];
						if(act->attached_actor>=0)
						actors_list[act->attached_actor]->que[k]=actors_list[act->attached_actor]->que[k+1];
					}
					act->que[k]=nothing;
					actors_list[act->attached_actor]->que[k]=nothing;
					add_command_to_actor(actor_id,command); //RECURSIVE!!!! should be done only one time
					return;
				}
			}
			//if we are here no emotes have been skipped
			LOG_ERROR("Too much commands in the queue for actor %d (%s) => resync!\n",
					  act->actor_id, act->actor_name);
#ifdef	ANIMATION_SCALING
			LOG_ERROR("animation_scale: %f\n", act->animation_scale);
#endif	/* ANIMATION_SCALING */
			for (i = 0; i < MAX_CMD_QUEUE; ++i)
				LOG_ERROR("%dth command in the queue: %d\n", i, (int)act->que[i]);
			update_all_actors();
		}
	}
}


void queue_emote(actor *act, emote_data *emote){
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

void add_emote_to_actor(int actor_id, int emote_id){

	actor *act;
	emote_data *emote;
	hash_entry *he;

	LOCK_ACTORS_LISTS();
	act=get_actor_ptr_from_id(actor_id);
	
	//printf("SERVER MSG\nwe have actor %i %p\n",actor_id,act);
	if(emote_id!=0) {
		//dirty, but avoids warnings :P
		he=hash_get(emotes,(void*)(NULL+emote_id));
		if(!he) {
			LOG_ERROR("%s (Emote) %i- NULL emote passed", cant_add_command,emote_id);
			UNLOCK_ACTORS_LISTS();
			return;
		}
		emote = he->item;
		/*if(emote->pose<=EMOTE_STANDING){
			//we have a pose, set it and return
			//printf("setting pose %i (%i) for actor %s\n",emote->id,emote->pose,act->actor_name);
			act->poses[emote->pose]=emote;
			//force set_idle
			if(emote->pose==EMOTE_SITTING) act->sit_idle=0;
			if(emote->pose==EMOTE_STANDING) act->stand_idle=0;
			UNLOCK_ACTORS_LISTS();
			return;
		}*/
	} else emote=NULL;
	
	//printf("emote message to be added %p\n",emote);
	if (!act) {
		LOG_ERROR("%s (Emote) %d - NULL actor passed", cant_add_command, emote);
	} else {
		queue_emote(act,emote);
	}
	UNLOCK_ACTORS_LISTS();

}



void get_actor_damage(int actor_id, int damage)
{
	//int i=0;
	actor * act;
	float blood_level;
	float bone_list[1024][3];
	int total_bones;
	int bone;
	float bone_x, bone_y, bone_z;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	act = get_actor_ptr_from_id(actor_id);

	if(!act){
		//if we got here, it means we don't have this actor, so get it from the server...
	} else {
		if(floatingmessages_enabled){
			act->last_health_loss=cur_time;
		}

		if (actor_id == yourself)
			set_last_damage(damage);

		act->damage=damage;
		act->damage_ms=2000;
		act->cur_health-=damage;

		if (act->cur_health <= 0) {
#ifdef NEW_SOUND
			add_death_sound(act);
#endif // NEW_SOUND
			increment_death_counter(act);
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
				ec_create_impact_blood(bone_x, bone_y, bone_z, ((float)rand()) * blood_level / RAND_MAX / 13.0, ((float)rand()) * blood_level / RAND_MAX / 13.0, ((float)rand()) * blood_level / RAND_MAX / 13.0, (poor_man ? 6 : 10), blood_level);
			}
		}
	}
}

void get_actor_heal(int actor_id, int quantity)
{
	//int i=0;
	actor *act;
#ifdef EXTRA_DEBUG
	ERR();
#endif
	act = get_actor_ptr_from_id(actor_id);

	if(!act){
		//if we got here, it means we don't have this actor, so get it from the server...
	} else {

		if (actor_id == yourself)
			set_last_heal(quantity);

		if(floatingmessages_enabled){
			act->damage=-quantity;
			act->damage_ms=2000;
			act->last_health_loss=cur_time;
		}

		act->cur_health+=quantity;
	}
	//if we got here, it means we don't have this actor, so get it from the server...
}

void get_actor_health(int actor_id, int quantity)
{
	//int i=0;
	actor *act;
#ifdef EXTRA_DEBUG
	ERR();
#endif
	act = get_actor_ptr_from_id(actor_id);

	if(!act){
		//if we got here, it means we don't have this actor, so get it from the server...
	} else {
//		if(floatingmessages_enabled){
			//act->damage=-quantity;
			//act->damage_ms=2000;
			//act->last_health_loss=cur_time;
//		}

		act->max_health=quantity;
	}
	//if we got here, it means we don't have this actor, so get it from the server...
}

void move_self_forward()
{
	int x,y,rot,tx,ty;

	actor *me = get_our_actor ();

	if(!me)return;//Wtf!?

	x=me->x_tile_pos;
	y=me->y_tile_pos;
	rot=(int)rint(me->z_rot/45.0f);
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

	//check to see if the coordinates are OUTSIDE the map
	if(ty<0 || tx<0 || tx>=tile_map_size_x*6 || ty>=tile_map_size_y*6) {
		return;
	}
	if (pf_follow_path) {
		pf_destroy_path();
	}

	move_to (tx, ty, 0);
}


void actor_check_string(actor_types *act, const char *section, const char *type, const char *value)
{
	if (value == NULL || *value=='\0')
	{
#ifdef DEBUG
		LOG_ERROR("Data Error in %s(%d): Missing %s.%s", act->actor_name, act->actor_type, section, type);
#endif // DEBUG
	}
}

void actor_check_int(actor_types *act, const char *section, const char *type, int value)
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
void free_emote_data(void *data){
	emote_data *emote=data;
	emote_frame *head,*frame,*tf;
	int i,j,k;
	if (!emote) return;
	for(i=0;i<EMOTE_ACTOR_TYPES;i++)
		for(j=0;j<4;j++)
			for(k=0;k<2;k++) {
				head=emote->anims[i][j][k];
				if(!head) continue;
				frame=head;
				while(frame){
					tf=frame->next;
					free(frame);
					frame=tf;	
				}
				flag_emote_frames(emote,head);
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
	hash_add(emotes,(void*)(NULL+id),(void*)emote);
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

int read_emotes_defs(const char *dir, const char *index)
{
	const xmlNode *root;
	xmlDoc *doc;
	char fname[120];
	int ok = 1;

	safe_snprintf(fname, sizeof(fname), "%s/%s", dir, index);

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
			if(shirt->arms_name==NULL || *shirt->arms_name=='\0')
				get_item_string_value(shirt->arms_name, sizeof(shirt->arms_name), default_node, (xmlChar*)"arms");
			if(shirt->model_name==NULL || *shirt->model_name=='\0'){
				get_item_string_value(shirt->model_name, sizeof(shirt->model_name), default_node, (xmlChar*)"mesh");
				shirt->mesh_index= cal_load_mesh(act, shirt->model_name, "shirt");
			}
			if(shirt->torso_name==NULL || *shirt->torso_name=='\0')
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
			if(skin->hands_name==NULL || *skin->hands_name=='\0')
				get_item_string_value(skin->hands_name, sizeof(skin->hands_name), default_node, (xmlChar*)"hands");
			if(skin->head_name==NULL || *skin->head_name=='\0')
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
			if(legs->legs_name==NULL || *legs->legs_name=='\0')
				get_item_string_value(legs->legs_name, sizeof(legs->legs_name), default_node, (xmlChar*)"skin");
			if(legs->model_name==NULL || *legs->model_name=='\0'){
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
			if(weapon->skin_name==NULL || *weapon->skin_name=='\0')
				get_item_string_value(weapon->skin_name, sizeof(weapon->skin_name), default_node, (xmlChar*)"skin");
			if(type_idx!=GLOVE_FUR && type_idx!=GLOVE_LEATHER){ // these dont have meshes
				if(weapon->model_name==NULL || *weapon->model_name=='\0'){
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
		if(part->skin_name==NULL || *part->skin_name=='\0')
			if(strcmp(part_name, "head")){ // heads don't have separate skins here
				get_item_string_value(part->skin_name, sizeof(part->skin_name), default_node, (xmlChar*)"skin");
			}
		if(part->model_name==NULL || *part->model_name=='\0'){
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
	if (!have_sound_config) return 0;

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
		if(part->model_name==NULL || *part->model_name=='\0'){
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
	int ok = 1, index;

	if (cfg == NULL) return 0;

	for (item = cfg; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			index = -1;
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
					hash_add(act->emote_frames, (void*)(NULL+j), (void*)anim);					
				}
				continue;
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
			if(boots->boots_name==NULL || *boots->boots_name=='\0')
				get_item_string_value(boots->boots_name, sizeof(boots->boots_name), default_node, (xmlChar*)"skin");
			if(boots->model_name==NULL || *boots->model_name=='\0'){
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
#ifdef NEW_SOUND
			} else if (!strcmp(name, "sounds")) {
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
		char	str[256];
		char    name[256];

		safe_strncpy(name, get_string_property(cfg, "type"), sizeof(name));
		safe_snprintf(str, sizeof(str), "Data Error in %s(%d): Actor ID out of range %d",
			name, act_idx, act_idx
		);
		LOG_ERROR(str);
		return 0;
	}

	act= &(actors_defs[act_idx]);
	// watch for loading an actor more then once
	if(act->actor_type > 0 || *act->actor_name){
		char	str[256];
		char    name[256];

		safe_strncpy(name, get_string_property(cfg, "type"), sizeof(name));
		safe_snprintf(str, sizeof(str), "Data Error in %s(%d): Already loaded %s(%d)",
			name, act_idx, act->actor_name, act->actor_type
		);
		LOG_ERROR(str);
	}
	ok= 1;
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

	safe_snprintf (fname, sizeof(fname), "%s/%s", dir, index);

	doc = xmlReadFile (fname, NULL, XML_PARSE_NOENT);
	if (doc == NULL) {
		LOG_ERROR("Unable to read actor definition file %s", fname);
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
	return ok;
}

void init_actor_defs()
{
	// initialize the whole thing to zero
	memset (actors_defs, 0, sizeof (actors_defs));
	memset (attached_actors_defs, 0, sizeof (attached_actors_defs));
#ifdef	NEW_TEXTURES
	set_invert_v_coord();
#endif	/* NEW_TEXTURES */
	read_actor_defs ("actor_defs", "actor_defs.xml");
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
		if (actors_defs[i].hardware_model)
			clear_buffers(&actors_defs[i]);
		CalCoreModel_Delete(actors_defs[i].coremodel);
	}
}

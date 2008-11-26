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
#ifdef NEW_LIGHTING
 #include "textures.h"
#endif
#include "actor_init.h"

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
int actor_part_sizes[ACTOR_NUM_PARTS] = {10, 40, 50, 100, 100, 100, 10, 20, 40, 60};		// Elements according to actor_parts_enum
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
int cal_load_mesh (actor_types *act, const char *fn, const char *kind);
#ifdef NEW_SOUND
int parse_actor_sounds (actor_types *act, xmlNode *cfg);
#endif	//NEW_SOUND

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

void animate_actors()
{
	int i;
	static int last_update= 0;
    int time_diff = cur_time-last_update;
    int tmp_time_diff;

	// lock the actors_list so that nothing can interere with this look
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	for(i=0; i<max_actors; i++) {
		if(actors_list[i]) {
			if(actors_list[i]->moving) {
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
                actors_list[i]->movement_time_left -= time_diff;
				if(actors_list[i]->movement_time_left <= 0){	//we moved all the way
					Uint8 last_command;
                    int dx, dy;

					actors_list[i]->moving= 0;	//don't move next time, ok?
					//now, we need to update the x/y_tile_pos, and round off
					//the x/y_pos according to x/y_tile_pos
					last_command= actors_list[i]->last_command;
                    if (get_motion_vector(last_command, &dx, &dy)) {
						actors_list[i]->x_tile_pos += dx;
						actors_list[i]->y_tile_pos += dy;

#ifndef MINIMAP2
						// and update the minimap if we need to
						if(actors_list[i]->actor_id == yourself){
							update_exploration_map();
						}
						minimap_touch();
#endif //MINIMAP2
						actors_list[i]->busy = 0;
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
					}
				}
			}

			if(actors_list[i]->rotating) {
				actors_list[i]->rotate_time_left -= time_diff;
				if (actors_list[i]->rotate_time_left <= 0) { //we rotated all the way
					actors_list[i]->rotating= 0;//don't rotate next time, ok?
                    tmp_time_diff = time_diff + actors_list[i]->rotate_time_left;
                }
                else {
                    tmp_time_diff = time_diff;
                }
				actors_list[i]->x_rot+= actors_list[i]->rotate_x_speed*tmp_time_diff;
				actors_list[i]->y_rot+= actors_list[i]->rotate_y_speed*tmp_time_diff;
				actors_list[i]->z_rot+= actors_list[i]->rotate_z_speed*tmp_time_diff;
				if(actors_list[i]->z_rot >= 360) {
					actors_list[i]->z_rot -= 360;
				} else if (actors_list[i]->z_rot <= 0) {
					actors_list[i]->z_rot += 360;
				}
			}

			actors_list[i]->anim_time += ((cur_time-last_update)*actors_list[i]->cur_anim.duration_scale)/1000.0;
#ifndef	DYNAMIC_ANIMATIONS
			if (actors_list[i]->calmodel!=NULL){
				CalModel_Update(actors_list[i]->calmodel, (((cur_time-last_update)*actors_list[i]->cur_anim.duration_scale)/1000.0));
				build_actor_bounding_box(actors_list[i]);
				missiles_rotate_actor_bones(actors_list[i]);
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
				if ((actors_list[i]->stop_animation==1)&&(actors_list[i]->anim_time>=actors_list[i]->cur_anim.duration)){
					actors_list[i]->busy=0;
					if (actors_list[i]->in_aim_mode == 2) {
						int item;

						// we really leave the aim mode only when the animation is finished
						actors_list[i]->in_aim_mode = 0;
						missiles_log_message("%s (%d): leaving range mode finished!\n",
											 actors_list[i]->actor_name, actors_list[i]->actor_id);

						// then we do all the item changes that have been delayed
						for (item = 0; item < actors_list[i]->delayed_item_changes_count; ++item) {
							if (actors_list[i]->delayed_item_changes[item] < 0) {
								missiles_log_message("%s (%d): unwearing item type %d now\n",
													 actors_list[i]->actor_name,
													 actors_list[i]->actor_id,
													 actors_list[i]->delayed_item_type_changes[item]);
								unwear_item_from_actor(actors_list[i]->actor_id,
													   actors_list[i]->delayed_item_type_changes[item]);
							}
							else {
								missiles_log_message("%s (%d): wearing item type %d now\n",
													 actors_list[i]->actor_name,
													 actors_list[i]->actor_id,
													 actors_list[i]->delayed_item_type_changes[item]);
								actor_wear_item(actors_list[i]->actor_id,
												actors_list[i]->delayed_item_type_changes[item],
												actors_list[i]->delayed_item_changes[item]);
							}
						}
						actors_list[i]->delayed_item_changes_count = 0;
					}
				}
			}

#ifndef ATTACHED_ACTORS
			// Schmurk: all the following code is not used actually!
			if ((actors_list[i]->IsOnIdle)&&(actors_list[i]->anim_time>=5.0)&&(actors_list[i]->stop_animation!=1)) {
				cal_actor_set_random_idle(i);
			}
			else if(!actors_list[i]->IsOnIdle && actors_list[i]->stand_idle && actors_list[i]->anim_time>=5.0){
				// lets see if we want to change the idle animation
				// make sure we have at least two idles, and add a randomizer to continue
				if(actors_defs[actors_list[i]->actor_type].cal_frames[cal_actor_idle2_frame].anim_index != -1 && RAND(0,50) == 0){
					// pick what we want the next idle to be
					// 75% chance to do idle1
					if(RAND(0, 3) == 0){
						// and check to see if we are changing the animation or not
						if(actors_list[i]->cur_anim.anim_index != actors_defs[actors_list[i]->actor_type].cal_frames[cal_actor_idle2_frame].anim_index){
							cal_actor_set_anim_delay(i, actors_defs[actors_list[i]->actor_type].cal_frames[cal_actor_idle2_frame], 0.5f); // normal idle
						}
					} else {
						// and check to see if we are changing the animation or not
						if(actors_list[i]->cur_anim.anim_index != actors_defs[actors_list[i]->actor_type].cal_frames[cal_actor_idle1_frame].anim_index){
							cal_actor_set_anim_delay(i, actors_defs[actors_list[i]->actor_type].cal_frames[cal_actor_idle1_frame], 0.5f); // normal idle
						}
					}
				}
			}
#else // ATTACHED_ACTORS
			// we change the idle animation only when the previous one is finished
			if (actors_list[i]->stand_idle && actors_list[i]->anim_time >= actors_list[i]->cur_anim.duration - 0.2)
			{
				if (!is_actor_held(actors_list[i]))
				{
					// 1% chance to do idle2
					if (actors_defs[actors_list[i]->actor_type].cal_frames[cal_actor_idle2_frame].anim_index != -1 && RAND(0, 99) == 0)
						cal_actor_set_anim(i, actors_defs[actors_list[i]->actor_type].cal_frames[cal_actor_idle2_frame]); // normal idle
					else
						cal_actor_set_anim(i, actors_defs[actors_list[i]->actor_type].cal_frames[cal_actor_idle1_frame]); // normal idle
				}
			}
#endif // ATTACHED_ACTORS

			if (actors_list[i]->cur_anim.anim_index==-1) actors_list[i]->busy=0;

			//first thing, decrease the damage time, so we will see the damage splash only for 2 seconds
			if(actors_list[i]->damage_ms) {
				actors_list[i]->damage_ms-=80;
				if(actors_list[i]->damage_ms<0)actors_list[i]->damage_ms=0;
			}

			//9 frames, not moving, and another command is queued farther on (based on how long we've done this action)
			if(!actors_list[i]->moving && !actors_list[i]->rotating){
				/*
					actors_list[i]->stop_animation=1;	//force stopping, not looping
					actors_list[i]->busy=0;	//ok, take the next command
					LOG_TO_CONSOLE(c_green2,"FREE");
					//Idle here?
				*/
			}

			if(actors_list[i]->stop_animation) {

				//we are done with this guy
				//Should we go into idle here?
			}
		}
	}
	UNLOCK_ACTORS_LISTS();
}

void set_on_idle(int actor_idx)
{
    actor *a = actors_list[actor_idx];
    if(!a->dead) {
        a->stop_animation=0;

        if(a->fighting){
            cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].cal_frames[cal_actor_combat_idle_frame]);
        }
        else if (a->in_aim_mode == 1) {
            cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].weapon[a->cur_weapon].cal_frames[cal_weapon_range_idle_frame]);
        }
        else if(!a->sitting) {
            // we are standing, see if we can activate a stand idle
            if(!a->stand_idle){
                if (actors_defs[a->actor_type].group_count == 0)
                {
#ifdef ATTACHED_ACTORS
					attachment_props *att_props = get_attachment_props_if_held(a);
					if (att_props)
						cal_actor_set_anim(actor_idx, att_props->cal_frames[cal_attached_idle_frame]);
					else
#endif // ATTACHED_ACTORS
                    // 75% chance to do idle1
                    if (actors_defs[a->actor_type].cal_frames[cal_actor_idle2_frame].anim_index != -1 && RAND(0, 3) == 0){
                        cal_actor_set_anim(actor_idx, actors_defs[a->actor_type].cal_frames[cal_actor_idle2_frame]); // normal idle
                    } else {
                        cal_actor_set_anim(actor_idx, actors_defs[a->actor_type].cal_frames[cal_actor_idle1_frame]); // normal idle
                    }
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
            if(!a->sit_idle) {
                cal_actor_set_anim(actor_idx,actors_defs[a->actor_type].cal_frames[cal_actor_idle_sit_frame]);
                a->sit_idle=1;
            }
        }
    }
}

#ifdef EMOTES
void handle_emote_command(int a, actor *act, emote_commands command)
{
	int k;
	int max_queue = 0;

	// Set the anim
	switch (command)
	{
		case wave:
			LOG_TO_CONSOLE(c_green2, "Wave emote command");
			cal_actor_set_anim(a, actors_defs[act->actor_type].cal_frames[cal_actor_emote_wave_frame]);
			break;
		case nod_head:
			LOG_TO_CONSOLE(c_green2, "Nod head emote command");
			cal_actor_set_anim(a, actors_defs[act->actor_type].cal_frames[cal_actor_emote_nod_head_frame]);
			break;
		case shake_head:
			LOG_TO_CONSOLE(c_green2, "Shake head emote command");
			cal_actor_set_anim(a, actors_defs[act->actor_type].cal_frames[cal_actor_emote_shake_head_frame]);
			break;
		case clap_hands:
			LOG_TO_CONSOLE(c_green2, "Clap hands emote command");
			cal_actor_set_anim(a, actors_defs[act->actor_type].cal_frames[cal_actor_emote_clap_hands_frame]);
			break;
		case shrug:
			LOG_TO_CONSOLE(c_green2, "Shrug emote command");
			cal_actor_set_anim(a, actors_defs[act->actor_type].cal_frames[cal_actor_emote_shrug_frame]);
			break;
		case scratch_head:
			LOG_TO_CONSOLE(c_green2, "Scratch head emote command");
			cal_actor_set_anim(a, actors_defs[act->actor_type].cal_frames[cal_actor_emote_scratch_head_frame]);
			break;
		case jump:
			LOG_TO_CONSOLE(c_green2, "Jump emote command");
			cal_actor_set_anim(a, actors_defs[act->actor_type].cal_frames[cal_actor_emote_jump_frame]);
			break;
		case stretch:
			LOG_TO_CONSOLE(c_green2, "Stretch emote command");
			cal_actor_set_anim(a, actors_defs[act->actor_type].cal_frames[cal_actor_emote_stretch_frame]);
			break;
		case bow:
			LOG_TO_CONSOLE(c_green2, "Bow emote command");
			cal_actor_set_anim(a, actors_defs[act->actor_type].cal_frames[cal_actor_emote_bow_frame]);
			break;
		default:
			LOG_ERROR("Unknown emote command: %d", command);
	}
	act->stop_animation = 1;

	// Move que down one command
	for (k = 0; k < MAX_EMOTE_QUEUE - 1; k++) {
		if (k > max_queue && act->emote_que[k] != emote_nothing)
			max_queue = k;
		act->emote_que[k] = act->emote_que[k+1];
	}
	act->emote_que[k] = emote_nothing;
}
#endif // EMOTES

//in case the actor is not busy, and has commands in it's que, execute them
void next_command()
{
	int i, index;
	int max_queue=0;

	for(i=0;i<max_actors;i++){
		if(!actors_list[i])continue;//actor exists?
		if(!actors_list[i]->busy){//Are we busy?
			if(actors_list[i]->que[0]==nothing){//Is the queue empty?
#ifdef EMOTES
				// If the que is empty, check for an emote to display
				if (actors_list[i]->emote_que[0] != emote_nothing)
					handle_emote_command(i, actors_list[i], actors_list[i]->emote_que[0]);
				else
#endif // EMOTES
					//if que is empty, set on idle
	                set_on_idle(i);
				actors_list[i]->last_command=nothing;//prevents us from not updating the walk/run animation
			} else {
				int actor_type;
				int last_command=actors_list[i]->last_command;
				float z_rot=actors_list[i]->z_rot;
				float targeted_z_rot;
				int k;
				int no_action = 0;

				actors_list[i]->sit_idle=0;
				actors_list[i]->stand_idle=0;

				actor_type=actors_list[i]->actor_type;

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
					case pain1: {
#ifdef ATTACHED_ACTORS
						attachment_props *att_props = get_attachment_props_if_held(actors_list[i]);
						if (att_props)
							cal_actor_set_anim(i, att_props->cal_frames[cal_attached_pain_frame]);
						else
#endif // ATTACHED_ACTORS
							cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_pain1_frame]);
						actors_list[i]->stop_animation=1;
						break;
					}
					case pain2: {
#ifdef ATTACHED_ACTORS
						attachment_props *att_props = get_attachment_props_if_held(actors_list[i]);
						if (att_props)
							cal_actor_set_anim(i, att_props->cal_frames[cal_attached_pain_frame]);
						else
#endif // ATTACHED_ACTORS
							cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_pain2_frame]);
						actors_list[i]->stop_animation=1;
						break;
					}
					case pick:
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
							you_sit_down();
						break;
					case stand_up:
						//LOG_TO_CONSOLE(c_green2,"stand_up");
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_stand_up_frame]);
						actors_list[i]->stop_animation=1;
						actors_list[i]->sitting=0;
						if(actors_list[i]->actor_id==yourself)
							you_stand_up();
						break;
					case enter_combat:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_in_combat_frame]);
						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;
						//if (actors_list[i]->actor_id==yourself) LOG_TO_CONSOLE(c_green2,"Enter Combat");
						break;
					case leave_combat:
						cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[cal_actor_out_combat_frame]);
						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=0;
						break;
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
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[index]);
						} else {
							index += 18;
							cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[index]);
						}
						actors_list[i]->stop_animation=1;
						actors_list[i]->fighting=1;

#ifdef NEW_SOUND
						// Maybe play a battlecry sound
						add_battlecry_sound(actors_list[i]);
#endif // NEW_SOUND
						break;
					case turn_left:
						//LOG_TO_CONSOLE(c_green2,"turn left");
						actors_list[i]->rotate_z_speed=45.0/540.0;
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
#ifdef ATTACHED_ACTORS
							attachment_props *att_props = get_attachment_props_if_held(actors_list[i]);
							if (att_props)
								cal_actor_set_anim(i, att_props->cal_frames[get_held_actor_motion_frame(actors_list[i])]);
							else
#endif // ATTACHED_ACTORS
							cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[get_actor_motion_frame(actors_list[i])]);
						}
						actors_list[i]->stop_animation=0;
						break;
					case turn_right:
					//LOG_TO_CONSOLE(c_green2,"turn right");
						actors_list[i]->rotate_z_speed=-45.0/540.0;
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
#ifdef ATTACHED_ACTORS
							attachment_props *att_props = get_attachment_props_if_held(actors_list[i]);
							if (att_props)
								cal_actor_set_anim(i, att_props->cal_frames[get_held_actor_motion_frame(actors_list[i])]);
							else
#endif // ATTACHED_ACTORS
							cal_actor_set_anim(i,actors_defs[actor_type].cal_frames[get_actor_motion_frame(actors_list[i])]);
						}
						actors_list[i]->stop_animation=0;
						break;

				case enter_aim_mode:
					missiles_log_message("%s (%d): cleaning the queue from enter_aim_mode command",
										 actors_list[i]->actor_name, actors_list[i]->actor_id);
					missiles_clean_range_actions_queue(actors_list[i]);

					if (actors_list[i]->in_aim_mode == 0) {
						missiles_log_message("%s (%d): enter in aim mode", actors_list[i]->actor_name, actors_list[i]->actor_id);
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

						missiles_log_message("%s (%d): aiming again (time=%d)", actors_list[i]->actor_name, actors_list[i]->actor_id, cur_time);
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

						range_rotation = missiles_compute_actor_rotation(&actors_list[i]->cal_h_rot_end,
																		 &actors_list[i]->cal_v_rot_end,
																		 actors_list[i], action->aim_position);
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
						}
					}
					break;

				case leave_aim_mode:
					if (actors_list[i]->in_aim_mode != 1) {
						if (actors_list[i]->cal_rotation_blend < 0.0 ||
							(actors_list[i]->cal_h_rot_end == 0.0 &&
							 actors_list[i]->cal_v_rot_end == 0.0)) {
							log_error("next_command: trying to leave range mode while we are not in it => aborting safely...");
							no_action = 1;
							break;
						}
						else {
							log_error("next_command: trying to leave range mode while we are not in it => continuing because of a wrong actor bones rotation!");
						}
					}

					missiles_log_message("%s (%d): leaving aim mode", actors_list[i]->actor_name, actors_list[i]->actor_id);
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
							log_error("next_command: trying to fire an arrow out of range mode => aborting!");
							no_action = 1;
							missiles_log_message("%s (%d): cleaning the queue from aim_mode_fire command (error)",
												 actors_list[i]->actor_name, actors_list[i]->actor_id);
							missiles_clean_range_actions_queue(actors_list[i]);
							break;
						}

						if (action->reload) {
							missiles_log_message("%s (%d): fire and reload", actors_list[i]->actor_name, actors_list[i]->actor_id);
							// launch fire and reload animation
							cal_actor_set_anim(i,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].cal_frames[cal_weapon_range_fire_frame]);
							actors_list[i]->in_aim_mode = 1;
						}
						else {
							missiles_log_message("%s (%d): fire and leave aim mode", actors_list[i]->actor_name, actors_list[i]->actor_id);
							// launch fire and leave aim mode animation
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
							action->fire_position[2] = height_map[tile_y*tile_map_size_x*6+tile_x]*0.2-2.2;
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

					//ok, now the movement, this is the tricky part
					default:
						if(actors_list[i]->que[0]>=move_n && actors_list[i]->que[0]<=move_nw) {
							float rotation_angle;
							int dx, dy;
#ifdef VARIABLE_SPEED
							int step_duration = actors_list[i]->step_duration;
#else // VARIABLE_SPEED
							int step_duration = DEFAULT_STEP_DURATION;
#endif // VARIABLE_SPEED
							struct cal_anim *walk_anim;
#ifdef ATTACHED_ACTORS
							attachment_props *att_props = get_attachment_props_if_held(actors_list[i]);
							if (att_props)
								walk_anim = &att_props->cal_frames[get_held_actor_motion_frame(actors_list[i])];
							else
#endif // ATTACHED_ACTORS
							walk_anim = &actors_defs[actor_type].cal_frames[get_actor_motion_frame(actors_list[i])];

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
							actors_list[i]->move_z_speed = -2.2 + height_map[(actors_list[i]->y_tile_pos+dy)*tile_map_size_x*6+actors_list[i]->x_tile_pos+dx]*0.2 - actors_list[i]->z_pos;
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

							targeted_z_rot=(actors_list[i]->que[0]-turn_n)*45.0f;
							rotation_angle=get_rotation_vector(z_rot,targeted_z_rot);
							actors_list[i]->rotate_z_speed=rotation_angle/360.0f;
							actors_list[i]->rotate_time_left=360;
							actors_list[i]->rotating=1;
							actors_list[i]->stop_animation=1;
							missiles_log_message("%s (%d): rotation %d requested", actors_list[i]->actor_name, actors_list[i]->actor_id, actors_list[i]->que[0] - turn_n);
						}
					}

					//mark the actor as being busy
					if (!no_action)
						actors_list[i]->busy=1;
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
						continue;
					}

					//move que down with one command
					for(k=0;k<MAX_CMD_QUEUE-1;k++) {
						if(k>max_queue && actors_list[i]->que[k]!=nothing)max_queue=k;
						actors_list[i]->que[k]=actors_list[i]->que[k+1];
					}
					actors_list[i]->que[k]=nothing;
				}
			}
		}
}

void free_actor_data(int actor_index)
{
	actor *act = actors_list[actor_index];
    if(act->calmodel!=NULL)
        model_delete(act->calmodel);
    if(act->remapped_colors) glDeleteTextures(1,&act->texture_id);
    if(act->is_enhanced_model){
        glDeleteTextures(1,&act->texture_id);
        if(act->body_parts)free(act->body_parts);
    }
#ifdef NEW_SOUND
    stop_sound(act->cur_anim_sound_cookie);
    act->cur_anim_sound_cookie = 0;
#endif	//NEW_SOUND
    ec_actor_delete(act);
}

void destroy_actor(int actor_id)
{
	int i;
#ifdef ATTACHED_ACTORS
    int attached_actor = -1;
#endif // ATTACHED_ACTORS

#ifdef EXTRA_DEBUG
	ERR();
#endif
	for(i=0;i<max_actors;i++){
		if(actors_list[i])//The timer thread doesn't free memory
			if(actors_list[i]->actor_id==actor_id){
				LOCK_ACTORS_LISTS();
#ifdef ATTACHED_ACTORS
                attached_actor = actors_list[i]->attached_actor;
#endif // ATTACHED_ACTORS

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
#ifdef ATTACHED_ACTORS
                    if (attached_actor == max_actors) attached_actor = i;
					if (actors_list[i] && actors_list[i]->attached_actor >= 0)
						actors_list[actors_list[i]->attached_actor]->attached_actor = i;
#endif // ATTACHED_ACTORS
				}

#ifdef ATTACHED_ACTORS
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
#endif // ATTACHED_ACTORS

				actor_under_mouse = NULL;
				UNLOCK_ACTORS_LISTS();
				break;
			}
	}
#ifndef MINIMAP2
	minimap_touch();
#endif //MINIMAP2
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
#ifndef MINIMAP2
	minimap_touch();
#endif //MINIMAP2
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

#ifdef ATTACHED_ACTORS
int push_command_in_actor_queue(unsigned char command, actor *act)
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
#endif // ATTACHED_ACTORS

void add_command_to_actor(int actor_id, unsigned char command)
{
	//int i=0;
	int k=0;
#ifdef ATTACHED_ACTORS
	int k2 = 0;
#endif // ATTACHED_ACTORS
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

#ifndef ATTACHED_ACTORS
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
#else // ATTACHED_ACTORS
		k = push_command_in_actor_queue(command, act);
		if (act->attached_actor >= 0)
			k2 = push_command_in_actor_queue(command, actors_list[act->attached_actor]);
		else
			k2 = k;
#endif // ATTACHED_ACTORS

		{
			actor * me = get_our_actor();
			if (me!=NULL)
				isme = act->actor_id == me->actor_id;
		}

		switch(command) {
		case enter_combat:
			act->async_fighting= 1;
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

#ifdef ATTACHED_ACTORS
		if (k != k2) {
			LOG_ERROR("Inconsistency between queues of attached actors %s (%d) and %s (%d)!",
					  act->actor_name,
					  act->actor_id,
					  actors_list[act->attached_actor]->actor_name,
					  actors_list[act->attached_actor]->actor_id);
		}
		else
#endif // ATTACHED_ACTORS
		if(k>MAX_CMD_QUEUE-2){
			int i;
			LOG_ERROR("Too much commands in the queue for actor %d (%s) => resync!",
					  act->actor_id, act->actor_name);
			for (i = 0; i < MAX_CMD_QUEUE; ++i)
				LOG_ERROR("%dth command in the queue: %d", i, (int)act->que[i]);
			update_all_actors();
		}
	}
}

#ifdef EMOTES
void add_emote_command_to_actor(actor * act, unsigned char command)
{
	int k = 0;
#ifdef EXTRA_DEBUG
	ERR();
#endif

	if (!act) {
		LOG_ERROR("%s (Emote) %d - NULL actor passed", cant_add_command, command);
	} else {

		for (k = 0; k < MAX_EMOTE_QUEUE; k++) {
			if (act->emote_que[k] == emote_nothing) {
				if (k <= MAX_EMOTE_QUEUE - 2) {
					act->emote_que[k] = command;
				}
				break;
			}
		}

		if (k > MAX_EMOTE_QUEUE - 2) {
			int i;
			LOG_ERROR("Too many commands in the emote queue for actor %d (%s)", act->actor_id, act->actor_name);
			for (i = 0; i < MAX_EMOTE_QUEUE; ++i)
				LOG_ERROR("%dth command in the emote queue: %d", i, (int)act->emote_que[i]);
		}
	}
}

// TODO: Link this function in somewhere to clear an actors emote que if it won't play
// (due to the actor not being idle any time soon)
void clear_emote_que(actor * act)
{
	int k;
	// Move que down one command
	for (k = 0; k < MAX_EMOTE_QUEUE; k++) {
		act->emote_que[k] = emote_nothing;
	}
}
#endif // EMOTES


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

#ifdef EMOTES
int parse_emote_def(emote_types *emote, xmlNode *node)
{
	xmlNode *item;
	int ok;

	if (node == NULL || node->children == NULL) return 0;

	emote->id = get_int_property(node, "id");
	if (emote->id < 0) {
		LOG_ERROR("Unable to find id property %s\n", node->name);
		return 0;
	}

	ok = 1;
	for (item = node->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"command") == 0) {
				get_string_value (emote->command, sizeof(emote->command), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"actor_type") == 0) {
				emote->actor_type = get_int_value(item);
			} else {
				LOG_ERROR("unknown emote property \"%s\"", item->name);
				ok = 0;
			}
		}
	}

	return ok;
}

int parse_emotes_defs(xmlNode *node)
{
	xmlNode *def;
	emote_types *emote = NULL;
	int ok = 1;

	for (def = node->children; def; def = def->next) {
		if (def->type == XML_ELEMENT_NODE)
			if (xmlStrcasecmp(def->name, (xmlChar*)"emote") == 0) {
				// Init the memory for emotes
				if (!emotes) {
					emote = (emote_types*)calloc(1, sizeof(emote_types));
					emotes = emote;
				} else {
					emote->next = (emote_types*)calloc(1, sizeof(emote_types));
					emote = emote->next;
				}
				ok &= parse_emote_def(emote, def);
			} else {
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
	xmlNode *root;
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
	emote_types *emote = NULL;
	emote_types *next = NULL;

	next = emote = emotes;
	while (next)
	{
		next = emote->next;
		free(emote);
		emote = next;
	}
}
#endif // EMOTES

xmlNode *get_default_node(xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
	char *group;

	// first, check for errors
	if(defaults == NULL || cfg == NULL){
        return NULL;
	}

	//lets find out what group to look for
	group = get_string_property(cfg, "group");

	// look for defaul entries with the same name
	for(item=defaults->children; item; item=item->next){
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, cfg->name) == 0){
				char *item_group;

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

int parse_actor_shirt (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
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
		xmlNode *default_node= get_default_node(cfg, defaults);

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

#if NEW_LIGHTING
	set_shirt_metadata(shirt);
#endif
	return ok;
}

int parse_actor_skin (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
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
		xmlNode *default_node= get_default_node(cfg, defaults);

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

#if NEW_LIGHTING
	set_skin_metadata(skin);
#endif

	return ok;
}

int parse_actor_legs (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
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
		xmlNode *default_node= get_default_node(cfg, defaults);

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

#if NEW_LIGHTING
	set_legs_metadata(legs);
#endif

	return ok;
}

int parse_actor_weapon_detail (actor_types *act, weapon_part *weapon, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
	char str[255];
	int ok, index;

	if (cfg == NULL || cfg->children == NULL) return 0;

	ok = 1;
	for (item = cfg->children; item; item = item->next) {
		if (item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp (item->name, (xmlChar*)"mesh") == 0) {
				get_string_value (weapon->model_name, sizeof (weapon->model_name), item);
				weapon->mesh_index = cal_load_weapon_mesh (act, weapon->model_name, "weapon");
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"skin") == 0) {
				get_string_value (weapon->skin_name, sizeof (weapon->skin_name), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"skinmask") == 0) {
				get_string_value (weapon->skin_mask, sizeof (weapon->skin_mask), item);
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"glow") == 0) {
				int mode = find_description_index (glow_mode_dict, (char*)item->children->content, "glow mode");
				if (mode < 0) mode = GLOW_NONE;
				weapon->glow = mode;
#ifdef NEW_SOUND
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up1") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_1_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up2") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_2_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up3") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_3_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up4") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_4_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up5") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_5_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up6") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_6_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up7") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_7_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up8") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_8_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up9") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_9_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_up10") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_up_10_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down1") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_1_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down2") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_2_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down3") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_3_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down4") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_4_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down5") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_5_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down6") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_6_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down7") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_7_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down8") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_8_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down9") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_9_frame], str, get_string_property(item, "sound_scale"));
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"snd_attack_down10") == 0) {
				get_string_value (str,sizeof(str),item);
     			cal_set_anim_sound(&weapon->cal_frames[cal_weapon_attack_down_10_frame], str, get_string_property(item, "sound_scale"));
#endif	//NEW_SOUND
			} else {
				index = -1;
				if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up1") == 0) {
					index = cal_weapon_attack_up_1_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up2") == 0) {
					index = cal_weapon_attack_up_2_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up3") == 0) {
					index = cal_weapon_attack_up_3_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up4") == 0) {
					index = cal_weapon_attack_up_4_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up5") == 0) {
					index = cal_weapon_attack_up_5_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up6") == 0) {
					index = cal_weapon_attack_up_6_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up7") == 0) {
					index = cal_weapon_attack_up_7_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up8") == 0) {
					index = cal_weapon_attack_up_8_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up9") == 0) {
					index = cal_weapon_attack_up_9_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_up10") == 0) {
					index = cal_weapon_attack_up_10_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down1") == 0) {
					index = cal_weapon_attack_down_1_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down2") == 0) {
					index = cal_weapon_attack_down_2_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down3") == 0) {
					index = cal_weapon_attack_down_3_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down4") == 0) {
					index = cal_weapon_attack_down_4_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down5") == 0) {
					index = cal_weapon_attack_down_5_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down6") == 0) {
					index = cal_weapon_attack_down_6_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down7") == 0) {
					index = cal_weapon_attack_down_7_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down8") == 0) {
					index = cal_weapon_attack_down_8_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down9") == 0) {
					index = cal_weapon_attack_down_9_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_attack_down10") == 0) {
					index = cal_weapon_attack_down_10_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_range_fire") == 0) {
					index = cal_weapon_range_fire_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_range_fire_out") == 0) {
					index = cal_weapon_range_fire_out_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_range_idle") == 0) {
					index = cal_weapon_range_idle_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_range_in") == 0) {
					index = cal_weapon_range_in_frame;
				} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_range_out") == 0) {
					index = cal_weapon_range_out_frame;
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
		} else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= parse_actor_weapon_detail (act, weapon, item->children, defaults);
		}
	}


	return ok;
}

int parse_actor_weapon (actor_types *act, xmlNode *cfg, xmlNode *defaults)
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

	// check for default entries, if found, use them to fill in missing data
	if(defaults){
		xmlNode *default_node= get_default_node(cfg, defaults);

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

#if NEW_LIGHTING
	set_weapon_metadata(weapon);
#endif

	return ok;
}

int parse_actor_body_part (actor_types *act, body_part *part, xmlNode *cfg, const char *part_name, xmlNode *default_node)
{
	xmlNode *item;
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

#if NEW_LIGHTING
	set_part_metadata(part);
#endif

	return ok;
}

int parse_actor_helmet (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *default_node= get_default_node(cfg, defaults);
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

#if NEW_LIGHTING
	set_part_metadata(helmet);
#endif

	return parse_actor_body_part(act,helmet, cfg->children, "helmet", default_node);
}

#ifdef NEW_SOUND
int parse_actor_sounds (actor_types *act, xmlNode *cfg)
{
	xmlNode *item;
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

int parse_actor_cape (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *default_node= get_default_node(cfg, defaults);
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

#if NEW_LIGHTING
	set_part_metadata(cape);
#endif

	return parse_actor_body_part(act,cape, cfg->children, "cape", default_node);
}

int parse_actor_head (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *default_node= get_default_node(cfg, defaults);
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


#if NEW_LIGHTING
	set_part_metadata(head);
#endif

	return parse_actor_body_part(act, head, cfg->children, "head", default_node);
}

int parse_actor_shield_part (actor_types *act, shield_part *part, xmlNode *cfg, xmlNode *default_node)
{
	xmlNode *item;
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

#if NEW_LIGHTING
	set_part_metadata(part);
#endif

	return ok;
}

int parse_actor_shield (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *default_node= get_default_node(cfg, defaults);
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

#if NEW_LIGHTING
	set_part_metadata(shield);
#endif

	return parse_actor_shield_part(act, shield, cfg->children, default_node);
}

int parse_actor_hair (actor_types *act, xmlNode *cfg, xmlNode *defaults)
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

#if NEW_LIGHTING
	set_hair_metadata(act->hair);
#endif

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
		log_error("Cal3d error: %s: %s\n", str, CalError_GetLastErrorDescription());
		return res;
	}
	coreanim=CalCoreModel_GetCoreAnimation(act->coremodel,res.anim_index);

	if (coreanim) {
		res.duration=CalCoreAnimation_GetDuration(coreanim);
	} else {
		log_error("No Anim: %s\n",str);
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

int parse_actor_frames (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
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
#ifdef EMOTES
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_emote_wave") == 0) {
				index = cal_actor_emote_wave_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_emote_nod_head") == 0) {
				index = cal_actor_emote_nod_head_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_emote_shake_head") == 0) {
				index = cal_actor_emote_shake_head_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_emote_clap_hands") == 0) {
				index = cal_actor_emote_clap_hands_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_emote_shrug") == 0) {
				index = cal_actor_emote_shrug_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_emote_scratch_head") == 0) {
				index = cal_actor_emote_scratch_head_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_emote_jump") == 0) {
				index = cal_actor_emote_jump_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_emote_stretch") == 0) {
				index = cal_actor_emote_stretch_frame;
			} else if (xmlStrcasecmp (item->name, (xmlChar*)"CAL_emote_bow") == 0) {
				index = cal_actor_emote_bow_frame;
#endif // EMOTES
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

#ifdef ATTACHED_ACTORS
int parse_actor_attachment (actor_types *act, xmlNode *cfg, int actor_type)
{
	xmlNode *item;
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
#endif // ATTACHED_ACTORS

int parse_actor_boots (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode *item;
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
		xmlNode *default_node= get_default_node(cfg, defaults);

		if(default_node){
			if(boots->boots_name==NULL || *boots->boots_name=='\0')
				get_item_string_value(boots->boots_name, sizeof(boots->boots_name), default_node, (xmlChar*)"skin");
		}
	}

	// check the critical information
	actor_check_string(act, "boots", "boots", boots->boots_name);

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
int cal_load_mesh (actor_types *act, const char *fn, const char *kind)
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
		log_error("Cal3d error: %s: %s\n", fn, CalError_GetLastErrorDescription());
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
		log_error("Cal3d error: %s: %s\n", fn, CalError_GetLastErrorDescription());
	}

	return res;
}

int	parse_actor_nodes (actor_types *act, xmlNode *cfg, xmlNode *defaults)
{
	xmlNode	*item;
	int	ok= 1;

	for(item=cfg->children; item; item=item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, (xmlChar*)"ghost") == 0) {
				act->ghost= get_bool_value (item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skin") == 0) {
				get_string_value(act->skin_name, sizeof (act->skin_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"mesh") == 0) {
				get_string_value(act->file_name, sizeof (act->file_name), item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"actor_scale")==0) {
				act->actor_scale= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"scale")==0) {
				act->scale= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"mesh_scale")==0) {
				act->mesh_scale= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"bone_scale")==0) {
				act->skel_scale= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"skeleton")==0) {
				char skeleton_name[MAX_FILE_PATH];
				get_string_value(skeleton_name, sizeof(skeleton_name), item);
				act->coremodel= CalCoreModel_New("Model");
				if(!CalCoreModel_ELLoadCoreSkeleton(act->coremodel, skeleton_name)) {
					log_error("Cal3d error: %s: %s\n", skeleton_name, CalError_GetLastErrorDescription());
					act->skeleton_type = -1;
				}
				else {
					act->skeleton_type = get_skeleton(act->coremodel, skeleton_name);
				}
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"walk_speed") == 0) { // unused
				act->walk_speed= get_float_value(item);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"run_speed") == 0) { // unused
				act->run_speed= get_float_value(item);
#ifdef VARIABLE_SPEED
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"step_duration") == 0) {
				act->step_duration = get_int_value(item);
#endif // VARIABLE_SPEED
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"defaults") == 0) {
				defaults= item;
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"frames") == 0) {
				ok &= parse_actor_frames(act, item->children, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"shirt") == 0) {
				ok &= parse_actor_shirt(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"hskin") == 0) {
				ok &= parse_actor_skin(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"hair") == 0) {
				ok &= parse_actor_hair(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"boots") == 0) {
				ok &= parse_actor_boots(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"legs") == 0) {
				ok &= parse_actor_legs(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"cape") == 0) {
				ok &= parse_actor_cape(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"head") == 0) {
				ok &= parse_actor_head(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"shield") == 0) {
				ok &= parse_actor_shield(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"weapon") == 0) {
				ok &= parse_actor_weapon(act, item, defaults);
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"helmet") == 0) {
				ok &= parse_actor_helmet(act, item, defaults);
#ifdef NEW_SOUND
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"sounds") == 0) {
				ok &= parse_actor_sounds(act, item->children);
#endif	//NEW_SOUND
#ifdef ATTACHED_ACTORS
			} else if(xmlStrcasecmp(item->name, (xmlChar*)"actor_attachment") == 0) {
				int id = get_int_property(item, "id");
				if (id < 0 || id >= MAX_ACTOR_DEFS) {
					LOG_ERROR("Unable to find id/property node %s\n", item->name);
					ok = 0;
				}
				else
					ok &= parse_actor_attachment(act, item, id);
#endif // ATTACHED_ACTORS
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

int parse_actor_script (xmlNode *cfg)
{
	int ok, act_idx, i;
#ifdef ATTACHED_ACTORS
	int j;
#endif // ATTACHED_ACTORS
	actor_types *act;
	struct CalCoreSkeleton *skel;

	if(cfg == NULL || cfg->children == NULL) return 0;

	act_idx= get_int_property(cfg, "id");
/*	if(act_idx < 0){
		act_idx= get_property(cfg, "type", "actor type", actor_type_dict);
	}
*/
	if(act_idx < 0 || act_idx >= MAX_ACTOR_DEFS){
		char	str[256];
		char    name[256];

		safe_strncpy(name, get_string_property(cfg, "type"), sizeof(name));
		safe_snprintf(str, sizeof(str), "Data Error in %s(%d): Actor ID out of range %d",
			name, act_idx, act_idx
		);
		log_error(str);
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
		log_error(str);
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

#ifdef ATTACHED_ACTORS
	for (i = 0; i < MAX_ACTOR_DEFS; ++i)
	{
		for (j = 0; j < NUM_ATTACHED_ACTOR_FRAMES; j++) {
			attached_actors_defs[act_idx].actor_type[i].cal_frames[j].anim_index = -1;
#ifdef NEW_SOUND
			attached_actors_defs[act_idx].actor_type[i].cal_frames[j].sound = -1;
#endif // NEW_SOUND
		}
	}
#endif // ATTACHED_ACTORS

#ifdef VARIABLE_SPEED
    act->step_duration = DEFAULT_STEP_DURATION; // default value
#endif // VARIABLE_SPEED

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
#ifdef NEW_LIGHTING
			strncpy(act->shirt[0].model_name, act->file_name, sizeof(act->shirt[0].model_name));
#endif
			act->shirt[0].mesh_index= cal_load_mesh(act, act->file_name, NULL); //save the single meshindex as torso
#ifdef NEW_LIGHTING
			set_shirt_metadata(&act->shirt[0]);
#endif
		}
		if (use_animation_program)
		{
			build_buffers(act);
		}
	}

	return ok;
}

int parse_actor_defs (xmlNode *node)
{
	xmlNode *def;
	int ok = 1;

	for (def = node->children; def; def = def->next) {
		if (def->type == XML_ELEMENT_NODE)
			if (xmlStrcasecmp (def->name, (xmlChar*)"actor") == 0) {
				ok &= parse_actor_script (def);
			} else {
				LOG_ERROR("parse error: actor or include expected");
				ok = 0;
			}
		else if (def->type == XML_ENTITY_REF_NODE) {
			ok &= parse_actor_defs (def->children);
		}
	}

	return ok;
}

#ifdef EXT_ACTOR_DICT
int parse_skin_colours(xmlNode *node)
{
	xmlNode *data;
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

int parse_glow_modes(xmlNode *node)
{
	xmlNode *data;
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

int parse_head_numbers(xmlNode *node)
{
	xmlNode *data;
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

int parse_actor_dict(xmlNode *node)
{
	xmlNode *data;
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

int parse_actor_part_sizes(xmlNode *node)
{
	xmlNode *data;
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

int parse_actor_data(xmlNode *node)
{
	xmlNode *data;
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
	xmlNode *root;
	xmlDoc *doc;
	char fname[120];
	int ok = 1;

	safe_snprintf (fname, sizeof(fname), "%s/%s", dir, index);

	doc = xmlReadFile (fname, NULL, 0);
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
#ifdef ATTACHED_ACTORS
	memset (attached_actors_defs, 0, sizeof (attached_actors_defs));
#endif // ATTACHED_ACTORS

	read_actor_defs ("actor_defs", "actor_defs.xml");
}

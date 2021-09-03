#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "new_actors.h"
#include "actors_list.h"
#include "actor_scripts.h"
#include "asc.h"
#include "bbox_tree.h"
#include "buffs.h"
#include "cal.h"
#include "console.h"
#include "dialogues.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "errors.h"
#include "filter.h"
#include "missiles.h"
#include "sound.h"
#include "textures.h"
#include "tiles.h"
#include "translate.h"
#ifdef CLUSTER_INSIDES
#include "cluster.h"
#endif
#include "eye_candy_wrapper.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "io/elfilewrapper.h"
#include "actor_init.h"

float sitting=1.0f;
glow_color glow_colors[10];

//build the glow color table
void build_glow_color_table()
{
	glow_colors[GLOW_NONE].r=0;
	glow_colors[GLOW_NONE].g=0;
	glow_colors[GLOW_NONE].b=0;

	glow_colors[GLOW_FIRE].r=0.5f;
	glow_colors[GLOW_FIRE].g=0.1f;
	glow_colors[GLOW_FIRE].b=0.1f;

	glow_colors[GLOW_COLD].r=0.1f;
	glow_colors[GLOW_COLD].g=0.1f;
	glow_colors[GLOW_COLD].b=0.5f;

	glow_colors[GLOW_THERMAL].r=0.5f;
	glow_colors[GLOW_THERMAL].g=0.1f;
	glow_colors[GLOW_THERMAL].b=0.5f;

	glow_colors[GLOW_MAGIC].r=0.5f;
	glow_colors[GLOW_MAGIC].g=0.4f;
	glow_colors[GLOW_MAGIC].b=0.0f;
}

//return a pointer to the new allocated actor
static actor* create_enhanced_actor(enhanced_actor *this_actor, float x_pos, float y_pos,
	float z_pos, float z_rot, float scale, int actor_id, const char* name)
{
	int texture_id;
	int k;
	actor *our_actor;
#ifdef CLUSTER_INSIDES
	int x, y;
#endif

#ifdef EXTRA_DEBUG
	ERR();
#endif

	//get the skin
	texture_id = load_enhanced_actor(this_actor, name);

	our_actor = calloc(1, sizeof(actor));
	if (!our_actor)
		return NULL;

	memset(our_actor->current_displayed_text, 0, sizeof(our_actor->current_displayed_text));
	our_actor->current_displayed_text_time_left =  0;

	our_actor->texture_id=texture_id;
	our_actor->is_enhanced_model=1;
	our_actor->actor_id=actor_id;

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
	our_actor->delay_texture_item_changes = 1;

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

	//clear the que
	for(k=0; k<MAX_CMD_QUEUE; k++)	our_actor->que[k]=nothing;
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



//	our_actor->model_data=0;
	our_actor->stand_idle=0;
	our_actor->sit_idle=0;
	our_actor->body_parts=this_actor;

	our_actor->attached_actor_id = -1;
	our_actor->attachment_shift[0] = our_actor->attachment_shift[1] = our_actor->attachment_shift[2] = 0.0;

#ifdef CLUSTER_INSIDES
	x = (int) (our_actor->x_pos / 0.5f);
	y = (int) (our_actor->y_pos / 0.5f);
	our_actor->cluster = get_cluster (x, y);
#endif

	if (actor_id == yourself)
	{
		// We have just returned from limbo (after teleport,
		// map change, disconnect, etc.) Since our position may
		// have changed, tell the bbox tree to update so
		// that our environment is recomputed.
		set_all_intersect_update_needed (main_bbox_tree);
	}

	return our_actor;
}

Uint32 delay_texture_item_change(actor* a, const int which_part, const int which_id)
{
	if (a == 0)
	{
		return 0;
	}

	if (a->delay_texture_item_changes != 0)
	{
		change_enhanced_actor(a->texture_id, a->body_parts);

		if (a->delayed_item_changes_count < MAX_ITEM_CHANGES_QUEUE)
		{
			a->delayed_item_changes[a->delayed_item_changes_count] = which_id;
			a->delayed_item_type_changes[a->delayed_item_changes_count] = which_part;
			a->delayed_item_changes_count++;

			return 1;
		}
	}

	return 0;
}

void unwear_item_from_actor(int actor_id, Uint8 which_part)
{
	actor *act;
	locked_list_ptr actors_list = lock_and_get_actor_from_id(actor_id, &act);
	if (actors_list)
	{
		unwear_item_from_actor_locked(act, which_part);
		release_locked_actors_list(actors_list);
	}
}

void unwear_item_from_actor_locked(actor *act, Uint8 which_part)
{
	if (which_part == KIND_OF_WEAPON)
	{
		ec_remove_weapon(act);
		if (act->cur_weapon == GLOVE_FUR || act->cur_weapon == GLOVE_LEATHER)
		{
			safe_strncpy(act->body_parts->hands_tex, act->body_parts->hands_tex_save,
				sizeof(act->body_parts->hands_tex));
		}
		if (!delay_texture_item_change(act, which_part, -1))
		{
			model_detach_mesh(act, actors_defs[act->actor_type].weapon[act->cur_weapon].mesh_index);
			act->body_parts->weapon_tex[0]=0;
			act->cur_weapon = WEAPON_NONE;
			act->body_parts->weapon_meshindex = -1;
		}
	}
	else if (which_part == KIND_OF_SHIELD)
	{
		if (act->in_aim_mode > 0)
		{
			if (act->delayed_item_changes_count < MAX_ITEM_CHANGES_QUEUE)
			{
				missiles_log_message("%s (%d): unwear item type %d delayed",
					act->actor_name, act->actor_id, which_part);
				act->delayed_item_changes[act->delayed_item_changes_count] = -1;
				act->delayed_item_type_changes[act->delayed_item_changes_count] = which_part;
				++act->delayed_item_changes_count;
			}
			else
			{
				LOG_ERROR("the item changes queue is full!");
			}
		}
		else
		{
			model_detach_mesh(act, act->body_parts->shield_meshindex);
			act->body_parts->shield_tex[0]=0;
			act->cur_shield = SHIELD_NONE;
			act->body_parts->shield_meshindex = -1;
		}
	}
	else if (which_part == KIND_OF_CAPE)
	{
		model_detach_mesh(act, act->body_parts->cape_meshindex);
		act->body_parts->cape_tex[0]=0;
		act->body_parts->cape_meshindex = -1;
	}
	else if (which_part == KIND_OF_HELMET)
	{
		model_detach_mesh(act, act->body_parts->helmet_meshindex);
		act->body_parts->helmet_tex[0]=0;
		act->body_parts->helmet_meshindex = -1;
	}
	else if (which_part == KIND_OF_NECK)
	{
		model_detach_mesh(act, act->body_parts->neck_meshindex);
		act->body_parts->neck_tex[0]=0;
		act->body_parts->neck_meshindex = -1;
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

#ifdef CUSTOM_LOOK
void custom_path(char * path, char * custom1, char * custom2) {
	char buffer[256];

	// check to see if ANY processing needs to be done
#ifdef CUSTOM_UPDATE
	if(!path || !*path || !custom_clothing)	return;
#else
	if(!path || !*path ) return;
#endif

	/* Check if custom1 has path readable */
	safe_snprintf(buffer, sizeof(buffer), "%s%s", custom1, path);
	if (el_custom_file_exists(buffer)) {
		strcpy(path, buffer);
		return;
	}

	/* Check if custom2 has path readable */
	safe_snprintf(buffer, sizeof(buffer), "%s%s", custom2, path);
	if (el_custom_file_exists(buffer)) {
		strcpy(path, buffer);
		return;
	}

	/* leave as is */
	return;
}
#endif  //CUSTOM_LOOK

/*
 * Common function to return only the player name in the specified buffer, stripped of possible leading colour
 * codes and terminated at the end of the name, before any guild tag.
 * The function returns index in name_buf of the character immediately after the name.
 */ 
static size_t get_onlyname(const char *name_buf, size_t name_buf_size, char *onlyname_buf, size_t onlyname_buf_size)
{
	size_t i, j;

	/* step past any leading colour codes */
	for (i = 0; (name_buf[i] != '\0') && (i < name_buf_size) && is_color(name_buf[i]); i++) /* do nothing */;

	/* copy characters, terminate at the end of the name or the colour code before the guild tag */
	for (j = 0; (name_buf[i] != '\0') && (i < name_buf_size) && is_printable(name_buf[i]) && (j < (onlyname_buf_size - 1)); i++, j++)
		onlyname_buf[j] = name_buf[i];

	/* if the name index is now on the colour code after the space bewtween name and guild tag, move back to the space */
	if ((name_buf[i] != '\0') && !is_printable(name_buf[i]) && (i > 0) && (j > 0) && (name_buf[i-1] == ' '))
	{
		i--;
		j--;
	}
	onlyname_buf[j] = '\0';
	my_tolower(onlyname_buf);

	return i;
}

void actor_wear_item(int actor_id,Uint8 which_part, Uint8 which_id)
{
#ifdef CUSTOM_LOOK
	char playerpath[256], guildpath[256], onlyname[32]={0};
#endif
	actor *act;
	locked_list_ptr actors_list = lock_and_get_actor_from_id(actor_id, &act);
	if (!actors_list)
		return;

#ifdef CUSTOM_LOOK
	get_onlyname(act->actor_name, sizeof(act->actor_name), onlyname, sizeof(onlyname));
	safe_snprintf(guildpath, sizeof(guildpath), "custom/guild/%d/", act->body_parts->guild_id);
	safe_snprintf(playerpath, sizeof(playerpath), "custom/player/%s/", onlyname);
#endif
	if (which_part == KIND_OF_WEAPON)
	{
		if (which_id == GLOVE_FUR || which_id == GLOVE_LEATHER)
		{
			safe_strncpy(act->body_parts->hands_tex,
				actors_defs[act->actor_type].weapon[which_id].skin_name,
				sizeof(act->body_parts->hands_tex));
			safe_strncpy(act->body_parts->hands_mask,
				actors_defs[act->actor_type].weapon[which_id].skin_mask,
				sizeof(act->body_parts->hands_mask));
#ifdef CUSTOM_LOOK
			custom_path(act->body_parts->hands_tex, playerpath, guildpath);
			custom_path(act->body_parts->hands_mask, playerpath, guildpath);
#endif
		}
		else
		{
			safe_strncpy(act->body_parts->weapon_tex,
				actors_defs[act->actor_type].weapon[which_id].skin_name,
				sizeof(act->body_parts->weapon_tex));
#ifdef CUSTOM_LOOK
			custom_path(act->body_parts->weapon_tex, playerpath, guildpath);
#endif
		}

		if (!delay_texture_item_change(act, which_part, which_id))
		{
			model_attach_mesh(act, actors_defs[act->actor_type].weapon[which_id].mesh_index);
			act->cur_weapon=which_id;
			act->body_parts->weapon_meshindex = actors_defs[act->actor_type].weapon[which_id].mesh_index;
			act->body_parts->weapon_glow=actors_defs[act->actor_type].weapon[which_id].glow;
			switch (which_id)
			{
				case SWORD_1_FIRE:
				case SWORD_2_FIRE:
				case SWORD_3_FIRE:
				case SWORD_4_FIRE:
				case SWORD_4_THERMAL:
				case SWORD_5_FIRE:
				case SWORD_5_THERMAL:
				case SWORD_6_FIRE:
				case SWORD_6_THERMAL:
				case SWORD_7_FIRE:
				case SWORD_7_THERMAL:
					ec_create_sword_of_fire(act, (poor_man ? 6 : 10));
					break;
				case SWORD_2_COLD:
				case SWORD_3_COLD:
				case SWORD_4_COLD:
				case SWORD_5_COLD:
				case SWORD_6_COLD:
				case SWORD_7_COLD:
					ec_create_sword_of_ice(act, (poor_man ? 6 : 10));
					break;
				case SWORD_3_MAGIC:
				case SWORD_4_MAGIC:
				case SWORD_5_MAGIC:
				case SWORD_6_MAGIC:
				case SWORD_7_MAGIC:
					ec_create_sword_of_magic(act, (poor_man ? 6 : 10));
					break;
				case SWORD_EMERALD_CLAYMORE:
					ec_create_sword_emerald_claymore(act, (poor_man ? 6 : 10));
					break;
				case SWORD_CUTLASS:
					ec_create_sword_cutlass(act, (poor_man ? 6 : 10));
					break;
				case SWORD_SUNBREAKER:
					ec_create_sword_sunbreaker(act, (poor_man ? 6 : 10));
					break;
				case SWORD_ORC_SLAYER:
					ec_create_sword_orc_slayer(act, (poor_man ? 6 : 10));
					break;
				case SWORD_EAGLE_WING:
					ec_create_sword_eagle_wing(act, (poor_man ? 6 : 10));
					break;
				case SWORD_JAGGED_SABER:
					ec_create_sword_jagged_saber(act, (poor_man ? 6 : 10));
					break;
				case STAFF_3: // staff of protection
					ec_create_staff_of_protection(act, (poor_man ? 6 : 10));
					break;
				case STAFF_4: // staff of the mage
					ec_create_staff_of_the_mage(act, (poor_man ? 6 : 10));
					break;
			}
		}
	}
	else if (which_part == KIND_OF_SHIELD)
	{
		safe_strncpy(act->body_parts->shield_tex,
			actors_defs[act->actor_type].shield[which_id].skin_name,
			sizeof(act->body_parts->shield_tex));
#ifdef CUSTOM_LOOK
		custom_path(act->body_parts->shield_tex, playerpath, guildpath);
#endif

		if (!delay_texture_item_change(act, which_part, which_id))
		{
			model_attach_mesh(act, actors_defs[act->actor_type].shield[which_id].mesh_index);
			act->body_parts->shield_meshindex=actors_defs[act->actor_type].shield[which_id].mesh_index;
			act->cur_shield=which_id;
			act->body_parts->shield_meshindex = actors_defs[act->actor_type].shield[which_id].mesh_index;
		}
	}
	else if (which_part == KIND_OF_CAPE)
	{
		safe_strncpy(act->body_parts->cape_tex,
			actors_defs[act->actor_type].cape[which_id].skin_name,
			sizeof(act->body_parts->cape_tex));
#ifdef CUSTOM_LOOK
		custom_path(act->body_parts->cape_tex, playerpath, guildpath);
#endif

		if (!delay_texture_item_change(act, which_part, which_id))
		{
			model_attach_mesh(act, actors_defs[act->actor_type].cape[which_id].mesh_index);
			act->body_parts->cape_meshindex=actors_defs[act->actor_type].cape[which_id].mesh_index;
		}
	}
	else if (which_part == KIND_OF_HELMET)
	{
		safe_strncpy(act->body_parts->helmet_tex,
			actors_defs[act->actor_type].helmet[which_id].skin_name,
			sizeof(act->body_parts->helmet_tex));
#ifdef CUSTOM_LOOK
		custom_path(act->body_parts->helmet_tex, playerpath, guildpath);
#endif

		if (!delay_texture_item_change(act, which_part, which_id))
		{
			model_attach_mesh(act, actors_defs[act->actor_type].helmet[which_id].mesh_index);
			act->body_parts->helmet_meshindex=actors_defs[act->actor_type].helmet[which_id].mesh_index;
		}
	}
	else if (which_part == KIND_OF_NECK)
	{
		assert(!"Using old client data" || actors_defs[act->actor_type].neck != NULL);
		safe_strncpy(act->body_parts->neck_tex,
			actors_defs[act->actor_type].neck[which_id].skin_name,
			sizeof(act->body_parts->neck_tex));
#ifdef CUSTOM_LOOK
		custom_path(act->body_parts->neck_tex, playerpath, guildpath);
#endif
		if (!delay_texture_item_change(act, which_part, which_id))
		{
			model_attach_mesh(act, actors_defs[act->actor_type].neck[which_id].mesh_index);
			act->body_parts->neck_meshindex=actors_defs[act->actor_type].neck[which_id].mesh_index;
		}
	}
	else if (which_part == KIND_OF_BODY_ARMOR)
	{
		safe_strncpy(act->body_parts->arms_tex,
			actors_defs[act->actor_type].shirt[which_id].arms_name,
			sizeof(act->body_parts->arms_tex));
		safe_strncpy(act->body_parts->torso_tex,
			actors_defs[act->actor_type].shirt[which_id].torso_name,
			sizeof(act->body_parts->torso_tex));
		safe_strncpy(act->body_parts->arms_mask,
			actors_defs[act->actor_type].shirt[which_id].arms_mask,
			sizeof(act->body_parts->arms_mask));
		safe_strncpy(act->body_parts->torso_mask,
			actors_defs[act->actor_type].shirt[which_id].torso_mask,
			sizeof(act->body_parts->torso_mask));
#ifdef CUSTOM_LOOK
		custom_path(act->body_parts->arms_tex, playerpath, guildpath);
		custom_path(act->body_parts->torso_tex, playerpath, guildpath);
		custom_path(act->body_parts->arms_mask, playerpath, guildpath);
		custom_path(act->body_parts->torso_mask, playerpath, guildpath);
#endif
		if (!delay_texture_item_change(act, which_part, which_id)
			&& actors_defs[act->actor_type].shirt[which_id].mesh_index != act->body_parts->torso_meshindex)
		{
			model_detach_mesh(act, act->body_parts->torso_meshindex);
			model_attach_mesh(act, actors_defs[act->actor_type].shirt[which_id].mesh_index);
			act->body_parts->torso_meshindex=actors_defs[act->actor_type].shirt[which_id].mesh_index;
		}
	}
	else if (which_part == KIND_OF_LEG_ARMOR)
	{
		safe_strncpy(act->body_parts->pants_tex,
			actors_defs[act->actor_type].legs[which_id].legs_name,
			sizeof(act->body_parts->pants_tex));
		safe_strncpy(act->body_parts->pants_mask,
			actors_defs[act->actor_type].legs[which_id].legs_mask,
			sizeof(act->body_parts->pants_mask));
#ifdef CUSTOM_LOOK
		custom_path(act->body_parts->pants_tex, playerpath, guildpath);
		custom_path(act->body_parts->pants_mask, playerpath, guildpath);
#endif
		if (!delay_texture_item_change(act, which_part, which_id)
			&& actors_defs[act->actor_type].legs[which_id].mesh_index != act->body_parts->legs_meshindex)
		{
			model_detach_mesh(act, act->body_parts->legs_meshindex);
			model_attach_mesh(act, actors_defs[act->actor_type].legs[which_id].mesh_index);
			act->body_parts->legs_meshindex=actors_defs[act->actor_type].legs[which_id].mesh_index;
		}
	}
	else if (which_part==KIND_OF_BOOT_ARMOR)
	{
		safe_strncpy(act->body_parts->boots_tex,
			actors_defs[act->actor_type].boots[which_id].boots_name,
			sizeof(act->body_parts->boots_tex));
		safe_strncpy(act->body_parts->boots_mask,
			actors_defs[act->actor_type].boots[which_id].boots_mask,
			sizeof(act->body_parts->boots_mask));
#ifdef CUSTOM_LOOK
		custom_path(act->body_parts->boots_tex, playerpath, guildpath);
		custom_path(act->body_parts->boots_mask, playerpath, guildpath);
#endif
		if (!delay_texture_item_change(act, which_part, which_id)
			&& actors_defs[act->actor_type].boots[which_id].mesh_index != act->body_parts->boots_meshindex)
		{
			model_detach_mesh(act, act->body_parts->boots_meshindex);
			model_attach_mesh(act, actors_defs[act->actor_type].boots[which_id].mesh_index);
			act->body_parts->boots_meshindex=actors_defs[act->actor_type].boots[which_id].mesh_index;
		}
	}

	release_locked_actors_list(actors_list);
}

void add_enhanced_actor_from_server (const char *in_data, int len)
{
	short actor_id;
	Uint32 buffs;
	short x_pos;
	short y_pos;
	short z_rot;
	short max_health;
	short cur_health;
	Uint32 actor_type;
	Uint8 skin;
	Uint8 hair;
#ifdef NEW_EYES
	Uint8 eyes;
#endif
	Uint8 shirt;
	Uint8 pants;
	Uint8 boots;
	Uint8 frame;
	Uint8 cape;
	Uint8 head;
	Uint8 shield;
	Uint8 weapon;
	Uint8 helmet;
	Uint8 neck;
	int dead=0;
	int kind_of_actor;
	enhanced_actor *this_actor;
#ifdef CUSTOM_LOOK
	char playerpath[256], guildpath[256];
#endif
	char onlyname[32]={0};
	Uint32 uniq_id = 0; // - Post ported.... We'll come up with something later...
	Uint32 guild_id;
	double f_x_pos,f_y_pos,f_z_rot;
	float   scale=1.0f;
	emote_data *pose=NULL;
	int attachment_type = -1;
	actor* actor, *attached;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	actor_id=SDL_SwapLE16(*((short *)(in_data)));
	buffs=(((SDL_SwapLE16(*((char*)(in_data+3)))>>3)&0x1F) | (SDL_SwapLE16(((*((char*)(in_data+5)))>>3)&0x1F)<<5));	// Strip the last 5 bits of the X and Y coords for the buffs
	x_pos=SDL_SwapLE16(*((short *)(in_data+2))) & 0x7FF;
	y_pos=SDL_SwapLE16(*((short *)(in_data+4))) & 0x7FF;
	buffs |= (SDL_SwapLE16(*((short *)(in_data+6))) & 0xFF80) << 3; // we get the 9 MSB for the buffs and leave the 7 LSB for a further use
	z_rot=SDL_SwapLE16(*((short *)(in_data+8)));
	actor_type=*(in_data+10);
	skin=*(in_data+12);
	hair=*(in_data+13);
	shirt=*(in_data+14);
	pants=*(in_data+15);
	boots=*(in_data+16);
	head=*(in_data+17);
	shield=*(in_data+18);
	weapon=*(in_data+19);
	cape=*(in_data+20);
	helmet=*(in_data+21);

#ifdef EXTRA_DEBUG
	ERR();
#endif

	if(actor_type >= MAX_ACTOR_DEFS || (actor_type > 0 && actors_defs[actor_type].actor_type != actor_type) ){
		char    str[256];

		safe_snprintf(str, sizeof(str), "Illegal/missing enhanced actor definition %d", actor_type);
		LOG_ERROR(str);
		return;		// We cannot load an actor without a def (seg fault) so bail here.
	}

	frame=*(in_data+22);
	max_health=SDL_SwapLE16(*((short *)(in_data+23)));
	cur_health=SDL_SwapLE16(*((short *)(in_data+25)));
	kind_of_actor=*(in_data+27);
#if defined CUSTOM_LOOK && defined UID
	//experimental code, not supported by the server
	uniq_id = SDL_SwapLE32(*((Uint32*)(in_data+28)));
	if(len > 32+(int)strlen(in_data+32)+2){
		scale=((float)SDL_SwapLE16(*((short *)(in_data+32+strlen(in_data+32)+1)))/((float)ACTOR_SCALE_BASE));
		if(len > 32+(int)strlen(in_data+32)+3)
			attachment_type = in_data[32+strlen(in_data+32)+3];
	}
#else
	if(len > 28+(int)strlen(in_data+28)+2){
		scale=((float)SDL_SwapLE16(*((short *)(in_data+28+strlen(in_data+28)+1)))/((float)ACTOR_SCALE_BASE));
		if(len > 28+(int)strlen(in_data+28)+3)
			attachment_type = (unsigned char)in_data[28+strlen(in_data+28)+3];
	}
#endif
	//the last bytes of the packet even if scale+attachment is not sent
#ifdef NEW_EYES
	if ((in_data[len-2] > EYES_GOLD)
#if (EYES_BROWN > 0) || (CHAR_MIN == SCHAR_MIN)
		|| (in_data[len-2] < EYES_BROWN)
#endif
	)
		eyes = EYES_BROWN;
	else
		eyes = in_data[len-2];
#endif
	neck=*(in_data+len-1);

	//translate from tile to world
	f_x_pos=x_pos*0.5;
	f_y_pos=y_pos*0.5;
	f_z_rot=z_rot;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	//get the current frame
	switch(frame) {
	case frame_walk:
	case frame_run:
		break;
	case frame_die1:
		dead=1;
		break;
	case frame_die2:
		dead=1;
		break;
	case frame_pain1:
	case frame_pain2:
	case frame_drop:
	case frame_idle:
	case frame_sit_idle:
	case frame_harvest:
	case frame_cast:
	case frame_ranged:
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
		if(frame>=frame_poses_start&&frame<=frame_poses_end) {
			//we have a pose, get it! (frame is the emote_id)
			hash_entry *he;
			he=hash_get(emotes,(void*)(uintptr_t)frame);
			if(!he) LOG_ERROR("unknown pose %d", frame);
			else pose = he->item;
			break;
		}
#ifdef UID
		LOG_ERROR("%s %d - %s\n", unknown_frame, frame, &in_data[32]);
#else
		LOG_ERROR("%s %d - %s\n", unknown_frame, frame, &in_data[28]);
#endif
	}

#ifdef EXTRA_DEBUG
	ERR();
#endif

	this_actor=calloc(1,sizeof(enhanced_actor));

	/* build a clean player name and a guild id */
	{
#ifdef UID
		const size_t name_index = 32;
#else
		const size_t name_index = 28;
#endif
#ifdef NEW_EYES
		const size_t max_name_len = len - name_index - 5;
#else
		const size_t max_name_len = len - name_index - 4;
#endif
		size_t guild_name_index = get_onlyname(&in_data[name_index], max_name_len, onlyname, sizeof(onlyname));

		/* the guild tag is space + colour code + name (max 4 characters) */
		guild_id = 0;
		this_actor->guild_tag_color = 0;
		if (guild_name_index < max_name_len)
		{
			const char *guild = &in_data[name_index + guild_name_index];
			size_t guild_tag_len = strnlen(guild, max_name_len - guild_name_index);
			/* get the colour code and hash of guildtag */
			if ((guild_tag_len > 2) && (guild_tag_len < 7) && (guild[0] == ' ') && is_color(guild[1]))
			{
				unsigned int shift = 0;
				size_t j;
				this_actor->guild_tag_color = from_color_char(guild[1]);
				for (j = 0; (j < (guild_tag_len - 2)) && (j < 4); j++, shift += 8)
					guild_id += guild[2+j] << shift;
			}
		}
	}

#ifdef  CUSTOM_LOOK
	/* precompute the paths to custom files */
	safe_snprintf(playerpath, sizeof(playerpath), "custom/player/%s/", onlyname);
	safe_snprintf(guildpath, sizeof(guildpath), "custom/guild/%u/", guild_id);
#endif  //CUSTOM_LOOK

	/* store the ids */
	this_actor->uniq_id = uniq_id;
	this_actor->guild_id = guild_id;

	//get the torso
	safe_strncpy(this_actor->arms_tex,actors_defs[actor_type].shirt[shirt].arms_name,sizeof(this_actor->arms_tex));
	safe_strncpy(this_actor->torso_tex,actors_defs[actor_type].shirt[shirt].torso_name,sizeof(this_actor->torso_tex));
	safe_strncpy(this_actor->arms_mask,actors_defs[actor_type].shirt[shirt].arms_mask,sizeof(this_actor->arms_mask));
	safe_strncpy(this_actor->torso_mask,actors_defs[actor_type].shirt[shirt].torso_mask,sizeof(this_actor->torso_mask));
	//skin
	safe_strncpy(this_actor->hands_tex,actors_defs[actor_type].skin[skin].hands_name,sizeof(this_actor->hands_tex));
	safe_strncpy(this_actor->hands_tex_save,actors_defs[actor_type].skin[skin].hands_name,sizeof(this_actor->hands_tex_save));
	safe_strncpy(this_actor->hands_mask,"",sizeof(this_actor->hands_mask));	// by default, nothing
	safe_strncpy(this_actor->head_tex,actors_defs[actor_type].skin[skin].head_name,sizeof(this_actor->head_tex));
	safe_strncpy(this_actor->head_base,actors_defs[actor_type].skin[skin].head_name,sizeof(this_actor->head_base));
	safe_strncpy(this_actor->head_mask,"",sizeof(this_actor->head_mask));	// by default, nothing
	if(*actors_defs[actor_type].head[head].skin_name)
		safe_strncpy(this_actor->head_tex,actors_defs[actor_type].head[head].skin_name,sizeof(this_actor->head_tex));
	if(*actors_defs[actor_type].head[head].skin_mask)
		safe_strncpy(this_actor->head_mask,actors_defs[actor_type].head[head].skin_mask,sizeof(this_actor->head_mask));
	safe_strncpy(this_actor->body_base,actors_defs[actor_type].skin[skin].body_name,sizeof(this_actor->body_base));
	safe_strncpy(this_actor->arms_base,actors_defs[actor_type].skin[skin].arms_name,sizeof(this_actor->arms_base));
	safe_strncpy(this_actor->legs_base,actors_defs[actor_type].skin[skin].legs_name,sizeof(this_actor->legs_base));
	safe_strncpy(this_actor->boots_base,actors_defs[actor_type].skin[skin].feet_name,sizeof(this_actor->boots_base));
	//hair
	safe_strncpy(this_actor->hair_tex,actors_defs[actor_type].hair[hair].hair_name,sizeof(this_actor->hair_tex));
#ifdef NEW_EYES
	//eyes
	if (actors_defs[actor_type].eyes && *actors_defs[actor_type].eyes[eyes].eyes_name != '\0')
	{
		safe_strncpy(this_actor->eyes_tex,actors_defs[actor_type].eyes[eyes].eyes_name,sizeof(this_actor->eyes_tex));
	}
	else
	{
		static int already_said = 0;
		if (!already_said)
		{
			char *message = "Looks like we compiled with NEW_EYES but do not have the textures";
			LOG_ERROR(message);
			LOG_TO_CONSOLE(c_red2,message);
			already_said = 1;
		}
	}
#endif
	//boots
	safe_strncpy(this_actor->boots_tex,actors_defs[actor_type].boots[boots].boots_name,sizeof(this_actor->boots_tex));
	safe_strncpy(this_actor->boots_mask,actors_defs[actor_type].boots[boots].boots_mask,sizeof(this_actor->boots_mask));
	//legs
	safe_strncpy(this_actor->pants_tex,actors_defs[actor_type].legs[pants].legs_name,sizeof(this_actor->pants_tex));
	safe_strncpy(this_actor->pants_mask,actors_defs[actor_type].legs[pants].legs_mask,sizeof(this_actor->pants_mask));

#ifdef CUSTOM_LOOK
	if(kind_of_actor != NPC)
	{
		//torso
		custom_path(this_actor->arms_tex, playerpath, guildpath);
		custom_path(this_actor->torso_tex, playerpath, guildpath);
		custom_path(this_actor->arms_mask, playerpath, guildpath);
		custom_path(this_actor->torso_mask, playerpath, guildpath);
		//skin
		custom_path(this_actor->hands_tex, playerpath, guildpath);
		custom_path(this_actor->hands_tex_save, playerpath, guildpath);
		custom_path(this_actor->head_tex, playerpath, guildpath);
		custom_path(this_actor->body_base, playerpath, guildpath);
		custom_path(this_actor->arms_base, playerpath, guildpath);
		custom_path(this_actor->legs_base, playerpath, guildpath);
		//hair
		custom_path(this_actor->hair_tex, playerpath, guildpath);
#ifdef NEW_EYES
		//eyes
		custom_path(this_actor->eyes_tex, playerpath, guildpath);
#endif
		//boots
		custom_path(this_actor->boots_tex, playerpath, guildpath);
		custom_path(this_actor->boots_mask, playerpath, guildpath);
		//legs
		custom_path(this_actor->pants_tex, playerpath, guildpath);
		custom_path(this_actor->pants_mask, playerpath, guildpath);
	}
#endif //CUSTOM_LOOK

	//cape
	if(cape!=CAPE_NONE)
		{
			safe_strncpy(this_actor->cape_tex,actors_defs[actor_type].cape[cape].skin_name,sizeof(this_actor->cape_tex));
#ifdef CUSTOM_LOOK
			if(kind_of_actor != NPC)
				custom_path(this_actor->cape_tex, playerpath, guildpath);
#endif //CUSTOM_LOOK
		}
	else
		{
			safe_strncpy(this_actor->cape_tex,"",sizeof(this_actor->cape_tex));
		}
	//shield
	if(shield!=SHIELD_NONE)
		{
			safe_strncpy(this_actor->shield_tex,actors_defs[actor_type].shield[shield].skin_name,sizeof(this_actor->shield_tex));
#ifdef CUSTOM_LOOK
			if(kind_of_actor != NPC)
				custom_path(this_actor->shield_tex, playerpath, guildpath);
#endif //CUSTOM_LOOK
		}
	else
		{
			safe_strncpy(this_actor->shield_tex,"",sizeof(this_actor->shield_tex));
		}

	if (weapon == GLOVE_FUR || weapon == GLOVE_LEATHER)
	{
		safe_strncpy(this_actor->hands_tex, actors_defs[actor_type].weapon[weapon].skin_name,sizeof(this_actor->hands_tex));
#ifdef CUSTOM_LOOK
		if(kind_of_actor != NPC)
			custom_path(this_actor->hands_tex, playerpath, guildpath);
#endif //CUSTOM_LOOK
	}
	else
	{
		safe_strncpy(this_actor->weapon_tex,actors_defs[actor_type].weapon[weapon].skin_name,sizeof(this_actor->weapon_tex));
#ifdef CUSTOM_LOOK
		if(kind_of_actor != NPC)
			custom_path(this_actor->weapon_tex, playerpath, guildpath);
#endif //CUSTOM_LOOK
	}

	this_actor->weapon_glow=actors_defs[actor_type].weapon[weapon].glow;

	//helmet
	if(helmet!=HELMET_NONE)
		{
			safe_strncpy(this_actor->helmet_tex,actors_defs[actor_type].helmet[helmet].skin_name,sizeof(this_actor->helmet_tex));

#ifdef CUSTOM_LOOK
			if(kind_of_actor != NPC)
				custom_path(this_actor->helmet_tex, playerpath, guildpath);
#endif //CUSTOM_LOOK
		}
	else
		{
			safe_strncpy(this_actor->helmet_tex,"",sizeof(this_actor->helmet_tex));
		}

	//neck
	if(neck!=NECK_NONE)
		{
			assert(!"Using old client data" || actors_defs[actor_type].neck != NULL);
			safe_strncpy(this_actor->neck_tex,actors_defs[actor_type].neck[neck].skin_name,sizeof(this_actor->neck_tex));
#ifdef CUSTOM_LOOK
			if(kind_of_actor != NPC)
				custom_path(this_actor->neck_tex, playerpath, guildpath);
#endif //CUSTOM_LOOK
		}
	else
		{
			safe_strncpy(this_actor->neck_tex,"",sizeof(this_actor->neck_tex));
		}

	actor = create_enhanced_actor(this_actor,f_x_pos,f_y_pos,0.0,f_z_rot,scale,actor_id, onlyname);
#ifdef EXTRA_DEBUG
	ERR();
#endif //EXTRA_DEBUG

	actor->async_fighting = 0;
	actor->async_x_tile_pos = x_pos;
	actor->async_y_tile_pos = y_pos;
	actor->async_z_rot = z_rot;

	actor->x_tile_pos=x_pos;
	actor->y_tile_pos=y_pos;
	actor->buffs=buffs;
	actor->actor_type=actor_type;
	actor->damage=0;
	actor->damage_ms=0;
	actor->sitting=0;
	actor->fighting=0;
	//test only
	actor->max_health=max_health;
	actor->cur_health=cur_health;

    actor->step_duration = actors_defs[actor_type].step_duration;
	if (actor->buffs & BUFF_DOUBLE_SPEED)
		actor->step_duration /= 2;

    actor->z_pos = get_actor_z(actor);
	if(frame==frame_sit_idle||(pose!=NULL&&pose->pose==EMOTE_SITTING)){ //sitting pose sent by the server
			actor->poses[EMOTE_SITTING]=pose;
			if(actor->actor_id==yourself)
				you_sit=1;
			actor->sitting=1;
		}
	else if(frame==frame_stand||(pose!=NULL&&pose->pose==EMOTE_STANDING)){//standing pose sent by server
			actor->poses[EMOTE_STANDING]=pose;
			if(actor->actor_id==yourself)
				you_sit=0;
		}
	else if(frame==frame_walk||(pose!=NULL&&pose->pose==EMOTE_WALKING)){//walking pose sent by server
			actor->poses[EMOTE_WALKING]=pose;
			if(actor->actor_id==yourself)
				you_sit=0;
		}
	else if(frame==frame_run||(pose!=NULL&&pose->pose==EMOTE_RUNNING)){//running pose sent by server
			actor->poses[EMOTE_RUNNING]=pose;
			if(actor->actor_id==yourself)
				you_sit=0;
		}
	else
		{
			if(actor->actor_id==yourself)
				you_sit=0;
			if(frame==frame_combat_idle)
				actor->fighting=1;
			else if (frame == frame_ranged)
				actor->in_aim_mode = 1;
		}
	//ghost or not?
	actor->ghost=actors_defs[actor_type].ghost;

	actor->dead=dead;
	actor->stop_animation=1;//helps when the actor is dead...
	actor->cur_weapon=weapon;
	actor->cur_shield=shield;
	actor->kind_of_actor=kind_of_actor;
	if(strlen(&in_data[28]) >= 30)
	{
		LOG_ERROR("%s (%d): %s/%d\n", bad_actor_name_length, actor->actor_type,&in_data[28], (int)strlen(&in_data[28]));
	}
	else
	{
		/* Extract the name for use in the tab completion list. */
		const unsigned char *name = (unsigned char*) in_data+28;
		char *ptr;
		char buffer[32];

		if(kind_of_actor != NPC) {
			/* Skip leading color codes */
			for (; *name && is_color (*name); name++) /* nothing */;
			safe_snprintf(buffer, sizeof(buffer), "%.30s", name);
			/* Remove guild tag, etc. */
			for(ptr = buffer; *ptr && *ptr > 0 && !isspace(*ptr); ptr++);
			*ptr = '\0';
			add_name_to_tablist(buffer);
		}
		safe_strncpy(actor->actor_name,&in_data[28],sizeof(actor->actor_name));
		if(caps_filter && my_isupper(actor->actor_name, -1)) {
			my_tolower(actor->actor_name);
		}
	}

	if (attachment_type >= 0 &&attachment_type < 255) //255 is not necessary, but it suppresses a warning in errorlog
		attached = create_actor_attachment(actor, attachment_type);
	else
		attached = NULL;

	if (actors_defs[actor_type].coremodel!=NULL) {
		actor->calmodel=model_new(actors_defs[actor_type].coremodel);

		if (actor->calmodel!=NULL) {
			//Setup cal3d model
			//actor->calmodel=CalModel_New(actors_defs[actor_type].coremodel);
			//Attach meshes
			model_attach_mesh(actor, actors_defs[actor_type].head[head].mesh_index);
			model_attach_mesh(actor, actors_defs[actor_type].shirt[shirt].mesh_index);
			model_attach_mesh(actor, actors_defs[actor_type].legs[pants].mesh_index);
			model_attach_mesh(actor, actors_defs[actor_type].boots[boots].mesh_index);
			actor->body_parts->torso_meshindex=actors_defs[actor_type].shirt[shirt].mesh_index;
			actor->body_parts->legs_meshindex=actors_defs[actor_type].legs[pants].mesh_index;
			actor->body_parts->head_meshindex=actors_defs[actor_type].head[head].mesh_index;
			actor->body_parts->boots_meshindex=actors_defs[actor_type].boots[boots].mesh_index;

			if (cape!=CAPE_NONE) model_attach_mesh(actor, actors_defs[actor_type].cape[cape].mesh_index);
			if (helmet!=HELMET_NONE) model_attach_mesh(actor, actors_defs[actor_type].helmet[helmet].mesh_index);
			if (weapon!=WEAPON_NONE) model_attach_mesh(actor, actors_defs[actor_type].weapon[weapon].mesh_index);
			if (shield!=SHIELD_NONE) model_attach_mesh(actor, actors_defs[actor_type].shield[shield].mesh_index);
			assert(!"Using old client data" || actors_defs[actor_type].neck != NULL);
			if (neck!=NECK_NONE) model_attach_mesh(actor, actors_defs[actor_type].neck[neck].mesh_index);
			actor->body_parts->neck_meshindex=actors_defs[actor_type].neck[neck].mesh_index;
			actor->body_parts->helmet_meshindex=actors_defs[actor_type].helmet[helmet].mesh_index;
			actor->body_parts->cape_meshindex=actors_defs[actor_type].cape[cape].mesh_index;
			actor->body_parts->shield_meshindex=actors_defs[actor_type].shield[shield].mesh_index;

			actor->cur_anim.anim_index=-1;
#ifdef NEW_SOUND
			stop_sound(actor->cur_anim_sound_cookie);
#endif // NEW_SOUND
			actor->cur_anim_sound_cookie= 0;
			actor->anim_time= 0.0;
			actor->last_anim_update= cur_time;

			if(dead){
				cal_actor_set_anim(actor, attached,
					actors_defs[actor->actor_type].cal_frames[cal_actor_die1_frame]);
				actor->stop_animation=1;
				CalModel_Update(actor->calmodel,1000);
			}
            else {
                /* Schmurk: we explicitly go on idle here to avoid weird
                 * flickering when actors appear */
                set_on_idle(actor, attached);
                /* CalModel_Update(actor->calmodel,0); */
            }
			build_actor_bounding_box(actor);
			if (use_animation_program)
			{
				set_transformation_buffers(actor);
			}
			switch (weapon)
			{
				case SWORD_1_FIRE:
				case SWORD_2_FIRE:
				case SWORD_3_FIRE:
				case SWORD_4_FIRE:
				case SWORD_4_THERMAL:
				case SWORD_5_FIRE:
				case SWORD_5_THERMAL:
				case SWORD_6_FIRE:
				case SWORD_6_THERMAL:
				case SWORD_7_FIRE:
				case SWORD_7_THERMAL:
					ec_create_sword_of_fire(actor, (poor_man ? 6 : 10));
					break;
				case SWORD_2_COLD:
				case SWORD_3_COLD:
				case SWORD_4_COLD:
				case SWORD_5_COLD:
				case SWORD_6_COLD:
				case SWORD_7_COLD:
					ec_create_sword_of_ice(actor, (poor_man ? 6 : 10));
					break;
				case SWORD_3_MAGIC:
				case SWORD_4_MAGIC:
				case SWORD_5_MAGIC:
				case SWORD_6_MAGIC:
				case SWORD_7_MAGIC:
					ec_create_sword_of_magic(actor, (poor_man ? 6 : 10));
					break;
				case SWORD_EMERALD_CLAYMORE:
					ec_create_sword_emerald_claymore(actor, (poor_man ? 6 : 10));
					break;
				case SWORD_CUTLASS:
					ec_create_sword_cutlass(actor, (poor_man ? 6 : 10));
					break;
				case SWORD_SUNBREAKER:
					ec_create_sword_sunbreaker(actor, (poor_man ? 6 : 10));
					break;
				case SWORD_ORC_SLAYER:
					ec_create_sword_orc_slayer(actor, (poor_man ? 6 : 10));
					break;
				case SWORD_EAGLE_WING:
					ec_create_sword_eagle_wing(actor, (poor_man ? 6 : 10));
					break;
				case SWORD_JAGGED_SABER:
					ec_create_sword_jagged_saber(actor, (poor_man ? 6 : 10));
					break;
				case STAFF_3: // staff of protection
					ec_create_staff_of_protection(actor, (poor_man ? 6 : 10));
					break;
				case STAFF_4: // staff of the mage
					ec_create_staff_of_the_mage(actor, (poor_man ? 6 : 10));
					break;
			}
		}
	}
	else
	{
		actor->calmodel=NULL;
	}

	/* //DEBUG
	if (actor->actor_id==yourself) {
		//actor_wear_item(actor->actor_id,KIND_OF_WEAPON,SWORD_1);
		//unwear_item_from_actor(actor->actor_id,KIND_OF_WEAPON);
		//actor_wear_item(actor->actor_id,KIND_OF_WEAPON,SWORD_2);
		//actor_wear_item(actor->actor_id,KIND_OF_HELMET,HELMET_IRON);
	}*/

	if (frame==frame_combat_idle)
	{
		// If fighting turn the horse and the fighter
		actor->fighting=1;
		if (attached)
		{
			//printf("A fighting horse! (%s)\n", actor->actor_name);
			attached->fighting=1;
			if (actor_weapon(actor)->turn_horse)
				rotate_actor_and_horse_locked(actor, attached, -1);
		}
	}

	if (actor_id == yourself)
		reset_camera_at_next_update = 1;
	update_actor_buffs_locked(actor, attached, buffs);

	check_if_new_actor_last_summoned(actor);

	add_actor_to_list(actor, attached);

#ifdef EXTRA_DEBUG
	ERR();
#endif
}

actor * add_actor_interface(float x, float y, float z_rot, float scale, int actor_type, const char *playername,
		short skin, short hair, short eyes, short shirt, short pants, short boots, short head)
{
	enhanced_actor * this_actor=calloc(1,sizeof(enhanced_actor));
	actor * a;

	//get the torso
	safe_strncpy(this_actor->arms_tex,actors_defs[actor_type].shirt[shirt].arms_name,sizeof(this_actor->arms_tex));
	safe_strncpy(this_actor->arms_mask,actors_defs[actor_type].shirt[shirt].arms_mask,sizeof(this_actor->arms_mask));
	safe_strncpy(this_actor->torso_tex,actors_defs[actor_type].shirt[shirt].torso_name,sizeof(this_actor->torso_tex));
	safe_strncpy(this_actor->torso_mask,actors_defs[actor_type].shirt[shirt].torso_mask,sizeof(this_actor->torso_mask));
	safe_strncpy(this_actor->hands_tex,actors_defs[actor_type].skin[skin].hands_name,sizeof(this_actor->hands_tex));
	safe_strncpy(this_actor->head_tex,actors_defs[actor_type].skin[skin].head_name,sizeof(this_actor->head_tex));
	safe_strncpy(this_actor->hair_tex,actors_defs[actor_type].hair[hair].hair_name,sizeof(this_actor->hair_tex));
#ifdef NEW_EYES
	safe_strncpy(this_actor->eyes_tex,actors_defs[actor_type].eyes[eyes].eyes_name,sizeof(this_actor->eyes_tex));
#endif
	safe_strncpy(this_actor->boots_tex,actors_defs[actor_type].boots[boots].boots_name,sizeof(this_actor->boots_tex));
	safe_strncpy(this_actor->boots_mask,actors_defs[actor_type].boots[boots].boots_mask,sizeof(this_actor->boots_mask));
	safe_strncpy(this_actor->pants_tex,actors_defs[actor_type].legs[pants].legs_name,sizeof(this_actor->pants_tex));
	safe_strncpy(this_actor->pants_mask,actors_defs[actor_type].legs[pants].legs_mask,sizeof(this_actor->pants_mask));

	a = create_enhanced_actor(this_actor, x*0.5f, y*0.5f, 0.00000001f, z_rot, scale, 0, NULL);

	a->x_tile_pos=x;
	a->y_tile_pos=y;
	a->actor_type=actor_type;
	//test only
	a->max_health=20;
	a->cur_health=20;

	a->stop_animation=1;//helps when the actor is dead...
	a->kind_of_actor=HUMAN;

	safe_snprintf(a->actor_name, sizeof(a->actor_name), playername);

	if (actors_defs[actor_type].coremodel!=NULL) {
		a->calmodel=model_new(actors_defs[actor_type].coremodel);

		if (a->calmodel!=NULL) {
			//Setup cal3d model
			//a->calmodel=CalModel_New(actors_defs[actor_type].coremodel);
			//Attach meshes
			model_attach_mesh(a, actors_defs[actor_type].head[head].mesh_index);
			model_attach_mesh(a, actors_defs[actor_type].shirt[shirt].mesh_index);
			model_attach_mesh(a, actors_defs[actor_type].legs[pants].mesh_index);
			model_attach_mesh(a, actors_defs[actor_type].boots[boots].mesh_index);

			a->body_parts->torso_meshindex=actors_defs[actor_type].shirt[shirt].mesh_index;
			a->body_parts->legs_meshindex=actors_defs[actor_type].legs[pants].mesh_index;
			a->body_parts->head_meshindex=actors_defs[actor_type].head[head].mesh_index;
			a->body_parts->boots_meshindex=actors_defs[actor_type].boots[boots].mesh_index;

			assert(!"Using old client data" || actors_defs[actor_type].neck != NULL);
			a->body_parts->neck_meshindex=actors_defs[actor_type].neck[NECK_NONE].mesh_index;
			a->body_parts->helmet_meshindex=actors_defs[actor_type].helmet[HELMET_NONE].mesh_index;
			a->body_parts->cape_meshindex=actors_defs[actor_type].cape[CAPE_NONE].mesh_index;
			a->body_parts->shield_meshindex=actors_defs[actor_type].shield[SHIELD_NONE].mesh_index;

			a->cur_anim.anim_index=-1;
			a->anim_time=0.0;
			a->last_anim_update= cur_time;
			CalModel_Update(a->calmodel,0);
			build_actor_bounding_box(a);
			if (use_animation_program)
			{
				set_transformation_buffers(a);
			}
		}
	} else a->calmodel=NULL;

	add_actor_to_list(a, NULL);

	return a;
}

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"

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

//return the ID (number in the actors_list[]) of the new allocated actor
int add_enhanced_actor(enhanced_actor *this_actor,char * frame_name,float x_pos, float y_pos,
					   float z_pos, float z_rot, int actor_id)
{
	int texture_id;
	int i;
	int k;
	actor *our_actor;
	no_bounding_box=1;
#ifdef EXTRA_DEBUG
	ERR();
#endif

	//get the skin
	texture_id= load_bmp8_enhanced_actor(this_actor, 255);

	our_actor = calloc(1, sizeof(actor));

	memset(our_actor->current_displayed_text, 0, MAX_CURRENT_DISPLAYED_TEXT_LEN);
	our_actor->current_displayed_text_time_left =  0;

	our_actor->texture_id=texture_id;
	our_actor->is_enhanced_model=1;
	our_actor->actor_id=actor_id;
	our_actor->x_pos=x_pos;
	our_actor->y_pos=y_pos;
	our_actor->z_pos=z_pos;

	our_actor->x_speed=0;
	our_actor->y_speed=0;
	our_actor->z_speed=0;

	our_actor->x_rot=0;
	our_actor->y_rot=0;
	our_actor->z_rot=z_rot;
	
	//reset the script related things
	our_actor->move_x_speed=0;
	our_actor->move_y_speed=0;
	our_actor->move_z_speed=0;
	our_actor->rotate_x_speed=0;
	our_actor->rotate_y_speed=0;
	our_actor->rotate_z_speed=0;
	our_actor->movement_frames_left=0;
	our_actor->moving=0;
	our_actor->rotating=0;
	our_actor->busy=0;
	our_actor->last_command=nothing;
	
	//clear the que
	for(k=0; k<MAX_CMD_QUEUE; k++)	our_actor->que[k]=nothing;

//	our_actor->model_data=0;
	my_strcp(our_actor->cur_frame,frame_name);
	our_actor->stand_idle=0;
	our_actor->sit_idle=0;
	our_actor->body_parts=this_actor;

	//find a free spot, in the actors_list
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	for(i=0; i<max_actors; i++)
		{
			if(!actors_list[i])	break;
		}
	
	actors_list[i]=our_actor;
	
	if(i>=max_actors)max_actors=i+1;
	
	no_bounding_box=0;
	//Will be unlocked later
	
	return i;
}

float cal_get_maxz(actor *act)
{
	float points[1024][3];
	int nrPoints;
	struct CalSkeleton *skel;
	float maxz;
	int i;

	skel=CalModel_GetSkeleton(act->calmodel);
	nrPoints = CalSkeleton_GetBonePoints(skel,&points[0][0]);
	maxz=points[0][2];
	for (i=1;i<nrPoints;++i) if (maxz<points[i][2]) maxz=points[i][2];
	return maxz;
}
		

void draw_enhanced_actor(actor * actor_id)
{
	//int i=0;
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	//int texture_id;
	char *cur_frame;
	float healtbar_z=0;
	bind_texture_id(actor_id->texture_id);
	cur_frame=actor_id->tmp.cur_frame;

	if (actors_defs[actor_id->actor_type].coremodel!=NULL){
		healtbar_z=cal_get_maxz(actor_id)+0.2;
	}
	
	if(actor_id->actor_id==yourself)sitting=healtbar_z/2.0f;

	glPushMatrix();//we don't want to affect the rest of the scene
	x_pos=actor_id->tmp.x_pos;
	y_pos=actor_id->tmp.y_pos;
	z_pos=actor_id->tmp.z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->tmp.y_tile_pos*tile_map_size_x*6+actor_id->tmp.x_tile_pos]*0.2f;

	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);

	x_rot=actor_id->tmp.x_rot;
	y_rot=actor_id->tmp.y_rot;
	z_rot=actor_id->tmp.z_rot;
	z_rot+=180;//test
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if (actors_defs[actor_id->actor_type].coremodel!=NULL) {
		cal_render_actor(actor_id);
	} 

	//////
	glPopMatrix();//restore the scene
	//now, draw their damage

	glPushMatrix();
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	glRotatef(-rz, 0.0f, 0.0f, 1.0f);

	draw_actor_banner(actor_id, healtbar_z);

	glPopMatrix();//we don't want to affect the rest of the scene
}

void unwear_item_from_actor(int actor_id,Uint8 which_part)
{
	int i;

	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				if(actors_list[i]->actor_id==actor_id)
					{
						if(which_part==KIND_OF_WEAPON)
							{
								if(actors_list[i]->cur_weapon == GLOVE_FUR || actors_list[i]->cur_weapon == GLOVE_LEATHER){
									my_strcp(actors_list[i]->body_parts->hands_tex, actors_list[i]->body_parts->hands_tex_save);
									glDeleteTextures(1,&actors_list[i]->texture_id);
									actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
																	
								}
								CalModel_DetachMesh(actors_list[i]->calmodel,actors_defs[actors_list[i]->actor_type].weapon[actors_list[i]->cur_weapon].mesh_index);
								actors_list[i]->body_parts->weapon_fn[0]=0;
								actors_list[i]->body_parts->weapon_tex[0]=0;
								actors_list[i]->cur_weapon = WEAPON_NONE;
								return;
							}

						if(which_part==KIND_OF_SHIELD)
							{
							    	CalModel_DetachMesh(actors_list[i]->calmodel,actors_list[i]->body_parts->shield_meshindex);
								actors_list[i]->body_parts->shield_fn[0]=0;
								actors_list[i]->body_parts->shield_tex[0]=0;
								actors_list[i]->cur_shield = SHIELD_NONE;
								return;
							}

						if(which_part==KIND_OF_CAPE)
							{
							    	CalModel_DetachMesh(actors_list[i]->calmodel,actors_list[i]->body_parts->cape_meshindex);
								actors_list[i]->body_parts->cape_fn[0]=0;
								actors_list[i]->body_parts->cape_tex[0]=0;
								return;
							}

						if(which_part==KIND_OF_HELMET)
							{
					     		    	CalModel_DetachMesh(actors_list[i]->calmodel,actors_list[i]->body_parts->helmet_meshindex);
								actors_list[i]->body_parts->helmet_fn[0]=0;
								actors_list[i]->body_parts->helmet_tex[0]=0;
								return;
							}
						return;
					}
		}

}

#ifdef CUSTOM_LOOK
void custom_path(char * path, char * custom1, char * custom2) {
	unsigned char buffer[256];
	FILE * fh;

	/* Check if custom1 has path readable */
	my_strcp(buffer, custom1);
	my_strcat(buffer, path);
	fh = fopen(buffer, "r");
	if (fh) {
		fclose(fh);
		my_strcp(path, buffer);
		return;
	}

	/* Check if custom2 has path readable */
	my_strcp(buffer, custom2);
	my_strcat(buffer, path);
	fh = fopen(buffer, "r");
	if (fh) {
		fclose(fh);
		my_strcp(path, buffer);
		return;
	}

	/* leave as is */
	return;
}
#endif

void actor_wear_item(int actor_id,Uint8 which_part, Uint8 which_id)
{
	int i;
#ifdef CUSTOM_LOOK
	unsigned char playerpath[256], guildpath[256];
#endif
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				if(actors_list[i]->actor_id==actor_id)
					{
#ifdef CUSTOM_LOOK
						snprintf(guildpath, sizeof(guildpath), "custom/guild/%d/", actors_list[i]->body_parts->guild_id);
						snprintf(playerpath, sizeof(playerpath), "custom/player/%d/", actors_list[i]->body_parts->uniq_id);
#endif
						if(which_part==KIND_OF_WEAPON)
							{
								if(which_id == GLOVE_FUR || which_id == GLOVE_LEATHER){
									my_strcp(actors_list[i]->body_parts->hands_tex, actors_defs[actors_list[i]->actor_type].weapon[which_id].skin_name);
#ifdef CUSTOM_LOOK
									custom_path(actors_list[i]->body_parts->hands_tex, playerpath, guildpath);
#endif
								}
								my_strcp(actors_list[i]->body_parts->weapon_tex,actors_defs[actors_list[i]->actor_type].weapon[which_id].skin_name);
#ifdef CUSTOM_LOOK
								custom_path(actors_list[i]->body_parts->weapon_tex, playerpath, guildpath);
#endif
								my_strcp(actors_list[i]->body_parts->weapon_fn,actors_defs[actors_list[i]->actor_type].weapon[which_id].model_name);
								CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actors_list[i]->actor_type].weapon[which_id].mesh_index);
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								actors_list[i]->cur_weapon=which_id;

								actors_list[i]->body_parts->weapon_glow=actors_defs[actors_list[i]->actor_type].weapon[which_id].glow;
								return;
							}

						if(which_part==KIND_OF_SHIELD)
							{
								my_strcp(actors_list[i]->body_parts->shield_tex,actors_defs[actors_list[i]->actor_type].shield[which_id].skin_name);
#ifdef CUSTOM_LOOK
								custom_path(actors_list[i]->body_parts->shield_tex, playerpath, guildpath);
#endif
								my_strcp(actors_list[i]->body_parts->shield_fn,actors_defs[actors_list[i]->actor_type].shield[which_id].model_name);
								CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actors_list[i]->actor_type].shield[which_id].mesh_index);
                                actors_list[i]->body_parts->shield_meshindex=actors_defs[actors_list[i]->actor_type].shield[which_id].mesh_index;
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								actors_list[i]->cur_shield=which_id;
								return;
							}

						if(which_part==KIND_OF_CAPE)
							{
								my_strcp(actors_list[i]->body_parts->cape_tex,actors_defs[actors_list[i]->actor_type].cape[which_id].skin_name);
#ifdef CUSTOM_LOOK
								custom_path(actors_list[i]->body_parts->cape_tex, playerpath, guildpath);
#endif
								my_strcp(actors_list[i]->body_parts->cape_fn,actors_defs[actors_list[i]->actor_type].cape[which_id].model_name);
								CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actors_list[i]->actor_type].cape[which_id].mesh_index);
								actors_list[i]->body_parts->cape_meshindex=actors_defs[actors_list[i]->actor_type].cape[which_id].mesh_index;
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								return;
							}

						if(which_part==KIND_OF_HELMET)
							{
								my_strcp(actors_list[i]->body_parts->helmet_tex,actors_defs[actors_list[i]->actor_type].helmet[which_id].skin_name);
#ifdef CUSTOM_LOOK
								custom_path(actors_list[i]->body_parts->helmet_tex, playerpath, guildpath);
#endif
								my_strcp(actors_list[i]->body_parts->helmet_fn,actors_defs[actors_list[i]->actor_type].helmet[which_id].model_name);
								CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actors_list[i]->actor_type].helmet[which_id].mesh_index);
								actors_list[i]->body_parts->helmet_meshindex=actors_defs[actors_list[i]->actor_type].helmet[which_id].mesh_index;
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								return;
							}

						if(which_part==KIND_OF_BODY_ARMOR)
							{
								my_strcp(actors_list[i]->body_parts->arms_tex,actors_defs[actors_list[i]->actor_type].shirt[which_id].arms_name);
								my_strcp(actors_list[i]->body_parts->torso_tex,actors_defs[actors_list[i]->actor_type].shirt[which_id].torso_name);
#ifdef CUSTOM_LOOK
								custom_path(actors_list[i]->body_parts->arms_tex, playerpath, guildpath);
								custom_path(actors_list[i]->body_parts->torso_tex, playerpath, guildpath);
#endif
								my_strcp(actors_list[i]->body_parts->torso_fn,actors_defs[actors_list[i]->actor_type].shirt[which_id].model_name);
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								return;
							}
						if(which_part==KIND_OF_LEG_ARMOR)
							{
								my_strcp(actors_list[i]->body_parts->pants_tex,actors_defs[actors_list[i]->actor_type].legs[which_id].legs_name);
#ifdef CUSTOM_LOOK
								custom_path(actors_list[i]->body_parts->pants_tex, playerpath, guildpath);
#endif
								my_strcp(actors_list[i]->body_parts->legs_fn,actors_defs[actors_list[i]->actor_type].legs[which_id].model_name);
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								return;
							}

						if(which_part==KIND_OF_BOOT_ARMOR)
							{
								my_strcp(actors_list[i]->body_parts->boots_tex,actors_defs[actors_list[i]->actor_type].boots[which_id].boots_name);
#ifdef CUSTOM_LOOK
								custom_path(actors_list[i]->body_parts->boots_tex, playerpath, guildpath);
#endif
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								return;
							}
						return;
					}
		}
}

void add_enhanced_actor_from_server(char * in_data)
{
	short actor_id;
	short x_pos;
	short y_pos;
	short z_pos;
	short z_rot;
	short max_health;
	short cur_health;
	Uint8 actor_type;
	Uint8 skin;
	Uint8 hair;
	Uint8 shirt;
	Uint8 pants;
	Uint8 boots;
	Uint8 frame;
	Uint8 cape;
	Uint8 head;
	Uint8 shield;
	Uint8 weapon;
	Uint8 helmet;
	int i;
	int dead=0;
	int kind_of_actor;
	enhanced_actor *this_actor;
#ifdef CUSTOM_LOOK
	unsigned char playerpath[256], guildpath[256];
	Uint32 uniq_id, guild_id; // - Post ported.... We'll come up with something later...
#endif

	char cur_frame[20];
	double f_x_pos,f_y_pos,f_z_pos,f_z_rot;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	actor_id=SDL_SwapLE16(*((short *)(in_data)));
	x_pos=SDL_SwapLE16(*((short *)(in_data+2)));
	y_pos=SDL_SwapLE16(*((short *)(in_data+4)));
	z_pos=SDL_SwapLE16(*((short *)(in_data+6)));
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
	frame=*(in_data+22);
	max_health=SDL_SwapLE16(*((short *)(in_data+23)));
	cur_health=SDL_SwapLE16(*((short *)(in_data+25)));
	kind_of_actor=*(in_data+27);
#if defined CUSTOM_LOOK && defined UID
	uniq_id = SDL_SwapLE32(*((Uint32*)(in_data+28)));
#endif

	//translate from tile to world
	f_x_pos=x_pos*0.5;
	f_y_pos=y_pos*0.5;
	f_z_pos=z_pos;
	f_z_rot=z_rot;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	//get the current frame
	switch(frame) {
	case frame_walk:
		my_strcp(cur_frame,actors_defs[actor_type].walk_frame);break;
	case frame_run:
		my_strcp(cur_frame,actors_defs[actor_type].run_frame);break;
	case frame_die1:
		my_strcp(cur_frame,actors_defs[actor_type].die1_frame);
		dead=1;
		break;
	case frame_die2:
		my_strcp(cur_frame,actors_defs[actor_type].die2_frame);
		dead=1;
		break;
	case frame_pain1:
		my_strcp(cur_frame,actors_defs[actor_type].pain1_frame);break;
	case frame_pain2:
		my_strcp(cur_frame,actors_defs[actor_type].pain2_frame);break;
	case frame_pick:
		my_strcp(cur_frame,actors_defs[actor_type].pick_frame);break;
	case frame_drop:
		my_strcp(cur_frame,actors_defs[actor_type].drop_frame);break;
	case frame_idle:
		my_strcp(cur_frame,actors_defs[actor_type].idle_frame);break;
	case frame_sit_idle:
		my_strcp(cur_frame,actors_defs[actor_type].idle_sit_frame);break;
	case frame_harvest:
		my_strcp(cur_frame,actors_defs[actor_type].harvest_frame);break;
	case frame_cast:
		my_strcp(cur_frame,actors_defs[actor_type].attack_cast_frame);break;
	case frame_attack_up_1:
		my_strcp(cur_frame,actors_defs[actor_type].attack_up_1_frame);break;
	case frame_attack_up_2:
		my_strcp(cur_frame,actors_defs[actor_type].attack_up_2_frame);break;
	case frame_attack_up_3:
		my_strcp(cur_frame,actors_defs[actor_type].attack_up_3_frame);break;
	case frame_attack_up_4:
		my_strcp(cur_frame,actors_defs[actor_type].attack_up_4_frame);break;
	case frame_attack_down_1:
		my_strcp(cur_frame,actors_defs[actor_type].attack_down_1_frame);break;
	case frame_attack_down_2:
		my_strcp(cur_frame,actors_defs[actor_type].attack_down_2_frame);break;
	case frame_combat_idle:
		my_strcp(cur_frame,actors_defs[actor_type].combat_idle_frame);break;
	default:
		{
			char str[120];
#ifdef UID
			snprintf (str, sizeof(str), "%s %d - %s\n", unknown_frame, frame, &in_data[32]);
#else
			snprintf(str, sizeof(str), "%s %d - %s\n",unknown_frame,frame,&in_data[28]);
#endif
			log_error(str);
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
				{
					if(actors_list[i]->actor_id==actor_id)
						{
							char str[256];
#ifdef UID
							snprintf (str, sizeof(str), "%s %d = %s => %s\n", duplicate_actors_str, actor_id, actors_list[i]->actor_name, &in_data[32]);
#else
							snprintf(str, sizeof(str), "%s %d = %s => %s\n",duplicate_actors_str,actor_id, actors_list[i]->actor_name ,&in_data[28]);
#endif
							log_error(str);
							destroy_actor(actors_list[i]->actor_id);//we don't want two actors with the same ID
							i--;// last actor was put here, he needs to be checked too
						}
#ifdef UID
					else if(kind_of_actor==COMPUTER_CONTROLLED_HUMAN && (actors_list[i]->kind_of_actor==COMPUTER_CONTROLLED_HUMAN || actors_list[i]->kind_of_actor==PKABLE_COMPUTER_CONTROLLED) && !my_strcompare(&in_data[32], actors_list[i]->actor_name))
#else
					else if(kind_of_actor==COMPUTER_CONTROLLED_HUMAN && (actors_list[i]->kind_of_actor==COMPUTER_CONTROLLED_HUMAN || actors_list[i]->kind_of_actor==PKABLE_COMPUTER_CONTROLLED) && !my_strcompare(&in_data[28], actors_list[i]->actor_name))
#endif
						{
							char str[256];
#ifdef UID
							snprintf (str, sizeof(str), "%s(%d) = %s => %s\n", duplicate_npc_actor, actor_id, actors_list[i]->actor_name, &in_data[32]);
#else
							snprintf(str, sizeof(str), "%s(%d) = %s => %s\n",duplicate_npc_actor,actor_id, actors_list[i]->actor_name ,&in_data[28]);
#endif
							log_error(str);
							destroy_actor(actors_list[i]->actor_id);//we don't want two actors with the same ID
							i--;// last actor was put here, he needs to be checked too
						}
				}
		}

#ifdef EXTRA_DEBUG
	ERR();
#endif

	this_actor=calloc(1,sizeof(enhanced_actor));

#ifdef CUSTOM_LOOK
// FIXME: NEW_CLIENT should have player AND guild id, otherwise BOTH should be guessed (lachesis)
	/* Guess player and guild id */
	{
		/* get the name string into a working buffer */
		unsigned char buffer[256], *name, *guild;
#ifdef UID
		my_strncp(buffer,&in_data[32],sizeof(buffer));
#else
		my_strncp(buffer,&in_data[28],sizeof(buffer));
#endif
		
		/* skip leading color codes */
		for (name = buffer; *name && (*name >= 127 + c_lbound) && (*name <= 127 + c_ubound); name++);
		
		/* search for string end or color mark */
		for (guild = name; *guild && ((*guild < 127 + c_lbound) || (*guild > 127 + c_ubound)); guild++);
		if (*guild) {
			/* separate the two strings */
			*guild = 0;
			guild++;
		}

		/* perform case insensitive comparison */
		my_tolower(name);
		my_tolower(guild);
		
#ifndef UID
		uniq_id = 0;
#endif

		/* guild tags of Erisian Power Foundation (guild 82) */
		if (!strcmp(guild, "chao") || !strcmp(guild, "eris") || ! strcmp(guild, "kali")) {
			guild_id = 82;
		} else {
			guild_id = 0;
		}
	}

	/* precompute the paths to custom files */
	snprintf(playerpath, 256, "custom/player/%d/", uniq_id);
	snprintf(guildpath, 256, "custom/guild/%d/", guild_id);

	/* store the ids */
	this_actor->uniq_id = uniq_id;
	this_actor->guild_id = guild_id;
#endif // CUSTOM_LOOK

	//get the torso
	my_strcp(this_actor->arms_tex,actors_defs[actor_type].shirt[shirt].arms_name);
	my_strcp(this_actor->torso_tex,actors_defs[actor_type].shirt[shirt].torso_name);
	my_strcp(this_actor->torso_fn,actors_defs[actor_type].shirt[shirt].model_name);
	//skin
	my_strcp(this_actor->hands_tex,actors_defs[actor_type].skin[skin].hands_name);
	my_strcp(this_actor->hands_tex_save,actors_defs[actor_type].skin[skin].hands_name);
	my_strcp(this_actor->head_tex,actors_defs[actor_type].skin[skin].head_name);
	//hair
	my_strcp(this_actor->hair_tex,actors_defs[actor_type].hair[hair].hair_name);
	//boots
	my_strcp(this_actor->boots_tex,actors_defs[actor_type].boots[boots].boots_name);
	//legs
	my_strcp(this_actor->pants_tex,actors_defs[actor_type].legs[pants].legs_name);
	my_strcp(this_actor->legs_fn,actors_defs[actor_type].legs[pants].model_name);

#ifdef CUSTOM_LOOK
	//torso
	custom_path(this_actor->arms_tex, playerpath, guildpath);
	custom_path(this_actor->torso_tex, playerpath, guildpath);
	custom_path(this_actor->torso_fn, playerpath, guildpath);
	//skin
	custom_path(this_actor->hands_tex, playerpath, guildpath);
	custom_path(this_actor->hands_tex_save, playerpath, guildpath);
	custom_path(this_actor->head_tex, playerpath, guildpath);
	//hair
	custom_path(this_actor->hair_tex, playerpath, guildpath);
	//boots
	custom_path(this_actor->boots_tex, playerpath, guildpath);
	//legs
	custom_path(this_actor->pants_tex, playerpath, guildpath);
	custom_path(this_actor->legs_fn, playerpath, guildpath);
#endif

	//cape
	if(cape!=CAPE_NONE)
		{
			my_strncp(this_actor->cape_tex,actors_defs[actor_type].cape[cape].skin_name,sizeof(this_actor->cape_tex));
			my_strncp(this_actor->cape_fn,actors_defs[actor_type].cape[cape].model_name,sizeof(this_actor->cape_fn));
#ifdef CUSTOM_LOOK
			custom_path(this_actor->cape_tex, playerpath, guildpath);
#endif
		}
	else
		{
			my_strncp(this_actor->cape_tex,"",sizeof(this_actor->cape_tex));
			my_strncp(this_actor->cape_fn,"",sizeof(this_actor->cape_fn));
		}
	//head
	my_strncp(this_actor->head_fn,actors_defs[actor_type].head[head].model_name,sizeof(this_actor->head_fn));
    
	//shield
	if(shield!=SHIELD_NONE)
		{
			my_strncp(this_actor->shield_tex,actors_defs[actor_type].shield[shield].skin_name,sizeof(this_actor->shield_tex));
			my_strncp(this_actor->shield_fn,actors_defs[actor_type].shield[shield].model_name,sizeof(this_actor->shield_fn));
#ifdef CUSTOM_LOOK
			custom_path(this_actor->shield_tex, playerpath, guildpath);
#endif
		}
	else
		{
			my_strncp(this_actor->shield_tex,"",sizeof(this_actor->shield_tex));
			my_strncp(this_actor->shield_fn,"",sizeof(this_actor->shield_fn));
		}

	my_strncp(this_actor->weapon_tex,actors_defs[actor_type].weapon[weapon].skin_name,sizeof(this_actor->weapon_tex));
#ifdef CUSTOM_LOOK
	custom_path(this_actor->weapon_tex, playerpath, guildpath);
#endif
	my_strncp(this_actor->weapon_fn,actors_defs[actor_type].weapon[weapon].model_name,sizeof(this_actor->weapon_fn));
	this_actor->weapon_glow=actors_defs[actor_type].weapon[weapon].glow;
	if(weapon == GLOVE_FUR || weapon == GLOVE_LEATHER){
		my_strncp(this_actor->hands_tex, actors_defs[actor_type].weapon[weapon].skin_name,sizeof(this_actor->hands_tex));
#ifdef CUSTOM_LOOK
		custom_path(this_actor->hands_tex, playerpath, guildpath);
#endif
	}

	//helmet
	if(helmet!=HELMET_NONE)
		{
			my_strncp(this_actor->helmet_tex,actors_defs[actor_type].helmet[helmet].skin_name,sizeof(this_actor->helmet_tex));
#ifdef CUSTOM_LOOK
			custom_path(this_actor->helmet_tex, playerpath, guildpath);
#endif
			my_strncp(this_actor->helmet_fn,actors_defs[actor_type].helmet[helmet].model_name,sizeof(this_actor->helmet_fn));
		}
	else
		{
			my_strncp(this_actor->helmet_tex,"",sizeof(this_actor->helmet_tex));
			my_strncp(this_actor->helmet_fn,"",sizeof(this_actor->helmet_fn));
		}

	i=add_enhanced_actor(this_actor,cur_frame,f_x_pos,f_y_pos,f_z_pos,f_z_rot,actor_id);
	
#ifdef EXTRA_DEBUG
	ERR();
#endif
	//The actors list is already locked here
	actors_list[i]->x_tile_pos=x_pos;
	actors_list[i]->y_tile_pos=y_pos;
	actors_list[i]->actor_type=actor_type;
	actors_list[i]->damage=0;
	actors_list[i]->damage_ms=0;
	actors_list[i]->sitting=0;
	actors_list[i]->fighting=0;
	//test only
	actors_list[i]->max_health=max_health;
	actors_list[i]->cur_health=cur_health;
	if(frame==frame_sit_idle)
		{
			if(actors_list[i]->actor_id==yourself)you_sit_down();
			actors_list[i]->sitting=1;
		}
	else if(frame==frame_combat_idle)
		actors_list[i]->fighting=1;

	//ghost or not?
	actors_list[i]->ghost=0;

	actors_list[i]->dead=dead;
	actors_list[i]->stop_animation=1;//helps when the actor is dead...
	actors_list[i]->cur_weapon=weapon;
	actors_list[i]->kind_of_actor=kind_of_actor;
	if(strlen(&in_data[28]) >= 30)
		{
			char str[120];
			snprintf(str, sizeof(str), "%s (%d): %s/%d\n", bad_actor_name_length, actors_list[i]->actor_type,&in_data[28], (int)strlen(&in_data[28]));
			log_error(str);
		}
	else 	{
			my_strncp(actors_list[i]->actor_name,&in_data[28],sizeof(actors_list[i]->actor_name));
			if(caps_filter && my_isupper(actors_list[i]->actor_name, -1)) my_tolower(actors_list[i]->actor_name);
		}

	if (actors_defs[actor_type].coremodel!=NULL) 
		actors_list[i]->calmodel=CalModel_New(actors_defs[actor_type].coremodel);

	if (actors_defs[actor_type].coremodel!=NULL) {
		//Setup cal3d model
		//actors_list[i]->calmodel=CalModel_New(actors_defs[actor_type].coremodel);
		//Attach meshes
		CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].head[head].mesh_index);
		CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].shirt[shirt].mesh_index);
		CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].legs[pants].mesh_index);

		if (cape!=CAPE_NONE) CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].cape[cape].mesh_index);
		if (helmet!=HELMET_NONE) CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].helmet[helmet].mesh_index);
		if (weapon!=WEAPON_NONE) CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].weapon[weapon].mesh_index);
		if (shield!=SHIELD_NONE) CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].shield[shield].mesh_index);

		actors_list[i]->body_parts->helmet_meshindex=actors_defs[actor_type].helmet[helmet].mesh_index;
		actors_list[i]->body_parts->cape_meshindex=actors_defs[actor_type].cape[cape].mesh_index;
		actors_list[i]->body_parts->shield_meshindex=actors_defs[actor_type].shield[shield].mesh_index;

		actors_list[i]->cur_anim.anim_index=-1;
		actors_list[i]->anim_time=0.0;
		
		if(dead){
			cal_actor_set_anim(i, actors_defs[actors_list[i]->actor_type].cal_die1_frame);
			actors_list[i]->stop_animation=1;
			CalModel_Update(actors_list[i]->calmodel,1000);
		} else  CalModel_Update(actors_list[i]->calmodel,0);
	}

	/* //DEBUG
	if (actors_list[i]->actor_id==yourself) {
		//actor_wear_item(actors_list[i]->actor_id,KIND_OF_WEAPON,SWORD_1);
		//unwear_item_from_actor(actors_list[i]->actor_id,KIND_OF_WEAPON);
		//actor_wear_item(actors_list[i]->actor_id,KIND_OF_WEAPON,SWORD_2);
		//actor_wear_item(actors_list[i]->actor_id,KIND_OF_HELMET,HELMET_IRON);
	}*/

	UNLOCK_ACTORS_LISTS();  //unlock it
#ifdef EXTRA_DEBUG
	ERR();
#endif
}

actor * add_actor_interface(float x, float y, float z_rot, int actor_type, short skin, short hair,
				short shirt, short pants, short boots, short head)
{
	enhanced_actor * this_actor=calloc(1,sizeof(enhanced_actor));
	actor * a;
	
//get the torso         
	my_strcp(this_actor->arms_tex,actors_defs[actor_type].shirt[shirt].arms_name);
	my_strcp(this_actor->torso_tex,actors_defs[actor_type].shirt[shirt].torso_name);
	my_strcp(this_actor->torso_fn,actors_defs[actor_type].shirt[shirt].model_name);
	my_strcp(this_actor->hands_tex,actors_defs[actor_type].skin[skin].hands_name);
	my_strcp(this_actor->head_tex,actors_defs[actor_type].skin[skin].head_name);
	my_strcp(this_actor->hair_tex,actors_defs[actor_type].hair[hair].hair_name);
	my_strcp(this_actor->boots_tex,actors_defs[actor_type].boots[boots].boots_name);
	my_strcp(this_actor->pants_tex,actors_defs[actor_type].legs[pants].legs_name);
	my_strcp(this_actor->legs_fn,actors_defs[actor_type].legs[pants].model_name);
	my_strcp(this_actor->head_fn,actors_defs[actor_type].head[head].model_name);

	a=actors_list[add_enhanced_actor(this_actor, actors_defs[actor_type].idle_frame, x*0.5f, y*0.5f, 0.00000001f, z_rot, 0)];

	a->x_tile_pos=x;
	a->y_tile_pos=y;
	a->actor_type=actor_type;
	//test only
	a->max_health=20;
	a->cur_health=20;

	a->stop_animation=1;//helps when the actor is dead...
	a->kind_of_actor=HUMAN;
	
	strncpy(a->actor_name,"Player",sizeof(a->actor_name));
	
	if (actors_defs[actor_type].coremodel!=NULL) 
		a->calmodel=CalModel_New(actors_defs[actor_type].coremodel);

	if (actors_defs[actor_type].coremodel!=NULL) {
		//Setup cal3d model
		//a->calmodel=CalModel_New(actors_defs[actor_type].coremodel);
		//Attach meshes
		CalModel_AttachMesh(a->calmodel,actors_defs[actor_type].head[head].mesh_index);
		CalModel_AttachMesh(a->calmodel,actors_defs[actor_type].shirt[shirt].mesh_index);
		CalModel_AttachMesh(a->calmodel,actors_defs[actor_type].legs[pants].mesh_index);

		a->body_parts->helmet_meshindex=actors_defs[actor_type].helmet[HELMET_NONE].mesh_index;
		a->body_parts->cape_meshindex=actors_defs[actor_type].cape[CAPE_NONE].mesh_index;
		a->body_parts->shield_meshindex=actors_defs[actor_type].shield[SHIELD_NONE].mesh_index;

		a->cur_anim.anim_index=-1;
		a->anim_time=0.0;
		CalModel_Update(a->calmodel,0);
	}
	
	UNLOCK_ACTORS_LISTS();  //unlock it

	return a;
}

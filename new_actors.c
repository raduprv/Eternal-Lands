#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
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
int add_enhanced_actor(enhanced_actor *this_actor, float x_pos, float y_pos,
					   float z_pos, float z_rot, float scale, int actor_id)
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
	our_actor->scale=scale;

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
	our_actor->stand_idle=0;
	our_actor->sit_idle=0;
	our_actor->body_parts=this_actor;

	//find a free spot, in the actors_list
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	for(i=0; i<max_actors; i++)
		{
			if(!actors_list[i])	break;
		}

	if (actor_id == yourself) your_actor = our_actor;
	actors_list[i]=our_actor;
	
	if(i>=max_actors)max_actors=i+1;
	
	no_bounding_box=0;
	//Will be unlocked later
	
	return i;
}

#ifndef	NEW_FRUSTUM
float cal_get_maxz(actor *act)
{
	float points[1024][3];  // caution, 1k point limit
	int nrPoints;
	struct CalSkeleton *skel;
	float maxz;
	int i;

	skel= CalModel_GetSkeleton(act->calmodel);
	nrPoints= CalSkeleton_GetBonePoints(skel,&points[0][0]);
	maxz= points[0][2];
	for(i=1; i<nrPoints; ++i) if(maxz<points[i][2]) maxz= points[i][2];
	return maxz;
}
#endif  //NEW_FRUSTUM


void draw_enhanced_actor(actor * actor_id, int banner)
{
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	float healthbar_z=0;
	bind_texture_id(actor_id->texture_id);

	if (actor_id->calmodel!=NULL){
#ifdef	NEW_FRUSTUM
		healthbar_z= actor_id->max_z+0.2;
#else
		healthbar_z= cal_get_maxz(actor_id)+0.2;
#endif
	}
	
	if(actor_id->actor_id==yourself)sitting=healthbar_z/2.0f;

	glPushMatrix();//we don't want to affect the rest of the scene
	x_pos=actor_id->tmp.x_pos;
	y_pos=actor_id->tmp.y_pos;
	z_pos=actor_id->tmp.z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->tmp.y_tile_pos*tile_map_size_x*6+actor_id->tmp.x_tile_pos]*0.2f;

	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);

	x_rot=actor_id->tmp.x_rot;
	y_rot=actor_id->tmp.y_rot;
	z_rot=-actor_id->tmp.z_rot;
	z_rot+=180;	//test
	glPushMatrix();
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if (actor_id->calmodel!=NULL) {
		cal_render_actor(actor_id);
	} 

	//now, draw their damage & nametag
	glPopMatrix();  // restore the matrix
	glRotatef(-rz, 0.0f, 0.0f, 1.0f);

	if (banner) draw_actor_banner(actor_id, healthbar_z);

	glPopMatrix();	//we don't want to affect the rest of the scene
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
								actors_list[i]->body_parts->weapon_tex[0]=0;
								actors_list[i]->cur_weapon = WEAPON_NONE;
								return;
							}

						if(which_part==KIND_OF_SHIELD)
							{
								CalModel_DetachMesh(actors_list[i]->calmodel,actors_list[i]->body_parts->shield_meshindex);
								actors_list[i]->body_parts->shield_tex[0]=0;
								actors_list[i]->cur_shield = SHIELD_NONE;
								return;
							}

						if(which_part==KIND_OF_CAPE)
							{
								CalModel_DetachMesh(actors_list[i]->calmodel,actors_list[i]->body_parts->cape_meshindex);
								actors_list[i]->body_parts->cape_tex[0]=0;
								return;
							}

						if(which_part==KIND_OF_HELMET)
							{
					     		CalModel_DetachMesh(actors_list[i]->calmodel,actors_list[i]->body_parts->helmet_meshindex);
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

	/* Check if custom1 has path readable */
	snprintf(buffer, sizeof(buffer), "%s%s", custom1, path);
	if(gzfile_exists(buffer)) {
		my_strcp(path, buffer);
		return;
	}

	/* Check if custom2 has path readable */
	snprintf(buffer, sizeof(buffer), "%s%s", custom2, path);
	if(gzfile_exists(buffer)) {
		my_strcp(path, buffer);
		return;
	}

	/* leave as is */
	return;
}
#endif  //CUSTOM_LOOK

void actor_wear_item(int actor_id,Uint8 which_part, Uint8 which_id)
{
	int i;
#ifdef CUSTOM_LOOK
	unsigned char playerpath[256], guildpath[256], onlyname[32]={0};
	int j;
#endif

	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				if(actors_list[i]->actor_id==actor_id)
					{
#ifdef CUSTOM_LOOK
						snprintf(guildpath, sizeof(guildpath), "custom/guild/%d/", actors_list[i]->body_parts->guild_id);
						for(j=0;j<30;j++){
                            if(actors_list[i]->actor_name[j]==' ' || actors_list[i]->actor_name[j]>125){
								j=31;
							}
							else if(actors_list[i]->actor_name[0]>'z'){
								onlyname[j]=actors_list[i]->actor_name[j+1];
							}
							else
							{
								onlyname[j]=actors_list[i]->actor_name[j];
							}
						}
						my_tolower(onlyname);
						snprintf(playerpath, sizeof(playerpath), "custom/player/%s/", onlyname);
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
								if(actors_defs[actors_list[i]->actor_type].shirt[which_id].mesh_index != actors_list[i]->body_parts->torso_meshindex)
								{
									CalModel_DetachMesh(actors_list[i]->calmodel,actors_list[i]->body_parts->torso_meshindex);
									CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actors_list[i]->actor_type].shirt[which_id].mesh_index);
									actors_list[i]->body_parts->torso_meshindex=actors_defs[actors_list[i]->actor_type].shirt[which_id].mesh_index;
								}
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
								if(actors_defs[actors_list[i]->actor_type].legs[which_id].mesh_index != actors_list[i]->body_parts->legs_meshindex)
								{
									CalModel_DetachMesh(actors_list[i]->calmodel,actors_list[i]->body_parts->legs_meshindex);
									CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actors_list[i]->actor_type].legs[which_id].mesh_index);
									actors_list[i]->body_parts->legs_meshindex=actors_defs[actors_list[i]->actor_type].legs[which_id].mesh_index;
								}
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
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								return;
							}
						return;
					}
		}
}

void add_enhanced_actor_from_server (const char *in_data, int len)
{
	short actor_id;
	Uint32 buffs;
	short x_pos;
	short y_pos;
	short z_pos;
	short z_rot;
	short max_health;
	short cur_health;
	Uint32 actor_type;
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
	unsigned char onlyname[32]={0};
	Uint32 j;
#endif
#if defined(CUSTOM_LOOK) || defined(MINIMAP)
	Uint32 uniq_id; // - Post ported.... We'll come up with something later...
    Uint32 guild_id;
#endif  //CUSTOM_LOOK || MINIMAP
	double f_x_pos,f_y_pos,f_z_pos,f_z_rot;
	float   scale=1.0f;
	
#ifdef EXTRA_DEBUG
	ERR();
#endif
	actor_id=SDL_SwapLE16(*((short *)(in_data)));
#ifndef EL_BIG_ENDIAN
	buffs=((*((char *)(in_data+3))>>3)&0x1F) | (((*((char*)(in_data+5))>>3)&0x1F)<<5);	// Strip the last 5 bits of the X and Y coords for the buffs
	x_pos=*((short *)(in_data+2)) & 0x7FF;
	y_pos=*((short *)(in_data+4)) & 0x7FF;
#else
	// If I understand the endian issue (and its possible I don't)
	// I need to base the bit mask around reversed bytes
	buffs=((SDL_SwapLE16(*((char*)(in_data+3)))>>3)&0x1F | (SDL_SwapLE16(((*((char*)(in_data+5)))>>3)&0x1F)<<5));	// Strip the last 5 bits of the X and Y coords for the buffs
	x_pos=SDL_SwapLE16(*((short *)(in_data+2))) & 0x7FF;
	y_pos=SDL_SwapLE16(*((short *)(in_data+4))) & 0x7FF;
#endif //EL_BIG_ENDIAN
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

	if(actor_type >= MAX_ACTOR_DEFS || (actor_type > 0 && actors_defs[actor_type].actor_type != actor_type) ){
		char    str[256];

		sprintf(str, "Illegal/missing enhanced actor definition %d", actor_type);
		log_error(str);
	}

	frame=*(in_data+22);
	max_health=SDL_SwapLE16(*((short *)(in_data+23)));
	cur_health=SDL_SwapLE16(*((short *)(in_data+25)));
	kind_of_actor=*(in_data+27);
#if defined CUSTOM_LOOK && defined UID
	uniq_id = SDL_SwapLE32(*((Uint32*)(in_data+28)));
	if(len > 32+strlen(in_data+32)+2){
		scale=((float)SDL_SwapLE16(*((short *)(in_data+32+strlen(in_data+32)+1)))/((float)ACTOR_SCALE_BASE));
	}
#else
	if(len > 28+strlen(in_data+28)+2){
		scale=((float)SDL_SwapLE16(*((short *)(in_data+28+strlen(in_data+28)+1)))/((float)ACTOR_SCALE_BASE));
	}
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
	case frame_attack_up_1:
	case frame_attack_up_2:
	case frame_attack_up_3:
	case frame_attack_up_4:
	case frame_attack_down_1:
	case frame_attack_down_2:
	case frame_combat_idle:
		break;
	default:
#ifdef UID
		log_error("%s %d - %s\n", unknown_frame, frame, &in_data[32]);
#else
		log_error("%s %d - %s\n", unknown_frame, frame, &in_data[28]);
#endif
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
#ifdef UID
							log_error("%s %d = %s => %s\n", duplicate_actors_str, actor_id, actors_list[i]->actor_name, &in_data[32]);
#else
							log_error("%s %d = %s => %s\n",duplicate_actors_str,actor_id, actors_list[i]->actor_name ,&in_data[28]);
#endif
							destroy_actor(actors_list[i]->actor_id);//we don't want two actors with the same ID
							i--;// last actor was put here, he needs to be checked too
						}
#ifdef UID
					else if(kind_of_actor==COMPUTER_CONTROLLED_HUMAN && (actors_list[i]->kind_of_actor==COMPUTER_CONTROLLED_HUMAN || actors_list[i]->kind_of_actor==PKABLE_COMPUTER_CONTROLLED) && !my_strcompare(&in_data[32], actors_list[i]->actor_name))
#else
					else if(kind_of_actor==COMPUTER_CONTROLLED_HUMAN && (actors_list[i]->kind_of_actor==COMPUTER_CONTROLLED_HUMAN || actors_list[i]->kind_of_actor==PKABLE_COMPUTER_CONTROLLED) && !my_strcompare(&in_data[28], actors_list[i]->actor_name))
#endif
						{
#ifdef UID
							log_error("%s(%d) = %s => %s\n", duplicate_npc_actor, actor_id, actors_list[i]->actor_name, &in_data[32]);
#else
							log_error("%s(%d) = %s => %s\n",duplicate_npc_actor,actor_id, actors_list[i]->actor_name ,&in_data[28]);
#endif
							destroy_actor(actors_list[i]->actor_id);//we don't want two actors with the same ID
							i--;// last actor was put here, he needs to be checked too
						}
				}
		}

#ifdef EXTRA_DEBUG
	ERR();
#endif

	this_actor=calloc(1,sizeof(enhanced_actor));

#if defined(CUSTOM_LOOK) || defined(MINIMAP)
	/* build a clean player name and a guild id */
	{
		/* get the name string into a working buffer */
		unsigned char buffer[256], *name, *guild;
#ifdef UID
		my_strncp(buffer,&in_data[32],sizeof(buffer));
#else
		my_strncp(buffer,&in_data[28],sizeof(buffer));
		uniq_id = 0;
#endif

		/* skip leading color codes */
		for(name=buffer; *name && (*name >= 127+c_lbound) && (*name <= 127+c_ubound); name++);
		/* trim off any guild tag, leaving solely the name (onlyname)*/
		for(j=0; name[j] && name[j]>32;j++){
			onlyname[j]=name[j];
		}
		
		/* search for string end or color mark */
		this_actor->guild_tag_color = 0;
		for (guild = name; *guild && ((*guild < 127 + c_lbound) || (*guild > 127 + c_ubound)); guild++);
		if (*guild) {
			/* separate the two strings */
			this_actor->guild_tag_color = *guild - 127;
			*guild = 0;
			guild++;
		}

		/* perform case insensitive comparison/hashing */
		my_tolower(name);
		my_tolower(onlyname);
		
		//perfect hashing of guildtag
 		switch(strlen(guild))
 		{
 		case 2:
 		    guild_id = guild[0] + (guild[1] << 8);
 		    break;
 		case 3:
 		    guild_id = guild[0] + (guild[1] << 8) + (guild[2] << 16);
 		    break;
 		default:
 		    guild_id = guild[0] + (guild[1] << 8) + (guild[2] << 16) + (guild[3] << 24);
 		    break;
		}
		this_actor->guild_id = guild_id;
	}

#ifdef  CUSTOM_LOOK
	/* precompute the paths to custom files */
	snprintf(playerpath, sizeof(playerpath), "custom/player/%s/", onlyname);
	snprintf(guildpath, sizeof(guildpath), "custom/guild/%u/", guild_id);
#endif  //CUSTOM_LOOK

	/* store the ids */
	this_actor->uniq_id = uniq_id;
	this_actor->guild_id = guild_id;
#endif	//CUSTOM_LOOK||MINIMAP

	//get the torso
	my_strncp(this_actor->arms_tex,actors_defs[actor_type].shirt[shirt].arms_name,sizeof(this_actor->arms_tex));
	my_strncp(this_actor->torso_tex,actors_defs[actor_type].shirt[shirt].torso_name,sizeof(this_actor->torso_tex));
	//skin
	my_strncp(this_actor->hands_tex,actors_defs[actor_type].skin[skin].hands_name,sizeof(this_actor->hands_tex));
	my_strncp(this_actor->hands_tex_save,actors_defs[actor_type].skin[skin].hands_name,sizeof(this_actor->hands_tex_save));
	my_strncp(this_actor->head_tex,actors_defs[actor_type].skin[skin].head_name,sizeof(this_actor->head_tex));
	//hair
	my_strncp(this_actor->hair_tex,actors_defs[actor_type].hair[hair].hair_name,sizeof(this_actor->hair_tex));
	//boots
	my_strncp(this_actor->boots_tex,actors_defs[actor_type].boots[boots].boots_name,sizeof(this_actor->boots_tex));
	//legs
	my_strncp(this_actor->pants_tex,actors_defs[actor_type].legs[pants].legs_name,sizeof(this_actor->pants_tex));

#ifdef CUSTOM_LOOK
	//torso
	custom_path(this_actor->arms_tex, playerpath, guildpath);
	custom_path(this_actor->torso_tex, playerpath, guildpath);
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
#endif //CUSTOM_LOOK

	//cape
	if(cape!=CAPE_NONE)
		{
			my_strncp(this_actor->cape_tex,actors_defs[actor_type].cape[cape].skin_name,sizeof(this_actor->cape_tex));
#ifdef CUSTOM_LOOK
			custom_path(this_actor->cape_tex, playerpath, guildpath);
#endif //CUSTOM_LOOK
		}
	else
		{
			my_strncp(this_actor->cape_tex,"",sizeof(this_actor->cape_tex));
		}
	//shield
	if(shield!=SHIELD_NONE)
		{
			my_strncp(this_actor->shield_tex,actors_defs[actor_type].shield[shield].skin_name,sizeof(this_actor->shield_tex));
#ifdef CUSTOM_LOOK
			custom_path(this_actor->shield_tex, playerpath, guildpath);
#endif //CUSTOM_LOOK
		}
	else
		{
			my_strncp(this_actor->shield_tex,"",sizeof(this_actor->shield_tex));
		}

	my_strncp(this_actor->weapon_tex,actors_defs[actor_type].weapon[weapon].skin_name,sizeof(this_actor->weapon_tex));
#ifdef CUSTOM_LOOK
	custom_path(this_actor->weapon_tex, playerpath, guildpath);
#endif //CUSTOM_LOOK
	this_actor->weapon_glow=actors_defs[actor_type].weapon[weapon].glow;
	if(weapon == GLOVE_FUR || weapon == GLOVE_LEATHER){
		my_strncp(this_actor->hands_tex, actors_defs[actor_type].weapon[weapon].skin_name,sizeof(this_actor->hands_tex));
#ifdef CUSTOM_LOOK
		custom_path(this_actor->hands_tex, playerpath, guildpath);
#endif //CUSTOM_LOOK
	}

	//helmet
	if(helmet!=HELMET_NONE)
		{
			my_strncp(this_actor->helmet_tex,actors_defs[actor_type].helmet[helmet].skin_name,sizeof(this_actor->helmet_tex));
#ifdef CUSTOM_LOOK
			custom_path(this_actor->helmet_tex, playerpath, guildpath);
#endif //CUSTOM_LOOK
		}
	else
		{
			my_strncp(this_actor->helmet_tex,"",sizeof(this_actor->helmet_tex));
		}

	i=add_enhanced_actor(this_actor,f_x_pos,f_y_pos,f_z_pos,f_z_rot,scale,actor_id);
	
#ifdef EXTRA_DEBUG
	ERR();
#endif //EXTRA_DEBUG
	//The actors list is already locked here
	
#ifdef COUNTERS
	actors_list[i]->async_fighting = 0;
	actors_list[i]->async_x_tile_pos = x_pos;
	actors_list[i]->async_y_tile_pos = y_pos;
	actors_list[i]->async_z_rot = z_rot;
#endif

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
	if(frame==frame_sit_idle)
		{
			if(actors_list[i]->actor_id==yourself)you_sit_down();
			actors_list[i]->sitting=1;
		}
	else
		{
			if(actors_list[i]->actor_id==yourself)you_stand_up();
			if(frame==frame_combat_idle)
				actors_list[i]->fighting=1;
		}

	//ghost or not?
	actors_list[i]->ghost=actors_defs[actor_type].ghost;

	actors_list[i]->dead=dead;
	actors_list[i]->stop_animation=1;//helps when the actor is dead...
	actors_list[i]->cur_weapon=weapon;
	actors_list[i]->kind_of_actor=kind_of_actor;
	if(strlen(&in_data[28]) >= 30)
	{
		log_error("%s (%d): %s/%d\n", bad_actor_name_length, actors_list[i]->actor_type,&in_data[28], (int)strlen(&in_data[28]));
	}
	else 
	{
		/* Extract the name for use in the tab completion list. */
		const unsigned char *name = in_data+28;
		unsigned char *ptr;
		unsigned char buffer[32];

		if(kind_of_actor != NPC) {
			/* Skip leading color codes */
			for (; *name && IS_COLOR(*name); name++);
			snprintf(buffer, sizeof(buffer), "%.30s", name);
			/* Remove guild tag, etc. */
			for(ptr = buffer; *ptr && *ptr <= 127 + c_lbound && !isspace(*ptr); ptr++);
			*ptr = '\0';
			add_name_to_tablist(buffer);
		}
		my_strncp(actors_list[i]->actor_name,&in_data[28],sizeof(actors_list[i]->actor_name));
		if(caps_filter && my_isupper(actors_list[i]->actor_name, -1)) {
			my_tolower(actors_list[i]->actor_name);
		}
	}

	if (actors_defs[actor_type].coremodel!=NULL) {
		actors_list[i]->calmodel=CalModel_New(actors_defs[actor_type].coremodel);

		if (actors_list[i]->calmodel!=NULL) {
			//Setup cal3d model
			//actors_list[i]->calmodel=CalModel_New(actors_defs[actor_type].coremodel);
			//Attach meshes
			CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].head[head].mesh_index);
			CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].shirt[shirt].mesh_index);
			CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].legs[pants].mesh_index);
			actors_list[i]->body_parts->torso_meshindex=actors_defs[actor_type].shirt[shirt].mesh_index;
			actors_list[i]->body_parts->legs_meshindex=actors_defs[actor_type].legs[pants].mesh_index;
			actors_list[i]->body_parts->head_meshindex=actors_defs[actor_type].head[head].mesh_index;

			if (cape!=CAPE_NONE) CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].cape[cape].mesh_index);
			if (helmet!=HELMET_NONE) CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].helmet[helmet].mesh_index);
			if (weapon!=WEAPON_NONE) CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].weapon[weapon].mesh_index);
			if (shield!=SHIELD_NONE) CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].shield[shield].mesh_index);

			actors_list[i]->body_parts->helmet_meshindex=actors_defs[actor_type].helmet[helmet].mesh_index;
			actors_list[i]->body_parts->cape_meshindex=actors_defs[actor_type].cape[cape].mesh_index;
			actors_list[i]->body_parts->shield_meshindex=actors_defs[actor_type].shield[shield].mesh_index;

			actors_list[i]->cur_anim.anim_index= -1;
			stop_sound(actors_list[i]->cur_anim_sound_cookie);
			actors_list[i]->cur_anim_sound_cookie= 0;
			actors_list[i]->anim_time= 0.0;
			actors_list[i]->last_anim_update= cur_time;

			if(dead){
				cal_actor_set_anim(i, actors_defs[actors_list[i]->actor_type].cal_die1_frame);
				actors_list[i]->stop_animation=1;
				CalModel_Update(actors_list[i]->calmodel,1000);
			} else  CalModel_Update(actors_list[i]->calmodel,0);
		}
	} else actors_list[i]->calmodel=NULL;

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

actor * add_actor_interface(float x, float y, float z_rot, float scale, int actor_type, short skin, short hair,
				short shirt, short pants, short boots, short head)
{
	enhanced_actor * this_actor=calloc(1,sizeof(enhanced_actor));
	actor * a;
	
	//get the torso
	my_strncp(this_actor->arms_tex,actors_defs[actor_type].shirt[shirt].arms_name,sizeof(this_actor->arms_tex));
	my_strncp(this_actor->torso_tex,actors_defs[actor_type].shirt[shirt].torso_name,sizeof(this_actor->torso_tex));
	my_strncp(this_actor->hands_tex,actors_defs[actor_type].skin[skin].hands_name,sizeof(this_actor->hands_tex));
	my_strncp(this_actor->head_tex,actors_defs[actor_type].skin[skin].head_name,sizeof(this_actor->head_tex));
	my_strncp(this_actor->hair_tex,actors_defs[actor_type].hair[hair].hair_name,sizeof(this_actor->hair_tex));
	my_strncp(this_actor->boots_tex,actors_defs[actor_type].boots[boots].boots_name,sizeof(this_actor->boots_tex));
	my_strncp(this_actor->pants_tex,actors_defs[actor_type].legs[pants].legs_name,sizeof(this_actor->pants_tex));

	a=actors_list[add_enhanced_actor(this_actor, x*0.5f, y*0.5f, 0.00000001f, z_rot, scale, 0)];

	a->x_tile_pos=x;
	a->y_tile_pos=y;
	a->actor_type=actor_type;
	//test only
	a->max_health=20;
	a->cur_health=20;

	a->stop_animation=1;//helps when the actor is dead...
	a->kind_of_actor=HUMAN;
	
	snprintf(a->actor_name, sizeof(a->actor_name), "Player");
	
	if (actors_defs[actor_type].coremodel!=NULL) {
		a->calmodel=CalModel_New(actors_defs[actor_type].coremodel);

		if (a->calmodel!=NULL) {
			//Setup cal3d model
			//a->calmodel=CalModel_New(actors_defs[actor_type].coremodel);
			//Attach meshes
			CalModel_AttachMesh(a->calmodel,actors_defs[actor_type].head[head].mesh_index);
			CalModel_AttachMesh(a->calmodel,actors_defs[actor_type].shirt[shirt].mesh_index);
			CalModel_AttachMesh(a->calmodel,actors_defs[actor_type].legs[pants].mesh_index);
			
			a->body_parts->torso_meshindex=actors_defs[actor_type].shirt[shirt].mesh_index;
			a->body_parts->legs_meshindex=actors_defs[actor_type].legs[pants].mesh_index;
			a->body_parts->head_meshindex=actors_defs[actor_type].head[head].mesh_index;

			a->body_parts->helmet_meshindex=actors_defs[actor_type].helmet[HELMET_NONE].mesh_index;
			a->body_parts->cape_meshindex=actors_defs[actor_type].cape[CAPE_NONE].mesh_index;
			a->body_parts->shield_meshindex=actors_defs[actor_type].shield[SHIELD_NONE].mesh_index;

			a->cur_anim.anim_index=-1;
			a->anim_time=0.0;
			a->last_anim_update= cur_time;
			CalModel_Update(a->calmodel,0);
		}
	} else a->calmodel=NULL;
	
	UNLOCK_ACTORS_LISTS();  //unlock it

	return a;
}

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"

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

	//ok, load the legs
	if(this_actor->legs_fn[0])
		{
			this_actor->legs=(md2*)load_md2_cache(this_actor->legs_fn);
			if(!this_actor->legs)
				{
    		        char str[120];
    		        sprintf(str,"%s: %s: %s\n",reg_error_str,error_body_part,this_actor->legs_fn);
    		        log_error(str);
    		        this_actor->legs=0;
				}
		}
	else this_actor->legs=0;

	//ok, load the head
	if(this_actor->head_fn[0])
		{
			this_actor->head=(md2*)load_md2_cache(this_actor->head_fn);
			if(!this_actor->head)
				{
    		        char str[120];
    		        sprintf(str,"%s: %s (%s): %s\n",reg_error_str,error_body_part,error_head,this_actor->head_fn);
    		        log_error(str);
    		        this_actor->head=0;
				}
		}
	else this_actor->head=0;

	//ok, load the torso
	if(this_actor->torso_fn[0])
		{
			this_actor->torso=(md2*)load_md2_cache(this_actor->torso_fn);
			if(!this_actor->torso)
				{
    		        char str[120];
    		        sprintf(str,"%s: %s (%s): %s\n",reg_error_str,error_body_part,error_torso,this_actor->torso_fn);
    		        log_error(str);
    		        this_actor->torso=0;
				}
		}
	else this_actor->torso=0;

	//ok, load the weapon
	if(this_actor->weapon_fn[0])
		{
			this_actor->weapon=(md2*)load_md2_cache(this_actor->weapon_fn);
			if(!this_actor->weapon)
				{
    		        char str[120];
    		        sprintf(str,"%s: %s (%s): %s\n",reg_error_str,error_body_part,error_weapon,this_actor->weapon_fn);
    		        log_error(str);
    		        this_actor->weapon=0;
				}
		}
	else this_actor->weapon=0;

	//ok, load the shield
	if(this_actor->shield_fn[0])
		{
			this_actor->shield=(md2*)load_md2_cache(this_actor->shield_fn);
			if(!this_actor->shield)
				{
    		        char str[120];
    		        sprintf(str,"%s: %s (%s): %s\n",reg_error_str,error_body_part,error_weapon,this_actor->shield_fn);
    		        log_error(str);
    		        this_actor->shield=0;
				}
		}
	else this_actor->shield=0;

	//ok, load the helmet
	if(this_actor->helmet_fn[0])
		{
			this_actor->helmet=(md2*)load_md2_cache(this_actor->helmet_fn);
			if(!this_actor->helmet)
				{
    		        char str[120];
    		        sprintf(str,"%s: %s (%s): %s\n",reg_error_str,error_body_part,error_helmet,this_actor->helmet_fn);
    		        log_error(str);
    		        this_actor->helmet=0;
				}
		}
	else this_actor->helmet=0;


	//ok, load the cape
	if(this_actor->cape_fn[0])
		{
			this_actor->cape=(md2*)load_md2_cache(this_actor->cape_fn);
			if(!this_actor->cape)
				{
    		        char str[120];
    		        sprintf(str,"%s: %s (%s): %s\n",reg_error_str,error_body_part,error_cape,this_actor->cape_fn);
    		        log_error(str);
    		        this_actor->cape=0;
				}
		}
	else this_actor->cape=0;

	//get the skin
	texture_id= load_bmp8_enhanced_actor(this_actor, 255);

	our_actor = calloc(1, sizeof(actor));

	memset(our_actor->current_displayed_text, 0, max_current_displayed_text_len);
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
	for(k=0;k<10;k++)our_actor->que[k]=nothing;



	our_actor->model_data=0;
	my_strcp(our_actor->cur_frame,frame_name);
	our_actor->stand_idle=0;
	our_actor->sit_idle=0;
	our_actor->body_parts=this_actor;

	//find a free spot, in the actors_list
	lock_actors_lists();	//lock it to avoid timing issues
	for(i=0;i<max_actors;i++)
		{
			if(!actors_list[i])break;
		}

	actors_list[i]=our_actor;
	if(i>=max_actors)max_actors=i+1;
	no_bounding_box=0;
	unlock_actors_lists();	//unlock it

	return i;
}




void draw_enhanced_actor(actor * actor_id)
{
	int i;
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	//int texture_id;
	char *cur_frame;
	float healtbar_z=0;

	bind_texture_id(actor_id->texture_id);
	cur_frame=actor_id->cur_frame;

	//now, go and find the current frame
	i=get_frame_number(actor_id->body_parts->head, cur_frame);;
	if(i >= 0)healtbar_z=actor_id->body_parts->head->offsetFrames[i].box.max_z;

	glPushMatrix();//we don't want to affect the rest of the scene
	x_pos=actor_id->x_pos;
	y_pos=actor_id->y_pos;
	z_pos=actor_id->z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;

	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);

	x_rot=actor_id->x_rot;
	y_rot=actor_id->y_rot;
	z_rot=actor_id->z_rot;
	z_rot+=180;//test
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if(actor_id->body_parts->legs)draw_model(actor_id->body_parts->legs,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->torso)draw_model(actor_id->body_parts->torso,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->head)draw_model(actor_id->body_parts->head,cur_frame,actor_id->ghost);

	if(actor_id->body_parts->weapon)
		{
			int glow;
			draw_model(actor_id->body_parts->weapon,cur_frame,actor_id->ghost);
			glow=actor_id->body_parts->weapon_glow;
			if(glow!=GLOW_NONE)draw_model_halo(actor_id->body_parts->weapon,cur_frame,glow_colors[glow].r,glow_colors[glow].g,glow_colors[glow].b);
		}

	if(actor_id->body_parts->shield)draw_model(actor_id->body_parts->shield,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->helmet)draw_model(actor_id->body_parts->helmet,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->cape)draw_model(actor_id->body_parts->cape,cur_frame,actor_id->ghost);


	//////
	glPopMatrix();//restore the scene
	//now, draw their damage

	glPushMatrix();
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	glRotatef(-rz, 0.0f, 0.0f, 1.0f);

	draw_actor_banner(actor_id, healtbar_z);

	glPopMatrix();//we don't want to affect the rest of the scene
	//if(!actor_id->ghost)glEnable(GL_LIGHTING);

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
								actors_list[i]->body_parts->weapon=0;
								actors_list[i]->body_parts->weapon_fn[0]=0;
								actors_list[i]->body_parts->weapon_tex[0]=0;
								return;
							}

						if(which_part==KIND_OF_SHIELD)
							{
								actors_list[i]->body_parts->shield=0;
								actors_list[i]->body_parts->shield_fn[0]=0;
								actors_list[i]->body_parts->shield_tex[0]=0;
								return;
							}

						if(which_part==KIND_OF_CAPE)
							{
								actors_list[i]->body_parts->cape=0;
								actors_list[i]->body_parts->cape_fn[0]=0;
								actors_list[i]->body_parts->cape_tex[0]=0;
								return;
							}

						if(which_part==KIND_OF_HELMET)
							{
								actors_list[i]->body_parts->helmet=0;
								actors_list[i]->body_parts->helmet_fn[0]=0;
								actors_list[i]->body_parts->helmet_tex[0]=0;
								return;
							}

						return;
					}
		}

}

void actor_wear_item(int actor_id,Uint8 which_part, Uint8 which_id)
{
	int i;

	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				if(actors_list[i]->actor_id==actor_id)
					{
						if(which_part==KIND_OF_WEAPON)
							{
								if(which_id == GLOVE_FUR || which_id == GLOVE_LEATHER){
									my_strcp(actors_list[i]->body_parts->hands_tex, actors_defs[actors_list[i]->actor_type].weapon[which_id].skin_name);
								}
								my_strcp(actors_list[i]->body_parts->weapon_tex,actors_defs[actors_list[i]->actor_type].weapon[which_id].skin_name);
								my_strcp(actors_list[i]->body_parts->weapon_fn,actors_defs[actors_list[i]->actor_type].weapon[which_id].model_name);
								no_bounding_box=1;
								actors_list[i]->body_parts->weapon=(md2*)load_md2_cache(actors_list[i]->body_parts->weapon_fn);
								no_bounding_box=0;
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								actors_list[i]->cur_weapon=which_id;

								actors_list[i]->body_parts->weapon_glow=actors_defs[actors_list[i]->actor_type].weapon[which_id].glow;

								return;
							}

						if(which_part==KIND_OF_SHIELD)
							{
								my_strcp(actors_list[i]->body_parts->shield_tex,actors_defs[actors_list[i]->actor_type].shield[which_id].skin_name);
								my_strcp(actors_list[i]->body_parts->shield_fn,actors_defs[actors_list[i]->actor_type].shield[which_id].model_name);
								no_bounding_box=1;
								actors_list[i]->body_parts->shield=(md2*)load_md2_cache(actors_list[i]->body_parts->shield_fn);
								no_bounding_box=0;
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								return;
							}

						if(which_part==KIND_OF_CAPE)
							{
								my_strcp(actors_list[i]->body_parts->cape_tex,actors_defs[actors_list[i]->actor_type].cape[which_id].skin_name);
								my_strcp(actors_list[i]->body_parts->cape_fn,actors_defs[actors_list[i]->actor_type].cape[which_id].model_name);
								no_bounding_box=1;
								actors_list[i]->body_parts->cape=(md2*)load_md2_cache(actors_list[i]->body_parts->cape_fn);
								no_bounding_box=0;
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								return;
							}

						if(which_part==KIND_OF_HELMET)
							{
								my_strcp(actors_list[i]->body_parts->helmet_tex,actors_defs[actors_list[i]->actor_type].helmet[which_id].skin_name);
								my_strcp(actors_list[i]->body_parts->helmet_fn,actors_defs[actors_list[i]->actor_type].helmet[which_id].model_name);
								no_bounding_box=1;
								actors_list[i]->body_parts->helmet=(md2*)load_md2_cache(actors_list[i]->body_parts->helmet_fn);
								no_bounding_box=0;
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								return;
							}

						if(which_part==KIND_OF_BODY_ARMOR)
							{
								my_strcp(actors_list[i]->body_parts->arms_tex,actors_defs[actors_list[i]->actor_type].shirt[which_id].arms_name);
								my_strcp(actors_list[i]->body_parts->torso_tex,actors_defs[actors_list[i]->actor_type].shirt[which_id].torso_name);
								my_strcp(actors_list[i]->body_parts->torso_fn,actors_defs[actors_list[i]->actor_type].shirt[which_id].model_name);
								no_bounding_box=1;
								actors_list[i]->body_parts->torso=(md2*)load_md2_cache(actors_list[i]->body_parts->torso_fn);
								no_bounding_box=0;
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								return;
							}
						if(which_part==KIND_OF_LEG_ARMOR)
							{
								my_strcp(actors_list[i]->body_parts->pants_tex,actors_defs[actors_list[i]->actor_type].legs[which_id].legs_name);
								my_strcp(actors_list[i]->body_parts->legs_fn,actors_defs[actors_list[i]->actor_type].legs[which_id].model_name);
								no_bounding_box=1;
								actors_list[i]->body_parts->legs=(md2*)load_md2_cache(actors_list[i]->body_parts->legs_fn);
								no_bounding_box=0;
								glDeleteTextures(1,&actors_list[i]->texture_id);
								actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
								return;
							}

						if(which_part==KIND_OF_BOOT_ARMOR)
							{
								my_strcp(actors_list[i]->body_parts->boots_tex,actors_defs[actors_list[i]->actor_type].boots[which_id].boots_name);
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

	char cur_frame[20];
	double f_x_pos,f_y_pos,f_z_pos,f_z_rot;

	actor_id=*((short *)(in_data));
	x_pos=*((short *)(in_data+2));
	y_pos=*((short *)(in_data+4));
	z_pos=*((short *)(in_data+6));
	z_rot=*((short *)(in_data+8));
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

	frame=*(in_data+22);
	max_health=*((short *)(in_data+23));
	cur_health=*((short *)(in_data+25));
	kind_of_actor=*(in_data+27);

	//translate from tile to world
	f_x_pos=x_pos*0.5;
	f_y_pos=y_pos*0.5;
	f_z_pos=z_pos;
	f_z_rot=z_rot;

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
		}
	}

	//find out if there is another actor with that ID
	//ideally this shouldn't happen, but just in case
	lock_actors_lists();    //lock it to avoid timing issues
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				{
					if(actors_list[i]->actor_id==actor_id)
						{
							char str[256];
							sprintf(str,"%s %d = %s => %s\n",duplicate_actors_str,actor_id, actors_list[i]->actor_name ,&in_data[28]);
							log_error(str);
							destroy_actor(actors_list[i]->actor_id);//we don't want two actors with the same ID
							i--;// last actor was put here, he needs to be checked too
						}
					else if(kind_of_actor==COMPUTER_CONTROLLED_HUMAN && (actors_list[i]->kind_of_actor==COMPUTER_CONTROLLED_HUMAN || actors_list[i]->kind_of_actor==PKABLE_COMPUTER_CONTROLLED) && !my_strcompare(&in_data[28], actors_list[i]->actor_name))
						{
							char str[256];
							sprintf(str,"%s(%d) = %s => %s\n",duplicate_npc_actor,actor_id, actors_list[i]->actor_name ,&in_data[28]);
							log_error(str);
							destroy_actor(actors_list[i]->actor_id);//we don't want two actors with the same ID
							i--;// last actor was put here, he needs to be checked too
						}
				}
		}
	unlock_actors_lists();  //unlock it

	this_actor=calloc(1,sizeof(enhanced_actor));

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
	//cape
	if(cape!=CAPE_NONE)
		{
			my_strcp(this_actor->cape_tex,actors_defs[actor_type].cape[cape].skin_name);
			my_strcp(this_actor->cape_fn,actors_defs[actor_type].cape[cape].model_name);
		}
	else
		{
			my_strcp(this_actor->cape_tex,"");
			my_strcp(this_actor->cape_fn,"");
		}
	//head
	my_strcp(this_actor->head_fn,actors_defs[actor_type].head[head].model_name);

	//shield
	if(shield!=SHIELD_NONE)
		{
			my_strcp(this_actor->shield_tex,actors_defs[actor_type].shield[shield].skin_name);
			my_strcp(this_actor->shield_fn,actors_defs[actor_type].shield[shield].model_name);
		}
	else
		{
			my_strcp(this_actor->shield_tex,"");
			my_strcp(this_actor->shield_fn,"");
		}

	my_strcp(this_actor->weapon_tex,actors_defs[actor_type].weapon[weapon].skin_name);
	my_strcp(this_actor->weapon_fn,actors_defs[actor_type].weapon[weapon].model_name);
	this_actor->weapon_glow=actors_defs[actor_type].weapon[weapon].glow;
	if(weapon == GLOVE_FUR || weapon == GLOVE_LEATHER){
		my_strcp(this_actor->hands_tex, actors_defs[actor_type].weapon[weapon].skin_name);
	}

	//helmet
	if(helmet!=HELMET_NONE)
		{
			my_strcp(this_actor->helmet_tex,actors_defs[actor_type].helmet[helmet].skin_name);
			my_strcp(this_actor->helmet_fn,actors_defs[actor_type].helmet[helmet].model_name);
		}
	else
		{
			my_strcp(this_actor->helmet_tex,"");
			my_strcp(this_actor->helmet_fn,"");
		}

	i=add_enhanced_actor(this_actor,cur_frame,f_x_pos,f_y_pos,f_z_pos,f_z_rot,actor_id);

	lock_actors_lists();    //lock it to avoid timing issues
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
			snprintf(str, 120, "%s (%d): %s/%d\n", bad_actor_name_length, actors_list[i]->actor_type,&in_data[28], (int)strlen(&in_data[28]));
			log_error(str);
			return;
		}

	my_strncp(actors_list[i]->actor_name,&in_data[28],30);
	if(caps_filter && my_isupper(actors_list[i]->actor_name, -1)) my_tolower(actors_list[i]->actor_name);
	unlock_actors_lists();  //unlock it

}



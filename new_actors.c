#include <stdlib.h>
#include <math.h>
#include "global.h"

void draw_body_part(md2 *model_data,char *cur_frame, int ghost)
{
	int i,j;
	float u,v;
	float x,y,z;
	char *dest_frame_name;
	char str[20];
	int numFrames;
    int numFaces;
    text_coord_md2 *offsetTexCoords;
    face_md2 *offsetFaces;
    frame_md2 *offsetFrames;
    vertex_md2 *vertex_pointer=NULL;

	numFaces=model_data->numFaces;
	numFrames=model_data->numFrames;
	offsetFaces=model_data->offsetFaces;
	offsetTexCoords=model_data->offsetTexCoords;
	offsetFrames=model_data->offsetFrames;


	//now, go and find the current frame
	i=0;
	while(i<numFrames)
	{
		dest_frame_name=(char *)&offsetFrames[i].name;
		//dest_frame_name=offsetFrames[i].name;
		if(strcmp(cur_frame,dest_frame_name)==0)//we found the current frame
			{
				vertex_pointer=offsetFrames[i].vertex_pointer;
				break;
			}
		i++;
	}

	i=0;
	if(vertex_pointer==NULL)//if there is no frame, use idle01
		{
			char str[120];
			sprintf(str, "couldn't find frame: %s\n",cur_frame);
			log_error(str);
			while(i<numFrames)
			{
				dest_frame_name=(char *)&offsetFrames[i].name;
				if(strcmp("idle01",dest_frame_name)==0)//we found the current frame
					{
						vertex_pointer=offsetFrames[i].vertex_pointer;
						break;
					}
				i++;
			}
		}

	if(vertex_pointer==NULL)// this REALLY shouldn't happen...
		{
			char str[120];
			sprintf(str, "couldn't find frame: %s\n",cur_frame);
			log_error(str);
			return;
		}

	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_TRIANGLES);
	for(j=0;j<numFaces;j++)
		{
			x=vertex_pointer[offsetFaces[j].a].x;
			y=vertex_pointer[offsetFaces[j].a].y;
			z=vertex_pointer[offsetFaces[j].a].z;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].at].u,offsetTexCoords[offsetFaces[j].at].v);
			glVertex3f(x,y,z);


			x=vertex_pointer[offsetFaces[j].b].x;
			y=vertex_pointer[offsetFaces[j].b].y;
			z=vertex_pointer[offsetFaces[j].b].z;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].bt].u,offsetTexCoords[offsetFaces[j].bt].v);
			glVertex3f(x,y,z);


			x=vertex_pointer[offsetFaces[j].c].x;
			y=vertex_pointer[offsetFaces[j].c].y;
			z=vertex_pointer[offsetFaces[j].c].z;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].ct].u,offsetTexCoords[offsetFaces[j].ct].v);
			glVertex3f(x,y,z);


		}
	glEnd();

}


//return the ID (number in the actors_list[]) of the new allocated actor
int add_enhanced_actor(enhanced_actor *this_actor,char * frame_name,float x_pos, float y_pos, float z_pos, float z_rot, int actor_id)
{
	int texture_id;
	int i;
	int k;
	md2 *returned_md2;
	actor *our_actor;
	no_bounding_box=1;

	our_actor = calloc(1, sizeof(actor));

	//find a free spot, in the actors_list
	i=0;
	while(i<1000)
		{
			if(!actors_list[i])break;
			i++;
		}

	//ok, load the legs
	if(this_actor->legs_fn[0])
		{
			this_actor->legs=(md2*)load_md2_cache(this_actor->legs_fn);
			if(!this_actor->legs)
			   {
    		        char str[120];
    		        sprintf(str,"Error: Can't load body part: %s\n",this_actor->legs_fn);
    		        log_error(str);
    		        this_actor->legs=0;
			        //return 0;
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
    		        sprintf(str,"Error: Can't load body part (head): %s\n",this_actor->head_fn);
    		        log_error(str);
    		        this_actor->head=0;
			        //return 0;
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
    		        sprintf(str,"Error: Can't load body part (torso): %s\n",this_actor->torso_fn);
    		        log_error(str);
    		        this_actor->torso=0;
			        //return 0;
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
    		        sprintf(str,"Error: Can't load body part (weapon): %s\n",this_actor->weapon_fn);
    		        log_error(str);
    		        this_actor->weapon=0;
			        //return 0;
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
    		        sprintf(str,"Error: Can't load body part (shield): %s\n",this_actor->shield_fn);
    		        log_error(str);
    		        this_actor->shield=0;
			        //return 0;
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
    		        sprintf(str,"Error: Can't load body part (helmet): %s\n",this_actor->helmet_fn);
    		        log_error(str);
    		        this_actor->helmet=0;
			        //return 0;
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
    		        sprintf(str,"Error: Can't load body part (cape): %s\n",this_actor->cape_fn);
    		        log_error(str);
    		        this_actor->cape=0;
			        //return 0;
			   }
		}
	else this_actor->cape=0;

	//get the skin
	texture_id=load_bmp8_enhanced_actor(this_actor, 255);
	//texture_id=load_texture_cache("./md2/textureformat.bmp",150);
	//texture_id=load_texture_cache("./md2/animals3.bmp",150);


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

	actors_list[i]=our_actor;
	no_bounding_box=0;
	return i;
}




void draw_enhanced_actor(actor * actor_id)
{
	int i,j;
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	float x,y,z;
	int texture_id;
	char *cur_frame;
	char str[20];
	float healtbar_x=-0.3f;
	float healtbar_y=0;
	float healtbar_z;
	float healtbar_x_len=0.5f;
	float healtbar_x_len_converted=0;
	float healtbar_z_len=0.05f;
	int numFrames;
	char *dest_frame_name;
	frame_md2 *offsetFrames;



	offsetFrames=actor_id->body_parts->head->offsetFrames;
	texture_id=actor_id->texture_id;
	//texture_id=texture_cache[actor_id->texture_id].texture_id;

	cur_frame=actor_id->cur_frame;

	x_pos=actor_id->x_pos;
	y_pos=actor_id->y_pos;
	z_pos=actor_id->z_pos;

	x_rot=actor_id->x_rot;
	y_rot=actor_id->y_rot;
	z_rot=actor_id->z_rot;

	z_rot+=180;//test

	//now, go and find the current frame
	i=0;
	numFrames=actor_id->body_parts->head->numFrames;
	while(i<numFrames)
	{
		dest_frame_name=(char *)&offsetFrames[i].name;
		//dest_frame_name=offsetFrames[i].name;
		if(strcmp(cur_frame,dest_frame_name)==0)//we found the current frame
			{
				healtbar_z=offsetFrames[i].box.max_z+0.2f;
				break;
			}
		i++;
	}

	if(last_texture!=texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_id);
			last_texture=texture_id;
		}
	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
	z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;

	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if(actor_id->body_parts->legs)draw_body_part(actor_id->body_parts->legs,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->torso)draw_body_part(actor_id->body_parts->torso,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->head)draw_body_part(actor_id->body_parts->head,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->weapon)draw_body_part(actor_id->body_parts->weapon,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->shield)draw_body_part(actor_id->body_parts->shield,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->helmet)draw_body_part(actor_id->body_parts->helmet,cur_frame,actor_id->ghost);
	if(actor_id->body_parts->cape)draw_body_part(actor_id->body_parts->cape,cur_frame,actor_id->ghost);


	//////
	glPopMatrix();//restore the scene
	//now, draw their damage

	glPushMatrix();
	//glTranslatef(x_pos, y_pos, z_pos);
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	glRotatef(-rz, 0.0f, 0.0f, 1.0f);

	//draw the health bar
	glDisable(GL_TEXTURE_2D);
	//choose color for the bar
	if(actor_id->cur_health>=actor_id->max_health/2)
	glColor3f(0,1,0);//green life bar
	else if(actor_id->cur_health>=actor_id->max_health/4 && actor_id->cur_health<actor_id->max_health/2)
	glColor3f(1,1,0);//yellow life bar
	else glColor3f(1,0,0);
	if(!actor_id->ghost)glDisable(GL_LIGHTING);

	if(view_health_bar && actor_id->cur_health>=0)
		{
			//get it's lenght
			if(actor_id->max_health)//we don't want a division by zero, now do we?
			healtbar_x_len_converted=healtbar_x_len*(float)((float)actor_id->cur_health/(float)actor_id->max_health);
			glBegin(GL_QUADS);
			glVertex3f(healtbar_x,healtbar_y,healtbar_z);
			glVertex3f(healtbar_x+healtbar_x_len_converted,healtbar_y,healtbar_z);
			glVertex3f(healtbar_x+healtbar_x_len_converted,healtbar_y,healtbar_z+healtbar_z_len);
			glVertex3f(healtbar_x,healtbar_y,healtbar_z+healtbar_z_len);
			glEnd();

			//draw the frame
			healtbar_y=0.001;
			glDepthFunc(GL_LEQUAL);
			glColor3f(0,0,0);
			glBegin(GL_LINES);
			glVertex3f(healtbar_x,healtbar_y,healtbar_z);
			glVertex3f(healtbar_x+healtbar_x_len,healtbar_y,healtbar_z);

			glVertex3f(healtbar_x,healtbar_y,healtbar_z+healtbar_z_len);
			glVertex3f(healtbar_x+healtbar_x_len,healtbar_y,healtbar_z+healtbar_z_len);

			glVertex3f(healtbar_x,healtbar_y,healtbar_z);
			glVertex3f(healtbar_x,healtbar_y,healtbar_z+healtbar_z_len);

			glVertex3f(healtbar_x+healtbar_x_len,healtbar_y,healtbar_z);
			glVertex3f(healtbar_x+healtbar_x_len,healtbar_y,healtbar_z+healtbar_z_len);
			glEnd();
		}

	glEnable(GL_TEXTURE_2D);
	glColor3f(1,0,0);

	glDepthFunc(GL_ALWAYS);
	if(actor_id->damage_ms)
		{
			sprintf(str,"%i",actor_id->damage);
			glColor3f(1,0.3f,0.3f);
			draw_ingame_string(-0.1,healtbar_z-2.0f,str,1,1);
		}
	glDepthFunc(GL_LESS);
	if(actor_id->actor_name[0] && view_names)
		{
			if(actor_id->kind_of_actor==NPC)glColor3f(0.3f,0.8f,1.0f);
			else if(actor_id->kind_of_actor==HUMAN || actor_id->kind_of_actor==COMPUTER_CONTROLLED_HUMAN)glColor3f(1.0f,1.0f,1.0f);
			else glColor3f(1.0f,1.0f,0.0f);
			draw_ingame_string(-(strlen(actor_id->actor_name)*SMALL_INGAME_FONT_X_LEN)/2,healtbar_z-0.7f,actor_id->actor_name,1,0);
		}
	glColor3f(1,1,1);
	glPopMatrix();//we don't want to affect the rest of the scene
	if(!actor_id->ghost)glEnable(GL_LIGHTING);


}

void unwear_item_from_actor(int actor_id,Uint8 which_part)
{
	int i=0;

	while(i<1000)
		{
			if(actors_list[i])
			if(actors_list[i]->actor_id==actor_id)
				{
					if(which_part==KIND_OF_WEAPON)
						{
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
			i++;
		}

}

actor_wear_item(int actor_id,Uint8 which_part, Uint8 which_id)
{
	int i=0;

	while(i<1000)
		{
			if(actors_list[i])
			if(actors_list[i]->actor_id==actor_id)
				{
					if(which_part==KIND_OF_WEAPON)
						{
							my_strcp(actors_list[i]->body_parts->weapon_tex,actors_defs[actors_list[i]->actor_type].weapon[which_id].skin_name);
							my_strcp(actors_list[i]->body_parts->weapon_fn,actors_defs[actors_list[i]->actor_type].weapon[which_id].model_name);
							no_bounding_box=1;
							actors_list[i]->body_parts->weapon=(md2*)load_md2_cache(actors_list[i]->body_parts->weapon_fn);
							no_bounding_box=0;
							glDeleteTextures(1,&actors_list[i]->texture_id);
							actors_list[i]->texture_id=load_bmp8_enhanced_actor(actors_list[i]->body_parts, 255);
							actors_list[i]->cur_weapon=which_id;
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
					return;
				}
			i++;
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
	if(frame==frame_walk)my_strcp(cur_frame,actors_defs[actor_type].walk_frame);
	else
	if(frame==frame_run)my_strcp(cur_frame,actors_defs[actor_type].run_frame);
	else
	if(frame==frame_die1)
		{
			my_strcp(cur_frame,actors_defs[actor_type].die1_frame);
			dead=1;
		}
	else
	if(frame==frame_die2)
		{
			my_strcp(cur_frame,actors_defs[actor_type].die2_frame);
			dead=1;
		}
	else
	if(frame==frame_pain1)my_strcp(cur_frame,actors_defs[actor_type].pain1_frame);
	else
	if(frame==frame_pain2)my_strcp(cur_frame,actors_defs[actor_type].pain2_frame);
	else
	if(frame==frame_pick)my_strcp(cur_frame,actors_defs[actor_type].pick_frame);
	else
	if(frame==frame_drop)my_strcp(cur_frame,actors_defs[actor_type].drop_frame);
	else
	if(frame==frame_idle)my_strcp(cur_frame,actors_defs[actor_type].idle_frame);
	else
	if(frame==frame_sit_idle)my_strcp(cur_frame,actors_defs[actor_type].idle_sit_frame);
	else
	if(frame==frame_harvest)my_strcp(cur_frame,actors_defs[actor_type].harvest_frame);
	else
	if(frame==frame_cast)my_strcp(cur_frame,actors_defs[actor_type].attack_cast_frame);
	else
	if(frame==frame_attack_up_1)my_strcp(cur_frame,actors_defs[actor_type].attack_up_1_frame);
	else
	if(frame==frame_attack_up_2)my_strcp(cur_frame,actors_defs[actor_type].attack_up_2_frame);
	else
	if(frame==frame_attack_up_3)my_strcp(cur_frame,actors_defs[actor_type].attack_up_3_frame);
	else
	if(frame==frame_attack_up_4)my_strcp(cur_frame,actors_defs[actor_type].attack_up_4_frame);
	else
	if(frame==frame_attack_down_1)my_strcp(cur_frame,actors_defs[actor_type].attack_down_1_frame);
	else
	if(frame==frame_attack_down_2)my_strcp(cur_frame,actors_defs[actor_type].attack_down_2_frame);
	else
	if(frame==frame_combat_idle)my_strcp(cur_frame,actors_defs[actor_type].combat_idle_frame);

	//find out if there is another actor with that ID
	//ideally this shouldn't happen, but just in case
	i=0;
	while(i<1000)
		{
			if(actors_list[i])
			if(actors_list[i]->actor_id==actor_id)
			destroy_actor(i);//we don't want two actors with thesame ID
			i++;
		}

	this_actor=calloc(1,sizeof(enhanced_actor));

	//get the torso
	my_strcp(this_actor->arms_tex,actors_defs[actor_type].shirt[shirt].arms_name);
	my_strcp(this_actor->torso_tex,actors_defs[actor_type].shirt[shirt].torso_name);
	my_strcp(this_actor->torso_fn,actors_defs[actor_type].shirt[shirt].model_name);
	//skin
	my_strcp(this_actor->hands_tex,actors_defs[actor_type].skin[skin].hands_name);
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
	if(frame==frame_sit_idle)actors_list[i]->sitting=1;
	else
	if(frame==frame_combat_idle)actors_list[i]->fighting=1;

	//ghost or not?
	actors_list[i]->ghost=0;

	actors_list[i]->dead=dead;
	actors_list[i]->stop_animation=1;//helps when the actor is dead...
	actors_list[i]->cur_weapon=weapon;
	actors_list[i]->kind_of_actor=kind_of_actor;
	sprintf(actors_list[i]->actor_name,&in_data[28]);

}



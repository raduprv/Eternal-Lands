#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"

//Threading support for actors_lists
void init_actors_lists()
{
	int	i;

	actors_lists_mutex=SDL_CreateMutex();
	lock_actors_lists();	//lock it to avoid timing issues
	for(i=0;i<1000;i++)actors_list[i]=0;
	unlock_actors_lists();	// release now that we are done
}

void end_actors_lists()
{
	SDL_DestroyMutex(actors_lists_mutex);
	actors_lists_mutex=NULL;
}

//Tests to see if a MD2 is already loaded. If it is, return the handle.
//If not, load it, and return the handle
md2 * load_md2_cache(char * file_name)
{
	int i;
	int j;
	int file_name_lenght;
	md2 * md2_id;

	file_name_lenght=strlen(file_name);

	for(i=0;i<1000;i++)
		{
			j=0;
			while(j<file_name_lenght)
				{
					if(md2_cache[i].file_name[j]!=file_name[j])break;
					j++;
				}
			if(file_name_lenght==j)//ok, md2 already loaded
				return md2_cache[i].md2_id;
		}
	//md2 not found in the cache, so load it, and store it
	md2_id=load_md2(file_name);

	//find a place to store it
	i=0;
	while(i<1000)
		{
			if(!md2_cache[i].file_name[0])//we found a place to store it
				{
					sprintf(md2_cache[i].file_name, "%s", file_name);
					md2_cache[i].md2_id=md2_id;
					return md2_id;
				}
			i++;
		}

	return md2_id;
}

//return the ID (number in the actors_list[]) of the new allocated actor
int add_actor(char * file_name,char * skin_name, char * frame_name,float x_pos,
			  float y_pos, float z_pos, float z_rot, char remappable, 
			  short skin_color, short hair_color, short shirt_color, 
			  short pants_color, short boots_color, int actor_id)
{
	int texture_id;
	int i;
	int k;
	md2 *returned_md2;
	actor *our_actor;

	our_actor = calloc(1, sizeof(actor));

	//find a free spot, in the actors_list
	lock_actors_lists();	//lock it to avoid timing issues
	for(i=0;i<max_actors;i++)
		{
			if(!actors_list[i])break;
		}

	returned_md2=load_md2_cache(file_name);
	if(!returned_md2)
		{
            char str[120];
			unlock_actors_lists();	// release now that we are done
            sprintf(str,"Error: Can't load actor: %s\n",file_name);
            log_error(str);
	        return 0;
		}
	if(!remappable)texture_id=load_texture_cache(skin_name,150);
	else
		{
			texture_id=load_bmp8_remapped_skin(skin_name,150,skin_color,hair_color,shirt_color,pants_color,boots_color);
		}

	our_actor->is_enhanced_model=0;
	our_actor->remapped_colors=remappable;
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



	our_actor->model_data=returned_md2;
	our_actor->texture_id=texture_id;
	my_strcp(our_actor->cur_frame,frame_name);
	my_strcp(our_actor->skin_name,skin_name);
	our_actor->skin=skin_color;
	our_actor->hair=hair_color;
	our_actor->pants=pants_color;
	our_actor->boots=boots_color;
	our_actor->shirt=shirt_color;
	our_actor->stand_idle=0;
	our_actor->sit_idle=0;

	actors_list[i]=our_actor;
	if(i>=max_actors)max_actors=i+1;
	unlock_actors_lists();	// release now that we are done
	return i;
}


void draw_actor_banner(actor * actor_id, float offset_z)
{
	char str[60];
	float healtbar_x=-0.25f*zoom_level/3.0f;
	float healtbar_y=0;
	float healtbar_z=offset_z+0.1f;	//was 0.2f
	float healtbar_x_len=0.5f*zoom_level/3.0f;
	float healtbar_x_len_converted=0;
	float healtbar_z_len=0.05f*zoom_level/3.0f;

	//draw the health bar
	glDisable(GL_TEXTURE_2D);
	//choose color for the bar
	if(actor_id->cur_health>=actor_id->max_health/2)
		glColor3f(0,1,0);	//green life bar
	//else if(actor_id->cur_health>=actor_id->max_health/4 && actor_id->cur_health<actor_id->max_health/2)
	else if(actor_id->cur_health>=actor_id->max_health/4)
		glColor3f(1,1,0);	//yellow life bar
	else glColor3f(1,0,0);	//red life bar
	if(!actor_id->ghost)glDisable(GL_LIGHTING);

	if(view_health_bar && actor_id->cur_health>=0)
		{
			//get it's lenght
			if(actor_id->max_health > 0)//we don't want a division by zero, now do we?
				{
					healtbar_x_len_converted=healtbar_x_len*(float)((float)actor_id->cur_health/(float)actor_id->max_health);
				}
			glBegin(GL_QUADS);
			glVertex3f(healtbar_x,healtbar_y,healtbar_z);
			glVertex3f(healtbar_x+healtbar_x_len_converted,healtbar_y,healtbar_z);
			glVertex3f(healtbar_x+healtbar_x_len_converted,healtbar_y,healtbar_z+healtbar_z_len);
			glVertex3f(healtbar_x,healtbar_y,healtbar_z+healtbar_z_len);
			glEnd();

			//draw the frame
			healtbar_y=0.001*zoom_level/3.0f;
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
			//draw_ingame_string(-0.1,healtbar_z-2.0f,str,1,1);
			draw_ingame_string(-0.1,healtbar_z/2.0f,str,1,1);
		}
	glDepthFunc(GL_LESS);
	if(actor_id->actor_name[0] && view_names)
		{
			if(actor_id->kind_of_actor==NPC)glColor3f(0.3f,0.8f,1.0f);
			else if(actor_id->kind_of_actor==HUMAN || actor_id->kind_of_actor==COMPUTER_CONTROLLED_HUMAN)glColor3f(1.0f,1.0f,1.0f);
			else glColor3f(1.0f,1.0f,0.0f);
			//draw_ingame_string(-(strlen(actor_id->actor_name)*SMALL_INGAME_FONT_X_LEN)/2,healtbar_z-0.7f,actor_id->actor_name,1,0);
			//TODO: use text length function instead of strlen
			//draw_ingame_string(-((float)strlen(actor_id->actor_name)*(SMALL_INGAME_FONT_X_LEN*zoom_level*name_zoom/3.0))/2.0,healtbar_z+0.05f,actor_id->actor_name,1,0);
			set_font(name_font);	// to variable length
			draw_ingame_string(-((float)get_string_width(actor_id->actor_name)*(SMALL_INGAME_FONT_X_LEN*zoom_level*name_zoom/3.0))/2.0/12.0,healtbar_z+(0.06f*zoom_level/3.0),actor_id->actor_name,1,0);
			set_font(0);	// back to fixed pitch
		}
	glColor3f(1,1,1);
	if(!actor_id->ghost)glEnable(GL_LIGHTING);

}

void draw_actor(actor * actor_id)
{
	int i;	//,j;
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	//float u,v; unused?
	int texture_id;
	char *cur_frame;
	//char str[20];
	//float healtbar_x=-0.3f;
	//float healtbar_y=0;
	float healtbar_z=0;
	//float healtbar_x_len=0.5f;
	//float healtbar_x_len_converted=0;
	//float healtbar_z_len=0.05f;

	int numFrames;
    int numFaces;
    text_coord_md2 *offsetTexCoords;
    face_md2 *offsetFaces;
    frame_md2 *offsetFrames;
    vertex_md2 *vertex_pointer=NULL;

	if(!actor_id->remapped_colors)texture_id=texture_cache[actor_id->texture_id].texture_id;
	else
		{
			//we have remaped colors, we don't store such textures into the cache
			texture_id=actor_id->texture_id;
		}

	cur_frame=actor_id->cur_frame;

	//now, go and find the current frame
	offsetFrames=actor_id->model_data->offsetFrames;
	numFrames=actor_id->model_data->numFrames;
	i=0;
	while(i<numFrames)
		{
			char *dest_frame_name;

			dest_frame_name=(char *)&offsetFrames[i].name;
			if(strcmp(cur_frame,dest_frame_name)==0)//we found the current frame
				{
					vertex_pointer=offsetFrames[i].vertex_pointer;
					healtbar_z=offsetFrames[i].box.max_z;
					break;
				}
			i++;
		}
	if(vertex_pointer==NULL)// this REALLY shouldn't happen...
		{
			char str[256];
			sprintf(str, "couldn't find frame: %s for %s\n",cur_frame,actor_id->actor_name);
			log_error(str);
			return;
		}

	if(last_texture!=texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_id);
			last_texture=texture_id;
		}

	x_pos=actor_id->x_pos;
	y_pos=actor_id->y_pos;
	z_pos=actor_id->z_pos;

	x_rot=actor_id->x_rot;
	y_rot=actor_id->y_rot;
	z_rot=actor_id->z_rot;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;

	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_TRIANGLES);
	offsetFaces=actor_id->model_data->offsetFaces;
	offsetTexCoords=actor_id->model_data->offsetTexCoords;
	numFaces=actor_id->model_data->numFaces;
	for(i=0;i<numFaces;i++)
		{
			float x,y,z;

			glTexCoord2f(offsetTexCoords[offsetFaces[i].at].u,offsetTexCoords[offsetFaces[i].at].v);
			x=vertex_pointer[offsetFaces[i].a].x;
			y=vertex_pointer[offsetFaces[i].a].y;
			z=vertex_pointer[offsetFaces[i].a].z;
			glVertex3f(x,y,z);

			glTexCoord2f(offsetTexCoords[offsetFaces[i].bt].u,offsetTexCoords[offsetFaces[i].bt].v);
			x=vertex_pointer[offsetFaces[i].b].x;
			y=vertex_pointer[offsetFaces[i].b].y;
			z=vertex_pointer[offsetFaces[i].b].z;
			glVertex3f(x,y,z);

			glTexCoord2f(offsetTexCoords[offsetFaces[i].ct].u,offsetTexCoords[offsetFaces[i].ct].v);
			x=vertex_pointer[offsetFaces[i].c].x;
			y=vertex_pointer[offsetFaces[i].c].y;
			z=vertex_pointer[offsetFaces[i].c].z;
			glVertex3f(x,y,z);
		}
	glEnd();

	glPopMatrix();//restore the scene
	//now, draw their damage
	glPushMatrix();
	//glTranslatef(x_pos, y_pos, z_pos);
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	glRotatef(-rz, 0.0f, 0.0f, 1.0f);

	draw_actor_banner(actor_id, healtbar_z);
	/*
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
			if(actor_id->ghost)glDisable(GL_BLEND);
			if(actor_id->kind_of_actor==NPC)glColor3f(0.3f,0.8f,1.0f);
			else if(actor_id->kind_of_actor==HUMAN || actor_id->kind_of_actor==COMPUTER_CONTROLLED_HUMAN)glColor3f(1.0f,1.0f,1.0f);
			else glColor3f(1.0f,1.0f,0.0f);
			//draw_ingame_string(-(strlen(actor_id->actor_name)*(int)((float)SMALL_INGAME_FONT_X_LEN*zoom_level/3.0))/2,healtbar_z-0.7f,actor_id->actor_name,1,0);
			draw_ingame_string(-(strlen(actor_id->actor_name)*(int)(((float)SMALL_INGAME_FONT_X_LEN)*zoom_level/3.0))/2,healtbar_z+0.0f,actor_id->actor_name,1,0);
			if(actor_id->ghost)glEnable(GL_BLEND);
		}
	glColor3f(1,1,1);
	*/
	glPopMatrix();//we don't want to affect the rest of the scene
	//if(!actor_id->ghost)glEnable(GL_LIGHTING);

}


void display_actors()
{
	int i;
	int x,y;
	x=-cx;
	y=-cy;

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//MD2s don't have real normals...
	glNormal3f(0.0f,0.0f,1.0f);

	//display only the non ghosts
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				if(!actors_list[i]->ghost)
					{
						int dist1;
						int dist2;

						dist1=x-actors_list[i]->x_pos;
						dist2=y-actors_list[i]->y_pos;
						if(dist1*dist1+dist2*dist2<=12*12)
							{
								if(actors_list[i]->is_enhanced_model)
									{
										draw_enhanced_actor(actors_list[i]);
									}
								else
									{
										draw_actor(actors_list[i]);
									}
								if(actors_list[i]->kind_of_actor==NPC)anything_under_the_mouse(i, UNDER_MOUSE_NPC);
								else
									if(actors_list[i]->kind_of_actor==HUMAN || actors_list[i]->kind_of_actor==COMPUTER_CONTROLLED_HUMAN)anything_under_the_mouse(i, UNDER_MOUSE_PLAYER);
									else anything_under_the_mouse(i, UNDER_MOUSE_ANIMAL);
							}
					}
		}


	//display only the ghosts
	glEnable(GL_BLEND);
    //we don't need the light, for ghosts
    glDisable(GL_LIGHTING);
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				if(actors_list[i]->ghost)
					{
						int dist1;
						int dist2;

						dist1=x-actors_list[i]->x_pos;
						dist2=y-actors_list[i]->y_pos;
						if(dist1*dist1+dist2*dist2<=12*12)
							{
								if(actors_list[i]->is_enhanced_model)
									{
										draw_enhanced_actor(actors_list[i]);
									}
								else
									{
										draw_actor(actors_list[i]);
									}
								if(actors_list[i]->kind_of_actor==NPC)anything_under_the_mouse(i, UNDER_MOUSE_NPC);
								else
									if(actors_list[i]->kind_of_actor==HUMAN || actors_list[i]->kind_of_actor==COMPUTER_CONTROLLED_HUMAN)anything_under_the_mouse(i, UNDER_MOUSE_PLAYER);
									else anything_under_the_mouse(i, UNDER_MOUSE_ANIMAL);
							}
					}
		}
	glDisable(GL_BLEND);
}


void add_actor_from_server(char * in_data)
{
	short actor_id;
	short x_pos;
	short y_pos;
	short z_pos;
	short z_rot;
	short max_health;
	short cur_health;
	short actor_type;
	char remapable;
	char skin;
	char hair;
	char shirt;
	char pants;
	char boots;
	char frame;
	int i;
	int dead=0;
	int kind_of_actor;

	char cur_frame[20];
	double f_x_pos,f_y_pos,f_z_pos,f_z_rot;

	actor_id=*((short *)(in_data));
	x_pos=*((short *)(in_data+2));
	y_pos=*((short *)(in_data+4));
	z_pos=*((short *)(in_data+6));
	z_rot=*((short *)(in_data+8));
	actor_type=*((short *)(in_data+10));
	remapable=*(in_data+11);
	skin=*(in_data+12);
	hair=*(in_data+13);
	shirt=*(in_data+14);
	pants=*(in_data+15);
	boots=*(in_data+16);
	frame=*(in_data+17);
	max_health=*((short *)(in_data+18));
	cur_health=*((short *)(in_data+20));
	kind_of_actor=*(in_data+22);

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
																			else 
																				{
																					char str[120];
																					sprintf(str,"Unknown frame %d for %s\n",frame,&in_data[23]);
																					log_error(str);
																				}
	//find out if there is another actor with that ID
	//ideally this shouldn't happen, but just in case
	lock_actors_lists();	//lock it to avoid timing issues
	for(i=0;i<max_actors;i++)
		{
			if(actors_list[i])
				if(actors_list[i]->actor_id==actor_id)
					{
						char str[256];
						sprintf(str,"Duplicate actor ID %d was %s now is %s\n",actor_id, actors_list[i]->actor_name ,&in_data[23]);
						log_error(str);
						destroy_actor(i);//we don't want two actors with the same ID
					}
		}

	i=add_actor(actors_defs[actor_type].file_name,actors_defs[actor_type].skin_name,cur_frame,
				f_x_pos, f_y_pos, f_z_pos, f_z_rot,remapable, skin, hair, shirt, pants, boots, actor_id);
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
	actors_list[i]->ghost=actors_defs[actor_type].ghost;

	actors_list[i]->dead=dead;
	actors_list[i]->stop_animation=1;//helps when the actor is dead...
	actors_list[i]->kind_of_actor=kind_of_actor;
	if(strlen(&in_data[23]) >= 30)
		{
			char str[120];
			snprintf(str, 120, "Bad actor name/length (%d): %s/%d\n", actors_list[i]->actor_type,&in_data[23], strlen(&in_data[23]));
			log_error(str);
			return;
		}
	my_strncp(actors_list[i]->actor_name,&in_data[23],30);
	unlock_actors_lists();	//unlock it
}




void draw_interface_body_part(md2 *model_data,float scale)
{
	int i,j;
	float x,y,z;
	char *dest_frame_name;
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
			if(strcmp("idle01",dest_frame_name)==0)//we found the current frame
				{
					vertex_pointer=offsetFrames[i].vertex_pointer;
					break;
				}
			i++;
		}


	if(vertex_pointer==NULL)return;


	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_TRIANGLES);
	for(j=0;j<numFaces;j++)
		{
			x=vertex_pointer[offsetFaces[j].a].x*scale;
			y=vertex_pointer[offsetFaces[j].a].y*scale;
			z=vertex_pointer[offsetFaces[j].a].z*scale;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].at].u,offsetTexCoords[offsetFaces[j].at].v);
			glVertex3f(x,y,z);


			x=vertex_pointer[offsetFaces[j].b].x*scale;
			y=vertex_pointer[offsetFaces[j].b].y*scale;
			z=vertex_pointer[offsetFaces[j].b].z*scale;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].bt].u,offsetTexCoords[offsetFaces[j].bt].v);
			glVertex3f(x,y,z);


			x=vertex_pointer[offsetFaces[j].c].x*scale;
			y=vertex_pointer[offsetFaces[j].c].y*scale;
			z=vertex_pointer[offsetFaces[j].c].z*scale;

			glTexCoord2f(offsetTexCoords[offsetFaces[j].ct].u,offsetTexCoords[offsetFaces[j].ct].v);
			glVertex3f(x,y,z);


		}
	glEnd();

}




//this actor will be resized. We want speed, so that's why we add a different function
void draw_interface_actor(actor * actor_id,float scale,int x_pos,int y_pos,
						  int z_pos, float x_rot,float y_rot, float z_rot)
{
	int texture_id;
	frame_md2 *offsetFrames;

	offsetFrames=actor_id->body_parts->head->offsetFrames;
	texture_id=actor_id->texture_id;

	x_rot+=180;//test
	z_rot+=180;//test

	if(last_texture!=texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_id);
			last_texture=texture_id;
		}
	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef(x_pos, y_pos, z_pos);
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

	if(actor_id->body_parts->legs)draw_interface_body_part(actor_id->body_parts->legs,scale);
	if(actor_id->body_parts->torso)draw_interface_body_part(actor_id->body_parts->torso,scale);
	if(actor_id->body_parts->head)draw_interface_body_part(actor_id->body_parts->head,scale);

	//////
	glPopMatrix();//restore the scene}
}

actor * add_actor_interface(int actor_type, short skin, short hair, 
							short shirt, short pants, short boots, short head)
{


	actor *our_actor;
	enhanced_actor *this_actor;
	int texture_id;

	this_actor=calloc(1,sizeof(enhanced_actor));
	our_actor = calloc(1, sizeof(actor));

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

	no_bounding_box=1;
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
    		        sprintf(str,"Error: Can't load body part: %s\n",this_actor->head_fn);
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
    		        sprintf(str,"Error: Can't load body part: %s\n",this_actor->torso_fn);
    		        log_error(str);
    		        this_actor->torso=0;
			        //return 0;
				}
		}
	else this_actor->torso=0;

	no_bounding_box=0;
	//get the skin
	texture_id=load_bmp8_enhanced_actor(this_actor, 255);
	our_actor->texture_id=texture_id;
	our_actor->body_parts=this_actor;
	no_bounding_box=0;
	return our_actor;


}


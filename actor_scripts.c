#include "global.h"
#include <math.h>

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


void move_to_next_frame()
{
	int i,j,l,k;
	char frame_name[16];
	char frame_number[3];
	int frame_no;
	int numFrames;
	char frame_exists;


	for(i=0;i<1000;i++)
		{
			//clear the strings
			for(k=0;k<16;k++)frame_name[k]=0;
			for(k=0;k<3;k++)frame_name[k]=0;

			if(actors_list[i]!=0)
			  {
				  //first thing, decrease the damage time, so we will see the damage splash only for 2 seconds
				  if(actors_list[i]->damage_ms)
				  	{
				  		actors_list[i]->damage_ms-=80;
				  		if(actors_list[i]->damage_ms<0)actors_list[i]->damage_ms=0;
					}
				  //get the frame number out of the frame name
				  l=strlen(actors_list[i]->cur_frame);
				  //get the frame number
				  j=0;
				  for(k=l-2;k<=l;k++)
				    {
				      frame_number[j]=actors_list[i]->cur_frame[k];
				      j++;
				    }
				  frame_no=atoi(frame_number);
				  //get the frame name
				  for(k=0;k<l-2;k++)frame_name[k]=actors_list[i]->cur_frame[k];
				  //increment the frame_no
				  frame_no++;
				  //transform back into string
				  frame_number[0]=(unsigned int)48+frame_no/10;
				  frame_number[1]=(unsigned int)48+frame_no%10;
				  //create the name of the next frame to look for
				  my_strcat(frame_name,frame_number);
				  //now see if we can find that frame
				  if(!actors_list[i]->is_enhanced_model)
				  numFrames=actors_list[i]->model_data->numFrames;
				  else
				  numFrames=actors_list[i]->body_parts->torso->numFrames;

				  frame_exists=0;
				  for(k=0;k<numFrames;k++)
				  	{
						if(!actors_list[i]->is_enhanced_model)
						if(strcmp(frame_name,actors_list[i]->model_data->offsetFrames[k].name)==0)
							{
								frame_exists=1;
								break;
							}
						if(actors_list[i]->is_enhanced_model)
						if(strcmp(frame_name,actors_list[i]->body_parts->torso->offsetFrames[k].name)==0)
							{
								frame_exists=1;
								break;
							}
					}

				 if(!frame_exists)//frame doesn't exist, move at the beginning of animation
				 if(actors_list[i]->stop_animation)
				 	{
						actors_list[i]->busy=0;//ok, take the next command
						continue;//we are done with this guy
					}
				 else
					{
						//frame_name has 2 extra numbers, at this point, due to the previous
						//strcat. So, remove those 2 extra numbers
						l=strlen(frame_name);
						frame_name[l-2]=0;
						my_strcat(frame_name,"01");
					}

				 sprintf(actors_list[i]->cur_frame, "%s",frame_name);

			  }
		}
}

void animate_actors()
{
	int i;
    float rotate_x_speed;
    float rotate_y_speed;
    float rotate_z_speed;

    char moving;
    char rotating;

	for(i=0;i<1000;i++)
		{
			if(actors_list[i])
			     {
                    if(actors_list[i]->moving)
                        {
                            actors_list[i]->movement_frames_left--;
                            if(!actors_list[i]->movement_frames_left)//we moved all the way
                            	{
									Uint8 last_command;
                            		actors_list[i]->moving=0;//don't move next time, ok?
                            		actors_list[i]->after_move_frames_left=3;//this is done to prevent going to idle imediatelly
                            		//now, we need to update the x/y_tile_pos, and round off
                            		//the x/y_pos according to x/y_tile_pos
                            		last_command=actors_list[i]->last_command;
                            		if(last_command==move_n || last_command==run_n)
                            		actors_list[i]->y_tile_pos++;
                            		else
                            		if(last_command==move_s || last_command==run_s)
                            		actors_list[i]->y_tile_pos--;
                            		else
                            		if(last_command==move_e || last_command==run_e)
                            		actors_list[i]->x_tile_pos++;
                            		else
                            		if(last_command==move_w || last_command==run_w)
                            		actors_list[i]->x_tile_pos--;
                            		else
                            		if(last_command==move_ne || last_command==run_ne)
                            		{actors_list[i]->x_tile_pos++;actors_list[i]->y_tile_pos++;}
                            		else
                            		if(last_command==move_se || last_command==run_se)
                            		{actors_list[i]->x_tile_pos++;actors_list[i]->y_tile_pos--;}
                            		else
                            		if(last_command==move_sw || last_command==run_sw)
                            		{actors_list[i]->x_tile_pos--;actors_list[i]->y_tile_pos--;}
                            		else
                            		if(last_command==move_nw || last_command==run_nw)
                            		{actors_list[i]->x_tile_pos--;actors_list[i]->y_tile_pos++;}
                            		//ok, now update the x/y_pos

                            		actors_list[i]->x_pos=actors_list[i]->x_tile_pos*0.5;
                            		actors_list[i]->y_pos=actors_list[i]->y_tile_pos*0.5;


								}
							else
								{
                            		actors_list[i]->x_pos+=actors_list[i]->move_x_speed;
                            		actors_list[i]->y_pos+=actors_list[i]->move_y_speed;
                            		actors_list[i]->z_pos+=actors_list[i]->move_z_speed;
								}

                        }
                     else //not moving
                     	{
							if(actors_list[i]->after_move_frames_left)
								{
									actors_list[i]->after_move_frames_left--;
									if(!actors_list[i]->after_move_frames_left)actors_list[i]->busy=0;

								}
						}

                    if(actors_list[i]->rotating)
                        {
                            actors_list[i]->rotate_frames_left--;
                            if(!actors_list[i]->rotate_frames_left)//we rotated all the way
                            actors_list[i]->rotating=0;//don't rotate next time, ok?

                            actors_list[i]->x_rot+=actors_list[i]->rotate_x_speed;
                            actors_list[i]->y_rot+=actors_list[i]->rotate_y_speed;
                            actors_list[i]->z_rot+=actors_list[i]->rotate_z_speed;
                        }
                 }
		}
}




//in case the actor is not busy, and has commands in it's que, execute them
void next_command()
{
	int i;

	for(i=0;i<1000;i++)
		{
			if(!actors_list[i])continue;//actor exists?
			//if(!actors_list[i]->is_enhanced_model)//test only
			if(!actors_list[i]->busy || (actors_list[i]->busy && actors_list[i]->after_move_frames_left && (actors_list[i]->que[0]>=move_n && actors_list[i]->que[0]<=move_nw)))//is it not busy?
//			if(!actors_list[i]->busy || (actors_list[i]->busy && (actors_list[i]->stand_idle || actors_list[i]->sit_idle)))//is it not busy?
			if(actors_list[i]->que[0]==nothing)//do we have something in the que?
			   	 {
					 	//if que is empty, set on idle
					 if(!actors_list[i]->dead)
					 {
						actors_list[i]->stop_animation=0;

					 	if(actors_list[i]->fighting)
					 		{
								my_strcp(actors_list[i]->cur_frame,actors_defs[actors_list[i]->actor_type].combat_idle_frame);
								goto after_fight_test;
							}

					 	if(!actors_list[i]->sitting)
					 		{
								if(!actors_list[i]->sit_idle)
									{
										my_strcp(actors_list[i]->cur_frame,actors_defs[actors_list[i]->actor_type].idle_frame);
										actors_list[i]->sit_idle=1;
									}
							}
						else
							{
								if(!actors_list[i]->stand_idle)
									{
										my_strcp(actors_list[i]->cur_frame,actors_defs[actors_list[i]->actor_type].idle_sit_frame);
										actors_list[i]->stand_idle=1;
									}
							}

						after_fight_test:
					 }

						actors_list[i]->last_command=nothing;//prevents us from not updating the walk/run animation
				 }
			else
			     {
					 int actor_type;
					 int last_command=actors_list[i]->last_command;
					 float z_rot=actors_list[i]->z_rot;
					 float targeted_z_rot;
					 int k;

					 actors_list[i]->sit_idle=0;
					 actors_list[i]->stand_idle=0;

					 actor_type=actors_list[i]->actor_type;

					 if(actors_list[i]->que[0]==kill_me)
					 	{
					 		if(actors_list[i]->remapped_colors)glDeleteTextures(1,&actors_list[i]->texture_id);
							free(actors_list[i]);
							actors_list[i]=0;
						}
					 else
					 if(actors_list[i]->que[0]==die1)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].die1_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->dead=1;
						}
					 else
					 if(actors_list[i]->que[0]==die2)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].die2_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->dead=1;
						}
					 else
					 if(actors_list[i]->que[0]==pain1)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].pain1_frame);
							actors_list[i]->stop_animation=1;
						}
					 else
					 if(actors_list[i]->que[0]==pain2)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].pain2_frame);
							actors_list[i]->stop_animation=1;
						}
					 else
					 if(actors_list[i]->que[0]==pick)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].pick_frame);
							actors_list[i]->stop_animation=1;
						}
					 else
					 if(actors_list[i]->que[0]==drop)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].drop_frame);
							actors_list[i]->stop_animation=1;
						}
					 else
					 if(actors_list[i]->que[0]==harvest)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].harvest_frame);
							actors_list[i]->stop_animation=1;
						}
					 else
					 if(actors_list[i]->que[0]==cast)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].attack_cast_frame);
							actors_list[i]->stop_animation=1;
						}
					 else
					 if(actors_list[i]->que[0]==ranged)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].attack_ranged_frame);
							actors_list[i]->stop_animation=1;
						}
					 else
					 if(actors_list[i]->que[0]==sit_down)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].sit_down_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->sitting=1;
							if(actors_list[i]->actor_id==yourself)you_sit=1;
						}
					 else
					 if(actors_list[i]->que[0]==stand_up)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].stand_up_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->sitting=0;
							if(actors_list[i]->actor_id==yourself)you_sit=0;
						}
					 else
					 if(actors_list[i]->que[0]==enter_combat)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].in_combat_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->fighting=1;
						}
					 else
					 if(actors_list[i]->que[0]==leave_combat)
					 	{
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].out_combat_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->fighting=0;
						}
					 else
					 if(actors_list[i]->que[0]==attack_up_1)
					 	{
							if(actors_list[i]->is_enhanced_model)sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_up1);
							else sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].attack_up_1_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->fighting=1;
						}
					 else
					 if(actors_list[i]->que[0]==attack_up_2)
					 	{
							if(actors_list[i]->is_enhanced_model)sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_up1);
							else sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].attack_up_2_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->fighting=1;

						}
					 else
					 if(actors_list[i]->que[0]==attack_up_3)
					 	{
							if(actors_list[i]->is_enhanced_model)sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_up2);
							else sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].attack_up_3_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->fighting=1;
						}
					 else
					 if(actors_list[i]->que[0]==attack_up_4)
					 	{
							if(actors_list[i]->is_enhanced_model)sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_up2);
							else sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].attack_up_4_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->fighting=1;
						}
					 else
					 if(actors_list[i]->que[0]==attack_down_1)
					 	{
							if(actors_list[i]->is_enhanced_model)sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_down1);
							else sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].attack_down_1_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->fighting=1;
						}
					 else
					 if(actors_list[i]->que[0]==attack_down_2)
					 	{
							if(actors_list[i]->is_enhanced_model)sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].weapon[actors_list[i]->cur_weapon].attack_down2);
							else sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].attack_down_2_frame);
							actors_list[i]->stop_animation=1;
							actors_list[i]->fighting=1;
						}
					 else
					 if(actors_list[i]->que[0]==turn_left)
					 	{
							actors_list[i]->rotate_z_speed=45.0/9.0;
							actors_list[i]->rotate_frames_left=9;
							actors_list[i]->rotating=1;
							//generate a fake movement, so we will know when to make the actor
							//not busy
							actors_list[i]->move_x_speed=0;
							actors_list[i]->move_y_speed=0;
							actors_list[i]->movement_frames_left=9;
							actors_list[i]->moving=1;
							//test
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].walk_frame);
							actors_list[i]->stop_animation=1;
						}
					 else
					 if(actors_list[i]->que[0]==turn_right)
					 	{
							actors_list[i]->rotate_z_speed=-45.0/9.0;
							actors_list[i]->rotate_frames_left=9;
							actors_list[i]->rotating=1;
							//generate a fake movement, so we will know when to make the actor
							//not busy
							actors_list[i]->move_x_speed=0;
							actors_list[i]->move_y_speed=0;
							actors_list[i]->movement_frames_left=9;
							actors_list[i]->moving=1;
							//test
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].walk_frame);
							actors_list[i]->stop_animation=1;
						}
					//ok, now the movement, this is the tricky part
					 else
					 if(actors_list[i]->que[0]>=move_n && actors_list[i]->que[0]<=move_nw)
					 	{
							float rotation_angle;

							if(last_command<move_n || last_command>move_nw)//update the frame name too
							sprintf(actors_list[i]->cur_frame,actors_defs[actor_type].walk_frame);
							actors_list[i]->stop_animation=0;
							if(last_command!=actors_list[i]->que[0])//we need to calculate the rotation...
								{
									targeted_z_rot=(actors_list[i]->que[0]-move_n)*45.0f;
									rotation_angle=get_rotation_vector(z_rot,targeted_z_rot);
									actors_list[i]->rotate_z_speed=rotation_angle/6;
									if(auto_camera)
									if(actors_list[i]->actor_id==yourself)
										{
											camera_rotation_speed=rotation_angle/18;
											camera_rotation_frames=18;
										}

									actors_list[i]->rotate_frames_left=6;
									actors_list[i]->rotating=1;
								}
							else targeted_z_rot=z_rot;
							//ok, now calculate the motion vector...
							actors_list[i]->move_x_speed=actors_defs[actor_type].walk_speed*sin(targeted_z_rot*3.1415926/180.0);
							actors_list[i]->move_y_speed=actors_defs[actor_type].walk_speed*cos(targeted_z_rot*3.1415926/180.0);
							actors_list[i]->movement_frames_left=18/4;
							actors_list[i]->after_move_frames_left=0;
							actors_list[i]->moving=1;
							//test to see if we have a diagonal movement, and if we do, adjust the speeds

							if((actors_list[i]->move_x_speed>0.01f || actors_list[i]->move_x_speed<-0.01f)
							&& (actors_list[i]->move_y_speed>0.01f || actors_list[i]->move_y_speed<-0.01f))
								{
									actors_list[i]->move_x_speed*=1.4142315;
									actors_list[i]->move_y_speed*=1.4142315;
								}

/*
							if(actors_list[i]->actor_id==yourself)
								{
									camera_x_speed=actors_list[i]->move_x_speed/3.0;
									camera_x_frames=36;
									camera_y_speed=actors_list[i]->move_y_speed/3.0;
									camera_y_frames=36;
								}
*/
						}
					else
					if(actors_list[i]->que[0]>=turn_n && actors_list[i]->que[0]<=turn_nw)
						{
							float rotation_angle;
							targeted_z_rot=(actors_list[i]->que[0]-turn_n)*45.0f;
							rotation_angle=get_rotation_vector(z_rot,targeted_z_rot);
							actors_list[i]->rotate_z_speed=rotation_angle/6.0f;
							actors_list[i]->rotate_frames_left=6;
							actors_list[i]->rotating=1;
							actors_list[i]->stop_animation=1;
						}

					//mark the actor as being busy
					actors_list[i]->busy=1;
					//save the last command. It is especially good for run and walk
					actors_list[i]->last_command=actors_list[i]->que[0];
					//move que down with one command
					for(k=0;k<10-1;k++)
						{
							actors_list[i]->que[k]=actors_list[i]->que[k+1];
						}
					actors_list[i]->que[k]=nothing;
				 }
		}
}


void destroy_actor(int actor_id)
{
	int i=0;

	while(i<1000)
		{
			if(actors_list[i])
			if(actors_list[i]->actor_id==actor_id)
				{
					if(actors_list[i]->remapped_colors)glDeleteTextures(1,&actors_list[i]->texture_id);
					if(actors_list[i]->is_enhanced_model)
						{
							glDeleteTextures(1,&actors_list[i]->texture_id);
							free(actors_list[i]->body_parts);
						}
					free(actors_list[i]);
					actors_list[i]=0;
					break;
				}
			i++;
		}
}

void destroy_all_actors()
{
	int i=0;

	while(i<1000)
		{
			if(actors_list[i])
				{
					if(actors_list[i]->remapped_colors)glDeleteTextures(1,&actors_list[i]->texture_id);
					if(actors_list[i]->is_enhanced_model)
						{
							glDeleteTextures(1,&actors_list[i]->texture_id);
							free(actors_list[i]->body_parts);
						}
					free(actors_list[i]);
					actors_list[i]=0;
				}
			i++;
		}
}




void update_all_actors()
{
	Uint8 str[40];

	//we got a nasty error, log it
	str[0]=127+c_red2;
	my_strcp(&str[1],"Resync with the server...");
	put_text_in_buffer(str,strlen(str),0);

	destroy_all_actors();
	str[0]=SEND_ME_MY_ACTORS;
	my_tcp_send(my_socket,str,1);
}

void add_command_to_actor(int actor_id, char command)
{
	int i=0;
	int k=0;

	while(i<1000)
		{
			if(actors_list[i])
			if(actors_list[i]->actor_id==actor_id)
				{
					for(k=0;k<10;k++)
						{
							if(actors_list[i]->que[k]==nothing)
								{
									//we are SEVERLY behind, just update all the actors in range
									if(k>8)
										{
											update_all_actors();
											return;
										}
									actors_list[i]->que[k]=command;
									break;
								}
						}

					return;
				}
			i++;
		}
	//if we got here, it means we don't have this actor, so get it from the server...
}

void get_actor_damage(int actor_id, Uint8 damage)
{
	int i=0;

	while(i<1000)
		{
			if(actors_list[i])
			if(actors_list[i]->actor_id==actor_id)
				{
					actors_list[i]->damage=damage;
					actors_list[i]->damage_ms=2000;
					actors_list[i]->cur_health-=damage;
					return;
				}
			i++;
		}
	//if we got here, it means we don't have this actor, so get it from the server...
}

void get_actor_heal(int actor_id, Uint8 quantity)
{
	int i=0;

	while(i<1000)
		{
			if(actors_list[i])
			if(actors_list[i]->actor_id==actor_id)
				{
					actors_list[i]->cur_health+=quantity;
					return;
				}
			i++;
		}
	//if we got here, it means we don't have this actor, so get it from the server...
}


void move_self_forward()
{
	int i,x,y,rot,tx,ty;
	Uint8 str[10];

	i=0;
	while(i<1000)
			{
				if(actors_list[i])
				if(actors_list[i]->actor_id==yourself)
					 {
						x=actors_list[i]->x_tile_pos;
						y=actors_list[i]->y_tile_pos;
						rot=actors_list[i]->z_rot;
						rot=unwindAngle_Degrees(rot);
						if(rot==0)
							{
								tx=x;
								ty=y+1;
							}
						else
						if(rot==45)
							{
								tx=x+1;
								ty=y+1;
							}
						else
						if(rot==90)
							{
								tx=x+1;
								ty=y;
							}
						else
						if(rot==135)
							{
								tx=x+1;
								ty=y-1;
							}
						else
						if(rot==180)
							{
								tx=x;
								ty=y-1;
							}
						else
						if(rot==225)
							{
								tx=x-1;
								ty=y-1;
							}
						else
						if(rot==270)
							{
								tx=x-1;
								ty=y;
							}
						if(rot==315)
							{
								tx=x-1;
								ty=y+1;
							}

						//check to see if the coordinates are OUTSIDE the map
						if(ty<0 || tx<0 || tx>=tile_map_size_x*6 || ty>=tile_map_size_y*6)return;

						str[0]=MOVE_TO;
						*((short *)(str+1))=tx;
						*((short *)(str+3))=ty;

						my_tcp_send(my_socket,str,5);
						return;
					 }
				i++;
			}

}


//speed=0.5/0.4;


void init_actor_defs()
{
		float default_walk_speed;

		default_walk_speed=2.0/18;

		actors_defs[human_female].ghost=0;
		sprintf(actors_defs[human_female].walk_frame,"walk01");
		sprintf(actors_defs[human_female].run_frame,"run01");
		sprintf(actors_defs[human_female].die1_frame,"dief01");
		sprintf(actors_defs[human_female].die2_frame,"dieb11");
		sprintf(actors_defs[human_female].pain1_frame,"pain01");
		sprintf(actors_defs[human_female].pick_frame,"pickup01");
		sprintf(actors_defs[human_female].drop_frame,"drop01");
		sprintf(actors_defs[human_female].idle_frame,"idle01");
		sprintf(actors_defs[human_female].idle_sit_frame,"sitidle01");
		sprintf(actors_defs[human_female].harvest_frame,"harvest01");
		sprintf(actors_defs[human_female].attack_cast_frame,"cast01");
		sprintf(actors_defs[human_female].sit_down_frame,"intosit01");
		sprintf(actors_defs[human_female].stand_up_frame,"outsit01");
		sprintf(actors_defs[human_female].in_combat_frame,"intofight01");
		sprintf(actors_defs[human_female].out_combat_frame,"outfight01");
		sprintf(actors_defs[human_female].combat_idle_frame,"fightidle01");

		sprintf(actors_defs[human_female].shirt[SHIRT_BLACK].arms_name,"./md2/arms1_black.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_BLACK].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_BLACK].torso_name,"./md2/torso1_black.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_BLUE].arms_name,"./md2/arms1_blue.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_BLUE].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_BLUE].torso_name,"./md2/torso1_blue.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_BROWN].arms_name,"./md2/arms1_brown.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_BROWN].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_BROWN].torso_name,"./md2/torso1_brown.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_GREY].arms_name,"./md2/arms1_gray.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_GREY].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_GREY].torso_name,"./md2/torso1_gray.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_GREEN].arms_name,"./md2/arms1_green.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_GREEN].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_GREEN].torso_name,"./md2/torso1_green.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_LIGHTBROWN].arms_name,"./md2/arms1_lightbrown.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_LIGHTBROWN].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_LIGHTBROWN].torso_name,"./md2/torso1_lightbrown.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_ORANGE].arms_name,"./md2/arms1_orange.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_ORANGE].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_ORANGE].torso_name,"./md2/torso1_orange.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_PINK].arms_name,"./md2/arms1_pink.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_PINK].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_PINK].torso_name,"./md2/torso1_pink.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_PURPLE].arms_name,"./md2/arms1_purple.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_PURPLE].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_PURPLE].torso_name,"./md2/torso1_purple.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_RED].arms_name,"./md2/arms1_red.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_RED].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_RED].torso_name,"./md2/torso1_red.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_WHITE].arms_name,"./md2/arms1_white.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_WHITE].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_WHITE].torso_name,"./md2/torso1_white.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_YELLOW].arms_name,"./md2/arms1_yellow.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_YELLOW].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_YELLOW].torso_name,"./md2/torso1_yellow.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_LEATHER_ARMOR].arms_name,"./md2/arms2.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_LEATHER_ARMOR].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_LEATHER_ARMOR].torso_name,"./md2/torso2.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_CHAIN_ARMOR].arms_name,"./md2/arms3.bmp");
		sprintf(actors_defs[human_female].shirt[SHIRT_CHAIN_ARMOR].model_name,"./md2/torso1_humanf.md2");
		sprintf(actors_defs[human_female].shirt[SHIRT_CHAIN_ARMOR].torso_name,"./md2/torso3.bmp");


		sprintf(actors_defs[human_female].skin[SKIN_BROWN].hands_name,"./md2/hands_brown.bmp");
		sprintf(actors_defs[human_female].skin[SKIN_BROWN].head_name,"./md2/head_humanfbrown.bmp");
		sprintf(actors_defs[human_female].skin[SKIN_NORMAL].hands_name,"./md2/hands_normal.bmp");
		sprintf(actors_defs[human_female].skin[SKIN_NORMAL].head_name,"./md2/head_humanfnormal.bmp");
		sprintf(actors_defs[human_female].skin[SKIN_PALE].hands_name,"./md2/hands_pale.bmp");
		sprintf(actors_defs[human_female].skin[SKIN_PALE].head_name,"./md2/head_humanfpale.bmp");
		sprintf(actors_defs[human_female].skin[SKIN_TAN].hands_name,"./md2/hands_tan.bmp");
		sprintf(actors_defs[human_female].skin[SKIN_TAN].head_name,"./md2/head_humanftan.bmp");

		sprintf(actors_defs[human_female].hair[HAIR_BLACK].hair_name,"./md2/hair_black.bmp");
		sprintf(actors_defs[human_female].hair[HAIR_BLOND].hair_name,"./md2/hair_blond.bmp");
		sprintf(actors_defs[human_female].hair[HAIR_BROWN].hair_name,"./md2/hair_brown.bmp");
		sprintf(actors_defs[human_female].hair[HAIR_GRAY].hair_name,"./md2/hair_gray.bmp");
		sprintf(actors_defs[human_female].hair[HAIR_RED].hair_name,"./md2/hair_red.bmp");
		sprintf(actors_defs[human_female].hair[HAIR_WHITE].hair_name,"./md2/hair_white.bmp");

		sprintf(actors_defs[human_female].boots[BOOTS_BLACK].boots_name,"./md2/boots1_black.bmp");
		sprintf(actors_defs[human_female].boots[BOOTS_BROWN].boots_name,"./md2/boots1_brown.bmp");
		sprintf(actors_defs[human_female].boots[BOOTS_DARKBROWN].boots_name,"./md2/boots1_darkbrown.bmp");
		sprintf(actors_defs[human_female].boots[BOOTS_DULLBROWN].boots_name,"./md2/boots1_dullbrown.bmp");
		sprintf(actors_defs[human_female].boots[BOOTS_LIGHTBROWN].boots_name,"./md2/boots1_lightbrown.bmp");
		sprintf(actors_defs[human_female].boots[BOOTS_ORANGE].boots_name,"./md2/boots1_orange.bmp");

		sprintf(actors_defs[human_female].legs[PANTS_BLACK].legs_name,"./md2/pants1_black.bmp");
		sprintf(actors_defs[human_female].legs[PANTS_BLACK].model_name,"./md2/legs1_humanf.md2");
		sprintf(actors_defs[human_female].legs[PANTS_BLUE].legs_name,"./md2/pants1_blue.bmp");
		sprintf(actors_defs[human_female].legs[PANTS_BLUE].model_name,"./md2/legs1_humanf.md2");
		sprintf(actors_defs[human_female].legs[PANTS_BROWN].legs_name,"./md2/pants1_brown.bmp");
		sprintf(actors_defs[human_female].legs[PANTS_BROWN].model_name,"./md2/legs1_humanf.md2");
		sprintf(actors_defs[human_female].legs[PANTS_DARKBROWN].legs_name,"./md2/pants1_darkbrown.bmp");
		sprintf(actors_defs[human_female].legs[PANTS_DARKBROWN].model_name,"./md2/legs1_humanf.md2");
		sprintf(actors_defs[human_female].legs[PANTS_GREY].legs_name,"./md2/pants1_gray.bmp");
		sprintf(actors_defs[human_female].legs[PANTS_GREY].model_name,"./md2/legs1_humanf.md2");
		sprintf(actors_defs[human_female].legs[PANTS_GREEN].legs_name,"./md2/pants1_green.bmp");
		sprintf(actors_defs[human_female].legs[PANTS_GREEN].model_name,"./md2/legs1_humanf.md2");
		sprintf(actors_defs[human_female].legs[PANTS_LIGHTBROWN].legs_name,"./md2/pants1_lightbrown.bmp");
		sprintf(actors_defs[human_female].legs[PANTS_LIGHTBROWN].model_name,"./md2/legs1_humanf.md2");
		sprintf(actors_defs[human_female].legs[PANTS_RED].legs_name,"./md2/pants1_red.bmp");
		sprintf(actors_defs[human_female].legs[PANTS_RED].model_name,"./md2/legs1_humanf.md2");
		sprintf(actors_defs[human_female].legs[PANTS_WHITE].legs_name,"./md2/pants1_white.bmp");
		sprintf(actors_defs[human_female].legs[PANTS_WHITE].model_name,"./md2/legs1_humanf.md2");

		sprintf(actors_defs[human_female].cape[CAPE_BLACK].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[human_female].cape[CAPE_BLACK].skin_name,"./md2/cape1_black.bmp");
		sprintf(actors_defs[human_female].cape[CAPE_BLUE].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[human_female].cape[CAPE_BLUE].skin_name,"./md2/cape1_blue.bmp");
		sprintf(actors_defs[human_female].cape[CAPE_BLUEGRAY].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[human_female].cape[CAPE_BLUEGRAY].skin_name,"./md2/cape1_bluegray.bmp");
		sprintf(actors_defs[human_female].cape[CAPE_BROWN].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[human_female].cape[CAPE_BROWN].skin_name,"./md2/cape1_brown.bmp");
		sprintf(actors_defs[human_female].cape[CAPE_BROWNGRAY].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[human_female].cape[CAPE_BROWNGRAY].skin_name,"./md2/cape1_browngray.bmp");
		sprintf(actors_defs[human_female].cape[CAPE_GRAY].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[human_female].cape[CAPE_GRAY].skin_name,"./md2/cape1_gray.bmp");
		sprintf(actors_defs[human_female].cape[CAPE_GREEN].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[human_female].cape[CAPE_GREEN].skin_name,"./md2/cape1_green.bmp");
		sprintf(actors_defs[human_female].cape[CAPE_GREENGRAY].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[human_female].cape[CAPE_GREENGRAY].skin_name,"./md2/cape1_greengray.bmp");
		sprintf(actors_defs[human_female].cape[CAPE_PURPLE].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[human_female].cape[CAPE_PURPLE].skin_name,"./md2/cape1_purple.bmp");
		sprintf(actors_defs[human_female].cape[CAPE_WHITE].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[human_female].cape[CAPE_WHITE].skin_name,"./md2/cape1_white.bmp");

		sprintf(actors_defs[human_female].head[HEAD_1].model_name,"./md2/head1_humanf.md2");
		sprintf(actors_defs[human_female].head[HEAD_2].model_name,"./md2/head2_humanf.md2");
		sprintf(actors_defs[human_female].head[HEAD_3].model_name,"./md2/head3_humanf.md2");
		sprintf(actors_defs[human_female].head[HEAD_4].model_name,"./md2/head4_humanf.md2");
		sprintf(actors_defs[human_female].head[HEAD_5].model_name,"./md2/head5_humanf.md2");

		sprintf(actors_defs[human_female].shield[SHIELD_WOOD].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[human_female].shield[SHIELD_WOOD].skin_name,"./md2/shield1_wood1.bmp");
		sprintf(actors_defs[human_female].shield[SHIELD_WOOD_ENHANCED].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[human_female].shield[SHIELD_WOOD_ENHANCED].skin_name,"./md2/shield1_wood2.bmp");
		sprintf(actors_defs[human_female].shield[SHIELD_IRON].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[human_female].shield[SHIELD_IRON].skin_name,"./md2/shield1_iron.bmp");
		sprintf(actors_defs[human_female].shield[SHIELD_STEEL].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[human_female].shield[SHIELD_STEEL].skin_name,"./md2/shield1_steel.bmp");


		sprintf(actors_defs[human_female].weapon[WEAPON_NONE].model_name,"");
		sprintf(actors_defs[human_female].weapon[WEAPON_NONE].skin_name,"");
		sprintf(actors_defs[human_female].weapon[WEAPON_NONE].attack_up1,"punchone01");
		sprintf(actors_defs[human_female].weapon[WEAPON_NONE].attack_up2,"punchtwo01");
		sprintf(actors_defs[human_female].weapon[WEAPON_NONE].attack_down1,"kickone01");
		sprintf(actors_defs[human_female].weapon[WEAPON_NONE].attack_down2,"kicktwo01");
		sprintf(actors_defs[human_female].weapon[SWORD_1].model_name,"./md2/sword1_tall.md2");
		sprintf(actors_defs[human_female].weapon[SWORD_1].skin_name,"./md2/sword1.bmp");
		sprintf(actors_defs[human_female].weapon[SWORD_1].attack_up1,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_1].attack_up2,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_1].attack_down1,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_1].attack_down2,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_2].model_name,"./md2/sword2_tall.md2");
		sprintf(actors_defs[human_female].weapon[SWORD_2].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[human_female].weapon[SWORD_2].attack_up1,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_2].attack_up2,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_2].attack_down1,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_2].attack_down2,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_3].model_name,"./md2/sword3_tall.md2");
		sprintf(actors_defs[human_female].weapon[SWORD_3].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[human_female].weapon[SWORD_3].attack_up1,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_3].attack_up2,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_3].attack_down1,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_3].attack_down2,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_4].model_name,"./md2/sword4_tall.md2");
		sprintf(actors_defs[human_female].weapon[SWORD_4].skin_name,"./md2/sword4.bmp");
		sprintf(actors_defs[human_female].weapon[SWORD_4].attack_up1,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_4].attack_up2,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_4].attack_down1,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_4].attack_down2,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_5].model_name,"./md2/sword5_tall.md2");
		sprintf(actors_defs[human_female].weapon[SWORD_5].skin_name,"./md2/sword5.bmp");
		sprintf(actors_defs[human_female].weapon[SWORD_5].attack_up1,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_5].attack_up2,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_5].attack_down1,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_5].attack_down2,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_6].model_name,"./md2/sword6_tall.md2");
		sprintf(actors_defs[human_female].weapon[SWORD_6].skin_name,"./md2/sword6.bmp");
		sprintf(actors_defs[human_female].weapon[SWORD_6].attack_up1,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_6].attack_up2,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_6].attack_down1,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_6].attack_down2,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_7].model_name,"./md2/sword7_tall.md2");
		sprintf(actors_defs[human_female].weapon[SWORD_7].skin_name,"./md2/sword7.bmp");
		sprintf(actors_defs[human_female].weapon[SWORD_7].attack_up1,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_7].attack_up2,"slashone01");
		sprintf(actors_defs[human_female].weapon[SWORD_7].attack_down1,"hackone01");
		sprintf(actors_defs[human_female].weapon[SWORD_7].attack_down2,"hackone01");
		sprintf(actors_defs[human_female].weapon[STAFF_1].model_name,"./md2/staff1_tall.md2");
		sprintf(actors_defs[human_female].weapon[STAFF_1].skin_name,"./md2/staff1_brown.bmp");
		sprintf(actors_defs[human_female].weapon[STAFF_1].attack_up1,"slashtwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_1].attack_up2,"slashtwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_1].attack_down1,"hacktwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_1].attack_down2,"hacktwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_2].model_name,"./md2/staff1_tall.md2");
		sprintf(actors_defs[human_female].weapon[STAFF_2].skin_name,"./md2/staff1_green.bmp");
		sprintf(actors_defs[human_female].weapon[STAFF_2].attack_up1,"slashtwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_2].attack_up2,"slashtwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_2].attack_down1,"hacktwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_2].attack_down2,"hacktwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_3].model_name,"./md2/staff2_tall.md2");
		sprintf(actors_defs[human_female].weapon[STAFF_3].skin_name,"./md2/staff3.bmp");
		sprintf(actors_defs[human_female].weapon[STAFF_3].attack_up1,"slashtwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_3].attack_up2,"slashtwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_3].attack_down1,"hacktwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_3].attack_down2,"hacktwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_4].model_name,"./md2/staff3_tall.md2");
		sprintf(actors_defs[human_female].weapon[STAFF_4].skin_name,"./md2/staff4.bmp");
		sprintf(actors_defs[human_female].weapon[STAFF_4].attack_up1,"slashtwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_4].attack_up2,"slashtwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_4].attack_down1,"hacktwo01");
		sprintf(actors_defs[human_female].weapon[STAFF_4].attack_down2,"hacktwo01");
		sprintf(actors_defs[human_female].weapon[HAMMER_1].model_name,"./md2/warhammer1_tall.md2");
		sprintf(actors_defs[human_female].weapon[HAMMER_1].skin_name,"./md2/warhammer1.bmp");
		sprintf(actors_defs[human_female].weapon[HAMMER_1].attack_up1,"slashone01");
		sprintf(actors_defs[human_female].weapon[HAMMER_1].attack_up2,"slashone01");
		sprintf(actors_defs[human_female].weapon[HAMMER_1].attack_down1,"hackone01");
		sprintf(actors_defs[human_female].weapon[HAMMER_1].attack_down2,"hackone01");
		sprintf(actors_defs[human_female].weapon[HAMMER_2].model_name,"./md2/warhammer2_tall.md2");
		sprintf(actors_defs[human_female].weapon[HAMMER_2].skin_name,"./md2/warhammer2.bmp");
		sprintf(actors_defs[human_female].weapon[HAMMER_2].attack_up1,"slashone01");
		sprintf(actors_defs[human_female].weapon[HAMMER_2].attack_up2,"slashone01");
		sprintf(actors_defs[human_female].weapon[HAMMER_2].attack_down1,"hackone01");
		sprintf(actors_defs[human_female].weapon[HAMMER_2].attack_down2,"hackone01");
		sprintf(actors_defs[human_female].weapon[PICKAX].model_name,"./md2/pickaxe1_tall.md2");
		sprintf(actors_defs[human_female].weapon[PICKAX].skin_name,"./md2/pickaxe1.bmp");
		sprintf(actors_defs[human_female].weapon[PICKAX].attack_up1,"slashone01");
		sprintf(actors_defs[human_female].weapon[PICKAX].attack_up2,"slashone01");
		sprintf(actors_defs[human_female].weapon[PICKAX].attack_down1,"hackone01");
		sprintf(actors_defs[human_female].weapon[PICKAX].attack_down2,"hackone01");

		sprintf(actors_defs[human_female].helmet[HELMET_IRON].model_name,"./md2/helmet1_tallf.md2");
		sprintf(actors_defs[human_female].helmet[HELMET_IRON].skin_name,"./md2/helmet1.bmp");

		actors_defs[human_female].walk_speed=default_walk_speed;
		actors_defs[human_female].run_speed=2.0/18;
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
		actors_defs[human_male].ghost=0;
		sprintf(actors_defs[human_male].skin_name,"./md2/human_male1.bmp");
		sprintf(actors_defs[human_male].file_name,"./md2/human_male.md2");
		sprintf(actors_defs[human_male].walk_frame,"walk01");
		sprintf(actors_defs[human_male].run_frame,"run01");
		sprintf(actors_defs[human_male].die1_frame,"dief01");
		sprintf(actors_defs[human_male].die2_frame,"dieb11");
		sprintf(actors_defs[human_male].pain1_frame,"pain01");
		sprintf(actors_defs[human_male].pick_frame,"pickup01");
		sprintf(actors_defs[human_male].drop_frame,"drop01");
		sprintf(actors_defs[human_male].idle_frame,"idle01");
		sprintf(actors_defs[human_male].idle_sit_frame,"sitidle01");
		sprintf(actors_defs[human_male].harvest_frame,"harvest01");
		sprintf(actors_defs[human_male].attack_cast_frame,"cast01");
		sprintf(actors_defs[human_male].sit_down_frame,"intosit01");
		sprintf(actors_defs[human_male].stand_up_frame,"outsit01");
		sprintf(actors_defs[human_male].in_combat_frame,"intofight01");
		sprintf(actors_defs[human_male].out_combat_frame,"outfight01");
		sprintf(actors_defs[human_male].combat_idle_frame,"fightidle01");

		sprintf(actors_defs[human_male].shirt[SHIRT_BLACK].arms_name,"./md2/arms1_black.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_BLACK].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_BLACK].torso_name,"./md2/torso1_black.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_BLUE].arms_name,"./md2/arms1_blue.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_BLUE].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_BLUE].torso_name,"./md2/torso1_blue.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_BROWN].arms_name,"./md2/arms1_brown.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_BROWN].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_BROWN].torso_name,"./md2/torso1_brown.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_GREY].arms_name,"./md2/arms1_gray.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_GREY].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_GREY].torso_name,"./md2/torso1_gray.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_GREEN].arms_name,"./md2/arms1_green.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_GREEN].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_GREEN].torso_name,"./md2/torso1_green.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_LIGHTBROWN].arms_name,"./md2/arms1_lightbrown.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_LIGHTBROWN].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_LIGHTBROWN].torso_name,"./md2/torso1_lightbrown.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_ORANGE].arms_name,"./md2/arms1_orange.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_ORANGE].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_ORANGE].torso_name,"./md2/torso1_orange.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_PINK].arms_name,"./md2/arms1_pink.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_PINK].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_PINK].torso_name,"./md2/torso1_pink.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_PURPLE].arms_name,"./md2/arms1_purple.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_PURPLE].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_PURPLE].torso_name,"./md2/torso1_purple.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_RED].arms_name,"./md2/arms1_red.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_RED].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_RED].torso_name,"./md2/torso1_red.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_WHITE].arms_name,"./md2/arms1_white.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_WHITE].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_WHITE].torso_name,"./md2/torso1_white.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_YELLOW].arms_name,"./md2/arms1_yellow.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_YELLOW].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_YELLOW].torso_name,"./md2/torso1_yellow.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_LEATHER_ARMOR].arms_name,"./md2/arms2.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_LEATHER_ARMOR].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_LEATHER_ARMOR].torso_name,"./md2/torso2.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_CHAIN_ARMOR].arms_name,"./md2/arms3.bmp");
		sprintf(actors_defs[human_male].shirt[SHIRT_CHAIN_ARMOR].model_name,"./md2/torso1_humanm.md2");
		sprintf(actors_defs[human_male].shirt[SHIRT_CHAIN_ARMOR].torso_name,"./md2/torso3.bmp");

		sprintf(actors_defs[human_male].skin[SKIN_BROWN].hands_name,"./md2/hands_brown.bmp");
		sprintf(actors_defs[human_male].skin[SKIN_BROWN].head_name,"./md2/head_humanmbrown.bmp");
		sprintf(actors_defs[human_male].skin[SKIN_NORMAL].hands_name,"./md2/hands_normal.bmp");
		sprintf(actors_defs[human_male].skin[SKIN_NORMAL].head_name,"./md2/head_humanmnormal.bmp");
		sprintf(actors_defs[human_male].skin[SKIN_PALE].hands_name,"./md2/hands_pale.bmp");
		sprintf(actors_defs[human_male].skin[SKIN_PALE].head_name,"./md2/head_humanmpale.bmp");
		sprintf(actors_defs[human_male].skin[SKIN_TAN].hands_name,"./md2/hands_tan.bmp");
		sprintf(actors_defs[human_male].skin[SKIN_TAN].head_name,"./md2/head_humanmtan.bmp");

		sprintf(actors_defs[human_male].hair[HAIR_BLACK].hair_name,"./md2/hair_black.bmp");
		sprintf(actors_defs[human_male].hair[HAIR_BLOND].hair_name,"./md2/hair_blond.bmp");
		sprintf(actors_defs[human_male].hair[HAIR_BROWN].hair_name,"./md2/hair_brown.bmp");
		sprintf(actors_defs[human_male].hair[HAIR_GRAY].hair_name,"./md2/hair_gray.bmp");
		sprintf(actors_defs[human_male].hair[HAIR_RED].hair_name,"./md2/hair_red.bmp");
		sprintf(actors_defs[human_male].hair[HAIR_WHITE].hair_name,"./md2/hair_white.bmp");

		sprintf(actors_defs[human_male].boots[BOOTS_BLACK].boots_name,"./md2/boots1_black.bmp");
		sprintf(actors_defs[human_male].boots[BOOTS_BROWN].boots_name,"./md2/boots1_brown.bmp");
		sprintf(actors_defs[human_male].boots[BOOTS_DARKBROWN].boots_name,"./md2/boots1_darkbrown.bmp");
		sprintf(actors_defs[human_male].boots[BOOTS_DULLBROWN].boots_name,"./md2/boots1_dullbrown.bmp");
		sprintf(actors_defs[human_male].boots[BOOTS_LIGHTBROWN].boots_name,"./md2/boots1_lightbrown.bmp");
		sprintf(actors_defs[human_male].boots[BOOTS_ORANGE].boots_name,"./md2/boots1_orange.bmp");

		sprintf(actors_defs[human_male].legs[PANTS_BLACK].legs_name,"./md2/pants1_black.bmp");
		sprintf(actors_defs[human_male].legs[PANTS_BLACK].model_name,"./md2/legs1_humanm.md2");
		sprintf(actors_defs[human_male].legs[PANTS_BLUE].legs_name,"./md2/pants1_blue.bmp");
		sprintf(actors_defs[human_male].legs[PANTS_BLUE].model_name,"./md2/legs1_humanm.md2");
		sprintf(actors_defs[human_male].legs[PANTS_BROWN].legs_name,"./md2/pants1_brown.bmp");
		sprintf(actors_defs[human_male].legs[PANTS_BROWN].model_name,"./md2/legs1_humanm.md2");
		sprintf(actors_defs[human_male].legs[PANTS_DARKBROWN].legs_name,"./md2/pants1_darkbrown.bmp");
		sprintf(actors_defs[human_male].legs[PANTS_DARKBROWN].model_name,"./md2/legs1_humanm.md2");
		sprintf(actors_defs[human_male].legs[PANTS_GREY].legs_name,"./md2/pants1_gray.bmp");
		sprintf(actors_defs[human_male].legs[PANTS_GREY].model_name,"./md2/legs1_humanm.md2");
		sprintf(actors_defs[human_male].legs[PANTS_GREEN].legs_name,"./md2/pants1_green.bmp");
		sprintf(actors_defs[human_male].legs[PANTS_GREEN].model_name,"./md2/legs1_humanm.md2");
		sprintf(actors_defs[human_male].legs[PANTS_LIGHTBROWN].legs_name,"./md2/pants1_lightbrown.bmp");
		sprintf(actors_defs[human_male].legs[PANTS_LIGHTBROWN].model_name,"./md2/legs1_humanm.md2");
		sprintf(actors_defs[human_male].legs[PANTS_RED].legs_name,"./md2/pants1_red.bmp");
		sprintf(actors_defs[human_male].legs[PANTS_RED].model_name,"./md2/legs1_humanm.md2");
		sprintf(actors_defs[human_male].legs[PANTS_WHITE].legs_name,"./md2/pants1_white.bmp");
		sprintf(actors_defs[human_male].legs[PANTS_WHITE].model_name,"./md2/legs1_humanm.md2");

		sprintf(actors_defs[human_male].cape[CAPE_BLACK].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[human_male].cape[CAPE_BLACK].skin_name,"./md2/cape1_black.bmp");
		sprintf(actors_defs[human_male].cape[CAPE_BLUE].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[human_male].cape[CAPE_BLUE].skin_name,"./md2/cape1_blue.bmp");
		sprintf(actors_defs[human_male].cape[CAPE_BLUEGRAY].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[human_male].cape[CAPE_BLUEGRAY].skin_name,"./md2/cape1_bluegray.bmp");
		sprintf(actors_defs[human_male].cape[CAPE_BROWN].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[human_male].cape[CAPE_BROWN].skin_name,"./md2/cape1_brown.bmp");
		sprintf(actors_defs[human_male].cape[CAPE_BROWNGRAY].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[human_male].cape[CAPE_BROWNGRAY].skin_name,"./md2/cape1_browngray.bmp");
		sprintf(actors_defs[human_male].cape[CAPE_GRAY].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[human_male].cape[CAPE_GRAY].skin_name,"./md2/cape1_gray.bmp");
		sprintf(actors_defs[human_male].cape[CAPE_GREEN].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[human_male].cape[CAPE_GREEN].skin_name,"./md2/cape1_green.bmp");
		sprintf(actors_defs[human_male].cape[CAPE_GREENGRAY].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[human_male].cape[CAPE_GREENGRAY].skin_name,"./md2/cape1_greengray.bmp");
		sprintf(actors_defs[human_male].cape[CAPE_PURPLE].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[human_male].cape[CAPE_PURPLE].skin_name,"./md2/cape1_purple.bmp");
		sprintf(actors_defs[human_male].cape[CAPE_WHITE].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[human_male].cape[CAPE_WHITE].skin_name,"./md2/cape1_white.bmp");

		sprintf(actors_defs[human_male].head[HEAD_1].model_name,"./md2/head1_humanm.md2");
		sprintf(actors_defs[human_male].head[HEAD_2].model_name,"./md2/head2_humanm.md2");
		sprintf(actors_defs[human_male].head[HEAD_3].model_name,"./md2/head3_humanm.md2");
		sprintf(actors_defs[human_male].head[HEAD_4].model_name,"./md2/head4_humanm.md2");
		sprintf(actors_defs[human_male].head[HEAD_5].model_name,"./md2/head5_humanm.md2");

		sprintf(actors_defs[human_male].shield[SHIELD_WOOD].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[human_male].shield[SHIELD_WOOD].skin_name,"./md2/shield1_wood1.bmp");
		sprintf(actors_defs[human_male].shield[SHIELD_WOOD_ENHANCED].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[human_male].shield[SHIELD_WOOD_ENHANCED].skin_name,"./md2/shield1_wood2.bmp");
		sprintf(actors_defs[human_male].shield[SHIELD_IRON].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[human_male].shield[SHIELD_IRON].skin_name,"./md2/shield1_iron.bmp");
		sprintf(actors_defs[human_male].shield[SHIELD_STEEL].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[human_male].shield[SHIELD_STEEL].skin_name,"./md2/shield1_steel.bmp");


		sprintf(actors_defs[human_male].weapon[WEAPON_NONE].model_name,"");
		sprintf(actors_defs[human_male].weapon[WEAPON_NONE].skin_name,"");
		sprintf(actors_defs[human_male].weapon[WEAPON_NONE].attack_up1,"punchone01");
		sprintf(actors_defs[human_male].weapon[WEAPON_NONE].attack_up2,"punchtwo01");
		sprintf(actors_defs[human_male].weapon[WEAPON_NONE].attack_down1,"kickone01");
		sprintf(actors_defs[human_male].weapon[WEAPON_NONE].attack_down2,"kicktwo01");
		sprintf(actors_defs[human_male].weapon[SWORD_1].model_name,"./md2/sword1_tall.md2");
		sprintf(actors_defs[human_male].weapon[SWORD_1].skin_name,"./md2/sword1.bmp");
		sprintf(actors_defs[human_male].weapon[SWORD_1].attack_up1,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_1].attack_up2,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_1].attack_down1,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_1].attack_down2,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_2].model_name,"./md2/sword2_tall.md2");
		sprintf(actors_defs[human_male].weapon[SWORD_2].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[human_male].weapon[SWORD_2].attack_up1,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_2].attack_up2,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_2].attack_down1,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_2].attack_down2,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_3].model_name,"./md2/sword3_tall.md2");
		sprintf(actors_defs[human_male].weapon[SWORD_3].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[human_male].weapon[SWORD_3].attack_up1,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_3].attack_up2,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_3].attack_down1,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_3].attack_down2,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_4].model_name,"./md2/sword4_tall.md2");
		sprintf(actors_defs[human_male].weapon[SWORD_4].skin_name,"./md2/sword4.bmp");
		sprintf(actors_defs[human_male].weapon[SWORD_4].attack_up1,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_4].attack_up2,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_4].attack_down1,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_4].attack_down2,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_5].model_name,"./md2/sword5_tall.md2");
		sprintf(actors_defs[human_male].weapon[SWORD_5].skin_name,"./md2/sword5.bmp");
		sprintf(actors_defs[human_male].weapon[SWORD_5].attack_up1,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_5].attack_up2,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_5].attack_down1,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_5].attack_down2,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_6].model_name,"./md2/sword6_tall.md2");
		sprintf(actors_defs[human_male].weapon[SWORD_6].skin_name,"./md2/sword6.bmp");
		sprintf(actors_defs[human_male].weapon[SWORD_6].attack_up1,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_6].attack_up2,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_6].attack_down1,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_6].attack_down2,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_7].model_name,"./md2/sword7_tall.md2");
		sprintf(actors_defs[human_male].weapon[SWORD_7].skin_name,"./md2/sword7.bmp");
		sprintf(actors_defs[human_male].weapon[SWORD_7].attack_up1,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_7].attack_up2,"slashone01");
		sprintf(actors_defs[human_male].weapon[SWORD_7].attack_down1,"hackone01");
		sprintf(actors_defs[human_male].weapon[SWORD_7].attack_down2,"hackone01");
		sprintf(actors_defs[human_male].weapon[STAFF_1].model_name,"./md2/staff1_tall.md2");
		sprintf(actors_defs[human_male].weapon[STAFF_1].skin_name,"./md2/staff1_brown.bmp");
		sprintf(actors_defs[human_male].weapon[STAFF_1].attack_up1,"slashtwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_1].attack_up2,"slashtwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_1].attack_down1,"hacktwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_1].attack_down2,"hacktwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_2].model_name,"./md2/staff1_tall.md2");
		sprintf(actors_defs[human_male].weapon[STAFF_2].skin_name,"./md2/staff1_green.bmp");
		sprintf(actors_defs[human_male].weapon[STAFF_2].attack_up1,"slashtwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_2].attack_up2,"slashtwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_2].attack_down1,"hacktwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_2].attack_down2,"hacktwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_3].model_name,"./md2/staff2_tall.md2");
		sprintf(actors_defs[human_male].weapon[STAFF_3].skin_name,"./md2/staff3.bmp");
		sprintf(actors_defs[human_male].weapon[STAFF_3].attack_up1,"slashtwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_3].attack_up2,"slashtwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_3].attack_down1,"hacktwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_3].attack_down2,"hacktwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_4].model_name,"./md2/staff3_tall.md2");
		sprintf(actors_defs[human_male].weapon[STAFF_4].skin_name,"./md2/staff4.bmp");
		sprintf(actors_defs[human_male].weapon[STAFF_4].attack_up1,"slashtwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_4].attack_up2,"slashtwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_4].attack_down1,"hacktwo01");
		sprintf(actors_defs[human_male].weapon[STAFF_4].attack_down2,"hacktwo01");
		sprintf(actors_defs[human_male].weapon[HAMMER_1].model_name,"./md2/warhammer1_tall.md2");
		sprintf(actors_defs[human_male].weapon[HAMMER_1].skin_name,"./md2/warhammer1.bmp");
		sprintf(actors_defs[human_male].weapon[HAMMER_1].attack_up1,"slashone01");
		sprintf(actors_defs[human_male].weapon[HAMMER_1].attack_up2,"slashone01");
		sprintf(actors_defs[human_male].weapon[HAMMER_1].attack_down1,"hackone01");
		sprintf(actors_defs[human_male].weapon[HAMMER_1].attack_down2,"hackone01");
		sprintf(actors_defs[human_male].weapon[HAMMER_2].model_name,"./md2/warhammer2_tall.md2");
		sprintf(actors_defs[human_male].weapon[HAMMER_2].skin_name,"./md2/warhammer2.bmp");
		sprintf(actors_defs[human_male].weapon[HAMMER_2].attack_up1,"slashone01");
		sprintf(actors_defs[human_male].weapon[HAMMER_2].attack_up2,"slashone01");
		sprintf(actors_defs[human_male].weapon[HAMMER_2].attack_down1,"hackone01");
		sprintf(actors_defs[human_male].weapon[HAMMER_2].attack_down2,"hackone01");
		sprintf(actors_defs[human_male].weapon[PICKAX].model_name,"./md2/pickaxe1_tall.md2");
		sprintf(actors_defs[human_male].weapon[PICKAX].skin_name,"./md2/pickaxe1.bmp");
		sprintf(actors_defs[human_male].weapon[PICKAX].attack_up1,"slashone01");
		sprintf(actors_defs[human_male].weapon[PICKAX].attack_up2,"slashone01");
		sprintf(actors_defs[human_male].weapon[PICKAX].attack_down1,"hackone01");
		sprintf(actors_defs[human_male].weapon[PICKAX].attack_down2,"hackone01");

		sprintf(actors_defs[human_male].helmet[HELMET_IRON].model_name,"./md2/helmet1_tall.md2");
		sprintf(actors_defs[human_male].helmet[HELMET_IRON].skin_name,"./md2/helmet1.bmp");

		actors_defs[human_male].walk_speed=default_walk_speed;
		actors_defs[human_male].run_speed=2.0/18;
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
		actors_defs[elf_female].ghost=0;
		sprintf(actors_defs[elf_female].skin_name,"./md2/elf_female1.bmp");
		sprintf(actors_defs[elf_female].file_name,"./md2/elf_female.md2");
		sprintf(actors_defs[elf_female].walk_frame,"walk01");
		sprintf(actors_defs[elf_female].run_frame,"run01");
		sprintf(actors_defs[elf_female].die1_frame,"dief01");
		sprintf(actors_defs[elf_female].die2_frame,"dieb11");
		sprintf(actors_defs[elf_female].pain1_frame,"pain01");
		sprintf(actors_defs[elf_female].pick_frame,"pickup01");
		sprintf(actors_defs[elf_female].drop_frame,"drop01");
		sprintf(actors_defs[elf_female].idle_frame,"idle01");
		sprintf(actors_defs[elf_female].idle_sit_frame,"sitidle01");
		sprintf(actors_defs[elf_female].harvest_frame,"harvest01");
		sprintf(actors_defs[elf_female].attack_cast_frame,"cast01");
		sprintf(actors_defs[elf_female].sit_down_frame,"intosit01");
		sprintf(actors_defs[elf_female].stand_up_frame,"outsit01");
		sprintf(actors_defs[elf_female].in_combat_frame,"intofight01");
		sprintf(actors_defs[elf_female].out_combat_frame,"outfight01");
		sprintf(actors_defs[elf_female].combat_idle_frame,"fightidle01");

		sprintf(actors_defs[elf_female].shirt[SHIRT_BLACK].arms_name,"./md2/arms1_black.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_BLACK].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_BLACK].torso_name,"./md2/torso1_black.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_BLUE].arms_name,"./md2/arms1_blue.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_BLUE].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_BLUE].torso_name,"./md2/torso1_blue.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_BROWN].arms_name,"./md2/arms1_brown.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_BROWN].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_BROWN].torso_name,"./md2/torso1_brown.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_GREY].arms_name,"./md2/arms1_gray.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_GREY].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_GREY].torso_name,"./md2/torso1_gray.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_GREEN].arms_name,"./md2/arms1_green.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_GREEN].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_GREEN].torso_name,"./md2/torso1_green.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_LIGHTBROWN].arms_name,"./md2/arms1_lightbrown.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_LIGHTBROWN].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_LIGHTBROWN].torso_name,"./md2/torso1_lightbrown.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_ORANGE].arms_name,"./md2/arms1_orange.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_ORANGE].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_ORANGE].torso_name,"./md2/torso1_orange.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_PINK].arms_name,"./md2/arms1_pink.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_PINK].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_PINK].torso_name,"./md2/torso1_pink.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_PURPLE].arms_name,"./md2/arms1_purple.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_PURPLE].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_PURPLE].torso_name,"./md2/torso1_purple.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_RED].arms_name,"./md2/arms1_red.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_RED].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_RED].torso_name,"./md2/torso1_red.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_WHITE].arms_name,"./md2/arms1_white.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_WHITE].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_WHITE].torso_name,"./md2/torso1_white.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_YELLOW].arms_name,"./md2/arms1_yellow.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_YELLOW].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_YELLOW].torso_name,"./md2/torso1_yellow.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_LEATHER_ARMOR].arms_name,"./md2/arms2.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_LEATHER_ARMOR].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_LEATHER_ARMOR].torso_name,"./md2/torso2.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_CHAIN_ARMOR].arms_name,"./md2/arms3.bmp");
		sprintf(actors_defs[elf_female].shirt[SHIRT_CHAIN_ARMOR].model_name,"./md2/torso1_elff.md2");
		sprintf(actors_defs[elf_female].shirt[SHIRT_CHAIN_ARMOR].torso_name,"./md2/torso3.bmp");

		sprintf(actors_defs[elf_female].skin[SKIN_BROWN].hands_name,"./md2/hands_brown.bmp");
		sprintf(actors_defs[elf_female].skin[SKIN_BROWN].head_name,"./md2/head_humanfbrown.bmp");
		sprintf(actors_defs[elf_female].skin[SKIN_NORMAL].hands_name,"./md2/hands_normal.bmp");
		sprintf(actors_defs[elf_female].skin[SKIN_NORMAL].head_name,"./md2/head_humanfnormal.bmp");
		sprintf(actors_defs[elf_female].skin[SKIN_PALE].hands_name,"./md2/hands_pale.bmp");
		sprintf(actors_defs[elf_female].skin[SKIN_PALE].head_name,"./md2/head_humanfpale.bmp");
		sprintf(actors_defs[elf_female].skin[SKIN_TAN].hands_name,"./md2/hands_tan.bmp");
		sprintf(actors_defs[elf_female].skin[SKIN_TAN].head_name,"./md2/head_humanftan.bmp");

		sprintf(actors_defs[elf_female].hair[HAIR_BLACK].hair_name,"./md2/hair_black.bmp");
		sprintf(actors_defs[elf_female].hair[HAIR_BLOND].hair_name,"./md2/hair_blond.bmp");
		sprintf(actors_defs[elf_female].hair[HAIR_BROWN].hair_name,"./md2/hair_brown.bmp");
		sprintf(actors_defs[elf_female].hair[HAIR_GRAY].hair_name,"./md2/hair_gray.bmp");
		sprintf(actors_defs[elf_female].hair[HAIR_RED].hair_name,"./md2/hair_red.bmp");
		sprintf(actors_defs[elf_female].hair[HAIR_WHITE].hair_name,"./md2/hair_white.bmp");

		sprintf(actors_defs[elf_female].boots[BOOTS_BLACK].boots_name,"./md2/boots1_black.bmp");
		sprintf(actors_defs[elf_female].boots[BOOTS_BROWN].boots_name,"./md2/boots1_brown.bmp");
		sprintf(actors_defs[elf_female].boots[BOOTS_DARKBROWN].boots_name,"./md2/boots1_darkbrown.bmp");
		sprintf(actors_defs[elf_female].boots[BOOTS_DULLBROWN].boots_name,"./md2/boots1_dullbrown.bmp");
		sprintf(actors_defs[elf_female].boots[BOOTS_LIGHTBROWN].boots_name,"./md2/boots1_lightbrown.bmp");
		sprintf(actors_defs[elf_female].boots[BOOTS_ORANGE].boots_name,"./md2/boots1_orange.bmp");

		sprintf(actors_defs[elf_female].legs[PANTS_BLACK].legs_name,"./md2/pants1_black.bmp");
		sprintf(actors_defs[elf_female].legs[PANTS_BLACK].model_name,"./md2/legs1_elff.md2");
		sprintf(actors_defs[elf_female].legs[PANTS_BLUE].legs_name,"./md2/pants1_blue.bmp");
		sprintf(actors_defs[elf_female].legs[PANTS_BLUE].model_name,"./md2/legs1_elff.md2");
		sprintf(actors_defs[elf_female].legs[PANTS_BROWN].legs_name,"./md2/pants1_brown.bmp");
		sprintf(actors_defs[elf_female].legs[PANTS_BROWN].model_name,"./md2/legs1_elff.md2");
		sprintf(actors_defs[elf_female].legs[PANTS_DARKBROWN].legs_name,"./md2/pants1_darkbrown.bmp");
		sprintf(actors_defs[elf_female].legs[PANTS_DARKBROWN].model_name,"./md2/legs1_elff.md2");
		sprintf(actors_defs[elf_female].legs[PANTS_GREY].legs_name,"./md2/pants1_gray.bmp");
		sprintf(actors_defs[elf_female].legs[PANTS_GREY].model_name,"./md2/legs1_elff.md2");
		sprintf(actors_defs[elf_female].legs[PANTS_GREEN].legs_name,"./md2/pants1_green.bmp");
		sprintf(actors_defs[elf_female].legs[PANTS_GREEN].model_name,"./md2/legs1_elff.md2");
		sprintf(actors_defs[elf_female].legs[PANTS_LIGHTBROWN].legs_name,"./md2/pants1_lightbrown.bmp");
		sprintf(actors_defs[elf_female].legs[PANTS_LIGHTBROWN].model_name,"./md2/legs1_elff.md2");
		sprintf(actors_defs[elf_female].legs[PANTS_RED].legs_name,"./md2/pants1_red.bmp");
		sprintf(actors_defs[elf_female].legs[PANTS_RED].model_name,"./md2/legs1_elff.md2");
		sprintf(actors_defs[elf_female].legs[PANTS_WHITE].legs_name,"./md2/pants1_white.bmp");
		sprintf(actors_defs[elf_female].legs[PANTS_WHITE].model_name,"./md2/legs1_elff.md2");

		sprintf(actors_defs[elf_female].cape[CAPE_BLACK].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[elf_female].cape[CAPE_BLACK].skin_name,"./md2/cape1_black.bmp");
		sprintf(actors_defs[elf_female].cape[CAPE_BLUE].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[elf_female].cape[CAPE_BLUE].skin_name,"./md2/cape1_blue.bmp");
		sprintf(actors_defs[elf_female].cape[CAPE_BLUEGRAY].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[elf_female].cape[CAPE_BLUEGRAY].skin_name,"./md2/cape1_bluegray.bmp");
		sprintf(actors_defs[elf_female].cape[CAPE_BROWN].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[elf_female].cape[CAPE_BROWN].skin_name,"./md2/cape1_brown.bmp");
		sprintf(actors_defs[elf_female].cape[CAPE_BROWNGRAY].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[elf_female].cape[CAPE_BROWNGRAY].skin_name,"./md2/cape1_browngray.bmp");
		sprintf(actors_defs[elf_female].cape[CAPE_GRAY].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[elf_female].cape[CAPE_GRAY].skin_name,"./md2/cape1_gray.bmp");
		sprintf(actors_defs[elf_female].cape[CAPE_GREEN].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[elf_female].cape[CAPE_GREEN].skin_name,"./md2/cape1_green.bmp");
		sprintf(actors_defs[elf_female].cape[CAPE_GREENGRAY].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[elf_female].cape[CAPE_GREENGRAY].skin_name,"./md2/cape1_greengray.bmp");
		sprintf(actors_defs[elf_female].cape[CAPE_PURPLE].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[elf_female].cape[CAPE_PURPLE].skin_name,"./md2/cape1_purple.bmp");
		sprintf(actors_defs[elf_female].cape[CAPE_WHITE].model_name,"./md2/cape1_tallf.md2");
		sprintf(actors_defs[elf_female].cape[CAPE_WHITE].skin_name,"./md2/cape1_white.bmp");

		sprintf(actors_defs[elf_female].head[HEAD_1].model_name,"./md2/head1_elff.md2");
		sprintf(actors_defs[elf_female].head[HEAD_2].model_name,"./md2/head2_elff.md2");
		sprintf(actors_defs[elf_female].head[HEAD_3].model_name,"./md2/head3_elff.md2");
		sprintf(actors_defs[elf_female].head[HEAD_4].model_name,"./md2/head4_elff.md2");
		sprintf(actors_defs[elf_female].head[HEAD_5].model_name,"./md2/head1_elff.md2");

		sprintf(actors_defs[elf_female].shield[SHIELD_WOOD].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[elf_female].shield[SHIELD_WOOD].skin_name,"./md2/shield1_wood1.bmp");
		sprintf(actors_defs[elf_female].shield[SHIELD_WOOD_ENHANCED].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[elf_female].shield[SHIELD_WOOD_ENHANCED].skin_name,"./md2/shield1_wood2.bmp");
		sprintf(actors_defs[elf_female].shield[SHIELD_IRON].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[elf_female].shield[SHIELD_IRON].skin_name,"./md2/shield1_iron.bmp");
		sprintf(actors_defs[elf_female].shield[SHIELD_STEEL].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[elf_female].shield[SHIELD_STEEL].skin_name,"./md2/shield1_steel.bmp");


		sprintf(actors_defs[elf_female].weapon[WEAPON_NONE].model_name,"");
		sprintf(actors_defs[elf_female].weapon[WEAPON_NONE].skin_name,"");
		sprintf(actors_defs[elf_female].weapon[WEAPON_NONE].attack_up1,"punchone01");
		sprintf(actors_defs[elf_female].weapon[WEAPON_NONE].attack_up2,"punchtwo01");
		sprintf(actors_defs[elf_female].weapon[WEAPON_NONE].attack_down1,"kickone01");
		sprintf(actors_defs[elf_female].weapon[WEAPON_NONE].attack_down2,"kicktwo01");
		sprintf(actors_defs[elf_female].weapon[SWORD_1].model_name,"./md2/sword1_tall.md2");
		sprintf(actors_defs[elf_female].weapon[SWORD_1].skin_name,"./md2/sword1.bmp");
		sprintf(actors_defs[elf_female].weapon[SWORD_1].attack_up1,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_1].attack_up2,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_1].attack_down1,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_1].attack_down2,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_2].model_name,"./md2/sword2_tall.md2");
		sprintf(actors_defs[elf_female].weapon[SWORD_2].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[elf_female].weapon[SWORD_2].attack_up1,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_2].attack_up2,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_2].attack_down1,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_2].attack_down2,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_3].model_name,"./md2/sword3_tall.md2");
		sprintf(actors_defs[elf_female].weapon[SWORD_3].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[elf_female].weapon[SWORD_3].attack_up1,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_3].attack_up2,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_3].attack_down1,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_3].attack_down2,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_4].model_name,"./md2/sword4_tall.md2");
		sprintf(actors_defs[elf_female].weapon[SWORD_4].skin_name,"./md2/sword4.bmp");
		sprintf(actors_defs[elf_female].weapon[SWORD_4].attack_up1,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_4].attack_up2,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_4].attack_down1,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_4].attack_down2,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_5].model_name,"./md2/sword5_tall.md2");
		sprintf(actors_defs[elf_female].weapon[SWORD_5].skin_name,"./md2/sword5.bmp");
		sprintf(actors_defs[elf_female].weapon[SWORD_5].attack_up1,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_5].attack_up2,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_5].attack_down1,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_5].attack_down2,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_6].model_name,"./md2/sword6_tall.md2");
		sprintf(actors_defs[elf_female].weapon[SWORD_6].skin_name,"./md2/sword6.bmp");
		sprintf(actors_defs[elf_female].weapon[SWORD_6].attack_up1,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_6].attack_up2,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_6].attack_down1,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_6].attack_down2,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_7].model_name,"./md2/sword7_tall.md2");
		sprintf(actors_defs[elf_female].weapon[SWORD_7].skin_name,"./md2/sword7.bmp");
		sprintf(actors_defs[elf_female].weapon[SWORD_7].attack_up1,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_7].attack_up2,"slashone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_7].attack_down1,"hackone01");
		sprintf(actors_defs[elf_female].weapon[SWORD_7].attack_down2,"hackone01");
		sprintf(actors_defs[elf_female].weapon[STAFF_1].model_name,"./md2/staff1_tall.md2");
		sprintf(actors_defs[elf_female].weapon[STAFF_1].skin_name,"./md2/staff1_brown.bmp");
		sprintf(actors_defs[elf_female].weapon[STAFF_1].attack_up1,"slashtwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_1].attack_up2,"slashtwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_1].attack_down1,"hacktwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_1].attack_down2,"hacktwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_2].model_name,"./md2/staff1_tall.md2");
		sprintf(actors_defs[elf_female].weapon[STAFF_2].skin_name,"./md2/staff1_green.bmp");
		sprintf(actors_defs[elf_female].weapon[STAFF_2].attack_up1,"slashtwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_2].attack_up2,"slashtwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_2].attack_down1,"hacktwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_2].attack_down2,"hacktwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_3].model_name,"./md2/staff2_tall.md2");
		sprintf(actors_defs[elf_female].weapon[STAFF_3].skin_name,"./md2/staff3.bmp");
		sprintf(actors_defs[elf_female].weapon[STAFF_3].attack_up1,"slashtwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_3].attack_up2,"slashtwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_3].attack_down1,"hacktwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_3].attack_down2,"hacktwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_4].model_name,"./md2/staff3_tall.md2");
		sprintf(actors_defs[elf_female].weapon[STAFF_4].skin_name,"./md2/staff4.bmp");
		sprintf(actors_defs[elf_female].weapon[STAFF_4].attack_up1,"slashtwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_4].attack_up2,"slashtwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_4].attack_down1,"hacktwo01");
		sprintf(actors_defs[elf_female].weapon[STAFF_4].attack_down2,"hacktwo01");
		sprintf(actors_defs[elf_female].weapon[HAMMER_1].model_name,"./md2/warhammer1_tall.md2");
		sprintf(actors_defs[elf_female].weapon[HAMMER_1].skin_name,"./md2/warhammer1.bmp");
		sprintf(actors_defs[elf_female].weapon[HAMMER_1].attack_up1,"slashone01");
		sprintf(actors_defs[elf_female].weapon[HAMMER_1].attack_up2,"slashone01");
		sprintf(actors_defs[elf_female].weapon[HAMMER_1].attack_down1,"hackone01");
		sprintf(actors_defs[elf_female].weapon[HAMMER_1].attack_down2,"hackone01");
		sprintf(actors_defs[elf_female].weapon[HAMMER_2].model_name,"./md2/warhammer2_tall.md2");
		sprintf(actors_defs[elf_female].weapon[HAMMER_2].skin_name,"./md2/warhammer2.bmp");
		sprintf(actors_defs[elf_female].weapon[HAMMER_2].attack_up1,"slashone01");
		sprintf(actors_defs[elf_female].weapon[HAMMER_2].attack_up2,"slashone01");
		sprintf(actors_defs[elf_female].weapon[HAMMER_2].attack_down1,"hackone01");
		sprintf(actors_defs[elf_female].weapon[HAMMER_2].attack_down2,"hackone01");
		sprintf(actors_defs[elf_female].weapon[PICKAX].model_name,"./md2/pickaxe1_tall.md2");
		sprintf(actors_defs[elf_female].weapon[PICKAX].skin_name,"./md2/pickaxe1.bmp");
		sprintf(actors_defs[elf_female].weapon[PICKAX].attack_up1,"slashone01");
		sprintf(actors_defs[elf_female].weapon[PICKAX].attack_up2,"slashone01");
		sprintf(actors_defs[elf_female].weapon[PICKAX].attack_down1,"hackone01");
		sprintf(actors_defs[elf_female].weapon[PICKAX].attack_down2,"hackone01");

		sprintf(actors_defs[elf_female].helmet[HELMET_IRON].model_name,"./md2/helmet1_tallf.md2");
		sprintf(actors_defs[elf_female].helmet[HELMET_IRON].skin_name,"./md2/helmet1.bmp");

		actors_defs[elf_female].walk_speed=default_walk_speed;
		actors_defs[elf_female].run_speed=2.0/18;
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
		actors_defs[elf_male].ghost=0;
		sprintf(actors_defs[elf_male].skin_name,"./md2/elf_male1.bmp");
		sprintf(actors_defs[elf_male].file_name,"./md2/elf_male.md2");
		sprintf(actors_defs[elf_male].walk_frame,"walk01");
		sprintf(actors_defs[elf_male].run_frame,"run01");
		sprintf(actors_defs[elf_male].die1_frame,"dief01");
		sprintf(actors_defs[elf_male].die2_frame,"dieb11");
		sprintf(actors_defs[elf_male].pain1_frame,"pain01");
		sprintf(actors_defs[elf_male].pick_frame,"pickup01");
		sprintf(actors_defs[elf_male].drop_frame,"drop01");
		sprintf(actors_defs[elf_male].idle_frame,"idle01");
		sprintf(actors_defs[elf_male].idle_sit_frame,"sitidle01");
		sprintf(actors_defs[elf_male].harvest_frame,"harvest01");
		sprintf(actors_defs[elf_male].attack_cast_frame,"cast01");
		sprintf(actors_defs[elf_male].sit_down_frame,"intosit01");
		sprintf(actors_defs[elf_male].stand_up_frame,"outsit01");
		sprintf(actors_defs[elf_male].in_combat_frame,"intofight01");
		sprintf(actors_defs[elf_male].out_combat_frame,"outfight01");
		sprintf(actors_defs[elf_male].combat_idle_frame,"fightidle01");

		sprintf(actors_defs[elf_male].shirt[SHIRT_BLACK].arms_name,"./md2/arms1_black.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_BLACK].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_BLACK].torso_name,"./md2/torso1_black.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_BLUE].arms_name,"./md2/arms1_blue.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_BLUE].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_BLUE].torso_name,"./md2/torso1_blue.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_BROWN].arms_name,"./md2/arms1_brown.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_BROWN].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_BROWN].torso_name,"./md2/torso1_brown.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_GREY].arms_name,"./md2/arms1_gray.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_GREY].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_GREY].torso_name,"./md2/torso1_gray.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_GREEN].arms_name,"./md2/arms1_green.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_GREEN].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_GREEN].torso_name,"./md2/torso1_green.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_LIGHTBROWN].arms_name,"./md2/arms1_lightbrown.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_LIGHTBROWN].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_LIGHTBROWN].torso_name,"./md2/torso1_lightbrown.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_ORANGE].arms_name,"./md2/arms1_orange.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_ORANGE].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_ORANGE].torso_name,"./md2/torso1_orange.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_PINK].arms_name,"./md2/arms1_pink.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_PINK].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_PINK].torso_name,"./md2/torso1_pink.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_PURPLE].arms_name,"./md2/arms1_purple.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_PURPLE].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_PURPLE].torso_name,"./md2/torso1_purple.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_RED].arms_name,"./md2/arms1_red.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_RED].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_RED].torso_name,"./md2/torso1_red.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_WHITE].arms_name,"./md2/arms1_white.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_WHITE].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_WHITE].torso_name,"./md2/torso1_white.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_YELLOW].arms_name,"./md2/arms1_yellow.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_YELLOW].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_YELLOW].torso_name,"./md2/torso1_yellow.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_LEATHER_ARMOR].arms_name,"./md2/arms2.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_LEATHER_ARMOR].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_LEATHER_ARMOR].torso_name,"./md2/torso2.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_CHAIN_ARMOR].arms_name,"./md2/arms3.bmp");
		sprintf(actors_defs[elf_male].shirt[SHIRT_CHAIN_ARMOR].model_name,"./md2/torso1_elfm.md2");
		sprintf(actors_defs[elf_male].shirt[SHIRT_CHAIN_ARMOR].torso_name,"./md2/torso3.bmp");

		sprintf(actors_defs[elf_male].skin[SKIN_BROWN].hands_name,"./md2/hands_brown.bmp");
		sprintf(actors_defs[elf_male].skin[SKIN_BROWN].head_name,"./md2/head_elfmbrown.bmp");
		sprintf(actors_defs[elf_male].skin[SKIN_NORMAL].hands_name,"./md2/hands_normal.bmp");
		sprintf(actors_defs[elf_male].skin[SKIN_NORMAL].head_name,"./md2/head_elfmnormal.bmp");
		sprintf(actors_defs[elf_male].skin[SKIN_PALE].hands_name,"./md2/hands_pale.bmp");
		sprintf(actors_defs[elf_male].skin[SKIN_PALE].head_name,"./md2/head_elfmpale.bmp");
		sprintf(actors_defs[elf_male].skin[SKIN_TAN].hands_name,"./md2/hands_tan.bmp");
		sprintf(actors_defs[elf_male].skin[SKIN_TAN].head_name,"./md2/head_elfmtan.bmp");

		sprintf(actors_defs[elf_male].hair[HAIR_BLACK].hair_name,"./md2/hair_black.bmp");
		sprintf(actors_defs[elf_male].hair[HAIR_BLOND].hair_name,"./md2/hair_blond.bmp");
		sprintf(actors_defs[elf_male].hair[HAIR_BROWN].hair_name,"./md2/hair_brown.bmp");
		sprintf(actors_defs[elf_male].hair[HAIR_GRAY].hair_name,"./md2/hair_gray.bmp");
		sprintf(actors_defs[elf_male].hair[HAIR_RED].hair_name,"./md2/hair_red.bmp");
		sprintf(actors_defs[elf_male].hair[HAIR_WHITE].hair_name,"./md2/hair_white.bmp");

		sprintf(actors_defs[elf_male].boots[BOOTS_BLACK].boots_name,"./md2/boots1_black.bmp");
		sprintf(actors_defs[elf_male].boots[BOOTS_BROWN].boots_name,"./md2/boots1_brown.bmp");
		sprintf(actors_defs[elf_male].boots[BOOTS_DARKBROWN].boots_name,"./md2/boots1_darkbrown.bmp");
		sprintf(actors_defs[elf_male].boots[BOOTS_DULLBROWN].boots_name,"./md2/boots1_dullbrown.bmp");
		sprintf(actors_defs[elf_male].boots[BOOTS_LIGHTBROWN].boots_name,"./md2/boots1_lightbrown.bmp");
		sprintf(actors_defs[elf_male].boots[BOOTS_ORANGE].boots_name,"./md2/boots1_orange.bmp");

		sprintf(actors_defs[elf_male].legs[PANTS_BLACK].legs_name,"./md2/pants1_black.bmp");
		sprintf(actors_defs[elf_male].legs[PANTS_BLACK].model_name,"./md2/legs1_elfm.md2");
		sprintf(actors_defs[elf_male].legs[PANTS_BLUE].legs_name,"./md2/pants1_blue.bmp");
		sprintf(actors_defs[elf_male].legs[PANTS_BLUE].model_name,"./md2/legs1_elfm.md2");
		sprintf(actors_defs[elf_male].legs[PANTS_BROWN].legs_name,"./md2/pants1_brown.bmp");
		sprintf(actors_defs[elf_male].legs[PANTS_BROWN].model_name,"./md2/legs1_elfm.md2");
		sprintf(actors_defs[elf_male].legs[PANTS_DARKBROWN].legs_name,"./md2/pants1_darkbrown.bmp");
		sprintf(actors_defs[elf_male].legs[PANTS_DARKBROWN].model_name,"./md2/legs1_elfm.md2");
		sprintf(actors_defs[elf_male].legs[PANTS_GREY].legs_name,"./md2/pants1_gray.bmp");
		sprintf(actors_defs[elf_male].legs[PANTS_GREY].model_name,"./md2/legs1_elfm.md2");
		sprintf(actors_defs[elf_male].legs[PANTS_GREEN].legs_name,"./md2/pants1_green.bmp");
		sprintf(actors_defs[elf_male].legs[PANTS_GREEN].model_name,"./md2/legs1_elfm.md2");
		sprintf(actors_defs[elf_male].legs[PANTS_LIGHTBROWN].legs_name,"./md2/pants1_lightbrown.bmp");
		sprintf(actors_defs[elf_male].legs[PANTS_LIGHTBROWN].model_name,"./md2/legs1_elfm.md2");
		sprintf(actors_defs[elf_male].legs[PANTS_RED].legs_name,"./md2/pants1_red.bmp");
		sprintf(actors_defs[elf_male].legs[PANTS_RED].model_name,"./md2/legs1_elfm.md2");
		sprintf(actors_defs[elf_male].legs[PANTS_WHITE].legs_name,"./md2/pants1_white.bmp");
		sprintf(actors_defs[elf_male].legs[PANTS_WHITE].model_name,"./md2/legs1_elfm.md2");

		sprintf(actors_defs[elf_male].cape[CAPE_BLACK].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[elf_male].cape[CAPE_BLACK].skin_name,"./md2/cape1_black.bmp");
		sprintf(actors_defs[elf_male].cape[CAPE_BLUE].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[elf_male].cape[CAPE_BLUE].skin_name,"./md2/cape1_blue.bmp");
		sprintf(actors_defs[elf_male].cape[CAPE_BLUEGRAY].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[elf_male].cape[CAPE_BLUEGRAY].skin_name,"./md2/cape1_bluegray.bmp");
		sprintf(actors_defs[elf_male].cape[CAPE_BROWN].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[elf_male].cape[CAPE_BROWN].skin_name,"./md2/cape1_brown.bmp");
		sprintf(actors_defs[elf_male].cape[CAPE_BROWNGRAY].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[elf_male].cape[CAPE_BROWNGRAY].skin_name,"./md2/cape1_browngray.bmp");
		sprintf(actors_defs[elf_male].cape[CAPE_GRAY].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[elf_male].cape[CAPE_GRAY].skin_name,"./md2/cape1_gray.bmp");
		sprintf(actors_defs[elf_male].cape[CAPE_GREEN].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[elf_male].cape[CAPE_GREEN].skin_name,"./md2/cape1_green.bmp");
		sprintf(actors_defs[elf_male].cape[CAPE_GREENGRAY].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[elf_male].cape[CAPE_GREENGRAY].skin_name,"./md2/cape1_greengray.bmp");
		sprintf(actors_defs[elf_male].cape[CAPE_PURPLE].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[elf_male].cape[CAPE_PURPLE].skin_name,"./md2/cape1_purple.bmp");
		sprintf(actors_defs[elf_male].cape[CAPE_WHITE].model_name,"./md2/cape1_tallm.md2");
		sprintf(actors_defs[elf_male].cape[CAPE_WHITE].skin_name,"./md2/cape1_white.bmp");

		sprintf(actors_defs[elf_male].head[HEAD_1].model_name,"./md2/head1_elfm.md2");
		sprintf(actors_defs[elf_male].head[HEAD_2].model_name,"./md2/head2_elfm.md2");
		sprintf(actors_defs[elf_male].head[HEAD_3].model_name,"./md2/head3_elfm.md2");
		sprintf(actors_defs[elf_male].head[HEAD_4].model_name,"./md2/head4_elfm.md2");
		sprintf(actors_defs[elf_male].head[HEAD_5].model_name,"./md2/head1_elfm.md2");

		sprintf(actors_defs[elf_male].shield[SHIELD_WOOD].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[elf_male].shield[SHIELD_WOOD].skin_name,"./md2/shield1_wood1.bmp");
		sprintf(actors_defs[elf_male].shield[SHIELD_WOOD_ENHANCED].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[elf_male].shield[SHIELD_WOOD_ENHANCED].skin_name,"./md2/shield1_wood2.bmp");
		sprintf(actors_defs[elf_male].shield[SHIELD_IRON].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[elf_male].shield[SHIELD_IRON].skin_name,"./md2/shield1_iron.bmp");
		sprintf(actors_defs[elf_male].shield[SHIELD_STEEL].model_name,"./md2/shield1_tall.md2");
		sprintf(actors_defs[elf_male].shield[SHIELD_STEEL].skin_name,"./md2/shield1_steel.bmp");


		sprintf(actors_defs[elf_male].weapon[WEAPON_NONE].model_name,"");
		sprintf(actors_defs[elf_male].weapon[WEAPON_NONE].skin_name,"");
		sprintf(actors_defs[elf_male].weapon[WEAPON_NONE].attack_up1,"punchone01");
		sprintf(actors_defs[elf_male].weapon[WEAPON_NONE].attack_up2,"punchtwo01");
		sprintf(actors_defs[elf_male].weapon[WEAPON_NONE].attack_down1,"kickone01");
		sprintf(actors_defs[elf_male].weapon[WEAPON_NONE].attack_down2,"kicktwo01");
		sprintf(actors_defs[elf_male].weapon[SWORD_1].model_name,"./md2/sword1_tall.md2");
		sprintf(actors_defs[elf_male].weapon[SWORD_1].skin_name,"./md2/sword1.bmp");
		sprintf(actors_defs[elf_male].weapon[SWORD_1].attack_up1,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_1].attack_up2,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_1].attack_down1,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_1].attack_down2,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_2].model_name,"./md2/sword2_tall.md2");
		sprintf(actors_defs[elf_male].weapon[SWORD_2].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[elf_male].weapon[SWORD_2].attack_up1,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_2].attack_up2,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_2].attack_down1,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_2].attack_down2,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_3].model_name,"./md2/sword3_tall.md2");
		sprintf(actors_defs[elf_male].weapon[SWORD_3].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[elf_male].weapon[SWORD_3].attack_up1,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_3].attack_up2,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_3].attack_down1,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_3].attack_down2,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_4].model_name,"./md2/sword4_tall.md2");
		sprintf(actors_defs[elf_male].weapon[SWORD_4].skin_name,"./md2/sword4.bmp");
		sprintf(actors_defs[elf_male].weapon[SWORD_4].attack_up1,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_4].attack_up2,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_4].attack_down1,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_4].attack_down2,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_5].model_name,"./md2/sword5_tall.md2");
		sprintf(actors_defs[elf_male].weapon[SWORD_5].skin_name,"./md2/sword5.bmp");
		sprintf(actors_defs[elf_male].weapon[SWORD_5].attack_up1,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_5].attack_up2,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_5].attack_down1,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_5].attack_down2,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_6].model_name,"./md2/sword6_tall.md2");
		sprintf(actors_defs[elf_male].weapon[SWORD_6].skin_name,"./md2/sword6.bmp");
		sprintf(actors_defs[elf_male].weapon[SWORD_6].attack_up1,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_6].attack_up2,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_6].attack_down1,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_6].attack_down2,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_7].model_name,"./md2/sword7_tall.md2");
		sprintf(actors_defs[elf_male].weapon[SWORD_7].skin_name,"./md2/sword7.bmp");
		sprintf(actors_defs[elf_male].weapon[SWORD_7].attack_up1,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_7].attack_up2,"slashone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_7].attack_down1,"hackone01");
		sprintf(actors_defs[elf_male].weapon[SWORD_7].attack_down2,"hackone01");
		sprintf(actors_defs[elf_male].weapon[STAFF_1].model_name,"./md2/staff1_tall.md2");
		sprintf(actors_defs[elf_male].weapon[STAFF_1].skin_name,"./md2/staff1_brown.bmp");
		sprintf(actors_defs[elf_male].weapon[STAFF_1].attack_up1,"slashtwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_1].attack_up2,"slashtwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_1].attack_down1,"hacktwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_1].attack_down2,"hacktwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_2].model_name,"./md2/staff1_tall.md2");
		sprintf(actors_defs[elf_male].weapon[STAFF_2].skin_name,"./md2/staff1_green.bmp");
		sprintf(actors_defs[elf_male].weapon[STAFF_2].attack_up1,"slashtwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_2].attack_up2,"slashtwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_2].attack_down1,"hacktwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_2].attack_down2,"hacktwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_3].model_name,"./md2/staff2_tall.md2");
		sprintf(actors_defs[elf_male].weapon[STAFF_3].skin_name,"./md2/staff3.bmp");
		sprintf(actors_defs[elf_male].weapon[STAFF_3].attack_up1,"slashtwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_3].attack_up2,"slashtwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_3].attack_down1,"hacktwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_3].attack_down2,"hacktwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_4].model_name,"./md2/staff3_tall.md2");
		sprintf(actors_defs[elf_male].weapon[STAFF_4].skin_name,"./md2/staff4.bmp");
		sprintf(actors_defs[elf_male].weapon[STAFF_4].attack_up1,"slashtwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_4].attack_up2,"slashtwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_4].attack_down1,"hacktwo01");
		sprintf(actors_defs[elf_male].weapon[STAFF_4].attack_down2,"hacktwo01");
		sprintf(actors_defs[elf_male].weapon[HAMMER_1].model_name,"./md2/warhammer1_tall.md2");
		sprintf(actors_defs[elf_male].weapon[HAMMER_1].skin_name,"./md2/warhammer1.bmp");
		sprintf(actors_defs[elf_male].weapon[HAMMER_1].attack_up1,"slashone01");
		sprintf(actors_defs[elf_male].weapon[HAMMER_1].attack_up2,"slashone01");
		sprintf(actors_defs[elf_male].weapon[HAMMER_1].attack_down1,"hackone01");
		sprintf(actors_defs[elf_male].weapon[HAMMER_1].attack_down2,"hackone01");
		sprintf(actors_defs[elf_male].weapon[HAMMER_2].model_name,"./md2/warhammer2_tall.md2");
		sprintf(actors_defs[elf_male].weapon[HAMMER_2].skin_name,"./md2/warhammer2.bmp");
		sprintf(actors_defs[elf_male].weapon[HAMMER_2].attack_up1,"slashone01");
		sprintf(actors_defs[elf_male].weapon[HAMMER_2].attack_up2,"slashone01");
		sprintf(actors_defs[elf_male].weapon[HAMMER_2].attack_down1,"hackone01");
		sprintf(actors_defs[elf_male].weapon[HAMMER_2].attack_down2,"hackone01");
		sprintf(actors_defs[elf_male].weapon[PICKAX].model_name,"./md2/pickaxe1_tall.md2");
		sprintf(actors_defs[elf_male].weapon[PICKAX].skin_name,"./md2/pickaxe1.bmp");
		sprintf(actors_defs[elf_male].weapon[PICKAX].attack_up1,"slashone01");
		sprintf(actors_defs[elf_male].weapon[PICKAX].attack_up2,"slashone01");
		sprintf(actors_defs[elf_male].weapon[PICKAX].attack_down1,"hackone01");
		sprintf(actors_defs[elf_male].weapon[PICKAX].attack_down2,"hackone01");

		sprintf(actors_defs[elf_male].helmet[HELMET_IRON].model_name,"./md2/helmet1_tall.md2");
		sprintf(actors_defs[elf_male].helmet[HELMET_IRON].skin_name,"./md2/helmet1.bmp");

		actors_defs[elf_male].walk_speed=default_walk_speed;
		actors_defs[elf_male].run_speed=2.0/18;
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
		actors_defs[dwarf_female].ghost=0;
		sprintf(actors_defs[dwarf_female].skin_name,"./md2/dwarf_female1.bmp");
		sprintf(actors_defs[dwarf_female].file_name,"./md2/dwarf_female.md2");
		sprintf(actors_defs[dwarf_female].walk_frame,"walk01");
		sprintf(actors_defs[dwarf_female].run_frame,"run01");
		sprintf(actors_defs[dwarf_female].die1_frame,"dief01");
		sprintf(actors_defs[dwarf_female].die2_frame,"dieb11");
		sprintf(actors_defs[dwarf_female].pain1_frame,"pain01");
		sprintf(actors_defs[dwarf_female].pick_frame,"pickup01");
		sprintf(actors_defs[dwarf_female].drop_frame,"drop01");
		sprintf(actors_defs[dwarf_female].idle_frame,"idle01");
		sprintf(actors_defs[dwarf_female].idle_sit_frame,"sitidle01");
		sprintf(actors_defs[dwarf_female].harvest_frame,"harvest01");
		sprintf(actors_defs[dwarf_female].attack_cast_frame,"cast01");
		sprintf(actors_defs[dwarf_female].sit_down_frame,"intosit01");
		sprintf(actors_defs[dwarf_female].stand_up_frame,"outsit01");
		sprintf(actors_defs[dwarf_female].in_combat_frame,"intofight01");
		sprintf(actors_defs[dwarf_female].out_combat_frame,"outfight01");
		sprintf(actors_defs[dwarf_female].combat_idle_frame,"fightidle01");

		sprintf(actors_defs[dwarf_female].shirt[SHIRT_BLACK].arms_name,"./md2/arms1_black.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_BLACK].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_BLACK].torso_name,"./md2/torso1_black.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_BLUE].arms_name,"./md2/arms1_blue.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_BLUE].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_BLUE].torso_name,"./md2/torso1_blue.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_BROWN].arms_name,"./md2/arms1_brown.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_BROWN].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_BROWN].torso_name,"./md2/torso1_brown.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_GREY].arms_name,"./md2/arms1_gray.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_GREY].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_GREY].torso_name,"./md2/torso1_gray.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_GREEN].arms_name,"./md2/arms1_green.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_GREEN].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_GREEN].torso_name,"./md2/torso1_green.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_LIGHTBROWN].arms_name,"./md2/arms1_lightbrown.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_LIGHTBROWN].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_LIGHTBROWN].torso_name,"./md2/torso1_lightbrown.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_ORANGE].arms_name,"./md2/arms1_orange.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_ORANGE].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_ORANGE].torso_name,"./md2/torso1_orange.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_PINK].arms_name,"./md2/arms1_pink.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_PINK].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_PINK].torso_name,"./md2/torso1_pink.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_PURPLE].arms_name,"./md2/arms1_purple.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_PURPLE].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_PURPLE].torso_name,"./md2/torso1_purple.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_RED].arms_name,"./md2/arms1_red.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_RED].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_RED].torso_name,"./md2/torso1_red.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_WHITE].arms_name,"./md2/arms1_white.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_WHITE].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_WHITE].torso_name,"./md2/torso1_white.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_YELLOW].arms_name,"./md2/arms1_yellow.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_YELLOW].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_YELLOW].torso_name,"./md2/torso1_yellow.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_LEATHER_ARMOR].arms_name,"./md2/arms2.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_LEATHER_ARMOR].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_LEATHER_ARMOR].torso_name,"./md2/torso2.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_CHAIN_ARMOR].arms_name,"./md2/arms3.bmp");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_CHAIN_ARMOR].model_name,"./md2/torso1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].shirt[SHIRT_CHAIN_ARMOR].torso_name,"./md2/torso3.bmp");

		sprintf(actors_defs[dwarf_female].skin[SKIN_BROWN].hands_name,"./md2/hands_brown.bmp");
		sprintf(actors_defs[dwarf_female].skin[SKIN_BROWN].head_name,"./md2/head_dwarffbrown.bmp");
		sprintf(actors_defs[dwarf_female].skin[SKIN_NORMAL].hands_name,"./md2/hands_normal.bmp");
		sprintf(actors_defs[dwarf_female].skin[SKIN_NORMAL].head_name,"./md2/head_dwarffnormal.bmp");
		sprintf(actors_defs[dwarf_female].skin[SKIN_PALE].hands_name,"./md2/hands_pale.bmp");
		sprintf(actors_defs[dwarf_female].skin[SKIN_PALE].head_name,"./md2/head_dwarffpale.bmp");
		sprintf(actors_defs[dwarf_female].skin[SKIN_TAN].hands_name,"./md2/hands_tan.bmp");
		sprintf(actors_defs[dwarf_female].skin[SKIN_TAN].head_name,"./md2/head_dwarfftan.bmp");

		sprintf(actors_defs[dwarf_female].hair[HAIR_BLACK].hair_name,"./md2/hair_black.bmp");
		sprintf(actors_defs[dwarf_female].hair[HAIR_BLOND].hair_name,"./md2/hair_blond.bmp");
		sprintf(actors_defs[dwarf_female].hair[HAIR_BROWN].hair_name,"./md2/hair_brown.bmp");
		sprintf(actors_defs[dwarf_female].hair[HAIR_GRAY].hair_name,"./md2/hair_gray.bmp");
		sprintf(actors_defs[dwarf_female].hair[HAIR_RED].hair_name,"./md2/hair_red.bmp");
		sprintf(actors_defs[dwarf_female].hair[HAIR_WHITE].hair_name,"./md2/hair_white.bmp");

		sprintf(actors_defs[dwarf_female].boots[BOOTS_BLACK].boots_name,"./md2/boots1_black.bmp");
		sprintf(actors_defs[dwarf_female].boots[BOOTS_BROWN].boots_name,"./md2/boots1_brown.bmp");
		sprintf(actors_defs[dwarf_female].boots[BOOTS_DARKBROWN].boots_name,"./md2/boots1_darkbrown.bmp");
		sprintf(actors_defs[dwarf_female].boots[BOOTS_DULLBROWN].boots_name,"./md2/boots1_dullbrown.bmp");
		sprintf(actors_defs[dwarf_female].boots[BOOTS_LIGHTBROWN].boots_name,"./md2/boots1_lightbrown.bmp");
		sprintf(actors_defs[dwarf_female].boots[BOOTS_ORANGE].boots_name,"./md2/boots1_orange.bmp");

		sprintf(actors_defs[dwarf_female].legs[PANTS_BLACK].legs_name,"./md2/pants1_black.bmp");
		sprintf(actors_defs[dwarf_female].legs[PANTS_BLACK].model_name,"./md2/legs1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].legs[PANTS_BLUE].legs_name,"./md2/pants1_blue.bmp");
		sprintf(actors_defs[dwarf_female].legs[PANTS_BLUE].model_name,"./md2/legs1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].legs[PANTS_BROWN].legs_name,"./md2/pants1_brown.bmp");
		sprintf(actors_defs[dwarf_female].legs[PANTS_BROWN].model_name,"./md2/legs1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].legs[PANTS_DARKBROWN].legs_name,"./md2/pants1_darkbrown.bmp");
		sprintf(actors_defs[dwarf_female].legs[PANTS_DARKBROWN].model_name,"./md2/legs1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].legs[PANTS_GREY].legs_name,"./md2/pants1_gray.bmp");
		sprintf(actors_defs[dwarf_female].legs[PANTS_GREY].model_name,"./md2/legs1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].legs[PANTS_GREEN].legs_name,"./md2/pants1_green.bmp");
		sprintf(actors_defs[dwarf_female].legs[PANTS_GREEN].model_name,"./md2/legs1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].legs[PANTS_LIGHTBROWN].legs_name,"./md2/pants1_lightbrown.bmp");
		sprintf(actors_defs[dwarf_female].legs[PANTS_LIGHTBROWN].model_name,"./md2/legs1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].legs[PANTS_RED].legs_name,"./md2/pants1_red.bmp");
		sprintf(actors_defs[dwarf_female].legs[PANTS_RED].model_name,"./md2/legs1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].legs[PANTS_WHITE].legs_name,"./md2/pants1_white.bmp");
		sprintf(actors_defs[dwarf_female].legs[PANTS_WHITE].model_name,"./md2/legs1_dwarff.md2");

		sprintf(actors_defs[dwarf_female].cape[CAPE_BLACK].model_name,"./md2/cape1_shortf.md2");
		sprintf(actors_defs[dwarf_female].cape[CAPE_BLACK].skin_name,"./md2/cape1_black.bmp");
		sprintf(actors_defs[dwarf_female].cape[CAPE_BLUE].model_name,"./md2/cape1_shortf.md2");
		sprintf(actors_defs[dwarf_female].cape[CAPE_BLUE].skin_name,"./md2/cape1_blue.bmp");
		sprintf(actors_defs[dwarf_female].cape[CAPE_BLUEGRAY].model_name,"./md2/cape1_shortf.md2");
		sprintf(actors_defs[dwarf_female].cape[CAPE_BLUEGRAY].skin_name,"./md2/cape1_bluegray.bmp");
		sprintf(actors_defs[dwarf_female].cape[CAPE_BROWN].model_name,"./md2/cape1_shortf.md2");
		sprintf(actors_defs[dwarf_female].cape[CAPE_BROWN].skin_name,"./md2/cape1_brown.bmp");
		sprintf(actors_defs[dwarf_female].cape[CAPE_BROWNGRAY].model_name,"./md2/cape1_shortf.md2");
		sprintf(actors_defs[dwarf_female].cape[CAPE_BROWNGRAY].skin_name,"./md2/cape1_browngray.bmp");
		sprintf(actors_defs[dwarf_female].cape[CAPE_GRAY].model_name,"./md2/cape1_shortf.md2");
		sprintf(actors_defs[dwarf_female].cape[CAPE_GRAY].skin_name,"./md2/cape1_gray.bmp");
		sprintf(actors_defs[dwarf_female].cape[CAPE_GREEN].model_name,"./md2/cape1_shortf.md2");
		sprintf(actors_defs[dwarf_female].cape[CAPE_GREEN].skin_name,"./md2/cape1_green.bmp");
		sprintf(actors_defs[dwarf_female].cape[CAPE_GREENGRAY].model_name,"./md2/cape1_shortf.md2");
		sprintf(actors_defs[dwarf_female].cape[CAPE_GREENGRAY].skin_name,"./md2/cape1_greengray.bmp");
		sprintf(actors_defs[dwarf_female].cape[CAPE_PURPLE].model_name,"./md2/cape1_shortf.md2");
		sprintf(actors_defs[dwarf_female].cape[CAPE_PURPLE].skin_name,"./md2/cape1_purple.bmp");
		sprintf(actors_defs[dwarf_female].cape[CAPE_WHITE].model_name,"./md2/cape1_shortf.md2");
		sprintf(actors_defs[dwarf_female].cape[CAPE_WHITE].skin_name,"./md2/cape1_white.bmp");

		sprintf(actors_defs[dwarf_female].head[HEAD_1].model_name,"./md2/head1_dwarff.md2");
		sprintf(actors_defs[dwarf_female].head[HEAD_2].model_name,"./md2/head2_dwarff.md2");
		sprintf(actors_defs[dwarf_female].head[HEAD_3].model_name,"./md2/head3_dwarff.md2");
		sprintf(actors_defs[dwarf_female].head[HEAD_4].model_name,"./md2/head4_dwarff.md2");
		sprintf(actors_defs[dwarf_female].head[HEAD_5].model_name,"./md2/head1_dwarff.md2");

		sprintf(actors_defs[dwarf_female].shield[SHIELD_WOOD].model_name,"./md2/shield1_short.md2");
		sprintf(actors_defs[dwarf_female].shield[SHIELD_WOOD].skin_name,"./md2/shield1_wood1.bmp");
		sprintf(actors_defs[dwarf_female].shield[SHIELD_WOOD_ENHANCED].model_name,"./md2/shield1_short.md2");
		sprintf(actors_defs[dwarf_female].shield[SHIELD_WOOD_ENHANCED].skin_name,"./md2/shield1_wood2.bmp");
		sprintf(actors_defs[dwarf_female].shield[SHIELD_IRON].model_name,"./md2/shield1_short.md2");
		sprintf(actors_defs[dwarf_female].shield[SHIELD_IRON].skin_name,"./md2/shield1_iron.bmp");
		sprintf(actors_defs[dwarf_female].shield[SHIELD_STEEL].model_name,"./md2/shield1_short.md2");
		sprintf(actors_defs[dwarf_female].shield[SHIELD_STEEL].skin_name,"./md2/shield1_steel.bmp");


		sprintf(actors_defs[dwarf_female].weapon[WEAPON_NONE].model_name,"");
		sprintf(actors_defs[dwarf_female].weapon[WEAPON_NONE].skin_name,"");
		sprintf(actors_defs[dwarf_female].weapon[WEAPON_NONE].attack_up1,"punchone01");
		sprintf(actors_defs[dwarf_female].weapon[WEAPON_NONE].attack_up2,"punchtwo01");
		sprintf(actors_defs[dwarf_female].weapon[WEAPON_NONE].attack_down1,"kickone01");
		sprintf(actors_defs[dwarf_female].weapon[WEAPON_NONE].attack_down2,"kicktwo01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_1].model_name,"./md2/sword1_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_1].skin_name,"./md2/sword1.bmp");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_1].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_1].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_1].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_1].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_2].model_name,"./md2/sword2_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_2].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_2].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_2].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_2].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_2].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_3].model_name,"./md2/sword3_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_3].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_3].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_3].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_3].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_3].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_4].model_name,"./md2/sword4_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_4].skin_name,"./md2/sword4.bmp");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_4].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_4].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_4].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_4].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_5].model_name,"./md2/sword5_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_5].skin_name,"./md2/sword5.bmp");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_5].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_5].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_5].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_5].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_6].model_name,"./md2/sword6_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_6].skin_name,"./md2/sword6.bmp");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_6].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_6].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_6].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_6].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_7].model_name,"./md2/sword7_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_7].skin_name,"./md2/sword7.bmp");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_7].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_7].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_7].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[SWORD_7].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_1].model_name,"./md2/staff1_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_1].skin_name,"./md2/staff1_brown.bmp");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_1].attack_up1,"slashtwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_1].attack_up2,"slashtwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_1].attack_down1,"hacktwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_1].attack_down2,"hacktwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_2].model_name,"./md2/staff1_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_2].skin_name,"./md2/staff1_green.bmp");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_2].attack_up1,"slashtwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_2].attack_up2,"slashtwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_2].attack_down1,"hacktwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_2].attack_down2,"hacktwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_3].model_name,"./md2/staff2_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_3].skin_name,"./md2/staff3.bmp");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_3].attack_up1,"slashtwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_3].attack_up2,"slashtwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_3].attack_down1,"hacktwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_3].attack_down2,"hacktwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_4].model_name,"./md2/staff3_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_4].skin_name,"./md2/staff4.bmp");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_4].attack_up1,"slashtwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_4].attack_up2,"slashtwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_4].attack_down1,"hacktwo01");
		sprintf(actors_defs[dwarf_female].weapon[STAFF_4].attack_down2,"hacktwo01");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_1].model_name,"./md2/warhammer1_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_1].skin_name,"./md2/warhammer1.bmp");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_1].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_1].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_1].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_1].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_2].model_name,"./md2/warhammer2_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_2].skin_name,"./md2/warhammer2.bmp");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_2].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_2].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_2].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[HAMMER_2].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[PICKAX].model_name,"./md2/pickaxe1_short.md2");
		sprintf(actors_defs[dwarf_female].weapon[PICKAX].skin_name,"./md2/pickaxe1.bmp");
		sprintf(actors_defs[dwarf_female].weapon[PICKAX].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[PICKAX].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_female].weapon[PICKAX].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_female].weapon[PICKAX].attack_down2,"hackone01");

		sprintf(actors_defs[dwarf_female].helmet[HELMET_IRON].model_name,"./md2/helmet1_short.md2");
		sprintf(actors_defs[dwarf_female].helmet[HELMET_IRON].skin_name,"./md2/helmet1.bmp");

		actors_defs[dwarf_female].walk_speed=default_walk_speed;
		actors_defs[dwarf_female].run_speed=2.0/18;
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
		actors_defs[dwarf_male].ghost=0;
		sprintf(actors_defs[dwarf_male].skin_name,"./md2/dwarf_male1.bmp");
		sprintf(actors_defs[dwarf_male].file_name,"./md2/dwarf_male.md2");
		sprintf(actors_defs[dwarf_male].walk_frame,"walk01");
		sprintf(actors_defs[dwarf_male].run_frame,"run01");
		sprintf(actors_defs[dwarf_male].die1_frame,"dief01");
		sprintf(actors_defs[dwarf_male].die2_frame,"dieb11");
		sprintf(actors_defs[dwarf_male].pain1_frame,"pain01");
		sprintf(actors_defs[dwarf_male].pick_frame,"pickup01");
		sprintf(actors_defs[dwarf_male].drop_frame,"drop01");
		sprintf(actors_defs[dwarf_male].idle_frame,"idle01");
		sprintf(actors_defs[dwarf_male].idle_sit_frame,"sitidle01");
		sprintf(actors_defs[dwarf_male].harvest_frame,"harvest01");
		sprintf(actors_defs[dwarf_male].attack_cast_frame,"cast01");
		sprintf(actors_defs[dwarf_male].sit_down_frame,"intosit01");
		sprintf(actors_defs[dwarf_male].stand_up_frame,"outsit01");
		sprintf(actors_defs[dwarf_male].in_combat_frame,"intofight01");
		sprintf(actors_defs[dwarf_male].out_combat_frame,"outfight01");
		sprintf(actors_defs[dwarf_male].combat_idle_frame,"fightidle01");

		sprintf(actors_defs[dwarf_male].shirt[SHIRT_BLACK].arms_name,"./md2/arms1_black.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_BLACK].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_BLACK].torso_name,"./md2/torso1_black.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_BLUE].arms_name,"./md2/arms1_blue.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_BLUE].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_BLUE].torso_name,"./md2/torso1_blue.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_BROWN].arms_name,"./md2/arms1_brown.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_BROWN].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_BROWN].torso_name,"./md2/torso1_brown.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_GREY].arms_name,"./md2/arms1_gray.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_GREY].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_GREY].torso_name,"./md2/torso1_gray.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_GREEN].arms_name,"./md2/arms1_green.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_GREEN].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_GREEN].torso_name,"./md2/torso1_green.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_LIGHTBROWN].arms_name,"./md2/arms1_lightbrown.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_LIGHTBROWN].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_LIGHTBROWN].torso_name,"./md2/torso1_lightbrown.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_ORANGE].arms_name,"./md2/arms1_orange.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_ORANGE].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_ORANGE].torso_name,"./md2/torso1_orange.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_PINK].arms_name,"./md2/arms1_pink.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_PINK].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_PINK].torso_name,"./md2/torso1_pink.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_PURPLE].arms_name,"./md2/arms1_purple.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_PURPLE].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_PURPLE].torso_name,"./md2/torso1_purple.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_RED].arms_name,"./md2/arms1_red.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_RED].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_RED].torso_name,"./md2/torso1_red.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_WHITE].arms_name,"./md2/arms1_white.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_WHITE].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_WHITE].torso_name,"./md2/torso1_white.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_YELLOW].arms_name,"./md2/arms1_yellow.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_YELLOW].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_YELLOW].torso_name,"./md2/torso1_yellow.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_LEATHER_ARMOR].arms_name,"./md2/arms2.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_LEATHER_ARMOR].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_LEATHER_ARMOR].torso_name,"./md2/torso2.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_CHAIN_ARMOR].arms_name,"./md2/arms3.bmp");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_CHAIN_ARMOR].model_name,"./md2/torso1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].shirt[SHIRT_CHAIN_ARMOR].torso_name,"./md2/torso3.bmp");

		sprintf(actors_defs[dwarf_male].skin[SKIN_BROWN].hands_name,"./md2/hands_brown.bmp");
		sprintf(actors_defs[dwarf_male].skin[SKIN_BROWN].head_name,"./md2/head_dwarfmbrown.bmp");
		sprintf(actors_defs[dwarf_male].skin[SKIN_NORMAL].hands_name,"./md2/hands_normal.bmp");
		sprintf(actors_defs[dwarf_male].skin[SKIN_NORMAL].head_name,"./md2/head_dwarfmnormal.bmp");
		sprintf(actors_defs[dwarf_male].skin[SKIN_PALE].hands_name,"./md2/hands_pale.bmp");
		sprintf(actors_defs[dwarf_male].skin[SKIN_PALE].head_name,"./md2/head_dwarfmpale.bmp");
		sprintf(actors_defs[dwarf_male].skin[SKIN_TAN].hands_name,"./md2/hands_tan.bmp");
		sprintf(actors_defs[dwarf_male].skin[SKIN_TAN].head_name,"./md2/head_dwarfmtan.bmp");

		sprintf(actors_defs[dwarf_male].hair[HAIR_BLACK].hair_name,"./md2/hair_black.bmp");
		sprintf(actors_defs[dwarf_male].hair[HAIR_BLOND].hair_name,"./md2/hair_blond.bmp");
		sprintf(actors_defs[dwarf_male].hair[HAIR_BROWN].hair_name,"./md2/hair_brown.bmp");
		sprintf(actors_defs[dwarf_male].hair[HAIR_GRAY].hair_name,"./md2/hair_gray.bmp");
		sprintf(actors_defs[dwarf_male].hair[HAIR_RED].hair_name,"./md2/hair_red.bmp");
		sprintf(actors_defs[dwarf_male].hair[HAIR_WHITE].hair_name,"./md2/hair_white.bmp");

		sprintf(actors_defs[dwarf_male].boots[BOOTS_BLACK].boots_name,"./md2/boots1_black.bmp");
		sprintf(actors_defs[dwarf_male].boots[BOOTS_BROWN].boots_name,"./md2/boots1_brown.bmp");
		sprintf(actors_defs[dwarf_male].boots[BOOTS_DARKBROWN].boots_name,"./md2/boots1_darkbrown.bmp");
		sprintf(actors_defs[dwarf_male].boots[BOOTS_DULLBROWN].boots_name,"./md2/boots1_dullbrown.bmp");
		sprintf(actors_defs[dwarf_male].boots[BOOTS_LIGHTBROWN].boots_name,"./md2/boots1_lightbrown.bmp");
		sprintf(actors_defs[dwarf_male].boots[BOOTS_ORANGE].boots_name,"./md2/boots1_orange.bmp");

		sprintf(actors_defs[dwarf_male].legs[PANTS_BLACK].legs_name,"./md2/pants1_black.bmp");
		sprintf(actors_defs[dwarf_male].legs[PANTS_BLACK].model_name,"./md2/legs1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].legs[PANTS_BLUE].legs_name,"./md2/pants1_blue.bmp");
		sprintf(actors_defs[dwarf_male].legs[PANTS_BLUE].model_name,"./md2/legs1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].legs[PANTS_BROWN].legs_name,"./md2/pants1_brown.bmp");
		sprintf(actors_defs[dwarf_male].legs[PANTS_BROWN].model_name,"./md2/legs1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].legs[PANTS_DARKBROWN].legs_name,"./md2/pants1_darkbrown.bmp");
		sprintf(actors_defs[dwarf_male].legs[PANTS_DARKBROWN].model_name,"./md2/legs1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].legs[PANTS_GREY].legs_name,"./md2/pants1_gray.bmp");
		sprintf(actors_defs[dwarf_male].legs[PANTS_GREY].model_name,"./md2/legs1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].legs[PANTS_GREEN].legs_name,"./md2/pants1_green.bmp");
		sprintf(actors_defs[dwarf_male].legs[PANTS_GREEN].model_name,"./md2/legs1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].legs[PANTS_LIGHTBROWN].legs_name,"./md2/pants1_lightbrown.bmp");
		sprintf(actors_defs[dwarf_male].legs[PANTS_LIGHTBROWN].model_name,"./md2/legs1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].legs[PANTS_RED].legs_name,"./md2/pants1_red.bmp");
		sprintf(actors_defs[dwarf_male].legs[PANTS_RED].model_name,"./md2/legs1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].legs[PANTS_WHITE].legs_name,"./md2/pants1_white.bmp");
		sprintf(actors_defs[dwarf_male].legs[PANTS_WHITE].model_name,"./md2/legs1_dwarfm.md2");

		sprintf(actors_defs[dwarf_male].cape[CAPE_BLACK].model_name,"./md2/cape1_shortm.md2");
		sprintf(actors_defs[dwarf_male].cape[CAPE_BLACK].skin_name,"./md2/cape1_black.bmp");
		sprintf(actors_defs[dwarf_male].cape[CAPE_BLUE].model_name,"./md2/cape1_shortm.md2");
		sprintf(actors_defs[dwarf_male].cape[CAPE_BLUE].skin_name,"./md2/cape1_blue.bmp");
		sprintf(actors_defs[dwarf_male].cape[CAPE_BLUEGRAY].model_name,"./md2/cape1_shortm.md2");
		sprintf(actors_defs[dwarf_male].cape[CAPE_BLUEGRAY].skin_name,"./md2/cape1_bluegray.bmp");
		sprintf(actors_defs[dwarf_male].cape[CAPE_BROWN].model_name,"./md2/cape1_shortm.md2");
		sprintf(actors_defs[dwarf_male].cape[CAPE_BROWN].skin_name,"./md2/cape1_brown.bmp");
		sprintf(actors_defs[dwarf_male].cape[CAPE_BROWNGRAY].model_name,"./md2/cape1_shortm.md2");
		sprintf(actors_defs[dwarf_male].cape[CAPE_BROWNGRAY].skin_name,"./md2/cape1_browngray.bmp");
		sprintf(actors_defs[dwarf_male].cape[CAPE_GRAY].model_name,"./md2/cape1_shortm.md2");
		sprintf(actors_defs[dwarf_male].cape[CAPE_GRAY].skin_name,"./md2/cape1_gray.bmp");
		sprintf(actors_defs[dwarf_male].cape[CAPE_GREEN].model_name,"./md2/cape1_shortm.md2");
		sprintf(actors_defs[dwarf_male].cape[CAPE_GREEN].skin_name,"./md2/cape1_green.bmp");
		sprintf(actors_defs[dwarf_male].cape[CAPE_GREENGRAY].model_name,"./md2/cape1_shortm.md2");
		sprintf(actors_defs[dwarf_male].cape[CAPE_GREENGRAY].skin_name,"./md2/cape1_greengray.bmp");
		sprintf(actors_defs[dwarf_male].cape[CAPE_PURPLE].model_name,"./md2/cape1_shortm.md2");
		sprintf(actors_defs[dwarf_male].cape[CAPE_PURPLE].skin_name,"./md2/cape1_purple.bmp");
		sprintf(actors_defs[dwarf_male].cape[CAPE_WHITE].model_name,"./md2/cape1_shortm.md2");
		sprintf(actors_defs[dwarf_male].cape[CAPE_WHITE].skin_name,"./md2/cape1_white.bmp");

		sprintf(actors_defs[dwarf_male].head[HEAD_1].model_name,"./md2/head1_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].head[HEAD_2].model_name,"./md2/head2_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].head[HEAD_3].model_name,"./md2/head3_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].head[HEAD_4].model_name,"./md2/head4_dwarfm.md2");
		sprintf(actors_defs[dwarf_male].head[HEAD_5].model_name,"./md2/head1_dwarfm.md2");

		sprintf(actors_defs[dwarf_male].shield[SHIELD_WOOD].model_name,"./md2/shield1_short.md2");
		sprintf(actors_defs[dwarf_male].shield[SHIELD_WOOD].skin_name,"./md2/shield1_wood1.bmp");
		sprintf(actors_defs[dwarf_male].shield[SHIELD_WOOD_ENHANCED].model_name,"./md2/shield1_short.md2");
		sprintf(actors_defs[dwarf_male].shield[SHIELD_WOOD_ENHANCED].skin_name,"./md2/shield1_wood2.bmp");
		sprintf(actors_defs[dwarf_male].shield[SHIELD_IRON].model_name,"./md2/shield1_short.md2");
		sprintf(actors_defs[dwarf_male].shield[SHIELD_IRON].skin_name,"./md2/shield1_iron.bmp");
		sprintf(actors_defs[dwarf_male].shield[SHIELD_STEEL].model_name,"./md2/shield1_short.md2");
		sprintf(actors_defs[dwarf_male].shield[SHIELD_STEEL].skin_name,"./md2/shield1_steel.bmp");


		sprintf(actors_defs[dwarf_male].weapon[WEAPON_NONE].model_name,"");
		sprintf(actors_defs[dwarf_male].weapon[WEAPON_NONE].skin_name,"");
		sprintf(actors_defs[dwarf_male].weapon[WEAPON_NONE].attack_up1,"punchone01");
		sprintf(actors_defs[dwarf_male].weapon[WEAPON_NONE].attack_up2,"punchtwo01");
		sprintf(actors_defs[dwarf_male].weapon[WEAPON_NONE].attack_down1,"kickone01");
		sprintf(actors_defs[dwarf_male].weapon[WEAPON_NONE].attack_down2,"kicktwo01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_1].model_name,"./md2/sword1_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_1].skin_name,"./md2/sword1.bmp");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_1].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_1].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_1].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_1].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_2].model_name,"./md2/sword2_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_2].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_2].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_2].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_2].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_2].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_3].model_name,"./md2/sword3_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_3].skin_name,"./md2/sword3.bmp");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_3].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_3].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_3].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_3].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_4].model_name,"./md2/sword4_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_4].skin_name,"./md2/sword4.bmp");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_4].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_4].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_4].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_4].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_5].model_name,"./md2/sword5_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_5].skin_name,"./md2/sword5.bmp");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_5].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_5].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_5].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_5].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_6].model_name,"./md2/sword6_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_6].skin_name,"./md2/sword6.bmp");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_6].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_6].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_6].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_6].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_7].model_name,"./md2/sword7_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_7].skin_name,"./md2/sword7.bmp");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_7].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_7].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_7].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[SWORD_7].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_1].model_name,"./md2/staff1_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_1].skin_name,"./md2/staff1_brown.bmp");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_1].attack_up1,"slashtwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_1].attack_up2,"slashtwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_1].attack_down1,"hacktwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_1].attack_down2,"hacktwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_2].model_name,"./md2/staff1_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_2].skin_name,"./md2/staff1_green.bmp");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_2].attack_up1,"slashtwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_2].attack_up2,"slashtwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_2].attack_down1,"hacktwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_2].attack_down2,"hacktwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_3].model_name,"./md2/staff2_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_3].skin_name,"./md2/staff3.bmp");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_3].attack_up1,"slashtwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_3].attack_up2,"slashtwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_3].attack_down1,"hacktwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_3].attack_down2,"hacktwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_4].model_name,"./md2/staff3_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_4].skin_name,"./md2/staff4.bmp");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_4].attack_up1,"slashtwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_4].attack_up2,"slashtwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_4].attack_down1,"hacktwo01");
		sprintf(actors_defs[dwarf_male].weapon[STAFF_4].attack_down2,"hacktwo01");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_1].model_name,"./md2/warhammer1_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_1].skin_name,"./md2/warhammer1.bmp");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_1].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_1].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_1].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_1].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_2].model_name,"./md2/warhammer2_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_2].skin_name,"./md2/warhammer2.bmp");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_2].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_2].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_2].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[HAMMER_2].attack_down2,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[PICKAX].model_name,"./md2/pickaxe1_short.md2");
		sprintf(actors_defs[dwarf_male].weapon[PICKAX].skin_name,"./md2/pickaxe1.bmp");
		sprintf(actors_defs[dwarf_male].weapon[PICKAX].attack_up1,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[PICKAX].attack_up2,"slashone01");
		sprintf(actors_defs[dwarf_male].weapon[PICKAX].attack_down1,"hackone01");
		sprintf(actors_defs[dwarf_male].weapon[PICKAX].attack_down2,"hackone01");

		sprintf(actors_defs[dwarf_male].helmet[HELMET_IRON].model_name,"./md2/helmet1_short.md2");
		sprintf(actors_defs[dwarf_male].helmet[HELMET_IRON].skin_name,"./md2/helmet1.bmp");

		actors_defs[dwarf_male].walk_speed=default_walk_speed;
		actors_defs[dwarf_male].run_speed=2.0/18;



		//////////////////////////////////////////////////////////////////////////////////////////
		actors_defs[wraith].ghost=1;
		sprintf(actors_defs[wraith].skin_name,"./md2/wraith.bmp");
		sprintf(actors_defs[wraith].file_name,"./md2/wraith.md2");
		sprintf(actors_defs[wraith].walk_frame,"frame01");
		sprintf(actors_defs[wraith].idle_frame,"frame01");
		sprintf(actors_defs[wraith].idle_sit_frame,"idledown01");
		sprintf(actors_defs[wraith].sit_down_frame,"sitdown01");
		sprintf(actors_defs[wraith].stand_up_frame,"getup01");
		actors_defs[wraith].walk_speed=default_walk_speed;
		actors_defs[wraith].run_speed=2.0/18;

		actors_defs[deer].ghost=0;
		sprintf(actors_defs[deer].skin_name,"./md2/wolf_deer.bmp");
		sprintf(actors_defs[deer].file_name,"./md2/deer.md2");
		sprintf(actors_defs[deer].walk_frame,"walk01");
		sprintf(actors_defs[deer].die1_frame,"die01");
		sprintf(actors_defs[deer].die2_frame,"dietwo01");
		sprintf(actors_defs[deer].pain1_frame,"pain01");
		sprintf(actors_defs[deer].pain2_frame,"paintwo01");
		sprintf(actors_defs[deer].idle_frame,"idle01");
		sprintf(actors_defs[deer].idle_sit_frame,"eat01");
		sprintf(actors_defs[deer].sit_down_frame,"ineat01");
		sprintf(actors_defs[deer].stand_up_frame,"outeat01");
		sprintf(actors_defs[deer].attack_cast_frame,"cast01");
		sprintf(actors_defs[deer].attack_up_1_frame,"attack01");
		sprintf(actors_defs[deer].attack_up_2_frame,"attack01");
		sprintf(actors_defs[deer].attack_up_3_frame,"attack01");
		sprintf(actors_defs[deer].attack_up_4_frame,"attack01");
		sprintf(actors_defs[deer].attack_down_1_frame,"attack01");
		sprintf(actors_defs[deer].attack_down_2_frame,"attack01");
		sprintf(actors_defs[deer].in_combat_frame,"intoidletwo01");
		sprintf(actors_defs[deer].out_combat_frame,"idleout01");
		sprintf(actors_defs[deer].combat_idle_frame,"idletwo01");
		actors_defs[deer].walk_speed=default_walk_speed;
		actors_defs[deer].run_speed=2.0/18;

		actors_defs[bear].ghost=0;
		sprintf(actors_defs[bear].skin_name,"./md2/bear.bmp");
		sprintf(actors_defs[bear].file_name,"./md2/bear.md2");
		sprintf(actors_defs[bear].walk_frame,"walk01");
		sprintf(actors_defs[bear].die1_frame,"die01");
		sprintf(actors_defs[bear].die2_frame,"dietwo01");
		sprintf(actors_defs[bear].pain1_frame,"pain01");
		sprintf(actors_defs[bear].pain2_frame,"paintwo01");
		sprintf(actors_defs[bear].pick_frame,"ineat01");
		sprintf(actors_defs[bear].drop_frame,"outeat01");
		sprintf(actors_defs[bear].idle_frame,"idle01");
		sprintf(actors_defs[bear].idle_sit_frame,"sleep01");
		sprintf(actors_defs[bear].harvest_frame,"eat01");
		sprintf(actors_defs[bear].attack_cast_frame,"cast01");
		sprintf(actors_defs[bear].attack_up_1_frame,"attack01");
		sprintf(actors_defs[bear].attack_up_2_frame,"attack01");
		sprintf(actors_defs[bear].attack_up_3_frame,"attackthr01");
		sprintf(actors_defs[bear].attack_up_4_frame,"attackthr01");
		sprintf(actors_defs[bear].attack_down_1_frame,"attacktwo01");
		sprintf(actors_defs[bear].attack_down_2_frame,"attacktwo01");
		sprintf(actors_defs[bear].in_combat_frame,"idletwo01");
		sprintf(actors_defs[bear].out_combat_frame,"idletwo01");
		sprintf(actors_defs[bear].combat_idle_frame,"idletwo01");
		sprintf(actors_defs[bear].sit_down_frame,"insleep01");
		sprintf(actors_defs[bear].stand_up_frame,"outsleep01");
		actors_defs[bear].walk_speed=default_walk_speed;
		actors_defs[bear].run_speed=2.0/18;

		actors_defs[bear2].ghost=0;
		sprintf(actors_defs[bear2].skin_name,"./md2/bear2.bmp");
		sprintf(actors_defs[bear2].file_name,"./md2/bear2.md2");
		sprintf(actors_defs[bear2].walk_frame,"walk01");
		sprintf(actors_defs[bear2].die1_frame,"die01");
		sprintf(actors_defs[bear2].die2_frame,"dietwo01");
		sprintf(actors_defs[bear2].pain1_frame,"pain01");
		sprintf(actors_defs[bear2].pain2_frame,"paintwo01");
		sprintf(actors_defs[bear2].pick_frame,"ineat01");
		sprintf(actors_defs[bear2].drop_frame,"outeat01");
		sprintf(actors_defs[bear2].idle_frame,"idle01");
		sprintf(actors_defs[bear2].idle_sit_frame,"sleep01");
		sprintf(actors_defs[bear2].harvest_frame,"eat01");
		sprintf(actors_defs[bear2].attack_cast_frame,"cast01");
		sprintf(actors_defs[bear2].attack_up_1_frame,"attack01");
		sprintf(actors_defs[bear2].attack_up_2_frame,"attack01");
		sprintf(actors_defs[bear2].attack_up_3_frame,"attack01");
		sprintf(actors_defs[bear2].attack_up_4_frame,"attack01");
		sprintf(actors_defs[bear2].attack_down_1_frame,"attacktwo01");
		sprintf(actors_defs[bear2].attack_down_2_frame,"attacktwo01");
		sprintf(actors_defs[bear2].in_combat_frame,"idletwo01");
		sprintf(actors_defs[bear2].out_combat_frame,"idletwo01");
		sprintf(actors_defs[bear2].combat_idle_frame,"idletwo01");
		sprintf(actors_defs[bear2].sit_down_frame,"insleep01");
		sprintf(actors_defs[bear2].stand_up_frame,"outsleep01");
		actors_defs[bear2].walk_speed=default_walk_speed;
		actors_defs[bear2].run_speed=2.0/18;

		actors_defs[wolf].ghost=0;
		sprintf(actors_defs[wolf].skin_name,"./md2/wolf_deer.bmp");
		sprintf(actors_defs[wolf].file_name,"./md2/wolf.md2");
		sprintf(actors_defs[wolf].walk_frame,"walk01");
		sprintf(actors_defs[wolf].die1_frame,"die01");
		sprintf(actors_defs[wolf].die2_frame,"dietwo01");
		sprintf(actors_defs[wolf].pain1_frame,"pain01");
		sprintf(actors_defs[wolf].pain2_frame,"paintwo01");
		sprintf(actors_defs[wolf].idle_frame,"idle01");
		sprintf(actors_defs[wolf].attack_cast_frame,"cast01");
		sprintf(actors_defs[wolf].attack_up_1_frame,"attack01");
		sprintf(actors_defs[wolf].attack_up_2_frame,"attack01");
		sprintf(actors_defs[wolf].attack_up_3_frame,"attack01");
		sprintf(actors_defs[wolf].attack_up_4_frame,"attack01");
		sprintf(actors_defs[wolf].attack_down_1_frame,"attacktwo01");
		sprintf(actors_defs[wolf].attack_down_2_frame,"attacktwo01");
		sprintf(actors_defs[wolf].in_combat_frame,"idletwo01");
		sprintf(actors_defs[wolf].out_combat_frame,"idletwo01");
		sprintf(actors_defs[wolf].combat_idle_frame,"idletwo01");
		sprintf(actors_defs[wolf].sit_down_frame,"ineat01");
		sprintf(actors_defs[wolf].stand_up_frame,"outeat01");
		actors_defs[wolf].walk_speed=default_walk_speed;
		actors_defs[wolf].run_speed=2.0/18;

		actors_defs[white_rabbit].ghost=0;
		sprintf(actors_defs[white_rabbit].skin_name,"./md2/bear.bmp");
		sprintf(actors_defs[white_rabbit].file_name,"./md2/rabbit2.md2");
		sprintf(actors_defs[white_rabbit].walk_frame,"walk01");
		sprintf(actors_defs[white_rabbit].die1_frame,"die01");
		sprintf(actors_defs[white_rabbit].die2_frame,"dietwo01");
		sprintf(actors_defs[white_rabbit].pain1_frame,"pain01");
		sprintf(actors_defs[white_rabbit].pain2_frame,"paintwo01");
		sprintf(actors_defs[white_rabbit].idle_frame,"idle01");
		sprintf(actors_defs[white_rabbit].attack_cast_frame,"cast01");
		sprintf(actors_defs[white_rabbit].attack_up_1_frame,"attack01");
		sprintf(actors_defs[white_rabbit].attack_up_2_frame,"attack01");
		sprintf(actors_defs[white_rabbit].attack_up_3_frame,"attack01");
		sprintf(actors_defs[white_rabbit].attack_up_4_frame,"attack01");
		sprintf(actors_defs[white_rabbit].attack_down_1_frame,"attack01");
		sprintf(actors_defs[white_rabbit].attack_down_2_frame,"attack01");
		sprintf(actors_defs[white_rabbit].in_combat_frame,"idle01");
		sprintf(actors_defs[white_rabbit].out_combat_frame,"idle01");
		sprintf(actors_defs[white_rabbit].combat_idle_frame,"idle01");
		sprintf(actors_defs[white_rabbit].idle_sit_frame,"eat01");
		sprintf(actors_defs[white_rabbit].sit_down_frame,"ineat01");
		sprintf(actors_defs[white_rabbit].stand_up_frame,"outeat01");
		actors_defs[white_rabbit].walk_speed=default_walk_speed;
		actors_defs[white_rabbit].run_speed=2.0/18;


		actors_defs[brown_rabbit].ghost=0;
		sprintf(actors_defs[brown_rabbit].skin_name,"./md2/bear.bmp");
		sprintf(actors_defs[brown_rabbit].file_name,"./md2/rabbit1.md2");
		sprintf(actors_defs[brown_rabbit].walk_frame,"walk01");
		sprintf(actors_defs[brown_rabbit].die1_frame,"die01");
		sprintf(actors_defs[brown_rabbit].die2_frame,"dietwo01");
		sprintf(actors_defs[brown_rabbit].pain1_frame,"pain01");
		sprintf(actors_defs[brown_rabbit].pain2_frame,"paintwo01");
		sprintf(actors_defs[brown_rabbit].idle_frame,"idle01");
		sprintf(actors_defs[brown_rabbit].attack_up_1_frame,"attack01");
		sprintf(actors_defs[brown_rabbit].attack_up_2_frame,"attack01");
		sprintf(actors_defs[brown_rabbit].attack_up_3_frame,"attack01");
		sprintf(actors_defs[brown_rabbit].attack_up_4_frame,"attack01");
		sprintf(actors_defs[brown_rabbit].attack_down_1_frame,"attack01");
		sprintf(actors_defs[brown_rabbit].attack_down_2_frame,"attack01");
		sprintf(actors_defs[brown_rabbit].idle_sit_frame,"eat01");
		sprintf(actors_defs[brown_rabbit].sit_down_frame,"ineat01");
		sprintf(actors_defs[brown_rabbit].stand_up_frame,"outeat01");
		sprintf(actors_defs[brown_rabbit].in_combat_frame,"idle01");
		sprintf(actors_defs[brown_rabbit].out_combat_frame,"idle01");
		sprintf(actors_defs[brown_rabbit].combat_idle_frame,"idle01");
		actors_defs[brown_rabbit].walk_speed=default_walk_speed;
		actors_defs[brown_rabbit].run_speed=2.0/18;

		actors_defs[boar].ghost=0;
		sprintf(actors_defs[boar].skin_name,"./md2/animals3.bmp");
		sprintf(actors_defs[boar].file_name,"./md2/boar.md2");
		sprintf(actors_defs[boar].walk_frame,"walk01");
		sprintf(actors_defs[boar].die1_frame,"die01");
		sprintf(actors_defs[boar].die2_frame,"dietwo01");
		sprintf(actors_defs[boar].pain1_frame,"pain01");
		sprintf(actors_defs[boar].pain2_frame,"paintwo01");
		sprintf(actors_defs[boar].idle_frame,"idle01");
		sprintf(actors_defs[boar].attack_up_1_frame,"attack01");
		sprintf(actors_defs[boar].attack_up_2_frame,"attack01");
		sprintf(actors_defs[boar].attack_up_3_frame,"attack01");
		sprintf(actors_defs[boar].attack_up_4_frame,"attack01");
		sprintf(actors_defs[boar].attack_down_1_frame,"attack01");
		sprintf(actors_defs[boar].attack_down_2_frame,"attack01");
		sprintf(actors_defs[boar].idle_sit_frame,"eat01");
		sprintf(actors_defs[boar].sit_down_frame,"ineat01");
		sprintf(actors_defs[boar].stand_up_frame,"outeat01");
		sprintf(actors_defs[boar].in_combat_frame,"idle01");
		sprintf(actors_defs[boar].out_combat_frame,"idle01");
		sprintf(actors_defs[boar].combat_idle_frame,"idle01");
		actors_defs[boar].walk_speed=default_walk_speed;
		actors_defs[boar].run_speed=2.0/18;

		actors_defs[snake1].ghost=0;
		sprintf(actors_defs[snake1].skin_name,"./md2/animals3.bmp");
		sprintf(actors_defs[snake1].file_name,"./md2/snake1.md2");
		sprintf(actors_defs[snake1].walk_frame,"walk01");
		sprintf(actors_defs[snake1].die1_frame,"die01");
		sprintf(actors_defs[snake1].die2_frame,"dietwo01");
		sprintf(actors_defs[snake1].pain1_frame,"pain01");
		sprintf(actors_defs[snake1].pain2_frame,"paintwo01");
		sprintf(actors_defs[snake1].idle_frame,"idle01");
		sprintf(actors_defs[snake1].attack_up_1_frame,"attack01");
		sprintf(actors_defs[snake1].attack_up_2_frame,"attack01");
		sprintf(actors_defs[snake1].attack_up_3_frame,"attack01");
		sprintf(actors_defs[snake1].attack_up_4_frame,"attack01");
		sprintf(actors_defs[snake1].attack_down_1_frame,"attack01");
		sprintf(actors_defs[snake1].attack_down_2_frame,"attack01");
		sprintf(actors_defs[snake1].idle_sit_frame,"sleep01");
		sprintf(actors_defs[snake1].sit_down_frame,"insleep01");
		sprintf(actors_defs[snake1].stand_up_frame,"outsleep01");
		sprintf(actors_defs[snake1].in_combat_frame,"idle01");
		sprintf(actors_defs[snake1].out_combat_frame,"idle01");
		sprintf(actors_defs[snake1].combat_idle_frame,"idle01");
		actors_defs[snake1].walk_speed=default_walk_speed;
		actors_defs[snake1].run_speed=2.0/18;

		actors_defs[snake2].ghost=0;
		sprintf(actors_defs[snake2].skin_name,"./md2/animals3.bmp");
		sprintf(actors_defs[snake2].file_name,"./md2/snake2.md2");
		sprintf(actors_defs[snake2].walk_frame,"walk01");
		sprintf(actors_defs[snake2].die1_frame,"die01");
		sprintf(actors_defs[snake2].die2_frame,"dietwo01");
		sprintf(actors_defs[snake2].pain1_frame,"pain01");
		sprintf(actors_defs[snake2].pain2_frame,"paintwo01");
		sprintf(actors_defs[snake2].idle_frame,"idle01");
		sprintf(actors_defs[snake2].attack_up_1_frame,"attack01");
		sprintf(actors_defs[snake2].attack_up_2_frame,"attack01");
		sprintf(actors_defs[snake2].attack_up_3_frame,"attack01");
		sprintf(actors_defs[snake2].attack_up_4_frame,"attack01");
		sprintf(actors_defs[snake2].attack_down_1_frame,"attack01");
		sprintf(actors_defs[snake2].attack_down_2_frame,"attack01");
		sprintf(actors_defs[snake2].idle_sit_frame,"sleep01");
		sprintf(actors_defs[snake2].sit_down_frame,"insleep01");
		sprintf(actors_defs[snake2].stand_up_frame,"outsleep01");
		sprintf(actors_defs[snake2].in_combat_frame,"idle01");
		sprintf(actors_defs[snake2].out_combat_frame,"idle01");
		sprintf(actors_defs[snake2].combat_idle_frame,"idle01");
		actors_defs[snake2].walk_speed=default_walk_speed;
		actors_defs[snake2].run_speed=2.0/18;

		actors_defs[snake3].ghost=0;
		sprintf(actors_defs[snake3].skin_name,"./md2/animals3.bmp");
		sprintf(actors_defs[snake3].file_name,"./md2/snake3.md2");
		sprintf(actors_defs[snake3].walk_frame,"walk01");
		sprintf(actors_defs[snake3].die1_frame,"die01");
		sprintf(actors_defs[snake3].die2_frame,"dietwo01");
		sprintf(actors_defs[snake3].pain1_frame,"pain01");
		sprintf(actors_defs[snake3].pain2_frame,"paintwo01");
		sprintf(actors_defs[snake3].idle_frame,"idle01");
		sprintf(actors_defs[snake3].attack_up_1_frame,"attack01");
		sprintf(actors_defs[snake3].attack_up_2_frame,"attack01");
		sprintf(actors_defs[snake3].attack_up_3_frame,"attack01");
		sprintf(actors_defs[snake3].attack_up_4_frame,"attack01");
		sprintf(actors_defs[snake3].attack_down_1_frame,"attack01");
		sprintf(actors_defs[snake3].attack_down_2_frame,"attack01");
		sprintf(actors_defs[snake3].idle_sit_frame,"sleep01");
		sprintf(actors_defs[snake3].sit_down_frame,"insleep01");
		sprintf(actors_defs[snake3].stand_up_frame,"outsleep01");
		sprintf(actors_defs[snake3].in_combat_frame,"idle01");
		sprintf(actors_defs[snake3].out_combat_frame,"idle01");
		sprintf(actors_defs[snake3].combat_idle_frame,"idle01");
		actors_defs[snake3].walk_speed=default_walk_speed;
		actors_defs[snake3].run_speed=2.0/18;

		actors_defs[fox].ghost=0;
		sprintf(actors_defs[fox].skin_name,"./md2/animals4.bmp");
		sprintf(actors_defs[fox].file_name,"./md2/fox.md2");
		sprintf(actors_defs[fox].walk_frame,"walk01");
		sprintf(actors_defs[fox].die1_frame,"die01");
		sprintf(actors_defs[fox].die2_frame,"dietwo01");
		sprintf(actors_defs[fox].pain1_frame,"pain01");
		sprintf(actors_defs[fox].pain2_frame,"paintwo01");
		sprintf(actors_defs[fox].idle_frame,"idle01");
		sprintf(actors_defs[fox].attack_cast_frame,"cast01");
		sprintf(actors_defs[fox].attack_up_1_frame,"attack01");
		sprintf(actors_defs[fox].attack_up_2_frame,"attack01");
		sprintf(actors_defs[fox].attack_up_3_frame,"attack01");
		sprintf(actors_defs[fox].attack_up_4_frame,"attack01");
		sprintf(actors_defs[fox].attack_down_1_frame,"attacktwo01");
		sprintf(actors_defs[fox].attack_down_2_frame,"attacktwo01");
		sprintf(actors_defs[fox].in_combat_frame,"idletwo01");
		sprintf(actors_defs[fox].out_combat_frame,"idletwo01");
		sprintf(actors_defs[fox].combat_idle_frame,"idletwo01");
		actors_defs[fox].walk_speed=default_walk_speed;
		actors_defs[fox].run_speed=2.0/18;

		actors_defs[puma].ghost=0;
		sprintf(actors_defs[puma].skin_name,"./md2/animals4.bmp");
		sprintf(actors_defs[puma].file_name,"./md2/mountainlion.md2");
		sprintf(actors_defs[puma].walk_frame,"walk01");
		sprintf(actors_defs[puma].die1_frame,"die01");
		sprintf(actors_defs[puma].die2_frame,"dietwo01");
		sprintf(actors_defs[puma].pain1_frame,"pain01");
		sprintf(actors_defs[puma].pain2_frame,"paintwo01");
		sprintf(actors_defs[puma].idle_frame,"idle01");
		sprintf(actors_defs[puma].attack_cast_frame,"cast01");
		sprintf(actors_defs[puma].attack_up_1_frame,"attack01");
		sprintf(actors_defs[puma].attack_up_2_frame,"attack01");
		sprintf(actors_defs[puma].attack_up_3_frame,"attack01");
		sprintf(actors_defs[puma].attack_up_4_frame,"attack01");
		sprintf(actors_defs[puma].attack_down_1_frame,"attack01");
		sprintf(actors_defs[puma].attack_down_2_frame,"attack01");
		sprintf(actors_defs[puma].in_combat_frame,"idletwo01");
		sprintf(actors_defs[puma].out_combat_frame,"idletwo01");
		sprintf(actors_defs[puma].combat_idle_frame,"idletwo01");
		actors_defs[puma].walk_speed=default_walk_speed;
		actors_defs[puma].run_speed=2.0/18;

		actors_defs[ogre_male_1].ghost=0;
		sprintf(actors_defs[ogre_male_1].skin_name,"./md2/ogrem1.bmp");
		sprintf(actors_defs[ogre_male_1].file_name,"./md2/ogrem1.md2");
		sprintf(actors_defs[ogre_male_1].walk_frame,"walk01");
		sprintf(actors_defs[ogre_male_1].die1_frame,"die01");
		sprintf(actors_defs[ogre_male_1].die2_frame,"die01");
		sprintf(actors_defs[ogre_male_1].pain1_frame,"pain01");
		sprintf(actors_defs[ogre_male_1].pain2_frame,"paintwo01");
		sprintf(actors_defs[ogre_male_1].idle_frame,"idle01");
		sprintf(actors_defs[ogre_male_1].attack_cast_frame,"cast01");
		sprintf(actors_defs[ogre_male_1].attack_up_1_frame,"punchone01");
		sprintf(actors_defs[ogre_male_1].attack_up_2_frame,"punchtwo01");
		sprintf(actors_defs[ogre_male_1].attack_up_3_frame,"punchone01");
		sprintf(actors_defs[ogre_male_1].attack_up_4_frame,"punchtwo01");
		sprintf(actors_defs[ogre_male_1].attack_down_1_frame,"kickone01");
		sprintf(actors_defs[ogre_male_1].attack_down_2_frame,"kickone01");
		sprintf(actors_defs[ogre_male_1].in_combat_frame,"fightstain01");
		sprintf(actors_defs[ogre_male_1].out_combat_frame,"fightstaout01");
		sprintf(actors_defs[ogre_male_1].combat_idle_frame,"fightidle01");
		actors_defs[ogre_male_1].walk_speed=default_walk_speed;
		actors_defs[ogre_male_1].run_speed=2.0/18;

		actors_defs[goblin_male_1].ghost=0;
		sprintf(actors_defs[goblin_male_1].skin_name,"./md2/goblinm1.bmp");
		sprintf(actors_defs[goblin_male_1].file_name,"./md2/goblinm1.md2");
		sprintf(actors_defs[goblin_male_1].walk_frame,"walk01");
		sprintf(actors_defs[goblin_male_1].die1_frame,"die01");
		sprintf(actors_defs[goblin_male_1].die2_frame,"die01");
		sprintf(actors_defs[goblin_male_1].pain1_frame,"pain01");
		sprintf(actors_defs[goblin_male_1].pain2_frame,"paintwo01");
		sprintf(actors_defs[goblin_male_1].idle_frame,"idle01");
		sprintf(actors_defs[goblin_male_1].attack_cast_frame,"cast01");
		sprintf(actors_defs[goblin_male_1].attack_up_1_frame,"punchone01");
		sprintf(actors_defs[goblin_male_1].attack_up_2_frame,"punchtwo01");
		sprintf(actors_defs[goblin_male_1].attack_up_3_frame,"punchone01");
		sprintf(actors_defs[goblin_male_1].attack_up_4_frame,"punchtwo01");
		sprintf(actors_defs[goblin_male_1].attack_down_1_frame,"kickone01");
		sprintf(actors_defs[goblin_male_1].attack_down_2_frame,"kickone01");
		sprintf(actors_defs[goblin_male_1].in_combat_frame,"fightstain01");
		sprintf(actors_defs[goblin_male_1].out_combat_frame,"fightstaout01");
		sprintf(actors_defs[goblin_male_1].combat_idle_frame,"fightidle01");
		actors_defs[goblin_male_1].walk_speed=default_walk_speed;
		actors_defs[goblin_male_1].run_speed=2.0/18;

		actors_defs[orc_male_1].ghost=0;
		sprintf(actors_defs[orc_male_1].skin_name,"./md2/orcm1.bmp");
		sprintf(actors_defs[orc_male_1].file_name,"./md2/orcm1.md2");
		sprintf(actors_defs[orc_male_1].walk_frame,"walk01");
		sprintf(actors_defs[orc_male_1].die1_frame,"die01");
		sprintf(actors_defs[orc_male_1].die2_frame,"die01");
		sprintf(actors_defs[orc_male_1].pain1_frame,"pain01");
		sprintf(actors_defs[orc_male_1].pain2_frame,"paintwo01");
		sprintf(actors_defs[orc_male_1].idle_frame,"idle01");
		sprintf(actors_defs[orc_male_1].attack_cast_frame,"cast01");
		sprintf(actors_defs[orc_male_1].attack_up_1_frame,"punchone01");
		sprintf(actors_defs[orc_male_1].attack_up_2_frame,"punchtwo01");
		sprintf(actors_defs[orc_male_1].attack_up_3_frame,"punchone01");
		sprintf(actors_defs[orc_male_1].attack_up_4_frame,"punchtwo01");
		sprintf(actors_defs[orc_male_1].attack_down_1_frame,"kickone01");
		sprintf(actors_defs[orc_male_1].attack_down_2_frame,"kickone01");
		sprintf(actors_defs[orc_male_1].in_combat_frame,"fightstain01");
		sprintf(actors_defs[orc_male_1].out_combat_frame,"fightstaout01");
		sprintf(actors_defs[orc_male_1].combat_idle_frame,"fightidle01");
		actors_defs[orc_male_1].walk_speed=default_walk_speed;
		actors_defs[orc_male_1].run_speed=2.0/18;

		actors_defs[orc_female_1].ghost=0;
		sprintf(actors_defs[orc_female_1].skin_name,"./md2/orcf1.bmp");
		sprintf(actors_defs[orc_female_1].file_name,"./md2/orcf1.md2");
		sprintf(actors_defs[orc_female_1].walk_frame,"walk01");
		sprintf(actors_defs[orc_female_1].die1_frame,"die01");
		sprintf(actors_defs[orc_female_1].die2_frame,"die01");
		sprintf(actors_defs[orc_female_1].pain1_frame,"pain01");
		sprintf(actors_defs[orc_female_1].pain2_frame,"paintwo01");
		sprintf(actors_defs[orc_female_1].idle_frame,"idle01");
		sprintf(actors_defs[orc_female_1].attack_cast_frame,"cast01");
		sprintf(actors_defs[orc_female_1].attack_up_1_frame,"punchone01");
		sprintf(actors_defs[orc_female_1].attack_up_2_frame,"punchtwo01");
		sprintf(actors_defs[orc_female_1].attack_up_3_frame,"punchone01");
		sprintf(actors_defs[orc_female_1].attack_up_4_frame,"punchtwo01");
		sprintf(actors_defs[orc_female_1].attack_down_1_frame,"kickone01");
		sprintf(actors_defs[orc_female_1].attack_down_2_frame,"kickone01");
		sprintf(actors_defs[orc_female_1].in_combat_frame,"fightstain01");
		sprintf(actors_defs[orc_female_1].out_combat_frame,"fightstaout01");
		sprintf(actors_defs[orc_female_1].combat_idle_frame,"fightidle01");
		actors_defs[orc_female_1].walk_speed=default_walk_speed;
		actors_defs[orc_female_1].run_speed=2.0/18;

		actors_defs[skeleton].ghost=0;
		sprintf(actors_defs[skeleton].skin_name,"./md2/ghouls.bmp");
		sprintf(actors_defs[skeleton].file_name,"./md2/skeleton.md2");
		sprintf(actors_defs[skeleton].walk_frame,"walk01");
		sprintf(actors_defs[skeleton].die1_frame,"die01");
		sprintf(actors_defs[skeleton].die2_frame,"die01");
		sprintf(actors_defs[skeleton].pain1_frame,"pain01");
		sprintf(actors_defs[skeleton].pain2_frame,"paintwo01");
		sprintf(actors_defs[skeleton].idle_frame,"idle01");
		sprintf(actors_defs[skeleton].attack_cast_frame,"cast01");
		sprintf(actors_defs[skeleton].attack_up_1_frame,"punchone01");
		sprintf(actors_defs[skeleton].attack_up_2_frame,"punchtwo01");
		sprintf(actors_defs[skeleton].attack_up_3_frame,"punchone01");
		sprintf(actors_defs[skeleton].attack_up_4_frame,"punchtwo01");
		sprintf(actors_defs[skeleton].attack_down_1_frame,"punchone01");
		sprintf(actors_defs[skeleton].attack_down_2_frame,"punchtwo01");
		sprintf(actors_defs[skeleton].in_combat_frame,"fightidle01");
		sprintf(actors_defs[skeleton].out_combat_frame,"fightidle01");
		sprintf(actors_defs[skeleton].combat_idle_frame,"fightidle01");
		sprintf(actors_defs[skeleton].sit_down_frame,"die01");
		sprintf(actors_defs[skeleton].stand_up_frame,"outdie01");
		sprintf(actors_defs[skeleton].idle_sit_frame,"dieidle01");
		actors_defs[skeleton].walk_speed=default_walk_speed;
		actors_defs[skeleton].run_speed=2.0/18;

		actors_defs[gargoyle1].ghost=0;
		sprintf(actors_defs[gargoyle1].skin_name,"./md2/gargoyle.bmp");
		sprintf(actors_defs[gargoyle1].file_name,"./md2/gargoyle1.md2");
		sprintf(actors_defs[gargoyle1].walk_frame,"walk01");
		sprintf(actors_defs[gargoyle1].die1_frame,"die01");
		sprintf(actors_defs[gargoyle1].die2_frame,"die01");
		sprintf(actors_defs[gargoyle1].pain1_frame,"pain01");
		sprintf(actors_defs[gargoyle1].pain2_frame,"paintwo01");
		sprintf(actors_defs[gargoyle1].idle_frame,"idle01");
		sprintf(actors_defs[gargoyle1].attack_cast_frame,"cast01");
		sprintf(actors_defs[gargoyle1].attack_up_1_frame,"punchone01");
		sprintf(actors_defs[gargoyle1].attack_up_2_frame,"punchtwo01");
		sprintf(actors_defs[gargoyle1].attack_up_3_frame,"punchone01");
		sprintf(actors_defs[gargoyle1].attack_up_4_frame,"punchtwo01");
		sprintf(actors_defs[gargoyle1].attack_down_1_frame,"kickone01");
		sprintf(actors_defs[gargoyle1].attack_down_2_frame,"kickone01");
		sprintf(actors_defs[gargoyle1].in_combat_frame,"fightstain01");
		sprintf(actors_defs[gargoyle1].out_combat_frame,"fightstaout01");
		sprintf(actors_defs[gargoyle1].combat_idle_frame,"fightidle01");
		sprintf(actors_defs[gargoyle1].sit_down_frame,"inidletwo01");
		sprintf(actors_defs[gargoyle1].stand_up_frame,"outidletwo01");
		sprintf(actors_defs[gargoyle1].idle_sit_frame,"idletwo01");
		actors_defs[gargoyle1].walk_speed=default_walk_speed;
		actors_defs[gargoyle1].run_speed=2.0/18;

		actors_defs[gargoyle2].ghost=0;
		sprintf(actors_defs[gargoyle2].skin_name,"./md2/gargoyle.bmp");
		sprintf(actors_defs[gargoyle2].file_name,"./md2/gargoyle2.md2");
		sprintf(actors_defs[gargoyle2].walk_frame,"walk01");
		sprintf(actors_defs[gargoyle2].die1_frame,"die01");
		sprintf(actors_defs[gargoyle2].die2_frame,"die01");
		sprintf(actors_defs[gargoyle2].pain1_frame,"pain01");
		sprintf(actors_defs[gargoyle2].pain2_frame,"paintwo01");
		sprintf(actors_defs[gargoyle2].idle_frame,"idle01");
		sprintf(actors_defs[gargoyle2].attack_cast_frame,"cast01");
		sprintf(actors_defs[gargoyle2].attack_up_1_frame,"punchone01");
		sprintf(actors_defs[gargoyle2].attack_up_2_frame,"punchtwo01");
		sprintf(actors_defs[gargoyle2].attack_up_3_frame,"punchone01");
		sprintf(actors_defs[gargoyle2].attack_up_4_frame,"punchtwo01");
		sprintf(actors_defs[gargoyle2].attack_down_1_frame,"kickone01");
		sprintf(actors_defs[gargoyle2].attack_down_2_frame,"kickone01");
		sprintf(actors_defs[gargoyle2].in_combat_frame,"fightstain01");
		sprintf(actors_defs[gargoyle2].out_combat_frame,"fightstaout01");
		sprintf(actors_defs[gargoyle2].combat_idle_frame,"fightidle01");
		sprintf(actors_defs[gargoyle2].sit_down_frame,"inidletwo01");
		sprintf(actors_defs[gargoyle2].stand_up_frame,"outidletwo01");
		sprintf(actors_defs[gargoyle2].idle_sit_frame,"idletwo01");
		actors_defs[gargoyle2].walk_speed=default_walk_speed;
		actors_defs[gargoyle2].run_speed=2.0/18;

		actors_defs[gargoyle3].ghost=0;
		sprintf(actors_defs[gargoyle3].skin_name,"./md2/gargoyle.bmp");
		sprintf(actors_defs[gargoyle3].file_name,"./md2/gargoyle3.md2");
		sprintf(actors_defs[gargoyle3].walk_frame,"walk01");
		sprintf(actors_defs[gargoyle3].die1_frame,"die01");
		sprintf(actors_defs[gargoyle3].die2_frame,"die01");
		sprintf(actors_defs[gargoyle3].pain1_frame,"pain01");
		sprintf(actors_defs[gargoyle3].pain2_frame,"paintwo01");
		sprintf(actors_defs[gargoyle3].idle_frame,"idle01");
		sprintf(actors_defs[gargoyle3].attack_cast_frame,"cast01");
		sprintf(actors_defs[gargoyle3].attack_up_1_frame,"punchone01");
		sprintf(actors_defs[gargoyle3].attack_up_2_frame,"punchtwo01");
		sprintf(actors_defs[gargoyle3].attack_up_3_frame,"punchone01");
		sprintf(actors_defs[gargoyle3].attack_up_4_frame,"punchtwo01");
		sprintf(actors_defs[gargoyle3].attack_down_1_frame,"kickone01");
		sprintf(actors_defs[gargoyle3].attack_down_2_frame,"kickone01");
		sprintf(actors_defs[gargoyle3].in_combat_frame,"fightstain01");
		sprintf(actors_defs[gargoyle3].out_combat_frame,"fightstaout01");
		sprintf(actors_defs[gargoyle3].combat_idle_frame,"fightidle01");
		sprintf(actors_defs[gargoyle3].sit_down_frame,"inidletwo01");
		sprintf(actors_defs[gargoyle3].stand_up_frame,"outidletwo01");
		sprintf(actors_defs[gargoyle3].idle_sit_frame,"idletwo01");
		actors_defs[gargoyle3].walk_speed=default_walk_speed;
		actors_defs[gargoyle3].run_speed=2.0/18;

		actors_defs[troll].ghost=0;
		sprintf(actors_defs[troll].skin_name,"./md2/troll.bmp");
		sprintf(actors_defs[troll].file_name,"./md2/troll1.md2");
		sprintf(actors_defs[troll].walk_frame,"walk01");
		sprintf(actors_defs[troll].die1_frame,"die01");
		sprintf(actors_defs[troll].die2_frame,"die01");
		sprintf(actors_defs[troll].pain1_frame,"pain01");
		sprintf(actors_defs[troll].pain2_frame,"paintwo01");
		sprintf(actors_defs[troll].idle_frame,"idle01");
		sprintf(actors_defs[troll].attack_cast_frame,"cast01");
		sprintf(actors_defs[troll].attack_up_1_frame,"punchone01");
		sprintf(actors_defs[troll].attack_up_2_frame,"punchtwo01");
		sprintf(actors_defs[troll].attack_up_3_frame,"punchone01");
		sprintf(actors_defs[troll].attack_up_4_frame,"punchtwo01");
		sprintf(actors_defs[troll].attack_down_1_frame,"kickone01");
		sprintf(actors_defs[troll].attack_down_2_frame,"kickone01");
		sprintf(actors_defs[troll].in_combat_frame,"fightstain01");
		sprintf(actors_defs[troll].out_combat_frame,"fightstaout01");
		sprintf(actors_defs[troll].combat_idle_frame,"fightidle01");
		actors_defs[troll].walk_speed=default_walk_speed;
		actors_defs[troll].run_speed=2.0/18;

		actors_defs[cyclops].ghost=0;
		sprintf(actors_defs[cyclops].skin_name,"./md2/cyclops.bmp");
		sprintf(actors_defs[cyclops].file_name,"./md2/cyclops1.md2");
		sprintf(actors_defs[cyclops].walk_frame,"walk01");
		sprintf(actors_defs[cyclops].die1_frame,"die01");
		sprintf(actors_defs[cyclops].die2_frame,"die01");
		sprintf(actors_defs[cyclops].pain1_frame,"pain01");
		sprintf(actors_defs[cyclops].pain2_frame,"paintwo01");
		sprintf(actors_defs[cyclops].idle_frame,"idle01");
		sprintf(actors_defs[cyclops].attack_cast_frame,"cast01");
		sprintf(actors_defs[cyclops].attack_up_1_frame,"punchone01");
		sprintf(actors_defs[cyclops].attack_up_2_frame,"punchtwo01");
		sprintf(actors_defs[cyclops].attack_up_3_frame,"punchone01");
		sprintf(actors_defs[cyclops].attack_up_4_frame,"punchtwo01");
		sprintf(actors_defs[cyclops].attack_down_1_frame,"kickone01");
		sprintf(actors_defs[cyclops].attack_down_2_frame,"kickone01");
		sprintf(actors_defs[cyclops].in_combat_frame,"fightstain01");
		sprintf(actors_defs[cyclops].out_combat_frame,"fightstaout01");
		sprintf(actors_defs[cyclops].combat_idle_frame,"fightidle01");
		actors_defs[cyclops].walk_speed=default_walk_speed;
		actors_defs[cyclops].run_speed=2.0/18;
}

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void end_actors_list();
 */

#ifdef ELC
#define DRAW_INGAME_NORMAL(x, y, our_string, max_lines)	draw_ingame_string(x, y, our_string, max_lines, INGAME_FONT_X_LEN, INGAME_FONT_Y_LEN)
#define DRAW_INGAME_SMALL(x, y, our_string, max_lines)	draw_ingame_string(x, y, our_string, max_lines, SMALL_INGAME_FONT_X_LEN, SMALL_INGAME_FONT_Y_LEN)
#define DRAW_INGAME_ALT(x, y, our_string, max_lines)	draw_ingame_string(x, y, our_string, max_lines, ALT_INGAME_FONT_X_LEN, ALT_INGAME_FONT_Y_LEN)
#endif

actor *actors_list[1000];
int max_actors=0;
SDL_mutex *actors_lists_mutex;	//used for locking between the timer and main threads

actor_types actors_defs[100];

void draw_actor_overtext( actor* actor_ptr ); /* forward declaration */

//Threading support for actors_lists
void init_actors_lists()
{
	int	i;

	actors_lists_mutex=SDL_CreateMutex();
	LOCK_ACTORS_LISTS();	//lock it to avoid timing issues
	for(i=0;i<1000;i++)actors_list[i]=NULL;
	UNLOCK_ACTORS_LISTS();	// release now that we are done
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
	actor *our_actor;

#ifdef EXTRA_DEBUG
	ERR();
#endif
	
	if(!remappable)texture_id=load_texture_cache(skin_name,150);
	else
		{
			texture_id=load_bmp8_remapped_skin(skin_name,150,skin_color,hair_color,shirt_color,pants_color,boots_color);
		}


	our_actor = calloc(1, sizeof(actor));

	memset(our_actor->current_displayed_text, 0, MAX_CURRENT_DISPLAYED_TEXT_LEN);
	our_actor->current_displayed_text_time_left =  0;

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

//	our_actor->model_data=returned_md2;
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

	//find a free spot, in the actors_list
	LOCK_ACTORS_LISTS();
	
	for(i=0;i<max_actors;i++)
		{
			if(!actors_list[i])break;
		}

	actors_list[i]=our_actor;
	if(i>=max_actors)max_actors=i+1;
	
	//It's unlocked later
	
	return i;
}

void set_health_color(float percent, float multiplier, float a)
{
	float r,g;

	r=(1.0f-percent)*2.0f;
	g=(percent/1.25f)*2.0f;

	if(r<0.0f)r=0.0f;
	else if(r>1.0f)r=1.0f;
	if(g<0.0f)g=0.0f;
	else if(g>1.0f)g=1.0f;

	glColor4f(r*multiplier,g*multiplier,0.0f, a);
}


void draw_actor_banner(actor * actor_id, float offset_z)
{
	char str[60];
	float healtbar_x=-0.25f*zoom_level/3.0f;
	float healtbar_y=0;
	float healtbar_z=offset_z+0.1f;	//was 0.2f
	float healtbar_x_len=0.5f*zoom_level/3.0f;
	float healtbar_x_len_converted=0;
	float healtbar_x_len_loss=0;
	float healtbar_x_loss_fade=1.0f;
	float healtbar_z_len=0.05f*zoom_level/3.0f;
	char temp[255];
	// are we activley drawing?
	if(SDL_GetAppState()&SDL_APPACTIVE){
		if(use_shadow_mapping)
			{
				glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
				glDisable(GL_TEXTURE_2D);
				ELglActiveTextureARB(shadow_unit);
				glDisable(depth_texture_target);
				disable_texgen();
				ELglActiveTextureARB(GL_TEXTURE0);
				glEnable(GL_TEXTURE_2D);
				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
			}
		//draw the health bar
		glDisable(GL_TEXTURE_2D);

		if(!actor_id->ghost)glDisable(GL_LIGHTING);

		if(view_health_bar && actor_id->cur_health>=0 && actor_id->max_health>0 && (!actor_id->dead))
			{
				float percentage = (float)actor_id->cur_health/(float)actor_id->max_health;
				//get it's length
				if(actor_id->last_health_loss && cur_time-actor_id->last_health_loss<1000){//only when using floatingmessages
					if(actor_id->damage>0){
						healtbar_x_len_converted=healtbar_x_len*percentage;
						healtbar_x_len_loss=healtbar_x_len*(float)((float)actor_id->damage/(float)actor_id->max_health);
						healtbar_x_loss_fade=1.0f-((float)(cur_time-actor_id->last_health_loss)/1000.0f);
					} else {
						healtbar_x_len_converted=healtbar_x_len*(float)((float)(actor_id->cur_health+actor_id->damage)/(float)actor_id->max_health);
						healtbar_x_len_loss=healtbar_x_len*(float)((float)(-actor_id->damage)/(float)actor_id->max_health);
						healtbar_x_loss_fade=((float)(cur_time-actor_id->last_health_loss)/1000.0f);
					}
				} else {
					healtbar_x_len_converted=healtbar_x_len*percentage;
					actor_id->last_health_loss=0;
				}

				glBegin(GL_QUADS);
				
				//choose tint color
				set_health_color(percentage, 0.5f, 1.0f);
				
				glVertex3f(healtbar_x,healtbar_y,healtbar_z);
				glVertex3f(healtbar_x+healtbar_x_len_converted,healtbar_y,healtbar_z);
				
				//choose color for the bar
				set_health_color(percentage, 1.0f, 1.0f);

				glVertex3f(healtbar_x+healtbar_x_len_converted,healtbar_y,healtbar_z+healtbar_z_len);
				glVertex3f(healtbar_x,healtbar_y,healtbar_z+healtbar_z_len);

				glEnd();

				if(healtbar_x_len_loss){
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					
					set_health_color(percentage, 0.5f, healtbar_x_loss_fade);
				
					glBegin(GL_QUADS);
						glVertex3f(healtbar_x+healtbar_x_len_converted, healtbar_y, healtbar_z);
						glVertex3f(healtbar_x+healtbar_x_len_converted+healtbar_x_len_loss, healtbar_y, healtbar_z);
					
					set_health_color(percentage, 1.0f, healtbar_x_loss_fade);
					
						glVertex3f(healtbar_x+healtbar_x_len_converted+healtbar_x_len_loss, healtbar_y, healtbar_z+healtbar_z_len);
						glVertex3f(healtbar_x+healtbar_x_len_converted, healtbar_y, healtbar_z+healtbar_z_len);
					glEnd();

					glDisable(GL_BLEND);
				}


				//draw the frame
				healtbar_y=0.001*zoom_level/3.0f;
				glDepthFunc(GL_LEQUAL);
				glColor3f(0,0,0);
				glBegin(GL_LINE_LOOP);
				glVertex3f(healtbar_x,healtbar_y,healtbar_z);
				glVertex3f(healtbar_x+healtbar_x_len,healtbar_y,healtbar_z);
				glVertex3f(healtbar_x+healtbar_x_len,healtbar_y,healtbar_z+healtbar_z_len);
				glVertex3f(healtbar_x,healtbar_y,healtbar_z+healtbar_z_len);
				glEnd();
			}

		glEnable(GL_TEXTURE_2D);
		glColor3f(1,0,0);

		glDepthFunc(GL_ALWAYS);
		if(actor_id->damage_ms)
			{
				if(floatingmessages_enabled){
					float a=1.0f-(float)(cur_time-actor_id->last_health_loss)/2000.0f;
					
					if(actor_id->damage>0){
						sprintf(str,"%i",actor_id->damage);
						glColor4f(1.0f, 0.1f, 0.2f, a);
					} else {
						sprintf(str,"%i",-actor_id->damage);
						glColor4f(0.3f, 1.0f, 0.3f, a);
					}
					
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
					draw_ingame_string(-(((float)get_string_width(str) * (0.17*zoom_level*name_zoom/3.0))/12.0)*0.5f, healtbar_z/2.0f+((1.0f-a)*0.5f), str, 1, 0.14, 0.21);
					glDisable(GL_BLEND);
				} else {
					sprintf(str,"%i",actor_id->damage);
					glColor3f(1,0.3f,0.3f);
					//draw_ingame_string(-0.1,healtbar_z-2.0f,str,1,1);
					//draw_ingame_string(-0.1,healtbar_z/2.0f,str,1,1);
					DRAW_INGAME_NORMAL(-0.1,healtbar_z/2.0f,str,1);
				}
			}
		glDepthFunc(GL_LESS);
		if(actor_id->actor_name[0] && (view_names || view_hp))
			{
				if(actor_id->ghost)glDisable(GL_BLEND);
				set_font(name_font);	// to variable length

				if(view_names)
					{
						if(actor_id->kind_of_actor==NPC)glColor3f(0.3f,0.8f,1.0f);
						else if(actor_id->kind_of_actor==HUMAN || actor_id->kind_of_actor==COMPUTER_CONTROLLED_HUMAN){
							switch(map_type){
								case 2:
									glColor3f(0.6f,0.9f,0.9f);
									break;
								case 1:
								default:
									glColor3f(1.0f,1.0f,1.0f);
							}
						} else if(actor_id->is_enhanced_model && (actor_id->kind_of_actor==PKABLE_HUMAN || actor_id->kind_of_actor==PKABLE_COMPUTER_CONTROLLED)) glColor3f(1.0f,0.0f,0.0f);
						else glColor3f(1.0f,1.0f,0.0f);
						//draw_ingame_string(-((float)get_string_width(actor_id->actor_name)*(SMALL_INGAME_FONT_X_LEN*zoom_level*name_zoom/3.0))/2.0/12.0,healtbar_z+(0.06f*zoom_level/3.0),actor_id->actor_name,1,0);
						strcpy(temp,actor_id->actor_name);
						if (actors_defs[actor_id->actor_type].coremodel!=NULL) strcat(temp," <CAL>");
						DRAW_INGAME_SMALL(-((float)get_string_width(actor_id->actor_name)*(SMALL_INGAME_FONT_X_LEN*zoom_level*name_zoom/3.0))/2.0/12.0,healtbar_z+(0.06f*zoom_level/3.0),temp,1);
					}
				if(view_hp && actor_id->cur_health > 0 && actor_id->max_health > 0 && (!actor_id->dead) && (actor_id->kind_of_actor != NPC))
					{
						char hp[200];
						float	off;

						//choose color for the health
						set_health_color((float)actor_id->cur_health/(float)actor_id->max_health, 1.0f, 1.0f);
						sprintf(hp,"%d/%d", actor_id->cur_health, actor_id->max_health);
						if(view_health_bar)	off= (0.7*zoom_level*name_zoom/3.0);
						else off= 0.0;
						DRAW_INGAME_ALT(-(((float)get_string_width(hp)*(ALT_INGAME_FONT_X_LEN*zoom_level*name_zoom/3.0))/2.0/12.0)+off,healtbar_z-(0.05*zoom_level*name_zoom/3.0),hp,1);
					}
				set_font(0);	// back to fixed pitch
				if(actor_id->ghost)glEnable(GL_BLEND);
			}
		if ((actor_id->current_displayed_text_time_left>0)&&(actor_id->current_displayed_text[0] != 0))
			{
				draw_actor_overtext( actor_id );
			}

		if(floatingmessages_enabled)drawactor_floatingmessages(actor_id->actor_id, healtbar_z);
		
		glColor3f(1,1,1);
		if(!actor_id->ghost)glEnable(GL_LIGHTING);
		if(use_shadow_mapping)
			{
				last_texture=-1;
				glPopAttrib();
			}
	}
}

void draw_bubble(float x_left, float x_right, float x_leg_left, float x_leg_right, float y_top, float y_bottom, float y_actor)
{
	const float r=0.1f;
	const float mul=3.14159265f/180.0f;
	int angle;
	
	glEnable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
	glBlendFunc(GL_NONE, GL_SRC_ALPHA);
	glBegin(GL_POLYGON);
	
	for(angle=90;angle<180;angle+=10){
		float rad=-mul*angle;
		glVertex3f(x_left+cos(rad)*r-r, 0.01f, y_bottom+r+sin(rad)*r);
	}
	
	for(angle=180;angle<270;angle+=10){
		float rad=-mul*angle;
		glVertex3f(x_left+cos(rad)*r-r, 0.01f, y_top-r+sin(rad)*r);
	}
	
	for(angle=270;angle<360;angle+=10){
		float rad=-mul*angle;
		glVertex3f(x_right+cos(rad)*r+r, 0.01f, y_top-r+sin(rad)*r);
	}
	
	for(angle=0;angle<90;angle+=10){
		float rad=-mul*angle;
		glVertex3f(x_right+cos(rad)*r+r, 0.01f, y_bottom+sin(rad)*r+r);
	}
	
	glEnd();
	
	glBegin(GL_POLYGON);
		glVertex3f(x_leg_right, 0.01f, y_bottom+0.02);
		glVertex3f(x_leg_right, 0.01f, y_actor);
		glVertex3f(x_leg_left, 0.01f, y_bottom+0.02);
	glEnd();	
	
	glDisable(GL_BLEND);
}

//-- Logan Dugenoux [5/26/2004]
void draw_actor_overtext( actor* actor_ptr )
{
	float z, w, h;
	float x_left, x_right, x_leg_left, x_leg_right, y_top, y_bottom, y_actor;
	float textwidth;
	float textheight;
	float margin;

	//-- decrease display time
	actor_ptr->current_displayed_text_time_left -= (cur_time-last_time);
	if(!(SDL_GetAppState()&SDL_APPACTIVE)) return;	// not actually drawing, fake it

	textwidth = ((float)get_string_width(actor_ptr->current_displayed_text)*(SMALL_INGAME_FONT_X_LEN*zoom_level*name_zoom/3.0))/12.0;
	textheight = (0.06f*zoom_level/3.0)*4;
	margin = 0.02f*zoom_level;
	z = 1.2f;// distance over the player
	if (actor_ptr->sitting)		z = 0.8f; // close if he's sitting
	w = textwidth+margin*2;
	h = textheight+margin*2;

	x_left=-w/2.0f;
	x_right=w/2.0f;
	x_leg_left=-0.3f;
	x_leg_right=0.0f;

	y_top=z+0.7f+h;
	y_bottom=z+0.7f;
	y_actor=z+0.2f;

	glDisable(GL_TEXTURE_2D);
	
	draw_bubble(x_left+0.01f, x_right-0.01f, x_leg_left, x_leg_right, y_top-0.01f, y_bottom+0.01f, y_actor+0.01f);
		
	glEnable(GL_TEXTURE_2D);

	//---
	// Draw text
	glColor3f(0.77f,0.57f,0.39f);
	
	DRAW_INGAME_SMALL(x_left+margin, y_bottom+margin,actor_ptr->current_displayed_text,1);
	
	//glDepthFunc(GL_LESS);
	if (actor_ptr->current_displayed_text_time_left<=0)
	{	// clear if needed
		actor_ptr->current_displayed_text_time_left = 0;
		actor_ptr->current_displayed_text[0] = 0;
	}
}

float cal_get_maxz2(actor *act)
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

void draw_actor(actor * actor_id)
{
	//int i;
	double x_pos,y_pos,z_pos;
	float x_rot,y_rot,z_rot;
	int texture_id;
	float healtbar_z=0;
	if(!actor_id->remapped_colors)texture_id=get_texture_id(actor_id->texture_id);
	else
		{
			//we have remaped colors, we don't store such textures into the cache
			texture_id=actor_id->texture_id;
		}
	bind_texture_id(texture_id);

	//now, go and find the current frame
	//i=get_frame_number(actor_id->model_data, actor_id->tmp.cur_frame);
	//if(i >= 0)healtbar_z=actor_id->model_data->offsetFrames[i].box.max_z;
	if (actors_defs[actor_id->actor_type].coremodel!=NULL){
		healtbar_z=cal_get_maxz2(actor_id)+0.2;
	}

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
	
	//glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	//glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	//glRotatef(y_rot, 0.0f, 1.0f, 0.0f);


	if (actors_defs[actor_id->actor_type].coremodel!=NULL) {
		glPushMatrix();
		z_rot+=180;//test
		glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
		glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
		glRotatef(y_rot, 0.0f, 1.0f, 0.0f);
		cal_render_actor(actor_id);
		glPopMatrix();
	} else {
		glPushMatrix();
		glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
		glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
		glRotatef(y_rot, 0.0f, 1.0f, 0.0f);
		glPopMatrix();
	}

	glPopMatrix();//restore the scene
	//now, draw their damage & nametag
	glPushMatrix();
	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);
	glRotatef(-rz, 0.0f, 0.0f, 1.0f);

	draw_actor_banner(actor_id, healtbar_z);
	glPopMatrix();//we don't want to affect the rest of the scene
}

void display_actors()
{
	int i;
	int x,y;
	int	has_ghosts=0;
	x=-cx;
	y=-cy;

	vertex_arrays_built=0;	// clear the counter
	//MD2s don't have real normals...
	glNormal3f(0.0f,0.0f,1.0f);
	if(have_multitexture)
		{
			ELglActiveTextureARB(base_unit);
			glEnable(GL_TEXTURE_2D);
			ELglClientActiveTextureARB(base_unit);
		}
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	//display only the non ghosts
	for(i=0;i<max_actors;i++)
		{
			actor *cur_actor= actors_list[i];
			if(cur_actor) {
				if(!cur_actor->ghost)
					{
						int dist1;
						int dist2;

						if(!cur_actor->tmp.have_tmp)continue;
						dist1=x-cur_actor->tmp.x_pos;
						dist2=y-cur_actor->tmp.y_pos;
						
						if(dist1*dist1+dist2*dist2<=12*12)
							{
								if(cur_actor->is_enhanced_model)
									{
										draw_enhanced_actor(cur_actor);
										//check for network data - reduces resyncs
										get_message_from_server();
										if(actors_list[i]==NULL || cur_actor!=actors_list[i])continue;//The server might destroy our actor in that very moment...
									}
								else
									{
										draw_actor(cur_actor);
									}
								if(cur_actor->kind_of_actor==NPC)anything_under_the_mouse(i, UNDER_MOUSE_NPC);
								else
									if(cur_actor->kind_of_actor==HUMAN || cur_actor->kind_of_actor==COMPUTER_CONTROLLED_HUMAN || 
									   (cur_actor->is_enhanced_model && (cur_actor->kind_of_actor==PKABLE_HUMAN || cur_actor->kind_of_actor==PKABLE_COMPUTER_CONTROLLED)))
										anything_under_the_mouse(i, UNDER_MOUSE_PLAYER);
									else anything_under_the_mouse(i, UNDER_MOUSE_ANIMAL);
							}
					}
				else
					{
						has_ghosts++;
					}
			}
		}

	//we don't need the light, for ghosts
	glDisable(GL_LIGHTING);
	//if any ghost has a glowing weapon, we need to reset the blend function each ghost actor.
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	if(has_ghosts){
		//display only the ghosts
		glEnable(GL_BLEND);
		for(i=0;i<max_actors;i++)
			{
				actor *cur_actor= actors_list[i];
				if(cur_actor)
					if(cur_actor->ghost)
						{
							int dist1;
							int dist2;

							if (!cur_actor->tmp.have_tmp) continue;
							dist1=x-cur_actor->tmp.x_pos;
							dist2=y-cur_actor->tmp.y_pos;
							
							if(dist1*dist1+dist2*dist2<=12*12)
								{
									if(cur_actor->is_enhanced_model)
										{
											draw_enhanced_actor(cur_actor);
										}
									else
										{
											draw_actor(cur_actor);
										}
									if(cur_actor->kind_of_actor==NPC)anything_under_the_mouse(i, UNDER_MOUSE_NPC);
									else
										if(cur_actor->kind_of_actor==HUMAN || cur_actor->kind_of_actor==COMPUTER_CONTROLLED_HUMAN || 
										   (cur_actor->is_enhanced_model && (cur_actor->kind_of_actor==PKABLE_HUMAN || cur_actor->kind_of_actor==PKABLE_COMPUTER_CONTROLLED)))
											anything_under_the_mouse(i, UNDER_MOUSE_PLAYER);
										else anything_under_the_mouse(i, UNDER_MOUSE_ANIMAL);
								}
						}
			}
	}

	glDisable(GL_BLEND);
	if(have_multitexture) ELglClientActiveTextureARB(base_unit);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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

	actor_id=SDL_SwapLE16(*((short *)(in_data)));
	x_pos=SDL_SwapLE16(*((short *)(in_data+2)));
	y_pos=SDL_SwapLE16(*((short *)(in_data+4)));
	z_pos=SDL_SwapLE16(*((short *)(in_data+6)));
	z_rot=SDL_SwapLE16(*((short *)(in_data+8)));
	actor_type=SDL_SwapLE16(*((short *)(in_data+10)));
	remapable=*(in_data+11);
	skin=*(in_data+12);
	hair=*(in_data+13);
	shirt=*(in_data+14);
	pants=*(in_data+15);
	boots=*(in_data+16);
	frame=*(in_data+17);
	max_health=SDL_SwapLE16(*((short *)(in_data+18)));
	cur_health=SDL_SwapLE16(*((short *)(in_data+20)));
	kind_of_actor=*(in_data+22);

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
			char str[120];
			sprintf(str,"%s %d - %s\n",unknown_frame,frame,&in_data[23]);
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
				if(actors_list[i]->actor_id==actor_id)
					{
						char str[256];
						sprintf(str,duplicate_actors_str,actor_id, actors_list[i]->actor_name ,&in_data[23]);
						log_error(str);
						destroy_actor(actors_list[i]->actor_id);//we don't want two actors with the same ID
						i--;// last actor was put here, he needs to be checked too
					}
		}

	i=add_actor(actors_defs[actor_type].file_name,actors_defs[actor_type].skin_name,cur_frame,
				f_x_pos, f_y_pos, f_z_pos, f_z_rot,remapable, skin, hair, shirt, pants, boots, actor_id);
	
	if(i==-1) return;//A nasty error occured and we couldn't add the actor. Ignore it.
	
	//The actors list is locked when we get here...
	
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
			snprintf(str, 120, "%s (%d): %s/%d\n", bad_actor_name_length, actors_list[i]->actor_type,&in_data[23], (int)strlen(&in_data[23]));
			log_error(str);
		}
	else my_strncp(actors_list[i]->actor_name,&in_data[23],30);

	if (actors_defs[actor_type].coremodel!=NULL) {
		//Setup cal3d model
		actors_list[i]->calmodel=CalModel_New(actors_defs[actor_type].coremodel);
		//Attach meshes
		CalModel_AttachMesh(actors_list[i]->calmodel,actors_defs[actor_type].shirt[(int)shirt].mesh_index);
		CalModel_Update(actors_list[i]->calmodel,0);
		actors_list[i]->cur_anim.anim_index=-1;
		actors_list[i]->IsOnIdle=0;
	}

	UNLOCK_ACTORS_LISTS();	//unlock it	
#ifdef EXTRA_DEBUG
	ERR();
#endif
	
}

//--- LoganDugenoux [5/25/2004]
#define MS_PER_CHAR	200
#define MINI_BUBBLE_MS	500
void	add_displayed_text_to_actor( actor * actor_ptr, const char* text )
{
	int len_to_add;
	len_to_add = strlen(text);
	strncpy( actor_ptr->current_displayed_text, text, MAX_CURRENT_DISPLAYED_TEXT_LEN-1 );
	actor_ptr->current_displayed_text[MAX_CURRENT_DISPLAYED_TEXT_LEN-1] = 0;
	actor_ptr->current_displayed_text_time_left = len_to_add*MS_PER_CHAR;

	actor_ptr->current_displayed_text_time_left += MINI_BUBBLE_MS;
}

//--- LoganDugenoux [5/25/2004]
actor *	get_actor_ptr_from_id( int actor_id )
{
	int i;
	for (i = 0; i < max_actors; i++)
	{
		if (actors_list[i]->actor_id == actor_id)
			return actors_list[i];
	}
	return NULL;
}

/* currently UNUSED
void end_actors_lists()
{
	SDL_DestroyMutex(actors_lists_mutex);
	actors_lists_mutex=NULL;
}
*/

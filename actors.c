#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"

actor *actors_list[1000];
int max_actors=0;
SDL_mutex *actors_lists_mutex;	//used for locking between the timer and main threads

actor_types actors_defs[40];

//Threading support for actors_lists
void init_actors_lists()
{
	int	i;

	actors_lists_mutex=SDL_CreateMutex();
	lock_actors_lists();	//lock it to avoid timing issues
	for(i=0;i<1000;i++)actors_list[i]=NULL;
	unlock_actors_lists();	// release now that we are done
}

void end_actors_lists()
{
	SDL_DestroyMutex(actors_lists_mutex);
	actors_lists_mutex=NULL;
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
			sprintf(str,"%s: %s: %s\n",reg_error_str,cant_load_actor,file_name);
			log_error(str);
			return 0;
		}
	if(!remappable)texture_id=load_texture_cache(skin_name,150);
	else
		{
			texture_id=load_bmp8_remapped_skin(skin_name,150,skin_color,hair_color,shirt_color,pants_color,boots_color);
		}

	memset(our_actor->current_displayed_text, 0, max_current_displayed_text_len);
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
		//choose color for the bar
		if(actor_id->cur_health>=actor_id->max_health/2)
			glColor3f(0,0.5,0);	//green life bar
		//else if(actor_id->cur_health>=actor_id->max_health/4 && actor_id->cur_health<actor_id->max_health/2)
		else if(actor_id->cur_health>=actor_id->max_health/4)
			glColor3f(0.5,0.5,0);	//yellow life bar
		else glColor3f(0.5,0,0);	//red life bar
		if(!actor_id->ghost)glDisable(GL_LIGHTING);

		if(view_health_bar && actor_id->cur_health>=0 && (!actor_id->dead))
			{
				//get it's lenght
				if(actor_id->max_health > 0)//we don't want a division by zero, now do we?
					{
						healtbar_x_len_converted=healtbar_x_len*(float)((float)actor_id->cur_health/(float)actor_id->max_health);
					}
				glBegin(GL_QUADS);
				glVertex3f(healtbar_x,healtbar_y,healtbar_z);
				glVertex3f(healtbar_x+healtbar_x_len_converted,healtbar_y,healtbar_z);
				//choose color for the bar
				if(actor_id->cur_health>=actor_id->max_health/2)
			  		glColor3f(0,1,0);	//green life bar
				else if(actor_id->cur_health>=actor_id->max_health/4)
			  		glColor3f(1,1,0);	//yellow life bar
				else glColor3f(1,0,0);	//red life bar

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
				//draw_ingame_string(-0.1,healtbar_z/2.0f,str,1,1);
				draw_ingame_normal(-0.1,healtbar_z/2.0f,str,1);
			}
		glDepthFunc(GL_LESS);
		if(actor_id->actor_name[0] && (view_names || view_hp))
			{
				if(actor_id->ghost)glDisable(GL_BLEND);
				set_font(name_font);	// to variable length

				if(view_names)
					{
						if(actor_id->kind_of_actor==NPC)glColor3f(0.3f,0.8f,1.0f);
						else if(actor_id->kind_of_actor==HUMAN || actor_id->kind_of_actor==COMPUTER_CONTROLLED_HUMAN)glColor3f(1.0f,1.0f,1.0f);
						else if(actor_id->is_enhanced_model && (actor_id->kind_of_actor==PKABLE_HUMAN || actor_id->kind_of_actor==PKABLE_COMPUTER_CONTROLLED))glColor3f(1.0f,0.0f,0.0f);
						else glColor3f(1.0f,1.0f,0.0f);
						//draw_ingame_string(-((float)get_string_width(actor_id->actor_name)*(SMALL_INGAME_FONT_X_LEN*zoom_level*name_zoom/3.0))/2.0/12.0,healtbar_z+(0.06f*zoom_level/3.0),actor_id->actor_name,1,0);
						draw_ingame_small(-((float)get_string_width(actor_id->actor_name)*(SMALL_INGAME_FONT_X_LEN*zoom_level*name_zoom/3.0))/2.0/12.0,healtbar_z+(0.06f*zoom_level/3.0),actor_id->actor_name,1);
					}
				if(view_hp && actor_id->cur_health > 0 && (!actor_id->dead) && (actor_id->kind_of_actor != NPC))
					{
						char hp[200];
						float	off;

						//choose color for the health
						if(actor_id->cur_health>=actor_id->max_health/2)
							glColor3f(0,1,0);	//green life bar
						else if(actor_id->cur_health>=actor_id->max_health/4)
							glColor3f(1,1,0);	//yellow life bar
						else glColor3f(1,0,0);	//red life bar
						sprintf(hp,"%d/%d", actor_id->cur_health, actor_id->max_health);
						if(view_health_bar)	off= (0.7*zoom_level*name_zoom/3.0);
						else off= 0.0;
						draw_ingame_alt(-(((float)get_string_width(hp)*(ALT_INGAME_FONT_X_LEN*zoom_level*name_zoom/3.0))/2.0/12.0)+off,healtbar_z-(0.05*zoom_level*name_zoom/3.0),hp,1);
					}
				set_font(0);	// back to fixed pitch
				if(actor_id->ghost)glEnable(GL_BLEND);
			}
		if ((actor_id->current_displayed_text_time_left>0)&&(actor_id->current_displayed_text[0] != 0))
			{
				draw_actor_overtext( actor_id );
			}

		glColor3f(1,1,1);
		if(!actor_id->ghost)glEnable(GL_LIGHTING);
		if(use_shadow_mapping)
			{
				last_texture=-1;
				glPopAttrib();
			}
	}
}

//-- Logan Dugenoux [5/26/2004]
void draw_actor_overtext( actor* actor_ptr )
{
	float z, w, h, fmargin, bulleZ;
	float ptx[10];
	float pty[10];
	float taillepicx, taillepicy;
	float delalageX, delalagePicX;
	float tailleTexteWidth, tailleTexteHeight;
	float decalageTexteX, decalageTexteY;
	int i;

	//-- decrease display time
	actor_ptr->current_displayed_text_time_left -= (cur_time-last_time);
	if(!(SDL_GetAppState()&SDL_APPACTIVE)) return;	// not actually drawing, fake it
	bulleZ = 0.01f;// put text a little bit closer than the bubble

	tailleTexteWidth = ((float)get_string_width(actor_ptr->current_displayed_text)
		*(SMALL_INGAME_FONT_X_LEN*zoom_level*name_zoom/3.0))/12.0;
	tailleTexteHeight = (0.06f*zoom_level/3.0)*4;
	fmargin = 0.02f*zoom_level;// border of the bubble size
	delalageX = 0.3f;// position of the bubble
	taillepicy = 0.7f;// size of the bubble 'leg'
	taillepicx = 0.1f;
	//delalagePicX = 0.3f;
	delalagePicX = 0.2f;
	z = 1.2f;// distance over the player
	if (actor_ptr->sitting)		z = 0.8f; // close if he's sitting
	w = tailleTexteWidth+fmargin*2;
	h = tailleTexteHeight+fmargin*2;

	// define bubble pts
	ptx[0] = 0;		pty[0] = z;
	ptx[1] = taillepicx+delalagePicX;	pty[1] = z+taillepicy;
	ptx[2] = w/2+delalageX+delalagePicX;	pty[2] = z+taillepicy;
	ptx[3] = w/2+delalageX+delalagePicX;	pty[3] = z+taillepicy+h;
	ptx[4] = -w/2+delalageX+delalagePicX;	pty[4] = z+taillepicy+h;
	ptx[5] = -w/2+delalageX+delalagePicX;	pty[5] = z+taillepicy;
	ptx[6] = -taillepicx+delalagePicX;	pty[6] = z+taillepicy;
	if (ptx[6]<=ptx[5])
	{// small texts
		ptx[6] = ptx[5];
		ptx[1] = ptx[5]+taillepicx*2;
	}

	decalageTexteX = -w/2+delalageX+delalagePicX+fmargin;
	decalageTexteY = z+taillepicy-fmargin+tailleTexteHeight/2;

	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	for (i=0;i<=5;i++)
	{
		glVertex3f(ptx[i],		bulleZ,		pty[i]);
		glVertex3f(ptx[i+1],	bulleZ,		pty[i+1]);
	}
	glVertex3f(ptx[6],		bulleZ,		pty[6]);
	glVertex3f(ptx[0],		bulleZ,		pty[0]);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	//---
	// Draw text
	glColor3f(0.4f,0.4f,0.4f);

	//draw_ingame_string(decalageTexteX,	decalageTexteY,actor_ptr->current_displayed_text,1,0);
	draw_ingame_small(decalageTexteX,	decalageTexteY,actor_ptr->current_displayed_text,1);

	//glDepthFunc(GL_LESS);
	if (actor_ptr->current_displayed_text_time_left<=0)
	{	// clear if needed
		actor_ptr->current_displayed_text_time_left = 0;
		actor_ptr->current_displayed_text[0] = 0;
	}
}


int get_frame_number(const md2 *model_data, const char *cur_frame)
{
	Uint32 frame;
	Uint8 str[256];

	for(frame=0; frame < model_data->numFrames; frame++)
		{
			if (!strcmp(cur_frame, (char *)&model_data->offsetFrames[frame].name))
				{
					return frame;
				}
		}
	snprintf(str, 256, "%s: %s: %s\n",reg_error_str,cant_find_frame,cur_frame);
	log_error(str);

	for(frame=0; frame < model_data->numFrames; frame++)
		{
			if(!strcmp("idle01", (char *)&model_data->offsetFrames[frame].name))
				{
					return frame;
				}
		}
	snprintf(str, 256, "%s: %s: %s\n",reg_error_str,cant_find_frame,"idle01");
	log_error(str);

	return -1;
}



void draw_model_halo(md2 *model_data,char *cur_frame, float r, float g, float b)
{
	int frame;
	int numFaces;
	int k;

	//glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_SRC_ALPHA);
	glDisable(GL_LIGHTING);

	glColor4f(r,g,b,0.99f);

	glPushMatrix();

	for(k=0;k<2;k++)
		{
				//glScalef(1.01f+0.01f*(float)k, 1.01f+0.01f*(float)k, 1.01f+0.01f*(float)k);
				glScalef(0.98f+0.05f*(float)k, 0.98f+0.05f*(float)k, 0.98f+0.05f*(float)k);

				frame = get_frame_number(model_data, cur_frame);
				if(frame < 0)	return;
				//track the usage
				cache_use(cache_md2, model_data->cache_ptr);
				if(!(SDL_GetAppState()&SDL_APPACTIVE)) continue;	// not actually drawing, fake it

				numFaces=model_data->numFaces;
				if(use_vertex_array > 0)
					{
						//TODO: smarter decision making and maybe trigger cleanup?
						if(!model_data->text_coord_array || !model_data->offsetFrames[frame].vertex_array)
							{
								build_md2_va(model_data, &model_data->offsetFrames[frame]);
							}
					}
				// determine the drawing method
				if(use_vertex_array > 0 && model_data->text_coord_array && model_data->offsetFrames[frame].vertex_array)
					{
						glTexCoordPointer(2,GL_FLOAT,0,model_data->text_coord_array);
						glVertexPointer(3,GL_FLOAT,0,model_data->offsetFrames[frame].vertex_array);

						//check_gl_errors();
						if(have_compiled_vertex_array)ELglLockArraysEXT(0, numFaces*3);
						glDrawArrays(GL_TRIANGLES, 0, numFaces*3);
						if(have_compiled_vertex_array)ELglUnlockArraysEXT();
					}
				else
					{
						int i;
						text_coord_md2 *offsetTexCoords;
						face_md2 *offsetFaces;
						vertex_md2 *vertex_pointer=NULL;
						//setup
						glBegin(GL_TRIANGLES);
						vertex_pointer=model_data->offsetFrames[frame].vertex_pointer;
						offsetFaces=model_data->offsetFaces;
						offsetTexCoords=model_data->offsetTexCoords;
						//draw each triangle
						for(i=0;i<numFaces;i++)
							{
								float x,y,z;

								x=vertex_pointer[offsetFaces[i].a].x;
								y=vertex_pointer[offsetFaces[i].a].y;
								z=vertex_pointer[offsetFaces[i].a].z;
								glVertex3f(x,y,z);

								x=vertex_pointer[offsetFaces[i].b].x;
								y=vertex_pointer[offsetFaces[i].b].y;
								z=vertex_pointer[offsetFaces[i].b].z;
								glVertex3f(x,y,z);

								x=vertex_pointer[offsetFaces[i].c].x;
								y=vertex_pointer[offsetFaces[i].c].y;
								z=vertex_pointer[offsetFaces[i].c].z;
								glVertex3f(x,y,z);
							}
						glEnd();
					}
				//check_gl_errors();
		}
	check_gl_errors();

	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	//glEnable(GL_CULL_FACE);
}



void draw_model(md2 *model_data,char *cur_frame, int ghost)
{
	int frame;
	int numFaces;

	frame = get_frame_number(model_data, cur_frame);
	if(frame < 0)	return;
	//track the usage
	cache_use(cache_md2, model_data->cache_ptr);
	if(!(SDL_GetAppState()&SDL_APPACTIVE)) return;	// not actually drawing, fake it

	numFaces=model_data->numFaces;
	check_gl_errors();
	glColor3f(1.0f, 1.0f, 1.0f);
	if(use_vertex_array > 0)
		{
			//TODO: smarter decision making and maybe trigger cleanup?
			if(!model_data->text_coord_array || !model_data->offsetFrames[frame].vertex_array)
				{
					build_md2_va(model_data, &model_data->offsetFrames[frame]);
				}
		}
	// determine the drawing method
	if(use_vertex_array > 0 && model_data->text_coord_array && model_data->offsetFrames[frame].vertex_array)
		{
			glTexCoordPointer(2,GL_FLOAT,0,model_data->text_coord_array);
			glVertexPointer(3,GL_FLOAT,0,model_data->offsetFrames[frame].vertex_array);

			//check_gl_errors();
			if(have_compiled_vertex_array)ELglLockArraysEXT(0, numFaces*3);
			glDrawArrays(GL_TRIANGLES, 0, numFaces*3);
			if(have_compiled_vertex_array)ELglUnlockArraysEXT();
		}
	else
		{
			int i;
			text_coord_md2 *offsetTexCoords;
			face_md2 *offsetFaces;
			vertex_md2 *vertex_pointer=NULL;
			//setup
			glBegin(GL_TRIANGLES);
			vertex_pointer=model_data->offsetFrames[frame].vertex_pointer;
			offsetFaces=model_data->offsetFaces;
			offsetTexCoords=model_data->offsetTexCoords;
			//draw each triangle
			if(have_multitexture)
				for(i=0;i<numFaces;i++)
					{
						float x,y,z;

						ELglMultiTexCoord2fARB(base_unit,offsetTexCoords[offsetFaces[i].at].u,offsetTexCoords[offsetFaces[i].at].v);
						x=vertex_pointer[offsetFaces[i].a].x;
	       					y=vertex_pointer[offsetFaces[i].a].y;
						z=vertex_pointer[offsetFaces[i].a].z;
						glVertex3f(x,y,z);

						ELglMultiTexCoord2fARB(base_unit,offsetTexCoords[offsetFaces[i].bt].u,offsetTexCoords[offsetFaces[i].bt].v);
						x=vertex_pointer[offsetFaces[i].b].x;
						y=vertex_pointer[offsetFaces[i].b].y;
						z=vertex_pointer[offsetFaces[i].b].z;
						glVertex3f(x,y,z);

						ELglMultiTexCoord2fARB(base_unit,offsetTexCoords[offsetFaces[i].ct].u,offsetTexCoords[offsetFaces[i].ct].v);
						x=vertex_pointer[offsetFaces[i].c].x;
						y=vertex_pointer[offsetFaces[i].c].y;
						z=vertex_pointer[offsetFaces[i].c].z;
						glVertex3f(x,y,z);
					}
			else
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
		}
	check_gl_errors();
}


void draw_actor(actor * actor_id)
{
	int i;
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
	i=get_frame_number(actor_id->model_data, actor_id->cur_frame);
	if(i >= 0)healtbar_z=actor_id->model_data->offsetFrames[i].box.max_z;

	glPushMatrix();//we don't want to affect the rest of the scene
	x_pos=actor_id->x_pos;
	y_pos=actor_id->y_pos;
	z_pos=actor_id->z_pos;

	if(z_pos==0.0f)//actor is walking, as opposed to flying, get the height underneath
		z_pos=-2.2f+height_map[actor_id->y_tile_pos*tile_map_size_x*6+actor_id->x_tile_pos]*0.2f;

	glTranslatef(x_pos+0.25f, y_pos+0.25f, z_pos);

	x_rot=actor_id->x_rot;
	y_rot=actor_id->y_rot;
	z_rot=-actor_id->z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);

#ifdef CAL3D
	render_cal3d_model(actor_id);
#else
	draw_model(actor_id->model_data, actor_id->cur_frame, actor_id->ghost);
#endif

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
	ELglActiveTextureARB(base_unit);
	glEnable(GL_TEXTURE_2D);
	ELglClientActiveTextureARB(base_unit);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	lock_actors_lists();	//lock it to avoid timing issues
	//display only the non ghosts
	for(i=0;i<max_actors;i++)
		{
			actor *cur_actor= actors_list[i];
			if(cur_actor) {
				if(!cur_actor->ghost)
					{
						int dist1;
						int dist2;

						dist1=x-cur_actor->x_pos;
						dist2=y-cur_actor->y_pos;
						if(dist1*dist1+dist2*dist2<=12*12)
							{
								if(cur_actor->is_enhanced_model)
									{
										draw_enhanced_actor(cur_actor);
										//check for network data - reduces resyncs
										get_message_from_server();
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

							dist1=x-cur_actor->x_pos;
							dist2=y-cur_actor->y_pos;
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
	unlock_actors_lists();	//unlock it since we are done

	glDisable(GL_BLEND);
	ELglClientActiveTextureARB(base_unit);
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
	//find out if there is another actor with that ID
	//ideally this shouldn't happen, but just in case
	lock_actors_lists();	//lock it to avoid timing issues
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
			return;
		}
	my_strncp(actors_list[i]->actor_name,&in_data[23],30);
	unlock_actors_lists();	//unlock it
}


//this actor will be resized. We want speed, so that's why we add a different function
void draw_interface_actor(actor * actor_id,float scale,int x_pos,int y_pos,
						  int z_pos, float x_rot,float y_rot, float z_rot)
{
	int texture_id;
	//frame_md2 *offsetFrames;

	//offsetFrames=actor_id->body_parts->head->offsetFrames;
	texture_id=actor_id->texture_id;

	x_rot+=180;//test
	z_rot+=180;//test

	bind_texture_id(texture_id);

	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef(x_pos, y_pos, z_pos);
	z_rot=-z_rot;
	glRotatef(z_rot, 0.0f, 0.0f, 1.0f);
	glRotatef(x_rot, 1.0f, 0.0f, 0.0f);
	glRotatef(y_rot, 0.0f, 1.0f, 0.0f);
	glScalef(scale,scale,scale);	//enlarge the actor

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	if(actor_id->body_parts->legs)draw_model(actor_id->body_parts->legs,"idle01",0);
	if(actor_id->body_parts->torso)draw_model(actor_id->body_parts->torso,"idle01",0);
	if(actor_id->body_parts->head)draw_model(actor_id->body_parts->head,"idle01",0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

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

	memset(our_actor->current_displayed_text, 0, max_current_displayed_text_len);
	our_actor->current_displayed_text_time_left =  0;

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
					sprintf(str,"%s: %s: %s\n",reg_error_str,error_body_part,this_actor->legs_fn);
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
					sprintf(str,"%s: %s: %s\n",reg_error_str,error_body_part,this_actor->head_fn);
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
					sprintf(str,"%s: %s: %s\n",reg_error_str,error_body_part,this_actor->torso_fn);
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


//--- LoganDugenoux [5/25/2004]
#define ms_per_char	200
#define mini_bubble_ms	500
void	add_displayed_text_to_actor( actor * actor_ptr, const char* text )
{
	int len_to_add;
	len_to_add = strlen(text);
	strncpy( actor_ptr->current_displayed_text, text, max_current_displayed_text_len-1 );
	actor_ptr->current_displayed_text[max_current_displayed_text_len-1] = 0;
	actor_ptr->current_displayed_text_time_left = len_to_add*ms_per_char;

	actor_ptr->current_displayed_text_time_left += mini_bubble_ms;
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

#include <stdlib.h>
#include <math.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void draw_test_light();
 */

typedef struct
{
	float x;
	float y;
	float z;
	float w;
}sun;

GLfloat global_lights[GLOBAL_LIGHTS_NO][4];

GLfloat sky_lights_c1[GLOBAL_LIGHTS_NO*2][4];
GLfloat sky_lights_c2[GLOBAL_LIGHTS_NO*2][4];
GLfloat sky_lights_c3[GLOBAL_LIGHTS_NO*2][4];
GLfloat sky_lights_c4[GLOBAL_LIGHTS_NO*2][4];

int	show_lights;
#ifndef	NEW_FRUSTUM
GLfloat light_0_position[4];
GLfloat light_0_diffuse[4];
GLfloat light_0_dist;

GLfloat light_1_position[4];
GLfloat light_1_diffuse[4];
GLfloat light_1_dist;

GLfloat light_2_position[4];
GLfloat light_2_diffuse[4];
GLfloat light_2_dist;

GLfloat light_3_position[4];
GLfloat light_3_diffuse[4];
GLfloat light_3_dist;

GLfloat light_4_position[4];
GLfloat light_4_diffuse[4];
GLfloat light_4_dist;

GLfloat light_5_position[4];
GLfloat light_5_diffuse[4];
GLfloat light_5_dist;

GLfloat light_6_position[4];
GLfloat light_6_diffuse[4];
GLfloat light_6_dist;
#endif

int	num_lights;	// the highest light number loaded
light *lights_list[MAX_LIGHTS];
unsigned char light_level=58;
sun sun_pos[60*3];
short game_minute=60;

int test_point_visible(float x,float y,float z)
{
	double MV[16];
	double PROJ[16];
	int viewp[4];
	double winx,winy,winz;
	float z_value;

	glGetDoublev(GL_MODELVIEW_MATRIX,&MV[0]);
	glGetDoublev(GL_PROJECTION_MATRIX,&PROJ[0]);
	glGetIntegerv(GL_VIEWPORT,&viewp[0]);

	gluProject(x,y,z,&MV[0],&PROJ[0],&viewp[0],&winx,&winy,&winz);
	glReadPixels(winx,winy,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&z_value);

	if (winz<z_value)
		return 1;
	else 	return 0;
}

#ifndef	NEW_FRUSTUM
void render_corona(float x,float y,float z,float r,float g,float b)
{
	float i;
	int res;

	res=0;
	for (i=-0.1;i<0.1;i=i+0.2) {
		if (test_point_visible(x,y,z+i)) res=1;
	}

	if (!res) return;

	glColor3f(r,g,b);
	glPushMatrix();
	glTranslatef(x,y,z);
	glRotatef(-rz, 0.0f, 0.0f, 1.0f);
	glRotatef(-rx, 1.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);glVertex3f(-2,2,0);
	glTexCoord2f(1,0);glVertex3f(2,2,0);
	glTexCoord2f(1,1);glVertex3f(2,-2,0);
	glTexCoord2f(0,1);glVertex3f(-2,-2,0);
	glEnd();
	glPopMatrix();
}

void render_coronas()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR,GL_ONE);
	get_and_set_texture_id(particle_textures[0]);

	render_corona(light_0_position[0],light_0_position[1],light_0_position[2],1,1,1);
	render_corona(light_1_position[0],light_1_position[1],light_1_position[2],1,1,1);
	render_corona(light_2_position[0],light_2_position[1],light_2_position[2],1,1,1);
	render_corona(light_3_position[0],light_3_position[1],light_3_position[2],1,1,1);
	render_corona(light_4_position[0],light_4_position[1],light_4_position[2],1,1,1);
	render_corona(light_5_position[0],light_5_position[1],light_5_position[2],1,1,1);
	render_corona(light_6_position[0],light_6_position[1],light_6_position[2],1,1,1);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}
#endif


int	max_enabled;
void disable_local_lights()
{
	max_enabled= -1;
	
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4);
    glDisable(GL_LIGHT5);
    glDisable(GL_LIGHT6);
}

void enable_local_lights()
{
	if(show_lights < 0) return;
	if (show_lights != max_enabled)	max_enabled= show_lights;
    glEnable(GL_LIGHT0);
    if(show_lights >= 1)	glEnable(GL_LIGHT1);
    if(show_lights >= 2)	glEnable(GL_LIGHT2);
    if(show_lights >= 3)	glEnable(GL_LIGHT3);
    if(show_lights >= 4)	glEnable(GL_LIGHT4);
    if(show_lights >= 5)	glEnable(GL_LIGHT5);
    if(show_lights >= 6)	glEnable(GL_LIGHT6);
}


void draw_lights()
{
#ifdef	NEW_FRUSTUM
	unsigned int i, j, l, start, stop;
	float vec4[0];
#else
	GLfloat spot_direction[] = { -0.0, -0.0, -0.0f };
#endif
	
	if(show_lights <0){
		if(max_enabled >= 0){
			disable_local_lights();
		}
		return;
	} else if(max_enabled != show_lights){
        enable_local_lights();
	}
	if(max_enabled >= 0 && show_lights != max_enabled)	enable_local_lights();
	
#ifndef	NEW_FRUSTUM
	glLightfv(GL_LIGHT0, GL_POSITION, light_0_position);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light_0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);

	if(show_lights >= 1){
		glLightfv(GL_LIGHT1, GL_POSITION, light_1_position);
		glLightfv(GL_LIGHT1,GL_DIFFUSE,light_1_diffuse);
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
	}
	if(show_lights >= 2){
		glLightfv(GL_LIGHT2, GL_POSITION, light_2_position);
		glLightfv(GL_LIGHT2,GL_DIFFUSE,light_2_diffuse);
		glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_direction);
	}
	if(show_lights >= 3){
		glLightfv(GL_LIGHT3, GL_POSITION, light_3_position);
		glLightfv(GL_LIGHT3,GL_DIFFUSE,light_3_diffuse);
		glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, spot_direction);
	}
	if(show_lights >= 4){
		glLightfv(GL_LIGHT4, GL_POSITION, light_4_position);
		glLightfv(GL_LIGHT4,GL_DIFFUSE,light_4_diffuse);
		glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, spot_direction);
	}
	if(show_lights >= 5){
		glLightfv(GL_LIGHT5, GL_POSITION, light_5_position);
		glLightfv(GL_LIGHT5,GL_DIFFUSE,light_5_diffuse);
		glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, spot_direction);
	}
	if(show_lights >= 6){
		glLightfv(GL_LIGHT6, GL_POSITION, light_6_position);
		glLightfv(GL_LIGHT6,GL_DIFFUSE,light_6_diffuse);
		glLightfv(GL_LIGHT6, GL_SPOT_DIRECTION, spot_direction);
	}
#else	
	j = 0;
	
	get_intersect_start_stop(main_bbox_tree, TYPE_LIGHT, &start, &stop);
	for (i = start; i < stop; i++)
	{
		l = get_intersect_item_ID(main_bbox_tree, i);
#ifdef EXTRA_DEBUG
		if (!lights_list[l])
		{
			ERR();
			continue;
		}
#endif
		vec4[0] = lights_list[l]->pos_x;
		vec4[1] = lights_list[l]->pos_y;
		vec4[2] = lights_list[l]->pos_z;
		vec4[3] = 1.0f;
		glLightfv(GL_LIGHT0+j, GL_POSITION, vec4);
		vec4[0] = lights_list[l]->r;
		vec4[1] = lights_list[l]->g;
		vec4[2] = lights_list[l]->b;
		vec4[3] = 1.0f;
		glLightfv(GL_LIGHT0+j, GL_DIFFUSE, vec4);
		if (j >= 6) break;
		else j++;
	}

#endif
}

#ifdef	NEW_FRUSTUM
void destroy_light(int i)
{
	if ((i < 0) || (i >= MAX_LIGHTS)) return;
	if (lights_list[i] == NULL) return;
	delete_light_from_abt(main_bbox_tree, i);
	free(lights_list[i]);
	lights_list[i] = NULL;
}
#endif

#ifdef	NEW_FRUSTUM
#if defined (MAP_EDITOR2) || defined (MAP_EDITOR)
int add_light(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat intensity, int locked, unsigned int dynamic)
#else
int add_light(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat intensity, unsigned int dynamic)
#endif
#else
#if defined (MAP_EDITOR2) || defined (MAP_EDITOR)
int add_light(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat intensity, int locked)
#else
int add_light(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat intensity)
#endif
#endif
{
	int i;
	light *new_light;
#ifdef	NEW_FRUSTUM
	AABBOX bbox;
#endif

	//find a free spot, in the lights list
	for (i = 0; i < MAX_LIGHTS; i++)
	{
		if (lights_list[i] == NULL)
			break;
	}
	
	if (i >= MAX_LIGHTS)
		// oops no way to store the new light
		return i;
		
	new_light = calloc(1, sizeof(light));

	new_light->pos_x= x;
	new_light->pos_y= y;
	new_light->pos_z= z;

	new_light->r= r*intensity;
	new_light->g= g*intensity;
	new_light->b= b*intensity;

#ifdef MAP_EDITOR2
	new_light->locked=locked;
#endif
	
	lights_list[i] = new_light;
	if (i >= num_lights) num_lights = i+1;	
#ifdef	NEW_FRUSTUM
	calc_light_aabb(&bbox, x, y, z, r*intensity, g*intensity, b*intensity, 1.41f, 1.0f, 0.05f);
	if ((main_bbox_tree_items != NULL) && (dynamic == 0)) add_light_to_list(main_bbox_tree_items, i, &bbox);
	else add_light_to_abt(main_bbox_tree, i, &bbox, dynamic);
#endif

	return i;
}

void cleanup_lights(void)
{
	int i;
	for(i = 0; i < MAX_LIGHTS; i++) {
		if(lights_list[i] != NULL) {
			free(lights_list[i]);
		}
	}
}

//get the lights visible in the scene
//should be called only when we change the camera pos
void update_scene_lights()
{
#ifndef	NEW_FRUSTUM
	int i;
	float x,y;
	float x_dist,y_dist,dist;
	char all_full=0;
	char max_changed=0;
	float max_dist=0;
	int max_light=0;

	show_lights= max_light= -1;
	x= -cx;
	y= -cy;
	//reset the lights
	light_0_dist= light_1_dist= light_2_dist= light_3_dist= light_4_dist= light_5_dist= light_6_dist= 60.0*60.0;

	light_0_diffuse[0]=0;light_0_diffuse[1]=0;light_0_diffuse[2]=0;light_0_diffuse[3]=1.0;

	light_1_diffuse[0]=0;light_1_diffuse[1]=0;light_1_diffuse[2]=0;light_1_diffuse[3]=1.0;

	light_2_diffuse[0]=0;light_2_diffuse[1]=0;light_2_diffuse[2]=0;light_2_diffuse[3]=1.0;

	light_3_diffuse[0]=0;light_3_diffuse[1]=0;light_3_diffuse[2]=0;light_3_diffuse[3]=1.0;

	light_4_diffuse[0]=0;light_4_diffuse[1]=0;light_4_diffuse[2]=0;light_4_diffuse[3]=1.0;

	light_5_diffuse[0]=0;light_5_diffuse[1]=0;light_5_diffuse[2]=0;light_5_diffuse[3]=1.0;

	light_6_diffuse[0]=0;light_6_diffuse[1]=0;light_6_diffuse[2]=0;light_6_diffuse[3]=1.0;

	for(i=0;i<MAX_LIGHTS;i++)
		{
			if(lights_list[i])
				{
					// is the light is close enough to worry about?
					x_dist= x-lights_list[i]->pos_x;
					y_dist= y-lights_list[i]->pos_y;
					dist= x_dist*x_dist+y_dist*y_dist;
					if(dist<30.0*30.0)
						{
							if(max_changed)	{
								max_dist=0;
								max_changed=0;
								if(light_0_dist>max_dist)
									{
										max_dist=light_0_dist;
										max_light=0;
									}
								if(light_1_dist>max_dist)
									{
										max_dist=light_1_dist;
										max_light=1;
									}
								if(light_2_dist>max_dist)
									{
										max_dist=light_2_dist;
										max_light=2;
									}
								if(light_3_dist>max_dist)
									{
										max_dist=light_3_dist;
										max_light=3;
									}
								if(light_4_dist>max_dist)
									{
										max_dist=light_4_dist;
										max_light=4;
									}
								if(light_5_dist>max_dist)
									{
										max_dist=light_5_dist;
										max_light=5;
									}
								if(light_6_dist>max_dist)
									{
										max_dist=light_6_dist;
										max_light=6;
									}
							}
							// we have all the lights and we are farther, next light
							if(all_full && dist > max_dist)	continue;
							
							if((light_0_dist>=50.0*50.0) || (all_full && (max_light==0)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;
									else show_lights= 0;
									
									light_0_position[0]=lights_list[i]->pos_x;
									light_0_position[1]=lights_list[i]->pos_y;
									light_0_position[2]=lights_list[i]->pos_z;
									light_0_position[3]=1.0;

									light_0_diffuse[0]=lights_list[i]->r;
									light_0_diffuse[1]=lights_list[i]->g;
									light_0_diffuse[2]=lights_list[i]->b;
									light_0_diffuse[3]=1.0;
									light_0_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=0;
										}
									continue;
								}
							if((light_1_dist>=50.0*50.0) || (all_full && (max_light==1)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;
									else show_lights= 1;

									light_1_position[0]=lights_list[i]->pos_x;
									light_1_position[1]=lights_list[i]->pos_y;
									light_1_position[2]=lights_list[i]->pos_z;
									light_1_position[3]=1.0;

									light_1_diffuse[0]=lights_list[i]->r;
									light_1_diffuse[1]=lights_list[i]->g;
									light_1_diffuse[2]=lights_list[i]->b;
									light_1_diffuse[3]=1.0;
									light_1_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=1;
										}
									continue;
								}
							if((light_2_dist>=50.0*50.0) || (all_full && (max_light==2)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;
									else show_lights= 2;

									light_2_position[0]=lights_list[i]->pos_x;
									light_2_position[1]=lights_list[i]->pos_y;
									light_2_position[2]=lights_list[i]->pos_z;
									light_2_position[3]=1.0;

									light_2_diffuse[0]=lights_list[i]->r;
									light_2_diffuse[1]=lights_list[i]->g;
									light_2_diffuse[2]=lights_list[i]->b;
									light_2_diffuse[3]=1.0;
									light_2_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=2;
										}
									continue;
								}
							if((light_3_dist>=50.0*50.0) || (all_full && (max_light==3)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;
									else show_lights= 3;

									light_3_position[0]=lights_list[i]->pos_x;
									light_3_position[1]=lights_list[i]->pos_y;
									light_3_position[2]=lights_list[i]->pos_z;
									light_3_position[3]=1.0;

									light_3_diffuse[0]=lights_list[i]->r;
									light_3_diffuse[1]=lights_list[i]->g;
									light_3_diffuse[2]=lights_list[i]->b;
									light_3_diffuse[3]=1.0;
									light_3_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=3;
										}
									continue;
								}
							if((light_4_dist>=50.0*50.0) || (all_full && (max_light==4)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;
									else show_lights= 4;

									light_4_position[0]=lights_list[i]->pos_x;
									light_4_position[1]=lights_list[i]->pos_y;
									light_4_position[2]=lights_list[i]->pos_z;
									light_4_position[3]=1.0;

									light_4_diffuse[0]=lights_list[i]->r;
									light_4_diffuse[1]=lights_list[i]->g;
									light_4_diffuse[2]=lights_list[i]->b;
									light_4_diffuse[3]=1.0;
									light_4_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=4;
										}
									continue;
								}
							if((light_5_dist>=50.0*50.0) || (all_full && (max_light==5)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;
									else show_lights= 5;

									light_5_position[0]=lights_list[i]->pos_x;
									light_5_position[1]=lights_list[i]->pos_y;
									light_5_position[2]=lights_list[i]->pos_z;
									light_5_position[3]=1.0;

									light_5_diffuse[0]=lights_list[i]->r;
									light_5_diffuse[1]=lights_list[i]->g;
									light_5_diffuse[2]=lights_list[i]->b;
									light_5_diffuse[3]=1.0;
									light_5_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=5;
										}
									continue;
								}
							if((light_6_dist>=50.0*50.0) || (all_full && (max_light==6)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;
									else show_lights= 6;

									light_6_position[0]=lights_list[i]->pos_x;
									light_6_position[1]=lights_list[i]->pos_y;
									light_6_position[2]=lights_list[i]->pos_z;
									light_6_position[3]=1.0;

									light_6_diffuse[0]=lights_list[i]->r;
									light_6_diffuse[1]=lights_list[i]->g;
									light_6_diffuse[2]=lights_list[i]->b;
									light_6_diffuse[3]=1.0;
									light_6_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=6;
										}
									all_full=1;
									continue;
								}

						}
				}
		}
#else
	unsigned int start, stop;

	get_intersect_start_stop(main_bbox_tree, TYPE_LIGHT, &start, &stop);
	show_lights = min2i(6, stop - start -1);
#endif
}

void init_lights()
{

	GLfloat light_diffuse[] = { 1.7, 1.3, 1.1, 0.0 };
	GLfloat no_light[] = { 0.0, 0.0, 0.0, 0.0 };
	float linear_att=1.41f;
	float cut_off=180;
	//most of the things in here are redundant, since we kind of set the light sources
	//to their default values. However, better safe than sorry.

	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT0,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT0,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT0);

	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT1,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT1,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT1,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT1);

	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT2,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT2,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT2,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT2,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT2);

	glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT3,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT3,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT3,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT3,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT3);

	glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT4,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT4,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT4,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT4,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT4);

	glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT5,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT5,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT5,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT5,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT5);

	glLightf(GL_LIGHT6, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT6,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT6,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT6,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT6,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT6);

	glLightfv(GL_LIGHT7,GL_AMBIENT,no_light);
	glLightfv(GL_LIGHT7,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT7,GL_DIFFUSE,no_light);
	glLightf(GL_LIGHT7,GL_CONSTANT_ATTENUATION,0);
	glEnable(GL_LIGHT7);


	glEnable(GL_LIGHTING);

	glNormal3f(0.0f,0.0f,1.0f);
}


void reset_material()
{
	GLfloat mat_emission[]={ 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_specular[]={ 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_ambient[]={ 1.0, 1.0, 1.0, 1.0 };

	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ambient);

}

void set_material(float r, float g, float b)
{
	GLfloat mat_emission[]={ r, g, b, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_emission);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_emission);
}

int sun_use_static_position=0;
GLfloat sun_ambient_light[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat difuse_light[] = { 0.0, 0.0, 0.0, 0.0 };
void draw_global_light()
{
	int i;
	GLfloat global_light_position[] = { 400.0, 400.0, 500.0, 0.0 };
	i=light_level;
	if(light_level>59)i=119-light_level;
#ifdef NEW_WEATHER
	if(i<0)i=0;
#else
	//this is for weather things, when the light level is not the normal light lvel of the current time
#ifndef MAP_EDITOR2
	i+=weather_light_offset;
#endif
	if(i<0)i=0;
	if(i>59)i=59;
#endif
	//add the thunder light to the ambient/difuse light

#ifdef NEW_WEATHER
	difuse_light[0] = weather_bias_light(global_lights[i][0] - 0.15f);
	difuse_light[1] = weather_bias_light(global_lights[i][1] - 0.15f);
	difuse_light[2] = weather_bias_light(global_lights[i][2] - 0.15f);
#else
#ifndef MAP_EDITOR2
	difuse_light[0]=global_lights[i][0]+(float)thunder_light_offset/90-0.15f;
	difuse_light[1]=global_lights[i][1]+(float)thunder_light_offset/60-0.15f;
	difuse_light[2]=global_lights[i][2]+(float)thunder_light_offset/15-0.15f;
#else
	difuse_light[0]=global_lights[i][0];
	difuse_light[1]=global_lights[i][1];
	difuse_light[2]=global_lights[i][2];
#endif
#endif

	if(map_type==2)
		{
			//the ambient light should be almost as the normal light, but a little bluer
			sun_ambient_light[0]=difuse_light[0]+.01f;
			sun_ambient_light[1]=difuse_light[1]+.01f;
			sun_ambient_light[2]=difuse_light[2];
		}
		else
		{
			//the ambient light should be half of the difuse light
			sun_ambient_light[0]=difuse_light[0]/3.5f+0.15f;
			sun_ambient_light[1]=difuse_light[1]/3.5f+0.15f;
			sun_ambient_light[2]=difuse_light[2]/3.5f+0.15f;
		}


	sun_ambient_light[3]=1.0f;
	glLightfv(GL_LIGHT7,GL_AMBIENT,sun_ambient_light);
	//We add (0.2,0.2,0.2) because that's the global ambient color, and we need it for shadows
	sun_ambient_light[0]+=0.2f;
	sun_ambient_light[1]+=0.2f;
	sun_ambient_light[2]+=0.2f;
	if(sun_use_static_position)glLightfv(GL_LIGHT7,GL_POSITION,global_light_position);
	else glLightfv(GL_LIGHT7,GL_POSITION,sun_position);
	glLightfv(GL_LIGHT7,GL_DIFFUSE,&difuse_light[0]);
}


void draw_dungeon_light()
{
	GLfloat global_light_position[] = { 400.0, 400.0, 500.0, 0.0 };
	GLfloat difuse_light[] = { 0.0, 0.0, 0.0, 0.0 };
	GLfloat ambient_light[4];

	//the ambient light should be half of the difuse light
	ambient_light[0]=ambient_r;
	ambient_light[1]=ambient_g;
	ambient_light[2]=ambient_b;
	ambient_light[3]=1.0f;
	glLightfv(GL_LIGHT7,GL_AMBIENT,ambient_light);
	glLightfv(GL_LIGHT7, GL_POSITION, global_light_position);
	glLightfv(GL_LIGHT7,GL_DIFFUSE,difuse_light);
}


void make_gradient_light(int start,int steps,float *light_table, float r_start,
						 float g_start, float b_start, float r_end, float g_end, float b_end)
{
	int i,j;
	float r_slope,g_slope,b_slope;

	r_slope=(r_end-r_start)/steps;
	g_slope=(g_end-g_start)/steps;
	b_slope=(b_end-b_start)/steps;

	j=0;
	for(i=start;i<start+steps;i++)
		{
			light_table[i*4+0]=r_start+r_slope*(float)j;
			light_table[i*4+1]=g_start+g_slope*(float)j;
			light_table[i*4+2]=b_start+b_slope*(float)j;
			light_table[i*4+3]=1.0f;
			j++;

		}
}


//build the light table for smooth transition between night and day
void build_global_light_table()
{
	//the sun light
  	make_gradient_light(0,30,(float *)global_lights,0.85f,0.85f,0.85f,0.32f,0.25f,0.25f);
	make_gradient_light(30,30,(float *)global_lights,0.318f,0.248f,0.248f,0.05f,0.05f,0.08f);
	//lake light
	make_gradient_light(0,30,(float *)sky_lights_c1,0.0f,0.3f,0.6f,0.6f,0.3f,0.0f);
	make_gradient_light(30,30,(float *)sky_lights_c1,0.6f,0.3f,0.0f,0.0f,0.01f,0.1f);
	make_gradient_light(60,30,(float *)sky_lights_c1,0.0f,0.1f,0.1f,0.6f,0.3f,0.3f);
	make_gradient_light(90,30,(float *)sky_lights_c1,0.6f,0.3f,0.3f,0.1f,0.3f,0.6f);

	make_gradient_light(0,30,(float *)sky_lights_c2,0.0f,0.4f,0.6f,0.6f,0.4f,0.0f);
	make_gradient_light(30,30,(float *)sky_lights_c2,0.6f,0.4f,0.0f,0.0f,0.1f,0.1f);
	make_gradient_light(60,30,(float *)sky_lights_c2,0.0f,0.1f,0.1f,0.6f,0.2f,0.1f);
	make_gradient_light(90,30,(float *)sky_lights_c2,0.6f,0.2f,0.1f,0.0f,0.4f,0.6f);

	make_gradient_light(0,30,(float *)sky_lights_c3,0.0f,0.7f,0.9f,0.9f,0.7f,0.0f);
	make_gradient_light(30,30,(float *)sky_lights_c3,0.9f,0.9f,0.0f,0.0f,0.1f,0.1f);
	make_gradient_light(60,30,(float *)sky_lights_c3,0.0f,0.1f,0.1f,0.5f,0.4f,0.4f);
	make_gradient_light(90,30,(float *)sky_lights_c3,0.5f,0.4f,0.4f,0.0f,0.7f,0.9f);


	make_gradient_light(0,30,(float *)sky_lights_c4,0.2f,0.8f,1.0f,1.0f,0.8f,0.2f);
	make_gradient_light(30,30,(float *)sky_lights_c4,1.0f,0.8f,0.2f,0.0f,0.1f,0.1f);
	make_gradient_light(60,30,(float *)sky_lights_c4,0.0f,0.1f,0.1f,0.7f,0.4f,0.5f);
	make_gradient_light(90,30,(float *)sky_lights_c4,0.7f,0.4f,0.5f,0.2f,0.8f,1.0f);

}

void build_sun_pos_table()
{
	float x,y,z,d;
	int i;
	int start=60;

	x=0;
	d=400;
	for(i=0;i<60*3;i++)
		{
			z = d*sin((float)(i+start)*0.6f*M_PI/180.0f);
			y = d*cos((float)(i+start)*0.6f*M_PI/180.0f);
			x+=0.5f;

			sun_pos[i].x=x;
			sun_pos[i].y=y;
			sun_pos[i].z=z;
			sun_pos[i].w=0.0f;
		}
}

void new_minute()
{
#ifdef EXTRA_DEBUG
	ERR();
#endif
	//morning starts at 0
	//game_minute=90;
	//is it morning?
	if(game_minute<60)light_level=game_minute+60;
	//check to see if it is full day
	if(game_minute>=60 && game_minute<60*3)light_level=0;
	//is it evening?
	if(game_minute>=60*3 && game_minute<60*4)light_level=game_minute-60*3;
	//full night?
	if(game_minute>=60*4)light_level=59;

	//is it day?
	if(game_minute>=30 && game_minute<60*3+30 && !dungeon)
		{
			disable_local_lights();
			is_day=1;
			sun_position[0]=sun_pos[game_minute-30].x;
			sun_position[1]=sun_pos[game_minute-30].y;
			sun_position[2]=sun_pos[game_minute-30].z;
			sun_position[3]=sun_pos[game_minute-30].w;
			calc_shadow_matrix();
		}
	else//it's too dark, or we are in a dungeon
		{
			is_day=0;
			enable_local_lights();
	    	sun_position[0]=sun_position[1]=sun_position[2]=0.0;
		}

}

/* currently UNUSED
void draw_test_light()
{
	GLfloat light_position[] = { 15.0, 15.0, 3.0, 1.0 };
	GLfloat light_position_2[] = { 5.0, 5.0, -3.0, 1.0 };
	GLfloat spot_direction[] = { 0.0, 0.0, -1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);

	glLightfv(GL_LIGHT1, GL_POSITION, light_position_2);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
}
*/

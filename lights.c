#include <stdlib.h>
#include <math.h>
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif
#ifdef	EYE_CANDY
#include "eye_candy_wrapper.h"
#endif	//EYE_CANDY

#ifdef NEW_LIGHTING
int last_texture_start = 0;
int last_dungeon;
int use_new_lighting = 0;
int night_shift_textures = 0;
int old_night_shift_textures = 0;
#endif // NEW_LIGHTING

#ifdef DEBUG_TIME
const float debug_time_accel = 120.0f;
#endif

#if defined(NEW_LIGHTING) || defined(DEBUG_TIME)
Uint64 old_time = 0;
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
#ifdef SKY_FPV_CURSOR
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
#endif /* SKY_FPV_CURSOR */

int	num_lights;	// the highest light number loaded
light *lights_list[MAX_LIGHTS];
unsigned char light_level=58;
#ifdef SKY_FPV_CURSOR
sun sun_pos[360];
sun sun_show[60*6];
#elif defined(NEW_LIGHTING)
sun sun_pos[60*6];
#else 
sun sun_pos[60*3];
#endif

short game_minute=0;

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
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	if (winz<z_value)
		return 1;
	else 	return 0;
}

int	max_enabled;
void disable_local_lights()
{
	max_enabled= -1;
	
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
#ifndef EYE_CANDY
    glDisable(GL_LIGHT4);
    glDisable(GL_LIGHT5);
    glDisable(GL_LIGHT6);
#endif
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void enable_local_lights()
{
	if(show_lights < 0) return;
	if (show_lights != max_enabled)	max_enabled= show_lights;
    glEnable(GL_LIGHT0);
    if(show_lights >= 1)	glEnable(GL_LIGHT1);
    if(show_lights >= 2)	glEnable(GL_LIGHT2);
    if(show_lights >= 3)	glEnable(GL_LIGHT3);
#ifndef EYE_CANDY
    if(show_lights >= 4)	glEnable(GL_LIGHT4);
    if(show_lights >= 5)	glEnable(GL_LIGHT5);
    if(show_lights >= 6)	glEnable(GL_LIGHT6);
#endif
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_lights()
{
	unsigned int i, j, l, start, stop;
	VECTOR4 vec4;
	
	if(show_lights <0){
		if(max_enabled >= 0){
			disable_local_lights();
		}
		return;
	} else if(max_enabled != show_lights){
        enable_local_lights();
	}
	if(max_enabled >= 0 && show_lights != max_enabled)	enable_local_lights();
	
	j= 0;
	
	get_intersect_start_stop(main_bbox_tree, TYPE_LIGHT, &start, &stop);
	for(i=start; i<stop; i++)
	{
		l= get_intersect_item_ID(main_bbox_tree, i);
		// and make sure it's a valid light
		if(l<0 || l>MAX_LIGHTS || !lights_list[l])
		{
#ifdef EXTRA_DEBUG
			ERR();
#endif
			continue;
		}
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
#ifndef EYE_CANDY
		if (j >= 6) break;
#else
		if (j >= 4) break;
#endif
		else j++;
	}

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void destroy_light(int i)
{
	if((i < 0) || (i >= MAX_LIGHTS)) return;
	if(lights_list[i] == NULL) return;
	delete_light_from_abt(main_bbox_tree, i);
	free(lights_list[i]);
	lights_list[i]= NULL;
}

#if defined (MAP_EDITOR2) || defined (MAP_EDITOR)
int add_light(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat intensity, int locked, unsigned int dynamic)
#else
int add_light(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat intensity, unsigned int dynamic)
#endif
{
	int i;
	light *new_light;
	AABBOX bbox;

	//find a free spot, in the lights list
	for(i=0; i<MAX_LIGHTS; i++)
	{
		if(lights_list[i] == NULL)
			break;
	}
	
	if(i >= MAX_LIGHTS)
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
	calc_light_aabb(&bbox, x, y, z, r*intensity, g*intensity, b*intensity, 1.41f, 1.0f, 0.004f); // 0.004 ~ 1/256
	if ((main_bbox_tree_items != NULL) && (dynamic == 0)) add_light_to_list(main_bbox_tree_items, i, bbox);
	else add_light_to_abt(main_bbox_tree, i, bbox, dynamic);

	return i;
}

void cleanup_lights(void)
{
	int i;
	
	for(i = 0; i < MAX_LIGHTS; i++) {
		if(lights_list[i] != NULL) {
			free(lights_list[i]);
			lights_list[i]= NULL;
		}
	}
}

//get the lights visible in the scene
//should be called only when we change the camera pos
void update_scene_lights()
{
	unsigned int start, stop;

	get_intersect_start_stop(main_bbox_tree, TYPE_LIGHT, &start, &stop);
	show_lights = min2i(6, stop - start -1);
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

#ifndef EYE_CANDY
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
	glLightf(GL_LIGHT6, GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT6);
#endif
	glLightfv(GL_LIGHT7,GL_AMBIENT,no_light);
	glLightfv(GL_LIGHT7,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT7,GL_DIFFUSE,no_light);
	glLightf(GL_LIGHT7,GL_CONSTANT_ATTENUATION,0);
	glEnable(GL_LIGHT7);

#ifdef	EYE_CANDY
	ec_add_light(GL_LIGHT4);
	ec_add_light(GL_LIGHT5);
	ec_add_light(GL_LIGHT6);
#endif	//EYE_CANDY

	glEnable(GL_LIGHTING);

	glNormal3f(0.0f,0.0f,1.0f);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


void reset_material()
{
	GLfloat mat_emission[]={ 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_specular[]={ 1.0, 1.0, 1.0, 1.0 };
#ifdef NEW_LIGHTING
	if (use_new_lighting)
	{
		GLfloat mat_ambient[]={ 0.4, 0.4, 0.4, 1.0 };
		GLfloat mat_diffuse[]={ 3.6, 3.6, 3.6, 1.0 };
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	}
	else
	{
		GLfloat mat_ambient[]={ 1.0, 1.0, 1.0, 1.0 };
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ambient);
	}
#else
	GLfloat mat_ambient[]={ 1.0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ambient);
#endif //NEW_LIGHTING

	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void set_material(float r, float g, float b)
{
	GLfloat mat_emission[]={ r, g, b, 1.0 };

	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_emission);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_emission);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
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
 #ifdef NEW_LIGHTING
 	if (use_new_lighting)
 	{
		difuse_light[0] = weather_bias_light(global_lights[i][0]);
		difuse_light[1] = weather_bias_light(global_lights[i][1]);
		difuse_light[2] = weather_bias_light(global_lights[i][2]);
	}
	else
	{
 #endif // NEW_LIGHTING
		difuse_light[0] = weather_bias_light(global_lights[i][0] - 0.15f);
		difuse_light[1] = weather_bias_light(global_lights[i][1] - 0.15f);
		difuse_light[2] = weather_bias_light(global_lights[i][2] - 0.15f);
 #ifdef NEW_LIGHTING
	}
 #endif
#else
#ifndef MAP_EDITOR2
 #ifdef NEW_LIGHTING
 	if (use_new_lighting)
 	{
		difuse_light[0]=global_lights[i][0]+(float)thunder_light_offset/90;
		difuse_light[1]=global_lights[i][1]+(float)thunder_light_offset/60;
		difuse_light[2]=global_lights[i][2]+(float)thunder_light_offset/15;
	}
	else
	{
 #endif // NEW_LIGHTING
		difuse_light[0]=global_lights[i][0]+(float)thunder_light_offset/90-0.15f;
		difuse_light[1]=global_lights[i][1]+(float)thunder_light_offset/60-0.15f;
		difuse_light[2]=global_lights[i][2]+(float)thunder_light_offset/15-0.15f;
 #ifdef NEW_LIGHTING
 	}
 #endif // NEW_LIGHTING
#else
	difuse_light[0]=global_lights[i][0];
	difuse_light[1]=global_lights[i][1];
	difuse_light[2]=global_lights[i][2];
#endif
#endif

//	printf("%f, %f, %f: %f, %f, %f\n", global_lights[i][0], global_lights[i][1], global_lights[i][2], weather_bias_light(global_lights[i][0]), weather_bias_light(global_lights[i][1]), weather_bias_light(global_lights[i][2]));

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

#ifdef NEW_LIGHTING
	if (use_new_lighting)
	{
//		glEnable(GL_LIGHT7);
//		printf("Light: %f, %f, %f\n", difuse_light[0], difuse_light[1], difuse_light[2]);
		difuse_light[0] *= 1.5f;
		difuse_light[1] *= 1.5f;
		difuse_light[2] *= 1.5f;
		sun_ambient_light[0] /= 3.0f;
		sun_ambient_light[1] /= 3.0f;
		sun_ambient_light[2] /= 3.0f;
	}
#endif	//NEW_LIGHTING

	sun_ambient_light[3]=1.0f;
	glLightfv(GL_LIGHT7,GL_AMBIENT,sun_ambient_light);
	//We add (0.2,0.2,0.2) because that's the global ambient color, and we need it for shadows
	sun_ambient_light[0]+=0.2f;
	sun_ambient_light[1]+=0.2f;
	sun_ambient_light[2]+=0.2f;
	if(sun_use_static_position)glLightfv(GL_LIGHT7,GL_POSITION,global_light_position);
	else glLightfv(GL_LIGHT7,GL_POSITION,sun_position);
	glLightfv(GL_LIGHT7,GL_DIFFUSE,&difuse_light[0]);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_dungeon_light()
{
	GLfloat global_light_position[] = { 400.0, 400.0, 500.0, 0.0 };
	GLfloat difuse_light[] = { 0.0, 0.0, 0.0, 0.0 };
	GLfloat ambient_light[4];

#ifdef NEW_LIGHTING
	if (use_new_lighting)
	{
		glEnable(GL_LIGHT7);
		difuse_light[0] = ambient_r / 2.0f;
		difuse_light[1] = ambient_g / 2.0f;
		difuse_light[2] = ambient_b / 2.0f;
		difuse_light[3] = 1.0;
		ambient_light[0] = ambient_r / 12.0f;
		ambient_light[1] = ambient_g / 12.0f;
		ambient_light[2] = ambient_b / 12.0f;
		ambient_light[3] = 1.0;
	}
	else
	{
#endif // NEW_LIGHTING
	//the ambient light should be half of the difuse light
		ambient_light[0]=ambient_r;
		ambient_light[1]=ambient_g;
		ambient_light[2]=ambient_b;
		ambient_light[3]=1.0f;
#ifdef NEW_LIGHTING
	}
#endif // NEW_LIGHTING
	glLightfv(GL_LIGHT7,GL_AMBIENT,ambient_light);
	glLightfv(GL_LIGHT7, GL_POSITION, global_light_position);
	glLightfv(GL_LIGHT7,GL_DIFFUSE,difuse_light);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
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
#ifdef NEW_LIGHTING
	if (use_new_lighting)
	{
		//the sun/moon light
  		make_gradient_light(0,15,(float *)global_lights,0.6f,0.6f,0.6f, 0.7f,0.45f,0.3f);
  		make_gradient_light(14,16,(float *)global_lights,0.7f,0.45f,0.3f, 0.65f,0.4f,0.25f);
  		make_gradient_light(29,16,(float *)global_lights,0.65f,0.4f,0.25f, 0.6f,0.35f,0.15f);
		make_gradient_light(44,16,(float *)global_lights,0.6f,0.35f,0.15f, 0.12f,0.12f,0.15f);
	
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
	else
	{
#endif // NEW_LIGHTING
#ifndef SKY_FPV_CURSOR
		//the sun light
		make_gradient_light(0,30,(float *)global_lights,0.85f,0.85f,0.85f,0.32f,0.25f,0.25f);
		make_gradient_light(30,30,(float *)global_lights,0.318f,0.248f,0.248f,0.06f,0.06f,0.08f);
	
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
#else /* SKY_FPV_CURSOR */
	//the sun light
  	make_gradient_light(0,30,(float *)global_lights,0.85f,0.85f,0.85f,0.32f,0.25f,0.25f);
	make_gradient_light(30,30,(float *)global_lights,0.318f,0.248f,0.248f,0.05f,0.05f,0.08f);
	//lake light
	make_gradient_light(0,30,(float *)sky_lights_c1,
			0.0f,0.7f,0.9f,
			0.6f, 0.0f, 0.0f);
	make_gradient_light(30,30,(float *)sky_lights_c1,
			0.6f, 0.0f, 0.0f,
			0.0f, 0.05f, 0.1f);
	make_gradient_light(60,30,(float *)sky_lights_c1,
			0.0f, 0.05f, 0.1f,
			0.6f, 0.0f, 0.3f);
	make_gradient_light(90,30,(float *)sky_lights_c1,
			0.6f, 0.0f, 0.3f,
			0.0f,0.7f,0.9f);

	make_gradient_light(0,30,(float *)sky_lights_c2,
			0.0f, 0.3f, 0.6f,
			0.6f,0.3f,0.0f);
	make_gradient_light(30,30,(float *)sky_lights_c2,
			0.6f,0.4f,0.0f,
			0.0f,0.025f,0.05f);
	make_gradient_light(60,30,(float *)sky_lights_c2,
			0.0f,0.025f,0.05f,
			0.6f,0.3f,0.0f);
	make_gradient_light(90,30,(float *)sky_lights_c2,
			0.6f,0.3f,0.0f,
			0.0f, 0.4f, 0.6f);

	make_gradient_light(0,30,(float *)sky_lights_c3,
			0.0f,0.3f,0.6f,
			0.2f,0.2f,0.3f);
	make_gradient_light(30,30,(float *)sky_lights_c3,
			0.2f,0.2f,0.3f,
			0.0f,0.01f,0.02f);
	make_gradient_light(60,30,(float *)sky_lights_c3,
			0.0f,0.01f,0.02f,
			0.2f,0.2f,0.3f);
	make_gradient_light(90,30,(float *)sky_lights_c3,
			0.2f,0.2f,0.3f,
			0.0f,0.3f,0.6f);

	make_gradient_light(0,30,(float *)sky_lights_c4,
			0.05f,0.2f,0.6f,
			0.0f,0.1f,0.4f);
	make_gradient_light(30,30,(float *)sky_lights_c4,
			0.0f,0.1f,0.4f,
			0.01f,0.01f,0.01f);
	make_gradient_light(60,30,(float *)sky_lights_c4,
			0.01f,0.01f,0.01f,
			0.0f,0.1f,0.4f);
	make_gradient_light(90,30,(float *)sky_lights_c4,
			0.0f,0.1f,0.4f,
			0.05f,0.2f,0.6f);
#endif /* SKY_FPV_CURSOR */
#ifdef NEW_LIGHTING
	}
#endif // NEW_LIGHTING

}

void build_sun_pos_table()
{
	float d = 400;
	int i;
#ifdef NEW_LIGHTING
	if (use_new_lighting)
	{
		for(i=0;i<360;i++)
		{
			sun_pos[i].x=d*cos((float)(i-30)*M_PI/180.0f);
			sun_pos[i].y=0.0f;
			sun_pos[i].z=d*(sin((float)(i-30)*M_PI/180.0f) + 0.6);
			if (sun_pos[i].z < 50)
			  sun_pos[i].z = 100 - sun_pos[i].z;
			sun_pos[i].w=0.0f;
		}
	}
	else
	{
#endif // NEW_LIGHTING
#ifndef SKY_FPV_CURSOR
		float x,y,z;
		int start=60;
		
		x=0;
		for(i=0;i<60*3;i++)
#else /* SKY_FPV_CURSOR */
	float x,y,z,step;
	int start;//60;

	x=0;
	for(i=0;i<60*6;i++)
#endif /* SKY_FPV_CURSOR */
		{
#ifndef SKY_FPV_CURSOR
			z = d*sin((float)(i+start)*0.6f*M_PI/180.0f);
			y = d*cos((float)(i+start)*0.6f*M_PI/180.0f);
			x+=0.5f;
#else /* SKY_FPV_CURSOR */
			step = 1.05f;
			start=-4.5f;
			
			z = sin((float)(i+start)*step*M_PI/180.0f);
			y = cos((float)(i+start)*step*M_PI/180.0f);
			//x+=0.5f;

			sun_show[i].x=x;
			sun_show[i].y=y;
			sun_show[i].z=z;
			sun_show[i].w=0.0f;

			step = 0.9f;
			start=9.0f;

			z = d*sin((float)(i+start)*step*M_PI/180.0f);
			y = d*cos((float)(i+start)*step*M_PI/180.0f);
			//x+=0.5f;
#endif /* SKY_FPV_CURSOR */

			sun_pos[i].x=x;
			sun_pos[i].y=y;
			sun_pos[i].z=z;
			sun_pos[i].w=0.0f;
		}
#ifdef NEW_LIGHTING
	}
#endif // NEW_LIGHTING
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
#ifdef NEW_LIGHTING
	if (use_new_lighting)
	{
		sun_position[0]=sun_pos[game_minute].x;
		sun_position[1]=sun_pos[game_minute].y;
		sun_position[2]=sun_pos[game_minute].z;
		sun_position[3]=sun_pos[game_minute].w;
		if (((game_minute >= 5) && (game_minute < 30)) || ((game_minute >= 210) && (game_minute < 235)))
		{
			disable_local_lights();
			is_day=0;
			calc_shadow_matrix();
		}
		else if(game_minute>=30 && game_minute<210 && !dungeon)
		{
			disable_local_lights();
			is_day=1;
			calc_shadow_matrix();
		}
		else//it's too dark, or we are in a dungeon
		{
			is_day=0;
			enable_local_lights();
		}
	}
	else
	{
#endif // NEW_LIGHTING
		if(game_minute>=30 && game_minute<60*3+30 && !dungeon)
		{
			disable_local_lights();
			is_day=1;
			sun_position[0]=sun_pos[game_minute-30].x;
			sun_position[1]=sun_pos[game_minute-30].y;
			sun_position[2]=sun_pos[game_minute-30].z;
			sun_position[3]=sun_pos[game_minute-30].w;
#ifdef SKY_FPV_CURSOR
			sun_appears[0]=sun_show[game_minute-30].x;
			sun_appears[1]=sun_show[game_minute-30].y;
			sun_appears[2]=sun_show[game_minute-30].z;
			sun_appears[3]=sun_show[game_minute-30].w;
#endif /* SKY_FPV_CURSOR */
			calc_shadow_matrix();
		}
		else//it's too dark, or we are in a dungeon
		{
			is_day=0;
			enable_local_lights();
#ifndef SKY_FPV_CURSOR
		    	sun_position[0]=sun_position[1]=sun_position[2]=0.0;
#else /* SKY_FPV_CURSOR */
			sun_position[0]=sun_position[1]=sun_position[2]=0.0;
			sun_appears[0]=sun_appears[1]=sun_appears[2]=0.0;
		}
		if(game_minute>=30){
			sun_position[0]=sun_pos[game_minute-30].x;
			sun_position[1]=sun_pos[game_minute-30].y;
			sun_position[2]=sun_pos[game_minute-30].z;
			sun_position[3]=sun_pos[game_minute-30].w;
		} else {
			sun_position[0]=sun_pos[game_minute+329].x;
			sun_position[1]=sun_pos[game_minute+329].y;
			sun_position[2]=sun_pos[game_minute+329].z;
			sun_position[3]=sun_pos[game_minute+329].w;
#endif /* SKY_FPV_CURSOR */
		}
#ifdef NEW_LIGHTING
	}
#endif // NEW_LIGHTING
}

#if defined(NEW_LIGHTING) || defined(DEBUG_TIME)

#ifndef WINDOWS
#include <sys/time.h>
#endif

void light_idle()
{
	Uint64 new_time;
#ifdef WINDOWS
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	new_time = ft.dwHighDateTime;
	new_time <<= 32;
	new_time |= ft.dwLowDateTime;
	new_time /= 10;
#else
	struct timeval t;
	gettimeofday(&t, NULL);
	new_time = ((Uint64)t.tv_sec)*1000000ul + (Uint64)t.tv_usec;
#endif
#ifdef DEBUG_TIME
	if (new_time / (Uint64)(60000000 / debug_time_accel) - old_time / (Uint64)(60000000 / debug_time_accel)){
		game_minute++;
		if (game_minute >= 360)
		      game_minute -= 360;
		new_minute();
	}
#endif

#ifdef DEBUG_TIME
	if (old_time != 0){
		int count = new_time / (Uint64)(250000 / debug_time_accel) - old_time / (Uint64)(250000 / debug_time_accel);
		if (count > 10)
			count = 10;
		int j;
		for (j = 0; j < count; j++){
#else
		if (new_time / 250000 - old_time / 250000){
#endif
		// A new second.
		// Reload the next texture cache entry to reset
		// the saturation for the current lighting.  Don't want to do too
		// many at once; we want this to be imperceptible.
#ifdef NEW_LIGHTING
	if (night_shift_textures || old_night_shift_textures)
	{
		int i;
		for (i = last_texture_start; i < TEXTURE_CACHE_MAX; i++)
		{
			if (texture_cache[i].file_name[0] && !texture_cache[i].load_err)
			{
				int alpha= texture_cache[i].alpha;
				if(alpha <= 0)
					reload_bmp8_color_key(texture_cache[i].file_name, alpha, texture_cache[i].texture_id);
				else
					reload_bmp8_fixed_alpha(texture_cache[i].file_name, alpha, texture_cache[i].texture_id);
				if ((dungeon == last_dungeon) && (night_shift_textures == old_night_shift_textures))
					break;
			}
		}
		if (i == TEXTURE_CACHE_MAX)
		{
			for (i = 0; i < last_texture_start; i++)
			{
				if (texture_cache[i].file_name[0] && !texture_cache[i].load_err)
				{
					int alpha= texture_cache[i].alpha;
					if(alpha <= 0)
						reload_bmp8_color_key(texture_cache[i].file_name, alpha, texture_cache[i].texture_id);
					else
						reload_bmp8_fixed_alpha(texture_cache[i].file_name, alpha, texture_cache[i].texture_id);
					if ((dungeon == last_dungeon) && (night_shift_textures == old_night_shift_textures))
						break;
				}
			}
		}
		last_texture_start = i + 1;
	}
	old_night_shift_textures = night_shift_textures;
#endif
	}
#ifdef DEBUG_TIME
	}
#endif
	old_time = new_time;
#ifdef NEW_LIGHTING
	last_dungeon = dungeon;
#endif
}
#endif

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

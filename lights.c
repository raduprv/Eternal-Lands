#include <stdlib.h>
#include <math.h>
#include "lights.h"
#include "bbox_tree.h"
#include "map.h"
#include "shadows.h"
#include "weather.h"
#ifdef CLUSTER_INSIDES
#include "cluster.h"
#endif
#ifdef EXTRA_DEBUG
#include "errors.h"
#endif
#include "eye_candy_wrapper.h"
#if defined NEW_LIGHTING || defined NIGHT_TEXTURES
#include "text.h"
#include "textures.h"
#endif
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#ifdef SKY_FPV
#include "elconfig.h"
#include "sky.h"
#ifdef NEW_WEATHER
#include "draw_scene.h"
#endif // NEW_WEATHER
#endif // SKY_FPV

#if defined NEW_LIGHTING || NIGHT_TEXTURES
int night_shift_textures = 0;
int old_night_shift_textures = 0;
int last_texture_start = 0;
int last_dungeon;
#endif
#ifdef NEW_LIGHTING
int use_new_lighting = 0;
float lighting_contrast = 0.5;
GLfloat day_ambient[4];
GLfloat day_diffuse[4];
GLfloat day_specular[4];
GLfloat dawn_ambient[4];
GLfloat dawn_diffuse[4];
GLfloat dawn_specular[4];
GLfloat night_ambient[4];
GLfloat night_diffuse[4];
GLfloat night_specular[4];
#endif // NEW_LIGHTING

#ifdef DEBUG_TIME
const float debug_time_accel = 120.0f;
#endif

#if defined(NEW_LIGHTING) || defined(DEBUG_TIME) || defined NIGHT_TEXTURES
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

GLfloat global_diffuse_light[GLOBAL_LIGHTS_NO][4];
#ifdef NEW_LIGHTING
GLfloat global_ambient_light[GLOBAL_LIGHTS_NO][4];
GLfloat global_specular_light[GLOBAL_LIGHTS_NO][4];
#endif

GLfloat sky_lights_c1[GLOBAL_LIGHTS_NO*2][4];
GLfloat sky_lights_c2[GLOBAL_LIGHTS_NO*2][4];
GLfloat sky_lights_c3[GLOBAL_LIGHTS_NO*2][4];
GLfloat sky_lights_c4[GLOBAL_LIGHTS_NO*2][4];

int	show_lights;
int	num_lights;	// the highest light number loaded
light *lights_list[MAX_LIGHTS];
unsigned char light_level=58;
#ifdef SKY_FPV
sun sun_pos[360];
sun sun_show[181];
#elif defined(NEW_LIGHTING)
sun sun_pos[60*6];
#else
sun sun_pos[60*3];
#endif

short game_minute = 0;
#ifdef SKY_FPV
short game_second = 0;
Uint32 next_second_time = 0;
short real_game_minute = 0;
short real_game_second = 0;
unsigned char freeze_time = 0;
#endif // SKY_FPV

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
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_lights()
{
	unsigned int i, j, l, start, stop;
	VECTOR4 vec4;
#ifdef CLUSTER_INSIDES_OLD
	short cluster = get_actor_cluster ();
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
	
	j= 0;
	
	get_intersect_start_stop(main_bbox_tree, TYPE_LIGHT, &start, &stop);
	for(i=start; i<stop; i++)
	{
		l= get_intersect_item_ID(main_bbox_tree, i);
		// and make sure it's a valid light
		if (l < 0 || l >= MAX_LIGHTS || !lights_list[l]
#ifdef CLUSTER_INSIDES_OLD
		   || (lights_list[l]->cluster && lights_list[l]->cluster != cluster)
#endif
		)
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
#ifdef NEW_LIGHTING
		if (use_new_lighting)
			glLightfv(GL_LIGHT0+j, GL_SPECULAR, vec4);
#endif
		if (j >= 4) break;
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

#ifdef CLUSTER_INSIDES
	new_light->cluster = get_cluster ((int)(x/0.5f), (int)(y/0.5f));
	current_cluster = new_light->cluster;
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
	GLfloat light_diffuse[] = { 0.0, 0.0, 0.0, 0.0 };
	GLfloat no_light[] = { 0.0, 0.0, 0.0, 0.0 };
	float linear_att=1.41f;
	float cut_off=180;
	//most of the things in here are redundant, since we kind of set the light sources
	//to their default values. However, better safe than sorry.

	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT0,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
#ifdef NEW_LIGHTING
	if (use_new_lighting)
		glLightfv(GL_LIGHT0,GL_SPECULAR,light_diffuse);
#endif
	glLightfv(GL_LIGHT0,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT0);

	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT1,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT1,GL_DIFFUSE,light_diffuse);
#ifdef NEW_LIGHTING
	if (use_new_lighting)
		glLightfv(GL_LIGHT0,GL_SPECULAR,light_diffuse);
#endif
	glLightfv(GL_LIGHT1,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT1);

	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT2,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT2,GL_DIFFUSE,light_diffuse);
#ifdef NEW_LIGHTING
	if (use_new_lighting)
		glLightfv(GL_LIGHT0,GL_SPECULAR,light_diffuse);
#endif
	glLightfv(GL_LIGHT2,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT2,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT2);

	glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT3,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT3,GL_DIFFUSE,light_diffuse);
#ifdef NEW_LIGHTING
	if (use_new_lighting)
		glLightfv(GL_LIGHT0,GL_SPECULAR,light_diffuse);
#endif
	glLightfv(GL_LIGHT3,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT3,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT3);

	glLightfv(GL_LIGHT7,GL_AMBIENT,no_light);
	glLightfv(GL_LIGHT7,GL_SPECULAR,no_light);
#ifdef NEW_LIGHTING
	if (use_new_lighting)
		glLightfv(GL_LIGHT0,GL_SPECULAR,light_diffuse);
#endif
	glLightfv(GL_LIGHT7,GL_DIFFUSE,no_light);
	glLightf(GL_LIGHT7,GL_CONSTANT_ATTENUATION,0);
	glEnable(GL_LIGHT7);

	ec_add_light(GL_LIGHT4);
	ec_add_light(GL_LIGHT5);
	ec_add_light(GL_LIGHT6);

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
#endif // NEW_LIGHTING
		GLfloat mat_ambient[]={ 1.0, 1.0, 1.0, 1.0 };
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ambient);
#ifdef NEW_LIGHTING
	}
#endif //NEW_LIGHTING

	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

#ifdef NEW_LIGHTING
void set_material_defaults()
{
	GLfloat mat_ambient[]={ 0.2, 0.2, 0.2, 1.0 };
	GLfloat mat_diffuse[]={ 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_specular[]={ 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_emission[]={ 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, 30);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}
#endif

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
GLfloat ambient_light[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat diffuse_light[] = { 0.0, 0.0, 0.0, 0.0 };
#ifdef NEW_LIGHTING
GLfloat specular_light[] = {0.0, 0.0, 0.0, 1.0 };
#endif
void draw_global_light()
{
	int i;
	GLfloat global_light_position[] = { 400.0, 400.0, 500.0, 0.0 };
#ifndef SKY_FPV
	i=light_level;
	if(light_level>59)i=119-light_level;
	//this is for weather things, when the light level is not the normal light lvel of the current time
# ifndef MAP_EDITOR2
	i+=weather_light_offset;
# endif // MAP_EDITOR2
	if(i<0)i=0;
	if(i>59)i=59;
#endif // SKY_FPV

	//add the thunder light to the ambient/diffuse light
#ifndef MAP_EDITOR2
# ifdef NEW_WEATHER
	{
		float ratios[MAX_WEATHER_TYPES];
		float sun_proj[3];
		float density;

		skybox_compute_element_projection(sun_proj, sun_position);
		weather_compute_ratios(ratios, sun_proj[0]*0.1-camera_x, sun_proj[1]*0.1-camera_y);
		density = weather_get_density_from_ratios(ratios);

		skybox_blend_current_colors(ambient_light, skybox_light_ambient, skybox_light_ambient_rainy, density);
		skybox_blend_current_colors(diffuse_light, skybox_light_diffuse, skybox_light_diffuse_rainy, density);
		// the thunder is handled elsewhere for the new weather
	}
# else // NEW_WEATHER
#  ifdef NEW_LIGHTING
 	if (use_new_lighting)
 	{
		ambient_light[0]=global_ambient_light[i][0]+(float)thunder_light_offset/90/8;
		ambient_light[1]=global_ambient_light[i][1]+(float)thunder_light_offset/60/8;
		ambient_light[2]=global_ambient_light[i][2]+(float)thunder_light_offset/15/8;
		diffuse_light[0]=global_diffuse_light[i][0]+(float)thunder_light_offset/90/4;
		diffuse_light[1]=global_diffuse_light[i][1]+(float)thunder_light_offset/60/4;
		diffuse_light[2]=global_diffuse_light[i][2]+(float)thunder_light_offset/15/4;
		specular_light[0]=global_specular_light[i][0]+(float)thunder_light_offset/90/4;
		specular_light[1]=global_specular_light[i][1]+(float)thunder_light_offset/60/4;
		specular_light[2]=global_specular_light[i][2]+(float)thunder_light_offset/15/4;
	}
	else
	{
#  endif // NEW_LIGHTING
#  ifndef SKY_FPV
		diffuse_light[0]=global_diffuse_light[i][0]+(float)thunder_light_offset/90-0.15f;
		diffuse_light[1]=global_diffuse_light[i][1]+(float)thunder_light_offset/60-0.15f;
		diffuse_light[2]=global_diffuse_light[i][2]+(float)thunder_light_offset/15-0.15f;
#  else // SKY_FPV
		skybox_blend_current_colors(ambient_light, skybox_light_ambient, skybox_light_ambient_rainy, weather_rain_intensity);
		skybox_blend_current_colors(diffuse_light, skybox_light_diffuse, skybox_light_diffuse_rainy, weather_rain_intensity);
		ambient_light[0] += (float)thunder_light_offset*0.03;
		ambient_light[1] += (float)thunder_light_offset*0.05;
		ambient_light[2] += (float)thunder_light_offset*0.06;
#  endif // SKY_FPV
#  ifdef NEW_LIGHTING
 	}
#  endif // NEW_LIGHTING
# endif // NEW_WEATHER
#else // MAP_EDITOR2
	diffuse_light[0]=global_diffuse_light[i][0];
	diffuse_light[1]=global_diffuse_light[i][1];
	diffuse_light[2]=global_diffuse_light[i][2];
#endif // MAP_EDITOR2

	for (i = 0; i < 3; i++)
	{
		if (diffuse_light[i] < 0.0f)
		{
			diffuse_light[i] = 0.0f;
		}
		if (ambient_light[i] < 0.0f)
		{
			ambient_light[i] = 0.0f;
		}
	}

#ifndef SKY_FPV
#ifdef NEW_LIGHTING
	if (!use_new_lighting)
	{
#endif
		if(map_type==2)
		{
			//the ambient light should be almost as the normal light, but a little bluer
			ambient_light[0]=diffuse_light[0]+.01f;
			ambient_light[1]=diffuse_light[1]+.01f;
			ambient_light[2]=diffuse_light[2];
		}
		else
		{
			//the ambient light should be half of the diffuse light
			ambient_light[0]=diffuse_light[0]/3.5f+0.15f;
			ambient_light[1]=diffuse_light[1]/3.5f+0.15f;
			ambient_light[2]=diffuse_light[2]/3.5f+0.15f;
		}
#ifdef NEW_LIGHTING
	}
#endif


	ambient_light[3]=1.0f;
#ifdef NEW_LIGHTING
/*
	printf("Ambient: %f, %f, %f\n", ambient_light[0], ambient_light[1], ambient_light[2]);
	printf("Diffuse: %f, %f, %f\n", diffuse_light[0], diffuse_light[1], diffuse_light[2]);
	printf("Specular: %f, %f, %f\n", specular_light[0], specular_light[1], specular_light[2]);
*/
        diffuse_light[3] = 1.0f;
        specular_light[3] = 1.0f;
	if (use_new_lighting)
	{
//		glEnable(GL_LIGHT7);
//		printf("Light: %f, %f, %f\n", diffuse_light[0], diffuse_light[1], diffuse_light[2]);
/*
		diffuse_light[0] *= 3.0f;
		diffuse_light[1] *= 3.0f;
		diffuse_light[2] *= 3.0f;
		specular_light[0] = diffuse_light[0] * 1.0f;
		specular_light[1] = diffuse_light[1] * 1.0f;
		specular_light[2] = diffuse_light[2] * 1.0f;
		ambient_light[0] /= 2.0f;
		ambient_light[1] /= 2.0f;
		ambient_light[2] /= 2.0f;
		glLightfv(GL_LIGHT7,GL_SPECULAR,specular_light);
*/
		glLightfv(GL_LIGHT7,GL_AMBIENT, ambient_light);
		glLightfv(GL_LIGHT7,GL_DIFFUSE, diffuse_light);
		glLightfv(GL_LIGHT7,GL_SPECULAR, specular_light);
	}
	else
	{
#endif	//NEW_LIGHTING

		glLightfv(GL_LIGHT7,GL_AMBIENT,ambient_light);
		//We add (0.2,0.2,0.2) because that's the global ambient color, and we need it for shadows

		ambient_light[0]+=0.2f;
		ambient_light[1]+=0.2f;
		ambient_light[2]+=0.2f;

		glLightfv(GL_LIGHT7, GL_DIFFUSE, diffuse_light);
#ifdef NEW_LIGHTING
	}
#endif

#else // SKY_FPV

	glLightfv(GL_LIGHT7, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT7, GL_DIFFUSE, diffuse_light);

#endif // SKY_FPV

	if (sun_use_static_position)
	{
		glLightfv(GL_LIGHT7, GL_POSITION, global_light_position);
	}
	else
	{
		if ((sun_position[0] == 0.0f) && (sun_position[1] == 0.0f) &&
			(sun_position[2] == 0.0f) && (sun_position[3] == 0.0f))
		{
			glLightfv(GL_LIGHT7, GL_POSITION, global_light_position);
		}
		else
		{
			glLightfv(GL_LIGHT7, GL_POSITION, sun_position);
		}
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_dungeon_light()
{
	GLfloat global_light_position[] = { 400.0, 400.0, 500.0, 0.0 };
	GLfloat diffuse_light[] = { 0.0, 0.0, 0.0, 0.0 };
	GLfloat ambient_light[4];
	int i;

#ifdef NEW_LIGHTING
	if (use_new_lighting)
	{
/*
		glEnable(GL_LIGHT7);
		diffuse_light[0] = ambient_r / 2.0f;
		diffuse_light[1] = ambient_g / 2.0f;
		diffuse_light[2] = ambient_b / 2.0f;
		diffuse_light[3] = 1.0;
		ambient_light[0] = ambient_r / 12.0f;
		ambient_light[1] = ambient_g / 12.0f;
		ambient_light[2] = ambient_b / 12.0f;
		ambient_light[3] = 1.0;
*/
		ambient_light[0]=global_ambient_light[59][0];
		ambient_light[1]=global_ambient_light[59][1];
		ambient_light[2]=global_ambient_light[59][2];
		diffuse_light[0]=global_diffuse_light[59][0];
		diffuse_light[1]=global_diffuse_light[59][1];
		diffuse_light[2]=global_diffuse_light[59][2];
		specular_light[0]=global_specular_light[59][0];
		specular_light[1]=global_specular_light[59][1];
		specular_light[2]=global_specular_light[59][2];
	}
	else
	{
#endif // NEW_LIGHTING
	//the ambient light should be half of the diffuse light
		ambient_light[0]=ambient_r;
		ambient_light[1]=ambient_g;
		ambient_light[2]=ambient_b;
		ambient_light[3]=1.0f;
#ifdef NEW_LIGHTING
	}
#endif // NEW_LIGHTING
	for (i = 0; i < 3; i++)
	{
		if (diffuse_light[i] < 0.0f)
		{
			diffuse_light[i] = 0.0f;
		}
		if (ambient_light[i] < 0.0f)
		{
			ambient_light[i] = 0.0f;
		}
	}

	glLightfv(GL_LIGHT7,GL_AMBIENT,ambient_light);
	glLightfv(GL_LIGHT7, GL_POSITION, global_light_position);
	glLightfv(GL_LIGHT7,GL_DIFFUSE,diffuse_light);
#ifdef NEW_LIGHTING
	if (use_new_lighting)
		glLightfv(GL_LIGHT7,GL_SPECULAR, specular_light);
#endif
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
/*
  		make_gradient_light(0,15,(float *)global_diffuse_light,0.6f,0.6f,0.6f, 0.7f,0.45f,0.3f);
  		make_gradient_light(14,16,(float *)global_diffuse_light,0.7f,0.45f,0.3f, 0.65f,0.4f,0.25f);
  		make_gradient_light(29,16,(float *)global_diffuse_light,0.65f,0.4f,0.25f, 0.6f,0.35f,0.15f);
		make_gradient_light(44,16,(float *)global_diffuse_light,0.6f,0.35f,0.15f, 0.12f,0.12f,0.15f);
*/
		make_gradient_light(0,15,(float *)global_ambient_light, day_ambient[0], day_ambient[1], day_ambient[2], dawn_ambient[0], dawn_ambient[1], dawn_ambient[2]);
		make_gradient_light(14,46,(float *)global_ambient_light, dawn_ambient[0], dawn_ambient[1], dawn_ambient[2], night_ambient[0], night_ambient[1], night_ambient[2]);
		make_gradient_light(0,15,(float *)global_diffuse_light, day_diffuse[0], day_diffuse[1], day_diffuse[2], dawn_diffuse[0], dawn_diffuse[1], dawn_diffuse[2]);
		make_gradient_light(14,46,(float *)global_diffuse_light, dawn_diffuse[0], dawn_diffuse[1], dawn_diffuse[2], night_diffuse[0], night_diffuse[1], night_diffuse[2]);
		make_gradient_light(0,15,(float *)global_specular_light, day_specular[0], day_specular[1], day_specular[2], dawn_specular[0], dawn_specular[1], dawn_specular[2]);
		make_gradient_light(14,46,(float *)global_specular_light, dawn_specular[0], dawn_specular[1], dawn_specular[2], night_specular[0], night_specular[1], night_specular[2]);
	
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
#ifndef SKY_FPV
		//the sun light
		make_gradient_light(0,30,(float *)global_diffuse_light,0.85f,0.85f,0.85f,0.32f,0.25f,0.25f);
		make_gradient_light(30,30,(float *)global_diffuse_light,0.318f,0.248f,0.248f,0.06f,0.06f,0.08f);
	
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
#else // SKY_FPV
	//the sun light
  	make_gradient_light(0,30,(float *)global_diffuse_light,0.85f,0.85f,0.85f,0.32f,0.25f,0.25f);
	make_gradient_light(30,30,(float *)global_diffuse_light,0.318f,0.248f,0.248f,0.05f,0.05f,0.08f);
	//lake light
	make_gradient_light(0,30,(float *)sky_lights_c1,
						0.3f, 0.7f, 0.9f,
						0.6f, 0.0f, 0.0f);
	make_gradient_light(30,30,(float *)sky_lights_c1,
						0.6f, 0.0f, 0.0f,
						0.0f, 0.05f, 0.1f);
	make_gradient_light(60,30,(float *)sky_lights_c1,
						0.0f, 0.05f, 0.1f,
						0.6f, 0.0f, 0.2f);
	make_gradient_light(90,30,(float *)sky_lights_c1,
						0.6f, 0.0f, 0.2f,
						0.3f, 0.7f, 0.9f);

	make_gradient_light(0,30,(float *)sky_lights_c2,
						0.2f, 0.5f, 0.8f,
						0.6f, 0.3f, 0.0f);
	make_gradient_light(30,30,(float *)sky_lights_c2,
						0.6f, 0.3f, 0.0f,
						0.0f, 0.025f, 0.05f);
	make_gradient_light(60,30,(float *)sky_lights_c2,
						0.0f, 0.025f, 0.05f,
						0.6f, 0.3f, 0.0f);
	make_gradient_light(90,30,(float *)sky_lights_c2,
						0.6f, 0.3f, 0.0f,
						0.2f, 0.5f, 0.8f);

	make_gradient_light(0,30,(float *)sky_lights_c3,
						0.1f, 0.3f, 0.7f,
						0.2f, 0.2f, 0.3f);
	make_gradient_light(30,30,(float *)sky_lights_c3,
						0.2f, 0.2f, 0.3f,
						0.0f, 0.01f, 0.02f);
	make_gradient_light(60,30,(float *)sky_lights_c3,
						0.0f, 0.01f, 0.02f,
						0.2f, 0.2f, 0.3f);
	make_gradient_light(90,30,(float *)sky_lights_c3,
						0.2f, 0.2f, 0.3f,
						0.1f, 0.3f, 0.7f);

	make_gradient_light(0,30,(float *)sky_lights_c4,
						0.05f, 0.2f, 0.6f,
						0.0f, 0.1f, 0.4f);
	make_gradient_light(30,30,(float *)sky_lights_c4,
						0.0f, 0.1f, 0.4f,
						0.01f, 0.01f, 0.01f);
	make_gradient_light(60,30,(float *)sky_lights_c4,
						0.01f, 0.01f, 0.01f,
						0.0f, 0.1f, 0.4f);
	make_gradient_light(90,30,(float *)sky_lights_c4,
						0.0f, 0.1f, 0.4f,
						0.05f, 0.2f, 0.6f);
#endif // SKY_FPV
#ifdef NEW_LIGHTING
	}
#endif // NEW_LIGHTING

}

void build_sun_pos_table()
{
#ifndef SKY_FPV
	float d = 400;
#endif // SKY_FPV
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
		float x,y,z;
#ifndef SKY_FPV
		int start=60;
		
		x=0;
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
#else // SKY_FPV
		float start, step;

		// position of the displayed sun
		step  = 189.0/180.0;
		start = -4.5;
		for(i = 0; i <= 180; i++)
		{
			z = sinf((start+i*step)*M_PI/180.0);
			x = cosf((start+i*step)*M_PI/180.0);
			y = 0;

			sun_show[i].x=x;
			sun_show[i].y=y;
			sun_show[i].z=z;
			sun_show[i].w=0.0;
		}

		// position of the light during the day
		step  = 162.0/180.0;
		start = 9.0;
		for(i = 0; i < 180; i++)
		{
			z = sinf((start+i*step)*M_PI/180.0);
			x = cosf((start+i*step)*M_PI/180.0);
			y = 0;

			sun_pos[i].x=x;
			sun_pos[i].y=y;
			sun_pos[i].z=z;
			sun_pos[i].w=0.0;
		}

		// position of the light during the night (used to light the moons)
		step  = 198.0/180.0;
		start = -27.0;
		for(i = 180; i < 360; i++)
		{
			z = sinf((start+i*step)*M_PI/180.0);
			x = cosf((start+i*step)*M_PI/180.0);
			y = 0;

			sun_pos[i].x=x;
			sun_pos[i].y=y;
			sun_pos[i].z=z;
			sun_pos[i].w=0.0;
		}
#endif // SKY_FPV
#ifdef NEW_LIGHTING
	}
#endif // NEW_LIGHTING
}

void new_minute()
{
#ifdef EXTRA_DEBUG
	ERR();
#endif
	if (!freeze_time) game_minute = real_game_minute;

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
#ifdef SKY_FPV
		if(game_minute >= 30 && game_minute <= 210 && !dungeon)
		{
			skybox_sun_position[0] = sun_show[game_minute-30].x;
			skybox_sun_position[1] = sun_show[game_minute-30].y;
			skybox_sun_position[2] = sun_show[game_minute-30].z;
			skybox_sun_position[3] = sun_show[game_minute-30].w;
#else // SKY_FPV
		if(game_minute>=30 && game_minute<60*3+30 && !dungeon)
		{
#endif // SKY_FPV
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
#ifndef SKY_FPV
			sun_position[0]=sun_position[1]=sun_position[2]=sun_position[3]=0.0;
#else // SKY_FPV
			int shift_time = (game_minute+330)%360;
			sun_position[0] = sun_pos[shift_time].x;
			sun_position[1] = sun_pos[shift_time].y;
			sun_position[2] = sun_pos[shift_time].z;
			sun_position[3] = sun_pos[shift_time].w;
			skybox_sun_position[0] = skybox_sun_position[1] = skybox_sun_position[2] = skybox_sun_position[3] = 0.0;
#endif // SKY_FPV
			is_day=0;
			enable_local_lights();
		}
#ifdef NEW_LIGHTING
	}
#endif // NEW_LIGHTING

#ifdef SKY_FPV
	skybox_update_positions();
	if (!skybox_update_every_frame)
		skybox_update_colors();
#endif // SKY_FPV
}

#ifdef SKY_FPV
void new_second()
{
	int cur_min = (game_minute+330)%360;
	int next_min = (game_minute+331)%360;
	float ratio2 = (float)game_second/60.0;
	float ratio1 = 1.0 - ratio2;
	
	if (!freeze_time) game_second = real_game_second;

	sun_position[0] = sun_pos[cur_min].x * ratio1 + sun_pos[next_min].x * ratio2;
	sun_position[1] = sun_pos[cur_min].y * ratio1 + sun_pos[next_min].y * ratio2;
	sun_position[2] = sun_pos[cur_min].z * ratio1 + sun_pos[next_min].z * ratio2;
	sun_position[3] = sun_pos[cur_min].w * ratio1 + sun_pos[next_min].w * ratio2;
	
	if (is_day)
	{
		skybox_sun_position[0] = sun_show[cur_min].x * ratio1 + sun_show[next_min].x * ratio2;
		skybox_sun_position[1] = sun_show[cur_min].y * ratio1 + sun_show[next_min].y * ratio2;
		skybox_sun_position[2] = sun_show[cur_min].z * ratio1 + sun_show[next_min].z * ratio2;
		skybox_sun_position[3] = sun_show[cur_min].w * ratio1 + sun_show[next_min].w * ratio2;
		calc_shadow_matrix();
	}
	
	skybox_update_positions();
	if (!skybox_update_every_frame)
		skybox_update_colors();
}
#endif // SKY_FPV

#if defined(NEW_LIGHTING) || defined(DEBUG_TIME) || defined NIGHT_TEXTURES

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
		int j;
		int count = new_time / (Uint64)(250000 / debug_time_accel) - old_time / (Uint64)(250000 / debug_time_accel);
		if (count > 10)
			count = 10;
		for (j = 0; j < count; j++){
#else
		if (new_time / 250000 - old_time / 250000){
#endif
		// A new second.
		// Reload the next texture cache entry to reset
		// the saturation for the current lighting.  Don't want to do too
		// many at once; we want this to be imperceptible.
#if defined NEW_LIGHTING || defined NIGHT_TEXTURES
	if (night_shift_textures || old_night_shift_textures)
	{
		int i;
		for (i = last_texture_start; i < TEXTURE_CACHE_MAX; i++)
		{
			if (texture_cache[i].file_name[0] && !texture_cache[i].load_err)
			{
				int alpha= texture_cache[i].alpha;
				if(alpha <= 0)
					reload_bmp8_color_key(&(texture_cache[i]), alpha, texture_cache[i].texture_id);
				else
					reload_bmp8_fixed_alpha(&(texture_cache[i]), alpha, texture_cache[i].texture_id);
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
						reload_bmp8_color_key(&(texture_cache[i]), alpha, texture_cache[i].texture_id);
					else
						reload_bmp8_fixed_alpha(&(texture_cache[i]), alpha, texture_cache[i].texture_id);
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
#if defined NEW_LIGHTING || defined NIGHT_TEXTURES
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

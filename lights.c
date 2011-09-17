#include <stdlib.h>
#include <math.h>
#include "lights.h"
#include "bbox_tree.h"
#include "map.h"
#include "multiplayer.h"
#include "shadows.h"
#include "weather.h"
#ifdef CLUSTER_INSIDES
#include "cluster.h"
#endif
#ifdef EXTRA_DEBUG
#include "errors.h"
#endif
#include "eye_candy_wrapper.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "elconfig.h"
#include "sky.h"
#include "draw_scene.h"


#ifdef DEBUG_TIME
const float debug_time_accel = 120.0f;
#endif

#ifdef DEBUG_TIME
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

GLfloat sky_lights_c1[GLOBAL_LIGHTS_NO*2][4];
GLfloat sky_lights_c2[GLOBAL_LIGHTS_NO*2][4];
GLfloat sky_lights_c3[GLOBAL_LIGHTS_NO*2][4];
GLfloat sky_lights_c4[GLOBAL_LIGHTS_NO*2][4];

int	show_lights;
int	num_lights;	// the highest light number loaded
light *lights_list[MAX_LIGHTS];
unsigned char light_level=58;
sun sun_pos[360];
sun sun_show[181];

short game_minute = 0;
short game_second = 0;
unsigned char freeze_time = 0;

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
		if ((l >= MAX_LIGHTS) || !lights_list[l]
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

	glLightfv(GL_LIGHT7,GL_AMBIENT,no_light);
	glLightfv(GL_LIGHT7,GL_SPECULAR,no_light);
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
		GLfloat mat_ambient[]={ 1.0, 1.0, 1.0, 1.0 };
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ambient);

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
GLfloat ambient_light[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat diffuse_light[] = { 0.0, 0.0, 0.0, 0.0 };
void draw_global_light()
{
	int i;
	GLfloat global_light_position[] = { 400.0, 400.0, 500.0, 0.0 };

	//add the thunder light to the ambient/diffuse light
#ifndef MAP_EDITOR2
	memcpy(ambient_light, skybox_light_ambient_color, 3*sizeof(float));
	memcpy(diffuse_light, skybox_light_diffuse_color, 3*sizeof(float));
	// the thunder is handled elsewhere for the new weather
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


	glLightfv(GL_LIGHT7, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT7, GL_DIFFUSE, diffuse_light);


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

	//the ambient light should be half of the diffuse light
		ambient_light[0]=ambient_r;
		ambient_light[1]=ambient_g;
		ambient_light[2]=ambient_b;
		ambient_light[3]=1.0f;
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

}

void build_sun_pos_table()
{
	int i;
		float x,y,z;
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
}

void new_minute()
{
#ifdef EXTRA_DEBUG
	ERR();
#endif
	if (!freeze_time) game_minute = real_game_minute;
	if (!freeze_time) game_second = real_game_second;

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
		if(game_minute >= 30 && game_minute < 210 && !dungeon)
		{
			skybox_sun_position[0] = sun_show[game_minute-30].x;
			skybox_sun_position[1] = sun_show[game_minute-30].y;
			skybox_sun_position[2] = sun_show[game_minute-30].z;
			skybox_sun_position[3] = sun_show[game_minute-30].w;
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
			int shift_time = (game_minute+330)%360;
			sun_position[0] = sun_pos[shift_time].x;
			sun_position[1] = sun_pos[shift_time].y;
			sun_position[2] = sun_pos[shift_time].z;
			sun_position[3] = sun_pos[shift_time].w;
			skybox_sun_position[0] = skybox_sun_position[1] = skybox_sun_position[2] = skybox_sun_position[3] = 0.0;
			is_day=0;
			enable_local_lights();
		}

	skybox_update_positions();
	if (skybox_update_delay > 0)
		skybox_update_colors();
}

void new_second()
{
	if (!freeze_time) game_second = real_game_second;

	if (skybox_update_delay < 1 || real_game_second % skybox_update_delay == 0)
	{
		int cur_min = (game_minute+330)%360;
		int next_min = (game_minute+331)%360;
		float ratio2 = (float)game_second/60.0;
		float ratio1 = 1.0 - ratio2;
			
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
		if (skybox_update_delay > 0)
			skybox_update_colors();
	}
	else if (skybox_update_delay > 1 && weather_get_intensity() > 0.0)
	{
		skybox_update_colors();
	}
}

#ifdef DEBUG_TIME

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
	if (new_time / (Uint64)(60000000 / debug_time_accel) - old_time / (Uint64)(60000000 / debug_time_accel)){
		game_minute++;
		if (game_minute >= 360)
		      game_minute -= 360;
		new_minute();
	}

	old_time = new_time;
}
#endif // DEBUG_TIME

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

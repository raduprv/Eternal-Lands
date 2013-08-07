
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <SDL.h>
#include <SDL_active.h>
#include "asc.h"
#include "sky.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "errors.h"
#include "gl_init.h"
#include "global.h"
#include "lights.h"
#include "map.h"
#include "multiplayer.h"
#include "reflection.h"
#include "shadows.h"
#include "textures.h"
#include "vmath.h"
#include "weather.h"

float skybox_clouds[360][4];
float skybox_clouds_detail[360][4];
float skybox_clouds_sunny[360][4];
float skybox_clouds_detail_sunny[360][4];
float skybox_clouds_rainy[360][4];
float skybox_clouds_detail_rainy[360][4];
float skybox_sky1[360][4];
float skybox_sky2[360][4];
float skybox_sky3[360][4];
float skybox_sky4[360][4];
float skybox_sky5[360][4];
float skybox_sky1_sunny[360][4];
float skybox_sky2_sunny[360][4];
float skybox_sky3_sunny[360][4];
float skybox_sky4_sunny[360][4];
float skybox_sky5_sunny[360][4];
float skybox_sun[360][4];
float skybox_fog[360][4];
float skybox_fog_sunny[360][4];
float skybox_fog_rainy[360][4];
float skybox_light_ambient[360][4];
float skybox_light_diffuse[360][4];
float skybox_light_ambient_rainy[360][4];
float skybox_light_diffuse_rainy[360][4];
float skybox_sky_color[4] = {0.0, 0.0, 0.0, 0.0};
float skybox_fog_color[4] = {0.0, 0.0, 0.0, 0.0};
float skybox_fog_density = 0.0;
float skybox_light_ambient_color[4];
float skybox_light_diffuse_color[4];
int skybox_no_clouds = 1;
int skybox_no_sun = 1;
int skybox_no_moons = 1;
int skybox_no_stars = 1;
int skybox_clouds_tex = -1;
int skybox_clouds_detail_tex = -1;

int skybox_show_sky = 1;
int skybox_show_clouds = 1;
int skybox_show_sun = 1;
int skybox_show_moons = 1;
int skybox_show_stars = 1;
int skybox_show_horizon_fog = 0;
float skybox_sunny_sky_bias = -0.3;
float skybox_sunny_clouds_bias = -0.3;
float skybox_sunny_fog_bias = -0.3;
float skybox_moonlight1_bias = 0.92;
float skybox_moonlight2_bias = 0.98;

float skybox_sun_position[4] = {0.0, 0.0, 0.0, 0.0};
float skybox_sun_projection[2] = {0.0, 0.0};
double skybox_time_d = 0.0;
double skybox_view[16];

float skybox_z = 0.0;

typedef struct
{
	int slices_count;
	int stacks_count;
	int vertices_count;
	int faces_count;
	GLfloat *vertices;
	GLfloat *normals;
	GLfloat *colors;
	GLfloat *tex_coords;
	GLuint *faces;
	float radius;
	float real_radius;
	float opening;
	float height;
	float texture_size;
	float tex_coords_conversion_factor;
	float conversion_factor;
} sky_dome;

typedef struct
{
	int vertices_count;
	int faces_count;
	GLfloat *vertices;
	GLfloat *tex_coords;
	GLuint *faces;
} sky_sphere;

sky_dome dome_sky = {0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0.0, 0.0, 0.0};
sky_dome dome_clouds = {0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0.0, 0.0, 0.0};

GLfloat *dome_clouds_detail_colors = NULL;
GLfloat *dome_clouds_colors_bis = NULL;
GLfloat *dome_clouds_detail_colors_bis = NULL;
GLfloat *dome_clouds_tex_coords_bis = NULL;
GLfloat *fog_colors = NULL;

sky_sphere moon_mesh = {0, 0, NULL, NULL, NULL};

GLfloat moon1_direction[3];
GLfloat moon2_direction[3];
GLfloat moon1_color[3];
GLfloat moon2_color[3];
GLfloat sun_color[4];
double moon_spin = 0.0;
float rain_coef = 0.0;
float day_alpha = 0.0;
skybox_type current_sky = SKYBOX_NONE;

#define NUM_STARS 3000
GLuint sky_lists;
int thick_clouds_tex;
int thick_clouds_detail_tex;
int moon_tex;
int sun_tex;

sky_dome create_dome(int slices, int stacks, float radius, float opening, int fake_opening, float first_angle, float texture_size)
{
	sky_dome dome = {0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, 0.0, 0.0, 0.0};
    int i, j;
    float angle, angle_step;
    int idx, vtx_idx;
	float max_sin_angle;
	float max_cos_angle;

    dome.slices_count = slices;
    dome.stacks_count = stacks;

    dome.vertices_count = dome.slices_count*(dome.stacks_count+1)+1;
    dome.faces_count = dome.slices_count*(1+2*dome.stacks_count);

    dome.vertices = (GLfloat*)malloc(3*dome.vertices_count*sizeof(GLfloat));
    dome.normals = (GLfloat*)malloc(3*dome.vertices_count*sizeof(GLfloat));
    dome.colors = (GLfloat*)malloc(4*dome.vertices_count*sizeof(GLfloat));
    dome.tex_coords = (GLfloat*)malloc(2*dome.vertices_count*sizeof(GLfloat));
    dome.faces = (GLuint*)malloc(3*dome.faces_count*sizeof(GLuint));

	dome.radius = radius;
    dome.opening = opening;

	angle = dome.opening * M_PI / 180.0;
	max_sin_angle = sinf(angle);
	max_cos_angle = cosf(angle);

    dome.real_radius = dome.radius/max_sin_angle;
    dome.height = dome.real_radius - dome.real_radius*max_cos_angle;
	dome.texture_size = texture_size;
	dome.tex_coords_conversion_factor = dome.radius / texture_size;

	texture_size *= dome.radius * max_cos_angle / (max_sin_angle * dome.real_radius);

	dome.conversion_factor = max_cos_angle * dome.radius;

    // we compute the vertices positions and normals
    i = idx = 0;
	if (first_angle > 0.0)
	{
		float cos_angle = cosf(angle);
		float sin_angle = sinf(angle);
        float z_pos = cos_angle * dome.real_radius + dome.height - dome.real_radius;
		float tmp = texture_size * sin_angle/cos_angle;
		float fake_angle = angle * fake_opening / opening;
		float fake_cos_angle = cosf(fake_angle);
		float fake_sin_angle = sinf(fake_angle);
        for (j = 0; j < dome.slices_count; ++j)
        {
            float teta = j * 2.0 * M_PI / (float)dome.slices_count;
			float cos_teta = cosf(teta);
			float sin_teta = sinf(teta);
			int idx3 = idx*3;
            dome.vertices[idx3  ] = sin_angle * cos_teta * dome.real_radius; 
            dome.vertices[idx3+1] = sin_angle * sin_teta * dome.real_radius;
            dome.vertices[idx3+2] = z_pos;
			dome.normals[idx3  ] = fake_sin_angle * cos_teta;
			dome.normals[idx3+1] = fake_sin_angle * sin_teta;
			dome.normals[idx3+2] = fake_cos_angle;
			dome.tex_coords[idx*2  ] = 0.5 + tmp * cos_teta;
			dome.tex_coords[idx*2+1] = 0.5 + tmp * sin_teta;
			++idx;
        }
        angle -= first_angle * M_PI / 180.0;
		angle_step = angle / (float)dome.stacks_count;
		++i;
	}
	else
	{
		angle_step = angle / (float)(dome.stacks_count+1);
	}
    for (; i <= dome.stacks_count; ++i)
    {
		float cos_angle = cosf(angle);
		float sin_angle = sinf(angle);
        float z_pos = cos_angle * dome.real_radius + dome.height - dome.real_radius;
		float tmp = texture_size * sin_angle/cos_angle;
		float fake_angle = angle * fake_opening / opening;
		float fake_cos_angle = cosf(fake_angle);
		float fake_sin_angle = sinf(fake_angle);
        for (j = 0; j < dome.slices_count; ++j)
        {
            float teta = j * 2.0 * M_PI / (float)dome.slices_count;
			float cos_teta = cosf(teta);
			float sin_teta = sinf(teta);
			int idx3 = idx*3;
            dome.vertices[idx3  ] = sin_angle * cos_teta * dome.real_radius; 
            dome.vertices[idx3+1] = sin_angle * sin_teta * dome.real_radius;
            dome.vertices[idx3+2] = z_pos;
			dome.normals[idx3  ] = fake_sin_angle * cos_teta;
			dome.normals[idx3+1] = fake_sin_angle * sin_teta;
			dome.normals[idx3+2] = fake_cos_angle;
			dome.tex_coords[idx*2  ] = 0.5 + tmp * cos_teta;
			dome.tex_coords[idx*2+1] = 0.5 + tmp * sin_teta;
			++idx;
        }
        angle -= angle_step;
    }

    // the vertex at the top of the dome
    dome.vertices[idx*3  ] = 0.0;
    dome.vertices[idx*3+1] = 0.0;
    dome.vertices[idx*3+2] = dome.height;
	dome.normals[idx*3  ] = 0.0;
	dome.normals[idx*3+1] = 0.0;
	dome.normals[idx*3+2] = 1.0;
	dome.tex_coords[idx*2  ] = 0.5;
	dome.tex_coords[idx*2+1] = 0.5;

    // we build the faces of the dome
    idx = 0;
	vtx_idx = 0;
    for (i = 0; i < dome.stacks_count; ++i)
    {
        for (j = 0; j < dome.slices_count; ++j)
        {
            int next = (j+1)%dome.slices_count;
            dome.faces[idx++] = vtx_idx+j;
            dome.faces[idx++] = vtx_idx+j+dome.slices_count;
            dome.faces[idx++] = vtx_idx+next;
            dome.faces[idx++] = vtx_idx+next;
            dome.faces[idx++] = vtx_idx+j+dome.slices_count;
            dome.faces[idx++] = vtx_idx+next+dome.slices_count;
        }
        vtx_idx += dome.slices_count;
    }

    // the last stack of faces at the top of the dome
    for (j = 0; j < dome.slices_count; ++j)
    {
        int next = (j+1)%dome.slices_count;
        dome.faces[idx++] = vtx_idx+j;
        dome.faces[idx++] = vtx_idx+dome.slices_count;
        dome.faces[idx++] = vtx_idx+next;
    }

	return dome;
}

sky_sphere create_sphere(int slices, int stacks)
{
	sky_sphere sphere;
    int i, j;
    int idx, vtx_idx;

    sphere.vertices_count = (slices+1)*(stacks+1);
    sphere.faces_count = slices*stacks*2;

    sphere.vertices = (GLfloat*)malloc(3*sphere.vertices_count*sizeof(GLfloat));
    sphere.tex_coords = (GLfloat*)malloc(2*sphere.vertices_count*sizeof(GLfloat));
    sphere.faces = (GLuint*)malloc(3*sphere.faces_count*sizeof(GLuint));

    // we compute the vertices positions and normals
    idx = 0;
    for (j = 0; j <= stacks; ++j)
    {
		float angle = j * M_PI / (float)stacks;
		float cos_angle = cosf(angle);
		float sin_angle = sinf(angle);
		float tex_j = j / (float)stacks;
        for (i = 0; i <= slices; ++i)
        {
            float teta = i * 2.0 * M_PI / (float)slices;
			float cos_teta = cosf(teta);
			float sin_teta = sinf(teta);
			int idx3 = idx*3;
            sphere.vertices[idx3  ] = sin_angle * cos_teta; 
            sphere.vertices[idx3+1] = sin_angle * sin_teta;
            sphere.vertices[idx3+2] = cos_angle;
			sphere.tex_coords[idx*2  ] = i / (float)slices;
			sphere.tex_coords[idx*2+1] = tex_j;
			++idx;
        }
    }

    // we build the faces of the dome
    idx = 0;
	vtx_idx = 0;
    for (j = 0; j < stacks; ++j)
    {
        for (i = 0; i < slices; ++i)
        {
            sphere.faces[idx++] = vtx_idx + i;
            sphere.faces[idx++] = vtx_idx + i   + slices+1;
            sphere.faces[idx++] = vtx_idx + i+1;
            sphere.faces[idx++] = vtx_idx + i+1;
            sphere.faces[idx++] = vtx_idx + i   + slices+1;
            sphere.faces[idx++] = vtx_idx + i+1 + slices+1;
        }
        vtx_idx += slices+1;
    }

	return sphere;
}

void destroy_dome(sky_dome *dome)
{
    if (dome->vertices  ) { free(dome->vertices  ); dome->vertices   = NULL; }
    if (dome->normals   ) { free(dome->normals   ); dome->normals    = NULL; }
    if (dome->colors    ) { free(dome->colors    ); dome->colors     = NULL; }
    if (dome->tex_coords) { free(dome->tex_coords); dome->tex_coords = NULL; }
    if (dome->faces     ) { free(dome->faces     ); dome->faces      = NULL; }
}

void destroy_sphere(sky_sphere *sphere)
{
    if (sphere->vertices  ) { free(sphere->vertices  ); sphere->vertices   = NULL; }
    if (sphere->tex_coords) { free(sphere->tex_coords); sphere->tex_coords = NULL; }
    if (sphere->faces     ) { free(sphere->faces     ); sphere->faces      = NULL; }
}

static __inline__ void draw_sphere(sky_sphere *sphere)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, sphere->vertices);
	glNormalPointer(GL_FLOAT, 0, sphere->vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, sphere->tex_coords);

	glDrawElements(GL_TRIANGLES, sphere->faces_count*3, GL_UNSIGNED_INT, sphere->faces);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void skybox_compute_z_position()
{
	if (far_plane < 500.0)
	{
		float zl = first_person ? 0.0 : zoom_level;
		float cos_rx = cosf(-rx*M_PI/180.0);
		float sin_rx = sinf(-rx*M_PI/180.0);
		float eye_xy = sin_rx*zl*camera_distance;
		float eye_z = cos_rx*zl*camera_distance-camera_z;
		float water_end = sin_rx*far_plane+(far_plane*cos_rx-eye_z)*cos_rx/sin_rx;

		skybox_z = eye_z*(water_end-500.0+eye_xy)/water_end;

		//printf("camera_z=%f rx=%f zoom_level=%f camera_distance=%f eye_xy=%f eye_z=%f water_end=%f skybox_z=%f\n", camera_z, rx, zoom_level, camera_distance, eye_xy, eye_z, water_end, skybox_z);
	}
	else
		skybox_z = 0.0;
}

float skybox_get_z_position()
{
	return skybox_z;
}

static __inline__ void skybox_vertex_to_ground_coords(sky_dome *dome, int i, float *gx, float *gy)
{
	*gx = dome->tex_coords_conversion_factor * (dome->tex_coords[i*2  ] - dome->tex_coords[dome->vertices_count*2-2]);
	*gy = dome->tex_coords_conversion_factor * (dome->tex_coords[i*2+1] - dome->tex_coords[dome->vertices_count*2-1]);
}

void skybox_direction_to_ground_coords(float dir[3], float *gx, float *gy)
{
	float sd = sqrtf(dir[0]*dir[0] + dir[1]*dir[1]);
	float t = sd / (dir[2] - dome_sky.height + dome_sky.real_radius);
	float gd = t * dome_clouds.conversion_factor;

	*gx = gd * dir[0] / sd;
	*gy = gd * dir[1] / sd;

	//printf("dir=%f,%f,%f t=%f gx=%f gy=%f\n", dir[0], dir[1], dir[2], t, *gx, *gy);
}

void skybox_coords_from_ground_coords(float sky_coords[3], float gx, float gy)
{
	float gd = sqrtf(gx*gx + gy*gy);
	float t = gd / dome_clouds.conversion_factor;
	float sd = dome_clouds.real_radius * t / sqrtf(1.0 + t*t);

	sky_coords[0] = sd * gx / gd;
	sky_coords[1] = sd * gy / gd;
	sky_coords[2] = sd / t + dome_sky.height - dome_sky.real_radius;
}

void skybox_compute_element_projection(float proj[3], float pos[3])
{
	float coef, a, b, c, delta;
	float r2 = dome_sky.real_radius*dome_sky.real_radius;
	float z = dome_sky.height - dome_sky.real_radius;

	c = z*z - r2;

	if (pos[0] != 0.0)
	{
		coef = pos[2]/pos[0];
		a = 1.0 + coef*coef;
		b = -2.0*coef*z;
		delta = b*b - 4*a*c;
		if (delta <= 0.0) fprintf(stderr, "delta=%f\n", delta);

		if (pos[0] < 0.0)
			proj[0] = (-b - sqrtf(delta)) / (2.0*a);
		else
			proj[0] = (-b + sqrtf(delta)) / (2.0*a);
	}
	else proj[0] = 0.0;

	if (pos[1] != 0.0)
	{
		coef = pos[2]/pos[1];
		a = 1.0 + coef*coef;
		b = -2.0*coef*z;
		delta = b*b - 4*a*c;
		if (delta <= 0.0) fprintf(stderr, "delta=%f\n", delta);
		
		if (pos[1] < 0.0)
			proj[1] = (-b - sqrtf(delta)) / (2.0*a);
		else
			proj[1] = (-b + sqrtf(delta)) / (2.0*a);
	}
	else proj[1] = 0.0;

	proj[2] = sqrtf(r2 - proj[0]*proj[0] - proj[1]*proj[1]) + z;
}

float skybox_get_height(float x, float y)
{
	float d2 = dome_sky.real_radius*dome_sky.real_radius - x*x - y*y;
	float d = d2 <= 0.0 ? 0.0 : sqrtf(d2);
	return (d +	dome_sky.height - dome_sky.real_radius);
}

void skybox_set_type(skybox_type sky)
{
    current_sky = sky;
}

void cloudy_sky();
void underworld_sky();

void skybox_display()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(skybox_view);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// we center the sky on the camera, not on the world
	glTranslatef(-camera_x, -camera_y, 0.0);

	switch (current_sky)
	{
	case SKYBOX_CLOUDY:
		cloudy_sky();
		break;

	case SKYBOX_UNDERWORLD:
		underworld_sky();
		break;

	case SKYBOX_NONE:
	default:
		break;
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void update_cloudy_sky_positions();

void skybox_update_positions()
{
	switch(current_sky)
	{
	case SKYBOX_CLOUDY:
		update_cloudy_sky_positions();
		break;

	case SKYBOX_UNDERWORLD:
	case SKYBOX_NONE:
	default:
		break;
	}
}

void update_cloudy_sky_colors();
void update_cloudy_sky_local_colors();
void update_underworld_sky_colors();

void skybox_update_colors()
{
	switch(current_sky)
	{
	case SKYBOX_CLOUDY:
		if (skybox_local_weather)
			update_cloudy_sky_local_colors();
		else
			update_cloudy_sky_colors();
		break;

	case SKYBOX_UNDERWORLD:
		update_underworld_sky_colors();
		break;

	case SKYBOX_NONE:
		skybox_sky_color[0] = 0.0;
		skybox_sky_color[1] = 0.0;
		skybox_sky_color[2] = 0.0;
		skybox_sky_color[3] = 1.0;
		skybox_fog_color[0] = 0.0;
		skybox_fog_color[1] = 0.0;
		skybox_fog_color[2] = 0.0;
		skybox_fog_color[3] = 1.0;
		skybox_fog_density = 0.01;
	default:
		break;
	}
}

void update_cloudy_sky_positions()
{
	float rot1[9], rot2[9], rot3[9];
	float moon1_vect[3] = {0.0, 0.0, 1.0};
	float moon2_vect[3] = {0.0, 0.0, 1.0};

	if (is_day)
	{
		float dir[3] = {skybox_sun_position[0] * 480.0,
						skybox_sun_position[1] * 480.0,
						skybox_sun_position[2] * 480.0};
		skybox_direction_to_ground_coords(dir, &skybox_sun_projection[0], &skybox_sun_projection[1]);
	}
	else
	{
		skybox_sun_projection[0] = skybox_sun_projection[1] = 0.0;
	}

	moon_spin = cur_time % (1296000*1000);
	moon_spin *= 360.0/(1296000.0/*seconds in large month*/ * 1000.0/*millisecond bump*/);
	moon_spin += skybox_time_d;

	MAT3_ROT_Y(rot1, -((float)game_minute+(float)game_second/60.0)*M_PI/180.0);
	MAT3_ROT_Y(rot2, moon_spin*M_PI/180.0);
	MAT3_MULT(rot3, rot1, rot2);
	MAT3_VECT3_MULT(moon1_direction, rot3, moon1_vect);

	MAT3_ROT_Z(rot2, M_PI/9.0);
	MAT3_MULT(rot3, rot1, rot2);
	MAT3_ROT_Y(rot1, moon_spin*M_PI/18.0);
	MAT3_MULT(rot2, rot3, rot1);
	MAT3_VECT3_MULT(moon2_direction, rot2, moon2_vect);
}

static __inline__ float get_sky_sunlight(const GLfloat normal[3])
{
	GLfloat dot = normal[0]*sun_position[0]+normal[1]*sun_position[1]+normal[2]*sun_position[2];
    if (skybox_show_sun && !skybox_no_sun)
        return (dot > skybox_sunny_sky_bias ? (dot-skybox_sunny_sky_bias)/(1.0-skybox_sunny_sky_bias) : 0.0);
    else
        return 0.0;
}

static __inline__ float get_clouds_sunlight(const GLfloat normal[3])
{
	GLfloat dot = normal[0]*sun_position[0]+normal[1]*sun_position[1]+normal[2]*sun_position[2];
    if (skybox_show_sun && !skybox_no_sun)
        return (dot > skybox_sunny_clouds_bias ? (dot-skybox_sunny_clouds_bias)/(1.0-skybox_sunny_clouds_bias) : 0.0);
    else
        return 0.0;
}

static __inline__ float get_fog_sunlight(const GLfloat normal[3])
{
	GLfloat dot = normal[0]*sun_position[0]+normal[1]*sun_position[1]+normal[2]*sun_position[2];
    if (skybox_show_sun && !skybox_no_sun)
        return (dot > skybox_sunny_fog_bias ? (dot-skybox_sunny_fog_bias)/(1.0-skybox_sunny_fog_bias) : 0.0);
    else
        return 0.0;
}

static __inline__ float get_moonlight1(const GLfloat normal[3])
{
	GLfloat dot1 = normal[0]*moon1_direction[0]+normal[1]*moon1_direction[1]+normal[2]*moon1_direction[2];
	GLfloat dot2 = -(sun_position[0]*moon1_direction[0]+sun_position[1]*moon1_direction[1]+sun_position[2]*moon1_direction[2])*0.5+0.5;
	dot1 = (dot1 > skybox_moonlight1_bias ? (dot1-skybox_moonlight1_bias)/(1.0-skybox_moonlight1_bias) : 0.0);
    if (skybox_show_moons && !skybox_no_moons)
        return dot1*dot2;
    else
        return 0.0;
}

static __inline__ float get_moonlight2(const GLfloat normal[3])
{
	GLfloat dot1 = normal[0]*moon2_direction[0]+normal[1]*moon2_direction[1]+normal[2]*moon2_direction[2];
	GLfloat dot2 = -(sun_position[0]*moon2_direction[0]+sun_position[1]*moon2_direction[1]+sun_position[2]*moon2_direction[2])*0.5+0.5;
	dot1 = (dot1 > skybox_moonlight2_bias ? (dot1-skybox_moonlight2_bias)/(1.0-skybox_moonlight2_bias) : 0.0);
    if (skybox_show_moons && !skybox_no_moons)
        return dot1*dot2;
    else
        return 0.0;
}

void update_cloudy_sky_colors()
{
    int i, idx, end;
	GLfloat color_sun[4];
	GLfloat color_sky[4];
	GLfloat color[4];
	float abs_light;
	float *normal, ml1, ml2;
	float x, y, lg;

	abs_light = light_level;
	if(light_level > 59)
	{
		abs_light = 119 - light_level;
	}
	abs_light = 1.0f - abs_light/59.0f;

	rain_coef = weather_get_density();

	// alpha adjustment for objects that should fade in daylight
	day_alpha = (1.0-abs_light)*(1.0-rain_coef);
	
	// color of the moons
	moon1_color[0] = 0.9;
	moon1_color[1] = 0.9;
	moon1_color[2] = 0.9;
	moon2_color[0] = 0.9;
	moon2_color[1] = 0.8;
	moon2_color[2] = 0.7;

	// color of the sun
	skybox_get_current_color(sun_color, skybox_sun);
	sun_color[0] *= 1.0 - rain_coef;
	sun_color[1] *= 1.0 - rain_coef;
	sun_color[2] *= 1.0 - rain_coef;
	sun_color[3] *= 1.0 - rain_coef;

	// color of the light
	skybox_blend_current_colors(skybox_light_ambient_color, skybox_light_ambient, skybox_light_ambient_rainy, rain_coef);
	skybox_blend_current_colors(skybox_light_diffuse_color, skybox_light_diffuse, skybox_light_diffuse_rainy, rain_coef);

	idx = 0;

    // we compute the color and the density of the fog
	skybox_blend_current_colors(skybox_fog_color, skybox_fog, skybox_fog_rainy, rain_coef);
	if (rain_coef > 0.0)
	{
		float fog_density;
		blend_colors(&fog_density, &skybox_fog[game_minute][3], &skybox_fog[(game_minute+1)%360][3], (float)game_second/60.0, 1);
		skybox_fog_color[0] *= (1.0-rain_coef) + rain_coef*weather_color[0];
		skybox_fog_color[1] *= (1.0-rain_coef) + rain_coef*weather_color[1];
		skybox_fog_color[2] *= (1.0-rain_coef) + rain_coef*weather_color[2];
		skybox_fog_color[3] = (1.0-rain_coef)*fog_density + rain_coef*skybox_fog_color[3];
	}
	skybox_fog_density = skybox_fog_color[3];
	skybox_fog_color[3] = 1.0;

    // we compute the colors of the fog around the dome according to the sun and moons positions
    skybox_blend_current_colors(color_sun, skybox_fog_sunny, skybox_fog_rainy, rain_coef);
    for (i = 0; i < dome_sky.slices_count; ++i)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		blend_colors(color, skybox_fog_color, color_sun, get_fog_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*weather_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*weather_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*weather_color[2];

		if (lightning_falling) {
			skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
			lg = weather_get_lightning_intensity(x-camera_x, y-camera_y);
			color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
			color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
			color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;
		}

		fog_colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		fog_colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		fog_colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
    }

	i = idx = 0;

	// clouds color
	skybox_blend_current_colors(color_sky, skybox_clouds, skybox_clouds_rainy, rain_coef);
	skybox_blend_current_colors(color_sun, skybox_clouds_sunny, skybox_clouds_rainy, rain_coef);
    while (i < dome_clouds.slices_count * 2)
    {
		normal = &dome_clouds.normals[i*3];
		ml1 = get_moonlight1(normal)*0.3*day_alpha;
		ml2 = get_moonlight2(normal)*0.2*day_alpha;

		blend_colors(color, color_sky, color_sun, get_clouds_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*weather_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*weather_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*weather_color[2];

		if (lightning_falling) {
			skybox_vertex_to_ground_coords(&dome_clouds, i, &x, &y);
			lg = weather_get_lightning_intensity(x-camera_x, y-camera_y);
			color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
			color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
			color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;
		}

		dome_clouds.colors[idx] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_clouds_colors_bis[idx++] = color[0];
		dome_clouds.colors[idx] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_clouds_colors_bis[idx++] = color[1];
		dome_clouds.colors[idx] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_clouds_colors_bis[idx++] = color[2];
        dome_clouds.colors[idx] = 0.0;
		dome_clouds_colors_bis[idx++] = 0.0;
		++i;
    }
    while (i < dome_clouds.vertices_count)
    {
		normal = &dome_clouds.normals[i*3];
		ml1 = get_moonlight1(normal)*0.3*day_alpha;
		ml2 = get_moonlight2(normal)*0.2*day_alpha;

		blend_colors(color, color_sky, color_sun, get_clouds_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*weather_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*weather_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*weather_color[2];
		
		if (lightning_falling) {
			skybox_vertex_to_ground_coords(&dome_clouds, i, &x, &y);
			lg = weather_get_lightning_intensity(x-camera_x, y-camera_y);
			color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
			color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
			color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;
		}

		dome_clouds.colors[idx] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_clouds_colors_bis[idx++] = color[0];
		dome_clouds.colors[idx] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_clouds_colors_bis[idx++] = color[1];
		dome_clouds.colors[idx] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_clouds_colors_bis[idx++] = color[2];
        dome_clouds.colors[idx] = 1.0;
		dome_clouds_colors_bis[idx++] = rain_coef;
		++i;
    }

	i = idx = 0;

	// clouds detail color
	skybox_blend_current_colors(color_sky, skybox_clouds_detail, skybox_clouds_detail_rainy, rain_coef);
	skybox_blend_current_colors(color_sun, skybox_clouds_detail_sunny, skybox_clouds_detail_rainy, rain_coef);
    while (i < dome_clouds.slices_count * 2)
    {
		blend_colors(color, color_sky, color_sun, get_clouds_sunlight(&dome_clouds.normals[i*3]), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*weather_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*weather_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*weather_color[2];

		if (lightning_falling) {
			skybox_vertex_to_ground_coords(&dome_clouds, i, &x, &y);
			lg = weather_get_lightning_intensity(x-camera_x, y-camera_y);
			color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
			color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
			color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;
		}

		dome_clouds_detail_colors[idx] = color[0];
		dome_clouds_detail_colors_bis[idx++] = color[0];
		dome_clouds_detail_colors[idx] = color[1];
		dome_clouds_detail_colors_bis[idx++] = color[1];
		dome_clouds_detail_colors[idx] = color[2];
		dome_clouds_detail_colors_bis[idx++] = color[2];
        dome_clouds_detail_colors[idx] = 0.0;
		dome_clouds_detail_colors_bis[idx++] = 0.0;
		++i;
    }
    while (i < dome_clouds.vertices_count)
    {
		blend_colors(color, color_sky, color_sun, get_clouds_sunlight(&dome_clouds.normals[i*3]), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*weather_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*weather_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*weather_color[2];

		if (lightning_falling) {
			skybox_vertex_to_ground_coords(&dome_clouds, i, &x, &y);
			lg = weather_get_lightning_intensity(x-camera_x, y-camera_y);
			color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
			color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
			color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;
		}

		dome_clouds_detail_colors[idx] = color[0];
		dome_clouds_detail_colors_bis[idx++] = color[0];
		dome_clouds_detail_colors[idx] = color[1];
		dome_clouds_detail_colors_bis[idx++] = color[1];
		dome_clouds_detail_colors[idx] = color[2];
		dome_clouds_detail_colors_bis[idx++] = color[2];
        dome_clouds_detail_colors[idx] = 1.0;
		dome_clouds_detail_colors_bis[idx++] = rain_coef;
		++i;
    }

	i = idx = 0;

	// sky color
	skybox_blend_current_colors(color_sky, skybox_sky1, skybox_fog_rainy, rain_coef);
	skybox_blend_current_colors(color_sun, skybox_sky1_sunny, skybox_fog_rainy, rain_coef);
	end = dome_sky.slices_count;
    while (i < end)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		blend_colors(color, color_sky, color_sun, get_sky_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*weather_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*weather_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*weather_color[2];

		if (lightning_falling) {
			skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
			lg = weather_get_lightning_intensity(x-camera_x, y-camera_y);
			color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
			color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
			color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;
		}

		dome_sky.colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_sky.colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_sky.colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_sky.colors[idx++] = 1.0;
		++i;
    }

	skybox_blend_current_colors(color_sky, skybox_sky2, skybox_fog_rainy, rain_coef);
	skybox_blend_current_colors(color_sun, skybox_sky2_sunny, skybox_fog_rainy, rain_coef);
	end += dome_sky.slices_count;
    while (i < end)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		blend_colors(color, color_sky, color_sun, get_sky_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*weather_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*weather_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*weather_color[2];

		if (lightning_falling) {
			skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
			lg = weather_get_lightning_intensity(x-camera_x, y-camera_y);
			color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
			color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
			color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;
		}

		dome_sky.colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_sky.colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_sky.colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_sky.colors[idx++] = 1.0;
		++i;
    }

	skybox_blend_current_colors(color_sky, skybox_sky3, skybox_fog_rainy, rain_coef);
	skybox_blend_current_colors(color_sun, skybox_sky3_sunny, skybox_fog_rainy, rain_coef);
	end += dome_sky.slices_count;
    while (i < end)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		blend_colors(color, color_sky, color_sun, get_sky_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*weather_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*weather_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*weather_color[2];

		if (lightning_falling) {
			skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
			lg = weather_get_lightning_intensity(x-camera_x, y-camera_y);
			color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
			color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
			color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;
		}

		dome_sky.colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_sky.colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_sky.colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_sky.colors[idx++] = 1.0;
		++i;
    }

	skybox_blend_current_colors(color_sky, skybox_sky4, skybox_fog_rainy, rain_coef);
	skybox_blend_current_colors(color_sun, skybox_sky4_sunny, skybox_fog_rainy, rain_coef);
	end += dome_sky.slices_count;
    while (i < end)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		blend_colors(color, color_sky, color_sun, get_sky_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*weather_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*weather_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*weather_color[2];

		if (lightning_falling) {
			skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
			lg = weather_get_lightning_intensity(x-camera_x, y-camera_y);
			color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
			color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
			color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;
		}

		dome_sky.colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_sky.colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_sky.colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_sky.colors[idx++] = 1.0;
		++i;
    }

	skybox_blend_current_colors(color_sky, skybox_sky5, skybox_fog_rainy, rain_coef);
	skybox_blend_current_colors(color_sun, skybox_sky5_sunny, skybox_fog_rainy, rain_coef);
    while (i < dome_sky.vertices_count)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		blend_colors(color, color_sky, color_sun, get_sky_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*weather_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*weather_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*weather_color[2];

		if (lightning_falling) {
			skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
			lg = weather_get_lightning_intensity(x-camera_x, y-camera_y);
			color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
			color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
			color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;
		}

		dome_sky.colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_sky.colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_sky.colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_sky.colors[idx++] = 1.0;
		++i;
    }

	memcpy(skybox_sky_color, color_sun, 4*sizeof(float));

	// color of the moons update
	moon1_color[0] *= (0.5 + 0.5*day_alpha)*(1.0-rain_coef);
	moon1_color[1] *= (0.5 + 0.5*day_alpha)*(1.0-rain_coef);
	moon1_color[2] *= (0.5 + 0.5*day_alpha)*(1.0-rain_coef);
	moon2_color[0] *= (0.5 + 0.5*day_alpha)*(1.0-rain_coef);
	moon2_color[1] *= (0.5 + 0.5*day_alpha)*(1.0-rain_coef);
	moon2_color[2] *= (0.5 + 0.5*day_alpha)*(1.0-rain_coef);
}

void update_cloudy_sky_local_colors()
{
    int i, idx, end;
	GLfloat color_sun[4];
	GLfloat color_sky[4];
	GLfloat color[4];
	GLfloat local_color[4];
	float abs_light;
	float ratios[MAX_WEATHER_TYPES];
	float *normal, ml1, ml2, x, y, lg;

	abs_light = light_level;
	if(light_level > 59)
	{
		abs_light = 119 - light_level;
	}
	abs_light = 1.0f - abs_light/59.0f;

	// alpha adjustment for objects that should fade in daylight
	day_alpha = (1.0-abs_light);
	
	// color of the moons
	moon1_color[0] = 0.9;
	moon1_color[1] = 0.9;
	moon1_color[2] = 0.9;
	moon2_color[0] = 0.9;
	moon2_color[1] = 0.8;
	moon2_color[2] = 0.7;

	// color of the sun
	skybox_get_current_color(sun_color, skybox_sun);
	weather_compute_ratios(ratios, skybox_sun_projection[0]-camera_x, skybox_sun_projection[1]-camera_y);
	rain_coef = weather_get_density_from_ratios(ratios);
	sun_color[0] *= 1.0 - rain_coef;
	sun_color[1] *= 1.0 - rain_coef;
	sun_color[2] *= 1.0 - rain_coef;
	sun_color[3] *= 1.0 - rain_coef;

	// color of the light
	skybox_blend_current_colors(skybox_light_ambient_color, skybox_light_ambient, skybox_light_ambient_rainy, rain_coef);
	skybox_blend_current_colors(skybox_light_diffuse_color, skybox_light_diffuse, skybox_light_diffuse_rainy, rain_coef);

	idx = 0;

    // we compute the color and the density of the fog
	rain_coef = weather_get_intensity();
	skybox_blend_current_colors(skybox_fog_color, skybox_fog, skybox_fog_rainy, rain_coef);
	if (rain_coef > 0.0)
	{
		float fog_density;
		rain_coef = weather_get_density();
		blend_colors(&fog_density, &skybox_fog[game_minute][3], &skybox_fog[(game_minute+1)%360][3], (float)game_second/60.0, 1);
		skybox_fog_color[0] *= (1.0-rain_coef) + rain_coef*weather_color[0];
		skybox_fog_color[1] *= (1.0-rain_coef) + rain_coef*weather_color[1];
		skybox_fog_color[2] *= (1.0-rain_coef) + rain_coef*weather_color[2];
		skybox_fog_color[3] = (1.0-rain_coef)*fog_density + rain_coef*skybox_fog_color[3];
	}
	skybox_fog_density = skybox_fog_color[3];
	skybox_fog_color[3] = 1.0;

    // we compute the colors of the fog around the dome according to the sun and moons positions
    for (i = 0; i < dome_sky.slices_count; ++i)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
		x -= camera_x;
		y -= camera_y;
		lg = weather_get_lightning_intensity(x, y);
		weather_compute_ratios(ratios, x, y);
		rain_coef = weather_get_density_from_ratios(ratios);
		weather_get_color_from_ratios(local_color, ratios);

		ml1 *= 1.0 - rain_coef;
		ml2 *= 1.0 - rain_coef;

		skybox_blend_current_colors(color_sun, skybox_fog_sunny, skybox_fog_rainy, rain_coef);
		blend_colors(color, skybox_fog_color, color_sun, get_fog_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*local_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*local_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*local_color[2];

		color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
		color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
		color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;

		fog_colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		fog_colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		fog_colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
    }

	i = idx = 0;

	// clouds color
    while (i < dome_clouds.slices_count * 2)
    {
		normal = &dome_clouds.normals[i*3];
		ml1 = get_moonlight1(normal)*0.3*day_alpha;
		ml2 = get_moonlight2(normal)*0.2*day_alpha;

		skybox_vertex_to_ground_coords(&dome_clouds, i, &x, &y);
		x -= camera_x;
		y -= camera_y;
		lg = weather_get_lightning_intensity(x, y);
		weather_compute_ratios(ratios, x, y);
		rain_coef = weather_get_density_from_ratios(ratios);
		weather_get_color_from_ratios(local_color, ratios);

		ml1 *= 1.0 - rain_coef;
		ml2 *= 1.0 - rain_coef;

		skybox_blend_current_colors(color_sky, skybox_clouds, skybox_clouds_rainy, rain_coef);
		skybox_blend_current_colors(color_sun, skybox_clouds_sunny, skybox_clouds_rainy, rain_coef);

		blend_colors(color, color_sky, color_sun, get_clouds_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*local_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*local_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*local_color[2];

		color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
		color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
		color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;

		dome_clouds.colors[idx] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_clouds_colors_bis[idx++] = color[0];
		dome_clouds.colors[idx] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_clouds_colors_bis[idx++] = color[1];
		dome_clouds.colors[idx] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_clouds_colors_bis[idx++] = color[2];
        dome_clouds.colors[idx] = 0.0;
		dome_clouds_colors_bis[idx++] = rain_coef;
		++i;
    }
    while (i < dome_clouds.vertices_count)
    {
		normal = &dome_clouds.normals[i*3];
		ml1 = get_moonlight1(normal)*0.3*day_alpha;
		ml2 = get_moonlight2(normal)*0.2*day_alpha;

		skybox_vertex_to_ground_coords(&dome_clouds, i, &x, &y);
		x -= camera_x;
		y -= camera_y;
		lg = weather_get_lightning_intensity(x, y);
		weather_compute_ratios(ratios, x, y);
		rain_coef = weather_get_density_from_ratios(ratios);
		weather_get_color_from_ratios(local_color, ratios);

		ml1 *= 1.0 - rain_coef;
		ml2 *= 1.0 - rain_coef;

		skybox_blend_current_colors(color_sky, skybox_clouds, skybox_clouds_rainy, rain_coef);
		skybox_blend_current_colors(color_sun, skybox_clouds_sunny, skybox_clouds_rainy, rain_coef);

		blend_colors(color, color_sky, color_sun, get_clouds_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*local_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*local_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*local_color[2];

		color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
		color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
		color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;

		dome_clouds.colors[idx] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_clouds_colors_bis[idx++] = color[0];
		dome_clouds.colors[idx] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_clouds_colors_bis[idx++] = color[1];
		dome_clouds.colors[idx] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_clouds_colors_bis[idx++] = color[2];
        dome_clouds.colors[idx] = 1.0;
		dome_clouds_colors_bis[idx++] = rain_coef;
		++i;
    }

	i = idx = 0;

	// clouds detail color
    while (i < dome_clouds.slices_count * 2)
    {
		skybox_vertex_to_ground_coords(&dome_clouds, i, &x, &y);
		x -= camera_x;
		y -= camera_y;
		weather_compute_ratios(ratios, x, y);
		rain_coef = weather_get_density_from_ratios(ratios);
		weather_get_color_from_ratios(local_color, ratios);

		skybox_blend_current_colors(color_sky, skybox_clouds_detail, skybox_clouds_detail_rainy, rain_coef);
		skybox_blend_current_colors(color_sun, skybox_clouds_detail_sunny, skybox_clouds_detail_rainy, rain_coef);

		blend_colors(color, color_sky, color_sun, get_clouds_sunlight(&dome_clouds.normals[i*3]), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*local_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*local_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*local_color[2];

		dome_clouds_detail_colors[idx] = color[0];
		dome_clouds_detail_colors_bis[idx++] = color[0];
		dome_clouds_detail_colors[idx] = color[1];
		dome_clouds_detail_colors_bis[idx++] = color[1];
		dome_clouds_detail_colors[idx] = color[2];
		dome_clouds_detail_colors_bis[idx++] = color[2];
        dome_clouds_detail_colors[idx] = 0.0;
		dome_clouds_detail_colors_bis[idx++] = 0.0;
		++i;
    }
    while (i < dome_clouds.vertices_count)
    {
		skybox_vertex_to_ground_coords(&dome_clouds, i, &x, &y);
		x -= camera_x;
		y -= camera_y;
		weather_compute_ratios(ratios, x, y);
		rain_coef = weather_get_density_from_ratios(ratios);
		weather_get_color_from_ratios(local_color, ratios);

		skybox_blend_current_colors(color_sky, skybox_clouds_detail, skybox_clouds_detail_rainy, rain_coef);
		skybox_blend_current_colors(color_sun, skybox_clouds_detail_sunny, skybox_clouds_detail_rainy, rain_coef);

		blend_colors(color, color_sky, color_sun, get_clouds_sunlight(&dome_clouds.normals[i*3]), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*local_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*local_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*local_color[2];

		dome_clouds_detail_colors[idx] = color[0];
		dome_clouds_detail_colors_bis[idx++] = color[0];
		dome_clouds_detail_colors[idx] = color[1];
		dome_clouds_detail_colors_bis[idx++] = color[1];
		dome_clouds_detail_colors[idx] = color[2];
		dome_clouds_detail_colors_bis[idx++] = color[2];
        dome_clouds_detail_colors[idx] = 1.0;
		dome_clouds_detail_colors_bis[idx++] = rain_coef;
		++i;
    }

	i = idx = 0;

	// sky color
	end = dome_sky.slices_count;
    while (i < end)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
		x -= camera_x;
		y -= camera_y;
		lg = weather_get_lightning_intensity(x, y);
		weather_compute_ratios(ratios, x, y);
		rain_coef = weather_get_density_from_ratios(ratios);
		weather_get_color_from_ratios(local_color, ratios);

		ml1 *= 1.0 - rain_coef;
		ml2 *= 1.0 - rain_coef;

		skybox_blend_current_colors(color_sky, skybox_sky1, skybox_fog_rainy, rain_coef);
		skybox_blend_current_colors(color_sun, skybox_sky1_sunny, skybox_fog_rainy, rain_coef);

		blend_colors(color, color_sky, color_sun, get_sky_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*local_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*local_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*local_color[2];

		color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
		color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
		color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;

		dome_sky.colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_sky.colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_sky.colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_sky.colors[idx++] = 1.0;
		++i;
    }

	end += dome_sky.slices_count;
    while (i < end)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
		x -= camera_x;
		y -= camera_y;
		lg = weather_get_lightning_intensity(x, y);
		weather_compute_ratios(ratios, x, y);
		rain_coef = weather_get_density_from_ratios(ratios);
		weather_get_color_from_ratios(local_color, ratios);

		ml1 *= 1.0 - rain_coef;
		ml2 *= 1.0 - rain_coef;

		skybox_blend_current_colors(color_sky, skybox_sky2, skybox_fog_rainy, rain_coef);
		skybox_blend_current_colors(color_sun, skybox_sky2_sunny, skybox_fog_rainy, rain_coef);

		blend_colors(color, color_sky, color_sun, get_sky_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*local_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*local_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*local_color[2];

		color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
		color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
		color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;

		dome_sky.colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_sky.colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_sky.colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_sky.colors[idx++] = 1.0;
		++i;
    }

	end += dome_sky.slices_count;
    while (i < end)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
		x -= camera_x;
		y -= camera_y;
		lg = weather_get_lightning_intensity(x, y);
		weather_compute_ratios(ratios, x, y);
		rain_coef = weather_get_density_from_ratios(ratios);
		weather_get_color_from_ratios(local_color, ratios);

		ml1 *= 1.0 - rain_coef;
		ml2 *= 1.0 - rain_coef;

		skybox_blend_current_colors(color_sky, skybox_sky3, skybox_fog_rainy, rain_coef);
		skybox_blend_current_colors(color_sun, skybox_sky3_sunny, skybox_fog_rainy, rain_coef);

		blend_colors(color, color_sky, color_sun, get_sky_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*local_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*local_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*local_color[2];

		color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
		color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
		color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;

		dome_sky.colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_sky.colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_sky.colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_sky.colors[idx++] = 1.0;
		++i;
    }

	end += dome_sky.slices_count;
    while (i < end)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
		x -= camera_x;
		y -= camera_y;
		lg = weather_get_lightning_intensity(x, y);
		weather_compute_ratios(ratios, x, y);
		rain_coef = weather_get_density_from_ratios(ratios);
		weather_get_color_from_ratios(local_color, ratios);

		ml1 *= 1.0 - rain_coef;
		ml2 *= 1.0 - rain_coef;

		skybox_blend_current_colors(color_sky, skybox_sky4, skybox_fog_rainy, rain_coef);
		skybox_blend_current_colors(color_sun, skybox_sky4_sunny, skybox_fog_rainy, rain_coef);
	
		blend_colors(color, color_sky, color_sun, get_sky_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*local_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*local_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*local_color[2];

		color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
		color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
		color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;

		dome_sky.colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_sky.colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_sky.colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_sky.colors[idx++] = 1.0;
		++i;
    }

    while (i < dome_sky.vertices_count)
    {
		normal = &dome_sky.normals[i*3];
		ml1 = get_moonlight1(normal)*0.15*day_alpha;
		ml2 = get_moonlight2(normal)*0.1*day_alpha;

		skybox_vertex_to_ground_coords(&dome_sky, i, &x, &y);
		x -= camera_x;
		y -= camera_y;
		lg = weather_get_lightning_intensity(x, y);
		weather_compute_ratios(ratios, x, y);
		rain_coef = weather_get_density_from_ratios(ratios);
		weather_get_color_from_ratios(local_color, ratios);

		ml1 *= 1.0 - rain_coef;
		ml2 *= 1.0 - rain_coef;

		skybox_blend_current_colors(color_sky, skybox_sky5, skybox_fog_rainy, rain_coef);
		skybox_blend_current_colors(color_sun, skybox_sky5_sunny, skybox_fog_rainy, rain_coef);
	
		blend_colors(color, color_sky, color_sun, get_sky_sunlight(normal), 3);

		color[0] *= (1.0 - rain_coef) + rain_coef*local_color[0];
		color[1] *= (1.0 - rain_coef) + rain_coef*local_color[1];
		color[2] *= (1.0 - rain_coef) + rain_coef*local_color[2];

		color[0] = color[0]*(1.0-lg) + lightning_color[0]*lg;
		color[1] = color[1]*(1.0-lg) + lightning_color[1]*lg;
		color[2] = color[2]*(1.0-lg) + lightning_color[2]*lg;

		dome_sky.colors[idx++] = color[0] + ml1*moon1_color[0] + ml2*moon2_color[0];
		dome_sky.colors[idx++] = color[1] + ml1*moon1_color[1] + ml2*moon2_color[1];
		dome_sky.colors[idx++] = color[2] + ml1*moon1_color[2] + ml2*moon2_color[2];
		dome_sky.colors[idx++] = 1.0;
		++i;
    }

	skybox_blend_current_colors(skybox_sky_color, skybox_sky5_sunny, skybox_fog_rainy, weather_get_density());

	// color of the moons update
	moon1_color[0] *= (0.5 + 0.5*day_alpha);
	moon1_color[1] *= (0.5 + 0.5*day_alpha);
	moon1_color[2] *= (0.5 + 0.5*day_alpha);
	moon2_color[0] *= (0.5 + 0.5*day_alpha);
	moon2_color[1] *= (0.5 + 0.5*day_alpha);
	moon2_color[2] *= (0.5 + 0.5*day_alpha);
}

void update_underworld_sky_colors()
{
	int i, idx, end;

	i = idx = 0;

	end = dome_sky.slices_count;
	while (i < end)
	{
		dome_sky.colors[idx++] = 230/255.0;
		dome_sky.colors[idx++] = 104/255.0;
		dome_sky.colors[idx++] =  11/255.0;
		dome_sky.colors[idx++] = 1.0;
		++i;
	}

	end += dome_sky.slices_count;
	while (i < end)
	{
		dome_sky.colors[idx++] = 230/255.0;
		dome_sky.colors[idx++] = 104/255.0;
		dome_sky.colors[idx++] =  11/255.0;
		dome_sky.colors[idx++] = 1.0;
		++i;
	}

	end += dome_sky.slices_count;
	while (i < end)
	{
		dome_sky.colors[idx++] = 201/255.0;
		dome_sky.colors[idx++] =  58/255.0;
		dome_sky.colors[idx++] =  13/255.0;
		dome_sky.colors[idx++] = 0.8;
		++i;
	}

	end += dome_sky.slices_count;
	while (i < end)
	{
		dome_sky.colors[idx++] =  83/255.0;
		dome_sky.colors[idx++] =  31/255.0;
		dome_sky.colors[idx++] =  23/255.0;
		dome_sky.colors[idx++] = 0.6;
		++i;
	}

    while (i < dome_sky.vertices_count)
    {
		dome_sky.colors[idx++] =  33/255.0;
		dome_sky.colors[idx++] =   6/255.0;
		dome_sky.colors[idx++] =  16/255.0;
		dome_sky.colors[idx++] = 0.0;
		++i;
	}

	memcpy(skybox_fog_color, &dome_sky.colors[(dome_sky.vertices_count-1)*4], 3*sizeof(float));
	skybox_fog_color[3] = 1.0;
	skybox_fog_density = 0.01;
	memcpy(skybox_sky_color, skybox_fog_color, 4*sizeof(float));
}

void draw_horizon_fog(float rain_coef)
{
    const GLfloat fade_values[] = {1.0, 0.75, 0.4, 0.0};
    int i;

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < dome_sky.slices_count; ++i)
    {
        const GLfloat *vtx = &dome_sky.vertices[i*3];
        const GLfloat *col = &fog_colors[i*3];
        glColor4f(col[0], col[1], col[2], fade_values[0]);
        glVertex3f(vtx[0], vtx[1], vtx[2]);
        glColor4f(col[0], col[1], col[2], fade_values[1]);
        glVertex3f(vtx[0], vtx[1], vtx[2] + 15.0);
    }
    glColor4f(fog_colors[0], fog_colors[1], fog_colors[2], fade_values[0]);
    glVertex3f(dome_sky.vertices[0], dome_sky.vertices[1], dome_sky.vertices[2]);
    glColor4f(fog_colors[0], fog_colors[1], fog_colors[2], fade_values[1]);
    glVertex3f(dome_sky.vertices[0], dome_sky.vertices[1], dome_sky.vertices[2] + 15.0);
    glEnd();

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < dome_sky.slices_count; ++i)
    {
        const GLfloat *vtx = &dome_sky.vertices[i*3];
        const GLfloat *col = &fog_colors[i*3];
        glColor4f(col[0], col[1], col[2], fade_values[1]);
        glVertex3f(vtx[0], vtx[1], vtx[2] + 15.0);
        glColor4f(col[0], col[1], col[2], fade_values[2]);
        glVertex3f(vtx[0], vtx[1], vtx[2] + 30.0);
    }
    glColor4f(fog_colors[0], fog_colors[1], fog_colors[2], fade_values[1]);
    glVertex3f(dome_sky.vertices[0], dome_sky.vertices[1], dome_sky.vertices[2] + 15.0);
    glColor4f(fog_colors[0], fog_colors[1], fog_colors[2], fade_values[2]);
    glVertex3f(dome_sky.vertices[0], dome_sky.vertices[1], dome_sky.vertices[2] + 30.0);
    glEnd();
    
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < dome_sky.slices_count; ++i)
    {
        const GLfloat *vtx = &dome_sky.vertices[i*3];
        const GLfloat *col = &fog_colors[i*3];
        glColor4f(col[0], col[1], col[2], fade_values[2]);
        glVertex3f(vtx[0], vtx[1], vtx[2] + 30.0);
        glColor4f(col[0], col[1], col[2], fade_values[3]);
        glVertex3f(vtx[0], vtx[1], vtx[2] + 45.0);
    }
    glColor4f(fog_colors[0], fog_colors[1], fog_colors[2], fade_values[2]);
    glVertex3f(dome_sky.vertices[0], dome_sky.vertices[1], dome_sky.vertices[2] + 30.0);
    glColor4f(fog_colors[0], fog_colors[1], fog_colors[2], fade_values[3]);
    glVertex3f(dome_sky.vertices[0], dome_sky.vertices[1], dome_sky.vertices[2] + 45.0);
    glEnd();
}

void cloudy_sky()
{
	static Uint32 last_cloud_time = 0;
	int i;
	GLfloat black_color[] = {0.0, 0.0, 0.0, 0.0};

	// disable lights not used for sky just in case
	switch(show_lights)
	{
	case 6:
		glDisable(GL_LIGHT6);
	case 5:
		glDisable(GL_LIGHT5);
	case 4:
		glDisable(GL_LIGHT4);
	case 3:
		glDisable(GL_LIGHT3);
	case 2:
		glDisable(GL_LIGHT2);
	case 1:
		glDisable(GL_LIGHT1);
	default:
		glDisable(GL_LIGHT0);
		break;
	}

	glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);

	glEnable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
	glEnable(GL_COLOR_MATERIAL);

    glDisable(GL_DEPTH_TEST);

	// we draw a ring to continue the sky a bit under the horizon
    if (skybox_show_horizon_fog)
    {
        glBegin(GL_QUAD_STRIP);
        for (i = 0; i < dome_sky.slices_count; ++i)
		{
            const GLfloat *vtx = &dome_sky.vertices[i*3];
            glColor3fv(&fog_colors[i*3]);
            glVertex3f(vtx[0]*0.9, vtx[1]*0.9, -dome_sky.height*0.1);
            glVertex3fv(vtx);
        }
        glColor4fv(&fog_colors[0]);
        glVertex3f(dome_sky.vertices[0]*0.9, dome_sky.vertices[1]*0.9, -dome_sky.height*0.1);
        glVertex3fv(&dome_sky.vertices[0]);
        glEnd();
    }
    else
    {
        glBegin(GL_QUAD_STRIP);
        for (i = 0; i < dome_sky.slices_count; ++i)
		{
            const GLfloat *vtx = &dome_sky.vertices[i*3];
            glColor3fv(&dome_sky.colors[i*4]);
            glVertex3f(vtx[0]*0.9, vtx[1]*0.9, -dome_sky.height*0.1);
            glVertex3fv(vtx);
        }
        glColor3fv(&dome_sky.colors[0]);
        glVertex3f(dome_sky.vertices[0]*0.9, dome_sky.vertices[1]*0.9, -dome_sky.height*0.1);
        glVertex3fv(&dome_sky.vertices[0]);
        glEnd();
    }

	// we draw the sky background
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, dome_sky.vertices);
    glColorPointer(4, GL_FLOAT, 0, dome_sky.colors);

    glDrawElements(GL_TRIANGLES, dome_sky.faces_count*3, GL_UNSIGNED_INT, dome_sky.faces);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

	glEnable(GL_BLEND);

    if (skybox_show_horizon_fog)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        draw_horizon_fog(rain_coef);
    }

    glEnable(GL_DEPTH_TEST); // we need it to draw moons and stars
    glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_COLOR, GL_ONE);

	// we draw the moons
	if(skybox_show_moons && !skybox_no_moons)
	{
		// the current light color is black so we change it to light the moons
		GLfloat light_color[] = {0.8, 0.8, 0.8, 1.0};
		glLightfv(GL_LIGHT7, GL_DIFFUSE, light_color);
		
		glPushMatrix();
		glRotatef((float)game_minute+(float)game_second/60.0, 0.0, -1.0, 0.0);
		
		glEnable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
#ifdef	NEW_TEXTURES
		bind_texture(moon_tex);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(moon_tex);
#endif	/* NEW_TEXTURES */
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black_color);

		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, moon1_color);

		glPushMatrix();
		glRotatef(moon_spin, 0.0, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 450.0);
		glRotatef(90.0, 1.0, 0.0, 0.0);
		glRotatef(moon_spin, 0.0, 0.0, 1.0);
		glScalef(20.0, 20.0, 20.0);
		//glCallList(sky_lists+3);
		draw_sphere(&moon_mesh);
		glPopMatrix();
		
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, moon2_color);
	
		glPushMatrix();
		glRotatef(20.0, 0.0, 0.0, 1.0);
		glRotatef(10.0*moon_spin, 0.0, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 480.0);
		glRotatef(90.0, 1.0, 0.0, 0.0);
		glRotatef(10.0*moon_spin, 0.0, 0.0, 1.0);
		glScalef(12.0, 12.0, 12.0);
		//glCallList(sky_lists+3);
		draw_sphere(&moon_mesh);
		glPopMatrix();
		
		glPopMatrix();
		glLightfv(GL_LIGHT7, GL_DIFFUSE, diffuse_light); // we restore the light color
		glEnable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHTING);
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // we don't want the stars to update the depth buffer
    glDisable(GL_TEXTURE_2D);

	// we draw the stars
	if (skybox_show_stars && !skybox_no_stars && day_alpha > 0.0)
	{
		glPushMatrix();
		glPointSize(1.0);
		glRotatef((float)game_minute+(float)game_second/60.0, 0.0, -1.0, 0.0);
		glColor4f(1.0, 1.0, 1.0, day_alpha);
		glCallList(sky_lists);
		glColor4f(1.0, 1.0, 1.0, day_alpha*0.5);
		glCallList(sky_lists+1);
		glColor4f(1.0, 1.0, 1.0, day_alpha*0.25);
		glCallList(sky_lists+2);
		glPopMatrix();
	}
	
    glDepthMask(GL_TRUE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	// we draw the clouds
	if (skybox_show_clouds && !skybox_no_clouds)
	{
		float clouds_step = (cur_time - last_cloud_time)*2E-6;

		// we update the tex coords for the first clouds layer
		if (dome_clouds.tex_coords[0] <= 1.0)
		{
			for (i = dome_clouds.vertices_count; i--; )
				dome_clouds.tex_coords[i*2] += clouds_step;
		}
		else
		{
			for (i = dome_clouds.vertices_count; i--; )
				dome_clouds.tex_coords[i*2] += clouds_step - 1.0;
		}

		// we update the tex coords for the second clouds layer
		if (dome_clouds_tex_coords_bis[0] <= 1.0)
		{
			for (i = dome_clouds.vertices_count; i--; )
				dome_clouds_tex_coords_bis[i*2] += clouds_step*2.0;
		}
		else
		{
			for (i = dome_clouds.vertices_count; i--; )
				dome_clouds_tex_coords_bis[i*2] += clouds_step*2.0 - 1.0;
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, 0, dome_clouds.vertices);
		glColorPointer(4, GL_FLOAT, 0, dome_clouds.colors);
		glTexCoordPointer(2, GL_FLOAT, 0, dome_clouds.tex_coords);
		
#ifdef	NEW_TEXTURES
		bind_texture(skybox_clouds_tex);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(skybox_clouds_tex);
#endif	/* NEW_TEXTURES */
		
		glDrawElements(GL_TRIANGLES, dome_clouds.faces_count*3, GL_UNSIGNED_INT, dome_clouds.faces);
		
		{
			// we draw the second clouds layer
#ifdef	NEW_TEXTURES
			bind_texture(thick_clouds_tex);
#else	/* NEW_TEXTURES */
			get_and_set_texture_id(thick_clouds_tex);
#endif	/* NEW_TEXTURES */
			glColorPointer(4, GL_FLOAT, 0, dome_clouds_colors_bis);
			glTexCoordPointer(2, GL_FLOAT, 0, dome_clouds_tex_coords_bis);
			glPushMatrix();
			glScalef(1.0, 1.0, 0.95);
			glDrawElements(GL_TRIANGLES, dome_clouds.faces_count*3, GL_UNSIGNED_INT, dome_clouds.faces);
			glPopMatrix();
		}

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
		
	glBlendFunc(GL_SRC_COLOR, GL_ONE);

	// draw the sun
	if (skybox_show_sun && !skybox_no_sun &&
		(skybox_sun_position[0] != 0.0 ||
		 skybox_sun_position[1] != 0.0 ||
		 skybox_sun_position[2] != 0.0))
	{
		glColor4fv(sun_color);
#ifdef	NEW_TEXTURES
		bind_texture(sun_tex);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(sun_tex);
#endif	/* NEW_TEXTURES */
		
		glPushMatrix();
		glScalef(480.0, 480.0, 480.0);
		glBegin(GL_QUADS);
		{
			//two cross products. Dangerous to use these functions. Float type essential.
			//Better to robustly produce two vectors in perpendicular plane elsewhere.
			VECTOR3 perp1, perp2, someVec = {0.0, 1.0, 0.0};
			VCross(perp1, someVec, skybox_sun_position);
			VCross(perp2, perp1, skybox_sun_position);
			glTexCoord2f(0,0);
			glVertex3f(skybox_sun_position[0]+.08*(perp1[0]+perp2[0]),
					   skybox_sun_position[1]+.08*(perp1[1]+perp2[1]),
					   skybox_sun_position[2]+.08*(perp1[2]+perp2[2]));
			glTexCoord2f(1,0);
			glVertex3f(skybox_sun_position[0]+.08*(-perp1[0]+perp2[0]),
					   skybox_sun_position[1]+.08*(-perp1[1]+perp2[1]),
					   skybox_sun_position[2]+.08*(-perp1[2]+perp2[2]));
			glTexCoord2f(1,1);
			glVertex3f(skybox_sun_position[0]+.08*(-perp1[0]-perp2[0]),
					   skybox_sun_position[1]+.08*(-perp1[1]-perp2[1]),
					   skybox_sun_position[2]+.08*(-perp1[2]-perp2[2]));
			glTexCoord2f(0,1);
			glVertex3f(skybox_sun_position[0]+.08*(perp1[0]-perp2[0]),
					   skybox_sun_position[1]+.08*(perp1[1]-perp2[1]),
					   skybox_sun_position[2]+.08*(perp1[2]-perp2[2]));
		}
		glEnd();
		glPopMatrix();
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// we draw the clouds detail
	if (skybox_show_clouds && !skybox_no_clouds)
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
#ifdef	NEW_TEXTURES
		bind_texture(skybox_clouds_detail_tex);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(skybox_clouds_detail_tex);
#endif	/* NEW_TEXTURES */
		glVertexPointer(3, GL_FLOAT, 0, dome_clouds.vertices);
		glColorPointer(4, GL_FLOAT, 0, dome_clouds_detail_colors);
		glTexCoordPointer(2, GL_FLOAT, 0, dome_clouds.tex_coords);

		glPushMatrix();
		glScalef(1.0, 1.0, 0.98);
		glDrawElements(GL_TRIANGLES, dome_clouds.faces_count*3, GL_UNSIGNED_INT, dome_clouds.faces);
		glPopMatrix();
		
		{
#ifdef	NEW_TEXTURES
			bind_texture(thick_clouds_detail_tex);
#else	/* NEW_TEXTURES */
			get_and_set_texture_id(thick_clouds_detail_tex);
#endif	/* NEW_TEXTURES */
			glColorPointer(4, GL_FLOAT, 0, dome_clouds_detail_colors_bis);
			glTexCoordPointer(2, GL_FLOAT, 0, dome_clouds_tex_coords_bis);
			glPushMatrix();
			glScalef(1.0, 1.0, 0.93);
			glDrawElements(GL_TRIANGLES, dome_clouds.faces_count*3, GL_UNSIGNED_INT, dome_clouds.faces);
			glPopMatrix();
		}

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	glDisable(GL_TEXTURE_2D);

	// we fade the sky that is under the horizon
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < dome_sky.slices_count; ++i)
	{
		const GLfloat *vtx = &dome_sky.vertices[i*3];
		glColor4fv(skybox_fog_color);
		glVertex3f(vtx[0]*0.9, vtx[1]*0.9, -dome_sky.height*0.1);
		glColor4f(skybox_fog_color[0], skybox_fog_color[1], skybox_fog_color[2], 0.0);
		glVertex3fv(vtx);
	}
	glColor4fv(skybox_fog_color);
	glVertex3f(dome_sky.vertices[0]*0.9, dome_sky.vertices[1]*0.9, -dome_sky.height*0.1);
	glColor4f(skybox_fog_color[0], skybox_fog_color[1], skybox_fog_color[2], 0.0);
	glVertex3fv(&dome_sky.vertices[0]);
	glEnd();

	// we mask all the elements that are under the horizon with the fog color
	glBegin(GL_TRIANGLE_FAN);
	glColor3fv(skybox_fog_color);
	glVertex3f(0.0, 0.0, -dome_sky.height);
	for (i = 0; i < dome_sky.slices_count; ++i)
	{
		const GLfloat *vtx = &dome_sky.vertices[i*3];
		glVertex3f(vtx[0]*0.9, vtx[1]*0.9, -dome_sky.height*0.1);
	}
	glVertex3f(dome_sky.vertices[0]*0.9, dome_sky.vertices[1]*0.9, -dome_sky.height*0.1);
	glEnd();
	
	glPopAttrib();
	reset_material();
	last_texture = -1;

    // we restore the lights
	if(!is_day||dungeon)
	{
		switch(show_lights)
		{
		case 6:
			glEnable(GL_LIGHT6);
		case 5:
			glEnable(GL_LIGHT5);
		case 4:
			glEnable(GL_LIGHT4);
		case 3:
			glEnable(GL_LIGHT3);
		case 2:
			glEnable(GL_LIGHT2);
		case 1:
			glEnable(GL_LIGHT1);
		default:
			glEnable(GL_LIGHT0);
			break;
		}
	}

	last_cloud_time = cur_time;
}

void underworld_sky()
{
	static Uint32 last_cloud_time = 0;
	int i;

	glTranslatef(0.0, 0.0, -40.0);

	glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

/* 	if(use_shadow_mapping) */
/* 	{ */
/* 		glDisable(GL_TEXTURE_2D); */
/* 		ELglActiveTextureARB(shadow_unit); */
/* 		glDisable(depth_texture_target); */
/* 		disable_texgen(); */
/* 		ELglActiveTextureARB(GL_TEXTURE0); */
/* 		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); */
/* 	} */
	
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	if (skybox_show_clouds)
	{
		float clouds_step = (cur_time - last_cloud_time)*1E-5;

		// we update the tex coords for the first clouds layer
		if (dome_clouds.tex_coords[0] <= 1.0)
		{
			for (i = dome_clouds.vertices_count; i--; )
				dome_clouds.tex_coords[i*2] += clouds_step*2.0;
		}
		else
		{
			for (i = dome_clouds.vertices_count; i--; )
				dome_clouds.tex_coords[i*2] += clouds_step*2.0 - 1.0;
		}

		// we update the tex coords for the second clouds layer
		if (dome_clouds_tex_coords_bis[0] <= 1.0)
		{
			for (i = dome_clouds.vertices_count; i--; )
				dome_clouds_tex_coords_bis[i*2] += clouds_step;
		}
		else
		{
			for (i = dome_clouds.vertices_count; i--; )
				dome_clouds_tex_coords_bis[i*2] += clouds_step - 1.0;
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
#ifdef	NEW_TEXTURES
		bind_texture(thick_clouds_detail_tex);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(thick_clouds_detail_tex);
#endif	/* NEW_TEXTURES */

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glVertexPointer(3, GL_FLOAT, 0, dome_clouds.vertices);
		
		glColor3f(0.8, 0.0, 0.0);
		glTexCoordPointer(2, GL_FLOAT, 0, dome_clouds_tex_coords_bis);
		glPushMatrix();
		glRotatef(90.0, 0.0, 0.0, 1.0);
		glDrawElements(GL_TRIANGLES, dome_clouds.faces_count*3, GL_UNSIGNED_INT, dome_clouds.faces);
		glPopMatrix();

		glColor3f(0.8, 0.2, 0.0);
		glTexCoordPointer(2, GL_FLOAT, 0, dome_clouds.tex_coords);
		glDrawElements(GL_TRIANGLES, dome_clouds.faces_count*3, GL_UNSIGNED_INT, dome_clouds.faces);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
	}

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, dome_sky.vertices);
	glColorPointer(4, GL_FLOAT, 0, dome_sky.colors);
	
	glDrawElements(GL_TRIANGLES, dome_sky.faces_count*3, GL_UNSIGNED_INT, dome_sky.faces);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glBegin(GL_QUAD_STRIP);
	for (i = 0; i < dome_sky.slices_count; ++i)
	{
		const GLfloat *vtx = &dome_sky.vertices[i*3];
		glColor3fv(&dome_sky.colors[i*4]);
		glVertex4f(vtx[0], vtx[1], vtx[2], 500.0);
		glVertex3fv(vtx);
	}
	glColor3fv(&dome_sky.colors[0]);
	glVertex4f(dome_sky.vertices[0], dome_sky.vertices[1], dome_sky.vertices[2], 500.0);
	glVertex3fv(&dome_sky.vertices[0]);
	glEnd();
	
	glPopAttrib();
	reset_material();
	last_texture = -1;

	last_cloud_time = cur_time;
}

#define XML_BOOL(s) (!xmlStrcasecmp((s), (xmlChar*)"yes") ||\
					 !xmlStrcasecmp((s), (xmlChar*)"true") ||\
					 !xmlStrcasecmp((s), (xmlChar*)"1"))

int skybox_parse_color_properties(xmlNode *node, float container[360][4])
{
	xmlAttr *attr;
	int t = -1;
	float r = 0.0, g = 0.0, b = 0.0, a = 1.0;
	int ok = 1;

	for (attr = node->properties; attr; attr = attr->next)
	{
		if (attr->type == XML_ATTRIBUTE_NODE)
		{
			if (xmlStrcasecmp (attr->name, (xmlChar*)"t") == 0)
				t =  atoi((char*)attr->children->content);
			else if (xmlStrcasecmp (attr->name, (xmlChar*)"r") == 0)
				r =  atof((char*)attr->children->content);
			else if (xmlStrcasecmp (attr->name, (xmlChar*)"g") == 0)
				g =  atof((char*)attr->children->content);
			else if (xmlStrcasecmp (attr->name, (xmlChar*)"b") == 0)
				b =  atof((char*)attr->children->content);
			else if (xmlStrcasecmp (attr->name, (xmlChar*)"a") == 0)
				a =  atof((char*)attr->children->content);
			else {
				LOG_ERROR("unknown attribute for color: %s", (char*)attr->name);
				ok = 0;
			}
		}
	}

	if (t >= 0 && t < 360)
	{
		container[t][0] = r <= 1.0 ? r : r / 255.0;
		container[t][1] = g <= 1.0 ? g : g / 255.0;
		container[t][2] = b <= 1.0 ? b : b / 255.0;
		container[t][3] = a <= 1.0 ? a : a / 255.0;
	}
	else
	{
		LOG_ERROR("the time attribute of the color doesn't exist or is wrong!");
		ok = 0;
	}

	return ok;
}

int skybox_parse_colors(xmlNode *node, float container[360][4])
{
	xmlNode	*item;
	xmlAttr *attr;
	int	ok = 1;
    int i, t;
    int reset = 0;

	if(node == NULL || node->children == NULL) return 0;

	for (attr = node->properties; attr; attr = attr->next) {
		if (attr->type == XML_ATTRIBUTE_NODE &&
			(!xmlStrcasecmp(attr->name, (xmlChar*)"reset") ||
			 !xmlStrcasecmp(attr->name, (xmlChar*)"overwrite"))) {
			reset = XML_BOOL(attr->children->content);
		}
		else {
			LOG_ERROR("unknown attribute for element: %s", (char*)attr->name);
			ok = 0;
		}
	}

    if (reset)
    {
        // we erase the previous color keys
        for (t = 360; t--; )
        {
            for (i = 3; i--; )
            {
                container[t][i] = 0.0;
            }
            container[t][3] = -1.0;
        }
    }
        
	for(item = node->children; item; item = item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, (xmlChar*)"color") == 0) {
				ok &= skybox_parse_color_properties(item, container);
			}
			else {
				LOG_ERROR("unknown node for element: %s", item->name);
				ok = 0;
			}
		}
		else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= skybox_parse_colors(item->children, container);
		}
	}

	return ok;
}

int skybox_parse_properties(xmlNode *node)
{
	xmlNode *item;
	xmlAttr *attr;
	int ok = 1;

	for(item = node->children; item; item = item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, (xmlChar*)"clouds") == 0) {
				for (attr = item->properties; attr; attr = attr->next)
				{
					if (attr->type == XML_ATTRIBUTE_NODE)
					{
						if (!xmlStrcasecmp (attr->name, (xmlChar*)"show"))
							skybox_no_clouds = !XML_BOOL(attr->children->content);
						else if (xmlStrcasecmp (attr->name, (xmlChar*)"texture") == 0)
#ifdef	NEW_TEXTURES
							skybox_clouds_tex = load_texture_cached((char*)attr->children->content, tt_mesh);
#else	/* NEW_TEXTURES */
							skybox_clouds_tex = load_texture_cache((char*)attr->children->content, 0);
#endif	/* NEW_TEXTURES */
						else if (xmlStrcasecmp (attr->name, (xmlChar*)"texture_detail") == 0)
#ifdef	NEW_TEXTURES
							skybox_clouds_detail_tex = load_texture_cached((char*)attr->children->content, tt_mesh);
#else	/* NEW_TEXTURES */
							skybox_clouds_detail_tex = load_texture_cache((char*)attr->children->content, 0);
#endif	/* NEW_TEXTURES */
						else {
							LOG_ERROR("unknown attribute for clouds: %s", (char*)attr->name);
							ok = 0;
						}
					}
				}
			}
			else if(xmlStrcasecmp(item->name, (xmlChar*)"sun") == 0) {
				for (attr = item->properties; attr; attr = attr->next)
					if (attr->type == XML_ATTRIBUTE_NODE) {
						if (!xmlStrcasecmp (attr->name, (xmlChar*)"show"))
							skybox_no_sun = !XML_BOOL(attr->children->content);
						else {
							LOG_ERROR("unknown attribute for sun: %s", (char*)attr->name);
							ok = 0;
						}
					}
			}
			else if(xmlStrcasecmp(item->name, (xmlChar*)"moons") == 0) {
				for (attr = item->properties; attr; attr = attr->next)
					if (attr->type == XML_ATTRIBUTE_NODE) {
						if (!xmlStrcasecmp (attr->name, (xmlChar*)"show"))
							skybox_no_moons = !XML_BOOL(attr->children->content);
						else {
							LOG_ERROR("unknown attribute for moons: %s", (char*)attr->name);
							ok = 0;
						}
					}
			}
			else if(xmlStrcasecmp(item->name, (xmlChar*)"stars") == 0) {
				for (attr = item->properties; attr; attr = attr->next)
					if (attr->type == XML_ATTRIBUTE_NODE) {
						if (!xmlStrcasecmp (attr->name, (xmlChar*)"show"))
							skybox_no_stars = !XML_BOOL(attr->children->content);
						else {
							LOG_ERROR("unknown attribute for stars: %s", (char*)attr->name);
							ok = 0;
						}
					}
			}
			else if(xmlStrcasecmp(item->name, (xmlChar*)"freeze_time") == 0) {
				int value = get_int_value(item);
				if (value >= 0 && value <= 359) {
					freeze_time = 1;
					game_minute = value;
					game_second = 0;
				}
				else {
					freeze_time = 0;
					game_minute = real_game_minute;
					game_second = real_game_second;
				}
			}
			else {
				LOG_ERROR("unknown node for properties: %s", item->name);
				ok = 0;
			}
		}
		else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= skybox_parse_properties(item->children);
		}
	}

	return ok;
}

int skybox_parse_defs(xmlNode *node, const char *map_name)
{
	xmlNode *def;
	int ok = 1;

	for (def = node->children; def; def = def->next) {
		if (def->type == XML_ELEMENT_NODE)
		{
			if (xmlStrcasecmp(def->name, (xmlChar*)"properties") == 0) {
				ok &= skybox_parse_properties(def);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"clouds") == 0) {
				ok &= skybox_parse_colors(def, skybox_clouds);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"clouds_detail") == 0) {
				ok &= skybox_parse_colors(def, skybox_clouds_detail);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"clouds_sunny") == 0) {
				ok &= skybox_parse_colors(def, skybox_clouds_sunny);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"clouds_detail_sunny") == 0) {
				ok &= skybox_parse_colors(def, skybox_clouds_detail_sunny);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"clouds_rainy") == 0) {
				ok &= skybox_parse_colors(def, skybox_clouds_rainy);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"clouds_detail_rainy") == 0) {
				ok &= skybox_parse_colors(def, skybox_clouds_detail_rainy);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky1") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky1);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky2") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky2);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky3") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky3);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky4") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky4);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky5") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky5);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky1_sunny") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky1_sunny);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky2_sunny") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky2_sunny);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky3_sunny") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky3_sunny);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky4_sunny") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky4_sunny);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky5_sunny") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky5_sunny);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sun") == 0) {
				ok &= skybox_parse_colors(def, skybox_sun);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"fog") == 0) {
				ok &= skybox_parse_colors(def, skybox_fog);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"fog_sunny") == 0) {
				ok &= skybox_parse_colors(def, skybox_fog_sunny);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"fog_rainy") == 0) {
				ok &= skybox_parse_colors(def, skybox_fog_rainy);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"light_ambient") == 0) {
				ok &= skybox_parse_colors(def, skybox_light_ambient);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"light_diffuse") == 0) {
				ok &= skybox_parse_colors(def, skybox_light_diffuse);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"light_ambient_rainy") == 0) {
				ok &= skybox_parse_colors(def, skybox_light_ambient_rainy);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"light_diffuse_rainy") == 0) {
				ok &= skybox_parse_colors(def, skybox_light_diffuse_rainy);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"map") == 0) {
				const char *name = get_string_property(def, "name");
				if (!strcasecmp(name, map_name)) {
					//printf("Found custom sky defs for the current map!\n");
					ok &= skybox_parse_defs(def, "");
				}
			}
			else {
				LOG_ERROR("unknown element for skybox: %s", def->name);
				ok = 0;
			}
		}
		else if (def->type == XML_ENTITY_REF_NODE) {
			ok &= skybox_parse_defs(def->children, map_name);
		}
	}

	return ok;
}

int skybox_read_defs(const char *file_name, const char *map_name)
{
	xmlNode *root;
	xmlDoc *doc;
	int ok = 1;

	doc = xmlReadFile(file_name, NULL, XML_PARSE_NOENT);
	if (doc == NULL) {
		LOG_ERROR("Unable to read skybox definition file %s", file_name);
		return 0;
	}

	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		LOG_ERROR("Unable to parse skybox definition file %s", file_name);
		ok = 0;
	} else if (xmlStrcasecmp(root->name, (xmlChar*)"skybox") != 0) {
		LOG_ERROR("Unknown key \"%s\" (\"skybox\" expected).", root->name);
		ok = 0;
	} else {
		ok = skybox_parse_defs(root, map_name);
	}

	xmlFreeDoc(doc);
	return ok;
}

int skybox_build_gradients(float container[360][4])
{
	int t;
	int first = 0;
	int prev, next, diff;

	while (first < 360 && container[first][3] < 0.0) ++first;

	if (first >= 360) return 0;

	prev = first;
	do
	{
		next = (prev+1)%360;
		while (container[next][3] < 0.0) next = (next+1)%360;
		diff = (next-prev+360)%360;
		if (diff) // prev != next
		{
			for (t = prev+1; t != next; t = (t+1)%360)
				blend_colors(container[t], container[prev], container[next],
							 (float)((t-prev+360)%360)/(float)diff, 4);
		}
		else // prev == next
		{
			for (t = prev+1; t != next; t = (t+1)%360)
				memcpy(container[t], container[prev], 4*sizeof(float));
		}
		prev = next;
	}
	while (prev != first);
	
	return 1;
}

void skybox_init_defs(const char *map_name)
{
    static char last_map[256] = "\0";
	int t;
	int i;

	for (t = 360; t--; )
	{
		for (i = 3; i--; )
		{
			skybox_clouds[t][i] = 0.0;
			skybox_clouds_detail[t][i] = 0.0;
			skybox_clouds_sunny[t][i] = 0.0;
			skybox_clouds_detail_sunny[t][i] = 0.0;
			skybox_clouds_rainy[t][i] = 0.0;
			skybox_clouds_detail_rainy[t][i] = 0.0;
			skybox_sky1[t][i] = 0.0;
			skybox_sky2[t][i] = 0.0;
			skybox_sky3[t][i] = 0.0;
			skybox_sky4[t][i] = 0.0;
			skybox_sky5[t][i] = 0.0;
			skybox_sky1_sunny[t][i] = 0.0;
			skybox_sky2_sunny[t][i] = 0.0;
			skybox_sky3_sunny[t][i] = 0.0;
			skybox_sky4_sunny[t][i] = 0.0;
			skybox_sky5_sunny[t][i] = 0.0;
			skybox_sun[t][i] = 0.0;
			skybox_fog[t][i] = 0.0;
			skybox_fog_sunny[t][i] = 0.0;
			skybox_fog_rainy[t][i] = 0.0;
			skybox_light_ambient[t][i] = 0.0;
			skybox_light_diffuse[t][i] = 0.0;
			skybox_light_ambient_rainy[t][i] = 0.0;
			skybox_light_diffuse_rainy[t][i] = 0.0;
		}
		skybox_clouds[t][3] = -1.0;
		skybox_clouds_detail[t][3] = -1.0;
		skybox_clouds_sunny[t][3] = -1.0;
		skybox_clouds_detail_sunny[t][3] = -1.0;
		skybox_clouds_rainy[t][3] = -1.0;
		skybox_clouds_detail_rainy[t][3] = -1.0;
		skybox_sky1[t][3] = -1.0;
		skybox_sky2[t][3] = -1.0;
		skybox_sky3[t][3] = -1.0;
		skybox_sky4[t][3] = -1.0;
		skybox_sky5[t][3] = -1.0;
		skybox_sky1_sunny[t][3] = -1.0;
		skybox_sky2_sunny[t][3] = -1.0;
		skybox_sky3_sunny[t][3] = -1.0;
		skybox_sky4_sunny[t][3] = -1.0;
		skybox_sky5_sunny[t][3] = -1.0;
		skybox_sun[t][3] = -1.0;
		skybox_fog[t][3] = -1.0;
		skybox_fog_sunny[t][3] = -1.0;
		skybox_fog_rainy[t][3] = -1.0;
		skybox_light_ambient[t][3] = -1.0;
		skybox_light_diffuse[t][3] = -1.0;
		skybox_light_ambient_rainy[t][3] = -1.0;
		skybox_light_diffuse_rainy[t][3] = -1.0;
	}

	skybox_no_clouds = 1;
	skybox_no_sun = 1;
	skybox_no_moons = 1;
	skybox_no_stars = 1;
	skybox_clouds_tex = -1;
	skybox_clouds_detail_tex = -1;
	freeze_time = 0;
	game_minute = real_game_minute;
	game_second = real_game_second;

    if (map_name) {
        int pos = strlen(map_name)-1;
        while (pos >= 0 && map_name[pos] != '/') --pos;
        strcpy(last_map, map_name+pos+1);
    }

    //printf("Loading sky defs for map '%s'\n", last_map);
	if (!skybox_read_defs("skybox/skybox_defs.xml", last_map))
		LOG_ERROR("Error while loading the skybox definitions.");
	
	if (!skybox_build_gradients(skybox_clouds))
		LOG_ERROR("no color key defined for 'clouds' element!");
	if (!skybox_build_gradients(skybox_clouds_detail))
		LOG_ERROR("no color key defined for 'clouds_detail' element!");
	if (!skybox_build_gradients(skybox_clouds_sunny))
		LOG_ERROR("no color key defined for 'clouds_sunny' element!");
	if (!skybox_build_gradients(skybox_clouds_detail_sunny))
		LOG_ERROR("no color key defined for 'clouds_detail_sunny' element!");
	if (!skybox_build_gradients(skybox_clouds_rainy))
		LOG_ERROR("no color key defined for 'clouds_rainy' element!");
	if (!skybox_build_gradients(skybox_clouds_detail_rainy))
		LOG_ERROR("no color key defined for 'clouds_detail_rainy' element!");
	if (!skybox_build_gradients(skybox_sky1))
		LOG_ERROR("no color key defined for 'sky1' element!");
	if (!skybox_build_gradients(skybox_sky2))
		LOG_ERROR("no color key defined for 'sky2' element!");
	if (!skybox_build_gradients(skybox_sky3))
		LOG_ERROR("no color key defined for 'sky3' element!");
	if (!skybox_build_gradients(skybox_sky4))
		LOG_ERROR("no color key defined for 'sky4' element!");
	if (!skybox_build_gradients(skybox_sky5))
		LOG_ERROR("no color key defined for 'sky5' element!");
	if (!skybox_build_gradients(skybox_sky1_sunny))
		LOG_ERROR("no color key defined for 'sky1_sunny' element!");
	if (!skybox_build_gradients(skybox_sky2_sunny))
		LOG_ERROR("no color key defined for 'sky2_sunny' element!");
	if (!skybox_build_gradients(skybox_sky3_sunny))
		LOG_ERROR("no color key defined for 'sky3_sunny' element!");
	if (!skybox_build_gradients(skybox_sky4_sunny))
		LOG_ERROR("no color key defined for 'sky4_sunny' element!");
	if (!skybox_build_gradients(skybox_sky5_sunny))
		LOG_ERROR("no color key defined for 'sky5_sunny' element!");
	if (!skybox_build_gradients(skybox_sun))
		LOG_ERROR("no color key defined for 'sun' element!");
	if (!skybox_build_gradients(skybox_fog))
		LOG_ERROR("no color key defined for 'fog' element!");
	if (!skybox_build_gradients(skybox_fog_sunny))
		LOG_ERROR("no color key defined for 'fog_sunny' element!");
	if (!skybox_build_gradients(skybox_fog_rainy))
		LOG_ERROR("no color key defined for 'fog_rainy' element!");
	if (!skybox_build_gradients(skybox_light_ambient))
		LOG_ERROR("no color key defined for 'light_ambient' element!");
	if (!skybox_build_gradients(skybox_light_diffuse))
		LOG_ERROR("no color key defined for 'light_diffuse' element!");
	if (!skybox_build_gradients(skybox_light_ambient_rainy))
		LOG_ERROR("no color key defined for 'light_ambient_rainy' element!");
	if (!skybox_build_gradients(skybox_light_diffuse_rainy))
		LOG_ERROR("no color key defined for 'light_diffuse_rainy' element!");

	skybox_update_positions();
	skybox_update_colors();
}

void skybox_init_gl()
{
/* 	GLUquadricObj *qobj; */
	GLfloat randx,randy,randz;
	int i;
	float maxr;
	float strs[NUM_STARS][3];

#ifdef	NEW_TEXTURES
	thick_clouds_tex = load_texture_cached("textures/thick_clouds", tt_mesh);
	thick_clouds_detail_tex = load_texture_cached("textures/thick_clouds_detail", tt_mesh);
	moon_tex = load_texture_cached("textures/moonmap", tt_mesh);
	sun_tex = load_texture_cached("textures/BrightSun", tt_mesh);
#else	/* NEW_TEXTURES */
	thick_clouds_tex = load_texture_cache("./textures/thick_clouds.bmp", 0);
	thick_clouds_detail_tex = load_texture_cache("./textures/thick_clouds_detail.bmp", 0);
	moon_tex=load_texture_cache("./textures/moonmap.bmp", 0);
	sun_tex=load_texture_cache("./textures/BrightSun.bmp", 255);
#endif	/* NEW_TEXTURES */

	sky_lists = glGenLists(3);

/* 	qobj = gluNewQuadric(); */
/* 	gluQuadricOrientation(qobj, GLU_OUTSIDE);	 */
/* 	gluQuadricNormals(qobj, GLU_SMOOTH); */
/* 	gluQuadricTexture(qobj, GL_TRUE); */
/* 	glNewList(sky_lists, GL_COMPILE); */
/* 	gluSphere(qobj, 1.0, 32, 16); */
/* 	glEndList(); */
/* 	gluDeleteQuadric(qobj); */

	srand(0);
	maxr = 1.0/RAND_MAX;
	for (i = 0; i < NUM_STARS; i++)
	{
		float norm;
		do
		{
			randx = rand()*(float)(maxr)-0.5f;
			randy = rand()*(float)(maxr)-0.5f;
			randz = rand()*(float)(maxr)-0.5f;
			norm = sqrt(randx*randx + randy*randy + randz*randz);
		} while (norm > 0.5f);

		strs[i][0] = 500 * randx / norm;
		strs[i][1] = 500 * randy / norm;
		strs[i][2] = 500 * randz / norm;
	}
	glNewList(sky_lists,GL_COMPILE);
	glBegin(GL_POINTS);
	for (i = 0; i < NUM_STARS/3; i++)
	{
		glVertex3fv(strs[i]);
	}
	glEnd();
	glEndList();
	glNewList(sky_lists+1,GL_COMPILE);
	glBegin(GL_POINTS);
	for (i = 1*NUM_STARS/3; i < 2*NUM_STARS/3; i++)
	{
		glVertex3fv(strs[i]);
	}
	glEnd();
	glEndList();
	glNewList(sky_lists+2,GL_COMPILE);
	glBegin(GL_POINTS);
	for (i = 2*NUM_STARS/3; i < NUM_STARS; i++)
	{
		glVertex3fv(strs[i]);
	}
	glEnd();
	glEndList();

	destroy_dome(&dome_sky);
	destroy_dome(&dome_clouds);
	destroy_sphere(&moon_mesh);
	if (dome_clouds_detail_colors) free(dome_clouds_detail_colors);
	if (dome_clouds_colors_bis) free(dome_clouds_colors_bis);
	if (dome_clouds_detail_colors_bis) free(dome_clouds_detail_colors_bis);
	if (dome_clouds_tex_coords_bis) free(dome_clouds_tex_coords_bis);
	if (fog_colors) free(fog_colors);

	dome_sky = create_dome(24, 12, 500.0, 80.0, 90.0, 3.5, 1.0);
	dome_clouds = create_dome(24, 12, 500.0, 80.0, 90.0, 2.0, 1.0);
	moon_mesh = create_sphere(24, 12);
	dome_clouds_detail_colors = (GLfloat*)malloc(4*dome_clouds.vertices_count*sizeof(GLfloat));
	dome_clouds_colors_bis = (GLfloat*)malloc(4*dome_clouds.vertices_count*sizeof(GLfloat));
	dome_clouds_detail_colors_bis = (GLfloat*)malloc(4*dome_clouds.vertices_count*sizeof(GLfloat));
	dome_clouds_tex_coords_bis = (GLfloat*)malloc(2*dome_clouds.vertices_count*sizeof(GLfloat));
	fog_colors = (GLfloat*)malloc(3*dome_sky.slices_count*sizeof(GLfloat));

	for (i = dome_clouds.vertices_count; i--; )
	{
		int idx = i*2;
		dome_clouds_tex_coords_bis[idx] = dome_clouds.tex_coords[idx];
		dome_clouds_tex_coords_bis[idx+1] = dome_clouds.tex_coords[idx+1]+0.5;
	}
}

void free_skybox()
{
	destroy_dome(&dome_sky);
	destroy_dome(&dome_clouds);
	destroy_sphere(&moon_mesh);
	if (dome_clouds_detail_colors) free(dome_clouds_detail_colors);
	if (dome_clouds_colors_bis) free(dome_clouds_colors_bis);
	if (dome_clouds_detail_colors_bis) free(dome_clouds_detail_colors_bis);
	if (dome_clouds_tex_coords_bis) free(dome_clouds_tex_coords_bis);
	if (fog_colors) free(fog_colors);
}


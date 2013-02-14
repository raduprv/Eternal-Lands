#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "weather.h"
#include "actors.h"
#include "asc.h"
#include "client_serv.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "global.h"
#include "map.h"
#include "misc.h"
#include "particles.h"
#include "textures.h"
#include "shadows.h"
#include "sound.h"
#include <SDL_timer.h>
#include "errors.h"
#include "text.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "sky.h"

int use_fog = 1;
int show_weather = 1;

int wind_speed_srv = 0;	//strength of wind, as set by server. 100 is about the max
int wind_direction_srv = 0;	//wind direction, in degrees, as set by server. 0 and 360 == north
int wind_speed = 0;	//strength of wind, based on server's setting and local randomization
int wind_direction = 0;	//wind direction, based on server's setting and local randomization


/* N E W   W E A T H E R *****************************************************/

#define MAX_RAIN_DROPS 10000
#define MAX_LIGHTNING_DEFS 20
#define MAX_THUNDERS 20

#define LIGHTNING_LIGHT_RADIUS 20.0
#define SOUND_SPEED 0.1 // real sound speed = 0.34029 m/ms

#define WEATHER_NONE 0
#define WEATHER_RAIN 1

typedef struct
{
	float pos1[3];
	float pos2[3];
} weather_drop;

typedef struct {
	int use_sprites;
	float density;
	float color[4];
	float size;
	float speed;
	float wind_effect;
	int texture;
} weather_def;

typedef struct
{
	float x_pos;
	float y_pos;
	float radius;
	int type;
	float intensity;
	float intensity_change_speed;
	int intensity_change_duration;
} weather_area;

typedef struct {
	int type;
	float x_pos;
	float y_pos;
	Uint32 time;
} thunder;

typedef struct {
	int texture;
	float coords[4]; // left bottom right top
} lightning_def;

weather_drop weather_drops[MAX_WEATHER_TYPES][MAX_RAIN_DROPS];
int weather_drops_count[MAX_WEATHER_TYPES];

weather_def weather_defs[MAX_WEATHER_TYPES];

weather_area weather_areas[MAX_WEATHER_AREAS];

lightning_def lightnings_defs[MAX_LIGHTNING_DEFS];
int lightnings_defs_count = 0;

thunder thunders[MAX_THUNDERS];
int thunders_count = 0;

Uint32 lightning_stop = 0;
int lightning_type;
float lightning_position[4] = {0.0, 0.0, 100.0, 1.0};
float lightning_sky_position[4] = {0.0, 0.0, 0.0, 1.0};
float lightning_color[4] = {0.9, 0.85, 1.0, 1.0};
float lightning_ambient_color[4] = {0.50, 0.45, 0.55, 1.0};
int lightning_falling = 0;

float weather_ratios[MAX_WEATHER_TYPES];

float current_weather_density;

#define RANDOM_TABLE_SIZE 100000
#define RAND_ONE (next_random_number())

float random_table[RANDOM_TABLE_SIZE];
int last_random_number = -1;

#ifdef NEW_SOUND
unsigned int rain_sound = 0;
#endif //NEW_SOUND

float weather_color[4] = {0.0, 0.0, 0.0, 1.0};
float fog_alpha;

// array used to build coordinates of quads to display particles
float weather_particles_coords[MAX_RAIN_DROPS*20];

static __inline__ float next_random_number()
{
	last_random_number = (last_random_number+1)%RANDOM_TABLE_SIZE;
	return random_table[last_random_number];
}

int weather_read_defs(const char *file_name);

void weather_init()
{
	int i;

	for (i = 0; i < MAX_WEATHER_TYPES; ++i)
	{
		weather_ratios[i] = 0.0;
		weather_drops_count[i] = 0;
		weather_defs[i].use_sprites = 0;
		weather_defs[i].density = 0.0;
		weather_defs[i].color[0] = 0.0;
		weather_defs[i].color[1] = 0.0;
		weather_defs[i].color[2] = 0.0;
		weather_defs[i].color[3] = 0.0;
		weather_defs[i].size = 0.0;
		weather_defs[i].speed = 0.0;
		weather_defs[i].wind_effect = 0.0;
		weather_defs[i].texture = -1;
	}
	weather_ratios[0] = 1.0;

	for (i = 0; i < MAX_WEATHER_AREAS; ++i)
	{
		weather_areas[i].type = 0;
	}

	// init the random numbers table
	for (i = 0; i < RANDOM_TABLE_SIZE; ++i)
		random_table[i] = (float)rand() / (float)RAND_MAX;

    // init the particles texture coordinates
    for (i = 0; i < MAX_RAIN_DROPS; ++i)
    {
        const int j = i*20;
        weather_particles_coords[j   ] = 0.0f;
        weather_particles_coords[j+1 ] = 0.0f;
        weather_particles_coords[j+5 ] = 1.0f;
        weather_particles_coords[j+6 ] = 0.0f;
        weather_particles_coords[j+10] = 1.0f;
        weather_particles_coords[j+11] = 1.0f;
        weather_particles_coords[j+15] = 0.0f;
        weather_particles_coords[j+16] = 1.0f;
    }

	weather_read_defs("./weather.xml");
}

void weather_clear()
{
	int i;
	thunders_count = 0;
	lightning_stop = 0;
	lightning_falling = 0;
	weather_ratios[0] = 1.0;
	for(i = 1; i < MAX_WEATHER_TYPES; ++i)
	{
		weather_ratios[i] = 0.0;
		weather_drops_count[i] = 0;
	}

	for (i = 0; i < MAX_WEATHER_AREAS; ++i)
	{
		weather_areas[i].type = 0;
	}
}

void weather_set_area(int area, float x, float y, float radius, int type, float intensity, int change_duration)
{
	if (weather_areas[area].type == 0) weather_areas[area].intensity = 0.0;

	weather_areas[area].type = type;
	weather_areas[area].x_pos = x;
	weather_areas[area].y_pos = y;
	weather_areas[area].radius = radius;
	weather_areas[area].intensity_change_duration = change_duration * 1000;
	weather_areas[area].intensity_change_speed = (intensity - weather_areas[area].intensity) / weather_areas[area].intensity_change_duration;

#ifdef DEBUG
	printf("setting area %d at %f,%f with radius %f\n",
		   area, x, y, radius);
#endif // DEBUG
}

void weather_get_from_server(const Uint8* data)
{
/*
	//first, catch non-precipitations
	if(data[0] == weather_effect_wind){
		wind_direction_srv = (2 * data[1])%360;
		wind_speed_srv = data[2];
		return;
	} else if(data[0] == weather_effect_leaves){
		//EC_TAG
		return;
	} else if(data[0] > MAX_WEATHER_TYPES){
#ifdef DEBUG
		LOG_TO_CONSOLE(c_red1, "Server sent an unknown weather type");
#endif // DEBUG
		LOG_ERROR("Server sent unknown weather type %d", data[0]);
		return;
		//from now on, deal with the set of precipitations
	} else if(data[2] == 0 && !weather_active()){
		return;	//stop? but we're already stopped...
	} else if(data[2] != 0 && weather_active()){
		set_weather_ratio(data[0], data[2]);
		return;
	} else if(data[2] != 0 && !weather_active()){
		set_weather_ratio(data[0], data[2]);
		start_weather(data[1], ((float)(data[2]))/100.0f);
		return;
	} else {
		stop_weather(data[1], ((float)(data[2]))/100.0f);
	}
*/
}

void weather_compute_ratios(float ratios[MAX_WEATHER_TYPES], float x, float y)
{
	int i;
	float sum = 0.0;

	// we reset all the values
	for (i = 0; i < MAX_WEATHER_TYPES; ++i)
		ratios[i] = 0.0;

	// we compute the intensity for each type of weather according to the areas
	for (i = 0; i < MAX_WEATHER_AREAS; ++i)
		if (weather_areas[i].type > 0)
		{
			float dx = x - weather_areas[i].x_pos;
			float dy = y - weather_areas[i].y_pos;
			float dist = dx*dx + dy*dy;
			float r = weather_areas[i].radius*weather_areas[i].radius;
			if (dist < r)
				ratios[weather_areas[i].type] += (1.0 - dist/r) * weather_areas[i].intensity;
		}

	// we compute the sum of all the intensities
	for (i = 1; i < MAX_WEATHER_TYPES; ++i)
		sum += ratios[i];

	if (sum < 1.0)
	{
		ratios[0] = 1.0 - sum;
	}
	else
	{
		// we normalize the intensities if needed
		ratios[0] = 0.0;
		for (i = 1; i < MAX_WEATHER_TYPES; ++i)
			ratios[i] /= sum;
	}
}

void weather_get_color_from_ratios(float color[4], float ratios[MAX_WEATHER_TYPES])
{
	int i, j;
	for(i = 0; i < 4; ++i)
	{
		color[i] = 0.0;
		for(j = 0; j < MAX_WEATHER_TYPES; ++j)
		{
			color[i] += weather_defs[j].color[i] * ratios[j];
		}
	}
}

static __inline__ void make_drop(int type, int i, float x, float y, float z)
{
	weather_drops[type][i].pos1[0] = x + 16.0f * RAND_ONE - 8.0f;
	weather_drops[type][i].pos1[1] = y + 16.0f * RAND_ONE - 8.0f;
	weather_drops[type][i].pos1[2] = z + 10.0f * RAND_ONE +  2.0f;
}

void update_wind(void)
{
	int dir_d = wind_direction_srv + (int)((float)(RAND_ONE * 60 - 30));
	int speed_d = wind_speed_srv + (int)((float)(RAND_ONE * ((float)wind_speed)/5.0f - ((float)wind_speed)/10.0f));
	if(dir_d < wind_direction){
		--wind_direction;
	} else if(dir_d > wind_direction){
		++wind_direction;
	}
	wind_direction %= 360;
	if(speed_d <= 0){
		speed_d = 0;
	} else if(speed_d > 1000){
		speed_d = 1000;
	}
	if(speed_d < wind_speed){
		--wind_speed;
	} else if(speed_d > wind_speed){
		++wind_speed;
	}
}

void update_weather_type(int type, float x, float y, float z, int ticks)
{
	int num_drops = weather_ratios[type] * weather_defs[type].density * particles_percentage * 0.01 * MAX_RAIN_DROPS;

	if (num_drops > MAX_RAIN_DROPS) num_drops = MAX_RAIN_DROPS;
	
	if (weather_drops_count[type] > num_drops)
		weather_drops_count[type] = num_drops;
	
	if (num_drops > 0)
	{
		int i;
		float x_move = weather_defs[type].wind_effect * sinf((float)wind_direction*M_PI/180) * wind_speed;
		float y_move = weather_defs[type].wind_effect * cosf((float)wind_direction*M_PI/180) * wind_speed;
		float z_move = -weather_defs[type].speed;
		float dt = ticks * 1E-3;
		float dx, dy, dz;
		
		for(i = 0; i < weather_drops_count[type]; ++i)
		{
			dx = x_move*(1.1-0.2*RAND_ONE);
			dy = y_move*(1.1-0.2*RAND_ONE);
			dz = z_move*(1.1-0.2*RAND_ONE);

			if (weather_drops[type][i].pos1[2] < z-1.0f)
			{
				// the drop should still exist so we recreate it
				make_drop(type, i, x, y, z);
			}
			else
			{
				// it's moving. so find out how much by, and wrap around the X/Y if it's too far away (as wind can cause)
				weather_drops[type][i].pos1[0] += dx * dt;
				if (weather_drops[type][i].pos1[0] < x - 8.0f)
				{
					weather_drops[type][i].pos1[0] += 16.0f;
					weather_drops[type][i].pos2[0] += 16.0f;
				}
				else if(weather_drops[type][i].pos1[0] > x + 8.0f)
				{
					weather_drops[type][i].pos1[0] -= 16.0f;
					weather_drops[type][i].pos2[0] -= 16.0f;
				}
				
				weather_drops[type][i].pos1[1] += dy * dt;
				if (weather_drops[type][i].pos1[1] < y - 8.0f)
				{
					weather_drops[type][i].pos1[1] += 16.0f;
					weather_drops[type][i].pos2[1] += 16.0f;
				}
				else if(weather_drops[type][i].pos1[1] > y + 8.0f)
				{
					weather_drops[type][i].pos1[1] -= 16.0f;
					weather_drops[type][i].pos2[1] -= 16.0f;
				}
				
				weather_drops[type][i].pos1[2] += dz * dt;
			}

			weather_drops[type][i].pos2[0] = weather_drops[type][i].pos1[0] - dx * 0.02;
			weather_drops[type][i].pos2[1] = weather_drops[type][i].pos1[1] - dy * 0.02;
			weather_drops[type][i].pos2[2] = weather_drops[type][i].pos1[2] - dz * 0.02;
		}
		
		// if there are not enough drops, we recreate new ones
		if (weather_drops_count[type] < num_drops)
		{
			for (i = weather_drops_count[type]; i < num_drops; ++i)
			{
				make_drop(type, i, x, y, z);
				weather_drops[type][i].pos2[0] = weather_drops[type][i].pos1[0] - x_move * 0.02;
				weather_drops[type][i].pos2[1] = weather_drops[type][i].pos1[1] - y_move * 0.02;
				weather_drops[type][i].pos2[2] = weather_drops[type][i].pos1[2] - z_move * 0.02;
			}
			weather_drops_count[type] = num_drops;
		}
	}
}

void weather_update()
{
	static Uint32 last_update = 0;
	Uint32 ticks = cur_time - last_update;
	int i;

	update_wind();

	// we update the lightning
	if (lightning_falling && cur_time > lightning_stop)
	{
		lightning_falling = 0;
		calc_shadow_matrix();
		if (skybox_update_delay > 0)
			skybox_update_colors();
	}

	// we update the areas
	for (i = 0; i < MAX_WEATHER_AREAS; ++i)
		if (weather_areas[i].type > 0 && weather_areas[i].intensity_change_duration > 0)
		{
			if (weather_areas[i].intensity_change_duration <= ticks)
			{
				weather_areas[i].intensity += weather_areas[i].intensity_change_speed*weather_areas[i].intensity_change_duration;
				weather_areas[i].intensity_change_duration = 0;
			}
			else
			{
				weather_areas[i].intensity += weather_areas[i].intensity_change_speed*ticks;
				weather_areas[i].intensity_change_duration -= ticks;
			}
			if (weather_areas[i].intensity_change_speed < 0.0)
			{
				if (weather_areas[i].intensity <= 0.001)
				{
					weather_areas[i].intensity = 0.0;
					weather_areas[i].type = 0;
				}
			}
			else
			{
				if (weather_areas[i].intensity > 1.0)
					weather_areas[i].intensity = 1.0;
			}
		}
	
	// we compute the ratios at the actor position
	weather_compute_ratios(weather_ratios, -camera_x, -camera_y);
	
	current_weather_density = weather_get_density_from_ratios(weather_ratios);

	// we compute the weather color at the actor position
	weather_get_color_from_ratios(weather_color, weather_ratios);
	
	// we update the weather types
	for (i = 1; i < MAX_WEATHER_TYPES; ++i)
		update_weather_type(i, -camera_x, -camera_y, 0.0, ticks);
	
	last_update = cur_time;
}

void weather_render_fog()
{
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
	glFogf(GL_FOG_DENSITY, skybox_fog_density);
	glFogfv(GL_FOG_COLOR, skybox_fog_color);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void weather_render()
{
	int type;
	int i;
	float modelview[16];
	float delta1[3], delta2[3];
	float color1[4], color2[4], light_level[3];

	skybox_get_current_color(color1, skybox_light_ambient);
	skybox_get_current_color(color2, skybox_light_diffuse);

	light_level[0] = 0.5 + (0.2 + color1[0] + color2[0]) * 0.5;
	light_level[1] = 0.5 + (0.2 + color1[1] + color2[1]) * 0.5;
	light_level[2] = 0.5 + (0.2 + color1[2] + color2[2]) * 0.5;

	if (lightning_falling)
	{
		light_level[0] += 0.5 * lightning_color[0];
		light_level[1] += 0.5 * lightning_color[1];
		light_level[2] += 0.5 * lightning_color[2];
	}

	if (light_level[0] > 1.0) light_level[0] = 1.0;
	if (light_level[1] > 1.0) light_level[1] = 1.0;
	if (light_level[2] > 1.0) light_level[2] = 1.0;

    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
          
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// we first render the weather that don't use particles
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisable(GL_TEXTURE_2D);
	
	for (type = 1; type < MAX_WEATHER_TYPES; ++type)
		if (!weather_defs[type].use_sprites && weather_drops_count[type] > 0)
		{
			glLineWidth(weather_defs[type].size);
			glColor4f(weather_defs[type].color[0]*light_level[0],
					  weather_defs[type].color[1]*light_level[1],
					  weather_defs[type].color[2]*light_level[2],
					  weather_defs[type].color[3]);
			glVertexPointer(3, GL_FLOAT, 0, weather_drops[type]);
	
			for (i = 0; i < weather_drops_count[type]; i += 1000) // to avoid long arrays
			{
				int nb = weather_drops_count[type] - i;
				if (nb > 1000) nb = 1000;
				glDrawArrays(GL_LINES, i, nb);
			}
		}
	
	glLineWidth(1.0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	// we then render the other precipitations as sprites
    glTexCoordPointer(2, GL_FLOAT, 5*sizeof(float), weather_particles_coords);
    glVertexPointer(3, GL_FLOAT, 5*sizeof(float), weather_particles_coords+2);
	for (type = 2; type < MAX_WEATHER_TYPES; ++type)
		if (weather_defs[type].use_sprites && weather_drops_count[type] > 0)
		{
			delta1[0] = weather_defs[type].size*(modelview[0]+modelview[1]);
			delta1[1] = weather_defs[type].size*(modelview[4]+modelview[5]);
			delta1[2] = weather_defs[type].size*(modelview[8]+modelview[9]);
			delta2[0] = weather_defs[type].size*(modelview[0]-modelview[1]);
			delta2[1] = weather_defs[type].size*(modelview[4]-modelview[5]);
			delta2[2] = weather_defs[type].size*(modelview[8]-modelview[9]);
			
			for (i = 0; i < weather_drops_count[type]; ++i)
			{
				const weather_drop *d = &weather_drops[type][i];
                const int j = i*20;
                weather_particles_coords[j+2 ] = d->pos1[0]-delta1[0];
                weather_particles_coords[j+3 ] = d->pos1[1]-delta1[1];
                weather_particles_coords[j+4 ] = d->pos1[2]-delta1[2];
                weather_particles_coords[j+7 ] = d->pos1[0]+delta2[0];
                weather_particles_coords[j+8 ] = d->pos1[1]+delta2[1];
                weather_particles_coords[j+9 ] = d->pos1[2]+delta2[2];
                weather_particles_coords[j+12] = d->pos1[0]+delta1[0];
                weather_particles_coords[j+13] = d->pos1[1]+delta1[1];
                weather_particles_coords[j+14] = d->pos1[2]+delta1[2];
                weather_particles_coords[j+17] = d->pos1[0]-delta2[0];
                weather_particles_coords[j+18] = d->pos1[1]-delta2[1];
                weather_particles_coords[j+19] = d->pos1[2]-delta2[2];
			}

			glColor4f(weather_defs[type].color[0]*light_level[0],
					  weather_defs[type].color[1]*light_level[1],
					  weather_defs[type].color[2]*light_level[2],
					  weather_defs[type].color[3]);
			
#ifdef	NEW_TEXTURES
			bind_texture(weather_defs[type].texture);
#else	/* NEW_TEXTURES */
			get_and_set_texture_id(weather_defs[type].texture);
#endif	/* NEW_TEXTURES */
            glDrawArrays(GL_QUADS, 0, weather_drops_count[type]);
		}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_BLEND);
	glPopAttrib();
#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

int weather_get_drops_count(int type)
{
	return weather_drops_count[type];
}

float weather_get_intensity()
{
	return (1.0 - weather_ratios[0]);
}

float weather_get_density_from_ratios(float ratios[MAX_WEATHER_TYPES])
{
	float density = 0.0;
	int i;
	for(i = 0; i < MAX_WEATHER_TYPES; ++i)
		density += weather_defs[i].density * ratios[i];
	return density;
}

float weather_get_density()
{
	return current_weather_density;
}

void weather_add_lightning(int type, float x, float y)
{
	if (thunders_count < MAX_THUNDERS && lightnings_defs_count > 0)
	{
		// store the thunder
		thunders[thunders_count].type = type;
		thunders[thunders_count].x_pos = x;
		thunders[thunders_count].y_pos = y;
		thunders[thunders_count].time = cur_time;
		++thunders_count;

        lightning_type = rand()%lightnings_defs_count;
        lightning_position[0] = x;
        lightning_position[1] = y;
        lightning_stop = cur_time + 200 + rand()%200;
        lightning_falling = 1;
        
        skybox_coords_from_ground_coords(lightning_sky_position,
                                         lightning_position[0] + camera_x,
                                         lightning_position[1] + camera_y);
        
        lightning_sky_position[0] -= camera_x;
        lightning_sky_position[1] -= camera_y;
        
		calc_shadow_matrix();

        if (skybox_update_delay > 0)
            skybox_update_colors();
	}
}

void weather_init_lightning_light()
{
	if (lightning_falling)
	{
		glLightfv(GL_LIGHT7, GL_AMBIENT, lightning_ambient_color);
		glLightfv(GL_LIGHT7, GL_DIFFUSE, lightning_color);
		glLightfv(GL_LIGHT7, GL_POSITION, lightning_position);
	}
}

void weather_render_lightning()
{
	if (lightning_falling)
	{
		const float *tex_coords = lightnings_defs[lightning_type].coords;
		float size = lightning_sky_position[2]*0.5*(tex_coords[2]-tex_coords[0])/(tex_coords[3]-tex_coords[1]);
		float dx = size*cosf(-rz*M_PI/180.0);
		float dy = size*sinf(-rz*M_PI/180.0);

		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_LIGHTING);
		glDisable(GL_FOG);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

#ifdef	NEW_TEXTURES
		bind_texture(lightnings_defs[lightning_type%lightnings_defs_count].texture);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(lightnings_defs[lightning_type%lightnings_defs_count].texture);
#endif	/* NEW_TEXTURES */
 
		glColor4fv(lightning_color);
		glBegin(GL_QUADS);
		glTexCoord2f(tex_coords[0], tex_coords[1]);
		glVertex3f(lightning_position[0]-dx, lightning_position[1]-dy, 0.0);
		glTexCoord2f(tex_coords[2], tex_coords[1]);
		glVertex3f(lightning_position[0]+dx, lightning_position[1]+dy, 0.0);
		glTexCoord2f(tex_coords[2], tex_coords[3]);
		glVertex3f(lightning_sky_position[0]+dx, lightning_sky_position[1]+dy, lightning_sky_position[2]);
		glTexCoord2f(tex_coords[0], tex_coords[3]);
		glVertex3f(lightning_sky_position[0]-dx, lightning_sky_position[1]-dy, lightning_sky_position[2]);
		glEnd();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPopAttrib();
	}
}

float weather_get_lightning_intensity(float x, float y)
{
	float dx = x-lightning_position[0];
	float dy = y-lightning_position[1];
	float dist = dx*dx + dy*dy;

	if (!lightning_falling || dist > LIGHTNING_LIGHT_RADIUS*LIGHTNING_LIGHT_RADIUS)
		return 0.0;
	else
		return 1.0 - dist / (LIGHTNING_LIGHT_RADIUS*LIGHTNING_LIGHT_RADIUS);
}

#ifdef NEW_SOUND
void weather_sound_control()
{
	static Uint32 last_sound_update = 0;
	int i;

	if (cur_time < last_sound_update + 200) return;
	else last_sound_update = cur_time;

	if (!sound_on)
	{
		rain_sound = 0;
	}
	else
	{
		if (weather_ratios[WEATHER_RAIN] > 0.0)
		{
			// Torg: Only load sounds when we need them so we aren't wasting sources.
			// This is really only for NEW_SOUND.
			if (rain_sound == 0)
#ifdef NEW_SOUND
				rain_sound = add_server_sound(snd_rain, 0, 0, weather_ratios[WEATHER_RAIN]);
			else
				sound_source_set_gain(rain_sound, weather_ratios[WEATHER_RAIN]);
#endif	//NEW_SOUND
		}
		else
		{
			if (rain_sound > 0)
			{
				stop_sound(rain_sound);
				rain_sound = 0;
			}
		}
	}
		
	for (i = 0; i < thunders_count; )
	{
		float dx = thunders[i].x_pos + camera_x;
		float dy = thunders[i].y_pos + camera_y;
		float dist = sqrtf(dx*dx + dy*dy);
		if (cur_time >= thunders[i].time + dist/SOUND_SPEED)
		{
			if (sound_on)
			{
				int snd_thunder = 0;
			
				switch (thunders[i].type)
				{
				case 0:  snd_thunder = snd_thndr_1; break;
				case 1:  snd_thunder = snd_thndr_2; break;
				case 2:  snd_thunder = snd_thndr_3; break;
				case 3:  snd_thunder = snd_thndr_4; break;
				case 4:  snd_thunder = snd_thndr_5; break;
				default: snd_thunder = 0;
				}
			
				if (snd_thunder)
				{
#ifdef NEW_SOUND
					add_server_sound(snd_thunder, 0, 0, 1.0f);
#endif	//NEW_SOUND
				}
			}
			
			// we remove the thunder from the list
			if (i < --thunders_count)
				memcpy(&thunders[i], &thunders[thunders_count], sizeof(thunder));
		}
		else ++i;
	}
}
#endif	//NEW_SOUND

#ifdef NEW_SOUND
float weather_adjust_gain(float in_gain, int in_cookie)
{
	if (weather_ratios[WEATHER_RAIN] > 0.0 && in_cookie != rain_sound)
	{
		// Dim down all the sounds, except the rain
		return in_gain * (1.0f - 0.5*weather_ratios[WEATHER_RAIN]);
	}

	return in_gain;
}
#endif // NEW_SOUND

int weather_parse_effect(xmlNode *node)
{
	xmlNode *item;
	xmlAttr *attr;
	int ok = 1;
	int id = -1;

	for (attr = node->properties; attr; attr = attr->next)
	{
		if (attr->type == XML_ATTRIBUTE_NODE)
		{
			if (!xmlStrcasecmp (attr->name, (xmlChar*)"id"))
				id = atoi((char*)attr->children->content);
			else {
				LOG_ERROR("unknown attribute for effect: %s", (char*)attr->name);
				ok = 0;
			}
		}
	}

	if (id < 1)
	{
		LOG_ERROR("wrong or missing id for weather effect");
		return 0;
	}

	for(item = node->children; item; item = item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp(item->name, (xmlChar*)"sprites") == 0) {
				weather_defs[id].use_sprites = get_bool_value(item);
			}
			else if (xmlStrcasecmp(item->name, (xmlChar*)"density") == 0) {
				weather_defs[id].density = get_float_value(item);
			}
			else if (xmlStrcasecmp(item->name, (xmlChar*)"color") == 0) {
				weather_defs[id].color[0] = 0.0;
				weather_defs[id].color[1] = 0.0;
				weather_defs[id].color[2] = 0.0;
				weather_defs[id].color[3] = 0.0;
				for (attr = item->properties; attr; attr = attr->next)
				{
					if (attr->type == XML_ATTRIBUTE_NODE)
					{
						if (!xmlStrcasecmp (attr->name, (xmlChar*)"r"))
							weather_defs[id].color[0] = atof((char*)attr->children->content);
						else if (!xmlStrcasecmp (attr->name, (xmlChar*)"g"))
							weather_defs[id].color[1] = atof((char*)attr->children->content);
						else if (!xmlStrcasecmp (attr->name, (xmlChar*)"b"))
							weather_defs[id].color[2] = atof((char*)attr->children->content);
						else if (!xmlStrcasecmp (attr->name, (xmlChar*)"a"))
							weather_defs[id].color[3] = atof((char*)attr->children->content);
						else {
							LOG_ERROR("unknown attribute for weather effect color: %s", (char*)attr->name);
							ok = 0;
						}
					}
				}
				weather_defs[id].color[0] /= 255.0;
				weather_defs[id].color[1] /= 255.0;
				weather_defs[id].color[2] /= 255.0;
				weather_defs[id].color[3] /= 255.0;
			}
			else if (xmlStrcasecmp(item->name, (xmlChar*)"size") == 0) {
				weather_defs[id].size = get_float_value(item);
			}
			else if (xmlStrcasecmp(item->name, (xmlChar*)"speed") == 0) {
				weather_defs[id].speed = get_float_value(item);
			}
			else if (xmlStrcasecmp(item->name, (xmlChar*)"wind_effect") == 0) {
				weather_defs[id].wind_effect = get_float_value(item);
			}
			else if (xmlStrcasecmp(item->name, (xmlChar*)"texture") == 0) {
#ifdef	NEW_TEXTURES
				weather_defs[id].texture = load_texture_cached((char*)item->children->content, tt_mesh);
#else	/* NEW_TEXTURES */
				weather_defs[id].texture = load_texture_cache((char*)item->children->content, 0);
#endif	/* NEW_TEXTURES */
			}
			else {
				LOG_ERROR("unknown node for weather effect: %s", item->name);
				ok = 0;
			}
		}
		else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= weather_parse_effect(item->children);
		}
	}

	return ok;
}

int weather_parse_lightning(xmlNode *node)
{
	xmlAttr *attr;
	int ok = 1;
	int id = lightnings_defs_count++;

	lightnings_defs[id].texture = -1;
	lightnings_defs[id].coords[0] = 0.0;
	lightnings_defs[id].coords[1] = 0.0;
	lightnings_defs[id].coords[2] = 0.0;
	lightnings_defs[id].coords[3] = 0.0;

	for (attr = node->properties; attr; attr = attr->next)
	{
		if (attr->type == XML_ATTRIBUTE_NODE)
		{
			if (!xmlStrcasecmp (attr->name, (xmlChar*)"texture"))
#ifdef	NEW_TEXTURES
				lightnings_defs[id].texture = load_texture_cached((char*)attr->children->content, tt_mesh);
#else	/* NEW_TEXTURES */
				lightnings_defs[id].texture = load_texture_cache((char*)attr->children->content, 0);
#endif	/* NEW_TEXTURES */
			else if (!xmlStrcasecmp (attr->name, (xmlChar*)"x1"))
				lightnings_defs[id].coords[0] = atof((char*)attr->children->content);
			else if (!xmlStrcasecmp (attr->name, (xmlChar*)"y1"))
				lightnings_defs[id].coords[1] = atof((char*)attr->children->content);
			else if (!xmlStrcasecmp (attr->name, (xmlChar*)"x2"))
				lightnings_defs[id].coords[2] = atof((char*)attr->children->content);
			else if (!xmlStrcasecmp (attr->name, (xmlChar*)"y2"))
				lightnings_defs[id].coords[3] = atof((char*)attr->children->content);
			else {
				LOG_ERROR("unknown attribute for weather effect color: %s", (char*)attr->name);
				ok = 0;
			}
		}
	}

	return ok;
}

int weather_parse_defs(xmlNode *node)
{
	xmlNode *def;
	int ok = 1;

	for (def = node->children; def; def = def->next) {
		if (def->type == XML_ELEMENT_NODE)
			if (xmlStrcasecmp(def->name, (xmlChar*)"effect") == 0) {
				ok &= weather_parse_effect(def);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"lightning") == 0) {
				ok &= weather_parse_lightning(def);
			}
			else {
				LOG_ERROR("unknown element for weather: %s", def->name);
				ok = 0;
			}
		else if (def->type == XML_ENTITY_REF_NODE) {
			ok &= weather_parse_defs(def->children);
		}
	}

	return ok;
}

int weather_read_defs(const char *file_name)
{
	xmlNode *root;
	xmlDoc *doc;
	int ok = 1;

	doc = xmlReadFile(file_name, NULL, 0);
	if (doc == NULL) {
		LOG_ERROR("Unable to read weather definition file %s", file_name);
		return 0;
	}

	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		LOG_ERROR("Unable to parse weather definition file %s", file_name);
		ok = 0;
	} else if (xmlStrcasecmp(root->name, (xmlChar*)"weather") != 0) {
		LOG_ERROR("Unknown key \"%s\" (\"weather\" expected).", root->name);
		ok = 0;
	} else {
		ok = weather_parse_defs(root);
	}

	xmlFreeDoc(doc);
	return ok;
}













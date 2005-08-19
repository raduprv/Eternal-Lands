#include <stdlib.h>
#include <math.h>
#include "global.h"
#include "pathfinder.h"
#include "lights.h"

#define MAX_THUNDERS 5

#define MAX_RAIN_DROPS 25000
#define RAIN_SPEED 2
#define RAIN_DROP_LEN 5

typedef struct
{
	int in_use;
	int light_offset;
	int seconds_till_sound;
	int thunder_type;
	int time_since_started;
	int x;
	int y;
	int rot;
}thunder;

typedef struct
{
	float x1[3], x2[3]; // vertices
}rain_drop;

int seconds_till_rain_starts=-1;
int seconds_till_rain_stops=-1;
int is_raining=0;
int rain_sound=0;
int weather_light_offset=0;
int rain_light_offset=0;
int thunder_light_offset;
thunder thunders[MAX_THUNDERS];
int lightning_text;

Uint32 rain_control_counter=0;
Uint32 thunder_control_counter=0;
int num_rain_drops=0, max_rain_drops=0;

rain_drop rain_drops[MAX_RAIN_DROPS];   /*!< Defines the number of rain drops */
int rain_table_valid = 0;

int use_fog=1;

#ifdef DEBUG
int rain_calls = 0;
int last_rain_calls = 0;
#endif

#ifdef DEBUG
GLfloat rain_color[4] = { 0.8f, 0.8f, 0.8f, 0.13f };
#else
static GLfloat rain_color[4] = { 0.8f, 0.8f, 0.8f, 0.13f }; // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
#endif
float rain_strength_bias = 1.0f;
GLfloat fogColor[4] = { 0.0f, 0.0f, 0.0f , 0.13f }; // use same alpha like in rain_color
float fogAlpha = 0.0f;

void build_rain_table()
{
	int i; float x, y, z;

	LOCK_ACTORS_LISTS();
	if (!your_actor) {
		UNLOCK_ACTORS_LISTS();
		return;
	}
	x = your_actor->x_pos; 
	y = your_actor->y_pos; 
	z = your_actor->z_pos; 
	UNLOCK_ACTORS_LISTS();

	for(i=0;i<MAX_RAIN_DROPS;i++)
		{
			rain_drops[i].x1[0] = x +  20.0f * ((float)rand() / RAND_MAX) - 10.0f;
			rain_drops[i].x1[1] = y +  20.0f * ((float)rand() / RAND_MAX) - 10.0f;
			rain_drops[i].x1[2] = z +  10.0f * ((float)rand() / RAND_MAX) + 10.0f;
			rain_drops[i].x2[0] = rain_drops[i].x1[0];
			rain_drops[i].x2[1] = rain_drops[i].x1[1];
			rain_drops[i].x2[2] = rain_drops[i].x1[2] - 0.08f;
		}
}

void update_rain()
{
	int i; float x, y, z;

	LOCK_ACTORS_LISTS();
	if (!your_actor) {
		UNLOCK_ACTORS_LISTS();
		return;
	}
	x = your_actor->x_pos; 
	y = your_actor->y_pos; 
	z = your_actor->z_pos; 
	UNLOCK_ACTORS_LISTS();

	for(i=0;i<num_rain_drops;i++)
		{
			if(rain_drops[i].x1[2] < -1.0f)
				{
					if ((seconds_till_rain_stops < 0) || (seconds_till_rain_stops > 62) || (seconds_till_rain_stops > 62 + ((30*i)/MAX_RAIN_DROPS))) {
						rain_drops[i].x1[0] = x + 20.0f * ((float)rand() / RAND_MAX) - 10.0f;
						rain_drops[i].x1[1] = y + 20.0f * ((float)rand() / RAND_MAX) - 10.0f;
						rain_drops[i].x1[2] = z + 10.0f * ((float)rand() / RAND_MAX) +  2.0f;
						rain_drops[i].x2[0] = rain_drops[i].x1[0];
						rain_drops[i].x2[1] = rain_drops[i].x1[1];
						rain_drops[i].x2[2] = rain_drops[i].x1[2] - 0.08f;

#ifdef DEBUG
						rain_calls++;
#endif
					}
				} else {
					rain_drops[i].x1[2] -= 0.40f;
					rain_drops[i].x2[2] -= 0.40f;
				}
		}
}

void render_rain()
{
	if(!num_rain_drops) return;
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4fv(rain_color);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	glVertexPointer(3,GL_FLOAT,0,rain_drops);
	glDrawArrays(GL_LINES,0,num_rain_drops);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glPopAttrib();
}

void rain_control()
{
	float rain_light_bias = 0.8 + 0.2*rain_strength_bias;
#ifdef DEBUG
	last_rain_calls = rain_calls;
	rain_calls -= last_rain_calls;
#endif

	if(rain_control_counter+1000<cur_time)
		{
			rain_control_counter=cur_time;
		}
	else return;

#ifdef NEW_CLIENT
	/* disable rain on snow maps for now */
	if (map_flags & SNOW) {
		is_raining = 0;
		if (rain_sound) {
			stop_sound(rain_sound);
			rain_sound = 0;
		}
		seconds_till_rain_starts = -1;
		seconds_till_rain_stops = -1;
		rain_light_offset=0;
		return;
	}
#endif

	//prepare to stop the rain?
	if(seconds_till_rain_stops!=-1) {
		rain_table_valid = 0;
		// gracefully stop rain
		if (seconds_till_rain_stops >= 90) {
			is_raining=1;
			if (!rain_sound) rain_sound=add_sound_object(snd_rain,0,0,0,1);
			num_rain_drops = rain_strength_bias*MAX_RAIN_DROPS;
			if (rain_sound) sound_object_set_gain(rain_sound, rain_strength_bias);
			seconds_till_rain_stops--;
		} else if (seconds_till_rain_stops > 60) {
			is_raining=1;
			if (!rain_sound) rain_sound=add_sound_object(snd_rain,0,0,0,1);
			num_rain_drops = rain_strength_bias*((seconds_till_rain_stops - 60)*MAX_RAIN_DROPS)/30;
			if (rain_sound) sound_object_set_gain(rain_sound, rain_strength_bias*(seconds_till_rain_stops - 60)/30.0f);
			seconds_till_rain_stops--;
		} else if(seconds_till_rain_stops) {
			if (is_raining) is_raining = 0;
			if(rain_sound) {
				stop_sound(rain_sound);
				rain_sound=0;
			}
			num_rain_drops = 0;
			seconds_till_rain_stops--;
		} else {
			if (is_raining) is_raining = 0;
			if(rain_sound) {
				stop_sound(rain_sound);
				rain_sound=0;
			}
			num_rain_drops = 0;
			seconds_till_rain_stops = -1;
			rain_light_offset=0;
			return;
		}

		//make it lighter each 3 seconds
		rain_light_offset=rain_light_bias*seconds_till_rain_stops/3;
		if(rain_light_offset<0)rain_light_offset=0;
	//prepare to start the rain?
	} else if(seconds_till_rain_starts!=-1) {
		if (!rain_table_valid) {
			build_rain_table();
			rain_table_valid = 1;
		}
		// gracefully start rain
		if (seconds_till_rain_starts >= 30) {
			num_rain_drops = 0;
			if (rain_sound) sound_object_set_gain(rain_sound, 0.0f);
			seconds_till_rain_starts--;
		} else if(seconds_till_rain_starts) {
			if(!is_raining) {
				is_raining=1;
				if (!rain_sound) rain_sound=add_sound_object(snd_rain,0,0,0,1);
			}
			num_rain_drops = rain_strength_bias*(30-seconds_till_rain_starts)*MAX_RAIN_DROPS/30.0f;
			if (rain_sound) sound_object_set_gain(rain_sound, rain_strength_bias*(1.0f - seconds_till_rain_starts/30.0f));
			seconds_till_rain_starts--;
		} else {
			if(!is_raining) {
				is_raining=1;
				if (!rain_sound) rain_sound=add_sound_object(snd_rain,0,0,0,1);
			}
			num_rain_drops = rain_strength_bias*MAX_RAIN_DROPS;
			seconds_till_rain_starts=-1;
			rain_table_valid = 0;
			rain_light_offset = rain_light_bias*30;
			return;
		}
		//make it darker each 3 seconds
		rain_light_offset=rain_light_bias*(30-seconds_till_rain_starts/3);
		if(rain_light_offset<0)rain_light_offset=0;
	} else {
		// neither ==> keep status
		rain_table_valid = 0;
		if (is_raining) {
			num_rain_drops = rain_strength_bias*MAX_RAIN_DROPS;
			if (rain_sound) {
				sound_object_set_gain(rain_sound, rain_strength_bias);
			} else {
				rain_sound=add_sound_object(snd_rain,0,0,0,1);
			}
		} else {
			num_rain_drops = 0;
			if (rain_sound) {
				stop_sound(rain_sound);
				rain_sound = 0;
			}
		}
	}
}


void thunder_control()
{
	int i;

#ifdef NEW_CLIENT
	if(map_flags&SNOW) return;
#endif

	if(thunder_control_counter+100<cur_time)
		{
			thunder_control_counter=cur_time;
		}
	else return;

	for(i=0;i<MAX_THUNDERS;i++)
		{
			if(thunders[i].in_use)
				{
					if(thunders[i].time_since_started<5)thunders[i].light_offset=thunders[i].time_since_started;
					else if(thunders[i].time_since_started>5 && thunders[i].time_since_started<10)thunders[i].light_offset=5;
					else if(thunders[i].time_since_started>10 &&thunders[i].time_since_started<15)thunders[i].light_offset=15-thunders[i].time_since_started;
					//should we start the sound?
					if(thunders[i].seconds_till_sound!=-1 && thunders[i].seconds_till_sound>=0)
						{
							thunders[i].seconds_till_sound--;
							if(!thunders[i].seconds_till_sound)
								{
									switch(thunders[i].thunder_type) {
									case 0:
										add_sound_object(snd_thndr_1,0,0,0,0);
										break;
									case 1:
										add_sound_object(snd_thndr_2,0,0,0,0);
										break;
									case 2:
										add_sound_object(snd_thndr_3,0,0,0,0);
										break;
									case 3:
										add_sound_object(snd_thndr_4,0,0,0,0);
										break;
									case 4:
										add_sound_object(snd_thndr_5,0,0,0,0);
										break;
									}
									thunders[i].seconds_till_sound=-1;//we are done with this sound

								}
						}
					thunders[i].time_since_started++;
					//is this thunder expired?
					if(thunders[i].time_since_started>20 && thunders[i].seconds_till_sound==-1)
						thunders[i].in_use=0;
				}
		}

}

void add_thunder(int type,int sound_delay)
{
	int i;

#ifdef NEW_CLIENT
	if(map_flags&SNOW) return;
#endif

	for(i=0;i<MAX_THUNDERS;i++)
		{
			if(!thunders[i].in_use)
				{
					thunders[i].in_use=1;
					thunders[i].light_offset=0;
					thunders[i].seconds_till_sound=sound_delay;
					thunders[i].thunder_type=type;
					thunders[i].time_since_started=0;
					thunders[i].x=rand()%500;
					thunders[i].y=rand()%500;
					thunders[i].rot=rand()%360;
					return;
				}
		}

}

void get_weather_light_level()
{
	int i;

	thunder_light_offset=0;

	for(i=0;i<MAX_THUNDERS;i++)
		{
			if(thunders[i].in_use)
				thunder_light_offset+=thunders[i].light_offset;
		}

	//thunders light is positive, while rain light is negative
	weather_light_offset=rain_light_offset;
}

void clear_thunders()
{
	int i;

	for(i=0;i<MAX_THUNDERS;i++)
		{
			thunders[i].in_use=0;
		}
}

void render_fog() {
	static GLfloat minDensity = 0.03f, maxDensity = 0.15f;
	GLfloat fogDensity;
	GLfloat rainStrength, rainAlpha, diffuseBias;
	int i; float tmpf;

#ifdef NEW_CLIENT
	if(!(map_flags&SNOW)) {
#endif
		rainStrength = num_rain_drops/(float) MAX_RAIN_DROPS;
		rainAlpha = 0.2f*rain_color[3]*rainStrength;
			// in dungeons and at night we use smaller light sources ==> less diffuse light
		diffuseBias = (dungeon || !is_day)? 0.2f : 0.5f;
		for (i=0; i<3; i++) {
			// blend ambient and diffuse light to build a base fog color
			GLfloat tmp = 0.5f*sun_ambient_light[i] + diffuseBias*difuse_light[i];
			if (is_raining) {
			// blend base color with rain color
				fogColor[i] = (1.0f - rainAlpha)*tmp + rainAlpha*rain_color[i];
			} else {
				fogColor[i] = tmp;
			}
		}
	
		// interpolate fog distance between min and max depending on rain
		// TODO: Is linear interpolation the best idea here?
		fogDensity = (is_raining)?  (rainStrength*maxDensity + (1.0f - rainStrength)*minDensity) : minDensity;
		tmpf = exp(-10.0f*fogDensity);
		fogAlpha = 1.0f - tmpf*tmpf;
		
#ifdef NEW_CLIENT
	} else {
		rainStrength = rain_strength_bias;
		rainAlpha = 0.2f*rain_color[3]*rainStrength;
			// in dungeons and at night we use smaller light sources ==> less diffuse light
		diffuseBias = (dungeon || !is_day)? 0.2f : 0.5f;
		for (i=0; i<3; i++) {
			// blend ambient and diffuse light to build a base fog color
			GLfloat tmp = 0.5f*sun_ambient_light[i] + diffuseBias*difuse_light[i];
			if (is_raining) {
			// blend base color with rain color
				fogColor[i] = (1.0f - rainAlpha)*tmp + rainAlpha*rain_color[i];
			} else {
				fogColor[i] = tmp;
			}
		}
	}
#endif

	// interpolate fog distance between min and max depending on rain
	// TODO: Is linear interpolation the best idea here?
	fogDensity = (is_raining)?  (rainStrength*maxDensity + (1.0f - rainStrength)*minDensity) : minDensity;
	tmpf = exp(-10.0f*fogDensity);
	fogAlpha = 1.0f - tmpf*tmpf;
	
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
	glFogf(GL_FOG_DENSITY, fogDensity);
	glFogfv(GL_FOG_COLOR, fogColor);
}

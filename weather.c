#ifdef NEW_WEATHER

#include <math.h>
#include <string.h>

#include "global.h"
#include "text.h"
#include "multiplayer.h"
#include "map_io.h"
#include "shadows.h"
#include "lights.h"
#include "actors.h"
#include "sound.h"

#define WEATHER_TYPE     0x07
#define WEATHER_NONE     0x00
#define WEATHER_RAIN     0x01
#define WEATHER_SNOW     0x02
#define WEATHER_SAND     0x03
#define WEATHER_TYPE4    0x04
#define WEATHER_TYPE5    0x05
#define WEATHER_TYPE6    0x06
#define WEATHER_TYPE7    0x07
#define WEATHER_ACTIVE   0x08
#define WEATHER_STARTING 0x10
#define WEATHER_STOPPING 0x20

#define WEATHER_DARKEN     30000 // ms
#define WEATHER_FADEIN     30000 // ms
#define WEATHER_FADEOUT    WEATHER_FADEIN // need to be equal
#define WEATHER_CLEAROFF   WEATHER_DARKEN // need to be equal
#define WEATHER_AFTER_FADE  5000 // ms

#define WEATHER_NUM_RAND 0x10000

#define MAX_RAIN_DROPS 25000

#define MAX_THUNDERS 20

typedef struct {
	int type;
	Uint32 time;
	char done;
} thunder;

typedef struct {
	float x1[3], x2[3]; // vertices
} rain_drop;

int use_fog = 1;

long weather_flags = WEATHER_NONE;
Uint32 weather_start_time;
Uint32 weather_stop_time;
Uint32 weather_time;
float weather_severity = 1.0;

const float rain_color[4] = { 0.8f, 0.8f, 0.8f, 0.13f };
const float snow_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const float sand_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const float default_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

const float
	min_fog = 0.03f,
	rain_fog = 0.15f,
	snow_fog = 0.15f,
	sand_fog = 0.15f;

int weather_rand_index = 0;
int weather_rand_values[WEATHER_NUM_RAND];

thunder thunders[MAX_THUNDERS];
int num_thunders = 0;
int lightning_stop = 0;

rain_drop rain_drops[MAX_RAIN_DROPS];   /*!< Defines the number of rain drops */

float fog_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float fog_alpha;

long get_weather_type();
float get_fadein_bias();
float get_fadeout_bias();
float get_fadeinout_bias();
float interpolate(float affinity, float first, float second);
void weather_srand(int seed);
int weather_rand();
void update_rain(int ticks, int num_rain_drops);
void render_rain(int num_rain_drops);
void update_snow(int ticks, float severity);
void render_snow(float severity);
void update_sand(int ticks, float severity);
void render_sand(float severity);

long get_weather_type()
{
	long result;

	switch (map_flags & (PLAINS|SNOW|DESERT)) {
#ifdef DEBUG
		case 0:      result = WEATHER_RAIN; break;
#else
		case 0:      result = WEATHER_NONE; break;
#endif
		case PLAINS: result = WEATHER_RAIN; break;
		case SNOW:   result = WEATHER_SNOW; break;
		case DESERT: result = WEATHER_SAND; break;
		default:
			LOG_TO_CONSOLE(c_red2, "Invalid map flags, combination of PLAINS, SNOW and DESERT not allowed");
			result = WEATHER_NONE;
	}

	return result;
}

/*
 * |<-----seconds_till_start----->|<--WEATHER_FADEIN-->|                   
 * +------------------------------+--------------------+--------------------------------------------------------->
 *  \                             :\                   :\
 *   receiving                    : starting           : reaching
 *   START_RAIN                   : fade in            : severity
 *                                :                    :
 *                                                  |<-----seconds_till_stop----->|
 *                                                  |       |<--WEATHER_FADEOUT-->|                   
 * +------------------------------+-----------------+--+----+---------------------+------------------------------>
 *                                :                /   :    : \                   : \
 *                                :      receiving     :    : starting            : reaching
 *                                :      STOP_RAIN     :    : fade out            : stop    
 *                                :                    :    :                     :
 *                                :                    :  |<-----seconds_till_start----->|<--WEATHER_FADEIN-->|                   
 * +------------------------------+--------------------+--+-+---------------------+------+--------------------+-->
 *                                :                    :   \:                     :      :\                   :\
 *                                :                    :    receiving             :      : starting           : reaching
 *                                :                    :    START_RAIN            :      : fade in            : severity
 *                                |                    |    |                     |      |                    |
 *                          _     |                    |    |                     |      |                    |
 *     effective severity _|      |              ......|::::|.......              |      |              ......|::
 *                         |_     |.......:::::::::::::|::::|:::::::::::::::......|      |.......:::::::::::::|::
 *                                                      
 */
float get_fadein_bias()
{
	// has fading in started yet?
	if (weather_time >= weather_start_time) {
		// are we still fading in?
		if (weather_time < weather_start_time + WEATHER_FADEIN) {
			return ((float)(weather_time - weather_start_time)) / WEATHER_FADEIN;
		} else {
			return 1.0f;
		}
	} else {
		return 0.0f;
	}
}

float get_fadeout_bias()
{
	// has fading out started yet?
	if (weather_time >= weather_stop_time - WEATHER_FADEOUT) {
		// are we still fading out?
		if (weather_time < weather_stop_time) {
			return ((float)(weather_stop_time - weather_time)) / WEATHER_FADEOUT;
		} else {
			return 0.0f;
		}
	} else {
		return 1.0f;
	}
}

float get_fadeinout_bias()
{
	if (weather_flags & WEATHER_ACTIVE) {
		switch (weather_flags & (WEATHER_STARTING|WEATHER_STOPPING)) {
			case 0:
				// neither fading in nor out
				return 1.0f;

			case WEATHER_STARTING:
				// fading in
				return get_fadein_bias();

			case WEATHER_STOPPING:
				// fading out
				return get_fadeout_bias();

			case WEATHER_STARTING|WEATHER_STOPPING:
				// possible overlap. If there is no overlap, one of both is 0 (1)
				// Lachesis: please avoid equality.
				if (weather_start_time + WEATHER_FADEIN <= weather_stop_time) {
					// early stop
					return get_fadein_bias() + get_fadeout_bias() - 1.0f;
				} else {
					// early start
					return get_fadein_bias() + get_fadeout_bias();
				}

			default: return 1.0f;
		}
	} else {
		return 0.0f;
	}
}

float interpolate(float affinity, float first, float second)
{
	// Lachesis: I like linear interpolation, but cosine would work too
	return (1.0f - affinity)*first + affinity*second;
}

void weather_srand(int seed) 
{
	int i;

	srand(seed);
	for (i = 0; i < WEATHER_NUM_RAND; i++) {
		weather_rand_values[i] = rand();
	}

	weather_rand_index = 0;
}

int weather_rand() {
	int result = weather_rand_values[weather_rand_index];
	weather_rand_index = (weather_rand_index + 1) % WEATHER_NUM_RAND;
	return result;
}

void update_rain(int ticks, int num_rain_drops)
{
	int i; float x, y, z;

	if (num_rain_drops > MAX_RAIN_DROPS) num_rain_drops = MAX_RAIN_DROPS;

	LOCK_ACTORS_LISTS();
	if (!your_actor) {
		UNLOCK_ACTORS_LISTS();
		return;
	}
	x = your_actor->x_pos; 
	y = your_actor->y_pos; 
	z = your_actor->z_pos; 
	UNLOCK_ACTORS_LISTS();

	if (ticks) {
		// update
		for(i=0;i<num_rain_drops;i++)
		{
			if(rain_drops[i].x1[2] < -1.0f)
			{
				rain_drops[i].x1[0] = x + 20.0f * ((float)weather_rand() / RAND_MAX) - 10.0f;
				rain_drops[i].x1[1] = y + 20.0f * ((float)weather_rand() / RAND_MAX) - 10.0f;
				rain_drops[i].x1[2] = z + 10.0f * ((float)weather_rand() / RAND_MAX) +  2.0f;
				rain_drops[i].x2[0] = rain_drops[i].x1[0];
				rain_drops[i].x2[1] = rain_drops[i].x1[1];
				rain_drops[i].x2[2] = rain_drops[i].x1[2] - 0.08f;
			} else {
				rain_drops[i].x1[2] -= 0.03f*ticks;
				rain_drops[i].x2[2] -= 0.03f*ticks;
			}
		}
	} else {
		// init
		for(i=0;i<MAX_RAIN_DROPS;i++)
		{
			rain_drops[i].x1[0] = x +  20.0f * ((float)weather_rand() / RAND_MAX) - 10.0f;
			rain_drops[i].x1[1] = y +  20.0f * ((float)weather_rand() / RAND_MAX) - 10.0f;
			rain_drops[i].x1[2] = z +  10.0f * ((float)weather_rand() / RAND_MAX) + 10.0f;
			rain_drops[i].x2[0] = rain_drops[i].x1[0];
			rain_drops[i].x2[1] = rain_drops[i].x1[1];
			rain_drops[i].x2[2] = rain_drops[i].x1[2] - 0.08f;
		}
	}
}

void render_rain(int num_rain_drops)
{
	if (!num_rain_drops) return;

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

void update_snow(int ticks, float severity)
{
}

void render_snow(float severity)
{
}

void update_sand(int ticks, float severity)
{
}

void render_sand(float severity)
{
}

void init_weather() {
	weather_srand(SDL_GetTicks());
}

void start_weather(int seconds_till_start, float severity)
{
	// if weather effects are already active, something went wrong
	if (weather_flags & WEATHER_STARTING) {
		LOG_TO_CONSOLE(c_red2, "Premature start of weather effect!");
	}

	// mark time when effect is intended to start
	weather_start_time = weather_time + 1000*seconds_till_start;
	// severity of effect
	weather_severity = severity;
	// determine type of effect
	weather_flags = (weather_flags & ~WEATHER_TYPE) | get_weather_type();
	// switch weather on
	weather_flags |= WEATHER_ACTIVE | WEATHER_STARTING; 
}

void stop_weather(int seconds_till_stop, float severity)
{
	if (! (weather_flags & WEATHER_ACTIVE)) {
		// We missed the start. So let's set up the data
		// severity of effect
		weather_severity = severity;
		// determine type of effect
		weather_flags = (weather_flags & ~WEATHER_TYPE) | get_weather_type();
	} else {
		if (weather_flags & WEATHER_STOPPING) {
			// WTH? we are already stopping!
			LOG_TO_CONSOLE(c_red2, "Double stop of weather effect!");
		}
	}

	weather_stop_time = weather_time + 1000*seconds_till_stop;
	// start the fade out
	weather_flags |= WEATHER_ACTIVE | WEATHER_STOPPING;
}

void clear_weather() {
	weather_flags = WEATHER_NONE;
	num_thunders = 0;
}

void render_fog()
{
	float current_severity = weather_severity * get_fadeinout_bias();
	float density;
	int i;
	float particle_alpha, diffuse_bias, tmpf;
	char have_particles;

	// if weather effect is active, get type-dependent data
	switch (weather_flags & (WEATHER_TYPE|WEATHER_ACTIVE)) {
		case WEATHER_RAIN|WEATHER_ACTIVE: 
			density = interpolate(current_severity, min_fog, rain_fog);
			memcpy(fog_color, rain_color, sizeof(fog_color));
			have_particles = 1;
			break;
		case WEATHER_SNOW|WEATHER_ACTIVE: 
			density = interpolate(current_severity, min_fog, snow_fog); 
			memcpy(fog_color, snow_color, sizeof(fog_color));
			have_particles = 1;
			break;
		case WEATHER_SAND|WEATHER_ACTIVE: 
			density = interpolate(current_severity, min_fog, sand_fog); 
			memcpy(fog_color, sand_color, sizeof(fog_color));
			have_particles = 1;
			break;
		default: 
			density = min_fog;
			memcpy(fog_color, default_color, sizeof(fog_color));
			have_particles = 0;
	}

	// estimate portion of scene colours that the fog covers
	tmpf = exp(-10.0f*density);
	fog_alpha = 1.0f - tmpf*tmpf;

	// estimate portion of scene colours that the particles cover
	particle_alpha = (have_particles)? 0.2f*fog_color[3]*current_severity : 0.0f;
	// in dungeons and at night we use smaller light sources ==> less diffuse light
	diffuse_bias = (dungeon || !is_day)? 0.2f : 0.5f;

	// compute fog color
	for (i = 0; i < 3; i++) {
		// blend ambient and diffuse light to build a base fog color
		float tmp = 0.5f*sun_ambient_light[i] + diffuse_bias*difuse_light[i];
		if (have_particles) {
			// blend base color with weather particle color
			fog_color[i] = interpolate(particle_alpha, tmp, fog_color[i]);
		} else {
			fog_color[i] = tmp;
		}
	}

	// set clear color to fog color
	glClearColor(fog_color[0], fog_color[1], fog_color[2], 0.0f);

	// set fog parameters
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
	glFogf(GL_FOG_DENSITY, density);
	glFogfv(GL_FOG_COLOR, fog_color);
}

void weather_color_bias(const float * src, float * dst) {
	int i;

	for (i = 0; i < 3; i++) {
		dst[i] = interpolate(fog_alpha, src[i], fog_color[i]);
	}

	dst[3] = src[3];
}

void render_weather()
{
	static Uint32 last_frame = 0;

	weather_time = SDL_GetTicks();

	if (weather_flags & WEATHER_ACTIVE) {
		// 0 means initialization
		Uint32 ticks = last_frame? weather_time - last_frame : 0;
		float severity = weather_severity * get_fadeinout_bias();
		int num_rain_drops;

		// update and render view
		switch (weather_flags & WEATHER_TYPE) {
			case WEATHER_RAIN:
				num_rain_drops = MAX_RAIN_DROPS * severity;
				update_rain(ticks, num_rain_drops);
				render_rain(num_rain_drops);
				break;
			case WEATHER_SNOW:
				update_snow(ticks, severity);
				render_snow(severity);
				break;
			case WEATHER_SAND:
				update_sand(ticks, severity);
				render_sand(severity);
		}

		last_frame = weather_time;

		// update flags
		if (weather_flags & WEATHER_STARTING) {
			if (weather_time > weather_start_time + WEATHER_CLEAROFF + WEATHER_AFTER_FADE) {
				weather_flags &= ~WEATHER_STARTING;
			}
		}

		if (weather_flags & WEATHER_STOPPING) {
			if (weather_time > weather_stop_time + WEATHER_FADEOUT + WEATHER_AFTER_FADE) {
				weather_flags &= ~WEATHER_STOPPING;
				if (! (weather_flags & WEATHER_STARTING)) {
					weather_flags &= ~WEATHER_ACTIVE;
				}
			}
		}
	} else {
		last_frame = 0;
	}
}

float get_weather_darken_bias()
{
	// has darkening started yet?
	if (weather_time >= weather_start_time - WEATHER_DARKEN) {
		// are we still darkening?
		if (weather_time < weather_start_time) {
			return 1.0f - ((float)(weather_start_time - weather_time)) / WEATHER_DARKEN;
		} else {
			return 1.0f;
		}
	} else {
		return 0.0f;
	}
}

float get_weather_clearoff_bias()
{
	// has clearing off started yet?
	if (weather_time >= weather_stop_time) {
		// are we still clearing?
		if (weather_time < weather_stop_time + WEATHER_CLEAROFF) {
			return 1.0f - ((float)(weather_time - weather_stop_time)) / WEATHER_CLEAROFF;
		} else {
			return 0.0f;
		}
	} else {
		return 1.0f;
	}
}

float get_weather_light_bias()
{
	if (weather_flags & WEATHER_ACTIVE) {
		switch (weather_flags & (WEATHER_STARTING|WEATHER_STOPPING)) {
			case 0:
				// neither fading in nor out
				return 1.0f;

			case WEATHER_STARTING:
				// fading in
				return get_weather_darken_bias();

			case WEATHER_STOPPING:
				// fading out
				return get_weather_clearoff_bias();

			case WEATHER_STARTING|WEATHER_STOPPING:
				// possible overlap. If there is no overlap, one of both is 0 (1)
				// Lachesis: please avoid equality.
				if (weather_start_time <= weather_stop_time + WEATHER_CLEAROFF) {
					// early stop
					return get_weather_darken_bias() + get_weather_clearoff_bias() - 1.0f;
				} else {
					// early start
					return get_weather_darken_bias() + get_weather_clearoff_bias();
				}
			default:
				return 1.0f;
		}
	} else {
		return 0.0f;
	}
}

float weather_bias_light(float value)
{
	static Uint32 last_call = 0;
	static char lightning = 0;

	if (weather_time >= last_call + 10) {
		if (weather_time < lightning_stop) {
			lightning = (weather_rand() <= RAND_MAX/2);
		} else {
			lightning = 0;
		}
		last_call = weather_time;
	}

	if (lightning) {
		return 1.0f;
	} else {
		const float bias = get_weather_light_bias();
		const float severity = 0.15f*weather_severity + 0.85f; // slightly bias light by weather severity
		float result = value*(1.0f - severity*bias);

		if (result < 0.0f) result = 0.0f;
		else if (result > 1.0f) result = 1.0f;
		
		return result;
	}
}

void weather_sound_control() {
	static int rain_sound = -1;

	if (rain_sound == -1) {
		int buffer;

		alGenSources(1, &rain_sound);

		buffer = get_loaded_buffer(snd_rain);
		if (buffer) alSourcei(rain_sound, AL_BUFFER, buffer);
		alSourcei(rain_sound, AL_LOOPING, AL_TRUE);
		alSourcei(rain_sound, AL_SOURCE_RELATIVE, AL_TRUE);
		alSourcef(rain_sound, AL_GAIN, 0.0f);
	}
	
	if (weather_flags & WEATHER_ACTIVE) {
		// 0 means initialization
		float severity = weather_severity * get_fadeinout_bias();
		int source_state;
		int i;

		// update and render view
		switch (weather_flags & WEATHER_TYPE) {
			case WEATHER_RAIN:
				alSourcef(rain_sound, AL_GAIN, severity);
				alGetSourcei(rain_sound, AL_SOURCE_STATE, &source_state);
				if (source_state != AL_PLAYING) alSourcePlay(rain_sound);
				break;
			case WEATHER_SNOW:
				alSourcePause(rain_sound);
				break;
			case WEATHER_SAND:
				alSourcePause(rain_sound);
				break;
			default:
				alSourcePause(rain_sound);
		}

		for (i = 0; i < num_thunders; i++) {
			if (!thunders[i].done) {
				if (cur_time >= thunders[i].time) {
					int snd_thunder = 0;
					
					switch (thunders[i].type) {
						case 0:  snd_thunder = snd_thndr_1; break;
						case 1:  snd_thunder = snd_thndr_2; break;
						case 2:  snd_thunder = snd_thndr_3; break;
						case 3:  snd_thunder = snd_thndr_4; break;
						case 4:  snd_thunder = snd_thndr_5; break;
						default: snd_thunder = 0;
					}

					if (snd_thunder) {
						add_sound_object(snd_thunder, 0, 0, 0, 0);
					}

					thunders[i].done = 1; // also allows main thread to write
				}
			}
		}
	} else {
		alSourcePause(rain_sound);
	}
}

void add_thunder(int type, int sound_delay)
{
	int i = -1, j;

	for (j = 0; j < num_thunders; j++) {
		if (thunders[j].done) {
			i = j;
			break;
		}
	}

	// no empty slots
	if (i == -1) {
		if (num_thunders < MAX_THUNDERS) {
			i = num_thunders;
			thunders[i].done = 1; // prevent timer thread from reading
			num_thunders++;
		} else {
			// no slots left
			return;
		}
	}

	thunders[i].type = type;
	thunders[i].time = weather_time + 1000*sound_delay;
	thunders[i].done = 0; // now allow timer thread to read

	// start a lightning
	lightning_stop = weather_time + 100 + (int)((300.0f * (float)weather_rand()) / RAND_MAX);
}

#else // def NEW_WEATHER

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

	if(map_flags&SNOW) return;

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

	if(map_flags&SNOW) return;

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

	if(!(map_flags&SNOW)) {
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

#endif

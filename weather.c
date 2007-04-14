#include <math.h>
#include <string.h>
#include "global.h"
#include "lights.h"

#ifdef NEW_WEATHER
#define MAX_RAIN_DROPS 100000
#else //!NEW_WEATHER
#define MAX_RAIN_DROPS 25000
#endif //NEW_WEATHER

typedef struct {
	float x1[3];
#ifndef NEW_WEATHER
	float x2[3]; // vertices
#endif //!NEW_WEATHER
} rain_drop;

rain_drop rain_drops[MAX_RAIN_DROPS];   /*!< Defines the number of rain drops */
int use_fog = 1;

int wind_speed_srv = 0,	//strength of wind, as set by server. 100 is about the max
	wind_direction_srv = 0,	//wind direction, in degrees, as set by server. 0 and 360 == north
	wind_speed = 0,	//strength of wind, based on server's setting and local randomization
	wind_direction = 0;	//wind direction, based on server's setting and local randomization

#ifdef NEW_WEATHER

#define RAND_ONE ((float)rand() / RAND_MAX)

#define WEATHER_TYPES 7	//including NONE

float weather_ratios[WEATHER_TYPES] = {	//Warning! must always add up to 1.0f!
	1.00f,	//WEATHER_NONE
	0.00f,	//WEATHER_RAIN
	0.00f,	//WEATHER_SNOW
	0.00f,	//WEATHER_HAIL
	0.00f,	//WEATHER_SAND
	0.00f,	//WEATHER_DUST
	0.00f	//WEATHER_LAVA
};

#define WEATHER_NONE	0x00
#define WEATHER_RAIN	0x01	//special case because of associated sound effects
#define WEATHER_ACTIVE	0x08
#define WEATHER_STARTING	0x10
#define WEATHER_STOPPING	0x20

#define WEATHER_DARKEN     30000 // ms
#define WEATHER_FADEIN     30000 // ms
#define WEATHER_FADEOUT    WEATHER_FADEIN // need to be equal
#define WEATHER_CLEAROFF   WEATHER_DARKEN // need to be equal
#define WEATHER_AFTER_FADE  5000 // ms

#define MAX_THUNDERS 20

typedef struct {
	int type;
	Uint32 time;
	char done;
} thunder;

long weather_flags = WEATHER_NONE;
Uint32 weather_start_time;
Uint32 weather_stop_time;
Uint32 weather_time;
float weather_severity = 1.0;
float severity_mod = 0.0f;

const float precip_colour[][4] = { 
	{ 0.0f, 0.0f, 0.0f, 0.00f },	//WEATHER_NONE
	{ 0.5f, 0.5f, 0.5f, 0.60f },	//WEATHER_RAIN
	{ 0.8f, 0.9f, 1.0f, 0.70f },	//WEATHER_SNOW
	{ 0.6f, 0.8f, 1.0f, 1.00f },	//WEATHER_HAIL
	{ 1.0f, 0.85f, 0.0f, 0.9f },	//WEATHER_SAND
	{ 0.5f, 0.3f, 0.1f, 0.75f },	//WEATHER_DUST
	{ 0.75f, 0.0f, 0.0f, 1.0f } };	//WEATHER_LAVA

const float min_fog = 0.01f;
const float fog_level[WEATHER_TYPES] = {
	0.00f,	//WEATHER_NONE
	0.15f,	//WEATHER_RAIN
	0.10f,	//WEATHER_SNOW
	0.10f,	//WEATHER_HAIL
	0.15f,	//WEATHER_SAND
	0.10f,	//WEATHER_DUST
	0.05f	//WEATHER_LAVA
};
const float precip_z_delta[WEATHER_TYPES] = {
	0.00f,	//WEATHER_NONE
	0.25f,	//WEATHER_RAIN
	0.005f,	//WEATHER_SNOW
	0.75f,	//WEATHER_HAIL
	0.04f,	//WEATHER_SAND
	0.005f,	//WEATHER_DUST
	0.05f	//WEATHER_LAVA
};
const float precip_wind_effect[WEATHER_TYPES] = {
	0.00f,	//WEATHER_NONE
	2.50f,	//WEATHER_RAIN
	0.10f,	//WEATHER_SNOW
	0.05f,	//WEATHER_HAIL
	1.50f,	//WEATHER_SAND
	0.50f,	//WEATHER_DUST
	1.00f	//WEATHER_LAVA
};
const float precip_part_per[WEATHER_TYPES] = {
	0.00f,	//WEATHER_NONE
	0.75f,	//WEATHER_RAIN
	1.00f,	//WEATHER_SNOW
	0.25f,	//WEATHER_HAIL
	1.00f,	//WEATHER_SAND
	1.00f,	//WEATHER_DUST
	0.01f	//WEATHER_LAVA
};


thunder thunders[MAX_THUNDERS];
int num_thunders = 0;
Uint32 lightning_stop = 0;

float rain_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float fog_alpha;

float get_fadein_bias();
float get_fadeout_bias();
float get_fadeinout_bias();
float interpolate(float affinity, float first, float second);
void update_rain(int ticks, int num_rain_drops);
void render_rain(int num_rain_drops);
void set_weather_ratio(Uint8 type, Uint8 value);


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



void set_weather_ratio(Uint8 type, Uint8 value){
	float fval = (float)(value%100)/100.0f;
	float total = 0.0f, cutr = 1.0f;
	int i;
	if(weather_ratios[type] >= fval){
		weather_ratios[type] = fval;
	}
	for(i = 1; i < WEATHER_TYPES; ++i){
		if(i != type){
			total += weather_ratios[i];
		}
	}
	if(total + fval >= 1.0f){
		total = 0.0f;
		cutr = (1.0f - fval)/total;
		for(i = 1; i < WEATHER_TYPES; ++i){
			if(i != type){
				weather_ratios[i] *= cutr;
				total += weather_ratios[i];
			}
		}
	}
	weather_ratios[i] = fval;
	weather_ratios[0] = 1.0f - (total + fval);
}


void get_weather_from_server(const Uint8* data){
	//first, catch non-precipitations
	if(data[0] == weather_effect_wind){
		wind_direction_srv = (2 * data[1])%360;
		wind_speed_srv = data[2];
		return;
	} else if(data[0] == weather_effect_leaves){
		//EC_TAG
		return;
	} else if(data[0] > WEATHER_TYPES){
		LOG_TO_CONSOLE(c_red1, "Server sent an unknown weather type");
		return;
		//from now on, deal with the set of precipitations
	}else if(data[2] == 0 && !(weather_flags & WEATHER_ACTIVE)){
		return;	//stop? but we're already stopped...
	} else if(data[2] != 0 && (weather_flags & WEATHER_ACTIVE)){
		set_weather_ratio(data[0], data[2]);
		return;
	} else if(data[2] != 0 && !(weather_flags & WEATHER_ACTIVE)){
		set_weather_ratio(data[0], data[2]);
		start_weather(data[1], ((float)(data[2]))/100.0f);
		return;
	} else {
		stop_weather(data[1], ((float)(data[2]))/100.0f);
	}
}



double precip_avg(const float * ary){
	double total = 0.0f;
	int i;
	for(i = 0; i < WEATHER_TYPES; ++i){
		total += ary[i] * weather_ratios[i];
	}
	return total;
}


void set_rain_color(void){
	int i, j;
	for(i = 0; i < 4; ++i){
		rain_color[i] = 0.0f;
		for(j = 0; j < WEATHER_TYPES; ++j){
			rain_color[i] += precip_colour[j][i] * weather_ratios[j];
		}
	}
	for(i = 0; i < 3; ++i){
		rain_color[i] = interpolate(weather_severity/2.0f, rain_color[i], 0.5f*sun_ambient_light[i] + ((dungeon || !is_day)? 0.2f : 0.5f)*difuse_light[i]);
	}
}

float interpolate(float affinity, float first, float second)
{
	// Lachesis: I like linear interpolation, but cosine would work too
	return (1.0f - affinity)*first + affinity*second;
}


void __inline__ make_rain_drop(int i, float x, float y, float z){
	rain_drops[i].x1[0] = x + 20.0f * RAND_ONE - 10.0f;
	rain_drops[i].x1[1] = y + 20.0f * RAND_ONE - 10.0f;
	rain_drops[i].x1[2] = z + 10.0f * RAND_ONE +  2.0f;
}

void update_wind(void){
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

void update_rain(int ticks, int num_rain_drops)
{
	int i; float x, y, z, wind_effect, x_move, y_move, max_part;
	double z_delta;

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

	z_delta = precip_avg(precip_z_delta)*ticks/100.0f;
	wind_effect = precip_avg(precip_wind_effect)/1000.0f;
	max_part = precip_avg(precip_part_per);
	x_move = wind_effect * ( sinf((float)wind_direction*M_PI/180) * wind_speed );
	y_move = wind_effect * ( cosf((float)wind_direction*M_PI/180) * wind_speed );

	if (ticks > 0) {
		// update
		for(i=0;i<num_rain_drops;i++){
			if(rain_drops[i].x1[2] < -1.0f){
				//is it below the ground? if so, do we redraw it?
				if(i < max_part*num_rain_drops){
					make_rain_drop(i, x, y, z);
				}
			} else {
				//it's moving. so find out how much by, and wrap around the X/Y if it's too far away (as wind can cause)
				rain_drops[i].x1[0] += x_move*(1.1-0.2*RAND_ONE);
				if(rain_drops[i].x1[0] < x - 20.0f){
					rain_drops[i].x1[0] = x + 20.0f;
				} else if(rain_drops[i].x1[0] > x + 20.0f){
					rain_drops[i].x1[0] = x - 20.0f;
				}
				rain_drops[i].x1[1] += y_move*(1.1-0.2*RAND_ONE);
				if(rain_drops[i].x1[1] < y - 20.0f){
					rain_drops[i].x1[1] = y + 20.0f;
				} else if(rain_drops[i].x1[1] > y + 20.0f){
					rain_drops[i].x1[1] = y - 20.0f;
				}
				rain_drops[i].x1[2] -= z_delta;
			}
		}
	} else {
		// init
		for(i=0;i<max_part*num_rain_drops;i++){
			make_rain_drop(i, x, y, z);
		}
	}
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
	// switch weather on
	weather_flags |= WEATHER_ACTIVE | WEATHER_STARTING; 
}

void stop_weather(int seconds_till_stop, float severity)
{
	if (! (weather_flags & WEATHER_ACTIVE)) {
		// We missed the start. So let's set up the data
		// severity of effect
		weather_severity = severity;
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

void clear_weather(){
	int i;
	weather_flags = WEATHER_NONE;
	num_thunders = 0;
	weather_ratios[0] = 1.0f;
	for(i = 1; i < WEATHER_TYPES; ++i){
		weather_ratios[i] = 0.0f;
	}
}

int weather_use_fog(){
	if (!use_fog) return 0;
	return weather_flags&WEATHER_ACTIVE;
}


void render_fog()
{
	float current_severity = weather_severity * get_fadeinout_bias();
	float density;
	int i;
	float particle_alpha, diffuse_bias, tmpf;
	char have_particles;

	if(weather_flags & WEATHER_ACTIVE){
		density = interpolate(current_severity, min_fog, precip_avg(fog_level));
		have_particles = 1;
	} else {
		have_particles = 0;
	}

	// estimate portion of scene colours that the fog covers
	tmpf = exp(-10.0f*density);
	fog_alpha = 1.0f - tmpf*tmpf;

	// estimate portion of scene colours that the particles cover
	particle_alpha = (have_particles)? 0.2f*rain_color[3]*current_severity : 0.0f;
	// in dungeons and at night we use smaller light sources ==> less diffuse light
	diffuse_bias = (dungeon || !is_day)? 0.2f : 0.5f;

	// compute fog color
	for (i = 0; i < 3; i++) {
		// blend ambient and diffuse light to build a base fog color
		float tmp = 0.5f*sun_ambient_light[i] + diffuse_bias*difuse_light[i];
		if (have_particles) {
			// blend base color with weather particle color
			rain_color[i] = interpolate(particle_alpha, tmp, rain_color[i]);
		} else {
			rain_color[i] = tmp;
		}
	}

	// set clear color to fog color
	glClearColor(rain_color[0], rain_color[1], rain_color[2], 0.0f);

	// set fog parameters
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
	glFogf(GL_FOG_DENSITY, density);
	glFogfv(GL_FOG_COLOR, rain_color);
}

void weather_color_bias(const float * src, float * dst) {
	int i;

	for (i = 0; i < 3; i++) {
		dst[i] = interpolate(fog_alpha, src[i], rain_color[i]);
	}

	dst[3] = src[3];
}

void render_weather()
{
	static Uint32 last_frame = 0;

	weather_time = SDL_GetTicks();

	update_wind();

	if (weather_flags & WEATHER_ACTIVE) {
		// 0 means initialization
		Uint32 ticks = last_frame? weather_time - last_frame : 0;
		float severity = weather_severity * severity_mod * get_fadeinout_bias();
		int num_rain_drops;

		// update and render view
		num_rain_drops = MAX_RAIN_DROPS * severity;
		update_rain(ticks, num_rain_drops);
		render_rain(num_rain_drops);

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
			lightning = (RAND_ONE < 0.5f);
		} else {
			lightning = 0;
		}
		last_call = weather_time;
	}

	if (lightning) {
		return 1.0f;
	} else {
		const float bias = get_weather_light_bias();
#ifdef NEW_LIGHTING
		const float severity = 0.9 * weather_severity; // Bias light by weather severity (the old way obliterates lighting, making everything but shadows black)
#else
		const float severity = 0.15f*weather_severity + 0.85f; // slightly bias light by weather severity
#endif
		float result = value*(1.0f - severity*bias);

		if (result < 0.0f) result = 0.0f;
		else if (result > 1.0f) result = 1.0f;
		
//		printf("%f, %f, %f, %f, %f -> %f\n", value, get_weather_light_bias(), bias, weather_severity, severity, result);
		return result;
	}
}

void weather_sound_control()
{
	static ALuint rain_sound = -1;
	if (!sound_on) {
		return;
	}

	if (rain_sound == -1) {
#ifdef NEW_SOUND
		rain_sound = add_sound_object(snd_rain,0,0);
		sound_source_set_gain(rain_sound,0.0f);
#else
		int buffer;
		alGenSources(1, &rain_sound);
		buffer = get_loaded_buffer(snd_rain);
		if (buffer) alSourcei(rain_sound, AL_BUFFER, buffer);
		alSourcei(rain_sound, AL_LOOPING, AL_TRUE);
		alSourcei(rain_sound, AL_SOURCE_RELATIVE, AL_TRUE);
		alSourcef(rain_sound, AL_GAIN, 0.0f);
#endif	//NEW_SOUND
	}
	
	if (weather_flags & WEATHER_ACTIVE)
	{
		// 0 means initialization
		float severity = weather_severity * severity_mod * get_fadeinout_bias();
		int source_state;
		int i;

		if(weather_ratios[WEATHER_RAIN] > 0.0f){
#ifdef NEW_SOUND
			sound_source_set_gain(rain_sound,severity*weather_ratios[WEATHER_RAIN]);
#else
			alSourcef(rain_sound, AL_GAIN, severity*weather_ratios[WEATHER_RAIN]);
			alGetSourcei(rain_sound, AL_SOURCE_STATE, &source_state);
			if (source_state != AL_PLAYING) alSourcePlay(rain_sound);
#endif	//NEW_SOUND
		} else {
#ifdef NEW_SOUND
				sound_source_set_gain(rain_sound,0.0f);
#else
				alSourcePause(rain_sound);
#endif	//NEW_SOUND
		}

		for (i = 0; i < num_thunders; i++)
		{
			if (!thunders[i].done)
			{
				if (cur_time >= thunders[i].time)
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
						add_sound_object(snd_thunder, 0, 0);
#else
						add_sound_object(snd_thunder, 0, 0, 0, 0);
#endif	//NEW_SOUND
					}
					thunders[i].done = 1; // also allows main thread to write
				}
			}
		}
	}
	else
	{
#ifdef NEW_SOUND
		sound_source_set_gain(rain_sound,0.0f);
#else
		alSourcePause(rain_sound);
#endif	//NEW_SOUND
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
	lightning_stop = weather_time + 100 + (Uint32)(300.0f * RAND_ONE);
}

#else // !def NEW_WEATHER

#ifdef DEBUG
GLfloat rain_color[4] = { 0.8f, 0.8f, 0.8f, 0.13f };
#else
static GLfloat rain_color[4] = { 0.8f, 0.8f, 0.8f, 0.13f }; // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
#endif



#define MAX_THUNDERS 5

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

int seconds_till_rain_starts=-1;
int seconds_till_rain_stops=-1;
int is_raining=0;
int rain_sound=0;
int weather_light_offset=0;
int rain_light_offset=0;
int thunder_light_offset;
thunder thunders[MAX_THUNDERS];

Uint32 rain_control_counter=0;
Uint32 thunder_control_counter=0;
int num_rain_drops=0, max_rain_drops=0;

int rain_table_valid = 0;

#ifdef DEBUG
int rain_calls = 0;
int last_rain_calls = 0;
#endif

float rain_strength_bias = 1.0f;
GLfloat fogColor[4] = { 0.0f, 0.0f, 0.0f , 0.13f }; // use same alpha like in rain_color
float fogAlpha = 0.0f;

void init_weather() {
	clear_thunders();
	build_rain_table();
}

void start_weather(int seconds_till_start, float severity)
{
	seconds_till_rain_starts= seconds_till_start;
	seconds_till_rain_stops= -1;
	rain_strength_bias= severity;
}

void stop_weather(int seconds_till_stop, float severity)
{
	seconds_till_rain_stops= seconds_till_stop;
	seconds_till_rain_starts= -1;
	rain_strength_bias= severity;	// failsafe incase we never saw the start (logged in and already raining?
}

void clear_weather()
{
	seconds_till_rain_starts= -1;
	seconds_till_rain_stops= 0;
	weather_light_offset= 0;
	rain_light_offset= 0;
}

void get_weather_from_server(const Uint8* data){
	if(data[0] == weather_effect_wind){
		wind_direction = wind_direction_srv = (2 * data[1])%360;
		wind_speed = wind_speed_srv = data[2];
		return;
	} else if(data[0] == weather_effect_leaves){
		//EC_TAG
		return;
	} else if(data[0] != weather_effect_rain){
		return;	//old weather can only handle rain
	}
	if(is_raining){
		stop_weather(data[1], ((float)(data[2]))/100.0f);
	} else {
		start_weather(data[1], ((float)(data[2]))/100.0f);
	}
}

void render_weather()
{
	if(is_raining && num_rain_drops > 0) render_rain(num_rain_drops);
}

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

void rain_control()
{
	float rain_light_bias = 0.8 + 0.2*rain_strength_bias;
	float rainParam;
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
	if (map_flags & MF_SNOW) {
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
		if (seconds_till_rain_stops > 60) {
			is_raining=1;
			rainParam = rain_strength_bias*(min2i(60,seconds_till_rain_stops) - 60)/30.0f;
			num_rain_drops = rainParam*MAX_RAIN_DROPS;
#ifdef NEW_SOUND
			if (!rain_sound || find_sound_source_from_cookie(rain_sound)<0)
				rain_sound=add_sound_object(snd_rain,0,0);
#else
			if(!rain_sound) rain_sound=add_sound_object(snd_rain,0,0,0,1);
#endif	//NEW_SOUND
			if (rain_sound) sound_source_set_gain(rain_sound, rainParam);
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
			if (rain_sound) sound_source_set_gain(rain_sound, 0.0f);
			seconds_till_rain_starts--;
		} else if(seconds_till_rain_starts) {
			if(!is_raining) {
				is_raining=1;
#ifdef NEW_SOUND
				if (!rain_sound || find_sound_source_from_cookie(rain_sound)<0)
					rain_sound=add_sound_object(snd_rain,0,0);
#else
				if (!rain_sound) rain_sound=add_sound_object(snd_rain,0,0,0,1);
#endif	//NEW_SOUND
			}
			rainParam = rain_strength_bias*(30-seconds_till_rain_starts)/30.0f;
			num_rain_drops = rainParam*MAX_RAIN_DROPS;
			if (rain_sound) sound_source_set_gain(rain_sound, rainParam);
			seconds_till_rain_starts--;
		} else {
			if(!is_raining) {
				is_raining=1;
#ifdef NEW_SOUND
				if (!rain_sound || find_sound_source_from_cookie(rain_sound)<0)
					rain_sound=add_sound_object(snd_rain,0,0);
#else
				if (!rain_sound) rain_sound=add_sound_object(snd_rain,0,0,0,1);
#endif	//NEW_SOUND
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
#ifdef NEW_SOUND
			if (rain_sound && find_sound_source_from_cookie(rain_sound)>=0)
#else
			if (rain_sound)
#endif	//NEW_SOUND
			{

				sound_source_set_gain(rain_sound, rain_strength_bias);
			} else {
#ifdef NEW_SOUND
				rain_sound=add_sound_object(snd_rain,0,0);
#else
				rain_sound=add_sound_object(snd_rain,0,0,0,1);
#endif	//NEW_SOUND
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
	int sounds[5]={snd_thndr_1,snd_thndr_2,snd_thndr_3,snd_thndr_4,snd_thndr_5};
	if(map_flags & MF_SNOW) return;

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
							if(thunders[i].thunder_type >=0 && thunders[i].thunder_type <5)
							{
#ifdef NEW_SOUND
								add_sound_object(sounds[thunders[i].thunder_type],0,0);
#else
								add_sound_object(sounds[thunders[i].thunder_type],0,0,0,0);
#endif	//NEW_SOUND
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

	if(map_flags & MF_SNOW) return;

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

	if(!(map_flags& MF_SNOW)) {
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

#endif	//NEW_WEATHER


void render_rain(int num_rain_drops)
{
	//TODO: it'd be really nifty if the points close to the camera were rendered as a particle or similar, while the rest stay points
	int idx, max;

	if(num_rain_drops <= 0) return;

	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef NEW_WEATHER
	set_rain_color();
	glPointSize(1.0);
#endif //NEW_WEATHER
	glColor4fv(rain_color);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
	glVertexPointer(3,GL_FLOAT,0,rain_drops);

	// ATI hack ... sometimes problems with large arrays
	idx = 0;
	max = num_rain_drops;
#ifdef NEW_WEATHER
	max *= precip_avg(precip_part_per);
#endif //NEW_WEATHER
	while(idx < max) {
	    int num;
		   
		num= max-idx;
		if(num > 3000){
			num= 3000;
		}
#ifdef NEW_WEATHER
	    glDrawArrays(GL_POINTS, idx, num);
#else //!NEW_WEATHER
	    glDrawArrays(GL_LINES, idx, num);
#endif //NEW_WEATHER
	    idx+= num;
	}

	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glPopAttrib();
}


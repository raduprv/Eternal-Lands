#ifndef __WEATHER_H__
#define __WEATHER_H__

#define MAX_THUNDERS 5

#define MAX_RAIN_DROPS 5000
#define RAIN_SPEED 2
#define rain_drop_len 5

extern int seconds_till_rain_starts;
extern int seconds_till_rain_stops;
extern int is_raining;
extern int rain_sound;
extern int weather_light_offset;
extern int rain_light_offset;
extern int thunder_light_offset;
extern int lightning_text;

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

thunder thunders[MAX_THUNDERS];

typedef struct
{
	short x;
	short y;
	short x2;
	short y2;
}rain_drop;

rain_drop rain_drops[MAX_RAIN_DROPS];

void build_rain_table();
void update_rain();
void render_rain();
void rain_control();
void thunder_control();
void add_thunder(int type,int sound_delay);
void get_weather_light_level();
void clear_thunders();
#endif

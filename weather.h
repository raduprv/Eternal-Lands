#ifndef __weather_H__
#define __weather_H__

#define MAX_THUNDERS 5

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


#endif

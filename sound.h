#ifndef __SOUND_H__
#define __SOUND_H__

#define max_sound_objects 200
#define max_sound_cache 100

#define SOUND_RANGE 40

typedef struct
{
	int positional;
	int x_pos;
	int y_pos;
	int channel;
	int loops_number;
	Mix_Chunk *chunk;
	int is_playing;
	int kill;

}sound_object;

typedef struct
{
	char file_name[60];
	Mix_Chunk *chunk;
}sound_file;

extern int have_sound;
extern int have_music;
extern int sound_on;
extern int music_on;
extern int no_sound;

extern sound_object sound_list[max_sound_objects];
extern sound_file sound_cache[max_sound_cache];

#endif


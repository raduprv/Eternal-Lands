#ifndef __SOUND_H__
#define __SOUND_H__

#define max_buffers 9
#define max_sources 16

#define BUFFER_SIZE (4096 * 16)
#define SLEEP_TIME 500

typedef struct {
	char file_name[64];
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int time;
} playlist_entry;

extern int have_sound;
extern int have_music;
extern int sound_on;
extern int music_on;
extern int no_sound;
extern int playing_music;

extern ALfloat sound_gain;
extern ALfloat music_gain;

void stop_sound(int i);
int add_sound_object(int sound_file,int x, int y,int positional,int loops);
void update_position();
void kill_local_sounds();
void turn_sound_off();
void turn_sound_on();
void init_sound();
void destroy_sound();
int realloc_sources();
ALuint get_loaded_buffer(int i);

void get_map_playlist();
void play_ogg_file(char *file_name);
void load_ogg_file(char *file_name);
void play_music(int list);
int update_music(void *dummy);
void stream_music(ALuint buffer);
void turn_music_off();
void turn_music_on();
void ogg_error(int code);

#define	lock_sound_list()	SDL_LockMutex(sound_list_mutex)
#define	unlock_sound_list()	SDL_UnlockMutex(sound_list_mutex);
#endif


#ifndef __SOUND_H__
#define __SOUND_H__

#define max_buffers 9
#define max_sources 16

#define snd_rain     0
#define snd_tele_in  1
#define snd_tele_out 2
#define snd_teleprtr 3
#define snd_thndr_1  4
#define snd_thndr_2  5
#define snd_thndr_3  6
#define snd_thndr_4  7
#define snd_thndr_5  8

extern int have_sound;
extern int have_music;
extern int sound_on;
extern int music_on;
extern int no_sound;
extern int used_sources;

extern char sound_files[max_buffers][30];
extern ALuint sound_source[max_sources];
extern ALuint sound_buffer[max_buffers];
SDL_mutex *sound_list_mutex;

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

#define	lock_sound_list()	SDL_LockMutex(sound_list_mutex)
#define	unlock_sound_list()	SDL_UnlockMutex(sound_list_mutex);
#endif


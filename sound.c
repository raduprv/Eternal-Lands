#include <ctype.h>
#include <string.h>
#include <SDL.h>
#include <SDL_thread.h>
#if defined NEW_SOUND && OGG_VORBIS
#include <math.h>
#endif // NEW_SOUND && OGG_VORBIS
#include "sound.h"
#include "asc.h"
#include "draw_scene.h"
#include "errors.h"
#include "init.h"
#include "lights.h"
#include "map.h"
#include "misc.h"
#ifdef NEW_SOUND
#include "spells.h"
#include "tiles.h"
#endif // NEW_SOUND
#include "translate.h"
#include "weather.h"
#include "io/map_io.h"
#ifdef	NEW_FILE_IO
#include "io/elpathwrapper.h"
#include "io/elfilewrapper.h"
#endif	//NEW_FILE_IO
#if defined NEW_SOUND && OGG_VORBIS
#include "actors.h"
#include "interface.h"
#endif // NEW_SOUND && OGG_VORBIS

#define MAX_FILENAME_LENGTH 80
#define MAX_BUFFERS 64
#define MAX_SOURCES 16

#ifdef NEW_SOUND
#if defined _EXTRA_SOUND_DEBUG && OSX
 #define printf LOG_ERROR
#endif

#define OGG_BUFFER_SIZE (1048576)
#define STREAM_BUFFER_SIZE (4096 * 16)
#else // NEW_SOUND
#define MUSIC_BUFFER_SIZE (4096 * 16)
#endif // NEW_SOUND
#define SLEEP_TIME 500

#ifdef _EXTRA_SOUND_DEBUG
/*
 #ifdef DEBUG
#define LOCK_SOUND_LIST() { static int i; static char str[50]; snprintf(str, sizeof(str), "LOCK_SOUND_LIST %d\n", i++); log_error_detailed(str, __FILE__, __FUNCTION__, __LINE__); SDL_LockMutex(sound_list_mutex); }
#define UNLOCK_SOUND_LIST() { static int i; static char str[50]; snprintf(str, sizeof(str), "LOCK_SOUND_LIST %d\n", i++); log_error_detailed(str, __FILE__, __FUNCTION__, __LINE__); SDL_UnlockMutex(sound_list_mutex); }
 #else // DEBUG
#define LOCK_SOUND_LIST() { static int i; printf("LOCK_SOUND_LIST %d - %s %s:%d sources: %d\n", i++, __FILE__, __FUNCTION__, __LINE__, used_sources); SDL_LockMutex(sound_list_mutex); }
#define UNLOCK_SOUND_LIST() { static int i; printf("UNLOCK_SOUND_LIST %d - %s %s:%d\n", i++, __FILE__, __FUNCTION__, __LINE__); SDL_UnlockMutex(sound_list_mutex); }
 #endif // DEBUG
*/
int locked = 0;
char last_file[50] = "";
char last_func[50] = "";
int last_line = 0;
#define	LOCK_SOUND_LIST() { int i = 0; while (locked) {	i++; if (i == 32000) printf("lock loop in %s, %s:%d - last lock: %s, %s:%d\n", __FILE__, __FUNCTION__, __LINE__, last_file, last_func, last_line); } safe_strncpy(last_file, __FILE__, strlen(last_file)); safe_strncpy(last_func, __FUNCTION__, strlen(last_func)); last_line = __LINE__; locked = 1; }
#define	UNLOCK_SOUND_LIST() { locked = 0; }
#else // _EXTRA_SOUND_DEBUG
#define	LOCK_SOUND_LIST() SDL_LockMutex(sound_list_mutex);
#define	UNLOCK_SOUND_LIST() SDL_UnlockMutex(sound_list_mutex);
#endif // _EXTRA_SOUND_DEBUG

typedef struct {
	char file_name[64];
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int time;
} playlist_entry;


#ifdef NEW_SOUND
#define ABS_MAX_SOURCES 64				// Define an absolute maximum for the sources (the size of the array)

#define MAX_SOUNDS 100

#ifdef OGG_VORBIS
#define MAX_STREAMS 7					// 1 music stream and up to 3 each for bg and crowds
#define STREAM_TYPE_NONE -1
#define STREAM_TYPE_SOUNDS 0
#define STREAM_TYPE_MUSIC 1
#define STREAM_TYPE_CROWD 2
#define NUM_STREAM_BUFFERS 4
#endif // OGG_VORBIS

#define MAX_BACKGROUND_DEFAULTS 8			// Maximum number of global default backgrounds
#define MAX_MAP_BACKGROUND_DEFAULTS 4		// Maximum number of default backgrounds per map
#define MAX_SOUND_MAP_NAME_LENGTH 60		// Maximum length of the name of the map
#define MAX_SOUND_MAP_BOUNDARIES 20			// Maximum number of boundary sets per map
#define MAX_ITEM_SOUND_IMAGE_IDS 30			// Maximum number of image id's linked to an item sound def
#define MAX_SOUND_TILE_TYPES 20				// Maximum number of different tile types
#define MAX_SOUND_TILES 30					// Maximum number of different tiles for a tile type
#define MAX_SOUND_TILES_SOUNDS 5			// Maximum number of different sound types for a tile type

#define MAX_SOUND_MAPS 150			// This value is the maximum number of maps sounds can be defined for
									// (Roja has suggested 150 is safe for now)
#define MAX_SOUND_EFFECTS 60		// This value should equal the max special_effect_enum
#define MAX_SOUND_PARTICLES 20		// This value should equal the number of particle effects
#define MAX_SOUND_ITEMS 5			// This is the number of sounds defined for "Use item" sfx

typedef enum
{
	STAGE_UNUSED = -1, STAGE_INTRO, STAGE_MAIN, STAGE_OUTRO, num_STAGES, STAGE_STREAM
} SOUND_STAGE;

typedef struct
{
	char file_path[MAX_FILENAME_LENGTH];	// Where to load the file from
	int loaded_status;						// Is the sample loaded yet
	int sample_num;							// Sample array ID (if loaded)
} sound_parts;

typedef struct
{
	char name[MAX_SOUND_NAME_LENGTH];
	sound_parts part[num_STAGES];	// The indices of the samples used
	float gain;						// The gain of this sound. The same sample may be defined under 2 sounds, with different gains. (default 1.0)
	int stereo;						// 1 is stereo, 0 is mono (default mono)
	float distance;					// Distance it can be heard, in meters
	int positional;					// 1=positional, 0=omni (default positional)
	int loops;						// 0=infinite, otherwise the number of loops (default 1)
	int fadeout_time;				// In milliseconds, only for omni sounds that loop. (default 0)
	int echo_delay;					// The value is the echo in MS. If 0, no echo (default 0)
	int echo_volume;				// In percent, 0 means no sound, 100 means as loud as the original sound (default 50)
	int time_of_the_day_flags;		// Bits 0-11 set each 1/2 hour of the 6-hour day (default 0xffff)
	unsigned int priority;			// If there are too many sounds to be played, highest value priority get culled (default 5)
	int type;						// The type of sound (environmental, actor, walking etc) for sound_opts (default Enviro)
} sound_type;

typedef struct
{
	ALuint buffer;							// If the sample is loaded, a buffer ID to play it.
	ALenum format;
	ALsizei size;							// Size of the sound data in bytes
#ifdef ALUT_WAV
	ALsizei freq;							// Frequency
#else // ALUT_WAV
	ALfloat freq;							// Frequency
#endif // ALUT_WAV
	ALint channels;							// Number of sound channels
	ALint bits;								// Bits per channel per sample
	int length;								// Duration in milliseconds
} sound_sample;

typedef struct
{
	int sound;
	int x;
	int y;
	int playing;
	float base_gain;
	float cur_gain;
	int loaded;
	unsigned int cookie;
} sound_loaded;

typedef struct
{
	ALuint source;						// A handle for the source
	int priority;						// Include this here for streams and to force a priority if needed
	int play_duration;
	int loaded_sound;					// Not used for streams
	SOUND_STAGE current_stage;			// Set as STAGE_STREAMS for streams
	unsigned int cookie;
} source_data;

typedef struct
{
	int sound;
	int time_of_day_flags;		// As for sound time_of_day_flags
	int map_type;				// At the moment, only 0 (outside) and 1 (dungeon). Checked against the dungeon flag.
} background_default;

typedef struct
{
	int bg_sound;
	int crowd_sound;
	int time_of_day_flags;		// As for sound time_of_day_flags
	int is_default;				// There can be up to 4 defaults for any one map, with unique times of day
								// Coords are ignored if is_default set.
	int x1;
	int y1;
	int x2;
	int y2;
	int x3;
	int y3;
	int x4;
	int y4;
} map_sound_boundary_def;

typedef struct
{
	int id;
	char name[MAX_SOUND_MAP_NAME_LENGTH];		// This isn't used, it is simply helpful when editing the config
	map_sound_boundary_def boundaries[MAX_SOUND_MAP_BOUNDARIES];
	int num_boundaries;
	int defaults[MAX_MAP_BACKGROUND_DEFAULTS];	// ID of the default boundaries
	int num_defaults;
} map_sound_data;

typedef struct
{
	int id;					// ID's in the config should match those of special_effect_enum
	int sound;
} effect_sound_data;

typedef struct
{
	char file[30];			// This is the filename (without extension) for a particle
	int sound;
} particle_sound_data;

typedef struct
{
	int image_id[MAX_ITEM_SOUND_IMAGE_IDS];
	int num_imageids;
	int sound;
} item_sound_data;

typedef struct
{
	char actor_types[256];
	int sound;
} tile_sounds;

typedef struct
{
	int tile_type[MAX_SOUND_TILES];
	int num_tile_types;
	tile_sounds sounds[MAX_SOUND_TILES_SOUNDS];
	int num_sounds;
	int default_sound;
} tile_sound_data;

#ifdef OGG_VORBIS
typedef struct
{
	int type;								// The type of this stream
	ALuint source;							// The source for this stream
	unsigned int cookie;					// A cookie for the source
	ALuint buffers[NUM_STREAM_BUFFERS];		// The stream buffers
	OggVorbis_File stream;					// The Ogg file handle for this stream
	vorbis_info * info;						// The Ogg info for this file handle
	int fade;								// The current fade value for this stream
	int fade_length;						// The length of the fade in or out (number of update_stream loops)
	int processed;							// Processed value (temporary storage)
	int playing;							// Is this stream currently playing
	int sound;								// The sound this stream is playing				(not used for music)
	int is_default;							// Is this sound a default sound for this type	(not used for music)
	map_sound_boundary_def * boundary;		// This is a pointer to the boundary in use
} stream_data;
#endif // OGG_VORBIS

#else // NEW_SOUND

#ifdef DEBUG
struct sound_object
{
	int file, x, y, positional, loops;
};
struct sound_object sound_objects[MAX_SOURCES];
#endif	// DEBUG

ALCdevice *mSoundDevice;
ALCcontext *mSoundContext;
#endif	// NEW_SOUND

int have_sound = 0;
int have_music = 0;
int sound_opts = 3;
int sound_on = 1;
int music_on = 1;
Uint8 inited = 0;
#ifdef NEW_SOUND
SDL_Thread *sound_streams_thread = NULL;
#else // NEW_SOUND
SDL_Thread *music_thread = NULL;
#endif // NEW_SOUND

ALfloat sound_gain = 1.0f;
ALfloat music_gain = 1.0f;
#ifdef NEW_SOUND
ALfloat crowd_gain = 1.0f;
ALfloat enviro_gain = 1.0f;
ALfloat actor_gain = 1.0f;
ALfloat walking_gain = 1.0f;
ALfloat gamewin_gain = 1.0f;
ALfloat client_gain = 1.0f;
#endif // NEW_SOUND

int used_sources = 0;						// the number of sources currently playing

#ifdef NEW_SOUND
char sound_devices[1000] = "";				// Set up a string to store the names of the available sound devices
int max_sources = MAX_SOURCES;				// Initialise our local maximum number of sources to the default
int max_streams = MAX_STREAMS;				// Initialise our local maximum number of streams to the default

int num_types = 0;							// Number of distinct sound types
int num_samples = 0;						// Number of actual sound files - a sound type can have > 1 sample
int num_sounds = 0;							// Number of sounds in the sounds_list
int sound_num_background_defaults = 0;		// Number of default background sounds
int sound_num_maps = 0;						// Number of maps we have sounds for
int sound_num_effects = 0;					// Number of effects we have sounds for
int sound_num_particles = 0;				// Number of particles we have sounds for
int sound_num_items = 0;					// Number of "Use item" actions we have sounds for
int sound_num_tile_types = 0;				// Number of tile type groups we have sounds for

int snd_cur_map = -1;
int cur_boundary = 0;
int dim_sounds_on_rain = 0;

// Each playing source is identified by a unique cookie.
unsigned int next_cookie = 1;
sound_loaded sounds_list[MAX_BUFFERS * 2];						// The loaded sounds
source_data sound_source_data[ABS_MAX_SOURCES];					// The active (playing) sources
sound_type sound_type_data[MAX_SOUNDS];							// Configuration of the sound types
sound_sample sound_sample_data[MAX_BUFFERS];					// Path & buffer data for each sample
background_default sound_background_defaults[MAX_BACKGROUND_DEFAULTS];	// Default background sounds
																		// (must have non-overlapping time of day flags)
int crowd_default;												// Default sound for crowd effects
int walking_default;											// Default sound for walking
map_sound_data sound_map_data[MAX_SOUND_MAPS];					// Data for map sfx
effect_sound_data sound_effect_data[MAX_SOUND_EFFECTS];			// Data for effect sfx
particle_sound_data sound_particle_data[MAX_SOUND_PARTICLES];	// Data for particle sfx
item_sound_data sound_item_data[MAX_SOUND_ITEMS];				// Data for item sfx
tile_sound_data sound_tile_data[MAX_SOUND_TILE_TYPES];			// Data for tile (walking) sfx
int server_sound[9];											// Map of server sounds to sound def ids
int sound_spell_data[10];										// Map of id's for spells-that-affect-you to sounds
#else
char sound_files[MAX_BUFFERS][MAX_FILENAME_LENGTH];
ALuint sound_source[MAX_SOURCES];
ALuint sound_buffer[MAX_BUFFERS];
#endif	//NEW_SOUND
SDL_mutex *sound_list_mutex;


#ifdef	OGG_VORBIS
#define MAX_PLAYLIST_ENTRIES 50

#ifdef NEW_SOUND
stream_data * music_stream = NULL;
stream_data streams[MAX_STREAMS];
#else // NEW_SOUND
FILE* ogg_file;
OggVorbis_File ogg_stream;
vorbis_info* ogg_info;
ALuint music_buffers[4];
ALuint music_source;

int playing_music = 0;
#endif // NEW_SOUND

playlist_entry playlist[MAX_PLAYLIST_ENTRIES];
int loop_list = 1;
int list_pos = -1;
#endif // OGG_VORBIS



/*    *** Declare some local functions ****
 * These functions shouldn't need to be accessed outside sound.c
 */
#ifdef OGG_VORBIS
void ogg_error(int code);
#endif // OGG_VORBIS

#ifndef NEW_SOUND

#ifdef OGG_VORBIS
void load_ogg_file(char *file_name);
void play_ogg_file(char *file_name);
void stream_music(ALuint buffer);
#endif // OGG_VORBIS
int realloc_sources();

#else // !NEW_SOUND

#ifdef OGG_VORBIS
int load_ogg_file(char *file_name, OggVorbis_File *oggFile);
int stream_ogg_file(char *file_name, stream_data * stream, int numBuffers);
int stream_ogg(ALuint buffer, OggVorbis_File * inStream, vorbis_info * info);
#ifdef ALUT_WAV
ALvoid * load_ogg_into_memory(char * szPath, ALenum *inFormat, ALsizei *inSize, ALsizei *inFreq);
#else // ALUT_WAV
ALvoid * load_ogg_into_memory(char * szPath, ALenum *inFormat, ALsizei *inSize, ALfloat *inFreq);
#endif // ALUT_WAV
static char * get_stream_type(int type);
void play_stream(int sound, stream_data * stream, ALfloat gain);
int start_stream(stream_data * stream);
void stop_stream(stream_data * stream);
void reset_stream(stream_data * stream);
int init_sound_stream(stream_data * stream, int type, int priority);
void destroy_stream(stream_data * stream);
int add_stream(int sound, int type, int boundary);
int check_for_valid_stream_sound(int tx, int ty, int type);
void check_for_new_streams(int tx, int ty);
int process_stream(stream_data * stream, ALfloat gain, int * sleep);
int check_stream(stream_data * stream, int day_time, int tx, int ty);
void play_song(int list_pos);
void find_next_song(int tx, int ty, int day_time);
#endif	// OGG_VORBIS

void clear_sound_data();
int ensure_sample_loaded(char * filename);
void set_sound_gain(source_data * pSource, int loaded_sound_num, float initial_gain);
int play_sound(int loaded_sound_num, int x, int y, float initial_gain);
source_data * get_available_source(int priority);
source_data *insert_sound_source_at_index(unsigned int index);
unsigned int get_next_cookie();
int get_loaded_sound_num();
void reset_buffer_details(int sample_num);
int store_sample_name(char *name);
int stop_sound_source_at_index(int index);
void unload_sound(int index);
int find_sound_from_cookie(unsigned int cookie);
int time_of_day_valid(int flags);
int sound_bounds_check(int x, int y, map_sound_boundary_def bounds);
#endif	// !NEW_SOUND


/*********************
 *  COMMON FUNCTIONS *
 *********************/

void toggle_music(int * var) {
	*var = !*var;
	if (!music_on) {
		turn_music_off();
	} else {
		turn_music_on();
	}
}

/*******************************
 * COMMON OGG VORBIS FUNCTIONS *
 *******************************/

#ifdef	OGG_VORBIS
void ogg_error(int code)
{
	switch(code)
	{     
		case OV_EREAD:
			LOG_ERROR(snd_media_read); break;
		case OV_ENOTVORBIS:
			LOG_ERROR(snd_media_notvorbis); break;
		case OV_EVERSION:
			LOG_ERROR(snd_media_ver_mismatch); break;
		case OV_EBADHEADER:
			LOG_ERROR(snd_media_invalid_header); break;
		case OV_EFAULT:
			LOG_ERROR(snd_media_internal_error); break;
		case OV_EOF:
			LOG_ERROR(snd_media_eof); break;
		case OV_HOLE:
			LOG_ERROR(snd_media_hole); break;
		case OV_EINVAL:
			LOG_ERROR(snd_media_einval); break;
		case OV_EBADLINK:
			LOG_ERROR(snd_media_ebadlink); break;
		case OV_FALSE:
			LOG_ERROR(snd_media_false); break;
		case OV_ENOSEEK:
			LOG_ERROR(snd_media_enoseek); break;
		default:
			LOG_ERROR(snd_media_ogg_error);
    }
}
#endif	// OGG_VORBIS


/**************************
 * COMMON MUSIC FUNCTIONS *
 **************************/

void get_map_playlist()
{
#ifdef	OGG_VORBIS
	int i=0, len;
	char map_list_file_name[256];
	char tmp_buf[1024];
	FILE *fp;
	char strLine[255];
	char *tmp;
	// NOTE: keep the max length in this format line in sync with
	// the size of tmp_buf
	static const char pl_line_fmt[] = "%d %d %d %d %d %1023s";

	if(!have_music)return;

	memset (playlist, 0, sizeof(playlist));

	tmp = strrchr (map_file_name, '/');
	if (tmp == NULL)
		tmp = map_file_name;
	else
		tmp++;
#ifndef NEW_FILE_IO
	safe_snprintf (map_list_file_name, sizeof (map_list_file_name), "./music/%s", tmp);
#else /* NEW_FILE_IO */
	safe_snprintf (map_list_file_name, sizeof (map_list_file_name), "music/%s", tmp);
#endif /* NEW_FILE_IO */
	len = strlen (map_list_file_name);
	tmp = strrchr (map_list_file_name, '.');
	if (tmp == NULL)
		tmp = &map_list_file_name[len];
	else
		tmp++;
	len -= strlen (tmp);
	safe_snprintf (tmp, sizeof (map_list_file_name) - len, "pll");

#ifndef NEW_FILE_IO
	// don't consider absence of playlist an error, so don't use my_fopen
	fp=fopen(map_list_file_name,"r");
#else /* NEW_FILE_IO */
	fp=open_file_data(map_list_file_name,"r");
#endif /* NEW_FILE_IO */
	if (fp == NULL) return;

	while(1)
	{
		if (fscanf (fp, pl_line_fmt, &playlist[i].min_x, &playlist[i].min_y, &playlist[i].max_x, &playlist[i].max_y, &playlist[i].time, tmp_buf) == 6)
		{
			// check for a comment
			tmp = strstr (tmp_buf, "--");
			if (tmp)
			{
				*tmp = '\0';
				len = strlen (tmp_buf);
				while (len > 0 && isspace (tmp_buf[len-1]))
				{
					len--;
					tmp_buf[len]= '\0';
				}
			}
			safe_strncpy (playlist[i].file_name, tmp_buf, sizeof(playlist[i].file_name));
			playlist[i].file_name[63]= '\0';
			if (++i >= MAX_PLAYLIST_ENTRIES)
				break;
		}
		if (!fgets (strLine, 100, fp)) break;
	}
	fclose(fp);
	loop_list=1;
	list_pos=-1;
#endif	// OGG_VORBIS
}

void play_music(int list)
{
#ifdef	OGG_VORBIS
	int i=0;
	char list_file_name[256];
	FILE *fp;
	char strLine[255];

	if(!have_music)return;

#ifndef NEW_FILE_IO
	safe_snprintf(list_file_name, sizeof(list_file_name), "./music/%d.pll", list);
	// don't consider absence of playlist an error, so don't use my_fopen
	fp=fopen(list_file_name,"r");
#else /* NEW_FILE_IO */
	safe_snprintf(list_file_name, sizeof(list_file_name), "music/%d.pll", list);
	fp=open_file_data(list_file_name, "r");
#endif /* NEW_FILE_IO */
	if(!fp)return;

	memset(playlist,0,sizeof(playlist));

	while (1)
	{
		if (fscanf (fp, "%254s", strLine) == 1)
		{
			my_strncp (playlist[i].file_name, strLine, sizeof (playlist[i].file_name));
			playlist[i].min_x = 0;
			playlist[i].min_y = 0;
			playlist[i].max_x = 10000;
			playlist[i].max_y = 10000;
			playlist[i].time = 2;
			if (++i > MAX_PLAYLIST_ENTRIES)
				break;
			if (!fgets (strLine, 100, fp))
				break;
		}
	}
	fclose(fp);
	loop_list=0;
	list_pos=0;
#ifdef NEW_SOUND
	play_song(list_pos);
#else // NEW_SOUND
	alSourcef (music_source, AL_GAIN, music_gain);
	play_ogg_file(playlist[list_pos].file_name);
#endif // NEW_SOUND
#endif	// OGG_VORBIS
}











#ifndef NEW_SOUND

/***********************
 * OLD SOUND FUNCTIONS *
 ***********************/

void turn_sound_on()
{
	int i,state=0;
	ALuint source;
	if(!inited)
	{
		init_sound();
	}
	if(!have_sound)
		return;
	LOCK_SOUND_LIST();
	sound_on = 1;
	for(i=0;i<used_sources;i++)
	{
		source = sound_source[i];
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if(state == AL_PAUSED)
			alSourcePlay(source);
	}
	UNLOCK_SOUND_LIST();
}

void turn_sound_off()
{
	int i=0,loop;
	ALuint source;
	if(!inited)
		return;
#ifdef OGG_VORBIS
	if(!music_on)
#endif // OGG_VORBIS
	{
		destroy_sound();
		return;
	}
	LOCK_SOUND_LIST();
	sound_on=0;
	while(i<used_sources)
	{
		source = sound_source[i];
		alGetSourcei(source, AL_LOOPING, &loop);
		if(loop == AL_TRUE)
			alSourcePause(source);
		else
		{
			alSourceStop(source);
		}
		++i;
	}
	UNLOCK_SOUND_LIST();
}

void toggle_sounds(int *var){
	*var=!*var;
	if(!sound_on){
		turn_sound_off();
	} else {
		turn_sound_on();
	}
}

void turn_music_on()
{
#ifdef	OGG_VORBIS
	int state;
	if(!inited){
		init_sound();
	}
	if(!have_music){
		return;
	}
	get_map_playlist();
	music_on = 1;
	if(music_thread == NULL){
		music_thread=SDL_CreateThread(update_music, 0);
	}
	alGetSourcei(music_source, AL_SOURCE_STATE, &state);
	if(state == AL_PAUSED) {
		alSourcePlay(music_source);
		playing_music = 1;
	}
#endif	// OGG_VORBIS
}

void turn_music_off()
{
	if(!sound_on && inited){
		destroy_sound();
		return;
	}
#ifdef	OGG_VORBIS
	if(!have_music)
		return;
	if(music_thread != NULL){
		int queued = 0;
		music_on=0;
		alSourceStop(music_source);
		alGetSourcei(music_source, AL_BUFFERS_QUEUED, &queued);
		while(queued-- > 0){
			ALuint buffer;		
			alSourceUnqueueBuffers(music_source, 1, &buffer);
		}
		SDL_WaitThread(music_thread,NULL);
		music_thread = NULL;
	}
	music_on = playing_music = 0;
#endif	// OGG_VORBIS
}

void destroy_sound()
{
	int i, error;
	ALCcontext *context;
	ALCdevice *device;
	if(!inited){
		return;
	}
	SDL_DestroyMutex(sound_list_mutex);
	sound_list_mutex=NULL;
	inited = have_sound = sound_on = 0;

#ifdef	OGG_VORBIS
	if(music_thread != NULL){
		int queued = 0;
		ALuint buffer;
		if (have_music)
		{
			music_on = playing_music = have_music = 0;
			alSourceStop(music_source);
			alGetSourcei(music_source, AL_BUFFERS_QUEUED, &queued);
			while(queued-- > 0){
				alSourceUnqueueBuffers(music_source, 1, &buffer);
			}
			alDeleteSources(1, &music_source);
			alDeleteBuffers(4, music_buffers);
			ov_clear(&ogg_stream);
		}
		SDL_WaitThread(music_thread,NULL);
		music_thread = NULL;
	}
#endif	// OGG_VORBIS
	alSourceStopv(used_sources, sound_source);
	alDeleteSources(used_sources, sound_source);
	for(i=0;i<MAX_BUFFERS;i++) {
		if(alIsBuffer(sound_buffer[i])) {
			alDeleteBuffers(1, sound_buffer+i);
		}
	}
	alcDestroyContext( mSoundContext );
	if(mSoundDevice) {
		alcCloseDevice( mSoundDevice );
	}
	/*
	 * alutExit() contains a problem with hanging on exit on some
	 * Linux systems.  The problem is with the call to
	 * alcMakeContextCurrent( NULL );  The folowing code is exactly
	 * what is in alutExit() minus that function call.  It causes
	 * no problems if the call is not there since the context is
	 * being destroyed right afterwards.
	 */
	context = alcGetCurrentContext();
	if(context != NULL) {
		device = alcGetContextsDevice(context);
		alcDestroyContext(context);
		if(device != NULL)
		{
			alcCloseDevice(device);
		}
	}

	if((error=alGetError()) != AL_NO_ERROR) 
	{
		char str[256];
		safe_snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alGetString(error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
	}

	used_sources = 0;
}


/************************
 * OGG VORBIS FUNCTIONS *
 ************************/

#ifdef OGG_VORBIS
void load_ogg_file(char *file_name)
{
	char file_name2[80];

	if(!have_music)return;

	ov_clear(&ogg_stream);

	if(file_name[0]!='.' && file_name[0]!='/')
		safe_snprintf (file_name2, sizeof (file_name2), "./music/%s", file_name);
	else
		safe_snprintf(file_name2, sizeof (file_name2), "%s", file_name);

	ogg_file = my_fopen(file_name2, "rb");

	if(ogg_file == NULL) {
		have_music=0;
		return;
	}

	if(ov_open(ogg_file, &ogg_stream, NULL, 0) < 0) {
		LOG_ERROR(snd_ogg_stream_error);
		have_music=0;
		return;
	}

	ogg_info = ov_info(&ogg_stream, -1);
}

void play_ogg_file(char *file_name) {
	int error,queued;

	if(!have_music)return;

	alSourceStop(music_source);
	alGetSourcei(music_source, AL_BUFFERS_QUEUED, &queued);
	while(queued-- > 0)
		{
			ALuint buffer;
			
			alSourceUnqueueBuffers(music_source, 1, &buffer);
		}

	load_ogg_file(file_name);
	if(!have_music)return;

	stream_music(music_buffers[0]);
	stream_music(music_buffers[1]);
	stream_music(music_buffers[2]);
	stream_music(music_buffers[3]);
    
	alSourceQueueBuffers(music_source, 4, music_buffers);
	alSourcePlay(music_source);

	if((error=alGetError()) != AL_NO_ERROR) 
    	{
    		LOG_ERROR("play_ogg_file %s: %s", my_tolower(reg_error_str), alGetString(error));
			have_music=0;
			return;
    	}
	playing_music = 1;	
}
#endif // OGG_VORBIS

/*******************
 * MUSIC FUNCTIONS *
 *******************/

void stream_music(ALuint buffer)
{
#ifdef OGG_VORBIS
    char data[MUSIC_BUFFER_SIZE];
    int  size = 0;
    int  section = 0;
    int  result = 0;
	int error = 0;
	char str[256];

    while(size < MUSIC_BUFFER_SIZE)
    {
#ifndef EL_BIG_ENDIAN
        result = ov_read(&ogg_stream, data + size, MUSIC_BUFFER_SIZE - size, 0, 2, 1,
						 &section);
#else
        result = ov_read(&ogg_stream, data + size, MUSIC_BUFFER_SIZE - size, 1, 2, 1,
						 &section);
#endif
		safe_snprintf(str, sizeof(str), "%d", result); //prevents optimization errors under Windows, but how/why?
        if((result > 0) || (result == OV_HOLE))		// OV_HOLE is informational
		{
            if (result != OV_HOLE) size += result;
		}
        else if(result < 0)
			ogg_error(result);
		else
			break;
    }
	if(!size)
		{
			playing_music = 0;//file's done, quit trying to play
			return;
		}
	if((error=alGetError()) != AL_NO_ERROR){
		LOG_ERROR("stream_music %s: %s", my_tolower(reg_error_str), alGetString(error));
		have_music=0;
	}

	alBufferData(buffer, AL_FORMAT_STEREO16, data, size, ogg_info->rate);

	if((error=alGetError()) != AL_NO_ERROR) 
    	{
    		LOG_ERROR("stream_music %s: %s", my_tolower(reg_error_str), alGetString(error));
			have_music=0;
    	}
#endif // OGG_VORBIS
}

int display_song_name()
{
#ifndef OGG_VORBIS
#ifdef ELC
	LOG_TO_CONSOLE(c_red2, snd_no_music);
#endif
#else // !OGG_VORBIS
	if(!playing_music){
		LOG_TO_CONSOLE(c_grey1, snd_media_music_stopped);
	}else{
		char musname[100];
		char *title = NULL, *artist = NULL;
		int i=0;
		vorbis_comment *comments;
		comments = ov_comment(&ogg_stream, -1);
		if(comments == NULL){
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, playlist[list_pos].file_name, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
			LOG_TO_CONSOLE(c_grey1, musname);
			return 1;
		}
		for(;i<comments->comments;++i){
			if((artist == NULL)&&(comments->comment_lengths[i] > 6)&&(my_strncompare(comments->user_comments[i],"artist", 6))){
				artist = comments->user_comments[i] + 7;
				if(title){break;}
			}else if((title == NULL)&&(comments->comment_lengths[i] > 6)&&(my_strncompare(comments->user_comments[i],"title", 5))){
				title = comments->user_comments[i] + 6;
				if(artist){break;}
			}
		}
		if(artist && title){
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info, title, artist, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
		}else if(title){
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, title, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
		}else{
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, playlist[list_pos].file_name, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
		}
		LOG_TO_CONSOLE(c_grey1, musname);
	}
#endif // !OGG_VORBIS
	return 1;
}

int update_music(void *dummy)
{
#ifdef	OGG_VORBIS
    int error,processed,state,state2,sleep,fade=0;
   	sleep = SLEEP_TIME;
	while(have_music && music_on)
		{
			SDL_Delay(sleep);
			if(playing_music)
				{
					int day_time = (game_minute>=30 && game_minute<60*3+30);
					int tx=-camera_x*2,ty=-camera_y*2;
					if(fade) {
						fade++;
						if(fade > 6) {
							int queued;
							fade = 0;
							playing_music = 0;
							alSourceStop(music_source);
							alGetSourcei(music_source, AL_BUFFERS_PROCESSED, &processed);
							alGetSourcei(music_source, AL_BUFFERS_QUEUED, &queued);
							while(queued-- > 0) {
								ALuint buffer;
								alSourceUnqueueBuffers(music_source, 1, &buffer);
							}
						}
						alSourcef(music_source, AL_GAIN, music_gain-((float)fade*(music_gain/6)));
						continue;
					}
					if(tx < playlist[list_pos].min_x ||
					   tx > playlist[list_pos].max_x ||
					   ty < playlist[list_pos].min_y ||
					   ty > playlist[list_pos].max_y ||
					   (playlist[list_pos].time != 2 &&
                        playlist[list_pos].time != day_time)) {
						fade = 1;
						continue;
					}
					alSourcef (music_source, AL_GAIN, music_gain);
					alGetSourcei(music_source, AL_BUFFERS_PROCESSED, &processed);
					alGetSourcei(music_source, AL_SOURCE_STATE, &state);
					state2=state;	//fake out the Dev-C++ optimizer!
					if(!processed) {
						if(sleep < SLEEP_TIME)sleep+=(SLEEP_TIME / 100);
						continue; //skip error checking et al
					}
					while(processed-- > 0)
						{
    						ALuint buffer;

							alSourceUnqueueBuffers(music_source, 1, &buffer);
							stream_music(buffer);
							alSourceQueueBuffers(music_source, 1, &buffer);
						}
					if(state2 != AL_PLAYING)
						{
							LOG_TO_CONSOLE(c_red1, snd_skip_speedup);
							//on slower systems, music can skip up to 10 times
							//if it skips more, it just can't play the music...
							if(sleep > (SLEEP_TIME / 10))
								sleep -= (SLEEP_TIME / 10);
							else if(sleep > 1) sleep = 1;
							else
								{
									LOG_TO_CONSOLE(c_red1, snd_too_slow);
									LOG_ERROR(snd_too_slow);
									turn_music_off();
									sleep = SLEEP_TIME;
									break;
								}
							alSourcePlay(music_source);
						}
					if((error=alGetError()) != AL_NO_ERROR)
						{
							LOG_ERROR("update_music %s: %s", my_tolower(reg_error_str), alGetString(error));
							have_music=0;
						}
				}
			else if(music_on)
				{
					int day_time = (game_minute>=30 && game_minute<60*3+30);
					int tx=-camera_x*2,ty=-camera_y*2;
					if(playlist[list_pos+1].file_name[0]) {
						list_pos++;
						if(tx > playlist[list_pos].min_x &&
						   tx < playlist[list_pos].max_x &&
						   ty > playlist[list_pos].min_y &&
						   ty < playlist[list_pos].max_y &&
                           (playlist[list_pos].time == 2 ||
                            playlist[list_pos].time == day_time)) {
							alSourcef (music_source, AL_GAIN, music_gain);
							play_ogg_file(playlist[list_pos].file_name);
						}
					} else if(loop_list)
						list_pos=-1;
					else
						get_map_playlist();
					continue;
				}
			if(exit_now) break;
		}
	if((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Music bug (update_music)\n");
#endif //_EXTRA_SOUND_DEBUG
	}
#endif	// OGG_VORBIS
	return 1;
}

void stop_sound(int i)
{
	if(!have_sound)return;
	if (i == 0) return;
	alSourceStop (i);
}

ALuint get_loaded_buffer(int i)
{
	int error;
#ifdef  ALUT_WAV
	ALsizei size,freq;
#ifndef OSX
	ALboolean loop;
#endif // OSX
#else  //ALUT_WAV
    ALsizei	size;
	ALfloat freq;
#endif  //ALUT_WAV
	ALenum  format;
	ALvoid*	data= NULL;
#ifndef	NEW_FILE_IO
	FILE *fin;
#else	//NEW_FILE_IO
	el_file_ptr file = NULL;
#endif	//NEW_FILE_IO
	
	if(!alIsBuffer(sound_buffer[i]))
	{
		// XXX FIXME (Grum): You have got to be kidding me...
		// alutLoadWAVFile doesn't provide any way to check if loading
		// a file succeeded. Well, at least, let's check if the file
		// actually exists...
		// Maybe use alutLoadWAV? But that doesn't seem to exist on 
		// OS/X...
#ifndef	NEW_FILE_IO
		fin = fopen (sound_files[i], "r");
		if (fin == NULL) 
		{
			LOG_ERROR(snd_wav_load_error, sound_files[i]);
			return 0;
		}
		// okay, the file exists and is readable, close it
		fclose (fin);
#else	//NEW_FILE_IO
		if (!el_file_exists(sound_files[i]))
		{
			LOG_ERROR(snd_wav_load_error, sound_files[i]);
			return 0;
		}
#endif	//NEW_FILE_IO

		alGenBuffers(1, sound_buffer+i);
			
		if((error=alGetError()) != AL_NO_ERROR) 
		{
			LOG_ERROR("%s: %s",snd_buff_error, alGetString(error));
			have_sound=0;
			have_music=0;
		}

#ifdef	NEW_FILE_IO
		file = el_open(sound_files[i]);
#endif	//NEW_FILE_IO
#ifdef ALUT_WAV
#ifdef OSX
		// OS X alutLoadWAVFile doesn't have a loop option... Oh well :-)
		// OpenAL 1.0 on Macs do not properly support alutLoadWAVMemory
		alutLoadWAVFile (sound_files[i], &format, &data, &size, &freq);
#else  //OSX
 #ifdef	NEW_FILE_IO
		alutLoadWAVMemory(el_get_pointer(file), &format, &data, &size, &freq, &loop);
 #else	//NEW_FILE_IO
		alutLoadWAVFile (sound_files[i], &format, &data, &size, &freq, &loop);
 #endif	//NEW_FILE_IO
#endif  //OSX
		alBufferData(sound_buffer[i],format,data,size,freq);
		alutUnloadWAV(format,data,size,freq);
#else  // ALUT_WAV
#ifdef	NEW_FILE_IO
		data = alutLoadMemoryFromFileImage(el_get_pointer(file), el_get_size(file), &format, &size, &freq);
#else	//NEW_FILE_IO
        	data= alutLoadMemoryFromFile (sound_files[i], &format, &size, &freq);
#endif	//NEW_FILE_IO
		if (data == AL_NONE)
		{
			LOG_ERROR ("Unable to load sound file %s\n", sound_files[i]);
		}
		else
		{
			alBufferData(sound_buffer[i],format,data,size,(int)freq);
			free(data);
		}
#endif  //ALUT_WAV
#ifdef	NEW_FILE_IO
		el_close(file);
#endif	//NEW_FILE_IO

		if ((error = alGetError ()) != AL_NO_ERROR)
			LOG_ERROR("(in get_loaded_buffer) %s: %s", snd_buff_error, alGetString(error));
	}
	return sound_buffer[i];
}

int add_sound_object(int sound_file,int x, int y,int positional,int loops)
{
	// XXX FIXME (Grum): do we really need to use tile coordinates?
	int error,tx,ty,distance,i;
	ALfloat sourcePos[]={ x, y, 0.0};
	ALfloat sourceVel[]={ 0.0, 0.0, 0.0};
	ALuint buffer;
	
	if(!have_sound)
		return 0;

	if(sound_file >= MAX_BUFFERS)
	{
		LOG_ERROR(snd_invalid_number);
		return 0;
	}

	LOCK_SOUND_LIST();

	i = used_sources;
	if (i>=MAX_SOURCES)
	{
		i = realloc_sources();
		if (i < 0)
		{
			// too much noise already (buffer full)
			UNLOCK_SOUND_LIST ();
			return 0;
		}
	}

	tx=-camera_x*2;
	ty=-camera_y*2;
	distance=(tx-x)*(tx-x)+(ty-y)*(ty-y);

	alGenSources(1, &sound_source[i]);
	if((error=alGetError()) != AL_NO_ERROR) 
	{
		LOG_ERROR("%s %d: %s", snd_source_error, i, alGetString(error));
		have_sound=0;
		have_music=0;
		UNLOCK_SOUND_LIST ();
		return 0;
	}

	buffer = get_loaded_buffer (sound_file);
	if (buffer == 0)
	{
		// can't read file
		UNLOCK_SOUND_LIST ();
		return 0;
	}

	alSourcef(sound_source[i], AL_PITCH, 1.0f);
	alSourcef(sound_source[i], AL_GAIN, sound_gain);
	alSourcei(sound_source[i], AL_BUFFER, buffer);
	alSourcefv(sound_source[i], AL_VELOCITY, sourceVel);
	alSourcefv(sound_source[i], AL_POSITION, sourcePos);
	if (!positional)
	{
		alSourcei(sound_source[i], AL_SOURCE_RELATIVE, AL_TRUE);
	}
	else 
	{
		alSourcei(sound_source[i], AL_SOURCE_RELATIVE, AL_FALSE);
		alSourcef(sound_source[i], AL_REFERENCE_DISTANCE , 10.0f);
		alSourcef(sound_source[i], AL_ROLLOFF_FACTOR , 4.0f);
	}

	if (loops)
	{
		alSourcei(sound_source[i], AL_LOOPING, AL_TRUE);
		alSourcePlay(sound_source[i]);
		if(!sound_on || (positional && (distance > 35*35)))
			alSourcePause(sound_source[i]);
	}
	else
	{
		alSourcei(sound_source[i], AL_LOOPING, AL_FALSE);
		if(sound_on)
			alSourcePlay(sound_source[i]);
	}

#ifdef DEBUG
	sound_objects[i].file = sound_file;
	sound_objects[i].x = x;
	sound_objects[i].y = y;
	sound_objects[i].positional = positional;
	sound_objects[i].loops = loops;
#endif
	
	used_sources++;

	UNLOCK_SOUND_LIST();
	return sound_source[i];
}

void remove_sound_object (int sound)
{
	int i;
	
	for (i = 0; i < used_sources; i++)
	{
		if (sound_source[i] == sound)
		{
			int j;
			alSourceStop (sound_source[i]);
			alDeleteSources(1, &sound_source[i]);
			for (j = i+1; j < used_sources; j++)
			{
				sound_source[j-1] = sound_source[j];
#ifdef DEBUG
				sound_objects[j-1] = sound_objects[j];
#endif
			}
			used_sources--;
#ifdef DEBUG
			sound_objects[used_sources].file = -1;
#endif
			break;
		}
	}
}

void sound_source_set_gain(int sound, float gain) {
	int i;
	
	for (i = 0; i < used_sources; i++)
	{
		if (sound_source[i] == sound)
		{
			alSourcef(sound_source[i],AL_GAIN, sound_gain * gain);
			break;
		}
	}
}

void update_position()
{
	int i,state,relative,error;
	int x,y,distance;
	int tx=-camera_x*2;
	int ty=-camera_y*2;
	ALfloat sourcePos[3];
	ALfloat listenerPos[]={tx,ty,0.0};

	if(!have_sound)return;
	LOCK_SOUND_LIST();

	alListenerfv(AL_POSITION,listenerPos);
	if((error=alGetError()) != AL_NO_ERROR){
		LOG_ERROR("update_position %s: %s", my_tolower(reg_error_str), alGetString(error));
		have_sound=0;
		have_music=0;
	}

	/* OpenAL doesn't have a method for culling by distance yet.
	   If it does get added, all this code can go */
	for(i=0;i<used_sources;i++)
	{
		alGetSourcei(sound_source[i], AL_SOURCE_RELATIVE, &relative);
		if(relative == AL_TRUE)continue;
		alGetSourcei(sound_source[i], AL_SOURCE_STATE, &state);
		alGetSourcefv(sound_source[i], AL_POSITION, sourcePos);
		x=sourcePos[0];y=sourcePos[1];
		distance=(tx-x)*(tx-x)+(ty-y)*(ty-y);
		if((state == AL_PLAYING) && (distance > 35*35))
			alSourcePause(sound_source[i]);
		else if (sound_on && (state == AL_PAUSED) && (distance < 30*30))
			alSourcePlay(sound_source[i]);
		if((error=alGetError()) != AL_NO_ERROR){
			LOG_ERROR("update_position %s: %s (source %d)", my_tolower(reg_error_str), alGetString(error), i);
			have_sound=0;
			have_music=0;
		}
	}
	UNLOCK_SOUND_LIST();
}

//kill all the sounds that loop infinitely
//usefull when we change maps, etc.
void kill_local_sounds()
{
	int error;
#ifdef OGG_VORBIS
	int queued, processed;
#ifdef	OSX //to fix music quiting when switching maps since used_sources is not updated properly, might be generally applicable
	if(have_music)
	{
		playing_music = 0;
		alSourceStop(music_source);
		alGetSourcei(music_source, AL_BUFFERS_PROCESSED, &processed);
		alGetSourcei(music_source, AL_BUFFERS_QUEUED, &queued);
		while(queued-- > 0) {
			ALuint buffer;
			alSourceUnqueueBuffers(music_source, 1, &buffer);
		}
	}
#endif // OSX
#endif // OGG_VORBIS
	if(!have_sound || !used_sources)return;
	LOCK_SOUND_LIST();
	alSourceStopv(used_sources,sound_source);
	if((error=alGetError()) != AL_NO_ERROR) 
	{
		LOG_ERROR("kill_local_sounds %s: %s", my_tolower(reg_error_str), alGetString(error));
		have_sound=0;
		have_music=0;
	}
	if (realloc_sources () != 0)
		LOG_ERROR(snd_stop_fail);
	UNLOCK_SOUND_LIST();
#ifdef	OGG_VORBIS
#ifndef OSX
	if(!have_music)
		return;
	playing_music = 0;
	alSourceStop(music_source);
	alGetSourcei(music_source, AL_BUFFERS_PROCESSED, &processed);
	alGetSourcei(music_source, AL_BUFFERS_QUEUED, &queued);
	while(queued-- > 0) {
		ALuint buffer;
		alSourceUnqueueBuffers(music_source, 1, &buffer);
	}
#endif // !OSX
#endif // OGG_VORBIS
}

int realloc_sources()
{
	int i;
	int state,error;
	int still_used=0;
	ALuint new_sources[MAX_SOURCES];
#ifdef DEBUG
	struct sound_object new_sound_objects[MAX_SOURCES];
#endif
	
	if(used_sources>MAX_SOURCES)
		used_sources=MAX_SOURCES;
	for(i=0;i<used_sources;i++)
	{
		alGetSourcei(sound_source[i], AL_SOURCE_STATE, &state);
		if((state == AL_PLAYING) || (state == AL_PAUSED))
		{
			new_sources[still_used] = sound_source[i];
#ifdef DEBUG
			new_sound_objects[still_used] = sound_objects[i];
#endif
			still_used++;
		}
		else
		{
			alDeleteSources(1, &sound_source[i]);
		}
	}

	for(i=0;i<still_used;i++)
	{
		sound_source[i] = new_sources[i];
#ifdef DEBUG
		sound_objects[i] = new_sound_objects[i];
#endif
	}
	for(i=still_used;i<MAX_SOURCES;i++)
	{
		sound_source[i]=-1;
#ifdef DEBUG
		sound_objects[i].file = -1;
#endif
	}
	used_sources=still_used;
	
	if((error=alGetError()) != AL_NO_ERROR) 
	{
		LOG_ERROR("snd_realloc %s: %s", my_tolower(reg_error_str), alGetString(error));
		have_sound=0;
		have_music=0;
	}
	if(used_sources>=MAX_SOURCES) 
	{
		LOG_TO_CONSOLE(c_red1,snd_sound_overflow);
		LOG_ERROR(snd_sound_overflow);
		return -1;
	}
	else
		return used_sources;
}

void init_sound()
{
	int i,error;
	ALfloat listenerPos[]={-camera_x*2,-camera_y*2,0.0};
	ALfloat listenerVel[]={0.0,0.0,0.0};
	ALfloat listenerOri[]={0.0,0.0,0.0,0.0,0.0,0.0};
	if(inited){
		return;
	}

#ifndef OSX
	alutInitWithoutContext(0, 0);
#endif // OSX

	have_sound=1;
#ifdef	OGG_VORBIS
	have_music=1;
#else // OGG_VORBIS
	have_music=0;
#endif	// OGG_VORBIS

	//NULL makes it use the default device.
	//to get a list of available devices, uncomment the following code, and read your error_log.txt
	//for windows users, it'll most likely only list DirectSound3D
	/*if ( alcIsExtensionPresent( NULL, "ALC_ENUMERATION_EXT" ) == AL_TRUE ){
		LOG_ERROR("Available sound devices: %s", alcGetString( NULL, ALC_DEVICE_SPECIFIER));
	}else{
		LOG_ERROR("ALC_ENUMERATION_EXT not found");
	}*/

	//if you want to use a different device, use, for example:
	//mSoundDevice = alcOpenDevice((ALubyte*) "DirectSound3D")
	mSoundDevice = alcOpenDevice( NULL );
	if((error=alcGetError(mSoundDevice)) != AL_NO_ERROR || !mSoundDevice){
		char str[256];
		safe_snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alcGetString(mSoundDevice,error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound=have_music=0;
		return;
	}

	mSoundContext = alcCreateContext( mSoundDevice, NULL );
	alcMakeContextCurrent( mSoundContext );

	sound_list_mutex=SDL_CreateMutex();

	if((error=alcGetError(mSoundDevice)) != AL_NO_ERROR || !mSoundContext || !sound_list_mutex){
		char str[256];
		safe_snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alcGetString(mSoundDevice,error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound=have_music=0;
		return;
	}

	// TODO: get this information from a file, sound.ini?	
	safe_snprintf (sound_files[snd_rain], sizeof (sound_files[snd_rain]), "%s/%s", datadir, "sound/rain1.wav");
	safe_snprintf (sound_files[snd_tele_in], sizeof (sound_files[snd_tele_in]), "%s/%s", datadir, "sound/teleport_in.wav");
	safe_snprintf (sound_files[snd_tele_out], sizeof (sound_files[snd_tele_out]), "%s/%s", datadir, "sound/teleport_out.wav");
	safe_snprintf (sound_files[snd_teleprtr], sizeof (sound_files[snd_teleprtr]), "%s/%s", datadir, "sound/teleporter.wav");
	safe_snprintf (sound_files[snd_thndr_1], sizeof (sound_files[snd_thndr_1]), "%s/%s", datadir, "sound/thunder1.wav");
	safe_snprintf (sound_files[snd_thndr_2], sizeof (sound_files[snd_thndr_2]), "%s/%s", datadir, "sound/thunder2.wav");
	safe_snprintf (sound_files[snd_thndr_3], sizeof (sound_files[snd_thndr_3]), "%s/%s", datadir, "sound/thunder3.wav");
	safe_snprintf (sound_files[snd_thndr_4], sizeof (sound_files[snd_thndr_4]), "%s/%s", datadir, "sound/thunder4.wav");
	safe_snprintf (sound_files[snd_thndr_5], sizeof (sound_files[snd_thndr_5]), "%s/%s", datadir, "sound/thunder5.wav");
	safe_snprintf (sound_files[snd_fire], sizeof (sound_files[snd_fire]), "%s/%s", datadir, "sound/fire.wav");

	alListenerfv(AL_POSITION,listenerPos);
	alListenerfv(AL_VELOCITY,listenerVel);
	alListenerfv(AL_ORIENTATION,listenerOri);
	alListenerf(AL_GAIN,1.0f);

	//poison data
	for(i=0;i<MAX_SOURCES;i++)
	{
		sound_source[i] = -1;
#ifdef DEBUG
		sound_objects[i].file = -1;
#endif // DEBUG
	}
	for(i=0;i<MAX_BUFFERS;i++)
		sound_buffer[i] = -1;

	//initialize music
#ifdef	OGG_VORBIS
	ogg_file = NULL;

	alGenBuffers(4, music_buffers);
	alGenSources(1, &music_source);
	alSource3f(music_source, AL_POSITION,        0.0, 0.0, 0.0);
	alSource3f(music_source, AL_VELOCITY,        0.0, 0.0, 0.0);
	alSource3f(music_source, AL_DIRECTION,       0.0, 0.0, 0.0);
	alSourcef (music_source, AL_ROLLOFF_FACTOR,  0.0          );
	alSourcei (music_source, AL_SOURCE_RELATIVE, AL_TRUE      );
	alSourcef (music_source, AL_GAIN,            music_gain);
#endif	// OGG_VORBIS
	if((error=alGetError()) != AL_NO_ERROR){
		char str[256];
		safe_snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alGetString(error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound=have_music=0;
		return;
	}
#ifndef MAP_EDITOR2
#ifndef NEW_WEATHER
	//force the rain sound to be recreated
	rain_sound = 0;
#endif //NEW_WEATHER
#endif //MAP_EDITOR2
	inited = 1;
}








#else	// !NEW_SOUND


/***********************
 * NEW SOUND FUNCTIONS *
 ***********************
 *
 * Welcome to EL's new sound system. There are basically 5 parts to the code.
 * 1. Init/destroy, 2. Config, 3. Sources, 4. Streams, 5. Individual sounds
 *
 * This description of the code is currently in its first phase and therefore very
 * general. I intend on expanding on it over time.
 *
 * The base of this code was written by d000hg and has been extended by Torg
 * (torg@grug.redirectme.net).
 *
 *
 *
 * -- Part 1 - Init/destroy --
 * This part deals with initialisation and destruction of the physical aspects of
 * the sound system itself. These include setting up the sound device and sources,
 * and has been seperated from configuration so as to allow sounds to be added and
 * removed from the system without being played. This is so if sound is enabled
 * during the life of any sounds they can be started immediately providing a more
 * complete experience.
 *
 * This code can be considered complete, although could do with a bit more tidying.
 *
 *
 * -- Part 2 - Configuration --
 * This part is responsible for reading and parsing the configuration info.
 * Configuration is by default found in sound/sound_config.xml. Some base XML
 * blocks may be stored in additional files. Documentation on the configuration
 * format can be found in the example sound_config.xml file. This code is
 * straight-foward and very simple to adjust/expand where necessary.
 *
 * This code can also be considered complete.
 *
 *
 * -- Part 3 - Sources --
 * This part handles allocating places to play sounds to the sounds that want to
 * be played. These "places" are OpenAL sources and are initialised in the code
 * in Part 1. They are shared between streams and individual sounds and this code
 * locates and available source and provides the calling function with a pointer
 * to the structure containing the details of this source. There is a mutex lock
 * used over this code to protect the sources array which is shuffled to keep the
 * sources sorted by priority.
 *
 * This part is the most recent code. It seems to be working ok, but is most
 * likely to contain bugs and is the hardest/most complex part to debug due to the
 * varied states of sources, and the sharing.
 *
 *
 * -- Part 4 - Configuration --
 * This part handles the streamed sounds. The configuration for it is stable and
 * very flexible. The logic should be straight-forward and it is seperated into
 * many functions.
 *
 * It can be considered complete, but requires a lot of config info generated.
 *
 *
 * -- Part 5s - Playing individual sounds --
 * This part is the code to handle and play individual sounds. This code is broken
 * into several functions. Sounds triggered in the code in Part 5b are added to
 * the list of sounds to be played. They are then loaded into buffers and linked
 * to a source to be played. There is an update function which is called on a
 * timer to parse this list of active and inactive sounds to update them. The
 * update function works directly on the source list and hence care needs to be
 * taken with mutex locks.
 *
 * This is now pretty stable, but if there are going to be complex bugs (other
 * than with sources - part 3) then it will most likely be here.
 *
 *
 * -- Part 5b - Triggering individual sounds --
 * This final part handles actually triggering the sounds. This code is primarily
 * scattered throughout the client code calling the add/remove sound functions
 * where necessary. Specifying places in the code to start and stop sounds is
 * quite trivial in most cases and any bugs are generally very simple to trace
 * and find.
 *
 * For now this section of code is considered stable, but unlikely to ever be
 * complete.
 *
 *
 */



/**************************
 * SOUND OPTION FUNCTIONS *
 **************************/

void turn_sound_on()
{
	int i, state = 0;
	ALuint source, error;
	
	if (!video_mode_set)
		return;			// Don't load the config until we have video (so we don't load before the loading screen)
	if (!inited)
		init_sound();
	if (!have_sound)
		return;
	LOCK_SOUND_LIST();
	sound_on = 1;
	for (i = 0; i < used_sources; i++)
	{
		source = sound_source_data[i].source;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if (state == AL_PAUSED)
			alSourcePlay(source);
	}
	UNLOCK_SOUND_LIST();
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error turning sound on\n");
#endif // _EXTRA_SOUND_DEBUG
	}
}

void turn_sound_off()
{
	int i = 0, loop;
	ALuint source, error;
	if (!inited)
		return;
#ifdef OGG_VORBIS
	if (!have_music || !music_on)
#endif // OGG_VORBIS
	{
		destroy_sound();
		return;
	}
	LOCK_SOUND_LIST();
	sound_on = 0;
	sound_opts = SOUNDS_NONE;
	while (i < used_sources)
	{
		if (sound_source_data[i].current_stage == STAGE_STREAM)
		{
			i++;			// This source is being used by a stream and handled lower down so ignore
			continue;
		}
		source = sound_source_data[i].source;
		alGetSourcei(source, AL_LOOPING, &loop);
		if (loop == AL_TRUE)
			alSourcePause(source);
		else
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Removing source with index %d\n", i);
#endif //_EXTRA_SOUND_DEBUG
			stop_sound_source_at_index(0);
			continue;
		}
		++i;
	}
	UNLOCK_SOUND_LIST();
#ifdef OGG_VORBIS
	for (i = 0; i < max_streams; i++)
	{
		if (streams[i].type != STREAM_TYPE_MUSIC)
		{
			destroy_stream(&streams[i]);
		}
	}
#endif // OGG_VORBIS
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error turning sound off\n");
#endif //_EXTRA_SOUND_DEBUG
	}
}

void turn_music_on()
{
#ifdef	OGG_VORBIS
	if (!video_mode_set)
		return;			// Don't load the config until we have video (so we don't load before the loading screen)
	music_on = 1;		// Set this here so if ness we will init the sound
	if (!inited)
	{
		init_sound();
	}
	if (!have_music)
		return;
	get_map_playlist();
#endif	// OGG_VORBIS
}

void turn_music_off()
{
#ifdef	OGG_VORBIS
	if ((!have_sound || sound_opts == SOUNDS_NONE) && inited)
	{
		destroy_sound();
		return;
	}
	if (!have_music)
		return;
	
	music_on = 0;
	if (sound_streams_thread != NULL)
	{
		if (music_stream)
		{
			destroy_stream(music_stream);
		}
	}
#endif	// OGG_VORBIS
}

void toggle_sounds(int *var) {
	if (*var != SOUNDS_NONE) {
		*var = SOUNDS_NONE;
		turn_sound_off();
	} else {
		*var = SOUNDS_CLIENT;
		turn_sound_on();
	}
}

/*
void change_sounds(int * var, int value)
{
	int old_val = sound_opts;
	
	if (value == 0) sound_on = 0;
	else sound_on = 1;

	if (value >= 0) *var = value; // We need to set this here so the sound stream doesn't quit if there is no music
	if (value == SOUNDS_NONE && old_val != SOUNDS_NONE)	{
		turn_sound_off();
	} else if (value != SOUNDS_NONE && (!have_sound || old_val == SOUNDS_NONE)) {
		turn_sound_on();
	}
}
*/






/************************
 * OGG VORBIS FUNCTIONS *
 ************************/

#ifdef	OGG_VORBIS
int load_ogg_file(char *file_name, OggVorbis_File *oggFile)
{
	FILE *file;
	int result = 0;

	file = my_fopen(file_name, "rb");

	if (file == NULL)
	{
		return 0;
	}

#ifndef _MSC_VER		// MS's compiler breaks things with files opened from different DLL's
	result = ov_open(file, oggFile, NULL, 0);
#else // !_MSC_VER
	result = ov_open_callbacks(file, oggFile, NULL, 0, OV_CALLBACKS_DEFAULT);
#endif
	if (result < 0)
	{
		LOG_ERROR("Error opening ogg file: %s. Ogg error: %d\n", file_name, result);
		fclose(file);
		return 0;
	}

	return 1;
}

int stream_ogg_file(char *file_name, stream_data * stream, int numBuffers)
{
	int result, more_stream = 0, i;
	
	stop_stream(stream);
	ov_clear(&stream->stream);
	result = load_ogg_file(file_name, &stream->stream);
	if (!result) {
		LOG_ERROR("Error loading ogg file: %s\n", file_name);
		return -1;
	}

	stream->info = ov_info(&stream->stream, -1);
	for (i = 0; i < numBuffers; i++)
	{
		more_stream = stream_ogg(stream->buffers[i], &stream->stream, stream->info);
		if (!more_stream)
		{
			LOG_ERROR("Error playing ogg file: %s\n", file_name);
			return more_stream;		// There was an error, probably too little data to fill the buffers
		}
	}
    
	alSourceQueueBuffers(stream->source, numBuffers, stream->buffers);
	result = start_stream(stream);
	
	return result;
}

int stream_ogg(ALuint buffer, OggVorbis_File * inStream, vorbis_info * info)
{
	char data[STREAM_BUFFER_SIZE];
	int size = 0;
	int section = 0;
	int result = 0;
	int error = 0;
	char str[256];
	ALenum format;

	if ((error=alGetError()) != AL_NO_ERROR)
	{
		LOG_ERROR("stream_ogg %s: %s", my_tolower(reg_error_str), alGetString(error));
	}

	size = 0;
	while (size < STREAM_BUFFER_SIZE)
	{
#ifndef EL_BIG_ENDIAN
        result = ov_read(inStream, data + size, STREAM_BUFFER_SIZE - size, 0, 2, 1, &section);
#else
        result = ov_read(inStream, data + size, STREAM_BUFFER_SIZE - size, 1, 2, 1, &section);
#endif
		safe_snprintf(str, sizeof(str), "%d", result); //prevents optimization errors under Windows, but how/why?
		if (result > 0)
			size += result;
		else if (result == OV_HOLE)
			// OV_HOLE is informational so ignore it
			size = size;
		else if(result < 0)
			ogg_error(result);
		else
			break;
	}

	if (!size)
		return 0;	// The file is finished, quit trying to play
	
	// Check the number of channels... always use 16-bit samples
	if (info->channels == 1)
		format = AL_FORMAT_MONO16;
	else
		format = AL_FORMAT_STEREO16;
	
	alBufferData(buffer, format, data, size, info->rate);

	if ((error=alGetError()) != AL_NO_ERROR) 
	{
		LOG_ERROR("stream_ogg %s: %s", my_tolower(reg_error_str), alGetString(error));
	}
	return 1;
}

#ifdef ALUT_WAV
ALvoid * load_ogg_into_memory(char * szPath, ALenum *inFormat, ALsizei *inSize, ALsizei *inFreq)
#else // ALUT_WAV
ALvoid * load_ogg_into_memory(char * szPath, ALenum *inFormat, ALsizei *inSize, ALfloat *inFreq)
#endif // ALUT_WAV
{
	int bitStream;
	int result;
	vorbis_info *pInfo;
	OggVorbis_File oggFile;
	ALenum format;
	ALsizei size;
	ALfloat freq;
	char * data;
	
	// Reset the variables
	bitStream = 0;
	result = 0;
	format = 0;
	size = 0;
	freq = 0.0f;
	
	// Load the file
	result = load_ogg_file(szPath, &oggFile);
	if (!result)
	{
		return NULL;
	}
	// Get some information about the OGG file
	pInfo = ov_info(&oggFile, -1);
	
	// Check the number of channels... always use 16-bit samples
	if (pInfo->channels == 1)
		format = AL_FORMAT_MONO16;
	else
		format = AL_FORMAT_STEREO16;
	
	// The frequency of the sampling rate
	freq = pInfo->rate;

	data = malloc(OGG_BUFFER_SIZE);
	
	// Hope OGG_BUFFER_SIZE is large enough for this sample - 1MB should be! Anything more should be streamed.
	while (size < OGG_BUFFER_SIZE)
	{
#ifndef EL_BIG_ENDIAN
		result = ov_read(&oggFile, data + size, OGG_BUFFER_SIZE - size, 0, 2, 1, &bitStream);
#else
		result = ov_read(&oggFile, data + size, OGG_BUFFER_SIZE - size, 1, 2, 1, &bitStream);
#endif
		if((result > 0) || (result == OV_HOLE))		// OV_HOLE is informational
		{
			if (result != OV_HOLE) size += result;
		}
		else if(result < 0)
			ogg_error(result);
		else
			break;
	}
	
	ov_clear(&oggFile);

	*inFormat = format;
	*inSize = size;
	*inFreq = freq;	
	return data;
}


/**************************
 * SOUND STREAM FUNCTIONS *
 **************************/

static char * get_stream_type(int type)
{
	return type == STREAM_TYPE_MUSIC ? "Music" : 
		(type == STREAM_TYPE_CROWD ? "Crowd" : 
		(type == STREAM_TYPE_SOUNDS ? "Background" : "None"));
}

void play_stream(int sound, stream_data * stream, ALfloat gain)
{
	char * file;
	char tmp_file_name[80];
	
	// Find the filename to play
	if (stream->type == STREAM_TYPE_MUSIC)
	{
		if (!have_music) return;
		
		if (playlist[sound].file_name[0]!='.' && playlist[sound].file_name[0]!='/')
			safe_snprintf (tmp_file_name, sizeof (tmp_file_name), "./music/%s", playlist[sound].file_name);
		else
			safe_snprintf(tmp_file_name, sizeof (tmp_file_name), "%s", playlist[sound].file_name);
		file = tmp_file_name;
	}
	else
	{
		file = sound_type_data[sound].part[STAGE_MAIN].file_path;
		if (!have_sound || sound == -1 || !strcmp(file, "")) return;
	}
	
	// Set the gain for this stream
	alSourcef (stream->source, AL_GAIN, gain);
	
	// Load the Ogg file and start the stream
	stream_ogg_file(file, stream, NUM_STREAM_BUFFERS);
	stream->sound = sound;
}

int start_stream(stream_data * stream)
{
	int state = 0, error;
	alGetSourcei(stream->source, AL_SOURCE_STATE, &state);
	if (state != AL_PLAYING) {
		alSourcePlay(stream->source);
	}
	stream->playing = 1;
	if ((error = alGetError()) != AL_NO_ERROR) 
	{
		LOG_ERROR("start_stream - Error starting stream - %s: %s", my_tolower(reg_error_str), alGetString(error));
		stream->playing = 0;
		return 0;
	}
	return 1;
}

void stop_stream(stream_data * stream)
{
	ALuint buffer;
	int queued, state = 0, error;
	
	stream->playing = 0;
	alGetSourcei(stream->source, AL_SOURCE_STATE, &state);
	if (state != AL_PAUSED)
		alSourceStop(stream->source);
	alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &stream->processed);
	alGetSourcei(stream->source, AL_BUFFERS_QUEUED, &queued);
	while (queued-- > 0)
	{
		alSourceUnqueueBuffers(stream->source, 1, &buffer);
	}
	if ((error = alGetError()) != AL_NO_ERROR) 
	{
		LOG_ERROR("stop_stream - Error stopping stream - %s: %s", my_tolower(reg_error_str), alGetString(error));
	}
}

void reset_stream(stream_data * stream)
{
#ifdef _EXTRA_SOUND_DEBUG
//	printf("Resetting stream: Type: %s, sound: %d, cookie: %d, source: %d\n", get_stream_type(stream->type), stream->sound, stream->cookie, stream->source);
#endif // _EXTRA_SOUND_DEBUG
	// Reset the data (not sources or buffers)
	stream->playing = 0;
	stream->info = NULL;
	stream->processed = 0;
	stream->sound = -1;
	stream->is_default = 0;
	stream->boundary = NULL;
	stream->fade = 0;
	stream->fade_length = 10;
}

int init_sound_stream(stream_data * stream, int type, int priority)
{
	source_data * source;
	int error;
	
	alGenBuffers(NUM_STREAM_BUFFERS, stream->buffers);

	LOCK_SOUND_LIST();
	source = get_available_source(priority);
	if (!source || !alIsSource(source->source))
		return 0;
	// Set some details so non-streamed functions see this source as used
	source->cookie = get_next_cookie();
	source->current_stage = STAGE_STREAM;
	source->loaded_sound = 0;
	// Make sure the data for this stream is clean
	reset_stream(stream);
	stream->type = type;
	stream->source = source->source;
	stream->cookie = source->cookie;
	UNLOCK_SOUND_LIST();
	
	// Reset the source details
	alSource3f(stream->source, AL_POSITION,        0.0, 0.0, 0.0);
	alSource3f(stream->source, AL_VELOCITY,        0.0, 0.0, 0.0);
	alSource3f(stream->source, AL_DIRECTION,       0.0, 0.0, 0.0);
	alSourcef (stream->source, AL_ROLLOFF_FACTOR,  0.0          );
	alSourcei (stream->source, AL_SOURCE_RELATIVE, AL_TRUE      );
	alSourcef (stream->source, AL_GAIN,            0.0);
	alSourcei (stream->source, AL_LOOPING,         AL_FALSE);
	alSourceStop(stream->source);
	alSourcei (stream->source, AL_BUFFER,          0);		// Make sure there are no buffers attached

	// Reset the error buffer
	if ((error = alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error initalising stream: Type: %s, %s\n", get_stream_type(type), alGetString(error));
#endif // _EXTRA_SOUND_DEBUG
	}
	
	return 1;
}

void destroy_stream(stream_data * stream)
{
	int error, i;

	if (!inited)
	   return;

#ifdef _EXTRA_SOUND_DEBUG
//	printf("Destroying stream: Type: %s, sound: %d, cookie: %d, source: %d\n", get_stream_type(stream->type), stream->sound, stream->cookie, stream->source);
#endif // _EXTRA_SOUND_DEBUG
	if (stream->type == STREAM_TYPE_NONE)
		return;			// In theory this stream isn't initialised
	
	if (stream->playing)
	{
		stop_stream(stream);
	}
	if (stream->type == STREAM_TYPE_MUSIC)
	{
		music_stream = NULL;
	}
	
	stream->type = STREAM_TYPE_NONE;
	if (stream->cookie != 0)
	{
		// Find which of our playing sources matches the handle for this stream
		LOCK_SOUND_LIST();
		i = find_sound_source_from_cookie(stream->cookie);
		if (i >= 0)
		{
			stop_sound_source_at_index(i);
		}
		UNLOCK_SOUND_LIST();
	}
	stream->source = 0;
	stream->cookie = 0;
	reset_stream(stream);
	for (i = 0; i < NUM_STREAM_BUFFERS; i++)
	{
		if (alIsBuffer(stream->buffers[i]))
			alDeleteBuffers(1, stream->buffers+i);
	}
	ov_clear(&stream->stream);

	// Reset the error buffer
	if ((error = alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error destroying stream: Type: %s, %s\n", get_stream_type(stream->type), alGetString(error));
#endif // _EXTRA_SOUND_DEBUG
	}
}

/* add_stream
 *
 * Processes adding a new stream if possible
 *
 * sound		the sound to play in the stream
 * type			the type of stream
 * boundary		the map boundary def being used for this stream (-1 for default sounds)
 */
int add_stream(int sound, int type, int boundary)
{
	stream_data * stream = NULL;
	int priority = 0, i, default_found = -1, type_count = 0;
#ifdef _EXTRA_SOUND_DEBUG
	int our_stream;
#endif // _EXTRA_SOUND_DEBUG
	
	if (sound == -1)
		return 0;
	
	// Check we aren't creating another music stream if one exists
	if (type == STREAM_TYPE_MUSIC && music_stream)
	{
		LOG_ERROR("Sound error: Tried to create additional music stream. This is a bug!\n");
		return 0;
	}
	
	for (i = 0; i < max_streams; i++)
	{
		// Check this sound isn't already being played in a stream
		if (streams[i].sound == sound && streams[i].type == type)
		{
#ifdef _EXTRA_SOUND_DEBUG
//			printf("add_stream error: This stream already exists! Stream ID: %d, sound: %d\n", i, sound);
#endif // _EXTRA_SOUND_DEBUG
			return 0;
		}
		if (streams[i].type == type)
		{
			// Count the number of streams of this type (for the priority calculation)
			type_count++;
			// Check if there is a default stream of this type that needs to be stopped
			if (streams[i].is_default)
			{
#ifdef _EXTRA_SOUND_DEBUG
				printf("add_stream: Found a default stream: Stream ID: %d, sound: %d\n", i, streams[i].sound);
#endif // _EXTRA_SOUND_DEBUG
				default_found = i;
			}
		}
		// Find a spot in the array
		if (!stream && streams[i].type == STREAM_TYPE_NONE)
		{
#ifdef _EXTRA_SOUND_DEBUG
			our_stream = i;
#endif // _EXTRA_SOUND_DEBUG
			stream = &streams[i];
		}
	}
	if (!stream)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("add_stream error: Unable to create stream! All streams used.\n");
#endif // _EXTRA_SOUND_DEBUG
		return 0;
	}
	// Calculate the priority of this stream (give the first 2 streams of this type top priority and then according to sound)
	if (type_count > 2)
		priority = sound_type_data[sound].priority;
	else
		priority = 1;
#ifdef _EXTRA_SOUND_DEBUG
	printf("add_stream: Initialising stream - Type: %s, Priority: %d, Boundary: %d\n", get_stream_type(type), priority, boundary);
#endif // _EXTRA_SOUND_DEBUG
	// Init the stream
	if (!init_sound_stream(stream, type, priority))
		return 0;
#ifdef _EXTRA_SOUND_DEBUG
	printf("add_stream: Done initialising stream.\n");
#endif // _EXTRA_SOUND_DEBUG
	// Play the stream
	play_stream(sound, stream, 0.0f);		// Fade the stream up
	if (!stream->playing)
	{
		// There was an error so remove the stream and bail
#ifdef _EXTRA_SOUND_DEBUG
		printf("add_stream: Eek, there was an error adding this stream! sound: %d\n", sound);
#endif // _EXTRA_SOUND_DEBUG
		if (type == STREAM_TYPE_MUSIC)
			destroy_stream(stream);
		return 0;
	}
	stream->type = type;
	stream->fade = 1;
	stream->fade_length = 10;
	if (boundary == -1)
	{
		stream->is_default = 1;
		stream->boundary = NULL;
	}
	else
	{
		stream->boundary = &sound_map_data[snd_cur_map].boundaries[boundary];
		stream->is_default = 0;
	}
#ifdef _EXTRA_SOUND_DEBUG
	printf("add_stream: Started stream %d. Type: %s, Sound: %d, Cookie: %u, Source: %d, Fade: %d, Default: %d\n", our_stream, get_stream_type(stream->type), stream->sound, stream->cookie, stream->source, stream->fade, stream->is_default);
#endif // _EXTRA_SOUND_DEBUG
	// If this is the music stream, set the pointer
	if (type == STREAM_TYPE_MUSIC)
		music_stream = stream;
	// We have definitely started this stream so stop any default we found
	if (default_found > -1)
		streams[default_found].fade = -1;
	
	return 1;		// Return success
}

int check_for_valid_stream_sound(int tx, int ty, int type)
{
	int i, j, snd = -1, playing, found = 0;

	if (snd_cur_map > -1 && sound_map_data[snd_cur_map].id > -1)
	{
		for (i = 0; i < sound_map_data[snd_cur_map].num_boundaries; i++)
		{
			if (i == sound_map_data[snd_cur_map].num_boundaries)
				i = 0;
			if (type == STREAM_TYPE_SOUNDS)
			{
				snd = sound_map_data[snd_cur_map].boundaries[i].bg_sound;
			}
			else if (type == STREAM_TYPE_CROWD)
			{
				snd = sound_map_data[snd_cur_map].boundaries[i].crowd_sound;
			}
			if (snd > -1 && sound_bounds_check(tx, ty, sound_map_data[snd_cur_map].boundaries[i]))
			{
				playing = 0;
				for (j = 0; j < max_streams; j++)
				{
					if (streams[j].sound == snd)
						playing = 1;
				}
				if (!playing)
				{
#ifdef _EXTRA_SOUND_DEBUG
					printf("Starting stream (%s) sound: %d, boundary: %d\n", get_stream_type(type), snd, i);
#endif //_EXTRA_SOUND_DEBUG
					add_stream(snd, type, i);
					found = 1;
				}
			}
		}
	}
	return found;
}

void check_for_new_streams(int tx, int ty)
{
	map_sound_data * cur_map;
	int i, found_bg = 0, found_crowd = 0;
	
	found_bg = check_for_valid_stream_sound(tx, ty, STREAM_TYPE_SOUNDS);
	if (no_near_enhanced_actors >= 5)
		found_crowd = check_for_valid_stream_sound(tx, ty, STREAM_TYPE_CROWD);
	
	if (!found_bg && !found_crowd)
	{

#ifdef _EXTRA_SOUND_DEBUG
//		printf("check_for_new_streams - Checking if we need a default background/crowd sound. Pos: %d, %d\n", tx, ty);
#endif //_EXTRA_SOUND_DEBUG
		
		// Check if we need a default background or default crowd sound (no other bg/crowd sounds, or they are fading out)
		for (i = 0; i < max_streams; i++)
		{
			if (no_near_enhanced_actors < 5 || (streams[i].type == STREAM_TYPE_CROWD && streams[i].fade >= 0))
				found_crowd = 1;		// We have a crowd sound already (or don't need one) so we don't need a default one
			if (streams[i].type == STREAM_TYPE_SOUNDS && streams[i].fade >= 0)
				found_bg = 1;		// We have a bg sound already so we don't need a default one
			if (found_bg && found_crowd)
				return;		// We have both so we don't need to continue
		}

#ifdef _EXTRA_SOUND_DEBUG
//		printf("check_for_new_streams - Background found: %s, Crowd found: %s. Checking for defaults for this map\n", found_bg == 0 ? "No" : "Yes", found_crowd == 0 ? "No" : "Yes");
#endif //_EXTRA_SOUND_DEBUG

		// We still aren't playing a sound for one type, so check for a map based sound
		cur_map = &sound_map_data[snd_cur_map];
		if (cur_map->num_defaults > 0)
		{
			for (i = 0; i < cur_map->num_defaults; i++)
			{
				if (!found_bg && time_of_day_valid(cur_map->boundaries[cur_map->defaults[i]].time_of_day_flags) &&
					cur_map->boundaries[cur_map->defaults[i]].bg_sound > -1)
				{
#ifdef _EXTRA_SOUND_DEBUG
//					printf("check_for_new_streams - Playing map default background sound: %d\n", cur_map->boundaries[cur_map->defaults[i]].bg_sound);
#endif //_EXTRA_SOUND_DEBUG
					add_stream(cur_map->boundaries[cur_map->defaults[i]].bg_sound, STREAM_TYPE_SOUNDS, -1);
					found_bg = 1;
				}
				if (!found_crowd && time_of_day_valid(cur_map->boundaries[cur_map->defaults[i]].time_of_day_flags) &&
					cur_map->boundaries[cur_map->defaults[i]].crowd_sound > -1)
				{
#ifdef _EXTRA_SOUND_DEBUG
					printf("check_for_new_streams - Playing map default crowd sound: %d\n", cur_map->boundaries[cur_map->defaults[i]].crowd_sound);
#endif //_EXTRA_SOUND_DEBUG
					add_stream(cur_map->boundaries[cur_map->defaults[i]].crowd_sound, STREAM_TYPE_CROWD, -1);
					found_crowd = 1;
				}
				if (found_bg && found_crowd)
					break;
			}
		}
		
		// Check if we need to find a global default background
		if (!found_bg && sound_num_background_defaults > 0)
		{
#ifdef _EXTRA_SOUND_DEBUG
//			printf("check_for_new_streams - No map defaults found, checking for global defaults\n");
#endif //_EXTRA_SOUND_DEBUG
			for (i = 0; i < sound_num_background_defaults; i++)
			{
				if (time_of_day_valid(sound_background_defaults[i].time_of_day_flags) &&
					(sound_background_defaults[i].map_type == dungeon && sound_background_defaults[i].sound > -1))
				{
#ifdef _EXTRA_SOUND_DEBUG
//					printf("check_for_new_streams - Playing default background sound: %d\n", sound_background_defaults[i].sound);
#endif //_EXTRA_SOUND_DEBUG
					add_stream(sound_background_defaults[i].sound, STREAM_TYPE_SOUNDS, -1);
					break;
				}
			}
		}
		
		// If we still aren't playing a crowd sound, check for a global default
		if (!found_crowd && crowd_default > -1)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("check_for_new_streams - Playing default crowd sound: %d\n", crowd_default);
#endif //_EXTRA_SOUND_DEBUG
			add_stream(crowd_default, STREAM_TYPE_CROWD, -1);
		}
	}
}

int process_stream(stream_data * stream, ALfloat gain, int * sleep)
{
	int error, state = AL_STOPPED;	// , state2;
	ALfloat new_gain = gain;
	
	// Check if we are starting/stopping this stream and continue the fade
	if (stream->fade < 0)
	{
		new_gain = gain - ((float)(-stream->fade) * (gain / stream->fade_length));
#ifdef _EXTRA_SOUND_DEBUG
		printf("Doing stream fade - stream: %s, sound: %d, fade: %d, gain: %f\n", get_stream_type(stream->type), stream->sound, stream->fade, new_gain);
#endif // _EXTRA_SOUND_DEBUG
		stream->fade -= 1;
	}
	else if (stream->fade > 0)
	{
		new_gain = gain - ((float)(stream->fade_length - stream->fade) * (gain / stream->fade_length));
#ifdef _EXTRA_SOUND_DEBUG
		printf("Doing stream fade - stream: %s, sound: %d, fade: %d, gain: %f\n", get_stream_type(stream->type), stream->sound, stream->fade, new_gain);
#endif // _EXTRA_SOUND_DEBUG
		stream->fade += 1;
	}
	
	// Check if we need to dim down the sounds due to rain
	if (stream->type != STREAM_TYPE_MUSIC)
	{
		new_gain = weather_adjust_gain(new_gain, -1);
#ifdef _EXTRA_SOUND_DEBUG
//		if (new_gain != gain)
//			printf("Volume adjusted by fade/weather for stream %s - Original gain: %f, New gain: %f\n", get_stream_type(stream->type), gain, new_gain);
#endif // _EXTRA_SOUND_DEBUG
	}

	alSourcef(stream->source, AL_GAIN, new_gain);
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("process_stream - Error setting gain: %s, Stream type: %s, Source: %d\n", alGetString(error), get_stream_type(stream->type), stream->source);
#endif // _EXTRA_SOUND_DEBUG
	}

	// Process the next portion of the stream
	alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &stream->processed);
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("process_stream - Error retrieving buffers processed value: %s, Stream type: %s, Source: %d\n", alGetString(error), get_stream_type(stream->type), stream->source);
#endif // _EXTRA_SOUND_DEBUG
	}
	if (!stream->processed)
	{
		if (*sleep < SLEEP_TIME)
			*sleep += (SLEEP_TIME / 100);
		return 1; // Skip error checking et al
	}
	while (stream->processed-- > 0)
	{
		ALuint buffer;

		alSourceUnqueueBuffers(stream->source, 1, &buffer);
		if ((error = alGetError()) != AL_NO_ERROR)
			LOG_ERROR("process_stream - Error unqueuing buffers: %s, Stream type: %s, Source: %d", alGetString(error), get_stream_type(stream->type), stream->source);

		stream->playing = stream_ogg(buffer, &stream->stream, stream->info);
		
		alSourceQueueBuffers(stream->source, 1, &buffer);
		if ((error = alGetError()) != AL_NO_ERROR)
			LOG_ERROR("process_stream - Error requeuing buffers: %s, Stream type: %s, Source: %d, Playing: %s", alGetString(error), get_stream_type(stream->type), stream->source, stream->playing == 0 ? "No" : "Yes");
	}
	
	// Check if the stream is still playing, and handle if not
	alGetSourcei(stream->source, AL_SOURCE_STATE, &state);
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("process_stream - Error retrieving state: %s, Stream type: %s, Source: %d\n", alGetString(error), get_stream_type(stream->type), stream->source);
#endif // _EXTRA_SOUND_DEBUG
	}
//	state2 = state;		// Fake out the Dev-C++ optimizer!		<-- FIXME: Is this still nessessary?
	if (state != AL_PLAYING)
	{
		LOG_TO_CONSOLE(c_red1, snd_skip_speedup);
#ifdef _EXTRA_SOUND_DEBUG
		printf("process_stream - Stream Skip!! Sleep: %d\n", *sleep);
#endif //_EXTRA_SOUND_DEBUG
		// Let streams skip up to 10 times. If it skips more, then stop the music/sound (not the best option).
		if (*sleep > (SLEEP_TIME / 10))
			*sleep -= (SLEEP_TIME / 10);
		else if (*sleep > 1) *sleep = 1;
		else
		{
			LOG_TO_CONSOLE(c_red1, snd_too_slow);
			LOG_ERROR(snd_too_slow);
			if (stream->type == STREAM_TYPE_MUSIC)
				turn_music_off();
			else
				turn_sound_off();
			*sleep = SLEEP_TIME;
			return 0;
		}
		alSourcePlay(stream->source);
	}
	
	// Check if the stream has finished and needs to be requeued
	if (!stream->playing)
	{
		play_stream(stream->sound, stream, gain);
	}
	
	// Clear any OpenAL errors
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("process_stream - OpenAL error: %s, Stream type: %s, Source: %d\n", alGetString(error), get_stream_type(stream->type), stream->source);
#endif // _EXTRA_SOUND_DEBUG
	}
	
	return 1;
}

int check_stream(stream_data * stream, int day_time, int tx, int ty)
{
	// Check if we have finished fading this stream down and can stop it
	if (stream->fade < -stream->fade_length)
	{
		stop_stream(stream);
		// If this stream isn't the music stream then destroy it
		if (stream->type != STREAM_TYPE_MUSIC)
		{
			destroy_stream(stream);
		}
		return 0;		// Stream is not continuing
	}
	// Check if we have finished fading this stream up and can stop the fade
	else if (stream->fade > stream->fade_length)
	{
		stream->fade = 0;
	}
	
	// Check if we need to trigger a stop in this stream
	if (stream->fade >= 0)
	{
		switch (stream->type)
		{
			case STREAM_TYPE_MUSIC:
				if (tx < playlist[list_pos].min_x ||
				   tx > playlist[list_pos].max_x ||
				   ty < playlist[list_pos].min_y ||
				   ty > playlist[list_pos].max_y ||
				   (playlist[list_pos].time != 2 &&
					playlist[list_pos].time != day_time))
				{
					stream->fade = -1;
				}
				break;
			case STREAM_TYPE_SOUNDS:
			case STREAM_TYPE_CROWD:
				if (stream->type == STREAM_TYPE_CROWD && no_near_enhanced_actors < 5)
				{
					stream->fade = -1;
				}
				if (stream->is_default)
				{
#ifdef _EXTRA_SOUND_DEBUG
//					printf("process_stream - Checking for valid %s sound. Pos: %d, %d\n", get_stream_type(stream->type), tx, ty);
#endif //_EXTRA_SOUND_DEBUG
					if (check_for_valid_stream_sound(tx, ty, stream->type))
						stream->fade = -1;
				}
				else
				{
					if (!sound_bounds_check(tx, ty, *stream->boundary))
					{
#ifdef _EXTRA_SOUND_DEBUG
						printf("process_stream - %s sound failed bounds check. Pos: %d, %d\n", get_stream_type(stream->type), tx, ty);
#endif //_EXTRA_SOUND_DEBUG
						stream->fade = -1;
					}
				}
				break;
		}
	}

	return 1;		// Stream is continuing (but may be fading down)
}

int update_streams(void * dummy)
{
    int sleep, day_time, i, tx, ty;
	ALfloat gain;

   	sleep = SLEEP_TIME;
	
#ifdef _EXTRA_SOUND_DEBUG
	printf("Starting streams thread\n");
#endif //_EXTRA_SOUND_DEBUG
	while (!exit_now && ((have_music && music_on) || (have_sound && sound_opts >= SOUNDS_ENVIRO)))
	{
		SDL_Delay(sleep);
		
		day_time = (game_minute >= 30 && game_minute < 60 * 3 + 30);
		// Get our position
		if (your_actor)
		{
			tx = your_actor->x_pos * 2;
			ty = your_actor->y_pos * 2;
		}
		else
		{
			tx = 0;
			ty = 0;
		}
		if (have_a_map && (tx > 0 || ty > 0))
		{
			// Check if we need to start or stop any streams
			if (have_music && music_on && (!music_stream || !music_stream->playing))
			{
				find_next_song(tx, ty, day_time);
			}
			if (have_sound && sound_opts > SOUNDS_NONE)
			{
				check_for_new_streams(tx, ty);
			}
			
			// Handle the streams
			for (i = 0; i < max_streams; i++)
			{
				if (streams[i].playing)
				{
					switch (streams[i].type)
					{
						case STREAM_TYPE_MUSIC:
							if (!have_music || !music_on)
								continue;			// We aren't playing music so skip this stream
							gain = music_gain;
							break;
						case STREAM_TYPE_SOUNDS:
							if (!have_sound || sound_opts == SOUNDS_NONE)
								continue;			// We aren't playing sounds so skip this stream
							gain = sound_gain * enviro_gain * sound_type_data[streams[i].sound].gain;
							break;
						case STREAM_TYPE_CROWD:
							if (!have_sound || sound_opts == SOUNDS_NONE)
								continue;			// We aren't playing sounds so skip this stream
							if (distanceSq_to_near_enhanced_actors == 0)
								distanceSq_to_near_enhanced_actors = 100.0f;	// Due to no actors when calc'ing
							gain = sound_gain * crowd_gain * sound_type_data[streams[i].sound].gain
									* sqrt(sqrt(no_near_enhanced_actors)) / sqrt(distanceSq_to_near_enhanced_actors) * 2;
							break;
					}
					if (check_stream(&streams[i], day_time, tx, ty))
						process_stream(&streams[i], gain, &sleep);
				}
			}
		}
	}
	
#ifdef _EXTRA_SOUND_DEBUG
	printf("Exiting streams thread. have_music: %d, music_on: %d, have_sound: %d, sound_opts: %d, exit_now: %d\n", have_music, music_on, have_sound, sound_opts, exit_now);
#endif //_EXTRA_SOUND_DEBUG
	return 1;
}
#endif // OGG_VORBIS




/*******************
 * MUSIC FUNCTIONS *
 *******************/

int display_song_name()
{
#ifndef OGG_VORBIS 
#ifdef ELC
	LOG_TO_CONSOLE(c_red2, snd_no_music);
#endif
#else // !OGG_VORBIS
	if (!music_stream->playing)
	{
		LOG_TO_CONSOLE(c_grey1, snd_media_music_stopped);
	}
	else
	{
		char musname[100];
		int t_played_min, t_played_sec, t_total_min, t_total_sec;
		char *title = NULL, *artist = NULL;
		int i=0;
		vorbis_comment *comments;
		
		t_played_min = (int)(ov_time_tell(&music_stream->stream) / 60);
		t_played_sec = (int)ov_time_tell(&music_stream->stream) % 60;
		t_total_min = (int)(ov_time_total(&music_stream->stream, -1) / 60);
		t_total_sec = (int)ov_time_total(&music_stream->stream, -1) % 60;

		comments = ov_comment(&music_stream->stream, -1);
		if(comments == NULL)
		{
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, playlist[list_pos].file_name, t_played_min, t_played_sec, t_total_min, t_total_sec);
			LOG_TO_CONSOLE(c_grey1, musname);
			return 1;
		}
		for (; i < comments->comments; ++i)
		{
			if ((artist == NULL) && (comments->comment_lengths[i] > 6) && (my_strncompare(comments->user_comments[i],"artist", 6)))
			{
				artist = comments->user_comments[i] + 7;
				if(title)
					break;
			}
			else if ((title == NULL) && (comments->comment_lengths[i] > 6) && (my_strncompare(comments->user_comments[i],"title", 5)))
			{
				title = comments->user_comments[i] + 6;
				if(artist)
					break;
			}
		}
		if (artist && title)
		{
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info, title, artist, t_played_min, t_played_sec, t_total_min, t_total_sec);
		}
		else if (title)
		{
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, title, t_played_min, t_played_sec, t_total_min, t_total_sec);
		}
		else
		{
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, playlist[list_pos].file_name, t_played_min, t_played_sec, t_total_min, t_total_sec);
		}
		LOG_TO_CONSOLE(c_grey1, musname);
	}
#endif // !OGG_VORBIS
	return 1;
}

void play_song(int list_pos)
{
	// Find the music stream
	if (music_stream)
	{
		play_stream(list_pos, music_stream, 0.0f);
		music_stream->fade = 1;
		return;
	}
	// Don't have a music stream yet so create one
	add_stream(list_pos, STREAM_TYPE_MUSIC, -1);
}

void find_next_song(int tx, int ty, int day_time)
{
	if (playlist[list_pos+1].file_name[0])
	{
		list_pos++;
		if (tx > playlist[list_pos].min_x &&
		   tx < playlist[list_pos].max_x &&
		   ty > playlist[list_pos].min_y &&
		   ty < playlist[list_pos].max_y &&
		   (playlist[list_pos].time == 2 ||
			playlist[list_pos].time == day_time))
		{
			// Found a song so play it
			play_song(list_pos);
		}
	}
	else if (loop_list)
		list_pos = -1;
	else
		get_map_playlist();
}




/************************************
 * COMMON SOURCE HANDLING FUNCTIONS *
 ************************************/
source_data * get_available_source(int priority)
{
	source_data * pSource;
	int i;
	
	// Search for an available source. The sources are ordered by decreasing play priority
	for (pSource = sound_source_data, i = 0; i < used_sources; ++i, ++pSource)
	{
		if (priority <= pSource->priority || (pSource->loaded_sound < 0 && pSource->current_stage != STAGE_STREAM))
		{
			if (pSource->loaded_sound >= 0)
			{
#ifdef _EXTRA_SOUND_DEBUG
				printf("Inserting new source at index %d/%d\n", i, used_sources);
#endif //_EXTRA_SOUND_DEBUG
				insert_sound_source_at_index(i);
#ifdef _EXTRA_SOUND_DEBUG
			}
			else
			{
				printf("Found available source at index %d/%d\n", i, used_sources);
#endif //_EXTRA_SOUND_DEBUG
			}
			return pSource;
		}
	}

	if (i == max_sources)
	{
		// All sources are used by higher-priority sounds
		return NULL;
	}
	else if (i == used_sources)
	{
		// This is the lowest-priority sound but there is a spare slot at the end of the list
#ifdef _EXTRA_SOUND_DEBUG
		printf("Creating a new source: %d/%d\n", used_sources, used_sources);
#endif //_EXTRA_SOUND_DEBUG

		pSource = insert_sound_source_at_index(used_sources);
	}
	
	pSource->priority = priority;
	
	return pSource;
}

// This takes a copy the first unused source object (or last one in the list if all used),
// moves all objects after #index along one place, and inserts the copied object at #index;
// It is ensured the source at #index is stopped with no buffers queued
source_data *insert_sound_source_at_index(unsigned int index)
{
	int i;
	source_data tempSource;
	// Take a copy of the source about to be overwritten
	tempSource = sound_source_data[min2i(used_sources, max_sources - 1)];
	// Ensure it is stopped and ready
	alSourceStop(tempSource.source);
	alSourcei(tempSource.source, AL_BUFFER, 0);
	tempSource.play_duration = 0;
	tempSource.current_stage = STAGE_UNUSED;
	tempSource.loaded_sound = -1;
	tempSource.cookie = 0;

	// Shunt source objects down a place
	for (i = min2i(used_sources, max_sources - 1); i > index; --i)
	{
		sound_source_data[i] = sound_source_data[i - 1];
	}

	// Now insert our stored object at #index
	sound_source_data[index] = tempSource;

	// Although it's not doing anything, we have added a new source to the playing set
	if (used_sources < max_sources)
		++used_sources;	

	// Return a pointer to this new source
	return &sound_source_data[index];
}

// This stops the source for sound_source_data[index]. Because this array will change, the index
// associated with a source will change, so this function should only be called if the index is
// known for certain.
int stop_sound_source_at_index(int index)
{
	ALuint error = AL_NO_ERROR;
	source_data *pSource, sourceTemp;
	
	if (index < 0 || index >= used_sources)
		return 0;
	
	pSource = &sound_source_data[index];
	// This unqueues any samples
	if (alIsSource(pSource->source))
	{
		alSourceStop(pSource->source);
		alSourcei(pSource->source, AL_BUFFER, 0);
	}
	else
	{
   		LOG_ERROR("Attempting to stop invalid sound source %d with index %d", (int)pSource->source, index);
	}
	// Clear any errors so as to not confuse other error handlers
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error '%s' stopping sound source with index: %d/%d.  Source: %d.\n", alGetString(error), index, used_sources, (int)pSource->source);
#endif //_EXTRA_SOUND_DEBUG
	}

	// We can't lose a source handle - copy this...
	sourceTemp = *pSource;
	//...shift all the next sources up a place, overwriting the stopped source...
	memcpy(pSource, pSource+1, sizeof(source_data) * (used_sources - (index + 1)));
	//...and put the saved object back in after them
	sound_source_data[used_sources - 1] = sourceTemp;

	// Reset the data for that now blank source
	sourceTemp.play_duration = 0;
	sourceTemp.current_stage = STAGE_UNUSED;
	sourceTemp.loaded_sound = -1;
	sourceTemp.cookie = 0;
	
	// Note that one less source is playing!
	--used_sources;
	
	return 1;
}



/*****************************************
 * INDIVIDUAL SOUND PROCESSING FUNCTIONS *
 *****************************************/


/* If the sample given by the filename is not currently loaded, create a
 * buffer and load it from the path given.
 * Returns the sample number for success or -1 on failure.
 */
int ensure_sample_loaded(char * filename)
{
	int i, j, k, sample_num, error;
	ALvoid *data;
#if defined ALUT_WAV && !defined OSX
	ALboolean loop;
#endif // ALUT_WAV && !OSX
	ALuint *pBuffer;
	sound_sample *pSample;

#ifdef _EXTRA_SOUND_DEBUG
//	printf("Ensuring the sample '%s' is loaded, or something\n", filename);
#endif //_EXTRA_SOUND_DEBUG
	
	// Check if this sample is already loaded and if so, return the sample ID
	for (i = 0; i < num_types; i++)
	{
#ifdef _EXTRA_SOUND_DEBUG
//		printf("Got here 1: %d\n", i);
#endif //_EXTRA_SOUND_DEBUG
		for (j = 0; j < 3; j++)
		{
#ifdef _EXTRA_SOUND_DEBUG
//			printf("Got here 2: %d\n", j);
#endif //_EXTRA_SOUND_DEBUG
			if (!strcasecmp(sound_type_data[i].part[j].file_path, filename)
				&& sound_type_data[i].part[j].sample_num > -1)
			{
#ifdef _EXTRA_SOUND_DEBUG
				printf("Found this sample already loaded: %s, sound %d, part %d\n", filename, i, j);
#endif //_EXTRA_SOUND_DEBUG
				return sound_type_data[i].part[j].sample_num;
			}
		}
	}
#ifdef _EXTRA_SOUND_DEBUG
//	printf("Got here\n");
#endif //_EXTRA_SOUND_DEBUG
	
	// Sample isn't loaded so check the number of samples for room
	sample_num = -1;
#ifdef _EXTRA_SOUND_DEBUG
//	printf("Got here\n");
#endif //_EXTRA_SOUND_DEBUG
	if (num_samples >= MAX_BUFFERS)
	{
		// We need to make room for this sample so find an inactive one (not loaded into a source atm)
		
		// FIXME!! There has to be a better way to do this! I'll review this when I get the chance.
		for (i = 0; i < num_samples; i++)
		{
			for (j = 0; j < used_sources; j++)
			{
				for (k = STAGE_INTRO; k <= STAGE_OUTRO; k++)
				{
					// Check if this sample is loaded into a source atm
					if (sound_type_data[sounds_list[sound_source_data[j].loaded_sound].sound].part[k].sample_num == i)
					{
#ifdef _EXTRA_SOUND_DEBUG
						printf("Found sample is loaded: sound %d, part %d\n", j, k);
#endif //_EXTRA_SOUND_DEBUG
						break;
					}
				}
			}
			if (j >= used_sources)
			{
				// This is an unused sample as had it been used then it would have broken the loop earlier
				reset_buffer_details(i);
				sample_num = i;
#ifdef _EXTRA_SOUND_DEBUG
				printf("Found unused sample slot: %d\n", i);
#endif //_EXTRA_SOUND_DEBUG
				break;
			}
		}
#ifdef _EXTRA_SOUND_DEBUG
//	printf("Got here\n");
#endif //_EXTRA_SOUND_DEBUG
		if (sample_num == -1)
		{
			// We didn't find an available sample slot so error and bail
#ifdef _EXTRA_SOUND_DEBUG
			LOG_ERROR("Error: Unable to load sample: %s, num samples: %d\n", filename, num_samples);
#endif //_EXTRA_SOUND_DEBUG
			return -1;
		}
	}
	else
	{
#ifdef _EXTRA_SOUND_DEBUG
//	printf("Got here\n");
#endif //_EXTRA_SOUND_DEBUG
		sample_num = num_samples;
		num_samples++;
	}
#ifdef _EXTRA_SOUND_DEBUG
	printf("Got sample num: %d\n", sample_num);
#endif //_EXTRA_SOUND_DEBUG
	pSample = &sound_sample_data[sample_num];
	
	// This file is not currently loaded so load it
#ifdef _EXTRA_SOUND_DEBUG
	printf("Attemping to load sound: File: %s\n", filename);
#endif //_EXTRA_SOUND_DEBUG

#ifdef	OGG_VORBIS
	// Do a crude check of the extension to choose which loader
	if (!strcasecmp(filename+(strlen(filename) - 4), ".ogg"))
	{
		// Assume it is actually an OggVorbis file and use our ogg loader
		data = load_ogg_into_memory(filename, &pSample->format, &pSample->size, &pSample->freq);
		if (!data)
		{
			// Couldn't load the file, so release this sample num
			num_samples--;
			// We have already dumped an error message so just return
			return -1;
		}
	}
	else
#endif	// OGG_VORBIS
	{
		// Use one of the WAV loaders (preferrably alutLoadMemoryFromFile as alutLoadWAVFile is depreciated
		// However, the newer function doesn't exist under Mac, and older Alut libs
		// This mess can be removed if we move to a completely ogg client
#ifndef  ALUT_WAV
        data = alutLoadMemoryFromFile(filename, &pSample->format, &pSample->size, &pSample->freq);
#else  // !ALUT_WAV
 #ifndef OSX
		alutLoadWAVFile(filename, &pSample->format, &data, &pSample->size, &pSample->freq, &loop);
 #else // !OSX
		alutLoadWAVFile(filename, &pSample->format, &data, &pSample->size, &pSample->freq);
 #endif  // !OSX
#endif  // !ALUT_WAV
		if (!data)
		{
			// Couldn't load the file
#ifdef ELC
	#if defined alutGetErrorString && alutGetError
			LOG_ERROR("%s: %s", snd_buff_error, alutGetErrorString(alutGetError()));
	#else
			LOG_ERROR("%s: %s", snd_buff_error, "Error opening WAV file");
	#endif
#else
#ifdef  ALUT_WAV
			printf("ensure_sample_loaded : alutLoadWAVFile(%s) = %s\n",	filename, "NO SOUND DATA");
#else
			printf("ensure_sample_loaded : alutLoadMemoryFromFile(%s) = %s\n",	filename, "NO SOUND DATA");
#endif  //ALUT_WAV
#endif  //ELC
			// Release this sample num
			num_samples--;
			return -1;
		}
	}
			
#ifdef _EXTRA_SOUND_DEBUG
	printf("Result: File: %s, Format: %d, Size: %d, Freq: %f\n", filename, (int)pSample->format, (int)pSample->size, (float)pSample->freq);
#endif //_EXTRA_SOUND_DEBUG

	// Create a buffer for the sample
	pBuffer = &pSample->buffer;
	alGenBuffers(1, pBuffer);
	if ((error=alGetError()) != AL_NO_ERROR) 
	{
		// Couldn't generate a buffer
#ifdef ELC
		LOG_ERROR("%s: %s", snd_buff_error, alGetString(error));
#else
		printf("ensure_sample_loaded ['%s',#%d]: alGenBuffers = %s\n",szPath, index, alGetString(error));
#endif
		*pBuffer = 0;
		return -1;
	}

	// Send this data to the buffer
	alBufferData(*pBuffer, pSample->format, data, pSample->size, pSample->freq);
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef ELC
		LOG_ERROR("%s: %s",snd_buff_error, alGetString(error));
#else
		printf("ensure_sample_loaded ['%s',#d]: alBufferData(%s) = %s\n",szPath, index, alGetString(error));
#endif
		alDeleteBuffers(1, pBuffer);
		return -1;
	}

	alGetBufferi(*pBuffer, AL_BITS, &pSample->bits);
	alGetBufferi(*pBuffer, AL_CHANNELS, &pSample->channels);
	pSample->length = (pSample->size*1000) / ((pSample->bits >> 3)*pSample->channels*pSample->freq);

	// Get rid of the temporary data
#ifdef  ALUT_WAV
 #ifdef	OGG_VORBIS
	// Do a crude check of the extension to see which loader we used
	if (!strcasecmp(filename+(strlen(filename) - 4), ".ogg"))
		free(data);
	else
		alutUnloadWAV(pSample->format, data, pSample->size, pSample->freq);
 #endif // OGG_VORBIS
#else
	free(data);
#endif  //ALUT_WAV

	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error in ensure_sample_loaded: %s, Filename: %s\n", alGetString(error), filename);
#endif //_EXTRA_SOUND_DEBUG
	}
	return sample_num;
}

int load_samples(sound_type * pType)
{
	int i;
	// Check we can load all samples used by this type
	for (i = 0; i < 3; ++i)
	{
		if (pType->part[i].sample_num < 0 && strcasecmp(pType->part[i].file_path, ""))
		{
			pType->part[i].sample_num = ensure_sample_loaded(pType->part[i].file_path);
			if (pType->part[i].sample_num < 0)
			{
#ifdef _EXTRA_SOUND_DEBUG
				printf("Error: problem loading sample: %s\n", pType->part[i].file_path);
#endif //_EXTRA_SOUND_DEBUG
				return 0;
			}
		}
	}
	return 1;
}

unsigned int add_server_sound(int type, int x, int y, int gain)
{
	int snd = -1;
	// Find the sound for this server sound type
	snd = server_sound[type];
	if (snd > -1)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Adding server sound: %d\n", type);
#endif //_EXTRA_SOUND_DEBUG
		return add_sound_object_gain(snd, x, y, 0, gain);
	}
	else
	{
		LOG_ERROR("Unable to find server sound: %i", type);
		return 0;
	}
}

/* Wrapper function for adding walking sounds.
 * It scales the gain before passing it to the normal add_sound_object_gain function
 */
unsigned int add_walking_sound(int type, int x, int y, int me, float scale)
{
//	float gain = 0.0f;
	// Calculate the gain for this scale
//	gain = (scale / 2.0f) + 0.5f;
	return add_sound_object_gain(type, x, y, me, scale);
}

/* Wrapper function for adding particle sounds.
 * It checks for any existing sounds in a similar location before loading this one
 */
unsigned int add_particle_sound(int type, int x, int y)
{
	int i;
	const int buffer = 3;
	// Check if there is another sound within +/-3 tiles around this position
	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		if (x >= sounds_list[i].x - buffer
			&& x <= sounds_list[i].x + buffer
			&& y >= sounds_list[i].y - buffer
			&& y <= sounds_list[i].y + buffer
			&& type == sounds_list[i].sound)
		{
			// There is a sound of this type already within this space so ignore this one
			return 0;
		}
	}
	return add_sound_object_gain(type, x, y, 0, 1.0f);
}

// Add a wrapper for under-the-influence-of-spell sounds
unsigned int add_spell_sound(int spell)
{
	if (spell >= 0 && spell < NUM_ACTIVE_SPELLS && sound_spell_data[spell] > -1)
	{
		// Add the sound
		return add_sound_object_gain(sound_spell_data[spell], 0, 0, 1, 1.0f);
	}
	return 0;
}

unsigned int add_sound_object(int type, int x, int y, int me)
{
	return add_sound_object_gain(type, x, y, me, 1.0f);
}

unsigned int add_sound_object_gain(int type, int x, int y, int me, float initial_gain)
{
	int tx, ty, distanceSq, sound_num, cookie;
	sound_type *pNewType;
	float maxDistanceSq = 0.0f;

/*	Torg: Checks for if sound is enabled etc have been removed as we should load sounds even if currently
	disabled, as they may be enabled within the duration of this sound (eg. rain and map sounds). We just
	won't play them yet.
*/

	// Check if we have a sound config, and thus if its worth doing anything
	if (num_types < 1)
		return 0;
	
	// Get our position
	if (your_actor)
	{
		tx = your_actor->x_pos * 2;
		ty = your_actor->y_pos * 2;
	}
	else
	{
		tx = 0;
		ty = 0;
	}
#ifdef _EXTRA_SOUND_DEBUG
	printf("Trying to add sound: %d (%s) at %d, %d. Position: %d, %d, Gain: %f\n", type, type > -1 ? sound_type_data[type].name : "not defined", x, y, tx, ty, initial_gain);
#endif //_EXTRA_SOUND_DEBUG
	if (type == -1)			// Invalid sound, ignore
		return 0;
	
	if (me)
	{
		// Override the x & y values to use the camera (listener) position because its me
		x = tx;
		y = ty;
	}

	// Check it's a valid type, get pType as a pointer to the type data
	if (type >= MAX_SOUNDS || type >= num_types || type < 0)
	{
#ifdef ELC
		LOG_ERROR("%s: %i", snd_invalid_number, type);
#endif
#ifdef _EXTRA_SOUND_DEBUG
		printf("Apparently an invalid type: %d\n", type);
#endif //_EXTRA_SOUND_DEBUG
		return 0;
	}
	pNewType = &sound_type_data[type];

	// Check if we have a main part. Refuse to play a sound which doesn't have one.
	if (!strcasecmp(pNewType->part[STAGE_MAIN].file_path, ""))
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Sound missing main part!!\n");
#endif //_EXTRA_SOUND_DEBUG
		return 0;
	}

	// Check if this sound is played at this time of day
	if (!time_of_day_valid(pNewType->time_of_the_day_flags))
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Not playing this sound at this time of the day: Flags: 0x%x, Time: %d\n", pNewType->time_of_the_day_flags, game_minute);
#endif //_EXTRA_SOUND_DEBUG
		return 0;
	}
	
	// Check this sound doesn't already exist in the sounds list and just isn't loaded
/*	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		if (sounds_list[i].sound == type && sounds_list[i].x == x && sounds_list[i].y == y && sounds_list)
			return 0;		// This sound already exists so let update_sound handle it
	}
*/	
	// Find a spot in the sounds list for this sound
	sound_num = get_loaded_sound_num();
	if (sound_num == -1)
	{
		// Check if we should bother erroring - an overflow of sounds when sound is disabled we can ignore
		if (have_sound && sound_opts != SOUNDS_NONE)
		{
#ifdef ELC
#ifdef _EXTRA_SOUND_DEBUG
			printf("Error: Too many sounds loaded!! n00b! Not playing this sound: %d (%s)\n", type, pNewType->name);
#endif //_EXTRA_SOUND_DEBUG
			LOG_ERROR("Error: Too many sounds loaded. Not playing this sound: %d (%s)\n", type, pNewType->name);
#endif
		}
		return 0;
	}
	
	// We got this far so add this sound to the list and assign it a cookie
	LOCK_SOUND_LIST();
	
	sounds_list[sound_num].sound = type;
	sounds_list[sound_num].x = x;
	sounds_list[sound_num].y = y;
	sounds_list[sound_num].loaded = 0;
	sounds_list[sound_num].playing = 0;
	sounds_list[sound_num].base_gain = initial_gain;
	sounds_list[sound_num].cur_gain = -1.0f;		// Make sure we set the gain when we first play the sound
	cookie = get_next_cookie();
	sounds_list[sound_num].cookie = cookie;
	
	// Check if we should try to load the samples (sound is enabled)
	if (have_sound && sound_opts != SOUNDS_NONE)
	{
		// Load all samples used by this type
		if (!load_samples(pNewType))
		{
			// Unable to load all the samples. We have already errored so mark it as not loaded and bail.
			sounds_list[sound_num].loaded = 0;
			UNLOCK_SOUND_LIST();
			return cookie;
		}
		sounds_list[sound_num].loaded = 1;
	
		// Check if we are playing this sound now (and need to load it into a source)
		distanceSq = (tx - x) * (tx - x) + (ty - y) * (ty - y);
		maxDistanceSq = pNewType->distance * pNewType->distance;

		if (!pNewType->positional || (distanceSq <= maxDistanceSq))
		{
			if (!play_sound(sound_num, x, y, initial_gain))
			{
#ifdef _EXTRA_SOUND_DEBUG
				printf("There was an error so not playing this sound: %d, Cookie: %d\n", type, cookie);
#endif //_EXTRA_SOUND_DEBUG
			}
		}
#ifdef _EXTRA_SOUND_DEBUG
		else
		{
			printf("Not playing this sound as we are out of range! maxDistanceSq: %d, distanceSq: %d. Loaded as: %d, Cookie: %d\n", (int)maxDistanceSq, distanceSq, sound_num, cookie);
		}
#endif //_EXTRA_SOUND_DEBUG
	}
#ifdef _EXTRA_SOUND_DEBUG
	else
	{
		// Sound isn't enabled so bail now. When sound is enabled, these sounds (if applicable) will be
		// loaded and played then
		printf("Not playing this sound as sound isn't enabled yet. Have sound: %d, Sound opts: %d, Cookie: %d\n", have_sound, sound_opts, cookie);
	}
#endif //_EXTRA_SOUND_DEBUG
	
	// We have added the sound to the list so return the cookie
	UNLOCK_SOUND_LIST();
	return cookie;
}

int play_sound(int sound_num, int x, int y, float initial_gain)
{
	int loops, error;
	ALuint buffer = 0;
	SOUND_STAGE stage;
	ALfloat sourcePos[] = {x, y, 0.0};
	ALfloat sourceVel[] = {0.0, 0.0, 0.0};
	source_data * pSource;
	sound_type * pNewType = &sound_type_data[sounds_list[sound_num].sound];
	
#ifdef _EXTRA_SOUND_DEBUG
	printf("Playing this sound: %d, Sound num: %d, Cookie: %d\n", sounds_list[sound_num].sound, sound_num, sounds_list[sound_num].cookie);
#endif //_EXTRA_SOUND_DEBUG

	// Check if we have a sound device and its worth continuing
	if (!inited)
		return 0;
	
	// Check if we need to load the samples into buffers
	if (!sounds_list[sound_num].loaded)
	{
		if (!load_samples(pNewType))
		{
			return 0;
		}
		sounds_list[sound_num].loaded = 1;
	}
	
	pSource = get_available_source(pNewType->priority);
	if (!pSource)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Sound overflow: %s, %d\n", pNewType->name, max_sources);
#endif //_EXTRA_SOUND_DEBUG
		LOG_ERROR("%s: %d sources allocated", snd_sound_overflow, max_sources);
		return 0;
	}

	// Check if we have an intro. We already quit if the sound doesn't have a main.
	stage = pNewType->part[STAGE_INTRO].sample_num < 0 ? STAGE_MAIN : STAGE_INTRO;

	// Initialise the source data to the first sample to be played for this sound
	pSource->play_duration = 0;
	pSource->loaded_sound = sound_num;
	pSource->current_stage = stage;

	alSourcef(pSource->source, AL_PITCH, 1.0f);
	set_sound_gain(pSource, sound_num, initial_gain);
	alSourcefv(pSource->source, AL_VELOCITY, sourceVel);
	alSourcefv(pSource->source, AL_POSITION, sourcePos);
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error with alSourcef calls: %s. Name: %s. Source: %d: %s\n", snd_source_error, pNewType->name, pSource->source, alGetString(error));
#endif //_EXTRA_SOUND_DEBUG
	}

	for (; stage < num_STAGES; ++stage)
	{
		// Get the buffer to be queued.
		if (pNewType->part[stage].sample_num < 0)
			break;
		buffer = sound_sample_data[pNewType->part[stage].sample_num].buffer;

		// If there are a finite number of loops for main sample, queue them all here
		if (stage == STAGE_MAIN)
		{	
			if (pNewType->loops > 0)
			{
				for (loops = 0; loops < pNewType->loops; ++loops)
				{
					alSourceQueueBuffers(pSource->source, 1, &buffer);
				}
			}
			else
			{
				alSourceQueueBuffers(pSource->source, 1, &buffer);
				// Dont queue an outro that will never get played!
				break;
			}
		}
		else 
		{
			alSourceQueueBuffers(pSource->source, 1, &buffer);
		}
	}
	if ((error=alGetError()) != AL_NO_ERROR) 
	{
#ifdef ELC
 #ifdef _EXTRA_SOUND_DEBUG
		printf("Error with alSourceQueueBuffers: %s. Name: %s. Source: (unknown now): %s", snd_source_error, pNewType->name, alGetString(error));
 #endif //_EXTRA_SOUND_DEBUG
		LOG_ERROR("Error with alSourceQueueBuffers: %s. Name: %s. Source: (unknown now): %s", snd_source_error, pNewType->name, alGetString(error));
#else	
		printf("add_sound_object (%s): alSourceQueueBuffers = %s\n", pNewType->name, alGetString(error));
#endif
		alSourcei(pSource->source, AL_BUFFER, 0);
		pSource->loaded_sound = -1;
		pSource->current_stage = STAGE_UNUSED;
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error queuing buffers: (%s): alSourceQueueBuffers = %s\n", pNewType->name, alGetString(error));
#endif //_EXTRA_SOUND_DEBUG
		return 0;
	}

	if (!pNewType->positional)
	{
		alSourcei(pSource->source, AL_SOURCE_RELATIVE, AL_TRUE);
	}
	else 
	{
		alSourcei(pSource->source, AL_SOURCE_RELATIVE, AL_FALSE);
		alSourcef(pSource->source, AL_REFERENCE_DISTANCE , 10.0f);
		alSourcef(pSource->source, AL_ROLLOFF_FACTOR , 4.0f);
	}
	if (pNewType->part[STAGE_INTRO].sample_num < 0 && pNewType->loops == 0)
		// 0 is infinite looping
		alSourcei(pSource->source,AL_LOOPING,AL_TRUE);
	else
		alSourcei(pSource->source,AL_LOOPING,AL_FALSE);

	alSourcePlay(pSource->source);
	sounds_list[sound_num].playing = 1;

	pSource->cookie = sounds_list[sound_num].cookie;

	return 1;	// Return success
}

unsigned int get_next_cookie()
{
	unsigned int cookie = next_cookie;
	
	// If next_cookie wraps around back to 0 then address this.
	if (++next_cookie == 0) ++next_cookie;
		
	return cookie;
}

int get_loaded_sound_num()
{
	int i;
	// Loop through the array looking for an unused spot (sound = -1 && cookie = 0)
	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		if (sounds_list[i].sound == -1 && sounds_list[i].cookie == 0)
		{
			return i;
		}
	}
	return -1;
}

void reset_buffer_details(int sample_num)
{
	int i, j;
	// Search through the sound types and remove any references to this sample number as it is being
	// overwritten
	for (i = 0; i < num_types; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (sound_type_data[i].part[j].sample_num == sample_num)
				sound_type_data[i].part[j].sample_num = -1;
		}
	}
	// Remove the buffer
	if (alIsBuffer(sound_sample_data[sample_num].buffer))
		alDeleteBuffers(1, &sound_sample_data[sample_num].buffer);
}

// This is passed a cookie, and searches for a sound and source (if ness) with this cookie
void stop_sound(unsigned long int cookie)
{
	int n, m;

	// Cookie of 0 is an invalid sound handle
	if (!have_sound || !cookie)
		return;

	// Find which sound matches this cookie
	n = find_sound_from_cookie(cookie);
#ifdef _EXTRA_SOUND_DEBUG
	printf("Removing cookie: %d, Sound number: %d\n", (int)cookie, n);
#endif //_EXTRA_SOUND_DEBUG
	if (n >= 0)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Stopping cookie %d with sound index %d. It is currently %splaying.\n", (int)cookie, n, sounds_list[n].playing ? "" : "not ");
#endif //_EXTRA_SOUND_DEBUG
		if (sounds_list[n].playing)
		{
			// Find which of our playing sources matches the handle passed
			LOCK_SOUND_LIST();
			m = find_sound_source_from_cookie(cookie);
			if (m >= 0)
			{
				stop_sound_source_at_index(m);
			}
			UNLOCK_SOUND_LIST();
		}
		sounds_list[n].playing = 0;
		// Check if we should unload this sound (is not a map sound)
		if (sound_type_data[sounds_list[n].sound].type != SOUNDS_MAP)
			unload_sound(n);
	}
}

void stop_sound_at_location(int x, int y)
{
	int i = 0;

	// If sound isn't enabled there isn't anything to do
	if (!have_sound || sound_opts == SOUNDS_NONE)
		return;

	// Search for a sound at the given location
	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		if (sounds_list[i].x == x && sounds_list[i].y == y)
		{
			stop_sound(i);
		}
	}
}

// Kill all the sounds. Useful when we change maps, etc.
void stop_all_sounds()
{
	int i;
	ALuint error;

	if (!have_sound)
		return;

#ifdef _EXTRA_SOUND_DEBUG
	printf("Stopping all individual sounds\n");
#endif //_EXTRA_SOUND_DEBUG
	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		if (sounds_list[i].cookie != 0)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Stopping sound %d (%s), cookie: %d, used_sources: %d\n", i, sound_type_data[sounds_list[sound_source_data[0].loaded_sound].sound].name, sounds_list[i].cookie, used_sources);
#endif //_EXTRA_SOUND_DEBUG
			stop_sound(sounds_list[i].cookie);
			// Force the unloading of all sounds (assume changing maps)
			unload_sound(i);
		}
	}

#ifdef OGG_VORBIS
	for (i = 0; i < max_streams; i++)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Stopping %s stream source: %d\n", get_stream_type(streams[i].type), streams[i].source);
#endif //_EXTRA_SOUND_DEBUG
		// Stop the music stream (we can use it again after the map change) but destroy any other streams
		if (streams[i].type == STREAM_TYPE_MUSIC)
			stop_stream(&streams[i]);
		else
			destroy_stream(&streams[i]);
	}
#endif // OGG_VORBIS

	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error killing all sounds\n");
#endif //_EXTRA_SOUND_DEBUG
	}
}

void unload_sound(int index)
{
	// Reset this sound
	sounds_list[index].sound = -1;
	sounds_list[index].x = -1;
	sounds_list[index].y = -1;
	sounds_list[index].playing = 0;
	sounds_list[index].loaded = 0;
	sounds_list[index].cookie = 0;
	num_sounds--;

	// FIXME: Should we unload the buffer as well??
}

// -- Update the sound system --
// We update our listener position and any sounds being played, as well as
// rebuilding the list of active sources, as some may have become inactive
// due to the sound ending or being out of range, or some may be back in
// range and need to be played. We also adjust volumes if ness.
void update_sound(int ms)
{
	int i = 0, error = AL_NO_ERROR;
	source_data *pSource;
	sound_sample *pSample;
	sound_type *pSoundType;
	ALuint deadBuffer;
	ALint numProcessed, buffer, state;

	int source;
	int x, y, distanceSq, maxDistSq;
	int relative;
	int tx = 0, ty = 0;
	ALfloat sourcePos[3] = {0.0f, 0.0f, 0.0f};
	ALfloat listenerPos[3] = {0.0f, 0.0f, 0.0f};
	ALfloat listenerVel[3] = {0.0f, 0.0f, 0.0f};
	ALfloat listenerOri[6] = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
#ifdef _EXTRA_SOUND_DEBUG
	int j, k, l;
#endif // _EXTRA_SOUND_DEBUG

	// Check if we have a sound config, and thus if its worth doing anything
	if (num_types < 1)
		return;

	// Check if we have our actor
	if (your_actor)
	{
		// Set our position and the listener variables
		tx = your_actor->x_pos * 2;
		ty = your_actor->y_pos * 2;
		listenerPos[0] = tx;
		listenerPos[1] = ty;
		if (your_actor->z_rot > 0 && your_actor->z_rot < 180) {
			listenerOri[0] = 1;
		} else if (your_actor->z_rot > 180 && your_actor->z_rot < 360) {
			listenerOri[0] = -1;
		} else {
			listenerOri[0] = 0;
		}
		if (your_actor->z_rot > 315 || your_actor->z_rot < 90) {
			listenerOri[1] = 1;
		} else if (your_actor->z_rot > 135 && your_actor->z_rot < 225) {
			listenerOri[1] = -1;
		} else {
			listenerOri[1] = 0;
		}
	}
	
	// Start to process the sounds
	LOCK_SOUND_LIST();

	// Check the sounds_list for anything to be loaded/played
	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		// Check for any non-looping sounds in the sounds_list that aren't being played that have passed their time
		// FIXME!!! (this needs to be coded)
		
		// Check for any sounds that aren't being played and check if they need to be because
		// sound has now been enabled or they have come back into range
		x = sounds_list[i].x;
		y = sounds_list[i].y;
		if (!sounds_list[i].playing && x > -1 && y > -1 && sounds_list[i].sound > -1)
		{
			pSoundType = &sound_type_data[sounds_list[i].sound];
			distanceSq = (tx - x) * (tx - x) + (ty - y) * (ty - y);
			maxDistSq = pSoundType->distance * pSoundType->distance;
			if (sound_opts != SOUNDS_NONE && (distanceSq < maxDistSq))
			{
				// This sound is back in range so load it into a source and play it
#ifdef _EXTRA_SOUND_DEBUG
				printf("Sound now in-range: %d (%s), Distance squared: %d, Max: %d\n", sounds_list[i].sound, pSoundType->name, distanceSq, maxDistSq);
#endif //_EXTRA_SOUND_DEBUG
				if (!play_sound(i, x, y, 1.0f))
				{
#ifdef _EXTRA_SOUND_DEBUG
					printf("Error restarting sound!!\n");
#endif //_EXTRA_SOUND_DEBUG
				}
			}
		}
	}
	
	// Check if we have a sound device and its worth continuing
	if (!inited)
	{
		UNLOCK_SOUND_LIST();
		return;
	}

	// Update the position of the listener
	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOri);

	// Check if we have any sounds playing currently (this includes streams so is sometimes a little redundant)
	if (!used_sources)
	{
		UNLOCK_SOUND_LIST();
		return;
	}

#ifdef ELC
#ifdef _EXTRA_SOUND_DEBUG
	j = 0;
#endif // _EXTRA_SOUND_DEBUG
	// Now, update the position of actor (animation) sounds
	for (i = 0; i < max_actors; i++)
	{
#ifdef _EXTRA_SOUND_DEBUG
		j++;
		if (j > MAX_ACTORS)
		{
			LOG_ERROR("update_sound race condition!! i = %d, max_actors = %d\n", i, max_actors);
			printf("update_sound race condition!! i = %d, max_actors = %d\n", i, max_actors);
			break;
		}
#endif // _EXTRA_SOUND_DEBUG
		if (!actors_list[i] || !actors_list[i]->cur_anim_sound_cookie)
			continue;
		
		source = find_sound_source_from_cookie(actors_list[i]->cur_anim_sound_cookie);
		if (source < 0)
			continue;
		
		// Check if this is the correct tile type, or if we need to play another sound
		if (actors_list[i]->moving && !actors_list[i]->fighting)
		{
			UNLOCK_SOUND_LIST();
			handle_walking_sound(actors_list[i], actors_list[i]->cur_anim.sound);
			LOCK_SOUND_LIST();
		}
		if (actors_list[i]->actor_id == yourself)
		{
			// If this is you, use the same position as the listener
			sourcePos[0] = tx;
			sourcePos[1] = ty;
		}
		else
		{
			sourcePos[0] = actors_list[i]->x_pos * 2;
			sourcePos[1] = actors_list[i]->y_pos * 2;
		}
		sourcePos[2] = 0.0f;
		alSourcefv(sound_source_data[source].source, AL_POSITION, sourcePos);
	}
#endif //ELC
	
	// Finally, update all the sources
	i = 0;
#ifdef _EXTRA_SOUND_DEBUG
	j = 0;
#endif // _EXTRA_SOUND_DEBUG
	pSource = sound_source_data;
	while (i < used_sources)
	{
#ifdef _EXTRA_SOUND_DEBUG
		j++;
		if (j > max_sources)
		{
			LOG_ERROR("update_sound race condition!! i = %d, used_sources = %d\n", i, used_sources);
			printf("update_sound race condition!! i = %d, used_sources = %d\n", i, used_sources);
			break;
		}
#endif // _EXTRA_SOUND_DEBUG
		
		// Check if this is a source we should ignore (streams)
		if (pSource->current_stage == STAGE_STREAM)
		{
			pSource->play_duration += ms;
			++i;
			++pSource;
			continue;
		}
		
		// Update the gain for this source if nessessary
		set_sound_gain(pSource, pSource->loaded_sound, sounds_list[pSource->loaded_sound].base_gain);
		
		// This test should be redundant
		if (pSource->loaded_sound < 0 || sounds_list[pSource->loaded_sound].sound < 0 || pSource->current_stage == STAGE_UNUSED)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Removing dud sound %d. Cookie: %d, Source: %d. Current stage: %d\n", pSource->loaded_sound, sounds_list[pSource->loaded_sound].cookie, i, pSource->current_stage);
#endif //_EXTRA_SOUND_DEBUG
			unload_sound(pSource->loaded_sound);
			stop_sound_source_at_index(i);
			continue;
		}
		pSoundType = &sound_type_data[sounds_list[pSource->loaded_sound].sound];
		pSample = &sound_sample_data[pSoundType->part[pSource->current_stage].sample_num];

		// Is this source still playing?
		alGetSourcei(pSource->source, AL_SOURCE_STATE, &state);
		if (state == AL_STOPPED)
		{
#ifdef _EXTRA_SOUND_DEBUG
//			printf("'%s' has stopped after sample '%s'\n", pSoundType->name, pSample->file_path);
#endif //_EXTRA_SOUND_DEBUG
			pSource->current_stage = num_STAGES;
		}
		else
		{
			// Find which buffer is playing
			alGetSourcei(pSource->source, AL_BUFFER, &buffer);
			if (buffer != pSample->buffer)
			{
				// The source has moved on to the next queued sample
#ifdef _EXTRA_SOUND_DEBUG
				printf("Cookie: %d, Loaded sound: %d, sound: %d, '%s' - sample '%s' has ended...", pSource->cookie, pSource->loaded_sound, sounds_list[pSource->loaded_sound].sound, pSoundType->name, pSoundType->part[pSource->current_stage].file_path);
				k = 0;
#endif //_EXTRA_SOUND_DEBUG
				while (++pSource->current_stage != num_STAGES)
				{
#ifdef _EXTRA_SOUND_DEBUG
					k++;
					if (k > num_STAGES)
					{
						LOG_ERROR("update_sound race condition!! cur stage = %d, num_stages = %d\n", pSource->current_stage, num_STAGES);
						printf("update_sound race condition!! cur stage = %d, num_stages = %d\n", pSource->current_stage, num_STAGES);
						break;
					}
#endif // _EXTRA_SOUND_DEBUG
					if (pSoundType->part[pSource->current_stage].sample_num < 0)
					{
						// No more samples to play
#ifdef _EXTRA_SOUND_DEBUG
						printf("no more samples for this type!\n");
#endif //_EXTRA_SOUND_DEBUG
						pSource->current_stage = num_STAGES;
						break;
					}
					pSample = &sound_sample_data[pSoundType->part[pSource->current_stage].sample_num];
					// Found the currently-playing buffer

					if (pSample->buffer == buffer)
					{
#ifdef _EXTRA_SOUND_DEBUG
						printf("next sample is '%s'\n", pSoundType->part[pSource->current_stage].file_path);
#endif //_EXTRA_SOUND_DEBUG
						if (pSource->current_stage == STAGE_MAIN && pSoundType->loops == 0)
						{
							// We've progressed to the main sample which loops infinitely.
#ifdef _EXTRA_SOUND_DEBUG
							l = 0;
#endif // _EXTRA_SOUND_DEBUG
							do
							{
#ifdef _EXTRA_SOUND_DEBUG
								l++;
								if (l > 10)
								{
									LOG_ERROR("update_sound race condition!! cur buffer = %d, k = %d\n", numProcessed, l);
									printf("update_sound race condition!! cur buffer = %d, k = %d\n", numProcessed, l);
									break;
								}
#endif // _EXTRA_SOUND_DEBUG
								// We only unqueue buffers explicitly here so that there's only the
								// MAIN sample queued for the looping. Normally we just set a zero
								// buffer to the source which automatically unqueues any buffers
								alGetSourcei(pSource->source, AL_BUFFERS_PROCESSED, &numProcessed);
								if (numProcessed-- > 0)
									alSourceUnqueueBuffers(pSource->source, 1, &deadBuffer);
							}
							while (numProcessed > 0);

							alSourcei(pSource->source, AL_LOOPING, AL_TRUE);
							if ((error=alGetError()) != AL_NO_ERROR)
							{
							#ifdef ELC
								LOG_ERROR("%s: %s", snd_buff_error, alGetString(error));
							#else
								printf("update_sound : alSourcei(pSource->source,AL_LOOPING,AL_TRUE) = %s\n",
									alGetString(error));
							#endif
							}
						}
						break;
					}
				}
			}
		}

		// Check if we need to remove this sound (its finished, or not playing that type anymore)
		// If the state is num_STAGES then the sound has ended (or gone wrong)
		if(pSource->current_stage == num_STAGES || pSoundType->type	> sound_opts)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Removing finished sound %d (%s) at cookie %d, source index %d, loaded_sound: %d\n", sounds_list[pSource->loaded_sound].sound, pSoundType->name, pSource->cookie, i, pSource->loaded_sound);
#endif //_EXTRA_SOUND_DEBUG
			unload_sound(pSource->loaded_sound);
			stop_sound_source_at_index(i);
			continue;
		}

		// Check if this source is out of range, or back in range (an error!)
		alGetSourcei(pSource->source, AL_SOURCE_RELATIVE, &relative);
		if (relative != AL_TRUE)
		{
			alGetSourcei(pSource->source, AL_SOURCE_STATE, &state);
			alGetSourcefv(pSource->source, AL_POSITION, sourcePos);
			x = sourcePos[0]; y = sourcePos[1];
			distanceSq = (tx - x) * (tx - x) + (ty - y) * (ty - y);
			maxDistSq = pSoundType->distance * pSoundType->distance;

			if ((state == AL_PLAYING) && (distanceSq > maxDistSq))
			{
#ifdef _EXTRA_SOUND_DEBUG
				printf("Pausing sound: %d (%s), Distance squared: %d, Max: %d\n", i, pSoundType->name, distanceSq, maxDistSq);
#endif //_EXTRA_SOUND_DEBUG
				// Free up this source
				stop_sound_source_at_index(i);
				sounds_list[pSource->loaded_sound].playing = 0;
			}
			else if (sound_opts != SOUNDS_NONE && (state == AL_PAUSED) && (distanceSq < maxDistSq))
			{
				LOG_ERROR("Sound error: We found a wasted source. Sound %d (%s) was loaded into a source and paused!!\n", i, pSoundType->name);
				alSourcePlay(pSource->source);
				sounds_list[pSource->loaded_sound].playing = 1;
			}
			if ((error=alGetError()) != AL_NO_ERROR) 
		    {
			#ifdef ELC
				LOG_ERROR("update_sound %s: %s", my_tolower(reg_error_str), alGetString(error));
			#endif
		    }
		}
		pSource->play_duration += ms;
		++i;
		++pSource;
	}

	UNLOCK_SOUND_LIST();
	
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error updating sound: %s\n", alGetString(error));
#endif //_EXTRA_SOUND_DEBUG
	}
}


/*********************
 * GENERAL FUNCTIONS *
 *********************/


void handle_walking_sound(actor * pActor, int def_snd)
{
	int snd, tile_type, cur_sound;
	
	if (actors_defs[pActor->actor_type].walk_snd_scale > 0.0f)
	{
		// This creature is large enough for a walking sound so look for one for this tile
		tile_type = get_tile_type((int)pActor->x_pos * 2, (int)pActor->y_pos * 2);
		snd = get_tile_sound(tile_type, actors_defs[pActor->actor_type].actor_name);
#ifdef _EXTRA_SOUND_DEBUG
//		printf("Actor: %s, Pos: %f, %f, Current tile type: %d, Sound: %d, Scale: %f\n", pActor->actor_name, pActor->x_pos, pActor->y_pos, tile_type, snd, actors_defs[pActor->actor_type].walk_snd_scale);
#endif // _EXTRA_SOUND_DEBUG

		if (snd == -1)
			// No sound for this tile, fall back on the passed default (from the animation)
			snd = def_snd;
		if (snd == -1)
			// Still no sound, so fall back on the global default
			snd = walking_default;

		// Check for something to do
		if (snd > -1)
		{			
			// Check if we have a sound and it is different to the current one
			cur_sound = find_sound_from_cookie(pActor->cur_anim_sound_cookie);
			if (cur_sound >= 0 && sounds_list[cur_sound].sound != snd)
			{
				// It is valid and different so remove the current sound before we add the new one
				stop_sound(pActor->cur_anim_sound_cookie);
			}
			if (cur_sound < 0 || sounds_list[cur_sound].sound != snd)
			{
				// Add the new sound
				pActor->cur_anim_sound_cookie = add_walking_sound(	snd,
																	2*pActor->x_pos,
																	2*pActor->y_pos,
																	pActor->actor_id == yourself ? 1 : 0,
																	actors_defs[pActor->actor_type].walk_snd_scale
																);
			}
		}
	}
}

int get_tile_sound(int tile_type, char * actor_type)
{
	int i, j, k;
	
	// Check for unknown/invalid tile type
	if (tile_type == -1)
		return -1;
	
	for (i = 0; i < MAX_SOUND_TILE_TYPES; i++)
	{
		for (j = 0; j < sound_tile_data[i].num_tile_types; j++)
		{
			if (sound_tile_data[i].tile_type[j] == tile_type)
			{
				// Found a matching tile type so find the actor type
				for (k = 0; k < sound_tile_data[i].num_sounds; k++)
				{
					if (get_string_occurance(actor_type, sound_tile_data[i].sounds[k].actor_types, strlen(actor_type), 0) > -1)
					{
						// Return the sound
						return sound_tile_data[i].sounds[k].sound;
					}
				}
				// Didn't find a sound, so return a default if it exists
				return sound_tile_data[i].default_sound;
			}
		}
	}
	// If we got here, we don't have a sound for this tile type
	return -1;
}

void setup_map_sounds (int map_num)
{
	int i;
#ifdef _EXTRA_SOUND_DEBUG
	char str[50];
	safe_snprintf(str, sizeof(str), "Map number: %d", map_num);
	LOG_TO_CONSOLE(c_red1, str);
#endif // _EXTRA_SOUND_DEBUG
	// Find the index for this map in our data
	snd_cur_map = -1;
	for (i = 0; i < sound_num_maps; i++)
	{
		if (map_num == sound_map_data[i].id)
		{
			snd_cur_map = i;
			return;
		}
	}
}

void set_sound_gain(source_data * pSource, int loaded_sound_num, float new_gain)
{
	float type_gain = 1.0f;
	int error;
	sound_type * this_snd = &sound_type_data[sounds_list[loaded_sound_num].sound];
	// Check what type this sound is and match it to the "type gain"
	switch (this_snd->type)
	{
		case SOUNDS_CROWD:
			type_gain = crowd_gain;
			break;
		case SOUNDS_CLIENT:
			type_gain = client_gain;
			break;
		case SOUNDS_WALKING:
			type_gain = walking_gain;
			break;
		case SOUNDS_ACTOR:
			type_gain = actor_gain;
			break;
		case SOUNDS_MAP:
			type_gain = enviro_gain;
			break;
		case SOUNDS_ENVIRO:
			type_gain = enviro_gain;
			break;
		case SOUNDS_GAMEWIN:
			type_gain = gamewin_gain;
			break;
	}
	// Check if we need to update the base gain for this sound
	if (new_gain != sounds_list[loaded_sound_num].base_gain)
		sounds_list[loaded_sound_num].base_gain = new_gain;

	// Check if we need to dim down the sounds due to rain
	if (this_snd->type != SOUNDS_CLIENT && this_snd->type != SOUNDS_GAMEWIN)
		new_gain = weather_adjust_gain(new_gain, pSource->cookie);

	// Check if we need to update the overall gain for this source
	if (sound_gain * type_gain * this_snd->gain * new_gain != sounds_list[loaded_sound_num].cur_gain)
	{
		sounds_list[loaded_sound_num].cur_gain = sound_gain * type_gain * this_snd->gain * new_gain;
		alSourcef(pSource->source, AL_GAIN, sounds_list[loaded_sound_num].cur_gain);
	}
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error setting sound gain: %f, sound: %d, error: %s\n", new_gain, loaded_sound_num, alGetString(error));
#endif //_EXTRA_SOUND_DEBUG
	}
	return;
}

void sound_source_set_gain(unsigned long int cookie, float gain)
{
	int n;
	source_data *pSource;

	// Source handle of 0 is a null source
	if (!have_sound || !cookie)
		return;
	// Find which of our playing sources matches the handle passed
	n = find_sound_source_from_cookie(cookie);
	if (n > 0)
	{
		pSource = &sound_source_data[n];
		set_sound_gain(pSource, pSource->loaded_sound, gain);
	}
	return;
}

// Find the index of the sound associated with this cookie.
int find_sound_from_cookie(unsigned int cookie)
{
	int n;

	if (!cookie || cookie == 0)
		return -1;

	for (n = 0; n < MAX_BUFFERS * 2; n++)
	{
		if (sounds_list[n].cookie == cookie)
			return n;
	}

	return -1;
}

// Find the index of the source associated with this cookie.
// Note that this result must not be stored, but used immediately;
int find_sound_source_from_cookie(unsigned int cookie)
{
	int n;
	source_data *pSource = sound_source_data;

	if (!cookie)
		return -1;

	for (n = 0, pSource = sound_source_data; n < used_sources; ++n, ++pSource)
	{
		if (pSource->cookie == cookie)
			return n;
	}

	return -1;
}

int get_index_for_sound_type_name(const char *name)
{
	int i;
	for(i = 0; i < num_types; ++i)
	{
		if (strcasecmp(sound_type_data[i].name, name) == 0)
			return i;
	}
	return -1;
}

// Look for a particle sound def matching the input filename
// (minus the directory ./particles/ and extension .part)
int get_sound_index_for_particle_file_name(const char *name)
{
	char my_name[MAX_FILENAME_LENGTH];
	int i;
	safe_strncpy(my_name, name+12, sizeof(my_name));
	my_name[strlen(my_name) - 5] = '\0';
	for(i = 0; i < num_types; ++i)
	{
		if (strcasecmp(sound_particle_data[i].file, my_name) == 0)
			return sound_particle_data[i].sound;
	}
	return -1;
}

// Looks for a sound effect def matching the input special effect
int get_sound_index_for_sfx(int sfx)
{
	int i;
	for(i = 0; i < num_types; ++i)
	{
		if (sound_effect_data[i].id == sfx)
			return sound_effect_data[i].sound;
	}
	return -1;
}

int get_index_for_inv_usewith_item_sound(int use_image_id, int with_image_id)
{
	int i;
	char name[MAX_SOUND_NAME_LENGTH];
	
#ifdef _EXTRA_SOUND_DEBUG
	printf("Searching for the sound for: %d on %d\n", use_image_id, with_image_id);
#endif //_EXTRA_SOUND_DEBUG
	snprintf(name, sizeof(name), "%d on %d", use_image_id, with_image_id);
	for (i = 0; i < num_types; ++i)
	{
		if (strcasecmp(sound_type_data[i].name, name) == 0)
			return i;
	}
	return -1;
}

int get_index_for_inv_use_item_sound(int image_id)
{
	int i, j;
#ifdef _EXTRA_SOUND_DEBUG
	printf("Searching for the sound for image ID: %d\n", image_id);
#endif //_EXTRA_SOUND_DEBUG
	for (i = 0; i < sound_num_items; ++i)
	{
		for (j = 0; j < sound_item_data[i].num_imageids; j++)
		{
			if (sound_item_data[i].image_id[j] == image_id)
				return sound_item_data[i].sound;
		}
	}
	return -1;
}

// Compare the input flags to the current time and return true if they match
int time_of_day_valid(int flags)
{
	return flags & (1 << (game_minute / 30));
}

/* sound_bounds_check
 *
 * Check if input point (x, y) is within the input boundary
 * 
 * We will do this by checking the angle of the line created between each of the 4 points of the boundaries and
 * comparing that angle to the line created by each point and our test point.
 *
 * With the equation we use, if the line aligns to an axis, arctan has no value, so we need to check for this
 * and use 90 degrees rather than calculate it
 *
 *
 * FIEME: Currently, the checks don't include one for that of a point of the boundary inside the outer bounds of
 * the polygon
 */
int sound_bounds_check(int x, int y, map_sound_boundary_def bounds)
{
	double a1, a2, b1, b2, c1, c2, d1, d2;
	double pi = 3.1415;
	double ra = pi / 2;		// ra = Right angle... meh

	// Check the angle of the line from the bottom left corner to the top left (point1 -> point2)
	if (bounds.x1 == bounds.x2) a1 = ra;
	else a1 = atan((bounds.y2 - bounds.y1) / (bounds.x2 - bounds.x1));
	if (bounds.x2 < bounds.x1) a1 += pi;
	// Check the angle of the line from the bottom left corner to our test point (point1 -> pointT)
	if (bounds.x1 == x) a2 = ra;
	else a2 = atan((y - bounds.y1) / (x - bounds.x1));
	if (x < bounds.x1) a2 += pi;
	// If our angle for the test point is less than the angle of the boundary line, then the point is outside
	if (a1 <= a2)
	{
		return 0;
	}

	
	// Check the angle of the line from the top left corner to the top right (point2 -> point3)
	if (bounds.y2 == bounds.y3) b1 = ra;
	else b1 = atan((bounds.x3 - bounds.x2) / (bounds.y2 - bounds.y3));
	if (bounds.y3 > bounds.y2) b1 += pi;
	// Check the angle of the line from the top left corner to our test point (point2 ->pointT)
	if (bounds.y2 == y) b2 = ra;
	else b2 = atan((x - bounds.x2) / (bounds.y2 - y));
	if (y > bounds.y2) b2 += pi;
	// If our angle for the test point is less than the angle of the boundary line, then the point is outside
	if (b1 <= b2)
	{
		return 0;
	}


	// Check the angle of the line from the top right corner to the bottom right (point3 -> point4)
	if (bounds.x3 == bounds.x4) c1 = ra;
	else c1 = atan((bounds.y3 - bounds.y4) / (bounds.x3 - bounds.x4));
	if (bounds.x4 > bounds.x3) c1 += pi;
	// Check the angle of the line from the top right corner to our test point (point3 -> pointT)
	if (bounds.x3 == x) c2 = ra;
	else c2 = atan((bounds.y3 - y) / (bounds.x3 - x));
	if (x > bounds.x3) c2 += pi;
	// If our angle for the test point is less than the angle of the boundary line, then the point is outside
	if (c1 <= c2)
	{
		return 0;
	}


	// Check the angle of the line from the bottom right corner to the bottom left (point4 -> point1)
	if (bounds.y4 == bounds.y1) d1 = ra;
	else d1 = atan((bounds.x4 - bounds.x1) / (bounds.y1 - bounds.y4));
	if (bounds.y1 < bounds.y4) d1 += pi;
	// Check the angle of the line from the bottom right corner to our test point (point4 -> pointT)
	if (bounds.y4 == y) d2 = ra;
	else d2 = atan((bounds.x4 - x) / (y - bounds.y4));
	if (y < bounds.y4) d2 += pi;
	// If our angle for the test point is less than the angle of the boundary line, then the point is outside
	if (d1 <= d2)
	{
		return 0;
	}

	// This point is inside the 4 lines
	return 1;
}

/******************
 * INIT FUNCTIONS *
 ******************/

void clear_sound_data()
{
	int i, j;
	
	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		sounds_list[i].sound = -1;
		sounds_list[i].x = -1;
		sounds_list[i].y = -1;
		sounds_list[i].cookie = 0;
		sounds_list[i].loaded = 0;
		sounds_list[i].playing = 0;
		sounds_list[i].base_gain = 0.0f;
		sounds_list[i].cur_gain = 0.0f;
	}
	for (i = 0; i < MAX_STREAMS; i++)
	{
		destroy_stream(&streams[i]);
	}
	for (i = 0; i < MAX_SOUNDS; i++)
	{
		sound_type_data[i].name[0] = '\0';
		for (j = 0; j < num_STAGES; j++)
		{
			sound_type_data[i].part[j].file_path[0] = '\0';
			sound_type_data[i].part[j].loaded_status = 0;
			sound_type_data[i].part[j].sample_num = -1;
		}
		sound_type_data[i].gain = 1.0f;
		sound_type_data[i].stereo = 0;
		sound_type_data[i].distance = 100.0f;
		sound_type_data[i].positional = 1;
		sound_type_data[i].loops = 1;
		sound_type_data[i].fadeout_time = 0;
		sound_type_data[i].echo_delay = 0;
		sound_type_data[i].echo_volume = 50;
		sound_type_data[i].time_of_the_day_flags = 0xffff;
		sound_type_data[i].priority = 5;
		sound_type_data[i].type = SOUNDS_ENVIRO;
	}
	for (i = 0; i < MAX_BUFFERS; i++)
	{
		if (alIsBuffer(sound_sample_data[i].buffer))
			alDeleteBuffers(1, &sound_sample_data[i].buffer);
		sound_sample_data[i].buffer = 0;
		sound_sample_data[i].format = 0;
		sound_sample_data[i].size = 0;
		sound_sample_data[i].freq = 0;
		sound_sample_data[i].channels = 0;
		sound_sample_data[i].bits = 0;
		sound_sample_data[i].length = 0;
	}
	for (i = 0; i < MAX_BACKGROUND_DEFAULTS; i++)
	{
		sound_background_defaults[i].time_of_day_flags = 0xffff;
		sound_background_defaults[i].map_type = 0;
		sound_background_defaults[i].sound = -1;
	}
	crowd_default = -1;
	walking_default = -1;
	for (i = 0; i < MAX_SOUND_MAPS; i++)
	{
		sound_map_data[i].id = 0;
		sound_map_data[i].name[0] = '\0';
		sound_map_data[i].num_boundaries = 0;
		for (j = 0; j < MAX_SOUND_MAP_BOUNDARIES; j++)
		{
			sound_map_data[i].boundaries[j].bg_sound = -1;
			sound_map_data[i].boundaries[j].crowd_sound = -1;
			sound_map_data[i].boundaries[j].time_of_day_flags = 0xffff;
			sound_map_data[i].boundaries[j].is_default = 0;
			sound_map_data[i].boundaries[j].x1 = 0;
			sound_map_data[i].boundaries[j].y1 = 0;
			sound_map_data[i].boundaries[j].x2 = 0;
			sound_map_data[i].boundaries[j].y2 = 0;
			sound_map_data[i].boundaries[j].x3 = 0;
			sound_map_data[i].boundaries[j].y3 = 0;
			sound_map_data[i].boundaries[j].x4 = 0;
			sound_map_data[i].boundaries[j].y4 = 0;
		}
		sound_map_data[i].num_defaults = 0;
		for (j = 0; j < MAX_MAP_BACKGROUND_DEFAULTS; j++)
		{
			sound_map_data[i].defaults[j] = 0;
		}
	}
	for (i = 0; i < MAX_SOUND_EFFECTS; i++)
	{
		sound_effect_data[i].id = 0;
		sound_effect_data[i].sound = -1;
	}
	for (i = 0; i < MAX_SOUND_PARTICLES; i++)
	{
		sound_particle_data[i].file[0] = '\0';
		sound_particle_data[i].sound = -1;
	}
	for (i = 0; i < MAX_SOUND_ITEMS; i++)
	{
		for (j = 0; j < MAX_ITEM_SOUND_IMAGE_IDS; j++)
		{
			sound_item_data[i].image_id[j] = -1;
		}
		sound_item_data[i].num_imageids = 0;
		sound_item_data[i].sound = -1;
	}
	for (i = 0; i < MAX_SOUND_TILE_TYPES; i++)
	{
		for (j = 0; j < MAX_SOUND_TILES; j++)
		{
			sound_tile_data[i].tile_type[j] = -1;
		}
		sound_tile_data[i].num_tile_types = 0;
		for (j = 0; j < MAX_SOUND_TILES_SOUNDS; j++)
		{
			sound_tile_data[i].sounds[j].actor_types[0] = '\0';
			sound_tile_data[i].sounds[j].sound = -1;
		}
		sound_tile_data[i].num_sounds = 0;
		sound_tile_data[i].default_sound = -1;
	}
	for (i = 0; i < 9; i++)
	{
		server_sound[i] = -1;
	}
	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		sound_spell_data[i] = -1;
	}

	num_types = 0;
	num_samples = 0;
	sound_num_background_defaults = 0;
	sound_num_maps = 0;
	sound_num_effects = 0;
	sound_num_particles = 0;
	sound_num_items = 0;
	sound_num_tile_types = 0;
}

void init_sound()
{
	ALCcontext *context;
	ALCdevice *device;
	ALfloat listenerPos[3] = {0.0f, 0.0f, 0.0f};
	ALfloat listenerVel[3] = {0.0f, 0.0f, 0.0f};
	ALfloat listenerOri[6] = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	int error;
	int i;
	
	// If we don't have sound/music then bail so we don't grab the soundcard.
	if (inited || (sound_opts == SOUNDS_NONE && music_on == 0))
		return;
		
#ifndef OSX
	alutInitWithoutContext(0, 0);
#endif // OSX

	// Begin by setting all data to a known state
	if (have_sound)
		destroy_sound();

	// Initialise OpenAL

	// Get a list of the available devices (not used yet)
	if ( alcIsExtensionPresent( NULL, "ALC_ENUMERATION_EXT" ) == AL_TRUE )
	{
		safe_strncpy(sound_devices, alcGetString( NULL, ALC_DEVICE_SPECIFIER), sizeof(sound_devices));
	}
	else
	{
		LOG_ERROR("Warning: ALC_ENUMERATION_EXT not found. Unable to retrieve list of sound devices.");
	}

	// If you want to use a specific device, use, for example:
	// device = alcOpenDevice((ALubyte*) "DirectSound3D")
	// NULL makes it use the default device
	device = alcOpenDevice( NULL );
	if ((error = alcGetError(device)) != AL_NO_ERROR || !device)
	{
		char str[256];
		safe_snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alcGetString(device, error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound = have_music = 0;
		return;
	}

	context = alcCreateContext( device, NULL );
	alcMakeContextCurrent( context );
	if ((error = alcGetError(device)) != AL_NO_ERROR || !context)
	{
		char str[256];
		safe_snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alcGetString(device, error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound = have_music = 0;
		return;
	}
	
	sound_list_mutex = SDL_CreateMutex();
	if (!sound_list_mutex)
	{
		char str[256];
		safe_snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, "Unable to create sound list mutex");
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound = have_music = 0;
		return;
	}

#if defined DEBUG && !defined ALUT_WAV
	// Dump the capabilities of this version of Alut
	printf("Alut supported Buffer MIME types: %s\n", alutGetMIMETypes(ALUT_LOADER_BUFFER));
	printf("Alut supported Memory MIME types: %s\n", alutGetMIMETypes(ALUT_LOADER_MEMORY));
#endif // DEBUG && !ALUT_WAV
	
	have_sound = 1;
#ifdef	OGG_VORBIS
	have_music = 1;
#else	// OGG_VORBIS
	have_music = 0;
#endif	// OGG_VORBIS

	// Setup the listener
	alListenerfv(AL_POSITION, listenerPos);
#ifdef _EXTRA_SOUND_DEBUG							// Debugging for Florian
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		printf("%s: Error setting up listener position - %s\n", snd_init_error, alGetString(error));
	}
#endif // _EXTRA_SOUND_DEBUG
	alListenerfv(AL_VELOCITY, listenerVel);
#ifdef _EXTRA_SOUND_DEBUG
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		printf("%s: Error setting up listener velocity - %s\n", snd_init_error, alGetString(error));
	}
#endif // _EXTRA_SOUND_DEBUG
	alListenerfv(AL_ORIENTATION, listenerOri);
#ifdef _EXTRA_SOUND_DEBUG
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		printf("%s: Error setting up listener orientation - %s\n", snd_init_error, alGetString(error));
	}
#endif // _EXTRA_SOUND_DEBUG

	if ((error = alGetError()) != AL_NO_ERROR)
	{
		char str[256];
		safe_snprintf(str, sizeof(str), "%s: Error setting up listener - %s\n", snd_init_error, alGetString(error));
		LOG_ERROR(str);
	}

	// Start with our max and see how many sources we can allocate
	max_sources = ABS_MAX_SOURCES;
	
	// Initialise sources
	LOCK_SOUND_LIST();
	
	// Init source data
	for (i = 0; i < max_sources; i++)
	{
		sound_source_data[i].source = 0;
		sound_source_data[i].loaded_sound = -1;
		sound_source_data[i].play_duration = 0;
		sound_source_data[i].current_stage = STAGE_UNUSED;
		sound_source_data[i].cookie = 0;
	}
	
	// Generate our sources
	for (i = 0; i < max_sources; i++)
	{
		alGenSources(1, &sound_source_data[i].source);
		if ((error = alGetError()) != AL_NO_ERROR) 
		{
			// Assume this is the limit of sources available
			max_sources = i;
			// Limit the available streams based on the number of sources (a stream takes twice the resources)
			if (max_sources <= 16)
				max_streams = 4;		// This only allows a music stream, a crowd stream and 2 backgrounds...
										// but it takes effectively 8 sources of <= 16
			if (max_sources == 0)
			{
				// We don't have any sources so error and disable sound
				LOG_ERROR("%s: %s - %s", snd_init_error, snd_source_error, alGetString(error));
				have_sound = have_music = 0;
				UNLOCK_SOUND_LIST();
				return;
			}
			break;
		}
	}
	UNLOCK_SOUND_LIST();
#ifdef _EXTRA_SOUND_DEBUG
	printf("Generated and using %d sources\n", max_sources);
#endif // _EXTRA_SOUND_DEBUG

	// Initialise streams thread
#ifdef	OGG_VORBIS
	if (sound_streams_thread == NULL) {
		sound_streams_thread = SDL_CreateThread(update_streams, 0);
	}
#endif	// OGG_VORBIS

	if (num_types == 0)
	{
		// We have no sounds defined so assume the config isn't loaded
		// As it isn't already loaded, assume the default config location
		load_sound_config_data(SOUND_CONFIG_PATH);
	}

#ifndef NEW_WEATHER
	// Force the rain sound to be recreated
	rain_sound = 0;
#endif //NEW_WEATHER
	inited = 1;

	// Reset the error buffer
	if ((error = alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("%s: %s", snd_init_error, alGetString(error));
#endif // _EXTRA_SOUND_DEBUG
	}
}

void destroy_sound()
{
	int i, error;
	ALCcontext *context;
	ALCdevice *device;
	if (!inited){
		return;
	}
	inited = have_sound = have_music = sound_on = music_on = 0;

#ifdef OGG_VORBIS
	for (i = 0; i < MAX_STREAMS; i++)
	{
		destroy_stream(&streams[i]);
	}
	if (sound_streams_thread != NULL)
	{
		SDL_WaitThread(sound_streams_thread, NULL);
		sound_streams_thread = NULL;
	}
#endif // OGG_VORBIS
	// Remove physical elements (sources and buffers)
	LOCK_SOUND_LIST();
	for (i = 0; i < MAX_SOURCES; i++)
	{
		if (alIsSource(sound_source_data[i].source) == AL_TRUE)
		{
			alSourceStopv(1, &sound_source_data[i].source);
			alDeleteSources(1, &sound_source_data[i].source);
		}
		sound_source_data[i].source = 0;
	}
	used_sources = 0;
	for (i = 0; i < MAX_BUFFERS; i++)
	{
		reset_buffer_details(i);
	}
	// Flag all sounds as unloaded, but don't remove them
	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		sounds_list[i].playing = 0;
		sounds_list[i].loaded = 0;
	}
	UNLOCK_SOUND_LIST();
	SDL_DestroyMutex(sound_list_mutex);
	sound_list_mutex = NULL;

	/*
	 * alutExit() contains a problem with hanging on exit on some
	 * Linux systems.  The problem is with the call to
	 * alcMakeContextCurrent( NULL );  The folowing code is exactly
	 * what is in alutExit() minus that function call.  It causes
	 * no problems if the call is not there since the context is
	 * being destroyed right afterwards.
	 */
	context = alcGetCurrentContext();
	if (context != NULL)
	{
		device = alcGetContextsDevice(context);
#ifndef LINUX
		alcMakeContextCurrent(NULL);
#endif // LINUX
		alcDestroyContext(context);
		if (device != NULL)
		{
			alcCloseDevice(device);
			device = NULL;
		}
	}

	if (device != NULL && (error = alcGetError(device)) != AL_NO_ERROR) 
	{
		char str[256];
		safe_snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alcGetString(device, error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
	}
}



/********************
 * CONFIG FUNCTIONS *
 ********************/

void parse_server_sounds()
{
	int i;
	// Parse the list of sounds according to client_serv.h and map them to our sound defs
	//
	// NOTE: This must be kept up-to-date with client_serv.h for it to be any use!!
	for (i = 0; i <= 9; i++)
	{
		switch(i)
		{
			case snd_rain:
				server_sound[snd_rain] = get_index_for_sound_type_name("Rain");
				break;
			case snd_tele_in:
				server_sound[snd_tele_in] = get_index_for_sound_type_name("Teleport_in");
				break;
			case snd_tele_out:
				server_sound[snd_tele_out] = get_index_for_sound_type_name("Teleport_out");
				break;
			case snd_teleprtr:
				server_sound[snd_teleprtr] = get_index_for_sound_type_name("Teleporter");
				break;
			case snd_thndr_1:
				server_sound[snd_thndr_1] = get_index_for_sound_type_name("Thunder1");
				break;
			case snd_thndr_2:
				server_sound[snd_thndr_2] = get_index_for_sound_type_name("Thunder2");
				break;
			case snd_thndr_3:
				server_sound[snd_thndr_3] = get_index_for_sound_type_name("Thunder3");
				break;
			case snd_thndr_4:
				server_sound[snd_thndr_4] = get_index_for_sound_type_name("Thunder4");
				break;
			case snd_thndr_5:
				server_sound[snd_thndr_5] = get_index_for_sound_type_name("Thunder5");
				break;
			case snd_fire:
				server_sound[snd_fire] = get_index_for_sound_type_name("FireBig");
				break;
		}
	}
}

void parse_sound_object(xmlNode *inNode)
{
	xmlNode *attributeNode=NULL;

	char content[50];
	int iVal=0;
	float fVal=0.0f;
	char *sVal = NULL;

	sound_type *pData = NULL;

	if (num_types >= MAX_SOUNDS)
	{
		LOG_ERROR("%s: Maximum number of sounds (%d) reached!", snd_config_error, MAX_SOUNDS);
		return;
	}
	
	pData = &sound_type_data[num_types++];

	sVal = (char *)xmlGetProp(inNode,(xmlChar*)"name");
	if (!sVal)
	{
		LOG_ERROR("%s: sound has no name", snd_config_error);
	}
	else
	{
		safe_strncpy(pData->name, sVal, sizeof(pData->name));
		
		attributeNode = inNode->xmlChildrenNode;
		while (attributeNode != NULL)
		{
			if (attributeNode->type == XML_ELEMENT_NODE)
			{
				get_string_value(content, sizeof(content), attributeNode);
				if (!xmlStrcmp (attributeNode->name, (xmlChar*)"intro_sound"))
				{
					if (!strcasecmp(pData->part[STAGE_INTRO].file_path, ""))
					{
						safe_strncpy(pData->part[STAGE_INTRO].file_path, (char *)content, sizeof(pData->part[STAGE_INTRO].file_path));
					}
					else
					{
						LOG_ERROR("%s: intro_index already set!", snd_config_error);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"main_sound"))
				{
					if (!strcasecmp(pData->part[STAGE_MAIN].file_path, ""))
					{
						safe_strncpy(pData->part[STAGE_MAIN].file_path, (char *)content, sizeof(pData->part[STAGE_MAIN].file_path));
					}
					else
					{
						LOG_ERROR("%s: main_index already set!", snd_config_error);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"outro_sound"))
				{
					if (!strcasecmp(pData->part[STAGE_OUTRO].file_path, ""))
					{
						safe_strncpy(pData->part[STAGE_OUTRO].file_path, (char *)content, sizeof(pData->part[STAGE_OUTRO].file_path));
					}
					else
					{
						LOG_ERROR("%s: outro_index already set!", snd_config_error);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"gain"))
				{
					fVal = (float)atof((char *)content);
					if (fVal > 0.0f)
						pData->gain = fVal;
					else
					{
						LOG_ERROR("%s: gain = %f in '%s'", snd_config_error, fVal, pData->name);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"stereo"))
				{
					iVal = atoi((char *)content);
					if (iVal == 0 || iVal == 1)
						pData->stereo = iVal;
					else
					{
						LOG_ERROR("%s: stereo = %d in '%s'", snd_config_error, iVal, pData->name);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"distance"))
				{
					fVal = (float)atof((char *)content);
					if (fVal > 0.0f)
						pData->distance = fVal;
					else
					{
						LOG_ERROR("%s: distance = %f in '%s'", snd_config_error, fVal, pData->name);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"positional"))
				{
					iVal = atoi((char *)content);
					if (iVal == 0 || iVal == 1)
						pData->positional = iVal;
					else
					{
						LOG_ERROR("%s: positional = %d in '%s'", snd_config_error, iVal, pData->name);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"loops"))
				{
					iVal = atoi((char *)content);
					if (iVal >= 0)
						pData->loops = iVal;
					else
					{
						LOG_ERROR("%s: loops = %d in '%s'", snd_config_error, iVal, pData->name);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"fadeout_time"))
				{
					iVal = atoi((char *)content);
					if (iVal >= 0)
						pData->fadeout_time = iVal;
					else
					{
						LOG_ERROR("%s: fadeout_time = %d in '%s'", snd_config_error, iVal, pData->name);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"echo_delay"))
				{
					iVal = atoi((char *)content);
					if (iVal >= 0)
						pData->echo_delay = iVal;
					else
					{
						LOG_ERROR("%s: echo_delay = %d in '%s'", snd_config_error, iVal, pData->name);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"echo_volume"))
				{
					iVal = atoi((char *)content);
					if (iVal >= 0)
						pData->echo_volume = iVal;
					else
					{
						LOG_ERROR("%s: echo_volume = %d in '%s'", snd_config_error, iVal, pData->name);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"time_of_day_flags"))
				{
					sscanf((char *)content, "%x", &iVal);
					if (iVal >= 0 && iVal <= 0xffff)
						pData->time_of_the_day_flags = iVal;
					else
					{
						LOG_ERROR("%s: time_of_the_day_flags = 0x%x in '%s'", snd_config_error, iVal, pData->name);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"priority"))
				{
					iVal = atoi((char *)content);
					if (iVal >= 0)
						pData->priority = iVal;
					else
					{
						LOG_ERROR("%s: priority = %d in '%s'", snd_config_error, iVal, pData->name);
					}
				}
				else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"type"))
				{
					if (!strcasecmp((char *)content, "environmental")) {
						iVal = SOUNDS_ENVIRO;
					} else if (!strcasecmp((char *)content, "actor")) {
						iVal = SOUNDS_ACTOR;
					} else if (!strcasecmp((char *)content, "walking")) {
						iVal = SOUNDS_WALKING;
					} else if (!strcasecmp((char *)content, "map")) {
						iVal = SOUNDS_MAP;
					} else if (!strcasecmp((char *)content, "crowd")) {
						iVal = SOUNDS_CROWD;
					} else if (!strcasecmp((char *)content, "client")) {
						iVal = SOUNDS_CLIENT;
					} else if (!strcasecmp((char *)content, "gamewin")) {
						iVal = SOUNDS_GAMEWIN;
					} else {
						LOG_ERROR("%s: Unknown type '%s' for sound '%s'", snd_config_error, content, pData->name);
					}
					if (iVal > SOUNDS_NONE && iVal <= SOUNDS_CLIENT)
						pData->type = iVal;
				}
				else
				{
					LOG_ERROR("%s: Unknown attribute '%s' for sound '%s'", snd_config_error, attributeNode->name, pData->name);
				}
			}
			else if (attributeNode->type == XML_ENTITY_REF_NODE)
			{
				LOG_ERROR("%s: Include not allowed in sound def", snd_config_error);
			}
			attributeNode = attributeNode->next;
		}
	}
	//this is a fix to a problem where a single looping sound would not loop.
//	if(pData->sample_indices[STAGE_INTRO] < 0 && pData->loops == 0)
//		pData->sample_indices[STAGE_INTRO] = pData->sample_indices[STAGE_MAIN];
}

void store_boundary_coords(char *coordinates, int * x, int * y)
{
	int i = 0, j = 0, element = 0;
	char tmp[5] = "";
	
	while (i != strlen(coordinates))
	{
		if (coordinates[i] != ',' && j < 5 && i != strlen(coordinates))
		{
			tmp[j] = coordinates[i];
			j++;
		}
		else
		{
			if (element == 0)
			{
				*x = atoi(tmp);
				for (j = 0; j < 5; j++)
				{
					tmp[j] = '\0';
				}
				j = 0;
				element++;
			}
			else break;
		}
		i++;
	}
	if (element == 1)
	{
		*y = atoi(tmp);
	}
	else
	{
		LOG_ERROR("%s: Error parsing coordinates", snd_config_error);
	}
	return;
}

void parse_map_sound(xmlNode *inNode)
{
	xmlNode *boundaryNode = NULL;
	xmlNode *attributeNode = NULL;

	char *sVal = NULL;
	char content[50];
	
	int iVal;

	map_sound_data *pMap = NULL;
	map_sound_boundary_def *pMapBoundary = NULL;

	if (sound_num_maps >= MAX_SOUND_MAPS)
	{
		LOG_ERROR("%s: Maximum number of maps reached!", snd_config_error);
		return;
	}
	pMap = &sound_map_data[sound_num_maps++];

	sVal = (char *)xmlGetProp(inNode,(xmlChar*)"id");
	if(!sVal)
	{
		pMap->id = -1;
		LOG_ERROR("%s: map has no id", snd_config_error);
	}
	else
	{
		pMap->id = atoi(sVal);
		
		sVal = (char *)xmlGetProp(inNode,(xmlChar*)"name");
		if (sVal)
		{
			safe_strncpy(pMap->name, sVal, sizeof(pMap->name));
		}
		
		for (boundaryNode = inNode->children; boundaryNode; boundaryNode = boundaryNode->next)
		{
			if (boundaryNode->type == XML_ELEMENT_NODE)
			{
				if(!xmlStrcasecmp(boundaryNode->name, (xmlChar*)"boundary_def"))
				{
					// Process this set of boundaries
					if (pMap->num_boundaries < MAX_SOUND_MAP_BOUNDARIES)
					{
						pMap->num_boundaries++;
						pMapBoundary = &pMap->boundaries[pMap->num_boundaries - 1];

						for (attributeNode = boundaryNode->children; attributeNode; attributeNode = attributeNode->next)
						{
							get_string_value(content, sizeof(content), attributeNode);
							if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"background"))
							{
								// Find the type of background sound for this set of boundaries
								pMapBoundary->bg_sound = get_index_for_sound_type_name(content);
								if (pMapBoundary->bg_sound == -1)
								{
									LOG_ERROR("%s: sound not found for map boundary type '%s' in map '%s'", snd_config_error,content, pMap->name);
								}
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"crowd"))
							{
								// Find the type of crowd sound for this set of boundaries
								pMapBoundary->crowd_sound = get_index_for_sound_type_name(content);
								if (pMapBoundary->crowd_sound == -1)
								{
									LOG_ERROR("%s: crowd sound not found for map boundary type '%s' in map '%s'", snd_config_error,content, pMap->name);
								}
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"time_of_day_flags"))
							{
								// Find the type of crowd sound for this set of boundaries
								sscanf((char *)content, "%x", &iVal);
								if (iVal >= 0 && iVal <= 0xffff)
									pMapBoundary->time_of_day_flags = iVal;
								else
								{
									LOG_ERROR("%s: time_of_day flags (%s) invalid for map boundary in map '%s'", snd_config_error,content, pMap->name);
								}
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"is_default"))
							{
								// Find the type of crowd sound for this set of boundaries
								iVal = atoi(content);
								if (iVal == 0 || iVal == 1)
								{
									pMapBoundary->is_default = iVal;
									if (pMap->num_defaults < MAX_MAP_BACKGROUND_DEFAULTS)
									{
										pMap->num_defaults++;
										pMap->defaults[pMap->num_defaults] = pMap->num_boundaries;
									}
									else
									{
										LOG_ERROR("%s: Maximum defaults reached for map '%s'", snd_config_error, pMap->name);
									}
								}
								else
								{
									LOG_ERROR("%s: is_default setting (%s) invalid for map boundary in map '%s'", snd_config_error, content, pMap->name);
								}
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point1"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->x1, &pMapBoundary->y1);
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point2"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->x2, &pMapBoundary->y2);
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point3"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->x3, &pMapBoundary->y3);
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point4"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->x4, &pMapBoundary->y4);
							}
						}
					}
					else
					{
						LOG_ERROR("%s: reached max boundaries for map '%s'", snd_config_error, pMap->name);
					}
				}
				else
				{
					LOG_ERROR("%s: Boundary definition expected. Found: %s", snd_config_error, attributeNode->name);
				}
			}
			else if (boundaryNode->type == XML_ENTITY_REF_NODE)
			{
				LOG_ERROR("%s: Include not allowed in map sound def", snd_config_error);
			}
		}
	}
}

void parse_effect_sound(xmlNode *inNode)
{
	xmlNode *attributeNode = NULL;
	effect_sound_data *pEffect = NULL;
	char sound[MAX_SOUND_NAME_LENGTH] = "";

	if (inNode->type == XML_ELEMENT_NODE)
	{
		if (sound_num_effects >= MAX_SOUND_EFFECTS)
		{
			LOG_ERROR("%s: Maximum number of effects reached!", snd_config_error);
			return;
		}
		pEffect = &sound_effect_data[sound_num_effects++];
	
		for (attributeNode = inNode->children; attributeNode; attributeNode = attributeNode->next)
		{
			if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"id"))
			{
				pEffect->id = get_float_value(attributeNode);
			}
			else if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"sound"))
			{
				get_string_value(sound, sizeof(sound), attributeNode);
				pEffect->sound = get_index_for_sound_type_name(sound);
				if (pEffect->sound == -1)
				{
					LOG_ERROR("%s: Unknown sound %s for effect %d", snd_config_error, sound, pEffect->id);
				}
			}
		}
	}
	else if (inNode->type == XML_ENTITY_REF_NODE)
	{
		LOG_ERROR("%s: Include not allowed in effect sound def", snd_config_error);
	}
}

void parse_particle_sound(xmlNode *inNode)
{
	xmlNode *attributeNode = NULL;
	particle_sound_data *pParticle = NULL;
	char sound[MAX_SOUND_NAME_LENGTH] = "";

	if (inNode->type == XML_ELEMENT_NODE)
	{
		if (sound_num_particles >= MAX_SOUND_PARTICLES)
		{
#ifdef ELC
			LOG_ERROR("%s: Maximum number of particles reached!", snd_config_error);
#else
			printf("%s: Maximum number of particles reached!", snd_config_error);
#endif
			return;
		}
		pParticle = &sound_particle_data[sound_num_particles++];
	
		for (attributeNode = inNode->children; attributeNode; attributeNode = attributeNode->next)
		{
			if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"file"))
			{
				get_string_value(pParticle->file, sizeof(pParticle->file), attributeNode);
			}
			else if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"sound"))
			{
				get_string_value(sound, sizeof(sound), attributeNode);
				pParticle->sound = get_index_for_sound_type_name(sound);
				if (pParticle->sound == -1)
				{
					LOG_ERROR("%s: Unknown sound %s for particle %s", snd_config_error, sound, pParticle->file);
				}
			}
		}
	}
	else if (inNode->type == XML_ENTITY_REF_NODE)
	{
		LOG_ERROR("%s: Include not allowed in particle sound def", snd_config_error);
	}
}

int check_for_valid_background_details(background_default * test)
{
	int err, i;
	err = 0;
	// Check this has a valid sound
	if (test->sound < 0) return -1;
	// Check this time_of_day_flag and map_type don't conflict with an existing default background
	for (i = 0; i < sound_num_background_defaults - 1; i++)
	{
		if ((sound_background_defaults[i].time_of_day_flags & test->time_of_day_flags) > 0 &&
			sound_background_defaults[i].map_type == test->map_type)
		{
			// Conflict
			err = -1;
			break;
		}
	}
	return err;
}

void parse_background_defaults(xmlNode *inNode)
{
	xmlNode *attributeNode = NULL;
	background_default *pBackgroundDefault = NULL;
	char content[MAX_SOUND_NAME_LENGTH] = "";

	int iVal = 0;
	
	if (inNode->type == XML_ELEMENT_NODE)
	{
		if (sound_num_background_defaults >= MAX_BACKGROUND_DEFAULTS)
		{
#ifdef ELC
			LOG_ERROR("%s: Maximum number of sounds reached!", snd_config_error);
#else
			printf("%s: Maximum number of sounds reached!", snd_config_error);
#endif
			return;
		}
		pBackgroundDefault = &sound_background_defaults[sound_num_background_defaults++];
	
		for (attributeNode = inNode->children; attributeNode; attributeNode = attributeNode->next)
		{
			get_string_value(content, sizeof(content), attributeNode);
			if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"time_of_day_flags"))
			{
				sscanf((char *)content, "%x", &iVal);
				if (iVal >= 0 && iVal <= 0xffff)
					pBackgroundDefault->time_of_day_flags = iVal;
				else
				{
					LOG_ERROR("%s: time_of_the_day_flags = 0x%x in background default", snd_config_error, iVal);
				}
			}
			else if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"map_type"))
			{
				iVal = atoi((char *)content);
				if(iVal >= 0)
					pBackgroundDefault->map_type = iVal;
				else
				{
					LOG_ERROR("%s: Unknown map_type %s in background default", snd_config_error, content);
				}
			}
			else if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"sound"))
			{
				pBackgroundDefault->sound = get_index_for_sound_type_name(content);
				if (pBackgroundDefault->sound == -1)
				{
					LOG_ERROR("%s: Unknown sound %s in background default", snd_config_error, content);
				}
			}
		}
		if (check_for_valid_background_details(pBackgroundDefault))
		{
			LOG_ERROR("%s: invalid/conflicting background defaults found!", snd_config_error);
			// This background is invalid so reset it to the defaults (and reuse it if there are any others)
			pBackgroundDefault->time_of_day_flags = 0xffff;
			pBackgroundDefault->map_type = 0;
			pBackgroundDefault->sound = -1;
			sound_num_background_defaults--;
		}
	}
	else if (inNode->type == XML_ENTITY_REF_NODE)
	{
		LOG_ERROR("%s: Include not allowed in defaults sound def", snd_config_error);
	}
}

void parse_sound_defaults(xmlNode *inNode)
{
	xmlNode *attributeNode = NULL;
	char sound[MAX_SOUND_NAME_LENGTH] = "";

	if (inNode->type == XML_ELEMENT_NODE)
	{
		for (attributeNode = inNode->children; attributeNode; attributeNode = attributeNode->next)
		{
			if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"background"))
			{
				parse_background_defaults(attributeNode);
			}
			else if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"crowd"))
			{
				get_string_value(sound, sizeof(sound), attributeNode);
				crowd_default = get_index_for_sound_type_name(sound);
				if (crowd_default == -1)
				{
					LOG_ERROR("%s: Unknown sound %s for crowd default", snd_config_error, sound);
				}
			}
			else if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"walking"))
			{
				get_string_value(sound, sizeof(sound), attributeNode);
				walking_default = get_index_for_sound_type_name(sound);
				if (walking_default == -1)
				{
					LOG_ERROR("%s: Unknown sound %s for crowd default", snd_config_error, sound);
				}
			}
		}
	}
	else if (inNode->type == XML_ENTITY_REF_NODE)
	{
		LOG_ERROR("%s: Include not allowed in defaults sound def", snd_config_error);
	}
}

void parse_item_image_ids(char * content, item_sound_data * pItem)
{
	int i, j;
	char temp[5];
	
	i = 0;
	j = -1;
	while (i < strlen(content))
	{
		if (content[i] == ',')
		{
			if (pItem->num_imageids < MAX_ITEM_SOUND_IMAGE_IDS)
			{
				pItem->image_id[pItem->num_imageids++] = atoi(temp);
				j = -1;
			}
			else
			{
				LOG_ERROR("%s: Too many image ids defined for item sound: %s", snd_config_error, content);
				return;		// Avoid any more errors
			}
		}
		else
		{
			j++;
			if (j < 5)
				temp[j] = content[i];
			else
			{
				LOG_ERROR("%s: Invalid image id defined for item sound: %s...", snd_config_error, temp);
				j = -1;
			}
		}
		i++;
	}
}

void parse_item_sound(xmlNode *inNode)
{
	xmlNode *attributeNode = NULL;
	char content[100] = "";
	item_sound_data * pItem;

	if (inNode->type == XML_ELEMENT_NODE)
	{
		if (sound_num_items < MAX_SOUND_ITEMS)
		{
			pItem = &sound_item_data[sound_num_items++];
		
			for (attributeNode = inNode->children; attributeNode; attributeNode = attributeNode->next)
			{
				get_string_value(content, sizeof(content), attributeNode);
				if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"image_ids"))
				{
					parse_item_image_ids(content, pItem);
				}
				else if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"sound"))
				{
					pItem->sound = get_index_for_sound_type_name(content);
					if (pItem->sound == -1)
					{
						LOG_ERROR("%s: Unknown sound %s for item sound", snd_config_error, content);
					}
				}
			}
		}
		else
		{
			LOG_ERROR("%s: Too many item sounds defined.", snd_config_error);
		}
	}
	else if (inNode->type == XML_ENTITY_REF_NODE)
	{
		LOG_ERROR("%s: Include not allowed in item sound def", snd_config_error);
	}
}

void parse_tile_types(char * content, tile_sound_data * pTileType)
{
	int i, j;
	char temp[5];
	
	i = 0;
	j = -1;
	while (i < strlen(content))
	{
		if (content[i] == ',')
		{
			if (pTileType->num_tile_types < MAX_SOUND_TILES)
			{
				pTileType->tile_type[pTileType->num_tile_types++] = atoi(temp);
				j = -1;
			}
			else
			{
				LOG_ERROR("%s: Too many tile types defined for tile type sound: %s, max: %d", snd_config_error, content, MAX_SOUND_TILES);
				return;		// Avoid any more errors
			}
		}
		else
		{
			j++;
			if (j < 5)
				temp[j] = content[i];
			else
			{
				LOG_ERROR("%s: Invalid tile type defined for tile type sound: %s...", snd_config_error, temp);
				j = -1;
			}
		}
		i++;
	}
}

void parse_tile_type_sound(xmlNode *inNode)
{
	xmlNode *attributeNode = NULL;
	char content[100] = "";
	tile_sound_data * pTileType;

	if (inNode->type == XML_ELEMENT_NODE)
	{
		if (sound_num_tile_types < MAX_SOUND_TILE_TYPES)
		{
			pTileType = &sound_tile_data[sound_num_tile_types++];
		
			for (attributeNode = inNode->children; attributeNode; attributeNode = attributeNode->next)
			{
				get_string_value(content, sizeof(content), attributeNode);
				if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"tiles"))
				{
					parse_tile_types(content, pTileType);
				}
				else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"actor_types"))
				{
					safe_strncpy(pTileType->sounds[pTileType->num_sounds].actor_types, content, sizeof(pTileType->sounds[pTileType->num_sounds].actor_types));
					safe_strncpy(content, get_string_property(attributeNode, "sound"), sizeof(content));
					pTileType->sounds[pTileType->num_sounds].sound = get_index_for_sound_type_name(content);
					if (pTileType->sounds[pTileType->num_sounds].sound == -1)
					{
						LOG_ERROR("%s: Unknown sound %s for tile type sound", snd_config_error, content);
					}
					// Increment the actor_types count
					pTileType->num_sounds++;
				}
				else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"default"))
				{
					pTileType->default_sound = get_index_for_sound_type_name(content);
					if (pTileType->default_sound == -1)
					{
						LOG_ERROR("%s: Unknown default sound %s for tile type sound", snd_config_error, content);
					}
				}
			}
		}
		else
		{
			LOG_ERROR("%s: Too many tile type sounds defined.", snd_config_error);
		}
	}
	else if (inNode->type == XML_ENTITY_REF_NODE)
	{
		LOG_ERROR("%s: Include not allowed in tile type sound def", snd_config_error);
	}
}

void parse_spell_sound(xmlNode *inNode)
{
	char content[100] = "";
	int i = -1;

	if (inNode->type == XML_ELEMENT_NODE)
	{
		get_string_value(content, sizeof(content), inNode);
		if (!xmlStrcasecmp(inNode->name, (xmlChar*)"spell_effect"))
		{
			i = get_int_property(inNode, "id");
			if (i >= 0 && i < NUM_ACTIVE_SPELLS)
			{
				sound_spell_data[i] = get_index_for_sound_type_name(content);
				if (sound_spell_data[i] == -1)
				{
					LOG_ERROR("%s: Unknown sound %s for spell effect sound", snd_config_error, content);
				}
			}
			else
			{
				LOG_ERROR("%s: Unknown spell effect ID %d for spell effect sound", snd_config_error, i);
			}
		}
	}
	else if (inNode->type == XML_ENTITY_REF_NODE)
	{
		LOG_ERROR("%s: Include not allowed in spell effect sound def", snd_config_error);
	}
}

int parse_sound_defs(xmlNode *node)
{
	xmlNode *def;
	int ok = 1;

	for (def = node->children; def; def = def->next)
	{
		if (def->type == XML_ELEMENT_NODE)
		{
			if (xmlStrcasecmp (def->name, (xmlChar*)"sound") == 0)
			{
				parse_sound_object(def);
			}
			else if (xmlStrcasecmp (def->name, (xmlChar*)"defaults") == 0)
			{
				parse_sound_defaults(def);
			}
			else if (xmlStrcasecmp (def->name, (xmlChar*)"map") == 0)
			{
				parse_map_sound(def);
			}
			else if (xmlStrcasecmp (def->name, (xmlChar*)"effect") == 0)
			{
				parse_effect_sound(def);
			}
			else if (xmlStrcasecmp (def->name, (xmlChar*)"particle") == 0)
			{
				parse_particle_sound(def);
			}
			else if (xmlStrcasecmp (def->name, (xmlChar*)"item") == 0)
			{
				parse_item_sound(def);
			}
			else if (xmlStrcasecmp (def->name, (xmlChar*)"tile_type") == 0)
			{
				parse_tile_type_sound(def);
			}
			else if (xmlStrcasecmp (def->name, (xmlChar*)"spell_effect") == 0)
			{
				parse_spell_sound(def);
			}
			else
			{
				LOG_ERROR("%s: Unknown element found: %s", snd_config_error, def->name);
				ok = 0;
			}
		}
		else if (def->type == XML_ENTITY_REF_NODE)
		{
			ok &= parse_sound_defs (def->children);
		}
	}

	return ok;
}

void load_sound_config_data (const char *file)
{
#ifdef ELC
	xmlDoc *doc;
	xmlNode *root=NULL;

#ifdef	NEW_FILE_IO
	if ((doc = xmlReadFile(file, NULL, 0)) == NULL)
#else	// NEW_FILE_IO
	char path[1024];

#ifndef WINDOWS
	safe_snprintf(path, sizeof(path), "%s/%s", datadir, file);
#else
	safe_snprintf(path, sizeof(path), "%s", file);
#endif // !WINDOWS

	// Can we open the file as xml?
	if ((doc = xmlReadFile(path, NULL, 0)) == NULL)
#endif	// NEW_FILE_IO
	{
		char str[200];
		safe_snprintf(str, sizeof(str), book_open_err_str, file);
		LOG_ERROR(str);
		LOG_TO_CONSOLE(c_red1,str);
	}
	// Can we find a root element
	else if ((root = xmlDocGetRootElement(doc))==NULL)
	{
		LOG_ERROR("%s: No XML root element found in '%s'", snd_config_error, file);
	}
	// Is the root the right type?
	else if ( xmlStrcmp( root->name, (xmlChar*)"sound_config" ) )
	{
		LOG_ERROR("%s: Invalid root element '%s' in '%s'", snd_config_error, root->name, file);
	}
	// We've found our expected root, now parse the children
	else
	{
		clear_sound_data();
		parse_sound_defs(root);
		parse_server_sounds();
	}

	xmlFree(doc);
#ifdef DEBUG
	print_sound_types();
#endif // DEBUG
#endif // ELC
}
#endif //!NEW_SOUND


/***********************
 * DEBUGGING FUNCTIONS *
 ***********************/

#ifdef DEBUG
#ifndef NEW_SOUND
void print_sound_objects()
{
	int i;
	
	for (i = 0; i < MAX_SOURCES; i++)
	{
		struct sound_object *obj = &(sound_objects[i]);
		if (obj->file >= 0)
		{
			printf ("position = %d, file = %d, x = %d, y = %d, positional = %d, loops = %d\n", i, obj->file, obj->x, obj->y, obj->positional, obj->loops);
		}
	}
	printf ("\n");
}
#else // !NEW_SOUND
void print_sound_types()
{
	int i, j;
	sound_type *pData = NULL;
	map_sound_data *pMap = NULL;
	effect_sound_data *pEffect = NULL;
	particle_sound_data *pParticle = NULL;
	
	printf("\nSOUND TYPE DATA\n===============\n");
	printf("There are %d sound types (max %d):\n", num_types, MAX_SOUNDS);
	for(i = 0; i < num_types; ++i)
	{
		pData = &sound_type_data[i];
		printf("Sound type '%s' #%d:\n"			, pData->name, i);
		printf("\tIntro sample = '%s'\n"		, pData->part[STAGE_INTRO].file_path);
		printf("\tMain sample = '%s'\n"			, pData->part[STAGE_MAIN].file_path);
		printf("\tOutro sample = '%s'\n"		, pData->part[STAGE_OUTRO].file_path);
		printf("\tGain = %f\n"					, pData->gain);
		printf("\tStereo = %d\n"				, pData->stereo);
		printf("\tDistance = %f\n"				, pData->distance);
		printf("\tPositional = %d\n"			, pData->positional);
		printf("\tLoops = %d\n"					, pData->loops);
		printf("\tFadeout time = %dms\n"		, pData->fadeout_time);
		printf("\tEcho delay = %dms\n"			, pData->echo_delay);
		printf("\tEcho volume = %d%%\n"			, pData->echo_volume);
		printf("\tTime of day flags = 0x%x\n"	, pData->time_of_the_day_flags);
		printf("\tPriority = %d\n"				, pData->priority);
		printf("\tType = %d\n\n"				, pData->type);
	}
	
	printf("\nMAP SOUND DATA\n===============\n");
	printf("There are %d map sounds:\n"		, sound_num_maps);
	for(i = 0; i < sound_num_maps; ++i)
	{
		pMap = &sound_map_data[i];
		printf("Map id: %d\n"				, pMap->id);
		printf("Map name: %s\n"				, pMap->name);
		printf("Num boundaries: %d\n"		, pMap->num_boundaries);
		printf("Boundaries:\n");
		for(j = 0; j < pMap->num_boundaries; ++j)
		{
			printf("\tBoundary num: %d\n"		, j);
			printf("\tBackground sound: %d\n"	, pMap->boundaries[j].bg_sound);
			printf("\tCrowd sound: %d\n"		, pMap->boundaries[j].crowd_sound);
			printf("\tX1: %d, Y1: %d, X2: %d, Y2: %d, X3: %d, Y3: %d, X4: %d, Y4: %d\n",
				pMap->boundaries[j].x1, pMap->boundaries[j].y1,
				pMap->boundaries[j].x2, pMap->boundaries[j].y2,
				pMap->boundaries[j].x3, pMap->boundaries[j].y3,
				pMap->boundaries[j].x4, pMap->boundaries[j].y4);
		}
		printf("\n");
	}
	
	printf("\nEFFECT SOUND DATA\n===============\n");
	printf("There are %d effect sounds:\n", sound_num_effects);
	for(i = 0; i < sound_num_effects; ++i)
	{
		pEffect = &sound_effect_data[i];
		printf("Effect ID: %d\n"		, pEffect->id);
		printf("Sound: %d\n"			, pEffect->sound);
	}
	
	printf("\nPARTICLE SOUND DATA\n===============\n");
	printf("There are %d particle sounds:\n", sound_num_particles);
	for(i = 0; i < sound_num_particles; ++i)
	{
		pParticle = &sound_particle_data[i];
		printf("Particle file: %s\n"	, pParticle->file);
		printf("Sound: %d\n"			, pParticle->sound);
	}
	
	printf("\nSERVER SOUNDS\n===============\n");
	printf("There are 10 server sounds:\n");
	for(i = 0; i <= 9; ++i)
	{
		printf("Server Sound: %d = %d\n", i, server_sound[i]);
	}
}

void print_sound_samples()
{
	int i;
	sound_sample *pData=NULL;
	printf("\nSOUND SAMPLE DATA\n===============\n");
	printf("There are %d sound samples (max %d):\n", num_samples, MAX_BUFFERS);

	for(i = 0; i < num_samples; ++i)
	{
		pData = &sound_sample_data[i];
		printf("Sample #%d:\n"			, i);
		printf("\tBuffer ID = %d\n"		, pData->buffer);
		printf("\tSize = %d\n"			, pData->size);
		printf("\tFrequency = %f\n"		, pData->freq);
		printf("\tChannels = %d\n"		, pData->channels);
		printf("\tBits = %d\n"			, pData->bits);
		printf("\tLength = %dms\n"		, pData->length);
	}
}

void print_sounds_list()
{
	int i;
	sound_loaded *pData = NULL;
	printf("\nLOADED SOUND DATA\n===============\n");
	printf("There are %d loaded sounds (max %d):\n", num_sounds, MAX_BUFFERS * 2);

	for(i = 0; i < num_sounds; ++i)
	{
		pData = &sounds_list[i];
		printf("Loaded sound #%d:\n"			, i);
		printf("\tSound type %d : '%s'\n"		, pData->sound, sound_type_data[pData->sound].name);
		printf("\tX = %d\n"						, pData->x);
		printf("\tY = %d\n"						, pData->y);
		printf("\tPlaying = %s\n"				, pData->playing == 0 ? "No" : "Yes");
		printf("\tBase gain = %f\n"				, pData->base_gain);
		printf("\tCurrent (final) gain = %f\n"	, pData->cur_gain);
		printf("\tSamples are loaded: %s\n"		, pData->loaded == 0 ? "No" : "Yes");
	}
}

void print_sound_sources()
{
	int i;
	source_data *pData = NULL;
	printf("\nSOUND SOURCE DATA\n===============\n");
	printf("There are %d sound sources (max %d):\n", used_sources, max_sources);

	for (i = 0; i < used_sources; ++i)
	{
		pData = &sound_source_data[i];
		printf("Source #%d:\n"				, i);
		printf("\tLoaded sound num = %d\n"	, pData->loaded_sound);
		printf("\tSound type = %d : '%s'\n"	, sounds_list[pData->loaded_sound].sound
											, sound_type_data[sounds_list[pData->loaded_sound].sound].name);
		printf("\tPlay duration = %dms\n"	, pData->play_duration);
		printf("\tSource ID = %d\n"			, pData->source);
	}
}

void print_sound_streams()
{
	int i;
	stream_data *pData = NULL;
	printf("\nSOUND STREAM DATA\n===============\n");
	printf("There are %d sound streams:\n", max_streams);

	for (i = 0; i < max_streams; ++i)
	{
		pData = &streams[i];
		printf("Stream #%d:\n"				, i);
		printf("\tType = %s\n"				, get_stream_type(pData->type));
		printf("\tSound = %d : '%s'\n"		, pData->sound
											, pData->type != STREAM_TYPE_MUSIC ? sound_type_data[pData->sound].name : playlist[pData->sound].file_name);
		printf("\tSource ID = %d\n"			, pData->source);
		printf("\tCookie = %u\n"			, pData->cookie);
		printf("\tPlaying = %s\n"			, pData->playing == 1 ? "Yes" : "No");
		printf("\tDefault = %s\n"			, pData->is_default == 1 ? "Yes" : "No");
		printf("\tCurrent fade = %d\n"		, pData->fade);
		printf("\tFade length = %d\n"		, pData->fade_length);
		printf("\tProcessed = %d\n"			, pData->processed);
	}
}

#endif // !NEW_SOUND
#endif //_DEBUG

#ifdef OSX
//	Below code originally from alAux.c of Pong3D
//
//	Contains:	Mac specific OpenAL auxzilary routines
//	
//	Copyright:  Copyright (c) 2007 Apple Inc., All Rights Reserved
// ----------------------------------------------------
//
//	alutLoadMemoryFromFile ( inFilename, ioFormat, ioSize, ioFrequency )
//
//	Purpose:	load memory from contents of a file
//
//	Inputs:		inFilename - address of complete file name ( path )
//				ioFormat - if not null, address where to store the format of the data
//				ioSize - if not null, address where to store the size of the data
//				ioFrequency - if not null, address where to store the frequency of the data
//
// Returns:	On success, a pointer to memory containing the loaded sound.
//				It returns AL_NONE on failure.
//
ALvoid* alutLoadMemoryFromFile (const char * inFilename,
                                ALenum *ioFormat,
                                ALsizei *ioSize,
                                ALfloat *ioFrequency)
{
	ALvoid* result = nil;

	ExtAudioFileRef tExtAudioFileRef = NULL;
	
	do {
		FSRef tFSRef;	// convert the path into an FSRef
		OSStatus status = FSPathMakeRef( ( const UInt8 * ) inFilename, &tFSRef, FALSE );
		if ( noErr != status ) break;
		
		FSCatalogInfo tFSCatalogInfo;	// determine the size of the file
		status = FSGetCatalogInfo( &tFSRef, kFSCatInfoDataSizes, &tFSCatalogInfo, NULL, NULL, NULL );
		if ( noErr != status ) break;
		
		// Open the input file
		status = ExtAudioFileOpen( &tFSRef, &tExtAudioFileRef );
		if ( noErr != status ) break;
		
		AudioStreamBasicDescription tASBD;	// get the description ( ASDB ) of the audio file on disk
		UInt32 asbd_size = sizeof( AudioStreamBasicDescription );
		status = ExtAudioFileGetProperty( tExtAudioFileRef, kExtAudioFileProperty_FileDataFormat, &asbd_size, &tASBD );
		if ( noErr != status ) break;
		
		// configure the ioput ASBD for 8-bit mono
		//tASBD.mSampleRate = tASBD.mSampleRate;	// don't change the sample rate
		tASBD.mFormatID = kAudioFormatLinearPCM;
		tASBD.mFormatFlags = kAudioFormatFlagIsPacked;
		tASBD.mBytesPerPacket = 1;
		tASBD.mFramesPerPacket = 1;
		tASBD.mBytesPerFrame = 1;
		tASBD.mChannelsPerFrame = 1;
		tASBD.mBitsPerChannel = 8;
		tASBD.mReserved = 0;
		
		// now set the ioput ASDB
		status = ExtAudioFileSetProperty( tExtAudioFileRef, kExtAudioFileProperty_ClientDataFormat, asbd_size, &tASBD );
		if ( noErr != status ) break;
		
		AudioBufferList tABL;
		
		tABL.mNumberBuffers = 1;
		tABL.mBuffers[0].mNumberChannels = 0;
		tABL.mBuffers[0].mDataByteSize = 0;
		tABL.mBuffers[0].mData = nil;
		tABL.mBuffers[0].mNumberChannels = tASBD.mChannelsPerFrame;
		tABL.mBuffers[0].mDataByteSize = tFSCatalogInfo.dataLogicalSize;
		
		SInt64 totalAudioFrames;	// determine the number of frames in the file
		UInt32 ioPropertyDataSize = sizeof( totalAudioFrames );
		status = ExtAudioFileGetProperty( tExtAudioFileRef, kExtAudioFileProperty_FileLengthFrames, &ioPropertyDataSize, &totalAudioFrames );
		if ( noErr != status ) break;
		
		UInt32 numberOfFrames = totalAudioFrames;
		
		// allocate a buffer to read it into
		tABL.mBuffers[0].mData = calloc( totalAudioFrames, sizeof( char ) );
		if ( !tABL.mBuffers[0].mData ) {
			status = memFullErr;
			break;
		}
		
		// read all the frames into the buffer
		status = ExtAudioFileRead( tExtAudioFileRef, &numberOfFrames, &tABL );
		if ( noErr != status ) {
			free( tABL.mBuffers[0].mData );
			break;
		}

		result = tABL.mBuffers[0].mData;

		if ( ioFormat ) {
			*ioFormat = AL_FORMAT_MONO8;
		}

		if ( ioSize ) {
			*ioSize = totalAudioFrames * sizeof( char );
		}

		if ( ioFrequency ) {
			*ioFrequency = tASBD.mSampleRate;
		}
		
	} while ( FALSE );
	
	if ( tExtAudioFileRef ) {	// close the audio file
		ExtAudioFileDispose( tExtAudioFileRef );
	}
	
	return result;
}	// alutLoadMemoryFromFile

// ----------------------------------------------------
//
//	alutCreateBufferFromFile ( inFilename )
//
//	Purpose:	Create OpenAL buffer from contents of a file
//
//	Inputs:		inFilename - address of complete file name ( path )
//
// Returns:	On success, a handle to an OpenAL buffer containing the loaded sound.
//				It returns AL_NONE on failure.
//
ALuint alutCreateBufferFromFile ( const char * inFilename )
{
	ALuint result = AL_NONE;	// assume failure
	ALvoid* tData = nil;

	do {
		ALenum tFormat;
		ALsizei tSize;
		ALfloat tFrequency;
		
		tData = alutLoadMemoryFromFile( inFilename, &tFormat, &tSize, &tFrequency );
		if ( !tData ) break;

		// generate a new OpenAL buffer
		alGenBuffers( 1, &result );
		ALenum status = alGetError( );
		if ( AL_NO_ERROR != status ) break;
		
		// load the file's data into our OpenAL buffer
		alBufferData( result, tFormat, tData, tSize, tFrequency );
		status = alGetError( );
		if ( AL_NO_ERROR != status ) break;
	} while ( FALSE );

	if ( tData ) {	// dispose of the data buffer
		free( tData );
	}
	
	return result;
}
#endif

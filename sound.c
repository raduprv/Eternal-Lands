#ifdef NEW_SOUND
#include <ctype.h>
#include <string.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <math.h>
#include "sound.h"
#include "asc.h"
#include "draw_scene.h"
#include "errors.h"
#include "init.h"
#include "lights.h"
#include "map.h"
#include "misc.h"
#include "translate.h"
#include "weather.h"
#include "2d_objects.h"
#include "3d_objects.h"
#include "spells.h"
#include "tiles.h"
#include "actors.h"
#include "interface.h"
#include "io/elpathwrapper.h"
#include "io/elfilewrapper.h"
#include "threads.h"

#if defined _EXTRA_SOUND_DEBUG && OSX
 #define printf LOG_ERROR
#endif

#define OGG_BUFFER_SIZE (1048576)
#define STREAM_BUFFER_SIZE (4096 * 16)
#define SLEEP_TIME 300		// Time to give CPU to other processes between loops of update_streams()

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
#define	LOCK_SOUND_LIST() CHECK_AND_LOCK_MUTEX(sound_list_mutex);
#define	UNLOCK_SOUND_LIST() CHECK_AND_UNLOCK_MUTEX(sound_list_mutex);
#endif // _EXTRA_SOUND_DEBUG

#define MAX_FILENAME_LENGTH 80
#define MAX_BUFFERS 64
#define MAX_SOURCES 16
#define ABS_MAX_SOURCES 64				// Define an absolute maximum for the sources (the size of the array)

#define MAX_SOUNDS 200

#define MAX_STREAMS 7					// 1 music stream and up to 3 each for bg and crowds
#define STREAM_TYPE_NONE -1
#define STREAM_TYPE_SOUNDS 0
#define STREAM_TYPE_MUSIC 1
#define STREAM_TYPE_CROWD 2
#define NUM_STREAM_BUFFERS 4

#define MAX_PLAYLIST_ENTRIES 50

#define MAX_SOUND_FILES (MAX_SOUNDS * MAX_SOUND_VARIANTS * num_STAGES)	// The total number of files if every part of every
																		// variant of every sound had a different file
#define MAX_BACKGROUND_DEFAULTS 8			// Maximum number of global default backgrounds
#define MAX_MAP_BACKGROUND_DEFAULTS 4		// Maximum number of default backgrounds per map
#define MAX_SOUND_MAP_NAME_LENGTH 60		// Maximum length of the name of the map
#define MAX_SOUND_VARIANTS 10				// Maximum number of different sounds allowed for the one sound type
#define MAX_SOUND_MAP_BOUNDARIES 20			// Maximum number of boundary sets per map
#define MAX_SOUND_WALK_BOUNDARIES 40		// Maximum number of walk boundary sets per map
#define MAX_ITEM_SOUND_IMAGE_IDS 30			// Maximum number of image id's linked to an item sound def
#define MAX_SOUND_TILE_TYPES 20				// Maximum number of different tile types
#define MAX_SOUND_TILES 30					// Maximum number of different tiles for a tile type
#define MAX_SOUND_TILES_SOUNDS 5			// Maximum number of different sound types for a tile type

#define MAX_SOUND_MAPS 150			// This value is the maximum number of maps sounds can be defined for
									// (Roja has suggested 150 is safe for now)
#define MAX_SOUND_EFFECTS 60		// This value should equal the max special_effect_enum
#define MAX_SOUND_PARTICLES 20		// This value should equal the number of particle effects
#define MAX_SOUND_ITEMS 5			// This is the number of sounds defined for "Use item" sfx

#define MAX_SOUND_WARNINGS 50		// The number of user defined sound warnings

typedef enum
{
	STAGE_UNUSED = -1, STAGE_INTRO, STAGE_MAIN, STAGE_OUTRO, num_STAGES, STAGE_STREAM
} SOUND_STAGE;

typedef struct _source_list
{
	ALuint source;					// The physical address of this source (not the array id of source_data as that changes)
	struct _source_list *next;
	struct _source_list *last;
} source_list;

typedef struct
{
	char file_path[MAX_FILENAME_LENGTH];	// Where to load the file from
	int sample_num;							// Sample array ID (if loaded)
} sound_file;

typedef struct
{
	sound_file *part[num_STAGES];	// Pointers to the files used for each stage
	float gain;						// The gain of this sound (default 1.0)
									// The same sample may be defined under 2 sound names, with different gains.
} sound_variants;

typedef struct
{
	char name[MAX_SOUND_NAME_LENGTH];
	sound_variants variant[MAX_SOUND_VARIANTS];
	int num_variants;
	int stereo;						// 1 is stereo, 0 is mono (default mono)
	float distance;					// Distance it can be heard, in meters
	int positional;					// 1=positional, 0=omni (default positional)
	int loops;						// 0=infinite, otherwise the number of loops (default 1)
	int fadeout_time;				// In milliseconds, only for omni sounds that loop. (default 0) -- NOT USED
	int echo_delay;					// The value is the echo in MS. If 0, no echo (default 0) -- NOT USED
	int echo_volume;				// In percent, 0 means no sound, 100 means as loud as the original sound (default 50) -- NOT USED
	int time_of_the_day_flags;		// Bits 0-11 set each 1/2 hour of the 6-hour day (default 0xffff)
	unsigned int priority;			// If there are too many sounds to be played, highest value priority get culled (default 5)
	int type;						// The type of sound - environmental, actor, walking etc. (default Enviro)
} sound_type;

typedef struct
{
	ALuint buffer;							// If the sample is loaded, a buffer ID to play it.
	ALenum format;
	ALsizei size;							// Size of the sound data in bytes
	ALfloat freq;							// Frequency
	ALint channels;							// Number of sound channels
	ALint bits;								// Bits per channel per sample
	int length;								// Duration in milliseconds
	source_list *sources;					// List of sources using this sample
} sound_sample;

typedef struct
{
	int sound;							// The ID of the sound type
	int variant;						// The variant of the sound type being played
	int x;
	int y;
	int loaded;							// Has this sound been loaded into a buffer
	int playing;						// Is this sound loaded into a source and currently playing
	float base_gain;					// The initial gain (before adjustments for weather and sound type volume etc)
	float cur_gain;						// The last actual gain. If the calculated new gain is different then do an alSourcef command
	unsigned int cookie;
	int lifetime;						// The length of time in ms the sound has existed
} sound_loaded;

typedef struct
{
	ALuint source;						// A handle for the source
	int priority;						// Include this here for streams and to force a priority if needed
	int play_duration;
	int loaded_sound;					// Sound ID loaded into this source (Not used for streams)
	SOUND_STAGE current_stage;			// Set as STAGE_STREAMS for streams
	unsigned int cookie;
	int sample[num_STAGES];				// The samples currently in use by this source
} source_data;

typedef struct
{
	int sound;
	int time_of_day_flags;		// As for sound time_of_day_flags
	int map_type;				// At the moment, only 0 (outside) and 1 (dungeon). Checked against the dungeon flag.
} background_default;

typedef struct
{
	int x;
	int y;
	double a;					// Angle of the line between this point and the next (not used for outer bounding rect)
} bound_point;

typedef struct
{
	int bg_sound;
	int crowd_sound;
	int time_of_day_flags;		// As for sound time_of_day_flags
	int is_default;				// There can be up to 4 defaults for any one map, with unique times of day
								// Coords are ignored if is_default set.
	bound_point p[4];			// Details of this point and its corresponding angle
	int int_point;				// Index of the internal point or -1 for no internal point
	bound_point o[2];			// Outer bounding rectangle (0 = lower left, 1 = upper right)
} map_sound_boundary_def;

typedef struct
{
	int id;
	char name[MAX_SOUND_MAP_NAME_LENGTH];		// This isn't used, it is simply helpful when editing the config
	map_sound_boundary_def boundaries[MAX_SOUND_MAP_BOUNDARIES];		// This is the boundaries for backgound and crowd sounds
	map_sound_boundary_def walk_boundaries[MAX_SOUND_WALK_BOUNDARIES];	// This is the boundaries for walking sounds (to fake 3d objects)
	int num_boundaries;
	int num_walk_boundaries;
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
	char actor_types[1024];
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

typedef struct
{
	int sound;
	char string[256];
} sound_warnings;

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
	int variant;							// The variant of the sound playing				(not used for music)
	int is_default;							// Is this sound a default sound for this type	(not used for music)
	map_sound_boundary_def * boundary;		// This is a pointer to the boundary in use
} stream_data;

typedef struct {
	char file_name[64];
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int time;
} playlist_entry;


int have_sound = 0;
int have_music = 0;
int no_sound = 0;
int sound_on = 1;
int music_on = 1;
Uint8 inited = 0;

SDL_Thread *sound_streams_thread = NULL;
SDL_mutex *sound_list_mutex = NULL;

stream_data * music_stream = NULL;
stream_data streams[MAX_STREAMS];

ALfloat sound_gain = 1.0f;
ALfloat music_gain = 1.0f;
ALfloat crowd_gain = 1.0f;
ALfloat enviro_gain = 1.0f;
ALfloat actor_gain = 1.0f;
ALfloat walking_gain = 1.0f;
ALfloat gamewin_gain = 1.0f;
ALfloat client_gain = 1.0f;
ALfloat warnings_gain = 1.0f;

int used_sources = 0;						// the number of sources currently playing

char sound_device[30] = "";					// Set up a string to store the names of the selected sound device
char sound_devices[1000] = "";				// Set up a string to store the names of the available sound devices
int max_sources = MAX_SOURCES;				// Initialise our local maximum number of sources to the default
int max_streams = MAX_STREAMS;				// Initialise our local maximum number of streams to the default

int num_types = 0;							// Number of distinct sound types
int num_samples = 0;						// Number of actual sound files - a sound type can have > 1 sample
int num_sound_files = 0;					// Number of individual sound files
int num_sounds = 0;							// Number of sounds in the sounds_list
int sound_num_background_defaults = 0;		// Number of default background sounds
int sound_num_maps = 0;						// Number of maps we have sounds for
int sound_num_effects = 0;					// Number of effects we have sounds for
int sound_num_particles = 0;				// Number of particles we have sounds for
int sound_num_items = 0;					// Number of "Use item" actions we have sounds for
int sound_num_tile_types = 0;				// Number of tile type groups we have sounds for
int num_sound_warnings = 0;					// Number of string warnings
int have_sound_config = 0;					// true if the sound config file was found
static int must_restart_spell_sounds = 0;	// true if sounds have been stopped, triggeres an attempt to restart

int snd_cur_map = -1;
int cur_boundary = 0;

unsigned int next_cookie = 1;									// Each playing source is identified by a unique cookie.
sound_loaded sounds_list[MAX_BUFFERS * 2];						// The loaded sounds
source_data sound_source_data[ABS_MAX_SOURCES];					// The active (playing) sources
sound_type sound_type_data[MAX_SOUNDS];							// Configuration of the sound types
sound_sample sound_sample_data[MAX_BUFFERS];					// Buffer data for each sample
sound_file sound_files[MAX_SOUND_FILES];						// File names for each individual sound file
background_default sound_background_defaults[MAX_BACKGROUND_DEFAULTS];	// Default background sounds
																		// (must have non-overlapping time of day flags)
int crowd_default;												// Default sound for crowd effects
int walking_default;											// Default sound for walking
map_sound_data sound_map_data[MAX_SOUND_MAPS];					// Data for map sfx
effect_sound_data sound_effect_data[MAX_SOUND_EFFECTS];			// Data for effect sfx
particle_sound_data sound_particle_data[MAX_SOUND_PARTICLES];	// Data for particle sfx
item_sound_data sound_item_data[MAX_SOUND_ITEMS];				// Data for item sfx
tile_sound_data sound_tile_data[MAX_SOUND_TILE_TYPES];			// Data for tile (walking) sfx
int server_sound[10];											// Map of server sounds to sound def ids
int sound_spell_data[10];										// Map of id's for spells-that-affect-you to sounds
sound_warnings warnings_list[MAX_SOUND_WARNINGS];				// List of strings to monitor for warning sounds
int afk_snd_warning;											// Whether to play a sound on receiving a message while AFK

playlist_entry playlist[MAX_PLAYLIST_ENTRIES];
int loop_list = 1;
int list_pos = -1;


/*    *** Declare some local functions ****
 * These functions shouldn't need to be accessed outside sound.c
 */

/* Ogg handling	*/
int load_ogg_file(char *file_name, OggVorbis_File *oggFile);
int stream_ogg_file(char *file_name, stream_data * stream, int numBuffers);
int stream_ogg(ALuint buffer, OggVorbis_File * inStream, vorbis_info * info);
ALvoid * load_ogg_into_memory(char * szPath, ALenum *inFormat, ALsizei *inSize, ALfloat *inFreq);
/* Stream handling */
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
/* Music functions */
void play_song(int list_pos);
void find_next_song(int tx, int ty, int day_time);

/* Source functions */
void add_source_to_lists(source_data *pSource);
void remove_source_from_lists(source_data *pSource);
void clear_source(source_data *pSource);
source_data * get_available_source(int priority);
source_data *insert_sound_source_at_index(unsigned int index);
void release_source(source_data *pSource, int index);
int stop_sound_source_at_index(int index);
void set_sound_gain(source_data * pSource, int loaded_sound_num, float initial_gain);
/* Sample functions */
int ensure_sample_loaded(char * in_filename);
int load_samples(sound_type * pType);
void release_sample(int sample_num);
void unload_sample(int sample_num);
/* Individual sound functions */
int play_sound(int loaded_sound_num, int x, int y, float initial_gain);
unsigned int get_next_cookie();
int get_loaded_sound_num();
void unload_sound(int index);
/* General functions */
int get_3d_obj_walk_sound(char * filename);
int get_2d_obj_walk_sound(char * filename);
int get_boundary_walk_sound(int x_pos, int y_pos);
int get_tile_sound(int tile_type, char * actor_type);
int find_sound_from_cookie(unsigned int cookie);
int time_of_day_valid(int flags);
int sound_bounds_check(int x, int y, map_sound_boundary_def * bounds);
int test_bounds_angles(int x, int y, int point, map_sound_boundary_def * bounds);
double calculate_bounds_angle(int x, int y, int point, map_sound_boundary_def * bounds);
#ifdef DEBUG_MAP_SOUND
void print_sound_boundary_coords(int map);
#endif // DEBUG_MAP_SOUND
/* Init functions */
void parse_snd_devices(ALCchar * in_array, char * sound_devs);



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
	must_restart_spell_sounds = 1;
}

void turn_sound_off()
{
	int i = 0;
	ALuint error;
	if (!inited)
		return;
	if (!have_music || !music_on)
	{
		destroy_sound();
		return;
	}
	LOCK_SOUND_LIST();
	sound_on = 0;
	while (i < used_sources)
	{
		if (sound_source_data[i].current_stage == STAGE_STREAM)
		{
			i++;			// This source is being used by a stream and handled lower down so ignore
			continue;
		}
#ifdef _EXTRA_SOUND_DEBUG
		printf("Removing source with index %d\n", i);
#endif //_EXTRA_SOUND_DEBUG
		stop_sound_source_at_index(0);
		continue;
	}
	UNLOCK_SOUND_LIST();
	for (i = 0; i < max_streams; i++)
	{
		if (streams[i].type != STREAM_TYPE_MUSIC)
		{
			destroy_stream(&streams[i]);
		}
	}
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error turning sound off\n");
#endif //_EXTRA_SOUND_DEBUG
	}
}

void turn_music_on()
{
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
}

void turn_music_off()
{
	if (!inited)
		return;
	if (!have_sound || !sound_on)
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
}

void toggle_sounds(int *var)
{
	*var = !*var;
	if (!sound_on) {
		turn_sound_off();
	} else {
		turn_sound_on();
	}
}

void toggle_music(int * var) {
	*var = !*var;
	if (!music_on) {
		turn_music_off();
	} else {
		turn_music_on();
	}
}

void disable_sound(int *var)
{
	*var = !*var;
	if (no_sound) {
		destroy_sound();
		clear_sound_data();
		have_sound_config = 0;
	} else {
		if (sound_on)
			turn_sound_on();
		if (music_on)
			turn_music_on();
	}
}


/************************
 * OGG VORBIS FUNCTIONS *
 ************************/

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

int load_ogg_file(char * file_name, OggVorbis_File *oggFile)
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

int stream_ogg_file(char * in_filename, stream_data * stream, int numBuffers)
{
	int result, more_stream = 0, i;
	char filename[200];
	
	stop_stream(stream);
	ov_clear(&stream->stream);
	
	// Add the datadir to the input filename and try to open it
	strcpy(filename, datadir);
	strcat(filename, in_filename);
	result = load_ogg_file(filename, &stream->stream);
	if (!result) {
		LOG_ERROR("Error loading ogg file: %s\n", filename);
		return -1;
	}

	stream->info = ov_info(&stream->stream, -1);
	for (i = 0; i < numBuffers; i++)
	{
		more_stream = stream_ogg(stream->buffers[i], &stream->stream, stream->info);
		if (!more_stream)
		{
			LOG_ERROR("Error playing ogg file: %s\n", filename);
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

	// Clear any previous AL errors
	if ((error = alGetError()) != AL_NO_ERROR) { }

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
			continue;
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

	if ((error = alGetError()) != AL_NO_ERROR) 
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("stream_ogg %s: %s, buffer: %d, format: %d, size: %d, rate: %ld", my_tolower(reg_error_str), alGetString(error), buffer, format, size, info->rate);
#endif // _EXTRA_SOUND_DEBUG
	}
	return 1;
}

ALvoid * load_ogg_into_memory(char * szPath, ALenum *inFormat, ALsizei *inSize, ALfloat *inFreq)
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
		if (!have_music || sound<0 || sound>=MAX_PLAYLIST_ENTRIES) return;

		if (playlist[sound].file_name[0]!='.' && playlist[sound].file_name[0]!='/')
			safe_snprintf (tmp_file_name, sizeof (tmp_file_name), "./music/%s", playlist[sound].file_name);
		else
			safe_snprintf(tmp_file_name, sizeof (tmp_file_name), "%s", playlist[sound].file_name);
		file = tmp_file_name;
	}
	else
	{
		if (sound<0 || sound>=MAX_SOUNDS || sound_type_data[sound].num_variants <= 0) return;

		// Choose a variant
		stream->variant = rand() % sound_type_data[sound].num_variants;
		file = sound_type_data[sound].variant[stream->variant].part[STAGE_MAIN]->file_path;
		if (!have_sound || sound == -1 || !strcmp(file, "")) return;
	}

	// Set the gain for this stream
	alSourcef(stream->source, AL_GAIN, gain * (stream->type == STREAM_TYPE_MUSIC ? 1.0f : sound_type_data[sound].variant[stream->variant].gain));

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
	{
		UNLOCK_SOUND_LIST();
		return 0;
	}
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
	stream->type = STREAM_TYPE_NONE;

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
	
	if (sound == -1)
		return 0;
	
	// Check we aren't creating another music stream if one exists
	if (type == STREAM_TYPE_MUSIC && music_stream)
	{
		LOG_ERROR("Sound stream error: Tried to create additional music stream. This is a bug!\n");
		return 0;
	}
	
	for (i = 0; i < max_streams; i++)
	{
		// Check this sound isn't already being played in a stream
		if (streams[i].sound == sound && streams[i].type == type)
		{
			return 0;
		}
		if (streams[i].type == type)
		{
			// Count the number of streams of this type (for the priority calculation)
			type_count++;
			// Check if there is a default stream of this type that needs to be stopped
			if (streams[i].is_default)
			{
				default_found = i;
			}
		}
		// Find a spot in the array
		if (!stream && streams[i].type == STREAM_TYPE_NONE)
		{
			stream = &streams[i];
		}
	}
	if (!stream)
	{
		return 0;
	}
	// Calculate the priority of this stream (give the first 2 streams of this type top priority and then according to sound)
	if (type_count > 2)
		priority = sound_type_data[sound].priority;
	else
		priority = 1;

	// Init the stream
	if (!init_sound_stream(stream, type, priority))
		return 0;			// We couldn't get a source. Sad but you get that.

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
	printf("add_stream: Started stream. Type: %s, Sound: %d, Cookie: %u, Source: %d, Fade: %d, Default: %d\n", get_stream_type(stream->type), stream->sound, stream->cookie, stream->source, stream->fade, stream->is_default);
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
			if (snd > -1 && sound_bounds_check(tx, ty, &sound_map_data[snd_cur_map].boundaries[i]))
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
	ALuint buffer;
	
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
		alSourceUnqueueBuffers(stream->source, 1, &buffer);
		if ((error = alGetError()) != AL_NO_ERROR)
			LOG_ERROR("process_stream - Error unqueuing buffers: %s, Stream type: %s, Source: %d", alGetString(error), get_stream_type(stream->type), stream->source);

		stream->playing = stream_ogg(buffer, &stream->stream, stream->info);
		
		if (stream->playing)
		{
			alSourceQueueBuffers(stream->source, 1, &buffer);
			if ((error = alGetError()) != AL_NO_ERROR)
				LOG_ERROR("process_stream - Error requeuing buffers: %s, Stream type: %s, Source: %d, Playing: %s", alGetString(error), get_stream_type(stream->type), stream->source, stream->playing == 0 ? "No" : "Yes");
		}
	}
	
	// Check if the stream is still playing, and handle if not
	alGetSourcei(stream->source, AL_SOURCE_STATE, &state);
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("process_stream - Error retrieving state: %s, Stream type: %s, Source: %d\n", alGetString(error), get_stream_type(stream->type), stream->source);
#endif // _EXTRA_SOUND_DEBUG
	}
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
		// Clear any remaining buffers, just in case
		alSourceStop(stream->source);
		alSourcei(stream->source, AL_BUFFER, 0);
		if ((error=alGetError()) != AL_NO_ERROR) { }	// Clear any errors before trying to replay the stream
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
				if (stream->type == STREAM_TYPE_CROWD && no_near_enhanced_actors < 2)
				{
					// Don't stop until there are less than 2 people around so it will fade down more naturally
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
					if (!sound_bounds_check(tx, ty, stream->boundary))
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
	ALfloat gain = 0.0;

   	sleep = SLEEP_TIME;

	init_thread_log("update_streams");

#ifdef _EXTRA_SOUND_DEBUG
	printf("Starting streams thread\n");
#endif //_EXTRA_SOUND_DEBUG
	while (!exit_now && ((have_music && music_on) || (have_sound && sound_on)))
	{
		SDL_Delay(sleep);
		
		day_time = (game_minute >= 30 && game_minute < 60 * 3 + 30);

		LOCK_ACTORS_LISTS();
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
			if (have_sound && sound_on)
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
							if (!have_sound || !sound_on)
								continue;			// We aren't playing sounds so skip this stream
							gain = sound_gain * enviro_gain * sound_type_data[streams[i].sound].variant[streams[i].variant].gain;
							break;
						case STREAM_TYPE_CROWD:
							if (!have_sound || !sound_on)
								continue;			// We aren't playing sounds so skip this stream
							if (distanceSq_to_near_enhanced_actors == 0)
								distanceSq_to_near_enhanced_actors = 100.0f;	// Due to no actors when calc'ing
// SSE optimized sqrt		gain = sound_gain * crowd_gain * sound_type_data[streams[i].sound].variant[streams[i].variant].gain
// 									* fastsqrt(fastsqrt(no_near_enhanced_actors)) * invsqrt(distanceSq_to_near_enhanced_actors) * 2;
							gain = sound_gain * crowd_gain * sound_type_data[streams[i].sound].variant[streams[i].variant].gain
									* sqrt(sqrt(no_near_enhanced_actors)) / sqrt(distanceSq_to_near_enhanced_actors) * 2;
							break;
					}
					if (check_stream(&streams[i], day_time, tx, ty))
						process_stream(&streams[i], gain, &sleep);
				}
			}
		}
		UNLOCK_ACTORS_LISTS();
	}
	// We are bailing so destroy any remaining streams
	for (i = 0; i < max_streams; i++)
	{
		destroy_stream(&streams[i]);
	}
#ifdef _EXTRA_SOUND_DEBUG
	printf("Exiting streams thread. have_music: %d, music_on: %d, have_sound: %d, sound_on: %d, exit_now: %d\n", have_music, music_on, have_sound, sound_on, exit_now);
#endif //_EXTRA_SOUND_DEBUG
	return 1;
}




/*******************
 * MUSIC FUNCTIONS *
 *******************/

void get_map_playlist()
{
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
	safe_snprintf (map_list_file_name, sizeof (map_list_file_name), "music/%s", tmp);
	len = strlen (map_list_file_name);
	tmp = strrchr (map_list_file_name, '.');
	if (tmp == NULL)
		tmp = &map_list_file_name[len];
	else
		tmp++;
	len -= strlen (tmp);
	safe_snprintf (tmp, sizeof (map_list_file_name) - len, "pll");

	fp=open_file_data(map_list_file_name,"r");
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
}

void play_music(int list)
{
	int i=0;
	char list_file_name[256];
	FILE *fp;
	char strLine[255];

	if(!have_music)return;

	safe_snprintf(list_file_name, sizeof(list_file_name), "music/%d.pll", list);
	fp=open_file_data(list_file_name, "r");
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
	play_song(list_pos);
}

int display_song_name()
{
	if (!music_stream || !music_stream->playing)
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




/*****************************
 * SOURCE HANDLING FUNCTIONS *
 *****************************/
void add_source_to_lists(source_data *pSource)
{
	int i;
	source_list *source_id;
	source_list *new_source_id;
	
	new_source_id = calloc(1, sizeof(source_list));
	new_source_id->source = pSource->source;
	new_source_id->next = NULL;
	new_source_id->last = NULL;
	
	for (i = 0; i < num_STAGES; i++)
	{
		if (pSource->sample[i] > -1)
		{
			// Get the current list of sources for this sample
			source_id = sound_sample_data[pSource->sample[i]].sources;
			if (source_id)
			{
				// Find the end of the list and add this source to it
				while (source_id->next)
					source_id = source_id->next;
				source_id->next = new_source_id;
				new_source_id->last = source_id;
			}
			else
			{
				// Create this source as the start of the list
				source_id = new_source_id;
			}
		}
	}
}

void remove_source_from_lists(source_data *pSource)
{
	int i;
	source_list *source_id;
	
	for (i = 0; i < num_STAGES; i++)
	{
		if (pSource->sample[i] > -1)
		{
			source_id = sound_sample_data[pSource->sample[i]].sources;
			while (source_id)
			{
				if (source_id->source == pSource->source)
				{
					// Check if there are sources before and after this one in the list
					// and adjust their last and nexts to remove this one from the list
					if (source_id->next && source_id->last)
					{
						source_id->next->last = source_id->last;
						source_id->last->next = source_id->next;
					}
					else if (source_id->next) // This is the first source
					{
						source_id->next->last = NULL;
					}
					else if (source_id->last) // This is the last source
					{
						source_id->last->next = NULL;
					}
					else	// This is the only source, so free the buffer for this sample
					{
						release_sample(pSource->sample[i]);
					}
					free(source_id);
					source_id = NULL;
				}
				else if (source_id && source_id->next)
				{
					source_id = source_id->next;
				}
			}
		}
	}
}

void clear_source(source_data *pSource)
{
	// Reset the source
	if (pSource->source > 0)
	{
		alSourceStop(pSource->source);
		alSourcei(pSource->source, AL_BUFFER, 0);
	}
	// If this source is attached to samples, clear it from their list
	remove_source_from_lists(pSource);
	// Reset the details of this source
	pSource->priority = 0;
	pSource->play_duration = 0;
	pSource->current_stage = STAGE_UNUSED;
	pSource->loaded_sound = -1;
	pSource->cookie = 0;
	pSource->sample[STAGE_INTRO] = -1;
	pSource->sample[STAGE_MAIN] = -1;
	pSource->sample[STAGE_OUTRO] = -1;
}

source_data * get_available_source(int priority)
{
	source_data * pSource;
	int i;
	
	if (used_sources == max_sources)
		return NULL;
	
	// Search for an available source. The sources are ordered by decreasing play priority
	for (pSource = sound_source_data, i = 0; i < max_sources - 1; ++i, ++pSource)
	{
		if (pSource->loaded_sound < 0 && pSource->current_stage != STAGE_STREAM)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Found available source at index %d/%d, Source: %d\n", i, used_sources, pSource->source);
#endif //_EXTRA_SOUND_DEBUG
			pSource->priority = priority;
			used_sources++;
			return pSource;
		}
		else if (priority <= pSource->priority)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Inserting new source at index %d/%d", i, used_sources);
#endif //_EXTRA_SOUND_DEBUG
			pSource = insert_sound_source_at_index(i);
#ifdef _EXTRA_SOUND_DEBUG
			printf(", Source: %d\n", pSource->source);
#endif //_EXTRA_SOUND_DEBUG
			pSource->priority = priority;
			used_sources++;
			return pSource;
		}
	}

	// All sources are used by higher-priority sounds
	return NULL;
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
	clear_source(&tempSource);

	// Shunt source objects down a place
	for (i = min2i(used_sources, max_sources - 1); i > index; --i)
	{
		sound_source_data[i] = sound_source_data[i - 1];
	}

	// Now insert our stored object at #index
	sound_source_data[index] = tempSource;

	// Return a pointer to this new source
	return &sound_source_data[index];
}

void release_source(source_data *pSource, int index)
{
	source_data sourceTemp;

	// Check we have the index for this source
	if (index == -1)
		for (index = 0; &sound_source_data[index] != pSource; index++) {}
	// Reset the data for this source
	clear_source(pSource);
	// We can't lose a source handle - copy this...
	sourceTemp = *pSource;
	//...shift all the next sources up a place, overwriting the stopped source...
	memmove(pSource, pSource+1, sizeof(source_data) * (used_sources - (index + 1)));
	//...and put the saved object back in after them
	sound_source_data[used_sources - 1] = sourceTemp;
	// Note that one less source is playing!
	--used_sources;
}

// This stops the source for sound_source_data[index]. Because this array will change, the index
// associated with a source will change, so this function should only be called if the index is
// known for certain.
int stop_sound_source_at_index(int index)
{
	ALuint error = AL_NO_ERROR;
	source_data *pSource;
	
	if (index < 0 || index >= used_sources)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Trying to unload invalid source! Index: %d, Num Sources: %d\n", index, used_sources);
#endif //_EXTRA_SOUND_DEBUG
		return 0;
	}
	
	// This should not happen
	if (index >= max_sources)
	{
		LOG_ERROR("Trying to unload invalid source! Index: %d, Num Sources: %d, Max Sources: %d\n", index, used_sources, max_sources);
		return 0;
	}
	
	pSource = &sound_source_data[index];

	// Error if source is invalid (lost)
	if (!alIsSource(sound_source_data[index].source))
	{
   		LOG_ERROR("Attempting to stop invalid sound source %d with index %d", (int)pSource->source, index);
	}

	release_source(pSource, index);

	// Clear any errors so as to not confuse other error handlers
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error '%s' stopping sound source with index: %d/%d.\n", alGetString(error), index, used_sources + 1);
#endif //_EXTRA_SOUND_DEBUG
	}

	return 1;
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

void sound_source_set_gain(unsigned long int cookie, float gain)
{
	int n;
	source_data *pSource;

	// Source handle of 0 is a null source
	if (!have_sound || !cookie)
		return;
	
	// Find which of our playing sources matches the handle passed
	LOCK_SOUND_LIST();
	n = find_sound_source_from_cookie(cookie);
	if (n > 0)
	{
		pSource = &sound_source_data[n];
		set_sound_gain(pSource, pSource->loaded_sound, gain);
	}
	UNLOCK_SOUND_LIST();
	return;
}

void set_sound_gain(source_data * pSource, int loaded_sound_num, float new_gain)
{
	float type_gain = 1.0f;
	int error;
	sound_type * this_snd = &sound_type_data[sounds_list[loaded_sound_num].sound];
	sound_variants * this_variant = &this_snd->variant[sounds_list[loaded_sound_num].variant];
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
		case SOUNDS_WARNINGS:
			type_gain = warnings_gain;
			break;
	}
	// Check if we need to update the base gain for this sound
	if (new_gain != sounds_list[loaded_sound_num].base_gain)
		sounds_list[loaded_sound_num].base_gain = new_gain;

	// Check if we need to dim down the sounds due to rain
	if (this_snd->type != SOUNDS_CLIENT && this_snd->type != SOUNDS_GAMEWIN && this_snd->type != SOUNDS_WARNINGS && this_snd->type != SOUNDS_ENVIRO)
		new_gain = weather_adjust_gain(new_gain, pSource->cookie);

	// Check if we need to update the overall gain for this source
	if (sound_gain * type_gain * this_variant->gain * new_gain != sounds_list[loaded_sound_num].cur_gain)
	{
		sounds_list[loaded_sound_num].cur_gain = sound_gain * type_gain * this_variant->gain * new_gain;
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




/*******************************
 * SAMPLE PROCESSING FUNCTIONS *
 *******************************/

/* If the sample given by the filename is not currently loaded, create a
 * buffer and load it from the path given.
 * Returns the sample number for success or -1 on failure.
 *
 * This function is likely to be very expensive as it initially traverses the types array
 * checking if the sample is loaded, then checks for a space in the buffer array, and
 * if it still hasn't found a spot, traverses the buffer and source arrays looking for a
 * buffer that isn't currently being played by a source!
 */
int ensure_sample_loaded(char * in_filename)
{
	int i, j, k, l, sample_num, error;
	ALvoid *data;
	ALsizei datasize;
	ALuint *pBuffer;
	sound_sample *pSample;
	char filename[200];				// This is for the full path to the file

	// Check if this sample is already loaded and if so, return the sample ID
	for (i = 0; i < MAX_SOUND_FILES; i++)
	{
		if (!strcasecmp(sound_files[i].file_path, in_filename) && sound_files[i].sample_num > -1)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Found this sample already loaded: %s, sound file: %d, sample num: %d\n", in_filename, i, sound_files[i].sample_num);
#endif //_EXTRA_SOUND_DEBUG
			return sound_files[i].sample_num;
		}
	}
	
	// Sample isn't loaded so find a space in the array
	sample_num = -1;
	for (i = 0; i < MAX_BUFFERS; i++)
	{
		// Check if this sample is free (no buffer exists)
		if (sound_sample_data[i].buffer == 0 || !alIsBuffer(sound_sample_data[i].buffer))
		{
			sample_num = i;
#ifdef _EXTRA_SOUND_DEBUG
			printf("Found a spot for this sample: %s, sample %d\n", in_filename, i);
#endif //_EXTRA_SOUND_DEBUG
			break;
		}
	}

	if (sample_num == -1)
	{
		// Don't have a spot yet so check for any inactive samples (not loaded into a source)
		// -- This part will be expensive!! Let's hope we don't get here often --
		int found;
#ifdef _EXTRA_SOUND_DEBUG
		char tmp[1000] = "";
		snprintf(tmp, sizeof(tmp), "Eek! Didn't want to get here. Sample (%s) is apparently not loaded and there is no space for it!", in_filename);
		LOG_TO_CONSOLE(c_red1, tmp);
		printf("Eek, nowhere for the sample: %s\n", in_filename);
#endif //_EXTRA_SOUND_DEBUG
		for (i = 0; i < MAX_BUFFERS; i++)
		{
			found = 0;
			for (j = 0; j < used_sources; j++)
			{
				for (k = 0; k < sound_type_data[sounds_list[sound_source_data[j].loaded_sound].sound].num_variants; k++)
				{
					for (l = STAGE_INTRO; l <= STAGE_OUTRO; l++)
					{
						// Check if this sample is loaded into a source atm
						if (sound_type_data[sounds_list[sound_source_data[j].loaded_sound].sound].variant[k].part[l] &&
							sound_type_data[sounds_list[sound_source_data[j].loaded_sound].sound].variant[k].part[l]->sample_num == i)
						{
							found = 1;
							break;
						}
					}
					if (found)
						break;	// No need to keep looking
				}
				if (found)
					break;	// No need to keep looking
			}
			if (!found)
			{
				// This is an unused sample
				unload_sample(i);
				sample_num = i;
#ifdef _EXTRA_SOUND_DEBUG
				printf("Found unused sample slot: %d\n", i);
#endif //_EXTRA_SOUND_DEBUG
				break;
			}
		}
		if (sample_num == -1)
		{
			// We didn't find an available sample slot so error and bail
#ifdef _EXTRA_SOUND_DEBUG
			LOG_ERROR("Error: Too many samples loaded. Unable to load sample: %s, num samples: %d\n", in_filename, num_samples);
#endif //_EXTRA_SOUND_DEBUG
			return -1;
		}
	}
	
#ifdef _EXTRA_SOUND_DEBUG
	printf("Got sample num: %d, Attemping to load sound: File: %s\n", sample_num, in_filename);
#endif //_EXTRA_SOUND_DEBUG

	num_samples++;
	pSample = &sound_sample_data[sample_num];

	// Add the data dir to the front of the input filename
	strcpy(filename, datadir);
	strcat(filename, in_filename);

	// Load the file into memory
	data = load_ogg_into_memory(filename, &pSample->format, &datasize, &pSample->freq);
	if (!data)
	{
		// Couldn't load the file, so release this sample num
		num_samples--;
		// We have already dumped an error message so just return
		return -1;
	}
			
#ifdef _EXTRA_SOUND_DEBUG
	printf("Result: File: %s, Format: %d, Size: %d, Freq: %f\n", filename, (int)pSample->format, (int)datasize, (float)pSample->freq);
#endif //_EXTRA_SOUND_DEBUG

	// Create a buffer for the sample
	pBuffer = &pSample->buffer;
	alGenBuffers(1, pBuffer);
	if ((error=alGetError()) != AL_NO_ERROR) 
	{
		// Couldn't generate a buffer
		LOG_ERROR("Error generating buffer: %s", alGetString(error));
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error generating buffer: %s\n", alGetString(error));
#endif //_EXTRA_SOUND_DEBUG
		*pBuffer = 0;
		// Get rid of the temporary data
		free(data);
		return -1;
	}

	// Send this data to the buffer
	alBufferData(*pBuffer, pSample->format, data, datasize, pSample->freq);
	if ((error=alGetError()) != AL_NO_ERROR)
	{
		LOG_ERROR("Error loading buffer: %s", alGetString(error));
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error loading buffer: %s\n", alGetString(error));
#endif //_EXTRA_SOUND_DEBUG
		alDeleteBuffers(1, pBuffer);
		// Get rid of the temporary data
		free(data);
		return -1;
	}

	alGetBufferi(*pBuffer, AL_BITS, &pSample->bits);
	alGetBufferi(*pBuffer, AL_CHANNELS, &pSample->channels);
	alGetBufferi(*pBuffer, AL_SIZE, &pSample->size);

	pSample->length = (pSample->size * 1000) / ((pSample->bits >> 3) * pSample->channels * pSample->freq);

	// Get rid of the temporary data
	free(data);

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
	int i, j;
	// Choose a variant
	j = rand() % pType->num_variants;
	// Check we can load all samples used by this type
	for (i = 0; i < num_STAGES; ++i)
	{
		if (pType->variant[j].part[i] && pType->variant[j].part[i]->sample_num < 0 && strcasecmp(pType->variant[j].part[i]->file_path, ""))
		{
			pType->variant[j].part[i]->sample_num = ensure_sample_loaded(pType->variant[j].part[i]->file_path);
			if (pType->variant[j].part[i]->sample_num < 0)
			{
#ifdef _EXTRA_SOUND_DEBUG
				printf("Error: problem loading sample: %s\n", pType->variant[j].part[i]->file_path);
#endif //_EXTRA_SOUND_DEBUG
				return -1;
			}
		}
	}
	return j;		// Return the variant we chose
}

void release_sample(int sample_num)
{
	sound_sample * pSample;

	// Check we have a valid sample_num
	if (sample_num < 0 || sample_num >= MAX_BUFFERS)
		return;
	
	pSample = &sound_sample_data[sample_num];

	// Release the buffer used by this sample
	if (alIsBuffer(pSample->buffer))
		alDeleteBuffers(1, &pSample->buffer);
	// Reset the array element
	pSample->buffer = 0;
	pSample->format = 0;
	pSample->size = 0;
	pSample->freq = 0;
	pSample->channels = 0;
	pSample->bits = 0;
	pSample->length = 0;
}

void unload_sample(int sample_num)
{
	int i, j, k;

	// Check we have a valid sample_num
	if (sample_num < 0 || sample_num >= MAX_BUFFERS)
		return;

	// Find the places this sample is listed and reset them
	for (i = 0; i < num_types; i++)
	{
		for (j = 0; j < sound_type_data[i].num_variants; j++)
		{
			for (k = 0; k < num_STAGES; k++)
			{
				if (sound_type_data[i].variant[j].part[k] && sound_type_data[i].variant[j].part[k]->sample_num == sample_num)
				{
					sound_type_data[i].variant[j].part[k]->sample_num = -1;
				}
			}
		}
	}
	// Release the buffer used by this sample and reset the array element
	release_sample(sample_num);
}




/*****************************************
 * INDIVIDUAL SOUND PROCESSING FUNCTIONS *
 *****************************************/

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

// Wrapper function for adding walking sounds.
unsigned int add_walking_sound(int type, int x, int y, int me, float scale)
{
	return add_sound_object_gain(type, x, y, me, scale);
}

/* Wrapper function for adding map based particle sounds
 *
 * Due to only a subset of sound information being programmed into the map data, sounds
 * are triggered by the addition of each particle system. This causes problems for fires,
 * fountains, waterfalls and anything that is made of multiple particle systems.
 *
 * To fix this, this function checks for any existing sounds in a similar location before
 * loading this one.
 */
unsigned int add_map_sound(int type, int x, int y)
{
	int i;
	const int buffer = 5;
	// Check if there is another sound within +/-5 (buffer) tiles around this position
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

// Wrapper for regular particle sounds (non-map based) to avoid the location check
unsigned int add_particle_sound(int type, int x, int y)
{
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

unsigned int add_death_sound(actor * act)
{
	int snd;
	// Check the type of this actor has a death sound (only used for enhanced actors as simple
	// actors are triggered though the cal animation)
	if (!act)
		return 0;
	snd = actors_defs[act->actor_type].cal_frames[cal_actor_die1_frame].sound;
	if (snd > -1)
	{
		return add_sound_object_gain(snd, act->x_pos, act->y_pos, act == your_actor ? 1 : 0, 1.0f);
	}
	return 0;
}

unsigned int add_battlecry_sound(actor * act)
{
	// Maybe play a battlecry sound
	if (act && rand() % 3 == 2)			// 1 chance in 3 to play
	{
		return add_sound_object_gain(actors_defs[act->actor_type].battlecry.sound,
										act->x_pos * 2,
										act->x_pos * 2,
										act == your_actor ? 1 : 0,
										actors_defs[act->actor_type].battlecry.scale
									);
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

/*	Torg: Checks for sound enabled etc have been removed as we should load sounds even if currently
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
		LOG_ERROR("%s: %i", snd_invalid_number, type);
#ifdef _EXTRA_SOUND_DEBUG
		printf("Apparently an invalid type: %d\n", type);
#endif //_EXTRA_SOUND_DEBUG
		return 0;
	}
	pNewType = &sound_type_data[type];

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
		if (have_sound && sound_on)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Error: Too many sounds loaded!! n00b! Not playing this sound: %d (%s)\n", type, pNewType->name);
#endif //_EXTRA_SOUND_DEBUG
			LOG_ERROR("Error: Too many sounds loaded. Not playing this sound: %d (%s)\n", type, pNewType->name);
		}
		return 0;
	}
	
	// We got this far so add this sound to the list and assign it a cookie
	LOCK_SOUND_LIST();
	
	sounds_list[sound_num].sound = type;
	sounds_list[sound_num].variant = -1;		// Haven't chosen a variant yet - that's done by load_samples()
	sounds_list[sound_num].x = x;
	sounds_list[sound_num].y = y;
	sounds_list[sound_num].loaded = 0;
	sounds_list[sound_num].playing = 0;
	sounds_list[sound_num].base_gain = initial_gain;
	sounds_list[sound_num].cur_gain = -1.0f;		// Make sure we set the gain when we first play the sound
	cookie = get_next_cookie();
	sounds_list[sound_num].cookie = cookie;
	num_sounds++;
	
	// Check if we should try to load the samples (sound is enabled)
	if (inited && have_sound && sound_on)
	{
		// Load all samples used by this type
		sounds_list[sound_num].variant = load_samples(pNewType);
		if (sounds_list[sound_num].variant < 0)
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
		printf("Not playing this sound as sound isn't enabled yet. Inited: %d, Have sound: %d, Sound on: %d, Cookie: %d\n", inited, have_sound, sound_on, cookie);
	}
#endif //_EXTRA_SOUND_DEBUG

	// We have added the sound to the list so return the cookie
	UNLOCK_SOUND_LIST();
	return cookie;
}

int play_sound(int sound_num, int x, int y, float initial_gain)
{
	int loops, error;
#ifdef _EXTRA_SOUND_DEBUG
	int err = 0;
#endif // _EXTRA_SOUND_DEBUG
	ALuint buffer = 0;
	SOUND_STAGE stage;
	ALfloat sourcePos[] = {x, y, 0.0};
	ALfloat sourceVel[] = {0.0, 0.0, 0.0};
	source_data * pSource;
	sound_variants * pVariant;
	sound_type * pNewType = &sound_type_data[sounds_list[sound_num].sound];

	// Check if we have a sound device and its worth continuing
	if (!inited)
		return 0;

#ifdef _EXTRA_SOUND_DEBUG
	printf("Playing this sound: %d, Sound num: %d, Cookie: %d\n", sounds_list[sound_num].sound, sound_num, sounds_list[sound_num].cookie);
#endif //_EXTRA_SOUND_DEBUG

	// Check if we need to load the samples into buffers
	if (!sounds_list[sound_num].loaded)
	{
		sounds_list[sound_num].variant = load_samples(pNewType);
		if (sounds_list[sound_num].variant < 0)
		{
			return 0;
		}
		sounds_list[sound_num].loaded = 1;
	}
	pVariant = &pNewType->variant[sounds_list[sound_num].variant];

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
	stage = !pVariant->part[STAGE_INTRO] || pVariant->part[STAGE_INTRO]->sample_num < 0 ? STAGE_MAIN : STAGE_INTRO;

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

#ifdef _EXTRA_SOUND_DEBUG
	if (!alIsSource(pSource->source))
	{
		printf("Source corruption! %d, %d, %d\n", pSource->source, sounds_list[sound_num].sound, sounds_list[sound_num].cookie);
		release_source(pSource, -1);
		return 0;
	}
	else
	{
		int tmp = 0;
		alGetSourcei(pSource->source, AL_SOURCE_TYPE, &tmp);
		if ((error=alGetError()) != AL_NO_ERROR)
		{
			printf("Error getting source status! %s, %d, %d, %d\n", alGetString(error), pSource->source, sounds_list[sound_num].sound, sounds_list[sound_num].cookie);
		}
		else if (tmp == AL_STATIC)
		{
			printf("Found a static source! This isn't meant to exist! %d, %d\n", pSource->source, sounds_list[sound_num].sound);
		}
	}
#endif //_EXTRA_SOUND_DEBUG
	
	// Ensure the source is ready for queueing (attach a NULL buffer to reset it)
	alSourcei(pSource->source, AL_BUFFER, 0);
#ifdef _EXTRA_SOUND_DEBUG
	if ((error=alGetError()) != AL_NO_ERROR)
	{
		printf("Error resetting source! %d, %d\n", pSource->source, sounds_list[sound_num].sound);
	}
#endif //_EXTRA_SOUND_DEBUG
	
	for (; stage < num_STAGES; ++stage)
	{
		// Get the buffer to be queued.
		if (!pVariant->part[stage] || pVariant->part[stage]->sample_num < 0)
			continue;
		buffer = sound_sample_data[pVariant->part[stage]->sample_num].buffer;
		pSource->sample[stage] = pVariant->part[stage]->sample_num;
#ifdef _EXTRA_SOUND_DEBUG
		printf("Stage: %d, Buffer address: %d\n", stage, buffer);
#endif //_EXTRA_SOUND_DEBUG
		
		// If there are a finite number of loops for main sample, queue them all here
		if (stage == STAGE_MAIN)
		{	
			if (pNewType->loops > 0)
			{
				for (loops = 0; loops < pNewType->loops; ++loops)
				{
					alSourceQueueBuffers(pSource->source, 1, &buffer);
#ifdef _EXTRA_SOUND_DEBUG
					if ((error=alGetError()) != AL_NO_ERROR)
					{
						printf("Error with sample alSourceQueueBuffers: %s, Name: %s. Source: %d, Loops: %d/%d\n", alGetString(error), pNewType->name, pSource->source, loops, pNewType->loops);
						err = 1;
					}
#endif //_EXTRA_SOUND_DEBUG
				}
			}
			else
			{
				alSourceQueueBuffers(pSource->source, 1, &buffer);
#ifdef _EXTRA_SOUND_DEBUG
				if ((error=alGetError()) != AL_NO_ERROR)
				{
					printf("Error with looped sample alSourceQueueBuffers: %s, Name: %s. Source: %d\n", alGetString(error), pNewType->name, pSource->source);
					err = 1;
				}
#endif //_EXTRA_SOUND_DEBUG
				// Dont queue an outro that will never get played!
				break;
			}
		}
		else 
		{
			alSourceQueueBuffers(pSource->source, 1, &buffer);
#ifdef _EXTRA_SOUND_DEBUG
			if ((error=alGetError()) != AL_NO_ERROR)
			{
				printf("Error with intro/outro sample alSourceQueueBuffers: %s, Name: %s. Source: %d\n", alGetString(error), pNewType->name, pSource->source);
				err = 1;
			}
#endif //_EXTRA_SOUND_DEBUG
		}
	}
	add_source_to_lists(pSource);
#ifdef _EXTRA_SOUND_DEBUG
	if (err != 0)
	{
#else //_EXTRA_SOUND_DEBUG
	if ((error=alGetError()) != AL_NO_ERROR)
	{
		LOG_ERROR("Error with alSourceQueueBuffers: %s, Name: %s. Source: %d\n", alGetString(error), pNewType->name, pSource->source);
#endif //_EXTRA_SOUND_DEBUG
		release_source(pSource, -1);
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
	if ((pVariant->part[STAGE_INTRO] && pVariant->part[STAGE_INTRO]->sample_num < 0 && pNewType->loops == 0) ||
		(!pVariant->part[STAGE_INTRO] && pNewType->loops == 0))
		// 0 is infinite looping
		alSourcei(pSource->source,AL_LOOPING,AL_TRUE);
	else
		alSourcei(pSource->source,AL_LOOPING,AL_FALSE);

	alSourcePlay(pSource->source);
	sounds_list[sound_num].playing = 1;

	pSource->cookie = sounds_list[sound_num].cookie;

	// Check and clear any AL error
	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error setting source properties/playing source: %s, Name: %s. Source: %d, Cookie: %d\n", alGetString(error), pNewType->name, pSource->source, sounds_list[sound_num].cookie);
#endif // _EXTRA_SOUND_DEBUG
	}

#ifdef _EXTRA_SOUND_DEBUG
	printf("Playing %d sources\n", used_sources);
#endif // _EXTRA_SOUND_DEBUG

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

// This is passed a cookie, and searches for a sound and source (if ness) with this cookie
void stop_sound(unsigned long int cookie)
{
	int n, m;

	// Cookie of 0 is an invalid sound handle
	if (!cookie)
		return;

	// Find which sound matches this cookie
	n = find_sound_from_cookie(cookie);
#ifdef _EXTRA_SOUND_DEBUG
	printf("Removing cookie: %ld, Sound number: %d\n", cookie, n);
#endif //_EXTRA_SOUND_DEBUG
	if (n >= 0)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Stopping cookie %ld with sound index %d. It is currently %splaying.\n", cookie, n, sounds_list[n].playing ? "" : "not ");
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
#ifdef _EXTRA_SOUND_DEBUG
			else
			{
				printf("ERROR! Unable to find source for cookie %ld with sound index %d. It is meant to be currently playing.\n", cookie, n);
			}
#endif //_EXTRA_SOUND_DEBUG
			UNLOCK_SOUND_LIST();
		}
		sounds_list[n].playing = 0;
		unload_sound(n);
	}
}

void stop_sound_at_location(int x, int y)
{
	int i = 0;

	// Search for a sound at the given location
	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		if (sounds_list[i].x == x && sounds_list[i].y == y)
		{
			stop_sound(sounds_list[i].cookie);
		}
	}
}

// Kill all the not playing and looping sounds. Useful when we change maps, etc.
void stop_all_sounds()
{
	int i;
	ALuint error;

	if (!inited) return;

#ifdef _EXTRA_SOUND_DEBUG
	printf("Stopping all individual sounds\n");
#endif //_EXTRA_SOUND_DEBUG
	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		// Check if this is a loaded sound that isn't going to finish by itself (not playing or loops)
		if (sounds_list[i].cookie != 0 && (!sounds_list[i].playing || sound_type_data[sounds_list[i].sound].loops == 0))
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Stopping sound %d (%s), cookie: %d, used_sources: %d\n", i, sound_type_data[sounds_list[sound_source_data[0].loaded_sound].sound].name, sounds_list[i].cookie, used_sources);
#endif //_EXTRA_SOUND_DEBUG
			stop_sound(sounds_list[i].cookie);
		}
	}

	for (i = 0; i < max_streams; i++)
	{
		// Fade the music stream down (we can use it again after the map change) but destroy any other streams
		if (streams[i].type == STREAM_TYPE_MUSIC)
			streams[i].fade = -1;
		else
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Stopping %s stream source: %d\n", get_stream_type(streams[i].type), streams[i].source);
#endif //_EXTRA_SOUND_DEBUG
			destroy_stream(&streams[i]);
		}
	}

	if ((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error killing all sounds\n");
#endif //_EXTRA_SOUND_DEBUG
	}

	must_restart_spell_sounds = 1;
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
	sounds_list[index].lifetime = 0;
	num_sounds--;
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
	sound_variants *pVariant;
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
	int j, k;
#endif // _EXTRA_SOUND_DEBUG
	int l;

	// Check if we have a sound config, and thus if its worth doing anything (or sound is disabled)
	if (num_types < 1 || no_sound)
		return;

	LOCK_ACTORS_LISTS();
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
		// a map change or sound-off will have stopped spell sounds,
		// now we have our actor, we can re-enable the spell sounds
		if (must_restart_spell_sounds && !disconnected)
		{
			restart_active_spell_sounds();
			must_restart_spell_sounds = 0;
		}
	}

	// Start to process the sounds
	LOCK_SOUND_LIST();

	// Check the sounds_list for anything to be loaded/played
	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		// If this sound is "live"
		if (sounds_list[i].sound > -1)
		{
			pSoundType = &sound_type_data[sounds_list[i].sound];
			// Update the length of time this sound has been alive
			sounds_list[i].lifetime += ms;
			// Check for any non-looping sounds in the sounds_list that aren't being played that have passed their time
			if (sounds_list[i].lifetime >= 1000 && !sounds_list[i].playing && pSoundType->loops != 0)
			{
#ifdef _EXTRA_SOUND_DEBUG
				printf("Sound (%d) timed out: %d (%s), Cookie: %d, Lifetime: %d\n", i, sounds_list[i].sound, pSoundType->name, sounds_list[i].cookie, sounds_list[i].lifetime);
#endif //_EXTRA_SOUND_DEBUG
				stop_sound(sounds_list[i].cookie);
				continue;
			}
			// Check for any sounds that aren't being played and check if they need to be because
			// sound has now been enabled or they have come back into range
			x = sounds_list[i].x;
			y = sounds_list[i].y;
			if (inited && !sounds_list[i].playing && x > -1 && y > -1 && sounds_list[i].sound > -1)
			{
				distanceSq = (tx - x) * (tx - x) + (ty - y) * (ty - y);
				maxDistSq = pSoundType->distance * pSoundType->distance;
				if (sound_on && (distanceSq < maxDistSq))
				{
					// This sound is back in range so load it into a source and play it
#ifdef _EXTRA_SOUND_DEBUG
					printf("Sound now in-range: %d (%s), Distance squared: %d, Max: %d\n", sounds_list[i].sound, pSoundType->name, distanceSq, maxDistSq);
#endif //_EXTRA_SOUND_DEBUG
					sounds_list[i].cur_gain = -1.0f;	// Make sure we recalculate this sound's volume
					if (!play_sound(i, x, y, sounds_list[i].base_gain))
					{
#ifdef _EXTRA_SOUND_DEBUG
						printf("Error restarting sound!!\n");
#endif //_EXTRA_SOUND_DEBUG
					}
				}
			}
		}
	}
	
	// Check if we have a sound device and its worth continuing
	if (!inited)
	{
		UNLOCK_SOUND_LIST();
		UNLOCK_ACTORS_LISTS();
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
		UNLOCK_ACTORS_LISTS();
		return;
	}

#ifdef _EXTRA_SOUND_DEBUG
	j = 0;
#endif // _EXTRA_SOUND_DEBUG
	// Now, update the position of actor (animation) sounds
	for (i = 0; i < max_actors; i++)
	{
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

	UNLOCK_ACTORS_LISTS();
	
	// Finally, update all the sources
	i = 0;
#ifdef _EXTRA_SOUND_DEBUG
	j = 0;
#endif // _EXTRA_SOUND_DEBUG
	
	while (i < used_sources)
	{
		pSource = &sound_source_data[i];
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

		// Check for invalid sources -- This test should be redundant!
		if (pSource->cookie == 0 || pSource->loaded_sound < 0 || sounds_list[pSource->loaded_sound].sound < 0 || pSource->current_stage == STAGE_UNUSED)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Removing dud sound %d. Cookie: %d, Source Num: %d, Source: %d. Current stage: %d\n", pSource->loaded_sound, pSource->cookie, i, pSource->source, pSource->current_stage);
#endif //_EXTRA_SOUND_DEBUG
			if (pSource->loaded_sound >= 0)
				unload_sound(pSource->loaded_sound);
			stop_sound_source_at_index(i);
			continue;
		}

		// Update the gain for this source if nessessary
		set_sound_gain(pSource, pSource->loaded_sound, sounds_list[pSource->loaded_sound].base_gain);

		pSoundType = &sound_type_data[sounds_list[pSource->loaded_sound].sound];
		pVariant = &pSoundType->variant[sounds_list[pSource->loaded_sound].variant];
		pSample = &sound_sample_data[pVariant->part[pSource->current_stage]->sample_num];

		// Is this source still playing?
		alGetSourcei(pSource->source, AL_SOURCE_STATE, &state);
		if (state == AL_STOPPED)
		{
#ifdef _EXTRA_SOUND_DEBUG
//			printf("'%s' has stopped after sample '%s'\n", pSoundType->name, pVariant->part[pSource->current_stage]->file_path);
#endif //_EXTRA_SOUND_DEBUG
			// Flag the sound as finished
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
				printf("Cookie: %d, Loaded sound: %d, sound: %d, '%s' - sample '%s' has ended...", pSource->cookie, pSource->loaded_sound, sounds_list[pSource->loaded_sound].sound, pSoundType->name, pVariant->part[pSource->current_stage]->file_path);
				k = 0;
#endif //_EXTRA_SOUND_DEBUG
				while (++pSource->current_stage <= num_STAGES)
				{
#ifdef _EXTRA_SOUND_DEBUG
					k++;
					if (k > num_STAGES)
					{
						LOG_ERROR("update_sound race condition!! cur stage = %d, num_stages = %d\n", pSource->current_stage, num_STAGES);
						printf("update_sound race condition!! cur stage = %d, num_stages = %d\n", pSource->current_stage, num_STAGES);
						pSource->current_stage = num_STAGES;
						break;
					}
#endif // _EXTRA_SOUND_DEBUG
					if (!pVariant->part[pSource->current_stage] || pVariant->part[pSource->current_stage]->sample_num < 0)
					{
						// No more samples to play
#ifdef _EXTRA_SOUND_DEBUG
						printf("no more samples for this type!\n");
#endif //_EXTRA_SOUND_DEBUG
						pSource->current_stage = num_STAGES;
						break;
					}
					pSample = &sound_sample_data[pVariant->part[pSource->current_stage]->sample_num];
					// Found the currently-playing buffer

					if (pSample->buffer == buffer)
					{
#ifdef _EXTRA_SOUND_DEBUG
						printf("next sample is '%s'\n", pVariant->part[pSource->current_stage]->file_path);
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
								LOG_ERROR("update_sound error: %s", alGetString(error));
							}
						}
						break;
					}
				}
			}
			else
			{
				if (pSoundType->loops != 0)
				{
					// Check if something odd happened and this source wasn't stopped when the sample finished (lifetime > sum of sample lengths)
					int lifetime = 0;
					for (l = 0; l <= pSource->current_stage; l++)
						if (pVariant->part[l])
							lifetime += sound_sample_data[pVariant->part[l]->sample_num].length;
					if (sounds_list[pSource->loaded_sound].lifetime > lifetime)
					{
#ifdef _EXTRA_SOUND_DEBUG
						printf("This sound has passed its lifetime (%d ms)! It should have been stopped!: %d (%s). Playing: %d, lifetime: %d, cookie: %d\n", 
							   lifetime, sounds_list[pSource->loaded_sound].sound, pSoundType->name, sounds_list[pSource->loaded_sound].playing,
							   sounds_list[pSource->loaded_sound].lifetime, sounds_list[pSource->loaded_sound].cookie);
#endif //_EXTRA_SOUND_DEBUG
						unload_sound(pSource->loaded_sound);
						stop_sound_source_at_index(i);
						continue;
					}
				}
			}
		}

		// Check if we need to remove this sound because its finished
		// If the state is num_STAGES then the sound has ended (or gone wrong)
		if (pSource->current_stage == num_STAGES)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Removing finished sound %d (%s) at cookie %d, source index %d, source %d, loaded_sound: %d\n", sounds_list[pSource->loaded_sound].sound, pSoundType->name, pSource->cookie, i, pSource->source, pSource->loaded_sound);
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
				sounds_list[pSource->loaded_sound].playing = 0;
				stop_sound_source_at_index(i);
				continue;
			}
			else if (sound_on && (state == AL_PAUSED) && (distanceSq < maxDistSq))
			{
				LOG_ERROR("Sound error: We found a wasted source. Sound %d (%s) was loaded into a source and paused!!\n", i, pSoundType->name);
				alSourcePlay(pSource->source);
				sounds_list[pSource->loaded_sound].playing = 1;
			}
			if ((error=alGetError()) != AL_NO_ERROR) 
		    {
				LOG_ERROR("update_sound %s: %s", my_tolower(reg_error_str), alGetString(error));
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
	float x, y;
	int snd, cur_sound;
	
	if (pActor->cur_anim.sound_scale > 0.0f)
	{
		// This creature is large enough for a walking sound so look for one
		x = pActor->x_pos;
		y = pActor->y_pos;

		// Start with the sound from the animation
		snd = def_snd;

/*		Unfortunatly these functions aren't ready for release.
		They need help to be accurate, possibly from the under_mouse or get_intersect type code.
		Alternatively a check for objects around the actor and then checking the size and rotation
		of each object to determine if the actor is standing on it might work.
		
		// Check for a 3d object we have a sound for
		snd = get_3d_obj_walk_sound(get_3dobject_at_location(x, y));

		// Check if we need to look for a 2d obj, and look if necessary
		if (snd == -1)
			snd = get_2d_obj_walk_sound(get_2dobject_at_location(x, y));
*/
		
		// If we still don't have a sound, check for a defined area (the same as map boundary areas).
		// NOTE: This code can and should be removed when the above functions are fixed.
		if (snd == -1)
			snd = get_boundary_walk_sound((int)(x * 2), (int)(y * 2));

		// Finally, check if we need to look for a tile, and look if necessary
		if (snd == -1)
			snd = get_tile_sound(get_tile_type((int)(x * 2), (int)(y * 2)), actors_defs[pActor->actor_type].actor_name);

#ifdef _EXTRA_SOUND_DEBUG
//		printf("Actor: %s, Pos: %f, %f, Found sound: %d, Scale: %f\n", pActor->actor_name, pActor->x_pos, pActor->y_pos, snd, actors_defs[pActor->actor_type].walk_snd_scale);
#endif // _EXTRA_SOUND_DEBUG

		if (snd == -1)
			// Still no sound, so fall back on the global default
			snd = walking_default;

		// Check for something to do
		if (snd > -1)
		{
			// Check if we have a sound and it is different to the current one
			cur_sound = find_sound_from_cookie(pActor->cur_anim_sound_cookie);
			if (cur_sound >= 0 && sounds_list[cur_sound].sound != snd && sound_type_data[sounds_list[cur_sound].sound].type == SOUNDS_WALKING)
			{
				// It is valid and different so remove the current sound before we add the new one
				stop_sound(pActor->cur_anim_sound_cookie);
			}
			if (cur_sound < 0 || sounds_list[cur_sound].sound != snd)
			{
				// Add the new sound
				pActor->cur_anim_sound_cookie = add_walking_sound(	snd,
																	2 * pActor->x_pos,
																	2 * pActor->y_pos,
																	pActor->actor_id == yourself ? 1 : 0,
																	pActor->cur_anim.sound_scale
																);
			}
		}
	}
}

int get_boundary_walk_sound(int tx, int ty)
{
	int i, snd = -1;

	if (snd_cur_map > -1 && sound_map_data[snd_cur_map].id > -1)
	{
		for (i = 0; i < sound_map_data[snd_cur_map].num_walk_boundaries; i++)
		{
			if (i == sound_map_data[snd_cur_map].num_walk_boundaries)
				i = 0;
			snd = sound_map_data[snd_cur_map].walk_boundaries[i].bg_sound;
			if (snd > -1 && sound_bounds_check(tx, ty, &sound_map_data[snd_cur_map].walk_boundaries[i]))
			{
				return snd;
			}
		}
	}
	return -1;
}

int get_3d_obj_walk_sound(char * filename)
{
	if (!strcasecmp(filename, ""))
		return -1;
#ifdef _EXTRA_SOUND_DEBUG
	printf("Searching for the sound for 3D object: %s\n", filename);
#endif //_EXTRA_SOUND_DEBUG
	return -1;
}

int get_2d_obj_walk_sound(char * filename)
{
	if (!strcasecmp(filename, ""))
		return -1;
#ifdef _EXTRA_SOUND_DEBUG
	printf("Searching for the sound for 2D object: %s\n", filename);
#endif //_EXTRA_SOUND_DEBUG
	return -1;
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
#ifdef DEBUG_MAP_SOUND
	char str[100];
	safe_snprintf(str, sizeof(str), "Map number: %d", map_num);
	LOG_TO_CONSOLE(c_red1, str);
#endif // DEBUG_MAP_SOUND
	// Find the index for this map in our data
	snd_cur_map = -1;
	for (i = 0; i < sound_num_maps; i++)
	{
		if (map_num == sound_map_data[i].id)
		{
			snd_cur_map = i;
#ifdef DEBUG_MAP_SOUND
			safe_snprintf(str, sizeof(str), "Snd config map ID: %d, Snd config map name: %s", i, sound_map_data[i].name);
			LOG_TO_CONSOLE(c_red1, str);
			print_sound_boundary_coords(map_num);
#endif // DEBUG_MAP_SOUND
			return;
		}
	}
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
	for(i = 0; i < sound_num_particles; ++i)
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
	for(i = 0; i < sound_num_effects; ++i)
	{
		if (sound_effect_data[i].id == sfx)
			return sound_effect_data[i].sound;
	}
	return -1;
}

int get_index_for_inv_usewith_item_sound(int use_image_id, int with_image_id)
{
	char name[MAX_SOUND_NAME_LENGTH];

#ifdef _EXTRA_SOUND_DEBUG
	printf("Searching for the sound for: %d on %d\n", use_image_id, with_image_id);
#endif //_EXTRA_SOUND_DEBUG
	snprintf(name, sizeof(name), "%d on %d", use_image_id, with_image_id);
	return get_index_for_sound_type_name(name);
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

int check_sound_loops(unsigned int cookie)
{
	int snd = find_sound_from_cookie(cookie);
	if (snd > -1 && sound_type_data[sounds_list[snd].sound].loops == 0)
		return 1;
	return 0;
}

void check_sound_alerts(const Uint8* text, size_t len, Uint8 channel)
{
	int i;
	for (i = 0; i < num_sound_warnings; i++)
	{
		if (safe_strcasestr((char *)text, len, warnings_list[i].string, strlen(warnings_list[i].string)))
		{
			add_sound_object_gain(warnings_list[i].sound, 0, 0, 1, 1.0f);
			return;		// Only play one sound
		}
	}
	return;
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
 * Initially test to see if the point given is within the outer boundary given by the extreme of the coords. If
 * it is, then continue with the more complex test of each boundary line.
 *
 * We will do this by checking the angle of the line created between each of the 4 points of the boundaries and
 * comparing that angle to the line created by each point and our test point.
 *
 */
int sound_bounds_check(int x, int y, map_sound_boundary_def * bounds)
{
	int pX, pY, npX, npY;
	int i, j, result;
	
	// Initially check if we are on or inside the outermost box
	if (x < bounds->o[0].x || y < bounds->o[0].y || x > bounds->o[1].x || y > bounds->o[1].y)
		return 0;	// We are outside the outer rectangle so can't be inside the polygon
	
	// Check if we have only 2 points (rectangle) and are therefore don't need to do anything more
	if (bounds->p[2].x == -1 || bounds->p[2].y == -1 || bounds->p[3].x == -1 || bounds->p[3].y == -1)
		return 1;
	
	// Check if we are inside the 4 lines of the polygon
	for (i = 0; i < 4; i++)
	{
		j = i + 1;
		if (j == 4) j = 0;	// Wrap the next point var around to 0
		pX = bounds->p[i].x;
		pY = bounds->p[i].y;
		npX = bounds->p[j].x;
		npY = bounds->p[j].y;
	
		/* Psuedo-code to explain this block of nastiness
		
		If (this is not an internal point, OR
			(if the x coord of our test point is within the x bounds of the line with p <= np AND
				((the y coord is within the line with p <= np AND the point is within the y bounds of the line (bottom left quadrant)) OR
				 (the y coord is within the line with p > np AND the point is within the y bounds of the line (top left quadrant))
			) OR
			(if the x coord is within the x bounds of the line with p > np AND
				((the y coord is within the line with p <= np AND the point is within the y bounds of the line (bottom right quadrant)) OR
				 (the y coord is within the line with p > np AND the the point is within the y bounds of the line (top right quadrant))
			)
		) ...then we need to test the angle otherwise we can ignore this point
		*/
		if (bounds->int_point != i ||
			(pX <= npX && x >= pX && x <= pX &&
				((pY <= npY && y >= pY && y <= pY) ||
				 (pY >  npY && y <= pY && y >= pY))
			) ||
			(pX > npX && x <= pX && x >= pX &&
				((pY <= npY && y >= pY && y <= pY) ||
				 (pY >  npY && y <= pY && y >= pY))
			)
		)
		{
			result = test_bounds_angles(x, y, i, bounds);
			if (result == 0) return 0;
		}
	}
	
	// This point is inside the 4 lines
	return 1;
}

int test_bounds_angles(int x, int y, int point, map_sound_boundary_def * bounds)
{
	double a = calculate_bounds_angle(x, y, point, bounds);
	
	// If our angle for the test point is greater than the angle of the boundary line, then the point is outside
	if (a > bounds->p[point].a)
		return 0;
	return 1;
}

/* This function is the complex one!
 *
 * If you notice the pattern for the subsitutions below, it is because we are rotating the 0 degree angle around
 * the axis to keep it on the outside of the polygon. We need to do this because if our test line crosses from
 * > 2pi to < 2pi it will give a false positive.
 *
 * For point 0 the "illegal zone" is straight down, for point 1 horizontally left, for point 2 straight up, and
 * for point 3 horizontally right.
 *
 * This is why point 0 (or A in my diagrams) is the bottom left as nothing should cross it there, and so on for
 * the other points around the polygon. Each "illegal zone" should be pointing outside almost all permutations of
 * a polygon.
 *
 * If none of this makes sense then grab one of my drawings and if it still doesn't then ask. Grab me and ask.
 *  - Torg -
 *
 * Inputs:
 * 		x				The x coord of our test point
 * 		y				The y coord of our test point
 * 		point			The ID of the boundary point we are calculating from
 *		bounds			The boundary we are calculating this point of
 */
double calculate_bounds_angle(int x, int y, int point, map_sound_boundary_def * bounds)
{
	int A = 0,
		B = 0,
		C = 0,
		D = 0;
	double pi = 3.1415;
	double ra = pi / 2;		// ra = Right angle... meh
	double a;

	//  Set up the subsitutions for the actual equation. (This is such a waste of space. Grrr)
	switch (point)
	{
		case 0:
			// Check the angle of the line from the bottom left corner to our test point (point0 -> pointT)
			A = bounds->p[0].y;
			B = y;
			C = bounds->p[0].x;
			D = x;
			break;
		case 1:
			// Check the angle of the line from the top left corner to our test point (point1 -> pointT)
			A = bounds->p[1].x;
			B = x;
			C = y;
			D = bounds->p[1].y;
			break;
		case 2:
			// Check the angle of the line from the top right corner to our test point (point2 -> pointT)
			A = y;
			B = bounds->p[2].y;
			C = x;
			D = bounds->p[2].x;
			break;
		case 3:
			// Check the angle of the line from the bottom right corner to our test point (point3 -> pointT)
			A = x;
			B = bounds->p[3].x;
			C = bounds->p[3].y;
			D = y;
			break;
	}
	// If the line aligns to an axis, arctan has no value, so we need to check for this and use 90 degrees (pi / 2)
	// Otherwise calculate the angle and adjust the negative
	if (A == B && C < D) a = ra;			// If axis 1 is aligned and the coord on axis 2 of the first point is less
	else if (A == B && C > D) a = -ra;		// If axis 1 is aligned and the coord on axis 2 of the first point is greater
	else a = atan2((D - C), (A - B));		// If we aren't on the axis, find the angle
	if (D < C) a += pi * 2;					// If the second point is below the axis (-pi) then boost the angle to the positive (2pi)
											// This gives us a proper range from 0 to 2pi counter clockwise around the polygon

	return a;
}

#ifdef DEBUG_MAP_SOUND
// Print the sound area boundaries onto the tab-map (called from interface.c)
//
// It is a known bug that this _does not_ scale to the map selected. The scale will stay the same as your currently loaded map!
//
void print_sound_boundaries(int map)
{
	int i, i_max, j, id = -1, scale = 6, num_def = 0;
	char buf[100];
	map_sound_boundary_def *bounds;
	bound_point p[4];

	// Check if this map matches an array id
	for (i = 0; i < MAX_SOUND_MAPS; i++) {
		if (map == sound_map_data[i].id) {
			id = i;
			break;
		}
	}
	if (id == -1) return;	// Didn't find the map in our array so bail
	
	// count default boundary sounds - they do not have points
	for (i=0; i<sound_map_data[id].num_boundaries; i++)
		if (sound_map_data[id].boundaries[i].is_default)
			num_def++;
	
	glEnable (GL_TEXTURE_2D);
	glColor3f (1.0f, 1.0f, 1.0f);
	safe_snprintf(buf, sizeof(buf), "Map Num: %d, Array ID: %d, Map Name: %s\nNum Bound: %d (%d def), Num Walk: %d", map, id, sound_map_data[id].name, 
				  sound_map_data[id].num_boundaries, num_def, sound_map_data[id].num_walk_boundaries);
	draw_string_zoomed(25, 180, (unsigned char*)buf, 2, 0.4);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
	// Draw boundaries for this map
	for (j = 0; j < 2; j++) {
		if (j == 0) {
			i_max = sound_map_data[id].num_walk_boundaries;
		} else {
			i_max = sound_map_data[id].num_boundaries;
		}
		for (i = 0; i < i_max; i++) {
			if (j == 0) {
				bounds = &sound_map_data[id].walk_boundaries[i];
				glColor3f (0.0f, 0.0f, 1.0f);
			} else {
				bounds = &sound_map_data[id].boundaries[i];
				if (bounds->is_default)
					continue;
				glColor3f (1.0f, 0.0f, 0.0f);
			}
			if (bounds->p[2].x == -1 || bounds->p[2].y == -1 || bounds->p[3].x == -1 || bounds->p[3].y == -1)
			{
				p[0].x = 51 + 200 * bounds->p[0].x / (tile_map_size_x * scale);
				p[1].x = 51 + 200 * bounds->p[1].x / (tile_map_size_x * scale);
				p[2].x = 51 + 200 * bounds->p[1].x / (tile_map_size_x * scale);
				p[3].x = 51 + 200 * bounds->p[0].x / (tile_map_size_x * scale);
				p[0].y = 201 - 200 * bounds->p[0].y / (tile_map_size_y * scale);
				p[1].y = 201 - 200 * bounds->p[0].y / (tile_map_size_y * scale);
				p[2].y = 201 - 200 * bounds->p[1].y / (tile_map_size_y * scale);
				p[3].y = 201 - 200 * bounds->p[1].y / (tile_map_size_y * scale);
			}
			else
			{
				p[0].x = 51 + 200 * bounds->p[0].x / (tile_map_size_x * scale);
				p[1].x = 51 + 200 * bounds->p[1].x / (tile_map_size_x * scale);
				p[2].x = 51 + 200 * bounds->p[2].x / (tile_map_size_x * scale);
				p[3].x = 51 + 200 * bounds->p[3].x / (tile_map_size_x * scale);
				p[0].y = 201 - 200 * bounds->p[0].y / (tile_map_size_x * scale);
				p[1].y = 201 - 200 * bounds->p[1].y / (tile_map_size_x * scale);
				p[2].y = 201 - 200 * bounds->p[2].y / (tile_map_size_x * scale);
				p[3].y = 201 - 200 * bounds->p[3].y / (tile_map_size_x * scale);
			}

			glVertex2i(p[0].x, p[0].y);
			glVertex2i(p[1].x, p[1].y);

			glVertex2i(p[1].x, p[1].y);
			glVertex2i(p[2].x, p[2].y);

			glVertex2i(p[2].x, p[2].y);
			glVertex2i(p[3].x, p[3].y);

			glVertex2i(p[3].x, p[3].y);
			glVertex2i(p[0].x, p[0].y);
		}
	}
	glEnd();
}

// Prints the sound boundary coords for the input map to stdout
void print_sound_boundary_coords(int map)
{
	int i, i_max, j, id = -1;
	map_sound_boundary_def *bounds;

	// Check if this map matches an array id
	for (i = 0; i < MAX_SOUND_MAPS; i++) {
		if (map == sound_map_data[i].id) {
			id = i;
			break;
		}
	}
	if (id == -1) return;	// Didn't find the map in our array so bail
	
	printf("Map Num: %d, Array ID: %d, Map Name: %s, ", map, id, sound_map_data[id].name);
	// Print boundaries for this map
	for (j = 0; j < 2; j++) {
		if (j == 0) {
			i_max = sound_map_data[id].num_walk_boundaries;
			printf("Num Walk Boundaries: %d\n", i_max);
		} else {
			i_max = sound_map_data[id].num_boundaries;
			printf("Num Boundaries: %d\n", i_max);
		}
		for (i = 0; i < i_max; i++) {
			if (j == 0) {
				bounds = &sound_map_data[id].walk_boundaries[i];
			} else {
				bounds = &sound_map_data[id].boundaries[i];
			}
			printf("Outer - 1: %d, %d; 2: %d, %d\n", bounds->o[0].x, bounds->o[0].y, bounds->o[1].x, bounds->o[1].y);
			printf("Points - 1: %d, %d; 2: %d, %d; 3: %d, %d; 4: %d, %d\n", bounds->p[0].x, bounds->p[0].y, bounds->p[1].x, bounds->p[1].y
				   															, bounds->p[2].x, bounds->p[2].y, bounds->p[3].x, bounds->p[3].y);
		}
	}
}
#endif // DEBUG_MAP_SOUND



/******************
 * INIT FUNCTIONS *
 ******************/

void clear_variant(sound_variants *pVariant)
{
	int i;
	for (i = 0; i < num_STAGES; i++)
	{
		pVariant->part[i] = NULL;
	}
	pVariant->gain = 1.0f;
}

void clear_sound_type(int type)
{
	int i;
	sound_type * sound;

	if (type < 0 || type > MAX_SOUNDS)
		return;

	sound = &sound_type_data[type];

	sound->name[0] = '\0';
	for (i = 0; i < MAX_SOUND_VARIANTS; i++)
	{
		clear_variant(&sound->variant[i]);
	}
	sound->num_variants = 0;
	sound->stereo = 0;
	sound->distance = 100.0f;
	sound->positional = 1;
	sound->loops = 1;
	sound->fadeout_time = 0;
	sound->echo_delay = 0;
	sound->echo_volume = 50;
	sound->time_of_the_day_flags = 0xffff;
	sound->priority = 5;
	sound->type = SOUNDS_ENVIRO;
}

void clear_boundary_data(map_sound_boundary_def * pBoundary)
{
	int i;
	
	pBoundary->bg_sound = -1;
	pBoundary->crowd_sound = -1;
	pBoundary->time_of_day_flags = 0xffff;
	pBoundary->is_default = 0;
	for (i = 0; i < 4; i++)
	{
		pBoundary->p[i].x = -1;
		pBoundary->p[i].y = -1;
		pBoundary->p[i].a = -1;
	}
	pBoundary->o[0].x = -1;
	pBoundary->o[0].y = -1;
	pBoundary->o[1].x = -1;
	pBoundary->o[1].y = -1;
	pBoundary->int_point = -1;
}

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
		clear_sound_type(i);
	}
	for (i = 0; i < MAX_BUFFERS; i++)
	{
		unload_sample(i);
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
			clear_boundary_data(&sound_map_data[i].boundaries[j]);
		}
		sound_map_data[i].num_walk_boundaries = 0;
		for (j = 0; j < MAX_SOUND_WALK_BOUNDARIES; j++)
		{
			clear_boundary_data(&sound_map_data[i].walk_boundaries[j]);
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
	for (i = 0; i < 10; i++)
	{
		server_sound[i] = -1;
	}
	for (i = 0; i < NUM_ACTIVE_SPELLS; i++)
	{
		sound_spell_data[i] = -1;
	}
	for (i = 0; i < MAX_SOUND_WARNINGS; i++)
	{
		warnings_list[i].sound = -1;
		warnings_list[i].string[0] = '\0';
	}
	for (i = 0; i < MAX_SOUND_FILES; i++)
	{
		sound_files[i].file_path[0] = '\0';
		sound_files[i].sample_num = -1;
	}

	num_types = 0;
	num_samples = 0;
	sound_num_background_defaults = 0;
	sound_num_maps = 0;
	sound_num_effects = 0;
	sound_num_particles = 0;
	sound_num_items = 0;
	sound_num_tile_types = 0;
	num_sound_warnings = 0;
}

/* done once at start up to create the sound list mutex */
void initial_sound_init(void)
{
	sound_list_mutex = SDL_CreateMutex();
	if (!sound_list_mutex)
	{
		LOG_ERROR("Fatal error, unable to create sound list mutex: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}
	return;
}

/* done once at exit to delete the sound list mutex */
void final_sound_exit(void)
{
	SDL_DestroyMutex(sound_list_mutex);
	sound_list_mutex = NULL;
}

void init_sound()
{
	ALCcontext *context;
	ALCdevice *device;
	ALfloat listenerPos[3] = {0.0f, 0.0f, 0.0f};
	ALfloat listenerVel[3] = {0.0f, 0.0f, 0.0f};
	ALfloat listenerOri[6] = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	ALCchar *device_list;
	int error;
	int i;
	
	// If we don't have sound/music then bail so we don't grab the soundcard.
	if (inited || no_sound || (!sound_on && !music_on))
		return;
		
	// Begin by setting all data to a known state
	if (have_sound)
		destroy_sound();

	// Initialise OpenAL

	// Get a list of the available devices (not used yet)
	if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") != AL_TRUE)
	{
		LOG_WARNING("ALC_ENUMERATION_EXT not found. Retrieving list of sound devices may fail.");
	}
	device_list = (char*) alcGetString(NULL, ALC_DEVICE_SPECIFIER);
	parse_snd_devices(device_list, sound_devices);
	LOG_INFO("Sound devices detected: %s\n", sound_devices);

	// If you want to use a specific device, use, for example:
	// device = alcOpenDevice((ALCchar*) "DirectSound3D")
	// NULL makes it use the default device
	LOG_INFO("Soundcard device attempted: %s", sound_device);
	device = alcOpenDevice((ALCchar*) sound_device);
	if ((error = alcGetError(device)) != AL_NO_ERROR || !device)
	{
		if (strcmp(sound_device, "")) {
			// Try the default device
			device = alcOpenDevice(NULL);
		}
		if ((error = alcGetError(device)) != AL_NO_ERROR || !device)
		{
			char str[256];
			safe_snprintf(str, sizeof(str), "alcOpenDevice(): %s: %s\n", snd_init_error, alcGetString(device, error));
			LOG_TO_CONSOLE(c_red1, str);
			LOG_ERROR(str);
			have_sound = have_music = 0;
			return;
		} else {
			LOG_ERROR("Soundcard device specified (%s) failed. Using default device: %s", sound_device, alcGetString(device, ALC_DEVICE_SPECIFIER));
		}
	} else {
		LOG_INFO("Soundcard device in-use: %s", alcGetString(device, ALC_DEVICE_SPECIFIER));
	}

	context = alcCreateContext( device, NULL );
	alcMakeContextCurrent( context );
	if ((error = alcGetError(device)) != AL_NO_ERROR || !context)
	{
		char str[256];
		safe_snprintf(str, sizeof(str), "context: %s: %s\n", snd_init_error, alcGetString(device, error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound = have_music = 0;
		return;
	}

	// Setup the listener
	alListenerfv(AL_POSITION, listenerPos);
#ifdef _EXTRA_SOUND_DEBUG							// Debugging for Florian
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		LOG_DEBUG_VERBOSE("%s: Error setting up listener position - %s\n", snd_init_error, alGetString(error));
	}
#endif // _EXTRA_SOUND_DEBUG
	alListenerfv(AL_VELOCITY, listenerVel);
#ifdef _EXTRA_SOUND_DEBUG
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		LOG_DEBUG_VERBOSE("%s: Error setting up listener velocity - %s\n", snd_init_error, alGetString(error));
	}
#endif // _EXTRA_SOUND_DEBUG
	alListenerfv(AL_ORIENTATION, listenerOri);
#ifdef _EXTRA_SOUND_DEBUG
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		LOG_DEBUG_VERBOSE("%s: Error setting up listener orientation - %s\n", snd_init_error, alGetString(error));
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
		clear_source(&sound_source_data[i]);
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
				char str[256];
				safe_snprintf(str, sizeof(str), "alGenSources(): %s: %s - %s\n", snd_init_error, snd_source_error, alGetString(error));
				LOG_TO_CONSOLE(c_red1, str);
				LOG_ERROR(str);
				have_sound = have_music = 0;
				UNLOCK_SOUND_LIST();
				return;
			}
			break;
		}
	}
	UNLOCK_SOUND_LIST();
#ifdef _EXTRA_SOUND_DEBUG
	LOG_DEBUG_VERBOSE("Generated and using %d sources\n", max_sources);
#endif // _EXTRA_SOUND_DEBUG

	have_sound = 1;
	have_music = 1;

	// Initialise streams thread
	if (sound_streams_thread == NULL) {
		sound_streams_thread = SDL_CreateThread(update_streams, 0);
	}

	if (num_types == 0)
	{
		// We have no sounds defined so assume the config isn't loaded
		// As it isn't already loaded, assume the default config location
		load_sound_config_data(SOUND_CONFIG_PATH);
	}

	inited = 1;

	// Reset the error buffer
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		LOG_DEBUG_VERBOSE("%s: %s", snd_init_error, alGetString(error));
	}
}

void destroy_sound()
{
	int i, error;
	ALCcontext *context;
	ALCdevice *device = NULL;
	if (!inited){
		return;
	}
	inited = have_sound = have_music = 0;

	for (i = 0; i < MAX_STREAMS; i++)
	{
		destroy_stream(&streams[i]);
	}
	if (sound_streams_thread != NULL)
	{
		SDL_WaitThread(sound_streams_thread, NULL);
		sound_streams_thread = NULL;
	}
	// Remove physical elements (sources and buffers)
	LOCK_SOUND_LIST();
	for (i = 0; i < ABS_MAX_SOURCES; i++)
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
		unload_sample(i);
	}
	for (i = 0; i < MAX_BUFFERS * 2; i++)
	{
		if (no_sound) {
			unload_sound(i);
		} else {
			// Flag all sounds as unloaded, but don't remove them
			sounds_list[i].playing = 0;
			sounds_list[i].loaded = 0;
		}
	}
	UNLOCK_SOUND_LIST();

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


// This function is to convert the \0 delimiter used by OpenAL into a comma so we can use
// the device list as an everyday C "string" (original string terminated by \0\0).
// For now the existance of a comma in a device name is irrelevent.
void parse_snd_devices(ALCchar * in_array, char * sound_devs)
{
	int i = 0;
	while (in_array[i] != '\0' || in_array[i+1] != '\0') {
		if (in_array[i] == '\0') {
			sound_devs[i] = ',';
		} else {
			sound_devs[i] = in_array[i];
		}
		i++;
	}
	sound_devs[i] = '\0';
}

/********************
 * CONFIG FUNCTIONS *
 ********************/

// Returns -1 if the string is already in the list, 0 on error, 1 on success or -2 if there are no more list slots
int add_to_sound_warnings_list(const char * text)
{
	int i, snd;
	char left[256];
	char right[256];
	int t, tp;
	
	if (text[0] == '\0' || text[0] == '#')
		return 1;		// Nothing to do so return success

	// Extract the sound name (left) and string (right) from the input text
	safe_strncpy (left, text, sizeof(left));
	for (t = 0; ; t++)
	{
		if (left[t] == '\0')
		{
			LOG_ERROR("Invalid sound warning declared: %s. Expected format 'sound = string'.", text);
			return 0;
		}
		if (left[t] == '=')
		{
			left[t] = '\0';
			for (tp = t - 1; tp >= 0 && isspace(left[tp]); tp--)
			{
				left[tp] = '\0';
			}
			for (tp = t + 1; left[tp] != '\0' && !(left[tp]&0x80) && isspace(left[tp]); tp++) ;
			safe_strncpy (right, &left[tp], sizeof(right));
			break;
		}
	}
	
	// See if this string is already in the list
	for (i = 0; i < MAX_SOUND_WARNINGS; i++)
	{
		if (warnings_list[i].sound >= 0)
		{
			if (my_strcompare (warnings_list[i].string, right))
				return -1; // Already in the list
		}
	}

	snd = get_index_for_sound_type_name(left);
	if (snd > -1)
	{
		// We have a valid sound so find a free spot
		for (i = 0; i < MAX_SOUND_WARNINGS; i++)
		{
			if (warnings_list[i].sound < 0)
			{
				// Excellent, a free spot
				warnings_list[i].sound = snd;
				safe_strncpy(warnings_list[i].string, right, sizeof(warnings_list[i].string));
				num_sound_warnings++;
				return 1;
			}
		}
		LOG_ERROR("Sound warning list is full. %d warnings loaded.", num_sound_warnings);
		return -2;		// If we are here, it means the warnings list is full
	}
	
	LOG_ERROR("Sound not found for sound warning: %s", text);
	return 0;		// The sound wasn't found
}

// Blatently stolen from the text filter code (filter.c)
void load_sound_warnings_list(const char *filename)
{
	int f_size;
	FILE * f = NULL;
	char * sound_warnings_list_mem;
	int istart, iend;
	char string[128];

	f = open_file_config (filename, "rb");
	if (f == NULL) return;

	// Ok, allocate memory for it and read it in
	fseek(f, 0, SEEK_END);
	f_size = ftell(f);
	if (f_size <= 0)
	{
		fclose(f);
		return;
	}
	
	sound_warnings_list_mem = (char *) calloc (f_size, 1);
	fseek(f, 0, SEEK_SET);
	if (fread(sound_warnings_list_mem, 1, f_size, f) != f_size)
	{
		LOG_ERROR("%s() read failed for file [%s]\n", __FUNCTION__, filename);
		free(sound_warnings_list_mem);
		fclose(f);
		return;
	}
	fclose(f);

	istart = 0;
	while (istart < f_size)
	{
		// Find end of the line
		for (iend = istart; iend < f_size; iend++)
		{
			if (sound_warnings_list_mem[iend] == '\n' || sound_warnings_list_mem[iend] == '\r')
				break;
		}

		// Copy the line and process it
		if (iend > istart)
		{
			safe_strncpy2(string, sound_warnings_list_mem+istart, sizeof(string), iend - istart);
			if (add_to_sound_warnings_list(string) == -2)		// -1 == already exists, -2 == list full
			{
				free(sound_warnings_list_mem);
				return; // Sound warnings list full
			}
		}

		// Move to next line
		istart = iend + 1;
	}

	free(sound_warnings_list_mem);
}



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

sound_file * init_sound_file(const char * content)
{
	int i;
	
	// Check if this file is already in our list of sound files
	for (i = 0; i < num_sound_files; i++)
	{
		if (!strcasecmp(sound_files[i].file_path, content)) {
			return &sound_files[i];
		}
	}
	// Not found so check we have space to add it to the list
	if (num_sound_files >= MAX_SOUND_FILES)
	{
		// Big problem! No more room to load sound files
		return NULL;
	}
	// Everything is ok so load it
	safe_strncpy(sound_files[i].file_path, content, sizeof(sound_files[i].file_path));
	num_sound_files++;
	return &sound_files[i];
}

sound_file * load_sound_part(sound_file *pPart, SOUND_STAGE stage, const char * content)
{
	char filename[200];
	char stage_name[6];
	
	safe_strncpy(stage_name, (stage == STAGE_INTRO ? "Intro" : (stage == STAGE_MAIN ? "Main" : "Outro")), sizeof(stage_name));

	if (!pPart || !strcasecmp(pPart->file_path, ""))
	{
		strcpy(filename, datadir);
		strcat(filename, content);
		if (file_exists(filename))
		{
			pPart = init_sound_file(content);
			if (pPart)
			{
				return pPart;
			}
			LOG_ERROR("%s: Too many sound files loaded!", snd_config_error);
		}
		else
		{
			LOG_ERROR("%s: %s stage file does not exist! %s", snd_config_error, stage_name, content);
		}
	}
	else
	{
		LOG_ERROR("%s: %s stage file already set!", snd_config_error, stage_name);
		return pPart;
	}
	return NULL;
}

void parse_sound_variant(const xmlNode *inNode, sound_type *inType)
{
	char content[50];
	float fVal = 0.0f;
	sound_variants * pData;
	const xmlNode *attributeNode;

	if (inType->num_variants >= MAX_SOUND_VARIANTS)
	{
		LOG_ERROR("%s: Too many sound variants defined for this sound type: %s", snd_config_error, inType->name);
		return;
	}

	pData = &inType->variant[inType->num_variants++];
	attributeNode = inNode->xmlChildrenNode;
	while (attributeNode != NULL)
	{
		if (attributeNode->type == XML_ELEMENT_NODE)
		{
			get_string_value(content, sizeof(content), attributeNode);
			if (!xmlStrcmp (attributeNode->name, (xmlChar*)"intro_sound"))
			{
				pData->part[STAGE_INTRO] = load_sound_part(pData->part[STAGE_INTRO], STAGE_INTRO, content);
			}
			else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"main_sound"))
			{
				pData->part[STAGE_MAIN] = load_sound_part(pData->part[STAGE_MAIN], STAGE_MAIN, content);
				if (!pData->part[STAGE_MAIN])
				{
					// We don't have a main stage file, therefore this variant is invalid!
					LOG_ERROR("%s: We do not have a main stage file so this variant for sound type '%s' is disabled", snd_config_error, inType->name);
					clear_variant(pData);
					inType->num_variants--;
					return;
				}
			}
			else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"outro_sound"))
			{
				pData->part[STAGE_OUTRO] = load_sound_part(pData->part[STAGE_OUTRO], STAGE_OUTRO, content);
			}
			else if (!xmlStrcmp (attributeNode->name, (xmlChar*)"gain"))
			{
				fVal = (float)atof((char *)content);
				if (fVal > 0.0f)
					pData->gain = fVal;
				else
				{
					LOG_ERROR("%s: Invalid gain = %s in '%s'", snd_config_error, content, inType->name);
				}
			}
		}
		else if (attributeNode->type == XML_ENTITY_REF_NODE)
		{
			LOG_ERROR("%s: Include not allowed in variant sound def", snd_config_error);
		}
		attributeNode = attributeNode->next;
	}
	// Check this variant has the required Main stage
	if (!pData->part[STAGE_MAIN])
	{
		LOG_ERROR("%s: Main stage not defined! A sound variant for this type is disabled: %s", snd_config_error, inType->name);
		clear_variant(pData);
		inType->num_variants--;
	}
	return;
}

void parse_sound_object(const xmlNode *inNode)
{
	const xmlNode *attributeNode = NULL;

	char content[50];
	int iVal = 0;
	float fVal = 0.0f;
	xmlChar *sVal = NULL;

	sound_type *pData = NULL;

	if (num_types >= MAX_SOUNDS)
	{
		LOG_ERROR("%s: Maximum number of sounds (%d) reached!", snd_config_error, MAX_SOUNDS);
		return;
	}

	pData = &sound_type_data[num_types++];

	sVal = xmlGetProp((xmlNode *)inNode, (const xmlChar*)"name");
	if (!sVal)
	{
		LOG_ERROR("%s: sound has no name", snd_config_error);
	}
	else
	{
		safe_strncpy(pData->name, (const char*)sVal, sizeof(pData->name));
		xmlFree(sVal);

		attributeNode = inNode->xmlChildrenNode;
		while (attributeNode != NULL)
		{
			if (attributeNode->type == XML_ELEMENT_NODE)
			{
				get_string_value(content, sizeof(content), attributeNode);
				if (!xmlStrcmp (attributeNode->name, (xmlChar*)"variant"))
				{
					parse_sound_variant(attributeNode, pData);
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
					iVal = SOUNDS_NONE;
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
					} else if (!strcasecmp((char *)content, "warnings")) {
						iVal = SOUNDS_WARNINGS;
					} else {
						LOG_ERROR("%s: Unknown type '%s' for sound '%s'", snd_config_error, content, pData->name);
					}
					if (iVal != SOUNDS_NONE)
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
	// Check this type has at least one variant
	if (pData->num_variants < 1)
	{
		LOG_ERROR("%s: No sound variants defined! This sound type is disabled: %s", snd_config_error, pData->name);
		clear_sound_type(num_types);
		num_types--;
	}
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

int validate_boundary(map_sound_boundary_def * bounds, char * map_name)
{
	int i;
	double a;

	// Check if this is a default
	if (bounds->is_default)
	{
		// Check if we have any points
		if (bounds->p[0].x != -1 || bounds->p[0].y != -1 || bounds->p[1].x != -1 || bounds->p[1].y != -1
			|| bounds->p[2].x != -1 || bounds->p[2].y != -1 || bounds->p[3].x != -1 || bounds->p[3].y != -1)
		{
			LOG_ERROR("Warning: Points defined for default boundary. Points will be ignored.\n");
		}
		return 1;		// Points are ignored for defaults
	}
	
	// Check we have details for at least 2 points
	if (bounds->p[0].x == -1 || bounds->p[0].y == -1 || bounds->p[1].x == -1 || bounds->p[1].y == -1)
	{
		LOG_ERROR("%s: Point missing for boundary in map '%s'. Non-default boundaries must contain 2 or 4 points.", snd_config_error, map_name);
		return 0;
	}
	
	// Calculate the outer box
	bounds->o[0].x = bounds->p[0].x;
	bounds->o[0].y = bounds->p[0].y;
	bounds->o[1].x = bounds->p[0].x;
	bounds->o[1].y = bounds->p[0].y;
	for (i = 0; i < 4; i++)
	{
		if (bounds->p[i].x > -1 && bounds->p[i].y > -1)
		{
			// Check if this point's x or y is greater than the current max or less than the current min and update
			if (bounds->p[i].x < bounds->o[0].x)
				bounds->o[0].x = bounds->p[i].x;
			if (bounds->p[i].y < bounds->o[0].y)
				bounds->o[0].y = bounds->p[i].y;
			if (bounds->p[i].x > bounds->o[1].x)
				bounds->o[1].x = bounds->p[i].x;
			if (bounds->p[i].y > bounds->o[1].y)
				bounds->o[1].y = bounds->p[i].y;
		}
	}

	// We have the outer box now so if we have only 2 points then we can bail	
	if (bounds->p[2].x == -1 && bounds->p[2].y == -1 && bounds->p[3].x == -1 && bounds->p[3].y == -1)
	{
		return 1;
	}
	else if (bounds->p[2].x == -1 || bounds->p[2].y == -1 || bounds->p[3].x == -1 || bounds->p[3].y == -1)
	{
		LOG_ERROR("%s: Point missing for boundary in map '%s'. Non-default boundaries must contain 2 or 4 points.", snd_config_error, map_name);
		return 0;
	}
	
	// Check if our 4 points are actually equal to the bounding box rectangle
	if (bounds->p[0].x == bounds->o[0].x && bounds->p[0].y == bounds->o[0].y &&
		bounds->p[1].x == bounds->o[0].x && bounds->p[1].y == bounds->o[1].y &&
		bounds->p[2].x == bounds->o[1].x && bounds->p[2].y == bounds->o[1].y &&
		bounds->p[3].x == bounds->o[1].x && bounds->p[3].y == bounds->o[0].y)
	{
		// This is the bounding box so remove points 1 and 2 as the corners and blank the others
		// to symbolise using the bounding box check only
		bounds->p[1].x = bounds->p[2].x;
		bounds->p[1].y = bounds->p[2].y;
		bounds->p[2].x = -1;
		bounds->p[2].y = -1;
		bounds->p[3].x = -1;
		bounds->p[3].y = -1;
		// Nothing left to do
		return 1;
	}

	// Find the angle of the line from the top left corner to the top right (point1 -> point2)
	bounds->p[0].a = calculate_bounds_angle(bounds->p[1].x, bounds->p[1].y, 0, bounds);

	// Find the angle of the line from the top right corner to the bottom right (point2 -> point3)
	bounds->p[1].a = calculate_bounds_angle(bounds->p[2].x, bounds->p[2].y, 1, bounds);

	// Find the angle of the line from the bottom right corner to the bottom left (point3 -> point4)
	bounds->p[2].a = calculate_bounds_angle(bounds->p[3].x, bounds->p[3].y, 2, bounds);
	
	// Find the angle of the line from the bottom left corner to the top left (point4 -> point1)
	bounds->p[3].a = calculate_bounds_angle(bounds->p[0].x, bounds->p[0].y, 3, bounds);


	// Check the angle of the line from the bottom left corner to the top right (point4 -> point2)
	a = calculate_bounds_angle(bounds->p[1].x, bounds->p[1].y, 3, bounds);
	if (bounds->p[3].a < a) bounds->int_point = 0;

	// Check the angle of the line from the top left corner to the bottom right (point1 -> point3)
	a = calculate_bounds_angle(bounds->p[2].x, bounds->p[2].y, 0, bounds);
	if (bounds->p[0].a < a) bounds->int_point = 1;

	// Check the angle of the line from the top right corner to the bottom left (point2 -> point4)
	a = calculate_bounds_angle(bounds->p[3].x, bounds->p[3].y, 1, bounds);
	if (bounds->p[1].a < a) bounds->int_point = 2;

	// Check the angle of the line from the bottom right corner to the top left (point3 -> point1)
	a = calculate_bounds_angle(bounds->p[0].x, bounds->p[0].y, 2, bounds);
	if (bounds->p[2].a < a) bounds->int_point = 3;

	return 1;
}

void parse_map_sound(const xmlNode *inNode)
{
	const xmlNode *boundaryNode = NULL;
	const xmlNode *attributeNode = NULL;

	xmlChar *sVal = NULL;
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

	sVal = xmlGetProp((xmlNode*)inNode,(const xmlChar*)"id");
	if (!sVal)
	{
		pMap->id = -1;
		LOG_ERROR("%s: map has no id", snd_config_error);
	}
	else
	{
		pMap->id = atoi((const char*)sVal);
		xmlFree(sVal);

		sVal = xmlGetProp((xmlNode*)inNode, (const xmlChar*)"name");
		if (sVal)
		{
			safe_strncpy(pMap->name, (const char*)sVal, sizeof(pMap->name));
			xmlFree(sVal);
		}

		for (boundaryNode = inNode->children; boundaryNode; boundaryNode = boundaryNode->next)
		{
			if (boundaryNode->type == XML_ELEMENT_NODE)
			{
				if(!xmlStrcasecmp(boundaryNode->name, (xmlChar*)"boundary_def"))
				{
					// Process this set of boundaries
					if (pMap->num_boundaries++ < MAX_SOUND_MAP_BOUNDARIES)
					{
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
									LOG_ERROR("%s: background sound not found for map boundary type '%s' in map '%s'", snd_config_error,content, pMap->name);
								}
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"crowd"))
							{
								// Find the type of crowd sound for this set of boundaries
								pMapBoundary->crowd_sound = get_index_for_sound_type_name(content);
								if (pMapBoundary->crowd_sound == -1)
								{
									LOG_ERROR("%s: crowd sound not found for map boundary type '%s' in map '%s'", snd_config_error, content, pMap->name);
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
									LOG_ERROR("%s: time_of_day flags (%s) invalid for map boundary in map '%s'", snd_config_error, content, pMap->name);
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
								store_boundary_coords(content, &pMapBoundary->p[0].x, &pMapBoundary->p[0].y);
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point2"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->p[1].x, &pMapBoundary->p[1].y);
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point3"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->p[2].x, &pMapBoundary->p[2].y);
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point4"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->p[3].x, &pMapBoundary->p[3].y);
							}
						}
						// Validate the boundary
						if (!validate_boundary(pMapBoundary, pMap->name))
						{
							// This one is invalid so remove it (we have already errored)
							clear_boundary_data(pMapBoundary);
							pMap->num_boundaries--;
						}
					}
					else
					{
						LOG_ERROR("%s: reached max boundaries for map '%s'", snd_config_error, pMap->name);
					}
				}
// This block is a temporary fix for detecting 2d and 3d objects. It can be removed once the other functionality is coded.
				else if(!xmlStrcasecmp(boundaryNode->name, (xmlChar*)"walk_boundary_def"))
				{
					// Process this set of boundaries
					if (pMap->num_walk_boundaries++ < MAX_SOUND_WALK_BOUNDARIES)
					{
						pMapBoundary = &pMap->walk_boundaries[pMap->num_walk_boundaries - 1];

						for (attributeNode = boundaryNode->children; attributeNode; attributeNode = attributeNode->next)
						{
							get_string_value(content, sizeof(content), attributeNode);
							if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"sound"))
							{
								// Find the sound for this set of boundaries
								pMapBoundary->bg_sound = get_index_for_sound_type_name(content);
								if (pMapBoundary->bg_sound == -1)
								{
									LOG_ERROR("%s: sound not found for walk boundary type '%s' in map '%s'", snd_config_error, content, pMap->name);
								}
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"time_of_day_flags"))
							{
								// Find the type of time of day flags for this set of boundaries
								sscanf((char *)content, "%x", &iVal);
								if (iVal >= 0 && iVal <= 0xffff)
									pMapBoundary->time_of_day_flags = iVal;
								else
								{
									LOG_ERROR("%s: time_of_day flags (%s) invalid for walk boundary in map '%s'", snd_config_error, content, pMap->name);
								}
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point1"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->p[0].x, &pMapBoundary->p[0].y);
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point2"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->p[1].x, &pMapBoundary->p[1].y);
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point3"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->p[2].x, &pMapBoundary->p[2].y);
							}
							else if (!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point4"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->p[3].x, &pMapBoundary->p[3].y);
							}
						}
						// Validate the boundary
						if (!validate_boundary(pMapBoundary, pMap->name))
						{
							// This one is invalid so remove it (we have already errored)
							clear_boundary_data(pMapBoundary);
							pMap->num_walk_boundaries--;
						}
					}
					else
					{
						LOG_ERROR("%s: reached max walk boundaries for map '%s'", snd_config_error, pMap->name);
					}
				}
// This block ^^ is a temporary fix for detecting 2d and 3d objects. It can be removed once the other functionality is coded.
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

void parse_effect_sound(const xmlNode *inNode)
{
	const xmlNode *attributeNode = NULL;
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

void parse_particle_sound(const xmlNode *inNode)
{
	const xmlNode *attributeNode = NULL;
	particle_sound_data *pParticle = NULL;
	char sound[MAX_SOUND_NAME_LENGTH] = "";

	if (inNode->type == XML_ELEMENT_NODE)
	{
		if (sound_num_particles >= MAX_SOUND_PARTICLES)
		{
			LOG_ERROR("%s: Maximum number of particles reached!", snd_config_error);
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

void parse_background_defaults(const xmlNode *inNode)
{
	const xmlNode *attributeNode = NULL;
	background_default *pBackgroundDefault = NULL;
	char content[MAX_SOUND_NAME_LENGTH] = "";

	int iVal = 0;
	
	if (inNode->type == XML_ELEMENT_NODE)
	{
		if (sound_num_background_defaults >= MAX_BACKGROUND_DEFAULTS)
		{
			LOG_ERROR("%s: Maximum number of background defaults reached!", snd_config_error);
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

void parse_sound_defaults(const xmlNode *inNode)
{
	const xmlNode *attributeNode = NULL;
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
	char temp[5] = "";
	
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
	// Add anything remaining as a last image id
	if (pItem->num_imageids < MAX_ITEM_SOUND_IMAGE_IDS)
	{
		pItem->image_id[pItem->num_imageids++] = atoi(temp);
	}
	else
	{
		LOG_ERROR("%s: Too many image ids defined for item sound: %s", snd_config_error, content);
	}
	return;
}

void parse_item_sound(const xmlNode *inNode)
{
	const xmlNode *attributeNode = NULL;
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
	char temp[5] = "";
	
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
	// Add anything remaining as a last tile type
	if (pTileType->num_tile_types < MAX_SOUND_TILES)
	{
		pTileType->tile_type[pTileType->num_tile_types++] = atoi(temp);
	}
	else
	{
		LOG_ERROR("%s: Too many tile types defined for tile type sound: %s, max: %d", snd_config_error, content, MAX_SOUND_TILES);
	}
	return;
}

void parse_tile_type_sound(const xmlNode *inNode)
{
	const xmlNode *attributeNode = NULL;
	char content[1024] = "";
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

void parse_spell_sound(const xmlNode *inNode)
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

int parse_sound_defs(const xmlNode *node)
{
	const xmlNode *def;
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
	xmlDoc *doc;
	const xmlNode *root = NULL;

	if (no_sound)
		return;

	if (!el_file_exists(file))
		return;

	if ((doc = xmlReadFile(file, NULL, XML_PARSE_NOENT)) == NULL)
	{
		char str[200];
		safe_snprintf(str, sizeof(str), snd_config_open_err_str, file);
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
		have_sound_config = 1;
		clear_sound_data();
		parse_sound_defs(root);
		parse_server_sounds();
		load_sound_warnings_list(SOUND_WARNINGS_PATH);
	}

	xmlFreeDoc(doc);
#ifdef DEBUG
	print_sound_types();
#endif // DEBUG

}

/***********************
 * DEBUGGING FUNCTIONS *
 ***********************/

#ifdef DEBUG
void print_sound_types()
{
	int i, j;
	sound_type *pData = NULL;
	map_sound_data *pMap = NULL;
	map_sound_boundary_def *pMapBoundary = NULL;
	effect_sound_data *pEffect = NULL;
	particle_sound_data *pParticle = NULL;
	item_sound_data *pItem = NULL;
	tile_sound_data *pTileType = NULL;
	
	printf("\nSOUND TYPE DATA\n===============\n");
	printf("There are %d sound types (max %d):\n", num_types, MAX_SOUNDS);
	for (i = 0; i < num_types; ++i)
	{
		pData = &sound_type_data[i];
		printf("Sound type '%s' #%d:\n"			, pData->name, i);
		for (j = 0; j < pData->num_variants; ++j)
		{
			printf("\tVariant %d/%d:\n"				, j, pData->num_variants);
			printf("\t\tIntro sample = '%s'\n"		, (pData->variant[j].part[STAGE_INTRO] ? pData->variant[j].part[STAGE_INTRO]->file_path : "(none)"));
			printf("\t\tMain sample = '%s'\n"		, (pData->variant[j].part[STAGE_MAIN] ? pData->variant[j].part[STAGE_MAIN]->file_path : "(none)"));
			printf("\t\tOutro sample = '%s'\n"		, (pData->variant[j].part[STAGE_OUTRO] ? pData->variant[j].part[STAGE_OUTRO]->file_path : "(none)"));
			printf("\t\tGain = %f\n"				, pData->variant[j].gain);
		}
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
	for (i = 0; i < sound_num_maps; ++i)
	{
		pMap = &sound_map_data[i];
		printf("Map id: %d\n"				, pMap->id);
		printf("Map name: %s\n"				, pMap->name);
		printf("Num boundaries: %d\n"		, pMap->num_boundaries);
		for (j = 0; j < pMap->num_boundaries; ++j)
		{
			pMapBoundary = &pMap->boundaries[j];
			printf("Boundary num: %d\n"			, j);
			printf("\tBackground sound: %d\n"	, pMapBoundary->bg_sound);
			printf("\tCrowd sound: %d\n"		, pMapBoundary->crowd_sound);
			printf("\tTime of day flags: 0x%x\n", pMapBoundary->time_of_day_flags);
			printf("\tDefault: %s\n"			, pMapBoundary->is_default == 0 ? "No" : "Yes");
			printf("\tX1, Y1, A1: (%d, %d, %f), X2, Y2, A2: (%d, %d, %f), X3, Y3, A3: (%d, %d, %f), X4, Y4, A4: (%d, %d, %f)\n",
				pMapBoundary->p[0].x, pMapBoundary->p[0].y, pMapBoundary->p[0].a,
				pMapBoundary->p[1].x, pMapBoundary->p[1].y, pMapBoundary->p[1].a,
				pMapBoundary->p[2].x, pMapBoundary->p[2].y, pMapBoundary->p[2].a,
				pMapBoundary->p[3].x, pMapBoundary->p[3].y, pMapBoundary->p[3].a);
			printf("\tOuter box - X1, Y1, A1: (%d, %d, %f); X2, Y2, A1: (%d, %d, %f)\n",
				pMapBoundary->o[0].x, pMapBoundary->o[0].y, pMapBoundary->o[0].a,
				pMapBoundary->o[1].x, pMapBoundary->o[1].y, pMapBoundary->o[1].a);
			printf("\tInternal point: %d\n"		, pMapBoundary->int_point);
		}
		printf("\n");
		printf("Num walk boundaries: %d\n"		, pMap->num_walk_boundaries);
		for (j = 0; j < pMap->num_walk_boundaries; ++j)
		{
			pMapBoundary = &pMap->walk_boundaries[j];
			printf("Walk boundary num: %d\n"	, j);
			printf("\tWalk sound: %d\n"			, pMapBoundary->bg_sound);
			printf("\tTime of day flags: 0x%x\n", pMapBoundary->time_of_day_flags);
			printf("\tX1, Y1, A1: (%d, %d, %f), X2, Y2, A2: (%d, %d, %f), X3, Y3, A3: (%d, %d, %f), X4, Y4, A4: (%d, %d, %f)\n",
				pMapBoundary->p[0].x, pMapBoundary->p[0].y, pMapBoundary->p[0].a,
				pMapBoundary->p[1].x, pMapBoundary->p[1].y, pMapBoundary->p[1].a,
				pMapBoundary->p[2].x, pMapBoundary->p[2].y, pMapBoundary->p[2].a,
				pMapBoundary->p[3].x, pMapBoundary->p[3].y, pMapBoundary->p[3].a);
			printf("\tOuter box - X1, Y1, A1: (%d, %d, %f); X2, Y2, A1: (%d, %d, %f)\n",
				pMapBoundary->o[0].x, pMapBoundary->o[0].y, pMapBoundary->o[0].a,
				pMapBoundary->o[1].x, pMapBoundary->o[1].y, pMapBoundary->o[1].a);
			printf("\tInternal point: %d\n"		, pMapBoundary->int_point);
		}
		printf("\n");
	}
	
	printf("\nEFFECT SOUND DATA\n===============\n");
	printf("There are %d effect sounds:\n", sound_num_effects);
	for (i = 0; i < sound_num_effects; ++i)
	{
		pEffect = &sound_effect_data[i];
		printf("Effect ID: %d\n"		, pEffect->id);
		printf("Sound: %d\n"			, pEffect->sound);
	}
	
	printf("\nPARTICLE SOUND DATA\n===============\n");
	printf("There are %d particle sounds:\n", sound_num_particles);
	for (i = 0; i < sound_num_particles; ++i)
	{
		pParticle = &sound_particle_data[i];
		printf("Particle file: %s\n"	, pParticle->file);
		printf("Sound: %d\n"			, pParticle->sound);
	}
	
	printf("\nITEM SOUND DATA\n===============\n");
	printf("There are %d item sounds:\n", sound_num_items);
	for (i = 0; i < sound_num_items; ++i)
	{
		pItem = &sound_item_data[i];
		printf("Sound: %d\n"				, pItem->sound);
		printf("Image ID's (%d total):\n"	, pItem->num_imageids);
		for (j = 0; j < pItem->num_imageids; ++j)
		{
			printf("%d, "					, pItem->image_id[j]);
		}
		printf("\n");
	}

	printf("\nTILE (WALKING) SOUND DATA\n===============\n");
	printf("There are %d tile type sounds:\n", sound_num_tile_types);
	for (i = 0; i < sound_num_tile_types; ++i)
	{
		pTileType = &sound_tile_data[i];
		printf("Tile: %d\n"							, i);
		printf("\tDefault sound: %d\n"				, pTileType->default_sound);
		printf("\tTile types (%d total):\n\t\t"		, pTileType->num_tile_types);
		for (j = 0; j < pTileType->num_tile_types; ++j)
		{
			printf("%d, "							, pTileType->tile_type[j]);
		}
		printf("\n\tNum sounds: %d\n"				, pTileType->num_sounds);
		for (j = 0; j < pTileType->num_sounds; ++j)
		{
			printf("\t\tSound: %d\n"				, pTileType->sounds[j].sound);
			printf("\t\tActor types: %s\n"			, pTileType->sounds[j].actor_types);
		}
	}

	printf("\nSPELL EFFECT SOUND DATA\n===============\n");
	printf("There are 10 spell effects:\n");
	for (i = 0; i <= 9; ++i)
	{
		printf("Spell Effect Sound: %d = %d\n", i, sound_spell_data[i]);
	}

	printf("\nSERVER SOUNDS\n===============\n");
	printf("There are 10 server sounds:\n");
	for (i = 0; i <= 9; ++i)
	{
		printf("Server Sound: %d = %d\n", i, server_sound[i]);
	}

	printf("\nTEXT WARNING SOUNDS\n===============\n");
	printf("There are %d/%d text warning sounds:\n", num_sound_warnings, MAX_SOUND_WARNINGS);
	for (i = 0; i < num_sound_warnings; ++i)
	{
		printf("Text Warning: %d\n", i);
		printf("\tText: %s\n", warnings_list[i].string);
		printf("\tSound: %d\n", warnings_list[i].sound);
	}
}

void print_sound_samples()
{
	int i;
	sound_sample *pData=NULL;
	printf("\nSOUND SAMPLE DATA\n===============\n");
	printf("There are %d sound samples loaded (max %d):\n", num_samples, MAX_BUFFERS);
	printf("(skipped sample numbers are buffers that have been used and released)\n");

	for (i = 0; i < MAX_BUFFERS; ++i)
	{
		pData = &sound_sample_data[i];
		if (alIsBuffer(pData->buffer))
		{
			printf("Sample %d:\n"			, i);
			printf("\tBuffer ID = %d\n"		, pData->buffer);
			printf("\tSize = %d\n"			, pData->size);
			printf("\tFrequency = %f\n"		, pData->freq);
			printf("\tChannels = %d\n"		, pData->channels);
			printf("\tBits = %d\n"			, pData->bits);
			printf("\tLength = %dms\n\n"	, pData->length);
		}
	}
}

void print_sounds_list()
{
	int i;
	sound_loaded *pData = NULL;
	printf("\nLOADED SOUND DATA\n===============\n");
	printf("There are %d loaded sounds (max %d):\n", num_sounds, MAX_BUFFERS * 2);

	for (i = 0; i < num_sounds; ++i)
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

#endif //_DEBUG
#endif // !NEW_SOUND

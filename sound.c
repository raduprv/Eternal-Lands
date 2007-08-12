#include <ctype.h>
#include <string.h>
#include <SDL.h>
#include <SDL_thread.h>
#ifdef NEW_SOUND
#include <math.h>
#endif // NEW_SOUND
#include "sound.h"
#include "asc.h"
#include "draw_scene.h"
#include "errors.h"
#include "init.h"
#include "lights.h"
#include "misc.h"
#include "translate.h"
#include "weather.h"
#include "io/map_io.h"
#ifdef	NEW_FILE_IO
#include "io/elpathwrapper.h"
#include "io/elfilewrapper.h"
#endif	//NEW_FILE_IO
#ifdef NEW_SOUND
#include "actors.h"
#endif //NEW_SOUND

#define MAX_FILENAME_LENGTH 80
#define MAX_BUFFERS 56			// Remember, music and bg sounds use 4 buffers each too
#define MAX_SOURCES 14			// Remember, music and bg sounds use a source each too

#ifdef NEW_SOUND
#define OGG_BUFFER_SIZE (1048576)
#define STREAM_BUFFER_SIZE (4096 * 16)
#else // NEW_SOUND
#define MUSIC_BUFFER_SIZE (4096 * 16)
#endif // NEW_SOUND
#define SLEEP_TIME 500

#define	LOCK_SOUND_LIST() SDL_LockMutex(sound_list_mutex);
#define	UNLOCK_SOUND_LIST() SDL_UnlockMutex(sound_list_mutex);

typedef struct {
	char file_name[64];
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int time;
} playlist_entry;


#ifdef NEW_SOUND
#define STREAM_TYPE_MUSIC 1
#define STREAM_TYPE_SOUNDS 0

#define MAX_SOUND_MAP_NAME_LENGTH 60
#define MAX_SOUND_MAP_BOUNDARIES 20
#define MAX_SOUND_MAPS 150			// This value is the maximum number of maps sounds can be defined for
									// (Roja has suggested 150 is safe for now)
#define MAX_SOUND_EFFECTS 60		// This value should equal the max special_effect_enum
#define MAX_SOUND_PARTICLES 20		// This value should equal the number of particle effects

typedef enum{STAGE_UNUSED=-1,STAGE_INTRO,STAGE_MAIN,STAGE_OUTRO,num_STAGES}SOUND_STAGE;
typedef struct
{
	char name[MAX_SOUND_NAME_LENGTH];
	int sample_indices[num_STAGES];	// The indices of the samples used
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
}sound_type;

typedef struct
{
	ALuint buffer;							//if the sample is loaded, a buffer ID to play it.
	char file_path[MAX_FILENAME_LENGTH];	//where to load the file from
	ALenum format;
	ALsizei size;							//size of the sound data in bytes
#ifdef ALUT_WAV
	ALsizei freq;							//frequency
#else // ALUT_WAV
	ALfloat freq;							//frequency
#endif // ALUT_WAV
	ALint channels;							//number fo sound channels
	ALint bits;								//bits per channel per sample
	int length;								//duration in milliseconds
	
	int loaded_status;						//0 - not loaded, 1 - loaded
}sound_sample;

typedef struct
{
	ALuint source;
	int play_duration;
	int sound_type;
	SOUND_STAGE current_stage;

	unsigned int cookie;
}source_data;

typedef struct
{
	int sound;
	int x1;
	int y1;
	int x2;
	int y2;
	int x3;
	int y3;
	int x4;
	int y4;
}map_sound_boundary_def;

typedef struct
{
	int id;
	char name[MAX_SOUND_MAP_NAME_LENGTH];		// This isn't used, it is simply helpful when editing the config
	map_sound_boundary_def boundaries[MAX_SOUND_MAP_BOUNDARIES];
	int num_boundaries;
}map_sound_data;

typedef struct
{
	int id;					// ID's in the config should match those of special_effect_enum
	int sound;
}effect_sound_data;

typedef struct
{
	char file[30];			// This is the filename (without extension) for a particle
	int sound;
}particle_sound_data;

#ifdef OGG_VORBIS
typedef struct
{
	int type;						// Is this the music stream (1), or sound fx stream (0)
	ALuint * source;
	OggVorbis_File * stream;
	vorbis_info * info;
	int processed;
}stream_data;
#endif // OGG_VORBIS

#ifdef DEBUG
struct sound_object
{
	int file, x, y, positional, loops;
};
struct sound_object sound_objects[MAX_SOURCES];
#endif	// DEBUG


#else // NEW_SOUND
ALCdevice *mSoundDevice;
ALCcontext *mSoundContext;
#endif	// NEW_SOUND

int have_sound=0;
int have_music=0;
int sound_opts=3;
int sound_on=1;
int music_on=1;
Uint8 inited = 0;
#ifdef NEW_SOUND
SDL_Thread *sound_streams_thread = NULL;
#else // NEW_SOUND
SDL_Thread *music_thread = NULL;
#endif // NEW_SOUND

ALfloat sound_gain=1.0f;
ALfloat music_gain=1.0f;

int used_sources=0;		// the number of sources currently playing

#ifdef NEW_SOUND
int num_types=0;		// number of distinct sound types
int num_samples=0;		// number of actual sound files -
						// a sound type can have > 1 sample

int sound_num_maps=0;		// Number of maps we have sounds for
int sound_num_effects=0;	// Number of effects we have sounds for
int sound_num_particles=0;	// Number of particles we have sounds for

int snd_cur_map = -1;
int cur_boundary = 0;

//each playing source is identified by a unique cookie.
unsigned int next_cookie = 1;
source_data sound_source_data[MAX_SOURCES];						// the active (playing) sources
sound_type sound_type_data[MAX_BUFFERS];						// configuration of the sound types
sound_sample sound_sample_data[MAX_BUFFERS];					// path & buffer data for each sample
map_sound_data sound_map_data[MAX_SOUND_MAPS];					// data for map sfx
effect_sound_data sound_effect_data[MAX_SOUND_EFFECTS];			// data for effect sfx
particle_sound_data sound_particle_data[MAX_SOUND_PARTICLES];	// data for particle sfx
int server_sound[9];											// map of server sounds to sound def ids
#else
char sound_files[MAX_BUFFERS][MAX_FILENAME_LENGTH];
ALuint sound_source[MAX_SOURCES];
ALuint sound_buffer[MAX_BUFFERS];
#endif	//NEW_SOUND
SDL_mutex *sound_list_mutex;


#ifdef	OGG_VORBIS
#define MAX_PLAYLIST_ENTRIES 50

#ifdef NEW_SOUND
stream_data sound_fx_stream;
OggVorbis_File sound_ogg_stream;
vorbis_info * sound_ogg_info;

ALuint sound_stream_buffers[4];
ALuint sound_stream_source;
int playing_sounds=0;

stream_data music_stream;
OggVorbis_File music_ogg_stream;
vorbis_info * music_ogg_info;
#else // NEW_SOUND
FILE* ogg_file;
OggVorbis_File ogg_stream;
vorbis_info* ogg_info;
#endif // NEW_SOUND

ALuint music_buffers[4];
ALuint music_source;

int playing_music=0;

playlist_entry playlist[MAX_PLAYLIST_ENTRIES];
int loop_list=1;
int list_pos=-1;
#endif	// OGG_VORBIS



/*    *** Declare some local functions ****
 * These functions shouldn't need to be accessed outside sound.c
 */
#ifdef OGG_VORBIS
#ifndef NEW_SOUND
void load_ogg_file(char *file_name);
void stream_music(ALuint buffer);
#else // !NEW_SOUND
int load_ogg_file(char *file_name, OggVorbis_File *oggFile);
int stream_ogg_file(char *file_name, ALuint inSource, OggVorbis_File * inStream, ALuint * inBuffers, int numBuffers);
int stream_ogg(ALuint buffer, OggVorbis_File * inStream, ALuint rate);
#ifdef ALUT_WAV
ALvoid * load_ogg_into_memory(char * szPath, ALenum *inFormat, ALsizei *inSize, ALsizei *inFreq);
#else // ALUT_WAV
ALvoid * load_ogg_into_memory(char * szPath, ALenum *inFormat, ALsizei *inSize, ALfloat *inFreq);
#endif // ALUT_WAV
int process_stream(stream_data * stream, ALfloat gain, int * sleep, int * fade, int tx, int ty);
#endif // !NEW_SOUND
void ogg_error(int code);
#endif	// OGG_VORBIS

#ifndef NEW_SOUND
int realloc_sources();
#else // !NEW_SOUND
void clear_sound_data();
int ensure_sample_loaded(int index);
source_data *insert_sound_source_at_index(unsigned int index);
int store_sample_name(char *name);
int stop_sound_source_at_index(int index);
#ifdef OGG_VORBIS
void init_sound_stream(ALuint * source, ALuint * buffers, int gain);
#endif // OGG_VORBIS
void load_sound_config_data(char *path);
#ifdef DEBUG
void print_sound_types();
#endif // DEBUG
#endif	// !NEW_SOUND


/*********************
 *  COMMON FUNCTIONS *
 *********************/

void turn_sound_on()
{
	int i,state=0;
	ALuint source, error;
	if(!inited)
	{
#ifdef NEW_SOUND
		init_sound(SOUND_CONFIG_PATH);
#else
		init_sound();
#endif	// NEW_SOUND
	}
	if(!have_sound)
		return;
	LOCK_SOUND_LIST();
	sound_on = 1;
	for(i=0;i<used_sources;i++)
	{
#ifdef NEW_SOUND
		source = sound_source_data[i].source;
#else
		source = sound_source[i];
#endif	// NEW_SOUND
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if(state == AL_PAUSED)
			alSourcePlay(source);
	}
	UNLOCK_SOUND_LIST();
#if defined NEW_SOUND && OGG_VORBIS
	alGetSourcei(sound_stream_source, AL_SOURCE_STATE, &state);
	if(state == AL_PAUSED) {
		alSourcePlay(sound_stream_source);
		playing_sounds = 1;
	}
#endif	// NEW_SOUND && OGG_VORBIS
	if((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error turning sound on\n");
#endif // _EXTRA_SOUND_DEBUG
	}
}

void turn_sound_off()
{
	int i=0,loop;
	ALuint source, error;
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
#ifdef NEW_SOUND
		source = sound_source_data[i].source;
#else
		source = sound_source[i];
#endif	//NEW_SOUND
		alGetSourcei(source, AL_LOOPING, &loop);
		if(loop == AL_TRUE)
			alSourcePause(source);
		else
		{
#ifdef NEW_SOUND
#ifdef _EXTRA_SOUND_DEBUG
			printf("Removing source with index %d\n", i);
#endif //_EXTRA_SOUND_DEBUG
			stop_sound_source_at_index(i);
			continue;
#else
			alSourceStop(source);
#endif	//NEW_SOUND
		}
		++i;
	}
	UNLOCK_SOUND_LIST();
#ifdef NEW_SOUND
	playing_sounds = 0;
	if(sound_streams_thread != NULL && !have_music)
	{
		int queued = 0;
		alSourceStop(sound_stream_source);
		alGetSourcei(sound_stream_source, AL_BUFFERS_QUEUED, &queued);
		while(queued-- > 0){
			ALuint buffer;		
			alSourceUnqueueBuffers(sound_stream_source, 1, &buffer);
		}
		SDL_WaitThread(sound_streams_thread,NULL);
		sound_streams_thread = NULL;
	}
	ov_clear(&sound_ogg_stream);
#endif // NEW_SOUND
	if((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error turning sound off\n");
#endif //_EXTRA_SOUND_DEBUG
	}
}

#ifdef NEW_SOUND
void change_sounds(int * var, int value)
{
	int old_val = sound_opts;
	
	if (value >= 0) *var = value; // We need to set this here so the sound stream doesn't quit if there is no music
	if (value == SOUNDS_NONE && old_val != SOUNDS_NONE)	{
		turn_sound_off();
	} else if (value != SOUNDS_NONE && (!have_sound || old_val == SOUNDS_NONE)) {
		turn_sound_on();
	}
}
#else
void toggle_sounds(int *var){
	*var=!*var;
	if(!sound_on){
		turn_sound_off();
	} else {
		turn_sound_on();
	}
}
#endif	//NEW_SOUND

void turn_music_on()
{
#ifdef	OGG_VORBIS
	int state;
	if(!inited){
#ifdef NEW_SOUND
		init_sound(SOUND_CONFIG_PATH);
#else
		init_sound();
#endif	//NEW_SOUND
	}
	if(!have_music){
		return;
	}
	get_map_playlist();
	music_on = 1;
#ifndef NEW_SOUND
	if(music_thread == NULL){
		music_thread=SDL_CreateThread(update_music, 0);
	}
#endif // !NEW_SOUND
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
#ifdef NEW_SOUND
	if(sound_streams_thread != NULL){
#else // NEW_SOUND
	if(music_thread != NULL){
#endif // NEW_SOUND
		int queued = 0;
		music_on=0;
		alSourceStop(music_source);
		alGetSourcei(music_source, AL_BUFFERS_QUEUED, &queued);
		while(queued-- > 0){
			ALuint buffer;		
			alSourceUnqueueBuffers(music_source, 1, &buffer);
		}
#ifdef NEW_SOUND
		if (!have_sound)
		{
			SDL_WaitThread(sound_streams_thread,NULL);
			sound_streams_thread = NULL;
		}
#else // NEW_SOUND
		SDL_WaitThread(music_thread,NULL);
		music_thread = NULL;
#endif // NEW_SOUND
	}
	music_on = playing_music = 0;
#endif	// OGG_VORBIS
}

void toggle_music(int * var){
	*var=!*var;
	if(!music_on){
		turn_music_off();
	} else {
		turn_music_on();
	}
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
#ifdef NEW_SOUND
	if(sound_streams_thread != NULL){
#else // NEW_SOUND
	if(music_thread != NULL){
#endif // NEW_SOUND
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
#ifdef NEW_SOUND
			ov_clear(&music_ogg_stream);
#else // NEW_SOUND
			ov_clear(&ogg_stream);
#endif // NEW_SOUND
		}
#ifdef NEW_SOUND
		if (playing_sounds)
		{
			playing_sounds = 0;
			queued = 0;
			ov_clear(&sound_ogg_stream);
			alSourceStop(sound_stream_source);
			alGetSourcei(sound_stream_source, AL_BUFFERS_QUEUED, &queued);
			while(queued-- > 0){
				alSourceUnqueueBuffers(sound_stream_source, 1, &buffer);
			}
			alDeleteSources(1, &sound_stream_source);
			alDeleteBuffers(4, sound_stream_buffers);
		}
		SDL_WaitThread(sound_streams_thread, NULL);
		sound_streams_thread = NULL;
#else // NEW_SOUND
		SDL_WaitThread(music_thread,NULL);
		music_thread = NULL;
#endif // NEW_SOUND
	}
#endif	// OGG_VORBIS
#ifdef NEW_SOUND
	for(i=0;i<MAX_SOURCES;i++)
	{
		if(alIsSource(sound_source_data[i].source) == AL_TRUE)
		{
			alSourceStopv(1, &sound_source_data[i].source);
			alDeleteSources(1, &sound_source_data[i].source);
			sound_source_data[i].source = 0;
		}
	}
	for(i=0;i<MAX_BUFFERS;i++)
	{
		if(alIsBuffer(sound_sample_data[i].buffer))
		{
			alDeleteBuffers(1, &sound_sample_data[i].buffer);
			sound_sample_data[i].loaded_status = 0;
		}
	}
#else
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
#endif	//NEW_SOUND
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

#ifdef NEW_SOUND
	clear_sound_data();
#endif	//NEW_SOUND
	used_sources = 0;
}


/************************
 * OGG VORBIS FUNCTIONS *
 ************************/

#ifdef	OGG_VORBIS
#ifndef NEW_SOUND
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


#else // !NEW_SOUND

int load_ogg_file(char *file_name, OggVorbis_File *oggFile)
{
	FILE *file;
	int result = 0;

	file = my_fopen(file_name, "rb");

	if (file == NULL)
	{
		return 0;
	}

#ifndef WINDOWS
	result = ov_open(file, oggFile, NULL, 0);
#else // !WINDOWS
	result = ov_open_callbacks(file, oggFile, NULL, 0, OV_CALLBACKS_DEFAULT);
#endif // !WINDOWS
	if (result < 0)
	{
		LOG_ERROR("Error opening ogg file: %s. Ogg error: %d\n", file_name, result);
		fclose(file);
		return 0;
	}

	return 1;
}

int stream_ogg_file(char *file_name, ALuint inSource, OggVorbis_File * inStream, ALuint * inBuffers, int numBuffers)
{
	int error, queued;
	int result, more_stream, i;
	vorbis_info * ogg_info;
	
	alSourceStop(inSource);
	alGetSourcei(inSource, AL_BUFFERS_QUEUED, &queued);
	while (queued-- > 0)
	{
		ALuint buffer;
		alSourceUnqueueBuffers(inSource, 1, &buffer);
	}

	ov_clear(inStream);
	result = load_ogg_file(file_name, inStream);
	if (!result) {
		LOG_ERROR("Error loading ogg file: %s\n", file_name);
		return -1;
	}

	ogg_info = ov_info(inStream, -1);
	for (i = 0; i < numBuffers; i++)
	{
		more_stream = stream_ogg(inBuffers[i], inStream, ogg_info->rate);
	}
    
	alSourceQueueBuffers(inSource, numBuffers, inBuffers);
	alSourcePlay(inSource);

	if((error=alGetError()) != AL_NO_ERROR) 
	{
		LOG_ERROR("stream_ogg_file %s: %s", my_tolower(reg_error_str), alGetString(error));
		return -1;
	}
	
	return more_stream;
}

int stream_ogg(ALuint buffer, OggVorbis_File * inStream, ALuint rate)
{
    char data[STREAM_BUFFER_SIZE];
    int  size = 0;
    int  section = 0;
    int  result = 0;
	int error = 0;
	char str[256];

	if ((error=alGetError()) != AL_NO_ERROR)
	{
		LOG_ERROR("stream_ogg %s: %s", my_tolower(reg_error_str), alGetString(error));
	}

    while (size < STREAM_BUFFER_SIZE)
    {
#ifndef EL_BIG_ENDIAN
        result = ov_read(inStream, data + size, STREAM_BUFFER_SIZE - size, 0, 2, 1, &section);
#else
        result = ov_read(inStream, data + size, STREAM_BUFFER_SIZE - size, 1, 2, 1, &section);
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
	if ((error=alGetError()) != AL_NO_ERROR)
	{
		LOG_ERROR("stream_ogg %s: %s", my_tolower(reg_error_str), alGetString(error));
	}

	if (!size)
	{
		return 0;	//file's done, quit trying to play
	}
	
	alBufferData(buffer, AL_FORMAT_STEREO16, data, size, rate);

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
	static ALenum format;
	static ALsizei size;
	static ALfloat freq;
	char * data;
	
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
	// end if
	
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
#endif // !NEW_SOUND

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


/*******************
 * MUSIC FUNCTIONS *
 *******************/

#ifndef NEW_SOUND
void stream_music(ALuint buffer)
{
#ifdef	OGG_VORBIS
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
#endif	// OGG_VORBIS
}

#else // !NEW_SOUND

#ifdef	OGG_VORBIS
void play_music_file(char *file_name)
{
	int result;
	char tmp_file_name[80];
	
	if(!have_music)return;

	if(file_name[0]!='.' && file_name[0]!='/')
		safe_snprintf (tmp_file_name, sizeof (tmp_file_name), "./music/%s", file_name);
	else
		safe_snprintf(tmp_file_name, sizeof (tmp_file_name), "%s", file_name);

	result = stream_ogg_file(tmp_file_name, music_source, &music_ogg_stream, music_buffers, 4);
	if (result == -1)
	{
		have_music = 0;
		return;
	}
	else if (result == 0)
	{
		playing_music = 0;
	}
	else
	{
		music_ogg_info = ov_info(&music_ogg_stream, -1);
		playing_music = 1;
	}
}

void play_sound_stream(int sound)
{
	int result;
	char tmp_file_name[80];
	char * file;

	file = sound_sample_data[sound_type_data[sound].sample_indices[STAGE_MAIN]].file_path;
	if(!have_sound || sound == -1 || !strcmp(file, ""))return;
		
	if(file[0]!='.' && file[0]!='/')
		safe_snprintf (tmp_file_name, sizeof (tmp_file_name), "./sound/%s", file);
	else
		safe_snprintf(tmp_file_name, sizeof (tmp_file_name), "%s", file);

	result = stream_ogg_file(tmp_file_name, sound_stream_source, &sound_ogg_stream, sound_stream_buffers, 4);
	if (result == 0 || result == -1)
	{
		playing_sounds = 0;
	}
	else if (result > 0)
	{
		sound_ogg_info = ov_info(&sound_ogg_stream, -1);
		playing_sounds = 1;
	}
}
#endif // OGG_VORBIS

void setup_map_sounds (int map_num)
{
	int i;
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

#endif // !NEW_SOUND

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
	alSourcef (music_source, AL_GAIN, music_gain);
#ifdef NEW_SOUND
	play_music_file(playlist[list_pos].file_name);
#else // NEW_SOUND
	play_ogg_file(playlist[list_pos].file_name);
#endif // NEW_SOUND
#endif	// OGG_VORBIS
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
#ifdef NEW_SOUND
		comments = ov_comment(&music_ogg_stream, -1);
#else // NEW_SOUND
		comments = ov_comment(&ogg_stream, -1);
#endif // NEW_SOUND
		if(comments == NULL){
#ifdef NEW_SOUND
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, playlist[list_pos].file_name, (int)(ov_time_tell(&music_ogg_stream)/60), (int)ov_time_tell(&music_ogg_stream)%60, (int)(ov_time_total(&music_ogg_stream,-1)/60), (int)ov_time_total(&music_ogg_stream,-1)%60);
#else // NEW_SOUND
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, playlist[list_pos].file_name, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
#endif // NEW_SOUND
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
#ifdef NEW_SOUND
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info, title, artist, (int)(ov_time_tell(&music_ogg_stream)/60), (int)ov_time_tell(&music_ogg_stream)%60, (int)(ov_time_total(&music_ogg_stream,-1)/60), (int)ov_time_total(&music_ogg_stream,-1)%60);
		}else if(title){
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, title, (int)(ov_time_tell(&music_ogg_stream)/60), (int)ov_time_tell(&music_ogg_stream)%60, (int)(ov_time_total(&music_ogg_stream,-1)/60), (int)ov_time_total(&music_ogg_stream,-1)%60);
		}else{
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, playlist[list_pos].file_name, (int)(ov_time_tell(&music_ogg_stream)/60), (int)ov_time_tell(&music_ogg_stream)%60, (int)(ov_time_total(&music_ogg_stream,-1)/60), (int)ov_time_total(&music_ogg_stream,-1)%60);
#else // NEW_SOUND
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info, title, artist, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
		}else if(title){
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, title, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
		}else{
			safe_snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, playlist[list_pos].file_name, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
#endif // NEW_SOUND
		}
		LOG_TO_CONSOLE(c_grey1, musname);
	}
#endif // !OGG_VORBIS
	return 1;
}



/***********************
 * OLD SOUND FUNCTIONS *
 ***********************/

#ifndef NEW_SOUND

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
	ALboolean loop;
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
	}http://www.google.com.au/advanced_search
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


void clear_sound_data()
{
	int i, j;
	
	for (i = 0; i < MAX_BUFFERS; i++)
	{
		sound_type_data[i].name[0] = '\0';
		for (j = 0; j < num_STAGES; j++)
		{
			sound_type_data[i].sample_indices[j] = -1;
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
		sound_sample_data[i].buffer = 0;
		sound_sample_data[i].file_path[0] = '\0';
		sound_sample_data[i].format = 0;
		sound_sample_data[i].size = 0;
		sound_sample_data[i].freq = 0;
		sound_sample_data[i].channels = 0;
		sound_sample_data[i].bits = 0;
		sound_sample_data[i].length = 0;
		sound_sample_data[i].loaded_status = 0;
	}
	for (i = 0; i < MAX_SOUND_MAPS; i++)
	{
		sound_map_data[i].id = 0;
		sound_map_data[i].name[0] = '\0';
		sound_map_data[i].num_boundaries = 0;
		for (j = 0; j < MAX_SOUND_MAP_BOUNDARIES; j++)
		{
			sound_map_data[i].boundaries[j].sound = -1;
			sound_map_data[i].boundaries[j].x1 = 0;
			sound_map_data[i].boundaries[j].y1 = 0;
			sound_map_data[i].boundaries[j].x2 = 0;
			sound_map_data[i].boundaries[j].y2 = 0;
			sound_map_data[i].boundaries[j].x3 = 0;
			sound_map_data[i].boundaries[j].y3 = 0;
			sound_map_data[i].boundaries[j].x4 = 0;
			sound_map_data[i].boundaries[j].y4 = 0;
		}
	}
	for (i = 0; i > MAX_SOUND_EFFECTS; i++)
	{
		sound_effect_data[i].id = 0;
		sound_effect_data[i].sound = -1;
	}
	for (i = 0; i > MAX_SOUND_PARTICLES; i++)
	{
		sound_particle_data[i].file[0] = '\0';
		sound_particle_data[i].sound = -1;
	}
	for (i = 0; i > 9; i++)
	{
		server_sound[i] = -1;
	}

	num_types=0;
	num_samples=0;
	sound_num_maps=0;
	sound_num_effects=0;
	sound_num_particles=0;
}

#ifdef	OGG_VORBIS

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

int process_stream(stream_data * stream, ALfloat gain, int * sleep, int * fade, int tx, int ty)
{
    int error, state, state2;
	int day_time = (game_minute >= 30 && game_minute < 60 * 3 + 30);
	if (*fade > 0)
	{
		*fade = *fade+1;
		if (*fade > 6)
		{
			int queued;
			*fade = 0;
			if (stream->type == STREAM_TYPE_MUSIC)
			{
				playing_music = 0;
			}
			else
			{
				playing_sounds = 0;
			}
			alSourceStop(*stream->source);
			alGetSourcei(*stream->source, AL_BUFFERS_PROCESSED, &stream->processed);
			alGetSourcei(*stream->source, AL_BUFFERS_QUEUED, &queued);
			while (queued-- > 0)
			{
				ALuint buffer;
				alSourceUnqueueBuffers(*stream->source, 1, &buffer);
			}
			return 0;
		}
		alSourcef(*stream->source, AL_GAIN, gain - ((float)*fade * (gain / 6)));
	}
	else
	{
		if (stream->type == STREAM_TYPE_MUSIC)
		{
			if (tx < playlist[list_pos].min_x ||
			   tx > playlist[list_pos].max_x ||
			   ty < playlist[list_pos].min_y ||
			   ty > playlist[list_pos].max_y ||
			   (playlist[list_pos].time != 2 &&
				playlist[list_pos].time != day_time))
			{
				*fade = 1;
				return 0;
			}
		}
		else
		{
			if (!sound_bounds_check(tx, ty, sound_map_data[snd_cur_map].boundaries[cur_boundary]))
			{
				*fade = 1;
				return 0;
			}
		}
		alSourcef(*stream->source, AL_GAIN, gain);
	}
	alGetSourcei(*stream->source, AL_BUFFERS_PROCESSED, &stream->processed);
	alGetSourcei(*stream->source, AL_SOURCE_STATE, &state);
	state2=state;	//fake out the Dev-C++ optimizer!
	if (!stream->processed)
	{
		if(*sleep < SLEEP_TIME)*sleep+=(SLEEP_TIME / 100);
		return 0; //skip error checking et al
	}
	while (stream->processed-- > 0)
	{
		ALuint buffer;

		alSourceUnqueueBuffers(*stream->source, 1, &buffer);
		if (stream->type == STREAM_TYPE_MUSIC)
			playing_music = stream_ogg(buffer, stream->stream, music_ogg_info->rate);
		else
			playing_sounds = stream_ogg(buffer, stream->stream, sound_ogg_info->rate);
		alSourceQueueBuffers(*stream->source, 1, &buffer);
	}
	if (state2 != AL_PLAYING)
	{
		LOG_TO_CONSOLE(c_red1, snd_skip_speedup);
		//on slower systems, music can skip up to 10 times
		//if it skips more, it just can't play the music...
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
		alSourcePlay(*stream->source);
	}
	if ((error=alGetError()) != AL_NO_ERROR)
	{
		LOG_ERROR("update_streams (music: %d) %s: %s", stream->type, my_tolower(reg_error_str), alGetString(error));
		if (stream->type == STREAM_TYPE_MUSIC)
			have_music = 0;
		else
			playing_sounds = 0;
	}
	
	return 1;
}

int update_streams(void *dummy)
{
    int error, sleep, music_fade = 0, sound_fade = 0;
	int cur_sound;
	int day_time;
	int tx, ty;
   	sleep = SLEEP_TIME;
	music_stream.type = STREAM_TYPE_MUSIC;
	music_stream.source = &music_source;
	music_stream.stream = &music_ogg_stream;
	music_stream.info = music_ogg_info;
	sound_fx_stream.type = STREAM_TYPE_SOUNDS;
	sound_fx_stream.source = &sound_stream_source;
	sound_fx_stream.stream = &sound_ogg_stream;
	sound_fx_stream.info = sound_ogg_info;
#ifdef _EXTRA_SOUND_DEBUG
	printf("Starting streams thread\n");
#endif //_EXTRA_SOUND_DEBUG
	while ((have_music && music_on) || (have_sound && sound_opts >= SOUNDS_ENVIRO))
	{
		SDL_Delay(sleep);
		day_time = (game_minute >= 30 && game_minute < 60 * 3 + 30);
		tx = -camera_x * 2;
		ty = -camera_y * 2;
		if (have_music && music_on)
		{
			// Process the music stream
			if (playing_music)
			{
				process_stream(&music_stream, music_gain, &sleep, &music_fade, tx, ty);
			}
			else
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
						alSourcef (music_source, AL_GAIN, music_gain);
						play_music_file(playlist[list_pos].file_name);
					}
				}
				else if (loop_list)
					list_pos=-1;
				else
					get_map_playlist();
			}
		}
		if (have_sound && sound_opts >= SOUNDS_ENVIRO)
		{
			// Process the sound effects stream
			if (playing_sounds)
			{
				process_stream(&sound_fx_stream, sound_gain * sound_type_data[cur_sound].gain, &sleep, &sound_fade, tx, ty);
			}
			else
			{
				if (snd_cur_map > -1 && sound_map_data[snd_cur_map].id > -1)
				{
					cur_boundary++;
					if (cur_boundary == sound_map_data[snd_cur_map].num_boundaries)
						cur_boundary = 0;
					if (sound_map_data[snd_cur_map].boundaries[cur_boundary].sound > -1 &&
						sound_bounds_check(tx, ty, sound_map_data[snd_cur_map].boundaries[cur_boundary]))
					{
						cur_sound = sound_map_data[snd_cur_map].boundaries[cur_boundary].sound;
						alSourcef (sound_stream_source, AL_GAIN, sound_gain * sound_type_data[cur_sound].gain);
						if (cur_sound > -1)
						{
							play_sound_stream(cur_sound);
						}
					}
				}
			}
		}
		if ((error=alGetError()) != AL_NO_ERROR)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Stream error: %s\n", alGetString(error));
#endif //_EXTRA_SOUND_DEBUG
		}
		// Check if we are quitting
		if (exit_now) break;
	}
#ifdef _EXTRA_SOUND_DEBUG
	printf("Exiting streams thread. have_music: %d, music_on: %d, have_sound: %d, sound_opts: %d, exit_now: %d\n", have_music, music_on, have_sound, sound_opts, exit_now);
#endif //_EXTRA_SOUND_DEBUG
	return 1;
}

#endif	// OGG_VORBIS




/* If the sample given by the index is not currently loaded, create a
 * buffer and load it from the path stored against this index.
 * Returns 0 for success.
 */
int ensure_sample_loaded(int index)
{
	ALvoid *data;
#ifdef ALUT_WAV
	ALboolean loop;
#endif // ALUT_WAV

	int error;
	sound_sample *pSample = &sound_sample_data[index];
	ALuint *pBuffer=&pSample->buffer;
	char *szPath=pSample->file_path;
	
	if (!pSample->loaded_status)
	{
		//This file is not currently loaded
#ifdef _EXTRA_SOUND_DEBUG
			printf("Attemping to load sound: File: %s\n", szPath);
#endif //_EXTRA_SOUND_DEBUG

#ifdef	OGG_VORBIS
		// Do a crude check of the extension to choose which loader
		if (!strcasecmp(szPath+(strlen(szPath) - 4), ".ogg"))
		{
			// Assume it is actually an OggVorbis file and use our ogg loader
			data = load_ogg_into_memory(szPath, &pSample->format, &pSample->size, &pSample->freq);
			if (!data)
			{
				// Couldn't load the file, but we have already dumped an error message so just return
				return 1;
			}
		}
		else
#endif	// OGG_VORBIS
		{
			// Use one of the WAV loaders (preferrably alutLoadMemoryFromFile as alutLoadWAVFile is depreciated
			// However, the newer function doesn't exist under Mac, and older Alut libs
			// This mess can be removed if we move to a completely ogg client
#ifndef  ALUT_WAV
	        data = alutLoadMemoryFromFile(szPath, &pSample->format, &pSample->size, &pSample->freq);
#else  // !ALUT_WAV
 #ifndef OSX
			alutLoadWAVFile(szPath,&pSample->format,&data,&pSample->size,&pSample->freq,&loop);
 #else // !OSX
			alutLoadWAVFile(szPath,&pSample->format,&data,&pSample->size,&pSample->freq);
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
				printf("ensure_sample_loaded : alutLoadWAVFile(%s) = %s\n",	szPath, "NO SOUND DATA");
#else
				printf("ensure_sample_loaded : alutLoadMemoryFromFile(%s) = %s\n",	szPath, "NO SOUND DATA");
#endif  //ALUT_WAV
#endif  //ELC
				return 1;
			}
		}
			
#ifdef _EXTRA_SOUND_DEBUG
			printf("Result: File: %s, Format: %d, Size: %d, Freq: %f\n", szPath, (int)pSample->format, (int)pSample->size, (float)pSample->freq);
#endif //_EXTRA_SOUND_DEBUG

		// Create a buffer for the file
		alGenBuffers(1, pBuffer);
		if ((error=alGetError()) != AL_NO_ERROR) 
		{
			// Couldn't generate a buffer
#ifdef ELC
			LOG_ERROR("%s: %s", snd_buff_error, alGetString(error));
#else
			printf("ensure_sample_loaded ['%s',#%d]: alGenBuffers = %s\n",szPath, index, alGetString(error));
#endif
			*pBuffer=0;
			return 2;
		}
		// Send this data to the buffer
		alBufferData(*pBuffer, pSample->format, data, pSample->size, pSample->freq);
		if((error=alGetError()) != AL_NO_ERROR)
		{
		#ifdef ELC
			LOG_ERROR("%s: %s",snd_buff_error, alGetString(error));
		#else
			printf("ensure_sample_loaded ['%s',#d]: alBufferData(%s) = %s\n",szPath, index, alGetString(error));
		#endif
			alDeleteBuffers(1, pBuffer);
			return 3;
		}

		alGetBufferi(*pBuffer, AL_BITS, &pSample->bits);
		alGetBufferi(*pBuffer, AL_CHANNELS, &pSample->channels);
		pSample->length = (pSample->size*1000) / ((pSample->bits >> 3)*pSample->channels*pSample->freq);

		// Get rid of the temporary data
#ifdef  ALUT_WAV
		alutUnloadWAV(pSample->format, data, pSample->size, pSample->freq);
#else
		free(data);
#endif  //ALUT_WAV
	}

	pSample->loaded_status = 1;
	if((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error in ensure_sample_loaded: %s, Index: %d\n", alGetString(error), index);
#endif //_EXTRA_SOUND_DEBUG
	}
	return 0;
}

unsigned int add_server_sound(int type,int x, int y)
{
	int snd = -1;
	// Find the sound for this server sound type
	snd = server_sound[type];
	if (snd > -1)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Adding server sound: %d\n", type);
#endif //_EXTRA_SOUND_DEBUG
		return add_sound_object(snd, x, y);
	}
	else
	{
		LOG_ERROR("Unable to find server sound: %i", type);
		return -1;
	}
}

unsigned int add_sound_object(int type,int x, int y)
{
	int i, loops, error, tx, ty, distanceSq, source = -1;
	source_data *pSource;
	sound_type *pType,*pNewType;
	float maxDistanceSq=0.0f;
	SOUND_STAGE stage;
	ALfloat sourcePos[]={ x, y, 0.0};
	ALfloat sourceVel[]={ 0.0, 0.0, 0.0};
	ALuint buffer=0;
	
#ifdef _EXTRA_SOUND_DEBUG
	tx = camera_x*(-2);
	ty = camera_y*(-2);
	printf("Trying to add sound: %d (%s) at %d, %d. Camera: %d, %d\n", type, sound_type_data[type].name, x, y, tx, ty);
#endif //_EXTRA_SOUND_DEBUG
	
	if (!have_sound)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Sound isn't enabled yet. Have_sound: %d\n", have_sound);
#endif //_EXTRA_SOUND_DEBUG
		return 0;
	}

	//check it's a valid type, get pType as a pointer to the type data
	if(type >= MAX_BUFFERS || type < 0)
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
	// Check if we are playing this type of sound
	if (pNewType->type > sound_opts) {
#ifdef _EXTRA_SOUND_DEBUG
		printf("Not playing this type of sound: %d\n", pNewType->type);
#endif //_EXTRA_SOUND_DEBUG
		return 0;
	}

	LOCK_SOUND_LIST();
	//the sources are ordered by decreasing play priority
	for(pSource=sound_source_data,i=0;i<used_sources;++i,++pSource)
	{
		pType = &sound_type_data[pSource->sound_type];
		if(pNewType->priority <= pType->priority || pSource->sound_type < 0)
		{
			source=i;
#ifdef _EXTRA_SOUND_DEBUG
			printf("Inserting new source at index %d/%d\n", i, used_sources);
#endif //_EXTRA_SOUND_DEBUG
			if(!(pSource->sound_type < 0))
			{
				insert_sound_source_at_index(i);
#ifdef _EXTRA_SOUND_DEBUG
//				printf("Got here (tried to add sound)\n");
#endif //_EXTRA_SOUND_DEBUG
			}
			else
			{
#ifdef _EXTRA_SOUND_DEBUG
//				printf("pSource->sound_type < 0 was true: %d\n", pSource->sound_type);
#endif //_EXTRA_SOUND_DEBUG
			}
			break;
		}
	}

#ifdef _EXTRA_SOUND_DEBUG
//	printf("Got here (after initial part of add sound)\n");
#endif //_EXTRA_SOUND_DEBUG

	//all sources are used by higher-priority sounds
	if(i==MAX_SOURCES)
	{
	#ifdef ELC
		LOG_ERROR(snd_sound_overflow);
	#else
		printf("add_sound_object ['%s',#%d] - sounds overflow!\n",pNewType->name,i);
	#endif
#ifdef _EXTRA_SOUND_DEBUG
		printf("Sound overflow: %s, %d\n", pNewType->name, i);
#endif //_EXTRA_SOUND_DEBUG

		return 0;
	}
	//this is the lowest-priority sound but there is a spare slot at the end of the list
	else if(i==used_sources)
	{
		source=i;
#ifdef _EXTRA_SOUND_DEBUG
		printf("Creating a new source: %d/%d\n", used_sources, used_sources);
#endif //_EXTRA_SOUND_DEBUG

		pSource = insert_sound_source_at_index(used_sources);
#ifdef _EXTRA_SOUND_DEBUG
//		printf("Got here (tried to add sound)\n");
#endif //_EXTRA_SOUND_DEBUG
	}

#ifdef _EXTRA_SOUND_DEBUG
//	printf("Check for a main part\n");
#endif //_EXTRA_SOUND_DEBUG
	//refuse to play a sound which doesn't have a main part
	if(pNewType->sample_indices[STAGE_MAIN]<0)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Sound missing main part!!\n");
#endif //_EXTRA_SOUND_DEBUG
		return 0;
	}

	//check we can load all buffers used by this type
	for(i=0;i<3;++i)
	{
		if(pNewType->sample_indices[i] >= 0 && ensure_sample_loaded(pNewType->sample_indices[i])!=0)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Error: problem loading sample: %d\n", pNewType->sample_indices[i]);
#endif //_EXTRA_SOUND_DEBUG
			return 0;
		}
	}

#ifdef _EXTRA_SOUND_DEBUG
//	printf("Got here. Initialising sound\n");
#endif //_EXTRA_SOUND_DEBUG

	//we already quit if the sound has only an outro
	stage = pNewType->sample_indices[STAGE_INTRO] < 0 ? STAGE_MAIN : STAGE_INTRO;
	
	//initialise the source data to the first sample to be played for this sound
	pSource->play_duration = 0;
	pSource->sound_type = type;
	pSource->current_stage = stage;

	alSourcef(pSource->source, AL_PITCH, 1.0f);
	alSourcef(pSource->source, AL_GAIN, sound_gain * pNewType->gain);
	alSourcefv(pSource->source, AL_VELOCITY, sourceVel);
	alSourcefv(pSource->source, AL_POSITION, sourcePos);
	if((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error with alSourcef calls: %s. Name: %s. Source: %d: %s\n", snd_source_error, pNewType->name, pSource->source, alGetString(error));
#endif //_EXTRA_SOUND_DEBUG
	}

	for(;stage<num_STAGES;++stage)
	{
		//get the buffer to be queued.
		if(pNewType->sample_indices[stage] < 0)
			break;
		buffer = sound_sample_data[pNewType->sample_indices[stage]].buffer;

		//if there are a finite number of loops for main sample, queue them all here
		if(stage == STAGE_MAIN)
		{	
			if(pNewType->loops > 0)
			{
				for(loops = 0;loops < pNewType->loops;++loops)
				{
					alSourceQueueBuffers(pSource->source,1,&buffer);
				}
			}
			else
			{
				alSourceQueueBuffers(pSource->source,1,&buffer);
				//dont queue an outro that will never get played!
				break;
			}
		}
		else 
		{
			alSourceQueueBuffers(pSource->source,1,&buffer);
		}
	}
	if((error=alGetError()) != AL_NO_ERROR) 
	{
#ifdef ELC
 #ifdef _EXTRA_SOUND_DEBUG
		printf("Error with alSourceQueueBuffers: %s. Name: %s. Source: %d: %s", snd_source_error, pNewType->name, source, alGetString(error));
 #else //_EXTRA_SOUND_DEBUG
		LOG_ERROR("Error with alSourceQueueBuffers: %s. Name: %s. Source: %d: %s", snd_source_error, pNewType->name, source, alGetString(error));
 #endif //_EXTRA_SOUND_DEBUG
#else	
		printf("add_sound_object (%s): alSourceQueueBuffers = %s\n", pNewType->name,alGetString(error));
#endif
		alSourcei(pSource->source,AL_BUFFER,0);
		UNLOCK_SOUND_LIST();
		pSource->sound_type = -1;
		pSource->current_stage = STAGE_UNUSED;
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error queuing buffers: (%s): alSourceQueueBuffers = %s\n", pNewType->name,alGetString(error));
#endif //_EXTRA_SOUND_DEBUG
		return 0;
	}

#ifdef _EXTRA_SOUND_DEBUG
//	printf("Setting more properties\n");
#endif //_EXTRA_SOUND_DEBUG
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
	//0 is infinite looping
	if(pNewType->sample_indices[STAGE_INTRO] < 0 && pNewType->loops==0)
		alSourcei(pSource->source,AL_LOOPING,AL_TRUE);
	else
		alSourcei(pSource->source,AL_LOOPING,AL_FALSE);

	tx = camera_x*(-2);
	ty = camera_y*(-2);
	distanceSq=(tx-x)*(tx-x)+(ty-y)*(ty-y);
	maxDistanceSq = pNewType->distance*pNewType->distance;

	alSourcePlay(pSource->source);
	if(sound_opts == SOUNDS_NONE || (pNewType->positional && (distanceSq > maxDistanceSq)))
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Not playing this sound as we are out of range! maxDistanceSq: %d, distanceSq: %d\n", (int)maxDistanceSq, distanceSq);
#endif //_EXTRA_SOUND_DEBUG
		alSourcePause(pSource->source);
	}

	UNLOCK_SOUND_LIST();
	
	pSource->cookie = next_cookie;
	//if next_cookie wraps around back to 0 (unlikely!) then address this.
	if(++next_cookie == 0)++next_cookie;
#ifdef _EXTRA_SOUND_DEBUG
	printf("Cookie %d. Playing this sound unless out of range.\n", pSource->cookie);
#endif //_EXTRA_SOUND_DEBUG
	return pSource->cookie;
}

void update_sound(int ms)
{
	int i=0,error;
	//we rebuild the list of active sources, as some may have become
	//inactive due to the sound ending.
	source_data *pSource = sound_source_data;
	sound_sample *pSample;
	sound_type *pSoundType;
	ALuint deadBuffer;
	ALint numProcessed, buffer, state;

	int source;
	int x,y,distanceSq,maxDistSq;
	int relative;
	int tx=-camera_x*2;
	int ty=-camera_y*2;
	ALfloat sourcePos[3]={0.0f,0.0f,0.0f};
	ALfloat listenerPos[]={tx,ty,0.0f};

	if(!have_sound || !used_sources)return;
	LOCK_SOUND_LIST();

#ifdef ELC
	//first, update the position of actor (animation) sounds
	for(i=0;i<max_actors;i++)
	{
		if(!actors_list[i] || !actors_list[i]->cur_anim_sound_cookie)
			continue;
		
		source = find_sound_source_from_cookie(actors_list[i]->cur_anim_sound_cookie);
		if(source < 0)
			continue;
		
		sourcePos[0] = actors_list[i]->x_pos*2;
		sourcePos[1] = actors_list[i]->y_pos*2;
		sourcePos[2] = 0.0f;
		alSourcefv(sound_source_data[source].source, AL_POSITION, sourcePos);
	}
#endif //ELC
	
	//now update the position of the listener
	alListenerfv(AL_POSITION,listenerPos);

	i = 0;
	while(i<used_sources)
	{
		//this test should be redundant
		if(pSource->sound_type < 0 || pSource->current_stage==STAGE_UNUSED)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Removing dud sound #%d. Sound type: %d. Current stage: %d\n",i, pSource->sound_type, pSource->current_stage);
#endif //_EXTRA_SOUND_DEBUG
			stop_sound_source_at_index(i);
			continue;
		}
		pSoundType = &sound_type_data[pSource->sound_type];
		pSample = &sound_sample_data[pSoundType->sample_indices[pSource->current_stage]];

		//is this source still playing?
		alGetSourcei(pSource->source,AL_SOURCE_STATE,&state);
		if(state==AL_STOPPED)
		{
#ifdef _EXTRA_SOUND_DEBUG
//			printf("'%s' has stopped after sample '%s'\n", pSoundType->name, pSample->file_path);
#endif //_EXTRA_SOUND_DEBUG
			pSource->current_stage = num_STAGES;
		}
		else
		{//find which buffer is playing
			alGetSourcei(pSource->source,AL_BUFFER,&buffer);
			if(buffer != pSample->buffer)
			{//has the source moved on to the next queued sample
#ifdef _EXTRA_SOUND_DEBUG
				printf("'%s' - sample '%s' has ended...", pSoundType->name, pSample->file_path);
#endif //_EXTRA_SOUND_DEBUG
				while(++pSource->current_stage != num_STAGES)
				{
					if(pSoundType->sample_indices[pSource->current_stage] < 0)
					{//no more samples to play
#ifdef _EXTRA_SOUND_DEBUG
						printf("no more samples for this type!\n");
#endif //_EXTRA_SOUND_DEBUG
						pSource->current_stage = num_STAGES;
						break;
					}
					pSample = &sound_sample_data[pSoundType->sample_indices[pSource->current_stage]];
					//found the currently-playing buffer

					if(pSample->buffer == buffer)
					{
#ifdef _EXTRA_SOUND_DEBUG
						printf("next sample is '%s'\n",pSample->file_path);
#endif //_EXTRA_SOUND_DEBUG
						if(pSource->current_stage == STAGE_MAIN && pSoundType->loops == 0)
						{//we've progressed to the main sample which loops infinitely.
							do
							{	//we only unqueue buffers explicitly here so that there's only the
								//MAIN sample queued for the looping. Normally we just set a zero
								//buffer to the source which automatically unqueues any buffers
								alGetSourcei(pSource->source,AL_BUFFERS_PROCESSED,&numProcessed);
								if(numProcessed-->0)
									alSourceUnqueueBuffers(pSource->source,1,&deadBuffer);
							}
							while(numProcessed>0);

							alSourcei(pSource->source,AL_LOOPING,AL_TRUE);
							if((error=alGetError()) != AL_NO_ERROR)
							{
							#ifdef ELC
								LOG_ERROR("%s: %s",snd_buff_error, alGetString(error));
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
//			printf("removing finished sound %d (%s) at source index %d\n", pSource->sound_type, pSoundType->name, i);
#endif //_EXTRA_SOUND_DEBUG
			stop_sound_source_at_index(i);
			continue;
		}

		alGetSourcei(pSource->source, AL_SOURCE_RELATIVE, &relative);
		if(relative != AL_TRUE)
		{
			alGetSourcei(pSource->source, AL_SOURCE_STATE, &state);
			alGetSourcefv(pSource->source, AL_POSITION, sourcePos);
			x=sourcePos[0];y=sourcePos[1];
			distanceSq=(tx-x)*(tx-x)+(ty-y)*(ty-y);
			maxDistSq = pSoundType->distance*pSoundType->distance;

			if((state == AL_PLAYING) && (distanceSq > maxDistSq))
			{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Pausing sound: %d (%s), Distance squared: %d, Max: %d\n", i, pSoundType->name, distanceSq, maxDistSq);
#endif //_EXTRA_SOUND_DEBUG
				alSourcePause(pSource->source);
			}
			else if (sound_opts != SOUNDS_NONE && (state == AL_PAUSED) && (distanceSq < maxDistSq))
			{
#ifdef _EXTRA_SOUND_DEBUG
			printf("Playing sound: %d (%s), Distance squared: %d, Max: %d\n", i, pSoundType->name, distanceSq, maxDistSq);
#endif //_EXTRA_SOUND_DEBUG
				alSourcePlay(pSource->source);
			}
			if((error=alGetError()) != AL_NO_ERROR) 
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
	if((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error updating sound using source: %d, error: %s\n", source, alGetString(error));
#endif //_EXTRA_SOUND_DEBUG
	}
}


//this takes a copy the first unused source object (or last one in the list if all used),
//moves all objects after #index along one place, and inserts the copied object at #index;
//it is ensured the source at #index is stopped with no buffers queued
source_data *insert_sound_source_at_index(unsigned int index)
{
	int i;
	source_data tempSource;
	//take a copy of the source about to be overwritten
	tempSource=sound_source_data[min2i(used_sources,MAX_SOURCES-1)];
	//ensure it is stopped and ready
	alSourceStop(tempSource.source);
	alSourcei(tempSource.source,AL_BUFFER,0);
	tempSource.play_duration=0;
	tempSource.current_stage=STAGE_UNUSED;
	tempSource.sound_type=-1;
	tempSource.cookie=0;

	//shunt source objects down a place
	for(i=min2i(used_sources,MAX_SOURCES-1);i>index;--i)
	{
		sound_source_data[i] = sound_source_data[i-1];
	}

	//now insert our stored object at #index
	sound_source_data[index] = tempSource;

	//although it's not doing anything, we have added a new source to the playing set
	if(used_sources < MAX_SOURCES)
		++used_sources;	

	//return a pointer to this new source
	return &sound_source_data[index];
}


//this stops the source for sound_source_data[index]. Because this array will change, the index
//associated with a source will change, so this function should only be called if the index is
//known for certain.
int stop_sound_source_at_index(int index)
{
	ALuint error;
	source_data *pSource,sourceTemp;
	if(index < 0 || index >= used_sources)
		return 0;
	pSource = &sound_source_data[index];
	//this unqueues any samples
	if (alIsSource(pSource->source))
	{
		alSourceStop(pSource->source);
		alSourcei(pSource->source,AL_BUFFER,0);
	}
	else
	{
   		LOG_ERROR("Attempting to stop invalid sound source %d with index %d", (int)pSource->source, index);
	}
	// Clear any errors so as to not confuse other error handlers
	if((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error '%s' stopping sound source with index: %d/%d.  Source: %d.\n", alGetString(error), index, used_sources, (int)pSource->source);
#endif //_EXTRA_SOUND_DEBUG
	}

	//we can't lose a source handle - copy this...
	sourceTemp = *pSource;
	//...shift all the next sources up a place, overwriting the stopped source...
	memcpy(pSource,pSource+1,sizeof(source_data)*(used_sources-(index+1)));
	//...and put the saved object back in after them
	sound_source_data[used_sources-1] = sourceTemp;
	
	//note that one less source is playing!
	--used_sources;
	return 1;
}

//this is passed a cookie, and searches for a source with this cookie
void stop_sound(unsigned long int cookie)
{
	int n;
	//source handle of 0 is a null source
	if(!have_sound || !cookie)
		return;
	//find which of our playing sources matches the handle passed
	n = find_sound_source_from_cookie(cookie);
	if(n >= 0)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Stopping cookie %d with sound source index %d\n", (int)cookie, n);
#endif //_EXTRA_SOUND_DEBUG
		stop_sound_source_at_index(n);
	}
}


//kill all the sounds.
//usefull when we change maps, etc.
void stop_all_sounds()
{
	int i;
	ALuint error;
#ifdef	OGG_VORBIS
	int musQueued, musProcessed;
	int sndQueued, sndProcessed;
	ALuint buffer;
#endif // OGG_VORBIS
	if(!have_sound || !used_sources)return;
	LOCK_SOUND_LIST();
#ifdef _EXTRA_SOUND_DEBUG
	printf("Stopping all individual sounds\n");
#endif //_EXTRA_SOUND_DEBUG
	for(i=0;i<used_sources;++i)
	{
		stop_sound_source_at_index(i);
	}
#ifdef	OGG_VORBIS
	if (have_music)
	{
		playing_music = 0;
#ifdef _EXTRA_SOUND_DEBUG
		printf("Stopping music source: %d\n", music_source);
#endif //_EXTRA_SOUND_DEBUG
		alSourceStop(music_source);
		alGetSourcei(music_source, AL_BUFFERS_PROCESSED, &musProcessed);
		alGetSourcei(music_source, AL_BUFFERS_QUEUED, &musQueued);
		while(musQueued-- > 0) {
			alSourceUnqueueBuffers(music_source, 1, &buffer);
		}
	}
	playing_sounds = 0;
#ifdef _EXTRA_SOUND_DEBUG
	printf("Stopping sound stream source: %d\n", sound_stream_source);
#endif //_EXTRA_SOUND_DEBUG
	alSourceStop(sound_stream_source);
	alGetSourcei(sound_stream_source, AL_BUFFERS_PROCESSED, &sndProcessed);
	alGetSourcei(sound_stream_source, AL_BUFFERS_QUEUED, &sndQueued);
	while(sndQueued-- > 0) {
		alSourceUnqueueBuffers(sound_stream_source, 1, &buffer);
	}
#endif	// OGG_VORBIS

	UNLOCK_SOUND_LIST();
	if((error=alGetError()) != AL_NO_ERROR)
	{
#ifdef _EXTRA_SOUND_DEBUG
		printf("Error killing all sounds\n");
#endif //_EXTRA_SOUND_DEBUG
	}
}


void sound_source_set_gain(unsigned long int cookie, float gain)
{
	int n;
	ALuint error;
	source_data *pSource;

	//source handle of 0 is a null source
	if(!have_sound || !cookie)
		return;
	//find which of our playing sources matches the handle passed
	for(n=0,pSource=sound_source_data;n<used_sources;++n,++pSource)
	{
		if(pSource->cookie == cookie)
		{
			alSourcef(pSource->source,AL_GAIN, sound_gain * sound_type_data[pSource->sound_type].gain * gain);
			if((error=alGetError()) != AL_NO_ERROR)
			{
#ifdef _EXTRA_SOUND_DEBUG
				printf("Error setting sound gain: %d, error: %s\n", (int)cookie, alGetString(error));
#endif //_EXTRA_SOUND_DEBUG
			}
			return;
		}
	}
}


int get_index_for_sound_type_name(const char *name)
{
	int i;
	for(i = 0; i < num_types; ++i)
	{
		if (strcasecmp(sound_type_data[i].name,name) == 0)
			return i;
	}
	return -1;
}

// Look for a particle sound def matching the input filename (minus the directory ./particles/ and extension .part)
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

//find the index of the source associated with this cookie.
//note that this result must not be stored, but used immediately;
int find_sound_source_from_cookie(unsigned int cookie)
{
	int n;
	source_data *pSource = sound_source_data;

	if(!cookie)
		return -1;

	for(n=0,pSource=sound_source_data;n<used_sources;++n,++pSource)
	{
		if(pSource->cookie==cookie)
			return n;
	}

	return -1;
}


//if the named sample is already used by any sound type, return
//the index of the sample. Otherwise save it and return the index,
//unless there are too many samples
int store_sample_name(char *name)
{
	int i;
	for(i=0;i<num_samples;++i)
	{	//return the index if found
		if(!strcmp(name,sound_sample_data[i].file_path))
			return i;
	}

	if(num_samples < MAX_BUFFERS)
	{
		// This file is not yet stored, store it
		safe_strncpy(sound_sample_data[num_samples].file_path, name, sizeof(sound_sample_data[num_samples].file_path));
		return num_samples++;
	}

	//all the 'slots' are full
	return -1;
}


#ifdef OGG_VORBIS
void init_sound_stream(ALuint * source, ALuint * buffers, int gain)
{
	alGenBuffers(4, buffers);
	alGenSources(1, source);
	alSource3f(*source, AL_POSITION,        0.0, 0.0, 0.0);
	alSource3f(*source, AL_VELOCITY,        0.0, 0.0, 0.0);
	alSource3f(*source, AL_DIRECTION,       0.0, 0.0, 0.0);
	alSourcef (*source, AL_ROLLOFF_FACTOR,  0.0          );
	alSourcei (*source, AL_SOURCE_RELATIVE, AL_TRUE      );
	alSourcef (*source, AL_GAIN,            gain);
}
#endif // OGG_VORBIS

void init_sound(char *sound_config_path)
{
	source_data *pSource, *sourceArrays[1] = {sound_source_data};
	ALCcontext *context;
	ALCdevice *device;
	int error;
	int i,j;
	
	if(inited)
		return;
		
#ifndef OSX
	alutInitWithoutContext(0, 0);
#endif // OSX

	// Begin by setting all data to a known state
	if(have_sound)
		destroy_sound();

	// Init sources
	for(i = 0; i < MAX_SOURCES; i++)
	{
		for(j = 0; j < sizeof(sourceArrays) / sizeof(sourceArrays[0]); ++j)
		{
			pSource = sourceArrays[j]+i;
			pSource->source = 0;
			pSource->sound_type = -1;
			pSource->play_duration = 0;
			pSource->current_stage = STAGE_UNUSED;
			pSource->cookie = 0;
		}
	}
	
	// Poison data
	clear_sound_data();
	
	//initialise OpenAL
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
	device = alcOpenDevice( NULL );
	if((error=alcGetError(device)) != AL_NO_ERROR || !device){
		char str[256];
		safe_snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alcGetString(device,error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound=have_music=0;
		return;
	}

	context = alcCreateContext( device, NULL );
	alcMakeContextCurrent( context );

	sound_list_mutex=SDL_CreateMutex();

	if((error=alcGetError(device)) != AL_NO_ERROR || !context || !sound_list_mutex){
		char str[256];
		safe_snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alcGetString(device,error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound=have_music=0;
		return;
	}

#ifdef DEBUG
	// Dump the capabilities of this version of Alut
	printf("Alut supported Buffer MIME types: %s\n", alutGetMIMETypes(ALUT_LOADER_BUFFER));
	printf("Alut supported Memory MIME types: %s\n", alutGetMIMETypes(ALUT_LOADER_MEMORY));
#endif // DEBUG
	
	have_sound=1;
#ifdef	OGG_VORBIS
	have_music=1;
#else	// OGG_VORBIS
	have_music=0;
#endif	// OGG_VORBIS

	LOCK_SOUND_LIST();
	for(i=0;i<MAX_SOURCES;i++)
	{
		alGenSources(1, &sound_source_data[i].source);
		//temp_source_data[i].source = sound_source_data[i].source;
		if((error=alGetError()) != AL_NO_ERROR) 
		{
			//this error code is designed for a single source, -1 indicates multiple sources
		#ifdef ELC
			LOG_ERROR("Error in init_sound: %s : %s", snd_source_error, alGetString(error));
		#endif // ELC
			have_sound=0;
			have_music=0;
			UNLOCK_SOUND_LIST();
			return;
		}
	}
	UNLOCK_SOUND_LIST();

	load_sound_config_data(sound_config_path);

	// Initialize streams
#ifdef	OGG_VORBIS
	init_sound_stream(&music_source, music_buffers, music_gain);
	init_sound_stream(&sound_stream_source, sound_stream_buffers, sound_gain);

	if(sound_streams_thread == NULL){
		sound_streams_thread = SDL_CreateThread(update_streams, 0);
	}
#endif	// OGG_VORBIS

#ifndef NEW_WEATHER
	///force the rain sound to be recreated
	rain_sound = 0;
#endif //NEW_WEATHER
	inited = 1;
}

/*	-- CONFIG FUNCTIONS -- */

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

	pData = &sound_type_data[num_types++];

	sVal = (char *)xmlGetProp(inNode,(xmlChar*)"name");
	if(!sVal)
	{
	#ifdef ELC
		LOG_ERROR("Sound config parse error: sound has no name");
	#else
		printf("Sound config parse error: sound has no name");
	#endif
	}
	else
	{
		safe_strncpy(pData->name, sVal, sizeof(pData->name));
		
		attributeNode = inNode->xmlChildrenNode;
		while(attributeNode != NULL)
		{
			if (attributeNode->type == XML_ELEMENT_NODE)
			{
				get_string_value(content, sizeof(content), attributeNode);
				if(!xmlStrcmp (attributeNode->name, (xmlChar*)"intro_sound"))
				{
					if(pData->sample_indices[STAGE_INTRO] < 0)
					{
						pData->sample_indices[STAGE_INTRO] = store_sample_name((char *)content);
					}
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: intro_index already set!");
				#else
						printf("Sound config parse error: intro_index already set!\n");
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"main_sound"))
				{
					if(pData->sample_indices[STAGE_MAIN] < 0)
					{
						pData->sample_indices[STAGE_MAIN] = store_sample_name((char *)content);
					}
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: main_index already set!");
				#else
						printf("Sound config parse error: main_index already set!\n");
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"outro_sound"))
				{
					if(pData->sample_indices[STAGE_OUTRO] < 0)
					{
						pData->sample_indices[STAGE_OUTRO] = store_sample_name((char *)content);
					}
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: outro_index already set!");
				#else
						printf("Sound config parse error: outro_index already set!\n");
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"gain"))
				{
					fVal = (float)atof((char *)content);
					if(fVal>0.0f)
						pData->gain = fVal;
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: gain = %f in '%s'",fVal,pData->name);
				#else
						printf("Sound config parse error: gain = %f in '%s'\n",fVal,pData->name);
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"stereo"))
				{
					iVal = atoi((char *)content);
					if(iVal==0 || iVal ==1)
						pData->stereo = iVal;
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: stereo = %d in '%s'",iVal,pData->name);
				#else
						printf("Sound config parse error: stereo = %d in '%s'\n",iVal,pData->name);
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"distance"))
				{
					fVal = (float)atof((char *)content);
					if(fVal>0.0f)
						pData->distance = fVal;
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: distance = %f in '%s'",fVal,pData->name);
				#else
						printf("Sound config parse error: distance = %f in '%s'\n",fVal,pData->name);
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"positional"))
				{
					iVal = atoi((char *)content);
					if(iVal==0 || iVal ==1)
						pData->positional = iVal;
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: positional = %d in '%s'",iVal,pData->name);
				#else
						printf("Sound config parse error: positional = %d in '%s'\n",iVal,pData->name);
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"loops"))
				{
					iVal = atoi((char *)content);
					if(iVal>=0)
						pData->loops = iVal;
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: loops = %d in '%s'",iVal,pData->name);
				#else
						printf("Sound config parse error: loops = %d in '%s'\n",iVal,pData->name);
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"fadeout_time"))
				{
					iVal = atoi((char *)content);
					if(iVal>=0)
						pData->fadeout_time = iVal;
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: fadeout_time = %d in '%s'",iVal,pData->name);
				#else
						printf("Sound config parse error: fadeout_time = %d in '%s'\n",iVal,pData->name);
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"echo_delay"))
				{
					iVal = atoi((char *)content);
					if(iVal>=0)
						pData->echo_delay = iVal;
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: echo_delay = %d in '%s'",iVal,pData->name);
				#else
						printf("Sound config parse error: echo_delay = %d in '%s'\n",iVal,pData->name);
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"echo_volume"))
				{
					iVal = atoi((char *)content);
					if(iVal>=0)
						pData->echo_volume = iVal;
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: echo_volume = %d in '%s'",iVal,pData->name);
				#else
						printf("Sound config parse error: echo_volume = %d in '%s'\n",iVal,pData->name);
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"time_of_day_flags"))
				{
					sscanf((char *)content,"%x",&iVal);
					if(iVal>=0 && iVal<=0xffff)
						pData->time_of_the_day_flags = iVal;
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: time_of_the_day_flags = 0x%x in '%s'",iVal,pData->name);
				#else
						printf("Sound config parse error: time_of_the_day_flags = 0x%x in '%s'\n",iVal,pData->name);
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"priority"))
				{
					iVal = atoi((char *)content);
					if(iVal>=0)
						pData->priority = iVal;
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: priority = %d in '%s'",iVal,pData->name);
				#else
						printf("Sound config parse error: priority = %d in '%s'\n",iVal,pData->name);
				#endif
					}
				}
				else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"type"))
				{
					if(!strcasecmp((char *)content, "environmental")) {
						iVal = SOUNDS_ENVIRO;
					} else if(!strcasecmp((char *)content, "actor")) {
						iVal = SOUNDS_ACTOR;
					} else if(!strcasecmp((char *)content, "walking")) {
						iVal = SOUNDS_WALKING;
					}
					if(iVal>=0)
						pData->type = iVal;
					else
					{
				#ifdef ELC
						LOG_ERROR("Sound config parse error: type = %s in '%s'",content,pData->name);
				#else
						printf("Sound config parse error: type = %s in '%s'\n",content,pData->name);
				#endif
					}
				}
			}
			else if (attributeNode->type == XML_ENTITY_REF_NODE)
			{
				LOG_ERROR("Sound config parse error: Include not allowed in sound def");
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
		LOG_ERROR("Sound config parse error: Error parsing coordinates");
	}
	return;
}

void parse_map_sound(xmlNode *inNode)
{
	xmlNode *boundaryNode = NULL;
	xmlNode *attributeNode = NULL;

	char *sVal = NULL;
	char content[50];

	map_sound_data *pMap = NULL;
	map_sound_boundary_def *pMapBoundary = NULL;

	pMap = &sound_map_data[sound_num_maps++];

	sVal = (char *)xmlGetProp(inNode,(xmlChar*)"id");
	if(!sVal)
	{
		pMap->id = -1;
	#ifdef ELC
		LOG_ERROR("Sound config parse error: map has no id");
	#else
		printf("Sound config parse error: map has no id");
	#endif
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
							if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"type"))
							{
								// Find the type of sound for this set of boundaries
								pMapBoundary->sound = get_index_for_sound_type_name(content);
								if (pMapBoundary->sound == -1)
								{
								#ifdef ELC
									LOG_ERROR("Sound config parse error: sound not found for map boundary type '%s' in map '%s'",content, pMap->name);
								#else
									printf("Sound config parse error: sound not found for map boundary type '%s' in map '%s'",content, pMap->name);
								#endif
								}
							}
							if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point1"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->x1, &pMapBoundary->y1);
							}
							if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point2"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->x2, &pMapBoundary->y2);
							}
							if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point3"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->x3, &pMapBoundary->y3);
							}
							if(!xmlStrcasecmp(attributeNode->name, (xmlChar*)"point4"))
							{
								// Parse the coordinates
								store_boundary_coords(content, &pMapBoundary->x4, &pMapBoundary->y4);
							}
						}
					}
					else
					{
					#ifdef ELC
						LOG_ERROR("Sound config parse error: reached max boundaries for map '%s'",pMap->name);
					#else
						printf("Sound config parse error: reached max boundaries for map '%s'",pMap->name);
					#endif
					}
				}
				else
				{
					LOG_ERROR("Sound config parse error: Boundary definition expected. Found: %s", attributeNode->name);
				}
			}
			else if (boundaryNode->type == XML_ENTITY_REF_NODE)
			{
				LOG_ERROR("Sound config parse error: Include not allowed in map sound def");
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
					LOG_ERROR("Sound config parse error: Unknown sound %s for effect %d", sound, pEffect->id);
				}
			}
		}
	}
	else if (inNode->type == XML_ENTITY_REF_NODE)
	{
		LOG_ERROR("Sound config parse error: Include not allowed in effect sound def");
	}
}

void parse_particle_sound(xmlNode *inNode)
{
	xmlNode *attributeNode = NULL;
	particle_sound_data *pParticle = NULL;
	char sound[MAX_SOUND_NAME_LENGTH] = "";

	if (inNode->type == XML_ELEMENT_NODE)
	{
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
					LOG_ERROR("Sound config parse error: Unknown sound %s for particle %s", sound, pParticle->file);
				}
			}
		}
	}
	else if (inNode->type == XML_ENTITY_REF_NODE)
	{
		LOG_ERROR("Sound config parse error: Include not allowed in particle sound def");
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
			else
			{
				LOG_ERROR("Sound config parse error: sound or include expected. Found: %s", def->name);
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

void load_sound_config_data(char *file)
{
	xmlDoc *doc;
	xmlNode *root=NULL;

	char path[1024];

#ifndef WINDOWS
	safe_snprintf(path, sizeof(path), "%s/%s", datadir, file);
#else
	safe_snprintf(path, sizeof(path), "%s", file);
#endif // !WINDOWS

	// Can we open the file as xml?
	if((doc = xmlReadFile(path, NULL, 0)) == NULL)
	{
	#ifdef ELC
		char str[200];
		safe_snprintf(str, sizeof(str), book_open_err_str, path);
		LOG_ERROR(str);
		LOG_TO_CONSOLE(c_red1,str);
	#endif
	}
	// Can we find a root element
	else if ((root = xmlDocGetRootElement(doc))==NULL)
	{
	#ifdef ELC
		LOG_ERROR("Error while parsing: %s", path);
	#endif
	}
	// Is the root the right type?
	else if ( xmlStrcmp( root->name, (xmlChar*)"sound_config" ) )
	{
	#ifdef ELC
		LOG_ERROR("Error in '%s' - root = '%s'", path, root->name);
	#endif
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
}
#endif	//!NEW_SOUND


/***********************
 * DEBUGGING FUNCTIONS *
 ***********************/

#ifdef DEBUG
#ifndef NEW_SOUND
void print_sound_objects ()
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
	int i, j, sample;
	sound_type *pData = NULL;
	map_sound_data *pMap = NULL;
	effect_sound_data *pEffect = NULL;
	particle_sound_data *pParticle = NULL;
	
	printf("\nSOUND TYPE DATA\n===============\n");
	printf("There are %d sound types (max %d) using %d samples (max %d):\n",
		num_types,MAX_BUFFERS,num_samples,MAX_BUFFERS);

	for(i=0;i<num_types;++i)
	{
		pData = &sound_type_data[i];
		printf("Sound type '%s' #%d:\n",pData->name, i);
		sample=pData->sample_indices[STAGE_INTRO];
		printf("\tIntro sample = '%s'(#%d)\n",(sample < 0) ? NULL : sound_sample_data[sample].file_path,sample);
		sample=pData->sample_indices[STAGE_MAIN];
		printf("\tMain sample = '%s'(#%d)\n",(sample < 0) ? NULL : sound_sample_data[sample].file_path,sample);
		sample=pData->sample_indices[STAGE_OUTRO];
		printf("\tOutro sample = '%s'(#%d)\n",(sample < 0) ? NULL : sound_sample_data[sample].file_path,sample);
		printf("\tGain = %f\n"					, pData->gain);
		printf("\tStereo = %d\n"				, pData->stereo);
		printf("\tDistance = %f\n"				, pData->distance);
		printf("\tPositional = %d\n"			, pData->positional);
		printf("\tLoops = %d\n"					, pData->loops);
		printf("\tFadeout time = %dms\n"		, pData->fadeout_time);
		printf("\tEcho delay = %dms\n"			, pData->echo_delay);
		printf("\tEcho volume = %d%%\n"			, pData->echo_volume);
		printf("\tTime of day flags = 0x%x\n"	, pData->time_of_the_day_flags);
		printf("\tPriority = %d\n\n"			, pData->priority);
	}
	printf("\nMAP SOUND DATA\n===============\n");
	printf("There are %d map sounds:\n", sound_num_maps);

	for(i=0;i<sound_num_maps;++i)
	{
		pMap = &sound_map_data[i];
		printf("Map id: %d\n", pMap->id);
		printf("Map name: %s\n", pMap->name);
		printf("Num boundaries: %d\n", pMap->num_boundaries);
		printf("Boundaries:\n");
		for(j=0;j<pMap->num_boundaries;++j)
		{
			printf("\tBoundary num: %d. Type: %d\n", j, pMap->boundaries[j].sound);
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

	for(i=0;i<sound_num_effects;++i)
	{
		pEffect = &sound_effect_data[i];
		printf("Effect ID: %d\n", pEffect->id);
		printf("Sound: %d\n", pEffect->sound);
	}
	printf("\nPARTICLE SOUND DATA\n===============\n");
	printf("There are %d particle sounds:\n", sound_num_particles);

	for(i=0;i<sound_num_particles;++i)
	{
		pParticle = &sound_particle_data[i];
		printf("Particle file: %s\n", pParticle->file);
		printf("Sound: %d\n", pParticle->sound);
	}
	printf("\nSERVER SOUNDS\n===============\n");
	printf("There are 10 server sounds:\n");

	for(i=0;i<=9;++i)
	{
		printf("Server Sound: %d = %d\n", i, server_sound[i]);
	}
}

void print_sound_samples()
{
	int i;
	sound_sample *pData=NULL;
	printf("\nSOUND SAMPLE DATA\n===============\n");
	printf("There are %d sound samples (max %d):\n",num_samples,MAX_BUFFERS);

	for(i=0;i<num_samples;++i)
	{
		pData = &sound_sample_data[i];
		printf("Sample #%d:\n",i);
		printf("\tPath = '%s'\n"				, pData->file_path);
		printf("\tStatus = '%s'\n"				, (pData->loaded_status!=1) ? "unloaded" : "loaded");
		if(pData->loaded_status==1)
		{
			printf("\tBuffer ID = %d\n"				, pData->buffer);
			printf("\tSize = %d\n"					, pData->size);
			printf("\tFrequency = %f\n"				, pData->freq);
			printf("\tChannels = %d\n"				, pData->channels);
			printf("\tBits = %d\n"					, pData->bits);
			printf("\tLength = %dms\n"				, pData->length);
		}
	}
}

void print_sound_sources()
{
	int i;
	source_data *pData=NULL;
	printf("\nSOUND SOURCE DATA\n===============\n");
	printf("There are %d sound sources (max %d):\n",used_sources,MAX_SOURCES);

	for(i=0;i<used_sources;++i)
	{
		pData = &sound_source_data[i];
		printf("Source #%d:\n",i);
		printf("\tSound type %d : '%s'\n"		, pData->sound_type, sound_type_data[pData->sound_type].name);
		printf("\tPlay duration = %dms\n"		, pData->play_duration);
		printf("\tSource ID = %d\n"				, pData->source);
	}
}
#endif // !NEW_SOUND
#endif //_DEBUG

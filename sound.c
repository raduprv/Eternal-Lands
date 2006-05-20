#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "sound.h"
#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif

#define MAX_BUFFERS 10
#define MAX_SOURCES 15

#define BUFFER_SIZE (4096 * 16)
#define SLEEP_TIME 500

#define	LOCK_SOUND_LIST()	SDL_LockMutex(sound_list_mutex)
#define	UNLOCK_SOUND_LIST()	SDL_UnlockMutex(sound_list_mutex);

typedef struct {
	char file_name[64];
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int time;
} playlist_entry;

int have_sound=0;
int have_music=0;
int sound_on=1;
int music_on=1;
Uint8 inited = 0;
SDL_Thread *music_thread = NULL;

ALfloat sound_gain=1.0f;
ALfloat music_gain=1.0f;

int used_sources = 0;

char sound_files[MAX_BUFFERS][30];
ALuint sound_source[MAX_SOURCES];
ALuint sound_buffer[MAX_BUFFERS];
SDL_mutex *sound_list_mutex;

#ifndef	NO_MUSIC
FILE* ogg_file;
OggVorbis_File ogg_stream;
vorbis_info* ogg_info;

ALuint music_buffers[4];
ALuint music_source;

int playing_music=0;

playlist_entry playlist[50];
int loop_list=1;
int list_pos=-1;
#endif	//NO_MUSIC

int realloc_sources();
void load_ogg_file(char *file_name);
void stream_music(ALuint buffer);
void ogg_error(int code);

#ifdef DEBUG
struct sound_object
{
	int file, x, y, positional, loops;
};

struct sound_object sound_objects[MAX_SOURCES];

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
#endif // def DEBUG

void stop_sound(int i)
{
	if(!have_sound)return;
	if (i == 0) return;
	alSourceStop (i);
}

void get_map_playlist()
{
#ifndef	NO_MUSIC
	int i=0, len;
	char map_list_file_name[256];
	char tmp_buf[1024];
	FILE *fp;
	char strLine[255];
	char *tmp;

	if(!have_music)
		return;

	memset (playlist, 0, sizeof(playlist));

	tmp = strrchr (map_file_name, '/');
	if (tmp == NULL)
		tmp = map_file_name;
	else
		tmp++;
	snprintf (map_list_file_name, sizeof (map_list_file_name), "./music/%s", tmp);
	len = strlen (map_list_file_name);
	tmp = strrchr (map_list_file_name, '.');
	if (tmp == NULL)
		tmp = &map_list_file_name[len];
	else
		tmp++;
	len -= strlen (tmp);
	snprintf (tmp, sizeof (map_list_file_name) - len, "pll");

	// don't consider absence of playlist an error, so don't use my_fopen
	fp=fopen(map_list_file_name,"r");
	if (fp == NULL)
		return;

	while(1)
	{
		fscanf(fp,"%d %d %d %d %d %s",&playlist[i].min_x,&playlist[i].min_y,&playlist[i].max_x,&playlist[i].max_y,&playlist[i].time,tmp_buf);
		// check for a comment
		tmp= strstr(tmp_buf, "--");
		if(tmp){
			*tmp= '\0';
			len= strlen(tmp_buf);
			while(len > 0 && isspace(tmp_buf[len-1])){
				len--;
				tmp_buf[len]= '\0';
			}
		}
		strncpy(playlist[i].file_name, tmp_buf, 64);
		playlist[i].file_name[63]= '\0';
		i++;
		if(!fgets(strLine, 100, fp))
			break;
	}
	fclose(fp);
	loop_list=1;
	list_pos=-1;
#endif	//NO_MUSIC
}

void play_ogg_file(char *file_name) {
#ifndef	NO_MUSIC
	int error,queued;

	if(!have_music)
		return;

	alSourceStop(music_source);
	alGetSourcei(music_source, AL_BUFFERS_QUEUED, &queued);
	while(queued-- > 0)
	{
		ALuint buffer;
		
		alSourceUnqueueBuffers(music_source, 1, &buffer);
	}

	load_ogg_file(file_name);
	if(!have_music)
		return;

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
#endif	//NO_MUSIC
}

void load_ogg_file(char *file_name) {
#ifndef	NO_MUSIC
	char file_name2[80];

	if(!have_music)
		return;

	ov_clear(&ogg_stream);

	if(file_name[0]!='.' && file_name[0]!='/')
		snprintf (file_name2, sizeof (file_name2), "./music/%s", file_name);
	else
		snprintf(file_name2, sizeof (file_name2), "%s", file_name);

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
#endif	//NO_MUSIC
}

ALuint get_loaded_buffer(int i)
{
	int error;
	ALsizei size,freq;
	ALenum  format;
	ALvoid  *data;
	ALboolean loop;
	FILE *fin;
	
	if(!alIsBuffer(sound_buffer[i]))
	{
		// XXX FIXME (Grum): You have got to be kidding me...
		// alutLoadWAVFile doesn't provide any way to check if loading
		// a file succeeded. Well, at least, let's check if the file
		// actually exists...
		// Maybe use alutLoadWAV? But that doesn't seem to exist on 
		// OS/X...
		fin = fopen (sound_files[i], "r");
		if (fin == NULL) 
		{
			LOG_ERROR(snd_wav_load_error, sound_files[i]);
			return 0;
		}
		// okay, the file exists and is readable, close it
		fclose (fin);

		alGenBuffers(1, sound_buffer+i);
			
		if((error=alGetError()) != AL_NO_ERROR) 
		{
			LOG_ERROR("%s: %s",snd_buff_error, alGetString(error));
			have_sound=0;
			have_music=0;
		}

#ifdef OSX
		// OS X alutLoadWAVFile doesn't have a loop option... Oh well :-)
		alutLoadWAVFile (sound_files[i], &format, &data, &size, &freq);
#else
		alutLoadWAVFile (sound_files[i], &format, &data, &size, &freq, &loop);
#endif
		alBufferData(sound_buffer[i],format,data,size,freq);
		alutUnloadWAV(format,data,size,freq);
	}
	return sound_buffer[i];
}

void play_music(int list) {
#ifndef	NO_MUSIC
	int i=0;
	char list_file_name[256];
	FILE *fp;
	char strLine[255];

	if(!have_music)
		return;

	snprintf(list_file_name, sizeof(list_file_name), "./music/%d.pll", list);
	// don't consider absence of playlist an error, so don't use my_fopen
	fp=fopen(list_file_name,"r");
	if(!fp)
		return;

	memset(playlist,0,sizeof(playlist));

	while(1)
	{
		fscanf(fp,"%s",playlist[i].file_name);
		playlist[i].min_x=0;
		playlist[i].min_y=0;
		playlist[i].max_x=10000;
		playlist[i].max_y=10000;
		playlist[i].time=2;
		i++;
		if(!fgets(strLine, 100, fp))break;
	}
	fclose(fp);
	loop_list=0;
	list_pos=0;
	alSourcef (music_source, AL_GAIN, music_gain);
	play_ogg_file(playlist[list_pos].file_name);
#endif	//NO_MUSIC
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

	tx=-cx*2;
	ty=-cy*2;
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

void sound_object_set_gain(int sound, float gain) {
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
	int tx=-cx*2;
	int ty=-cy*2;
	ALfloat sourcePos[3];
	ALfloat listenerPos[]={tx,ty,0.0};

	if(!have_sound)return;
	LOCK_SOUND_LIST();

	alListenerfv(AL_POSITION,listenerPos);

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
	}
	if((error=alGetError()) != AL_NO_ERROR) 
	{
		LOG_ERROR("update_position %s: %s", my_tolower(reg_error_str), alGetString(error));
		have_sound=0;
		have_music=0;
	}
	UNLOCK_SOUND_LIST();
}

int update_music(void *dummy)
{
#ifndef	NO_MUSIC
	int error,processed,state,state2,sleep,fade=0;
   	sleep = SLEEP_TIME;
	while(have_music && music_on)
	{
		SDL_Delay(sleep);
		if(playing_music)
		{
			int day_time = (game_minute>=30 && game_minute<60*3+30);
			int tx=-cx*2,ty=-cy*2;
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
			int tx=-cx*2,ty=-cy*2;
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
		if(exit_now)
			break;
	}
#endif	//NO_MUSIC
	return 1;
}

void stream_music(ALuint buffer) {
#ifndef	NO_MUSIC
	char data[BUFFER_SIZE];
	int  size = 0;
	int  section = 0;
	int  result = 0;
	int error = 0;
	char str[256];

	while(size < BUFFER_SIZE)
	{
		result = ov_read(&ogg_stream, data + size, BUFFER_SIZE - size, 0, 2, 1,
						 &section);
		snprintf(str, sizeof(str), "%d", result); //prevents optimization errors under Windows, but how/why?
		if((result > 0) || (result == OV_HOLE))		// OV_HOLE is informational
		{
			if (result != OV_HOLE)
			{
				size += result;
			}
		}
		else if(result < 0)
		{
			ogg_error(result);
		}
		else
		{
			break;
		}
	}
	if(!size)
	{
		playing_music = 0;//file's done, quit trying to play
		return;
	}

	alBufferData(buffer, AL_FORMAT_STEREO16, data, size, ogg_info->rate);

	if((error=alGetError()) != AL_NO_ERROR) 
	{
		LOG_ERROR("stream_music %s: %s", my_tolower(reg_error_str), alGetString(error));
		have_music=0;
	}
#endif	//NO_MUSIC
}


//kill all the sounds that loop infinitely
//usefull when we change maps, etc.
void kill_local_sounds()
{
	int error,queued,processed;
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
#ifndef	NO_MUSIC
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
#endif	//NO_MUSIC
}

void turn_sound_off()
{
	int i,loop;
	if(!inited){
		return;
	} else 
#ifndef NO_MUSIC
		if(!music_on)
#endif //NO_MUSIC
	{
		destroy_sound();
		return;
	}
	LOCK_SOUND_LIST();
	sound_on=0;
	for(i=0;i<used_sources;i++)
	{
		alGetSourcei(sound_source[i], AL_LOOPING, &loop);
		if(loop == AL_TRUE)
			alSourcePause(sound_source[i]);
		else
			alSourceStop(sound_source[i]);
	}
	UNLOCK_SOUND_LIST();
}

void turn_sound_on()
{
	int i,state=0;
	if(!inited){
		init_sound();
	}
	if(!have_sound){
		return;
	}
	sound_on=1;
	LOCK_SOUND_LIST();
	for(i=0;i<used_sources;i++)
	{
		alGetSourcei(sound_source[i], AL_SOURCE_STATE, &state);
		if(state == AL_PAUSED)
			alSourcePlay(sound_source[i]);
	}
	UNLOCK_SOUND_LIST();
}

void toggle_sounds(int * var){
	*var=!*var;
	if(!sound_on){
		turn_sound_off();
	} else {
		turn_sound_on();
	}
}

void toggle_music(int * var){
	*var=!*var;
	if(!music_on){
		turn_music_off();
	} else {
		turn_music_on();
	}
}

void turn_music_off()
{
	if(!sound_on && inited){
		destroy_sound();
		return;
	}
#ifndef	NO_MUSIC
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
#endif	//NO_MUSIC
}

void turn_music_on()
{
#ifndef	NO_MUSIC
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
#endif	//NO_MUSIC
}

void init_sound()
{
	int i,error;
	ALfloat listenerPos[]={-cx*2,-cy*2,0.0};
	ALfloat listenerVel[]={0.0,0.0,0.0};
	ALfloat listenerOri[]={0.0,0.0,0.0,0.0,0.0,0.0};
	if(inited){
		return;
	}
	have_sound=1;
#ifndef	NO_MUSIC
	have_music=1;
#else
	have_music=0;
#endif	//NO_MUSIC
	//NULL makes it use the default device.
	//if you want to use a different device, use, for example: ((ALubyte*) "DirectSound3D")
	mSoundDevice = alcOpenDevice( NULL );
	if((error=alGetError()) != AL_NO_ERROR || !mSoundDevice){
		char str[256];
		snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alGetString(error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound=have_music=0;
		return;
	}

	mSoundContext = alcCreateContext( mSoundDevice, NULL );
	alcMakeContextCurrent( mSoundContext );

	sound_list_mutex=SDL_CreateMutex();

	if((error=alGetError()) != AL_NO_ERROR || !mSoundContext || !sound_list_mutex){
		char str[256];
		snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alGetString(error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound=have_music=0;
		return;
	}

	// TODO: get this information from a file, sound.ini?	
	my_strcp(sound_files[snd_rain],"./sound/rain1.wav");
	my_strcp(sound_files[snd_tele_in],"./sound/teleport_in.wav");
	my_strcp(sound_files[snd_tele_out],"./sound/teleport_out.wav");
	my_strcp(sound_files[snd_teleprtr],"./sound/teleporter.wav");
	my_strcp(sound_files[snd_thndr_1],"./sound/thunder1.wav");
	my_strcp(sound_files[snd_thndr_2],"./sound/thunder2.wav");
	my_strcp(sound_files[snd_thndr_3],"./sound/thunder3.wav");
	my_strcp(sound_files[snd_thndr_4],"./sound/thunder4.wav");
	my_strcp(sound_files[snd_thndr_5],"./sound/thunder5.wav");
	my_strcp(sound_files[snd_fire],"./sound/fire.wav");

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
#endif
	}
	for(i=0;i<MAX_BUFFERS;i++)
		sound_buffer[i] = -1;

	//initialize music
#ifndef	NO_MUSIC
	ogg_file = NULL;

	alGenBuffers(4, music_buffers);
	alGenSources(1, &music_source);
	alSource3f(music_source, AL_POSITION,        0.0, 0.0, 0.0);
	alSource3f(music_source, AL_VELOCITY,        0.0, 0.0, 0.0);
	alSource3f(music_source, AL_DIRECTION,       0.0, 0.0, 0.0);
	alSourcef (music_source, AL_ROLLOFF_FACTOR,  0.0          );
	alSourcei (music_source, AL_SOURCE_RELATIVE, AL_TRUE      );
	alSourcef (music_source, AL_GAIN,            music_gain);
#endif	//NO_MUSIC
	if((error=alGetError()) != AL_NO_ERROR){
		char str[256];
		snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alGetString(error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
		have_sound=have_music=0;
		return;
	}
#ifndef NEW_WEATHER
	//force the rain sound to be recreated
	rain_sound = 0;
#endif //NEW_WEATHER
	inited = 1;
}

void destroy_sound()
{
	int i, error;
	if(!inited){
		return;
	}
	SDL_DestroyMutex(sound_list_mutex);
	sound_list_mutex=NULL;
	inited = have_sound = sound_on = 0;

#ifndef	NO_MUSIC
	music_on = playing_music = have_music = 0;
	if(music_thread != NULL){
		int queued = 0;
		alSourceStop(music_source);
		alGetSourcei(music_source, AL_BUFFERS_QUEUED, &queued);
		while(queued-- > 0){
			ALuint buffer;		
			alSourceUnqueueBuffers(music_source, 1, &buffer);
		}
		SDL_WaitThread(music_thread,NULL);
		music_thread = NULL;
	}
	alDeleteSources(1, &music_source);
	alDeleteBuffers(4, music_buffers);
	ov_clear(&ogg_stream);
#endif	//NO_MUSIC
	alSourceStopv(used_sources, sound_source);
	alDeleteSources(used_sources, sound_source);
	for(i=0;i<MAX_BUFFERS;i++) {
		if(alIsBuffer(sound_buffer[i])) {
			alDeleteBuffers(1, sound_buffer+i);
		}
	}
	alcDestroyContext( mSoundContext );
	if(mSoundDevice){
		alcCloseDevice( mSoundDevice );
	}
	alutExit();

	if((error=alGetError()) != AL_NO_ERROR) 
	{
		char str[256];
		snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alGetString(error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
	}
	used_sources = 0;
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

void ogg_error(int code)
{
#ifndef	NO_MUSIC
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
#endif	//NO_MUSIC
}

int display_song_name(){
#ifdef NO_MUSIC
	LOG_TO_CONSOLE(c_red2, snd_no_music);
#else //!NO_MUSIC
	if(!playing_music){
		LOG_TO_CONSOLE(c_grey1, snd_media_music_stopped);
	}else{
		char musname[100];
		char *title = NULL, *artist = NULL;
		int i=0;
		vorbis_comment *comments;
		comments = ov_comment(&ogg_stream, -1);
		if(comments == NULL){
			snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, playlist[list_pos].file_name, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
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
			snprintf(musname, sizeof(musname), snd_media_ogg_info, title, artist, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
		}else if(title){
			snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, title, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
		}else{
			snprintf(musname, sizeof(musname), snd_media_ogg_info_noartist, playlist[list_pos].file_name, (int)(ov_time_tell(&ogg_stream)/60), (int)ov_time_tell(&ogg_stream)%60, (int)(ov_time_total(&ogg_stream,-1)/60), (int)ov_time_total(&ogg_stream,-1)%60);
		}
		LOG_TO_CONSOLE(c_grey1, musname);
	}
#endif //NO_MUSIC
	return 1;
}

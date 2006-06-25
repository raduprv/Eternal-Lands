#ifdef MAP_EDITOR2
#include "../map_editor2/global.h"
#else
#include "global.h"
#endif
#include    <ctype.h>

#define MAX_FILENAME_LENGTH 80
#define MAX_BUFFERS 60 //remember, music uses 4 buffers too
#define MAX_SOURCES 15 //remember, music uses a source too

#define MUSIC_BUFFER_SIZE (4096 * 16)
#define SLEEP_TIME 500

#define	LOCK_SOUND_LIST()//	SDL_LockMutex(sound_list_mutex)
#define	UNLOCK_SOUND_LIST()//	SDL_UnlockMutex(sound_list_mutex);

typedef struct {
	char file_name[64];
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int time;
} playlist_entry;


#ifdef NEW_SOUND
typedef enum{STAGE_UNUSED=-1,STAGE_INTRO,STAGE_MAIN,STAGE_OUTRO,num_STAGES}SOUND_STAGE;
typedef struct
{
	char name[MAX_SOUND_NAME_LENGTH];
	int sample_indices[num_STAGES];	//the indices of the samples used
	int stereo;						//1 is stereo, 0 is mono (default mono)
	float distance;					//distance it can be heard, in meters
	int positional;					//1=positional, 0=omni (default positional)
	int loops;						//0=infinite, otherwise the number of loops (default 1)
	int fadeout_time;				//in milliseconds, only for omni sounds that loop. (default 0)
	int echo_delay;					//the value is the echo in MS. If 0, no echo (default 0)
	int echo_volume;				//in percent, 0 means no sound, 100 means as loud as the original sound (default 50)
	int time_of_the_day_flags;		//bits 0-11 set each 1/2 hour of the 6-hour day (default 0xffff)
	unsigned int priority;			//if there are too many sounds to be played, highest value priority get culled (default 5)
	int type;						// the type of sound (environmental, actor, walking etc) for sound_opts (default Enviro)
}sound_type;

typedef struct
{
	ALuint buffer;					//if the sample is loaded, a buffer ID to play it.
	char file_path[MAX_FILENAME_LENGTH];	//where to load the file from
	ALenum format;
	ALsizei size;							//size of the sound data in bytes
	ALsizei freq;							//frequency
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
#endif	//NEW_SOUND

int have_sound=0;
int have_music=0;
int sound_opts=3;
int sound_on=1;
int music_on=1;
Uint8 inited = 0;
SDL_Thread *music_thread = NULL;

ALfloat sound_gain=1.0f;
ALfloat music_gain=1.0f;

#ifdef NEW_SOUND
int num_types=0;		// number of distinct sound types
int num_samples=0;		// number of actual sound files -
						// a sound type can have > 1 sample
#endif	//NEW_SOUND
int used_sources=0;		// the number of sources currently playing

#ifdef NEW_SOUND
//each playing source is identified by a unique cookie.
unsigned int next_cookie = 1;
source_data sound_source_data[MAX_SOURCES];	//the active (playing) sources
sound_type sound_type_data[MAX_BUFFERS];	// configuration of the sound types
sound_sample sound_sample_data[MAX_BUFFERS];// path & buffer data for each sample	
#else
char sound_files[MAX_BUFFERS][30];
ALuint sound_source[MAX_SOURCES];
ALuint sound_buffer[MAX_BUFFERS];
#endif	//NEW_SOUND
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

#ifdef NEW_SOUND
void load_sound_config_data(char *path);
int ensure_sample_loaded(int index);
source_data *insert_sound_source_at_index(unsigned int index);
int store_sample_name(char *name);
int store_sample_name(char *name);
int stop_sound_source_at_index(int index);
#else
int realloc_sources();
#endif	//NEW_SOUND
void load_ogg_file(char *file_name);
void stream_music(ALuint buffer);
void ogg_error(int code);

#ifndef NEW_SOUND
#ifdef DEBUG
struct sound_object
{
	int file, x, y, positional, loops;
};
struct sound_object sound_objects[MAX_SOURCES];
#endif	//DEBUG
#endif	//NEW_SOUND

#ifdef NEW_SOUND
//this stops the source for sound_source_data[index]. Because this array will change, the index
//associated with a source will change, so this function should only be called if the index is
//known for certain.
int stop_sound_source_at_index(int index)
{
	source_data *pSource,sourceTemp;
	if(index < 0 || index >= used_sources)
		return 0;
	pSource = &sound_source_data[index];
	//this unqueues any samples
	alSourcei(pSource->source,AL_BUFFER,0);
	alSourceStop(pSource->source);

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
		stop_sound_source_at_index(n);
}
#else
void stop_sound(int i)
{
	if(!have_sound)return;
	if (i == 0) return;
	alSourceStop (i);
}
#endif //NEW_SOUND

void get_map_playlist()
{
#ifndef	NO_MUSIC
	int i=0, len;
	char map_list_file_name[256];
	char tmp_buf[1024];
	FILE *fp;
	char strLine[255];
	char *tmp;

	if(!have_music)return;

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
	if (fp == NULL) return;

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
			if(!fgets(strLine, 100, fp))break;
		}
	fclose(fp);
	loop_list=1;
	list_pos=-1;
#endif	//NO_MUSIC
}

void play_ogg_file(char *file_name) {
#ifndef	NO_MUSIC
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
#endif	//NO_MUSIC
}

void load_ogg_file(char *file_name)
{
#ifndef	NO_MUSIC
	char file_name2[80];

	if(!have_music)return;

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

#ifndef NEW_SOUND
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
#else
//if the sample given by the index is not currently loaded, create a
//buffer and load it from the path stored against this index.
//returns 0 for success.
int ensure_sample_loaded(int index)
{
	ALvoid *data;
	ALboolean loop;

	int error;
	sound_sample *pSample = &sound_sample_data[index];
	ALuint *pBuffer=&pSample->buffer;
	char *szPath=pSample->file_path;
	
	if (!pSample->loaded_status)
	{//this file is not currently loaded

		//try to open the file
#ifndef OSX
		alutLoadWAVFile(szPath,&pSample->format,&data,&pSample->size,&pSample->freq,&loop);
#else
		alutLoadWAVFile(szPath,&pSample->format,&data,&pSample->size,&pSample->freq);
#endif
		if(!data)
		{//couldn't load the file
		#ifdef ELC
			LOG_ERROR("%s: %s",snd_buff_error, "NO SOUND DATA");
		#else
			printf("ensure_sample_loaded : alutLoadWAVFile(%s) = %s\n",
				szPath, "NO SOUND DATA");
		#endif
			return 1;
		}

		//create a buffer
		alGenBuffers(1, pBuffer);
		if((error=alGetError()) != AL_NO_ERROR) 
		{//couldn't generate a buffer
		#ifdef ELC
			LOG_ERROR("%s: %s",snd_buff_error, alGetString(error));
		#else
			printf("ensure_sample_loaded ['%s',#%d]: alGenBuffers = %s\n",szPath, index, alGetString(error));
		#endif
			*pBuffer=0;
			return 2;
		}
		//send this data to the buffer
		alBufferData(*pBuffer,pSample->format,data,pSample->size,pSample->freq);
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

		alGetBufferi(*pBuffer,AL_BITS,&pSample->bits);
		alGetBufferi(*pBuffer,AL_CHANNELS,&pSample->channels);
		pSample->length = (pSample->size*1000) / ((pSample->bits >> 3)*pSample->channels*pSample->freq);

		//get rid of the temporary data
		alutUnloadWAV(pSample->format,data,pSample->size,pSample->freq);
    }

	pSample->loaded_status = 1;
	return 0;
}
#endif	//NEW_SOUND

void play_music(int list) {
#ifndef	NO_MUSIC
	int i=0;
	char list_file_name[256];
	FILE *fp;
	char strLine[255];

	if(!have_music)return;

	snprintf(list_file_name, sizeof(list_file_name), "./music/%d.pll", list);
	// don't consider absence of playlist an error, so don't use my_fopen
	fp=fopen(list_file_name,"r");
	if(!fp)return;

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

#ifdef NEW_SOUND
unsigned int add_sound_object(int type,int x, int y)
{
	int i,loops,error,tx,ty,distanceSq,source;
	source_data *pSource;
	sound_type *pType,*pNewType;
	float maxDistanceSq=0.0f;
	SOUND_STAGE stage;
	ALfloat sourcePos[]={ x, y, 0.0};
	ALfloat sourceVel[]={ 0.0, 0.0, 0.0};
	ALuint buffer=0;
	
	if(!have_sound)return 0;

	//check it's a valid type, get pType as a pointer to the type data
	if(type >= MAX_BUFFERS || type < 0)
	{
	#ifdef ELC
		LOG_ERROR(snd_invalid_number);
	#endif
		return 0;
	}
	pNewType = &sound_type_data[type];
	// Check if we are playing this type of sound
	if (pNewType->type > sound_opts) {
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
			printf("add_sound_object(%s) - inserting at index %d/%d\n",pNewType->name,i,used_sources);
	#endif //_EXTRA_SOUND_DEBUG
			if(!(pSource->sound_type < 0))
				insert_sound_source_at_index(i);
			break;
		}
	}

	//all sources are used by higher-priority sounds
	if(i==MAX_SOURCES)
	{
	#ifdef ELC
		LOG_ERROR(snd_sound_overflow);
	#else
		printf("add_sound_object ['%s',#%d] - sounds overflow!\n",pNewType->name,i);
	#endif
		return 0;
	}
	//this is the lowest-priority sound but there is a spare slot at the end of the list
	else if(i==used_sources)
	{
		source=i;
		pSource = insert_sound_source_at_index(used_sources);
	}

	//refuse to play a sound which doesn't have a main part
	if(pNewType->sample_indices[STAGE_MAIN]<0)
		return 0;

	//check we can load all buffers used by this type
	for(i=0;i<3;++i)
	{
		if(pNewType->sample_indices[i] >= 0 && ensure_sample_loaded(pNewType->sample_indices[i])!=0)
			return 0;
	}
	//we already quit if the sound has only an outro
	stage = pNewType->sample_indices[STAGE_INTRO] < 0 ? STAGE_MAIN : STAGE_INTRO;
	
	//initialise the source data to the first sample to be played for this sound
	pSource->play_duration = 0;
	pSource->sound_type = type;
	pSource->current_stage = stage;

	alSourcef(pSource->source, AL_PITCH, 1.0f);
	alSourcef(pSource->source, AL_GAIN, sound_gain);
	alSourcefv(pSource->source, AL_VELOCITY, sourceVel);
	alSourcefv(pSource->source, AL_POSITION, sourcePos);

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
					alSourceQueueBuffers(pSource->source,1,&buffer);
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
		LOG_ERROR("%s %d: %s", snd_source_error, source, alGetString(error));
	#else	
		printf("add_sound_object (%s): alSourceQueueBuffers = %s\n", pNewType->name,alGetString(error));
	#endif
		alSourcei(pSource->source,AL_BUFFER,0);
		UNLOCK_SOUND_LIST();
		pSource->sound_type = -1;
		pSource->current_stage = STAGE_UNUSED;
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
	//0 is infinite looping
	if(pNewType->sample_indices[STAGE_INTRO] < 0 && pNewType->loops==0)
		alSourcei(pSource->source,AL_LOOPING,AL_TRUE);
	else
		alSourcei(pSource->source,AL_LOOPING,AL_FALSE);

	tx = cx*(-2);
	ty = cy*(-2);
	distanceSq=(tx-x)*(tx-x)+(ty-y)*(ty-y);
	maxDistanceSq = pNewType->distance*pNewType->distance;

	alSourcePlay(pSource->source);
	if(sound_opts == SOUNDS_NONE || (pNewType->positional && (distanceSq > maxDistanceSq)))
		alSourcePause(pSource->source);

	UNLOCK_SOUND_LIST();
	
	pSource->cookie = next_cookie;
	//if next_cookie wraps around back to 0 (unlikely!) then address this.
	if(++next_cookie == 0)++next_cookie;
	return pSource->cookie;
}
#else
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
#endif	//NEW_SOUND

#ifndef NEW_SOUND
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
#endif	//NEW_SOUND


#ifdef NEW_SOUND
void sound_source_set_gain(unsigned long int cookie, float gain)
{
	int n;
	source_data *pSource;

	//source handle of 0 is a null source
	if(!have_sound || !cookie)
		return;
	//find which of our playing sources matches the handle passed
	for(n=0,pSource=sound_source_data;n<used_sources;++n,++pSource)
	{
		if(pSource->cookie == cookie)
		{
			alSourcef(pSource->source,AL_GAIN, sound_gain * gain);
			return;
		}
	}
}
#else
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
#endif	//NEW_SOUND

#ifdef NEW_SOUND
void update_sound(int ms)
{
	int i=0,error;
	//we rebuild the list of active sources, as some may have become
	//inactive due to the sound ending.
	source_data *pSource = sound_source_data;
	sound_sample *pSample;
	sound_type *pSoundType;
	ALuint buffer,deadBuffer,state;
	ALint numProcessed;

	int source;
	int x,y,distanceSq,maxDistSq;
	int relative;
	int tx=-cx*2;
	int ty=-cy*2;
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

	while(i<used_sources)
	{
		//this test should be redundant
		if(pSource->sound_type < 0 || pSource->current_stage==STAGE_UNUSED)
		{
#ifdef _EXTRA_SOUND_DEBUG
			printf("removing dud sound #%d\n",i);
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
			printf("'%s' has stopped after sample '%s'\n",pSoundType->name,pSample->file_path);
#endif //_EXTRA_SOUND_DEBUG
			pSource->current_stage = num_STAGES;
		}
		else
		{//find which buffer is playing
			alGetSourcei(pSource->source,AL_BUFFER,&buffer);
			if(buffer != pSample->buffer)
			{//has the source moved on to the next queued sample
#ifdef _EXTRA_SOUND_DEBUG
				printf("'%s' - sample '%s' has ended...",pSoundType->name,pSample->file_path);
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

		if(pSource->current_stage == num_STAGES)
		{//if the state is num_STAGES then the sound has ended (or gone wrong)
#ifdef _EXTRA_SOUND_DEBUG
			printf("removing finished sound '%s'\n",pSoundType->name);
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
				alSourcePause(pSource->source);
			else if (sound_opts != SOUNDS_NONE && (state == AL_PAUSED) && (distanceSq < pSoundType->distance*.9f))
				alSourcePlay(pSource->source);
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
}
#else
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
#endif	//NEW_SOUND

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
			if(exit_now) break;
		}
#endif	//NO_MUSIC
	return 1;
}

void stream_music(ALuint buffer)
{
#ifndef	NO_MUSIC
    char data[MUSIC_BUFFER_SIZE];
    int  size = 0;
    int  section = 0;
    int  result = 0;
	int error = 0;
	char str[256];

    while(size < MUSIC_BUFFER_SIZE)
    {
        result = ov_read(&ogg_stream, data + size, MUSIC_BUFFER_SIZE - size, 0, 2, 1,
						 &section);
		snprintf(str, sizeof(str), "%d", result); //prevents optimization errors under Windows, but how/why?
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

	alBufferData(buffer, AL_FORMAT_STEREO16, data, size, ogg_info->rate);

	if((error=alGetError()) != AL_NO_ERROR) 
    	{
    		LOG_ERROR("stream_music %s: %s", my_tolower(reg_error_str), alGetString(error));
			have_music=0;
    	}
#endif	//NO_MUSIC
}

#ifdef NEW_SOUND
//kill all the sounds.
//usefull when we change maps, etc.
void stop_all_sounds()
{
	int i;
#ifndef	NO_MUSIC
	int musQueued,musProcessed;
#endif //NO_MUSIC
	if(!have_sound || !used_sources)return;
	LOCK_SOUND_LIST();
	for(i=0;i<used_sources;++i)
	{
		stop_sound_source_at_index(i);
	}
#ifndef	NO_MUSIC
	if(!have_music)return;
	playing_music = 0;
	alSourceStop(music_source);
	alGetSourcei(music_source, AL_BUFFERS_PROCESSED, &musProcessed);
	alGetSourcei(music_source, AL_BUFFERS_QUEUED, &musQueued);
	while(musQueued-- > 0) {
		ALuint buffer;
		alSourceUnqueueBuffers(music_source, 1, &buffer);
	}
#endif	//NO_MUSIC
	UNLOCK_SOUND_LIST();
}
#else
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
#endif	//NEW_SOUND

void turn_sound_off()
{
	int i=0,loop;
	ALuint source;
	if(!inited)
		return;
#ifndef NO_MUSIC
	if(!music_on)
#endif //NO_MUSIC
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
			stop_sound_source_at_index(i);
			continue;
#else
			alSourceStop(source);
#endif	//NEW_SOUND
		}
		++i;
	}
	UNLOCK_SOUND_LIST();
}

void turn_sound_on()
{
	int i,state=0;
	ALuint source;
	if(!inited)
	{
#ifdef NEW_SOUND
		init_sound(SOUND_CONFIG_PATH);
#else
		init_sound();
#endif	//NEW_SOUND
	}
	if(!have_sound)
		return;
	LOCK_SOUND_LIST();
	sound_on=1;
	for(i=0;i<used_sources;i++)
	{
#ifdef NEW_SOUND
		source = sound_source_data[i].source;
#else
		source = sound_source[i];
#endif	//NEW_SOUND
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		if(state == AL_PAUSED)
			alSourcePlay(source);
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

void change_sounds(int * var, int value){
	if(!have_sound && value != SOUNDS_NONE) {
#ifdef NEW_SOUND
		init_sound(SOUND_CONFIG_PATH);
#else
		init_sound();
#endif	//NEW_SOUND
	}
	if(value == SOUNDS_NONE && sound_opts != SOUNDS_NONE) {
		turn_sound_off();
	} else if(value != SOUNDS_NONE && sound_opts == SOUNDS_NONE) {
		turn_sound_on();
	}
	if(value>=0) *var=value;
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

#ifdef NEW_SOUND
void init_sound(char *sound_config_path)
{
	source_data *pSource,*sourceArrays[1]={sound_source_data};//,temp_source_data};
	int error;
	int i,j;
	
	if(inited)
		return;

	//begin by setting all data to a known state
	if(have_sound)
		destroy_sound();

	//poison data
	for(i=0;i<MAX_SOURCES;i++)
	{
		for(j=0;j<sizeof(sourceArrays)/sizeof(sourceArrays[0]);++j)
		{
			pSource = sourceArrays[j]+i;
			pSource->source = 0;
			pSource->sound_type = -1;
			pSource->play_duration = 0;
			pSource->current_stage = STAGE_UNUSED;
			pSource->cookie = 0;
		}
	}
	for(i=0;i<MAX_BUFFERS;i++)
	{
		sound_sample_data[i].buffer = 0;
		sound_sample_data[i].file_path[0] = '\0';
		sound_sample_data[i].loaded_status = 0;
	}

	//initialise blank/default soundtype data
	for(i=0;i<MAX_BUFFERS;i++)
	{
		sound_type_data[i].name[0] = '\0';
		sound_type_data[i].sample_indices[STAGE_INTRO]=-1;
		sound_type_data[i].sample_indices[STAGE_MAIN]=-1;
		sound_type_data[i].sample_indices[STAGE_OUTRO]=-1;
		sound_type_data[i].stereo=0;
		sound_type_data[i].distance=100.0f;
		sound_type_data[i].positional=1;
		sound_type_data[i].loops=1;
		sound_type_data[i].fadeout_time=0;
		sound_type_data[i].echo_delay=0;
		sound_type_data[i].echo_volume=50;
		sound_type_data[i].time_of_the_day_flags=0xffff;
		sound_type_data[i].priority=5;
		sound_type_data[i].type=SOUNDS_ENVIRO;
	}
	
	//initialise OpenAL
	///NULL makes it use the default device.
	///if you want to use a different device, use, for example: ((ALubyte*) "DirectSound3D")
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

	have_sound=1;
#ifndef	NO_MUSIC
	have_music=1;
#else
	have_music=0;
#endif	//NO_MUSIC

	LOCK_SOUND_LIST();
	for(i=0;i<MAX_SOURCES;i++)
	{
		alGenSources(1, &sound_source_data[i].source);
		//temp_source_data[i].source = sound_source_data[i].source;
		if((error=alGetError()) != AL_NO_ERROR) 
		{
			//this error code is designed for a single source, -1 indicates multiple sources
		#ifdef ELC
			LOG_ERROR("%s : %s", snd_source_error, -1, alGetString(error));
		#endif
			have_sound=0;
			have_music=0;
			UNLOCK_SOUND_LIST();
			return;
		}
	}
	UNLOCK_SOUND_LIST();

	load_sound_config_data(sound_config_path);

#ifndef NEW_WEATHER
	///force the rain sound to be recreated
	rain_sound = 0;
#endif //NEW_WEATHER
	inited = 1;
}
#else
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
#endif	//NEW_SOUND

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
#endif	//NEW_SOUND
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
			alcCloseDevice(device);
	}

	if((error=alGetError()) != AL_NO_ERROR) 
	{
		char str[256];
		snprintf(str, sizeof(str), "%s: %s\n", snd_init_error, alGetString(error));
		LOG_TO_CONSOLE(c_red1, str);
		LOG_ERROR(str);
	}

#ifdef NEW_SOUND
	num_samples = 0;
#endif	//NEW_SOUND
	used_sources = 0;
}

#ifndef NEW_SOUND
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
#endif	//NEW_SOUND

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
#ifdef ELC
	LOG_TO_CONSOLE(c_red2, snd_no_music);
#endif
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

#ifdef NEW_SOUND
void load_sound_config_data(char *file)
{
	xmlDoc *doc;
	xmlNode *root=NULL;
	xmlNode *soundNode=NULL;
	xmlNode *attributeNode=NULL;

	xmlChar *content=NULL;
	xmlChar *introFile=NULL;
	xmlChar *mainFile=NULL;
	xmlChar *outroFile=NULL;

	int iVal=0;
	float fVal=0.0f;
	char *sVal = NULL;
	char path[1024];

	sound_type *pData=NULL;

#ifndef WINDOWS
	snprintf(path, sizeof(path), "%s/%s", datadir, file);
#else
	snprintf(path, sizeof(path), "%s", file);
#endif // !WINDOWS

	//can we open the file as xml?
	if((doc = xmlReadFile(path, NULL, 0)) == NULL)
	{
	#ifdef ELC
		char str[200];
		snprintf(str, sizeof(str), book_open_err_str, path);
		LOG_ERROR(str);
		LOG_TO_CONSOLE(c_red1,str);
	#endif
	}
	//can we get a root element
	else if ((root = xmlDocGetRootElement(doc))==NULL)
	{
	#ifdef ELC
		LOG_ERROR("Error while parsing: %s", path);
	#endif
	}
	//is the root the right type?
	else if ( xmlStrcmp( root->name, (xmlChar*)"sound_definitions" ) )
	{
	#ifdef ELC
		LOG_ERROR("Error in '%s' - root = '%s'", path,root->name);
	#endif
	}
	//we've found our expected root, now parse the children
	else
	{
		soundNode = root->xmlChildrenNode;
		while(soundNode != NULL)
		{
			//only look at sound objects
			if(!xmlStrcmp( soundNode->name, (xmlChar*)"sound" ) )
			{
				pData = &sound_type_data[num_types++];

				sVal = xmlGetProp(soundNode,(xmlChar*)"name");
				if(sVal)
					strncpy(pData->name,sVal,MAX_SOUND_NAME_LENGTH);
				else
				{
				#ifdef ELC
					LOG_ERROR("load_sound_config_data : sound has no name in '%s'",path);
				#else
					printf("load_sound_config_data : sound has no name in '%s'",path);
				#endif
				}
				attributeNode = soundNode->xmlChildrenNode;
				
				while(attributeNode != NULL)
				{
					content = xmlNodeListGetString(doc, attributeNode->xmlChildrenNode, 1);
					if(!xmlStrcmp (attributeNode->name, (xmlChar*)"intro_sound"))
					{
						if(pData->sample_indices[STAGE_INTRO] < 0)
						{
							pData->sample_indices[STAGE_INTRO] = store_sample_name(content);
						}
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : intro_index already set!");
					#else
							printf("load_sound_config_data : intro_index already set!\n");
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"main_sound"))
					{
						if(pData->sample_indices[STAGE_MAIN] < 0)
						{
							pData->sample_indices[STAGE_MAIN] = store_sample_name(content);
						}
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : main_index already set!");
					#else
							printf("load_sound_config_data : main_index already set!\n");
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"outro_sound"))
					{
						if(pData->sample_indices[STAGE_OUTRO] < 0)
						{
							pData->sample_indices[STAGE_OUTRO] = store_sample_name(content);
						}
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : outro_index already set!");
					#else
							printf("load_sound_config_data : outro_index already set!\n");
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"stereo"))
					{
						iVal = atoi(content);
						if(iVal==0 || iVal ==1)
							pData->stereo = iVal;
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : stereo = %d in '%s'",iVal,pData->name);
					#else
							printf("load_sound_config_data : stereo = %d in '%s'\n",iVal,pData->name);
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"distance"))
					{
						fVal = (float)atof(content);
						if(fVal>0.0f)
							pData->distance = fVal;
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : distance = %f in '%s'",fVal,pData->name);
					#else
							printf("load_sound_config_data : distance = %f in '%s'\n",fVal,pData->name);
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"positional"))
					{
						iVal = atoi(content);
						if(iVal==0 || iVal ==1)
							pData->positional = iVal;
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : positional = %d in '%s'",iVal,pData->name);
					#else
							printf("load_sound_config_data : positional = %d in '%s'\n",iVal,pData->name);
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"loops"))
					{
						iVal = atoi(content);
						if(iVal>=0)
							pData->loops = iVal;
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : loops = %d in '%s'",iVal,pData->name);
					#else
							printf("load_sound_config_data : loops = %d in '%s'\n",iVal,pData->name);
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"fadeout_time"))
					{
						iVal = atoi(content);
						if(iVal>=0)
							pData->fadeout_time = iVal;
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : fadeout_time = %d in '%s'",iVal,pData->name);
					#else
							printf("load_sound_config_data : fadeout_time = %d in '%s'\n",iVal,pData->name);
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"echo_delay"))
					{
						iVal = atoi(content);
						if(iVal>=0)
							pData->echo_delay = iVal;
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : echo_delay = %d in '%s'",iVal,pData->name);
					#else
							printf("load_sound_config_data : echo_delay = %d in '%s'\n",iVal,pData->name);
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"echo_volume"))
					{
						iVal = atoi(content);
						if(iVal>=0)
							pData->echo_volume = iVal;
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : echo_volume = %d in '%s'",iVal,pData->name);
					#else
							printf("load_sound_config_data : echo_volume = %d in '%s'\n",iVal,pData->name);
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"time_of_day_flags"))
					{
						sscanf(content,"%x",&iVal);
						if(iVal>=0 && iVal<=0xffff)
							pData->time_of_the_day_flags = iVal;
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : time_of_the_day_flags = 0x%x in '%s'",iVal,pData->name);
					#else
							printf("load_sound_config_data : time_of_the_day_flags = 0x%x in '%s'\n",iVal,pData->name);
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"priority"))
					{
						iVal = atoi(content);
						if(iVal>=0)
							pData->priority = iVal;
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : priority = %d in '%s'",iVal,pData->name);
					#else
							printf("load_sound_config_data : priority = %d in '%s'\n",iVal,pData->name);
					#endif
						}
					}
					else if(!xmlStrcmp (attributeNode->name, (xmlChar*)"type"))
					{
						if(!strcasecmp(content, "environmental")) {
							iVal = SOUNDS_ENVIRO;
						} else if(!strcasecmp(content, "actor")) {
							iVal = SOUNDS_ACTOR;
						} else if(!strcasecmp(content, "walking")) {
							iVal = SOUNDS_WALKING;
						}
						if(iVal>=0)
							pData->type = iVal;
						else
						{
					#ifdef ELC
							LOG_ERROR("load_sound_config_data : type = %s in '%s'",content,pData->name);
					#else
							printf("load_sound_config_data : type = %s in '%s'\n",content,pData->name);
					#endif
						}
					}
					attributeNode = attributeNode->next;
				}
				//this is a fix to a problem where a single looping sound would not loop.
//				if(pData->sample_indices[STAGE_INTRO] < 0 && pData->loops == 0)
//					pData->sample_indices[STAGE_INTRO] = pData->sample_indices[STAGE_MAIN];
			
			}
			soundNode = soundNode->next;
		}
	}

	xmlFree(doc);
}

int get_index_for_sound_type_name(char *name)
{
	int i;
	for(i=0;i<num_types;++i)
	{
		if(strcasecmp(sound_type_data[i].name,name)==0)
			return i;
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
		strncpy(sound_sample_data[num_samples].file_path,name,MAX_FILENAME_LENGTH);
		return num_samples++;
	}

	//all the 'slots' are full
	return -1;
}


//this takes a copy the first unused source object (or last one in the list if all used),
//moves all objects after #index along one place, and inserts the copied object at #index;
//it is ensured the source at #index is stopped with no buffers queued
source_data *insert_sound_source_at_index(unsigned int index)
{
	int i;
	source_data tempSource;
	//take a copy of the source about to be overwritten
	tempSource=sound_source_data[min(used_sources,MAX_SOURCES-1)];
	//ensure it is stopped and ready
	alSourcei(tempSource.source,AL_BUFFER,0);
	alSourceStop(tempSource.source);
	tempSource.play_duration=0;
	tempSource.current_stage=STAGE_UNUSED;
	tempSource.sound_type=-1;
	tempSource.cookie=0;

	//shunt source objects down a place
	for(i=min(used_sources,MAX_SOURCES-1);i>index;--i)
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
#endif	//NEW_SOUND

#ifdef DEBUG
#ifdef NEW_SOUND
void print_sound_types()
{
	int i,sample;
	sound_type *pData=NULL;
	printf("\nSOUND TYPE DATA\n===============\n");
	printf("There are %d sound types (max %d) using %d samples (max %d):\n",
		num_types,MAX_BUFFERS,num_samples,MAX_BUFFERS);

	for(i=0;i<num_types;++i)
	{
		pData = &sound_type_data[i];
		printf("Sound type '%s':\n",pData->name);
		sample=pData->sample_indices[STAGE_INTRO];
		printf("\tIntro sample = '%s'(#%d)\n",(sample < 0) ? NULL : sound_sample_data[sample].file_path,sample);
		sample=pData->sample_indices[STAGE_MAIN];
		printf("\tMain sample = '%s'(#%d)\n",(sample < 0) ? NULL : sound_sample_data[sample].file_path,sample);
		sample=pData->sample_indices[STAGE_OUTRO];
		printf("\tOutro sample = '%s'(#%d)\n",(sample < 0) ? NULL : sound_sample_data[sample].file_path,sample);
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
			printf("\tFrequency = %d\n"				, pData->freq);
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
#else
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
#endif //NEW_SOUND
#endif //_DEBUG

#include <string.h>
#include <math.h>
#include "global.h"

int used_sources = 0;

char sound_files[max_buffers][30];
ALuint sound_source[max_sources];
ALuint sound_buffer[max_buffers];
SDL_mutex *sound_list_mutex;

#ifndef	NO_MUSIC
char music_files[max_songs][30];
FILE* ogg_file;
OggVorbis_File ogg_stream;
vorbis_info* ogg_info;

ALuint music_buffers[2];
ALuint music_source;

int playing_music = 0;
#endif	//NO_MUSIC

void stop_sound(int i)
{
	if(!have_sound)return;
	alSourceStop(i);
}

void load_ogg_file(int i) {
#ifndef	NO_MUSIC
	ov_clear(&ogg_stream);

	ogg_file = fopen(music_files[i], "rb");
	if(!ogg_file) {
		char	str[256];
		snprintf(str, 256, "Failed to load ogg file: %s\n", music_files[i]);
		log_to_console(c_red1, str);
		LogError(str);
		have_music=0;
		return;
	}

	if(ov_open(ogg_file, &ogg_stream, NULL, 0) < 0) {
		log_to_console(c_red1, "Failed to load ogg stream\n");
		LogError("Failed to load ogg stream");
		have_music=0;
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
	if(!alIsBuffer(sound_buffer[i]))
		{
			alGenBuffers(1, sound_buffer+i);
			
			if((error=alGetError()) != AL_NO_ERROR) 
				{
					char	str[256];
					snprintf(str, 256, "Error creating buffer: %s\n", alGetString(error));
					log_to_console(c_red1, str);
					LogError(str);
					have_sound=0;
					have_music=0;
				}

			alutLoadWAVFile(sound_files[i],&format,&data,&size,&freq,&loop);
			alBufferData(sound_buffer[i],format,data,size,freq);
			alutUnloadWAV(format,data,size,freq);
		}
	return sound_buffer[i];
}

void play_music(int i) {
#ifndef	NO_MUSIC
    int error,queued;
    if(!have_music)return;

	if(i >= max_songs)
		{
			LogError("Got invalid song number");
			return;
		}

	alSourceStop(music_source);
	alGetSourcei(music_source, AL_BUFFERS_QUEUED, &queued);
	while(queued-- > 0)
		{
			ALuint buffer;
			
			alSourceUnqueueBuffers(music_source, 1, &buffer);
		}
	
	load_ogg_file(i);

    stream_music(music_buffers[0]);
	stream_music(music_buffers[1]);
    
    alSourceQueueBuffers(music_source, 2, music_buffers);
    alSourcePlay(music_source);

	if((error=alGetError()) != AL_NO_ERROR) 
    	{
     		char	str[256];
    		snprintf(str, 256, "play_music error: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		LogError(str);
			have_music=0;
    	}
	playing_music = 1;
#endif	//NO_MUSIC
}

int add_sound_object(int sound_file,int x, int y,int positional,int loops)
{
	int error,tx,ty,distance,i;
	ALfloat sourcePos[]={ x, y, 0.0};
	ALfloat sourceVel[]={ 0.0, 0.0, 0.0};

	if(!have_sound)return 0;

	if(sound_file >= max_buffers)
		{
			LogError("Got invalid sound number\n");
			return 0;
		}

	lock_sound_list();

	i=used_sources++;

	if(used_sources>max_sources)
		i=realloc_sources();
	if(i<0)
		return 0;

	tx=-cx*2;
	ty=-cy*2;
	distance=sqrt((tx-x)*(tx-x)+(ty-y)*(ty-y));

	alGenSources(1, &sound_source[i]);

	if((error=alGetError()) != AL_NO_ERROR) 
    	{
    		char	str[256];
    		snprintf(str, 256, "Error creating a source %d: %s\n", i, alGetString(error));
    		log_to_console(c_red1, str);
    		LogError(str);
			have_sound=0;
			have_music=0;
			return 0;
    	}

	alSourcef(sound_source[i], AL_PITCH, 1.0f);
	alSourcef(sound_source[i], AL_GAIN, 1.0f);
	alSourcei(sound_source[i], AL_BUFFER,get_loaded_buffer(sound_file));
	alSourcefv(sound_source[i], AL_VELOCITY, sourceVel);
	alSourcefv(sound_source[i], AL_POSITION, sourcePos);
	if(!positional)
		alSourcei(sound_source[i], AL_SOURCE_RELATIVE, AL_TRUE);
	else 
		{
			alSourcei(sound_source[i], AL_SOURCE_RELATIVE, AL_FALSE);
			alSourcef(sound_source[i], AL_REFERENCE_DISTANCE , 15.0f);
			alSourcef(sound_source[i], AL_ROLLOFF_FACTOR , 5.0f);
		}

	if (loops)
		{
			alSourcei(sound_source[i], AL_LOOPING, AL_TRUE);
			alSourcePlay(sound_source[i]);
			if(!sound_on || (positional && (distance > 30)))
				alSourcePause(sound_source[i]);
		}
	else
		{
			alSourcei(sound_source[i], AL_LOOPING, AL_FALSE);
			if(sound_on)
				alSourcePlay(sound_source[i]);
		}
	unlock_sound_list();
	return sound_source[i];
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
	lock_sound_list();

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
			distance=sqrt((tx-x)*(tx-x)+(ty-y)*(ty-y));
			if((state == AL_PLAYING) && (distance > 30))
				alSourcePause(sound_source[i]);
			else if (sound_on && (state == AL_PAUSED) && (distance < 25))
				alSourcePlay(sound_source[i]);
		}
	if((error=alGetError()) != AL_NO_ERROR) 
    	{
     		char	str[256];
    		snprintf(str, 256, "update_position error: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		LogError(str);
			have_sound=0;
			have_music=0;
    	}
	unlock_sound_list();
}

void update_music()
{
#ifndef	NO_MUSIC
    int error,processed,state;
	if(!have_music || !playing_music)return;

    alGetSourcei(music_source, AL_BUFFERS_PROCESSED, &processed);
	alGetSourcei(music_source, AL_SOURCE_STATE, &state);

	if(!processed)return; //skip error checking et al
    while(processed-- > 0)
    {
        ALuint buffer;
        
        alSourceUnqueueBuffers(music_source, 1, &buffer);
		stream_music(buffer);
		alSourceQueueBuffers(music_source, 1, &buffer);
    }
	if(state == AL_STOPPED)
		alSourcePlay(music_source);
	if((error=alGetError()) != AL_NO_ERROR)
    	{
     		char	str[256];
    		snprintf(str, 256, "update_music error: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		LogError(str);
			have_music=0;
    	}
#endif	//NO_MUSIC
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
		sprintf(str, "%d", result);	//prevents optimization errors under Windows, but how/why?
        if(result > 0)
            size += result;
        else if(result < 0)
			ogg_error(result);
		else
			break;
    }
	if(!size)return;

	alBufferData(buffer, AL_FORMAT_STEREO16, data, size, ogg_info->rate);

	if((error=alGetError()) != AL_NO_ERROR) 
    	{
    		snprintf(str, 256, "stream_music error: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		LogError(str);
			have_music=0;
    	}
#endif	//NO_MUSIC
}


//kill all the sounds that loop infinitely
//usefull when we change maps, etc.
void kill_local_sounds()
{
	int error;
	if(!have_sound || !used_sources)return;
	lock_sound_list();
	alSourceStopv(used_sources,sound_source);
	if((error=alGetError()) != AL_NO_ERROR) 
    	{
     		char	str[256];
    		snprintf(str, 256, "kill_local_sounds error: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		LogError(str);
			have_sound=0;
			have_music=0;
    	}
	if(realloc_sources())
		{
			log_to_console(c_red1, "Failed to stop all sounds.\n");
			LogError("Failed to stop all sounds.");
		}
	unlock_sound_list();
}

void turn_sound_off()
{
	int i,loop;
	if(!have_sound)return;
	lock_sound_list();
	sound_on=0;
	for(i=0;i<used_sources;i++)
		{
			alGetSourcei(sound_source[i], AL_LOOPING, &loop);
			if(loop == AL_TRUE)
				alSourcePause(sound_source[i]);
			else
				alSourceStop(sound_source[i]);
		}
	unlock_sound_list();
}

void turn_sound_on()
{
	int i,state=0;
	if(!have_sound)return;
	lock_sound_list();
	sound_on=1;
	for(i=0;i<used_sources;i++)
		{
			alGetSourcei(sound_source[i], AL_SOURCE_STATE, &state);
			if(state == AL_PAUSED)
				alSourcePlay(sound_source[i]);
		}
	unlock_sound_list();
}

void turn_music_off()
{
#ifndef	NO_MUSIC
	if(!have_music)return;
	music_on=0;
	alSourcePause(music_source);
	playing_music = 0;
#endif	//NO_MUSIC
}

void turn_music_on()
{
#ifndef	NO_MUSIC
	static int i = ogg_el1 - 1;
	//int state
	if(!have_music)return;
	music_on=1;
	i++;
	if(i >= max_songs) i = 0;
	play_music(i);
	//alGetSourcei(music_source, AL_SOURCE_STATE, &state);
	//if(state == AL_PAUSED) {
	//alSourcePlay(music_source);
	playing_music = 1;
	//}
#endif	//NO_MUSIC
}

void init_sound()
{
	int i,error;
	ALfloat listenerPos[]={-cx*2,-cy*2,0.0};
	ALfloat listenerVel[]={0.0,0.0,0.0};
	ALfloat listenerOri[]={0.0,0.0,0.0,0.0,0.0,0.0};
	have_sound=1;
#ifndef	NO_MUSIC
	have_music=1;
#else
	have_music=0;
#endif	//NO_MUSIC
	alutInit(0, NULL);
	sound_list_mutex=SDL_CreateMutex();

	if((error=alGetError()) != AL_NO_ERROR) 
    	{
     		char	str[256];
    		snprintf(str, 256, "Error initializing sound: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		LogError(str);
			have_sound=0;
			have_music=0;
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

#ifndef	NO_MUSIC
	my_strcp(music_files[ogg_housewaltz],"./music/housewaltz.ogg");
    my_strcp(music_files[ogg_overworld],"./music/overworld.ogg");
	my_strcp(music_files[ogg_windyvillage],"./music/windyvillage.ogg");
	my_strcp(music_files[ogg_mountainwoods],"./music/mountainwoods.ogg");
	my_strcp(music_files[ogg_thedarkness],"./music/thedarkness.ogg");
	my_strcp(music_files[ogg_el1],"./music/el1.ogg");
	my_strcp(music_files[ogg_el2],"./music/el2.ogg");
	my_strcp(music_files[ogg_el3],"./music/el3.ogg");
	my_strcp(music_files[ogg_el4],"./music/el4.ogg");
	my_strcp(music_files[ogg_el5],"./music/el5.ogg");
	my_strcp(music_files[ogg_el6],"./music/el6.ogg");
	my_strcp(music_files[ogg_el7],"./music/el7.ogg");
#endif	//NO_MUSIC

	alListenerfv(AL_POSITION,listenerPos);
	alListenerfv(AL_VELOCITY,listenerVel);
	alListenerfv(AL_ORIENTATION,listenerOri);

	//poison data
	for(i=0;i<max_sources;i++)
		sound_source[i] = -1;
	for(i=0;i<max_buffers;i++)
		sound_buffer[i] = -1;

	//initialize music
#ifndef	NO_MUSIC
	ogg_file = NULL;

    alGenBuffers(2, music_buffers);
	alGenSources(1, &music_source);
    alSource3f(music_source, AL_POSITION,        0.0, 0.0, 0.0);
    alSource3f(music_source, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(music_source, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef (music_source, AL_ROLLOFF_FACTOR,  0.0          );
    alSourcei (music_source, AL_SOURCE_RELATIVE, AL_TRUE      );
#endif	//NO_MUSIC
}

void destroy_sound()
{
	int i;
	if(!have_sound)return;
	SDL_DestroyMutex(sound_list_mutex);
	sound_list_mutex=NULL;

#ifndef	NO_MUSIC
	alSourceStop(music_source);
    alDeleteSources(1, &music_source);
    alDeleteBuffers(2, music_buffers);
    ov_clear(&ogg_stream);
#endif	//NO_MUSIC
	alSourceStopv(used_sources, sound_source);
	alDeleteSources(used_sources, sound_source);
	for(i=0;i<max_buffers;i++)
		if(alIsBuffer(sound_buffer[i]))
			alDeleteBuffers(1, sound_buffer+i);
    alutExit();
}

int realloc_sources()
{
	int i;
	int state,error;
	int still_used=0;
	ALuint new_sources[max_sources];
	if(used_sources>max_sources)
		used_sources=max_sources;
	for(i=0;i<used_sources;i++)
		{
			alGetSourcei(sound_source[i], AL_SOURCE_STATE, &state);
			if((state == AL_PLAYING) || (state == AL_PAUSED))
				{
					new_sources[still_used] = sound_source[i];
					still_used++;
				}
			else
				alDeleteSources(1, &sound_source[i]);
		}

	for(i=0;i<still_used;i++)
		sound_source[i] = new_sources[i];
	for(i=still_used;i<max_sources;i++)
		sound_source[i]=-1;
	used_sources=still_used;
	
	if((error=alGetError()) != AL_NO_ERROR) 
    	{
     		char	str[256];
    		snprintf(str, 256, "realloc_sources error: %s", alGetString(error));
    		log_to_console(c_red1, str);
    		LogError(str);
			have_sound=0;
			have_music=0;
    	}
	if(used_sources>=max_sources) 
    	{
    		log_to_console(c_red1,"Too many sounds.\n");
    		LogError("Too many sounds.");
			return -1;
    	}
	else
		return used_sources;
}

void ogg_error(int code)
{
#ifndef	NO_MUSIC
	char error_string[80];
    switch(code)
    {
        case OV_EREAD:
            strcpy(error_string, "Read from media.\n"); break;
        case OV_ENOTVORBIS:
            strcpy(error_string, "Not Vorbis data.\n"); break;
        case OV_EVERSION:
            strcpy(error_string, "Vorbis version mismatch.\n"); break;
        case OV_EBADHEADER:
            strcpy(error_string, "Invalid Vorbis header.\n"); break;
        case OV_EFAULT:
            strcpy(error_string, "Internal logic fault (bug or heap/stack corruption.\n"); break;
        default:
            strcpy(error_string, "Unknown Ogg error.\n");
    }
	log_to_console(c_red1,error_string);
	log_error(error_string);
#endif	//NO_MUSIC
}

#include <string.h>
#include <math.h>
#include "global.h"

int used_sources;

char sound_files[max_buffers][30];
ALuint sound_source[max_sources];
ALuint sound_buffer[max_buffers];
SDL_mutex *sound_list_mutex;

char music_files[max_songs][30];
FILE* ogg_file;
OggVorbis_File ogg_stream;

ALuint music_buffers[2];
ALuint music_source;

void stop_sound(int i)
{
	if(!have_sound)return;
	alSourceStop(i);
}

void load_ogg_file(int i) {
	ogg_file = fopen(music_files[i], "rb");
	if(!ogg_file) {
		char	str[256];
		sprintf(str, "Failed to load ogg file: %s\n", music_files[i]);
		log_to_console(c_red1, str);
		log_error(str);
		have_music=0;
		return;
	}

	ov_clear(&ogg_stream);

	if(ov_open(ogg_file, &ogg_stream, NULL, 0) < 0) {
		log_to_console(c_red1, "Failed to load ogg stream\n");
		log_error("Failed to load ogg stream\n");
		have_music=0;
	}
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
					sprintf(str, "Error creating buffer: %s\n", alGetString(error));
					log_to_console(c_red1, str);
					log_error(str);
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

    int error,queued;
    if(!have_music)return;

	if(i >= max_songs)
		{
			log_error("Got invalid song number\n");
			return;
		}

    alSourceStop(music_source);
    alGetSourcei(music_source, AL_BUFFERS_QUEUED, &queued);

    while(queued--)
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
    		sprintf(str, "play_music error: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		log_error(str);
			have_music=0;
    	}
}

int add_sound_object(int sound_file,int x, int y,int positional,int loops)
{
	int error,tx,ty,distance,i;
	ALfloat sourcePos[]={ x, y, 0.0};
	ALfloat sourceVel[]={ 0.0, 0.0, 0.0};

	if(!have_sound)return 0;

	if(sound_file >= max_buffers)
		{
			log_error("Got invalid sound number\n");
			return 0;
		}

	lock_sound_list();

	i=used_sources;
	used_sources++;

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
    		sprintf(str, "Error creating a source %d: %s\n", i, alGetString(error));
    		log_to_console(c_red1, str);
    		log_error(str);
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
    		sprintf(str, "update_position error: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		log_error(str);
			have_sound=0;
			have_music=0;
    	}
	unlock_sound_list();
}

void update_music() {

    int error,processed;
	if(!have_music)return;

    alGetSourcei(music_source, AL_BUFFERS_PROCESSED, &processed);

	if(!processed)return; //skip error checking et al
    while(processed--)
    {
        ALuint buffer;
        
        alSourceUnqueueBuffers(music_source, 1, &buffer);
		stream_music(buffer);
        alSourceQueueBuffers(music_source, 1, &buffer);
    }
	if((error=alGetError()) != AL_NO_ERROR) 
    	{
     		char	str[256];
    		sprintf(str, "update_music error: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		log_error(str);
			have_music=0;
    	}
}

void stream_music(ALuint buffer) {
    char data[BUFFER_SIZE];
    int  size = 0;
    int  section;
    int  result = 0;
	int error;

    while(size < BUFFER_SIZE)
    {
        result = ov_read(&ogg_stream, data + size, BUFFER_SIZE - size, 0, 2, 1,
						 &section);
    
        if(result > 0)
            size += result;
        else
            break;
    }
	if(!size)return;

	alBufferData(buffer, AL_FORMAT_STEREO16, data, size, ov_bitrate(&ogg_stream,-1));

	if((error=alGetError()) != AL_NO_ERROR) 
    	{
     		char	str[256];
    		sprintf(str, "stream_music error: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		log_error(str);
			have_music=0;
    	}
}


//kill all the sounds that loop infinitely
//usefull when we change maps, etc.
void kill_local_sounds()
{
	int error;
	if(!have_sound)return;
	lock_sound_list();
	alSourceStopv(used_sources,sound_source);
	if((error=alGetError()) != AL_NO_ERROR) 
    	{
     		char	str[256];
    		sprintf(str, "kill_local_sounds error: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		log_error(str);
			have_sound=0;
			have_music=0;
    	}
	if(realloc_sources())
		{
			log_to_console(c_red1, "Failed to stop all sounds.\n");
			log_error("Failed to stop all sounds.\n");
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
	if(!have_music)return;
	music_on=0;
	alSourcePause(music_source);
}

void turn_music_on()
{
	static int i = -1;
	if(!have_music)return;
	music_on=1;
	i++;
	if(i >= max_songs) i = 0;
	play_music(i);
	//alSourcePlay(music_source);
}

void init_sound()
{
	int i,error;
	ALfloat listenerPos[]={-cx*2,-cy*2,0.0};
	ALfloat listenerVel[]={0.0,0.0,0.0};
	ALfloat listenerOri[]={0.0,0.0,0.0,0.0,0.0,0.0};
	have_sound=1;
	have_music=1;

	alutInit(0, NULL);
	sound_list_mutex=SDL_CreateMutex();

	if((error=alGetError()) != AL_NO_ERROR) 
    	{
     		char	str[256];
    		sprintf(str, "Error initializing sound: %s\n", alGetString(error));
    		log_to_console(c_red1, str);
    		log_error(str);
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

	my_strcp(music_files[ogg_housewaltz],"./music/housewaltz.ogg");
    my_strcp(music_files[ogg_overworld],"./music/overworld.ogg");
	my_strcp(music_files[ogg_windyvillage],"./music/windyvillage.ogg");
	my_strcp(music_files[ogg_mountainwoods],"./music/mountainwoods.ogg");
	my_strcp(music_files[ogg_thedarkness],"./music/thedarkness.ogg");

	alListenerfv(AL_POSITION,listenerPos);
	alListenerfv(AL_VELOCITY,listenerVel);
	alListenerfv(AL_ORIENTATION,listenerOri);

	//poison data
	for(i=0;i<max_sources;i++)
		sound_source[i] = -1;
	for(i=0;i<max_buffers;i++)
		sound_buffer[i] = -1;

	//initialize music

	ogg_file = NULL;
	
    alGenBuffers(2, music_buffers);
    alGenSources(1, &music_source);
    alSource3f(music_source, AL_POSITION,        0.0, 0.0, 0.0);
    alSource3f(music_source, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(music_source, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef (music_source, AL_ROLLOFF_FACTOR,  0.0          );
    alSourcei (music_source, AL_SOURCE_RELATIVE, AL_TRUE      );
}

void destroy_sound()
{
	int i;
	if(!have_sound)return;
	SDL_DestroyMutex(sound_list_mutex);
	sound_list_mutex=NULL;

	alSourceStop(music_source);
	alSourceStopv(used_sources, sound_source);
	alDeleteSources(used_sources, sound_source);
    alDeleteSources(1, &music_source);
	for(i=0;i<max_buffers;i++)
		if(alIsBuffer(sound_buffer[i]))
			alDeleteBuffers(1, sound_buffer+i);
    alDeleteBuffers(2, music_buffers);
    ov_clear(&ogg_stream);
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
    		sprintf(str, "realloc_sources error.\n %s", alGetString(error));
    		log_to_console(c_red1, str);
    		log_error(str);
			have_sound=0;
			have_music=0;
    	}
	if(used_sources>=max_sources) 
    	{
    		log_to_console(c_red1,"Too many sounds.\n");
    		log_error("Too many sounds.\n");
			return -1;
    	}
	else
		return used_sources;
}

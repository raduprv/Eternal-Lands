#include <string.h>
#include "global.h"
#include <math.h>

void stop_sound(int i)
{
	if(!have_sound)return;
	alSourceStop(i);
}

int add_sound_object(int sound_file,int x, int y,int positional,int loops)
{
	int error,tx,ty,distance,i=0;
	ALfloat sourcePos[]={ x, y, 0.0};
	ALfloat sourceVel[]={ 0.0, 0.0, 0.0};

	if(!have_sound)return 0;
	lock_sound_list();

	if(used_sources)
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
    		log_to_console(c_red1,"Error creating a source.\n");
    		log_error("Error creating a source.\n");
			printf("%s\n",alGetString(error));
			have_sound=0;
			have_music=0;
			return 0;
    	}

	alSourcef(sound_source[i], AL_PITCH, 1.0f);
	alSourcef(sound_source[i], AL_GAIN, 1.0f);
	alSourcei(sound_source[i], AL_BUFFER,sound_buffer[sound_file]);
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
			if(positional && (distance < 25))
				alSourcePause(sound_source[i]);
		}
	unlock_sound_list();
	return sound_source[i];
}

void update_position()
{
	int i,state,relative,error;
	int x,y,tx,ty,distance;
	ALfloat sourcePos[3];

	if(!have_sound)return;
	lock_sound_list();

	tx=-cx*2;
	ty=-cy*2;

	ALfloat listenerPos[]={tx,ty,0.0};
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
    		log_to_console(c_red1,"update_position error.\n");
    		log_error("update_position error.\n");
			printf("%s\n",alGetString(error));
			have_sound=0;
			have_music=0;
    	}
	unlock_sound_list();
}

//kill all the sounds that loop infinitely
//usefull when we change maps, etc.
void kill_local_sounds()
{
	int tostop,error;
	if(!have_sound)return;
	lock_sound_list();
	tostop=used_sources;
	if(used_sources)
		tostop--;
	alSourceStopv(tostop,sound_source);
	if((error=alGetError()) != AL_NO_ERROR) 
    	{
    		log_to_console(c_red1,"kill_local_sounds error.\n");
    		log_error("kill_local_sounds error.\n");
			printf("%s\n",alGetString(error));
			have_sound=0;
			have_music=0;
    	}
	unlock_sound_list();
}

void turn_sound_off()
{
	int i,loop;
	if(!have_sound)return;
	sound_on=0;
	for(i=0;i<used_sources;i++)
		{
			alGetSourcei(sound_source[i], AL_LOOPING, &loop);
			if(loop == AL_TRUE)
				alSourcePause(sound_source[i]);
			else
				alSourceStop(sound_source[i]);
		}
}

void turn_sound_on()
{
	int i,state=0;
	if(!have_sound)return;
	sound_on=1;
	for(i=0;i<used_sources;i++)
		{
			alGetSourcei(sound_source[i], AL_SOURCE_STATE, &state);
			if(state == AL_PAUSED)
				alSourcePlay(sound_source[i]);
		}
}

void init_sound()
{
	int i;
	have_sound=1;
	have_music=1;
	ALsizei size,freq;
	ALenum  format;
	ALvoid  *data;
	ALboolean loop;
	ALfloat listenerPos[]={-cx*2,-cy*2,0.0};
	ALfloat listenerVel[]={0.0,0.0,0.0};
	ALfloat listenerOri[]={0.0,0.0,0.0,0.0,0.0,0.0};

	alutInit(0, NULL) ; 
	sound_list_mutex=SDL_CreateMutex();

	if(alGetError() != AL_NO_ERROR) 
    	{
    		log_to_console(c_red1,"Error initializing sound.\n");
    		log_error("Error initializing sound.\n");
			have_sound=0;
			have_music=0;
    	}

	// Generate buffers
	alGenBuffers(max_buffers, sound_buffer);
    
	if(alGetError() != AL_NO_ERROR) 
    	{
    		log_to_console(c_red1,"Error creating buffers.\n");
    		log_error("Error creating buffers.\n");
			have_sound=0;
			have_music=0;
    	}
	
	my_strcp(sound_files[0],"./sound/rain1.wav");
	my_strcp(sound_files[1],"./sound/teleport_in.wav");
	my_strcp(sound_files[2],"./sound/teleport_out.wav");
	my_strcp(sound_files[3],"./sound/teleporter.wav");
	my_strcp(sound_files[4],"./sound/thunder1.wav");
	my_strcp(sound_files[5],"./sound/thunder2.wav");
	my_strcp(sound_files[6],"./sound/thunder3.wav");
	my_strcp(sound_files[7],"./sound/thunder4.wav");
	my_strcp(sound_files[8],"./sound/thunder5.wav");

	for(i=0;i<max_buffers;i++)
		{
#ifndef WINDOWS
			alutLoadWAVFile(sound_files[i],&format,&data,&size,&freq,&loop);
#else
			alutLoadWAVFile(sound_files[i],&format,&data,&size,&freq);
#endif
			alBufferData(sound_buffer[i],format,data,size,freq);
			alutUnloadWAV(format,data,size,freq);
		}

	alListenerfv(AL_POSITION,listenerPos);
	alListenerfv(AL_VELOCITY,listenerVel);
	alListenerfv(AL_ORIENTATION,listenerOri);
}

void destroy_sound()
{ 
	SDL_DestroyMutex(sound_list_mutex);
	sound_list_mutex=NULL;

	alSourceStopv(used_sources, sound_source);
	alDeleteSources(used_sources, sound_source);
	alDeleteBuffers(max_buffers, sound_buffer);
    alutExit();
}

int realloc_sources()
{
	int i;
	int state,error;
	int still_used=0;
	ALuint new_sources[max_sources];
	for(i=0;i<max_sources;i++)
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
	used_sources=still_used;
	
	if((error=alGetError()) != AL_NO_ERROR) 
    	{
    		log_to_console(c_red1,"realloc_sources error.\n");
    		log_error("realloc_sources error.\n");
			printf("%s\n",alGetString(error));
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

#include <string.h>
#include <math.h>
#include "global.h"

int have_sound=0;
int have_music=0;
int sound_on=1;
int music_on=1;
int no_sound=0;

ALfloat sound_gain=1.0f;
ALfloat music_gain=1.0f;

int used_sources = 0;

char sound_files[max_buffers][30];
ALuint sound_source[max_sources];
ALuint sound_buffer[max_buffers];
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

void stop_sound(int i)
{
	if(!have_sound)return;
	alSourceStop(i);
}

void get_map_playlist()
{
#ifndef	NO_MUSIC
	int len,i=0;
	char map_list_file_name[256];
	FILE *fp;
	char strLine[255];

	if(!have_music)return;

	memset(playlist,0,sizeof(playlist));

	strcpy(map_list_file_name,"./music");
	strcat(map_list_file_name,map_file_name+6);
	len=strlen(map_list_file_name);
	map_list_file_name[len-3]='p';
	map_list_file_name[len-2]='l';
	map_list_file_name[len-1]='l';

	fp=fopen(map_list_file_name,"r");
	if(!fp)return;

	while(1)
		{
			fscanf(fp,"%d %d %d %d %d %s",&playlist[i].min_x,&playlist[i].min_y,&playlist[i].max_x,&playlist[i].max_y,&playlist[i].time,playlist[i].file_name);
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
     		char	str[256];
    		snprintf(str, 256,"play_ogg_file %s: %s", my_tolower(reg_error_str), alGetString(error));
    		LogError(str);
			have_music=0;
			return;
    	}
	playing_music = 1;	
#endif	//NO_MUSIC
}

void load_ogg_file(char *file_name) {
#ifndef	NO_MUSIC
	char file_name2[80];

	if(!have_music)return;

	ov_clear(&ogg_stream);

	if(file_name[0]!='.' && file_name[0]!='/') {
		strcpy(file_name2,"./music/");
		strcat(file_name2,file_name);
	} else
		strcpy(file_name2,file_name);

	ogg_file = fopen(file_name2, "rb");

	if(!ogg_file) {
		char	str[256];
		snprintf(str, 256, "%s: %s", snd_ogg_load_error, file_name);
		LogError(str);
		have_music=0;
		return;
	}

	if(ov_open(ogg_file, &ogg_stream, NULL, 0) < 0) {
		LogError(snd_ogg_stream_error);
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
					snprintf(str, 256, "%s: %s",snd_buff_error, alGetString(error));
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

void play_music(int list) {
#ifndef	NO_MUSIC
	int i=0;
	char list_file_name[256];
	FILE *fp;
	char strLine[255];

	if(!have_music)return;

	sprintf(list_file_name,"./music/%d.pll",list);
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

int add_sound_object(int sound_file,int x, int y,int positional,int loops)
{
	int error,tx,ty,distance,i;
	ALfloat sourcePos[]={ x, y, 0.0};
	ALfloat sourceVel[]={ 0.0, 0.0, 0.0};

	if(!have_sound)return 0;

	if(sound_file >= max_buffers)
		{
			LogError(snd_invalid_number);
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
	distance=(tx-x)*(tx-x)+(ty-y)*(ty-y);

	alGenSources(1, &sound_source[i]);

	if((error=alGetError()) != AL_NO_ERROR) 
    	{
    		char	str[256];
    		snprintf(str, 256, "%s %d: %s", snd_source_error, i, alGetString(error));
    		LogError(str);
			have_sound=0;
			have_music=0;
			return 0;
    	}

	alSourcef(sound_source[i], AL_PITCH, 1.0f);
	alSourcef(sound_source[i], AL_GAIN, sound_gain);
	alSourcei(sound_source[i], AL_BUFFER,get_loaded_buffer(sound_file));
	alSourcefv(sound_source[i], AL_VELOCITY, sourceVel);
	alSourcefv(sound_source[i], AL_POSITION, sourcePos);
	if(!positional)
		alSourcei(sound_source[i], AL_SOURCE_RELATIVE, AL_TRUE);
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
			distance=(tx-x)*(tx-x)+(ty-y)*(ty-y);
			if((state == AL_PLAYING) && (distance > 35*35))
				alSourcePause(sound_source[i]);
			else if (sound_on && (state == AL_PAUSED) && (distance < 30*30))
				alSourcePlay(sound_source[i]);
		}
	if((error=alGetError()) != AL_NO_ERROR) 
    	{
     		char	str[256];
    		snprintf(str, 256, "update_position %s: %s", my_tolower(reg_error_str), alGetString(error));
    		LogError(str);
			have_sound=0;
			have_music=0;
    	}
	unlock_sound_list();
}

int update_music(void *dummy)
{
#ifndef	NO_MUSIC
    int error,processed,state,state2,sleep,fade=0;
    char str[256];
   	sleep = SLEEP_TIME;
	while(have_music)
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
							log_to_console(c_red1, snd_skip_speedup);
							//on slower systems, music can skip up to 10 times
							//if it skips more, it just can't play the music...
							if(sleep > (SLEEP_TIME / 10))
								sleep -= (SLEEP_TIME / 10);
							else if(sleep > 1) sleep = 1;
							else
								{
									log_to_console(c_red1, snd_too_slow);
									LogError(snd_too_slow);
									turn_music_off();
									sleep = SLEEP_TIME;
									break;
								}
							alSourcePlay(music_source);
						}
					if((error=alGetError()) != AL_NO_ERROR)
						{
							snprintf(str, 256, "update_music %s: %s", my_tolower(reg_error_str), alGetString(error));
							LogError(str);
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
		}
#endif	//NO_MUSIC
	return 0;
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
	if(!size)
		{
			playing_music = 0;//file's done, quit trying to play
			return;
		}

	alBufferData(buffer, AL_FORMAT_STEREO16, data, size, ogg_info->rate);

	if((error=alGetError()) != AL_NO_ERROR) 
    	{
    		snprintf(str, 256, "stream_music %s: %s", my_tolower(reg_error_str), alGetString(error));
    		LogError(str);
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
	lock_sound_list();
	alSourceStopv(used_sources,sound_source);
	if((error=alGetError()) != AL_NO_ERROR) 
    	{
     		char	str[256];
    		snprintf(str, 256, "kill_local_sounds %s: %s", my_tolower(reg_error_str), alGetString(error));
    		LogError(str);
			have_sound=0;
			have_music=0;
    	}
	if(realloc_sources())
		LogError(snd_stop_fail);
	unlock_sound_list();
#ifndef	NO_MUSIC
	if(!have_music)return;
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
	int state;
	if(!have_music)return;
	music_on=1;
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
    		snprintf(str, 256, "%s: %s\n", snd_init_error, alGetString(error));
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

	alListenerfv(AL_POSITION,listenerPos);
	alListenerfv(AL_VELOCITY,listenerVel);
	alListenerfv(AL_ORIENTATION,listenerOri);
	alListenerf(AL_GAIN,1.0f);

	//poison data
	for(i=0;i<max_sources;i++)
		sound_source[i] = -1;
	for(i=0;i<max_buffers;i++)
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
    alDeleteBuffers(4, music_buffers);
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
    		snprintf(str, 256, "snd_realloc %s: %s", my_tolower(reg_error_str), alGetString(error));
    		LogError(str);
			have_sound=0;
			have_music=0;
    	}
	if(used_sources>=max_sources) 
    	{
    		log_to_console(c_red1,snd_sound_overflow);
    		LogError(snd_sound_overflow);
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
            strcpy(error_string, snd_media_read); break;
        case OV_ENOTVORBIS:
            strcpy(error_string, snd_media_notvorbis); break;
        case OV_EVERSION:
            strcpy(error_string, snd_media_ver_mismatch); break;
        case OV_EBADHEADER:
            strcpy(error_string, snd_media_invalid_header); break;
        case OV_EFAULT:
            strcpy(error_string, snd_media_internal_error); break;
        default:
            strcpy(error_string, snd_media_ogg_error);
    }
	LogError(error_string);
#endif	//NO_MUSIC
}

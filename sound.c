#include "global.h"
#include <math.h>

Mix_Music *current_music;

void play_music()
{
	//current_music=Mix_LoadMUS("./music/quiet_ruins.ogg");
	current_music=Mix_LoadMUS("./music/gp_v08.mid");
	Mix_PlayMusic(current_music, 2);
}

// make a channelDone function
void channelDone(int channel)
{
	int i;
    /* find the entry that use this channel, and then free the chunk, to set the channel
    as free */
	for(i=0;i<max_sound_objects;i++)
		{
			if(sound_list[i].channel==channel)
			if((sound_list[i].kill && sound_list[i].loops_number==-1) || !sound_list[i].loops_number)
				{
					sound_list[i].chunk=0;
					sound_list[i].channel=-1;
					return;
				}
			else
				{
					sound_list[i].is_playing=0;//this basically makes us recheck if it is in range again,
					sound_list[i].channel=-1;
					//after we move the camera
					return;
				}
		}

}

void stop_sound(int i)
{
	if(!have_sound)return;
	sound_list[i].kill=1;
	//Mix_FadeOutChannel(sound_list[i].channel,2000);
	Mix_HaltChannel(sound_list[i].channel);
}

//Tests to see if an obj_2d object is already loaded. If it is, return the handle.
//If not, load it, and return the handle
Mix_Chunk * load_chunk_cache(char * file_name)
{
	int i;
	int j;
	int file_name_lenght;
	Mix_Chunk * chunk;

	file_name_lenght=strlen(file_name);

	for(i=0;i<max_sound_cache;i++)
		{
			j=0;
			while(j<file_name_lenght)
				{
					if(sound_cache[i].file_name[j]!=file_name[j])break;
					j++;
				}
			if(file_name_lenght==j)//ok, Mix_Chunk already loaded
			return sound_cache[i].chunk;
		}
	//asc not found in the cache, so load it, and store it
	chunk=Mix_LoadWAV(file_name);

	//find a place to store it
	i=0;
	while(i<max_sound_cache)
		{
			if(!sound_cache[i].file_name[0])//we found a place to store it
				{
					my_strcp(sound_cache[i].file_name, file_name);
					sound_cache[i].chunk=chunk;
					return chunk;
				}
			i++;
		}

	return chunk;
}

int add_sound_object(char * file_name,int x, int y,int positional,int loops,int smooth_start)
{
	int i;
	Mix_Chunk * chunk;

	Uint16 distance;
	int tx,ty;

	tx=-cx*2;
	ty=-cy*2;

	if(!have_sound)return 0;

	distance=sqrt((tx-x)*(tx-x)+(ty-y)*(ty-y));

	for(i=0;i<max_sound_objects;i++)
		{
			if(!sound_list[i].chunk)break;
		}


	chunk=load_chunk_cache(file_name);
	if(!chunk)
		{
            char str[120];
            sprintf(str,"Error: Can't load sound: %s\n",file_name);
            log_error(str);
            return 0;
		}

	sound_list[i].chunk=chunk;
	sound_list[i].x_pos=x;
	sound_list[i].y_pos=y;
	sound_list[i].positional=positional;
	sound_list[i].loops_number=loops;
	sound_list[i].kill=0;

	if(!loops || distance<SOUND_RANGE || !positional)
		{
			if(!smooth_start)sound_list[i].channel=Mix_PlayChannel(-1,chunk,loops);
			else Mix_FadeInChannel(-1,chunk,loops,2000);
			sound_list[i].is_playing=1;
			//don't give a chance to the channel play if sound off
			if(!sound_on)Mix_Pause(sound_list[i].channel);

		}
	else
		{
			sound_list[i].is_playing=0;
			sound_list[i].channel=-1;
		}
	return i;

}

void add_sound_from_server(short number,short x, short y,short positional,short loops)
{
	char file_name[32];

	sprintf(file_name,"./sound/snd%i.wav",number);
	add_sound_object(file_name,x,y,positional,loops,0);
}

void update_positional_sounds()
{
	int i;

	if(!have_sound)return;
	debug_float=-1;

	for(i=0;i<max_sound_objects;i++)
	{
		if(sound_list[i].chunk)
		if(sound_list[i].positional)
		if(sound_list[i].is_playing)
			{
				int x,y;
				Uint16 distance;
				Uint8 dist;
				int angle;
				int tx,ty;


				x=sound_list[i].x_pos;
				y=sound_list[i].y_pos;
				tx=-cx*2;
				ty=-cy*2;

				distance=sqrt((tx-x)*(tx-x)+(ty-y)*(ty-y));
				distance*=6;
				debug_float=(atan2(tx-x,ty-y))/(3.1415926/180);
				if(debug_float<0)debug_float=360+debug_float;

				angle=(atan2(tx-x,ty-y))/(3.1415926/180);
				if(angle<0)angle+=360;
				angle=rz-angle;

				if(distance<=255)dist=distance;
				else dist=255;
				Mix_SetPosition(sound_list[i].channel, angle, dist);
			}

	}
}

void check_range_sounds()
{
	int i;

	if(!have_sound)return;

	for(i=0;i<max_sound_objects;i++)
	{
		if(sound_list[i].chunk)
		if(sound_list[i].positional)
		if(sound_list[i].loops_number==-1)
			{
				int x,y;
				Uint16 distance;
				int tx,ty;


				x=sound_list[i].x_pos;
				y=sound_list[i].y_pos;
				tx=-cx*2;
				ty=-cy*2;

				distance=sqrt((tx-x)*(tx-x)+(ty-y)*(ty-y));

				//test to see if out of range
				if(sound_list[i].is_playing && distance>SOUND_RANGE)
					{
						Mix_HaltChannel(sound_list[i].channel);
						//Mix_HaltChannel(-1);
						continue;
					}
				//test to see if we have an in range sound
				if(!sound_list[i].is_playing && distance<SOUND_RANGE)
					{
						sound_list[i].channel=Mix_PlayChannel(-1,sound_list[i].chunk,-1);

						//don't give a chance to the channel play if sound off
						if(!sound_on)Mix_Pause(sound_list[i].channel);

						if(sound_list[i].channel==-1)
							{
								log_to_console(c_red1,"Huston, we have a problem...");
	                		}

						sound_list[i].is_playing=1;
						continue;
					}

			}

	}
}

//kill all the sounds that loop infinitely
//usefull when we change maps, etc.
void kill_local_sounds()
{
	int i;

	if(!have_sound)return;

	for(i=0;i<max_sound_objects;i++)
	{
		if(sound_list[i].chunk)
		if(sound_list[i].loops_number==-1)
			{
				if(!sound_list[i].is_playing)
				sound_list[i].chunk=0;
				else
					{
						sound_list[i].kill=1;
						Mix_HaltChannel(sound_list[i].channel);
					}
			}
	}
}

void turn_sound_off()
{
	if(!have_sound)return;
	sound_on=0;
	Mix_Pause(-1);
}

void turn_sound_on()
{
	if(!have_sound)return;
	sound_on=1;
	Mix_Resume(-1);
}

void init_sound()
{
	int i;


	//clear all the channels
	for(i=0;i<max_sound_objects;i++)
		{
			sound_list[i].chunk=0;
			sound_list[i].channel=-1;
		}


	//init the sound
    if(SDL_InitSubSystem(SDL_INIT_AUDIO)<0)
        {
   		    char str[120];
    		sprintf(str, "Couldn't initialize the sound...: %s\n", SDL_GetError());
    		log_to_console(c_red1,str);
    		log_error(str);
        }
    else
    //if(Mix_OpenAudio(22050,MIX_DEFAULT_FORMAT,2,1024)<0)
    if(Mix_OpenAudio(22050,MIX_DEFAULT_FORMAT,2,2048)<0)
         {
   		    char str[120];
    		sprintf(str, "Hmm.. couldn't set the sampling rate or something...: %s\n", SDL_GetError());
    		log_to_console(c_red1,str);
			have_sound=0;
			have_music=0;
			return;
		 }
	else
     	{
			 have_sound=1;
			 have_music=1;
	 	}

	Mix_AllocateChannels(32);
	// set the callback for when a channel stops playing
	Mix_ChannelFinished(channelDone);

}





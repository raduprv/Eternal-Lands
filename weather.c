#include <stdlib.h>
#include "global.h"

int rain_control_counter=0;
int thunder_control_counter=0;
int num_rain_drops=MAX_RAIN_DROPS;
char rand_rain[8192];

void build_rain_table()
{
	int i;

	for(i=0;i<MAX_RAIN_DROPS;i++)
		{
			rain_drops[i].x=rand()%500;
			rain_drops[i].y=rand()%500;
			rain_drops[i].x2=rain_drops[i].x;
			rain_drops[i].y2=rain_drops[i].y+rain_drop_len;
		}
	// setup the motion table as well
	for(i=0;i<8192;i++)
		{
			rand_rain[i]=rand()&0xff;
		}

}

void update_rain()
{
	int i;
	int speed_var=rand()%8192;	// TODO: reduce the number of calls to this
	for(i=0;i<num_rain_drops;i++)
		{
			rain_drops[i].y-=RAIN_SPEED+(rand_rain[speed_var]&0x1);
			rain_drops[i].y2-=RAIN_SPEED+(rand_rain[speed_var]&0x1);
			speed_var++;
			speed_var&=(8192-1);	// limit the high end
			if(rain_drops[i].y<0)
			{
				rain_drops[i].x=rand()%500;
				rain_drops[i].y=500;
				rain_drops[i].x2=rain_drops[i].x;
				rain_drops[i].y2=500-rain_drop_len;
			}
		}
}

void render_rain()
{

   	glDisable(GL_DEPTH_TEST);
   	glDisable(GL_TEXTURE_2D);

   	glViewport(0, 0, window_width, window_height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//glOrtho(0.0, (GLdouble)500, (GLdouble)500, 0.0, -250.0, 250.0);
	glOrtho(500, (GLdouble)0, (GLdouble)0, 500, -250.0, 250.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();


	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2,GL_SHORT,0,rain_drops);
	glEnable(GL_BLEND);
  	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
  	glColor4f(0.1f,0.1f,0.1f,0.6f);
  	glDrawArrays(GL_LINES,0,num_rain_drops);
	glDisable(GL_BLEND);
	glDisableClientState(GL_VERTEX_ARRAY);


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}

void rain_control()
{
	if(rain_control_counter+1000<cur_time)
		{
			rain_control_counter=cur_time;
		}
	else return;

	if(seconds_till_rain_starts==-1 && seconds_till_rain_stops==-1)return;

	//prepare to stop the rain?
	if(seconds_till_rain_stops!=-1)
		{
			if(!seconds_till_rain_stops)
				{
					seconds_till_rain_stops=-1;
					rain_light_offset=0;
					return;
				}
			else
				seconds_till_rain_stops--;
			//stop the actual rain, before the light level is back to it's normal values
			if(seconds_till_rain_stops<30)
				{
					is_raining=0;
					if(rain_sound)
						{
							stop_sound(rain_sound);
							rain_sound=0;
						}
				}
			else
			if(!is_raining)
				{
					is_raining=1;
					rain_sound=add_sound_object("./sound/rain1.wav",0,0,0,-1,1);
				}


			//make it lighter each 3 seconds
			rain_light_offset=seconds_till_rain_stops/3;
			if(rain_light_offset<0)rain_light_offset=0;
		}

	//prepare to start the rain?
	if(seconds_till_rain_starts!=-1)
		{
			if(!seconds_till_rain_starts)
				{
					seconds_till_rain_starts=-1;
					if(!is_raining)
						{
							is_raining=1;
							rain_sound=add_sound_object("./sound/rain1.wav",0,0,0,-1,1);
							//find out how heavy the rainfall will be
							num_rain_drops=(MAX_RAIN_DROPS+rand()%MAX_RAIN_DROPS)/2;
						}
					rain_light_offset=30;
					return;
				}
			else
				seconds_till_rain_starts--;
			//make it darker each 3 seconds
			rain_light_offset=30-seconds_till_rain_starts/3;
			if(rain_light_offset<0)rain_light_offset=0;
		}
}


void thunder_control()
{
	int i;

	if(thunder_control_counter+100<cur_time)
		{
			thunder_control_counter=cur_time;
		}
	else return;

	for(i=0;i<MAX_THUNDERS;i++)
		{
			if(thunders[i].in_use)
				{
					if(thunders[i].time_since_started<5)thunders[i].light_offset=thunders[i].time_since_started;
					else
					if(thunders[i].time_since_started>5 && thunders[i].time_since_started<10)thunders[i].light_offset=5;
					else
					if(thunders[i].time_since_started>10 &&thunders[i].time_since_started<15)thunders[i].light_offset=15-thunders[i].time_since_started;
					//should we start the sound?
					if(thunders[i].seconds_till_sound!=-1 && thunders[i].seconds_till_sound>=0)
						{
							thunders[i].seconds_till_sound--;
							if(!thunders[i].seconds_till_sound)
								{
									if(thunders[i].thunder_type==0)add_sound_object("./sound/thunder1.wav",0,0,0,0,0);
									else
									if(thunders[i].thunder_type==1)add_sound_object("./sound/thunder2.wav",0,0,0,0,0);
									else
									if(thunders[i].thunder_type==2)add_sound_object("./sound/thunder3.wav",0,0,0,0,0);
									else
									if(thunders[i].thunder_type==3)add_sound_object("./sound/thunder4.wav",0,0,0,0,0);
									else
									if(thunders[i].thunder_type==4)add_sound_object("./sound/thunder5.wav",0,0,0,0,0);
									thunders[i].seconds_till_sound=-1;//we are done with this sound

								}
						}
					thunders[i].time_since_started++;
					//is this thunder expired?
					if(thunders[i].time_since_started>20 && thunders[i].seconds_till_sound==-1)
					thunders[i].in_use=0;
				}
		}

}

void add_thunder(int type,int sound_delay)
{
	int i;

	for(i=0;i<MAX_THUNDERS;i++)
		{
			if(!thunders[i].in_use)
				{
					thunders[i].in_use=1;
					thunders[i].light_offset=0;
					thunders[i].seconds_till_sound=sound_delay;
					thunders[i].thunder_type=type;
					thunders[i].time_since_started=0;
					thunders[i].x=rand()%500;
					thunders[i].y=rand()%500;
					thunders[i].rot=rand()%360;
					return;
				}
		}

}

void get_weather_light_level()
{
	int i;

	thunder_light_offset=0;

	for(i=0;i<MAX_THUNDERS;i++)
		{
			if(thunders[i].in_use)
			thunder_light_offset+=thunders[i].light_offset;
		}

	//thunders light is positive, while rain light is negative
	weather_light_offset=rain_light_offset;
}

void clear_thunders()
{
	int i;

	for(i=0;i<MAX_THUNDERS;i++)
		{
			thunders[i].in_use=0;
		}
}

/*
void render_ligtning()
{
	int i;
	float u_start,v_start;
	int frame;

   		glDisable(GL_DEPTH_TEST);

    	glViewport(0, 0, window_width, window_height);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(500, (GLdouble)0, (GLdouble)0, 500, -250.0, 250.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

	glEnable(GL_BLEND);
  	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
    glAlphaFunc(GL_GREATER,0.18f);
	if(last_texture!=texture_cache[lightning_text].texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_cache[lightning_text].texture_id);
			last_texture=texture_cache[lightning_text].texture_id;
		}

	for(i=0;i<MAX_THUNDERS;i++)
		{
			if(thunders[i].in_use && thunders[i].time_since_started<16)
				{
					frame=thunders[i].time_since_started/2;
					u_start=1.0f/8*frame;
					v_start=1.0f;
					glLoadIdentity();
					glTranslatef(thunders[i].x,thunders[i].y, 0);
					glRotatef(thunders[i].rot, 0.0f, 0.0f, 1.0f);
					glBegin(GL_QUADS);


					glTexCoord2f(u_start,0);
					glVertex3i(-15,-250,0);

					glTexCoord2f(u_start,v_start);
					glVertex3i(-15,250,0);

					glTexCoord2f(u_start+0.25f/2,v_start);
					glVertex3i(15,250,0);

					glTexCoord2f(u_start+0.25f/2,0);
					glVertex3i(15,-250,0);
					glEnd();

				}
		}
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);



		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		glEnable(GL_DEPTH_TEST);
}
*/



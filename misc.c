#include <stdlib.h>
#include <math.h>
#include "global.h"

Uint8 last_pixel_color[4];

void reset_under_the_mouse()
	{
		if(!read_mouse_now)return;
		last_pixel_color[0]=0;
		last_pixel_color[1]=0;
		last_pixel_color[2]=0;
		object_under_mouse=-1;
		thing_under_the_mouse=UNDER_MOUSE_NOTHING;
	}

int anything_under_the_mouse(int object_id, int object_type)
{
	char pixels[16]={0};

	if(!read_mouse_now)return 0;
	glReadBuffer(GL_BACK);
	glReadPixels(mouse_x, window_height-mouse_y, 1, 1, GL_RGB, GL_BYTE, &pixels);
	if(pixels[0]!=last_pixel_color[0] || pixels[1]!=last_pixel_color[1] || pixels[2]!=last_pixel_color[2])
		{
			last_pixel_color[0]=pixels[0];
			last_pixel_color[1]=pixels[1];
			last_pixel_color[2]=pixels[2];

			if(object_type==UNDER_MOUSE_NO_CHANGE)return 0;

			if(object_type==UNDER_MOUSE_PLAYER || object_type==UNDER_MOUSE_NPC || object_type==UNDER_MOUSE_ANIMAL)
				{
					actor_under_mouse=actors_list[object_id];
					object_id=actors_list[object_id]->actor_id;
				} else {
					actor_under_mouse=NULL;
				}
			object_under_mouse=object_id;

			thing_under_the_mouse=object_type;
			return 1;//there is something
		}
	return 0;//no collision, sorry

}

static GLfloat  model[16];
static GLfloat  proj[16];
static GLint    viewport[4];

void save_scene_matrix()
{
	glGetFloatv(GL_MODELVIEW_MATRIX, model);
	glGetFloatv(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, viewport);
	viewport[2]/=2;
	viewport[3]/=2;
}

static void
project_ortho(GLfloat ox, GLfloat oy, GLfloat oz, GLfloat * wx, GLfloat * wy)
{
	GLfloat tmp[3];

	// apply modelview matrix
	tmp[0] = model[0*4+0] * ox + model[1*4+0] * oy + model[2*4+0] * oz + model[3*4+0];
	tmp[1] = model[0*4+1] * ox + model[1*4+1] * oy + model[2*4+1] * oz + model[3*4+1];
	tmp[2] = model[0*4+2] * ox + model[1*4+2] * oy + model[2*4+2] * oz + model[3*4+2];
	
	// apply projection matrix
	ox = proj[0*4+0] * tmp[0] + proj[3*4+0];
	oy = proj[1*4+1] * tmp[1] + proj[3*4+1];
	oz = proj[2*4+2] * tmp[2] + proj[3*4+2];
	
	// viewport
	*wx = viewport[0] + (1 + ox) * viewport[2];
	*wy = viewport[1] + (1 + oy) * viewport[3];
}

int mouse_in_sphere(float x, float y, float z, float radius)
{
	GLfloat  winx, winy;
	int m_y = window_height - mouse_y;
	
	project_ortho(x, y, z, &winx, &winy);
	
	radius = proj[0*4+0]*radius+proj[3*4+0];
	radius = (1.0+radius)*viewport[2];
	
	return (mouse_x >= winx - radius  &&  mouse_x <= winx + radius  &&
			m_y     >= winy - radius  &&  m_y     <= winy + radius);
}

void find_last_url(char * source_string, int len)
{
	Uint8 cur_char;
	int i,j,url_start;
	int last_url_start=0;
	int final_url_start=0;
	int final_url_2=0;
	int www=1;

	while(1)
		{
			url_start=get_string_occurance("www.",source_string+final_url_start+1,len-last_url_start,1);
			if(url_start==-1)break;
			if(final_url_start<url_start+last_url_start)final_url_start=url_start+last_url_start;
			last_url_start+=url_start;
		}

	last_url_start=0;
	while(1)
		{
			url_start=get_string_occurance("http://",source_string+final_url_2+1,len-last_url_start,1);
			if(url_start==-1)break;
			if(final_url_2<url_start+last_url_start)final_url_2=url_start+last_url_start;
			last_url_start+=url_start;
		}


	if(!final_url_start && !final_url_2)return;//no URL found

	//see if the last url was the www or http:// one
	if(final_url_2>final_url_start)
		{
			final_url_start=final_url_2;
			www=0;//means the URL starts with http
		}


	//ok, we have an URL, now get it
	j=0;
	if(www)
		{
			my_strcp(current_url,"http://");
			j=7;
		}

	for(i=final_url_start;i<len;i++)
		{
			if(j>158)return;//URL too long, perhaps an exploit attempt
			cur_char=source_string[i];
			if(!cur_char || cur_char==' ' || cur_char==0x0a || cur_char=='<'
			|| cur_char=='>' || cur_char=='|' || cur_char=='"' || cur_char==']')break;
			current_url[j]=cur_char;
			j++;
		}
	current_url[j]=0;
	have_url=1;


}


int go_to_url(void *dummy)
{

	char browser_command[400];

	if(!have_url)return 0;

	my_strcp(browser_command,broswer_name);
	my_strcat(browser_command," \"");
	my_strcat(browser_command,current_url);
	my_strcat(browser_command,"\"");

	system(browser_command);

	return 0;

}


#include "global.h"

void draw_string(int x, int y,unsigned char * our_string,int max_lines)
{
	float u_start,u_end,v_start,v_end;
	int col,row;
	int displayed_font_x_size=11;
	int displayed_font_y_size=18;

	int font_x_size=18;
	int font_y_size=21;

	unsigned char cur_char;
	int i;
	int cur_x,cur_y;
	int current_lines=0;

   	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
    glAlphaFunc(GL_GREATER,0.1f);

	if(last_texture!=texture_cache[font_text].texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_cache[font_text].texture_id);
			last_texture=texture_cache[font_text].texture_id;
		}

	i=0;
	cur_x=x;
	cur_y=y;
	glBegin(GL_QUADS);
	while(1)
		{
			cur_char=our_string[i];
			if(!cur_char)break;
			if(cur_char=='\n')
				{
					cur_y+=displayed_font_y_size;
					cur_x=x;
					i++;
					current_lines++;
					if(current_lines>=max_lines)break;
					continue;
				}
			if(cur_char>=127)
				{
					float r,g,b;
					//must be a color
					cur_char-=127;
					r=colors_list[cur_char].r1;
					g=colors_list[cur_char].g1;
					b=colors_list[cur_char].b1;
					glColor3f(r,g,b);
					i++;
					continue;
				}
			//first, see where that char is, in the font.bmp
			cur_char-=32;
			col=cur_char/14;
			row=cur_char%14;

			//now get the texture coordinates
			u_start=(float)(row*font_x_size)/256.0f;
			u_end=(float)(row*font_x_size+font_x_size-7)/256.0f;
			v_start=(float)1.0f-(1+col*font_y_size)/256.0f;
			v_end=(float)1.0f-(col*font_y_size+font_y_size-2)/256.0f;

			glTexCoord2f(u_start,v_start);
			glVertex3i(cur_x,cur_y,0);

			glTexCoord2f(u_start,v_end);
			glVertex3i(cur_x,cur_y+displayed_font_y_size,0);

			glTexCoord2f(u_end,v_end);
			glVertex3i(cur_x+displayed_font_x_size,cur_y+displayed_font_y_size,0);

			glTexCoord2f(u_end,v_start);
			glVertex3i(cur_x+displayed_font_x_size,cur_y,0);

			cur_x+=displayed_font_x_size;


			i++;
		}


    glEnd();
	glDisable(GL_ALPHA_TEST);
}
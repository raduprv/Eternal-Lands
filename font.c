#include "global.h"

// converts a character into which entry in font.bmp to use, negative on error or no output
int find_font_char(unsigned char cur_char)
{
	if(cur_char<FONT_START_CHAR)	//null or invalid character
		{
			return -1;
		}
	else if(cur_char>=127)
		{
			if(cur_char<=127+c_grey4)
				{
					float r,g,b;
					//must be a color
					cur_char-=127;
					r=(float)colors_list[cur_char].r1/255.0f;
					g=(float)colors_list[cur_char].g1/255.0f;
					b=(float)colors_list[cur_char].b1/255.0f;
					glColor3f(r,g,b);
					return(-1);	// nothing to do
			 	}
			else if(cur_char>127+c_grey4)
				{
					if(cur_char==252)
						cur_char=UUML;
					else if(cur_char==233)
						cur_char=EACUTE;
					else if(cur_char==226)
						cur_char=ACIRC;
					else if(cur_char==224)
						cur_char=AGRAVE;
					else if(cur_char==231)
						cur_char=CCEDIL;
					else if(cur_char==234)
						cur_char=ECIRC;
					else if(cur_char==235)
						cur_char=EUML;
					else if(cur_char==232)
						cur_char=EGRAVE;
					else if(cur_char==239)
						cur_char=IUML;
					else if(cur_char==244)
						cur_char=OCIRC;
					else if(cur_char==251)
						cur_char=UGRAVE;
					else if(cur_char==230)
						cur_char=aELIG;
					else if(cur_char==248)
						cur_char=oSLASH;
					else if(cur_char==229)
						cur_char=aRING;
					else if(cur_char==198)
						cur_char=AELIG;
					else if(cur_char==216)
						cur_char=OSLASH;
					else if(cur_char==197)
						cur_char=ARING;
					else if(cur_char==228)
						cur_char=aUMLAUT;
					else if(cur_char==246)
						cur_char=oUMLAUT;
					else if(cur_char==252)
						cur_char=uUMLAUT;
					else if(cur_char==196)
						cur_char=AUMLAUT;
					else if(cur_char==214)
						cur_char=OUMLAUT;
					else if(cur_char==220)
						cur_char=UUMLAUT;
					else if(cur_char==223)
						cur_char=DOUBLES;
					else
						{
							return -1;	//ignore it
						}
					if(cur_char>=UUML && cur_char<=ARING)
						{
							cur_char-=(UUML-127);
						}
				}
		}

	// finally, adjust for the missing lead characters
	cur_char-=FONT_START_CHAR;

	return((int)cur_char);
}

// returns how far to move for the next char, or negative on error
int	draw_char_scaled(unsigned char cur_char, int cur_x, int cur_y, int displayed_font_x_size, int displayed_font_y_size)
{
	float u_start,u_end,v_start,v_end;
	int chr,col,row;
	int displayed_font_x_width;
	int font_x_size=FONT_X_SPACING;
	int font_y_size=FONT_Y_SPACING;

	chr= find_font_char(cur_char);
	if(chr < 0)	// watch for illegal/non-display characters
		{
			return 0;
		}

	// first, see where that char is, in the font.bmp
	col=chr/FONT_CHARS_PER_LINE;
	row=chr%FONT_CHARS_PER_LINE;

	// TODO:get the font width for this character
	displayed_font_x_width=displayed_font_x_size;

	//now get the texture coordinates
	u_start=(float)(row*font_x_size)/256.0f;
	u_end=(float)(row*font_x_size+font_x_size-7)/256.0f;
	v_start=(float)1.0f-(1+col*font_y_size)/256.0f;
	v_end=(float)1.0f-(col*font_y_size+font_y_size-2)/256.0f;

	// and place the text from the graphics on the map
	glTexCoord2f(u_start,v_start);
	glVertex3i(cur_x,cur_y,0);

	glTexCoord2f(u_start,v_end);
	glVertex3i(cur_x,cur_y+displayed_font_y_size,0);

	glTexCoord2f(u_end,v_end);
	glVertex3i(cur_x+displayed_font_x_width,cur_y+displayed_font_y_size,0);

	glTexCoord2f(u_end,v_start);
	glVertex3i(cur_x+displayed_font_x_width,cur_y,0);

	return(displayed_font_x_width);	// return how far to move for the next character
}

void draw_string(int x, int y,unsigned char * our_string,int max_lines)
{
	//float u_start,u_end,v_start,v_end;
	//int col,row;
	int displayed_font_x_size=11;
	int displayed_font_y_size=18;

	//int font_x_size=FONT_X_SPACING;
	//int font_y_size=FONT_Y_SPACING;

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
			// watch for special characters
			if(!cur_char)	// end of line
				{
					break;
				}
			else if (cur_char == '\n')	// newline
				{
					cur_y+=displayed_font_y_size;
					cur_x=x;
					i++;
					current_lines++;
					if(current_lines>=max_lines)break;
					continue;
				}

			/*
			if(cur_char>127+c_grey4)
				{
					if(cur_char==252)
						cur_char=UUML;
					else if(cur_char==233)
						cur_char=EACUTE;
					else if(cur_char==226)
						cur_char=ACIRC;
					else if(cur_char==224)
						cur_char=AGRAVE;
					else if(cur_char==231)
						cur_char=CCEDIL;
					else if(cur_char==234)
						cur_char=ECIRC;
					else if(cur_char==235)
						cur_char=EUML;
					else if(cur_char==232)
						cur_char=EGRAVE;
					else if(cur_char==239)
						cur_char=IUML;
					else if(cur_char==244)
						cur_char=OCIRC;
					else if(cur_char==251)
						cur_char=UGRAVE;
					else if(cur_char==230)
						cur_char=aELIG;
					else if(cur_char==248)
						cur_char=oSLASH;
					else if(cur_char==229)
						cur_char=aRING;
					else if(cur_char==198)
						cur_char=AELIG;
					else if(cur_char==216)
						cur_char=OSLASH;
					else if(cur_char==197)
						cur_char=ARING;
					else if(cur_char==228)
						cur_char=aUMLAUT;
					else if(cur_char==246)
						cur_char=oUMLAUT;
					else if(cur_char==252)
						cur_char=uUMLAUT;
					else if(cur_char==196)
						cur_char=AUMLAUT;
					else if(cur_char==214)
						cur_char=OUMLAUT;
					else if(cur_char==220)
						cur_char=UUMLAUT;
					else if(cur_char==223)
						cur_char=DOUBLES;
					else
						{
							i++;
							continue;
						}
				}

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
			else if(cur_char>=127 && cur_char<=127+c_grey4)
				{
					float r,g,b;
					//must be a color
					cur_char-=127;
					r=(float)colors_list[cur_char].r1/255.0f;
					g=(float)colors_list[cur_char].g1/255.0f;
					b=(float)colors_list[cur_char].b1/255.0f;
					glColor3f(r,g,b);
					i++;
					continue;
				}
			else if(cur_char>=UUML && cur_char<=ARING)
				{
					cur_char-=(UUML-127);
				}
			else if(cur_char!='\n' && cur_char<FONT_START_CHAR)//invalid character
				{
					i++;
					continue;
				}
			//first, see where that char is, in the font.bmp
			cur_char-=FONT_START_CHAR;
			col=cur_char/FONT_CHARS_PER_LINE;
			row=cur_char%FONT_CHARS_PER_LINE;

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
			*/

			cur_x+=draw_char_scaled(cur_char, cur_x, cur_y, displayed_font_x_size, displayed_font_y_size);

			i++;
		}


    glEnd();
	glDisable(GL_ALPHA_TEST);

}

void draw_string_small(int x, int y,unsigned char * our_string,int max_lines)
{
	//float u_start,u_end,v_start,v_end;
	//int col,row;
	int displayed_font_x_size=8;
	int displayed_font_y_size=15;

	//int font_x_size=FONT_X_SPACING;
	//int font_y_size=FONT_Y_SPACING;

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
			if(!cur_char)
				{
					break;
				}
			else if(cur_char=='\n')
				{
					cur_y+=displayed_font_y_size;
					cur_x=x;
					i++;
					current_lines++;
					if(current_lines>=max_lines)break;
					continue;
				}
			/*
			else if(cur_char>=127)
				{
					float r,g,b;
					//must be a color
					cur_char-=127;
					r=(float)colors_list[cur_char].r1/255.0f;
					g=(float)colors_list[cur_char].g1/255.0f;
					b=(float)colors_list[cur_char].b1/255.0f;
					glColor3f(r,g,b);
					i++;
					continue;
				}
			else if(cur_char!='\n' && cur_char<FONT_START_CHAR)//invalid character
				{
					i++;
					continue;
				}
			//first, see where that char is, in the font.bmp
			cur_char-=FONT_START_CHAR;
			col=cur_char/FONT_CHARS_PER_LINE;
			row=cur_char%FONT_CHARS_PER_LINE;

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
			*/

			cur_x+=draw_char_scaled(cur_char, cur_x, cur_y, displayed_font_x_size, displayed_font_y_size);

			i++;
		}


    glEnd();
	glDisable(GL_ALPHA_TEST);
}

void draw_ingame_string(float x, float y,unsigned char * our_string,
						int max_lines,int big)
{
	float u_start,u_end,v_start,v_end;
	int col,row;
	float displayed_font_x_size;
	float displayed_font_y_size;

	int font_x_size=FONT_X_SPACING;
	int font_y_size=FONT_Y_SPACING;

	unsigned char cur_char;
	int chr;
	int i;
	float cur_x,cur_y;
	int current_lines=0;

	if(big)
		{
			displayed_font_x_size=0.17*zoom_level/3.0;
			displayed_font_y_size=0.25*zoom_level/3.0;
		}
	else
		{
			displayed_font_x_size=SMALL_INGAME_FONT_X_LEN*zoom_level/3.0;
			displayed_font_y_size=SMALL_INGAME_FONT_Y_LEN*zoom_level/3.0;
		}


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
			if(!cur_char)
				{
					break;
				}
			else if(cur_char=='\n')
				{
					cur_y+=displayed_font_y_size;
					cur_x=x;
					i++;
					current_lines++;
					if(current_lines>=max_lines)break;
					continue;
				}
			/*
			if(cur_char>127+c_grey4)
				{
					if(cur_char==252)
						cur_char=UUML;
					else if(cur_char==233)
						cur_char=EACUTE;
					else if(cur_char==226)
						cur_char=ACIRC;
					else if(cur_char==224)
						cur_char=AGRAVE;
					else if(cur_char==231)
						cur_char=CCEDIL;
					else if(cur_char==234)
						cur_char=ECIRC;
					else if(cur_char==235)
						cur_char=EUML;
					else if(cur_char==232)
						cur_char=EGRAVE;
					else if(cur_char==239)
						cur_char=IUML;
					else if(cur_char==244)
						cur_char=OCIRC;
					else if(cur_char==251)
						cur_char=UGRAVE;
					else if(cur_char==230)
						cur_char=aELIG;
					else if(cur_char==248)
						cur_char=oSLASH;
					else if(cur_char==229)
						cur_char=aRING;
					else if(cur_char==198)
						cur_char=AELIG;
					else if(cur_char==216)
						cur_char=OSLASH;
					else if(cur_char==197)
						cur_char=ARING;
					else if(cur_char==228)
						cur_char=aUMLAUT;
					else if(cur_char==246)
						cur_char=oUMLAUT;
					else if(cur_char==252)
						cur_char=uUMLAUT;
					else if(cur_char==196)
						cur_char=AUMLAUT;
					else if(cur_char==214)
						cur_char=OUMLAUT;
					else if(cur_char==220)
						cur_char=UUMLAUT;
					else if(cur_char==223)
						cur_char=DOUBLES;
					else
						{
							i++;
							continue;
						}
				}

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
			else if(cur_char>=127 && cur_char<=127+c_grey4)
				{
					float r,g,b;
					//must be a color
					cur_char-=127;
					r=(float)colors_list[cur_char].r1/255.0f;
					g=(float)colors_list[cur_char].g1/255.0f;
					b=(float)colors_list[cur_char].b1/255.0f;
					glColor3f(r,g,b);
					i++;
					continue;
				}
			else if(cur_char>=UUML && cur_char<=ARING)
				{
					cur_char-=(UUML-127);
				}
			else if(cur_char!='\n' && cur_char<FONT_START_CHAR)//invalid character
				{
					i++;
					continue;
				}
			//first, see where that char is, in the font.bmp
			cur_char-=FONT_START_CHAR;
			*/
			chr=find_font_char(cur_char);
			if(chr >= 0)
				{
					col=chr/FONT_CHARS_PER_LINE;
					row=chr%FONT_CHARS_PER_LINE;

					//now get the texture coordinates
					u_start=(float)(row*font_x_size)/256.0f;
					u_end=(float)(row*font_x_size+font_x_size-7)/256.0f;
					v_start=(float)1.0f-(1+col*font_y_size)/256.0f;
					v_end=(float)1.0f-(col*font_y_size+font_y_size-2)/256.0f;


					/*glTexCoord2f(u_start,v_start);
					glVertex3f(cur_x,cur_y,1+displayed_font_y_size);
		
					glTexCoord2f(u_start,v_end);
					glVertex3f(cur_x,cur_y,1);

					glTexCoord2f(u_end,v_end);
					glVertex3f(cur_x+displayed_font_x_size,cur_y,1);

					glTexCoord2f(u_end,v_start);
					glVertex3f(cur_x+displayed_font_x_size,cur_y,1+displayed_font_y_size);
					*/
					glTexCoord2f(u_start,v_start);
					glVertex3f(cur_x,0,cur_y+displayed_font_y_size);
		
					glTexCoord2f(u_start,v_end);
					glVertex3f(cur_x,0,cur_y);

					glTexCoord2f(u_end,v_end);
					glVertex3f(cur_x+displayed_font_x_size,0,cur_y);

					glTexCoord2f(u_end,v_start);
					glVertex3f(cur_x+displayed_font_x_size,0,cur_y+displayed_font_y_size);
					

					cur_x+=displayed_font_x_size;
				}

			i++;
		}


    glEnd();
	glDisable(GL_ALPHA_TEST);
}




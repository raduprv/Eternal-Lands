#include <stdlib.h>
#include <string.h>
#include "font.h"
#include "../errors.h"
#include "../client_serv.h"
#include "../colors.h"
#include "../textures.h"

static int font_text = 0;

int	cur_font_num=0;
int	max_fonts=0;
font_info	*fonts[10];
int	chat_font=0;
int	name_font=0;


// converts a character into which entry in font.bmp to use, negative on error or no output
int find_font_char(unsigned char cur_char)
{
	if(cur_char >= 127 && cur_char<=127+c_grey4)
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
	return(get_font_char(cur_char));
}

int get_font_char(unsigned char cur_char)
{
	if(cur_char<FONT_START_CHAR)	//null or invalid character
		{
			return -1;
		}
	else if(cur_char>=127)
		{
			if(cur_char<=127+c_grey4)
				{
					//color, won't show
					return -1;
				}
			/*
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
				*/
		}

	// finally, adjust for the missing lead characters
	cur_char-=FONT_START_CHAR;

	return((int)cur_char);
}

// returns how far to move for the next char, or negative on error
int	draw_char_scaled(unsigned char cur_char, int cur_x, int cur_y, float displayed_font_x_size, float displayed_font_y_size)
{
	float u_start,u_end,v_start,v_end;
	int chr,col,row;
	int displayed_font_x_width;
	int font_x_size=FONT_X_SPACING;
	int font_y_size=FONT_Y_SPACING;
	int	font_bit_width, ignored_bits;

	chr= find_font_char(cur_char);
	if(chr < 0)	// watch for illegal/non-display characters
		{
			return 0;
		}

	// first, see where that char is, in the font.bmp
	col=chr/FONT_CHARS_PER_LINE;
	row=chr%FONT_CHARS_PER_LINE;

	//displayed_font_x_width=(int)displayed_font_x_size;
	font_bit_width=get_font_width(chr);
	displayed_font_x_width=(int)(0.5f+((float)font_bit_width)*displayed_font_x_size/12.0);
	ignored_bits=(12-font_bit_width)/2;	// how many bits on each side of the char are ignored?

	//now get the texture coordinates
	u_start=(float)(row*font_x_size+ignored_bits)/256.0f;
	u_end=(float)(row*font_x_size+font_x_size-7-ignored_bits)/256.0f;
#ifdef	NEW_TEXTURES
	v_start = (1+col*font_y_size)/256.0f;
	v_end = (col*font_y_size+font_y_size-1)/256.0f;
#else	/* NEW_TEXTURES */
	v_start=(float)1.0f-(1+col*font_y_size)/256.0f;
	v_end=(float)1.0f-(col*font_y_size+font_y_size-1)/256.0f;
#endif	/* NEW_TEXTURES */
	//v_end=(float)1.0f-(col*font_y_size+font_y_size-2)/256.0f;

	// and place the text from the graphics on the map
	glTexCoord2f(u_start,v_start);
	glVertex3i(cur_x,cur_y,0);

	glTexCoord2f(u_start,v_end);
	glVertex3i(cur_x,cur_y+(displayed_font_y_size+1),0);

	glTexCoord2f(u_end,v_end);
	glVertex3i(cur_x+displayed_font_x_width,cur_y+(displayed_font_y_size+1),0);

	glTexCoord2f(u_end,v_start);
	glVertex3i(cur_x+displayed_font_x_width,cur_y,0);

	return(displayed_font_x_width);	// return how far to move for the next character
}

void draw_string(int x, int y,const unsigned char * our_string,int max_lines)
{
	draw_string_zoomed(x, y,our_string,max_lines, 1.0f);
}

void draw_string_zoomed(int x, int y,const unsigned char * our_string,int max_lines, float text_zoom)
{
	float displayed_font_x_size=11.0*text_zoom;
	float displayed_font_y_size=18.0*text_zoom;

	unsigned char cur_char;
	int i;
	int cur_x,cur_y;
	int current_lines=0;

   	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
    glAlphaFunc(GL_GREATER,0.1f);
#ifdef	NEW_TEXTURES
	bind_texture(font_text);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(font_text);
#endif	/* NEW_TEXTURES */

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

			cur_x+=draw_char_scaled(cur_char, cur_x, cur_y, displayed_font_x_size, displayed_font_y_size);

			i++;
		}


    glEnd();
	glDisable(GL_ALPHA_TEST);

}

void draw_string_small(int x, int y,const unsigned char * our_string,int max_lines)
{
	int displayed_font_x_size=8;
	int displayed_font_y_size=15;

	unsigned char cur_char;
	int i;
	int cur_x,cur_y;
	int current_lines=0;

   	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
    glAlphaFunc(GL_GREATER,0.1f);
#ifdef	NEW_TEXTURES
	bind_texture(font_text);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(font_text);
#endif	/* NEW_TEXTURES */

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

			cur_x+=draw_char_scaled(cur_char, cur_x, cur_y, displayed_font_x_size, displayed_font_y_size);

			i++;
		}


    glEnd();
	glDisable(GL_ALPHA_TEST);
}

#ifdef	ELC
void draw_ingame_string(float x, float y,const unsigned char * our_string,
						int max_lines,int big)
{
	float u_start,u_end,v_start,v_end;
	int col,row;
	float displayed_font_x_size;
	float displayed_font_y_size;

	float displayed_font_x_width;
	int font_x_size=FONT_X_SPACING;
	int font_y_size=FONT_Y_SPACING;
	int	font_bit_width, ignored_bits;

	unsigned char cur_char;
	int chr;
	int i;
	float cur_x,cur_y;
	int current_lines=0;

	if(big)
		{
			displayed_font_x_size=0.17*zoom_level*name_zoom/3.0;
			displayed_font_y_size=0.25*zoom_level*name_zoom/3.0;
		}
	else
		{
			displayed_font_x_size=SMALL_INGAME_FONT_X_LEN*zoom_level*name_zoom/3.0;
			displayed_font_y_size=SMALL_INGAME_FONT_Y_LEN*zoom_level*name_zoom/3.0;
		}


   	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
    glAlphaFunc(GL_GREATER,0.1f);
#ifdef	NEW_TEXTURES
	bind_texture(font_text);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(font_text);
#endif	/* NEW_TEXTURES */

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
			else if(cur_char >127 && cur_char<=127+c_grey4)
				{
					glEnd();	//Ooops - NV bug fix!!
				}
			chr=find_font_char(cur_char);
			if(chr >= 0)
				{
					col=chr/FONT_CHARS_PER_LINE;
					row=chr%FONT_CHARS_PER_LINE;


					font_bit_width=get_font_width(chr);
					displayed_font_x_width=((float)font_bit_width)*displayed_font_x_size/12.0;
					ignored_bits=(12-font_bit_width)/2;	// how many bits on each side of the char are ignored?
					if(ignored_bits < 0)ignored_bits=0;

					//now get the texture coordinates
					u_start=(float)(row*font_x_size+ignored_bits)/256.0f;
					u_end=(float)(row*font_x_size+font_x_size-7-ignored_bits)/256.0f;
#ifdef	NEW_TEXTURES
					v_start = (float)(1+col*font_y_size)/256.0f;
					v_end = (float)(col*font_y_size+font_y_size-1)/256.0f;
#else	/* NEW_TEXTURES */
					v_start=(float)1.0f-(1+col*font_y_size)/256.0f;
					v_end=(float)1.0f-(col*font_y_size+font_y_size-1)/256.0f;
#endif	/* NEW_TEXTURES */
					//v_end=(float)1.0f-(col*font_y_size+font_y_size-2)/256.0f;

					glTexCoord2f(u_start,v_start);
					glVertex3f(cur_x,0,cur_y+displayed_font_y_size);
		
					glTexCoord2f(u_start,v_end);
					glVertex3f(cur_x,0,cur_y);

					glTexCoord2f(u_end,v_end);
					glVertex3f(cur_x+displayed_font_x_width,0,cur_y);

					glTexCoord2f(u_end,v_start);
					glVertex3f(cur_x+displayed_font_x_width,0,cur_y+displayed_font_y_size);
					

					//cur_x+=displayed_font_x_size;
					cur_x+=displayed_font_x_width;
				}
			else if(cur_char >127 && cur_char<=127+c_grey4)
				{
					glBegin(GL_QUADS);	//Ooops - NV bug fix!!
				}

			i++;
		}

    glEnd();
	glDisable(GL_ALPHA_TEST);
}
#endif	//ELC



// font handling
int get_font_width(int cur_char)
{
	if (cur_char < 0)	return 0;
	return (fonts[cur_font_num]->widths[cur_char] + fonts[cur_font_num]->spacing);
}

int get_char_width(unsigned char cur_char)
{
	return get_font_width(get_font_char(cur_char));
}


int get_string_width(const unsigned char *str)
{
	return get_nstring_width(str, strlen((const char*) str));
}


int get_nstring_width(const unsigned char *str, int len)
{
	int	i, wdt=0;
	for(i=0; i<len; i++) wdt+=get_char_width(str[i]);
	return wdt;
}


int init_fonts()
{
	int	i;
	max_fonts=0;
	for(i=0; i<10; i++)	fonts[i]=NULL;
	load_font(0, "./textures/font.bmp");
	load_font(1, "./textures/fontv.bmp");
	//set the default font
	cur_font_num=0;
	font_text=fonts[0]->texture_id;

	return(0);
}

int load_font(int num, char *file)
{
	int	i;

	// error checking
	if(num < 0 || num >= 10)
		{
			return -1;
		}
	// allocate space if needed
	if(fonts[num] == NULL)
		{
			fonts[num]=(font_info *)calloc(1, sizeof(font_info));
			if(fonts[num] == NULL)
				{
					log_error(__FILE__, __LINE__, "Unable to load font");
					return -1;
				}
		}
	//watch the highest font
	if(num >= max_fonts)
		{
			max_fonts=num+1;
		}
	// set default font info
	strcpy(fonts[num]->name, "default");
	fonts[num]->spacing=0;
	for(i=0; i<9*FONT_CHARS_PER_LINE; i++) fonts[num]->widths[i]=12;
	// load texture
#ifdef	NEW_TEXTURES
	fonts[num]->texture_id = load_texture_cached(file, tt_font);
#else	/* NEW_TEXTURES */
	fonts[num]->texture_id=load_texture_cache(file, 0);
#endif	/* NEW_TEXTURES */
	// load font information
	// TODO: write this and remove the hack!
	if(num > 0){
		static int widths[]={
			4,2,7,11,8,12,12,2,7,7,9,10,3,8,
			2,10,10,10,8,8,10,7,9,9,9,9,3,3,
			10,10,10,9,12,12,9,10,10,9,9,10,9,8,
			7,11,8,11,10,11,9,11,11,9,10,9,12,12,
			12,12,10,6,10,6,10,12,3,11,9,9,9,9,
			8,9,9,4,6,10,4,11,9,10,9,9,8,8,
			8,9,10,12,10,10,9,8,2,8,10,8,12,12,
			12,12,12,12,12,12,12,12,12,12,12,12,12,12,
			12,12,12,12,12,12,12,12,12,12,12,12,12,12,
		};
		memcpy(fonts[num]->widths, widths, sizeof(widths));
		fonts[num]->spacing=4;
	}

	//and return
	return num;
}


int	set_font(int num)
{
	if(num >= 0 && num < max_fonts && fonts[num] && fonts[num]->texture_id >= 0)
		{
			cur_font_num=num;
			font_text=fonts[cur_font_num]->texture_id;
		}

	return cur_font_num;
}


void remove_font(int num)
{
	if(num < max_fonts && fonts[num])
		{
			free(fonts[num]);
			fonts[num]=NULL;
			if (num == max_fonts-1)
				{
					max_fonts--;
				}
		}
}

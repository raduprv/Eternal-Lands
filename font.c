#include "global.h"
#include <stdlib.h>
#include <string.h>

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *
 * void remove_font(int);
 */

/*!
 * \name constants for fonts
 */
/*! @{ */
#define FONT_START_CHAR	32	/*!< first character in font.bmp */
#define FONT_CHARS_PER_LINE	14	/*!< how manu chars per line in font.bmp? */
#define	FONT_X_SPACING	18	/*!< X spacing of font in font.bmp */
#define	FONT_Y_SPACING	21	/*!< Y spacing of font in font.bmp */
#define FONTS_ARRAY_SIZE	10
/*! @} */

/*!
 * font info structure
 */
typedef struct	{
	int	spacing;
	int texture_id; /*!< id of the texture used for the font */
	int	widths[10*FONT_CHARS_PER_LINE];
	char name[32]; /*!< name of the font */
} font_info;

int	cur_font_num=0;
int	max_fonts=0;
font_info	*fonts[FONTS_ARRAY_SIZE];
int	chat_font=0;
int	name_font=0;
int	book_font=0;

/* forward declarations added due to code cleanup */
int get_font_char(unsigned char cur_char);
int get_font_width(int cur_char);
int get_nstring_width(const unsigned char *str, int len);
int set_font_parameters (int num);
/* end of added forward declarations */

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
			else if(cur_char>127+c_grey4)
				{
					switch(cur_char) {
					case 193:
						cur_char=AACCENT;break;
					case 196:
						cur_char=AUMLAUT;break;
					case 197:
						cur_char=ARING;break;
					case 198:
						cur_char=AELIG;break;
					case 201:
						cur_char=EACCENT;break;
					case 205:
						cur_char=IACCENT;break;
					case 209:
						cur_char=ENYE;break;
					case 211:
						cur_char=OACCENT;break;
					case 214:
						cur_char=OUMLAUT;break;
					case 216:
						cur_char=OSLASH;break;
					case 218:
						cur_char=UACCENT;break;
					case 220:
						cur_char=UUMLAUT;break;
					case 223:
						cur_char=DOUBLES;break;
					case 224:
						cur_char=AGRAVE;break;
					case 225:
						cur_char=aACCENT;break;
					case 226:
						cur_char=ACIRC;break;
					case 228:
						cur_char=aUMLAUT;break;
					case 229:
						cur_char=aRING;break;
					case 230:
						cur_char=aELIG;break;
					case 231:
						cur_char=CCEDIL;break;
					case 232:
						cur_char=EGRAVE;break;
					case 233:
						cur_char=EACUTE;break;
					case 234:
						cur_char=ECIRC;break;
					case 235:
						cur_char=EUML;break;
					case 237:
						cur_char=iACCENT;break;
					case 239:
						cur_char=IUML;break;
					case 241:
						cur_char=EnyE;break;
					case 243:
						cur_char=oACCENT;break;
					case 244:
						cur_char=OCIRC;break;
					case 246:
						cur_char=oUMLAUT;break;
					case 248:
						cur_char=oSLASH;break;
					case 250:
						cur_char=uACCENT;break;
					case 251:
						cur_char=UGRAVE;break;
					case 252:
						cur_char=UUML;break;
					/* this case is redundant. which one is right?
					case 252:
						cur_char=uUMLAUT;break; */
					default:
						return -1;	//ignore it
					}
					if(cur_char>=UUML && cur_char<=UACCENT)
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
	v_start=(float)1.0f-(1+col*font_y_size)/256.0f;
	v_end=(float)1.0f-(col*font_y_size+font_y_size-1)/256.0f;
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

void draw_string(int x, int y, const unsigned char * our_string, int max_lines)
{
	draw_string_zoomed(x, y, our_string, max_lines, 1.0f);
}

void draw_string_zoomed(int x, int y, const unsigned char * our_string, int max_lines, float text_zoom)
{
	float displayed_font_x_size= 11.0*text_zoom;
	float displayed_font_y_size= 18.0*text_zoom;

	unsigned char cur_char;
	int i;
	int cur_x,cur_y;
	int current_lines= 0;

   	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER,0.1f);
	get_and_set_texture_id(font_text);

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
			else if (cur_char == '\n' || cur_char == '\r')	// newline
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

/*
// returns how far to move for the next char, or negative on error
int draw_font_char_scaled (int font, unsigned char ch, int x, int y, float zoom)
{
	float u_start, u_end, v_start, v_end;
	int chr, col, row;
	int displayed_font_width, displayed_font_height;
	int font_x_size = FONT_X_SPACING;
	int font_y_size = FONT_Y_SPACING;
	int font_bit_width, ignored_bits;

	set_font (font);
	chr = find_font_char (ch);
	if(chr < 0)
	{
		// watch for illegal/non-display characters
		return 0;
	}

	// first, see where that char is, in the font.bmp
	row = chr / FONT_CHARS_PER_LINE;
	col = chr % FONT_CHARS_PER_LINE;

	font_bit_width = get_font_width (chr);
	displayed_font_width = (int) (0.5f + zoom * font_bit_width);
	displayed_font_height = (int) (zoom * 18);
	
	ignored_bits = (12 - font_bit_width) / 2;	// how many bits on each side of the char are ignored?

	//now get the texture coordinates
	u_start = (float) (col * font_x_size + ignored_bits) / 256.0f;
	u_end   = (float) (col * font_x_size + font_x_size - 7 - ignored_bits) / 256.0f;
	v_start = (float) 1.0f - (1 + row * font_y_size) / 256.0f;
	v_end   = (float) 1.0f - (row * font_y_size + font_y_size - 1) / 256.0f;

	// and place the text from the graphics on the map
	glTexCoord2f (u_start, v_start);
	glVertex3i (x, y, 0);

	glTexCoord2f (u_start, v_end);
	glVertex3i (x, y + displayed_font_height + 1, 0);

	glTexCoord2f (u_end, v_end);
	glVertex3i (x + displayed_font_width, y + displayed_font_height + 1, 0);

	glTexCoord2f (u_end, v_start);
	glVertex3i (x + displayed_font_width, y, 0);

	return displayed_font_width;	// return how far to move for the next character
}
*/

void draw_string_clipped(int x, int y, const unsigned char * our_string, int width, int height)
{
	draw_string_zoomed_clipped (x, y, our_string, -1, width, height, 1.0f);
}

void draw_string_zoomed_clipped (int x, int y, const unsigned char* our_string, int cursor_pos, int width, int height, float text_zoom)
{
	float displayed_font_x_size = 11.0 * text_zoom;
	float displayed_font_y_size = 18.0 * text_zoom;

	unsigned char cur_char;
	int i;
	int cur_x, cur_y;
	int cursor_x = x-1, cursor_y = y-1;
		
	if (width < displayed_font_x_size || height < displayed_font_y_size)
		// no point in trying
		return;

   	glEnable (GL_ALPHA_TEST);	// enable alpha filtering, so we have some alpha key
	glAlphaFunc (GL_GREATER, 0.1f);
	get_and_set_texture_id (font_text);

	i = 0;
	cur_x = x;
	cur_y = y;
	glBegin (GL_QUADS);
	while (1)
	{
		if (i == cursor_pos)
		{
			cursor_x = cur_x;
			cursor_y = cur_y;
			if (cursor_x - x > width - displayed_font_x_size)
			{
				cursor_x = x;
				cursor_y = cur_y + displayed_font_y_size;
			}
				
		}

		cur_char = our_string[i];
		// watch for special characters
		if (!cur_char) 
		{
			// end of string
			break;
		}
		else if (cur_char == '\n' || cur_char == '\r')
		{
			// newline
			cur_y += displayed_font_y_size;
			if (cur_y - y > height - displayed_font_y_size) break;
			cur_x = x;
			i++;
			continue;
		}

//		cur_x += draw_font_char_scaled (0, cur_char, cur_x, cur_y, text_zoom);
		cur_x += draw_char_scaled (cur_char, cur_x, cur_y, displayed_font_x_size, displayed_font_y_size);
		
		i++;
		if (cur_x - x > width - displayed_font_x_size)
		{
			// ignore rest of this line
			while (our_string[i] != '\0' && our_string[i] != '\n' && our_string[i] != '\r') i++;
		}
	}

	if (cursor_x >= x && cursor_y >= y && cursor_y - y <= height - displayed_font_y_size)
	{
		draw_char_scaled ('_', cursor_x, cursor_y, displayed_font_x_size, displayed_font_y_size);
	}
	
	glEnd();
	glDisable(GL_ALPHA_TEST);
	
}

int reset_soft_breaks (char *str, float zoom, int width)
{
	//displayed_font_x_width=(int)displayed_font_x_size;
	int font_bit_width;
	float displayed_font_x_size = 11.0 * zoom;
	int ichar, iline, nlines;
	float line_width;

	// remove all old soft line breaks
	for (ichar = 0; str[ichar] != '\0'; ichar++)
		if (str[ichar] == '\r') str[ichar] = ' ';

	ichar = 0;
	nlines = 0;
	while (1)
	{
		// search the line until it's wider than the screen or
		// a line break is found
		line_width = 0.0;
		for (iline = ichar; str[iline] != '\0'; iline++)
		{
			if (str[iline] == '\n') break;
			//line_width += zoom * get_char_width (str[iline]);
			font_bit_width = get_font_width (str[iline]);
			line_width += (int) (0.5f + font_bit_width * displayed_font_x_size / 12.0f);
			if (line_width > width) break;
		}
		if (str[iline] == '\0') break;
		
		if (line_width > width)
		{
			// now search back again for the last space
			for ( ; iline > ichar; iline--)
			{
				if (str[iline] == ' ')
				{
					str[iline] = '\r';
					break;
				}
			}
		}
		
		nlines++;
		ichar = iline + 1;
	}
	
	return nlines;
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
	get_and_set_texture_id(font_text);

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
						int max_lines, float font_x_scale, float font_y_scale)
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

	/*
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
	*/
	displayed_font_x_size=font_x_scale*zoom_level*name_zoom/3.0;
	displayed_font_y_size=font_y_scale*zoom_level*name_zoom/3.0;

   	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
    glAlphaFunc(GL_GREATER,0.1f);
	get_and_set_texture_id(font_text);

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
					v_start=(float)1.0f-(1+col*font_y_size)/256.0f;
					v_end=(float)1.0f-(col*font_y_size+font_y_size-1)/256.0f;
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
	return get_nstring_width(str, strlen(str));
}


int get_nstring_width(const unsigned char *str, int len)
{
	int	i, wdt=0;
	for(i=0; i<len; i++) wdt+= get_char_width(str[i]);
	// adjust to ignore the final spacing
	wdt-= fonts[cur_font_num]->spacing;

	return wdt;
}


int init_fonts ()
{
	int	i;

	max_fonts = 0;
	for(i = 0; i < FONTS_ARRAY_SIZE; i++)
		fonts[i] = NULL;
	
	if (set_font_parameters (0) < 0) return 0;
	if (set_font_parameters (1) < 0) return 0;
	if (set_font_parameters (2) < 0) return 0;

	cur_font_num = 0;

	return 1;
}

int load_font_textures ()
{
	if (fonts[0] == NULL || fonts[1] == NULL || fonts[2] == NULL)
	{
		int i;
		for (i = 0; i < FONTS_ARRAY_SIZE; i++)
			if (fonts[i] != NULL) free (fonts[i]);
		if ( !init_fonts () ) return 0;
	}
	fonts[0]->texture_id = load_texture_cache ("./textures/font.bmp", 0);
	fonts[1]->texture_id = load_texture_cache ("./textures/fontv.bmp", 0);
	fonts[2]->texture_id = load_texture_cache ("./textures/font2.bmp", 0);

	//set the default font
	cur_font_num = 0;
	font_text = fonts[0]->texture_id;
	
	return 1;
}

int set_font_parameters (int num)
{
	int	i;

	// error checking
	if(num < 0 || num >= FONTS_ARRAY_SIZE)
		{
			return -1;
		}
	// allocate space if needed
	if(fonts[num] == NULL)
		{
			fonts[num]=(font_info *)calloc(1, sizeof(font_info));
			if(fonts[num] == NULL)
				{
					log_error(cant_load_font);
					return -1;
				}
		}
	//watch the highest font
	if(num >= max_fonts)
		{
			max_fonts=num+1;
		}
	// set default font info
	my_strcp (fonts[num]->name, "default");
	fonts[num]->spacing=0;

	// load font information
	// TODO: write this and remove the hack!
	if(num==0)for(i=0; i<10*FONT_CHARS_PER_LINE; i++) fonts[num]->widths[i]=12;
	if(num==1){
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
			12,12,12,12,12,12,12,12,12,12,12,12,12,12,
		};
		memcpy(fonts[num]->widths, widths, sizeof(widths));
		fonts[num]->spacing=4;
	}
	if(num==2){
		static int widths[]={
			 8,  8,  8, 10,  8, 10, 10,  8,  8,  8,  8, 10,  8,  8,
			 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
			10, 10, 10,  8, 12, 10, 10, 10, 10, 10, 10, 10, 10, 10,
			10, 10, 10, 10, 10, 10, 10, 10,  8, 10, 10, 10, 10, 10,
			10, 10, 10, 10, 10, 10, 10,  8,  8,  8,  8,  8,  8,  8,
			10,  8,  8,  8,  8,  8,  8, 10,  8,  8,  8,  8,  8,  8,
			 8,  8,  8, 10,  8,  8,  8, 10,  8, 10, 10,  8, 10,  8,
			 8,  8, 10, 10, 10,  8, 10, 10,  8,  8,  8, 12, 12, 12, 
			10, 10, 12, 10, 12, 12, 12,
		};
		memcpy(fonts[num]->widths, widths, sizeof(widths));
		fonts[num]->spacing=2;
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

/* currently UNUSED
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
*/

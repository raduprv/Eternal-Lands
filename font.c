#include <stdlib.h>
#include <string.h>
#include "global.h"

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void remove_font(int);
 */

#define FONT_START_CHAR	32
#define FONT_CHARS_PER_LINE	14
#define	FONT_X_SPACING	18
#define	FONT_Y_SPACING	21
#define FONTS_ARRAY_SIZE	10

typedef struct	{
	int	spacing;
	int texture_id;
	int	widths[10*FONT_CHARS_PER_LINE];
	char name[32];
} font_info;

int	cur_font_num=0;
int	max_fonts=0;
font_info	*fonts[FONTS_ARRAY_SIZE];
int	chat_font=0;
int	name_font=0;
int	book_font=0;

int get_font_char(unsigned char cur_char);
int get_font_width(int cur_char);
int get_nstring_width(const unsigned char *str, int len);
int set_font_parameters (int num);

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

void draw_messages (int x, int y, text_message *msgs, int msgs_size, Uint8 filter, int msg_start, int offset_start, int cursor, int width, int height, float text_zoom)
{
	float displayed_font_x_size = 11.0 * text_zoom;
	float displayed_font_y_size = 18.0 * text_zoom;

	unsigned char cur_char;
	int i;
	int imsg, ichar;
	int cur_x, cur_y;
	int cursor_x = x-1, cursor_y = y-1;
	unsigned char ch;

	ch = msgs[msg_start].data[offset_start];
	if (ch < 127 || ch > 127 + c_grey4)
	{
		// search backwards for the last color
		for (ichar = offset_start-1; ichar >= 0; ichar--)
		{
			ch = msgs[msg_start].data[ichar];
			if (ch >= 127 && ch <= 127 + c_grey4)
			{
				float r, g, b;
				ch -= 127;
				r = colors_list[ch].r1 / 255.0f;
				g = colors_list[ch].g1 / 255.0f;
				b = colors_list[ch].b1 / 255.0f;
				glColor3f (r, g, b);
				break;
			}
		}
	}
	
	imsg = msg_start;
	ichar = offset_start;
	if (msgs[imsg].data == NULL) return;

	if (width < displayed_font_x_size || height < displayed_font_y_size)
		// no point in trying
		return;

	if (filter != FILTER_ALL)
	{
		for (;;) {
			char skip = 0;
			int channel = msgs[imsg].chan_idx;
			if (channel != filter && channel != CHAT_MODPM) {
				switch (channel) {
					case CHAT_LOCAL:    skip = local_chat_separate;    break;
					case CHAT_PERSONAL: skip = personal_chat_separate; break;
					case CHAT_GM:       skip = guild_chat_separate;    break;
					case CHAT_SERVER:   skip = server_chat_separate;   break;
					case CHAT_MOD:      skip = mod_chat_separate;      break;
					case CHAT_MODPM:                                   break;
					default:            skip = 1;
				}
			}
			if (skip) {
				ichar = 0;
				if (++imsg >= msgs_size) imsg = 0;
				if (msgs[imsg].data == NULL || imsg == msg_start) break;
			} else {
				break;
			}
		}
		if (msgs[imsg].data == NULL) return;
	}

   	glEnable (GL_ALPHA_TEST);	// enable alpha filtering, so we have some alpha key
	glAlphaFunc (GL_GREATER, 0.1f);
	get_and_set_texture_id (font_text);

	i = 0;
	cur_x = x;
	cur_y = y;
	glBegin (GL_QUADS);
	while (1)
	{
		if (i == cursor)
		{
			cursor_x = cur_x;
			cursor_y = cur_y;
			if (cursor_x - x > width - displayed_font_x_size)
			{
				cursor_x = x;
				cursor_y = cur_y + displayed_font_y_size;
			}
				
		}

		cur_char = msgs[imsg].data[ichar];
		// watch for special characters
		if (cur_char == '\0') 
		{
			// end of message
			if (++imsg >= msgs_size) imsg = 0;
			if (filter != FILTER_ALL)
			{
				for (;;) {
					char skip = 0;
					int channel = msgs[imsg].chan_idx;
					if (channel != filter) {
						switch (channel) {
							case CHAT_LOCAL:    skip = local_chat_separate;    break;
							case CHAT_PERSONAL: skip = personal_chat_separate; break;
							case CHAT_GM:       skip = guild_chat_separate;    break;
							case CHAT_SERVER:   skip = server_chat_separate;   break;
							case CHAT_MOD:      skip = mod_chat_separate;      break;
							case CHAT_MODPM:                                   break;
							default:            skip = 1;
						}
					}
					if (skip) {
						if (++imsg >= msgs_size) imsg = 0;
						if (msgs[imsg].data == NULL || imsg == msg_start) break;
					} else {
						break;
					}
				}
			}
			if (msgs[imsg].data == NULL || imsg == msg_start) break;
			ichar = 0;
#ifdef MULTI_CHANNEL
			if (msgs[imsg].chan_idx >= CHAT_CHANNEL1 && msgs[imsg].chan_idx <= CHAT_CHANNEL3 && use_windowed_chat==1)
			{
				// when using the window, the input buffer does nasty things.
				// if not using tabs or window, this isn't used at all(but if
				// you're on several channels you're asking for problems anyway)
				if (current_channel + CHAT_CHANNEL1 != msgs[imsg].chan_idx)
				{
					msgs[imsg].data[0] = (Uint8)(127+c_grey2);
				}
				else
				{
					msgs[imsg].data[0] = (Uint8)(127+c_grey1);
				}
			}
#endif
			rewrap_message(&msgs[imsg], text_zoom, width, NULL);
		}
		
		if (cur_char == '\n' || cur_char == '\r' || cur_char == '\0')
		{
			// newline
			cur_y += displayed_font_y_size;
			if (cur_y - y > height - displayed_font_y_size) break;
			cur_x = x;
			if (cur_char != '\0') ichar++;
			i++;
			continue;
		}

		cur_x += draw_char_scaled (cur_char, cur_x, cur_y, displayed_font_x_size, displayed_font_y_size);
		
		ichar++;
		i++;
		if (cur_x - x > width - displayed_font_x_size)
		{
			// ignore rest of this line
			while (msgs[imsg].data[ichar] != '\0' && msgs[imsg].data[ichar] != '\n' && msgs[imsg].data[ichar] != '\r')
			{
				ichar++;
				i++;
			}
		}
	}

	if (cursor_x >= x && cursor_y >= y && cursor_y - y <= height - displayed_font_y_size)
	{
		draw_char_scaled ('_', cursor_x, cursor_y, displayed_font_x_size, displayed_font_y_size);
	}
	
	glEnd();
	glDisable(GL_ALPHA_TEST);	
}

int draw_string (int x, int y, const unsigned char * our_string, int max_lines)
{
	return draw_string_zoomed_width (x, y, our_string, window_width, max_lines, 1.0f);
}

int draw_string_width(int x, int y, const unsigned char * our_string, int max_width, int max_lines)
{
	return draw_string_zoomed_width (x, y, our_string, max_width, max_lines, 1.0f);
}

int draw_string_zoomed (int x, int y, const unsigned char * our_string, int max_lines, float text_zoom)
{
	return draw_string_zoomed_width (x, y, our_string, window_width, max_lines, text_zoom);
}

int draw_string_zoomed_width (int x, int y, const unsigned char * our_string, int max_width, int max_lines, float text_zoom)
{
	float displayed_font_x_size= 11.0*text_zoom;
	float displayed_font_y_size= 18.0*text_zoom;

	unsigned char cur_char;
	int i;
	int cur_x,cur_y;
	int current_lines= 1;

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
					if(current_lines>max_lines)break;
					continue;
				}
			else if (cur_x+displayed_font_x_size-x>=max_width){
				cur_y+=displayed_font_y_size;
				cur_x=x;
				current_lines++;
				if(current_lines>max_lines)break;
			}

			cur_x+=draw_char_scaled(cur_char, cur_x, cur_y, displayed_font_x_size, displayed_font_y_size);

			i++;
		}


	glEnd();
	glDisable(GL_ALPHA_TEST);
	
	return current_lines;
}

void draw_string_clipped(int x, int y, const unsigned char * our_string, int width, int height)
{
	draw_string_zoomed_clipped (x, y, our_string, -1, width, height, 1.0f);
}

void draw_string_zoomed_clipped (int x, int y, const unsigned char* our_string, int cursor_pos, int width, int height, float text_zoom)
{
	float displayed_font_x_size = DEFAULT_FONT_X_LEN * text_zoom;
	float displayed_font_y_size = DEFAULT_FONT_Y_LEN * text_zoom;

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

#ifdef DEBUG
void print_string_escaped (const char *str)
{
	int i;
	
	for (i=0; str[i]; i++)
		if (str[i]=='\r')
			printf ("\\r");
		else if (str[i] == '\n')
			printf ("\\n\n");
		else if ((unsigned char)str[i] < 127+c_red1 || (unsigned char)str[i] > 127+c_grey4)
			printf ("%c", str[i]);
	
	printf ("\nlen = %d\n", i);
}
#endif

int reset_soft_breaks (char *str, int len, int size, float zoom, int width, int *cursor)
{
	char buf[1024]; 
	int font_bit_width;
	int nlines;
	float line_width;
	int isrc, ibuf, idst;
	int lastline, lastsrc;
	int nchar;
	int dcursor = 0;

	if (str == NULL) return 0;
	
	nlines = 1;
	isrc = ibuf = idst = 0;
	line_width = 0;
	lastsrc = 0;
	while (1)
	{
		lastline = 0;
	
		// fill the buffer
		while (isrc < len && str[isrc] != '\0')
		{
			// skip old line breaks
			if (str[isrc] == '\r')
			{
				if (cursor && isrc < *cursor) dcursor--;
				isrc++;
				continue;
			}
			
			// see if it's an explicit line break
			if (str[isrc] == '\n')
			{
				nlines++;
				line_width = 0;
			} else {
				font_bit_width = (int) (0.5f + get_font_width (str[isrc]) * zoom);
				if (line_width + font_bit_width > width)
				{
					// search back for a space
					for (nchar = 0; ibuf-nchar-1 > lastline; nchar++)
						if (buf[ibuf-nchar-1] == ' ') break;
					if (ibuf-nchar-1 <= lastline)
						// no space found, introduce a break in 
						// the middle of the word
						nchar = 0;
					
					// introduce the break, and reset the counters
					ibuf -= nchar;
					isrc -= nchar;
					// ignore any old soft breaks after the space
					while (str[isrc] == '\r') isrc++;

					buf[ibuf] = '\r';
					nlines++; ibuf++;
					if (cursor && isrc < *cursor) dcursor++;
					if (ibuf >= sizeof (buf) - 1) break;
					
					lastline = ibuf;
					line_width = font_bit_width;
				} else {
					line_width += font_bit_width;
				}
			}

			
			// copy the character into the buffer
			buf[ibuf] = str[isrc];
			isrc++; ibuf++;
			if (ibuf >= sizeof (buf) - 1) break;
		}
		if (str[isrc] == '\0') isrc = len;
		
		// buffer is full or end of string, let's copy back
		nchar = ibuf;
		if (isrc < len && isrc - lastsrc < nchar)
			nchar = isrc - lastsrc;

		for (idst = 0; idst < nchar; idst++)
		{
			if (lastsrc + idst >= size - 1) break;
			str[lastsrc+idst] = buf[idst];
		}
		
		// see if we're done
		if (isrc >= len) 
		{
			str[lastsrc+idst] = '\0';
			break;
		}
				
		// move stuff to the beginning of the buffer, if necessary
		nchar = sizeof (buf) - ibuf - 1;
		for (idst = 0; idst < nchar; idst++)
			buf[idst] = buf[ibuf+idst];
		
		// update the counters
		ibuf = nchar;
		lastsrc = isrc;
	}
	
	if (cursor) *cursor += dcursor;
	return nlines;
}

void draw_string_small(int x, int y,const unsigned char * our_string,int max_lines)
{
	int displayed_font_x_size=SMALL_FONT_X_LEN;
	int displayed_font_y_size=SMALL_FONT_Y_LEN;

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
	
	for(i=0; i<len; i++) {
		wdt+= get_char_width(str[i]);
	}

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
	if (set_font_parameters (3) < 0) return 0;

	cur_font_num = 0;

	return 1;
}

void cleanup_fonts(void)
{
	int i;

	for(i = 0; i < FONTS_ARRAY_SIZE; i++) {
		if(fonts[i] != NULL) {
			free(fonts[i]);
		}
	}
}

void reload_fonts()
{
	int i;
	int poor_man_save=poor_man;
	int use_mipmaps_save=use_mipmaps;

	poor_man=0;
	use_mipmaps=0;

	for(i=0;i < FONTS_ARRAY_SIZE; i++){
		if(fonts[i] != NULL){
			if(fonts[i]->texture_id>=0){
				glDeleteTextures(1, &texture_cache[fonts[i]->texture_id].texture_id);
				texture_cache[fonts[i]->texture_id].texture_id=0;
				get_texture_id(fonts[i]->texture_id);
			}
		}
	}

	poor_man=poor_man_save;
	use_mipmaps=use_mipmaps_save;
}

int load_font_textures ()
{
	int poor_man_save=poor_man;
	int use_mipmaps_save=use_mipmaps;
	
	if (fonts[0] == NULL || fonts[1] == NULL || fonts[2] == NULL || fonts[3]==NULL )
	{
		int i;
		for (i = 0; i < FONTS_ARRAY_SIZE; i++) {
			if (fonts[i] != NULL)
				free (fonts[i]);
			fonts[i] = NULL;
		}
		if ( !init_fonts () ) return 0;
	}
	
	poor_man=0;
	use_mipmaps=0;
	
	fonts[0]->texture_id = load_texture_cache ("./textures/font.bmp", 0);
	fonts[1]->texture_id = load_texture_cache ("./textures/fontv.bmp", 0);
	fonts[2]->texture_id = load_texture_cache ("./textures/font2.bmp", 0);
	fonts[3]->texture_id = load_texture_cache ("./textures/font3.bmp", 0);
	
	poor_man=poor_man_save;
	use_mipmaps=use_mipmaps_save;

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
	if(num==0||num==3)for(i=0; i<10*FONT_CHARS_PER_LINE; i++) fonts[num]->widths[i]=12;
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

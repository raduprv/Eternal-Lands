#include <stdlib.h>
#include <string.h>
  #ifdef WINDOWS
    #include <io.h>
  #else //!WINDOWS
    #include <glob.h>
    #include <unistd.h>
  #endif //WINDOWS
#ifdef TTF
#include <SDL2/SDL_ttf.h>
#endif
#include "font.h"
#include "asc.h"
#include "chat.h"
#include "client_serv.h"
#include "colors.h"
#include "elconfig.h"
#include "errors.h"
#include "init.h"
#include "gl_init.h"
#include "interface.h"
#include "misc.h"
#include "textures.h"
#ifndef SKY_FPV_OPTIONAL
#include "draw_scene.h"
#endif // SKY_FPV_OPTIONAL

#define FONT_START_CHAR	32
#define FONT_CHARS_PER_LINE	14
#define	FONT_X_SPACING	18
#define	FONT_Y_SPACING	21
#define FONTS_ARRAY_SIZE	10

#ifdef SKY_FPV_OPTIONAL
#define font_scale 10.0f
#endif // SKY_FPV_OPTIONAL

typedef struct
{
	Uint32 texture_id;
#ifdef TTF
	GLuint texture_id_gl;
#endif
	int texture_width;
	int texture_height;
	int char_widths[FONTS_ARRAY_SIZE * FONT_CHARS_PER_LINE];
	int block_width;
	int block_height;
	int spacing;
} font_info;

#ifdef TTF
int use_ttf = 0;
#endif

int	cur_font_num=0;
int	max_fonts=0;
font_info *fonts[FONTS_ARRAY_SIZE];
char font_names[FONTS_ARRAY_SIZE][30] = { 0 };
int	chat_font=0;
int	name_font=0;
int	book_font=0;

int set_font_parameters (int num);

// CHECK
int pos_selected(int msg, int ichar, select_info* select)
{
	int d = 0;
	if (select == NULL) return 0;
	if (TEXT_FIELD_SELECTION_EMPTY(select)) return 0;
	if (select->em > select->sm) d = 1;
	else if ((select->em == select->sm) && (select->ec >= select->sc)) d = 1;
	else d = -1;
	if (d * (msg - select->sm) > d * (select->em - select->sm)) return 0;
	if ((msg == select->em) && (d * ichar > d * select->ec)) return 0;
	if (d * msg < d * select->sm) return 0;
	if ((msg == select->sm) && (d * ichar < d * select->sc)) return 0;
	return 1;
}

static int get_font_char(unsigned char c)
{
	static const int pos_table[256] = {
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
		16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
		32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
		48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
		64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
		80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  -1,
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		-1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
		-1, 122,  -1,  -1, 109, 118, 116,  -1,  -1, 123,  -1,  -1,  -1, 125,  -1,  -1,
		-1, 120,  -1, 127,  -1,  -1, 110,  -1, 117,  -1, 129,  -1, 111,  -1,  -1, 112,
		98, 121,  97,  -1, 106, 115, 113,  99, 102,  96, 100, 101, 124, 124,  -1, 103,
		-1, 119, 126, 126, 104,  -1, 107,  -1, 114, 105, 128,  -1, 108,  -1,  -1,  -1
	};

	return pos_table[c];
}

static int get_font_width(int pos)
{
	// ignore unknown characters
	if (pos < 0)
		return 0;

	// return width of character + spacing between chars (supports variable width fonts)
	return (fonts[cur_font_num]->char_widths[pos] + fonts[cur_font_num]->spacing);
}

static void set_color(int color)
{
	float r = (float) colors_list[color].r1 / 255.0f;
	float g = (float) colors_list[color].g1 / 255.0f;
	float b = (float) colors_list[color].b1 / 255.0f;
	//This fixes missing letters in the font on some clients
	//No idea why going from 3f to 4f helps, but it does
	glColor4f(r, g, b, 1.0);
}

static void get_texture_coordinates(const font_info *info, int chr,
	float *u_start, float *u_end, float *v_start, float *v_end)
{
	int row, col;
	int bw, bh, cw;

	row = chr / FONT_CHARS_PER_LINE;
	col = chr % FONT_CHARS_PER_LINE;

	bw = info->block_width;
	bh = info->block_height;
	cw = info->char_widths[chr];

	*u_start = (float)(col * bw) / info->texture_width;
	*u_end = (float)(col * bw + cw) / info->texture_width;
	*v_start = (float)(1 + row * bh) / info->texture_height;
	*v_end = (float)(row * bh + bh - 1) / info->texture_height;
}

// returns how far to move for the next char, or negative on error
static int draw_char_scaled(const font_info *info, unsigned char cur_char,
	int x, int y, float zoom)
{
	float u_start, u_end, v_start, v_end;
	int chr;
	int char_width, char_height;
	int cw;

	if (is_color(cur_char))
	{
		set_color(from_color_char(cur_char));
		return 0;
	}

	chr = get_font_char(cur_char);
	if (chr < 0) // watch for illegal/non-display characters
	{
		return 0;
	}

	cw = info->char_widths[chr];
	char_width = (int)(zoom * cw + 0.5);
	char_height = (int)(zoom * info->block_height + 0.5);

	get_texture_coordinates(info, chr, &u_start, &u_end, &v_start, &v_end);

	// and place the text from the graphics on the map
	glTexCoord2f(u_start, v_start); glVertex3i(x, y, 0);
	glTexCoord2f(u_start, v_end);   glVertex3i(x, y + char_height, 0);
	glTexCoord2f(u_end,   v_end);   glVertex3i(x + char_width, y + char_height, 0);
	glTexCoord2f(u_end,   v_start); glVertex3i(x + char_width, y, 0);

	return (int)(zoom * (cw + info->spacing) + 0.5);
}

static void bind_font_texture()
{
	const font_info *info = fonts[cur_font_num];
	if (info->texture_id != (Uint32)-1)
	{
		bind_texture(info->texture_id);
	}
#ifdef TTF
	else
	{
		bind_texture_id(info->texture_id_gl);
	}
#endif
}

void draw_messages(int x, int y, text_message *msgs, int msgs_size, Uint8 filter,
	int msg_start, int offset_start, int cursor, int width, int height,
	float text_zoom, select_info* select)
{
	font_info *info = fonts[cur_font_num];
	float displayed_font_x_size;
	float displayed_font_y_size;

	float selection_red = 255 / 255.0f;
	float selection_green = 162 / 255.0f;
	float selection_blue = 0;

	unsigned char cur_char;
	int i;
	int imsg, ichar;
	int cur_x, cur_y;
	int cursor_x = x-1, cursor_y = y-1;
	unsigned char ch;
	int cur_line = 0;
	int cur_col = 0;
	unsigned char last_color_char = 0;
	int in_select = 0;

	displayed_font_x_size = text_zoom * info->block_width;
	displayed_font_y_size = text_zoom * info->block_height;

	if (width < displayed_font_x_size || height < displayed_font_y_size)
		// no point in trying
		return;

	imsg = msg_start;
	ichar = offset_start;
#ifndef MAP_EDITOR2
	if (filter != FILTER_ALL)
	{
		// skip all messages of the wrong channel
		while (1)
		{
			if (skip_message(&msgs[imsg], filter))
			{
				ichar = 0;
				if (++imsg >= msgs_size) imsg = 0;
				if (imsg == msg_start || msgs[imsg].data == NULL || msgs[imsg].deleted)
					// nothing to draw
					return;
			}
			else
			{
				break;
			}
		}
	}
#endif //! MAP_EDITOR2
	if (msgs[imsg].data == NULL || msgs[imsg].deleted) return;

	ch = msgs[imsg].data[ichar];
	if (!is_color(ch))
	{
		// search backwards for the last color
		for (i = ichar-1; i >= 0; i--)
		{
			ch = msgs[imsg].data[i];
			if (is_color(ch))
			{
				set_color(from_color_char(ch));
				last_color_char = ch;
				break;
			}
		}

		if (i < 0)
		{
			// no color character found, try the message color
			if (msgs[imsg].r >= 0)
				glColor3f(msgs[imsg].r, msgs[imsg].g, msgs[imsg].b);
		}
	}

 	glEnable(GL_ALPHA_TEST);	// enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER, 0.1f);
	bind_font_texture();

	i = 0;
	cur_x = x;
	cur_y = y;
	glBegin(GL_QUADS);
	while (1)
	{
		if (i == cursor)
		{
			cursor_x = cur_x;
			cursor_y = cur_y;
			if (cursor_x - x + displayed_font_x_size > width)
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
			if (++imsg >= msgs_size)
			{
				imsg = 0;
			}
#ifndef MAP_EDITOR2
			if (filter != FILTER_ALL)
			{
				// skip all messages of the wrong channel
				while (skip_message(&msgs[imsg], filter))
				{
					if (++imsg >= msgs_size) imsg = 0;
					if (msgs[imsg].data == NULL || imsg == msg_start) break;
				}
			}
#endif
			if (imsg == msg_start || msgs[imsg].data == NULL || msgs[imsg].deleted) break;
			// Grum 2020-04-07: why do we rewrap here? And not on the first message?
			rewrap_message(&msgs[imsg], text_zoom, width, NULL);
			ichar = 0;
			last_color_char = 0;
		}

		if (select != NULL && select->lines && select->lines[cur_line].msg == -1)
		{
			select->lines[cur_line].msg = imsg;
			select->lines[cur_line].chr = ichar;
		}

		if (cur_char == '\n' || cur_char == '\r' || cur_char == '\0')
		{
			// newline
			cur_y += displayed_font_y_size;
			if (cur_y - y + displayed_font_y_size > height) break;
			cur_x = x;
			if (cur_char != '\0') ichar++;
			i++;
			cur_line++;
			cur_col = 0;
			continue;
		}

		if (pos_selected(imsg, ichar, select))
		{
			if (!in_select)
			{
				glColor3f(selection_red, selection_green, selection_blue);
				in_select = 1;
			}
		}
		else
		{
			if (in_select)
			{
				if (last_color_char)
					set_color(from_color_char(last_color_char));
				else if (msgs[imsg].r >= 0)
					glColor3f(msgs[imsg].r, msgs[imsg].g, msgs[imsg].b);
				else
					set_color(c_grey1);

				in_select = 0;
			}
		}

		if (is_color(cur_char))
		{
			last_color_char = cur_char;
			if (in_select)
			{
				// don't draw color characters in a selection
				i++;
				ichar++;
				continue;
			}
		}

		cur_x += draw_char_scaled(info, cur_char, cur_x, cur_y, text_zoom);
		cur_col++;

		ichar++;
		i++;
		if (cur_x - x + displayed_font_x_size > width)
		{
			// ignore rest of this line, but keep track of
			// color characters
			while (1)
			{
				ch = msgs[imsg].data[ichar];
				if (ch == '\0' || ch == '\n' || ch == '\r')
					break;
				if (is_color(ch))
					last_color_char = ch;
				ichar++;
				i++;
			}
		}
	}

	if (cursor_x >= x && cursor_y >= y && cursor_y - y + displayed_font_y_size <= height)
	{
		draw_char_scaled(info, '_', cursor_x, cursor_y, text_zoom);
	}

	glEnd();
	glDisable(GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

int draw_string_shadowed_zoomed(int x, int y, const unsigned char* our_string,
	int max_lines, float fr, float fg, float fb, float br, float bg, float bb,
	float zoom)
{
	int px, py;

	// set shadow colour
	glColor3f(br, bg, bb);
	for (px = -1; px < 2; px++)
	{
		for (py = -1; py < 2; py++)
		{
			if (px!=0 || py!=0)
			{
				draw_string_zoomed(x+px, y+py, our_string, max_lines, zoom);
			}
		}
	}

	//set foreground colour
	glColor3f(fr, fg, fb);
	return draw_string_zoomed(x, y, our_string, max_lines, zoom);
}

int draw_string_shadowed_width(int x, int y, const unsigned char* our_string,
	int max_width, int max_lines, float fr, float fg, float fb,
	float br, float bg, float bb)
{
	float zoom = (float)max_width / get_string_width(our_string);
	return draw_string_shadowed_zoomed(x, y, our_string, max_lines, fr, fg, fb,
		br, bg, bb, zoom);
}

int draw_string_zoomed(int x, int y, const unsigned char* our_string,
	int max_lines, float text_zoom)
{
	return draw_string_zoomed_width(x, y, our_string, window_width, max_lines, text_zoom);
}

int draw_string_zoomed_width(int x, int y, const unsigned char *our_string,
	int max_width, int max_lines, float text_zoom)
{
	font_info *info = fonts[cur_font_num];
	float displayed_font_x_size = text_zoom * info->block_width;
	float displayed_font_y_size = text_zoom * info->block_height;

	unsigned char cur_char;
	int i;
	int cur_x, cur_y;
	int current_lines = 1;

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER,0.1f);
	bind_font_texture();

	i = 0;
	cur_x = x;
	cur_y = y;
	glBegin(GL_QUADS);
	for (i = 0; our_string[i] ; ++i)
	{
		cur_char = our_string[i];
		// watch for special characters
		if (cur_char == '\n' || cur_char == '\r')	// newline
		{
			cur_y += displayed_font_y_size;
			cur_x = x;
			if (++current_lines > max_lines)
				break;
		}
		else
		{
			if (cur_x - x + displayed_font_x_size >= max_width)
			{
				cur_y += displayed_font_y_size;
				cur_x = x;
				if (++current_lines > max_lines)
					break;
			}
			cur_x += draw_char_scaled(info, cur_char, cur_x, cur_y, text_zoom);
		}
	}

	glEnd();
	glDisable(GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return current_lines;
}

static void draw_string_zoomed_clipped(int x, int y, const unsigned char* our_string,
	int cursor_pos, int width, int height, float text_zoom)
{
	font_info *info = fonts[cur_font_num];
	float displayed_font_x_size = text_zoom * info->block_width;
	float displayed_font_y_size = text_zoom * info->block_height;

	unsigned char cur_char;
	int i;
	int cur_x, cur_y;
	int cursor_x = x-1, cursor_y = y-1;

	if (width < displayed_font_x_size || height < displayed_font_y_size)
		// no point in trying
		return;

	glEnable (GL_ALPHA_TEST);	// enable alpha filtering, so we have some alpha key
	glAlphaFunc (GL_GREATER, 0.1f);
	bind_font_texture();

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
			if (cur_y - y + displayed_font_y_size > height) break;
			cur_x = x;
			i++;
		}
		else
		{
			cur_x += draw_char_scaled(info, cur_char, cur_x, cur_y, text_zoom);

			i++;
			if (cur_x - x + displayed_font_x_size > width)
			{
				// ignore rest of this line
				// FIXME: color characters are also ignored
				while (our_string[i] != '\0' && our_string[i] != '\n' && our_string[i] != '\r') i++;
			}
		}
	}

	if (cursor_x >= x && cursor_y >= y && cursor_y - y + displayed_font_y_size <= height)
	{
		draw_char_scaled(info, '_', cursor_x, cursor_y, text_zoom);
	}

	glEnd();
	glDisable(GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_string_clipped(int x, int y, const unsigned char* our_string,
	int width, int height)
{
	draw_string_zoomed_clipped(x, y, our_string, -1, width, height, 1.0f);
}

int reset_soft_breaks (char *str, int len, int size, float zoom, int width, int *cursor, float *max_line_width)
{
	char *buf;
	int ibuf;
	int nchar;
	int font_bit_width;
	int nlines;
	float line_width;
	int isrc, idst;
	int lastline;
	int dcursor = 0;
	/* the generic special text window code needs to know the
	   maximum line length in pixels.  This information is used
	   but was previously throw away in this function.  Others
	   may fine it useful for setting the winow size, so pass back
	   to the caller if they provide somewhere to store it. */
	float local_max_line_width = 0;

	// error checking
	if (str == NULL || width <= 0 || size <= 0) {
		return 0;
	}

	/* strip existing soft breaks before we start,
		to avoid complicated code later */
	for (isrc=0, idst=0; isrc<len; isrc++)
	{
		if (str[isrc] == '\r')
		{
			/* move the cursor back if after this point */
			if ((cursor != NULL) && (isrc < *cursor))
				dcursor--;
		}
		else
			str[idst++] = str[isrc];
	}
	len = idst;
	str[len] = 0;

	/* allocate the working buffer so it can hold the maximum
	   the source string can take.  Previously, the fixed length
	   buffer was sometimes not big enough.  The code looked
	   to attempt to cope but was floored.  When ever the wrap
	   caused more characters to be in the output, some of the
	   source would be lost.  This is still possable if the source
	   size cannot take the extra characters.  For example, try
	   #glinfo and watch as the end characters are lost.  At least
	   characters are no longer lost wrap process. If you make
	   size large enough for no character will be lost.  Twice
	   the actual string length is probably enough */
	buf = (char *)calloc(size, sizeof(char));

	nlines = 1;
	isrc = ibuf = idst = 0;
	line_width = 0;
	lastline = 0;

	// fill the buffer
	while (isrc < len && str[isrc] != '\0')
	{
		// see if it's an explicit line break
		if (str[isrc] == '\n') {
			nlines++;
			if (line_width > local_max_line_width)
				local_max_line_width = line_width;
			line_width = 0;
		} else {
			font_bit_width = (int) (0.5f + get_char_width (str[isrc]) * DEFAULT_FONT_X_LEN * zoom / 12.0f);
			if (line_width + font_bit_width >= width)
			{
				// search back for a space
				for (nchar = 0; ibuf-nchar-1 > lastline; nchar++) {
					if (buf[ibuf-nchar-1] == ' ') {
						break;
					}
				}
				if (ibuf-nchar-1 <= lastline)
					// no space found, introduce a break in
					// the middle of the word
					nchar = 0;

				// introduce the break, and reset the counters
				ibuf -= nchar;
				isrc -= nchar;

				buf[ibuf] = '\r';
				nlines++; ibuf++;
				if (cursor && isrc < *cursor) {
					dcursor++;
				}
				if (ibuf >= size - 1) {
					break;
				}

				if (line_width > local_max_line_width)
					local_max_line_width = line_width;

				lastline = ibuf;
				line_width = font_bit_width;
			} else {
				line_width += font_bit_width;
			}
		}

		// copy the character into the buffer
		buf[ibuf] = str[isrc];
		isrc++; ibuf++;

		if (ibuf >= size - 1) {
			break;
		}
	}

	safe_strncpy(str, buf, size * sizeof(char));
	str[size-1] = '\0';

	if (cursor) {
		*cursor += dcursor;
		if(*cursor > size-1) {
			/* If there's a better way to detect this, please do */
			*cursor = size-1;
		}
	}
	free(buf);
	if (line_width > local_max_line_width)
		local_max_line_width = line_width;
	if (max_line_width!=NULL)
		*max_line_width = local_max_line_width;
	return nlines;
}

void draw_string_small_zoomed(int x, int y, const unsigned char* our_string,
	int max_lines, float zoom)
{
	draw_string_zoomed(x, y, our_string, max_lines, zoom * DEFAULT_SMALL_RATIO);
}

void draw_string_small_shadowed_zoomed(int x, int y, const unsigned char* our_string,
	int max_lines, float fr, float fg, float fb, float br, float bg, float bb,
	float zoom)
{
	draw_string_shadowed_zoomed(x, y, our_string, max_lines, fr, fg, fb, br, bg, bb,
		zoom * DEFAULT_SMALL_RATIO);
}

#ifdef	ELC
#ifndef MAP_EDITOR2

void draw_ortho_ingame_string(float x, float y,float z,
	const unsigned char * our_string, int max_lines,
	float font_x_scale, float font_y_scale)
{
	font_info *info = fonts[cur_font_num];
	float u_start,u_end,v_start,v_end;
	float displayed_font_x_size;
	float displayed_font_y_size;

	float displayed_font_x_width;
	int	font_bit_width;

	unsigned char cur_char;
	int chr;
	int i;
	float cur_x,cur_y;
	int current_lines=0;

	displayed_font_x_size=font_x_scale*name_zoom*12.0;
	displayed_font_y_size=font_y_scale*name_zoom*12.0;

	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER,0.1f);
	bind_font_texture();

	cur_x = x;
	cur_y = y;
	glBegin(GL_QUADS);

	for (i = 0; our_string[i]; ++i)
	{
		cur_char = our_string[i];
		if (cur_char=='\n')
		{
			cur_y += displayed_font_y_size;
			cur_x = x;
			if (++current_lines >= max_lines)
				break;
		}
		else if (is_color(cur_char))
		{
			glEnd(); // Ooops - NV bug fix!!
			set_color(from_color_char(cur_char));
			glBegin(GL_QUADS);
		}
		else
		{
			chr = get_font_char(cur_char);
			if (chr >= 0)
			{
				font_bit_width = get_font_width(chr);
				displayed_font_x_width = ((float)font_bit_width)*displayed_font_x_size/12.0;
				get_texture_coordinates(info, chr, &u_start, &u_end, &v_start, &v_end);

				glTexCoord2f(u_start,v_start);
				glVertex3f(cur_x,cur_y+displayed_font_y_size,z);

				glTexCoord2f(u_start,v_end);
				glVertex3f(cur_x,cur_y,z);

				glTexCoord2f(u_end,v_end);
				glVertex3f(cur_x+displayed_font_x_width,cur_y,z);

				glTexCoord2f(u_end,v_start);
				glVertex3f(cur_x+displayed_font_x_width,cur_y+displayed_font_y_size,z);

				cur_x+=displayed_font_x_width;
			}
		}
	}

	glEnd();
	glDisable(GL_ALPHA_TEST);
}

void draw_ingame_string(float x, float y, const unsigned char *our_string,
	int max_lines, float font_x_scale, float font_y_scale)
{
	font_info *info = fonts[cur_font_num];
	float u_start,u_end,v_start,v_end;
	float displayed_font_x_size;
	float displayed_font_y_size;

	float displayed_font_x_width;
	int	font_bit_width;

	unsigned char cur_char;
	int chr;
	int i;
	float cur_x,cur_y;
	int current_lines=0;
#ifdef SKY_FPV_OPTIONAL
	double model[16], proj[16],hx,hy,hz;
	int view[4];

	displayed_font_x_size=font_x_scale*name_zoom*12.0*font_scale;
	displayed_font_y_size=font_y_scale*name_zoom*12.0*font_scale;

	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);
	gluProject((double)x,0.0f,(double)y,model, proj, view, &hx,&hy,&hz);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(view[0],view[2]+view[0],view[1],view[3]+view[1],0.0f,-1.0f);
#else // SKY_FPV_OPTIONAL
	displayed_font_x_size=font_x_scale*zoom_level*name_zoom/3.0;
	displayed_font_y_size=font_y_scale*zoom_level*name_zoom/3.0;
#endif // not SKY_FPV_OPTIONAL

	glEnable(GL_ALPHA_TEST);//enable alpha filtering, so we have some alpha key
	glAlphaFunc(GL_GREATER,0.1f);
	bind_font_texture();

#ifdef SKY_FPV_OPTIONAL
	cur_x = hx;
	cur_y = hy;
#else // SKY_FPV_OPTIONAL
	cur_x = x;
	cur_y = y;
#endif // SKY_FPV_OPTIONAL

	glBegin(GL_QUADS);
	for (i = 0; our_string[i]; ++i)
	{
		cur_char = our_string[i];
		if (cur_char == '\n')
		{
			cur_y += displayed_font_y_size;
#ifdef SKY_FPV_OPTIONAL
			cur_x = hx;
#else
			cur_x = x;
#endif
			if (++current_lines>=max_lines)
				break;
		}
		else if (is_color(cur_char))
		{
			glEnd(); // Ooops - NV bug fix!!
			set_color(from_color_char(cur_char));
			glBegin(GL_QUADS); // Ooops - NV bug fix!!
		}
		else
		{
			chr = get_font_char(cur_char);
			if (chr >= 0)
			{
				font_bit_width = get_font_width(chr);
				displayed_font_x_width=((float)font_bit_width)*displayed_font_x_size/12.0;

				get_texture_coordinates(info, chr, &u_start, &u_end, &v_start, &v_end);
#ifdef SKY_FPV_OPTIONAL
				glVertex3f(cur_x,cur_y+displayed_font_y_size,0);

				glTexCoord2f(u_start, v_end);
				glVertex3f(cur_x, cur_y, 0);

				glTexCoord2f(u_end, v_end);
				glVertex3f(cur_x+displayed_font_x_width, cur_y, 0);

				glTexCoord2f(u_end, v_start);
				glVertex3f(cur_x+displayed_font_x_width, cur_y+displayed_font_y_size, 0);
#else // SKY_FPV_OPTIONAL
				glTexCoord2f(u_start, v_start);
				glVertex3f(cur_x, 0, cur_y+displayed_font_y_size);

				glTexCoord2f(u_start, v_end);
				glVertex3f(cur_x, 0, cur_y);

				glTexCoord2f(u_end, v_end);
				glVertex3f(cur_x+displayed_font_x_width, 0, cur_y);

				glTexCoord2f(u_end, v_start);
				glVertex3f(cur_x+displayed_font_x_width, 0, cur_y+displayed_font_y_size);
#endif // SKY_FPV_OPTIONAL
				cur_x += displayed_font_x_width;
			}
		}
	}
	glEnd();
	glDisable(GL_ALPHA_TEST);

#ifdef SKY_FPV_OPTIONAL
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
#endif // SKY_FPV_OPTIONAL
}
#endif //!MAP_EDITOR_2
#endif	//ELC



// font handling
int get_char_width(unsigned char cur_char)
{
	return get_font_width(get_font_char(cur_char));
}

int get_string_width(const unsigned char* str)
{
	int width = 0;
	const unsigned char *c;

	for (c = str; *c; ++c)
		width += get_char_width(*c);

	// adjust to ignore the final spacing
	return width - fonts[cur_font_num]->spacing;
}

int init_fonts()
{
	int i;

	max_fonts = 0;
	for (i = 0; i < FONTS_ARRAY_SIZE; i++)
	{
		fonts[i] = NULL;
		if (set_font_parameters(i) < 0)
			return 0;
	}

	cur_font_num = 0;

	return 1;
}

void cleanup_fonts(void)
{
	int i;

	for(i = 0; i < FONTS_ARRAY_SIZE; i++)
	{
		if (fonts[i] != NULL)
		{
			free(fonts[i]);
		}
	}
}

int load_font_textures ()
{
	static const char texture_dir[] = "textures/";

	size_t i = 0;
	char *glob_pattern;
#ifdef WINDOWS
	struct _finddata_t c_file;
	long hFile;
#else //WINDOWS
	int ret;
	glob_t glob_res;
	size_t j;
#endif //WINDOWS
	char file[60] = "";
	char str[60] = "";

	if (fonts[0] == NULL || fonts[1] == NULL || fonts[2] == NULL || fonts[3]==NULL )
	{
		for (i = 0; i < FONTS_ARRAY_SIZE; i++) {
			if (fonts[i] != NULL)
				free (fonts[i]);
			fonts[i] = NULL;
		}
		if ( !init_fonts () ) return 0;
	}

	fonts[0]->texture_id = load_texture_cached("textures/font.dds", tt_font);
	// Force the selection of the base font.
	safe_strncpy(font_names[0], "Type 1", sizeof(font_names[0]));
	add_multi_option("chat_font", font_names[0]);
	add_multi_option("name_font", font_names[0]);

	// Find what font's exist and load them
	glob_pattern = malloc(strlen(datadir)+sizeof(texture_dir)+10+1); //+10 = font*.bmp*
	sprintf(glob_pattern, "%s%sfont*.dds", datadir, texture_dir);

	i = 1;
#ifdef WINDOWS
	if( (hFile = _findfirst( glob_pattern, &c_file )) == -1L ){
		free(glob_pattern);
		return 0;
	}
	do {
		int	len;

		safe_strncpy(file, c_file.name, sizeof(file));
#else //!WINDOWS
	ret = glob(glob_pattern, 0, NULL, &glob_res);
	if (ret != 0)
	{
		LOG_ERROR("Unable to find any font textures\n");
		free(glob_pattern);
		return 0;
	}
	j = 0;
	while (j < glob_res.gl_pathc && i < FONTS_ARRAY_SIZE)
	{
		int	len;

		safe_strncpy(file, glob_res.gl_pathv[j]+sizeof(texture_dir)-1+strlen(datadir), sizeof(file));
#endif //WINDOWS
		len= strlen(file);
		if (((len + sizeof(texture_dir) - 1) < sizeof(str))
			&& !strncasecmp(file, "font", 4)
			&& has_suffix(file, len, ".dds", 4))
		{
			safe_snprintf(str, sizeof(str), "./textures/%s", file); //Use a relative path here, load_texture_cache_deferred() is using the path wrappers.
			file[len - 4] = 0;
			fonts[i]->texture_id = load_texture_cached(str, tt_font);
			safe_snprintf(font_names[i], sizeof(font_names[i]), "Type %i - %s", i + 1, file);
			add_multi_option("chat_font", font_names[i]);
			add_multi_option("name_font", font_names[i]);
			i++;
		}
#ifndef WINDOWS
		j++;
#endif //WINDOWS
	}
#ifdef WINDOWS
	while ( _findnext( hFile, &c_file ) == 0 );
	_findclose( hFile );
#else //!WINDOWS
	globfree(&glob_res);
#endif //WINDOWS
	free(glob_pattern);

	//set the default font
	cur_font_num = 0;

	return 1;
}

int set_font_parameters(int num)
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
					LOG_ERROR(cant_load_font);
					return -1;
				}
		}
	//watch the highest font
	if(num >= max_fonts)
		{
			max_fonts=num+1;
		}
	// set default font info
	fonts[num]->texture_width = 256;
	fonts[num]->texture_height = 256;
	fonts[num]->spacing=0;
	fonts[num]->block_width = FONT_X_SPACING;
	fonts[num]->block_height = FONT_Y_SPACING;

	// load font information
	// TODO: write this and remove the hack!
	for(i=0; i<FONTS_ARRAY_SIZE*FONT_CHARS_PER_LINE; i++) fonts[num]->char_widths[i]=12;
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
		memcpy(fonts[num]->char_widths, widths, sizeof(widths));
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
		memcpy(fonts[num]->char_widths, widths, sizeof(widths));
		fonts[num]->spacing=2;
	}

	//and return
	return num;
}


int	set_font(int num)
{
	if (num >= 0 && num < max_fonts && fonts[num] && fonts[num]->texture_id >= 0)
	{
		cur_font_num=num;
	}

	return cur_font_num;
}

#ifdef TTF
static int next_power_of_two(int n)
{
	int res = 1;
	while (res < n)
		res *= 2;
	return res;
}

static int render_glyph(Uint16 glyph, int i, int j, int size,
	TTF_Font *font, SDL_Surface *surface)
{
	static const SDL_Color white = { r: 0xff, g: 0xff, b: 0xff, a: 0xff };
	static const SDL_Color black = { r: 0x00, g: 0x00, b: 0x00, a: 0x10 };
	SDL_Surface* glyph_surface;
	SDL_Rect area, glyph_area;
	int w, h;
	int err;

	glyph_surface = TTF_RenderGlyph_Shaded(font, glyph, white, black);
	if (!glyph_surface)
	{
		LOG_ERROR("Failed to render TTF glyph: %s", TTF_GetError());
		return 0;
	}

	w = glyph_surface->w;
	h = glyph_surface->h;

	glyph_area.x = 0;
	glyph_area.y = 0;
	glyph_area.w = w;
	glyph_area.h = h;
	area.x = j*size;
	area.y = i*size;
	area.w = w;
	area.h = h;

	SDL_SetSurfaceAlphaMod(glyph_surface, 0xFF);
	SDL_SetSurfaceBlendMode(glyph_surface, SDL_BLENDMODE_NONE);
	err = SDL_BlitSurface(glyph_surface, &glyph_area, surface, &area);
	SDL_FreeSurface(glyph_surface);
	if (err)
	{
		LOG_ERROR("Failed to write glyph to surface: %s", SDL_GetError());
		return 0;
	}

	return w;
}

static int font_line_skip(const char* file_name, int pt_size)
{
	int size;
	TTF_Font *font = TTF_OpenFont(file_name, pt_size);
	if (!font)
	{
		LOG_ERROR("Failed to open TrueType font %s: %s", file_name, TTF_GetError());
		return 0;
	}
	size = TTF_FontLineSkip(font);
	TTF_CloseFont(font);
	return size;
}

static int find_point_size(const char* file_name)
{
	int min_pt_size, max_pt_size, min_size, max_size;

	min_pt_size = 10;
	min_size = font_line_skip(file_name, min_pt_size);
	if (min_size == 0)
		return 0;
	if (min_size >= FONT_Y_SPACING)
		return min_pt_size;

	max_pt_size = 40;
	max_size = font_line_skip(file_name, max_pt_size);
	if (max_size == 0)
		return 0;
	if (max_size <= FONT_Y_SPACING)
		return max_pt_size;

	while (max_pt_size > min_pt_size + 1)
	{
		int pt_size = (min_pt_size + max_pt_size) / 2;
		int size = font_line_skip(file_name, pt_size);

		if (size == 0)
			return 0;
		if (size == FONT_Y_SPACING)
			return pt_size;
		if (size < FONT_Y_SPACING)
		{
			min_pt_size = pt_size;
			min_size = size;
		}
		else
		{
			max_pt_size = pt_size;
			max_size = size;
		}
	}

	return FONT_Y_SPACING - min_size < max_size - FONT_Y_SPACING
		? min_pt_size : max_pt_size;
}

int has_glyph(unsigned char c)
{
	return get_font_char(c) >= 0;
}

int build_ttf_texture_atlas(const char* file_name)
{
	static const Uint16 glyphs[] = {
		' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-',
		'.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';',
		'<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
		'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
		'X', 'Y', 'Z', '[', '\\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e',
		'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
		't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 252, 233, 226, // üéâ'
		224, 231, 234, 235, 232, 239, 244, 249, 228, 246, 252, 196, 214, 220, // àçêëèïôùäöüÄÖÜ
		223, 230, 248, 229, 198, 216, 197, 241, 209, 225, 193, 201, 237, 205, // ßæøåÆØÅñÑáÁÉíÍ
		243, 211, 250, 218, 251, 238                                          // óÓúÚûî

	};
	static const int nr_glyphs = sizeof(glyphs) / sizeof(glyphs[0]);

	TTF_Font *font;
	int i_glyph;
	font_info *info;
	int nr_rows, size, width, height;
	SDL_Surface *image;
	GLuint texture_id;
	int pt_size;
	int font_num;
	const char* name;

	for (font_num = 0; font_num < FONTS_ARRAY_SIZE; ++font_num)
	{
		if (!font_names[font_num][0])
			break;
	}
	if (font_num >= FONTS_ARRAY_SIZE)
	{
		LOG_ERROR("No space to store new font", font_num);
		return 0;
	}

	info = calloc(1, sizeof(font_info));
	if (!info)
	{
		LOG_ERROR("Failed to allocate font information");
		return 0;
	}

	// Try to find a font size where the fot height is approximately equal to
	// the nr of pixels used here. We could scale later, but using an appropriate
	// font size gives better looking results
	pt_size = find_point_size(file_name);
	if (pt_size == 0)
		return 0;
	font = TTF_OpenFont(file_name, pt_size);
	if (!font)
	{
		LOG_ERROR("Failed to open TrueType font %s: %s", file_name, TTF_GetError());
		return 0;
	}

	size = TTF_FontLineSkip(font);
	nr_rows = (nr_glyphs+FONT_CHARS_PER_LINE-1) / FONT_CHARS_PER_LINE;
	width = next_power_of_two(FONT_CHARS_PER_LINE*size);
	height = next_power_of_two(nr_rows * size);
	image = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000
#else
		0xFF000000,
		0x00FF0000,
		0x0000FF00,
		0x000000FF
#endif
	);
	if (!image)
	{
		LOG_ERROR("Failed to create surface for TTF texture atlas: %s", TTF_GetError());
		TTF_CloseFont(font);
		return 0;
	}

	for (i_glyph = 0; i_glyph < nr_glyphs; ++i_glyph)
	{
		int i = i_glyph / FONT_CHARS_PER_LINE;
		int j = i_glyph % FONT_CHARS_PER_LINE;
		int w = render_glyph(glyphs[i_glyph], i, j, size, font, image);
		if (w == 0)
		{
			SDL_FreeSurface(image);
			TTF_CloseFont(font);
			return 0;
		}
		info->char_widths[i_glyph] = w;
	}

	glGenTextures(1, &texture_id);
	bind_texture_id(texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		image->pixels);

	SDL_FreeSurface(image);
	info->spacing = 0;
	info->texture_id = (Uint32)-1;
	info->texture_id_gl = texture_id;
	info->texture_width = width;
	info->texture_height = height;
	info->block_width = size;
	info->block_height = size;
	fonts[font_num] = info;

	name = TTF_FontFaceFamilyName(font);
	safe_strncpy(font_names[font_num], name, sizeof(font_names[font_num]));
	add_multi_option("chat_font", font_names[font_num]);
	add_multi_option("name_font", font_names[font_num]);

	TTF_CloseFont(font);

	return 0;
}
#endif // TTF

int get_line_height(int font_num)
{
#ifdef TTF
	if (font_num < 0 || font_num >= FONTS_ARRAY_SIZE || !fonts[font_num])
		return 0;
	return fonts[font_num]->block_height;
#else
	return DEFAULT_FONT_Y_LEN;
#endif
}

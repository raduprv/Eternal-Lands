#ifndef __FONT_H__
#define __FONT_H__

// constants for fonts
#define FONT_START_CHAR	32	// first character in font.bmp
#define FONT_CHARS_PER_LINE	14	// how manu chars per line in font.bmp?
#define	FONT_X_SPACING	18	//X spacing of font in font.bmp
#define	FONT_Y_SPACING	21	//Y spacing of font in font.bmp

// contstant sizes
#define	INGAME_FONT_X_LEN 0.17
#define	INGAME_FONT_Y_LEN 0.25
#define SMALL_INGAME_FONT_X_LEN 0.12
#define SMALL_INGAME_FONT_Y_LEN 0.17
#define ALT_INGAME_FONT_X_LEN 0.10
#define ALT_INGAME_FONT_Y_LEN 0.15

typedef struct	{
	int	spacing;
	int texture_id;
	int	widths[9*FONT_CHARS_PER_LINE];
	char name[32];
} font_info;

extern int	cur_font_num;
extern int	max_fonts;
extern	font_info	*fonts[];

extern int	chat_font;
extern int	name_font;

void draw_string(int x, int y,const unsigned char * our_string,int max_lines);
void draw_string_zoomed(int x, int y,const unsigned char * our_string,int max_lines, float text_zoom);
void draw_string_small(int x, int y,const unsigned char * our_string,int max_lines);
#ifdef	ELC
void draw_ingame_string(float x, float y, const unsigned char * our_string, int max_lines, float font_x_scale, float font_y_scale);
#define draw_ingame_normal(x, y, our_string, max_lines)	draw_ingame_string(x, y, our_string, max_lines, INGAME_FONT_X_LEN, INGAME_FONT_Y_LEN)
#define draw_ingame_small(x, y, our_string, max_lines)	draw_ingame_string(x, y, our_string, max_lines, SMALL_INGAME_FONT_X_LEN, SMALL_INGAME_FONT_Y_LEN)
#define draw_ingame_alt(x, y, our_string, max_lines)	draw_ingame_string(x, y, our_string, max_lines, ALT_INGAME_FONT_X_LEN, ALT_INGAME_FONT_Y_LEN)
#endif	//ELC
int	draw_char_scaled(unsigned char cur_char, int cur_x, int cur_y, float displayed_font_x_size, float displayed_font_y_size);

int get_font_char(unsigned char cur_char);
int find_font_char(unsigned char cur_char);
int get_font_width(int cur_char);
int get_char_width(unsigned char cur_char);
int get_string_width(const unsigned char *str);
int get_nstring_width(const unsigned char *str, int len);

int load_font(int num, char *file);
int	set_font(int num);
void remove_font(int num);
int init_fonts();

#endif


#ifndef __FONT_H__
#define __FONT_H__

// constants for fonts
#define FONT_START_CHAR	32	// first character in font.bmp
#define FONT_CHARS_PER_LINE	14	// how manu chars per lin in font.bmp?
#define	FONT_X_SPACING	18	//X spacing of font in font.bmp
#define	FONT_Y_SPACING	21	//Y spacing of font in font.bmp

void draw_string(int x, int y,unsigned char * our_string,int max_lines);
void draw_string_small(int x, int y,unsigned char * our_string,int max_lines);
void draw_ingame_string(float x, float y,unsigned char * our_string,
						int max_lines,int big);
#endif

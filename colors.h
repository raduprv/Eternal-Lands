#ifndef __COLORS_H__
#define __COLORS_H__

typedef struct
{
	Uint8 r1;
	Uint8 g1;
	Uint8 b1;
	Uint8 r2;
	Uint8 g2;
	Uint8 b2;
	Uint8 r3;
	Uint8 g3;
	Uint8 b3;
	Uint8 r4;
	Uint8 g4;
	Uint8 b4;

} color_rgb;

extern color_rgb colors_list[30];

void init_colors();
#endif

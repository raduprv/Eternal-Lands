#include "global.h"

color_rgb colors_list[25];

void init_colors()
{
	int i;

	i=c_red;
	colors_list[i].r1=(float)255/255;
	colors_list[i].g1=(float)20/255;
	colors_list[i].b1=(float)20/255;
	colors_list[i].r2=(float)255/255;
	colors_list[i].g2=(float)40/255;
	colors_list[i].b2=(float)40/255;
	colors_list[i].r3=(float)255/255;
	colors_list[i].g3=(float)60/255;
	colors_list[i].b3=(float)60/255;
	colors_list[i].r4=(float)255/255;
	colors_list[i].g4=(float)90/255;
	colors_list[i].b4=(float)90/255;

	i=c_blue;
	colors_list[i].r1=(float)20/255;
	colors_list[i].g1=(float)30/255;
	colors_list[i].b1=(float)255/255;
	colors_list[i].r2=(float)20/255;
	colors_list[i].g2=(float)45/255;
	colors_list[i].b2=(float)255/255;
	colors_list[i].r3=(float)30/255;
	colors_list[i].g3=(float)55/255;
	colors_list[i].b3=(float)255/255;
	colors_list[i].r4=(float)30/255;
	colors_list[i].g4=(float)70/255;
	colors_list[i].b4=(float)255/255;

	i=c_green;
	colors_list[i].r1=(float)20/255;
	colors_list[i].g1=(float)255/255;
	colors_list[i].b1=(float)20/255;
	colors_list[i].r2=(float)35/255;
	colors_list[i].g2=(float)255/255;
	colors_list[i].b2=(float)35/255;
	colors_list[i].r3=(float)50/255;
	colors_list[i].g3=(float)255/255;
	colors_list[i].b3=(float)50/255;
	colors_list[i].r4=(float)65/255;
	colors_list[i].g4=(float)250/255;
	colors_list[i].b4=(float)65/255;

	i=c_yellow;
	colors_list[i].r1=(float)200/255;
	colors_list[i].g1=(float)200/255;
	colors_list[i].b1=(float)0/255;
	colors_list[i].r2=(float)220/255;
	colors_list[i].g2=(float)220/255;
	colors_list[i].b2=(float)0/255;
	colors_list[i].r3=(float)235/255;
	colors_list[i].g3=(float)235/255;
	colors_list[i].b3=(float)0/255;
	colors_list[i].r4=(float)255/255;
	colors_list[i].g4=(float)255/255;
	colors_list[i].b4=(float)0/255;

	i=c_orange;
	colors_list[i].r1=(float)200/255;
	colors_list[i].g1=(float)150/255;
	colors_list[i].b1=(float)0/255;
	colors_list[i].r2=(float)220/255;
	colors_list[i].g2=(float)170/255;
	colors_list[i].b2=(float)0/255;
	colors_list[i].r3=(float)235/255;
	colors_list[i].g3=(float)180/255;
	colors_list[i].b3=(float)0/255;
	colors_list[i].r4=(float)255/255;
	colors_list[i].g4=(float)190/255;
	colors_list[i].b4=(float)0/255;

	i=c_violet;
	colors_list[i].r1=(float)200/255;
	colors_list[i].g1=(float)0/255;
	colors_list[i].b1=(float)150/255;
	colors_list[i].r2=(float)220/255;
	colors_list[i].g2=(float)0/255;
	colors_list[i].b2=(float)170/255;
	colors_list[i].r3=(float)235/255;
	colors_list[i].g3=(float)0/255;
	colors_list[i].b3=(float)180/255;
	colors_list[i].r4=(float)255/255;
	colors_list[i].g4=(float)0/255;
	colors_list[i].b4=(float)190/255;

	i=c_light_red;
	colors_list[i].r1=(float)255/255;
	colors_list[i].g1=(float)70/255;
	colors_list[i].b1=(float)70/255;
	colors_list[i].r2=(float)255/255;
	colors_list[i].g2=(float)90/255;
	colors_list[i].b2=(float)90/255;
	colors_list[i].r3=(float)255/255;
	colors_list[i].g3=(float)120/255;
	colors_list[i].b3=(float)120/255;
	colors_list[i].r4=(float)255/255;
	colors_list[i].g4=(float)150/255;
	colors_list[i].b4=(float)150/255;

	i=c_light_blue;
	colors_list[i].r1=(float)20/255;
	colors_list[i].g1=(float)180/255;
	colors_list[i].b1=(float)200/255;
	colors_list[i].r2=(float)30/255;
	colors_list[i].g2=(float)190/255;
	colors_list[i].b2=(float)220/255;
	colors_list[i].r3=(float)40/255;
	colors_list[i].g3=(float)200/255;
	colors_list[i].b3=(float)235/255;
	colors_list[i].r4=(float)40/255;
	colors_list[i].g4=(float)210/255;
	colors_list[i].b4=(float)255/255;

	i=c_light_green;
	colors_list[i].r1=(float)20/255;
	colors_list[i].g1=(float)200/255;
	colors_list[i].b1=(float)80/255;
	colors_list[i].r2=(float)30/255;
	colors_list[i].g2=(float)220/255;
	colors_list[i].b2=(float)100/255;
	colors_list[i].r3=(float)40/255;
	colors_list[i].g3=(float)235/255;
	colors_list[i].b3=(float)120/255;
	colors_list[i].r4=(float)50/255;
	colors_list[i].g4=(float)255/255;
	colors_list[i].b4=(float)140/255;
/*
	i=;
	colors_list[i].r1=(float)/255;
	colors_list[i].g1=(float)/255;
	colors_list[i].b1=(float)/255;
	colors_list[i].r2=(float)/255;
	colors_list[i].g2=(float)/255;
	colors_list[i].b2=(float)/255;
	colors_list[i].r3=(float)/255;
	colors_list[i].g3=(float)/255;
	colors_list[i].b3=(float)/255;
	colors_list[i].r4=(float)/255;
	colors_list[i].g4=(float)/255;
	colors_list[i].b4=(float)/255;
*/

}

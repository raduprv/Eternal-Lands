#include "global.h"

/*
	border pixmap copied from this image:
	http://www.macdevcenter.com/mac/2003/02/28/graphics/image008.jpg
	
	Image ordering as follows:
	
	--------.
	        |1
	window  |2
		    |2
	--------'3
	6666666654

*/

int use_alpha_border = 1;

static const struct {
	unsigned int width;
	unsigned int height;
	float data[20][13];
} alphamap1 = {13, 20, 
	{
		{0.74f,0.83f,0.87f,0.91f,0.96f,0.91f,0.96f,0.96f,1.00f,0.96f,1.00f,1.00f,1.00f},
		{0.70f,0.83f,0.83f,0.87f,0.91f,0.91f,0.96f,0.96f,1.00f,0.96f,1.00f,1.00f,1.00f},
		{0.74f,0.78f,0.87f,0.87f,0.91f,0.91f,0.96f,0.91f,0.96f,0.96f,1.00f,0.96f,1.00f},
		{0.74f,0.78f,0.78f,0.87f,0.87f,0.91f,0.96f,0.96f,0.91f,0.96f,1.00f,0.96f,1.00f},
		{0.74f,0.74f,0.83f,0.83f,0.91f,0.87f,0.91f,0.91f,0.96f,0.96f,1.00f,0.96f,1.00f},
		{0.65f,0.78f,0.78f,0.87f,0.83f,0.87f,0.87f,0.96f,0.91f,1.00f,0.96f,0.96f,0.96f},
		{0.74f,0.74f,0.83f,0.78f,0.87f,0.87f,0.96f,0.91f,1.00f,0.96f,0.96f,0.96f,0.96f},
		{0.65f,0.74f,0.74f,0.83f,0.78f,0.87f,0.87f,0.96f,0.91f,1.00f,0.96f,0.96f,1.00f},
		{0.70f,0.70f,0.74f,0.78f,0.87f,0.87f,0.91f,0.91f,0.96f,0.91f,0.96f,0.96f,0.96f},
		{0.65f,0.70f,0.70f,0.78f,0.78f,0.91f,0.96f,0.96f,0.96f,0.96f,1.00f,0.96f,1.00f},
		{0.65f,0.70f,0.74f,0.78f,0.83f,0.78f,0.87f,0.87f,0.96f,0.96f,0.96f,0.96f,0.96f},
		{0.57f,0.70f,0.65f,0.83f,0.78f,0.87f,0.87f,0.91f,0.96f,1.00f,0.96f,0.96f,1.00f},
		{0.70f,0.61f,0.70f,0.74f,0.83f,0.83f,0.87f,0.83f,0.96f,1.00f,1.00f,1.00f,1.00f},
		{0.57f,0.70f,0.70f,0.78f,0.74f,0.83f,0.87f,0.91f,0.91f,1.00f,0.96f,0.96f,1.00f},
		{0.61f,0.61f,0.74f,0.70f,0.78f,0.83f,0.87f,0.87f,0.96f,0.91f,0.96f,0.96f,0.96f},
		{0.57f,0.57f,0.61f,0.74f,0.70f,0.78f,0.91f,0.87f,0.91f,0.96f,0.96f,0.96f,0.96f},
		{0.52f,0.52f,0.65f,0.65f,0.74f,0.78f,0.83f,0.83f,0.96f,0.96f,1.00f,0.96f,1.00f},
		{0.43f,0.57f,0.61f,0.70f,0.70f,0.78f,0.78f,0.87f,0.87f,0.96f,0.91f,1.00f,0.96f},
		{0.52f,0.52f,0.65f,0.65f,0.74f,0.74f,0.83f,0.83f,0.96f,0.91f,1.00f,0.96f,1.00f},
		{0.43f,0.52f,0.57f,0.65f,0.65f,0.78f,0.83f,0.87f,0.91f,0.96f,0.91f,1.00f,0.96f}
	}
};

static const struct {
	unsigned int width;
	unsigned int height;
	float data[2][13];
} alphamap2 = {13, 2, 
	{
		{0.52f,0.52f,0.65f,0.65f,0.74f,0.78f,0.83f,0.83f,0.96f,0.96f,1.00f,0.96f,1.00f},
		{0.43f,0.57f,0.61f,0.70f,0.70f,0.78f,0.78f,0.87f,0.87f,0.96f,0.91f,1.00f,0.96f}
	}
};

static const struct {
	unsigned int width;
	unsigned int height;
	float data[5][13];
} alphamap3 = {13, 5, 
	{
		{0.43f,0.57f,0.61f,0.70f,0.70f,0.78f,0.78f,0.87f,0.87f,0.96f,0.91f,1.00f,0.96f},
		{0.52f,0.52f,0.65f,0.65f,0.74f,0.78f,0.83f,0.83f,0.96f,0.91f,1.00f,0.96f,1.00f},
		{0.48f,0.57f,0.61f,0.70f,0.70f,0.78f,0.78f,0.91f,0.91f,0.91f,0.91f,1.00f,0.96f},
		{0.48f,0.57f,0.70f,0.65f,0.78f,0.78f,0.83f,0.83f,0.96f,0.91f,1.00f,0.96f,0.96f},
		{0.48f,0.61f,0.65f,0.74f,0.70f,0.83f,0.78f,0.91f,0.87f,0.96f,0.91f,0.96f,0.96f}
	}
};

static const struct {
	unsigned int width;
	unsigned int height;
	float data[20][13];
} alphamap4 = {13, 20, 
	{
		{0.48f,0.61f,0.61f,0.74f,0.74f,0.83f,0.83f,0.91f,0.87f,0.96f,1.00f,1.00f,0.96f},
		{0.61f,0.61f,0.70f,0.70f,0.78f,0.78f,0.91f,0.87f,0.96f,0.96f,1.00f,0.96f,0.96f},
		{0.57f,0.70f,0.70f,0.74f,0.78f,0.83f,0.87f,0.96f,0.91f,1.00f,1.00f,1.00f,0.96f},
		{0.61f,0.61f,0.70f,0.74f,0.83f,0.83f,0.91f,0.91f,0.91f,0.96f,0.96f,0.96f,0.96f},
		{0.57f,0.70f,0.74f,0.78f,0.78f,0.87f,0.87f,1.00f,0.91f,0.96f,0.96f,0.96f,0.96f},
		{0.74f,0.70f,0.74f,0.78f,0.83f,0.83f,0.91f,0.91f,0.91f,0.96f,0.96f,0.96f,0.96f},
		{0.70f,0.74f,0.74f,0.83f,0.83f,0.91f,0.87f,0.96f,0.91f,0.96f,0.96f,0.96f,1.00f},
		{0.74f,0.70f,0.78f,0.83f,0.87f,0.87f,0.96f,0.91f,1.00f,0.96f,1.00f,1.00f,1.00f},
		{0.74f,0.83f,0.83f,0.87f,0.87f,0.91f,0.91f,0.96f,0.96f,1.00f,0.96f,0.96f,0.96f},
		{0.83f,0.83f,0.87f,0.91f,0.91f,0.87f,0.91f,0.96f,0.96f,1.00f,1.00f,0.96f,1.00f},
		{0.83f,0.91f,0.87f,0.91f,0.91f,0.96f,0.91f,0.96f,0.96f,0.96f,0.96f,0.96f,0.96f},
		{0.87f,0.78f,0.91f,0.91f,0.91f,0.96f,1.00f,0.96f,0.96f,0.96f,1.00f,0.96f,1.00f},
		{0.87f,0.91f,0.91f,0.91f,0.91f,1.00f,0.96f,1.00f,0.96f,0.96f,1.00f,1.00f,1.00f},
		{0.87f,0.91f,0.91f,0.96f,0.96f,0.96f,0.96f,0.96f,0.96f,0.96f,0.96f,0.96f,1.00f},
		{0.96f,1.00f,0.91f,1.00f,0.96f,0.96f,0.91f,0.96f,0.96f,1.00f,1.00f,0.96f,1.00f},
		{1.00f,0.96f,0.96f,0.91f,0.96f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f},
		{0.91f,0.96f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f},
		{1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f},
		{1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f},
		{1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f}
	}
};

static const struct {
	unsigned int width;
	unsigned int height;
	float data[20][14];
} alphamap5 = {14, 20, 
	{
		{0.00f,0.04f,0.00f,0.04f,0.00f,0.13f,0.13f,0.26f,0.17f,0.30f,0.30f,0.39f,0.35f,0.48f},
		{0.09f,0.04f,0.13f,0.04f,0.17f,0.13f,0.22f,0.22f,0.30f,0.30f,0.39f,0.39f,0.48f,0.52f},
		{0.09f,0.13f,0.09f,0.13f,0.13f,0.22f,0.17f,0.22f,0.26f,0.39f,0.35f,0.48f,0.43f,0.57f},
		{0.22f,0.13f,0.26f,0.17f,0.22f,0.17f,0.30f,0.26f,0.35f,0.35f,0.43f,0.43f,0.52f,0.52f},
		{0.22f,0.22f,0.17f,0.26f,0.26f,0.35f,0.30f,0.39f,0.35f,0.43f,0.43f,0.52f,0.48f,0.57f},
		{0.30f,0.22f,0.30f,0.30f,0.35f,0.30f,0.39f,0.39f,0.48f,0.43f,0.57f,0.52f,0.61f,0.61f},
		{0.30f,0.35f,0.30f,0.39f,0.35f,0.43f,0.39f,0.48f,0.48f,0.57f,0.52f,0.61f,0.57f,0.65f},
		{0.43f,0.39f,0.43f,0.35f,0.48f,0.43f,0.48f,0.48f,0.57f,0.52f,0.61f,0.57f,0.65f,0.65f},
		{0.43f,0.52f,0.43f,0.48f,0.52f,0.57f,0.57f,0.61f,0.57f,0.65f,0.61f,0.70f,0.70f,0.74f},
		{0.57f,0.57f,0.57f,0.57f,0.61f,0.57f,0.61f,0.61f,0.65f,0.65f,0.70f,0.70f,0.74f,0.74f},
		{0.57f,0.65f,0.57f,0.70f,0.57f,0.70f,0.61f,0.70f,0.70f,0.78f,0.74f,0.78f,0.78f,0.83f},
		{0.70f,0.65f,0.65f,0.65f,0.74f,0.70f,0.74f,0.70f,0.74f,0.74f,0.78f,0.74f,0.83f,0.83f},
		{0.70f,0.74f,0.70f,0.74f,0.74f,0.74f,0.74f,0.78f,0.78f,0.83f,0.78f,0.87f,0.78f,0.91f},
		{0.83f,0.78f,0.83f,0.78f,0.83f,0.78f,0.87f,0.78f,0.83f,0.83f,0.83f,0.74f,0.87f,0.87f},
		{0.83f,0.91f,0.83f,0.87f,0.83f,0.87f,0.83f,0.87f,0.78f,0.91f,0.87f,0.91f,0.87f,0.96f},
		{0.96f,0.87f,0.96f,0.87f,0.96f,0.87f,0.96f,0.91f,0.96f,0.87f,0.96f,0.96f,1.00f,0.91f},
		{0.91f,0.96f,0.91f,0.91f,0.91f,0.96f,0.91f,0.96f,0.96f,1.00f,0.91f,1.00f,0.96f,0.96f},
		{1.00f,0.91f,1.00f,0.96f,1.00f,0.96f,1.00f,0.96f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f},
		{0.96f,1.00f,0.96f,1.00f,0.96f,0.96f,0.96f,0.96f,0.96f,1.00f,1.00f,1.00f,1.00f,1.00f},
		{1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f}
	}
};

static const struct {
	unsigned int width;
	unsigned int height;
	float data[20][2];
} alphamap6 = {2, 20, 
	{
		{0.00f,0.04f},
		{0.09f,0.00f},
		{0.04f,0.13f},
		{0.22f,0.09f},
		{0.17f,0.22f},
		{0.26f,0.26f},
		{0.26f,0.39f},
		{0.43f,0.35f},
		{0.43f,0.52f},
		{0.57f,0.52f},
		{0.57f,0.65f},
		{0.74f,0.61f},
		{0.70f,0.74f},
		{0.83f,0.78f},
		{0.83f,0.91f},
		{0.96f,0.87f},
		{0.91f,0.96f},
		{1.00f,0.91f},
		{0.96f,1.00f},
		{1.00f,1.00f}
	}
};

void draw_window_alphaborder(window_info *win) {
	int x, y, offsety, offsetx;
	int rightx = win->len_x + 1;
	int bottomy = win->len_y + 1;
	const float r = win->back_color[0];
	const float g = win->back_color[1];
	const float b = win->back_color[2];
	
	glColor3f(r, g, g);
	glBegin(GL_LINE_LOOP);
	glVertex3i(0, 0, 0);
	glVertex3i(win->len_x, 0, 0);
	glVertex3i(win->len_x, win->len_y, 0);
	glVertex3i(0, win->len_y, 0);
	glEnd();

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_SRC_ALPHA);
	glPointSize(1.0f);
	glBegin(GL_POINTS);
	
	for(y = 0; y < alphamap1.height; y++) {
		for(x = 0; x < alphamap1.width; x++) {
			glColor4f(r, g, b, alphamap1.data[y][x]);
			glVertex3i(rightx + x, y, 0);
			glVertex3i(-x - 1, y, 0);
		}
	}
	for(y = alphamap1.height; y < bottomy - alphamap3.height; y++) {
		offsety = (y - alphamap1.height) % alphamap2.height;
		for(x = 0; x < alphamap2.width; x++) {
			glColor4f(r, g, b, alphamap2.data[offsety][x]);
			glVertex3i(rightx + x, y, 0);
			glVertex3i(-x - 1, y, 0);
		}
	}
	offsety = bottomy - alphamap3.height;
	for(y = 0; y < alphamap3.height; y++) {
		for(x = 0; x < alphamap3.width; x++) {
			glColor4f(r, g, b, alphamap3.data[y][x]);
			glVertex3i(rightx + x, y + offsety, 0);
			glVertex3i(-x - 1, y + offsety, 0);
		}
	}
	
	for(y = 0; y < alphamap4.height; y++) {
		for(x = 0; x < alphamap4.width; x++) {
			glColor4f(r, g, b, alphamap4.data[y][x]);
			glVertex3i(rightx + x, y + bottomy, 0);
			glVertex3i(-x - 1, y + bottomy, 0);
		}
	}
	
	for(y = 0; y < alphamap5.height; y++) {
		for(x = 0; x < alphamap5.width; x++) {
			glColor4f(r, g, b, alphamap5.data[y][x]);
			glVertex3i(rightx - alphamap5.width + x, y + bottomy, 0);
			glVertex3i(alphamap5.width - x - 1, y + bottomy, 0);
		}
	}
	
	for(x = alphamap5.width; x < rightx - alphamap5.width; x++) {
		offsetx = (x - alphamap5.width) % alphamap6.width;
		for(y = 0; y < alphamap6.height; y++) {
			glColor4f(r, g, b, alphamap6.data[y][offsetx]);
			glVertex3i(x, y + bottomy, 0);
		}
	}
	
	
	glEnd();
	glDisable(GL_BLEND);
}

#include <stdlib.h>
#include "highlight.h"
#include "global.h"
#include "platform.h"
#include "tiles.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include <math.h>

#define HIGHLIGHT_MARKER_LIFESPAN	(500)
#define NUMBER_OF_HIGHLIGHT_MARKERS	(10)

typedef struct {
	short x;
	short y;
	int timeleft;
	int type;
	int active;
} highlight_marker;

highlight_marker markers[NUMBER_OF_HIGHLIGHT_MARKERS];

int highlighting_enabled = 1;

highlight_marker *get_free_highlight_marker(short x, short y) {
	int i;
	//first try to find one that already occupies this tile
	for(i = 0; i < NUMBER_OF_HIGHLIGHT_MARKERS; i++) {
		if (markers[i].active && markers[i].x == x && markers[i].y == y) {
			return &markers[i];
		}
	}
	//otherwise, find the first free slot
	for(i = 0; i < NUMBER_OF_HIGHLIGHT_MARKERS; i++) {
		if (!markers[i].active) {
			return &markers[i];
		}
	}
	return NULL;
}

void add_highlight(short x, short y, int type) {
	highlight_marker *m = get_free_highlight_marker(x, y);
	if (m == NULL) return;
	
	m->x = x;
	m->y = y;
	m->type = type;
	m->timeleft = HIGHLIGHT_MARKER_LIFESPAN;
	m->active = 1;
}

void display_highlight_marker(const highlight_marker *marker) {
	// (a) varies from 1..0 depending on the age of this marker
	const float a = ((float)marker->timeleft) / HIGHLIGHT_MARKER_LIFESPAN;
	
	float z = get_tile_height(marker->x, marker->y);
	
	float x = (float)marker->x/2;
	float y = (float)marker->y/2;
	
	/*
	The highlighting marker is four polygons like the one below rotated with C
	 as center.
		
	A---B---+
	|   |   |
	D---+---+
	|   |   |
	+---+---C
	(polygon is A->B->C->D->A)
		
	where C is supposed to be the center of the highlighted tile (offset by
	 center_offset_x, center_offset_y though).
	Distance A->B is (dx), distance A->D is (dy)
	*/
	
	const float dx = (TILESIZE_X / 6);
	const float dy = (TILESIZE_Y / 6);
	
	//Move the offset of the C point closer to the actual center of the tile depending
	// on how "old" this marker is. We want the highlighting marker to shrink close to
	// the center as the marker gets older, and we also want the shrinking to be faster
	// when the marker is large, and slower as it gets to the center, hence the (a*a)
	// instead of just (a).
	const float center_offset_x = ((TILESIZE_X / 2) * (a*a));
	const float center_offset_y = ((TILESIZE_X / 2) * (a*a));

	//we want the marker to start a bit above ground and move itself closer to the
	// ground as it gets older.
	z += a*0.3f;
	
	// place x,y in the center of the highlighting tile
	x += (TILESIZE_X / 2);
	y += (TILESIZE_Y / 2);
	
	switch (marker->type) {
		case HIGHLIGHT_TYPE_WALKING_DESTINATION:
			glColor4f(0.0f, 1.0f, 0.0f, a);
			break;
		case HIGHLIGHT_TYPE_SPELL_TARGET:
			glColor4f(0.0f, 0.0f, 1.0f, a);
			break;
		case HIGHLIGHT_TYPE_ATTACK_TARGET:
			glColor4f(1.0f, 0.0f, 0.0f, a);
			break;
		case HIGHLIGHT_TYPE_LOCK:
			glColor4f(1.0f, 1.0f, 0.0f, a);
			break;
	}
	
	glBegin(GL_POLYGON);
	glVertex3f(x - 2*dx - center_offset_x, y - 2*dy - center_offset_y, z);
	glVertex3f(x - 1*dx - center_offset_x, y - 2*dy - center_offset_y, z);
	glVertex3f(x - 0*dx - center_offset_x, y - 0*dy - center_offset_y, z);
	glVertex3f(x - 2*dx - center_offset_x, y - 1*dy - center_offset_y, z);
	glVertex3f(x - 2*dx - center_offset_x, y - 2*dy - center_offset_y, z);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x + 2*dx + center_offset_x, y - 2*dy - center_offset_y, z);
	glVertex3f(x + 1*dx + center_offset_x, y - 2*dy - center_offset_y, z);
	glVertex3f(x + 0*dx + center_offset_x, y - 0*dy - center_offset_y, z);
	glVertex3f(x + 2*dx + center_offset_x, y - 1*dy - center_offset_y, z);
	glVertex3f(x + 2*dx + center_offset_x, y - 2*dy - center_offset_y, z);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x + 2*dx + center_offset_x, y + 2*dy + center_offset_y, z);
	glVertex3f(x + 1*dx + center_offset_x, y + 2*dy + center_offset_y, z);
	glVertex3f(x + 0*dx + center_offset_x, y + 0*dy + center_offset_y, z);
	glVertex3f(x + 2*dx + center_offset_x, y + 1*dy + center_offset_y, z);
	glVertex3f(x + 2*dx + center_offset_x, y + 2*dy + center_offset_y, z);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(x - 2*dx - center_offset_x, y + 2*dy + center_offset_y, z);
	glVertex3f(x - 1*dx - center_offset_x, y + 2*dy + center_offset_y, z);
	glVertex3f(x - 0*dx - center_offset_x, y + 0*dy + center_offset_y, z);
	glVertex3f(x - 2*dx - center_offset_x, y + 1*dy + center_offset_y, z);
	glVertex3f(x - 2*dx - center_offset_x, y + 2*dy + center_offset_y, z);
	glEnd();
}

void display_highlight_markers() {
	int i;
	if (!highlighting_enabled) return;
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_ALPHA_TEST);

	for(i = 0; i < NUMBER_OF_HIGHLIGHT_MARKERS; i++) {
		if (markers[i].active) {
			markers[i].timeleft -= (cur_time - last_time);
			if (markers[i].timeleft > 0) {
				display_highlight_marker(&markers[i]);
			} else {
				// This marker has lived long enough now.
				markers[i].active = 0;
			}
		}
	}

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

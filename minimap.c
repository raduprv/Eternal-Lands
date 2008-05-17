
#include <stdlib.h>
#include <string.h>
#include "minimap.h"
#include "asc.h"
#include "buddy.h"
#include "colors.h"
#include "elwindows.h"
#include "errors.h"
#include "framebuffer.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "init.h"
#include "interface.h"
#include "mines.h"
#include "misc.h"
#include "spells.h"
#include "textures.h"
#include "tiles.h"
#include "pathfinder.h"
#include "translate.h"
#include "io/map_io.h"
#include "io/elpathwrapper.h"
#include "io/elfilewrapper.h"

#ifndef MINIMAP2

static const int minimap_size = 256;
static const float float_minimap_size = 256.0;
static const int view_range = 30;


GLuint minimap_texture = 0;
GLuint circle_texture = 0;
GLuint exploration_texture = 0;
GLuint minimap_fbo = 0;
GLuint minimap_fbo_depth_buffer = 0;
GLuint minimap_fbo_texture = 0;
int redraw_fbo = 0;
int minimap_win = -1;
int minimap_win_x = 5;
int minimap_win_y = 20;
int minimap_flags = 1<<2;
int minimap_zoom = 0;		//zoom in, from 0 being only the visible area, 5 being full on largest maps
GLubyte exploration_map[256][256];
char current_exploration_map_filename[256];

static int max_zoom = 0;
/*
 * TODO:
 *
 *  -draw arrow showing players direction?
 *  -update description of the draw function below
 *
 * POSSIBLE OPTIMIZATION:
 *  -the window's content only changes when actors are updated or map is changed.
 *   maybe draw on another buffer so the window dont have to be redrawn every frame?
 */

int minimap_get_pin(void)
{
	return minimap_flags & 1<<0;
}

static __inline__ int minimap_get_above(void)
{
	return minimap_flags & 1<<1;
}

static __inline__ int minimap_get_FOW(void){
	//Note: Since this should default to on, we use an inverted bit. Since no-where else
	//will ever care what the bit is set to (it will only be bitwise toggled or checked
	//through through this function) this is currently safe
	return minimap_flags & 1<<2;
}

static __inline__ float minimap_get_zoom ()
{
	float mul = 1.0;
	int zoom;
	for (zoom = 0; zoom < max_zoom - minimap_zoom; zoom++)
		mul += mul;
	return 1.0 / mul;
}

static __inline__ void full_zoom(void)
{
	minimap_zoom = 0;
	minimap_touch();
}

static __inline__ void increase_zoom(void)
{
	if(minimap_zoom > max_zoom)minimap_zoom = max_zoom - 1;
	else if(minimap_zoom > 0)--minimap_zoom;
	minimap_touch();
}

static __inline__ void decrease_zoom(void){
	if(minimap_zoom < max_zoom)++minimap_zoom;
	minimap_touch();
}

static __inline__ void no_zoom(void)
{
	if(minimap_zoom < max_zoom)minimap_zoom = max_zoom;
	minimap_touch();
}

void minimap_touch(void)
{
	redraw_fbo = 1;
}

void minimap_free_framebuffer()
{
	free_color_framebuffer(&minimap_fbo, &minimap_fbo_depth_buffer, NULL,
		&minimap_fbo_texture);
}

void minimap_make_framebuffer()
{
	minimap_free_framebuffer();
	make_color_framebuffer(minimap_size, minimap_size, &minimap_fbo, &minimap_fbo_depth_buffer, NULL,
		&minimap_fbo_texture);
	minimap_touch();
}

void minimap_display_sidebar();

static __inline__ int draw_framebuffer()
{
	if(use_frame_buffer && !redraw_fbo && minimap_fbo > 0 && minimap_fbo_texture > 0)
	{
		bind_texture_id(minimap_fbo_texture);
		glBegin(GL_QUADS);
			glTexCoord2f (0.0f, 1.0f);
			glVertex2f (0.0f, 0.0f);
			glTexCoord2f (0.0f, 0.0f);
			glVertex2f (0.0f, float_minimap_size);
			glTexCoord2f (1.0f, 0.0f);
			glVertex2f (float_minimap_size, float_minimap_size);
			glTexCoord2f (1.0f, 1.0f);
			glVertex2f (float_minimap_size, 0.0f);
		glEnd();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
		return 1;
	}
	return 0;
}

static __inline__ float minimap_translate(float value, float p, float zoom_multip)
{
	return (value - p) / zoom_multip + 0.5*float_minimap_size;
}

static __inline__ void draw_actor_points(float zoom_multip, float px, float py)
{
	float size_x = float_minimap_size / (tile_map_size_x * 6);
	float size_y = float_minimap_size / (tile_map_size_y * 6);
	actor *a;
	int i;
	float x, y;

	glDisable(GL_TEXTURE_2D);

	//display the actors
	glPointSize(max2f(3.0f,1.2f * size_x / zoom_multip));
	glBegin(GL_POINTS);

	for (i = 0; i < max_actors; i++)
	{
		if (actors_list[i])
		{
			a = actors_list[i];
			x = a->x_tile_pos * size_x;
			y = float_minimap_size - (a->y_tile_pos * size_y);
			if (minimap_zoom < max_zoom)
			{
				//adjustments to the other actor positions for zoom
				x = minimap_translate(x, px, zoom_multip);
				y = minimap_translate(y, py, zoom_multip);
			}
			if (a->kind_of_actor == NPC)
			{
				glColor3f(0.0f,0.0f,1.0f); //blue NPCs
			}
			else if(a->actor_id == yourself)
			{
				glColor3f(0.0f,1.0f,0.0f); //green yourself
			}
			else if(a->is_enhanced_model && (a->kind_of_actor ==  PKABLE_HUMAN || a->kind_of_actor == PKABLE_COMPUTER_CONTROLLED))
			{
				glColor3f(1.0f,0.0f,0.0f); //red PKable
			}
			else if(a->is_enhanced_model && is_in_buddylist(a->actor_name))
			{
				glColor3f(0.0f,0.9f,1.0f); //aqua buddy
			}
			else if (is_color ((unsigned char)a->actor_name[0]))
			{
				if(a->is_enhanced_model && is_in_buddylist(a->actor_name))
				{
					glColor3f(0.0f,0.9f,1.0f); // aqua buddy
				}
				else
				{	// Use the colour of their name. This gives purple bots, green demigods, etc.
					int color = from_color_char (a->actor_name[0]);
					glColor3ub (colors_list[color].r1,
						colors_list[color].g1,
						colors_list[color].b1);
				}
			}
			else if(!a->is_enhanced_model)
			{
				if (a->dead) 
				{
					glColor3f(0.8f, 0.8f, 0.0f); // dark yellow: dead animal/monster
				}
				else // alive
				{
					glColor3f(1.0f, 1.0f, 0.0f); // yellow animal/monster
				}
			}
			else
			{
				glColor3f(1.0f, 1.0f, 1.0f); // white other player
			}
			// Draw it!
			glVertex2f(x, y);
		}
	}
	
	// mines
	for (i = 0; i < NUM_MINES; i++)
	{
		mine *m = &mine_list[i];
		if (m->obj_3d_id != -1)
		{
			x = m->x * size_x;
			y = float_minimap_size - (m->y * size_y);
			if (minimap_zoom < max_zoom)
			{
				// adjustments to the other mine positions for zoom
				x = minimap_translate(x, px, zoom_multip);
				y = minimap_translate(y, py, zoom_multip);
			}
			glColor3f(0.15f, 0.65f, 0.45f); 
			glVertex2f(x, y);
		}
	}

	glEnd();//GL_POINTS

	if (pf_follow_path)
	{
		x = pf_dst_tile->x * size_x;
		y = float_minimap_size - (pf_dst_tile->y * size_y);

		if (x != px || y != py)
		{
			if (minimap_zoom < max_zoom)
			{
				//adjustments to the other actor positions for zoom
				x = minimap_translate(x, px, zoom_multip);
				y = minimap_translate(y, py, zoom_multip);
			}
			glBegin(GL_LINES);
			glColor3f(1.0f,0.0f,0.0f); //red
			glVertex2i(x-5, y-5);
			glVertex2i(x+5, y+5);
			glVertex2i(x-5, y+5);
			glVertex2i(x+5, y-5);
			glEnd();//GL_LINES
		}
	}

	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glEnable(GL_TEXTURE_2D);
}

static __inline__ void draw_map(float zoom_multip, float px, float py)
{
	float sx = 0.0f, sy = 0.0f;

	glPushMatrix();

	sx = 128.0f;
	sy = 128.0f;

	if (minimap_zoom < max_zoom)
	{
		//adjustments to the other actor positions for zoom
		sx = minimap_translate(sx, px, zoom_multip);
		sy = minimap_translate(sy, py, zoom_multip);
	}

	glTranslatef(sx, sy, 0.0f);
	glScalef(1.0f / zoom_multip, 1.0f / zoom_multip, 1.0f);

	if(minimap_get_FOW())
	{
		//draw exploration map here.
		glEnable(GL_TEXTURE_2D);
		bind_texture_id(exploration_texture);
		glColor4f(0.25f, 0.25f, 0.25f, 0.0f);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(-128.0f, +128.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(+128.0f, +128.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(+128.0f, -128.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(-128.0f, -128.0f);
		glEnd();

		//white circle around player
		bind_texture_id(circle_texture);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);


		glPopMatrix();
		glPushMatrix();

		if (minimap_zoom < max_zoom)
		{
			glTranslatef(128.0f, 128.0f, 0.0f);
		}
		else
		{
			glTranslatef(px, py, 0.0f);
		}
		glScalef(0.25f / zoom_multip, 0.25f / zoom_multip, 1.0f);

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(-128.0f, +128.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(+128.0f, +128.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(+128.0f, -128.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(-128.0f, -128.0f);
		glEnd();
		glPopMatrix();
		glPushMatrix();
		glTranslatef(sx, sy, 0.0f);
		glScalef(1.0 / zoom_multip, 1.0 / zoom_multip, 1.0f);

		glBlendFunc(GL_DST_COLOR, GL_ZERO);
	}

	//draw the minimap
	bind_texture_id(minimap_texture);
	glColor4f(1.0f,1.0f,1.0f,1.0f);

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(-128.0f, +128.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(+128.0f, +128.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(+128.0f, -128.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(-128.0f, -128.0f);
	glEnd();

	glPopMatrix();

	if (minimap_get_FOW())
	{
		glDisable(GL_BLEND);
	}
}

static __inline__ void draw_into_minimap_fbo(float zoom_multip, float px, float py)
{
	GLint view_port[4];

	//Okay, we don't have an updated one yet, so draw a new one
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	glGetIntegerv(GL_VIEWPORT, view_port);
	CHECK_GL_ERRORS();
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, minimap_fbo);
	glViewport(0, 0, minimap_size, minimap_size);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	redraw_fbo = 0;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, (GLdouble)minimap_size, (GLdouble)minimap_size, 0.0, -250.0, 250.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if (minimap_zoom < max_zoom)
	{
		float min_x, min_y, max_x, max_y;

		//adjustments to the other actor positions for zoom
		min_x = minimap_translate(0.0f, px, zoom_multip);
		min_y = minimap_translate(0.0f, py, zoom_multip);
		max_x = minimap_translate(float_minimap_size, px, zoom_multip);
		max_y = minimap_translate(float_minimap_size, py, zoom_multip);

		min_x = clampf(min_x, 0.0f, float_minimap_size);
		min_y = clampf(min_y, 0.0f, float_minimap_size);
		max_x = clampf(max_x, 0.0f, float_minimap_size);
		max_y = clampf(max_y, 0.0f, float_minimap_size);

		glEnable(GL_SCISSOR_TEST);
		//clip the drawable region to the map
		glScissor(min_x+0.5, 0.5 + (float_minimap_size - max_y), 0.5+(max_x - min_x), 0.5 + (max_y - min_y));
	}

	draw_map(zoom_multip, px, py);
	draw_actor_points(zoom_multip, px, py);

	if (minimap_zoom < max_zoom)
	{
		glDisable(GL_SCISSOR_TEST);
	}

	bind_texture_id(0);
	//If we rendered to a framebuffer, draw it
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glViewport(view_port[0], view_port[1], view_port[2], view_port[3]);
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}

/* 
 * draw minimap
 *   
 * It first fills the window with grey, that will be fog area, then it draws white
 * circles on player and guildmates positions, that will be visible areas, then it
 * draws black on unexplored areas. then the minimap is drawn with blending function
 * GL_DST_COLOR, GL_ZERO. then draw boxes where the actors are.
 */
int display_minimap_handler(window_info *win)
{
	float zoom_multip;
	float size_x = float_minimap_size / (tile_map_size_x * 6);
	float size_y = float_minimap_size / (tile_map_size_y * 6);
	float view_distance, px = 0.0f, py = 0.0f;
	actor *me;

	zoom_multip = minimap_get_zoom();

	if(!minimap_texture) 
	{
		//there's no minimap for this map :( draw a X
		glDisable(GL_TEXTURE_2D);
		glColor3f(1.0f, 0.0f, 0.0f);
		glLineWidth(3.0f);
		glBegin(GL_LINES);
		glVertex3i(30, 30, 1);
		glVertex3i(226, 226, 1);
		glVertex3i(226, 30, 1);
		glVertex3i(30, 226, 1);
		glEnd();
		glLineWidth(1.0f); 
		glEnable(GL_TEXTURE_2D);
		glColor3f(0.8f,0.8f,0.8f);
		draw_string_small(14,125,(unsigned char*)"no minimap for this place",1);
		return 0;
	}
	//draw minimap

	//get player position in window coordinates
	if( (me = get_our_actor ()) == NULL)
	{
		//Don't know who we are? can't draw then
		return 0;
	}
	px = me->x_tile_pos * size_x;
	py = float_minimap_size - (me->y_tile_pos * size_y);
	//Do we already have a valid framebuffer texture? If so, we'll use that

	if (draw_framebuffer())
	{
		minimap_display_sidebar(win);
		return 0;
	}

	if (use_frame_buffer)
	{
		redraw_fbo = 0;

		draw_into_minimap_fbo(zoom_multip, px, py);
		draw_framebuffer();

		minimap_display_sidebar(win);
		return 0;
	}

	view_distance = size_x * view_range * zoom_multip;

	if (minimap_zoom < max_zoom)
	{
		float min_x, min_y, max_x, max_y;

		//adjustments to the other actor positions for zoom
		min_x = minimap_translate (0.0f, px, zoom_multip);
		min_y = minimap_translate (0.0f, py, zoom_multip);
		max_x = minimap_translate (float_minimap_size, px, zoom_multip);
		max_y = minimap_translate (float_minimap_size, py, zoom_multip);

		min_x = clampf (min_x, 0.0f, float_minimap_size);
		min_y = clampf (min_y, 0.0f, float_minimap_size);
		max_x = clampf (max_x, 0.0f, float_minimap_size);
		max_y = clampf (max_y, 0.0f, float_minimap_size);

		//draw a black background for the window
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glBegin(GL_QUADS);
			glVertex2f (0.0f, 0.0f);
			glVertex2f (0.0f, float_minimap_size);
			glVertex2f (float_minimap_size, float_minimap_size);
			glVertex2f (float_minimap_size, 0.0f);
		glEnd();

		glEnable(GL_SCISSOR_TEST);
		//clip the drawable region to the map
		glScissor(win->cur_x + (int)(min_x+0.5f), window_height - (win->cur_y + (int)max_y), max_x - min_x, max_y - min_y);
	}
	else
	{
		//lets not draw outside the window :)
		//maybe put this in elwindows.c around the call to display handler?
		glEnable(GL_SCISSOR_TEST);
		glScissor(win->cur_x, window_height - win->cur_y - minimap_size, minimap_size, minimap_size);
	}

	draw_map(zoom_multip, px, py);
	draw_actor_points(zoom_multip, px, py);

	glDisable(GL_SCISSOR_TEST);

	minimap_display_sidebar(win);

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 0;
}


void minimap_display_sidebar(window_info *win){
	glDisable(GL_TEXTURE_2D);
	glColor3f(win->border_color[0],win->border_color[1],win->border_color[2]);

	// zoom button boxes
	glBegin(GL_LINE_STRIP);
		glVertex2i(win->len_x, ELW_BOX_SIZE*2);
		glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*2);
		glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*6);
		glVertex2i(win->len_x, ELW_BOX_SIZE*6);
	glEnd();
	glBegin(GL_LINES);
		glVertex2i(win->len_x, ELW_BOX_SIZE*3);
		glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*3);
		glVertex2i(win->len_x, ELW_BOX_SIZE*4);
		glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*4);
		glVertex2i(win->len_x, ELW_BOX_SIZE*5);
		glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*5);
	glEnd();

	//zoom right in button
	glBegin(GL_LINE_STRIP);
		glVertex2i(win->len_x-3, ELW_BOX_SIZE*2.5-1);
		glVertex2i(win->len_x-ELW_BOX_SIZE/2+1, ELW_BOX_SIZE*2.5-1);
		glVertex2i(win->len_x-ELW_BOX_SIZE/2+1, ELW_BOX_SIZE*2+3);
		glVertex2i(win->len_x-ELW_BOX_SIZE/2-1, ELW_BOX_SIZE*2+3);
		glVertex2i(win->len_x-ELW_BOX_SIZE/2-1, ELW_BOX_SIZE*2.5-1);
		glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*2.5-1);
		glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*2.5+1);
		glVertex2i(win->len_x-ELW_BOX_SIZE/2-1, ELW_BOX_SIZE*2.5+1);
		glVertex2i(win->len_x-ELW_BOX_SIZE/2-1, ELW_BOX_SIZE*3-3);
		glVertex2i(win->len_x-ELW_BOX_SIZE/2+1, ELW_BOX_SIZE*3-3);
		glVertex2i(win->len_x-ELW_BOX_SIZE/2+1, ELW_BOX_SIZE*2.5+1);
		glVertex2i(win->len_x-3, ELW_BOX_SIZE*2.5+1);
		glVertex2i(win->len_x-3, ELW_BOX_SIZE*2.5-1);
	glEnd();

	//plus/minus buttons
	glLineWidth(2.0f);
	glBegin(GL_LINES);
		glVertex2i(win->len_x-ELW_BOX_SIZE/2, ELW_BOX_SIZE*3+3);
		glVertex2i(win->len_x-ELW_BOX_SIZE/2, ELW_BOX_SIZE*4-3);
		glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*3.5);
		glVertex2i(win->len_x-3, ELW_BOX_SIZE*3.5);
		glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*4.5);
		glVertex2i(win->len_x-3, ELW_BOX_SIZE*4.5);
	glEnd();
	glLineWidth(1.0f);

	//zoom right out button
	glBegin(GL_LINE_STRIP);
		glVertex2i(win->len_x-3, ELW_BOX_SIZE*5.5-1);
		glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*5.5-1);
		glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*5.5+1);
		glVertex2i(win->len_x-3, ELW_BOX_SIZE*5.5+1);
		glVertex2i(win->len_x-3, ELW_BOX_SIZE*5.5-1);
	glEnd();

	//pinned button box
	glBegin(GL_LINE_STRIP);
		glVertex2i(win->len_x, ELW_BOX_SIZE*7);
		glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*7);
		glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*8);
		glVertex2i(win->len_x, ELW_BOX_SIZE*8);
	glEnd();

	//pinned button
	glBegin(GL_TRIANGLES);
	if(minimap_get_pin()){
		glVertex2i(win->len_x-ELW_BOX_SIZE/2, ELW_BOX_SIZE*8-5);
		glVertex2i(win->len_x-ELW_BOX_SIZE+5, ELW_BOX_SIZE*7+5);
		glVertex2i(win->len_x-5, ELW_BOX_SIZE*7+5);
	} else {
		glVertex2i(win->len_x-ELW_BOX_SIZE/2, ELW_BOX_SIZE*7+5);
		glVertex2i(win->len_x-ELW_BOX_SIZE+5, ELW_BOX_SIZE*8-5);
		glVertex2i(win->len_x-5, ELW_BOX_SIZE*8-5);
	}
	glEnd();

	//above button box
	glBegin(GL_LINE_STRIP);
		glVertex2i(win->len_x, ELW_BOX_SIZE*9);
		glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*9);
		glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*10);
		glVertex2i(win->len_x, ELW_BOX_SIZE*10);
	glEnd();

	//above button
	glBegin(GL_LINE_LOOP);
		glVertex2i(win->len_x-3, ELW_BOX_SIZE*9+8);
		glVertex2i(win->len_x-ELW_BOX_SIZE+7, ELW_BOX_SIZE*9+8);
		glVertex2i(win->len_x-ELW_BOX_SIZE+7, ELW_BOX_SIZE*10-4);
		glVertex2i(win->len_x-3, ELW_BOX_SIZE*10-4);
	glEnd();
	if(minimap_get_above()){
		glBegin(GL_QUADS);
	} else {
		glBegin(GL_LINE_LOOP);
	}
		glVertex2i(win->len_x-7, ELW_BOX_SIZE*9+4);
		glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*9+4);
		glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*10-8);
		glVertex2i(win->len_x-7, ELW_BOX_SIZE*10-8);
	glEnd();

	//FOW button box
	glBegin(GL_LINE_STRIP);
		glVertex2i(win->len_x, ELW_BOX_SIZE*11);
		glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*11);
		glVertex2i(win->len_x-ELW_BOX_SIZE, ELW_BOX_SIZE*12);
		glVertex2i(win->len_x, ELW_BOX_SIZE*12);
	glEnd();
	if(minimap_get_FOW()){
		glBegin(GL_LINE_STRIP);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*11+3);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*11+3);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*12-3);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*12-3);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*11+3);
		glEnd();
		//draw square
	} else {
		glBegin(GL_QUADS);
			//draw solid square with hole. Done with 4 trapezoids.
			//top:
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*11+3);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*11+3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2-3, ELW_BOX_SIZE*11.5-3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2+3, ELW_BOX_SIZE*11.5-3);
			//bottom:
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*12-3);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*12-3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2-3, ELW_BOX_SIZE*11.5+3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2+3, ELW_BOX_SIZE*11.5+3);
			//right:
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*11+3);
			glVertex2i(win->len_x-3, ELW_BOX_SIZE*12-3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2+3, ELW_BOX_SIZE*11.5+3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2+3, ELW_BOX_SIZE*11.5-3);
			//left:
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*11+3);
			glVertex2i(win->len_x-ELW_BOX_SIZE+3, ELW_BOX_SIZE*12-3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2-3, ELW_BOX_SIZE*11.5+3);
			glVertex2i(win->len_x-ELW_BOX_SIZE/2-3, ELW_BOX_SIZE*11.5-3);
		glEnd();
	}
	glLineWidth(1.0f);
	glEnable(GL_TEXTURE_2D);
}

/*
 * Handler for clicks into minimap. Coordinates are given as window pixels with origin at bottom-left corner
 */   
static int minimap_walkto(int mx, int my){
	float size_x = tile_map_size_x * 6;
	float size_y = tile_map_size_y * 6;
	float fmx = mx, fmy = my;
	int dx, dy;

	/* Safety check. Avoid division by zero */
	if (float_minimap_size == 0)
		return 0;

	/* Special handling of zoomed states. The map is centered around the actor */
	if (minimap_zoom < max_zoom)
	{
		float px, py;
		float zoom_multip;
		actor *me;

		/* Get zoom and actor position (tile coordinates) */
		zoom_multip = minimap_get_zoom();
		if ( (me = get_our_actor ()) == NULL)
			return 0;

		/* Convert tile coordinates to minimap pixels */
		px = (me->x_tile_pos / size_x) * float_minimap_size;
		py = (me->y_tile_pos / size_y) * float_minimap_size;

		/* Convert window coordinates to minimap pixels and center around actor */
		fmx = (fmx - 0.5 * float_minimap_size) * zoom_multip + px;
		fmy = (fmy - 0.5 * float_minimap_size) * zoom_multip + py;
	}

	/* Convert minimap coordinates to tile coordinates */
	dx = 0.5 + (fmx / float_minimap_size) * size_x;
	dy = 0.5 + (fmy / float_minimap_size) * size_y;

	/* Do path finding */
	if (pf_find_path(dx, dy))
	{
		minimap_touch();
		return 1;
	}

	return 0;
}

int click_minimap_handler(window_info * win, int mx, int my, Uint32 flags){
	for(;pow(2,4+max_zoom) <= tile_map_size_x;++max_zoom);
	if((flags & ELW_WHEEL) && my < win->len_y && mx > 0 && mx < win->len_x-ELW_BOX_SIZE){
		if(flags & ELW_WHEEL_UP){
			increase_zoom();
		} else {
			decrease_zoom();
		}
		return 1;
	}
	if(!flags & ELW_LEFT_MOUSE){
		return 0;
	} else if (my < win->len_y && mx > 0 && mx < win->len_x-ELW_BOX_SIZE) {
		return minimap_walkto(mx, win->len_y - my);
	} else if(mx < win->len_x-ELW_BOX_SIZE || mx > win->len_x){
		return 0;
	} else if(my >= ELW_BOX_SIZE*2 && my < ELW_BOX_SIZE*3){
		full_zoom();
		return 1;
	} else if(my >= ELW_BOX_SIZE*3 && my < ELW_BOX_SIZE*4){
		increase_zoom();
		return 1;
	} else if(my >= ELW_BOX_SIZE*4 && my < ELW_BOX_SIZE*5){
		decrease_zoom();
		return 1;
	} else if(my >= ELW_BOX_SIZE*5 && my < ELW_BOX_SIZE*6){
		no_zoom();
		return 1;
	} else if(my >= ELW_BOX_SIZE*7 && my < ELW_BOX_SIZE*8){
		//Pin
		minimap_flags ^= 1<<0;	//bitwise toggle
		return 1;
	} else if(my >= ELW_BOX_SIZE*9 && my < ELW_BOX_SIZE*10){
		//Above others
		minimap_flags ^= 1<<1;	//bitwise toggle
		//TODO: ability to set window above others. 
		return 1;
	} else if(my >= ELW_BOX_SIZE*11 && my < ELW_BOX_SIZE*12){
		//fog of war
		minimap_flags ^= 1<<2;	//bitwise toggle
		minimap_touch();
		return 1;
	}
	return 0;
}


int mouseover_minimap_handler(window_info * win, int mx, int my, Uint32 flags){
	if(mx < win->len_x-ELW_BOX_SIZE || mx > win->len_x){
	} else if(my >= ELW_BOX_SIZE*2 && my < ELW_BOX_SIZE*3){
		show_help("Zoom completely in", mx+12, my+10);
	} else if(my >= ELW_BOX_SIZE*3 && my < ELW_BOX_SIZE*4){
		show_help("Zoom in", mx+12, my+10);
	} else if(my >= ELW_BOX_SIZE*4 && my < ELW_BOX_SIZE*5){
		show_help("Zoom out", mx+12, my+10);
	} else if(my >= ELW_BOX_SIZE*5 && my < ELW_BOX_SIZE*6){
		show_help("Zoom completely out", mx+12, my+10);
	} else if(my >= ELW_BOX_SIZE*7 && my < ELW_BOX_SIZE*8){
		if(minimap_get_pin()){
			show_help("Un-pin window", mx+12, my+10);
		} else {
			show_help("Pin window (ignores alt+d)", mx+12, my+10);
		}
	} else if(my >= ELW_BOX_SIZE*9 && my < ELW_BOX_SIZE*10){
		if(minimap_get_above()){
			show_help("Un-above window", mx+12, my+10);
		} else {
			show_help("Display above other windows", mx+12, my+10);
		}
	} else if(my >= ELW_BOX_SIZE*11 && my < ELW_BOX_SIZE*12){
		if(minimap_get_FOW()){
			show_help("Disable Fog Of War", mx+12, my+10);
		} else {
			show_help("Enable Fog Of War", mx+12, my+10);
		}
	}
	return 0;
}


void update_exploration_map()
{
	float size_x = tile_map_size_x * 6;
	float size_y = tile_map_size_y * 6;
	int px = 0, py = 0, i = 0, j = 0, view_distance = 0, d = 0, vd_square = 0;
	int explored = 0;
	GLubyte c;
	actor *me;
	
	if(!minimap_texture || minimap_win < 0)
		return;
	
	//get player position in window coordinates
	if ( (me = get_our_actor ()) == NULL)
	{
		return;
	}
	px = (me->x_tile_pos / size_x) * float_minimap_size;
	py = (me->y_tile_pos / size_y) * float_minimap_size;
	
	view_distance = (float_minimap_size / size_x) * 0.9 * view_range;
	vd_square = view_distance*view_distance;

	for(i = -view_distance; i < view_distance; i++)
		for(j = -view_distance; j < view_distance; j++)
			if(0 < (px + i) && (px + i) < minimap_size && 0 < (py + j) && (py + j) < minimap_size)
			{
				d = i*i + j*j;
				if(d <= vd_square)
					if(exploration_map[px + i][py + j] < (GLubyte)0xFF)
					{
						if(d > (vd_square / 4)) //fade out exploration map from half to full radius
						{
							c = (GLubyte)0xFF - (GLubyte)0xFF * (float)((float)(d - (vd_square / 4)) / (float)vd_square);
							if (exploration_map[px + i][py + j] < c)
								exploration_map[px + i][py + j] = c;
						}
						else
							exploration_map[px + i][py + j] = (GLubyte)0xFF;
						explored = 1;
					}
			}
	
	if (explored)
	{
		glBindTexture(GL_TEXTURE_2D, exploration_texture);	//failsafe
		bind_texture_id(exploration_texture);

		if(have_extension(arb_texture_compression))
		{
			if(have_extension(ext_texture_compression_s3tc))
				glTexImage2D (GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, minimap_size, minimap_size, 0, GL_LUMINANCE,GL_UNSIGNED_BYTE, &exploration_map);
			else
				glTexImage2D (GL_TEXTURE_2D, 0, GL_COMPRESSED_LUMINANCE_ARB, minimap_size, minimap_size, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &exploration_map);
		}
		else
			glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE, minimap_size, minimap_size, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &exploration_map);
		
		CHECK_GL_ERRORS();
	}
}


void load_exploration_map ()
{
	FILE *fp = NULL;
	char exploration_map_filename[256];

	if(!minimap_texture)
		return;

	my_strcp (exploration_map_filename, map_file_name);
	exploration_map_filename[strlen(exploration_map_filename)-4] = 0;
	strcat (exploration_map_filename, ".xm");
	safe_strncpy (current_exploration_map_filename, exploration_map_filename, sizeof (current_exploration_map_filename));
	fp = open_file_config (exploration_map_filename, "rb");
	if(fp)
	{
		fread(exploration_map, sizeof(GLubyte), 256 * 256, fp);
		fclose(fp);
	}
	else
	{
		memset(exploration_map, 0, 256 * 256 * sizeof(GLubyte));
	}
	
	if(poor_man)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if(use_mipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	if(have_extension(arb_texture_compression))
	{
		if(have_extension(ext_texture_compression_s3tc))
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 256, 256,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,&exploration_map);
		else
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_LUMINANCE, 256, 256,0,GL_ALPHA,GL_UNSIGNED_BYTE,&exploration_map);
		
	}
	else
		glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE, 256, 256,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,&exploration_map);
	
	update_exploration_map();

	CHECK_GL_ERRORS();	
}

void save_exploration_map()
{
	FILE *fp = NULL;
	
	if(!minimap_texture)
		return;
	
	fp = open_file_config (current_exploration_map_filename, "wb");
	if (fp)
	{
		fwrite(exploration_map, sizeof(GLubyte), 256 * 256, fp);
		fclose(fp);
	}
	else
	{
		//log error and quit
	}	
}

void change_minimap(){
	char minimap_file_name[256];
	int size;
	int zoom_diff = max_zoom - minimap_zoom;
	texture_cache_struct tex;

	if(minimap_win < 0)
		return;
	save_exploration_map();

	//unload all textures
	if(minimap_texture)
		glDeleteTextures(1,&minimap_texture);
	if(circle_texture)
		glDeleteTextures(1,&circle_texture);
	if(exploration_texture)
		glDeleteTextures(1,&exploration_texture);
	if(use_frame_buffer)
		minimap_free_framebuffer();

	//make filename
	my_strcp(minimap_file_name,map_file_name);
	minimap_file_name[strlen(minimap_file_name)-4] = '\0';
	strcat(minimap_file_name, ".bmp");

	//load textures
	my_strcp(tex.file_name, minimap_file_name);
	if (!el_file_exists(minimap_file_name))
		minimap_texture = 0;
	else
		minimap_texture = load_bmp8_fixed_alpha(&tex,128);
	my_strcp(tex.file_name, "./textures/circle.bmp");
	circle_texture = load_bmp8_fixed_alpha(&tex,0);
	glGenTextures(1, &exploration_texture);
	bind_texture_id(exploration_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	load_exploration_map();

	if(use_frame_buffer)
		minimap_make_framebuffer();

	max_zoom = 0;
	for (size = 16; size <= tile_map_size_x; size += size)
		max_zoom++;
	// keep same ratio to max zoom if possible
	minimap_zoom = max_zoom - zoom_diff;
	if (minimap_zoom < 0)
		minimap_zoom = 0;
	else if (minimap_zoom > max_zoom)
		minimap_zoom = max_zoom;
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


void display_minimap()
{
	if(minimap_win < 0)
	{
		//init minimap
		minimap_win = create_window("Minimap", windows_on_top?-1:game_root_win, 0, minimap_win_x, minimap_win_y, 256+ELW_BOX_SIZE, 256+1, ELW_WIN_DEFAULT);
		set_window_handler(minimap_win, ELW_HANDLER_DISPLAY, &display_minimap_handler);	
		set_window_handler(minimap_win, ELW_HANDLER_CLICK, &click_minimap_handler);	
		set_window_handler(minimap_win, ELW_HANDLER_MOUSEOVER, &mouseover_minimap_handler);	

		change_minimap();
	} else {
		show_window(minimap_win);
		select_window(minimap_win);
	}
}

#else //MINIMAP2

float minimap_size_coefficient;
int minimap_size;
float float_minimap_size;
float minimap_tiles_distance = 48;
float radius_shift = 0.707106779283f;

static int compass_direction = 1;

GLuint compass_tex;
GLuint minimap_texture = 0;
GLuint exploration_texture = 0;
GLuint minimap_fbo = 0;
GLuint minimap_fbo_depth_buffer = 0;
GLuint minimap_fbo_texture = 0;
int minimap_win = -1;
int minimap_win_x = 5;
int minimap_win_y = 20;
GLubyte exploration_map[256][256];
char current_exploration_map_filename[256];

static __inline__ float minimap_get_zoom ()
{
	float zoom = minimap_tiles_distance * 2 / (tile_map_size_x*6);
	if(!use_frame_buffer)
	{
		zoom *= radius_shift;
	}
	return zoom;
}

void minimap_free_framebuffer()
{
	free_color_framebuffer(&minimap_fbo, &minimap_fbo_depth_buffer, NULL,
		&minimap_fbo_texture);
}

void minimap_make_framebuffer()
{
	minimap_free_framebuffer();
	make_color_framebuffer(minimap_size, minimap_size, &minimap_fbo, &minimap_fbo_depth_buffer, NULL,
		&minimap_fbo_texture);
}

static __inline__ int draw_framebuffer()
{
	float fsin,fcos,x,y;
	int i;
	if(use_frame_buffer && minimap_fbo > 0 && minimap_fbo_texture > 0)
	{
		bind_texture_id(minimap_fbo_texture);

		glTranslatef(0.0f, 16.0f, 0.0f);

		glBegin(GL_POLYGON);
		for (i=0; i<=360; i +=10) 
		{
			fsin = sin(i*0.0174532925f)/2 + 0.5f;
			fcos = cos((i + 180)*0.0174532925f)/2 + 0.5f;
			x = sin((i)*0.0174532925)/2*float_minimap_size+float_minimap_size/2;
			y = cos((i)*0.0174532925f)/2*float_minimap_size+float_minimap_size/2;
			glTexCoord2f(fsin, fcos);			
			glVertex2f(x, y);	
		}	
		glEnd();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
		return 1;
	}
	return 0;
}

__inline__ void rotate_actor_points(float zoom_multip, float px, float py)
{
	float x,y;
	x = (px - (float_minimap_size/2) ) + float_minimap_size/2;
	y = (py - (float_minimap_size/2) ) + float_minimap_size/2;

	glTranslatef(float_minimap_size/2, float_minimap_size/2, 0.0f);

	glRotatef(-compass_direction*rz, 0.0f,0.0f,1.0f );
	glTranslatef(-x,-y,0.0f);

	glScalef(1.0f / zoom_multip, 1.0f / zoom_multip, 1.0f);
	x = x * zoom_multip * ((1.0f / zoom_multip) - 1);
	y = y * zoom_multip * ((1.0f / zoom_multip) - 1);
	glTranslatef(-x,-y,0.0f);
}

__inline__ void rotate_at_player(float zoom_multip, float px, float py)
{
	float x,y;
	x = (px - (float_minimap_size/2) );
	y = (py - (float_minimap_size/2) );
	glRotatef(-compass_direction*rz, 0.0f,0.0f,1.0f );
	glTranslatef(-x,-y,0.0f);
	
	glScalef(1.0f / zoom_multip, 1.0f / zoom_multip, 1.0f);

	x = x * zoom_multip * ((1.0f / zoom_multip) - 1);
	y = y * zoom_multip * ((1.0f / zoom_multip) - 1);
	glTranslatef(-x,-y,0.0f);
}

__inline__ void rotate_click_coords(float * x,float * y)
{
	float fx,fy;
	float angel = -compass_direction*rz*0.0174532925;

	fx = cos(angel) * (*x - float_minimap_size/2) - sin(angel) * (*y - float_minimap_size/2);
	fy = sin(angel) * (*x - float_minimap_size/2) + cos(angel) * (*y - float_minimap_size/2);
	*x = fx + float_minimap_size/2;
	*y = fy + float_minimap_size/2;
}

__inline__ int is_within_radius(float mx, float my,float px,float py,float radius)
{
	float distance;

	distance = sqrt(pow(px - mx,2) + pow(py-my,2));
	if(distance <= radius)
		return 1;
	else 
		return 0;
}

static __inline__ void draw_actor_points(float zoom_multip, float px, float py)
{
	float size_x = float_minimap_size / (tile_map_size_x * 6);
	float size_y = float_minimap_size / (tile_map_size_y * 6);
	actor *a;
	int i;
	float x, y;

	glPushMatrix();
	glDisable(GL_TEXTURE_2D);

	//display the actors
	glPointSize(6);

	rotate_actor_points(zoom_multip,px,py);

	glBegin(GL_POINTS);

	for (i = 0; i < max_actors; i++)
	{
		if (actors_list[i])
		{
			a = actors_list[i];
			x = a->x_tile_pos * size_x;
			y = float_minimap_size - (a->y_tile_pos * size_y);

			if(!use_frame_buffer)
			{
				x = x - (x - px)/3;
				y = y - (y - py)/3;
			}

			if (a->kind_of_actor == NPC)
			{
				glColor3f(0.0f,0.0f,1.0f); //blue NPCs
			}
			else if(a->actor_id == yourself)
			{
				glColor3f(0.0f,1.0f,0.0f); //green yourself
			}
			else if(a->is_enhanced_model && (a->kind_of_actor ==  PKABLE_HUMAN || a->kind_of_actor == PKABLE_COMPUTER_CONTROLLED))
			{
				glColor3f(1.0f,0.0f,0.0f); //red PKable
			}
			else if(a->is_enhanced_model && is_in_buddylist(a->actor_name))
			{
				glColor3f(0.0f,0.9f,1.0f); //aqua buddy
			}
			else if (is_color ((unsigned char)a->actor_name[0]))
			{
				if(a->is_enhanced_model && is_in_buddylist(a->actor_name))
				{
					glColor3f(0.0f,0.9f,1.0f); // aqua buddy
				}
				else
				{	// Use the colour of their name. This gives purple bots, green demigods, etc.
					int color = from_color_char (a->actor_name[0]);
					glColor3ub (colors_list[color].r1,
						colors_list[color].g1,
						colors_list[color].b1);
				}
			}
			else if(!a->is_enhanced_model)
			{
				if (a->dead) 
				{
					glColor3f(0.8f, 0.8f, 0.0f); // dark yellow: dead animal/monster
				}
				else // alive
				{
					glColor3f(1.0f, 1.0f, 0.0f); // yellow animal/monster
				}
			}
			else
			{
				glColor3f(1.0f, 1.0f, 1.0f); // white other player
			}
			// Draw it!
			glVertex2f(x, y);
		}
	}
	
	// mines
	for (i = 0; i < NUM_MINES; i++)
	{
		mine *m = &mine_list[i];
		if (m->obj_3d_id != -1)
		{
			x = m->x * size_x;
			y = float_minimap_size - (m->y * size_y);

			if(!use_frame_buffer)
			{
				x = x - (x - px)/3;
				y = y - (y - py)/3;
			}

			glColor3f(0.15f, 0.65f, 0.45f); 
			glVertex2f(x, y);
		}
	}

	glEnd();//GL_POINTS

	glPopMatrix();
	
	if (pf_follow_path)
	{
		x = pf_dst_tile->x * size_x;
		y = float_minimap_size - (pf_dst_tile->y * size_y);

		if (x != px || y != py)
		{
			float diff = 8.0f*zoom_multip;



			if(!use_frame_buffer)
			{
				x = x - (x - px)/3;
				y = y - (y - py)/3;
			}

			//no point calling is_within_radius if using frame buffer
			if(use_frame_buffer || is_within_radius(x,y,px,py,minimap_tiles_distance*zoom_multip*2)) 
			{
				glPushMatrix();
				glDisable(GL_TEXTURE_2D);
				rotate_actor_points(zoom_multip,px,py);
				glBegin(GL_LINES);
				glColor3f(1.0f,0.0f,0.0f); //red
				glVertex2f(x-diff, y-diff);
				glVertex2f(x+diff, y+diff);
				glVertex2f(x-diff, y+diff);
				glVertex2f(x+diff, y-diff);
				glEnd();//GL_LINES
				glPopMatrix();
			}
		}
	}

	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glEnable(GL_TEXTURE_2D);
}

static __inline__ void draw_compass()
{
	glPushMatrix();
	glColor3f(1.0f,1.0f,1.0f);
	glTranslatef(float_minimap_size/2, float_minimap_size/2, 0.0f);
	glRotatef(-compass_direction*rz - 90, 0.0f,0.0f,1.0f );
	glRotatef(180, 1.0f,0.0f,0.0f );
	glEnable(GL_TEXTURE_2D); 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	bind_texture_id(compass_tex);

	glBegin(GL_QUADS); 
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(-float_minimap_size/2, float_minimap_size/2);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(float_minimap_size/2, float_minimap_size/2);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(float_minimap_size/2, -float_minimap_size/2);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(-float_minimap_size/2, -float_minimap_size/2);
	glEnd();

	glDisable(GL_ALPHA_TEST); 
	glDisable( GL_BLEND );
	glPopMatrix();
}

static __inline__ void draw_map(float zoom_multip, float px, float py)
{
	float sx = 0.0f, sy = 0.0f;
	int i;
	float coef = 91 * zoom_multip * minimap_size_coefficient;
	float x, y;
	float fsinold = -1.0,fcosold = -1.0;
	int shift;
	float fsin,fcos;

	glPushMatrix();

	sx = float_minimap_size/2;
	sy = float_minimap_size/2;


	glTranslatef(sx, sy, 0.0f);

	//draw the minimap
	bind_texture_id(minimap_texture);
	glColor4f(1.0f,1.0f,1.0f,1.0f);

	rotate_at_player(zoom_multip,px,py);
	
	if (use_frame_buffer)
	{
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(-float_minimap_size/2, float_minimap_size/2);
			glTexCoord2f(1.0f, 0.0f);
			glVertex2f(float_minimap_size/2, float_minimap_size/2);
			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(float_minimap_size/2, -float_minimap_size/2);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(-float_minimap_size/2, -float_minimap_size/2);
		glEnd();
	}
	else
	{
		//fsin is always in the range [0..1] at shift 0 (no need to check it)
		//fcos is always less than 1 at shift 0
		fcos = -radius_shift* zoom_multip + 1 - py/float_minimap_size;
		if(fcos < 0.0f)
		{
			//find an appropriate starting shift where both fsin and fcos are in the [0..1] range
			for (shift=0; shift<360; shift +=10) 
			{
				fsin = sin((shift)*0.0174532925f)*radius_shift* zoom_multip +  px/float_minimap_size;
				fcos = cos((shift+ 180)*0.0174532925f)*radius_shift* zoom_multip + 1 - (py/float_minimap_size);
				if((fsin >= 0.0f) && (fsin <= 1.0f) && (fcos >= 0.0f) && (fcos <= 1.0f))
				{
					break;
				}
			}
		}
		else
			shift = 0;

		glBegin(GL_POLYGON);
			for (i=0; i<=360; i +=10) 
			{
				
				fsin = sin((i+shift)*0.0174532925f)*radius_shift* zoom_multip +  px/float_minimap_size;
				fcos = cos((i+shift+ 180)*0.0174532925f)*radius_shift* zoom_multip + 1 - (py/float_minimap_size);

				if(fsin > 1.0f)
					fsin = fsinold;
				else if (fsin < 0.0f)
					fsin = fsinold;
				else
				{
					x = sin((i+shift)*0.0174532925)/radius_shift*coef + (px - float_minimap_size/2);
					fsinold = fsin;
				}

				if(fcos > 1.0f)
					fcos = fcosold;
				else if (fcos < 0.0f)
					fcos = fcosold;
				else
				{
					y = cos((i+shift)*0.0174532925f)/radius_shift*coef + (py - float_minimap_size/2);
					fcosold = fcos;
				}
				glTexCoord2f(fsin, fcos);			
				glVertex2f(x, y);		
			}	
		glEnd();

	}
	

	glPopMatrix();

	//draw the compass texture
	
	draw_compass();
}

static __inline__ void draw_into_minimap_fbo(float zoom_multip, float px, float py)
{
	GLint view_port[4];

	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	glGetIntegerv(GL_VIEWPORT, view_port);
	CHECK_GL_ERRORS();
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, minimap_fbo);
	glViewport(0, 0, minimap_size, minimap_size);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, (GLdouble)minimap_size, (GLdouble)minimap_size, 0.0, -250.0, 250.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	draw_map(zoom_multip, px, py);
	draw_actor_points(zoom_multip, px, py);

	bind_texture_id(0);
	//If we rendered to a framebuffer, draw it
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	ELglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glViewport(view_port[0], view_port[1], view_port[2], view_port[3]);
	CHECK_GL_ERRORS();
	CHECK_FBO_ERRORS();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void draw_minimap_title_bar(window_info *win)
{
	float u_first_start= (float)31/255;
	float u_first_end= 0;
	float v_first_start= 1.0f-(float)160/255;
	float v_first_end= 1.0f-(float)175/255;

	float u_last_start= 0;
	float u_last_end= (float)31/255;
	float v_last_start= 1.0f-(float)160/255;
	float v_last_end= 1.0f-(float)175/255;

	glPushMatrix();

	glColor3f(1.0f,1.0f,1.0f);
	
	get_and_set_texture_id(icons_text);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0.03f);

	glBegin(GL_QUADS);

	glTexCoord2f(u_first_end, v_first_start);
	glVertex3i(win->len_x/2-32, ELW_TITLE_HEIGHT, 0);
	glTexCoord2f(u_first_end, v_first_end);
	glVertex3i(win->len_x/2-32, 0, 0);
	glTexCoord2f(u_first_start, v_first_end);
	glVertex3i(win->len_x/2, 0, 0);
	glTexCoord2f(u_first_start, v_first_start);
	glVertex3i(win->len_x/2, ELW_TITLE_HEIGHT, 0);

	glTexCoord2f(u_last_end, v_last_start);
	glVertex3i(win->len_x/2, ELW_TITLE_HEIGHT, 0);
	glTexCoord2f(u_last_end, v_last_end);
	glVertex3i(win->len_x/2, 0, 0);
	glTexCoord2f(u_last_start, v_last_end);
	glVertex3i(win->len_x/2+32, 0, 0);
	glTexCoord2f(u_last_start, v_last_start);
	glVertex3i(win->len_x/2+32, ELW_TITLE_HEIGHT, 0);
	
	glEnd();
	glDisable(GL_ALPHA_TEST);

	glPopMatrix();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

int display_minimap_handler(window_info *win)
{
	float zoom_multip;
	float size_x = float_minimap_size / (tile_map_size_x * 6);
	float size_y = float_minimap_size / (tile_map_size_y * 6);
	float px = 0.0f, py = 0.0f;
	actor *me;
	float x,y;
	int i;

	draw_minimap_title_bar(win);
	
	zoom_multip = minimap_get_zoom();

	if(!minimap_texture) 
	{
		//there's no minimap for this map :( draw a X
		glTranslatef(0.0f, 16.0f, 0.0f);
		glPushMatrix();
		glDisable(GL_TEXTURE_2D);
		glColor3f(1.0f, 0.0f, 0.0f);
		glLineWidth(3.0f);
		glBegin(GL_LINES);
			glVertex2f(-radius_shift*float_minimap_size/2 + float_minimap_size/2,-radius_shift*float_minimap_size/2 + float_minimap_size/2);
			glVertex2f(radius_shift*float_minimap_size/2 + float_minimap_size/2,radius_shift*float_minimap_size/2 + float_minimap_size/2);
			glVertex2f(-radius_shift*float_minimap_size/2 + float_minimap_size/2,radius_shift*float_minimap_size/2 + float_minimap_size/2);
			glVertex2f(radius_shift*float_minimap_size/2 + float_minimap_size/2,-radius_shift*float_minimap_size/2 + float_minimap_size/2);
		glEnd();

		glEnable(GL_TEXTURE_2D);
		glPopMatrix();
		draw_compass();
		return 0;
	}
	//draw minimap

	//get player position in window coordinates
	if( (me = get_our_actor ()) == NULL)
	{
		//Don't know who we are? can't draw then
		return 0;
	}
	px = me->x_tile_pos * size_x;
	py = float_minimap_size - (me->y_tile_pos * size_y);

	

	if (use_frame_buffer)
	{
		draw_into_minimap_fbo(zoom_multip, px, py);
		draw_framebuffer();

		return 0;
	}

	glTranslatef(0.0f, 16.0f, 0.0f);

	//draw black background
	glColor3f(0.0f,0.0f,0.0f);
	glBegin(GL_POLYGON);
	for (i=0; i<=360; i +=10) 
	{
		x = sin((i)*0.0174532925)/2*float_minimap_size+float_minimap_size/2;
		y = cos((i)*0.0174532925f)/2*float_minimap_size+float_minimap_size/2;
		glVertex2f(x, y);	
	}
	glEnd();

	draw_map(zoom_multip, px, py);
	draw_actor_points(zoom_multip, px, py);


#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE

	return 0;
}

/*
 * Handler for clicks into minimap. Coordinates are given as window pixels with origin at bottom-left corner
 */   
static int minimap_walkto(int mx, int my)
{
	float size_x = tile_map_size_x * 6;
	float size_y = tile_map_size_y * 6;
	float fmx = mx, fmy = my;
	float px, py;
	float zoom_multip;
	actor *me;

	/* Safety check. Avoid division by zero */
	if (float_minimap_size == 0)
		return 0;

	

	/* Get zoom and actor position (tile coordinates) */
	zoom_multip = minimap_get_zoom();
	if ( (me = get_our_actor ()) == NULL)
		return 0;

	/* Convert tile coordinates to minimap pixels */
	px = (me->x_tile_pos / size_x) * float_minimap_size;
	py = (me->y_tile_pos / size_y) * float_minimap_size;

	rotate_click_coords(&fmx,&fmy);

	fmx = me->x_tile_pos - minimap_tiles_distance 
		+ minimap_tiles_distance * 2 * fmx/float_minimap_size;
	fmy = me->y_tile_pos - minimap_tiles_distance 
		+ minimap_tiles_distance * 2 * fmy/float_minimap_size;

	/* Do path finding */
	if (pf_find_path(fmx, fmy))
	{
		return 1;
	}

	return 0;
}

int click_minimap_handler(window_info * win, int mx, int my, Uint32 flags)
{
	if(my >= 16 && left_click)
	{
		//check if the click is in the round area
		if(is_within_radius(mx,my,float_minimap_size/2,float_minimap_size/2,float_minimap_size/2))
			return minimap_walkto(mx, win->len_y - my);
	} 

	return 0;
}

void load_exploration_map ()
{
	FILE *fp = NULL;
	char exploration_map_filename[256];

	if(!minimap_texture)
		return;

	my_strcp (exploration_map_filename, map_file_name);
	exploration_map_filename[strlen(exploration_map_filename)-4] = 0;
	strcat (exploration_map_filename, ".xm");
	safe_strncpy (current_exploration_map_filename, exploration_map_filename, sizeof (current_exploration_map_filename));
	fp = open_file_config (exploration_map_filename, "rb");
	if(fp)
	{
		fread(exploration_map, sizeof(GLubyte), 256 * 256, fp);
		fclose(fp);
	}
	else
	{
		memset(exploration_map, 0, 256 * 256 * sizeof(GLubyte));
	}
	
	if(poor_man)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else if(use_mipmaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	if(have_extension(arb_texture_compression))
	{
		if(have_extension(ext_texture_compression_s3tc))
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGB_S3TC_DXT1_EXT, minimap_size, minimap_size,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,&exploration_map);
		else
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_LUMINANCE, minimap_size, minimap_size,0,GL_ALPHA,GL_UNSIGNED_BYTE,&exploration_map);
		
	}
	else
		glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE, minimap_size, minimap_size,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,&exploration_map);
	

	CHECK_GL_ERRORS();	
}

void save_exploration_map()
{
	FILE *fp = NULL;
	
	if(!minimap_texture)
		return;
	
	fp = open_file_config (current_exploration_map_filename, "wb");
	if (fp)
	{
		fwrite(exploration_map, sizeof(GLubyte), 256 * 256, fp);
		fclose(fp);
	}
	else
	{
		//log error and quit
	}	
}

void change_minimap(){
	char minimap_file_name[256];
	texture_cache_struct tex;

	if(minimap_win < 0)
		return;
	save_exploration_map();

	//unload all textures
	if(minimap_texture)
		glDeleteTextures(1,&minimap_texture);
	if(compass_tex)
		glDeleteTextures(1,&compass_tex);
	if(exploration_texture)
		glDeleteTextures(1,&exploration_texture);
	if(use_frame_buffer)
		minimap_free_framebuffer();

	//make filename
	my_strcp(minimap_file_name,map_file_name);
	minimap_file_name[strlen(minimap_file_name)-4] = '\0';
	strcat(minimap_file_name, ".bmp");

	//load textures
	my_strcp(tex.file_name, minimap_file_name);
	if (!el_file_exists(minimap_file_name))
		minimap_texture = 0;
	else
		minimap_texture = load_bmp8_fixed_alpha(&tex,128);
	
	my_strcp(tex.file_name, "./textures/compass.bmp");
	compass_tex = load_bmp8_fixed_alpha_with_transparent_color(&tex,255,0,0,0);

	glGenTextures(1, &exploration_texture);
	bind_texture_id(exploration_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	load_exploration_map();

	if(use_frame_buffer)
		minimap_make_framebuffer();

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


void display_minimap()
{
	window_info *win;

	minimap_size_coefficient = 0.7f;
	minimap_size = 256 * minimap_size_coefficient;
	float_minimap_size = 256.0 * minimap_size_coefficient;

	if(minimap_win < 0)
	{
		//init minimap
		minimap_win = create_window("Minimap", windows_on_top?-1:game_root_win, 0, minimap_win_x, minimap_win_y, 
			minimap_size, minimap_size+16, ELW_SHOW|ELW_TITLE_NAME|ELW_ALPHA_BORDER|ELW_SWITCHABLE_OPAQUE|ELW_DRAGGABLE);
		set_window_handler(minimap_win, ELW_HANDLER_DISPLAY, &display_minimap_handler);	
		set_window_handler(minimap_win, ELW_HANDLER_CLICK, &click_minimap_handler);	
		win = &(windows_list.window[minimap_win]);
		win->owner_drawn_title_bar = 1;

		change_minimap();
	} else {
		show_window(minimap_win);
		select_window(minimap_win);
	}
}


#endif //MINIMAP2

//EOF

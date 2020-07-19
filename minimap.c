
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include "minimap.h"
#include "asc.h"
#include "buddy.h"
#include "colors.h"
#include "context_menu.h"
#include "cursors.h"
#include "elconfig.h"
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
#include "named_colours.h"
#include "spells.h"
#include "textures.h"
#include "tiles.h"
#include "pathfinder.h"
#include "translate.h"
#include "io/map_io.h"
#include "io/elpathwrapper.h"
#include "io/elfilewrapper.h"
#include "mapwin.h"
#include "map.h"
#include "image_loading.h"

float minimap_tiles_distance = 48;
int rotate_minimap = 1;
int pin_minimap = 0;
int open_minimap_on_start = 0;
float minimap_size_coefficient = 0.7f;

static int minimap_size;
static float float_minimap_size;
static int enable_controls = 0;
static int title_len = 0;

static GLuint compass_tex;
static GLuint minimap_texture = 0;
static GLuint exploration_texture = 0;
//static GLubyte exploration_map[256][256];
//static char current_exploration_map_filename[256];

static __inline__ float minimap_get_zoom ()
{
	float zoom = minimap_tiles_distance * 2 / (tile_map_size_x*6);
	return zoom;
}

static __inline__ void rotate_actor_points(float zoom_multip, float px, float py)
{
	float x,y;
	x = (px - (float_minimap_size/2) ) + float_minimap_size/2;
	y = (py - (float_minimap_size/2) ) + float_minimap_size/2;

	glTranslatef(float_minimap_size/2, float_minimap_size/2, 0.0f);

	if(rotate_minimap)
		glRotatef(-rz, 0.0f,0.0f,1.0f );
	glTranslatef(-x,-y,0.0f);

	glScalef(1.0f / zoom_multip, 1.0f / zoom_multip, 1.0f);
	x = x * zoom_multip * ((1.0f / zoom_multip) - 1);
	y = y * zoom_multip * ((1.0f / zoom_multip) - 1);
	glTranslatef(-x,-y,0.0f);
}

static __inline__ void rotate_at_player(float zoom_multip, float px, float py)
{
	float x,y;
	x = (px - (float_minimap_size/2) );
	y = (py - (float_minimap_size/2) );
	if(rotate_minimap)
		glRotatef(-rz, 0.0f,0.0f,1.0f );
	glTranslatef(-x,-y,0.0f);
	
	glScalef(1.0f / zoom_multip, 1.0f / zoom_multip, 1.0f);

	x = x * zoom_multip * ((1.0f / zoom_multip) - 1);
	y = y * zoom_multip * ((1.0f / zoom_multip) - 1);
	glTranslatef(-x,-y,0.0f);
}

static __inline__ void rotate_click_coords(float * x,float * y)
{
	if(rotate_minimap)
	{
		float fx,fy;
		float angel = -rz*0.0174532925f;
		fx = cos(angel) * (*x - float_minimap_size/2.0f) - sin(angel) * (*y - float_minimap_size/2.0f);
		fy = sin(angel) * (*x - float_minimap_size/2.0f) + cos(angel) * (*y - float_minimap_size/2.0f);
		*x = fx + float_minimap_size/2;
		*y = fy + float_minimap_size/2;
	}
}

static __inline__ int is_within_radius(float mx, float my,float px,float py,float radius)
{
	float distance;

	distance = sqrt(pow(px - mx,2) + pow(py-my,2));
	if(distance <= radius)
		return 1;
	else 
		return 0;
}

static __inline__ void draw_actor_points(window_info *win, float zoom_multip, float px, float py)
{
	float size_x = float_minimap_size / (tile_map_size_x * 6);
	float size_y = float_minimap_size / (tile_map_size_y * 6);
	actor *a;
	int i;
	float x, y;

	glPushMatrix();
	glDisable(GL_TEXTURE_2D);

	//display the actors
	glEnable( GL_POINT_SMOOTH );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glPointSize((int)(0.5 + win->current_scale * 8));

	rotate_actor_points(zoom_multip,px,py);

	glBegin(GL_POINTS);

	for (i = 0; i < max_actors; i++)
	{
		if (actors_list[i])
		{
			a = actors_list[i];
			if (a->attached_actor != -1 && a->actor_id == -1)
				continue;
			x = a->x_tile_pos * size_x;
			y = float_minimap_size - (a->y_tile_pos * size_y);

			glColor4f(0.0f,0.0f,0.0f,1.0f);
			glVertex2f(x+2*zoom_multip, y+2*zoom_multip);

			if (a->kind_of_actor == NPC)
				elglColourN("minimap.npc");
			else if(a->actor_id == yourself)
				elglColourN("minimap.yourself");
			else if(a->is_enhanced_model && (a->kind_of_actor ==  PKABLE_HUMAN || a->kind_of_actor == PKABLE_COMPUTER_CONTROLLED))
				elglColourN("minimap.pkable");
			else if(a->is_enhanced_model && is_in_buddylist(a->actor_name))
				elglColourN("minimap.buddy");
			else if (is_color ((unsigned char)a->actor_name[0]))
			{
				if(a->is_enhanced_model && is_in_buddylist(a->actor_name))
					elglColourN("minimap.buddy");
				else
				{	// Use the colour of their name. This gives purple bots, green demigods, etc.
					int color = from_color_char (a->actor_name[0]);
					glColor4ub (colors_list[color].r1,
						colors_list[color].g1,
						colors_list[color].b1, 255);
				}
			}
			else if(!a->is_enhanced_model)
			{
				if (a->dead) 
					elglColourN("minimap.deadcreature");
				else // alive
					elglColourN("minimap.creature");
			}
			else
				elglColourN("minimap.otherplayer");
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
			if(is_within_radius(x,y,px,py,zoom_multip*(minimap_size/2-15)))
			{
				elglColourN("minimap.mine");
				glVertex2f(x, y);
			}
		}
	}

	glEnd();//GL_POINTS
	glDisable(GL_BLEND);
	glDisable(GL_POINT_SMOOTH);

	glPopMatrix();
	
	if (pf_follow_path)
	{
		x = pf_dst_tile->x * size_x;
		y = float_minimap_size - (pf_dst_tile->y * size_y);

		if (x != px || y != py)
		{
			float diff = 6.0f * zoom_multip * win->current_scale;

			if(is_within_radius(x,y,px,py,zoom_multip*(minimap_size/2-15)))
			{
				glPushMatrix();
				glDisable(GL_TEXTURE_2D);
				rotate_actor_points(zoom_multip,px,py);
				glBegin(GL_LINES);
				elglColourN("minimap.cross");
				glVertex2f(x-diff, y-diff);
				glVertex2f(x+diff, y+diff);
				glVertex2f(x-diff, y+diff);
				glVertex2f(x+diff, y-diff);
				glEnd();//GL_LINES
				glPopMatrix();
			}
		}
	}

	//draw map markings
	for(i=0;i<max_mark;i++){
		if(!marks[i].server_side) continue;
		x= marks[i].x*size_x;
		y= float_minimap_size-(marks[i].y*size_y);
		if (x != px || y != py)
		{
			float diff = 4.0f*zoom_multip;
	
			if(is_within_radius(x,y,px,py,zoom_multip*(minimap_size/2-15)))
			{
				glPushMatrix();
				glDisable(GL_TEXTURE_2D);
				rotate_actor_points(zoom_multip,px,py);
				glBegin(GL_LINES);
				elglColourN("minimap.servermark");
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
	if(rotate_minimap)
		glRotatef(-rz - 90, 0.0f,0.0f,1.0f );
	else
		glRotatef(-90, 0.0f,0.0f,1.0f );
	glRotatef(180, 1.0f,0.0f,0.0f );
	glEnable(GL_TEXTURE_2D); 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	bind_texture(compass_tex);

	glBegin(GL_QUADS); 
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(-float_minimap_size/2, float_minimap_size/2);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(float_minimap_size/2, float_minimap_size/2);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(float_minimap_size/2, -float_minimap_size/2);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(-float_minimap_size/2, -float_minimap_size/2);
	glEnd();

	glDisable(GL_ALPHA_TEST); 
	glDisable( GL_BLEND );
	glPopMatrix();
}

static __inline__ void draw_map(window_info *win,float zoom_multip, float px, float py)
{
	float sx = 0.0f, sy = 0.0f;
	int i;
	float x, y;

	glPushMatrix();

	sx = float_minimap_size/2;
	sy = float_minimap_size/2;

	glTranslatef(sx, sy, 0.0f);

	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 1); 
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	glColor3f(0.0f,0.0f,0.0f);
	glBegin(GL_POLYGON);
		for (i=0; i<=360; i +=10) 
		{
			x = sin((i)*0.0174532925f)/2*float_minimap_size;
			y = cos((i)*0.0174532925f)/2*float_minimap_size;
			glVertex2f(x, y);	
		}
		glEnd();

	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// re-enable the drawing in the current buffer
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glEnable(GL_TEXTURE_2D);

	//draw the map
	bind_texture(minimap_texture);
	glColor4f(1.0f,1.0f,1.0f,1.0f);

	rotate_at_player(zoom_multip,px,py);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(-float_minimap_size/2, float_minimap_size/2);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(float_minimap_size/2, float_minimap_size/2);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(float_minimap_size/2, -float_minimap_size/2);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(-float_minimap_size/2, -float_minimap_size/2);
	glEnd();

	glDisable(GL_STENCIL_TEST);

	glPopMatrix();
	if (compass_tex) 
	{
		//draw the compass texture
		draw_compass();
	}
}

static void draw_minimap_title_bar(window_info *win)
{
	float u_first_start= (float)31/255;
	float u_first_end = 0.5f/255.0f;
	float v_first_start = (float)160/255;
	float v_first_end = (float)175/255;

	float u_last_start = 0.5f/255.0f;
	float u_last_end = (float)31/255;
	float v_last_start = (float)160/255;
	float v_last_end = (float)175/255;

	int close_button_x = win->len_x/2 + title_len - 1;

	glPushMatrix();

	glColor3f(1.0f,1.0f,1.0f);
	
	bind_texture(icons_text);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0.03f);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);

	glTexCoord2f(u_first_end, v_first_start);
	glVertex3i(win->len_x/2-title_len, win->title_height, 0);
	glTexCoord2f(u_first_end, v_first_end);
	glVertex3i(win->len_x/2-title_len, 0, 0);
	glTexCoord2f(u_first_start, v_first_end);
	glVertex3i(win->len_x/2, 0, 0);
	glTexCoord2f(u_first_start, v_first_start);
	glVertex3i(win->len_x/2, win->title_height, 0);

	glTexCoord2f(u_last_end, v_last_start);
	glVertex3i(win->len_x/2, win->title_height, 0);
	glTexCoord2f(u_last_end, v_last_end);
	glVertex3i(win->len_x/2, 0, 0);
	glTexCoord2f(u_last_start, v_last_end);
	glVertex3i(win->len_x/2+title_len, 0, 0);
	glTexCoord2f(u_last_start, v_last_start);
	glVertex3i(win->len_x/2+title_len, win->title_height, 0);
	
	glEnd();
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);

	//draw the X background
	glColor3f(0.156f,0.078f,0.0f);
	glBegin(GL_POLYGON);
		glVertex2f(close_button_x + win->title_height, win->title_height);
		glVertex2f(close_button_x, win->title_height);
		glVertex2f(close_button_x, 0);
		glVertex2f(close_button_x + win->title_height, 0);
	glEnd();
	//draw the rectngle
	glColor3f(win->line_color[0],win->line_color[1],win->line_color[2]);
	glBegin(GL_LINE_STRIP);
		glVertex2i(close_button_x + win->title_height, win->title_height);
		glVertex2i(close_button_x, win->title_height);
		glVertex2i(close_button_x, 0);
		glVertex2i(close_button_x + win->title_height, 0);
		glVertex2i(close_button_x + win->title_height, win->title_height);
	glEnd();
	//draw the X
	draw_cross(close_button_x + win->title_height / 2, win->title_height / 2,
		win->title_height / 2 - win->title_height / 6, 1);
	glPopMatrix();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static int display_minimap_handler(window_info *win)
{
	float zoom_multip;
	float size_x = float_minimap_size / (tile_map_size_x * 6);
	float size_y = float_minimap_size / (tile_map_size_y * 6);
	float px = 0.0f, py = 0.0f;
	actor *me;
	float x,y;
	int i;

	if (win->pos_x + win->len_x/2 < 0)
		move_window(win->window_id, win->pos_id, win->pos_loc, -win->len_x/2, win->pos_y);
	else if (win->pos_x > window_width - win->len_x/2)
		move_window(win->window_id, win->pos_id, win->pos_loc, window_width - win->len_x/2, win->pos_y);
	if (win->pos_y < 0)
		move_window(win->window_id, win->pos_id, win->pos_loc, win->pos_x, win->title_height);
	else if (win->pos_y > window_height - win->title_height)
		move_window(win->window_id, win->pos_id, win->pos_loc, win->pos_x, window_height - win->title_height);

	if (enable_controls)
	{
		draw_minimap_title_bar(win);
		enable_controls = 0;
	}
	
	zoom_multip = minimap_get_zoom();

	if(!minimap_texture) 
	{
		//there's no minimap for this map :( draw a X
		glTranslatef(0.0f, win->title_height, 0.0f);
		glPushMatrix(); 
		glDisable(GL_TEXTURE_2D);
		//draw black background
		glColor3f(0.0f,0.0f,0.0f);
		glBegin(GL_POLYGON);
		for (i=0; i<=360; i +=10) 
		{
			x = sin((i)*0.0174532925f)/2*float_minimap_size+float_minimap_size/2;
			y = cos((i)*0.0174532925f)/2*float_minimap_size+float_minimap_size/2;
			glVertex2f(x, y);	
		}
		glEnd();

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

	glTranslatef(0.0f, win->title_height, 0.0f);

	glDisable(GL_TEXTURE_2D);
	//draw black background
	glColor3f(0.0f,0.0f,0.0f);
	glBegin(GL_POLYGON);
	for (i=0; i<=360; i +=10) 
	{
		x = sin((i)*0.0174532925f)/2*float_minimap_size+float_minimap_size/2;
		y = cos((i)*0.0174532925f)/2*float_minimap_size+float_minimap_size/2;
		glVertex2f(x, y);	
	}
	glEnd();

	draw_map(win,zoom_multip, px, py);
	draw_actor_points(win, zoom_multip, px, py);


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
	float fmx = mx, fmy = my;
	actor *me;

	if ( (me = get_our_actor ()) == NULL)
		return 0;

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

static void increase_zoom()
{
	minimap_tiles_distance -=8;
	if(minimap_tiles_distance < 48)
		minimap_tiles_distance = 48;
}

static void decrease_zoom()
{
	minimap_tiles_distance +=8;
	if(minimap_tiles_distance > 144)
		minimap_tiles_distance = 144;
}

static int click_minimap_handler(window_info * win, int mx, int my, Uint32 flags)
{
	int close_button_x = win->len_x/2 + title_len - 1;
	if(left_click)
	{
		//check for close button click
		if((mx >=close_button_x) && (mx <=close_button_x + win->title_height) 
			&&	(my <= win->title_height))
		{
			hide_window(win->window_id);
			return 1;
		}
		else if(my >= win->title_height)
		{
			//check if the click is in the round area
			if(is_within_radius(mx,my-win->title_height,float_minimap_size/2,float_minimap_size/2,float_minimap_size/2))
			{
				minimap_walkto(mx, win->len_y - my);
				return 1;
			}
		}
		// title bar?
		else if ((mx > win->len_x/2-title_len) && (mx < win->len_x/2+title_len) && (my >= 0) && (my <= 2*win->title_height))
			return 1;
	}
	else if((flags & ELW_WHEEL) && is_within_radius(mx,my-win->title_height,float_minimap_size/2,float_minimap_size/2,float_minimap_size/2))
	{
		if(flags & ELW_WHEEL_UP)
			increase_zoom();
		else
			decrease_zoom();
		return 1;
	}

	return 0;
}

static int keypress_minimap_handler (window_info *win, int mx, int my, SDL_Keycode key_code, Uint32 key_unicode, Uint16 key_mod)
{
	if (is_within_radius(mx,my-win->title_height,float_minimap_size/2,float_minimap_size/2,float_minimap_size/2))
	{
		if((key_code == SDLK_KP_PLUS) || (key_code == SDLK_PAGEUP))
		{
			increase_zoom();
			return 1;
		}
		else if ((key_code == SDLK_KP_MINUS) ||  (key_code == SDLK_PAGEDOWN))
		{
			decrease_zoom();
			return 1;
		}
	}

	return 0;
}

#if 0
static void load_exploration_map (void)
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
		if (fread(exploration_map, sizeof(GLubyte), 256 * 256, fp) != 256 * 256)
		{
			memset(exploration_map, 0, 256 * 256 * sizeof(GLubyte));
			LOG_ERROR("%s() read failed for file [%s]\n", __FUNCTION__, exploration_map_filename);
		}
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

void save_exploration_map(void)
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
#endif

void change_minimap(void)
{
	char minimap_file_name[256];

	if(get_id_MW(MW_MINIMAP) < 0)
		return;
	//save_exploration_map();

	//unload all textures
	if(exploration_texture)
		glDeleteTextures(1,&exploration_texture);

	//make filename
	if (check_image_name(map_file_name, sizeof(minimap_file_name), minimap_file_name) == 1)
	{
		minimap_texture = load_texture_cached(minimap_file_name, tt_image);
	}
	else
	{
		minimap_texture = 0;
	}

	compass_tex = load_texture_cached("./textures/compass", tt_gui);

	glGenTextures(1, &exploration_texture);
	bind_texture_id(exploration_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//load_exploration_map();

#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

static int mouseover_minimap_handler(window_info * win, int mx, int my, Uint32 flags)
{
	if(is_within_radius(mx,my-win->title_height,float_minimap_size/2,float_minimap_size/2,float_minimap_size/2) ||
		((mx > win->len_x/2-title_len) && (mx < win->len_x/2+title_len+win->title_height) && (my >= 0) && (my <= 2*win->title_height)))
	{
		elwin_mouse=CURSOR_ARROW;
		enable_controls = 1;
		return 1;
	}
	else
		return 0;
}

static int cm_minimap_title_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	return cm_title_handler(win, widget_id, mx, my, option);
}

static int ui_scale_minimap_handler(window_info *win)
{
	title_len = (int)(0.5 + win->current_scale * 32);
	if (title_len + win->title_height > win->len_x/2)
		title_len = win->len_x/2 - win->title_height;
	cm_remove_regions(win->cm_id);
	cm_add_region(win->cm_id, win->window_id, win->len_x/2-title_len, 0, title_len*2, win->title_height );
	resize_window(win->window_id, minimap_size, minimap_size + win->title_height);
	return 1;
}

void display_minimap(void)
{
	int minimap_win = get_id_MW(MW_MINIMAP);

	minimap_size = 256 * minimap_size_coefficient;
	float_minimap_size = 256.0 * minimap_size_coefficient;

	if(minimap_tiles_distance < 48)
		minimap_tiles_distance = 48;
	if(minimap_tiles_distance > 144)
		minimap_tiles_distance = 144;

	if(minimap_win < 0)
	{
		//init minimap
		window_info *win;
		minimap_win = create_window(win_minimap, (not_on_top_now(MW_MANU) ?game_root_win : -1), 0, get_pos_x_MW(MW_MINIMAP), get_pos_y_MW(MW_MINIMAP), 
			minimap_size, minimap_size+ELW_TITLE_HEIGHT, ELW_USE_UISCALE|ELW_CLICK_TRANSPARENT|ELW_SHOW|ELW_TITLE_NAME|ELW_ALPHA_BORDER|ELW_SWITCHABLE_OPAQUE|ELW_DRAGGABLE);
		set_id_MW(MW_MINIMAP, minimap_win);
		set_window_handler(minimap_win, ELW_HANDLER_DISPLAY, &display_minimap_handler);	
		set_window_handler(minimap_win, ELW_HANDLER_CLICK, &click_minimap_handler);	
		set_window_handler(minimap_win, ELW_HANDLER_MOUSEOVER, &mouseover_minimap_handler);	
		set_window_handler(minimap_win, ELW_HANDLER_KEYPRESS, (int (*)())&keypress_minimap_handler );
		set_window_handler(minimap_win, ELW_HANDLER_UI_SCALE, &ui_scale_minimap_handler);
		win = &(windows_list.window[minimap_win]);
		win->owner_drawn_title_bar = 1;
		change_minimap();
		
		if (!cm_valid(win->cm_id))
		{
			win->cm_id = cm_create(cm_title_menu_str, cm_minimap_title_handler);
			cm_grey_line(win->cm_id, 1, 1);
			cm_bool_line(win->cm_id, 2, &windows_on_top, "windows_on_top");
		}

		ui_scale_minimap_handler(win);
		check_proportional_move(MW_MINIMAP);

		cm_add(win->cm_id, cm_minimap_menu_str, NULL);
		cm_add_region(win->cm_id, minimap_win, win->len_x/2-title_len, 0, title_len*2, win->title_height );
		cm_bool_line(win->cm_id, ELW_CM_MENU_LEN+1, &rotate_minimap, "rotate_minimap");
		cm_bool_line(win->cm_id, ELW_CM_MENU_LEN+2, &pin_minimap, "pin_minimap");
		cm_bool_line(win->cm_id, ELW_CM_MENU_LEN+3, &open_minimap_on_start, NULL);
	} else {
		show_window(minimap_win);
		select_window(minimap_win);
	}
}



//EOF

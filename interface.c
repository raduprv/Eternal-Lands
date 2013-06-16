#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include <errno.h>
#include "interface.h"
#include "asc.h"
#include "bbox_tree.h"
#include "chat.h"
#include "consolewin.h"
#include "cursors.h"
#include "elwindows.h"
#include "elmemory.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "langselwin.h"
#include "loginwin.h"
#include "map.h"
#include "mapwin.h"
#include "new_character.h"
#include "openingwin.h"
#include "pathfinder.h"
#include "rules.h"
#ifdef DEBUG_MAP_SOUND
#include "sound.h"
#endif // DEBUG_MAP_SOUND
#include "spells.h"
#include "textures.h"
#include "tiles.h"
#include "translate.h"
#include "update.h"
#include "weather.h"
#include "io/map_io.h"
 #include "errors.h"
 #include "io/elpathwrapper.h"
 #include "io/elfilewrapper.h"
#include "3d_objects.h"
#ifdef	NEW_TEXTURES
#include "image_loading.h"
#endif	/* NEW_TEXTURES */

#define DEFAULT_CONTMAPS_SIZE 20

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void draw_menu_title_bar(int, int, int);
 */

int mouse_x;
int mouse_y;
int right_click;
int middle_click;
int left_click;

char username_box_selected=1;
char password_box_selected=0;
char username_str[20]={0};
char password_str[20]={0};
char display_password_str[20]={0};
int username_text_length=0;
int password_text_length=0;

int have_a_map=0;
int auto_camera=0;
int view_health_bar=1;
int view_ether_bar=0;
int view_names=1;
int view_hp=0;
int view_ether=0;
int view_chat_text_as_overtext=0;
int view_mode_instance=0;
float view_mode_instance_banner_height=5.0f;

//instance mode banners config:
int im_creature_view_names = 1;
int im_creature_view_hp = 1;
int im_creature_view_hp_bar = 0;
int im_creature_banner_bg = 0;
int im_other_player_view_names = 1;
int im_other_player_view_hp = 1;
int im_other_player_view_hp_bar = 0;
int im_other_player_banner_bg = 0;
int im_other_player_show_banner_on_damage = 0;

int limit_fps=0;

int action_mode=ACTION_WALK;

Uint32 click_time=0;

GLdouble model_mat[16];
GLdouble projection_mat[16];
GLint viewport[4];

// Grum: attempt to work around bug in Ati linux drivers.
int ati_click_workaround = 0;

float mapmark_zoom=0.3f;

void save_scene_matrix ()
{
	glGetDoublev (GL_MODELVIEW_MATRIX, model_mat);
	glGetDoublev (GL_PROJECTION_MATRIX, projection_mat);
	glGetIntegerv (GL_VIEWPORT, viewport);
}

void get_world_x_y (short *scene_x, short *scene_y)
{
	double sx, sy, sz;
	float mouse_z;

	glReadPixels (mouse_x, window_height-mouse_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &mouse_z);

	// XXX FIXME (Grum): hack to work around a bug in the Ati drivers or
	// a giant misconception on the part of all EL developers so far.
	if (ati_click_workaround && bpp == 32)
		mouse_z = ldexp (mouse_z, 8);
	
	gluUnProject (mouse_x, window_height-hud_y-mouse_y, mouse_z, model_mat, projection_mat, viewport, &sx, &sy, &sz);
	
	*scene_x = (sx / 0.5);
	*scene_y = (sy / 0.5);
}

void get_old_world_x_y (short *scene_x, short *scene_y)
{
	AABBOX box;
	float t, x, y, z, t1, t2, tx, ty, dx, dy, h, len, h1, h2, h3, h4;
	int i, j, h_max, h_min, sx, sy, zx, zy, i_min, i_max, j_min, j_max;

	x = click_line.center[X];
	y = click_line.center[Y];
	z = click_line.center[Z];
	dx = click_line.direction[X];
	dy = click_line.direction[Y];
	t = min2f(4.0f, max2f(-2.2f, z));
	if (t != z)
	{
		t = (t-z)/click_line.direction[Z];
		x += t*dx;
		y += t*dy;
	}
	t = 6.2f / click_line.direction[Z];
	len = min2f(max2f(t, -t), click_line.length);

	len = click_line.length;
	if (max2f(dx, -dx) < 0.000001f)
	{
		zx = 1;
		sx = 0;
	}
	else
	{
		zx = 0;
		if (dx < 0.0f) sx = -1;
		else sx = 1;
	}
	if (max2f(dy, -dy) < 0.000001f)
	{
		zy = 1;
		sy = 0;
	}
	else
	{
		zy = 0;
		if (dy < 0.0f) sy = -1;
		else sy = 1;
	}
	i = x*2.0f;
	j = y*2.0f;
	
	i_min = min2f(x, x+dx*len)*2.0f;
	i_max = max2f(x, x+dx*len)*2.0f;
	j_min = min2f(y, y+dy*len)*2.0f;
	j_max = max2f(y, y+dy*len)*2.0f;
	
	i_min = max2i(min2i(i_min, tile_map_size_x*6), 0);
	i_max = max2i(min2i(i_max, tile_map_size_x*6-1), 0);
	j_min = max2i(min2i(j_min, tile_map_size_y*6), 0);
	j_max = max2i(min2i(j_max, tile_map_size_y*6-1), 0);

	i = min2i(max2i(i, i_min), i_max-1);
	j = min2i(max2i(j, j_min), j_max-1);
	
	h = 0.0f;
	while ((i >= i_min) && (j >= j_min) && (i < i_max) && (j < j_max))
	{
		h1 = get_tile_height(i, j);
		h2 = get_tile_height(i + 1, j);
		h3 = get_tile_height(i, j + 1);
		h4 = get_tile_height(i + 1, j + 1);
		h_max = max2f(max2f(h1, h2), max2f(h3, h4));
		h_min = min2f(min2f(h1, h2), min2f(h3, h4));
		tx = i*0.5f;
		ty = j*0.5f;
		box.bbmin[X] = tx;
		box.bbmin[Y] = ty;
		box.bbmin[Z] = h_min;
		box.bbmax[X] = tx+1.0f;
		box.bbmax[Y] = ty+1.0f;
		box.bbmax[Z] = h_max;
		if (click_line_bbox_intersection(box))
		{
			box.bbmin[X] = tx+0.0f;
			box.bbmin[Y] = ty+0.0f;
			box.bbmin[Z] = h1;
			box.bbmax[X] = tx+0.5f;
			box.bbmax[Y] = ty+0.5f;
			box.bbmax[Z] = h1;
			if (click_line_bbox_intersection(box))
			{
				h = h1;
				break;
			}
			box.bbmin[X] = tx+0.5f;
			box.bbmin[Y] = ty+0.0f;
			box.bbmin[Z] = h2;
			box.bbmax[X] = tx+1.0f;
			box.bbmax[Y] = ty+0.5f;
			box.bbmax[Z] = h2;
			if (click_line_bbox_intersection(box))
			{
				h = h2;
				break;
			}
			box.bbmin[X] = tx+0.0f;
			box.bbmin[Y] = ty+0.5f;
			box.bbmin[Z] = h3;
			box.bbmax[X] = tx+0.5f;
			box.bbmax[Y] = ty+1.0f;
			box.bbmax[Z] = h3;
			if (click_line_bbox_intersection(box))
			{
				h = h3;
				break;
			}
			box.bbmin[X] = tx+0.5f;
			box.bbmin[Y] = ty+0.5f;
			box.bbmin[Z] = h4;
			box.bbmax[X] = tx+1.0f;
			box.bbmax[Y] = ty+1.0f;
			box.bbmax[Z] = h4;
			if (click_line_bbox_intersection(box))
			{
				h = h4;
				break;
			}
		}
		if ((zx == 1) && (zy == 1)) break;
		if (zx == 0) t1 = (tx+sx-x)/dx;
		else t1 = 10e30;
		if (zx == 0) t2 = (ty+sy-y)/dy;
		else t2 = -10e30;
		if (t1 < t2) i += 2*sx;
		else j += 2*sy;
		
		t = min2f(t1, t2);
		x += dx*t;
		y += dy*t;
	}
	t = (h-click_line.center[Z])/click_line.direction[Z];
	*scene_x = (click_line.center[X]+click_line.direction[X]*t) / 0.5;
	*scene_y = (click_line.center[Y]+click_line.direction[Y]*t) / 0.5;
}

void Enter2DModeExtended(int width, int height)
{
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	if (use_fog) glDisable(GL_FOG);
	glPushAttrib(GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, width, height);

#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, (GLdouble)width, (GLdouble)height, 0.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void Enter2DMode()
{
	Enter2DModeExtended(window_width, window_height);
}

void Leave2DMode()
{
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
	glViewport(0, hud_y, window_width-hud_x, window_height-hud_y);
	if (use_fog) glEnable(GL_FOG);
	else glDisable(GL_FOG);
	//glViewport(0, 0, window_width-hud_x, window_height-hud_y);	// Reset The Current Viewport
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

/* The video modes start with index 1. So field 0 stands for video mode 1 */
video_mode_t video_modes[] = {
	{ 640,  480, 16, NULL, { 0, 0}}, /*  1 */
	{ 640,  480, 32, NULL, { 0, 0}}, /*  2 */
	{ 800,  600, 16, NULL, { 0, 0}}, /*  3 */
	{ 800,  600, 32, NULL, { 0, 0}}, /*  4 */
	{1024,  768, 16, NULL, { 0, 0}}, /*  5 */
	{1024,  768, 32, NULL, { 0, 0}}, /*  6 */
	{1152,  864, 16, NULL, { 0, 0}}, /*  7 */
	{1152,  864, 32, NULL, { 0, 0}}, /*  8 */
	{1280, 1024, 16, NULL, { 0, 0}}, /*  9 */
	{1280, 1024, 32, NULL, { 0, 0}}, /* 10 */
	{1600, 1200, 16, NULL, { 0, 0}}, /* 11 */
	{1600, 1200, 32, NULL, { 0, 0}}, /* 12 */
	{1280,  800, 16, NULL, { 0, 0}}, /* 13 */
	{1280,  800, 32, NULL, { 0, 0}}, /* 14 */
	{1440,  900, 16, NULL, { 0, 0}}, /* 15 */
	{1440,  900, 32, NULL, { 0, 0}}, /* 16 */
	{1680, 1050, 16, NULL, { 0, 0}}, /* 17 */
	{1680, 1050, 32, NULL, { 0, 0}}, /* 18 */
	{1400, 1050, 16, NULL, { 0, 0}}, /* 19 */
	{1400, 1050, 32, NULL, { 0, 0}}, /* 20 */
	{ 800,  480, 16, NULL, { 0, 0}}, /* 21 */
	{ 800,  480, 32, NULL, { 0, 0}}, /* 22 */
	{1920, 1200, 16, NULL, { 0, 0}}, /* 23 */
	{1920, 1200, 32, NULL, { 0, 0}}, /* 24 */
	{1024,  600, 16, NULL, { 0, 0}}, /* 25 */
	{1024,  600, 32, NULL, { 0, 0}}, /* 26 */
	{1920, 1080, 16, NULL, { 0, 0}}, /* 27 */
	{1920, 1080, 32, NULL, { 0, 0}}, /* 28 */
	{1366, 768, 16, NULL, { 0, 0}}, /* 29 */
	{1366, 768, 32, NULL, { 0, 0}}, /* 30 */
};
const int video_modes_count = sizeof(video_modes)/sizeof(*video_modes);

void build_video_mode_array()
{
	int i;
	int flags;

	if (full_screen)
		flags=SDL_OPENGL|SDL_FULLSCREEN;
	else
		flags=SDL_OPENGL;

	for(i = 0; i < video_modes_count; i++)
	{
		video_modes[i].flags.selected = 0;
		video_modes[i].flags.supported = 0;


		if(bpp == video_modes[i].bpp 
#ifdef WINDOWS
			|| full_screen
#endif
			) 
		{
			if (SDL_VideoModeOK(video_modes[i].width, video_modes[i].height, video_modes[i].bpp, flags))
				video_modes[i].flags.supported = 1;
		}

	}
	if (video_mode > 0)
		video_modes[video_mode-1].flags.selected=1;
}

void draw_console_pic(int which_texture)
{
#ifdef	NEW_TEXTURES
	bind_texture(which_texture);

	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);
	//draw the texture

	glTexCoord2f(0.0f, 0.0f);
	glVertex3i(0,0,0);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3i(0,window_height,0);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3i(window_width,window_height,0);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3i(window_width,0,0);

	glEnd();
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(which_texture);

	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);
	//draw the texture

	glTexCoord2f(0.0f,1.0f);
	glVertex3i(0,0,0);

	glTexCoord2f(0.0f,0.0f);
	glVertex3i(0,window_height,0);

	glTexCoord2f(1.0f,0.0f);
	glVertex3i(window_width,window_height,0);

	glTexCoord2f(1.0f,1.0f);
	glVertex3i(window_width,0,0);

	glEnd();
#endif	/* NEW_TEXTURES */
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void draw_2d_thing(float u_start,float v_start,float u_end,float v_end,int x_start,
				   int y_start,int x_end,int y_end)
{
	glTexCoord2f(u_start,v_end);
	glVertex3i(x_start,y_end,0);

	glTexCoord2f(u_start,v_start);
	glVertex3i(x_start,y_start,0);

	glTexCoord2f(u_end,v_start);
	glVertex3i(x_end,y_start,0);

	glTexCoord2f(u_end,v_end);
	glVertex3i(x_end,y_end,0);
}

void draw_2d_thing_r(float u_start,float v_start,float u_end,float v_end,int x_start,
				   int y_start,int x_end,int y_end)
{
	glTexCoord2f(u_start,v_start);
	glVertex3i(x_start,y_end,0);

	glTexCoord2f(u_end,v_start);
	glVertex3i(x_start,y_start,0);

	glTexCoord2f(u_end,v_end);
	glVertex3i(x_end,y_start,0);

	glTexCoord2f(u_start,v_end);
	glVertex3i(x_end,y_end,0);
}

void add_char_to_username(unsigned char ch)
{
	if (((ch>=48 && ch<=57) || (ch>=65 && ch<=90) || (ch>=97 && ch<=122) || (ch=='_'))
		&& username_text_length < MAX_USERNAME_LENGTH - 1)		// MAX_USERNAME_LENGTH includes the null terminator
	{
		username_str[username_text_length]=ch;
		username_str[username_text_length+1]=0;
		username_text_length++;
	}
	if(ch==SDLK_DELETE || ch==SDLK_BACKSPACE)
	{
		if (username_text_length > 0)
			username_text_length--;
		else
			username_text_length = 0;
		username_str[username_text_length] = '\0';
	}
}

void add_char_to_password(unsigned char ch)
{
	if ((ch>=32 && ch<=126) && password_text_length < MAX_USERNAME_LENGTH - 1)		// MAX_USERNAME_LENGTH includes the null terminator
	{
		password_str[password_text_length]=ch;
		display_password_str[password_text_length]='*';
		password_str[password_text_length+1]=0;
		display_password_str[password_text_length+1]=0;
		password_text_length++;
	}
	if (ch==SDLK_DELETE || ch==SDLK_BACKSPACE)
	{
		if (password_text_length > 0)
			password_text_length--;
		else
			password_text_length = 0;
		display_password_str[password_text_length] = '\0';
		password_str[password_text_length] = '\0';
	}
}

void draw_ingame_interface()
{
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
#ifdef	OLD_CLOSE_BAG
	// watch for closing a bag
	if(ground_items_win >= 0)
		{
			int	old_view= view_ground_items;

			view_ground_items= get_show_window(ground_items_win);
			// watch for telling the server we need to close the bag
			if(old_view && !view_ground_items)
				{
					unsigned char protocol_name;

					protocol_name= S_CLOSE_BAG;
					my_tcp_send(my_socket,&protocol_name,1);
				}
		}
#endif	//OLD_CLOSE_BAG

	glColor3f(1.0f, 1.0f, 1.0f);
	draw_hud_frame();

	//draw_hud_interface();
	display_spells_we_have();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

GLuint map_text;
static int cont_text = -1; // index in texture cache for continent map

GLuint inspect_map_text = 0;
int show_continent_map_boundaries = 1;
GLuint legend_text=0;
int cur_map;  //Is there a better way to do this?

#ifdef DEBUG_MAP_SOUND
int cur_tab_map = -1;
#endif // DEBUG_MAP_SOUND

#ifdef	NEW_TEXTURES
static const char* cont_map_file_names[] =
{
	"./maps/seridia",
	"./maps/irilion"
};
#else	/* NEW_TEXTURES */
static const char* cont_map_file_names[] = {
	"./maps/seridia.bmp",
	"./maps/irilion.bmp"
};
#endif	/* NEW_TEXTURES */
static const int nr_continents = sizeof (cont_map_file_names) / sizeof (const char *);
struct draw_map *continent_maps = NULL;

void read_mapinfo ()
{
	FILE *fin;
	char line[256];
	int maps_size, imap;
	char *cmt_pos;
	char cont_name[64];
	unsigned short continent, x_start, y_start, x_end, y_end;
	char map_name[128];
	
	maps_size = DEFAULT_CONTMAPS_SIZE;
	continent_maps = calloc (maps_size, sizeof (struct draw_map));
	imap = 0;
	
	fin = open_file_data ("mapinfo.lst", "r");
	if (fin == NULL){
		LOG_ERROR("%s: %s \"mapinfo.lst\": %s\n", reg_error_str, cant_open_file, strerror(errno));
	} else {
		while (fgets (line, sizeof (line), fin) != NULL)
		{
			// strip comments
			cmt_pos = strchr (line, '#');
			if (cmt_pos != NULL)
				*cmt_pos = '\0';

			if (sscanf (line, "%63s %hu %hu %hu %hu %127s ", cont_name, &x_start, &y_start, &x_end, &y_end, map_name) != 6)
				// not a valid line
				continue;

			if (strcasecmp (cont_name, "Seridia") == 0)
				continent = 0;
			else if (strcasecmp (cont_name, "Irilion") == 0)
				continent = 1;
			else
				// not a valid continent
				continue;

			if (imap >= maps_size - 1)
			{
				// Uh oh, we didn't allocate enough space
				maps_size += DEFAULT_CONTMAPS_SIZE;
				continent_maps = realloc (continent_maps, maps_size * sizeof (struct draw_map));
			}

			continent_maps[imap].cont = continent;
			continent_maps[imap].x_start = x_start;
			continent_maps[imap].y_start = y_start;
			continent_maps[imap].x_end = x_end;
			continent_maps[imap].y_end = y_end;
			continent_maps[imap].name = malloc ((strlen (map_name) + 1) * sizeof (char));
			strcpy(continent_maps[imap].name, map_name);
			imap++;
		}
		
		fclose (fin);
	}

	continent_maps[imap].cont = 0;
	continent_maps[imap].x_start = 0;
	continent_maps[imap].y_start = 0;
	continent_maps[imap].x_end = 0;
	continent_maps[imap].y_end = 0;
	continent_maps[imap].name = NULL;
}

int switch_to_game_map()
{
#ifdef	NEW_TEXTURES
	char buffer[1024];
#else	/* NEW_TEXTURES */
	int len;
	texture_cache_struct tex;
#endif	/* NEW_TEXTURES */
	short int cur_cont;
	static short int old_cont = -1;
	
	/* check we loaded the mapinfo data */
	if (continent_maps == NULL || continent_maps[0].name == NULL)
	{
		LOG_TO_CONSOLE(c_yellow2,err_nomap_str);
		return 0;
	}
	
#ifdef	NEW_TEXTURES
	if (check_image_name(map_file_name, sizeof(buffer), buffer) == 1)
	{
		map_text = load_texture_cached(buffer, tt_image);
	}
	else
	{
		map_text = 0;
	}
#else	/* NEW_TEXTURES */
	my_strcp(tex.file_name,map_file_name);
	len=strlen(tex.file_name);
	tex.file_name[len-3]='b';
	tex.file_name[len-2]='m';
	tex.file_name[len-1]='p';
	tex.alpha = 128;
	if (!el_file_exists(tex.file_name))
		map_text = 0;
	else
		map_text=load_bmp8_fixed_alpha(&tex, tex.alpha);
#endif	/* NEW_TEXTURES */
	if(!map_text)
	{
		LOG_TO_CONSOLE(c_yellow2,err_nomap_str);
		return 0;
	}
	
	if (cur_map < 0)
	{
		cur_cont = -1;
	}
	else
	{
		cur_cont = continent_maps[cur_map].cont;
	}
	if (cur_cont != old_cont && cur_cont >= 0 && cur_cont < nr_continents)
	{
#ifdef	NEW_TEXTURES
		cont_text = load_texture_cached (cont_map_file_names[cur_cont], tt_image);
#else	/* NEW_TEXTURES */
		cont_text = load_texture_cache (cont_map_file_names[cur_cont], 128);
#endif	/* NEW_TEXTURES */
		old_cont = cur_cont;
	}
#ifdef DEBUG_MAP_SOUND
	cur_tab_map = cur_map;
#endif // DEBUG_MAP_SOUND
	
	if(current_cursor != CURSOR_ARROW)
	{
		change_cursor(CURSOR_ARROW);
	}
	return 1;
}

void switch_from_game_map()
{
#ifndef	NEW_TEXTURES
	glDeleteTextures(1,&map_text);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
#endif	/* NEW_TEXTURES */
}

static void draw_mark_filter(void)
{
	int screen_x=0;
	int screen_y=0;

	// display the Mark filter title
	glColor3f(1.0f,1.0f,0.0f);
	screen_x = 25 - 1.5*strlen(label_mark_filter);
	screen_y = 150 + 22;
	draw_string_zoomed(screen_x, screen_y, (unsigned char*)label_mark_filter, 1, 0.3);
	
	// if filtering marks, display the label and the current filter text
	if (mark_filter_active) {
		char * show_mark_filter_text;
		int max_show_len = 15;
		if (strlen(mark_filter_text) > max_show_len)
		  show_mark_filter_text = &mark_filter_text[strlen(mark_filter_text)-max_show_len];
		else if (strlen(mark_filter_text) == 0)
			show_mark_filter_text = "_";
		else
		  show_mark_filter_text = mark_filter_text;
		screen_x = 25 - 1.5*strlen(show_mark_filter_text);
		screen_y = 150 + 29;
		draw_string_zoomed(screen_x, screen_y, (unsigned char*)show_mark_filter_text, 1, 0.3);
	}
	// display which key to activate the filter
	else
	{
		char buf[20];
		get_key_string(K_MARKFILTER, buf, sizeof(buf));
		draw_string_zoomed(25 - 1.5*strlen(buf), 150 + 29, (const unsigned char *)buf, 1, 0.3);
	}
}

static void draw_marks(marking *the_marks, int the_max_mark, int the_tile_map_size_x, int the_tile_map_size_y)
{
	size_t i;
	int screen_x=0;
	int screen_y=0;

	// crave the markings
	for(i=0;i<the_max_mark;i++)
	 {
		int x = the_marks[i].x;
		int y = the_marks[i].y;
		if ( x > 0 ) {

			// if filtering marks, don't display if it doesn't match the current filter
			if (mark_filter_active
				  && (get_string_occurance(mark_filter_text, the_marks[i].text, strlen(the_marks[i].text), 1) == -1))
				continue;

			screen_x=(51+200*x/(the_tile_map_size_x*6));
			screen_y=201-200*y/(the_tile_map_size_y*6);

			if(!the_marks[i].server_side) glColor3f((float)the_marks[i].r/255,(float)the_marks[i].g/255,(float)the_marks[i].b/255);//glColor3f(0.4f,1.0f,0.0f);
			else glColor3f(0.33f,0.6f,1.0f);
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINES);
				glVertex2i(screen_x-9*mapmark_zoom,screen_y-9*mapmark_zoom);
				glVertex2i(screen_x+6*mapmark_zoom,screen_y+6*mapmark_zoom);
			
				glVertex2i(screen_x+6*mapmark_zoom,screen_y-9*mapmark_zoom);
				glVertex2i(screen_x-9*mapmark_zoom,screen_y+6*mapmark_zoom);
			glEnd();
				glEnable(GL_TEXTURE_2D);
				if(!the_marks[i].server_side) glColor3f((float)the_marks[i].r/255,(float)the_marks[i].g/255,(float)the_marks[i].b/255);//glColor3f(0.2f,1.0f,0.0f);
				else glColor3f(0.33f,0.6f,1.0f);
			draw_string_zoomed(screen_x, screen_y, (unsigned char*)the_marks[i].text, 1, mapmark_zoom);
		}
	}
}

void draw_coordinates(int the_tile_map_size_x, int the_tile_map_size_y)
{
	int screen_x=0;
	int screen_y=0;
	int map_x, map_y;

	// draw coordinates
	if (pf_get_mouse_position_extended(mouse_x, mouse_y, &map_x, &map_y, the_tile_map_size_x, the_tile_map_size_y)) {
		// we're pointing on the map, display position
		char buf[10];
		safe_snprintf(buf, sizeof(buf), "%d,%d", map_x, map_y);
		glColor3f(1.0f,1.0f,0.0f);
		screen_x = 25 - 1.5*strlen(buf);
		screen_y = 150 + 8;
		draw_string_zoomed(screen_x, screen_y, (unsigned char*)buf, 1, 0.3);
		screen_x = 25 - 1.5*strlen(label_cursor_coords);
		screen_y = 150 + 1;
		draw_string_zoomed(screen_x, screen_y, (unsigned char*)label_cursor_coords, 1, 0.3);
	}
}

void draw_game_map (int map, int mouse_mini)
{     
	int screen_x=0;
	int screen_y=0;
	int x=-1,y=-1;
	float x_size=0,y_size=0;
	GLuint map_small, map_large;
	actor *me;
	static int fallback_text = -1;

	// if we don't have a continent texture (instance may be), fallback to blank paper
	if (cont_text < 0)
	{
		if (fallback_text < 0)
#ifdef	NEW_TEXTURES
		{
			fallback_text = load_texture_cached("./textures/paper1", tt_gui);
		}
#else	/* NEW_TEXTURES */
			fallback_text = load_texture_cache ("./textures/paper1.bmp", 0);
#endif	/* NEW_TEXTURES */
		cont_text = fallback_text;
	}
	
	if(map){
#ifdef	NEW_TEXTURES
		map_small = cont_text;
#else	/* NEW_TEXTURES */
		map_small=get_texture_id(cont_text);
#endif	/* NEW_TEXTURES */
		if(inspect_map_text == 0) {
			map_large=map_text;
		} else {
			map_large = inspect_map_text;
		}
	} else {
		map_small=map_text;
#ifdef	NEW_TEXTURES
		map_large = cont_text;
#else	/* NEW_TEXTURES */
		map_large=get_texture_id(cont_text);
#endif	/* NEW_TEXTURES */
		if(cur_map!=-1){
			x_size = ((float)(continent_maps[cur_map].x_end - continent_maps[cur_map].x_start)) / tile_map_size_x;
			y_size = ((float)(continent_maps[cur_map].y_end - continent_maps[cur_map].y_start)) / tile_map_size_y;
		} else {
			x_size=y_size=0;
		}
	}
	
   	glDisable(GL_DEPTH_TEST);
   	glDisable(GL_LIGHTING);
    
	glViewport(0, 0 + hud_y, window_width-hud_x, window_height-hud_y);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glOrtho(300, (GLdouble)0, (GLdouble)0, 200, -250.0, 250.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.0f, 0.0f, 0.0f);

	glBegin(GL_QUADS);
		glVertex2i(0,   0);	
		glVertex2i(300, 0);
		glVertex2i(300, 200);
		glVertex2i(0,   200);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0f,1.0f,1.0f);
    	
#ifdef	NEW_TEXTURES
	bind_texture(map_large);

	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3i(50,0,0); 
		glTexCoord2f(1.0f, 0.0f); glVertex3i(50,200,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3i(250,200,0);
		glTexCoord2f(0.0f, 1.0f); glVertex3i(250,0,0);
	glEnd();
#else	/* NEW_TEXTURES */
	bind_texture_id(map_large);

	glBegin(GL_QUADS);
		glTexCoord2f(1.0f,0.0f); glVertex3i(50,0,0); 
		glTexCoord2f(1.0f,1.0f); glVertex3i(50,200,0);
		glTexCoord2f(0.0f,1.0f); glVertex3i(250,200,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(250,0,0);
	glEnd();
#endif	/* NEW_TEXTURES */

	if (mouse_mini)
		glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
	else
		glColor4f (0.7f, 0.7f, 0.7f, 0.7f);
    	
	glEnable(GL_ALPHA_TEST);
	
#ifdef	NEW_TEXTURES
	bind_texture(map_small);

    	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3i(250,150,0);
		glTexCoord2f(1.0f, 0.0f); glVertex3i(250,200,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3i(300,200,0);
		glTexCoord2f(0.0f, 1.0f); glVertex3i(300,150,0);
	glEnd();
#else	/* NEW_TEXTURES */
	bind_texture_id(map_small);

    	glBegin(GL_QUADS);
		glTexCoord2f(1.0f,0.0f); glVertex3i(250,150,0);
		glTexCoord2f(1.0f,1.0f); glVertex3i(250,200,0);
		glTexCoord2f(0.0f,1.0f); glVertex3i(300,200,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(300,150,0);
	glEnd();
#endif	/* NEW_TEXTURES */
	
	glDisable(GL_ALPHA_TEST);
	
	glColor3f(1.0f,1.0f,1.0f);
    	
#ifdef	NEW_TEXTURES
	bind_texture(legend_text);

    	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3i(250,50,0);
		glTexCoord2f(1.0f, 0.0f); glVertex3i(250,150,0);
		glTexCoord2f(0.0f, 0.0f); glVertex3i(300,150,0);
		glTexCoord2f(0.0f, 1.0f); glVertex3i(300,50,0);
	glEnd();
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(legend_text);

    	glBegin(GL_QUADS);
		glTexCoord2f(1.0f,0.0f); glVertex3i(250,50,0);
		glTexCoord2f(1.0f,1.0f); glVertex3i(250,150,0);
		glTexCoord2f(0.0f,1.0f); glVertex3i(300,150,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(300,50,0);
	glEnd();
#endif	/* NEW_TEXTURES */

// this is necessary for the text over map
// need to execute this for any map now
// because of the coordinate display - Lachesis
	if(map/*&&(adding_mark||max_mark>0)*/){
   		glViewport(0, 0 + hud_y, window_width-hud_x, window_height-hud_y);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho((GLdouble)0, (GLdouble)300, (GLdouble)200, (GLdouble)0, -250.0, 250.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// Draw help for toggling the mini-map
		{
			char buf[80];
			char keybuf[20];
			glEnable(GL_TEXTURE_2D);
			safe_snprintf(buf, sizeof(buf), "%s %s", win_minimap, get_key_string(K_MINIMAP, keybuf, sizeof(keybuf)));
			glColor3f (1.0f, 1.0f, 0.0f);
			draw_string_zoomed(25 - 1.5*strlen(buf), 150 + 43, (const unsigned char *)buf, 1, 0.3);
		}
 
		// draw a temporary mark until the text is entered
		if (adding_mark) {
			int x = mark_x;
			int y = mark_y;

			screen_x=(51+200*x/(tile_map_size_x*6));
			screen_y=201-200*y/(tile_map_size_y*6);

			glColor3f(1.0f,1.0f,0.0f);
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINES);
				glVertex2i(screen_x-9*mapmark_zoom,screen_y-9*mapmark_zoom);
				glVertex2i(screen_x+6*mapmark_zoom,screen_y+6*mapmark_zoom);

				glVertex2i(screen_x+6*mapmark_zoom,screen_y-9*mapmark_zoom);
				glVertex2i(screen_x-9*mapmark_zoom,screen_y+6*mapmark_zoom);
			glEnd();
		        glEnable(GL_TEXTURE_2D);
		        glColor3f(1.0f,1.0f,0.0f);
			draw_string_zoomed (screen_x, screen_y, (unsigned char*)input_text_line.data, 1, mapmark_zoom);
		}

		draw_mark_filter();
		if(inspect_map_text == 0) {
			draw_marks(marks, max_mark, tile_map_size_x, tile_map_size_y);
			draw_coordinates(tile_map_size_x, tile_map_size_y);
		}
		else {
			draw_marks(temp_marks, max_temp_mark, temp_tile_map_size_x, temp_tile_map_size_y);
			draw_coordinates(temp_tile_map_size_x, temp_tile_map_size_y);
		}
	}

	//if we're following a path, draw the destination on the map
	if (pf_follow_path && inspect_map_text == 0)
	{
		int px = pf_dst_tile->x;
		int py = pf_dst_tile->y;

		if (!map)
		{
			if (cur_map!=-1)
			{
				screen_x = 300 - (50 + 200 * ( (px * x_size / 6) + continent_maps[cur_map].x_start) / 512);
				screen_y = 200 * ( (py * y_size / 6) + continent_maps[cur_map].y_start) / 512;
			}
			else
			{
				screen_x = screen_y = 0;
			}
		} 
		else
		{
			screen_x=51 +200*px/(tile_map_size_x*6);
			screen_y=201-200*py/(tile_map_size_y*6);
		}

		glColor3f(1.0f,0.0f,0.0f);
		
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);

		glVertex2i(screen_x-9*mapmark_zoom,screen_y-9*mapmark_zoom);
		glVertex2i(screen_x+6*mapmark_zoom,screen_y+6*mapmark_zoom);

		glVertex2i(screen_x+6*mapmark_zoom,screen_y-9*mapmark_zoom);
		glVertex2i(screen_x-9*mapmark_zoom,screen_y+6*mapmark_zoom);

		glEnd();
	}
	
	//ok, now let's draw our possition...
	if ( (me = get_our_actor ()) != NULL && inspect_map_text == 0)
	{
		x = me->x_tile_pos;
		y = me->y_tile_pos;
	}
	else
	{
		//We don't exist (usually happens when teleporting)
		x = -1;
		y = -1;
	}

	if (!map)
	{
		if (cur_map != -1)
		{
			screen_x = 300 - (50 + 200 * ( (x * x_size / 6) + continent_maps[cur_map].x_start) / 512);
			screen_y = 200 * ( (y * y_size / 6) + continent_maps[cur_map].y_start) / 512;
		}
		else
		{
			screen_x = screen_y = 0;
		}
	} 
	else 
	{
		screen_x=51 +200*x/(tile_map_size_x*6);
		screen_y=201-200*y/(tile_map_size_y*6);
	}
	
	if ( (map || !dungeon) && x != -1 )
	{
		glColor3f (0.0f, 0.0f, 1.0f);
		glDisable (GL_TEXTURE_2D);
		glPushAttrib(GL_LINE_BIT);
		glLineWidth(2.0);
		glEnable(GL_LINE_SMOOTH);
		glBegin (GL_LINES);

		glVertex2i(screen_x-9*mapmark_zoom,screen_y-9*mapmark_zoom);
		glVertex2i(screen_x+6*mapmark_zoom,screen_y+6*mapmark_zoom);

		glVertex2i(screen_x+6*mapmark_zoom,screen_y-9*mapmark_zoom);
		glVertex2i(screen_x-9*mapmark_zoom,screen_y+6*mapmark_zoom);

		glEnd();
		glPopAttrib();
	}

	if(!map && show_continent_map_boundaries && cont_text!=fallback_text) {
		int i;
		/* Convert mouse coordinates to map coordinates (stolen from pf_get_mouse_position()) */
		int min_mouse_x = (window_width-hud_x)/6;
		int min_mouse_y = 0;
		int max_mouse_x = min_mouse_x+((window_width-hud_x)/1.5);
		int max_mouse_y = window_height - hud_y;
		int screen_map_width = max_mouse_x - min_mouse_x;
		int screen_map_height = max_mouse_y - min_mouse_y;
		int m_px = ((mouse_x-min_mouse_x) * 512) / screen_map_width;
		int m_py = 512 - ((mouse_y * 512) / screen_map_height);
		int mouse_over = -1;

		glColor3f (0.267f, 0.267f, 0.267f);
		glDisable (GL_TEXTURE_2D);
		glBegin(GL_LINES);
		/* Draw borders for the maps except the one with the mouse over it */
		glColor3f (0.267f, 0.267f, 0.267f);
		for(i = 0; continent_maps[i].name != NULL; i++) {
			if(continent_maps[i].cont == continent_maps[cur_map].cont) {
				if(!mouse_mini && mouse_over == -1
				&& m_px > continent_maps[i].x_start && m_px < continent_maps[i].x_end
				&& m_py > continent_maps[i].y_start && m_py < continent_maps[i].y_end)
				{
					/* Mouse over this map */
					mouse_over = i;
				} else {
					int x_start = 300-(50+200*continent_maps[i].x_start/512);
					int x_end = 300-(50+200*continent_maps[i].x_end/512);
					int y_start = 200*continent_maps[i].y_start / 512;
					int y_end = 200*continent_maps[i].y_end / 512;
					
					glVertex2i(x_start, y_start);
					glVertex2i(x_start, y_end);

					glVertex2i(x_start, y_end);
					glVertex2i(x_end, y_end);

					glVertex2i(x_end, y_end);
					glVertex2i(x_end, y_start);

					glVertex2i(x_end, y_start);
					glVertex2i(x_start, y_start);
				}
			}
		}
		/* Draw border for the map with the mouse over it */
		if(mouse_over >= 0) {
			float flash_effect_colour = 0.90f - sin((float)SDL_GetTicks()/100.0f) / 10.0f;
			int x_start = 300-(50+200*continent_maps[mouse_over].x_start/512);
			int x_end = 300-(50+200*continent_maps[mouse_over].x_end/512);
			int y_start = 200*continent_maps[mouse_over].y_start / 512;
			int y_end = 200*continent_maps[mouse_over].y_end / 512;

			glColor3f(flash_effect_colour, flash_effect_colour, flash_effect_colour);
			glVertex2i(x_start, y_start);
			glVertex2i(x_start, y_end);

			glVertex2i(x_start, y_end);
			glVertex2i(x_end, y_end);

			glVertex2i(x_end, y_end);
			glVertex2i(x_end, y_start);

			glVertex2i(x_end, y_start);
			glVertex2i(x_start, y_start);
		}
		glEnd();
	}

#ifdef DEBUG_MAP_SOUND
	// If we are in map view (not continent view) draw the sound area boundaries
	if (map) {
		print_sound_boundaries(cur_tab_map);
	}
#endif // DEBUG_MAP_SOUND

	if (map)
	{
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	glEnable (GL_TEXTURE_2D);
	glColor3f (1.0f, 1.0f, 1.0f);

	glMatrixMode (GL_MODELVIEW);
	glPopMatrix ();
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
#ifdef OPENGL_TRACE
	CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


int put_mark_on_position(int map_x, int map_y, char * name)
{
		if (map_x < 0
		|| map_x >= tile_map_size_x*6
		|| map_y < 0
		|| map_y >= tile_map_size_y*6)
		{
			return 0;
		}
		if (max_mark>=MAX_USER_MARKS)
		{
			LOG_TO_CONSOLE(c_red2, err_mapmarks_str);
			return 0;
		}
		marks[max_mark].x = map_x;
		marks[max_mark].y = map_y;
		memset(marks[max_mark].text,0,sizeof(marks[max_mark].text));
		
		my_strncp(marks[max_mark].text,name,sizeof(marks[max_mark].text));
		marks[max_mark].text[strlen(marks[max_mark].text)]=0;

		marks[max_mark].server_side=0;

		marks[max_mark].r=curmark_r;
		marks[max_mark].g=curmark_g;
		marks[max_mark].b=curmark_b;
		
		max_mark++;
		save_markings();
		return 1;
}

void put_mark_on_map_on_mouse_position()
{
		if (pf_get_mouse_position(mouse_x, mouse_y, &mark_x, &mark_y)) {
				/* Lachesis: reusing available code from pathfinder.c
        int min_mouse_x = (window_width-hud_x)/6;
        int min_mouse_y = 0;

        int max_mouse_x = min_mouse_x+((window_width-hud_x)/1.5);
        int max_mouse_y = window_height - hud_y;

        int screen_map_width = max_mouse_x - min_mouse_x;
        int screen_map_height = max_mouse_y - min_mouse_y;

        // FIXME (Malaclypse): should be moved above the screen_map_* init, to avoid additional computation
        if (mouse_x < min_mouse_x
        || mouse_x > max_mouse_x
        || mouse_y < min_mouse_y
        || mouse_y > max_mouse_y) {
                return;
        }

        mark_x = ((mouse_x - min_mouse_x) * tile_map_size_x * 6) / screen_map_width;
        mark_y = (tile_map_size_y * 6) - ((mouse_y * tile_map_size_y * 6) / screen_map_height);
				*/
        adding_mark = 1;
		}
}
int put_mark_on_current_position(char *name)
{
	actor *me = get_our_actor ();

	if (me != NULL)
	{	
		if (put_mark_on_position(me->x_tile_pos, me->y_tile_pos, name))
			return 1;
		/* Lachesis: reusing available code
		marks[max_mark].x = me->x_tile_pos;
		marks[max_mark].y = me->y_tile_pos;
		memset(marks[max_mark].text,0,500);
		
		my_strncp(marks[max_mark].text,name,500);
		marks[max_mark].text[strlen(marks[max_mark].text)]=0;
		max_mark++;
		save_markings();
		*/
	}
	return 0;		
}

void delete_mark_on_map_on_mouse_position()
{
	int min_mouse_x = (window_width-hud_x)/6;
	int min_mouse_y = 0;
	int mx , my , i;
	int max_mouse_x = min_mouse_x+((window_width-hud_x)/1.5);
	int max_mouse_y = window_height - hud_y;

	int screen_map_width = max_mouse_x - min_mouse_x;
	int screen_map_height = max_mouse_y - min_mouse_y;

	int min_distance; 
	marking * closest_mark;

	// FIXME (Malaclypse): should be moved above the screen_map_* init, to avoid additional computation
	if (mouse_x < min_mouse_x
	|| mouse_x > max_mouse_x
	|| mouse_y < min_mouse_y
	|| mouse_y > max_mouse_y) {
		return;
	}

	mx = ((mouse_x - min_mouse_x) * tile_map_size_x * 6) / screen_map_width;
	my = (tile_map_size_y * 6) - ((mouse_y * tile_map_size_y * 6) / screen_map_height);

	// delete mark closest to cursor
	min_distance = 20*20; // only check close marks
	closest_mark = NULL;
	for ( i = 0 ; i < max_mark ; i ++ ) {
		int distance, dx, dy;
		marking * const mark = &marks[i]; 

		// skip masked marks
		if (mark->x < 0 || mark->server_side) continue;
		// get mark to cursor distance (squared)
		dx = mark->x - mx;
		dy = mark->y - my;
		distance = dx*dx + dy*dy;

		// prefer deleting closer and newer marks
		if (distance <= min_distance) {
			// found a closer mark
			closest_mark = mark;
			min_distance = distance;
		}
	}

	if (closest_mark != NULL) {
		// we found a close mark
		closest_mark->x =  -1 ;
		closest_mark->y =  -1 ;
	}

	save_markings();
	load_map_marks(); // simply to compact the array and make room for new marks

}


void hide_all_root_windows ()
{
	if (game_root_win >= 0) hide_window (game_root_win);
	if (console_root_win >= 0) hide_window (console_root_win);
	if (map_root_win >= 0) hide_window (map_root_win);
	if (login_root_win >= 0) hide_window (login_root_win);
	if (rules_root_win >= 0) hide_window (rules_root_win);
	if (opening_root_win >= 0) hide_window (opening_root_win);
	if (newchar_root_win >= 0) hide_window (newchar_root_win);
	if (update_root_win >= 0) hide_window (update_root_win);
	if (langsel_rootwin >= 0) hide_window (langsel_rootwin);
}

void resize_all_root_windows (Uint32 w, Uint32 h)
{
	if (game_root_win >= 0) resize_window (game_root_win, w, h);
	if (console_root_win >= 0) resize_window (console_root_win, w, h);
	if (map_root_win >= 0) resize_window (map_root_win, w, h);
	if (login_root_win >= 0) resize_window (login_root_win, w, h);
	if (rules_root_win >= 0) resize_window (rules_root_win, w, h);
	if (opening_root_win >= 0) resize_window (opening_root_win, w, h);
	if (newchar_root_win >= 0) resize_window (newchar_root_win, w, h);
	if (update_root_win >= 0) resize_window (update_root_win, w, h);
	if (input_widget->window_id != chat_win) {
		widget_resize (input_widget->window_id, input_widget->id, w-HUD_MARGIN_X, input_widget->len_y);
		widget_move (input_widget->window_id, input_widget->id, 0, h-input_widget->len_y-HUD_MARGIN_Y);
	}
#ifdef NEW_NEW_CHAR_WINDOW
	resize_newchar_hud_window();
#endif
}

/* currently UNUSED
void draw_menu_title_bar(int x, int y, int x_len)
{
	float u_first_start=(float)31/256;
	float u_first_end=0;
	float v_first_start=1.0f-(float)160/256;
	float v_first_end=1.0f-(float)175/256;

	float u_middle_start=(float)32/256;
	float u_middle_end=(float)63/256;
	float v_middle_start=1.0f-(float)160/256;
	float v_middle_end=1.0f-(float)175/256;

	float u_last_start=0;
	float u_last_end=(float)31/256;
	float v_last_start=1.0f-(float)160/256;
	float v_last_end=1.0f-(float)175/256;

	int segments_no;
	int i;

	glColor3f(1.0f,1.0f,1.0f);
	//ok, now draw that shit...
	segments_no=x_len/32;

	get_and_set_texture_id(icons_text);
	glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.03f);
	glBegin(GL_QUADS);

	glTexCoord2f(u_first_end,v_first_start);
	glVertex3i(x,y,0);
	glTexCoord2f(u_first_end,v_first_end);
	glVertex3i(x,y+16,0);
	glTexCoord2f(u_first_start,v_first_end);
	glVertex3i(x+32,y+16,0);
	glTexCoord2f(u_first_start,v_first_start);
	glVertex3i(x+32,y,0);

	for(i=1;i<segments_no-1;i++)
		{
			glTexCoord2f(u_middle_end,v_middle_start);
			glVertex3i(x+i*32,y,0);
			glTexCoord2f(u_middle_end,v_middle_end);
			glVertex3i(x+i*32,y+16,0);
			glTexCoord2f(u_middle_start,v_middle_end);
			glVertex3i(x+i*32+32,y+16,0);
			glTexCoord2f(u_middle_start,v_middle_start);
			glVertex3i(x+i*32+32,y,0);
		}

	glTexCoord2f(u_last_end,v_last_start);
	glVertex3i(x+i*32,y,0);
	glTexCoord2f(u_last_end,v_last_end);
	glVertex3i(x+i*32,y+16,0);
	glTexCoord2f(u_last_start,v_last_end);
	glVertex3i(x+i*32+32,y+16,0);
	glTexCoord2f(u_last_start,v_last_start);
	glVertex3i(x+i*32+32,y,0);

	glEnd();
	glDisable(GL_ALPHA_TEST);
}
*/

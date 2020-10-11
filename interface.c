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
#include "elconfig.h"
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
#include "image_loading.h"
#include "weather.h"

#define DEFAULT_CONTMAPS_SIZE 20

int mouse_x = 0;
int mouse_y = 0;
int right_click = 0;
int middle_click = 0;
int left_click = 0;

dynamic_banner_colour_def dynamic_banner_colour =
{
	.yourself = 1,
	.other_players = 1,
	.creatures = 1
};

int have_a_map=0;
int view_health_bar=1;
int view_ether_bar=0;
int view_names=1;
int view_hp=0;
int view_ether=0;
int view_chat_text_as_overtext=0;
int view_mode_instance=0;
float view_mode_instance_banner_height=5.0f;
float view_mode_instance_damage_height=5.0f;

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

Uint32 click_time=0;

int small_map_screen_x_left = 0;
int small_map_screen_x_right = 0;
int small_map_screen_y_top = 0;
int small_map_screen_y_bottom = 0;
int main_map_screen_x_left = 0;
int main_map_screen_x_right = 0;
int main_map_screen_y_top = 0;
int main_map_screen_y_bottom = 0;

static float map_font_scale_fac = 1.0f;

static GLdouble model_mat[16];
static GLdouble projection_mat[16];
static GLint viewport[4];

// Grum: attempt to work around bug in Ati linux drivers.
int ati_click_workaround = 0;

void save_scene_matrix (void)
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

void Enter2DMode(void)
{
	Enter2DModeExtended(window_width, window_height);
}

void Leave2DMode(void)
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
	{ 640,  480, 16, NULL}, /*  1 */
	{ 640,  480, 32, NULL}, /*  2 */
	{ 800,  600, 16, NULL}, /*  3 */
	{ 800,  600, 32, NULL}, /*  4 */
	{1024,  768, 16, NULL}, /*  5 */
	{1024,  768, 32, NULL}, /*  6 */
	{1152,  864, 16, NULL}, /*  7 */
	{1152,  864, 32, NULL}, /*  8 */
	{1280, 1024, 16, NULL}, /*  9 */
	{1280, 1024, 32, NULL}, /* 10 */
	{1600, 1200, 16, NULL}, /* 11 */
	{1600, 1200, 32, NULL}, /* 12 */
	{1280,  800, 16, NULL}, /* 13 */
	{1280,  800, 32, NULL}, /* 14 */
	{1440,  900, 16, NULL}, /* 15 */
	{1440,  900, 32, NULL}, /* 16 */
	{1680, 1050, 16, NULL}, /* 17 */
	{1680, 1050, 32, NULL}, /* 18 */
	{1400, 1050, 16, NULL}, /* 19 */
	{1400, 1050, 32, NULL}, /* 20 */
	{ 800,  480, 16, NULL}, /* 21 */
	{ 800,  480, 32, NULL}, /* 22 */
	{1920, 1200, 16, NULL}, /* 23 */
	{1920, 1200, 32, NULL}, /* 24 */
	{1024,  600, 16, NULL}, /* 25 */
	{1024,  600, 32, NULL}, /* 26 */
	{1920, 1080, 16, NULL}, /* 27 */
	{1920, 1080, 32, NULL}, /* 28 */
	{1366, 768, 16, NULL}, /* 29 */
	{1366, 768, 32, NULL}, /* 30 */
	{2560, 1440, 16, NULL}, /* 31 */
	{2560, 1440, 32, NULL}, /* 32 */
	{3840, 2160, 16, NULL}, /* 33 */
	{3840, 2160, 32, NULL}, /* 34 */
};
const int video_modes_count = sizeof(video_modes)/sizeof(*video_modes);

void draw_console_pic(int which_texture)
{
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

GLuint map_text;
static int cont_text = -1; // index in texture cache for continent map

GLuint inspect_map_text = 0;
int show_continent_map_boundaries = 1;
GLuint legend_text=0;
int cur_map;  //Is there a better way to do this?

#ifdef DEBUG_MAP_SOUND
int cur_tab_map = -1;
#endif // DEBUG_MAP_SOUND

typedef struct
{
	char* name;
	char* file_name;
} cont_overview_maps_t;
static cont_overview_maps_t * cont_overview_maps = NULL;
static int nr_continents = 0;
struct draw_map *continent_maps = NULL;

/*	Free allocated memory during exit
*/
void cleanup_mapinfo(void)
{
	size_t i;

	LOG_INFO("cont_overview_maps[]");
	for (i = 0; i < nr_continents; i++)
	{
		free(cont_overview_maps[i].name);
		free(cont_overview_maps[i].file_name);
	}
	LOG_INFO("cont_overview_maps");
	free(cont_overview_maps);
	cont_overview_maps = NULL;
	nr_continents = 0;

	LOG_INFO("continent_maps[]");
	for (i = 0; continent_maps[i].name; i++)
		free(continent_maps[i].name);
	LOG_INFO("continent_maps");
	free (continent_maps);
	continent_maps = NULL;
}

//	Read the list of continent maps from file or fallback to original hardwired list.
//
static void read_cont_info(void)
{
	FILE *fin;
	char line[256];

	fin = open_file_data ("continfo.lst", "r");

	// if the file does not exist, fall back to hardwired continents
	if (fin == NULL)
	{
		size_t i;
		cont_overview_maps_t tmp_cont_maps[] = { { "Seridia", "./maps/seridia" }, { "Irilion", "./maps/irilion" } };
		nr_continents = sizeof(tmp_cont_maps) / sizeof(cont_overview_maps_t);
		cont_overview_maps = malloc(nr_continents * sizeof(cont_overview_maps_t));
		for (i = 0; i < nr_continents; i++)
		{
			cont_overview_maps[i].name = malloc(strlen(tmp_cont_maps[i].name) + 1);
			strcpy(cont_overview_maps[i].name, tmp_cont_maps[i].name);
			cont_overview_maps[i].file_name = malloc(strlen(tmp_cont_maps[i].file_name) + 1);
			strcpy(cont_overview_maps[i].file_name, tmp_cont_maps[i].file_name);
		}
		LOG_INFO("Using hardwired continent overview maps: %d", nr_continents);
	}

	// else, if the file exists read one contenent definition per line, "<continent name> <map image file name>"
	else
	{
		char name[256], file_name[256], tmp[1];
		nr_continents = 0;
		while (fgets (line, sizeof (line), fin) != NULL)
		{
			// strip comments, the # and anything after is ignored
			char *cmt_pos = strchr(line, '#');
			if (cmt_pos != NULL)
				*cmt_pos = '\0';
			// only use lines with exactly 2 strings, both stripped of leading and trailing space
			if (sscanf (line, "%s %s %s", name, file_name, tmp) != 2)
				continue;
			cont_overview_maps = realloc(cont_overview_maps, (nr_continents + 1) * sizeof(cont_overview_maps_t));
			cont_overview_maps[nr_continents].name = malloc(strlen(name) + 1);
			strcpy(cont_overview_maps[nr_continents].name, name);
			cont_overview_maps[nr_continents].file_name = malloc(strlen(file_name) + 1);
			strcpy(cont_overview_maps[nr_continents].file_name, file_name);
			//printf("%d: name=[%s] file_name=[%s]\n", nr_continents, cont_overview_maps[nr_continents].name, cont_overview_maps[nr_continents].file_name);
			nr_continents++;
		}
		fclose (fin);
		LOG_INFO("Using file defined continent overview maps: count %d", nr_continents);
	}
}

void read_mapinfo (void)
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

	read_cont_info();
	fin = open_file_data ("mapinfo.lst", "r");
	if (fin == NULL){
		LOG_ERROR("%s: %s \"mapinfo.lst\": %s\n", reg_error_str, cant_open_file, strerror(errno));
	} else {
		while (fgets (line, sizeof (line), fin) != NULL)
		{
			size_t i;
			int cont_found = 0;
			char weather_name[11] = "";

			// strip comments
			cmt_pos = strchr (line, '#');
			if (cmt_pos != NULL)
				*cmt_pos = '\0';

			// weather_name is optional so a valid line is 6 or more parameters read
			if (sscanf (line, "%63s %hu %hu %hu %hu %127s %10s", cont_name, &x_start, &y_start, &x_end, &y_end, map_name, weather_name) < 6)
				// not a valid line
				continue;

			cont_found = 0;
			for (i = 0; i < nr_continents; ++i)
			{
				if (strcasecmp(cont_name, cont_overview_maps[i].name) == 0)
				{
					continent = i;
					cont_found = 1;
					break;
				}
			}
			if (!cont_found)
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
			continent_maps[imap].weather = get_weather_type_from_string(weather_name);
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
	continent_maps[imap].weather = 0;
}

int switch_to_game_map(void)
{
	char buffer[1024];
	short int cur_cont;
	static short int old_cont = -1;
	int size;

	/* check we loaded the mapinfo data */
	if (continent_maps == NULL || continent_maps[0].name == NULL)
	{
		LOG_TO_CONSOLE(c_yellow2,err_nomap_str);
		return 0;
	}

	if (check_image_name(map_file_name, sizeof(buffer), buffer) == 1)
	{
		map_text = load_texture_cached(buffer, tt_image);
	}
	else
	{
		map_text = 0;
	}
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
		cont_text = load_texture_cached (cont_overview_maps[cur_cont].file_name, tt_image);
		old_cont = cur_cont;
	}
#ifdef DEBUG_MAP_SOUND
	cur_tab_map = cur_map;
#endif // DEBUG_MAP_SOUND

	if(current_cursor != CURSOR_ARROW)
	{
		change_cursor(CURSOR_ARROW);
	}

	map_font_scale_fac = get_global_scale();
	// Set screen coordinates of the edges of the map
	size = min2i(4*(window_width-hud_x)/5, window_height-hud_y);
	small_map_screen_x_left = (window_width - hud_x - 5*size/4) / 2;
	small_map_screen_x_right = small_map_screen_x_left + size/4;
	small_map_screen_y_top = (window_height - hud_y - size) / 2;
	small_map_screen_y_bottom = small_map_screen_y_top + size/4;
	main_map_screen_x_left = small_map_screen_x_right;
	main_map_screen_x_right = main_map_screen_x_left + size;
	main_map_screen_y_top = small_map_screen_y_top;
	main_map_screen_y_bottom = main_map_screen_y_top + size;

	return 1;
}

static void draw_mark_filter(void)
{
	int x = small_map_screen_x_left + (small_map_screen_x_right - small_map_screen_x_left) / 2;
	int h = small_map_screen_y_bottom - small_map_screen_y_top, y = main_map_screen_y_bottom - h;

	// display the Mark filter title
	glColor3f(1.0f,1.0f,0.0f);
	draw_text(x, (int)(y+0.44*h), (const unsigned char*)label_mark_filter, strlen(label_mark_filter),
		MAPMARK_FONT, TDO_ALIGNMENT, CENTER, TDO_ZOOM, map_font_scale_fac, TDO_END);

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
		draw_text(x, (int)(y+0.58*h), (const unsigned char*)show_mark_filter_text,
			strlen(show_mark_filter_text), MAPMARK_FONT, TDO_ALIGNMENT, CENTER,
			TDO_ZOOM, map_font_scale_fac, TDO_END);
	}
	// display which key to activate the filter
	else
	{
		char buf[20];
		get_key_string(K_MARKFILTER, buf, sizeof(buf));
		draw_text(x, (int)(y+0.58*h), (const unsigned char *)buf, strlen(buf), MAPMARK_FONT,
			TDO_ALIGNMENT, CENTER, TDO_ZOOM, map_font_scale_fac, TDO_END);
	}
}

static void draw_marks(marking *the_marks, int the_max_mark, int the_tile_map_size_x, int the_tile_map_size_y)
{
	size_t i;
	int screen_x=0;
	int screen_y=0;
	float mapmark_zoom = map_font_scale_fac * font_scales[MAPMARK_FONT];
	int left = main_map_screen_x_left, width = main_map_screen_x_right - main_map_screen_x_left,
		bottom = main_map_screen_y_bottom, height = main_map_screen_y_bottom - main_map_screen_y_top;

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

			screen_x = left + width*x/(the_tile_map_size_x*6);
			screen_y = bottom - height*y/(the_tile_map_size_y*6);

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
			draw_text(screen_x, screen_y, (const unsigned char*)the_marks[i].text, strlen(the_marks[i].text),
				MAPMARK_FONT, TDO_ZOOM, map_font_scale_fac, TDO_END);
		}
	}
}

void draw_coordinates(int the_tile_map_size_x, int the_tile_map_size_y)
{
	int x = small_map_screen_x_left + (small_map_screen_x_right - small_map_screen_x_left) / 2;
	int h = small_map_screen_y_bottom - small_map_screen_y_top, y = main_map_screen_y_bottom - h;
	int map_x, map_y;

	// draw coordinates
	if (pf_get_mouse_position_extended(mouse_x, mouse_y, &map_x, &map_y, the_tile_map_size_x, the_tile_map_size_y)) {
		// we're pointing on the map, display position
		char buf[10];
		safe_snprintf(buf, sizeof(buf), "%d,%d", map_x, map_y);
		glColor3f(1.0f,1.0f,0.0f);
		draw_text(x, y+0.16*h, (const unsigned char*)buf, strlen(buf), MAPMARK_FONT,
			TDO_ALIGNMENT, CENTER, TDO_ZOOM, map_font_scale_fac, TDO_END);
		draw_text(x, y+0.02*h, (const unsigned char*)label_cursor_coords, strlen(label_cursor_coords),
			MAPMARK_FONT, TDO_ALIGNMENT, CENTER, TDO_ZOOM, map_font_scale_fac, TDO_END);
	}
}

void draw_game_map (int map, int mouse_mini)
{
	float mapmark_zoom = map_font_scale_fac * font_scales[MAPMARK_FONT];
	int screen_x=0;
	int screen_y=0;
	int x=-1,y=-1;
	float x_size=0,y_size=0;
	GLuint map_small, map_large;
	actor *me;
	static int fallback_text = -1;
	int win_width, win_height;
	int main_l = main_map_screen_x_left, main_r = main_map_screen_x_right,
		main_w = main_r - main_l;
	int main_t = main_map_screen_y_top, main_b = main_map_screen_y_bottom,
		main_h = main_b - main_t;
	int small_l = small_map_screen_x_left, small_r = small_map_screen_x_right,
		small_w = small_r - small_l;
	int small_t = small_map_screen_y_top, small_b = small_map_screen_y_bottom,
		small_h = small_b - small_t;

	// if we don't have a continent texture (instance may be), fallback to blank paper
	if (cont_text < 0)
	{
		if (fallback_text < 0)
		{
			fallback_text = load_texture_cached("./textures/paper1", tt_gui);
		}
		cont_text = fallback_text;
	}

	if(map){
		map_small = cont_text;
		if(inspect_map_text == 0) {
			map_large=map_text;
		} else {
			map_large = inspect_map_text;
		}
	} else {
		map_small=map_text;
		map_large = cont_text;
		if(cur_map!=-1){
			x_size = ((float)(continent_maps[cur_map].x_end - continent_maps[cur_map].x_start)) / tile_map_size_x;
			y_size = ((float)(continent_maps[cur_map].y_end - continent_maps[cur_map].y_start)) / tile_map_size_y;
		} else {
			x_size=y_size=0;
		}
	}

	win_width = window_width - hud_x;
	win_height = window_height - hud_y;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glViewport(0, hud_y, win_width, win_height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glOrtho(0.0, (GLdouble)win_width, (GLdouble)win_height, 0.0, -250.0, 250.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glLineWidth(get_global_scale());

	// Draw a black background
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS);
		glVertex2i(0,         0);
		glVertex2i(0,         win_height);
		glVertex2i(win_width, win_height);
		glVertex2i(win_width, 0);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	glColor3f(1.0, 1.0, 1.0);

	bind_texture(map_large);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex3i(main_l, main_b, 0);
		glTexCoord2f(0.0f, 0.0f); glVertex3i(main_l, main_t, 0);
		glTexCoord2f(1.0f, 0.0f); glVertex3i(main_r, main_t, 0);
		glTexCoord2f(1.0f, 1.0f); glVertex3i(main_r, main_b, 0);
	glEnd();

	if (mouse_mini)
		glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
	else
		glColor4f (0.7f, 0.7f, 0.7f, 0.7f);

	glEnable(GL_ALPHA_TEST);

	bind_texture(map_small);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex3i(small_l, small_b, 0);
		glTexCoord2f(0.0f, 0.0f); glVertex3i(small_l, small_t, 0);
		glTexCoord2f(1.0f, 0.0f); glVertex3i(small_r, small_t, 0);
		glTexCoord2f(1.0f, 1.0f); glVertex3i(small_r, small_b, 0);
	glEnd();

	glDisable(GL_ALPHA_TEST);

	glColor3f(1.0f,1.0f,1.0f);

	bind_texture(legend_text);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex3i(small_l, small_b + 2*small_h, 0);
		glTexCoord2f(0.0f, 0.0f); glVertex3i(small_l, small_b,             0);
		glTexCoord2f(1.0f, 0.0f); glVertex3i(small_r, small_b,             0);
		glTexCoord2f(1.0f, 1.0f); glVertex3i(small_r, small_b + 2*small_h, 0);
	glEnd();

// this is necessary for the text over map
// need to execute this for any map now
// because of the coordinate display - Lachesis
	if(map/*&&(adding_mark||max_mark>0)*/)
	{
		// Draw help for toggling the mini-map
		{
			char buf[80];
			char keybuf[20];
			glEnable(GL_TEXTURE_2D);
			safe_snprintf(buf, sizeof(buf), "%s %s", win_minimap, get_key_string(K_MINIMAP, keybuf, sizeof(keybuf)));
			glColor3f (1.0f, 1.0f, 0.0f);
			draw_text(small_l+small_w/2, (int)(main_t + 0.965*main_h), (const unsigned char *)buf,
				strlen(buf), MAPMARK_FONT, TDO_ALIGNMENT, CENTER, TDO_ZOOM, map_font_scale_fac, TDO_END);
		}

		// draw a temporary mark until the text is entered
		if (adding_mark)
		{
			int x = mark_x;
			int y = mark_y;

			screen_x = main_l + main_w*x/(tile_map_size_x*6);
			screen_y = main_b - main_h*y/(tile_map_size_y*6);

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
			draw_text(screen_x, screen_y, (const unsigned char*)input_text_line.data,
				strlen(input_text_line.data), MAPMARK_FONT, TDO_ZOOM, map_font_scale_fac, TDO_END);
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
				screen_x = main_l + main_w * ( (px * x_size / 6) + continent_maps[cur_map].x_start) / 512;
				screen_y = main_b - main_h * ( (py * y_size / 6) + continent_maps[cur_map].y_start) / 512;
			}
			else
			{
				screen_x = screen_y = 0;
			}
		}
		else
		{
			screen_x = main_l + main_w*px/(tile_map_size_x*6);
			screen_y = main_b - main_h*py/(tile_map_size_y*6);
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

	//ok, now let's draw our position...
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
			screen_x = main_l + main_w * ( (x * x_size / 6) + continent_maps[cur_map].x_start) / 512;
			screen_y = main_b - main_h * ( (y * y_size / 6) + continent_maps[cur_map].y_start) / 512;
		}
		else
		{
			screen_x = screen_y = 0;
		}
	}
	else
	{
		screen_x = main_l + main_w*x/(tile_map_size_x*6);
		screen_y = main_b - main_h*y/(tile_map_size_y*6);
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
		int screen_map_width = main_map_screen_x_right - main_map_screen_x_left;
		int screen_map_height = main_map_screen_y_bottom - main_map_screen_y_top;
		int m_px = ((mouse_x-main_map_screen_x_left) * 512) / screen_map_width;
		int m_py = 512 - ((mouse_y-main_map_screen_y_top) * 512) / screen_map_height;
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
					int x_start = main_l + main_w*continent_maps[i].x_start/512;
					int x_end = main_l + main_w*continent_maps[i].x_end/512;
					int y_start = main_b - main_h*continent_maps[i].y_start / 512;
					int y_end = main_b - main_h*continent_maps[i].y_end / 512;

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
			int x_start = main_l + main_w*continent_maps[mouse_over].x_start/512;
			int x_end = main_l + main_w*continent_maps[mouse_over].x_end/512;
			int y_start = main_b - main_h*continent_maps[mouse_over].y_start / 512;
			int y_end = main_b - main_h*continent_maps[mouse_over].y_end / 512;

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

	glEnable (GL_TEXTURE_2D);
	glColor3f (1.0f, 1.0f, 1.0f);

	glLineWidth(1.0f);

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


int put_mark_on_position(int map_x, int map_y, const char * name)
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
		rtrim_string(marks[max_mark].text); //remove trailing white space

		marks[max_mark].server_side=0;
		marks[max_mark].server_side_id=-1;

		marks[max_mark].r=curmark_r;
		marks[max_mark].g=curmark_g;
		marks[max_mark].b=curmark_b;

		max_mark++;
		save_markings();
		return 1;
}

void put_mark_on_map_on_mouse_position(void)
{
	if (pf_get_mouse_position(mouse_x, mouse_y, &mark_x, &mark_y))
		adding_mark = 1;
}
int put_mark_on_current_position(const char *name)
{
	actor *me = get_our_actor ();

	if (me != NULL)
	{
		if (put_mark_on_position(me->x_tile_pos, me->y_tile_pos, name))
			return 1;
	}
	return 0;
}

void delete_mark_on_map_on_mouse_position(void)
{
	int mx , my , i;

	int screen_map_width = main_map_screen_x_right - main_map_screen_x_left;
	int screen_map_height = main_map_screen_y_bottom - main_map_screen_y_top;

	int min_distance;
	marking * closest_mark;

	// FIXME (Malaclypse): should be moved above the screen_map_* init, to avoid additional computation
	if (mouse_x < main_map_screen_x_left || mouse_x > main_map_screen_x_right
		|| mouse_y < main_map_screen_y_top || mouse_y > main_map_screen_y_bottom) {
		return;
	}

	mx = ((mouse_x - main_map_screen_x_left) * tile_map_size_x * 6) / screen_map_width;
	my = (tile_map_size_y * 6) - ((mouse_y - main_map_screen_y_top) * tile_map_size_y * 6) / screen_map_height;

	// delete mark closest to cursor
	min_distance = screen_map_width/10 * screen_map_height/10; // only check close marks
	closest_mark = NULL;
	for ( i = 0 ; i < max_mark ; i ++ ) {
		int distance, dx, dy;
		marking * const mark = &marks[i];

		// skip marks not shown due to filter
		if (mark_filter_active
			  && (get_string_occurance(mark_filter_text, mark->text, strlen(mark->text), 1) == -1))
			continue;

		// skip masked marks
		if (mark->x < 0)
			continue;

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
		if (closest_mark->server_side) {
			hash_delete(server_marks,(void *)(uintptr_t)(closest_mark->server_side_id));
			save_server_markings();
		}
	}

	save_markings();
	load_map_marks(); // simply to compact the array and make room for new marks

}


void destroy_all_root_windows (void)
{
	if (game_root_win >= 0) destroy_window (game_root_win);
	if (get_id_MW(MW_CONSOLE) >= 0) destroy_window (get_id_MW(MW_CONSOLE));
	if (get_id_MW(MW_TABMAP) >= 0) destroy_window (get_id_MW(MW_TABMAP));
	if (login_root_win >= 0) destroy_window (login_root_win);
	if (rules_root_win >= 0) destroy_window (rules_root_win);
	if (opening_root_win >= 0) destroy_window (opening_root_win);
	if (newchar_root_win >= 0) destroy_window (newchar_root_win);
	if (update_root_win >= 0) destroy_window (update_root_win);
	if (langsel_rootwin >= 0) destroy_window (langsel_rootwin);
}
void hide_all_root_windows (void)
{
	if (game_root_win >= 0) hide_window (game_root_win);
	hide_window_MW(MW_CONSOLE);
	hide_window_MW(MW_TABMAP);
	if (login_root_win >= 0) hide_window (login_root_win);
	if (rules_root_win >= 0) hide_window (rules_root_win);
	if (opening_root_win >= 0) hide_window (opening_root_win);
	if (newchar_root_win >= 0) hide_window (newchar_root_win);
	if (update_root_win >= 0) hide_window (update_root_win);
	if (langsel_rootwin >= 0) hide_window (langsel_rootwin);
}

void resize_all_root_windows (Uint32 ow, Uint32 w, Uint32 oh, Uint32 h)
{
	move_windows_proportionally((float)w / (float)ow, (float)h / (float)oh);
	if (game_root_win >= 0) resize_window (game_root_win, w, h);
	if (get_id_MW(MW_CONSOLE) >= 0) resize_window (get_id_MW(MW_CONSOLE), w, h);
	if (get_id_MW(MW_TABMAP) >= 0) resize_window (get_id_MW(MW_TABMAP), w, h);
	if (login_root_win >= 0) resize_window (login_root_win, w, h);
	if (rules_root_win >= 0) resize_window (rules_root_win, w, h);
	if (opening_root_win >= 0) resize_window (opening_root_win, w, h);
	if (newchar_root_win >= 0) resize_window (newchar_root_win, w, h);
	if (update_root_win >= 0) resize_window (update_root_win, w, h);
	if (langsel_rootwin >= 0) resize_window (langsel_rootwin, w, h);
	if ((input_widget != NULL) && (input_widget->window_id != get_id_MW(MW_CHAT))) {
		widget_resize (input_widget->window_id, input_widget->id, w-HUD_MARGIN_X, input_widget->len_y);
		widget_move (input_widget->window_id, input_widget->id, 0, h-input_widget->len_y-HUD_MARGIN_Y);
	}
	resize_newchar_hud_window();
}

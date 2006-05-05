#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "global.h"
#include "elwindows.h"

#define DEFAULT_CONTMAPS_SIZE 20

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void draw_menu_title_bar(int, int, int);
 */

int mouse_x;
int mouse_y;
int mouse_delta_x;
int mouse_delta_y;
int right_click;
int middle_click;
int left_click;

int login_screen_menus;
char username_box_selected=1;
char password_box_selected=0;
char username_str[20]={0};
char password_str[20]={0};
char display_password_str[20]={0};
int username_text_length=0;
int password_text_length=0;

int have_a_map=0;
char create_char_error_str[520];
char log_in_error_str[520];
int combat_mode=0;
int auto_camera=0;
int view_health_bar=1;
int view_names=1;
int view_hp=0;
int view_chat_text_as_overtext=0;
int limit_fps=0;

int action_mode=ACTION_WALK;

Uint32 click_time=0;
int click_speed=300;

// Grum: attempt to work around bug in Ati linux drivers.
int ati_click_workaround = 0;

void get_world_x_y()
{
  	float mouse_z,z;
	glReadPixels(mouse_x, window_height-mouse_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &mouse_z);
	// XXX FIXME (Grum): hack to work around a bug in the Ati drivers or
	// a giant misconception on the part of all EL developers so far.
	if (ati_click_workaround && bpp == 32)
		mouse_z = ldexp (mouse_z, 8);
	unproject_ortho(mouse_x,window_height-hud_y-mouse_y,mouse_z,&scene_mouse_x,&scene_mouse_y,&z);
}

void get_old_world_x_y()
{
#ifndef NEW_FRUSTUM
	float x1,y1,z1, x2,y2,z2, l;
	/* Shoot a ray through the pixel and determine its intersection with the ground plane (z == 0) */
	unproject_ortho(mouse_x,window_height-hud_y-mouse_y,0.0f,&x1,&y1,&z1);
	unproject_ortho(mouse_x,window_height-hud_y-mouse_y,1.0f,&x2,&y2,&z2);
#else
	AABBOX box;
	float t, x, y, z, t1, t2, tx, ty, dx, dy, h, len;
	int i, j, h_max, h_min, h1, h2, h3, h4, sx, sy, zx, zy, i_min, i_max, j_min, j_max;
#endif
	
#ifndef NEW_FRUSTUM
	l = z2 / (z2 - z1);
	scene_mouse_x = l*x1 + (1 - l)*x2;
	scene_mouse_y = l*y1 + (1 - l)*y2;
#else
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
	i_max = max2i(min2i(i_max, tile_map_size_x*6), 0);
	j_min = max2i(min2i(j_min, tile_map_size_y*6), 0);
	j_max = max2i(min2i(j_max, tile_map_size_y*6), 0);

	i = min2i(max2i(i, i_min), i_max-1);
	j = min2i(max2i(j, j_min), j_max-1);
	
	h = 0.0f;
	while ((i >= i_min) && (j >= j_min) && (i < i_max) && (j < j_max))
	{
		h1 = height_map[j*tile_map_size_x*6+i];
		h2 = height_map[j*tile_map_size_x*6+i+1];
		h3 = height_map[(j+1)*tile_map_size_x*6+i];
		h4 = height_map[(j+1)*tile_map_size_x*6+i+1];
		h_max = max2i(max2i(h1, h2), max2i(h3, h4));
		h_min = min2i(min2i(h1, h2), min2i(h3, h4));
		tx = i*0.5f;
		ty = j*0.5f;
		box.bbmin[X] = tx;
		box.bbmin[Y] = ty;
		box.bbmin[Z] = h_min*0.2f-2.2f;
		box.bbmax[X] = tx+1.0f;
		box.bbmax[Y] = ty+1.0f;
		box.bbmax[Z] = h_max*0.2f-2.2f;
		if (click_line_bbox_intersection(box))
		{
			box.bbmin[X] = tx+0.0f;
			box.bbmin[Y] = ty+0.0f;
			box.bbmin[Z] = h1*0.2f-2.2f;
			box.bbmax[X] = tx+0.5f;
			box.bbmax[Y] = ty+0.5f;
			box.bbmax[Z] = h1*0.2f-2.2f;
			if (click_line_bbox_intersection(box))
			{
				h = h1*0.2f-2.2f;
				break;
			}
			box.bbmin[X] = tx+0.5f;
			box.bbmin[Y] = ty+0.0f;
			box.bbmin[Z] = h2*0.2f-2.2f;
			box.bbmax[X] = tx+1.0f;
			box.bbmax[Y] = ty+0.5f;
			box.bbmax[Z] = h2*0.2f-2.2f;
			if (click_line_bbox_intersection(box))
			{
				h = h2*0.2f-2.2f;
				break;
			}
			box.bbmin[X] = tx+0.0f;
			box.bbmin[Y] = ty+0.5f;
			box.bbmin[Z] = h3*0.2f-2.2f;
			box.bbmax[X] = tx+0.5f;
			box.bbmax[Y] = ty+1.0f;
			box.bbmax[Z] = h3*0.2f-2.2f;
			if (click_line_bbox_intersection(box))
			{
				h = h3*0.2f-2.2f;
				break;
			}
			box.bbmin[X] = tx+0.5f;
			box.bbmin[Y] = ty+0.5f;
			box.bbmin[Z] = h4*0.2f-2.2f;
			box.bbmax[X] = tx+1.0f;
			box.bbmax[Y] = ty+1.0f;
			box.bbmax[Z] = h4*0.2f-2.2f;
			if (click_line_bbox_intersection(box))
			{
				h = h4*0.2f-2.2f;
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
	scene_mouse_x = click_line.center[X]+click_line.direction[X]*t;
	scene_mouse_y = click_line.center[Y]+click_line.direction[Y]*t;
#endif
}

void Enter2DMode()
{
#ifdef	NEW_WEATHER
	if (weather_use_fog()) glDisable(GL_FOG);
#else
	if (use_fog) glDisable(GL_FOG);
#endif
	glPushAttrib(GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, window_width, window_height);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, (GLdouble)window_width, (GLdouble)window_height, 0.0, -250.0, 250.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void Leave2DMode()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
	glViewport(0, hud_y, window_width-hud_x, window_height-hud_y);
#ifdef	NEW_WEATHER
	if (weather_use_fog()) glEnable(GL_FOG);
#else
	if (use_fog) glEnable(GL_FOG);
#endif
	else glDisable(GL_FOG);
	//glViewport(0, 0, window_width-hud_x, window_height-hud_y);	// Reset The Current Viewport
}

mode_flag video_modes[12];

void build_video_mode_array()
{
	int i;
	int flags;

	for(i=0;i<12;i++)
		{
			video_modes[i].selected=0;
			video_modes[i].supported=0;
		}
	video_modes[video_mode-1].selected=1;

	if(full_screen)flags=SDL_OPENGL|SDL_FULLSCREEN;
	else
		flags=SDL_OPENGL;

#ifdef WINDOWS
	if(bpp==16 || full_screen){
#else
	if(bpp==16){
#endif
		if(SDL_VideoModeOK(640, 480, 16, flags))video_modes[0].supported=1;
		if(SDL_VideoModeOK(800, 600, 16, flags))video_modes[2].supported=1;
		if(SDL_VideoModeOK(1024, 768, 16, flags))video_modes[4].supported=1;
		if(SDL_VideoModeOK(1152, 864, 16, flags))video_modes[6].supported=1;
		if(SDL_VideoModeOK(1280, 1024, 16, flags))video_modes[8].supported=1;
		if(SDL_VideoModeOK(1600, 1200, 16, flags))video_modes[10].supported=1;
		if(SDL_VideoModeOK(1280, 800, 16, flags))video_modes[12].supported=1;
		if(SDL_VideoModeOK(1440, 900, 16, flags))video_modes[14].supported=1;
		if(SDL_VideoModeOK(1680, 1050, 16, flags))video_modes[16].supported=1;
	}
#ifdef WINDOWS
	if(bpp==32 || full_screen){
#else
	if(bpp==32){
#endif
		if(SDL_VideoModeOK(640, 480, 32, flags))video_modes[1].supported=1;
		if(SDL_VideoModeOK(800, 600, 32, flags))video_modes[3].supported=1;
		if(SDL_VideoModeOK(1024, 768, 32, flags))video_modes[5].supported=1;
		if(SDL_VideoModeOK(1152, 864, 32, flags))video_modes[7].supported=1;
		if(SDL_VideoModeOK(1280, 1024, 32, flags))video_modes[9].supported=1;
		if(SDL_VideoModeOK(1600, 1200, 32, flags))video_modes[11].supported=1;
		if(SDL_VideoModeOK(1280, 800, 32, flags))video_modes[13].supported=1;
		if(SDL_VideoModeOK(1440, 900, 32, flags))video_modes[15].supported=1;
		if(SDL_VideoModeOK(1680, 1050, 32, flags))video_modes[17].supported=1;
	}
//TODO: Add wide screen resolutions
//1400x1050
}

void draw_console_pic(int which_texture)
{
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


void init_opening_interface()
{
	CHECK_GL_ERRORS();
	login_screen_menus=load_texture_cache("./textures/login_menu.bmp",0);
	CHECK_GL_ERRORS();
	login_text=load_texture_cache("./textures/login_back.bmp",255);
	CHECK_GL_ERRORS();
}

void add_char_to_username(unsigned char ch)
{
	if(((ch>=48 && ch<=57) || (ch>=65 && ch<=90) || (ch>=97 && ch<=122) || (ch=='_')) && username_text_length<15)
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
	if ((ch>=32 && ch<=126) && password_text_length<15)
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
	//check_menus_out_of_screen();

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

	glColor3f(1.0f, 1.0f, 1.0f);
	draw_hud_frame();

	//draw_hud_interface();
	display_spells_we_have();
}

GLuint map_text;
GLuint cont_text;
GLuint legend_text=0;
int cur_map;  //Is there a better way to do this?

const char* cont_map_file_names[] = {
	"./maps/seridia.bmp",
	"./maps/irilion.bmp"
};
const int nr_continents = sizeof (cont_map_file_names) / sizeof (const char *);
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
	
	fin = my_fopen ("mapinfo.lst", "r");
	imap = 0;
	if (fin != NULL)
	{
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
			strcpy (continent_maps[imap].name, map_name);
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
	int len;
	char map_map_file_name[256];
	short int cur_cont;
	static short int old_cont = 0;
	
	my_strcp(map_map_file_name,map_file_name);
	len=strlen(map_map_file_name);
	map_map_file_name[len-3]='b';
	map_map_file_name[len-2]='m';
	map_map_file_name[len-1]='p';
	map_text=load_bmp8_fixed_alpha(map_map_file_name,128);
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
		cont_text = load_texture_cache (cont_map_file_names[cur_cont], 128);
		old_cont = cur_cont;
	}
	
	if(current_cursor != CURSOR_ARROW)
	{
		change_cursor(CURSOR_ARROW);
	}
	return 1;
}

void switch_from_game_map()
{
	glDeleteTextures(1,&map_text);
}


void draw_game_map (int map, int mouse_mini)
{     
	int screen_x=0;
	int screen_y=0;
	int map_x, map_y;
	int x=-1,y=-1;
	int i;
	float x_size=0,y_size=0;
	GLuint map_small, map_large;
	
	if(map){
		map_small=get_texture_id(cont_text);
		map_large=map_text;
	} else {
		map_small=map_text;
		map_large=get_texture_id(cont_text);
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
    	
	bind_texture_id(map_large);
    
	glBegin(GL_QUADS);
		glTexCoord2f(1.0f,0.0f); glVertex3i(50,0,0); 
		glTexCoord2f(1.0f,1.0f); glVertex3i(50,200,0);
		glTexCoord2f(0.0f,1.0f); glVertex3i(250,200,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(250,0,0);
	glEnd();

	if (mouse_mini)
		glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
	else
		glColor4f (0.7f, 0.7f, 0.7f, 0.7f);
    	
	glEnable(GL_ALPHA_TEST);
	
	bind_texture_id(map_small);
    	
    	glBegin(GL_QUADS);
		glTexCoord2f(1.0f,0.0f); glVertex3i(250,150,0);
		glTexCoord2f(1.0f,1.0f); glVertex3i(250,200,0);
		glTexCoord2f(0.0f,1.0f); glVertex3i(300,200,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(300,150,0);
	glEnd();
	
	glDisable(GL_ALPHA_TEST);
	
	glColor3f(1.0f,1.0f,1.0f);
    	
	get_and_set_texture_id(legend_text);
    
    	glBegin(GL_QUADS);
		glTexCoord2f(1.0f,0.0f); glVertex3i(250,50,0);
		glTexCoord2f(1.0f,1.0f); glVertex3i(250,150,0);
		glTexCoord2f(0.0f,1.0f); glVertex3i(300,150,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(300,50,0);
	glEnd();

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
 
		// draw a temporary mark until the text is entered
		if (adding_mark) {
        	        int x = mark_x;
        	        int y = mark_y;

			screen_x=(51+200*x/(tile_map_size_x*6));
			screen_y=201-200*y/(tile_map_size_y*6);

			glColor3f(1.0f,1.0f,0.0f);
			glDisable(GL_TEXTURE_2D);
			glBegin(GL_LINES);
				glVertex2i(screen_x-3,screen_y-3);
				glVertex2i(screen_x+2,screen_y+2);

				glVertex2i(screen_x+2,screen_y-3);
				glVertex2i(screen_x-3,screen_y+2);
			glEnd();
		        glEnable(GL_TEXTURE_2D);
		        glColor3f(1.0f,1.0f,0.0f);
			draw_string_zoomed (screen_x, screen_y, input_text_line.data, 1, 0.3);
		}


		// crave the markings
		for(i=0;i<max_mark;i++)
		 {
			int x = marks[i].x;
			int y = marks[i].y;
			if ( x > 0 ) {
				screen_x=(51+200*x/(tile_map_size_x*6));
				screen_y=201-200*y/(tile_map_size_y*6);

				glColor3f(0.4f,1.0f,0.0f);
				glDisable(GL_TEXTURE_2D);
				glBegin(GL_LINES);
					glVertex2i(screen_x-3,screen_y-3);
					glVertex2i(screen_x+2,screen_y+2);
				
					glVertex2i(screen_x+2,screen_y-3);
					glVertex2i(screen_x-3,screen_y+2);
				glEnd();
	        		glEnable(GL_TEXTURE_2D);
	        		glColor3f(0.2f,1.0f,0.0f);
				draw_string_zoomed(screen_x,screen_y,marks[i].text,1,0.3);
			}
	 	}

		// draw coordinates
		if (pf_get_mouse_position(mouse_x, mouse_y, &map_x, &map_y)) {
			// we're pointing on the map, display position
			char buf[10];
			sprintf(buf, "%d,%d", map_x, map_y);
			glColor3f(1.0f,1.0f,0.0f);
			screen_x = 25 - 1.5*strlen(buf);
			screen_y = 150 + 11;
			draw_string_zoomed(screen_x, screen_y, buf, 1, 0.3);
			screen_x = 25 - 1.5*strlen(label_cursor_coords);
			screen_y = 150 + 4;
			draw_string_zoomed(screen_x, screen_y, label_cursor_coords, 1, 0.3);
		}

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	//if we're following a path, draw the destination on the map
	if (pf_follow_path)
	{
		int px = pf_dst_tile->x;
		int py = pf_dst_tile->y;

		if (!map)
		{
			if (cur_map!=-1)
			{
				screen_x = 300 - (50 + 200 * ( (x_size / 6 * px) + continent_maps[cur_map].x_start) / 512);
				screen_y = 200 * ( (y_size / 6 * py) + continent_maps[cur_map].y_start) / 512;
			}
			else
			{
				screen_x = screen_y = 0;
			}
		} 
		else
		{
			screen_x = 300 - ( 50 + 200 * px / (tile_map_size_x * 6) );
			screen_y = 200 * py / (tile_map_size_y * 6);
		}

		glColor3f(1.0f,0.0f,0.0f);
		
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);

		glVertex2i(screen_x-3,screen_y-3);
		glVertex2i(screen_x+2,screen_y+2);

		glVertex2i(screen_x+2,screen_y-3);
		glVertex2i(screen_x-3,screen_y+2);

		glEnd();
	}
	
	//ok, now let's draw our possition...
	if(your_actor != NULL) {
		x = your_actor->x_tile_pos;
		y = your_actor->y_tile_pos;
	} else {
		//We don't exist (usually happens when teleporting)
		x = -1;
		y = -1;
	}

	if (!map)
	{
		if (cur_map != -1)
		{
			screen_x = 300 - (50 + 200 * ( (x_size / 6 * x) + continent_maps[cur_map].x_start) / 512);
			screen_y = 200 * ( (y_size / 6 * y) + continent_maps[cur_map].y_start) / 512;
		} 
		else 
		{
			screen_x = screen_y = 0;
		}
	} 
	else 
	{
		screen_x = 300 - ( 50 + 200 * x / (tile_map_size_x * 6) );
		screen_y = 200 * y / (tile_map_size_y * 6);
	}
	
	if ( (map || !dungeon) && x != -1 )
	{
		glColor3f (0.0f, 0.0f, 1.0f);
		glDisable (GL_TEXTURE_2D);
		glBegin (GL_LINES);
			
		glVertex2i (screen_x-3, screen_y-3);
		glVertex2i (screen_x+2, screen_y+2);

		glVertex2i (screen_x+2, screen_y-3);
		glVertex2i (screen_x-3, screen_y+2);

		glEnd();
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
}


int put_mark_on_position(int map_x, int map_y, char * name)
{
		if (map_x < 0
		|| map_x >= tile_map_size_x*6
		|| map_y < 0
		|| map_y >= tile_map_size_y*6) {
						return 0;
		}
		marks[max_mark].x = map_x;
		marks[max_mark].y = map_y;
		memset(marks[max_mark].text,0,500);
		
		my_strncp(marks[max_mark].text,name,500);
		marks[max_mark].text[strlen(marks[max_mark].text)]=0;
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
void put_mark_on_current_position(char *name)
{
	actor *me = pf_get_our_actor();

	if (me != NULL)
	{	
		put_mark_on_position(me->x_tile_pos, me->y_tile_pos, name);
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
		if (mark->x < 0) continue;
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
}

void save_markings()
{
      FILE * fp;
      char marks_file[256];
      int i;

#ifndef WINDOWS
      snprintf (marks_file, sizeof(marks_file), "%s/.elc/%s.txt", getenv ("HOME"), strrchr (map_file_name,'/') + 1);
#else
      snprintf (marks_file, sizeof (marks_file), "%s.txt", strrchr (map_file_name,'/') + 1);
#endif
      fp = my_fopen(marks_file,"w");
      if ( fp ) {
	  for ( i = 0 ; i < max_mark ; i ++)
    	     if ( marks[i].x > 0 )
                fprintf(fp,"%d %d %s\n",marks[i].x,marks[i].y,marks[i].text);
          fclose(fp);
        };
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
	if (input_widget->window_id != chat_win) {
		widget_resize (input_widget->window_id, input_widget->id, w-HUD_MARGIN_X, input_widget->len_y);
		widget_move (input_widget->window_id, input_widget->id, 0, h-input_widget->len_y-HUD_MARGIN_Y);
	}
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

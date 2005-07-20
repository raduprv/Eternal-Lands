#include <string.h>
#include "global.h"
#include "elwindows.h"
#include <math.h>

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
int username_text_lenght=0;
int password_text_lenght=0;

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
	float window_ratio;
	float x,y,x1,y1,a,t;
	actor *p=pf_get_our_actor();
	if(!p) return;
	
	window_ratio=(GLfloat)window_width/(GLfloat)window_height;
	x=(float)((mouse_x)*2.0f*window_ratio*(float)zoom_level/(float)(window_width-hud_x))-(window_ratio*zoom_level);
	y=(float)((window_height-hud_y-mouse_y+0.05*window_height)*2.0f*zoom_level/(window_height-hud_y))-(2.0*zoom_level/2.0f);

	a=(rz)*3.1415926/180;
	t=(rx)*3.1415926/180;

	y=(float)y/(float)cos(t);

	x1=x*cos(a)+y*sin(a);
	y1=y*cos(a)-x*sin(a);

	scene_mouse_x=p->x_pos+x1;
	scene_mouse_y=p->y_pos+y1;
}

void Enter2DMode()
{
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
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(640, 480, 16, flags))video_modes[0].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(640, 480, 32, flags))video_modes[1].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(800, 600, 16, flags))video_modes[2].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(800, 600, 32, flags))video_modes[3].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(1024, 768, 16, flags))video_modes[4].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(1024, 768, 32, flags))video_modes[5].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(1152, 864, 16, flags))video_modes[6].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(1152, 864, 32, flags))video_modes[7].supported=1;

#ifdef WINDOWS
	if(bpp==16 || full_screen)
#else
		if(bpp==16)
#endif
			if(SDL_VideoModeOK(1280, 1024, 16, flags))video_modes[8].supported=1;

#ifdef WINDOWS
	if(bpp==32 || full_screen)
#else
		if(bpp==32)
#endif
			if(SDL_VideoModeOK(1280, 1024, 32, flags))video_modes[9].supported=1;
#ifdef WINDOWS
        if(bpp==16 || full_screen)
#else
                if(bpp==16)
#endif
                        if(SDL_VideoModeOK(1600, 1200, 16, flags))video_modes[10].supported=1;
#ifdef WINDOWS
        if(bpp==32 || full_screen)
#else
                if(bpp==32)
#endif
                        if(SDL_VideoModeOK(1600, 1200, 32, flags))video_modes[11].supported=1;
//TODO: Add wide screen resolutions
//1280x800
//1400x1050
//1440x900
//1680x1050
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
	if(((ch>=48 && ch<=57) || (ch>=65 && ch<=90) || (ch>=97 && ch<=122) || (ch=='_')) && username_text_lenght<15)
		{
			username_str[username_text_lenght]=ch;
			username_str[username_text_lenght+1]=0;
			username_text_lenght++;
		}
	if( ( ch==SDLK_DELETE || ch==SDLK_BACKSPACE) && username_text_lenght > 0 )
		{
			username_text_lenght--;
			username_str[username_text_lenght]=0;
		}
}

void add_char_to_password(unsigned char ch)
{
	if((ch>=32 && ch<=126) && password_text_lenght<15)
		{
			password_str[password_text_lenght]=ch;
			display_password_str[password_text_lenght]='*';
			password_str[password_text_lenght+1]=0;
			display_password_str[password_text_lenght+1]=0;
			password_text_lenght++;
		}
	if( ( ch==SDLK_DELETE || ch==SDLK_BACKSPACE) && password_text_lenght > 0 )
		{
			password_text_lenght--;
			display_password_str[password_text_lenght]=0;
			password_str[password_text_lenght]=0;
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

const struct draw_map seridia_maps[] = {
        {409,107,450,147,"./maps/startmap.elm"},//0 - Isla Prima
	{184,162,395,359,"./maps/map2.elm"},//1 - Whitestone
	{84,352,180,448,"./maps/map3.elm"}, //2 - Desert Pines
	{336,118,387,165,"./maps/map4f.elm"},//3 - Tirnym
	{230,405,281,451,"./maps/map5nf.elm"},//4 - VOTD
	{84,270,177,357,"./maps/map6nf.elm"}, //5 - Portland
	{87,169,175,270,"./maps/map7.elm"}, //6 - Morcraven
	{130,128,178,168,"./maps/map8.elm"},//7 - Naralik
	{180,75,275,165,"./maps/map9f.elm"}, //8 - Grubani
	{0,0,0,0,"./maps/map10.elm"},     //9 -
	{282,358,385,454,"./maps/map11.elm"},//10 - Tarsengaard
	{232,359,283,403,"./maps/map12.elm"},//11 - Nordcarn
	{181,363,231,408,"./maps/map13.elm"},//12 - Southern KF
	{178,406,227,443,"./maps/map14f.elm"},//13 - KF
	{2,324,75,431,"./maps/map15f.elm"},    //14 - Tahraji
	{0,0,0,0,NULL} //Last map
};

int switch_to_game_map()
{
	int len;
	char map_map_file_name[256];

	my_strcp(map_map_file_name,map_file_name);
	len=strlen(map_map_file_name);
	map_map_file_name[len-3]='b';
	map_map_file_name[len-2]='m';
	map_map_file_name[len-1]='p';
	map_text=load_bmp8_fixed_alpha(map_map_file_name,128);
	if(!map_text){
		LOG_TO_CONSOLE(c_yellow2,"There is no map for this place.");
		return 0;
	}
	if(current_cursor!=CURSOR_ARROW)change_cursor(CURSOR_ARROW);
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
			x_size=(float)((float)(seridia_maps[cur_map].x_end - seridia_maps[cur_map].x_start))/(float)tile_map_size_x;
			y_size=(float)((float)(seridia_maps[cur_map].y_end - seridia_maps[cur_map].y_start))/(float)tile_map_size_y;
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
				screen_x = 300 - (50 + 200 * ( (x_size / 6 * px) + seridia_maps[cur_map].x_start) / 512);
				screen_y = 200 * ( (y_size / 6 * py) + seridia_maps[cur_map].y_start) / 512;
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
	for (i = 0; i < max_actors; i++)
	{
		if (actors_list[i] && actors_list[i]->actor_id == yourself && actors_list[i]->tmp.have_tmp)
		{
			x = actors_list[i]->tmp.x_tile_pos;
			y = actors_list[i]->tmp.y_tile_pos;
			break;
		}
	}

	if (!map)
	{
		if (cur_map != -1)
		{
			screen_x = 300 - (50 + 200 * ( (x_size / 6 * x) + seridia_maps[cur_map].x_start) / 512);
			screen_y = 200 * ( (y_size / 6 * y) + seridia_maps[cur_map].y_start) / 512;
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

        // FIXME (Malaclypse): should be moved above the screen_map_* init, to avoid additional computation
        if (mouse_x < min_mouse_x
        || mouse_x > max_mouse_x
        || mouse_y < min_mouse_y
        || mouse_y > max_mouse_y) {
                return;
        }

        mx = ((mouse_x - min_mouse_x) * tile_map_size_x * 6) / screen_map_width;
        my = (tile_map_size_y * 6) - ((mouse_y * tile_map_size_y * 6) / screen_map_height);

	for ( i = 0 ; i < max_mark ; i ++ ) 
	    if (( abs( mx - marks[i].x) < 20 ) && (abs( my - marks[i].y) < 20 ) )
              {
		marks[i].x =  -1 ;
		marks[i].y =  -1 ;
		break;
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

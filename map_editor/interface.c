#include "tiles.h"
#include "global.h"
#include "eye_candy_window.h"
#include <math.h>
#include "../asc.h"

#define TOOLTIP_MOUSE_X_SHIFT 10
#define TOOLTIP_MOUSE_Y_SHIFT 10
#define UNSCALED_TOOLBAR_BUTTON_WIDTH 32
#define UNSCALED_TOOLBAR_BUTTON_HEIGHT 32

static int toolbar_mouseover = 0;
static int toolbar_button_width = UNSCALED_TOOLBAR_BUTTON_WIDTH;
static int toolbar_button_height = UNSCALED_TOOLBAR_BUTTON_HEIGHT;

typedef enum
{
	TOOLBAR_BUTTON_MODE_TILE,
	TOOLBAR_BUTTON_MODE_2D,
	TOOLBAR_BUTTON_MODE_3D,
	TOOLBAR_BUTTON_MODE_PARTICLES,
#ifdef EYE_CANDY
	TOOLBAR_BUTTON_MODE_EYE_CANDY,
#endif
	TOOLBAR_BUTTON_MODE_LIGHT,
	TOOLBAR_BUTTON_MODE_HEIGHT,
	TOOLBAR_BUTTON_MODE_MAP,
	TOOLBAR_BUTTON_TOOL_SELECT,
	TOOLBAR_BUTTON_TOOL_CLONE,
	TOOLBAR_BUTTON_TOOL_NEW,
	TOOLBAR_BUTTON_TOOL_KILL,
	TOOLBAR_BUTTON_SAVE_MAP,
	TOOLBAR_BUTTON_OPEN_MAP,
	TOOLBAR_BUTTON_NEW_MAP,
	TOOLBAR_MAX_BUTTON
} toolbar_button_idx;

typedef struct {
	float u_start;
	float v_start;
	float u_end;
	float v_end;
	const char* tooltip;
} toolbar_button;

static toolbar_button toolbar[TOOLBAR_MAX_BUTTON] = {
		{ (float)64/255,  0.0f,          (float)96/255,  (float)32/255, "Mode Tile"      },
		{ (float)32/255,  0.0f,          (float)64/255,  (float)32/255, "Mode 2D"        },
		{ 0.0f,           0.0f,          (float)32/255,  (float)32/255, "Mode 3D"        },
		{ (float)192/255, (float)32/255, (float)224/255, (float)64/255, "Mode Particles" },
#ifdef EYE_CANDY
		{ (float)224/255, (float)32/255, (float)255/255, (float)64/255, "Mode Eye Candy" },
#endif
		{ (float)96/255,  0.0f,          (float)128/255, (float)32/255, "Mode Light"     },
		{ (float)160/255, (float)32/255, (float)192/255, (float)64/255, "Mode Height"    },
		{ (float)128/255, 0.0f,          (float)160/255, (float)32/255, "Mode Map"       },
		{ 0.0f,           (float)32/255, (float)32/255,  (float)64/255, "Select"         },
		{ (float)32/255,  (float)32/255, (float)64/255,  (float)64/255, "Clone"          },
		{ (float)224/255, 0.0f,          (float)256/255, (float)32/255, "New Object"     },
		{ (float)192/255, 0.0f,          (float)224/255, (float)32/255, "Kill"           },
		{ (float)64/255,  (float)32/255, (float)96/255,  (float)64/255, "Save Map"       },
		{ (float)96/255,  (float)32/255, (float)128/255, (float)64/255, "Open Map"       },
		{ (float)128/255, (float)32/255, (float)160/255, (float)64/255, "New Map"        }
};

int show_position_on_minimap=0;

void set_toolbar_button_size(void)
{
	float scale = get_global_scale();
	toolbar_button_width = (int)(UNSCALED_TOOLBAR_BUTTON_WIDTH*scale + 0.5);
	toolbar_button_height = (int)(UNSCALED_TOOLBAR_BUTTON_HEIGHT*scale + 0.5);
}

int check_interface_buttons()
{
	toolbar_button_idx idx;

#ifdef EYE_CANDY
	if ((minimap_on) && (cur_mode==mode_eye_candy) && (eye_candy_confirmed))
	{
		if (left_click)
		  add_eye_candy_point();
		else
		  last_ec_index = -2;
	}
#endif
	if ((left_click!=1 && right_click!=1)
		|| mouse_x >= TOOLBAR_MAX_BUTTON * toolbar_button_width || mouse_y >= toolbar_button_height)
	{
		return -1;//no interface buttons were selected
	}

	idx = mouse_x / toolbar_button_width;
	if (left_click == 1)
	{
#ifdef EYE_CANDY
		eye_candy_done_adding_effect();
#endif

		switch (idx)
		{
			case TOOLBAR_BUTTON_MODE_TILE:
				cur_mode = mode_tile;
				break;
			case TOOLBAR_BUTTON_MODE_2D:
				cur_mode = mode_2d;
				break;
			case TOOLBAR_BUTTON_MODE_3D:
				cur_mode = mode_3d;
				break;
			case TOOLBAR_BUTTON_MODE_PARTICLES:
				cur_mode = mode_particles;
				break;
#ifdef	EYE_CANDY
			case TOOLBAR_BUTTON_MODE_EYE_CANDY:
				cur_mode = mode_eye_candy;
				break;
#endif	//EYE_CANDY
			case TOOLBAR_BUTTON_MODE_LIGHT:
				cur_mode = mode_light;
				break;
			case TOOLBAR_BUTTON_MODE_HEIGHT:
				cur_mode = mode_height;
				break;
			case TOOLBAR_BUTTON_MODE_MAP:
				cur_mode = mode_map;
				break;
			case TOOLBAR_BUTTON_TOOL_SELECT:
				cur_tool = tool_select;
				break;
			case TOOLBAR_BUTTON_TOOL_CLONE:
				cur_tool = tool_clone;
				break;
			case TOOLBAR_BUTTON_TOOL_NEW:
			{
				if (cur_mode == mode_3d)
				{
					SDL_Event event;
					open_3d_obj();
					while (SDL_PollEvent (&event));	//clears all the events
					left_click = 2;
				}
				else if (cur_mode == mode_2d)
				{
					SDL_Event event;
					open_2d_obj();
					while (SDL_PollEvent (&event));	//clears all the events
					left_click = 2;
				}
				else if(cur_mode == mode_particles)
				{
					SDL_Event event;
					open_particles_obj();
					while (SDL_PollEvent (&event));	//clears all the events
					left_click = 2;
				}
#ifdef	EYE_CANDY
				else if(cur_mode == mode_eye_candy)
				{
					SDL_Event event;
//#ifdef LINUX
					open_eye_candy_obj();
//#endif
					while (SDL_PollEvent (&event));	//clears all the events
					left_click = 2;
				}
#endif	//EYE_CANDY
				else if (cur_mode == mode_tile)
				{
					display_tiles_list();
					cur_tool = tool_select;
					selected_tile=255;
				}
				else if (cur_mode == mode_height)
				{
					display_heights_list();
					cur_tool = tool_select;
					selected_height = -1;
				}
				else if (cur_mode == mode_light)
				{
					cur_tool=tool_select;
					selected_light=add_light(scene_mouse_x,scene_mouse_y,3.0f,1.0f,1.0f,1.0f,1.0f,0);
				}

				break;
			}
			case TOOLBAR_BUTTON_TOOL_KILL:
				cur_tool = tool_kill;
				break;
			case TOOLBAR_BUTTON_SAVE_MAP:
			{
				SDL_Event event;
				save_map_file();
				while (SDL_PollEvent (&event));	//clears all the events
				left_click=2;

				break;
			}
			case TOOLBAR_BUTTON_OPEN_MAP:
			{
				SDL_Event event;
				open_map_file();
				while (SDL_PollEvent (&event));	//clears all the events
				left_click = 2;

				break;
			}
			case TOOLBAR_BUTTON_NEW_MAP:
				display_new_map_menu();
				break;
			case TOOLBAR_MAX_BUTTON:
				// Not a button, sould not get here anyway
				break;
		}
	}
	else if (right_click == 1)
	{
		switch (idx)
		{
			case TOOLBAR_BUTTON_MODE_TILE:
				view_tile = !view_tile;
				break;
			case TOOLBAR_BUTTON_MODE_2D:
				view_2d = !view_2d;
				break;
			case TOOLBAR_BUTTON_MODE_3D:
				view_3d = !view_3d;
				break;
			case TOOLBAR_BUTTON_MODE_PARTICLES:
				if(shift_on)
					view_particle_handles = !view_particle_handles;
				else
					view_particles = !view_particles;
				break;
			case TOOLBAR_BUTTON_MODE_LIGHT:
				view_light = !view_light;
				break;
			case TOOLBAR_BUTTON_MODE_HEIGHT:
				view_height = !view_height;
				break;
			default:
				// ignore
				break;
		}
	}

	return 1;
}

void check_toolbar_mouseover(void)
{
	if (mouse_x < TOOLBAR_MAX_BUTTON * toolbar_button_width && mouse_y < toolbar_button_height)
		toolbar_mouseover = mouse_x / toolbar_button_width;
	else
		toolbar_mouseover = -1;
}

void get_world_x_y()
{
	float window_ratio;
	float x,y,x1,y1,a,t;
	window_ratio=(GLfloat)window_width/(GLfloat)window_height;

	//x=(float)((mouse_x)*2.8f*(float)zoom_level/(float)window_width)-(2.8f*zoom_level/2.0f);
	x=(float)((mouse_x)*window_ratio*2.0*(float)zoom_level/(float)window_width)-(window_ratio*zoom_level);
	y=(float)((window_height-mouse_y)*2.0f*zoom_level/window_height)-(2.0*zoom_level/2.0f);

	a=(rz)*3.1415926/180;
	t=(rx)*3.1415926/180;

	y=(float)y/(float)cos(t);

	x1=x*cos(a)+y*sin(a);
	y1=y*cos(a)-x*sin(a);

	scene_mouse_x=-camera_x+x1;
	scene_mouse_y=-camera_y+y1;

#ifdef EYE_CANDY
	if ((cur_mode == mode_eye_candy) && (!minimap_on))
	{
//		const float z = -2.2f + height_map[(int)(scene_mouse_y*2)*tile_map_size_x*6+(int)(scene_mouse_x*2)] * 0.2;
		update_eye_candy_position(scene_mouse_x, scene_mouse_y);
	}
#endif
}

void Enter2DMode()
{
		glPushAttrib(GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_LIGHTING);
   		glDisable(GL_DEPTH_TEST);

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_DST_ALPHA,GL_SRC_ALPHA);

    	glViewport(0, 0, window_width, window_height);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0, (GLdouble)window_width, (GLdouble)window_height, 0.0, 0.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
}

void Leave2DMode()
{
	//glDisable(GL_BLEND);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
}

void draw_toolbar_button(toolbar_button_idx idx)
{
	const toolbar_button *button = toolbar + idx;
	int x_start = idx * toolbar_button_width;
	int x_end = (idx+1) * toolbar_button_width;
	int y_start = 0;
	int y_end = toolbar_button_height;
	draw_2d_thing(
		button->u_start, button->v_start,
		button->u_end,   button->v_end,
		x_start,         y_start,
		x_end,           y_end
	);
}

void draw_2d_thing(float u_start,float v_start,float u_end,float v_end,int x_start,int y_start,int x_end,int y_end)
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

void draw_toolbar()
{
	bind_texture(buttons_text);

	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_QUADS);

	if(cur_mode!=mode_tile && view_tile)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_tile && cur_mode!=mode_tile)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_MODE_TILE);

	if(cur_mode!=mode_2d && view_2d)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_2d && cur_mode!=mode_2d)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_MODE_2D);

	if(cur_mode!=mode_3d && view_3d)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_3d && cur_mode!=mode_3d)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_MODE_3D);

	if(cur_mode!=mode_particles && view_particles)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_particles && cur_mode!=mode_particles)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_MODE_PARTICLES);

#ifdef	EYE_CANDY
	if(cur_mode!=mode_eye_candy && view_eye_candy)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_eye_candy && cur_mode!=mode_eye_candy)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_MODE_EYE_CANDY);
#endif	//EYE_CANDY

	if(cur_mode!=mode_light && view_light)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_light && cur_mode!=mode_light)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_MODE_LIGHT);

	if(cur_mode!=mode_height && view_height)
	glColor3f(1.0f,1.0f,1.0f);
	else if(!view_height && cur_mode!=mode_height)glColor3f(0.3f,0.3f,0.3f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_MODE_HEIGHT);

	if(cur_mode!=mode_map)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_MODE_MAP);

	if(cur_tool!=tool_select)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_TOOL_SELECT);

	if(cur_tool!=tool_clone)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_TOOL_CLONE);

	if(cur_tool!=tool_new)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_TOOL_NEW);

	if(cur_tool!=tool_kill)
	glColor3f(1.0f,1.0f,1.0f);
	else glColor3f(0.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_TOOL_KILL);

	glColor3f(1.0f,1.0f,1.0f);
	draw_toolbar_button(TOOLBAR_BUTTON_SAVE_MAP);
	draw_toolbar_button(TOOLBAR_BUTTON_OPEN_MAP);
	draw_toolbar_button(TOOLBAR_BUTTON_NEW_MAP);

	glEnd();

	if (toolbar_mouseover >= 0 && toolbar_mouseover < TOOLBAR_MAX_BUTTON && view_tooltips)
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		draw_string(mouse_x + TOOLTIP_MOUSE_X_SHIFT, mouse_y + TOOLTIP_MOUSE_Y_SHIFT,
			(const unsigned char *) toolbar[toolbar_mouseover].tooltip, 1);
	}
}

void draw_3d_obj_info()
{
	unsigned char str[200];
	int x_menu,y_menu;
	if (cur_mode!=mode_3d || selected_3d_object==-1 || objects_list[selected_3d_object] == NULL)
		return;
		
	x_menu=0;
	y_menu=window_height-72;
	//draw a black rectangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(x_menu,y_menu+70,0);
	glVertex3i(x_menu,y_menu,0);
	glVertex3i(x_menu+600,y_menu,0);
	glVertex3i(x_menu+600,y_menu+70,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	x_menu+=2;
	y_menu+=2;

	sprintf((char *)str, "X Pos: %03.2f",objects_list[selected_3d_object]->x_pos);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Y Pos: %03.2f",objects_list[selected_3d_object]->y_pos);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Pos: %03.2f",objects_list[selected_3d_object]->z_pos);
	draw_string(x_menu,y_menu,str,1);
/////////////////////////////////////////////////
	x_menu+=12*12;
	y_menu-=17*2;

	sprintf((char *)str, "X Rot: %03.2f",objects_list[selected_3d_object]->x_rot);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Rot: %03.2f",objects_list[selected_3d_object]->z_rot);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Y Rot: %03.2f",objects_list[selected_3d_object]->y_rot);
	draw_string(x_menu,y_menu,str,1);
/////////////////////////////////////////////////
	x_menu+=12*12;
	y_menu-=17*2;

	sprintf((char *)str, "Red  : %03.2f",objects_list[selected_3d_object]->color[0]);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Green: %03.2f",objects_list[selected_3d_object]->color[1]);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Blue : %03.2f",objects_list[selected_3d_object]->color[2]);
	draw_string(x_menu,y_menu,str,1);
///////////////////////////////////////////////////////////////////////
	x_menu+=12*12;
	y_menu-=17*2;

	sprintf((char *)str, "[B]lended : %s",objects_list[selected_3d_object]->blended ? "Yes" : "No");
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Self [L]it: %s",objects_list[selected_3d_object]->self_lit ? "Yes" : "No");
	draw_string(x_menu,y_menu,str,1);


	y_menu+=17;
	sprintf((char *)str, "Object ID: %i",selected_3d_object);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	x_menu-=12*12*3;
	snprintf((char *)str, sizeof(str), "FN: %s",objects_list[selected_3d_object]->file_name);
	draw_string(x_menu,y_menu,str,1);
}

void draw_2d_obj_info()
{
	unsigned char str[128];
	int x_menu,y_menu;
	if(cur_mode!=mode_2d || selected_2d_object==-1||obj_2d_list[selected_2d_object]==NULL)return;

	x_menu=0;
	y_menu=window_height-72;
	//draw a black rectangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(x_menu,y_menu+70,0);
	glVertex3i(x_menu,y_menu,0);
	glVertex3i(x_menu+600,y_menu,0);
	glVertex3i(x_menu+600,y_menu+70,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


	x_menu+=2;
	y_menu+=2;

	sprintf((char *)str, "X Pos: %03.2f",obj_2d_list[selected_2d_object]->x_pos);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Y Pos: %03.2f",obj_2d_list[selected_2d_object]->y_pos);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Pos: %03.2f",obj_2d_list[selected_2d_object]->z_pos);
	draw_string(x_menu,y_menu,str,1);
/////////////////////////////////////////////////
	x_menu+=12*12;
	y_menu-=17*2;

	sprintf((char *)str, "X Rot: %03.2f",obj_2d_list[selected_2d_object]->x_rot);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Rot: %03.2f",obj_2d_list[selected_2d_object]->z_rot);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Object ID: %i",selected_2d_object);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	x_menu-=12*12;
	snprintf((char *)str, sizeof(str), "FN: %s",obj_2d_list[selected_2d_object]->file_name);
	draw_string(x_menu,y_menu,str,1);

}

void draw_light_info()
{
	unsigned char str[128];
	int x_menu,y_menu;
	if(cur_mode!=mode_light || selected_light==-1 || lights_list[selected_light] == NULL)return;

	x_menu=0;
	y_menu=window_height-72;
	//draw a black rectangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(x_menu,y_menu+70,0);
	glVertex3i(x_menu,y_menu,0);
	glVertex3i(x_menu+400,y_menu,0);
	glVertex3i(x_menu+400,y_menu+70,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


	x_menu+=2;
	y_menu+=2;

	sprintf((char *)str, "X Pos: %03.2f",lights_list[selected_light]->pos_x);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Y Pos: %03.2f",lights_list[selected_light]->pos_y);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Pos: %03.2f",lights_list[selected_light]->pos_z);
	draw_string(x_menu,y_menu,str,1);
/////////////////////////////////////////////////
	x_menu+=12*12;
	y_menu-=17*2;

	sprintf((char *)str, "R: %03.2f",lights_list[selected_light]->r);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "G: %03.2f",lights_list[selected_light]->g);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "B: %03.2f",lights_list[selected_light]->b);
	draw_string(x_menu,y_menu,str,1);

	x_menu+=12*8;
	y_menu-=17*2;

	sprintf((char *)str, "Light ID: %i",selected_light);
	draw_string(x_menu,y_menu,str,1);
}



void draw_height_info()
{
	unsigned char str[128];
	int x_menu,y_menu;
	if(cur_mode!=mode_height || selected_height==-1)return;

	x_menu=0;
	y_menu=window_height-62;
	//draw a black rectangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(x_menu,y_menu+60,0);
	glVertex3i(x_menu,y_menu,0);
	glVertex3i(x_menu+200,y_menu,0);
	glVertex3i(x_menu+200,y_menu+60,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


	x_menu+=2;
	y_menu+=2;

	sprintf((char *)str, "X Pos: %i",(int)(scene_mouse_x*2.0f));
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Y Pos: %i",(int)(scene_mouse_y*2.0f));
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "Z Pos: %03.2f",(float)selected_height*0.2f-11.0f*0.2f);
	draw_string(x_menu,y_menu,str,1);
}



int tiles_win = 0;

int display_tiles_handler(window_info *win)
{

	int i,j;
	int x_start,y_start,x_end,y_end;

	//draw a black rectangle
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3i(0,win->len_y-1,0);
	glVertex3i(0,20,0);
	glVertex3i(win->len_x-1,20,0);
	glVertex3i(win->len_x-1,win->len_y-1,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	for(i=tile_offset,j=0;i<tiles_no && j<64 ;i++,j++)
		{

			x_start=48*(j%8);
			x_end=x_start+47;

			y_start=48*(j/8);
			y_start+=20;
			y_end=y_start+47;

			if(!i)//we have a lake tile
			{
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_LIGHTING);
				glColor3f(0,0.6f,1.0f);
				glBegin(GL_QUADS);
				glVertex3i(x_start,y_end,0);
				glVertex3i(x_start,y_start,0);
				glVertex3i(x_end,y_start,0);
				glVertex3i(x_end,y_end,0);
				glEnd();
				glEnable(GL_LIGHTING);
				glEnable(GL_TEXTURE_2D);
			}
			else
			{
				bind_texture(tile_list[i]);

				glBegin(GL_QUADS);
				glTexCoord2f(0,0.0f);
				glVertex3i(x_start,y_end,0);

				glTexCoord2f(0,1.0f);
				glVertex3i(x_start,y_start,0);

				glTexCoord2f(1.0f,1.0f);
				glVertex3i(x_end,y_start,0);

				glTexCoord2f(1.0f,0.0f);
				glVertex3i(x_end,y_end,0);

				glEnd();
			}
		}

	return 1;
}

int check_tiles_interface (window_info *win, int _x, int _y)
{
	int tile_id;

	if (_x > win->len_x - 20 && mouse_y < 20)
	{
		return 0;
	}
	
	if (_y < 20) return 0; 

	tile_id = 8 * ( (_y-20) / 48 ) + _x / 48;	
	tile_id+=tile_offset;
	if(tile_id>tiles_no)return 0;//check to see if we clicked on an empty tile

	cur_tool=tool_select;
	selected_tile=tile_id;

	return 1;
}

void display_tiles_list()
{
	if(tiles_win <= 0){
		tiles_win = create_window("tiles", 0, 0, 64, 128, 48*8, 48*8+20, ELW_WIN_DEFAULT);

		set_window_handler(tiles_win, ELW_HANDLER_DISPLAY, &display_tiles_handler );
		set_window_handler(tiles_win, ELW_HANDLER_CLICK, &check_tiles_interface );
		
	} else {
		show_window(tiles_win);
		select_window(tiles_win);
	}
	display_window(tiles_win);
}



int height_win = 0;

int display_heights_handler(window_info *win)
{

	int i;
	int x_start,y_start,x_end,y_end;

	//draw a black rectangle
	//n_tile_menu_offset isn't needed in a window...
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3i(0,win->len_y-1,0);
	glVertex3i(0,20,0);
	glVertex3i(win->len_x-1,20,0);
	glVertex3i(win->len_x-1,win->len_y-1,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	
	for(i=0;i<32;i++)
		{

			x_start=32*(i%8);
			x_end=x_start+31;

			y_start=32*(i/8);
			y_start+=20;
			y_end=y_start+31;

			change_color_height(i);
			glBegin(GL_QUADS);

			glVertex3i(x_start,y_end,0);
			glVertex3i(x_start,y_start,0);
			glVertex3i(x_end,y_start,0);
			glVertex3i(x_end,y_end,0);

			glEnd();
		}
	glEnable(GL_TEXTURE_2D);
	
	return 1;
}

int check_height_interface (window_info *win, int _x, int _y)
{
	int height_id;
	
	if (_y < 20) return 0; 
	//check to see if we clicked outside our rectangle

	height_id = 8 * ( (_y - 20) / 32 ) + _x / 32;
	cur_tool=tool_select;
	selected_height=height_id;
	
	return 1;                 // Disable clickthroughs?
}

void display_heights_list()
{
	if(height_win <= 0){
		height_win = create_window("heights", 0, 0, 64, 128, 32*8, 32*4+20, ELW_WIN_DEFAULT);

		set_window_handler(height_win, ELW_HANDLER_DISPLAY, &display_heights_handler );
		set_window_handler(height_win, ELW_HANDLER_CLICK, &check_height_interface );
		
	} else {
		show_window(height_win);
		select_window(height_win);
	}
	display_window(height_win);
}

void get_minimap_dimensions(int *x, int *y, int *width, int *height)
{
	int tile_size = min2i(window_width / tile_map_size_x, window_height / tile_map_size_y);
	int w = tile_map_size_x * tile_size;
	int h = tile_map_size_y * tile_size;

	if (x)
		*x = (window_width - w) / 2;
	if (y)
		*y = (window_height - h) / 2;
	if (width)
		*width = w;
	if (height)
		*height = h;
}

void check_mouse_minimap()
{
	int x0, y0, y1, width, height;
	get_minimap_dimensions(&x0, &y0, &width, &height);
	y1 = y0 + height;

	if (mouse_x < x0 || mouse_y < y0 || mouse_x > x0 + width || mouse_y > y1)
		return;

	mx = (float)(mouse_x - x0) * 3 * tile_map_size_x / width;
	my = (float)(y1 - mouse_y) * 3 * tile_map_size_y / height;
	minimap_on = 0;
}

void draw_mouse_minimap()
{
	int x0, y0, y1, width, height, x, y;

	get_minimap_dimensions(&x0, &y0, &width, &height);
	y1 = y0 + height;

	if (mouse_x < x0 || mouse_y < y0 || mouse_x > x0 + width || mouse_y > y1)
		return;

	mx = (float)(mouse_x - x0) * 3 * tile_map_size_x / width;
	my = (float)(y1 - mouse_y) * 3 * tile_map_size_y / height;
	x = (int)(mx) / 3;
	y = (int)(my) / 3;

	for (int xp = max2i(0, x-2); xp < min2i(tile_map_size_x, x + 2); x++)
	{
		for (int yp = max2i(0, y-2); yp < min2i(tile_map_size_y, y + 2); y++)
		{
			tile_map[y * tile_map_size_x + x] = ((cur_tool==tool_kill) ? 255 : selected_tile);
			if (cur_tool==tool_kill || selected_tile == 0 || selected_tile == 20 || selected_tile == 21)
			{
				kill_height_map_at_texture_tile(yp * tile_map_size_x + xp);
			}
		}
	}

	map_has_changed=1;
}

int map_has_changed=1;

void generate_ground_tiles_texture(GLuint *texture)
{
	// Drawing full textures for individual tiles makes them much too small, so you cannot
	// make out the details of the texture. With below constant, the map is divided into
	// tile_text_frac x tile_text_frac full sized textures.
	static const int tile_text_frac = 4;

	unsigned char prev_text_id, *text_id;
	int x0, y0, width, height, tile_size, m;
	get_minimap_dimensions(&x0, &y0, &width, &height);
	tile_size = width / tile_map_size_x;

	m = min2i(tile_map_size_x, tile_map_size_y) / tile_text_frac;

	text_id = tile_map;
	prev_text_id = *text_id;
	bind_texture(tile_list[prev_text_id]);
	glBegin(GL_QUADS);
	for (int i = 0, y = y0; i < tile_map_size_y; ++i, y += tile_size)
	{
		float ty0 = (float)(i%m) / m;
		float ty1 = (float)(i%m + 1) / m;
		for (int j = 0, x = x0; j < tile_map_size_x; ++j, x += tile_size, ++text_id)
		{
			if (*text_id < 255)
			{
				float tx0 = (float)(j%m) / m;
				float tx1 = (float)(j%m+1) / m;
				if (*text_id != prev_text_id)
				{
					glEnd();
					prev_text_id = *text_id;
					bind_texture(tile_list[prev_text_id]);
					glBegin(GL_QUADS);
				}

				draw_2d_thing(tx0, ty0, tx1, ty1, x, y, x+tile_size, y+tile_size);
			}
		}
	}
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 0.0f, 0.0f);

	// OK, now check the 3d objects... we want them all to show up as red dots...
	glPointSize((float)min2i(window_width, window_height) / 256);
	glBegin(GL_POINTS);
	for (int i = 0; i < MAX_OBJ_3D; i++)
	{
		if (objects_list[i] && objects_list[i]->blended != 20)
		{
			int x = x0 + (tile_size * objects_list[i]->x_pos) / 3;
			int y = y0 + (tile_size * objects_list[i]->y_pos) / 3;
			glVertex2i(x, y);
		}
	}
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	if (!*texture)
		glGenTextures(1, texture);

	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glReadBuffer(GL_BACK);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x0, y0, width, height, 0);
}

void draw_minimap()
{
	static const int marker_size = 7;
	static GLuint ground_tiles_text = 0;

	int x0, y0, x1, y1, width, height;

	if (map_has_changed)
	{
		generate_ground_tiles_texture(&ground_tiles_text);
		map_has_changed = 0;
	}

	get_minimap_dimensions(&x0, &y0, &width, &height);
	x1 = x0 + width;
	y1 = y0 + height + x1-x1;
	glBindTexture(GL_TEXTURE_2D, ground_tiles_text);
	glBegin(GL_QUADS);
	draw_2d_thing(0.0f, 0.0f, 1.0f, 1.0f, x0, y0, x1, y1);
	glEnd();

	if (show_position_on_minimap)
	{
		int tile_size = width / tile_map_size_x;
		int x, y;

		x = x0 + (int)(mx * tile_size) / 3;
		y = y1 - (int)(my * tile_size) / 3;

		glDisable(GL_TEXTURE_2D);
		glColor3f(1.0f, 0.0f, 0.0f);

		glLineWidth((float)min2i(window_width, window_height) / 256);
		glBegin(GL_LINES);
		glVertex2i(x - marker_size, y + marker_size);
		glVertex2i(x + marker_size, y - marker_size);

		glVertex2i(x + marker_size, y + marker_size);
		glVertex2i(x - marker_size, y - marker_size);
		glEnd();

		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
	}

#ifdef EYE_CANDY
	draw_bounds_on_minimap();
#endif
}

int map_size=0;

int new_map_display_handler ()
{
	glColor3f (1.0f,1.0f,0.0f);
	draw_string (2, 2, (const unsigned char*) "    Map Size", 1);
		
	if (map_size == 0)
		glColor3f (0.0f, 0.5f, 1.0f);
	else 
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (2, 2*17+2, (const unsigned char*) "Very Small [16x16]", 1);

	if (map_size == 1)
		glColor3f (0.0f, 0.5f, 1.0f);
	else
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (2, 3*17+2, (const unsigned char*) "Small      [32x32]", 1);
		
	if (map_size == 2)
		glColor3f (0.0f, 0.5f, 1.0f);
	else 
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (2, 4*17+2, (const unsigned char*) "Medium     [64x64]", 1);

	if (map_size == 3)
		glColor3f (0.0f, 0.5f, 1.0f);
	else 
		glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (2, 5*17+2, (const unsigned char*) "Large      [128x128]", 1);

	glColor3f (1.0f, 1.0f, 1.0f);
	draw_string (2, 8*17+2, (const unsigned char*) "   [Ok]    [Cancel]", 1);
	
	return 1;
}

int new_map_click_handler (window_info *win, int _x, int _y)
{
	if (_y > 2*17+2 && _y < 3*17+2)
	{
		map_size = 0;
	}
	else if (_y > 3*17+2 && _y < 4*17+2)
	{
		map_size = 1;
	}
	else if (_y > 4*17+2 && _y < 5*17+2)
	{
		map_size = 2;
	}
	else if (_y > 5*17+2 && _y < 6*17+2)
	{
		map_size = 3;
	}
	else if (_y > 6*17+2 && _y < 7*17+2)
	{
		map_size = 4;
	}
	else if (_y > 8*17+2 && _y < 9*17+2)
	{
		if (_x > 3*12+2 && _x < 7*12+2)
		{
			hide_window (new_map_menu);
			switch (map_size)
			{
				case 0: new_map (16, 16); break;
				case 1: new_map (32, 32); break;
				case 2: new_map (64, 64); break;
				case 3: new_map (128, 128); break;
				case 4: new_map (256, 256); break;
				default: LOG_ERROR ("Unknown map size!");
			}
		}
		else if (_x > 11*12+2 && _x < 19*12+2)
		{
			hide_window (new_map_menu);
		}
	}
	else
	{
		return 0;
	}
	
	return 1;
}

void display_new_map_menu()
{
	if (new_map_menu < 0)
	{
		int x_menu, y_menu, w_menu, h_menu;
		
		x_menu = 90;
		y_menu = 82;
		w_menu = 240;
		h_menu = 170;
		new_map_menu = create_window ("New map", -1, 0, x_menu, y_menu, w_menu, h_menu, ELW_WIN_DEFAULT);
		
		set_window_handler (new_map_menu, ELW_HANDLER_DISPLAY, new_map_display_handler);
		set_window_handler (new_map_menu, ELW_HANDLER_CLICK, new_map_click_handler);
		
		show_window (new_map_menu);select_window (new_map_menu);
	}
	else
	{
		show_window (new_map_menu);
	}
}

void display_map_settings()
{
	unsigned char str[128];
	int x_menu,y_menu;

	if (
	    (cur_mode != mode_map)
	   )
	  return;

	x_menu=0;
	y_menu=window_height-62;
	//draw a black rectangle
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(x_menu,y_menu+170,0);
	glVertex3i(x_menu,y_menu,0);
	glVertex3i(x_menu+250,y_menu,0);
	glVertex3i(x_menu+250,y_menu+170,0);
	glColor3f(1.0f,1.0f,1.0f);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);


	x_menu+=2;
	y_menu+=2;

	sprintf((char *)str, "R: %03.2f",ambient_r);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "G: %03.2f",ambient_g);
	draw_string(x_menu,y_menu,str,1);

	y_menu+=17;
	sprintf((char *)str, "B: %03.2f",ambient_b);
	draw_string(x_menu,y_menu,str,1);

	y_menu-=17*2;
	x_menu+=90;

	sprintf((char *)str, "[D]ungeon: %s",dungeon ? "Yes" : "No");
	draw_string(x_menu,y_menu,str,1);

}

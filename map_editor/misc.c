#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifndef _MSC_VER
  #include <dirent.h>
  #include <errno.h>
#endif //_MSC_VER
#include <sys/types.h>
#include <sys/stat.h>
#include "tiles.h"
#include "../asc.h"
#include "global.h"
//#ifdef OSX
//#include <ApplicationServices/ApplicationServices.h>
//#endif
#ifdef EYE_CANDY
#include "eye_candy_window.h"
#endif

extern char* selected_file;

char* selected_file;
float grid_height=0.02f;

void draw_checkbox(int startx, int starty, int checked)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINE_LOOP);
		glVertex2i(startx,starty);
		glVertex2i(startx+17,starty);
		glVertex2i(startx+17,starty+17);
		glVertex2i(startx,starty+17);
	glEnd();

	if(checked){
		glBegin(GL_LINES);
			glVertex2i(startx,starty);
			glVertex2i(startx+17,starty+17);
			
			glVertex2i(startx+17,starty);
			glVertex2i(startx,starty+17);
		glEnd();
	}

	glEnable(GL_TEXTURE_2D);
	glPopAttrib();
}

void kill_height_map_at_texture_tile(int tex_pos){
  int start_point;
  int h_x, h_y;
  start_point = (tex_pos*6)+((30*tile_map_size_x)*(tex_pos/tile_map_size_x));
  for(h_y=0; h_y<6; h_y++){
    for(h_x=0; h_x<6; h_x++){
      height_map[start_point+h_x+(h_y*tile_map_size_x*6)]=0;
    }
  }
}

/*
int evaluate_colision()
{
	char pixels[16]={0};
	glReadBuffer(GL_BACK);
	glReadPixels(mouse_x, window_height-mouse_y, 1, 1, GL_LUMINANCE, GL_BYTE, &pixels);
	if(pixels[0])return 1;//there is something
	return 0;//no collision, sorry
}
*/

int evaluate_colision (float *ref)
{
	float z;
	glReadBuffer (GL_BACK);
	glReadPixels (mouse_x, window_height-mouse_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
	printf("<ref, z>: <%f, %f>\n", *ref, z);
	if(z < *ref) 
	{
		*ref = z;
		return 1;//there is something
	}
	return 0; //no collision, sorry
}

void get_3d_object_under_mouse()
{
	//ok, first of all, let's see what objects we have in range...
	int i, count, x, y, dist1, dist2, hits;
	GLint viewport[4];
	GLuint *buffer, z_coordinate;
	double matrix[16];
	
	selected_3d_object = -1;
	x = (int)-camera_x;
	y = (int)-camera_y;

	count = 0;
	for (i = 0; i < MAX_OBJ_3D; i++)
	{
		if (objects_list[i] && objects_list[i]->blended != 20)
		{
			dist1 = x - (int)objects_list[i]->x_pos;
			dist2 = y - (int)objects_list[i]->y_pos;
			if (dist1 * dist1 + dist2 * dist2 <= ((40 * 40) * (zoom_level / 15.75f)))
			{
				count++;
			}
		}
	}

#ifndef EYE_CANDY
	if (count == 0)
	{
		return;
	}
#endif
	count++;
	count *= 4;
	glGetIntegerv(GL_VIEWPORT, viewport);
	buffer = malloc(count * sizeof(GLuint));
	glSelectBuffer(count, buffer);

	glGetDoublev(GL_PROJECTION_MATRIX, matrix);

	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(mouse_x, window_height-mouse_y, 1.0, 1.0, viewport);
	glMultMatrixd(matrix);
	glMatrixMode(GL_MODELVIEW);

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glPushMatrix();
	glLoadIdentity();					// Reset The Matrix
	Move();

	for (i = 0; i < MAX_OBJ_3D; i++)
	{
		if (objects_list[i] && objects_list[i]->blended != 20)
		{
			dist1 = x - (int)objects_list[i]->x_pos;
			dist2 = y - (int)objects_list[i]->y_pos;
			if (dist1 * dist1 + dist2 * dist2 <= ((40 * 40) * (zoom_level / 15.75f)))
			{
				glLoadName(i);
				draw_3d_object(objects_list[i]);
			}
		}
	}

#ifdef EYE_CANDY
	if (cur_mode == mode_eye_candy)
	{
	  draw_eye_candy_selectors();
	}
#endif

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_CULL_FACE);
	glDisableClientState(GL_VERTEX_ARRAY);

	hits = glRenderMode(GL_RENDER);
	z_coordinate = 0xFFFFFFFF;
	for (i = 0; i < hits; i++)
	{
		if (buffer[(i * 4) + 1] < z_coordinate)
		{
			selected_3d_object = buffer[(i*4)+3];
			z_coordinate = buffer[(i*4)+1];
		}
	}
	free(buffer);
}

void kill_3d_object(int object_id)
{
	/*free(objects_list[object_id]);
	objects_list[object_id]=0;//kill any refference to it*/
	objects_list[object_id]->blended=20;
	selected_3d_object=-1;//we have no selected object now...
}

void move_3d_object(int object_id)
{
    float window_ratio;
    float x,y,x1,y1,a,t,t1;
    window_ratio=(GLfloat)window_width/(GLfloat)window_height;

    //x=(float)((mouse_x)*2.8f*(float)zoom_level/(float)window_width)-(2.8f*zoom_level/2.0f);
    x=(float)((mouse_x)*window_ratio*2.0*(float)zoom_level/(float)window_width)-(window_ratio*zoom_level);
    y=(float)((window_height-mouse_y)*2.0f*zoom_level/window_height)-(2.0*zoom_level/2.0f);

    a=(rz)*3.1415926/180;
    t=(rx)*3.1415926/180;
    t1=(rx+90)*3.1415926/180;

    y=((float)y - objects_list[object_id]->z_pos*cos(t1))/(float)cos(t);

    x1=x*cos(a)+y*sin(a);
    y1=y*cos(a)-x*sin(a);

    objects_list[object_id]->x_pos = -camera_x+x1;
	objects_list[object_id]->y_pos = -camera_y+y1;
}

void clone_3d_object(int object_id)
{
	float z_pos,x_rot,y_rot,z_rot,r,g,b;
	char self_lit,blended;

	z_pos=objects_list[object_id]->z_pos;
	x_rot=objects_list[object_id]->x_rot;
	y_rot=objects_list[object_id]->y_rot;
	z_rot=objects_list[object_id]->z_rot;
	self_lit=objects_list[object_id]->self_lit;
	blended=objects_list[object_id]->blended;
	r=objects_list[object_id]->color[0];
	g=objects_list[object_id]->color[1];
	b=objects_list[object_id]->color[2];

	z_pos=(z_pos == 0.01f)?0.02f:(z_pos == 0.02f?0.01f:z_pos);

	selected_3d_object=add_e3d(objects_list[object_id]->file_name,scene_mouse_x,scene_mouse_y,z_pos,x_rot,y_rot,z_rot,self_lit,blended,r,g,b);
	cur_tool=tool_select;//change the current tool
}

#ifndef LINUX
void get_proper_path (const char *path, const char *dir, char *proper_path, int size)
{
	int i, j;
	int dir_len = strlen (dir);
	
	if (strncmp (path, dir, dir_len) != 0)
	{
		LOG_ERROR ("Unable to strip off relative path!");
		exit (1);
	}
	
	j = 0;
	proper_path[j++] = '.';
	if (path[dir_len] != '\\' && path[dir_len] != '/')
		proper_path[j++] = '/';
	for (i = dir_len; path[i] != '\0'; i++)
	{
		if (path[i] == '\\')
			proper_path[j++] = '/';
		else
			proper_path[j++] = path[i];
		if (j >= size)
		{
			LOG_ERROR ("buffer for proper path too small!");
			exit (1);
		}
	}
	proper_path[j] = '\0';
}	

void open_3d_obj()
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH], temp[MAX_PATH];

  ZeroMemory (&ofn, sizeof (ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof (ofn);
  ofn.hwndOwner = 0;
  ofn.lpstrFilter = "Custom e3d (*.e3d)\0*.e3d\0Compressed e3d (*.e3d.gz)\0*.e3d.gz\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "e3d";
  strcpy(temp,exec_path);
  strcat(temp,"\\3dobjects\\");
  ofn.lpstrInitialDir = temp;

    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (GetOpenFileName (&ofn))
    {
		char proper_path[128];
		get_proper_path (szFileName, exec_path, proper_path, sizeof (proper_path) );
		if(!strcmp(proper_path+strlen(proper_path)-3, ".gz")) proper_path[strlen(proper_path)-3]= '\0';
		selected_3d_object=add_e3d(proper_path,scene_mouse_x,scene_mouse_y,0,0,0,0,0,0,0,0,0);
		cur_tool=tool_select;//change the current tool
    }

}
#endif
////////////////////////////////////////////////////////////////////////////////////
//////////////////////2D stuff here/////////////////////////////////////////////////
void get_2d_object_under_mouse()
{
	//ok, first of all, let's see what objects we have in range...
	int i, count, x, y, dist1, dist2, hits;
	GLint viewport[4];
	GLuint *buffer, z_coordinate;
	double matrix[16];

	selected_2d_object = -1;
	x = (int)-camera_x;
	y = (int)-camera_y;

	count = 0;
	for (i = 0; i < MAX_OBJ_2D; i++)
	{
		if (obj_2d_list[i])
		{
			dist1 = x - (int)obj_2d_list[i]->x_pos;
			dist2 = y - (int)obj_2d_list[i]->y_pos;
			if (dist1 * dist1 + dist2 * dist2 <= ((40 * 40) * (zoom_level / 15.75f)))
			{
				count++;
			}
		}
	}

	if (count == 0)
	{
		return;
	}
	count++;
	count *= 4;
	glGetIntegerv(GL_VIEWPORT, viewport);
	buffer = malloc(count * sizeof(GLuint));
	glSelectBuffer(count, buffer);

	glGetDoublev(GL_PROJECTION_MATRIX, matrix);

	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(mouse_x, window_height-mouse_y, 1.0, 1.0, viewport);
	glMultMatrixd(matrix);
	glMatrixMode(GL_MODELVIEW);

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glPushMatrix();
	glLoadIdentity();					// Reset The Matrix
	Move();

	for (i = 0; i < MAX_OBJ_2D; i++)
	{
		if (obj_2d_list[i])
		{
			dist1 = x - (int)obj_2d_list[i]->x_pos;
			dist2 = y - (int)obj_2d_list[i]->y_pos;
			if (dist1 * dist1 + dist2 * dist2 <= ((40 * 40) * (zoom_level / 15.75f)))
			{
				glLoadName(i);
				draw_2d_object(obj_2d_list[i]);
			}
		}
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	hits = glRenderMode(GL_RENDER);
	z_coordinate = 0xFFFFFFFF;
	for (i = 0; i < hits; i++)
	{
		if (buffer[(i * 4) + 1] < z_coordinate)
		{
			selected_2d_object = buffer[(i*4)+3];
			z_coordinate = buffer[(i*4)+1];
		}
	}
	free(buffer);
}

void kill_2d_object(int object_id)
{
	free(obj_2d_list[object_id]);
	obj_2d_list[object_id]=0;//kill any refference to it
	selected_2d_object=-1;//we have no selected object now...
}

void move_2d_object(int object_id)
{
	obj_2d_list[object_id]->x_pos=scene_mouse_x;
	obj_2d_list[object_id]->y_pos=scene_mouse_y;
}

void clone_2d_object(int object_id)
{
	float z_pos,x_rot,z_rot;
	int i,collide;
	static int up=0;

	z_pos=obj_2d_list[object_id]->z_pos;
	x_rot=obj_2d_list[object_id]->x_rot;
	z_rot=obj_2d_list[object_id]->z_rot;

	if(ctrl_on)
		{
			collide=0;
			for(i=0;i<MAX_OBJ_2D;i++)
				{
					if(obj_2d_list[i])
						{
							int dist1;
							int dist2;

							dist1=(int)(scene_mouse_x-obj_2d_list[i]->x_pos);
							dist2=(int)(scene_mouse_y-obj_2d_list[i]->y_pos);
							if(dist1*dist1+dist2*dist2<=1)
								collide++;
						}
				}
			if(collide>1)
				{
					up++;
					if(up>=4)
						up=0;
					if(up)
						z_pos+=0.01;
					else
						z_pos-=0.04;
					if(z_pos<0.01)
						z_pos=0.01;
				}
			z_rot=rand()%360;
		}

	selected_2d_object=add_2d_obj(obj_2d_list[object_id]->file_name,scene_mouse_x,scene_mouse_y,z_pos,x_rot,0,z_rot);
	cur_tool=tool_select;//change the current tool
}

#ifndef LINUX
void open_2d_obj()
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH],temp[MAX_PATH];

  ZeroMemory (&ofn, sizeof (ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof (ofn);
  ofn.hwndOwner = 0;
  ofn.lpstrFilter = "2D Object (*.2d0)\0*.2d0\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "2d0";
  strcpy(temp,exec_path);
  strcat(temp,"\\2dobjects\\");
  ofn.lpstrInitialDir = temp;

    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |OFN_NOCHANGEDIR;
    if (GetOpenFileName (&ofn))
    {
		char proper_path[128];
		get_proper_path (szFileName, exec_path, proper_path, sizeof (proper_path) );
				selected_2d_object=add_2d_obj(proper_path,scene_mouse_x,scene_mouse_y,0.001f,0,0,0);
		cur_tool=tool_select;//change the current tool
    }
}
#endif
////////////////////////////////////////////////////////////////////////////
////////////particle things/////////////////////////////////////////////////////
void draw_particle_handle(particle_sys *system)
{
	glColor3f(0.0f,0.1f,0.1f);

	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef (system->x_pos,system->y_pos,system->z_pos);

	glBegin(GL_QUADS);
		glVertex3f(-0.1f, -0.1f,  0.1f);	// Bottom Left Of The Texture and Quad
		glVertex3f( 0.1f, -0.1f,  0.1f);	// Bottom Right Of The Texture and Quad
		glVertex3f( 0.1f,  0.1f,  0.1f);	// Top Right Of The Texture and Quad
		glVertex3f(-0.1f,  0.1f,  0.1f);	// Top Left Of The Texture and Quad
		// Back Face
		glVertex3f(-0.1f, -0.1f, -0.1f);	// Bottom Right Of The Texture and Quad
		glVertex3f(-0.1f,  0.1f, -0.1f);	// Top Right Of The Texture and Quad
		glVertex3f( 0.1f,  0.1f, -0.1f);	// Top Left Of The Texture and Quad
		glVertex3f( 0.1f, -0.1f, -0.1f);	// Bottom Left Of The Texture and Quad
		// Top Face
		glVertex3f(-0.1f,  0.1f, -0.1f);	// Top Left Of The Texture and Quad
		glVertex3f(-0.1f,  0.1f,  0.1f);	// Bottom Left Of The Texture and Quad
		glVertex3f( 0.1f,  0.1f,  0.1f);	// Bottom Right Of The Texture and Quad
		glVertex3f( 0.1f,  0.1f, -0.1f);	// Top Right Of The Texture and Quad
		// Bottom Face
		glVertex3f(-0.1f, -0.1f, -0.1f);	// Top Right Of The Texture and Quad
		glVertex3f( 0.1f, -0.1f, -0.1f);	// Top Left Of The Texture and Quad
		glVertex3f( 0.1f, -0.1f,  0.1f);	// Bottom Left Of The Texture and Quad
		glVertex3f(-0.1f, -0.1f,  0.1f);	// Bottom Right Of The Texture and Quad
		// Right face
		glVertex3f( 0.1f, -0.1f, -0.1f);	// Bottom Right Of The Texture and Quad
		glVertex3f( 0.1f,  0.1f, -0.1f);	// Top Right Of The Texture and Quad
		glVertex3f( 0.1f,  0.1f,  0.1f);	// Top Left Of The Texture and Quad
		glVertex3f( 0.1f, -0.1f,  0.1f);	// Bottom Left Of The Texture and Quad
		// Left Face
		glVertex3f(-0.1f, -0.1f, -0.1f);	// Bottom Left Of The Texture and Quad
		glVertex3f(-0.1f, -0.1f,  0.1f);	// Bottom Right Of The Texture and Quad
		glVertex3f(-0.1f,  0.1f,  0.1f);	// Top Right Of The Texture and Quad
		glVertex3f(-0.1f,  0.1f, -0.1f);	// Top Left Of The Texture and Quad
	glEnd();

	glPopMatrix();//restore the scene
}

void display_particle_handles()
{
	int i;

	glDisable(GL_TEXTURE_2D);
	LOCK_PARTICLES_LIST();
	for(i=0;i<MAX_PARTICLE_SYSTEMS;i++)
	{
		if(particles_list[i])
			draw_particle_handle(particles_list[i]);
	}
	UNLOCK_PARTICLES_LIST();
	glEnable(GL_TEXTURE_2D);
}

void get_particles_object_under_mouse()
{
	//ok, first of all, let's see what objects we have in range...
	int i, count, x, y, dist1, dist2, hits;
	GLint viewport[4];
	GLuint *buffer, z_coordinate;
	double matrix[16];

	selected_particles_object = -1;
	x = (int)-camera_x;
	y = (int)-camera_y;

	count = 0;

	LOCK_PARTICLES_LIST();
	for (i = 0; i < MAX_PARTICLE_SYSTEMS; i++)
	{
		if (particles_list[i])
		{
			dist1 = x - (int)particles_list[i]->x_pos;
			dist2 = y - (int)particles_list[i]->y_pos;
			if (dist1 * dist1 + dist2 * dist2 <= ((40 * 40) * (zoom_level / 15.75f)))
			{
				count++;
			}
		}
	}

	if (count == 0)
	{
		UNLOCK_PARTICLES_LIST();
		return;
	}
	count++;
	count *= 4;
	glGetIntegerv(GL_VIEWPORT, viewport);
	buffer = malloc(count * sizeof(GLuint));
	glSelectBuffer(count, buffer);

	glGetDoublev(GL_PROJECTION_MATRIX, matrix);

	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(mouse_x, window_height-mouse_y, 1.0, 1.0, viewport);
	glMultMatrixd(matrix);
	glMatrixMode(GL_MODELVIEW);

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();					// Reset The Matrix
	Move();

	for (i = 0; i < MAX_PARTICLE_SYSTEMS; i++)
	{
		if (particles_list[i])
		{
			dist1 = x - (int)particles_list[i]->x_pos;
			dist2 = y - (int)particles_list[i]->y_pos;
			if (dist1 * dist1 + dist2 * dist2 <= ((40 * 40) * (zoom_level / 15.75f)))
			{
				glLoadName(i);
				draw_particle_handle(particles_list[i]);
			}
		}
	}

	UNLOCK_PARTICLES_LIST();
	glEnable(GL_TEXTURE_2D);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	hits = glRenderMode(GL_RENDER);
	z_coordinate = 0xFFFFFFFF;
	for (i = 0; i < hits; i++)
	{
		if (buffer[(i * 4) + 1] < z_coordinate)
		{
			selected_particles_object = buffer[(i*4)+3];
			z_coordinate = buffer[(i*4)+1];
		}
	}
	free(buffer);
}

void kill_particles_object(int object_id) {
	if(!particles_list[object_id])return;
	LOCK_PARTICLES_LIST();
	if(particles_list[object_id]->def->use_light){
		free(lights_list[particles_list[object_id]->light]);
		lights_list[particles_list[object_id]->light]=NULL;
	}
	free(particles_list[object_id]);
	particles_list[object_id]=0;//kill any reference to it
	selected_particles_object=-1;//we have no selected object now...
	UNLOCK_PARTICLES_LIST();
}

void move_particles_object(int object_id) {
	LOCK_PARTICLES_LIST();
	if(!particles_list[object_id])
		{
			if(object_id==selected_particles_object)selected_particles_object=-1;
			UNLOCK_PARTICLES_LIST();
			return;
		}
	particles_list[object_id]->x_pos=scene_mouse_x;
	particles_list[object_id]->y_pos=scene_mouse_y;
	if(particles_list[object_id]->def->use_light){
		move_light(particles_list[object_id]->light);
	}
	UNLOCK_PARTICLES_LIST();
}

void clone_particles_object(int object_id) {
	LOCK_PARTICLES_LIST();
	if(!particles_list[object_id])return;
	selected_particles_object=create_particle_sys(particles_list[object_id]->def,scene_mouse_x,scene_mouse_y,particles_list[object_id]->z_pos);
	UNLOCK_PARTICLES_LIST();
}

////////////////////////////////////////////////////////////////////////////
////////////tile things/////////////////////////////////////////////////////
#ifdef	NEW_TEXTURES
void load_all_tiles()
{
	int i;
	char str[80];

	memset(map_tiles, 0, sizeof(map_tiles));

	for(i = 0; i < 255; i++)
	{
		sprintf(str, "./3dobjects/tile%i.dds", i);

		tiles_no = i;

		load_image_data(str, 1, 1, 1, 0, &map_tiles[i]);
	}
}
#else	/* NEW_TEXTURES */
texture_struct *load_texture(const char * file_name, texture_struct *tex, Uint8 alpha);

void load_all_tiles()
{
	int i;
	int cur_text;
	char str[80];
	for(i=0;i<255;i++)
	{
		sprintf(str,"./3dobjects/tile%i.dds",i);
		if(is_water_tile(i) && is_reflecting(i)) cur_text=load_texture_cache(str,70);
		else cur_text=load_texture_cache(str,255);
		if(cur_text==-1)return;
		tile_list[i]=cur_text;
		tiles_no=i;
#ifdef	OLD_TEXTURE_LOADER
		//map_tiles[i].img=load_bmp8_color_key_no_texture_img(str,map_tiles+i,255);
		load_bmp8_texture(str,map_tiles+i,255);
#else	//OLD_TEXTURE_LOADER
		load_texture(str,map_tiles+i,255);
#endif	//OLD_TEXTURE_LOADER
	}
	map_tiles[255].texture=NULL;
}
#endif	/* NEW_TEXTURES */

void move_tile()
{
	float x_start,y_start;

	x_start=scene_mouse_x-1.5f;
	y_start=scene_mouse_y-1.5f;

	if(!selected_tile)//we have a lake tile
	{
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glColor3f(0,0.6f,1.0f);
		glBegin(GL_QUADS);
		glVertex3f(x_start,y_start+3, 0.001f);
		glVertex3f(x_start,y_start, 0.001f);
		glVertex3f(x_start+3, y_start,0.001f);
		glVertex3f(x_start+3, y_start+3,0.001f);
		glEnd();
		glEnable(GL_LIGHTING);
		glEnable(GL_TEXTURE_2D);
	}
	else
	{
#ifdef	NEW_TEXTURES
		bind_texture(tile_list[selected_tile]);
#else	/* NEW_TEXTURES */
		get_and_set_texture_id(tile_list[selected_tile]);
#endif	/* NEW_TEXTURES */
		glBegin(GL_QUADS);

 		glTexCoord2f(0, 0.0f);
		glVertex3f(x_start,y_start+3, 0.001f);
		glTexCoord2f(0, 1.0f);
		glVertex3f(x_start,y_start, 0.001f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(x_start+3, y_start,0.001f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(x_start+3, y_start+3,0.001f);
		glEnd();
	}

}

void draw_light_source(light * object_id)
{
	float x_pos,y_pos,z_pos;

	x_pos=object_id->pos_x;
	y_pos=object_id->pos_y;
	z_pos=object_id->pos_z;

 	glDisable(GL_LIGHTING);
 	glDisable(GL_TEXTURE_2D);
    glColor3f(object_id->r/5.0f,object_id->g/5.0f,object_id->b/5.0f);

	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef (x_pos, y_pos, z_pos);

	glBegin(GL_QUADS);
		glVertex3f(-0.2f, -0.2f,  0.2f);	// Bottom Left Of The Texture and Quad
		glVertex3f( 0.2f, -0.2f,  0.2f);	// Bottom Right Of The Texture and Quad
		glVertex3f( 0.2f,  0.2f,  0.2f);	// Top Right Of The Texture and Quad
		glVertex3f(-0.2f,  0.2f,  0.2f);	// Top Left Of The Texture and Quad
		// Back Face
		glVertex3f(-0.2f, -0.2f, -0.2f);	// Bottom Right Of The Texture and Quad
		glVertex3f(-0.2f,  0.2f, -0.2f);	// Top Right Of The Texture and Quad
		glVertex3f( 0.2f,  0.2f, -0.2f);	// Top Left Of The Texture and Quad
		glVertex3f( 0.2f, -0.2f, -0.2f);	// Bottom Left Of The Texture and Quad
		// Top Face
		glVertex3f(-0.2f,  0.2f, -0.2f);	// Top Left Of The Texture and Quad
		glVertex3f(-0.2f,  0.2f,  0.2f);	// Bottom Left Of The Texture and Quad
		glVertex3f( 0.2f,  0.2f,  0.2f);	// Bottom Right Of The Texture and Quad
		glVertex3f( 0.2f,  0.2f, -0.2f);	// Top Right Of The Texture and Quad
		// Bottom Face
		glVertex3f(-0.2f, -0.2f, -0.2f);	// Top Right Of The Texture and Quad
		glVertex3f( 0.2f, -0.2f, -0.2f);	// Top Left Of The Texture and Quad
		glVertex3f( 0.2f, -0.2f,  0.2f);	// Bottom Left Of The Texture and Quad
		glVertex3f(-0.2f, -0.2f,  0.2f);	// Bottom Right Of The Texture and Quad
		// Right face
		glVertex3f( 0.2f, -0.2f, -0.2f);	// Bottom Right Of The Texture and Quad
		glVertex3f( 0.2f,  0.2f, -0.2f);	// Top Right Of The Texture and Quad
		glVertex3f( 0.2f,  0.2f,  0.2f);	// Top Left Of The Texture and Quad
		glVertex3f( 0.2f, -0.2f,  0.2f);	// Bottom Left Of The Texture and Quad
		// Left Face
		glVertex3f(-0.2f, -0.2f, -0.2f);	// Bottom Left Of The Texture and Quad
		glVertex3f(-0.2f, -0.2f,  0.2f);	// Bottom Right Of The Texture and Quad
		glVertex3f(-0.2f,  0.2f,  0.2f);	// Top Right Of The Texture and Quad
		glVertex3f(-0.2f,  0.2f, -0.2f);	// Top Left Of The Texture and Quad
	glEnd();

	glPopMatrix();//restore the scene

  glEnable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
}


void visualise_lights()
{
	//ok, first of all, let's see what objects we have in range...
	int i;
	int x,y;

	x=(int)-camera_x;
	y=(int)-camera_y;

	glPushMatrix();
	glLoadIdentity();					// Reset The Matrix
	Move();

	for (i = 0; i < MAX_LIGHTS; i++)
	{
		if (lights_list[i])
		{
			int dist1 = x - (int) lights_list[i]->pos_x;
			int dist2 = y - (int) lights_list[i]->pos_y;

			if (dist1*dist1+dist2*dist2 <= ((40*40)*(zoom_level/15.75f)))
				draw_light_source(lights_list[i]);
		}
	}

	glPopMatrix();
}

void get_light_under_mouse()
{
	//ok, first of all, let's see what objects we have in range...
	int i, count, x, y, dist1, dist2, hits;
	GLint viewport[4];
	GLuint *buffer, z_coordinate;
	double matrix[16];

	selected_light = -1;
	x = (int)-camera_x;
	y = (int)-camera_y;

	count = 0;
	for (i = 0; i < MAX_LIGHTS; i++)
	{
		if (lights_list[i])
		{
			dist1 = x - (int)lights_list[i]->pos_x;
			dist2 = y - (int)lights_list[i]->pos_y;
			if (dist1 * dist1 + dist2 * dist2 <= ((40 * 40) * (zoom_level / 15.75f)))
			{
				count++;
			}
		}
	}

	if (count == 0)
	{
		return;
	}
	count++;
	count *= 4;
	glGetIntegerv(GL_VIEWPORT, viewport);
	buffer = malloc(count * sizeof(GLuint));
	glSelectBuffer(count, buffer);

	glGetDoublev(GL_PROJECTION_MATRIX, matrix);

	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(mouse_x, window_height-mouse_y, 1.0, 1.0, viewport);
	glMultMatrixd(matrix);
	glMatrixMode(GL_MODELVIEW);

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glPushMatrix();
	glLoadIdentity();					// Reset The Matrix
	Move();

	for (i = 0; i < MAX_LIGHTS; i++)
	{
		if (lights_list[i])
		{
			dist1 = x - (int)lights_list[i]->pos_x;
			dist2 = y - (int)lights_list[i]->pos_y;
			if (dist1 * dist1 + dist2 * dist2 <= ((40 * 40) * (zoom_level / 15.75f)))
			{
				glLoadName(i);
				draw_light_source(lights_list[i]);
			}
		}
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	hits = glRenderMode(GL_RENDER);
	z_coordinate = 0xFFFFFFFF;
	for (i = 0; i < hits; i++)
	{
		if (buffer[(i * 4) + 1] < z_coordinate)
		{
			selected_light = buffer[(i*4)+3];
			z_coordinate = buffer[(i*4)+1];
		}
	}
	free(buffer);

}

void move_light(int object_id)
{
	lights_list[object_id]->pos_x=scene_mouse_x;
	lights_list[object_id]->pos_y=scene_mouse_y;
}

void kill_light(int object_id)
{
	free(lights_list[object_id]);
	lights_list[object_id]=0;//kill any refference to it
	selected_light=-1;//we have no selected light now...
}

void clone_light(int object_id)
{
	float z_pos,r,g,b;

	z_pos=lights_list[object_id]->pos_z;
	r=lights_list[object_id]->r;
	g=lights_list[object_id]->g;
	b=lights_list[object_id]->b;

	selected_light=add_light(scene_mouse_x,scene_mouse_y,z_pos,r,g,b,1.0f,lights_list[object_id]->locked);
	cur_tool=tool_select;//change the current tool
}

void change_color_height(unsigned char cur_height)
{
		if(cur_height==0)glColor4f(0.0f,0.0f,0.0f,0.5f);
		if(cur_height==1)glColor4f(0.1f,0.1f,0.36f,0.5f);
		if(cur_height==2)glColor4f(0.1f,0.2f,0.42f,0.5f);
		if(cur_height==3)glColor4f(0.1f,0.2f,0.49f,0.5f);
		if(cur_height==4)glColor4f(0.1f,0.2f,0.56f,0.5f);
		if(cur_height==5)glColor4f(0.1f,0.3f,0.63f,0.5f);
		if(cur_height==6)glColor4f(0.1f,0.3f,0.70f,0.5f);
		if(cur_height==7)glColor4f(0.1f,0.3f,0.76f,0.5f);
		if(cur_height==8)glColor4f(0.1f,0.3f,0.84f,0.5f);
		if(cur_height==9)glColor4f(0.1f,0.4f,0.93f,0.5f);
		if(cur_height==10)glColor4f(0.1f,0.4f,1.0f,0.5f);

		if(cur_height==11)glColor4f(0.0f,0.5f,0.0f,0.5f);
		if(cur_height==12)glColor4f(0.1f,0.55f,0.0f,0.5f);
		if(cur_height==13)glColor4f(0.15f,0.6f,0.0f,0.5f);
		if(cur_height==14)glColor4f(0.2f,0.65f,0.0f,0.5f);
		if(cur_height==15)glColor4f(0.25f,0.7f,0.0f,0.5f);
		if(cur_height==16)glColor4f(0.3f,0.75f,0.0f,0.5f);
		if(cur_height==17)glColor4f(0.35f,0.8f,0.0f,0.5f);
		if(cur_height==18)glColor4f(0.4f,0.85f,0.0f,0.5f);
		if(cur_height==19)glColor4f(0.45f,0.9f,0.0f,0.5f);
		if(cur_height==20)glColor4f(0.5f,0.95f,0.0f,0.5f);
		if(cur_height==21)glColor4f(0.6f,1.0f,0.0f,0.5f);

		if(cur_height==22)glColor4f(0.7f,0.55f,0.0f,0.5f);
		if(cur_height==23)glColor4f(0.65f,0.5f,0.0f,0.5f);
		if(cur_height==24)glColor4f(0.6f,0.45f,0.0f,0.5f);
		if(cur_height==25)glColor4f(0.55f,0.4f,0.0f,0.5f);
		if(cur_height==26)glColor4f(0.5f,0.35f,0.0f,0.5f);
		if(cur_height==27)glColor4f(0.45f,0.3f,0.0f,0.5f);
		if(cur_height==28)glColor4f(0.4f,0.25f,0.0f,0.5f);
		if(cur_height==29)glColor4f(0.35f,0.2f,0.0f,0.5f);
		if(cur_height==30)glColor4f(0.3f,0.15f,0.0f,0.5f);
		if(cur_height==31)glColor4f(0.25f,0.1f,0.0f,0.5f);
}

void move_height_tile()
{
	float x_start,y_start,z;

	x_start=scene_mouse_x-0.25f;
	y_start=scene_mouse_y-0.25f;
	//x_start=scene_mouse_x;
	//y_start=scene_mouse_y;

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	change_color_height(selected_height);

	if(!heights_3d)z=0.02f;
	else z=(float)selected_height*0.2f-11.0f*0.2f+0.02f;

	if(ctrl_on && alt_on)
	{
		glBegin(GL_QUADS);
		glVertex3f(x_start-2.5f,y_start+2.5f+0.5f, z);
		glVertex3f(x_start-2.5f,y_start-2.5f, z);
		glVertex3f(x_start+2.5f+0.5f, y_start-2.5f,z);
		glVertex3f(x_start+2.5f+0.5f, y_start+2.5f+0.5f,z);
		glEnd();
	}
	else
	if(alt_on)
	{
		glBegin(GL_QUADS);
		glVertex3f(x_start-1.5f,y_start+1.5f+0.5f, z);
		glVertex3f(x_start-1.5f,y_start-1.5f, z);
		glVertex3f(x_start+1.5f+0.5f, y_start-1.5f,z);
		glVertex3f(x_start+1.5f+0.5f, y_start+1.5f+0.5f,z);
		glEnd();
	}
	else
	if(ctrl_on)
	{
		glBegin(GL_QUADS);
		glVertex3f(x_start-0.5f,y_start+0.5f+0.5f, z);
		glVertex3f(x_start-0.5f,y_start-0.5f, z);
		glVertex3f(x_start+0.5f+0.5f, y_start-0.5f,z);
		glVertex3f(x_start+0.5f+0.5f, y_start+0.5f+0.5f,z);
		glEnd();
	}
	else
	{
		glBegin(GL_QUADS);
		glVertex3f(x_start,y_start+0.5f, z);
		glVertex3f(x_start,y_start, z);
		glVertex3f(x_start+0.5f, y_start,z);
		glVertex3f(x_start+0.5f, y_start+0.5f,z);
		glEnd();
	}

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
}

void draw_big_height_tile(int size)
{
	int x,y;
	int x1,x2;
	int y1,y2;

	x=(int)(scene_mouse_x/0.5f);
	y=(int)(scene_mouse_y/0.5f);

	x1=x-size;
	x2=x+size;
	y1=y-size;
	y2=y+size;


	if(x1<0)x1=0;
	if(y1<0)y1=0;

	if(x2>=tile_map_size_x*6-1)x2=tile_map_size_x*6-1;
	if(y2>=tile_map_size_y*6-1)y2=tile_map_size_y*6-1;

	for(y=y1;y<=y2;y++)
	for(x=x1;x<=x2;x++)
	height_map[y*tile_map_size_x*6+x]=selected_height;
}

void floodfill (unsigned char new_height, int x, int y)
{
	// Grum: in the absolute worst case scenario (checkerboard patterns on
	// the whole map) we may run into stack problems with this function,
	// but it's simple, and it's fast.

	int size = tile_map_size_x * 6;
	unsigned char old_height = height_map[y*size+x];
	int i, imin, imax;
	
	for (i = x; i >= 0 && height_map[y*size+i] == old_height; i--)
		height_map[y*size+i] = new_height;
	imin = i > 0 ? i : 0;
	for (i = x+1; i < size && height_map[y*size+i] == old_height; i++)
		height_map[y*size+i] = new_height;
	imax = i < size-1 ? i : size-1;
	
	if (y > 0)
	{
		for (i = x; i >= imin; i--)
			if (height_map[(y-1)*size+i] == old_height)
				floodfill (new_height, i, y-1);
		for (i = x+1; i <= imax; i++)
			if (height_map[(y-1)*size+i] == old_height)
				floodfill (new_height, i, y-1);
	}
	
	if (y < size-1)
	{
		for (i = x; i >= imin; i--)
			if (height_map[(y+1)*size+i] == old_height)
				floodfill (new_height, i, y+1);
		for (i = x+1; i <= imax; i++)
			if (height_map[(y+1)*size+i] == old_height)
				floodfill (new_height, i, y+1);		
	}
}

void map_floodfill()
{
	int x, y;
	int size = tile_map_size_x * 6;
	unsigned char old_height;
     
	x=(int)(scene_mouse_x/0.5f);
	y=(int)(scene_mouse_y/0.5f);
	
	old_height = height_map[y*size+x];
	if (old_height == selected_height) return;
	
	floodfill (selected_height, x, y);
}

void draw_heights_wireframe()
{
	int x,y;
	int x_start,x_end,y_start,y_end;
	float x_scaled,y_scaled;

	if(camera_x<0)x=(int)((camera_x*-1.0)/3.0*6.0);
	else x=(int)(camera_x/3.0);
	if(camera_y<0)y=(int)((camera_y*-1.0)/3.0*6.0);
	else y=(int)(camera_y/3.0);
	x_start=(int)x-4*6;
	y_start=(int)y-4*6;
	x_end=(int)x+4*6;
	y_end=(int)y+4*6;
	if(x_start<0)x_start=0;
	if(x_end>=tile_map_size_x*6)x_end=tile_map_size_x*6-1;
	if(y_start<0)y_start=0;
	if(y_end>=tile_map_size_y*6)y_end=tile_map_size_y*6-1;

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glColor3f(gcr,gcg,gcb);

	for(y=y_start;y<=y_end;y++)
		{
			y_scaled=y*0.5f;
			for(x=x_start;x<=x_end;x++)
				{

					x_scaled=x*0.5f;
					glBegin(GL_LINE_LOOP);
					glVertex3f(x_scaled,y_scaled+0.5f, grid_height);
					glVertex3f(x_scaled,y_scaled, grid_height);
					glVertex3f(x_scaled+0.5f, y_scaled, grid_height);
					glVertex3f(x_scaled+0.5f, y_scaled+0.5f, grid_height);
					glEnd();
				}
		}

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
}


void draw_height_map()
{
	int x_start,x_end,y_start,y_end;
	int x,y;
	float x_scaled,y_scaled;
	unsigned char cur_height;

	//get only the tiles around the camera
	//we have the axes inverted, btw the go from 0 to -255
	if(camera_x<0)x=(int)((camera_x*-1.0)/3.0*6.0);
	else x=(int)(camera_x/3.0);
	if(camera_y<0)y=(int)((camera_y*-1.0)/3.0*6.0);
	else y=(int)(camera_y/3.0);
	x_start=x-(int)(zoom_level*6.0);
	y_start=y-(int)(zoom_level*6.0);
	x_end=x+(int)(zoom_level*6.0);
	y_end=y+(int)(zoom_level*6.0);
	if(x_start<0)x_start=0;
	if(x_end>=tile_map_size_x*6)x_end=tile_map_size_x*6-1;
	if(y_start<0)y_start=0;
	if(y_end>=tile_map_size_y*6)y_end=tile_map_size_y*6-1;

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glBegin(GL_QUADS);
	for(y=y_start;y<=y_end;y++)
		{
			y_scaled=y*0.5f;
			for(x=x_start;x<=x_end;x++)
				{
					float z;

					x_scaled=x*0.5f;
					cur_height=height_map[y*tile_map_size_x*6+x];
					cur_height=cur_height&31;
					change_color_height(cur_height);

					if(!heights_3d)z=0.01f;
					else z=(float)cur_height*0.2f-11.0f*0.2f+0.01f;


	 				glVertex3f(x_scaled,y_scaled+0.5f, z);
					glVertex3f(x_scaled,y_scaled, z);
					glVertex3f(x_scaled+0.5f, y_scaled,z);
					glVertex3f(x_scaled+0.5f, y_scaled+0.5f,z);
				}
		}
	glEnd();

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
}

#ifndef LINUX
void open_map_file()
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH],temp[MAX_PATH];

  ZeroMemory(&ofn, sizeof(ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = 0;
  ofn.lpstrFilter = "Eternal Lands Map (*.elm*)\0*.elm*\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "elm";
  strcpy(temp,exec_path);
  strcat(temp,"\\maps\\");
  ofn.lpstrInitialDir = temp;

    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (GetOpenFileName (&ofn))
    {
		char proper_path[MAX_PATH];
		get_proper_path(szFileName, exec_path, proper_path, sizeof(proper_path));
		load_map(proper_path);
    }
}
#endif

extern particle_sys_def def;
#ifndef LINUX
void save_map_file()
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH], temp[MAX_PATH];

  ZeroMemory (&ofn, sizeof (ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof (ofn);
  ofn.hwndOwner = 0;
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
#ifdef	LIBZW
  ofn.lpstrFilter = "Compressed Map (*.elm.gz)\0*.elm.gz\0\0";
  ofn.lpstrDefExt = "elm.gz";
#else	//LIBZW
  ofn.lpstrFilter = "Eternal Lands Map (*.elm)\0*.elm\0\0";
  ofn.lpstrDefExt = "elm";
#endif	//LIBZW
  strcpy(temp,exec_path);
  strcat(temp,"\\maps\\");
  ofn.lpstrInitialDir = temp;

    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
      OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileName (&ofn))
    {
		char proper_path[128];
		get_proper_path ( szFileName, exec_path, proper_path, sizeof (proper_path) );
		save_map(proper_path);
    }
}

void save_particle_def_file()
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH], temp[MAX_PATH];

  ZeroMemory (&ofn, sizeof (ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof (ofn);
  ofn.hwndOwner = 0;
  ofn.lpstrFilter = "Particle File (*.part)\0*.part\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "part";
  strcpy(temp,exec_path);
  strcat(temp,"\\particles\\");
  ofn.lpstrInitialDir = temp;

    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
      OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileName (&ofn))
    {
    		char proper_path[128];
		get_proper_path ( szFileName, exec_path, proper_path, sizeof (proper_path) );
		strcpy(def.file_name,proper_path);
		save_particle_def(&def);
    }
}

void open_particles_obj()
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH],temp[MAX_PATH];

  ZeroMemory (&ofn, sizeof (ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof (ofn);
  ofn.hwndOwner = 0;
  ofn.lpstrFilter = "Particle File (*.part)\0*.part\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "part";
  strcpy(temp,exec_path);
  strcat(temp,"\\particles\\");
  ofn.lpstrInitialDir = temp;

    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |OFN_NOCHANGEDIR;
    if (GetOpenFileName (&ofn))
    {
		char proper_path[128];
		get_proper_path ( szFileName, exec_path, proper_path, sizeof (proper_path) );
		selected_particles_object=add_particle_sys(proper_path,scene_mouse_x,scene_mouse_y,0.0);
		cur_tool=tool_select;//change the current tool
		particles_list[selected_particles_object]->ttl=-1; // we dont want the particle sys to disapear
    }
}
#endif

#ifdef LINUX



void open_3d_obj()
{
#ifdef GTK2
	show_open_window("Open 3D object", obj_3d_folder, e3d_filter);
#else
	gtk_window_set_title(GTK_WINDOW(effect_selector), "open 3d object");
	//gtk_file_selection_complete(GTK_FILE_SELECTION(file_selector), "*.e3d");
	continue_with = OPEN_3D_OBJ;
	gtk_widget_show(effect_selector);
#endif
}
#endif
void open_3d_obj_continued()
{
  if (selected_file)
  {
		char *file = selected_file;
		size_t datadir_len = strlen(datadir);

		if(!strcmp(selected_file+strlen(selected_file)-3, ".gz")) {
			selected_file[strlen(selected_file)-3]= '\0';
		}
		if(datadir_len > 0 && strncmp(selected_file, datadir, datadir_len) == 0) {
			/* add_e3d() wants a path relative to the data dir */
			if (file[datadir_len-1] == '/')
				file += datadir_len-2;
			else
				file += datadir_len-1;
			*file = '.';
		}
		selected_3d_object=add_e3d(file,scene_mouse_x,scene_mouse_y,0,0,0,0,0,0,0,0,0);
		cur_tool=tool_select;//change the current tool
		if(selected_particles_object>=0) {
			// we dont want the particle sys to disapear
			particles_list[selected_particles_object]->ttl=-1;
		}
#ifdef GTK2
		cur_mode=mode_3d;
#endif
  }
}

#ifdef LINUX
void open_particles_obj()
{
#ifdef GTK2
	show_open_window("Open particle file", particles_folder, part_filter);
#else
	gtk_window_set_title(GTK_WINDOW(file_selector), "open particles object");
	continue_with = OPEN_PARTICLES_OBJ;
	gtk_widget_show(file_selector);
#endif //GTK2
}
#endif //LINUX
void open_particles_obj_continued()
{
  if (selected_file)
    {
		char *file = selected_file;
		size_t datadir_len = strlen(datadir);

		if(datadir_len > 0 && strncmp(selected_file, datadir, datadir_len) == 0) {
			/* add_particle_sys() wants a path relative to the data dir */
			if (file[datadir_len-1] == '/')
				file += datadir_len-2;
			else
				file += datadir_len-1;
			*file = '.';
		}
		selected_particles_object=add_particle_sys(file,scene_mouse_x,scene_mouse_y,0.0);
		cur_tool=tool_select;//change the current tool
#ifdef GTK2
		cur_mode=mode_particles;
#endif
    }
}

//#ifdef LINUX
void open_eye_candy_obj()
{
#ifdef GTK2
	show_eye_candy_window();
#else
	gtk_window_set_title(GTK_WINDOW(file_selector), "Select eye candy effect");
	continue_with = OPEN_EYE_CANDY_OBJ;
	gtk_widget_show(file_selector);
#endif
}
//#endif
void open_eye_candy_obj_continued()
{
  if (selected_file)
    {

		cur_tool=tool_select;//change the current tool
#ifdef GTK2
		cur_mode=mode_eye_candy;
#endif
    }
}

#ifdef LINUX
void open_2d_obj()
{
#ifdef GTK2
	show_open_window("Open 2D object", obj_2d_folder, e2d_filter);
#else
  gtk_window_set_title(GTK_WINDOW(file_selector), "open 2d object");
  //  gtk_file_selection_complete(GTK_FILE_SELECTION(file_selector), "*.e2d");
  continue_with = OPEN_2D_OBJ;
  gtk_widget_show(file_selector);
#endif
}
#endif
void open_2d_obj_continued()
{
	if (selected_file)
	{
		size_t datadir_len = strlen(datadir);
		char *file = selected_file;

		if(datadir_len > 0 && strncmp(selected_file, datadir, datadir_len) == 0) {
			/* add_2d_obj() expects a path relative to datadir */
			if (file[datadir_len-1] == '/')
				file += datadir_len-2;
			else
				file += datadir_len-1;
			*file = '.';
		}
		selected_2d_object=add_2d_obj(file,scene_mouse_x,scene_mouse_y,0.001f,0,0,0);
		cur_tool=tool_select;//change the current tool
#ifdef GTK2
		cur_mode=mode_2d;
#endif
	}
}
#ifdef LINUX
void open_map_file()
{
#ifdef GTK2
	show_open_window("Open map", map_folder, map_filter);
#else
  gtk_window_set_title(GTK_WINDOW(file_selector), "open map");
  //  gtk_file_selection_complete(GTK_FILE_SELECTION(file_selector), "*.elm");
  continue_with = OPEN_MAP;
  gtk_widget_show(file_selector);
#endif
}
#endif
void open_map_file_continued()
{
  if (selected_file)load_map(selected_file);
#ifdef GTK2
  cur_mode=mode_3d;
#endif
}
#ifdef LINUX
void save_map_file()
{
#ifdef GTK2
	char dir[200];

	strcpy(dir, datadir);
	strcat(dir, "/maps/");
	show_save_window("Save map", dir, map_file_name, map_filter);
#else
  gtk_window_set_title(GTK_WINDOW(file_selector), "save map");
  //  gtk_file_selection_complete(GTK_FILE_SELECTION(file_selector), "*.elm");
  continue_with = SAVE_MAP;
  gtk_widget_show(file_selector);
#endif
}
#endif
void save_map_file_continued()
{
  if (selected_file)
	  save_map(selected_file);
}
#ifdef LINUX
void save_particle_def_file()
{
#ifdef GTK2
	char dir[200];

	strcpy(dir, datadir);
	strcat(dir, "/particles/");
	show_save_window("Save particle effect", dir, particle_file_name, part_filter);
#else
	gtk_window_set_title(GTK_WINDOW(file_selector), "save particle system definition");
	continue_with = SAVE_PARTICLE_DEF;
	gtk_widget_show(file_selector);
#endif
}
#endif

void save_particle_def_file_continued()
{
	if(!selected_file)return;
	strncpy(def.file_name,selected_file,80);
	save_particle_def(&def);
}

off_t get_file_size(const char *fname)
{
	struct stat fstat;
	if (stat(fname, &fstat) < 0)
		return -1;
	return fstat.st_size;
}

//warning: when checking directories, do not include the trailing slash, for portability reasons
int file_exists(const char *fname)
{
	int statres;
	struct stat fstat;

	statres= stat(fname, &fstat);
	if(statres < 0)
	{
		statres= errno;
	}
	if(statres != ENOENT && statres != 0)
	{
		//something went wrong...
		LOG_ERROR("Error when checking file or directory %s (error code %d)\n", fname, statres);
		return -1;
	}
	else
	{
		return (statres != ENOENT);
	}
}

int gzfile_exists(const char *fname)
{
#ifdef	ZLIB
	char	gzfname[1024];

	strcpy(gzfname, fname);
	strcat(gzfname, ".gz");
	if(file_exists(gzfname)){
		return 1;
	}
#endif

	return(file_exists(fname));
}

FILE *my_fopen (const char *fname, const char *mode)
{
	FILE *file = fopen (fname, mode);
	if (file == NULL)
	{
		char str[256];
		snprintf(str, sizeof (str), "%s: %s \"%s\"", reg_error_str, cant_open_file, fname);
		LOG_ERROR(str);
	}
	return file;
}

#ifdef ZLIB
gzFile * my_gzopen(const char * filename, const char * mode)
{
	char gzfilename[1024];
	gzFile * result;

	snprintf(gzfilename, sizeof(gzfilename), "%s.gz", filename);
	result= gzopen(gzfilename, mode);
	if(result == NULL) {
		// didn't work, try the name that was specified
		result= gzopen(filename, mode);
	}
	if(result == NULL) {
		LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, cant_open_file, filename);
	}

	return result;
}
#endif // ZLIB



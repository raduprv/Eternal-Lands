#include "global.h"

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

int evaluate_colision()
{
	char pixels[16]={0};
	glReadBuffer(GL_BACK);
	glReadPixels(mouse_x, window_height-mouse_y, 1, 1, GL_LUMINANCE, GL_BYTE, &pixels);
	if(pixels[0])return 1;//there is something
	return 0;//no collision, sorry
}

void get_3d_object_under_mouse()
{
	//ok, first of all, let's see what objects we have in range...
	int i;
	int x,y;

	selected_3d_object=-1;
	x=(int)-cx;
	y=(int)-cy;


	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glPushMatrix();
	glLoadIdentity();					// Reset The Matrix
    Move();

	for(i=0;i<max_obj_3d;i++)
		{
			if(objects_list[i])
			     {
			         int dist1;
			         int dist2;

			         dist1=x-(int)objects_list[i]->x_pos;
			         dist2=y-(int)objects_list[i]->y_pos;
			         if(dist1*dist1+dist2*dist2<=((40*40)*(zoom_level/15.75f)))
                     	draw_3d_object(objects_list[i]);
                     if(evaluate_colision())
                     	{
                     		selected_3d_object=i;
                     		glClear(GL_COLOR_BUFFER_BIT);
						}
                 }
		}
	glPopMatrix();
	glDisable(GL_CULL_FACE);
	glDisableClientState(GL_VERTEX_ARRAY);
}


void kill_3d_object(int object_id)
{
	free(objects_list[object_id]);
	objects_list[object_id]=0;//kill any refference to it
	selected_3d_object=-1;//we have no selected object now...
}

void move_3d_object(int object_id)
{
	objects_list[object_id]->x_pos=scene_mouse_x;
	objects_list[object_id]->y_pos=scene_mouse_y;
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
	r=objects_list[object_id]->r;
	g=objects_list[object_id]->g;
	b=objects_list[object_id]->b;

	z_pos=(z_pos == 0.01f)?0.02f:(z_pos == 0.02f?0.01f:z_pos);

	selected_3d_object=add_e3d(objects_list[object_id]->file_name,scene_mouse_x,scene_mouse_y,z_pos,x_rot,y_rot,z_rot,self_lit,blended,r,g,b);
	cur_tool=tool_select;//change the current tool
}

#ifndef LINUX
void open_3d_obj()
{
  OPENFILENAME ofn;
  char szFileName[MAX_PATH], temp[MAX_PATH];

  ZeroMemory (&ofn, sizeof (ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof (ofn);
  ofn.hwndOwner = 0;
  ofn.lpstrFilter = "Custom e3d (*.e3d)\0*.e3d\0\0";
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
		int fn_len;
		int app_dir_len;
		int i,j;

		//get the proper path
		fn_len=strlen(szFileName);
		app_dir_len=strlen(exec_path);
		j=0;
		proper_path[0]='.';
		for(i=app_dir_len;i<fn_len;i++,j++)proper_path[j+1]=szFileName[i];
		proper_path[j+1]=0;

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
	int i;
	int x,y;

	selected_2d_object=-1;
	x=(int)-cx;
	y=(int)-cy;

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glPushMatrix();
	glLoadIdentity();					// Reset The Matrix
    Move();

	for(i=0;i<max_obj_2d;i++)
		{
			if(obj_2d_list[i])
			     {
			         int dist1;
			         int dist2;

			         dist1=x-(int)obj_2d_list[i]->x_pos;
			         dist2=y-(int)obj_2d_list[i]->y_pos;
			         if(dist1*dist1+dist2*dist2<=((40*40)*(zoom_level/15.75f)))
                     	draw_2d_object(obj_2d_list[i]);
                     if(evaluate_colision())
                     	{
                     		selected_2d_object=i;
                     		glClear(GL_COLOR_BUFFER_BIT);
						}
                 }
		}
	glPopMatrix();
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
	char self_lit,blended;
	int i,collide;
	static int up=0;

	z_pos=obj_2d_list[object_id]->z_pos;
	x_rot=obj_2d_list[object_id]->x_rot;
	z_rot=obj_2d_list[object_id]->z_rot;

	if(ctrl_on)
		{
			collide=0;
			for(i=0;i<max_obj_2d;i++)
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
		int fn_len;
		int app_dir_len;
		int i,j;

		//get the proper path
		fn_len=strlen(szFileName);
		app_dir_len=strlen(exec_path);
		j=0;
		proper_path[0]='.';
		for(i=app_dir_len;i<fn_len;i++,j++)proper_path[j+1]=szFileName[i];
		proper_path[j+1]=0;

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
	lock_particles_list();
	for(i=0;i<max_particle_systems;i++)
		if(particles_list[i])
			draw_particle_handle(particles_list[i]);
	unlock_particles_list();
	glEnable(GL_TEXTURE_2D);
}

void get_particles_object_under_mouse() {
	//ok, first of all, let's see what objects we have in range...
	int i;
	int x,y;

	selected_particles_object=-1;
	x=(int)-cx;
	y=(int)-cy;

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glLoadIdentity();					// Reset The Matrix
	Move();

	lock_particles_list();
	for(i=0;i<max_particle_systems;i++)
		{
			if(particles_list[i])
			     {
			         int dist1;
			         int dist2;

			         dist1=x-(int)particles_list[i]->x_pos;
			         dist2=y-(int)particles_list[i]->y_pos;
			         if(dist1*dist1+dist2*dist2<=((40*40)*(zoom_level/15.75f)))
					draw_particle_handle(particles_list[i]);
				 if(evaluate_colision())
					{
						selected_particles_object=i;
						glClear(GL_COLOR_BUFFER_BIT);
					}
			     }
		}
	glPopMatrix();
	unlock_particles_list();
	glEnable(GL_TEXTURE_2D);
}

void kill_particles_object(int object_id) {
	lock_particles_list();
	free(particles_list[object_id]);
	particles_list[object_id]=0;//kill any reference to it
	selected_particles_object=-1;//we have no selected object now...
	unlock_particles_list();
}

void move_particles_object(int object_id) {
	lock_particles_list();
	if(!particles_list[object_id])
		{
			if(object_id==selected_particles_object)selected_particles_object=-1;
			unlock_particles_list();
			return;
		}
	particles_list[object_id]->x_pos=scene_mouse_x;
	particles_list[object_id]->y_pos=scene_mouse_y;
	unlock_particles_list();
}

void clone_particles_object(int object_id) {
	lock_particles_list();
	if(!particles_list[object_id])return;
	selected_particles_object=create_particle_sys(particles_list[object_id]->def,scene_mouse_x,scene_mouse_y,particles_list[object_id]->z_pos);
	unlock_particles_list();
}

////////////////////////////////////////////////////////////////////////////
////////////tile things/////////////////////////////////////////////////////
void load_all_tiles()
{
	int i;
	int cur_text;
	char str[80];
	for(i=1;i<255;i++)
	{
		sprintf(str,"./tiles/tile%i.bmp",i);
		cur_text=load_texture_cache(str,255);
		if(cur_text==-1)return;
		tile_list[i]=cur_text;
		tiles_no=i;
	}
}


void get_tile_under_mouse_from_list()
{
	int tile_id;
	int mx,my;

	mx=mouse_x-(int)x_tile_menu_offset;
	my=mouse_y-(int)y_tile_menu_offset;

	if(mx>64*8 || my>64*8 || mx<0 || my<0)return;//check to see if we clicked outside our rectangle

	mx/=64;
	my/=64;
	tile_id=my*8+mx;
	tile_id+=tile_offset;
	if(tile_id>tiles_no)return;//check to see if we clicked on an empty tile

	view_tiles_list=0;
	cur_tool=tool_select;
	selected_tile=tile_id;
}

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
		glBindTexture(GL_TEXTURE_2D, get_texture_id(tile_list[selected_tile]));

		glBegin(GL_QUADS);

 		glTexCoord2f(0, 1.0f);
		glVertex3f(x_start,y_start+3, 0.001f);
		glTexCoord2f(0, 0);
		glVertex3f(x_start,y_start, 0.001f);
		glTexCoord2f(1.0f, 0);
		glVertex3f(x_start+3, y_start,0.001f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(x_start+3, y_start+3,0.001f);
		glEnd();
	}

}

void draw_light_source(light * object_id)
{
	float x,y,z,u,v;
	float x_pos,y_pos,z_pos;

	int i,k;

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

	x=(int)-cx;
	y=(int)-cy;

	glPushMatrix();
	glLoadIdentity();					// Reset The Matrix
    Move();

	for(i=0;i<max_lights;i++)
		{
			if(lights_list[i])
			     {
			         int dist1;
			         int dist2;

			         dist1=x-(int)lights_list[i]->pos_x;
			         dist2=y-(int)lights_list[i]->pos_y;
			         if(dist1*dist1+dist2*dist2<=((40*40)*(zoom_level/15.75f)))
                     	draw_light_source(lights_list[i]);

                 }
		}
	glPopMatrix();
}

void get_light_under_mouse()
{
	//ok, first of all, let's see what objects we have in range...
	int i;
	int x,y;

	selected_light=-1;
	x=(int)-cx;
	y=(int)-cy;

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glPushMatrix();
	glLoadIdentity();					// Reset The Matrix
    Move();

	for(i=0;i<max_lights;i++)
		{
			if(lights_list[i])
			     {
			         int dist1;
			         int dist2;

			         dist1=x-(int)lights_list[i]->pos_x;
			         dist2=y-(int)lights_list[i]->pos_y;
			         if(dist1*dist1+dist2*dist2<=((40*40)*(zoom_level/15.75f)))
                     	draw_light_source(lights_list[i]);
                     if(evaluate_colision())
                     	{
                     		selected_light=i;
                     		glClear(GL_COLOR_BUFFER_BIT);
						}
                 }
		}
	glPopMatrix();
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

	selected_light=add_light(scene_mouse_x,scene_mouse_y,z_pos,r,g,b,1.0f);
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


void get_height_under_mouse_from_list()
{
	int height_id;
	int mx,my;

	mx=mouse_x-(int)x_tile_menu_offset;
	my=mouse_y-(int)y_tile_menu_offset;

	if(mx>32*8 || my>32*4 || mx<0 || my<0)return;//check to see if we clicked outside our rectangle

	mx/=32;
	my/=32;
	height_id=my*8+mx;
	view_heights_list=0;
	cur_tool=tool_select;
	selected_height=height_id;
}

void draw_big_height_tile(int size)
{
	int x,y;
	int x1,x2;
	int y1,y2;

	x=(int)(scene_mouse_x/0.5f);
	y=(int)(scene_mouse_y/0.5f);
	if(size==2){
	  x1=x-5;
	  x2=x+5;
	  y1=y-5;
	  y2=y+5;
	}
	else if(size==1)
		{
			x1=x-3;
			x2=x+3;
			y1=y-3;
			y2=y+3;
		}
	else
		{
			x1=x-1;
			x2=x+1;
			y1=y-1;
			y2=y+1;
		}

	if(x1<0)x1=0;
	if(y1<0)y1=0;

	if(x2>=tile_map_size_x*6-1)x2=tile_map_size_x*6-1;
	if(y2>=tile_map_size_y*6-1)y2=tile_map_size_y*6-1;

	for(y=y1;y<=y2;y++)
	for(x=x1;x<=x2;x++)
	height_map[y*tile_map_size_x*6+x]=selected_height;
}

void draw_heights_wireframe()
{
	int x,y;
	int x_start,x_end,y_start,y_end;
	float x_scaled,y_scaled;

	if(cx<0)x=(int)((cx*-1.0)/3.0*6.0);
	else x=(int)(cx/3.0);
	if(cy<0)y=(int)((cy*-1.0)/3.0*6.0);
	else y=(int)(cy/3.0);
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
					glVertex3f(x_scaled,y_scaled+0.5f, 0.02f);
					glVertex3f(x_scaled,y_scaled, 0.02f);
					glVertex3f(x_scaled+0.5f, y_scaled,0.02f);
					glVertex3f(x_scaled+0.5f, y_scaled+0.5f,0.02f);
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
	if(cx<0)x=(int)((cx*-1.0)/3.0*6.0);
	else x=(int)(cx/3.0);
	if(cy<0)y=(int)((cy*-1.0)/3.0*6.0);
	else y=(int)(cy/3.0);
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

  ZeroMemory (&ofn, sizeof (ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof (ofn);
  ofn.hwndOwner = 0;
  ofn.lpstrFilter = "Eternal Lands Map (*.elm)\0*.elm\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "elm";
  strcpy(temp,exec_path);
  strcat(temp,"\\maps\\");
  ofn.lpstrInitialDir = temp;

    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (GetOpenFileName (&ofn))
    {
		char proper_path[128];
		int fn_len;
		int app_dir_len;
		int i,j;

		//get the proper path
		fn_len=strlen(szFileName);
		app_dir_len=strlen(exec_path);
		j=0;
		proper_path[0]='.';
		for(i=app_dir_len;i<fn_len;i++,j++)proper_path[j+1]=szFileName[i];
		proper_path[j+1]=0;

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
  ofn.lpstrFilter = "Eternal Lands Map (*.elm)\0*.elm\0\0";
  ofn.lpstrFile = szFileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrDefExt = "elm";
  strcpy(temp,exec_path);
  strcat(temp,"\\maps\\");
  ofn.lpstrInitialDir = temp;

    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY |
      OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileName (&ofn))
    {
		char proper_path[128];
		int fn_len;
		int app_dir_len;
		int i,j;

		//get the proper path
		fn_len=strlen(szFileName);
		app_dir_len=strlen(exec_path);
		j=0;
		proper_path[0]='.';
		for(i=app_dir_len;i<fn_len;i++,j++)proper_path[j+1]=szFileName[i];
		proper_path[j+1]=0;

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
		strcpy(def.file_name,szFileName);
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
		int fn_len;
		int app_dir_len;
		int i,j;

		//get the proper path
		fn_len=strlen(szFileName);
		app_dir_len=strlen(exec_path);
		j=0;
		proper_path[0]='.';
		for(i=app_dir_len;i<fn_len;i++,j++)proper_path[j+1]=szFileName[i];
		proper_path[j+1]=0;

		selected_particles_object=add_particle_sys(proper_path,scene_mouse_x,scene_mouse_y,0.0);
		cur_tool=tool_select;//change the current tool
		particles_list[selected_particles_object]->ttl=-1; // we dont want the particle sys to disapear
    }
}
#endif

#ifdef LINUX



void open_3d_obj()
{
  gtk_window_set_title(GTK_WINDOW(file_selector), "open 3d object");
  //  gtk_file_selection_complete(GTK_FILE_SELECTION(file_selector), "*.e3d");
  continue_with = OPEN_3D_OBJ;
  gtk_widget_show(file_selector);
}
void open_3d_obj_continued()
{
  if (selected_file)
    {

		selected_3d_object=add_e3d(selected_file,scene_mouse_x,scene_mouse_y,0,0,0,0,0,0,0,0,0);
		cur_tool=tool_select;//change the current tool
		particles_list[selected_particles_object]->ttl=-1; // we dont want the particle sys to disapear
    }
}

void open_particles_obj()
{
  gtk_window_set_title(GTK_WINDOW(file_selector), "open particles object");
  continue_with = OPEN_PARTICLES_OBJ;
  gtk_widget_show(file_selector);
}

void open_particles_obj_continued()
{
  if (selected_file)
    {

		selected_particles_object=add_particle_sys(selected_file,scene_mouse_x,scene_mouse_y,0.0);
		cur_tool=tool_select;//change the current tool
    }
}


void open_2d_obj()
{
  gtk_window_set_title(GTK_WINDOW(file_selector), "open 2d object");
  //  gtk_file_selection_complete(GTK_FILE_SELECTION(file_selector), "*.e2d");
  continue_with = OPEN_2D_OBJ;
  gtk_widget_show(file_selector);
}
void open_2d_obj_continued()
{
  if (selected_file)
    {
		selected_2d_object=add_2d_obj(selected_file,scene_mouse_x,scene_mouse_y,0.001f,0,0,0);
		cur_tool=tool_select;//change the current tool
    }
}

void open_map_file()
{
  gtk_window_set_title(GTK_WINDOW(file_selector), "open map");
  //  gtk_file_selection_complete(GTK_FILE_SELECTION(file_selector), "*.elm");
  continue_with = OPEN_MAP;
  gtk_widget_show(file_selector);
}

void open_map_file_continued()
{
  if (selected_file)load_map(selected_file);
}

void save_map_file()
{
  gtk_window_set_title(GTK_WINDOW(file_selector), "save map");
  //  gtk_file_selection_complete(GTK_FILE_SELECTION(file_selector), "*.elm");
  continue_with = SAVE_MAP;
  gtk_widget_show(file_selector);
}

void save_map_file_continued()
{
  if (selected_file)save_map(selected_file);
}

void save_particle_def_file()
{
	gtk_window_set_title(GTK_WINDOW(file_selector), "save particle system definition");
	continue_with = SAVE_PARTICLE_DEF;
	gtk_widget_show(file_selector);
}


void save_particle_def_file_continued()
{
	if(!selected_file)return;
	strncpy(def.file_name,selected_file,80);
	save_particle_def(&def);
}

#endif

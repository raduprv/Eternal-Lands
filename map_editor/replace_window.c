#include "replace_window.h"
#include "2d_objects.h"
#include "e3d.h"
#include "elwindows.h"
#include "font.h"
#include "interface.h"
#include "tiles.h"
#include "../particles.h"
#include "../platform.h"

int view_replace_window=0;
int replace_window_x=20;
int replace_window_y=100;
int replace_window_x_len=360;
int replace_window_y_len=90;
int replace_window_win=0;

#ifdef X86_64
typedef long int INT;
#else
typedef int INT;
#endif

INT oid=-1,nid=-1, mode=1;
char cOid[100],cNid[100];
float new_object_red = 0.0f, new_object_green = 0.0f, new_object_blue = 0.0f;



//buttons

int d1_x1=245,d1_x2=320,d1_y1=10,d1_y2=25;

int d2_x1=245,d2_x2=320,d2_y1=30,d2_y2=45;

int d3_x1=5,d3_x2=100,d3_y1=50,d3_y2=70;

int d4_x1=245,d4_x2=320,d4_y1=50,d4_y2=70;



void display_replace_window()

{

	if(replace_window_win <= 0){

		replace_window_win= create_window("replace_window", 0, 0, replace_window_x, replace_window_y, replace_window_x_len, replace_window_y_len, ELW_WIN_DEFAULT);



		set_window_handler(replace_window_win, ELW_HANDLER_DISPLAY, &display_replace_window_handler );

		set_window_handler(replace_window_win, ELW_HANDLER_CLICK, &check_replace_window_interface );

		

	} else {

		show_window(replace_window_win);

		select_window(replace_window_win);

	}

	display_window(replace_window_win);

}



int display_replace_window_handler(window_info *win)

{

	char temp[100];



	glEnable(GL_BLEND);

	glBlendFunc(GL_ONE,GL_SRC_ALPHA);

	glDisable(GL_TEXTURE_2D);

	glDisable(GL_BLEND);

	glColor3f(0.77f,0.57f,0.39f);

	



	glBegin(GL_LINE_LOOP);

	glVertex3i(d1_x1,d1_y1,0);

	glVertex3i(d1_x2,d1_y1,0);

	glVertex3i(d1_x2,d1_y2,0);

	glVertex3i(d1_x1,d1_y2,0);

	glEnd();

	glBegin(GL_LINE_LOOP);

	glVertex3i(d2_x1,d2_y1,0);

	glVertex3i(d2_x2,d2_y1,0);

	glVertex3i(d2_x2,d2_y2,0);

	glVertex3i(d2_x1,d2_y2,0);

	glEnd();

	glBegin(GL_LINE_LOOP);

	glVertex3i(d3_x1,d3_y1,0);

	glVertex3i(d3_x2,d3_y1,0);

	glVertex3i(d3_x2,d3_y2,0);

	glVertex3i(d3_x1,d3_y2,0);

	glEnd();

	glBegin(GL_LINE_LOOP);

	glVertex3i(d4_x1,d4_y1,0);

	glVertex3i(d4_x2,d4_y1,0);

	glVertex3i(d4_x2,d4_y2,0);

	glVertex3i(d4_x1,d4_y2,0);

	glEnd();



	glEnable(GL_TEXTURE_2D);

   

	// The X

	draw_string(0+win->len_x-16,0+2,(const unsigned char *) "X",1);

	sprintf(temp,"Original: %d",(int)oid);

	draw_string(10,10,(const unsigned char *)temp,1);

	draw_string(250,10,(const unsigned char *)"Select",1);

	sprintf(temp,"New: %d",(int)nid);

	draw_string(10,30,(const unsigned char *)temp,1);

	draw_string(250,30,(const unsigned char *)"Select",1);

	draw_string(10,50,(const unsigned char *)"Replace!",1);

	

	sprintf(temp,"Mode: %d",(int)mode);

	draw_string(130,50,(const unsigned char *)temp,1);

	draw_string(250,50,(const unsigned char *)"Change",1);



	return 1;

}





int check_replace_window_interface(window_info *win, int _x, int _y)

{

	// Grum: this shouldn't happen

	if (view_replace_window && _x > win->len_x-20 && _y <= 20)

	{

		view_replace_window=0;

		return 1;

	}

	if (_x > d1_x1 && _x <= d1_x2 && _y > d1_y1 && _y <= d1_y2)

	{

		if (selected_particles_object != -1 && mode == 4)

		{

			oid = (INT) particles_list[selected_particles_object]->def;

		}

		else if (selected_3d_object != -1 && mode == 3)

		{

			oid = (INT) objects_list[selected_3d_object]->e3d_data;

			strcpy (cOid, objects_list[selected_3d_object]->file_name);

		}

		else if (selected_2d_object != -1 && mode == 2)

		{

			oid = (INT) obj_2d_list[selected_2d_object]->obj_pointer;

			strcpy (cOid, obj_2d_list[selected_2d_object]->file_name);

		}

		else if (selected_tile != 255 && mode == 1)

		{

			oid=selected_tile;

		}

	}

	

	if (_x > d2_x1 && _x <= d2_x2 && _y > d2_y1 && _y <= d2_y2)

	{

		if (selected_particles_object != -1 && mode == 4)

		{

			nid = (INT) particles_list[selected_particles_object]->def;

		}

		else if (selected_3d_object != -1 && mode == 3)

		{

			nid = (INT)objects_list[selected_3d_object]->e3d_data;

			strcpy (cNid, objects_list[selected_3d_object]->file_name);

			new_object_red = objects_list[selected_3d_object]->color[0];

			new_object_green = objects_list[selected_3d_object]->color[1];

			new_object_blue = objects_list[selected_3d_object]->color[2];

		}

		else if (selected_2d_object != -1 && mode == 2)

		{

			nid = (INT) obj_2d_list[selected_2d_object]->obj_pointer;

			strcpy (cNid, obj_2d_list[selected_2d_object]->file_name);

		}

		else if (selected_tile != 255 && mode == 1)

		{

			nid=selected_tile;

		}

	}



	if (_x > d3_x1 && _x <= d3_x2 && _y > d3_y1 && _y <= d3_y2 && oid != -1 && nid != -1)

	{

		int i=0;

		if (mode == 4 && nid != -1 && oid != -1)

		{

			LOCK_PARTICLES_LIST();

			for (; i < MAX_PARTICLE_SYSTEMS; i++)

			{

				if (particles_list[i] != NULL)

				{

					if ((INT)particles_list[i]->def == oid)

					{

						int j;

						particle_sys_def *def = (particle_sys_def*)nid;

						particles_list[i]->def = def;

						particles_list[i]->particle_count = def->total_particle_no;

						particles_list[i]->ttl = def->ttl;

						memset (particles_list[i]->particles, 0, MAX_PARTICLES);

						for (j = 0; j < def->total_particle_no; j++)

							create_particle (particles_list[i], &(particles_list[i]->particles[j]));

					}

				}

			}

			UNLOCK_PARTICLES_LIST ();

		}

		else if (mode == 3 && nid != -1 && oid != -1)

		{

			for ( ; i < MAX_OBJ_3D; i++)

			{

				if (objects_list[i] != NULL)

				{

					if ((INT)objects_list[i]->e3d_data == oid)

					{

						objects_list[i]->e3d_data = (e3d_object *)nid;

						strcpy (objects_list[i]->file_name, cNid);

						objects_list[i]->color[0] = new_object_red;

						objects_list[i]->color[1] = new_object_green;

						objects_list[i]->color[2] = new_object_blue;

					}

				}

			}

		}

		else if (mode == 2 && nid != -1 && oid != -1)

		{

			for ( ; i < MAX_OBJ_2D; i++)

			{

				if (obj_2d_list[i] != NULL)

				{

					if ((INT)obj_2d_list[i]->obj_pointer == oid)

					{

						obj_2d_list[i]->obj_pointer = (obj_2d_def *)nid;

						strcpy (obj_2d_list[i]->file_name, cNid);

					}

				}

			}

		}

		else if (mode == 1 && nid != -1 && oid != -1)

		{

			for (i = 0; i < tile_map_size_x*tile_map_size_y; i++)

			{

				if (tile_map[i] == oid)

					tile_map[i] = nid;

			}

		}

	}



	if (_x > d4_x1 && _x <= d4_x2 && _y > d4_y1 && _y <= d4_y2)

	{

		mode++;

		nid = -1;

		oid = -1;

		if (mode > 4) mode = 1;

	}

	  

	return 1;

}


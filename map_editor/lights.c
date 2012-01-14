#include "global.h"
#include <math.h>

void draw_test_light()
{
   GLfloat light_position[] = { 15.0, 15.0, 3.0, 1.0 };
   GLfloat light_position_2[] = { 5.0, 5.0, -3.0, 1.0 };
   GLfloat spot_direction[] = { 0.0, 0.0, -1.0f };

   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);

   glLightfv(GL_LIGHT1, GL_POSITION, light_position_2);
   glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
}

void disable_local_lights()
{
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4);
    glDisable(GL_LIGHT5);
    glDisable(GL_LIGHT6);
}

void enable_local_lights()
{
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);
    glEnable(GL_LIGHT4);
    glEnable(GL_LIGHT5);
    glEnable(GL_LIGHT6);
}


void draw_lights()
{
	GLfloat spot_direction[] = { -0.0, -0.0, -0.0f };

   glLightfv(GL_LIGHT0, GL_POSITION, light_0_position);
   glLightfv(GL_LIGHT0,GL_DIFFUSE,light_0_diffuse);
   glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);

   glLightfv(GL_LIGHT1, GL_POSITION, light_1_position);
   glLightfv(GL_LIGHT1,GL_DIFFUSE,light_1_diffuse);
   glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);

   glLightfv(GL_LIGHT2, GL_POSITION, light_2_position);
   glLightfv(GL_LIGHT2,GL_DIFFUSE,light_2_diffuse);
   glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_direction);

   glLightfv(GL_LIGHT3, GL_POSITION, light_3_position);
   glLightfv(GL_LIGHT3,GL_DIFFUSE,light_3_diffuse);
   glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, spot_direction);

   glLightfv(GL_LIGHT4, GL_POSITION, light_4_position);
   glLightfv(GL_LIGHT4,GL_DIFFUSE,light_4_diffuse);
   glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, spot_direction);

   glLightfv(GL_LIGHT5, GL_POSITION, light_5_position);
   glLightfv(GL_LIGHT5,GL_DIFFUSE,light_5_diffuse);
   glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, spot_direction);

   glLightfv(GL_LIGHT6, GL_POSITION, light_6_position);
   glLightfv(GL_LIGHT6,GL_DIFFUSE,light_6_diffuse);
   glLightfv(GL_LIGHT6, GL_SPOT_DIRECTION, spot_direction);

}

int add_light(GLfloat x, GLfloat y, GLfloat z, GLfloat r, GLfloat g, GLfloat b, GLfloat intensity, int locked)
{
	int i;
	light *new_light;

	new_light=(light *)calloc(1, sizeof(light));

	new_light->pos_x=x;
	new_light->pos_y=y;
	new_light->pos_z=z;

	new_light->r=r*intensity;
	new_light->g=g*intensity;
	new_light->b=b*intensity;

	new_light->locked=locked;

	//find a free spot, in the lights list
	for (i = 0; i < MAX_LIGHTS; i++)
	{
		if (!lights_list[i])
		{
			lights_list[i] = new_light;
			break;
		}
	}

	return i;
}

//get the lights visible in the scene
//should be called only when we change the camera pos
void update_scene_lights()
{
	int i;
	float x,y;
	float x_dist,y_dist,dist;
	char all_full=0;
	char max_changed=0;
	float max_dist=0;
	int max_light=0;

	x=-camera_x;
	y=-camera_y;
	//reset the lights
	light_0_dist=255.0f;light_1_dist=255.0f;light_2_dist=255.0f;light_3_dist=255.0f;
	light_4_dist=255.0f;light_5_dist=255.0f;light_6_dist=255.0f;

	light_0_diffuse[0]=0;light_0_diffuse[1]=0;light_0_diffuse[2]=0;light_0_diffuse[3]=1.0;

	light_1_diffuse[0]=0;light_1_diffuse[1]=0;light_1_diffuse[2]=0;light_1_diffuse[3]=1.0;

	light_2_diffuse[0]=0;light_2_diffuse[1]=0;light_2_diffuse[2]=0;light_2_diffuse[3]=1.0;

	light_3_diffuse[0]=0;light_3_diffuse[1]=0;light_3_diffuse[2]=0;light_3_diffuse[3]=1.0;

	light_4_diffuse[0]=0;light_4_diffuse[1]=0;light_4_diffuse[2]=0;light_4_diffuse[3]=1.0;

	light_5_diffuse[0]=0;light_5_diffuse[1]=0;light_5_diffuse[2]=0;light_5_diffuse[3]=1.0;

	light_6_diffuse[0]=0;light_6_diffuse[1]=0;light_6_diffuse[2]=0;light_6_diffuse[3]=1.0;

	for (i = 0; i < MAX_LIGHTS; i++)
		{
			if(lights_list[i])
				{

					if(max_changed)
						{
							max_dist=0;
							if(light_0_dist>max_dist)
								{
									max_dist=light_0_dist;
									max_light=0;
								}
							if(light_1_dist>max_dist)
								{
									max_dist=light_1_dist;
									max_light=1;
								}
							if(light_2_dist>max_dist)
								{
									max_dist=light_2_dist;
									max_light=2;
								}
							if(light_3_dist>max_dist)
								{
									max_dist=light_3_dist;
									max_light=3;
								}
							if(light_4_dist>max_dist)
								{
									max_dist=light_4_dist;
									max_light=4;
								}
							if(light_5_dist>max_dist)
								{
									max_dist=light_5_dist;
									max_light=5;
								}
							if(light_6_dist>max_dist)
								{
									max_dist=light_6_dist;
									max_light=6;
								}
							max_changed=0;
						}
					x_dist=x-lights_list[i]->pos_x;
					y_dist=y-lights_list[i]->pos_y;
					dist=x_dist*x_dist+y_dist*y_dist;
					if(dist<30*30)
						{
							if((light_0_dist==255.0f) || (all_full && (max_light==0)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;

									light_0_position[0]=lights_list[i]->pos_x;
									light_0_position[1]=lights_list[i]->pos_y;
									light_0_position[2]=lights_list[i]->pos_z;
									light_0_position[3]=1.0;

									light_0_diffuse[0]=lights_list[i]->r;
									light_0_diffuse[1]=lights_list[i]->g;
									light_0_diffuse[2]=lights_list[i]->b;
									light_0_diffuse[3]=1.0;
									light_0_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=0;
										}
									continue;
								}
							if((light_1_dist==255.0f) || (all_full && (max_light==1)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;

									light_1_position[0]=lights_list[i]->pos_x;
									light_1_position[1]=lights_list[i]->pos_y;
									light_1_position[2]=lights_list[i]->pos_z;
									light_1_position[3]=1.0;

									light_1_diffuse[0]=lights_list[i]->r;
									light_1_diffuse[1]=lights_list[i]->g;
									light_1_diffuse[2]=lights_list[i]->b;
									light_1_diffuse[3]=1.0;
									light_1_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=1;
										}
									continue;
								}
							if((light_2_dist==255.0f) || (all_full && (max_light==2)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;

									light_2_position[0]=lights_list[i]->pos_x;
									light_2_position[1]=lights_list[i]->pos_y;
									light_2_position[2]=lights_list[i]->pos_z;
									light_2_position[3]=1.0;

									light_2_diffuse[0]=lights_list[i]->r;
									light_2_diffuse[1]=lights_list[i]->g;
									light_2_diffuse[2]=lights_list[i]->b;
									light_2_diffuse[3]=1.0;
									light_2_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=2;
										}
									continue;
								}
							if((light_3_dist==255.0f) || (all_full && (max_light==3)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;

									light_3_position[0]=lights_list[i]->pos_x;
									light_3_position[1]=lights_list[i]->pos_y;
									light_3_position[2]=lights_list[i]->pos_z;
									light_3_position[3]=1.0;

									light_3_diffuse[0]=lights_list[i]->r;
									light_3_diffuse[1]=lights_list[i]->g;
									light_3_diffuse[2]=lights_list[i]->b;
									light_3_diffuse[3]=1.0;
									light_3_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=3;
										}
									continue;
								}
							if((light_4_dist==255.0f) || (all_full && (max_light==4)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;

									light_4_position[0]=lights_list[i]->pos_x;
									light_4_position[1]=lights_list[i]->pos_y;
									light_4_position[2]=lights_list[i]->pos_z;
									light_4_position[3]=1.0;

									light_4_diffuse[0]=lights_list[i]->r;
									light_4_diffuse[1]=lights_list[i]->g;
									light_4_diffuse[2]=lights_list[i]->b;
									light_4_diffuse[3]=1.0;
									light_4_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=4;
										}
									continue;
								}
							if((light_5_dist==255.0f) || (all_full && (max_light==5)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;

									light_5_position[0]=lights_list[i]->pos_x;
									light_5_position[1]=lights_list[i]->pos_y;
									light_5_position[2]=lights_list[i]->pos_z;
									light_5_position[3]=1.0;

									light_5_diffuse[0]=lights_list[i]->r;
									light_5_diffuse[1]=lights_list[i]->g;
									light_5_diffuse[2]=lights_list[i]->b;
									light_5_diffuse[3]=1.0;
									light_5_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=5;
										}
									continue;
								}
							if((light_6_dist==255.0f) || (all_full && (max_light==6)))
								{
									//see if we should recompute the max distance
									if(all_full)max_changed=1;

									light_6_position[0]=lights_list[i]->pos_x;
									light_6_position[1]=lights_list[i]->pos_y;
									light_6_position[2]=lights_list[i]->pos_z;
									light_6_position[3]=1.0;

									light_6_diffuse[0]=lights_list[i]->r;
									light_6_diffuse[1]=lights_list[i]->g;
									light_6_diffuse[2]=lights_list[i]->b;
									light_6_diffuse[3]=1.0;
									light_6_dist=dist;
									if(dist>max_dist)
										{
											max_dist=dist;
											max_light=6;
										}
									all_full=1;
									continue;
								}

						}
				}
		}

}

void init_lights()
{

   GLfloat light_diffuse[] = { 1.7, 1.3, 1.1, 0.0 };
   GLfloat no_light[] = { 0.0, 0.0, 0.0, 0.0 };
   float linear_att=1.41f;
   float cut_off=180;
   //most of the things in here are redundant, since we kind of set the light sources
   //to their default values. However, better safe than sorry.


	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT0,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT0,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT0);

	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT1,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT1,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT1,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT1);

	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT2,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT2,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT2,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT2,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT2);

	glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT3,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT3,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT3,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT3,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT3);

	glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT4,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT4,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT4,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT4,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT4);

	glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT5,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT5,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT5,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT5,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT5);

	glLightf(GL_LIGHT6, GL_SPOT_CUTOFF, cut_off);
	glLightfv(GL_LIGHT6,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT6,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT6,GL_AMBIENT,no_light);
	glLightf(GL_LIGHT6,GL_LINEAR_ATTENUATION,linear_att);
    glEnable(GL_LIGHT6);

	glLightfv(GL_LIGHT7,GL_AMBIENT,no_light);
	glLightfv(GL_LIGHT7,GL_SPECULAR,no_light);
	glLightfv(GL_LIGHT7,GL_DIFFUSE,no_light);
	glLightf(GL_LIGHT7,GL_CONSTANT_ATTENUATION,0);
	glEnable(GL_LIGHT7);


     glEnable(GL_LIGHTING);

     glNormal3f(0.0f,0.0f,1.0f);
}


void reset_material()
{
	GLfloat mat_emission[]={ 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_specular[]={ 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_ambient[]={ 1.0, 1.0, 1.0, 1.0 };

	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ambient);

}

void set_material(float r, float g, float b)
{
	GLfloat mat_emission[]={ r, g, b, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_emission);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_emission);
}



void draw_global_light()
{
     int i;
     GLfloat global_light_position[] = { 400.0, 400.0, 500.0, 0.0 };
     GLfloat ambient_light[] = { 0.0, 0.0, 0.0, 0.0 };
     i=light_level;
     if(light_level>59)i=119-light_level;
     //the ambient light should be half of the difuse light
     ambient_light[0]=global_lights[i][0]/1.5f;
     ambient_light[1]=global_lights[i][1]/1.5f;
     ambient_light[2]=global_lights[i][2]/1.5f;
     ambient_light[3]=1.0f;
	 glLightfv(GL_LIGHT7,GL_AMBIENT,ambient_light);
     glLightfv(GL_LIGHT7, GL_POSITION, global_light_position);
	 glLightfv(GL_LIGHT7,GL_DIFFUSE,&global_lights[i][0]);


}

void draw_dungeon_light()
{
     GLfloat global_light_position[] = { 400.0, 400.0, 500.0, 0.0 };
     GLfloat difuse_light[] = { 0.0, 0.0, 0.0, 0.0 };
     GLfloat ambient_light[4];
     //the ambient light should be half of the difuse light
     ambient_light[0]=ambient_r;
     ambient_light[1]=ambient_g;
     ambient_light[2]=ambient_b;
     ambient_light[3]=1.0f;
	 glLightfv(GL_LIGHT7,GL_AMBIENT,ambient_light);
     glLightfv(GL_LIGHT7, GL_POSITION, global_light_position);
	 glLightfv(GL_LIGHT7,GL_DIFFUSE,difuse_light);

}

void make_gradient_light(int start,int steps,float *light_table, float r_start, float g_start, float b_start, float r_end, float g_end, float b_end)
{
	int i,j;
	float r_slope,g_slope,b_slope;

	r_slope=(r_end-r_start)/steps;
	g_slope=(g_end-g_start)/steps;
	b_slope=(b_end-b_start)/steps;

	j=0;
	for(i=start;i<start+steps;i++)
		{
			light_table[i*4+0]=r_start+r_slope*(float)j;
			light_table[i*4+1]=g_start+g_slope*(float)j;
			light_table[i*4+2]=b_start+b_slope*(float)j;
			light_table[i*4+3]=1.0f;
			j++;

		}
}

//build the light table for smooth transition between night and day
void build_global_light_table()
{
	//the sun light
	make_gradient_light(0,30,(float *)global_lights,0.85f,0.85f,0.85f,0.32f,0.25f,0.25f);
	make_gradient_light(30,30,(float *)global_lights,0.318f,0.248f,0.248f,0.05f,0.05f,0.08f);
	//lake light
	make_gradient_light(0,30,(float *)sky_lights_c1,0.0f,0.3f,0.6f,0.6f,0.3f,0.0f);
	make_gradient_light(30,30,(float *)sky_lights_c1,0.6f,0.3f,0.0f,0.0f,0.01f,0.1f);
	make_gradient_light(60,30,(float *)sky_lights_c1,0.0f,0.1f,0.1f,0.6f,0.3f,0.3f);
	make_gradient_light(90,30,(float *)sky_lights_c1,0.6f,0.3f,0.3f,0.1f,0.3f,0.6f);

	make_gradient_light(0,30,(float *)sky_lights_c2,0.0f,0.4f,0.6f,0.6f,0.4f,0.0f);
	make_gradient_light(30,30,(float *)sky_lights_c2,0.6f,0.4f,0.0f,0.0f,0.1f,0.1f);
	make_gradient_light(60,30,(float *)sky_lights_c2,0.0f,0.1f,0.1f,0.6f,0.2f,0.1f);
	make_gradient_light(90,30,(float *)sky_lights_c2,0.6f,0.2f,0.1f,0.0f,0.4f,0.6f);

	make_gradient_light(0,30,(float *)sky_lights_c3,0.0f,0.7f,0.9f,0.9f,0.7f,0.0f);
	make_gradient_light(30,30,(float *)sky_lights_c3,0.9f,0.9f,0.0f,0.0f,0.1f,0.1f);
	make_gradient_light(60,30,(float *)sky_lights_c3,0.0f,0.1f,0.1f,0.5f,0.4f,0.4f);
	make_gradient_light(90,30,(float *)sky_lights_c3,0.5f,0.4f,0.4f,0.0f,0.7f,0.9f);


	make_gradient_light(0,30,(float *)sky_lights_c4,0.2f,0.8f,1.0f,1.0f,0.8f,0.2f);
	make_gradient_light(30,30,(float *)sky_lights_c4,1.0f,0.8f,0.2f,0.0f,0.1f,0.1f);
	make_gradient_light(60,30,(float *)sky_lights_c4,0.0f,0.1f,0.1f,0.7f,0.4f,0.5f);
	make_gradient_light(90,30,(float *)sky_lights_c4,0.7f,0.4f,0.5f,0.2f,0.8f,1.0f);

}

void build_sun_pos_table()
{
	float x,y,z,d;
	int i;
	int start=60;

	x=0;
	d=400;
	for(i=0;i<60*3;i++)
	{
   		z = d*sin((float)(i+start)*0.6f*3.1415926/180);
		y = d*cos((float)(i+start)*0.6f*3.1415926/180);
		x+=0.5f;

		sun_pos[i].x=x;
		sun_pos[i].y=y;
		sun_pos[i].z=z;
		sun_pos[i].w=0.0f;
	}
}

void new_minute()
{
	//morning starts at 0
	//is it morning?
	if(game_minute<60)light_level=game_minute+60;
	//is it evening?
	if(game_minute>=60*3 && game_minute<60*4)light_level=game_minute-60*3;
	//is it day?
	if(game_minute>=30 && game_minute<60*3+30 && !dungeon)
		{
			disable_local_lights();
			day_shadows_on=1;
			night_shadows_on=0;
			fLightPos[0]=sun_pos[game_minute-30].x;
			fLightPos[1]=sun_pos[game_minute-30].y;
			fLightPos[2]=sun_pos[game_minute-30].z;
			fLightPos[3]=sun_pos[game_minute-30].w;
			SetShadowMatrix();
		}
	else//it's too dark, or we are ina  dungeon
		{
			day_shadows_on=0;
			night_shadows_on=1;
			enable_local_lights();
		}
	//check to see if it is full day
	if(game_minute>=60 && game_minute<60*3)light_level=0;
	//full night?
	if(game_minute>=60*4)light_level=59;

}

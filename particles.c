#include <stdlib.h>
#include "global.h"

//Threading support for particals_list
void init_particles_list()
{
	int	i;

	particles_list_mutex=SDL_CreateMutex();
	lock_particles_list();	//lock it to avoid timing issues
	for(i=0;i<max_particle_systems;i++)particles_list[i]=0;
	unlock_particles_list();	// release now that we are done
}

void end_particles_list()
{
	SDL_DestroyMutex(particles_list_mutex);
	particles_list_mutex=NULL;
}


void draw_particle_sys(particle_sys *system_id)
{
	float x_pos,y_pos,z_pos;
	int total_particle_no;
	int i,part_type;

	x_pos=system_id->x_pos;
	y_pos=system_id->y_pos;
	z_pos=system_id->z_pos;
	total_particle_no=system_id->total_particle_no;
	part_type=system_id->part_type;

	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef (x_pos, y_pos, z_pos);
	glRotatef(-rz, 0.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS);
	lock_particles_list();	//lock it to avoid timing issues
	for(i=0;i<total_particle_no;i++)
		if(!system_id->particles[i].free)
		if(system_id->particles[i].z >= 0.0f)
			{
				float r,g,b,a,x,y,z;
				float u_start,u_end,v_start,v_end;
				int size;
				float x_len=0.15f;
				float z_len=0.15f;

				r=system_id->particles[i].r;
				g=system_id->particles[i].g;
				b=system_id->particles[i].b;
				a=system_id->particles[i].a;
				x=system_id->particles[i].x;
				y=system_id->particles[i].y;
				z=system_id->particles[i].z;
				size=system_id->particles[i].size;
				//determine the u/v location
				u_start=(float)size*8/64;
				u_end=u_start+(float)8/64;
				v_start=1.0f-(float)part_type*8/64;
				v_end=v_start-(float)8/64;

				glColor4f(r,g,b,a);

				glTexCoord2f(u_start,v_start);
				glVertex3f(x,y,z);

				glTexCoord2f(u_start,v_end);
				glVertex3f(x,y,z+z_len);

				glTexCoord2f(u_end,v_end);
				glVertex3f(x+x_len,y,z+z_len);

				glTexCoord2f(u_end,v_start);
				glVertex3f(x+x_len,y,z);


			}
	unlock_particles_list();	// release now that we are done
	glEnd();

	glPopMatrix();

}



int add_teleporter(float x_pos, float y_pos, float z_pos)
{
	int total_particle_no=800;

	particle_sys *system_id;

	float x,y,z,r,g,b,a;
	int size;
	int j,i;

	float start_r=0.3f;
	float start_g=0.6f;
	float start_b=0.9f;
	float start_a=0.2f;

	float start_x=0.0f;
	float start_y=0.0f;
	float start_z=1.5f;

	float start_x_random_deviation=7.4f;
	float start_y_random_deviation=7.4f;
	float start_z_random_deviation=7.5f;

	float start_r_random_deviation=0.03f;
	float start_g_random_deviation=0.1f;
	float start_b_random_deviation=0.1f;
	float start_a_random_deviation=0.3f;


	//allocate memory for this particle system
	system_id=(particle_sys *)calloc(1,sizeof(particle_sys));
	lock_particles_list();	//lock it to avoid timing issues
	//now, find a place for this system
	for(i=0;i<max_particle_systems;i++)
		{
			if(!particles_list[i])
				{
					particles_list[i]=system_id;
					break;
				}
		}


	//now set the free flag, on all the particles
	for(j=0;j<total_particle_no;j++)system_id->particles[j].free=1;

	system_id->x_pos=x_pos;
	system_id->y_pos=y_pos;
	system_id->z_pos=z_pos;
	system_id->part_sys_type=TELEPORTER_PARTICLE_SYS;
	system_id->part_type=TELEPORTER_PARTICLE;
	system_id->particle_count=800;
	system_id->total_particle_no=800;


	//find a free space
	for(j=0;j<total_particle_no;j++)
		if(system_id->particles[j].free)
			{
				//finally, we found a spot
				//calculate the color
				r=start_r+particle_rand(start_r_random_deviation);
				g=start_g+particle_rand(start_g_random_deviation);
				b=start_b+particle_rand(start_b_random_deviation);
				a=start_a+particle_rand(start_a_random_deviation);
				//get the position
				while(1)
					{
						x=start_x+particle_rand(start_x_random_deviation);
						y=start_y+particle_rand(start_y_random_deviation);
						z=start_z+particle_rand(start_z_random_deviation);
						if(x*x+y*y<0.0625f)break;//try again, the particle is out of our circular range
					}
				size=rand()%7;
				//ok, now put the things there
				system_id->particles[j].x=x;
				system_id->particles[j].y=y;
				system_id->particles[j].z=z;
				system_id->particles[j].r=r;
				system_id->particles[j].g=g;
				system_id->particles[j].b=b;
				system_id->particles[j].a=a;
				system_id->particles[j].size=size;

				//mark as occupied
				system_id->particles[j].free=0;
			}
	unlock_particles_list();	// release now that we are done
	return i;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
int add_teleport_in(int x_pos, int y_pos)
{
	int total_particle_no=800;
	float z_pos;

	particle_sys *system_id;

	float x,y,z,r,g,b,a;
	int size;
	int j,i;

	float start_r=0.4f;
	float start_g=1.0f;
	float start_b=0.6f;
	float start_a=0.5f;

	float start_x=0.0f;
	float start_y=0.0f;
	float start_z=0.5f;

	float start_x_random_deviation=7.4f;
	float start_y_random_deviation=7.4f;
	float start_z_random_deviation=6.5f;

	float start_r_random_deviation=0.3f;
	float start_g_random_deviation=0.3f;
	float start_b_random_deviation=0.1f;
	float start_a_random_deviation=0.3f;

	//get the Z
	z_pos=-2.2f+height_map[y_pos*tile_map_size_x*6+x_pos]*0.2f;


	//allocate memory for this particle system
	system_id=(particle_sys *)calloc(1,sizeof(particle_sys));
	//now, find a place for this system
	lock_particles_list();
	for(i=0;i<max_particle_systems;i++)
		{
			if(!particles_list[i])
				{
					particles_list[i]=system_id;
					break;
				}
		}


	//now set the free flag, on all the particles
	for(j=0;j<total_particle_no;j++)system_id->particles[j].free=1;

	system_id->x_pos=(float)x_pos/2+0.25f;
	system_id->y_pos=(float)y_pos/2+0.25f;
	system_id->z_pos=z_pos;
	system_id->part_sys_type=TELEPORT_OUT_PARTICLE_SYS;
	system_id->part_type=SPARKS_PARTICLE;
	system_id->particle_count=800;
	system_id->total_particle_no=800;
	system_id->system_ttl=8;


	//find a free space
	for(j=0;j<total_particle_no;j++)
		if(system_id->particles[j].free)
			{
				//finally, we found a spot
				//calculate the color
				r=start_r+particle_rand(start_r_random_deviation);
				g=start_g+particle_rand(start_g_random_deviation);
				b=start_b+particle_rand(start_b_random_deviation);
				a=start_a+particle_rand(start_a_random_deviation);
				//get the position
				while(1)
					{
						x=start_x+particle_rand(start_x_random_deviation);
						y=start_y+particle_rand(start_y_random_deviation);
						z=start_z+particle_rand(start_z_random_deviation);
						if(x*x+y*y<0.0625f || z<0)break;//try again, the particle is out of our circular range
					}
				size=rand()%7;
				//ok, now put the things there
				system_id->particles[j].x=x;
				system_id->particles[j].y=y;
				system_id->particles[j].z=z;
				system_id->particles[j].r=r;
				system_id->particles[j].g=g;
				system_id->particles[j].b=b;
				system_id->particles[j].a=a;
				system_id->particles[j].size=size;

				//mark as occupied
				system_id->particles[j].free=0;
			}

	update_teleport_out(system_id);
	update_teleport_out(system_id);
	update_teleport_out(system_id);
	update_teleport_out(system_id);
	unlock_particles_list();
	return i;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
int add_teleport_out(int x_pos, int y_pos)
{
	int total_particle_no=800;
	float z_pos;

	particle_sys *system_id;

	float x,y,z,r,g,b,a;
	int size;
	int j,i;

	float start_r=0.9f;
	float start_g=0.6f;
	float start_b=0.0f;
	float start_a=0.5f;

	float start_x=0.0f;
	float start_y=0.0f;
	float start_z=0.5f;

	float start_x_random_deviation=7.4f;
	float start_y_random_deviation=7.4f;
	float start_z_random_deviation=6.5f;

	float start_r_random_deviation=0.3f;
	float start_g_random_deviation=0.3f;
	float start_b_random_deviation=0.1f;
	float start_a_random_deviation=0.3f;

	//get the Z
	z_pos=-2.2f+height_map[y_pos*tile_map_size_x*6+x_pos]*0.2f;


	//allocate memory for this particle system
	system_id=(particle_sys *)calloc(1,sizeof(particle_sys));
	//now, find a place for this system
	lock_particles_list();
	for(i=0;i<max_particle_systems;i++)
		{
			if(!particles_list[i])
				{
					particles_list[i]=system_id;
					break;
				}
		}


	//now set the free flag, on all the particles
	for(j=0;j<total_particle_no;j++)system_id->particles[j].free=1;

	system_id->x_pos=(float)x_pos/2+0.25f;
	system_id->y_pos=(float)y_pos/2+0.25f;
	system_id->z_pos=z_pos;
	system_id->part_sys_type=TELEPORT_OUT_PARTICLE_SYS;
	system_id->part_type=SPARKS_PARTICLE;
	system_id->particle_count=800;
	system_id->total_particle_no=800;
	system_id->system_ttl=8;


	//find a free space
	for(j=0;j<total_particle_no;j++)
		if(system_id->particles[j].free)
			{
				//finally, we found a spot
				//calculate the color
				r=start_r+particle_rand(start_r_random_deviation);
				g=start_g+particle_rand(start_g_random_deviation);
				b=start_b+particle_rand(start_b_random_deviation);
				a=start_a+particle_rand(start_a_random_deviation);
				//get the position
				while(1)
					{
						x=start_x+particle_rand(start_x_random_deviation);
						y=start_y+particle_rand(start_y_random_deviation);
						z=start_z+particle_rand(start_z_random_deviation);
						if(x*x+y*y<0.0625f || z<-2)break;//try again, the particle is out of our circular range
					}
				size=rand()%7;
				//ok, now put the things there
				system_id->particles[j].x=x;
				system_id->particles[j].y=y;
				system_id->particles[j].z=z;
				system_id->particles[j].r=r;
				system_id->particles[j].g=g;
				system_id->particles[j].b=b;
				system_id->particles[j].a=a;
				system_id->particles[j].size=size;

				//mark as occupied
				system_id->particles[j].free=0;
			}

	update_teleport_out(system_id);
	update_teleport_out(system_id);
	update_teleport_out(system_id);
	update_teleport_out(system_id);
	unlock_particles_list();
	return i;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int add_bag_in(int x_pos, int y_pos)
{
	int total_particle_no=300;
	float z_pos;

	particle_sys *system_id;

	float x,y,z,r,g,b,a;
	int size;
	int j,i;

	float start_r=0.8f;
	float start_g=0.6f;
	float start_b=0.6f;
	float start_a=0.5f;

	float start_x=0.0f;
	float start_y=0.0f;
	float start_z=0.5f;

	float start_x_random_deviation=1.4f;
	float start_y_random_deviation=1.4f;
	float start_z_random_deviation=4.5f;

	float start_r_random_deviation=0.05f;
	float start_g_random_deviation=0.04f;
	float start_b_random_deviation=0.01f;
	float start_a_random_deviation=0.3f;

	//get the Z
	z_pos=-2.2f+height_map[y_pos*tile_map_size_x*6+x_pos]*0.2f;


	//allocate memory for this particle system
	system_id=(particle_sys *)calloc(1,sizeof(particle_sys));
	//now, find a place for this system
	lock_particles_list();
	for(i=0;i<max_particle_systems;i++)
		{
			if(!particles_list[i])
				{
					particles_list[i]=system_id;
					break;
				}
		}


	//now set the free flag, on all the particles
	for(j=0;j<total_particle_no;j++)system_id->particles[j].free=1;

	system_id->x_pos=(float)x_pos/2+0.25f;
	system_id->y_pos=(float)y_pos/2+0.25f;
	system_id->z_pos=z_pos;
	system_id->part_sys_type=BAG_IN_PARTICLE_SYS;
	system_id->part_type=SPARKS_PARTICLE;
	system_id->particle_count=300;
	system_id->total_particle_no=300;
	system_id->system_ttl=8;


	//find a free space
	for(j=0;j<total_particle_no;j++)
		if(system_id->particles[j].free)
			{
				//finally, we found a spot
				//calculate the color
				r=start_r+particle_rand(start_r_random_deviation);
				g=start_g+particle_rand(start_g_random_deviation);
				b=start_b+particle_rand(start_b_random_deviation);
				a=start_a+particle_rand(start_a_random_deviation);
				//get the position
				while(1)
					{
						x=start_x+particle_rand(start_x_random_deviation);
						y=start_y+particle_rand(start_y_random_deviation);
						z=start_z+particle_rand(start_z_random_deviation);
						if(x*x+y*y<0.0625f || z<0)break;//try again, the particle is out of our circular range
					}
				size=rand()%7;
				//ok, now put the things there
				system_id->particles[j].x=x;
				system_id->particles[j].y=y;
				system_id->particles[j].z=z;
				system_id->particles[j].r=r;
				system_id->particles[j].g=g;
				system_id->particles[j].b=b;
				system_id->particles[j].a=a;
				system_id->particles[j].size=size;

				//mark as occupied
				system_id->particles[j].free=0;
			}

	update_teleport_out(system_id);
	update_teleport_out(system_id);
	update_teleport_out(system_id);
	update_teleport_out(system_id);
	unlock_particles_list();
	return i;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int add_bag_out(int x_pos, int y_pos)
{
	int total_particle_no=300;
	float z_pos;

	particle_sys *system_id;

	float x,y,z,r,g,b,a;
	int size;
	int j,i;

	float start_r=0.8f;
	float start_g=0.2f;
	float start_b=0.2f;
	float start_a=0.5f;

	float start_x=0.0f;
	float start_y=0.0f;
	float start_z=0.5f;

	float start_x_random_deviation=1.4f;
	float start_y_random_deviation=1.4f;
	float start_z_random_deviation=4.5f;

	float start_r_random_deviation=0.05f;
	float start_g_random_deviation=0.04f;
	float start_b_random_deviation=0.01f;
	float start_a_random_deviation=0.3f;

	//get the Z
	z_pos=-2.2f+height_map[y_pos*tile_map_size_x*6+x_pos]*0.2f;


	//allocate memory for this particle system
	system_id=(particle_sys *)calloc(1,sizeof(particle_sys));
	//now, find a place for this system
	lock_particles_list();
	for(i=0;i<max_particle_systems;i++)
		{
			if(!particles_list[i])
				{
					particles_list[i]=system_id;
					break;
				}
		}


	//now set the free flag, on all the particles
	for(j=0;j<total_particle_no;j++)system_id->particles[j].free=1;

	system_id->x_pos=(float)x_pos/2+0.25f;
	system_id->y_pos=(float)y_pos/2+0.25f;
	system_id->z_pos=z_pos;
	system_id->part_sys_type=BAG_OUT_PARTICLE_SYS;
	system_id->part_type=SPARKS_PARTICLE;
	system_id->particle_count=300;
	system_id->total_particle_no=300;
	system_id->system_ttl=8;


	//find a free space
	for(j=0;j<total_particle_no;j++)
		if(system_id->particles[j].free)
			{
				//finally, we found a spot
				//calculate the color
				r=start_r+particle_rand(start_r_random_deviation);
				g=start_g+particle_rand(start_g_random_deviation);
				b=start_b+particle_rand(start_b_random_deviation);
				a=start_a+particle_rand(start_a_random_deviation);
				//get the position
				while(1)
					{
						x=start_x+particle_rand(start_x_random_deviation);
						y=start_y+particle_rand(start_y_random_deviation);
						z=start_z+particle_rand(start_z_random_deviation);
						if(x*x+y*y<0.0625f || z<0)break;//try again, the particle is out of our circular range
					}
				size=rand()%7;
				//ok, now put the things there
				system_id->particles[j].x=x;
				system_id->particles[j].y=y;
				system_id->particles[j].z=z;
				system_id->particles[j].r=r;
				system_id->particles[j].g=g;
				system_id->particles[j].b=b;
				system_id->particles[j].a=a;
				system_id->particles[j].size=size;

				//mark as occupied
				system_id->particles[j].free=0;
			}

	update_teleport_out(system_id);
	update_teleport_out(system_id);
	update_teleport_out(system_id);
	update_teleport_out(system_id);
	unlock_particles_list();
	return i;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void display_particles()
{

	int i;
	int x,y;

	x=-cx;
	y=-cy;

	if(last_texture!=texture_cache[particles_text].texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, texture_cache[particles_text].texture_id);
			last_texture=texture_cache[particles_text].texture_id;
		}
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_SRC_ALPHA_SATURATE);
	glDisable(GL_CULL_FACE);

	lock_particles_list();
	for(i=0;i<max_particle_systems;i++)
		{
			if(particles_list[i])
				{
					int dist1;
					int dist2;

					dist1=x-particles_list[i]->x_pos;
					dist2=y-particles_list[i]->y_pos;
					if(dist1*dist1+dist2*dist2<=15*15)
						draw_particle_sys(particles_list[i]);
				}
		}
	unlock_particles_list();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void update_teleporter(particle_sys *system_id)
{
	int i;
	int particle_count;
	int total_particle_no=800;
	int particles_to_add_per_frame=800;
	int particles_to_add=0;

	float x,y,z,r,g,b,a;
	int size,rand_size;
	int j;

	float start_r=0.3f;
	float start_g=0.6f;
	float start_b=0.9f;
	float start_a=0.2f;

	float start_x=0.0f;
	float start_y=0.0f;
	float start_z=0.0f;


	float start_x_random_deviation=3.6f;
	float start_y_random_deviation=3.6f;
	float start_z_random_deviation=3.1f;

	float start_r_random_deviation=0.03f;
	float start_g_random_deviation=0.1f;
	float start_b_random_deviation=0.1f;
	float start_a_random_deviation=0.3f;

	float path_x_deviation=0.0f;
	float path_y_deviation=0.0f;
	float path_z_deviation=0.05f;

	float path_r_deviation=0.01;
	float path_g_deviation=0.01;
	float path_b_deviation=0.01;
	float path_a_deviation=0.004;

	float path_x_random_deviation=0.1f;
	float path_y_random_deviation=0.1f;
	float path_z_random_deviation=0.2f;

	float path_r_random_deviation=0.01f;
	float path_g_random_deviation=0.01f;
	float path_b_random_deviation=0.01f;
	float path_a_random_deviation=0.01f;


	particle_count=system_id->particle_count;

	if(particle_count < system_id->total_particle_no)
		{
			particles_to_add=system_id->total_particle_no-particle_count;
			if(particles_to_add>particles_to_add_per_frame)
				particles_to_add=particles_to_add_per_frame;
		}
	//see if we need to add new particles
	lock_particles_list();
	if(particles_to_add)
		for(j=i=0;i<particles_to_add;i++)
			{
				//find a free space
				for(;j<total_particle_no;j++)
					if(system_id->particles[j].free)
						{
							//finally, we found a spot
							//calculate the color
							r=start_r+particle_rand(start_r_random_deviation);
							g=start_g+particle_rand(start_g_random_deviation);
							b=start_b+particle_rand(start_b_random_deviation);
							a=start_a+particle_rand(start_a_random_deviation);
							//get the position
							while(1)
								{
									x=start_x+particle_rand(start_x_random_deviation);
									y=start_y+particle_rand(start_y_random_deviation);
									z=start_z+particle_rand(start_z_random_deviation);
									if(z<0)z=0;
									if(x*x+y*y<0.15f)break;//try again, the particle is out of our circular range
								}
							size=rand()%7;
							//ok, now put the things there
							system_id->particles[j].x=x;
							system_id->particles[j].y=y;
							system_id->particles[j].z=z;
							system_id->particles[j].r=r;
							system_id->particles[j].g=g;
							system_id->particles[j].b=b;
							system_id->particles[j].a=a;
							system_id->particles[j].size=size;

							//mark as occupied
							system_id->particles[j].free=0;
							//increase the particle count
							system_id->particle_count++;
							break;	//done looping
						}

			}

	//excellent, now we have to actually update the particles
	//find an used particle
	for(j=0;j<total_particle_no;j++)
		if(!system_id->particles[j].free)
			{
				if(system_id->particles[j].z>2.0f)
					{
						//poor particle, it died :(
						system_id->particles[j].free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				x=system_id->particles[j].x;
				y=system_id->particles[j].y;
				z=system_id->particles[j].z;
				r=system_id->particles[j].r;
				g=system_id->particles[j].g;
				b=system_id->particles[j].b;
				a=system_id->particles[j].a;
				size=system_id->particles[j].size;

				//calculate the color
				r+=path_r_deviation+particle_rand(path_r_random_deviation);
				g+=path_g_deviation+particle_rand(path_g_random_deviation);
				b+=path_b_deviation+particle_rand(path_b_random_deviation);
				a+=path_a_deviation+particle_rand(path_a_random_deviation);
				//get the position

				x+=path_x_deviation+particle_rand(path_x_random_deviation);
				y+=path_y_deviation+particle_rand(path_y_random_deviation);
				z+=path_z_deviation+particle_rand(path_z_random_deviation);

				rand_size=rand()%3;
				if(rand_size==1 && size>0)size--;
				if(rand_size==2 && size<6)size++;
				//ok, now put the things there
				system_id->particles[j].x=x;
				system_id->particles[j].y=y;
				system_id->particles[j].z=z;
				system_id->particles[j].r=r;
				system_id->particles[j].g=g;
				system_id->particles[j].b=b;
				system_id->particles[j].a=a;
				system_id->particles[j].size=size;
			}
	unlock_particles_list();

}
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void update_teleport_in(particle_sys *system_id)
{
	int i;
	int particle_count;
	int total_particle_no=800;
	int particles_to_add_per_frame=800;
	int particles_to_add=0;

	float x,y,z,r,g,b,a;
	int size,rand_size;
	int j;

	float start_r=0.4f;
	float start_g=1.0f;
	float start_b=0.8f;
	float start_a=0.5f;

	float start_x=0.0f;
	float start_y=0.0f;
	float start_z=0.0f;


	float start_x_random_deviation=3.6f;
	float start_y_random_deviation=3.6f;
	float start_z_random_deviation=3.1f;

	float start_r_random_deviation=0.3f;
	float start_g_random_deviation=0.3f;
	float start_b_random_deviation=0.1f;
	float start_a_random_deviation=0.3f;

	float path_x_deviation=0.0f;
	float path_y_deviation=0.0f;
	float path_z_deviation=0.04f;

	float path_x_random_deviation=0.1f;
	float path_y_random_deviation=0.1f;
	float path_z_random_deviation=0.1f;

	float path_r_random_deviation=0.01f;
	float path_g_random_deviation=0.01f;
	float path_b_random_deviation=0.01f;
	float path_a_random_deviation=0.01f;


	particle_count=system_id->particle_count;

	if(particle_count < system_id->total_particle_no)
		{
			particles_to_add=system_id->total_particle_no-particle_count;
			if(particles_to_add>particles_to_add_per_frame)
				particles_to_add=particles_to_add_per_frame;
		}
	//see if we need to add new particles
	lock_particles_list();
	if(system_id->system_ttl)
		if(particles_to_add)
			for(j=i=0;i<particles_to_add;i++)
				{
					//find a free space
					for(;j<total_particle_no;j++)
						if(system_id->particles[j].free)
							{
								//finally, we found a spot
								//calculate the color
								r=start_r+particle_rand(start_r_random_deviation);
								g=start_g+particle_rand(start_g_random_deviation);
								b=start_b+particle_rand(start_b_random_deviation);
								a=start_a+particle_rand(start_a_random_deviation);
								//get the position
								while(1)
									{
										x=start_x+particle_rand(start_x_random_deviation);
										y=start_y+particle_rand(start_y_random_deviation);
										z=start_z+particle_rand(start_z_random_deviation);
										if(z<0)z=0;
										if(x*x+y*y<0.15f || z<0)break;//try again, the particle is out of our circular range
									}
								size=rand()%7;
								//ok, now put the things there
								system_id->particles[j].x=x;
								system_id->particles[j].y=y;
								system_id->particles[j].z=z;
								system_id->particles[j].r=r;
								system_id->particles[j].g=g;
								system_id->particles[j].b=b;
								system_id->particles[j].a=a;
								system_id->particles[j].size=size;

								//mark as occupied
								system_id->particles[j].free=0;
								//increase the particle count
								system_id->particle_count++;
								break;	//done looping
							}

				}

	//excellent, now we have to actually update the particles
	//find an used particle
	for(j=0;j<total_particle_no;j++)
		if(!system_id->particles[j].free)
			{
				if(system_id->particles[j].z>2.0f)
					{
						//poor particle, it died :(
						system_id->particles[j].free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				x=system_id->particles[j].x;
				y=system_id->particles[j].y;
				z=system_id->particles[j].z;
				r=system_id->particles[j].r;
				g=system_id->particles[j].g;
				b=system_id->particles[j].b;
				a=system_id->particles[j].a;
				size=system_id->particles[j].size;

				//calculate the color
				r+=particle_rand(path_r_random_deviation);
				g+=particle_rand(path_g_random_deviation);
				b+=particle_rand(path_b_random_deviation);
				a+=particle_rand(path_a_random_deviation);
				//get the position

				x+=path_x_deviation+particle_rand(path_x_random_deviation);
				y+=path_y_deviation+particle_rand(path_y_random_deviation);
				z+=path_z_deviation+particle_rand(path_z_random_deviation);

				rand_size=rand()%3;
				if(rand_size==1 && size>0)size--;
				if(rand_size==2 && size<6)size++;
				//ok, now put the things there
				system_id->particles[j].x=x;
				system_id->particles[j].y=y;
				system_id->particles[j].z=z;
				system_id->particles[j].r=r;
				system_id->particles[j].g=g;
				system_id->particles[j].b=b;
				system_id->particles[j].a=a;
				system_id->particles[j].size=size;
			}
	if(system_id->system_ttl)system_id->system_ttl--;

	if(!system_id->system_ttl && !system_id->particle_count)
		//if there are no more particles to add, and the TTL expired, then kill this evil system
		for(i=0;i<max_particle_systems;i++)
			if(particles_list[i]==system_id)
				{
					free(system_id);
					particles_list[i]=0;
					break;
				}
	unlock_particles_list();

}
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void update_bag_in(particle_sys *system_id)
{
	int i;
	int particle_count;
	int total_particle_no=300;
	int particles_to_add_per_frame=300;
	int particles_to_add=0;

	float x,y,z,r,g,b,a;
	int size,rand_size;
	int j;

	float start_r=0.8f;
	float start_g=0.6f;
	float start_b=0.1f;
	float start_a=0.2f;

	float start_x=0.0f;
	float start_y=0.0f;
	float start_z=0.0f;


	float start_x_random_deviation=2.6f;
	float start_y_random_deviation=2.6f;
	float start_z_random_deviation=4.1f;

	float start_r_random_deviation=0.03f;
	float start_g_random_deviation=0.03f;
	float start_b_random_deviation=0.01f;
	float start_a_random_deviation=0.3f;

	float path_x_deviation=0.0f;
	float path_y_deviation=0.0f;
	float path_z_deviation=0.04f;

	float path_x_random_deviation=0.1f;
	float path_y_random_deviation=0.1f;
	float path_z_random_deviation=0.1f;

	float path_r_random_deviation=0.01f;
	float path_g_random_deviation=0.01f;
	float path_b_random_deviation=0.01f;
	float path_a_random_deviation=0.01f;


	particle_count=system_id->particle_count;

	if(particle_count < system_id->total_particle_no)
		{
			particles_to_add=system_id->total_particle_no-particle_count;
			if(particles_to_add>particles_to_add_per_frame)
				particles_to_add=particles_to_add_per_frame;
		}
	//see if we need to add new particles
	lock_particles_list();
	if(system_id->system_ttl)
		if(particles_to_add)
			for(j=i=0;i<particles_to_add;i++)
				{
					//find a free space
					for(;j<total_particle_no;j++)
						if(system_id->particles[j].free)
							{
								//finally, we found a spot
								//calculate the color
								r=start_r+particle_rand(start_r_random_deviation);
								g=start_g+particle_rand(start_g_random_deviation);
								b=start_b+particle_rand(start_b_random_deviation);
								a=start_a+particle_rand(start_a_random_deviation);
								//get the position
								while(1)
									{
										x=start_x+particle_rand(start_x_random_deviation);
										y=start_y+particle_rand(start_y_random_deviation);
										z=start_z+particle_rand(start_z_random_deviation);
										if(z<0)z=0;
										if(x*x+y*y<0.15f || z<0)break;//try again, the particle is out of our circular range
									}
								size=rand()%7;
								//ok, now put the things there
								system_id->particles[j].x=x;
								system_id->particles[j].y=y;
								system_id->particles[j].z=z;
								system_id->particles[j].r=r;
								system_id->particles[j].g=g;
								system_id->particles[j].b=b;
								system_id->particles[j].a=a;
								system_id->particles[j].size=size;

								//mark as occupied
								system_id->particles[j].free=0;
								//increase the particle count
								system_id->particle_count++;
								break;	//done looping
							}

				}

	//excellent, now we have to actually update the particles
	//find an used particle
	for(j=0;j<total_particle_no;j++)
		if(!system_id->particles[j].free)
			{
				if(system_id->particles[j].z>1.0f)
					{
						//poor particle, it died :(
						system_id->particles[j].free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				x=system_id->particles[j].x;
				y=system_id->particles[j].y;
				z=system_id->particles[j].z;
				r=system_id->particles[j].r;
				g=system_id->particles[j].g;
				b=system_id->particles[j].b;
				a=system_id->particles[j].a;
				size=system_id->particles[j].size;

				//calculate the color
				r+=particle_rand(path_r_random_deviation);
				g+=particle_rand(path_g_random_deviation);
				b+=particle_rand(path_b_random_deviation);
				a+=particle_rand(path_a_random_deviation);
				//get the position

				x+=path_x_deviation+particle_rand(path_x_random_deviation);
				y+=path_y_deviation+particle_rand(path_y_random_deviation);
				z+=path_z_deviation+particle_rand(path_z_random_deviation);

				rand_size=rand()%3;
				if(rand_size==1 && size>0)size--;
				if(rand_size==2 && size<6)size++;
				//ok, now put the things there
				system_id->particles[j].x=x;
				system_id->particles[j].y=y;
				system_id->particles[j].z=z;
				system_id->particles[j].r=r;
				system_id->particles[j].g=g;
				system_id->particles[j].b=b;
				system_id->particles[j].a=a;
				system_id->particles[j].size=size;
			}
	if(system_id->system_ttl)system_id->system_ttl--;

	if(!system_id->system_ttl && !system_id->particle_count)
		//if there are no more particles to add, and the TTL expired, then kill this evil system
		for(i=0;i<max_particle_systems;i++)
			if(particles_list[i]==system_id)
				{
					free(system_id);
					particles_list[i]=0;
					break;
				}
	unlock_particles_list();

}
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void update_bag_out(particle_sys *system_id)
{
	int i;
	int particle_count;
	int total_particle_no=300;
	int particles_to_add_per_frame=300;
	int particles_to_add=0;

	float x,y,z,r,g,b,a;
	int size,rand_size;
	int j;

	float start_r=0.8f;
	float start_g=0.8f;
	float start_b=0.8f;
	float start_a=0.2f;

	float start_x=0.0f;
	float start_y=0.0f;
	float start_z=0.0f;


	float start_x_random_deviation=2.6f;
	float start_y_random_deviation=2.6f;
	float start_z_random_deviation=4.1f;

	float start_r_random_deviation=0.03f;
	float start_g_random_deviation=0.03f;
	float start_b_random_deviation=0.01f;
	float start_a_random_deviation=0.3f;

	float path_x_deviation=0.0f;
	float path_y_deviation=0.0f;
	float path_z_deviation=0.04f;

	float path_x_random_deviation=0.1f;
	float path_y_random_deviation=0.1f;
	float path_z_random_deviation=0.1f;

	float path_r_random_deviation=0.01f;
	float path_g_random_deviation=0.01f;
	float path_b_random_deviation=0.01f;
	float path_a_random_deviation=0.01f;


	particle_count=system_id->particle_count;

	if(particle_count < system_id->total_particle_no)
		{
			particles_to_add=system_id->total_particle_no-particle_count;
			if(particles_to_add>particles_to_add_per_frame)
				particles_to_add=particles_to_add_per_frame;
		}
	//see if we need to add new particles
	lock_particles_list();
	if(system_id->system_ttl)
		if(particles_to_add)
			for(j=i=0;i<particles_to_add;i++)
				{
					//find a free space
					for(;j<total_particle_no;j++)
						if(system_id->particles[j].free)
							{
								//finally, we found a spot
								//calculate the color
								r=start_r+particle_rand(start_r_random_deviation);
								g=start_g+particle_rand(start_g_random_deviation);
								b=start_b+particle_rand(start_b_random_deviation);
								a=start_a+particle_rand(start_a_random_deviation);
								//get the position
								while(1)
									{
										x=start_x+particle_rand(start_x_random_deviation);
										y=start_y+particle_rand(start_y_random_deviation);
										z=start_z+particle_rand(start_z_random_deviation);
										if(z<0)z=0;
										if(x*x+y*y<0.15f || z<0)break;//try again, the particle is out of our circular range
									}
								size=rand()%7;
								//ok, now put the things there
								system_id->particles[j].x=x;
								system_id->particles[j].y=y;
								system_id->particles[j].z=z;
								system_id->particles[j].r=r;
								system_id->particles[j].g=g;
								system_id->particles[j].b=b;
								system_id->particles[j].a=a;
								system_id->particles[j].size=size;

								//mark as occupied
								system_id->particles[j].free=0;
								//increase the particle count
								system_id->particle_count++;
								break;	//done looping
							}

				}

	//excellent, now we have to actually update the particles
	//find an used particle
	for(j=0;j<total_particle_no;j++)
		if(!system_id->particles[j].free)
			{
				if(system_id->particles[j].z>1.0f)
					{
						//poor particle, it died :(
						system_id->particles[j].free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				x=system_id->particles[j].x;
				y=system_id->particles[j].y;
				z=system_id->particles[j].z;
				r=system_id->particles[j].r;
				g=system_id->particles[j].g;
				b=system_id->particles[j].b;
				a=system_id->particles[j].a;
				size=system_id->particles[j].size;

				//calculate the color
				r+=particle_rand(path_r_random_deviation);
				g+=particle_rand(path_g_random_deviation);
				b+=particle_rand(path_b_random_deviation);
				a+=particle_rand(path_a_random_deviation);
				//get the position

				x+=path_x_deviation+particle_rand(path_x_random_deviation);
				y+=path_y_deviation+particle_rand(path_y_random_deviation);
				z+=path_z_deviation+particle_rand(path_z_random_deviation);

				rand_size=rand()%3;
				if(rand_size==1 && size>0)size--;
				if(rand_size==2 && size<6)size++;
				//ok, now put the things there
				system_id->particles[j].x=x;
				system_id->particles[j].y=y;
				system_id->particles[j].z=z;
				system_id->particles[j].r=r;
				system_id->particles[j].g=g;
				system_id->particles[j].b=b;
				system_id->particles[j].a=a;
				system_id->particles[j].size=size;
			}
	if(system_id->system_ttl)system_id->system_ttl--;

	if(!system_id->system_ttl && !system_id->particle_count)
		//if there are no more particles to add, and the TTL expired, then kill this evil system
		for(i=0;i<max_particle_systems;i++)
			if(particles_list[i]==system_id)
				{
					free(system_id);
					particles_list[i]=0;
					break;
				}
	unlock_particles_list();

}
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void update_teleport_out(particle_sys *system_id)
{
	int i;
	int particle_count;
	int total_particle_no=800;
	int particles_to_add_per_frame=800;
	int particles_to_add=0;

	float x,y,z,r,g,b,a;
	int size,rand_size;
	int j;

	float start_r=0.9f;
	float start_g=0.6f;
	float start_b=0.1f;
	float start_a=0.5f;

	float start_x=0.0f;
	float start_y=0.0f;
	float start_z=0.0f;


	float start_x_random_deviation=3.6f;
	float start_y_random_deviation=3.6f;
	float start_z_random_deviation=3.1f;

	float start_r_random_deviation=0.3f;
	float start_g_random_deviation=0.3f;
	float start_b_random_deviation=0.1f;
	float start_a_random_deviation=0.3f;

	float path_x_deviation=0.0f;
	float path_y_deviation=0.0f;
	float path_z_deviation=0.04f;

	float path_x_random_deviation=0.1f;
	float path_y_random_deviation=0.1f;
	float path_z_random_deviation=0.1f;

	float path_r_random_deviation=0.01f;
	float path_g_random_deviation=0.01f;
	float path_b_random_deviation=0.01f;
	float path_a_random_deviation=0.01f;


	particle_count=system_id->particle_count;

	if(particle_count < system_id->total_particle_no)
		{
			particles_to_add=system_id->total_particle_no-particle_count;
			if(particles_to_add>particles_to_add_per_frame)
				particles_to_add=particles_to_add_per_frame;
		}
	//see if we need to add new particles
	lock_particles_list();
	if(system_id->system_ttl)
		if(particles_to_add)
			for(j=i=0;i<particles_to_add;i++)
				{
					//find a free space
					for(;j<total_particle_no;j++)
						if(system_id->particles[j].free)
							{
								//finally, we found a spot
								//calculate the color
								r=start_r+particle_rand(start_r_random_deviation);
								g=start_g+particle_rand(start_g_random_deviation);
								b=start_b+particle_rand(start_b_random_deviation);
								a=start_a+particle_rand(start_a_random_deviation);
								//get the position
								while(1)
									{
										x=start_x+particle_rand(start_x_random_deviation);
										y=start_y+particle_rand(start_y_random_deviation);
										z=start_z+particle_rand(start_z_random_deviation);
										if(z<0)z=0;
										if(x*x+y*y<0.15f || z<-2)break;//try again, the particle is out of our circular range
									}
								size=rand()%7;
								//ok, now put the things there
								system_id->particles[j].x=x;
								system_id->particles[j].y=y;
								system_id->particles[j].z=z;
								system_id->particles[j].r=r;
								system_id->particles[j].g=g;
								system_id->particles[j].b=b;
								system_id->particles[j].a=a;
								system_id->particles[j].size=size;

								//mark as occupied
								system_id->particles[j].free=0;
								//increase the particle count
								system_id->particle_count++;
								break;	//done looping
							}

				}

	//excellent, now we have to actually update the particles
	//find an used particle
	for(j=0;j<total_particle_no;j++)
		if(!system_id->particles[j].free)
			{
				if(system_id->particles[j].z>2.0f)
					{
						//poor particle, it died :(
						system_id->particles[j].free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				x=system_id->particles[j].x;
				y=system_id->particles[j].y;
				z=system_id->particles[j].z;
				r=system_id->particles[j].r;
				g=system_id->particles[j].g;
				b=system_id->particles[j].b;
				a=system_id->particles[j].a;
				size=system_id->particles[j].size;

				//calculate the color
				r+=particle_rand(path_r_random_deviation);
				g+=particle_rand(path_g_random_deviation);
				b+=particle_rand(path_b_random_deviation);
				a+=particle_rand(path_a_random_deviation);
				//get the position

				x+=path_x_deviation+particle_rand(path_x_random_deviation);
				y+=path_y_deviation+particle_rand(path_y_random_deviation);
				z+=path_z_deviation+particle_rand(path_z_random_deviation);

				rand_size=rand()%3;
				if(rand_size==1 && size>0)size--;
				if(rand_size==2 && size<6)size++;
				//ok, now put the things there
				system_id->particles[j].x=x;
				system_id->particles[j].y=y;
				system_id->particles[j].z=z;
				system_id->particles[j].r=r;
				system_id->particles[j].g=g;
				system_id->particles[j].b=b;
				system_id->particles[j].a=a;
				system_id->particles[j].size=size;
			}
	if(system_id->system_ttl)system_id->system_ttl--;

	if(!system_id->system_ttl && !system_id->particle_count)
		//if there are no more particles to add, and the TTL expired, then kill this evil system
		for(i=0;i<max_particle_systems;i++)
			if(particles_list[i]==system_id)
				{
					free(system_id);
					particles_list[i]=0;
					break;
				}
	unlock_particles_list();

}
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////



void update_particles()
{

	int i;
	int x,y;

	x=-cx;
	y=-cy;
	lock_particles_list();
	for(i=0;i<max_particle_systems;i++)
		{
			if(particles_list[i])
				{
					int dist1;
					int dist2;

					dist1=x-particles_list[i]->x_pos;
					dist2=y-particles_list[i]->y_pos;
					if(dist1*dist1+dist2*dist2<=15*15)
			         	{
							if(particles_list[i]->part_sys_type==TELEPORTER_PARTICLE_SYS)
								update_teleporter(particles_list[i]);
                     		else if(particles_list[i]->part_sys_type==TELEPORT_IN_PARTICLE_SYS)
								update_teleport_in(particles_list[i]);
							else if(particles_list[i]->part_sys_type==TELEPORT_OUT_PARTICLE_SYS)
								update_teleport_out(particles_list[i]);
							else if(particles_list[i]->part_sys_type==BAG_IN_PARTICLE_SYS)
								update_bag_in(particles_list[i]);
							else if(particles_list[i]->part_sys_type==BAG_OUT_PARTICLE_SYS)
								update_bag_out(particles_list[i]);
						}
				}
		}
	unlock_particles_list();
}


void add_teleporters_from_list(Uint8 *teleport_list)
{
	Uint16 teleporters_no;
	int i;
	int teleport_x,teleport_y,teleport_type,my_offset;
	float x,y,z;

	teleporters_no=*((Uint16 *)(teleport_list));
	lock_particles_list();	//lock it to avoid timing issues
	for(i=0;i<teleporters_no;i++)
		{
			my_offset=i*5+2;
			teleport_x=*((Uint16 *)(teleport_list+my_offset));
			teleport_y=*((Uint16 *)(teleport_list+my_offset+2));
			teleport_type=*((Uint16 *)(teleport_list+my_offset+4));
			//put the sound
			add_sound_object("./sound/teleporter.wav",teleport_x,teleport_y,1,-1,0);
			//later on, maybe we want to have different visual types
			//now, get the Z position
			z=-2.2f+height_map[teleport_y*tile_map_size_x*6+teleport_x]*0.2f;
			//convert from height values to meters
			x=(float)teleport_x/2;
			y=(float)teleport_y/2;
			//center the object
			x=x+0.25f;
			y=y+0.25f;


			add_teleporter(x,y,z);
			add_e3d("./3dobjects/misc_objects/portal1.e3d",x,y,z,0,0,0,1,0,1.0f,1.0f,1.0f);

		}
	unlock_particles_list();

}

void destroy_all_particles()
{
	int i;
	lock_particles_list();
	for(i=0;i<max_particle_systems;i++)
		if(particles_list[i])
			{
				free(particles_list[i]);
				particles_list[i]=0;
			}
	unlock_particles_list();

}



#include <stdlib.h>
#include <math.h>
#include "SDL_opengl.h"
#include "global.h"
#include "string.h"

SDL_mutex *particles_list_mutex;	//used for locking between the timer and main threads
int particle_textures[8];
particle_sys *particles_list[max_particle_systems];


/******************************************************
 *           PARTICLE SYSTEM DEFINITIONS              *
 ******************************************************/
#define max_particle_defs 50
particle_sys_def *defs_list[max_particle_defs];

#ifndef ELC
Uint32	clean_file_name(Uint8 *dest, const Uint8 *src, Uint32 max_len)
{
	Uint32	len;
	Uint32	i;

	len=strlen(src);
	if(len >= max_len)len=max_len-1;
	for(i=0;i<len;i++)
		{
			if(src[i]=='\\')
				{
					dest[i]='/';
				}
			else
				{
					dest[i]=src[i];
				}
		}
	//always place a null that the end
	dest[len]='\0';
	return(len);
}
#endif

#define PARTICLE_DEF_VERSION 2
particle_sys_def *load_particle_def(const char *filename)
{
	int version=0,i;
	char cleanpath[128];
	FILE *f=NULL;
	particle_sys_def *def=NULL;

	clean_file_name(cleanpath,filename,128);

	//Check if it's already loaded
	for(i=0;i<max_particle_defs;i++)
		if(defs_list[i] && !strcmp(cleanpath,defs_list[i]->file_name))
			return defs_list[i];

	//Check if we have a free slot for it
	for(i=0;i<max_particle_defs;i++)
		if(!defs_list[i])
			{
				defs_list[i]=(particle_sys_def *)calloc(1,sizeof(particle_sys_def));
				def=defs_list[i];
				break;
			}
	if(!def)return NULL;

	f=fopen(cleanpath,"r");
	if(!f)
		{
			char str[120];
			sprintf(str,"Can't open %s",cleanpath);
			LogError(str);
			free(def);
			defs_list[i]=NULL;
			return NULL;
		}

	fscanf(f,"%i\n",&version);

	if(version!=PARTICLE_DEF_VERSION)
		{
			char str[256];
			snprintf(str,256,"Particle file %s version (%i) doesn't match file reader version (%i)!",filename,version,PARTICLE_DEF_VERSION);
			LogError(str);
			fclose(f);
			return NULL;
		}

	// System info
	strncpy(def->file_name,filename,79);
	def->file_name[79]=0;
	fscanf(f,"%i\n",&def->part_sys_type);
	fscanf(f,"%x,%x\n",&def->sblend,&def->dblend);
	fscanf(f,"%i\n",&def->total_particle_no);
	fscanf(f,"%i\n",&def->ttl);
	fscanf(f,"%i\n",&def->part_texture);
	fscanf(f,"%f\n",&def->part_size);
	fscanf(f,"%i\n",&def->random_func);
	// Particle creation info
	fscanf(f,"%f,%f,%f\n",&def->minx,&def->miny,&def->minz);
	fscanf(f,"%f,%f,%f\n",&def->maxx,&def->maxy,&def->maxz);
	fscanf(f,"%f\n",&def->constrain_rad_sq);
	fscanf(f,"%f,%f,%f\n",&def->vel_minx,&def->vel_miny,&def->vel_minz);
	fscanf(f,"%f,%f,%f\n",&def->vel_maxx,&def->vel_maxy,&def->vel_maxz);
	fscanf(f,"%f,%f,%f,%f\n",&def->minr,&def->ming,&def->minb,&def->mina);
	fscanf(f,"%f,%f,%f,%f\n",&def->maxr,&def->maxg,&def->maxb,&def->maxa);
	// Particle update info
	fscanf(f,"%f,%f,%f\n",&def->acc_minx,&def->acc_miny,&def->acc_minz);
	fscanf(f,"%f,%f,%f\n",&def->acc_maxx,&def->acc_maxy,&def->acc_maxz);
	fscanf(f,"%f,%f,%f,%f\n",
	       &def->mindr,&def->mindg,&def->mindb,&def->minda);
	fscanf(f,"%f,%f,%f,%f\n",
	       &def->maxdr,&def->maxdg,&def->maxdb,&def->maxda);

	if(def->total_particle_no>max_particles)
		{
		  char str[256];
		  snprintf(str,256,"Particle file %s tries to define %i particles, when %i is the maximum!",filename,def->total_particle_no,max_particles);
		  LogError(str);
		  def->total_particle_no=max_particles;
		}

	fclose(f);

	return def;
}

/*******************************************************************
 *            INITIALIZATION AND CLEANUP FUNCTIONS                 *
 *******************************************************************/
//Threading support for particles_list
void init_particles_list()
{
	int	i;

	particles_list_mutex=SDL_CreateMutex();
	lock_particles_list();	//lock it to avoid timing issues
	for(i=0;i<max_particle_systems;i++)particles_list[i]=0;
	for(i=0;i<max_particle_defs;i++)defs_list[i]=0;
	unlock_particles_list();	// release now that we are done
}

void end_particles_list()
{
	lock_particles_list();
	destroy_all_particles();
	destroy_all_particle_defs();
	unlock_particles_list();
	SDL_DestroyMutex(particles_list_mutex);
	particles_list_mutex=NULL;
}

void destroy_all_particle_defs()
{
	int i;
	for(i=0;i<max_particle_defs;i++)
		{
			free(defs_list[i]);
			defs_list[i]=NULL;
		}
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

/*********************************************************************
 *          CREATION OF NEW PARTICLES AND SYSTEMS                    *
 *********************************************************************/
int add_particle_sys(char *file_name,float x_pos,float y_pos,float z_pos)
{
	particle_sys_def *def=load_particle_def(file_name);
	if(!def)return -1;
	return create_particle_sys(def,x_pos,y_pos,z_pos);
}

int add_particle_sys_at_tile(char *file_name,int x_tile,int y_tile)
{
  return add_particle_sys(file_name,(float)x_tile/2.0+0.25f,(float)y_tile/2.0+0.25f,-2.2f+height_map[y_tile*tile_map_size_x*6+x_tile]*0.2f);
}


void create_particle(particle_sys *sys,particle *result)
{
	particle_sys_def *def=sys->def;
	if(def->random_func==0) {
		do {
			result->x=particle_random(def->minx,def->maxx);
			result->y=particle_random(def->miny,def->maxy);
			result->z=particle_random(def->minz,def->maxz);
		} while(def->constrain_rad_sq>0 && (result->x*result->x+result->y*result->y)>def->constrain_rad_sq);

		result->vx=particle_random(def->vel_minx,def->vel_maxx);
		result->vy=particle_random(def->vel_miny,def->vel_maxy);
		result->vz=particle_random(def->vel_minz,def->vel_maxz);

		result->r=particle_random(def->minr,def->maxr);
		result->g=particle_random(def->ming,def->maxg);
		result->b=particle_random(def->minb,def->maxb);
		result->a=particle_random(def->mina,def->maxa);
	} else {
		do {
			result->x=particle_random2(def->minx,def->maxx);
			result->y=particle_random2(def->miny,def->maxy);
			result->z=particle_random2(def->minz,def->maxz);
		} while(def->constrain_rad_sq>0 && (result->x*result->x+result->y*result->y)>def->constrain_rad_sq);

		result->vx=particle_random2(def->vel_minx,def->vel_maxx);
		result->vy=particle_random2(def->vel_miny,def->vel_maxy);
		result->vz=particle_random2(def->vel_minz,def->vel_maxz);

		result->r=particle_random2(def->minr,def->maxr);
		result->g=particle_random2(def->ming,def->maxg);
		result->b=particle_random2(def->minb,def->maxb);
		result->a=particle_random2(def->mina,def->maxa);
	}
	result->x+=sys->x_pos;
	result->y+=sys->y_pos;
	result->z+=sys->z_pos;

	result->free=0;
}

int create_particle_sys(particle_sys_def *def,float x,float y,float z)
{
	int	i,psys;
	particle_sys *system_id;

	if(!def)return -1;

	//allocate memory for this particle system
	system_id=(particle_sys *)calloc(1,sizeof(particle_sys));

	lock_particles_list();
	//now, find a place for this system
	for(psys=0;psys<max_particle_systems;psys++)
		{
			if(!particles_list[psys])
				{
					particles_list[psys]=system_id;
					break;
				}
		}
	if(psys==max_particle_systems)
		{
			free(system_id);
			unlock_particles_list();
			return -1;
		}

	system_id->x_pos=x;
	system_id->y_pos=y;
	system_id->z_pos=z;
	system_id->def=def;
	system_id->particle_count=def->total_particle_no;
	system_id->ttl=def->ttl;

	for(i=0;i<def->total_particle_no;i++)create_particle(system_id,&(system_id->particles[i]));
	unlock_particles_list();

	return psys;
}

/**********************************************************************
 *                      RENDERING FUNCTIONS                           *
 **********************************************************************/
void draw_text_particle_sys(particle_sys *system_id)
{
	float x_pos,y_pos,z_pos;
	int total_particle_no;
	int i;
	float x_len=0.065f*system_id->def->part_size;
	float z_len=x_len;

	x_pos=system_id->x_pos;
	y_pos=system_id->y_pos;
	z_pos=system_id->z_pos;
	total_particle_no=system_id->def->total_particle_no;

	get_and_set_texture_id(particle_textures[system_id->def->part_texture]);

	check_gl_errors();
	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef (x_pos, y_pos, z_pos);
	glRotatef(-rz, 0.0f, 0.0f, 1.0f);
	glTranslatef (-x_pos, -y_pos, -z_pos);
	check_gl_errors();
	lock_particles_list();	//lock it to avoid timing issues
	for(i=0;i<total_particle_no;i++)
	  {
		if(system_id->particles[i].free) continue;
		if(system_id->particles[i].z >= 0.0f)
			{
				float r,g,b,a,x,y,z;

				r=system_id->particles[i].r;
				g=system_id->particles[i].g;
				b=system_id->particles[i].b;
				a=system_id->particles[i].a;
				x=system_id->particles[i].x;
				y=system_id->particles[i].y;
				z=system_id->particles[i].z;

				glBegin(GL_TRIANGLE_STRIP);
				glColor4f(r,g,b,a);

				glTexCoord2f(0.0f,1.0f);
				glVertex3f(x-x_len,y,z-z_len);

				glTexCoord2f(0.0f,0.0f);
				glVertex3f(x-x_len,y,z+z_len);

				glTexCoord2f(1.0f,1.0f);
				glVertex3f(x+x_len,y,z-z_len);

				glTexCoord2f(1.0f,0.0f);
				glVertex3f(x+x_len,y,z+z_len);

				glEnd();

			}
	  }
	unlock_particles_list();	// release now that we are done
	check_gl_errors();

	glPopMatrix();
}

void draw_point_particle_sys(particle_sys *system_id)
{
#ifdef ELC
	float x_pos,y_pos,z_pos;
	int total_particle_no;
	int i;

	x_pos=system_id->x_pos;
	y_pos=system_id->y_pos;
	z_pos=system_id->z_pos;
	total_particle_no=system_id->def->total_particle_no;

	check_gl_errors();
	glPushMatrix();//we don't want to affect the rest of the scene
	glTranslatef (x_pos, y_pos, z_pos);
	glRotatef(-rz, 0.0f, 0.0f, 1.0f);
	glTranslatef (-x_pos, -y_pos, -z_pos);
	check_gl_errors();
	glEnable(GL_POINT_SPRITE_NV);
	glTexEnvf(GL_POINT_SPRITE_NV,GL_COORD_REPLACE_NV,GL_TRUE);
	glPointSize(system_id->def->part_size*(5.5f-zoom_level)*4.4f);
	get_and_set_texture_id(particle_textures[system_id->def->part_texture]);
#ifdef USE_VERTEX_ARRAYS
	if(use_vertex_array)
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			glVertexPointer(3,GL_FLOAT,sizeof(particle),&(system_id->particles[0].x));
			glColorPointer(3,GL_FLOAT,sizeof(particle),&(system_id->particles[0].r));
			lock_particles_list(); //lock it to avoid timing issues
			glDrawArrays(GL_POINTS,0,total_particle_no);
			unlock_particles_list();// release now that we are done
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);

		}
	else
#endif
 {
	glBegin(GL_POINTS);
	lock_particles_list();	//lock it to avoid timing issues
	for(i=0;i<total_particle_no;i++)
	  {
		if(system_id->particles[i].free) continue;
		if(system_id->particles[i].z >= 0.0f)
			{
				glColor4f(system_id->particles[i].r,system_id->particles[i].g,system_id->particles[i].b,system_id->particles[i].a);
				glVertex3f(system_id->particles[i].x,system_id->particles[i].y,system_id->particles[i].z);
			}
	  }
	unlock_particles_list();	// release now that we are done
	glEnd();
 }
	glDisable(GL_POINT_SPRITE_NV);
	check_gl_errors();

	glPopMatrix();
#endif
}

#ifndef ELC
int have_point_sprite=0;
#endif

void draw_particle_sys(particle_sys *system_id) {
	if(have_point_sprite)
		draw_point_particle_sys(system_id);
	else
		draw_text_particle_sys(system_id);
}

void display_particles()
{

	int i;
	int x,y;
	GLenum sblend=GL_SRC_ALPHA,dblend=GL_ONE;

	x=-cx;
	y=-cy;

	check_gl_errors();
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	check_gl_errors();

	glBlendFunc(sblend,dblend);
	glDisable(GL_CULL_FACE);

	check_gl_errors();
	lock_particles_list();
	// Perhaps we should have a depth sort here..?
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
						  if(particles_list[i]->def->sblend!=sblend || particles_list[i]->def->dblend!=dblend)
							{
								sblend=particles_list[i]->def->sblend;
								dblend=particles_list[i]->def->dblend;
								glBlendFunc(sblend,dblend);
							}
							draw_particle_sys(particles_list[i]);
						}
				}
		}
	unlock_particles_list();
	check_gl_errors();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

/******************************************************************************
 *                           UPDATE FUNCTIONS                                 *
 ******************************************************************************/
void update_fountain_sys(particle_sys *system_id) {
	int i,j;
	int particle_count;
	int total_particle_no;
	int particles_to_add=0,particles_to_add_per_frame=800;

	particle_count=system_id->particle_count;
	total_particle_no=system_id->def->total_particle_no;

	if(particle_count < system_id->def->total_particle_no)
		{
			particles_to_add=system_id->def->total_particle_no-particle_count;
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
							create_particle(system_id,&(system_id->particles[j]));
							system_id->particle_count++;
							break;
						}
				}

	//excellent, now we have to actually update the particles
	//find an used particle
	for(j=0;j<total_particle_no;j++)
		if(!system_id->particles[j].free)
			{
				if(system_id->particles[j].a<0.0f)
					{
						//poor particle, it died :(
						system_id->particles[j].free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				if(system_id->particles[j].z<0.0f)
					{
						system_id->particles[j].z=0.001f;
						system_id->particles[j].vz=-system_id->particles[j].vz;
					}
				system_id->particles[j].x+=system_id->particles[j].vx;
				system_id->particles[j].y+=system_id->particles[j].vy;
				system_id->particles[j].z+=system_id->particles[j].vz;
				system_id->particles[j].vx+=particle_random(system_id->def->acc_minx,system_id->def->acc_maxx);
				system_id->particles[j].vy+=particle_random(system_id->def->acc_miny,system_id->def->acc_maxy);
				system_id->particles[j].vz+=particle_random(system_id->def->acc_minz,system_id->def->acc_maxz);
				system_id->particles[j].r+=particle_random(system_id->def->mindr,system_id->def->maxdr);
				system_id->particles[j].g+=particle_random(system_id->def->mindg,system_id->def->maxdg);
				system_id->particles[j].b+=particle_random(system_id->def->mindb,system_id->def->maxdb);
				system_id->particles[j].a+=particle_random(system_id->def->minda,system_id->def->maxda);
			}
	unlock_particles_list();
}

void update_burst_sys(particle_sys *system_id)
{
	int j;
	int particle_count;
	int total_particle_no;

	particle_count=system_id->particle_count;
	total_particle_no=system_id->def->total_particle_no;

	lock_particles_list();

	//now we have to actually update the particles
	//find an used particle
	for(j=0;j<total_particle_no;j++)
		if(!system_id->particles[j].free)
			{
				float distx=system_id->particles[j].x-system_id->x_pos;
				float disty=system_id->particles[j].y-system_id->y_pos;
				float distz=system_id->particles[j].z-system_id->z_pos;
				float dist_sq=distx*distx+disty*disty+distz*distz;
				if(dist_sq>system_id->def->constrain_rad_sq*9.0 || dist_sq<0.01)
					{
						//poor particle, it died :(
						system_id->particles[j].free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				if(system_id->particles[j].vx>-0.01 &&
				   system_id->particles[j].vx<0.01 &&
				   system_id->particles[j].vy>-0.01 &&
				   system_id->particles[j].vy<0.01 &&
				   system_id->particles[j].vz>-0.01 &&
				   system_id->particles[j].vz<0.01)
					{
						float len=0.25/sqrt(distx*distx+disty*disty+distz*distz);
						system_id->particles[j].vx=distx*len;
						system_id->particles[j].vy=disty*len;
						system_id->particles[j].vz=distz*len;
					}			
				system_id->particles[j].x+=system_id->particles[j].vx;
				system_id->particles[j].y+=system_id->particles[j].vy;
				system_id->particles[j].z+=system_id->particles[j].vz;

				system_id->particles[j].r+=particle_random(system_id->def->mindr,system_id->def->maxdr);
				system_id->particles[j].g+=particle_random(system_id->def->mindg,system_id->def->maxdg);
				system_id->particles[j].b+=particle_random(system_id->def->mindb,system_id->def->maxdb);
				system_id->particles[j].a+=particle_random(system_id->def->minda,system_id->def->maxda);
			}
	unlock_particles_list();
}

void update_fire_sys(particle_sys *system_id)
{
	int i;
	int particle_count;
	int particles_to_add_per_frame=800;
	int particles_to_add=0;

	int j;

	particle_count=system_id->particle_count;

	if(system_id->ttl && (particle_count < system_id->def->total_particle_no))
		{
			particles_to_add=system_id->def->total_particle_no-particle_count;
			if(particles_to_add>particles_to_add_per_frame)
				particles_to_add=particles_to_add_per_frame;
		}
	//see if we need to add new particles
	lock_particles_list();

	particle_count=system_id->particle_count;
	if(particles_to_add)
		for(j=i=0;i<particles_to_add;i++)
			{
				//find a free space
				for(;j<system_id->def->total_particle_no;j++)
					if(system_id->particles[j].free)
						{
							//finally, we found a spot
							create_particle(system_id,&(system_id->particles[j]));
							if(system_id->particles[j].z<0)system_id->particles[j].z=0;
							//increase the particle count
							system_id->particle_count++;
							break;	//done looping
						}

			}

	//excellent, now we have to actually update the particles
	//find an used particle
	for(j=0;j<system_id->def->total_particle_no;j++)
		if(!system_id->particles[j].free)
			{
				if(system_id->particles[j].a<0.0f)
					{
						//poor particle, it died :(
						system_id->particles[j].free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				
				// Fires don't use acceleration as usual...
				system_id->particles[j].x+=system_id->particles[j].vx+particle_random(system_id->def->acc_minx,system_id->def->acc_maxx);
				system_id->particles[j].y+=system_id->particles[j].vy+particle_random(system_id->def->acc_miny,system_id->def->acc_maxy);
				system_id->particles[j].z+=system_id->particles[j].vz+particle_random(system_id->def->acc_minz,system_id->def->acc_maxz);

				system_id->particles[j].r+=particle_random(system_id->def->mindr,system_id->def->maxdr);
				system_id->particles[j].g+=particle_random(system_id->def->mindg,system_id->def->maxdg);
				system_id->particles[j].b+=particle_random(system_id->def->mindb,system_id->def->maxdb);
				system_id->particles[j].a+=particle_random(system_id->def->minda,system_id->def->maxda);

			}
	particle_count=system_id->particle_count;
	unlock_particles_list();

}

void update_teleporter_sys(particle_sys *system_id)
{
	int i;
	int particle_count;
	int particles_to_add_per_frame=800;
	int particles_to_add=0;

	int j;

	particle_count=system_id->particle_count;

	if(particle_count < system_id->def->total_particle_no)
		{
			particles_to_add=system_id->def->total_particle_no-particle_count;
			if(particles_to_add>particles_to_add_per_frame)
				particles_to_add=particles_to_add_per_frame;
		}
	//see if we need to add new particles
	lock_particles_list();
	if(particles_to_add)
		for(j=i=0;i<particles_to_add;i++)
			{
				//find a free space
				for(;j<system_id->def->total_particle_no;j++)
					if(system_id->particles[j].free)
						{
							//finally, we found a spot
							create_particle(system_id,&(system_id->particles[j]));
							if(system_id->particles[j].z<0)system_id->particles[j].z=0;
							//increase the particle count
							system_id->particle_count++;
							break;	//done looping
						}

			}

	//excellent, now we have to actually update the particles
	//find an used particle
	for(j=0;j<system_id->def->total_particle_no;j++)
		if(!system_id->particles[j].free)
			{
				if(system_id->particles[j].z>2.0f)
					{
						//poor particle, it died :(
						system_id->particles[j].free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				
				// Teleporters don't use acceleration as usual...
				system_id->particles[j].x+=system_id->particles[j].vx+particle_random2(system_id->def->acc_minx,system_id->def->acc_maxx);
				system_id->particles[j].y+=system_id->particles[j].vy+particle_random2(system_id->def->acc_miny,system_id->def->acc_maxy);
				system_id->particles[j].z+=system_id->particles[j].vz+particle_random2(system_id->def->acc_minz,system_id->def->acc_maxz);

				system_id->particles[j].r+=particle_random2(system_id->def->mindr,system_id->def->maxdr);
				system_id->particles[j].g+=particle_random2(system_id->def->mindg,system_id->def->maxdg);
				system_id->particles[j].b+=particle_random2(system_id->def->mindb,system_id->def->maxdb);
				system_id->particles[j].a+=particle_random2(system_id->def->minda,system_id->def->maxda);

			}
	unlock_particles_list();
}

void update_teleport_sys(particle_sys *system_id)
{
	int i;
	int particle_count;
	int total_particle_no=800;
	int particles_to_add_per_frame=800;
	int particles_to_add=0;

	int j;

	particle_count=system_id->particle_count;

	if(particle_count < system_id->def->total_particle_no)
		{
			particles_to_add=system_id->def->total_particle_no-particle_count;
			if(particles_to_add>particles_to_add_per_frame)
				particles_to_add=particles_to_add_per_frame;
		}
	//see if we need to add new particles
	lock_particles_list();
	if(system_id->ttl)
		if(particles_to_add)
			for(j=i=0;i<particles_to_add;i++)
				{
					//find a free space
					for(;j<total_particle_no;j++)
						if(system_id->particles[j].free)
							{
							create_particle(system_id,&(system_id->particles[j]));
							system_id->particles[j].x=0;
							system_id->particles[j].y=0;
							system_id->particles[j].z=0;
							system_id->particle_count++;

								break;
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

				// Teleports don't use acceleration as usual...
				system_id->particles[j].x+=system_id->particles[j].vx+particle_random2(system_id->def->acc_minx,system_id->def->acc_maxx);
				system_id->particles[j].y+=system_id->particles[j].vy+particle_random2(system_id->def->acc_miny,system_id->def->acc_maxy);
				system_id->particles[j].z+=system_id->particles[j].vz+particle_random2(system_id->def->acc_minz,system_id->def->acc_maxz);

				system_id->particles[j].r+=particle_random2(system_id->def->mindr,system_id->def->maxdr);
				system_id->particles[j].g+=particle_random2(system_id->def->mindg,system_id->def->maxdg);
				system_id->particles[j].b+=particle_random2(system_id->def->mindb,system_id->def->maxdb);
				system_id->particles[j].a+=particle_random2(system_id->def->minda,system_id->def->maxda);

			}
	unlock_particles_list();
}

void update_bag_part_sys(particle_sys *system_id)
{
	int i;
	int particle_count;
	int total_particle_no=300;
	int particles_to_add_per_frame=300;
	int particles_to_add=0;

	int j;

	particle_count=system_id->particle_count;

	if(particle_count < system_id->def->total_particle_no)
		{
			particles_to_add=system_id->def->total_particle_no-particle_count;
			if(particles_to_add>particles_to_add_per_frame)
				particles_to_add=particles_to_add_per_frame;
		}
	//see if we need to add new particles
	lock_particles_list();
	if(system_id->ttl)
		if(particles_to_add)
			for(j=i=0;i<particles_to_add;i++)
				{
					//find a free space
					for(;j<total_particle_no;j++)
						if(system_id->particles[j].free)
							{
								//finally, we found a spot
							create_particle(system_id,&(system_id->particles[j]));
							if(system_id->particles[j].z<0)system_id->particles[j].z=0;
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
				// Bags don't use acceleration as usual...
				system_id->particles[j].x+=system_id->particles[j].vx+particle_random2(system_id->def->acc_minx,system_id->def->acc_maxx);
				system_id->particles[j].y+=system_id->particles[j].vy+particle_random2(system_id->def->acc_miny,system_id->def->acc_maxy);
				system_id->particles[j].z+=system_id->particles[j].vz+particle_random2(system_id->def->acc_minz,system_id->def->acc_maxz);

				system_id->particles[j].r+=particle_random2(system_id->def->mindr,system_id->def->maxdr);
				system_id->particles[j].g+=particle_random2(system_id->def->mindg,system_id->def->maxdg);
				system_id->particles[j].b+=particle_random2(system_id->def->mindb,system_id->def->maxdb);
				system_id->particles[j].a+=particle_random2(system_id->def->minda,system_id->def->maxda);
			}
	unlock_particles_list();
}

void update_particles() {
	int i;
	lock_particles_list();
	for(i=0;i<max_particle_systems;i++)
		if(particles_list[i])
			{
			  switch(particles_list[i]->def->part_sys_type)
				{
				case(TELEPORTER_PARTICLE_SYS):
					update_teleporter_sys(particles_list[i]);
					break;
                     		case(TELEPORT_PARTICLE_SYS):
					update_teleport_sys(particles_list[i]);
					break;
				case(BAG_PARTICLE_SYS):
					update_bag_part_sys(particles_list[i]);
					break;
				case(BURST_PARTICLE_SYS):
					update_burst_sys(particles_list[i]);
					break;
				case(FIRE_PARTICLE_SYS):
					update_fire_sys(particles_list[i]);
					break;
				case(FOUNTAIN_PARTICLE_SYS):
					update_fountain_sys(particles_list[i]);
					break;
				}
			  if(particles_list[i]->ttl>0)particles_list[i]->ttl--;
			  if(!particles_list[i]->ttl && !particles_list[i]->particle_count)
			  //if there are no more particles to add, and the TTL expired, then kill this evil system
				{
					free(particles_list[i]);
					particles_list[i]=0;
				}
				
			}
	unlock_particles_list();
}

/******************************************************************************
 *                        MISC HELPER FUNCTIONS                               *
 ******************************************************************************/
#ifdef ELC
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
			add_sound_object(snd_teleprtr,teleport_x,teleport_y,1,1);
			//later on, maybe we want to have different visual types
			//now, get the Z position
			z=-2.2f+height_map[teleport_y*tile_map_size_x*6+teleport_x]*0.2f;
			//convert from height values to meters
			x=(float)teleport_x/2;
			y=(float)teleport_y/2;
			//center the object
			x=x+0.25f;
			y=y+0.25f;


			add_particle_sys("./particles/teleporter.part",x,y,z);
			add_e3d("./3dobjects/misc_objects/portal1.e3d",x,y,z,0,0,0,1,0,1.0f,1.0f,1.0f);

			//mark the teleporter as an unwalkable so that the pathfinder
			//won't try to plot a path through it
			pf_tile_map[teleport_y*tile_map_size_x*6+teleport_x].z = 0;
		}
	unlock_particles_list();

}

void dump_part_sys_info()
{
	char str[256];
	int i,partdefs=0,partsys=0;
	log_to_console(c_grey1,"-- PARTICLE SYSTEM DUMP --");
	if(have_point_sprite)
	  log_to_console(c_grey1,"Using point sprites");
	else
	  log_to_console(c_grey1,"Using textured quads");
	log_to_console(c_grey1,"Definitions:");
	for(i=0;i<max_particle_defs;i++)
		if(defs_list[i])
			{
				partdefs++;
				log_to_console(c_grey1,defs_list[i]->file_name);
			}
	sprintf(str,"#definitions: %i",partdefs);
	log_to_console(c_grey1,str);
	for(i=0;i<max_particle_systems;i++)
		if(particles_list[i])partsys++;
	sprintf(str,"#systems: %i",partsys);
	log_to_console(c_grey1,str);
}
#endif

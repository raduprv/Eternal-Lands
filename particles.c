#include <stdlib.h>
#include <math.h>
#include "SDL_opengl.h"
#include "global.h"
#include "string.h"

#define PART_SYS_VISIBLE_DIST_SQ 10*10

int particles_percentage=100;
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
			sprintf(str,"%s %s",cant_open_file,cleanpath);
			LogError(str);
			free(def);
			defs_list[i]=NULL;
			return NULL;
		}

	fscanf(f,"%i\n",&version);

	if(version!=PARTICLE_DEF_VERSION)
		{
			char str[256];
			snprintf(str,256,particles_filever_wrong,filename,version,PARTICLE_DEF_VERSION);
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
	def->total_particle_no*=(float)particles_percentage/100.0;
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
		  snprintf(str,256,particle_system_overrun,filename,def->total_particle_no,max_particles);
		  LogError(str);
		  def->total_particle_no=max_particles;
		}
	if(def->constrain_rad_sq>0.0)
		{
			float rad=sqrt(def->constrain_rad_sq);
			int fixed=def->minx>rad || def->maxx<-rad || def->miny>rad || def->maxy<-rad;
			if(def->minx>rad)def->minx=rad-0.1;
			if(def->maxx<-rad)def->maxx=-rad+0.1;
			if(def->miny>rad)def->miny=rad-0.1;
			if(def->maxy<-rad)def->maxy=-rad+0.1;
			if(def->minx*def->maxx>0.0 || def->miny*def->maxy>0.0)
				{
					float dist=def->minx*def->minx+def->miny*def->miny;
					if(dist>def->constrain_rad_sq)
						{
							def->minx*=sqrt(def->constrain_rad_sq/dist)-0.1;
							def->miny*=sqrt(def->constrain_rad_sq/dist)-0.1;
							fixed=1;
						}
					dist=def->minx*def->minx+def->maxy*def->maxy;
					if(dist>def->constrain_rad_sq)
						{
							def->minx*=sqrt(def->constrain_rad_sq/dist)-0.1;
							def->maxy*=sqrt(def->constrain_rad_sq/dist)-0.1;
							fixed=1;
						}
					dist=def->maxx*def->maxx+def->maxy*def->maxy;
					if(dist>def->constrain_rad_sq)
						{
							def->maxx*=sqrt(def->constrain_rad_sq/dist)-0.1;
							def->maxy*=sqrt(def->constrain_rad_sq/dist)-0.1;
							fixed=1;
						}
					dist=def->maxx*def->maxx+def->miny*def->miny;
					if(dist>def->constrain_rad_sq)
						{
							def->maxx*=sqrt(def->constrain_rad_sq/dist)-0.1;
							def->miny*=sqrt(def->constrain_rad_sq/dist)-0.1;
							fixed=1;
						}
				}
			if(fixed)
				{
					 char str[256];
					 snprintf(str,256,particle_strange_pos,filename);
					 LogError(str);
				}

		}


	fclose(f);

	return def;
}

int save_particle_def(particle_sys_def *def)
{
	char cleanpath[128];
	FILE *f=NULL;

	clean_file_name(cleanpath,def->file_name,128);

	f=fopen(cleanpath,"w");
	if(!f)
		{
			char str[120];
			sprintf(str,"%s %s",cant_open_file,cleanpath);
			LogError(str);
			return 0;
		}

	fprintf(f,"%i\n",PARTICLE_DEF_VERSION);

	// System info
	fprintf(f,"%i\n",def->part_sys_type);
	fprintf(f,"%x,%x\n",def->sblend,def->dblend);
	fprintf(f,"%i\n",def->total_particle_no);
	fprintf(f,"%i\n",def->ttl);
	fprintf(f,"%i\n",def->part_texture);
	fprintf(f,"%f\n",def->part_size);
	fprintf(f,"%i\n",def->random_func);
	// Particle creation info
	fprintf(f,"%f,%f,%f\n",def->minx,def->miny,def->minz);
	fprintf(f,"%f,%f,%f\n",def->maxx,def->maxy,def->maxz);
	fprintf(f,"%f\n",def->constrain_rad_sq);
	fprintf(f,"%f,%f,%f\n",def->vel_minx,def->vel_miny,def->vel_minz);
	fprintf(f,"%f,%f,%f\n",def->vel_maxx,def->vel_maxy,def->vel_maxz);
	fprintf(f,"%f,%f,%f,%f\n",def->minr,def->ming,def->minb,def->mina);
	fprintf(f,"%f,%f,%f,%f\n",def->maxr,def->maxg,def->maxb,def->maxa);
	// Particle update info
	fprintf(f,"%f,%f,%f\n",def->acc_minx,def->acc_miny,def->acc_minz);
	fprintf(f,"%f,%f,%f\n",def->acc_maxx,def->acc_maxy,def->acc_maxz);
	fprintf(f,"%f,%f,%f,%f\n",def->mindr,def->mindg,def->mindb,def->minda);
	fprintf(f,"%f,%f,%f,%f\n",def->maxdr,def->maxdg,def->maxdb,def->maxda);

	fclose(f);
	return 1;
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
	particle *p;

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

	for(i=0,p=&system_id->particles[0];i<def->total_particle_no;i++,p++)create_particle(system_id,p);
	unlock_particles_list();

	return psys;
}

/**********************************************************************
 *                      RENDERING FUNCTIONS                           *
 **********************************************************************/
void draw_text_particle_sys(particle_sys *system_id)
{
	float x_pos,y_pos,z_pos;
	int i;
	float z_len=0.065f*system_id->def->part_size;
	float x_len=z_len*cos(-rz*3.1415926/180.0);
	float y_len=z_len*sin(-rz*3.1415926/180.0);
	particle *p;

	lock_particles_list();	//lock it to avoid timing issues

	x_pos=system_id->x_pos;
	y_pos=system_id->y_pos;
	z_pos=system_id->z_pos;

	check_gl_errors();
	get_and_set_texture_id(particle_textures[system_id->def->part_texture]);

	for(i=0,p=&system_id->particles[0];i<system_id->def->total_particle_no;i++,p++)
		{
			if(!p->free)
				{
					glBegin(GL_TRIANGLE_STRIP);
					glColor4f(p->r,p->g,p->b,p->a);

					glTexCoord2f(0.0f,1.0f);
					glVertex3f(p->x-x_len,p->y-y_len,p->z+z_len);

					glTexCoord2f(0.0f,0.0f);
					glVertex3f(p->x-x_len,p->y-y_len,p->z-z_len);

					glTexCoord2f(1.0f,1.0f);
					glVertex3f(p->x+x_len,p->y+y_len,p->z+z_len);

					glTexCoord2f(1.0f,0.0f);
					glVertex3f(p->x+x_len,p->y+y_len,p->z-z_len);

					glEnd();
				}
		}
	unlock_particles_list();	// release now that we are done
	check_gl_errors();
}

void draw_point_particle_sys(particle_sys *system_id)
{
#ifdef ELC
	int i;
	particle *p;

	check_gl_errors();
	glEnable(GL_POINT_SPRITE_NV);
	glTexEnvf(GL_POINT_SPRITE_NV,GL_COORD_REPLACE_NV,GL_TRUE);
	glPointSize(system_id->def->part_size*(5.5f-zoom_level)*4.4f);
	get_and_set_texture_id(particle_textures[system_id->def->part_texture]);
#if 0
	//#ifdef USE_VERTEX_ARRAYS
	// This might be useful if we allow more particles per system.
	// It does, however, render free particles... 
	if(use_vertex_array)
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			lock_particles_list(); //lock it to avoid timing issues
			glVertexPointer(3,GL_FLOAT,sizeof(particle),&(system_id->particles[0].x));
			glColorPointer(4,GL_FLOAT,sizeof(particle),&(system_id->particles[0].r));
			glDrawArrays(GL_POINTS,0,system_id->total_particle_no);
			unlock_particles_list();// release now that we are done
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);

		}
	else
#endif
 {
	glBegin(GL_POINTS);
	lock_particles_list();	//lock it to avoid timing issues
	for(i=0,p=&system_id->particles[0];i<system_id->def->total_particle_no;i++,p++)
	  {
		if(!p->free)
			{
				glColor4f(p->r,p->g,p->b,p->a);
				glVertex3f(p->x,p->y,p->z);
			}
	  }
	unlock_particles_list();	// release now that we are done
	glEnd();
 }
	glDisable(GL_POINT_SPRITE_NV);
	check_gl_errors();
#endif
}

#ifndef ELC
int have_point_sprite=0;
#endif

void display_particles()
{

	int i;
	int x,y;
	GLenum sblend=GL_SRC_ALPHA,dblend=GL_ONE;

	if(!particles_percentage)
	  return;

	x=-cx;
	y=-cy;

	check_gl_errors();
	glPushAttrib(GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

	glBlendFunc(sblend,dblend);

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
					if(dist1*dist1+dist2*dist2<=PART_SYS_VISIBLE_DIST_SQ)
						{
							if(particles_list[i]->def->sblend!=sblend || particles_list[i]->def->dblend!=dblend)
								{
									sblend=particles_list[i]->def->sblend;
									dblend=particles_list[i]->def->dblend;
									glBlendFunc(sblend,dblend);
								}
							if(have_point_sprite)
								draw_point_particle_sys(particles_list[i]);
							else
								draw_text_particle_sys(particles_list[i]);
						}
				}
		}
	unlock_particles_list();
	glDisable(GL_CULL_FACE); //Intel fix
	glPopAttrib();
	check_gl_errors();
}

/******************************************************************************
 *                           UPDATE FUNCTIONS                                 *
 ******************************************************************************/
void update_fountain_sys(particle_sys *system_id) {
	int i,j;
	int total_particle_no;
	int particles_to_add=0;
	particle *p;

	total_particle_no=system_id->def->total_particle_no;

	//see if we need to add new particles
	lock_particles_list();
	if(system_id->ttl)
		particles_to_add=total_particle_no-system_id->particle_count;

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
	//find used particles
	for(j=0,p=&system_id->particles[0];j<total_particle_no;j++,p++)
		if(!p->free)
			{
				if(p->a<0.0f)
					{
						//poor particle, it died :(
						p->free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				if(p->z<0.0f)
					{
						p->z=0.001f;
						p->vz=-p->vz;
					}
				p->x+=p->vx;
				p->y+=p->vy;
				p->z+=p->vz;
				p->vx+=particle_random(system_id->def->acc_minx,system_id->def->acc_maxx);
				p->vy+=particle_random(system_id->def->acc_miny,system_id->def->acc_maxy);
				p->vz+=particle_random(system_id->def->acc_minz,system_id->def->acc_maxz);
				p->r+=particle_random(system_id->def->mindr,system_id->def->maxdr);
				p->g+=particle_random(system_id->def->mindg,system_id->def->maxdg);
				p->b+=particle_random(system_id->def->mindb,system_id->def->maxdb);
				p->a+=particle_random(system_id->def->minda,system_id->def->maxda);
			}
	unlock_particles_list();
}

void update_burst_sys(particle_sys *system_id)
{
	int j;
	int total_particle_no;
	particle *p;

	total_particle_no=system_id->def->total_particle_no;

	lock_particles_list();

	//find used particles
	for(j=0,p=&system_id->particles[0];j<total_particle_no;j++,p++)
		if(!p->free)
			{
				float distx=p->x-system_id->x_pos;
				float disty=p->y-system_id->y_pos;
				float distz=p->z-system_id->z_pos;
				float dist_sq=distx*distx+disty*disty+distz*distz;
				if(dist_sq>system_id->def->constrain_rad_sq*9.0 || dist_sq<0.01)
					{
						//poor particle, it died :(
						p->free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				if(p->vx>-0.01 && p->vx<0.01 &&
				   p->vy>-0.01 && p->vy<0.01 &&
				   p->vz>-0.01 && p->vz<0.01)
					{
						float len=0.25/sqrt(dist_sq);
						p->vx=distx*len;
						p->vy=disty*len;
						p->vz=distz*len;
					}			
				p->x+=p->vx;
				p->y+=p->vy;
				p->z+=p->vz;

				p->r+=particle_random(system_id->def->mindr,system_id->def->maxdr);
				p->g+=particle_random(system_id->def->mindg,system_id->def->maxdg);
				p->b+=particle_random(system_id->def->mindb,system_id->def->maxdb);
				p->a+=particle_random(system_id->def->minda,system_id->def->maxda);
			}
	unlock_particles_list();
}

void update_fire_sys(particle_sys *system_id)
{
	int i;
	int particles_to_add=0;
	int total_particle_no=system_id->def->total_particle_no;
	particle *p;
	int j;
	
	//see if we need to add new particles
	lock_particles_list();

	if(system_id->ttl)
		particles_to_add=total_particle_no-system_id->particle_count;

	for(j=i=0;i<particles_to_add;i++)
		{
			//find a free space
			for(;j<total_particle_no;j++)
				if(system_id->particles[j].free)
					{
						//finally, we found a spot
						create_particle(system_id,&(system_id->particles[j]));
						//increase the particle count
						system_id->particle_count++;
						j++;
						break;	//done looping
					}
			}

	//excellent, now we have to actually update the particles
	//find a used particle
	for(j=0,p=&system_id->particles[0];j<total_particle_no;j++,p++)
		if(!p->free)
			{
				if(p->a<0.0f)
					{
						//poor particle, it died :(
						p->free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				
				// Fires don't use acceleration as usual...
				p->x+=p->vx+particle_random(system_id->def->acc_minx,system_id->def->acc_maxx);
				p->y+=p->vy+particle_random(system_id->def->acc_miny,system_id->def->acc_maxy);
				p->z+=p->vz+particle_random(system_id->def->acc_minz,system_id->def->acc_maxz);

				p->r+=particle_random(system_id->def->mindr,system_id->def->maxdr);
				p->g+=particle_random(system_id->def->mindg,system_id->def->maxdg);
				p->b+=particle_random(system_id->def->mindb,system_id->def->maxdb);
				p->a+=particle_random(system_id->def->minda,system_id->def->maxda);

			}
	unlock_particles_list();
}

void update_teleporter_sys(particle_sys *system_id)
{
	int i;
	int particles_to_add=0;
	int total_particle_no=system_id->def->total_particle_no;
	particle *p;
	int j;

	//see if we need to add new particles
	lock_particles_list();

	if(system_id->ttl)
		particles_to_add=total_particle_no-system_id->particle_count;
	for(j=i=0;i<particles_to_add;i++)
		{
			//find a free space
			for(;j<total_particle_no;j++)
				if(system_id->particles[j].free)
					{
						//finally, we found a spot
						create_particle(system_id,&(system_id->particles[j]));
						if(system_id->particles[j].z<system_id->z_pos)system_id->particles[j].z=system_id->z_pos;
						//increase the particle count
						system_id->particle_count++;
						break;	//done looping
					}

		}

	//excellent, now we have to actually update the particles
	//find used particles
	for(j=0,p=&system_id->particles[0];j<total_particle_no;j++,p++)
		if(!p->free)
			{
				if(p->z>system_id->z_pos+2.0f)
					{
						//poor particle, it died :(
						p->free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				
				// Teleporters don't use acceleration as usual...
				p->x+=p->vx+particle_random2(system_id->def->acc_minx,system_id->def->acc_maxx);
				p->y+=p->vy+particle_random2(system_id->def->acc_miny,system_id->def->acc_maxy);
				p->z+=p->vz+particle_random2(system_id->def->acc_minz,system_id->def->acc_maxz);

				p->r+=particle_random2(system_id->def->mindr,system_id->def->maxdr);
				p->g+=particle_random2(system_id->def->mindg,system_id->def->maxdg);
				p->b+=particle_random2(system_id->def->mindb,system_id->def->maxdb);
				p->a+=particle_random2(system_id->def->minda,system_id->def->maxda);

			}
	unlock_particles_list();
}

void update_teleport_sys(particle_sys *system_id)
{
	int i;
	int total_particle_no=system_id->def->total_particle_no;
	int particles_to_add=0;
	particle *p;
	int j;

	//see if we need to add new particles
	lock_particles_list();
	if(system_id->ttl)
		particles_to_add=total_particle_no-system_id->particle_count;
	for(j=i=0;i<particles_to_add;i++)
		{
			//find a free space
			for(;j<total_particle_no;j++)
				if(system_id->particles[j].free)
					{
						create_particle(system_id,&(system_id->particles[j]));
						system_id->particles[j].x=system_id->x_pos;
						system_id->particles[j].y=system_id->y_pos;
						system_id->particles[j].z=system_id->z_pos;
						system_id->particle_count++;

						break;
					}

		}

	//excellent, now we have to actually update the particles
	//find used particles
	for(j=0,p=&system_id->particles[0];j<total_particle_no;j++,p++)
		if(!p->free)
			{
				if(p->z>system_id->z_pos+2.0f)
					{
						//poor particle, it died :(
						p->free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}

				// Teleports don't use acceleration as usual...
				p->x+=p->vx+particle_random2(system_id->def->acc_minx,system_id->def->acc_maxx);
				p->y+=p->vy+particle_random2(system_id->def->acc_miny,system_id->def->acc_maxy);
				p->z+=p->vz+particle_random2(system_id->def->acc_minz,system_id->def->acc_maxz);

				p->r+=particle_random2(system_id->def->mindr,system_id->def->maxdr);
				p->g+=particle_random2(system_id->def->mindg,system_id->def->maxdg);
				p->b+=particle_random2(system_id->def->mindb,system_id->def->maxdb);
				p->a+=particle_random2(system_id->def->minda,system_id->def->maxda);

			}
	unlock_particles_list();
}

void update_bag_part_sys(particle_sys *system_id)
{
	int i;
	int total_particle_no=system_id->def->total_particle_no;
	int particles_to_add=0;
	particle *p;
	int j;

	//see if we need to add new particles
	lock_particles_list();
	if(system_id->ttl)
		particles_to_add=total_particle_no-system_id->particle_count;
	for(j=i=0;i<particles_to_add;i++)
		{
			//find a free space
			for(;j<total_particle_no;j++)
				if(system_id->particles[j].free)
					{
						//finally, we found a spot
						create_particle(system_id,&(system_id->particles[j]));
						if(system_id->particles[j].z<system_id->z_pos)system_id->particles[j].z=system_id->z_pos;
						//increase the particle count
						system_id->particle_count++;
						break;	//done looping
					}
		}

	//excellent, now we have to actually update the particles
	//find used particles
	for(j=0,p=&system_id->particles[0];j<total_particle_no;j++,p++)
		if(!p->free)
			{
				if(p->z>system_id->z_pos+1.0f)
					{
						//poor particle, it died :(
						p->free=1;
						if(system_id->particle_count)system_id->particle_count--;
						continue;
					}
				// Bags don't use acceleration as usual...
				p->x+=p->vx+particle_random2(system_id->def->acc_minx,system_id->def->acc_maxx);
				p->y+=p->vy+particle_random2(system_id->def->acc_miny,system_id->def->acc_maxy);
				p->z+=p->vz+particle_random2(system_id->def->acc_minz,system_id->def->acc_maxz);

				p->r+=particle_random2(system_id->def->mindr,system_id->def->maxdr);
				p->g+=particle_random2(system_id->def->mindg,system_id->def->maxdg);
				p->b+=particle_random2(system_id->def->mindb,system_id->def->maxdb);
				p->a+=particle_random2(system_id->def->minda,system_id->def->maxda);
			}
	unlock_particles_list();
}

void update_particles() {
	int i,x=-cx,y=-cy;
	if(!particles_percentage)
	  return;
	lock_particles_list();
	for(i=0;i<max_particle_systems;i++)
		{
		if(particles_list[i])
			{
			int xdist=x-particles_list[i]->x_pos;
			int ydist=y-particles_list[i]->y_pos;
#ifdef ELC
			// Systems with a TTL need to be updated, even if they are far away
			// Though, if we're using the map editor we always want to update, otherwise the preview int the
			// particles window won't update correctly...
			if(particles_list[i]->ttl<0 && xdist*xdist+ydist*ydist>PART_SYS_VISIBLE_DIST_SQ)
				continue;
#endif
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
	log_to_console(c_grey1,particle_system_dump);
	if(!particles_percentage)
		{
			log_to_console(c_grey1,particles_disabled_str);
			return;
		}
	if(have_point_sprite)
	  log_to_console(c_grey1,point_sprites_enabled);
	else
	  log_to_console(c_grey1,using_textured_quads);
	log_to_console(c_grey1,definitions_str);
	for(i=0;i<max_particle_defs;i++)
		if(defs_list[i])
			{
				partdefs++;
				log_to_console(c_grey1,defs_list[i]->file_name);
			}
	sprintf(str,"#%s: %i",my_tolower(definitions_str),partdefs);
	log_to_console(c_grey1,str);
	for(i=0;i<max_particle_systems;i++)
		if(particles_list[i])partsys++;
	sprintf(str,"#%s: %i",part_sys_str,partsys);
	log_to_console(c_grey1,str);
	sprintf(str,"#%s: %i%%",part_part_str,particles_percentage);
	log_to_console(c_grey1,str);
}
#endif

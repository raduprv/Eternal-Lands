#include <stdlib.h>
#include <math.h>
#ifndef WINDOWS
#include <locale.h>
#endif
//#include "SDL_opengl.h"
#ifdef MAP_EDITOR
#include "../map_editor/global.h"
#else
#include "global.h"
#endif
#include "string.h"

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * void dump_part_sys_info();
 */

#define TELEPORTER_PARTICLE_SYS 0
#define TELEPORT_PARTICLE_SYS 1
#define BAG_PARTICLE_SYS 2
#define BURST_PARTICLE_SYS 3
#define FIRE_PARTICLE_SYS 4
#define FOUNTAIN_PARTICLE_SYS 5

#define PARTICLE_RANDOM(min,max) (min+(max-min)*(rand()/(float)RAND_MAX))
#define PARTICLE_RANDOM2(min,max) (min+0.5*(max-min)+0.5*(max-min)/(float)((rand()%200)-100+0.5))

#define PART_SYS_VISIBLE_DIST_SQ 18*18

int particles_percentage=100;
SDL_mutex *particles_list_mutex;	//used for locking between the timer and main threads
int particle_textures[8];
particle_sys *particles_list[MAX_PARTICLE_SYSTEMS];

/******************************************************
 *           PARTICLE SYSTEM DEFINITIONS              *
 ******************************************************/
#define MAX_PARTICLE_DEFS 500
particle_sys_def *defs_list[MAX_PARTICLE_DEFS];

void destroy_all_particle_defs();

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

int part_strcmp(char * s1, char *s2)
{
	while(*s1 && *s2)
		{
			if(*s1!=*s2 && (*s1!='/' && *s1!='\\' && *s2!='/' && *s2!='\\' )) return 1;
			s1++;s2++;
		}
	return *s1!=*s2;
}

// Grum: perhaps the addition of a sound definition to the files would warrant
// a version number update (from 2 to 3), but it'll still work with v. 2 system
// (without sound, of course), so I'll leave it
#define PARTICLE_DEF_VERSION 2

particle_sys_def *load_particle_def(const char *filename)
{
	int version=0,i;
	char cleanpath[128];
	FILE *f=NULL;
	particle_sys_def *def=NULL;

	clean_file_name(cleanpath,filename,128);

	//Check if it's already loaded
	for(i=0;i<MAX_PARTICLE_DEFS;i++)
		if(defs_list[i] && !part_strcmp(cleanpath,defs_list[i]->file_name))
			return defs_list[i];

	//Check if we have a free slot for it
	for(i=0;i<MAX_PARTICLE_DEFS;i++)
		if(!defs_list[i])
			{
				defs_list[i]=(particle_sys_def *)calloc(1,sizeof(particle_sys_def));
				def=defs_list[i];
				break;
			}
	if(!def)return NULL;

	f=my_fopen(cleanpath,"r");
	if(!f)
		{
			free(def);
			defs_list[i]=NULL;
			return NULL;
		}

	fscanf(f,"%i\n",&version);

	if(version!=PARTICLE_DEF_VERSION)
		{
			char str[256];
			snprintf(str,256,particles_filever_wrong,filename,version,PARTICLE_DEF_VERSION);
			LOG_ERROR(str);
			fclose(f);
			return NULL;
		}
#ifndef WINDOWS
	setlocale(LC_NUMERIC,"en_US");
#endif
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
	fscanf(f,"%i\n",&def->use_light);
	fscanf(f,"%f,%f,%f\n",&def->lightx,&def->lighty,&def->lightz);
	fscanf(f,"%f,%f,%f\n",&def->lightr,&def->lightg,&def->lightb);
#ifdef NEW_CLIENT
	fscanf (f, "%d,%d,%d\n", &def->sound_nr, &def->positional, &def->loop);
#endif
	
	if(def->total_particle_no>MAX_PARTICLES)
		{
		  char str[256];
		  snprintf(str,256,particle_system_overrun,filename,def->total_particle_no,MAX_PARTICLES);
		  LOG_ERROR(str);
		  def->total_particle_no=MAX_PARTICLES;
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
					 LOG_ERROR(str);
				}

		}
	
	fclose(f);

	return def;
}

#ifdef MAP_EDITOR
int save_particle_def(particle_sys_def *def)
{
	char cleanpath[128];
	FILE *f=NULL;

	clean_file_name ( cleanpath, def->file_name, sizeof (cleanpath) );

	f=my_fopen(cleanpath,"w");
	if(!f) return 0;

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
	// Particle light info
	fprintf(f,"%i\n",def->use_light);
	fprintf(f,"%f,%f,%f\n",def->lightx,def->lighty,def->lightz);
	fprintf(f,"%f,%f,%f\n",def->lightr,def->lightg,def->lightb);
#ifdef NEW_CLIENT
	fprintf (f, "%d,%d,%d\n", def->sound_nr, def->positional, def->loop);
#endif

	fclose(f);
	return 1;
}
#endif

/*******************************************************************
 *            INITIALIZATION AND CLEANUP FUNCTIONS                 *
 *******************************************************************/
//Threading support for particles_list
void init_particles_list()
{
	int	i;

	particles_list_mutex=SDL_CreateMutex();
	LOCK_PARTICLES_LIST();	//lock it to avoid timing issues
	for(i=0;i<MAX_PARTICLE_SYSTEMS;i++)particles_list[i]=0;
	for(i=0;i<MAX_PARTICLE_DEFS;i++)defs_list[i]=0;
	UNLOCK_PARTICLES_LIST();	// release now that we are done
}

void end_particles_list()
{
	LOCK_PARTICLES_LIST();
	destroy_all_particles();
	destroy_all_particle_defs();
	UNLOCK_PARTICLES_LIST();
	SDL_DestroyMutex(particles_list_mutex);
	particles_list_mutex=NULL;
}

void destroy_all_particle_defs()
{
	int i;
	for(i=0;i<MAX_PARTICLE_DEFS;i++)
		{
			free(defs_list[i]);
			defs_list[i]=NULL;
		}
}

void destroy_all_particles()
{
	int i;
	LOCK_PARTICLES_LIST();
	for(i=0;i<MAX_PARTICLE_SYSTEMS;i++)
		{
			if(!particles_list[i])continue;
			if(particles_list[i]->def && particles_list[i]->def->use_light && lights_list[particles_list[i]->light]) {
				free(lights_list[particles_list[i]->light]);
				lights_list[particles_list[i]->light]=NULL;
			}
			free(particles_list[i]);
			particles_list[i]=0;
		}
	UNLOCK_PARTICLES_LIST();

}

void add_fire_at_tile (int kind, Uint16 x_tile, Uint16 y_tile)
{
	float x = 0.5f * x_tile + 0.25f;
	float y = 0.5f * y_tile + 0.25f;
	float z = 0.0;

	switch (kind)
	{
		case 2:
#ifdef NEW_CLIENT
			add_particle_sys ("./particles/fire_big.part", x, y, z);
#else
			add_particle_sys ("./particles/fire_big.part", x, y, z, snd_fire, 1, 1);
#endif
			break;
		case 1:
		default:
#ifdef NEW_CLIENT
			add_particle_sys ("./particles/fire_small.part", x, y, z);
#else
			add_particle_sys ("./particles/fire_small.part", x, y, z, snd_fire, 1, 1);
#endif
	}
}

void remove_fire_at_tile (Uint16 x_tile, Uint16 y_tile)
{
	float x = 0.5f * x_tile + 0.25f;
	float y = 0.5f * y_tile + 0.25f;
	int i;
	particle_sys *sys;
	
	for (i = 0; i < MAX_PARTICLE_SYSTEMS; i++)
	{
		sys = particles_list[i];
		if (particles_list[i] && strncmp (sys->def->file_name, "./particles/fire_", 17) == 0 && sys->x_pos == x && sys->y_pos == y)
		{
			if (sys->def->use_light && lights_list[sys->light])
			{
				free (lights_list[sys->light]);
				lights_list[sys->light] = NULL;
			}
#ifndef MAP_EDITOR
			if (sys->sound != 0)
				remove_sound_object (sys->sound);
#endif
			free (sys);
			particles_list[i] = NULL;
		}
	}
}

/*********************************************************************
 *          CREATION OF NEW PARTICLES AND SYSTEMS                    *
 *********************************************************************/
#ifdef NEW_CLIENT
int add_particle_sys (char *file_name, float x_pos, float y_pos, float z_pos)
{
	particle_sys_def *def = load_particle_def(file_name);
	if (!def) return -1;

	return create_particle_sys (def, x_pos, y_pos, z_pos);
}
#else
int add_particle_sys (char *file_name, float x_pos, float y_pos, float z_pos, int sound, int positional, int loop)
{
	particle_sys_def *def=load_particle_def(file_name);
	if(!def)return -1;

	return create_particle_sys (def, x_pos, y_pos, z_pos, sound, positional, loop);
}
#endif

#ifdef NEW_CLIENT
int add_particle_sys_at_tile (char *file_name, int x_tile, int y_tile)
{
	return add_particle_sys (file_name, (float) x_tile / 2.0 + 0.25f, (float) y_tile / 2.0 + 0.25f, -2.2f + height_map[y_tile*tile_map_size_x*6+x_tile] * 0.2f);
}
#else
int add_particle_sys_at_tile (char *file_name, int x_tile, int y_tile, int sound, int positional, int loop)
{
	return add_particle_sys (file_name, (float)x_tile/2.0+0.25f, (float)y_tile/2.0+0.25f, -2.2f+height_map[y_tile*tile_map_size_x*6+x_tile]*0.2f, sound, positional, loop);
}
#endif

void create_particle(particle_sys *sys,particle *result)
{
	particle_sys_def *def=sys->def;
	if(def->random_func==0) {
		do {
			result->x=PARTICLE_RANDOM(def->minx,def->maxx);
			result->y=PARTICLE_RANDOM(def->miny,def->maxy);
			result->z=PARTICLE_RANDOM(def->minz,def->maxz);
		} while(def->constrain_rad_sq>0 && (result->x*result->x+result->y*result->y)>def->constrain_rad_sq);

		result->vx=PARTICLE_RANDOM(def->vel_minx,def->vel_maxx);
		result->vy=PARTICLE_RANDOM(def->vel_miny,def->vel_maxy);
		result->vz=PARTICLE_RANDOM(def->vel_minz,def->vel_maxz);

		result->r=PARTICLE_RANDOM(def->minr,def->maxr);
		result->g=PARTICLE_RANDOM(def->ming,def->maxg);
		result->b=PARTICLE_RANDOM(def->minb,def->maxb);
		result->a=PARTICLE_RANDOM(def->mina,def->maxa);
	} else {
		do {
			result->x=PARTICLE_RANDOM2(def->minx,def->maxx);
			result->y=PARTICLE_RANDOM2(def->miny,def->maxy);
			result->z=PARTICLE_RANDOM2(def->minz,def->maxz);
		} while(def->constrain_rad_sq>0 && (result->x*result->x+result->y*result->y)>def->constrain_rad_sq);

		result->vx=PARTICLE_RANDOM2(def->vel_minx,def->vel_maxx);
		result->vy=PARTICLE_RANDOM2(def->vel_miny,def->vel_maxy);
		result->vz=PARTICLE_RANDOM2(def->vel_minz,def->vel_maxz);

		result->r=PARTICLE_RANDOM2(def->minr,def->maxr);
		result->g=PARTICLE_RANDOM2(def->ming,def->maxg);
		result->b=PARTICLE_RANDOM2(def->minb,def->maxb);
		result->a=PARTICLE_RANDOM2(def->mina,def->maxa);
	}
	result->x+=sys->x_pos;
	result->y+=sys->y_pos;
	result->z+=sys->z_pos;

	result->free=0;
}

#ifdef NEW_CLIENT
int create_particle_sys (particle_sys_def *def, float x, float y, float z)
#else
int create_particle_sys (particle_sys_def *def, float x, float y, float z, int sound, int positional, int loop)
#endif
{
	int	i,psys;
	particle_sys *system_id;
	particle *p;

	if(!def)return -1;

	//allocate memory for this particle system
	system_id=(particle_sys *)calloc(1,sizeof(particle_sys));

	LOCK_PARTICLES_LIST();
	//now, find a place for this system
	for(psys=0;psys<MAX_PARTICLE_SYSTEMS;psys++)
		{
			if(!particles_list[psys])
				{
					particles_list[psys]=system_id;
					break;
				}
		}
	if(psys==MAX_PARTICLE_SYSTEMS)
		{
			free(system_id);
			UNLOCK_PARTICLES_LIST();
			return -1;
		}

	system_id->x_pos=x;
	system_id->y_pos=y;
	system_id->z_pos=z;
	system_id->def=def;
	system_id->particle_count=def->total_particle_no;
	system_id->ttl=def->ttl;

	if(def->use_light) {
#ifdef MAP_EDITOR
		system_id->light=add_light(def->lightx+x, def->lighty+y, def->lightz+z, def->lightr, def->lightg, def->lightb,1.0f,1);
#else
		system_id->light=add_light(def->lightx+x, def->lighty+y, def->lightz+z, def->lightr, def->lightg, def->lightb,1.0f);
#endif
	}

	for(i=0,p=&system_id->particles[0];i<def->total_particle_no;i++,p++)create_particle(system_id,p);
	
#ifndef MAP_EDITOR
#ifdef NEW_CLIENT
	if (def->sound_nr < 0 || no_sound)
		system_id->sound = 0;
	else
		system_id->sound = add_sound_object (def->sound_nr, (int)(x+x-0.5), (int)(y+y-0.5), def->positional, def->loop);
#endif
#endif

	UNLOCK_PARTICLES_LIST();

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

	LOCK_PARTICLES_LIST();	//lock it to avoid timing issues

	x_pos=system_id->x_pos;
	y_pos=system_id->y_pos;
	z_pos=system_id->z_pos;

	CHECK_GL_ERRORS();
	get_and_set_texture_id(particle_textures[system_id->def->part_texture]);

	for(i=0,p=&system_id->particles[0];i<system_id->def->total_particle_no;i=i+5,p=p+5)
		{
			if(!p->free)
				{
					glPushMatrix();
					glTranslatef(p->x,p->y,p->z);
					glScalef((1-p->a*0.4)*7,(1-p->a*0.4)*7,(1-p->a*0.4)*7);
					glRotatef(p->a*1000.0,0,1,0);
					glBegin(GL_TRIANGLE_STRIP);
					glColor4f(p->r,p->g,p->b,p->a);

					glTexCoord2f(0.0f,1.0f);
					glVertex3f(-x_len,-y_len,+z_len);

					glTexCoord2f(0.0f,0.0f);
					glVertex3f(-x_len,-y_len,-z_len);

					glTexCoord2f(1.0f,1.0f);
					glVertex3f(x_len,y_len,+z_len);

					glTexCoord2f(1.0f,0.0f);
					glVertex3f(x_len,y_len,-z_len);

					glEnd();
					glPopMatrix();
				}
		}
	UNLOCK_PARTICLES_LIST();	// release now that we are done
	CHECK_GL_ERRORS();
}

void draw_point_particle_sys(particle_sys *system_id)
{
#ifdef ELC
	int i;
	particle *p;

	CHECK_GL_ERRORS();
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
			LOCK_PARTICLES_LIST(); //lock it to avoid timing issues
			glVertexPointer(3,GL_FLOAT,sizeof(particle),&(system_id->particles[0].x));
			glColorPointer(4,GL_FLOAT,sizeof(particle),&(system_id->particles[0].r));
			glDrawArrays(GL_POINTS,0,system_id->total_particle_no);
			UNLOCK_PARTICLES_LIST();// release now that we are done
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);

		}
	else
#endif
 {
	glBegin(GL_POINTS);
	LOCK_PARTICLES_LIST();	//lock it to avoid timing issues
	for(i=0,p=&system_id->particles[0];i<system_id->def->total_particle_no;i++,p++)
	  {
		if(!p->free)
			{
				glColor4f(p->r,p->g,p->b,p->a);
				glVertex3f(p->x,p->y,p->z);
			}
	  }
	UNLOCK_PARTICLES_LIST();	// release now that we are done
	glEnd();
 }
	glDisable(GL_POINT_SPRITE_NV);
	CHECK_GL_ERRORS();
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

	CHECK_GL_ERRORS();
	glPushAttrib(GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

	glBlendFunc(sblend,dblend);

	LOCK_PARTICLES_LIST();
	// Perhaps we should have a depth sort here..?
	for(i=0;i<MAX_PARTICLE_SYSTEMS;i++)
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
	UNLOCK_PARTICLES_LIST();
	glDisable(GL_CULL_FACE); //Intel fix
	glPopAttrib();
	CHECK_GL_ERRORS();
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
	LOCK_PARTICLES_LIST();
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
				p->vx+=PARTICLE_RANDOM(system_id->def->acc_minx,system_id->def->acc_maxx);
				p->vy+=PARTICLE_RANDOM(system_id->def->acc_miny,system_id->def->acc_maxy);
				p->vz+=PARTICLE_RANDOM(system_id->def->acc_minz,system_id->def->acc_maxz);
				p->r+=PARTICLE_RANDOM(system_id->def->mindr,system_id->def->maxdr);
				p->g+=PARTICLE_RANDOM(system_id->def->mindg,system_id->def->maxdg);
				p->b+=PARTICLE_RANDOM(system_id->def->mindb,system_id->def->maxdb);
				p->a+=PARTICLE_RANDOM(system_id->def->minda,system_id->def->maxda);
			}
	UNLOCK_PARTICLES_LIST();
}

void update_burst_sys(particle_sys *system_id)
{
	int j;
	int total_particle_no;
	particle *p;

	total_particle_no=system_id->def->total_particle_no;

	LOCK_PARTICLES_LIST();

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

				p->r+=PARTICLE_RANDOM(system_id->def->mindr,system_id->def->maxdr);
				p->g+=PARTICLE_RANDOM(system_id->def->mindg,system_id->def->maxdg);
				p->b+=PARTICLE_RANDOM(system_id->def->mindb,system_id->def->maxdb);
				p->a+=PARTICLE_RANDOM(system_id->def->minda,system_id->def->maxda);
			}
	UNLOCK_PARTICLES_LIST();
}

void update_fire_sys(particle_sys *system_id)
{
	int i;
	int particles_to_add=0;
	int total_particle_no=system_id->def->total_particle_no;
	particle *p;
	int j;
	
	//see if we need to add new particles
	LOCK_PARTICLES_LIST();

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
				p->x+=p->vx+PARTICLE_RANDOM(system_id->def->acc_minx,system_id->def->acc_maxx);
				p->y+=p->vy+PARTICLE_RANDOM(system_id->def->acc_miny,system_id->def->acc_maxy);
				p->z+=p->vz+PARTICLE_RANDOM(system_id->def->acc_minz,system_id->def->acc_maxz);

				p->r+=PARTICLE_RANDOM(system_id->def->mindr,system_id->def->maxdr);
				p->g+=PARTICLE_RANDOM(system_id->def->mindg,system_id->def->maxdg);
				p->b+=PARTICLE_RANDOM(system_id->def->mindb,system_id->def->maxdb);
				p->a+=PARTICLE_RANDOM(system_id->def->minda,system_id->def->maxda);

			}
	UNLOCK_PARTICLES_LIST();
}

void update_teleporter_sys(particle_sys *system_id)
{
	int i;
	int particles_to_add=0;
	int total_particle_no=system_id->def->total_particle_no;
	particle *p;
	int j;

	//see if we need to add new particles
	LOCK_PARTICLES_LIST();

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
				p->x+=p->vx+PARTICLE_RANDOM2(system_id->def->acc_minx,system_id->def->acc_maxx);
				p->y+=p->vy+PARTICLE_RANDOM2(system_id->def->acc_miny,system_id->def->acc_maxy);
				p->z+=p->vz+PARTICLE_RANDOM2(system_id->def->acc_minz,system_id->def->acc_maxz);

				p->r+=PARTICLE_RANDOM2(system_id->def->mindr,system_id->def->maxdr);
				p->g+=PARTICLE_RANDOM2(system_id->def->mindg,system_id->def->maxdg);
				p->b+=PARTICLE_RANDOM2(system_id->def->mindb,system_id->def->maxdb);
				p->a+=PARTICLE_RANDOM2(system_id->def->minda,system_id->def->maxda);

			}
	UNLOCK_PARTICLES_LIST();
}

void update_teleport_sys(particle_sys *system_id)
{
	int i;
	int total_particle_no=system_id->def->total_particle_no;
	int particles_to_add=0;
	particle *p;
	int j;

	//see if we need to add new particles
	LOCK_PARTICLES_LIST();
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
				p->x+=p->vx+PARTICLE_RANDOM2(system_id->def->acc_minx,system_id->def->acc_maxx);
				p->y+=p->vy+PARTICLE_RANDOM2(system_id->def->acc_miny,system_id->def->acc_maxy);
				p->z+=p->vz+PARTICLE_RANDOM2(system_id->def->acc_minz,system_id->def->acc_maxz);

				p->r+=PARTICLE_RANDOM2(system_id->def->mindr,system_id->def->maxdr);
				p->g+=PARTICLE_RANDOM2(system_id->def->mindg,system_id->def->maxdg);
				p->b+=PARTICLE_RANDOM2(system_id->def->mindb,system_id->def->maxdb);
				p->a+=PARTICLE_RANDOM2(system_id->def->minda,system_id->def->maxda);

			}
	UNLOCK_PARTICLES_LIST();
}

void update_bag_part_sys(particle_sys *system_id)
{
	int i;
	int total_particle_no=system_id->def->total_particle_no;
	int particles_to_add=0;
	particle *p;
	int j;

	//see if we need to add new particles
	LOCK_PARTICLES_LIST();
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
				p->x+=p->vx+PARTICLE_RANDOM2(system_id->def->acc_minx,system_id->def->acc_maxx);
				p->y+=p->vy+PARTICLE_RANDOM2(system_id->def->acc_miny,system_id->def->acc_maxy);
				p->z+=p->vz+PARTICLE_RANDOM2(system_id->def->acc_minz,system_id->def->acc_maxz);

				p->r+=PARTICLE_RANDOM2(system_id->def->mindr,system_id->def->maxdr);
				p->g+=PARTICLE_RANDOM2(system_id->def->mindg,system_id->def->maxdg);
				p->b+=PARTICLE_RANDOM2(system_id->def->mindb,system_id->def->maxdb);
				p->a+=PARTICLE_RANDOM2(system_id->def->minda,system_id->def->maxda);
			}
	UNLOCK_PARTICLES_LIST();
}

void update_particles() {
	int i;
#ifdef ELC
	int x = -cx, y = -cy;
#endif

	if(!particles_percentage)
	  return;
	LOCK_PARTICLES_LIST();
	for(i=0;i<MAX_PARTICLE_SYSTEMS;i++)
		{
		if(particles_list[i])
			{
#ifdef ELC
			int xdist=x-particles_list[i]->x_pos;
			int ydist=y-particles_list[i]->y_pos;
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
					if(particles_list[i]->def->use_light && lights_list[particles_list[i]->light]) {
						free(lights_list[particles_list[i]->light]);
						lights_list[particles_list[i]->light]=NULL;
					}
					free(particles_list[i]);
					particles_list[i]=0;
				}
				
			}
		}
	UNLOCK_PARTICLES_LIST();
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

	teleporters_no=SDL_SwapLE16(*((Uint16 *)(teleport_list)));
	LOCK_PARTICLES_LIST();	//lock it to avoid timing issues
	for(i=0;i<teleporters_no;i++)
		{
			my_offset=i*5+2;
			teleport_x=SDL_SwapLE16(*((Uint16 *)(teleport_list+my_offset)));
			teleport_y=SDL_SwapLE16(*((Uint16 *)(teleport_list+my_offset+2)));
			teleport_type=SDL_SwapLE16(*((Uint16 *)(teleport_list+my_offset+4)));
						
			//later on, maybe we want to have different visual types
			//now, get the Z position
			if(teleport_y*tile_map_size_x*6+teleport_x<tile_map_size_y*6*tile_map_size_x*6)
				z=-2.2f+height_map[teleport_y*tile_map_size_x*6+teleport_x]*0.2f;
			else continue;
			//convert from height values to meters
			x=(float)teleport_x/2;
			y=(float)teleport_y/2;
			//center the object
			x=x+0.25f;
			y=y+0.25f;

#ifdef NEW_CLIENT
			add_particle_sys ("./particles/teleporter.part", x, y, z);
#else
			add_particle_sys ("./particles/teleporter.part", x, y, z, snd_teleprtr, 1, 1);
#endif
			sector_add_3do(add_e3d("./3dobjects/misc_objects/portal1.e3d",x,y,z,0,0,0,1,0,1.0f,1.0f,1.0f));

			//mark the teleporter as an unwalkable so that the pathfinder
			//won't try to plot a path through it
			pf_tile_map[teleport_y*tile_map_size_x*6+teleport_x].z = 0;
		}
	UNLOCK_PARTICLES_LIST();

}

/* currently UNUSED
void dump_part_sys_info()
{
	char str[256];
	int i,partdefs=0,partsys=0;
	LOG_TO_CONSOLE(c_grey1,particle_system_dump);
	if(!particles_percentage)
		{
			LOG_TO_CONSOLE(c_grey1,particles_disabled_str);
			return;
		}
	if(have_point_sprite)
	  LOG_TO_CONSOLE(c_grey1,point_sprites_enabled);
	else
	  LOG_TO_CONSOLE(c_grey1,using_textured_quads);
	LOG_TO_CONSOLE(c_grey1,definitions_str);
	for(i=0;i<MAX_PARTICLE_DEFS;i++)
		if(defs_list[i])
			{
				partdefs++;
				LOG_TO_CONSOLE(c_grey1,defs_list[i]->file_name);
			}
	sprintf(str,"#%s: %i",my_tolower(definitions_str),partdefs);
	LOG_TO_CONSOLE(c_grey1,str);
	for(i=0;i<MAX_PARTICLE_SYSTEMS;i++)
		if(particles_list[i])partsys++;
	sprintf(str,"#%s: %i",part_sys_str,partsys);
	LOG_TO_CONSOLE(c_grey1,str);
	sprintf(str,"#%s: %i%%",part_part_str,particles_percentage);
	LOG_TO_CONSOLE(c_grey1,str);
}
*/
#endif

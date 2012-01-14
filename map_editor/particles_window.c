#include "../asc.h"
#include "global.h"

int particles_window = -1;
int view_particles_window=0;
static int particles_window_x=15;
static int particles_window_y=50;
static int particles_window_x_len=600;
static int particles_window_y_len=470;

particle_sys_def def;
static int part_sys=-1;
static GLfloat def_light_position[4];
static GLfloat def_light_diffuse[4];
const GLfloat def_light_spot_direction[]={0.0f,0.0f,0.0f};

void reset_def()
{
	def.file_name[0]=0;
	def.part_sys_type=4;
	def.sblend=GL_ONE;
	def.dblend=GL_ONE;
	def.total_particle_no=600;
	def.ttl=-1;
	def.part_texture=0;
	def.part_size=1.0;
	def.random_func=0;
	def.minx=def.miny=def.minz=-0.5;
	def.maxx=def.maxy=def.maxz=0.5;
	def.constrain_rad_sq=-1.0;
	def.vel_minx=def.vel_miny=def.vel_minz=0.0;
	def.vel_maxx=def.vel_maxy=def.vel_maxz=0.0;
	def.minr=def.ming=def.minb=def.mina=1.0;
	def.maxr=def.maxg=def.maxb=def.maxa=1.0;
	def.acc_minx=def.acc_miny=def.acc_minz=0.0;
	def.acc_maxx=def.acc_maxy=def.acc_maxz=0.0;
	def.mindr=def.mindg=def.mindb=def.minda=-0.02;
	def.maxdr=def.maxdg=def.maxdb=def.maxda=-0.02;
	def.use_light=0;
	def.lightx=def.lighty=def.lightz=0;
	def.lightr=def.lightg=def.lightb=0;
	
	def.sound_nr = -1;
#ifdef OLD_SOUND
	def.positional = 0;
	def.loop = 0;
#endif //OLD_SOUND
}

void set_and_draw_particle_lights(){
}

void check_particle_sys_alive()
{
	// If we've selected a new particle system, copy its definition and kill the current system
	if(selected_particles_object>=0 && particles_list[selected_particles_object])
		{
			memcpy(&def,particles_list[selected_particles_object]->def,sizeof(particle_sys_def));
			selected_particles_object=-1;
			if(part_sys>=0 && particles_list[part_sys])
				{
					particles_list[part_sys]->ttl=0;
					particles_list[part_sys]->particle_count=0;				      
				}
			part_sys=-1;
		}
	// If the current system has died, recreate it.
	if(part_sys<0 || !particles_list[part_sys])
	{
		part_sys=create_particle_sys(&def,-666.0,-666.0,1.0);
	}
}

void create_particles_window ()
{
	if (particles_window < 0)
	{
		particles_window = create_window ("particles", 0, 0, particles_window_x, particles_window_y, particles_window_x_len, particles_window_y_len, ELW_WIN_DEFAULT & ~ELW_SHOW);

		set_window_handler (particles_window, ELW_HANDLER_DISPLAY, &display_particles_window_handler);
		set_window_handler (particles_window, ELW_HANDLER_CLICK, &check_particles_window_interface);
		reset_def();
	}
}

void toggle_particles_window()
{
	if(part_sys>=0 && particles_list[part_sys])
		{
			particles_list[part_sys]->ttl=0;
			particles_list[part_sys]->particle_count=0;				      
		}
	part_sys=-1;
	view_particles_window=!view_particles_window;
}

char *get_blend_func_string(int func)
{
	switch(func)
		{
		case(0):
			return "ZERO";
		case(1):
			return "ONE";
		case(0x0300):
			return "SRC_COLOR";
		case(0x0301):
			return "ONE_MINUS_SRC_COLOR";
		case(0x0302):
			return "SRC_ALPHA";
		case(0x0303):
			return "ONE_MINUS_SRC_ALPHA";
		case(0x0306):
			return "DST_COLOR";
		case(0x0308):
			return "ONE_MINUS_DST_COLOR";
		default:
			return "error";
		}
}

static int src_blend_funcs[]={0,1,0x0302,0x0303,0x0306,0x0308};
static int dst_blend_funcs[]={0,1,0x0300,0x0301,0x0302,0x0303};

int get_next_src_blend_func(int func)
{
	int i;
	for(i=0;i<6;i++)
		if(src_blend_funcs[i]==func)break;
	if(i==6)return src_blend_funcs[0];
	return src_blend_funcs[(i+1)%6];
}

int get_next_dst_blend_func(int func)
{
	int i;
	for(i=0;i<6;i++)
		if(dst_blend_funcs[i]==func)break;
	if(i==6)return dst_blend_funcs[0];
	return dst_blend_funcs[(i+1)%6];
}


static int pm_width=40,pm_height=20;
void display_plus_minus(int x,int y)
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINE_LOOP);
		glVertex2i(x,y);
		glVertex2i(x,y+pm_height);
		glVertex2i(x+pm_width,y+pm_height);
		glVertex2i(x+pm_width,y);
	glEnd();
	glBegin(GL_LINES);
		glVertex2i(x+pm_width/2,y);
		glVertex2i(x+pm_width/2,y+pm_height);
	glEnd();

	glEnable(GL_TEXTURE_2D);
	draw_string(x+2,y,(const unsigned char*) "+ -",1);
	glPopAttrib();
}

int check_plus_minus_hit(int x,int y,int mousex,int mousey)
{
	if(mousex<x || mousex>(x+pm_width) || mousey<y || mousey>(y+pm_height))
		return 0;
	if(mousex<x+pm_width/2)return 1;
	return 2;
}


#define GLOBAL_TOP 8
static int systypex=8,systypey=GLOBAL_TOP,systypex2=228;
static int blendx=8,blendy=GLOBAL_TOP+40,blendx2=268,blendy2=GLOBAL_TOP+60,blendy3=GLOBAL_TOP+80;
static int particlenox=8,particlenoy=GLOBAL_TOP+80,particlenox2=228;
static int ttlx=8,ttly=GLOBAL_TOP+100,ttlx2=95;
static int randx=140,randy=GLOBAL_TOP+100,randx2=228;
static int sizex=8,sizey=GLOBAL_TOP+140,sizex2=228;

static int previewx=278,previewy=8,previewx2=578,previewy2=308;
static int sel_handle_bottom=328;

#define COLOR_TOP 168
static int colorx=8,colory=COLOR_TOP,colorx2=28,colorx3=148,colorx4=268;
static int colorry=COLOR_TOP+20,colorgy=COLOR_TOP+40,colorby=COLOR_TOP+60,coloray=COLOR_TOP+80;

#define PREVIEW_PARTICLE_TEXTURE 0
#define PREVIEW_PARTICLE_STARTPOS 1
#define PREVIEW_PARTICLE_CONSTRAINT 2
#define PREVIEW_PARTICLE_STARTVEL 3
#define PREVIEW_PARTICLE_ACC 4
#define PREVIEW_PARTICLE_LIGHT 5
#define PREVIEW_PARTICLE_SOUND 6

static int preview_display_particle_handles=0;

void draw_velocity(float x,float y,float z,float x2,float y2,float z2)
{
	glColor3f(1.0,0.0,0.0);
	glBegin(GL_LINES);
		glVertex3f(x,y,z);
		glVertex3f(x2,y2,z2);
	glEnd();
	glBegin(GL_POINTS);
		glVertex3f(x2,y2,z2);
	glEnd();
}

static float preview_zoom=1.5;
void display_particles_window_preview(window_info *win)
{
	float save_cx=camera_x,save_cy=camera_y,save_cz=camera_z;
	float save_rx=rx;
	int save_view_particles=view_particles;
	int viewx=win->pos_x+previewx+1;
	int vieww=previewx2-previewx-2;
	int viewy=window_height-(win->pos_y+previewy2)+1;
	int viewh=previewy2-previewy-2;

	camera_x=-particles_list[part_sys]->x_pos;
	camera_y=-particles_list[part_sys]->y_pos;
	camera_z=-particles_list[part_sys]->z_pos;
	rx=-75.0;

	view_particles=1;

	glPushAttrib(GL_VIEWPORT_BIT|GL_SCISSOR_BIT|GL_CURRENT_BIT|GL_ENABLE_BIT|GL_POINT_BIT);
	glEnable(GL_SCISSOR_TEST);
	glViewport(viewx,viewy,vieww,viewh);
	glScissor(viewx,viewy,vieww,viewh);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0*preview_zoom,1.0*preview_zoom,-1.0*preview_zoom,1.0*preview_zoom,-40.0,40.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glPushMatrix();
 	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0,0.0, camera_z);
	glEnable(GL_LIGHTING);
	if(def.use_light){
		draw_dungeon_light();
		glEnable(GL_LIGHT0);
		def_light_position[0]=def.lightx;def_light_position[1]=def.lighty;def_light_position[2]=def.lightz;def_light_position[3]=1.0f;
		def_light_diffuse[0]=def.lightr;def_light_diffuse[1]=def.lightg;def_light_diffuse[2]=def.lightb;def_light_diffuse[3]=1.0f;
		glLightfv(GL_LIGHT0, GL_POSITION, def_light_position);
		glLightfv(GL_LIGHT0,GL_DIFFUSE, def_light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, def_light_spot_direction);
	}
	glDepthMask(GL_TRUE);
	// Draw a few tiles as a background
#ifdef	NEW_TEXTURES
	bind_texture(tile_list[1]);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(tile_list[1]);
#endif	/* NEW_TEXTURES */
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(-3.0,3.0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(-3.0,0.0);
	glTexCoord2f(0, 0.0f);
	glVertex2f(0.0,3.0);
	glTexCoord2f(0, 1.0f);
	glVertex2f(0.0,0.0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(3.0,3.0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(3.0,0.0);
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(-3.0,0.0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(-3.0,-3.0);
	glTexCoord2f(0, 0.0f);
	glVertex2f(0.0,0.0);
	glTexCoord2f(0, 1.0f);
	glVertex2f(0.0,-3.0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(3.0,0.0);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(3.0,-3.0);
	glEnd();
	if(def.use_light)glDisable(GL_LIGHT0);
	glPopMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	Move();
	display_particles();
	glDisable(GL_TEXTURE_2D);
	glLoadIdentity();
 	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glPointSize(2.0);
	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_XOR);
	switch(preview_display_particle_handles)
		{
		case(PREVIEW_PARTICLE_CONSTRAINT):
		case(PREVIEW_PARTICLE_STARTPOS):
			glColor3f(1.0,1.0,1.0);
			if(def.constrain_rad_sq>0.0001)
				{
					float angle,radius=sqrt(def.constrain_rad_sq);
					glBegin(GL_LINE_LOOP);
					for(angle=0.0;angle<6.25;angle+=0.5)
						{
							float x=radius*cos(angle),y=radius*sin(angle);
							if(x<def.minx)x=def.minx;
							else if(x>def.maxx)x=def.maxx;
							if(y<def.miny)y=def.miny;
							else if(y>def.maxy)y=def.maxy;
							glVertex3f(x,y,def.minz);
						}
					glEnd();
					glBegin(GL_LINE_LOOP);
					for(angle=0.0;angle<6.25;angle+=0.5)
						{
							float x=radius*cos(angle),y=radius*sin(angle);
							if(x<def.minx)x=def.minx;
							else if(x>def.maxx)x=def.maxx;
							if(y<def.miny)y=def.miny;
							else if(y>def.maxy)y=def.maxy;
							glVertex3f(x,y,def.maxz);
						}
					glEnd();
				}
			else
				{			
					glBegin(GL_LINE_LOOP);
						glVertex3f(def.minx,def.miny,def.minz);
						glVertex3f(def.minx,def.maxy,def.minz);
						glVertex3f(def.maxx,def.maxy,def.minz);
						glVertex3f(def.maxx,def.miny,def.minz);
					glEnd();
					glBegin(GL_LINE_LOOP);
						glVertex3f(def.minx,def.miny,def.maxz);
						glVertex3f(def.minx,def.maxy,def.maxz);
						glVertex3f(def.maxx,def.maxy,def.maxz);
						glVertex3f(def.maxx,def.miny,def.maxz);
					glEnd();
				}
			break;
		case(PREVIEW_PARTICLE_STARTVEL):
			draw_velocity(-0.1,0.0,0.0,10.0*def.vel_minx-0.1,10.0*def.vel_miny,10.0*def.vel_minz);
			draw_velocity(0.1,0.0,0.0,10.0*def.vel_maxx+0.1,10.0*def.vel_maxy,10.0*def.vel_maxz);
			break;
		case(PREVIEW_PARTICLE_ACC):
			draw_velocity(-0.1,0.0,0.0,10.0*def.acc_minx-0.1,10.0*def.acc_miny,10.0*def.acc_minz);
			draw_velocity(0.1,0.0,0.0,10.0*def.acc_maxx+0.1,10.0*def.acc_maxy,10.0*def.acc_maxz);
			break;
		case(PREVIEW_PARTICLE_LIGHT):
			draw_velocity(0.0,0.0,0.0,def.lightx,def.lighty,def.lightz);
		}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();

	camera_x=save_cx;
	camera_y=save_cy;
	camera_z=save_cz;
	rx=save_rx;
	view_particles=save_view_particles;
}

int display_particles_window_handler(window_info *win)
{
	char temp[100];
	char *preview_display_handle_strings[]={"Texture","Start position","Constraint","Start velocity","Acceleration","Lights", "Sound"};

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.77f,0.57f,0.39f);

	glBegin(GL_LINE_LOOP);
		glVertex2i(blendx,blendy);
		glVertex2i(blendx,blendy3);
		glVertex2i(blendx2,blendy3);
		glVertex2i(blendx2,blendy);
	glEnd();
	glBegin(GL_LINES);
		glVertex2i(blendx,blendy2);
		glVertex2i(blendx2,blendy2);
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex2i(previewx,previewy);
		glVertex2i(previewx,previewy2);
		glVertex2i(previewx2,previewy2);
		glVertex2i(previewx2,previewy);
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex2i(previewx,previewy2);
		glVertex2i(previewx,sel_handle_bottom);
		glVertex2i(previewx2,sel_handle_bottom);
		glVertex2i(previewx2,previewy2);
	glEnd();

	glBegin(GL_TRIANGLE_STRIP);
		glColor4f(def.minr,def.ming,def.minb,def.mina);
		glVertex2i(colorx2+2,coloray+22);
		glVertex2i(colorx2+2,coloray+38);
		glVertex2i(colorx3-2,coloray+22);
		glVertex2i(colorx3-2,coloray+38);
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
		glColor4f(def.maxr,def.maxg,def.maxb,def.maxa);
		glVertex2i(colorx3+2,coloray+22);
		glVertex2i(colorx3+2,coloray+38);
		glVertex2i(colorx4-2,coloray+22);
		glVertex2i(colorx4-2,coloray+38);
	glEnd();

	glColor3f(0.77f,0.57f,0.39f);
	glEnable(GL_TEXTURE_2D);

	LOCK_PARTICLES_LIST();
	check_particle_sys_alive();

	display_particles_window_preview(win);

	snprintf(temp,99,"System type: %i",def.part_sys_type);
	draw_string(systypex+2,systypey+2,(const unsigned char*) temp,1);
	display_plus_minus(systypex2,systypey);

	draw_string(blendx+2,blendy-18,(const unsigned char*) "Blend modes",1);
	snprintf(temp,99,"S: %s",get_blend_func_string(def.sblend));
	draw_string(blendx+2,blendy+2,(const unsigned char*) temp,1);
	snprintf(temp,99,"D: %s",get_blend_func_string(def.dblend));
	draw_string(blendx+2,blendy2+2,(const unsigned char*) temp,1);

	snprintf(temp,99,"#particles: %i",def.total_particle_no);
	draw_string(particlenox+2,particlenoy+2,(const unsigned char*) temp,1);
	display_plus_minus(particlenox2,particlenoy);

	snprintf(temp,99,"TTL: %i",def.ttl);
	draw_string(ttlx+2,ttly+2,(const unsigned char*) temp,1);
	display_plus_minus(ttlx2,ttly);

	snprintf(temp,99,"RND: %i",def.random_func);
	draw_string(randx+2,randy+2,(const unsigned char*) temp,1);
	display_plus_minus(randx2,randy);

	snprintf(temp,99,"Particle size: %.1f",def.part_size);
	draw_string(sizex+2,sizey+2,(const unsigned char*) temp,1);
	display_plus_minus(sizex2,sizey);

	draw_string(colorx2+2,colory+2,(const unsigned char*) "min",1);
	draw_string(colorx3+2,colory+2,(const unsigned char*) "max",1);
	draw_string(colorx+2,colorry+2,(const unsigned char*) "R",1);
	draw_string(colorx+2,colorgy+2,(const unsigned char*) "G",1);
	draw_string(colorx+2,colorby+2,(const unsigned char*) "B",1);
	draw_string(colorx+2,coloray+2,(const unsigned char*) "A",1);
	snprintf(temp,99,"%.2f",def.minr);
	draw_string(colorx2+12,colorry+2,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.ming);
	draw_string(colorx2+12,colorgy+2,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.minb);
	draw_string(colorx2+12,colorby+2,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.mina);
	draw_string(colorx2+12,coloray+2,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.maxr);
	draw_string(colorx3+12,colorry+2,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.maxg);
	draw_string(colorx3+12,colorgy+2,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.maxb);
	draw_string(colorx3+12,colorby+2,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.maxa);
	draw_string(colorx3+12,coloray+2,(const unsigned char*) temp,1);
	display_plus_minus(colorx3-pm_width,colorry);
	display_plus_minus(colorx3-pm_width,colorgy);
	display_plus_minus(colorx3-pm_width,colorby);
	display_plus_minus(colorx3-pm_width,coloray);
	display_plus_minus(colorx4-pm_width,colorry);
	display_plus_minus(colorx4-pm_width,colorgy);
	display_plus_minus(colorx4-pm_width,colorby);
	display_plus_minus(colorx4-pm_width,coloray);

	draw_string(colorx+2,colorry+112,(const unsigned char*) "dR",1);
	draw_string(colorx+2,colorgy+112,(const unsigned char*) "dG",1);
	draw_string(colorx+2,colorby+112,(const unsigned char*) "dB",1);
	draw_string(colorx+2,coloray+112,(const unsigned char*) "dA",1);
	snprintf(temp,99,"%.2f",def.mindr);
	draw_string(colorx2+12,colorry+112,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.mindg);
	draw_string(colorx2+12,colorgy+112,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.mindb);
	draw_string(colorx2+12,colorby+112,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.minda);
	draw_string(colorx2+12,coloray+112,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.maxdr);
	draw_string(colorx3+12,colorry+112,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.maxdg);
	draw_string(colorx3+12,colorgy+112,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.maxdb);
	draw_string(colorx3+12,colorby+112,(const unsigned char*) temp,1);
	snprintf(temp,99,"%.2f",def.maxda);
	draw_string(colorx3+12,coloray+112,(const unsigned char*) temp,1);
	display_plus_minus(colorx3-pm_width,colorry+110);
	display_plus_minus(colorx3-pm_width,colorgy+110);
	display_plus_minus(colorx3-pm_width,colorby+110);
	display_plus_minus(colorx3-pm_width,coloray+110);
	display_plus_minus(colorx4-pm_width,colorry+110);
	display_plus_minus(colorx4-pm_width,colorgy+110);
	display_plus_minus(colorx4-pm_width,colorby+110);
	display_plus_minus(colorx4-pm_width,coloray+110);

	snprintf(temp,99,"Display: %s",preview_display_handle_strings[preview_display_particle_handles]);
	draw_string(previewx+2,previewy2+2,(const unsigned char*) temp,1);

	switch(preview_display_particle_handles)
		{
		case(PREVIEW_PARTICLE_STARTPOS):
			draw_string(previewx+2,sel_handle_bottom+2,(const unsigned char*) "Min",1);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+2,(const unsigned char*) "Max",1);
			snprintf(temp,99,"x: %.2f",def.minx);
			draw_string(previewx+2,sel_handle_bottom+32,(const unsigned char*) temp,1);
			snprintf(temp,99,"y: %.2f",def.miny);
			draw_string(previewx+2,sel_handle_bottom+52,(const unsigned char*) temp,1);
			snprintf(temp,99,"z: %.2f",def.minz);
			draw_string(previewx+2,sel_handle_bottom+72,(const unsigned char*) temp,1);
			snprintf(temp,99,"x: %.2f",def.maxx);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+32,(const unsigned char*) temp,1);
			snprintf(temp,99,"y: %.2f",def.maxy);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+52,(const unsigned char*) temp,1);
			snprintf(temp,99,"z: %.2f",def.maxz);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+72,(const unsigned char*) temp,1);
			break;
		case(PREVIEW_PARTICLE_CONSTRAINT):
			if(def.constrain_rad_sq<0.0)
				draw_string(previewx+2,sel_handle_bottom+32,(const unsigned char*) "No radius constraint",1);
			else
				{
					snprintf(temp,99,"Actual radius: %.3f",sqrt(def.constrain_rad_sq));
					draw_string(previewx+2,sel_handle_bottom+2,(const unsigned char*) temp,1);
					snprintf(temp,99,"Squared radius: %.3f",def.constrain_rad_sq);
					draw_string(previewx+2,sel_handle_bottom+32,(const unsigned char*) temp,1);
				}
			break;
		case(PREVIEW_PARTICLE_STARTVEL):
			draw_string(previewx+2,sel_handle_bottom+2,(const unsigned char*) "Min",1);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+2,(const unsigned char*) "Max",1);
			snprintf(temp,99,"x: %.2f",def.vel_minx);
			draw_string(previewx+2,sel_handle_bottom+32,(const unsigned char*) temp,1);
			snprintf(temp,99,"y: %.2f",def.vel_miny);
			draw_string(previewx+2,sel_handle_bottom+52,(const unsigned char*) temp,1);
			snprintf(temp,99,"z: %.2f",def.vel_minz);
			draw_string(previewx+2,sel_handle_bottom+72,(const unsigned char*) temp,1);
			snprintf(temp,99,"x: %.2f",def.vel_maxx);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+32,(const unsigned char*) temp,1);
			snprintf(temp,99,"y: %.2f",def.vel_maxy);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+52,(const unsigned char*) temp,1);
			snprintf(temp,99,"z: %.2f",def.vel_maxz);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+72,(const unsigned char*) temp,1);
			break;
		case(PREVIEW_PARTICLE_ACC):
			draw_string(previewx+2,sel_handle_bottom+2,(const unsigned char*) "Min",1);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+2,(const unsigned char*) "Max",1);
			snprintf(temp,99,"x: %.2f",def.acc_minx);
			draw_string(previewx+2,sel_handle_bottom+32,(const unsigned char*) temp,1);
			snprintf(temp,99,"y: %.2f",def.acc_miny);
			draw_string(previewx+2,sel_handle_bottom+52,(const unsigned char*) temp,1);
			snprintf(temp,99,"z: %.2f",def.acc_minz);
			draw_string(previewx+2,sel_handle_bottom+72,(const unsigned char*) temp,1);
			snprintf(temp,99,"x: %.2f",def.acc_maxx);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+32,(const unsigned char*) temp,1);
			snprintf(temp,99,"y: %.2f",def.acc_maxy);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+52,(const unsigned char*) temp,1);
			snprintf(temp,99,"z: %.2f",def.acc_maxz);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+72,(const unsigned char*) temp,1);
			break;
		case(PREVIEW_PARTICLE_TEXTURE):
			get_and_set_particle_texture_id (def.part_texture);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ZERO);
			glBegin(GL_TRIANGLE_STRIP);
				glColor3f(1.0,1.0,1.0);
				glTexCoord2f(0.0,0.0);
				glVertex2i(previewx+(previewx2-previewx-60)/2,sel_handle_bottom+21);
				glTexCoord2f(0.0,1.0);
				glVertex2i(previewx+(previewx2-previewx-60)/2,sel_handle_bottom+81);
				glTexCoord2f(1.0,0.0);
				glVertex2i(previewx2-(previewx2-previewx-60)/2,sel_handle_bottom+21);
				glTexCoord2f(1.0,1.0);
				glVertex2i(previewx2-(previewx2-previewx-60)/2,sel_handle_bottom+81);
			glEnd();
			glDisable(GL_BLEND);
			break;
		case(PREVIEW_PARTICLE_LIGHT):
			draw_string(previewx+2,sel_handle_bottom+4,(const unsigned char*) "Use lights: ",1);
			snprintf(temp,99,"x: %.2f",def.lightx);
			draw_string(previewx+2,sel_handle_bottom+32,(const unsigned char*) temp,1);
			snprintf(temp,99,"y: %.2f",def.lighty);
			draw_string(previewx+2,sel_handle_bottom+52,(const unsigned char*) temp,1);
			snprintf(temp,99,"z: %.2f",def.lightz);
			draw_string(previewx+2,sel_handle_bottom+72,(const unsigned char*) temp,1);
			snprintf(temp,99,"r: %.2f",def.lightr);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+32,(const unsigned char*) temp,1);
			snprintf(temp,99,"g: %.2f",def.lightg);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+52,(const unsigned char*) temp,1);
			snprintf(temp,99,"b: %.2f",def.lightb);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+72,(const unsigned char*) temp,1);
			break;
		case PREVIEW_PARTICLE_SOUND:
			snprintf (temp, sizeof (temp), "Sound number: %d", def.sound_nr);
			draw_string (previewx + 2, sel_handle_bottom + 32,(const unsigned char*)  temp, 1);
#ifdef OLD_SOUND
			draw_string (previewx + 2, sel_handle_bottom + 57,(const unsigned char*)  "Positional: ", 1);
			draw_string (previewx + 2, sel_handle_bottom + 77,(const unsigned char*)  "Loops: ", 1);
#endif //OLD_SOUND
			break;
		}
	switch(preview_display_particle_handles)
	{
		case PREVIEW_PARTICLE_SOUND:
			display_plus_minus (previewx2-pm_width, sel_handle_bottom + 30);
#ifdef OLD_SOUND
			draw_checkbox (previewx2-pm_width, sel_handle_bottom+55, def.positional);
			draw_checkbox (previewx2-pm_width, sel_handle_bottom+75, def.loop);
#endif //OLD_SOUND
			break;
		case(PREVIEW_PARTICLE_LIGHT):
			draw_checkbox(previewx+11*12-2,sel_handle_bottom+4,def.use_light);
		case(PREVIEW_PARTICLE_STARTPOS):
		case(PREVIEW_PARTICLE_STARTVEL):
		case(PREVIEW_PARTICLE_ACC):
			display_plus_minus(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+30);
			display_plus_minus(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+50);
			display_plus_minus(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+70);
			display_plus_minus(previewx2-pm_width,sel_handle_bottom+50);
			display_plus_minus(previewx2-pm_width,sel_handle_bottom+70);
		default:
			display_plus_minus(previewx2-pm_width,sel_handle_bottom+30);
	}

	glColor3f(1.0f,1.0f,1.0f);
	snprintf(temp,99,"System info: TTL==%i, #particles==%i",particles_list[part_sys]->ttl,particles_list[part_sys]->particle_count);
	draw_string(10,450,(const unsigned char*) temp,1);
	UNLOCK_PARTICLES_LIST();

#ifdef	NEW_TEXTURES
	bind_texture(buttons_text);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(buttons_text);
#endif	/* NEW_TEXTURES */
	glBegin(GL_QUADS);
       	draw_2d_thing((float)64/255,1.0f-(float)32/255,(float)96/255,1.0f-(float)64/255, 10,380,42,412);
	glEnd();

	return 1;
}

int check_particles_window_interface(window_info *win, int _x, int _y)
{
	int tmp,i;
	int minx,miny,minz,maxx,maxy,maxz;
	float incr=0.01;

	// Grum: should this happen ?
	// Grum: no it shouldn't
   	if (_x > win->len_x-20 && _y <= 20)
	{
		toggle_particles_window();
		return 1;
	}

	// XXX (Grum): what's wrong with using the flags!?
	if(shift_on/*flags&ELW_SHIFT*/)incr=0.1;
	if(ctrl_on/*flags&ELW_CTRL*/)incr=1.0;

	LOCK_PARTICLES_LIST();
	check_particle_sys_alive();

	tmp = check_plus_minus_hit (systypex2, systypey, _x, _y);
	if(tmp==1 && def.part_sys_type<5)def.part_sys_type++;
	else if(tmp==2 && def.part_sys_type>0)def.part_sys_type--;

	if(_x>blendx && _x<blendx2 && _y>blendy && _y<blendy3)
		{
			if(_y<blendy2)def.sblend=get_next_src_blend_func(def.sblend);
			else def.dblend=get_next_dst_blend_func(def.dblend);
		}

	tmp=check_plus_minus_hit(particlenox2,particlenoy,_x,_y);
	if(tmp==1 && def.total_particle_no<MAX_PARTICLES)
		{
			// If we add particles to an existing system, we must make sure they are free
			for(i=def.total_particle_no;i<MAX_PARTICLES;i++)particles_list[part_sys]->particles[i].free=1;
			def.total_particle_no+=50;
		}
	else if(tmp==2 && def.total_particle_no>0)def.total_particle_no-=50;

	tmp=check_plus_minus_hit(ttlx2,ttly,_x,_y);
	if(tmp==1)
		{
			if(def.ttl>0)def.ttl++;
			else def.ttl=1;
			particles_list[part_sys]->ttl=def.ttl;
		}
	else if(tmp==2 && def.ttl>-1)
		{
			if(def.ttl>1)def.ttl--;
			else def.ttl=-1;
			particles_list[part_sys]->ttl=def.ttl;
		}

	tmp=check_plus_minus_hit(randx2,randy,_x,_y);
	if(tmp==1)def.random_func=1;
	else if(tmp==2)def.random_func=0;

	tmp=check_plus_minus_hit(sizex2,sizey,_x,_y);
	if(tmp==1)def.part_size+=0.1;
	else if(tmp==2 && def.part_size>0.1)def.part_size-=0.1;

	tmp=check_plus_minus_hit(colorx3-pm_width,colorry,_x,_y);
	if(tmp==1)def.minr+=incr;
	else if(tmp==2)def.minr-=incr;
	tmp=check_plus_minus_hit(colorx3-pm_width,colorgy,_x,_y);
	if(tmp==1)def.ming+=incr;
	else if(tmp==2)def.ming-=incr;
	tmp=check_plus_minus_hit(colorx3-pm_width,colorby,_x,_y);
	if(tmp==1)def.minb+=incr;
	else if(tmp==2)def.minb-=incr;
	tmp=check_plus_minus_hit(colorx3-pm_width,coloray,_x,_y);
	if(tmp==1)def.mina+=incr;
	else if(tmp==2)def.mina-=incr;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorry,_x,_y);
	if(tmp==1)def.maxr+=incr;
	else if(tmp==2)def.maxr-=incr;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorgy,_x,_y);
	if(tmp==1)def.maxg+=incr;
	else if(tmp==2)def.maxg-=incr;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorby,_x,_y);
	if(tmp==1)def.maxb+=incr;
	else if(tmp==2)def.maxb-=incr;
	tmp=check_plus_minus_hit(colorx4-pm_width,coloray,_x,_y);
	if(tmp==1)def.maxa+=incr;
	else if(tmp==2)def.maxa-=incr;

	tmp=check_plus_minus_hit(colorx3-pm_width,colorry+110,_x,_y);
	if(tmp==1)def.mindr+=incr;
	else if(tmp==2)def.mindr-=incr;
	tmp=check_plus_minus_hit(colorx3-pm_width,colorgy+110,_x,_y);
	if(tmp==1)def.mindg+=incr;
	else if(tmp==2)def.mindg-=incr;
	tmp=check_plus_minus_hit(colorx3-pm_width,colorby+110,_x,_y);
	if(tmp==1)def.mindb+=incr;
	else if(tmp==2)def.mindb-=incr;
	tmp=check_plus_minus_hit(colorx3-pm_width,coloray+110,_x,_y);
	if(tmp==1)def.minda+=incr;
	else if(tmp==2)def.minda-=incr;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorry+110,_x,_y);
	if(tmp==1)def.maxdr+=incr;
	else if(tmp==2)def.maxdr-=incr;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorgy+110,_x,_y);
	if(tmp==1)def.maxdg+=incr;
	else if(tmp==2)def.maxdg-=incr;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorby+110,_x,_y);
	if(tmp==1)def.maxdb+=incr;
	else if(tmp==2)def.maxdb-=incr;
	tmp=check_plus_minus_hit(colorx4-pm_width,coloray+110,_x,_y);
	if(tmp==1)def.maxda+=incr;
	else if(tmp==2)def.maxda-=incr;


	minx=check_plus_minus_hit(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+30,_x,_y);
	miny=check_plus_minus_hit(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+50,_x,_y);
	minz=check_plus_minus_hit(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+70,_x,_y);
	maxx=check_plus_minus_hit(previewx2-pm_width,sel_handle_bottom+30,_x,_y);
	maxy=check_plus_minus_hit(previewx2-pm_width,sel_handle_bottom+50,_x,_y);
	maxz=check_plus_minus_hit(previewx2-pm_width,sel_handle_bottom+70,_x,_y);
	switch(preview_display_particle_handles)
	{
		case(PREVIEW_PARTICLE_STARTPOS):
			if(minx==1)def.minx+=incr;
			else if(minx==2)def.minx-=incr;
			else if(miny==1)def.miny+=incr;
			else if(miny==2)def.miny-=incr;
			else if(minz==1)def.minz+=incr;
			else if(minz==2)def.minz-=incr;
			else if(maxx==1)def.maxx+=incr;
			else if(maxx==2)def.maxx-=incr;
			else if(maxy==1)def.maxy+=incr;
			else if(maxy==2)def.maxy-=incr;
			else if(maxz==1)def.maxz+=incr;
			else if(maxz==2)def.maxz-=incr;
			break;
		case(PREVIEW_PARTICLE_STARTVEL):
			if(minx==1)def.vel_minx+=incr;
			else if(minx==2)def.vel_minx-=incr;
			else if(miny==1)def.vel_miny+=incr;
			else if(miny==2)def.vel_miny-=incr;
			else if(minz==1)def.vel_minz+=incr;
			else if(minz==2)def.vel_minz-=incr;
			else if(maxx==1)def.vel_maxx+=incr;
			else if(maxx==2)def.vel_maxx-=incr;
			else if(maxy==1)def.vel_maxy+=incr;
			else if(maxy==2)def.vel_maxy-=incr;
			else if(maxz==1)def.vel_maxz+=incr;
			else if(maxz==2)def.vel_maxz-=incr;
			break;
		case(PREVIEW_PARTICLE_ACC):
			if(minx==1)def.acc_minx+=incr;
			else if(minx==2)def.acc_minx-=incr;
			else if(miny==1)def.acc_miny+=incr;
			else if(miny==2)def.acc_miny-=incr;
			else if(minz==1)def.acc_minz+=incr;
			else if(minz==2)def.acc_minz-=incr;
			else if(maxx==1)def.acc_maxx+=incr;
			else if(maxx==2)def.acc_maxx-=incr;
			else if(maxy==1)def.acc_maxy+=incr;
			else if(maxy==2)def.acc_maxy-=incr;
			else if(maxz==1)def.acc_maxz+=incr;
			else if(maxz==2)def.acc_maxz-=incr;
			break;
		case(PREVIEW_PARTICLE_CONSTRAINT):
			if(maxx==1)
				{
					if(def.constrain_rad_sq<0.01)def.constrain_rad_sq=0.011;
					else def.constrain_rad_sq+=0.01;
				}
			else if(maxx==2)
				{
					if(def.constrain_rad_sq>0.01)def.constrain_rad_sq-=0.01;
					else def.constrain_rad_sq=-1.0;
				}
			break;
		case(PREVIEW_PARTICLE_TEXTURE):
			if(maxx==1 && def.part_texture<7)def.part_texture++;
			else if(maxx==2 && def.part_texture>0)def.part_texture--;
			break;
		case(PREVIEW_PARTICLE_LIGHT):
			if(minx==1)def.lightx+=incr;
			else if(minx==2)def.lightx-=incr;
			else if(miny==1)def.lighty+=incr;
			else if(miny==2)def.lighty-=incr;
			else if(minz==1)def.lightz+=incr;
			else if(minz==2)def.lightz-=incr;
			else if(maxx==1){if(def.lightr+incr<=5.0f)def.lightr+=incr;}
			else if(maxx==2){if(def.lightr-incr>=0.0f)def.lightr-=incr;}
			else if(maxy==1){if(def.lightg+incr<=5.0f)def.lightg+=incr;}
			else if(maxy==2){if(def.lightg-incr>=0.0f)def.lightg-=incr;}
			else if(maxz==1){if(def.lightr+incr<=5.0f)def.lightb+=incr;}
			else if(maxz==2){if(def.lightr-incr>=0.0f)def.lightb-=incr;}
			
			// XXX FIXME (Grum): get rid of mouse_x/y
			if(mouse_x>previewx+11*12+15 && mouse_x<previewx+11*12+30){
				if(mouse_y<sel_handle_bottom+72 && mouse_y>sel_handle_bottom+52) {
				def.use_light=!def.use_light;
			}}
	
			break;
		case PREVIEW_PARTICLE_SOUND:
			if (maxx == 1 && def.sound_nr < 9) def.sound_nr++;
			if (maxx == 2 && def.sound_nr > -1) def.sound_nr--;
#ifdef OLD_SOUND
			if (_x > previewx2 - pm_width && _x < previewx2 - pm_width + 15 && _y > sel_handle_bottom+55 && _y < sel_handle_bottom+55+15)
			{
				def.positional = !def.positional;
			}
			if (_x > previewx2 - pm_width && _x < previewx2 - pm_width + 15 && _y > sel_handle_bottom+75 && _y < sel_handle_bottom+75+15)
			{
				def.loop = !def.loop;
			}
#endif //OLD_SOUND
			break;
	}

	// We must make sure that we haven't set up values so that _no_ particles are created within the constraint (since that
	// would make the system hang.)
	if(def.constrain_rad_sq>0.0)
		{
			float rad=sqrt(def.constrain_rad_sq);
			if(def.minx>rad)def.minx=rad-0.1;
			if(def.maxx<-rad)def.maxx=-rad+0.1;
			if(def.miny>rad)def.miny=rad-0.1;
			if(def.maxy<-rad)def.maxy=-rad+0.1;
			if(def.minx*def.maxx>0.0 || def.miny*def.maxy>0.0)
				{
					float dist=def.minx*def.minx+def.miny*def.miny;
					if(dist>def.constrain_rad_sq)
						{
							def.minx*=sqrt(def.constrain_rad_sq/dist)-0.1;
							def.miny*=sqrt(def.constrain_rad_sq/dist)-0.1;
						}
					dist=def.minx*def.minx+def.maxy*def.maxy;
					if(dist>def.constrain_rad_sq)
						{
							def.minx*=sqrt(def.constrain_rad_sq/dist)-0.1;
							def.maxy*=sqrt(def.constrain_rad_sq/dist)-0.1;
						}
					dist=def.maxx*def.maxx+def.maxy*def.maxy;
					if(dist>def.constrain_rad_sq)
						{
							def.maxx*=sqrt(def.constrain_rad_sq/dist)-0.1;
							def.maxy*=sqrt(def.constrain_rad_sq/dist)-0.1;
						}
					dist=def.maxx*def.maxx+def.miny*def.miny;
					if(dist>def.constrain_rad_sq)
						{
							def.maxx*=sqrt(def.constrain_rad_sq/dist)-0.1;
							def.miny*=sqrt(def.constrain_rad_sq/dist)-0.1;
						}
				}
		}

	if(_x>previewx && _x<previewx2 && _y>previewy2 && _y<sel_handle_bottom)
		preview_display_particle_handles=(preview_display_particle_handles+1)%7;

	// Save definition
	if(_x>=10 && _x<=42 && _y>=380 && _y<=412)save_particle_def_file();

	UNLOCK_PARTICLES_LIST();
	return 1;
}

void particles_win_move_preview(float zmove) {
	LOCK_PARTICLES_LIST();
	check_particle_sys_alive();
	particles_list[part_sys]->z_pos+=zmove;
	UNLOCK_PARTICLES_LIST();
}

void particles_win_zoomin(){
	preview_zoom -= ctrl_on ? 2.5f : 0.25f;
	if(preview_zoom<1.0f) preview_zoom = 1.0f;
}

void particles_win_zoomout(){
	preview_zoom += ctrl_on ? 2.5f : 0.25f;	
}

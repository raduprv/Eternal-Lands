#include "global.h"

int particles_window=0;
int view_particles_window=0;
static int particles_window_x=15;
static int particles_window_y=50;
static int particles_window_x_len=600;
static int particles_window_y_len=440;

particle_sys_def def;
static int part_sys=-1;

//TODO: Maybe add support for finer/coarser changes by using modifier keys (alt, shift, ctrl)??
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
		part_sys=create_particle_sys(&def,-666.0,-666.0,1.0);
}

void display_particles_window()
{
	if(particles_window<=0)
		{
			particles_window=create_window("particles",0,0,particles_window_x,particles_window_y,particles_window_x_len,particles_window_y_len,ELW_WIN_DEFAULT);
			set_window_handler(particles_window,ELW_HANDLER_DISPLAY,&display_particles_window_handler);
			set_window_handler(particles_window,ELW_HANDLER_CLICK,&check_particles_window_interface);
			reset_def();
		}
	else
		{
			show_window(particles_window);
			select_window(particles_window);
		}
	display_window(particles_window);
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
	draw_string(x+2,y,"+ -",1);
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
	float save_cx=cx,save_cy=cy,save_cz=cz;
	float save_rx=rx;
	int save_view_particles=view_particles;
	int viewx=win->pos_x+previewx+1;
	int vieww=previewx2-previewx-2;
	int viewy=window_height-(win->pos_y+previewy2)+1;
	int viewh=previewy2-previewy-2;

	cx=-particles_list[part_sys]->x_pos;
	cy=-particles_list[part_sys]->y_pos;
	cz=-particles_list[part_sys]->z_pos;
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
	glTranslatef(0.0,0.0, cz);
	glEnable(GL_LIGHTING);
	glDepthMask(GL_TRUE);
	// Draw a few tiles as a background
	get_and_set_texture_id(tile_list[1]);
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(1.0f, 0);
	glVertex2f(-3.0,3.0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(-3.0,0.0);
	glTexCoord2f(0, 1.0f);
	glVertex2f(0.0,3.0);
	glTexCoord2f(0, 0);
	glVertex2f(0.0,0.0);
	glTexCoord2f(1.0f, 0);
	glVertex2f(3.0,3.0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(3.0,0.0);
	glEnd();
	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(1.0f, 0);
	glVertex2f(-3.0,0.0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(-3.0,-3.0);
	glTexCoord2f(0, 1.0f);
	glVertex2f(0.0,0.0);
	glTexCoord2f(0, 0);
	glVertex2f(0.0,-3.0);
	glTexCoord2f(1.0f, 0);
	glVertex2f(3.0,0.0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(3.0,-3.0);
	glEnd();
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
		}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();

	cx=save_cx;
	cy=save_cy;
	cz=save_cz;
	rx=save_rx;
	view_particles=save_view_particles;
}

int display_particles_window_handler(window_info *win)
{
	char temp[100];
	char *preview_display_handle_strings[]={"Texture","Start position","Constraint","Start velocity","Acceleration"};

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

	lock_particles_list();
	check_particle_sys_alive();

	display_particles_window_preview(win);

	snprintf(temp,99,"System type: %i",def.part_sys_type);
	draw_string(systypex+2,systypey+2,temp,1);
	display_plus_minus(systypex2,systypey);

	draw_string(blendx+2,blendy-18,"Blend modes",1);
	snprintf(temp,99,"S: %s",get_blend_func_string(def.sblend));
	draw_string(blendx+2,blendy+2,temp,1);
	snprintf(temp,99,"D: %s",get_blend_func_string(def.dblend));
	draw_string(blendx+2,blendy2+2,temp,1);

	snprintf(temp,99,"#particles: %i",def.total_particle_no);
	draw_string(particlenox+2,particlenoy+2,temp,1);
	display_plus_minus(particlenox2,particlenoy);

	snprintf(temp,99,"TTL: %i",def.ttl);
	draw_string(ttlx+2,ttly+2,temp,1);
	display_plus_minus(ttlx2,ttly);

	snprintf(temp,99,"RND: %i",def.random_func);
	draw_string(randx+2,randy+2,temp,1);
	display_plus_minus(randx2,randy);

	snprintf(temp,99,"Particle size: %.1f",def.part_size);
	draw_string(sizex+2,sizey+2,temp,1);
	display_plus_minus(sizex2,sizey);

	draw_string(colorx2+2,colory+2,"min",1);
	draw_string(colorx3+2,colory+2,"max",1);
	draw_string(colorx+2,colorry+2,"R",1);
	draw_string(colorx+2,colorgy+2,"G",1);
	draw_string(colorx+2,colorby+2,"B",1);
	draw_string(colorx+2,coloray+2,"A",1);
	snprintf(temp,99,"%.2f",def.minr);
	draw_string(colorx2+12,colorry+2,temp,1);
	snprintf(temp,99,"%.2f",def.ming);
	draw_string(colorx2+12,colorgy+2,temp,1);
	snprintf(temp,99,"%.2f",def.minb);
	draw_string(colorx2+12,colorby+2,temp,1);
	snprintf(temp,99,"%.2f",def.mina);
	draw_string(colorx2+12,coloray+2,temp,1);
	snprintf(temp,99,"%.2f",def.maxr);
	draw_string(colorx3+12,colorry+2,temp,1);
	snprintf(temp,99,"%.2f",def.maxg);
	draw_string(colorx3+12,colorgy+2,temp,1);
	snprintf(temp,99,"%.2f",def.maxb);
	draw_string(colorx3+12,colorby+2,temp,1);
	snprintf(temp,99,"%.2f",def.maxa);
	draw_string(colorx3+12,coloray+2,temp,1);
	display_plus_minus(colorx3-pm_width,colorry);
	display_plus_minus(colorx3-pm_width,colorgy);
	display_plus_minus(colorx3-pm_width,colorby);
	display_plus_minus(colorx3-pm_width,coloray);
	display_plus_minus(colorx4-pm_width,colorry);
	display_plus_minus(colorx4-pm_width,colorgy);
	display_plus_minus(colorx4-pm_width,colorby);
	display_plus_minus(colorx4-pm_width,coloray);

	draw_string(colorx+2,colorry+112,"dR",1);
	draw_string(colorx+2,colorgy+112,"dG",1);
	draw_string(colorx+2,colorby+112,"dB",1);
	draw_string(colorx+2,coloray+112,"dA",1);
	snprintf(temp,99,"%.2f",def.mindr);
	draw_string(colorx2+12,colorry+112,temp,1);
	snprintf(temp,99,"%.2f",def.mindg);
	draw_string(colorx2+12,colorgy+112,temp,1);
	snprintf(temp,99,"%.2f",def.mindb);
	draw_string(colorx2+12,colorby+112,temp,1);
	snprintf(temp,99,"%.2f",def.minda);
	draw_string(colorx2+12,coloray+112,temp,1);
	snprintf(temp,99,"%.2f",def.maxdr);
	draw_string(colorx3+12,colorry+112,temp,1);
	snprintf(temp,99,"%.2f",def.maxdg);
	draw_string(colorx3+12,colorgy+112,temp,1);
	snprintf(temp,99,"%.2f",def.maxdb);
	draw_string(colorx3+12,colorby+112,temp,1);
	snprintf(temp,99,"%.2f",def.maxda);
	draw_string(colorx3+12,coloray+112,temp,1);
	display_plus_minus(colorx3-pm_width,colorry+110);
	display_plus_minus(colorx3-pm_width,colorgy+110);
	display_plus_minus(colorx3-pm_width,colorby+110);
	display_plus_minus(colorx3-pm_width,coloray+110);
	display_plus_minus(colorx4-pm_width,colorry+110);
	display_plus_minus(colorx4-pm_width,colorgy+110);
	display_plus_minus(colorx4-pm_width,colorby+110);
	display_plus_minus(colorx4-pm_width,coloray+110);

	snprintf(temp,99,"Display: %s",preview_display_handle_strings[preview_display_particle_handles]);
	draw_string(previewx+2,previewy2+2,temp,1);

	switch(preview_display_particle_handles)
		{
		case(PREVIEW_PARTICLE_STARTPOS):
			draw_string(previewx+2,sel_handle_bottom+2,"Min",1);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+2,"Max",1);
			snprintf(temp,99,"x: %.2f",def.minx);
			draw_string(previewx+2,sel_handle_bottom+22,temp,1);
			snprintf(temp,99,"y: %.2f",def.miny);
			draw_string(previewx+2,sel_handle_bottom+42,temp,1);
			snprintf(temp,99,"z: %.2f",def.minz);
			draw_string(previewx+2,sel_handle_bottom+62,temp,1);
			snprintf(temp,99,"x: %.2f",def.maxx);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+22,temp,1);
			snprintf(temp,99,"y: %.2f",def.maxy);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+42,temp,1);
			snprintf(temp,99,"z: %.2f",def.maxz);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+62,temp,1);
			break;
		case(PREVIEW_PARTICLE_CONSTRAINT):
			if(def.constrain_rad_sq<0.0)
				draw_string(previewx+2,sel_handle_bottom+22,"No radius constraint",1);
			else
				{
					snprintf(temp,99,"Actual radius: %.3f",sqrt(def.constrain_rad_sq));
					draw_string(previewx+2,sel_handle_bottom+2,temp,1);
					snprintf(temp,99,"Squared radius: %.3f",def.constrain_rad_sq);
					draw_string(previewx+2,sel_handle_bottom+22,temp,1);
				}
			break;
		case(PREVIEW_PARTICLE_STARTVEL):
			draw_string(previewx+2,sel_handle_bottom+2,"Min",1);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+2,"Max",1);
			snprintf(temp,99,"x: %.2f",def.vel_minx);
			draw_string(previewx+2,sel_handle_bottom+22,temp,1);
			snprintf(temp,99,"y: %.2f",def.vel_miny);
			draw_string(previewx+2,sel_handle_bottom+42,temp,1);
			snprintf(temp,99,"z: %.2f",def.vel_minz);
			draw_string(previewx+2,sel_handle_bottom+62,temp,1);
			snprintf(temp,99,"x: %.2f",def.vel_maxx);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+22,temp,1);
			snprintf(temp,99,"y: %.2f",def.vel_maxy);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+42,temp,1);
			snprintf(temp,99,"z: %.2f",def.vel_maxz);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+62,temp,1);
			break;
		case(PREVIEW_PARTICLE_ACC):
			draw_string(previewx+2,sel_handle_bottom+2,"Min",1);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+2,"Max",1);
			snprintf(temp,99,"x: %.2f",def.acc_minx);
			draw_string(previewx+2,sel_handle_bottom+22,temp,1);
			snprintf(temp,99,"y: %.2f",def.acc_miny);
			draw_string(previewx+2,sel_handle_bottom+42,temp,1);
			snprintf(temp,99,"z: %.2f",def.acc_minz);
			draw_string(previewx+2,sel_handle_bottom+62,temp,1);
			snprintf(temp,99,"x: %.2f",def.acc_maxx);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+22,temp,1);
			snprintf(temp,99,"y: %.2f",def.acc_maxy);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+42,temp,1);
			snprintf(temp,99,"z: %.2f",def.acc_maxz);
			draw_string(previewx+2+(previewx2-previewx)/2,sel_handle_bottom+62,temp,1);
			break;
		case(PREVIEW_PARTICLE_TEXTURE):
			get_and_set_texture_id(particle_textures[def.part_texture]);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ZERO);
			glBegin(GL_TRIANGLE_STRIP);
				glColor3f(1.0,1.0,1.0);
				glTexCoord2f(0.0,1.0);
				glVertex2i(previewx+(previewx2-previewx-60)/2,sel_handle_bottom+21);
				glTexCoord2f(0.0,0.0);
				glVertex2i(previewx+(previewx2-previewx-60)/2,sel_handle_bottom+81);
				glTexCoord2f(1.0,1.0);
				glVertex2i(previewx2-(previewx2-previewx-60)/2,sel_handle_bottom+21);
				glTexCoord2f(1.0,0.0);
				glVertex2i(previewx2-(previewx2-previewx-60)/2,sel_handle_bottom+81);
			glEnd();
			glDisable(GL_BLEND);
			break;
		}
	switch(preview_display_particle_handles)
		{
		case(PREVIEW_PARTICLE_STARTPOS):
		case(PREVIEW_PARTICLE_STARTVEL):
		case(PREVIEW_PARTICLE_ACC):
			display_plus_minus(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+20);
			display_plus_minus(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+40);
			display_plus_minus(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+60);
			display_plus_minus(previewx2-pm_width,sel_handle_bottom+40);
			display_plus_minus(previewx2-pm_width,sel_handle_bottom+60);
		default:
			display_plus_minus(previewx2-pm_width,sel_handle_bottom+20);
		}

	snprintf(temp,99,"System info: TTL==%i, #particles==%i",particles_list[part_sys]->ttl,particles_list[part_sys]->particle_count);
	draw_string(10,420,temp,1);
	unlock_particles_list();

	get_and_set_texture_id(buttons_text);
	glColor3f(1.0,1.0,1.0);
	glBegin(GL_QUADS);
	draw_2d_thing((float)128/255,1.0f-(float)32/255,(float)160/255,1.0f-(float)64/255, 10,380,42,412);
	draw_2d_thing((float)64/255,1.0f-(float)32/255,(float)96/255,1.0f-(float)64/255, 74,380,106,412);
	glEnd();

	return 1;
}

int check_particles_window_interface(window_info *win, int mx, int my, Uint32 flags)
{
	int x,y,tmp,i;
	int minx,miny,minz,maxx,maxy,maxz;

	if(mouse_x>win->pos_x+win->len_x || mouse_x<win->pos_x || mouse_y<win->pos_y || mouse_y>win->pos_y+win->len_y)return 0;

   	if(mouse_x>(win->pos_x+win->len_x-20) && mouse_y<=win->pos_y+20)
	{
		toggle_particles_window();
		return 1;
	}


	lock_particles_list();
	check_particle_sys_alive();

	x=mouse_x-win->pos_x;
	y=mouse_y-win->pos_y;

	tmp=check_plus_minus_hit(systypex2,systypey,x,y);
	if(tmp==1 && def.part_sys_type<5)def.part_sys_type++;
	else if(tmp==2 && def.part_sys_type>0)def.part_sys_type--;

	if(x>blendx && x<blendx2 && y>blendy && y<blendy3)
		{
			if(y<blendy2)def.sblend=get_next_src_blend_func(def.sblend);
			else def.dblend=get_next_dst_blend_func(def.dblend);
		}

	tmp=check_plus_minus_hit(particlenox2,particlenoy,x,y);
	if(tmp==1 && def.total_particle_no<max_particles)
		{
			// If we add particles to an existing system, we must make sure they are free
			for(i=def.total_particle_no;i<max_particles;i++)particles_list[part_sys]->particles[i].free=1;
			def.total_particle_no+=50;
		}
	else if(tmp==2 && def.total_particle_no>0)def.total_particle_no-=50;

	tmp=check_plus_minus_hit(ttlx2,ttly,x,y);
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

	tmp=check_plus_minus_hit(randx2,randy,x,y);
	if(tmp==1)def.random_func=1;
	else if(tmp==2)def.random_func=0;

	tmp=check_plus_minus_hit(sizex2,sizey,x,y);
	if(tmp==1)def.part_size+=0.1;
	else if(tmp==2 && def.part_size>0.1)def.part_size-=0.1;

	tmp=check_plus_minus_hit(colorx3-pm_width,colorry,x,y);
	if(tmp==1)def.minr+=0.05;
	else if(tmp==2)def.minr-=0.05;
	tmp=check_plus_minus_hit(colorx3-pm_width,colorgy,x,y);
	if(tmp==1)def.ming+=0.05;
	else if(tmp==2)def.ming-=0.05;
	tmp=check_plus_minus_hit(colorx3-pm_width,colorby,x,y);
	if(tmp==1)def.minb+=0.05;
	else if(tmp==2)def.minb-=0.05;
	tmp=check_plus_minus_hit(colorx3-pm_width,coloray,x,y);
	if(tmp==1)def.mina+=0.05;
	else if(tmp==2)def.mina-=0.05;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorry,x,y);
	if(tmp==1)def.maxr+=0.05;
	else if(tmp==2)def.maxr-=0.05;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorgy,x,y);
	if(tmp==1)def.maxg+=0.05;
	else if(tmp==2)def.maxg-=0.05;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorby,x,y);
	if(tmp==1)def.maxb+=0.05;
	else if(tmp==2)def.maxb-=0.05;
	tmp=check_plus_minus_hit(colorx4-pm_width,coloray,x,y);
	if(tmp==1)def.maxa+=0.05;
	else if(tmp==2)def.maxa-=0.05;

	tmp=check_plus_minus_hit(colorx3-pm_width,colorry+110,x,y);
	if(tmp==1)def.mindr+=0.01;
	else if(tmp==2)def.mindr-=0.01;
	tmp=check_plus_minus_hit(colorx3-pm_width,colorgy+110,x,y);
	if(tmp==1)def.mindg+=0.01;
	else if(tmp==2)def.mindg-=0.01;
	tmp=check_plus_minus_hit(colorx3-pm_width,colorby+110,x,y);
	if(tmp==1)def.mindb+=0.01;
	else if(tmp==2)def.mindb-=0.01;
	tmp=check_plus_minus_hit(colorx3-pm_width,coloray+110,x,y);
	if(tmp==1)def.minda+=0.01;
	else if(tmp==2)def.minda-=0.01;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorry+110,x,y);
	if(tmp==1)def.maxdr+=0.01;
	else if(tmp==2)def.maxdr-=0.01;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorgy+110,x,y);
	if(tmp==1)def.maxdg+=0.01;
	else if(tmp==2)def.maxdg-=0.01;
	tmp=check_plus_minus_hit(colorx4-pm_width,colorby+110,x,y);
	if(tmp==1)def.maxdb+=0.01;
	else if(tmp==2)def.maxdb-=0.01;
	tmp=check_plus_minus_hit(colorx4-pm_width,coloray+110,x,y);
	if(tmp==1)def.maxda+=0.01;
	else if(tmp==2)def.maxda-=0.01;


	minx=check_plus_minus_hit(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+20,x,y);
	miny=check_plus_minus_hit(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+40,x,y);
	minz=check_plus_minus_hit(previewx+(previewx2-previewx)/2-pm_width,sel_handle_bottom+60,x,y);
	maxx=check_plus_minus_hit(previewx2-pm_width,sel_handle_bottom+20,x,y);
	maxy=check_plus_minus_hit(previewx2-pm_width,sel_handle_bottom+40,x,y);
	maxz=check_plus_minus_hit(previewx2-pm_width,sel_handle_bottom+60,x,y);
	switch(preview_display_particle_handles)
		{
		case(PREVIEW_PARTICLE_STARTPOS):
			if(minx==1)def.minx+=0.1;
			else if(minx==2)def.minx-=0.1;
			else if(miny==1)def.miny+=0.1;
			else if(miny==2)def.miny-=0.1;
			else if(minz==1)def.minz+=0.1;
			else if(minz==2)def.minz-=0.1;
			else if(maxx==1)def.maxx+=0.1;
			else if(maxx==2)def.maxx-=0.1;
			else if(maxy==1)def.maxy+=0.1;
			else if(maxy==2)def.maxy-=0.1;
			else if(maxz==1)def.maxz+=0.1;
			else if(maxz==2)def.maxz-=0.1;
			break;
		case(PREVIEW_PARTICLE_STARTVEL):
			if(minx==1)def.vel_minx+=0.01;
			else if(minx==2)def.vel_minx-=0.01;
			else if(miny==1)def.vel_miny+=0.01;
			else if(miny==2)def.vel_miny-=0.01;
			else if(minz==1)def.vel_minz+=0.01;
			else if(minz==2)def.vel_minz-=0.01;
			else if(maxx==1)def.vel_maxx+=0.01;
			else if(maxx==2)def.vel_maxx-=0.01;
			else if(maxy==1)def.vel_maxy+=0.01;
			else if(maxy==2)def.vel_maxy-=0.01;
			else if(maxz==1)def.vel_maxz+=0.01;
			else if(maxz==2)def.vel_maxz-=0.01;
			break;
		case(PREVIEW_PARTICLE_ACC):
			if(minx==1)def.acc_minx+=0.01;
			else if(minx==2)def.acc_minx-=0.01;
			else if(miny==1)def.acc_miny+=0.01;
			else if(miny==2)def.acc_miny-=0.01;
			else if(minz==1)def.acc_minz+=0.01;
			else if(minz==2)def.acc_minz-=0.01;
			else if(maxx==1)def.acc_maxx+=0.01;
			else if(maxx==2)def.acc_maxx-=0.01;
			else if(maxy==1)def.acc_maxy+=0.01;
			else if(maxy==2)def.acc_maxy-=0.01;
			else if(maxz==1)def.acc_maxz+=0.01;
			else if(maxz==2)def.acc_maxz-=0.01;
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

	if(x>previewx && x<previewx2 && y>previewy2 && y<sel_handle_bottom)
		preview_display_particle_handles=(preview_display_particle_handles+1)%5;

	// Reset system
	if(x>=10 && x<=42 && y>=380 && y<=412)
		{
			particles_list[part_sys]->ttl=0;
			particles_list[part_sys]->particle_count=0;				      
			part_sys=-1;
		}

	// Save definition
	if(x>=74 && x<=106 && y>=380 && y<=412)save_particle_def_file();

	unlock_particles_list();
	return 1;
}

void particles_win_move_preview(float zmove) {
	lock_particles_list();
	check_particle_sys_alive();
	particles_list[part_sys]->z_pos+=zmove;
	unlock_particles_list();
}

void particles_win_zoomin(){
	preview_zoom -= ctrl_on ? 2.5f : 0.25f;
	if(preview_zoom<1.0f) preview_zoom = 1.0f;
}

void particles_win_zoomout(){
	preview_zoom += ctrl_on ? 2.5f : 0.25f;	
}

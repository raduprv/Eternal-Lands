#ifdef SKY_FPV_CURSOR

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <SDL.h>
#include <SDL_active.h>
#include "sky.h"
#include "draw_scene.h"
#include "gl_init.h"
#include "global.h"
#include "lights.h"
#include "map.h"
#include "reflection.h"
#include "shadows.h"
#include "textures.h"
#include "vmath.h"
#include "weather.h"

#define SLICES 21
float foo[4],foo1[4],foo2[4],foo3[4];
float slice_list[SLICES][2] = {
{1,	0},
{0.95,	0.31},
{0.81,	0.59},
{0.59,	0.81},
{0.31,	0.95},
{0,	1},
{-0.31,	0.95},
{-0.59,	0.81},
{-0.81,	0.59},
{-0.95,	0.31},
{-1,	0},
{-0.95,	-0.31},
{-0.81,	-0.59},
{-0.59,	-0.81},
{-0.31,	-0.95},
{0,	-1},
{0.31,	-0.95},
{0.59,	-0.81},
{0.81,	-0.59},
{0.95,	-0.31},
{1,	0}

};



void (*display_sky)(int);

#ifdef NEW_WEATHER
#define fogColor rain_color
#endif //NEW_WEATHER


GLuint skyLists;
int show_moons = 1,show_sun = 1,show_stars = 1,horizon_fog = 1,clouds1 = 1,clouds2 = 1,reflect_sky=1;
int clouds_tex;
int cloud_detail_tex;
int smokey_cloud_tex;
int moon_tex;
int sun_tex;
double time_d;
#define NUM_STARS 3000
float strs[NUM_STARS][3];
float sun_appears[4]={0,0,0,0};
double LongView[16];
void cloud_layer1(int clouds);
void cloud_layer2(int clouds);
void sky_color(int sky);
int cloud_1=CLOUDS_THICK, cloud_2=CLOUDS_NONE, sky=SKY_COLOR;
int cur_stencil;
float *fog[4];
float *colors[10];

int skydisk_on=0;
float skydisk[SKYDISK_SECTORS][SKYDISK_DIVS][SKYDISK_SLICES][3];
#define SKYSWAP(a,b) a[0]=b[1]; a[1]=b[0]; a[2]=b[2];


void cloudy_sky(int reflected);
void animated_sky(int reflected);
void simple_sky(int reflected);
void reflected_sky(int reflected);

/*
  colorSkyCyl - local utility function.
  
  TODO: make this work on a predefined vertex array stack or something.

  Creates a colored cylinder, one color from the array per stack,
  smooth blending from one stack to the next. Stacks are 1.0 high each,
  cylinder is 1.0 radius. Stacks start a z = 0.0f and go to z = 1.0f * (stacks-1)
  
*/
void colorSkyCyl(int stacks, float **thecolors)
{
	int s,j;
	glBegin(GL_QUAD_STRIP);
	for (s = 0; s < stacks; s++)
	{
		for (j = 0; j < SLICES; j++)
		{
			glColor4fv(thecolors[s]);
			glVertex3f(slice_list[j][0],slice_list[j][1],s);
			glColor4fv(thecolors[s+1]);
			glVertex3f(slice_list[j][0],slice_list[j][1],s+1);
		}
	}
	glEnd();
}

//affine blend 2 colors
//dst is col destination
//post: dst contains src1 if t is 1, src2 if t is 0, blend if 1>t>0
//      possiblilty of wacky if t not in [0,1]
void colBlend4(float dst[4], float src1[4], float src2[4], float t)
{
	dst[0] = t*src1[0] + (1.0-t)*src2[0];
	dst[1] = t*src1[1] + (1.0-t)*src2[1];
	dst[2] = t*src1[2] + (1.0-t)*src2[2];
	dst[3] = t*src1[3] + (1.0-t)*src2[3];
}



void sky_type(int sky)
{
	switch (sky)
	{
		case UNDERWORLD_SKY:
			display_sky = animated_sky;
			break;
		case INTERIORS_SKY:
			//display_sky = simple_sky;
			display_sky = NULL;
			break;
		case CLOUDY_SKY:
		default:
			display_sky = reflected_sky;
	}
}

void cloud_layer1(int clouds)
{
	cloud_1 = clouds;
}
void cloud_layer2(int clouds)
{
	cloud_2 = clouds;
}

void sky_color(int sky_col)
{
	sky = sky_col;
}

void simple_sky(int reflected)
{
	static float spin = 0;
	float abs_light;

	abs_light=light_level;
	if(light_level>59)abs_light=119-light_level;
	abs_light = 1.0f-abs_light/59.0f;

	spin = cur_time%250000;
	spin*=360.0f/250000.0f;
	if(SDL_GetAppState()&SDL_APPACTIVE)
	{
		glPushMatrix();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixd(LongView);
		glMatrixMode(GL_MODELVIEW);

		glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
		glRotatef(rx,1.0,0.0,0.0);
		glRotatef(rz,0.0,0.0,1.0);
        if (reflected)
            glScalef(1.0, 1.0, -1.0);
		if(use_shadow_mapping)
		{
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(shadow_unit);
			glDisable(depth_texture_target);
			disable_texgen();
			ELglActiveTextureARB(GL_TEXTURE0);
			glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}

		glDisable(GL_LIGHTING);
		glDisable(GL_FOG);
		glEnable(GL_COLOR_MATERIAL);
		glDisable(GL_TEXTURE_2D);
		glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
		glTranslatef(0,0,-1);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		glColor4f(0.0,0.0,0.0,0.0);
		glCallList(skyLists);

		fog[0][0] = fog[1][0] = fog[2][0] = fogColor[0];
		fog[0][1] = fog[1][1] = fog[2][1] = fogColor[1];
		fog[0][2] = fog[1][2] = fog[2][2] = fogColor[2];

		fog[0][3] = 1.0f;
		fog[1][3] = 0.7f;
		fog[2][3] = 0.3f;
		fog[3][3] = 0.0f;

		glPushMatrix();
		glScalef(100.0,100.0,10.0);
		colorSkyCyl(3,fog);
		glPopMatrix();
		glColor4f(1, 1,1,1);
		glDisable(GL_COLOR_MATERIAL);
		if(use_shadow_mapping)
		{
			last_texture=-1;
		}
		glColor4f(0.4,0.6,1.0,1.0);
		glEnable(GL_FOG);
		glCallList(skyLists+1);


		glPopAttrib();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
}


void cloudy_sky(int reflected)
{
	static double spin = 0;
	float alph,cloudCol1[4],cloudCol2[4],cloudCol3[4],cloudCol4[4];
	float abs_light;
	float t;

	int i;
	VECTOR4 vec_black={0.0,0.0,0.0,0.0};


	abs_light=light_level;
	if(light_level>59)
	{
		abs_light=119-light_level;

	}
	abs_light = 1.0f-abs_light/59.0f;
	spin = cur_time%( 1296000 * 1000 );
	spin*=360.0/( 1296000.0/*seconds in large month*/ * 1000.0/*millisecond bump*/ );
	spin += time_d; //



	glDisable(GL_DEPTH_TEST);

#ifdef NEW_WEATHER
	if (sky!=SKY_COLOR&&weather_active())
#else /* !NEW_WEATHER */
	if (sky!=SKY_COLOR&&(seconds_till_rain_stops<1||seconds_till_rain_stops>59)&&seconds_till_rain_starts<1)
#endif /* NEW_WATHER */
	{
		for	(i = 0;i < 3; i++)
		{
				fog[0][i] = fog[1][i] = fog[2][i] = fog[3][i] = fogColor[i];
				cloudCol1[i] = fogColor[i]-0.05f;
				cloudCol2[i] = fogColor[i]+0.05f;
				cloudCol3[i] = fogColor[i]-0.05f;
				cloudCol4[i] = fogColor[i]+0.05f;
			}
		cloudCol1[3] = 1.0f;
		cloudCol2[3] = 1.0f;
		cloudCol3[3] = 1.0;
		cloudCol4[3] = 1.0;

		fog[0][3] = 1.0f;
		fog[1][3] = 0.95f;
		fog[2][3] = 0.5f;
		fog[3][3] = 0.0f;

		glColor4fv(fogColor);

		glCallList(skyLists);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
	}
	else 
	{
#ifdef NEW_WEATHER
		if(weather_get_fadeout_bias() < 1.0f && weather_get_fadeout_bias() > 0.0f){
			t = 1.0f-weather_get_fadeout_bias();
#else /* !NEW_WEATHER */
		if(seconds_till_rain_stops>0 && seconds_till_rain_stops<60){
			t = 1.0f-seconds_till_rain_stops/60.0f;
#endif /* NEW_WEATHER */
			for	(i = 0;i < 3; i++)
			{

				fog[0][i] = fog[1][i] = fog[2][i] = fog[3][i] = fogColor[i];
				cloudCol1[i] = t*(fogColor[i]-0.1f)+(1.0f-t)*(fogColor[i]-0.05f);
				cloudCol2[i] = t*(diffuse_light[i]+0.1f)+(1.0f-t)*(fogColor[i]+0.05f);

				cloudCol3[i] = t*(fogColor[i]-0.1f)+(1.0f-t)*(fogColor[i]-0.05f);
				cloudCol4[i] = t*(diffuse_light[i]+0.1f)+(1.0f-t)*(fogColor[i]+0.05f);

			}
			cloudCol1[3] = 1.0f;
			cloudCol2[3] = 1.0f;
			cloudCol3[3] = t;
			cloudCol4[3] = t;

			fog[0][3] = 1.0f;
			fog[1][3] = 0.95f;
			fog[2][3] = 0.5f;
			fog[3][3] = 0.0f;

			colBlend4(foo, sky_lights_c1[light_level],fogColor,t);
			colBlend4(foo1, sky_lights_c2[light_level],fogColor,t);
			colBlend4(foo2, sky_lights_c3[light_level],fogColor,t);
			colBlend4(foo3, sky_lights_c4[light_level],fogColor,t);
			colors[3] = foo3;
			colors[2] = foo2;
			colors[1] = foo1;
			colors[0] = foo;
		}
#ifdef NEW_WEATHER
		else if(weather_get_fadein_bias() < 1.0f && weather_get_fadein_bias() > 0.0f){
			t = weather_get_fadein_bias();
#else /* !NEW_WEATHER */
		else if(seconds_till_rain_starts>0 && seconds_till_rain_starts<60){
			t = seconds_till_rain_starts/60.0f;
#endif /* NEW_WEATHER */

			for	(i = 0;i < 3; i++)
			{

				fog[0][i] = fog[1][i] = fog[2][i] = fog[3][i] = fogColor[i];
				cloudCol1[i] = t*(fogColor[i]-0.1f)+(1.0f-t)*(fogColor[i]-0.05f);
				cloudCol2[i] = t*(diffuse_light[i]+0.1f)+(1.0f-t)*(fogColor[i]+0.05f);
				cloudCol3[i] = t*(fogColor[i]-0.1f)+(1.0f-t)*(fogColor[i]-0.05f);
				cloudCol4[i] = t*(diffuse_light[i]+0.1f)+(1.0f-t)*(fogColor[i]+0.05f);
				
			}
			cloudCol1[3] = 1.0f;
			cloudCol2[3] = 1.0f;
			cloudCol3[3] = t;
			cloudCol4[3] = t;

			fog[0][3] = 1.0f;
			fog[1][3] = 0.95f;
			fog[2][3] = 0.5f;
			fog[3][3] = 0.0f;

			colBlend4(foo, sky_lights_c1[light_level],fogColor,t);
			colBlend4(foo1, sky_lights_c2[light_level],fogColor,t);
			colBlend4(foo2, sky_lights_c3[light_level],fogColor,t);
			colBlend4(foo3, sky_lights_c4[light_level],fogColor,t);
			
			colors[3] = foo3;
			colors[2] = foo2;
			colors[1] = foo1;
			colors[0] = foo;
		}
		else
		{
			t = 1.0f;
			colors[3]=sky_lights_c4[light_level];

			for	(i = 0;i < 3; i++)
			{

				fog[0][i] = fog[1][i] = fog[2][i] = fog[3][i] = fogColor[i];
				cloudCol1[i] = fogColor[i]-0.1f;
				cloudCol2[i] = diffuse_light[i]+0.1f;
				
			}
			cloudCol1[3] = 1.0f;
			cloudCol2[3] = 1.0f;
			cloudCol3[3] = 0.0f;
			cloudCol4[3] = 0.0f;

			fog[0][3] = 1.0f;
			fog[1][3] = 0.95f;
			fog[2][3] = 0.5f;
			fog[3][3] = 0.0f;

			colors[2]=sky_lights_c3[light_level];
			colors[1]=sky_lights_c2[light_level];
			colors[0]=sky_lights_c1[light_level];

		}

		glDisable(GL_TEXTURE_2D);
		glColor4fv(colors[3]);
		if (!horizon_fog)  colors[0] = fogColor;
		glCallList(skyLists);
		glPushMatrix();
		glTranslatef(0,0,-0.1f);
		glScalef(100.0,100.0,15.0);
		colorSkyCyl(3,colors);
		glPopMatrix();
	
	
	

		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_COLOR,GL_ONE);

		//Rotate sky for time of day
		if(show_moons)
		{
			glPushMatrix();
			glRotatef((float)game_minute,1,0,0);
			//Alpha adjustment for objects that should fade in daylight
			alph = (1.0-abs_light)*t;

			glEnable(GL_CULL_FACE);
			glEnable(GL_DEPTH_TEST);
			{
				VECTOR4 vec_moon = {alph/2.0f+0.6f,alph/2.0f+0.6f,alph/2.0f+0.6f,1.0};
				glEnable(GL_LIGHTING);
				glDisable(GL_COLOR_MATERIAL);
				get_and_set_texture_id(moon_tex);
	 			glMaterialfv(GL_FRONT, GL_AMBIENT, vec_black);

				vec_moon[1] /= 1.2f;
				vec_moon[2] /= 1.5f;


				glMaterialfv(GL_FRONT, GL_DIFFUSE, vec_moon);

				glPushMatrix();
				glRotatef(20.0f, 0.0f,0.0f, 1.0f);
				glRotatef(10.0*spin,1.0f,0.0f,0.0f);
				glTranslatef(0.0f,0.0f,.75f);
				glScalef(.5,.5,.5);
				glCallList(skyLists+3);
				glPopMatrix();


				vec_moon[1] *= 1.2f;
				vec_moon[2] *= 1.5f;

    			glMaterialfv(GL_FRONT, GL_DIFFUSE, vec_moon);

				glPushMatrix();
				glRotatef(spin,1,0,0);
				glTranslatef(0.0f,0.0f,1.0f);
				glRotatef(spin-80.0f,1,0,0);
				glCallList(skyLists+3);
				glPopMatrix();

			}
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			glPopMatrix();
		}
		glEnable(GL_COLOR_MATERIAL);



	//draw sun
		alph = t;
		glColor4f(1.0*alph,0.9*alph,0.5*alph,1.0);
		if (show_sun && ((sun_appears[0]*sun_appears[0])+(sun_appears[1]*sun_appears[1])+(sun_appears[2]*sun_appears[2]) > 0.01))
		{
    		get_and_set_texture_id(sun_tex);
			/*if(have_extension(arb_point_parameters) && have_extension(arb_point_sprite))
			{
        		glEnable(GL_POINT_SPRITE_NV);
				glTexEnvf(GL_POINT_SPRITE_NV,GL_COORD_REPLACE_NV,GL_TRUE);
				glPointSize(window_height*0.01/perspective);
				glBegin(GL_POINTS);
				glVertex3fv(sun_appears);
				glEnd();
			}
			else*/
			{
				glBegin(GL_QUADS);
				{
					//two cross products. Dangerous to use these functions. Float type essential.
					//Better to robustly produce two vectors in perpendicular plane elsewhere.
					VECTOR3 perp1, perp2, someVec={1,0,0};
					VCross(perp1, someVec, sun_appears);
					VCross(perp2, perp1, sun_appears);
					glTexCoord2f(0,0);		glVertex3f(sun_appears[0]+.08*(perp1[0]+perp2[0]), sun_appears[1]+.08*(perp1[1]+perp2[1]),sun_appears[2]+.08*(perp1[2]+perp2[2]));
					glTexCoord2f(1,0);		glVertex3f(sun_appears[0]+.08*(-perp1[0]+perp2[0]), sun_appears[1]+.08*(-perp1[1]+perp2[1]),sun_appears[2]+.08*(-perp1[2]+perp2[2]));
					glTexCoord2f(1,1);		glVertex3f(sun_appears[0]+.08*(-perp1[0]-perp2[0]), sun_appears[1]+.08*(-perp1[1]-perp2[1]),sun_appears[2]+.08*(-perp1[2]-perp2[2]));
					glTexCoord2f(0,1);		glVertex3f(sun_appears[0]+.08*(perp1[0]-perp2[0]), sun_appears[1]+.08*(perp1[1]-perp2[1]),sun_appears[2]+.08*(perp1[2]-perp2[2]));
				}
				glEnd();
			}
		}


		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		if (show_stars)
		{
			glPushMatrix();
			glRotatef((float)game_minute,1,0,0);
			alph = (1.0-abs_light)*t;

#ifdef	USE_SHADER
			if (reflected && water_shader_quality > 0)
#else	// USE_SHADER
			if (reflected && use_frame_buffer)
#endif	// USE_SHADER
				glPointSize(2.0);
			else
				glPointSize(1.0);
			glColor4f(1.0,1.0,1.0,alph);
			glCallList(skyLists+2);
			glColor4f(1.0,1.0,1.0,alph/2.0);
			glCallList(skyLists+4);
			glColor4f(1.0,1.0,1.0,alph/4.0);
			glCallList(skyLists+5);
			glPopMatrix();
		}
		glDisable(GL_DEPTH_TEST);
	}


	if ((cloud_1 != CLOUDS_NONE)&&clouds1)
	{
		glPushMatrix();
		glScalef(6,6,1);
		glRotatef(90.0f,0.0f,1.0f,0.0f);
		glRotatef(spin*1000,0.0,0.0,1.0);
		get_and_set_texture_id(clouds_tex);
		glColor4fv(cloudCol2);
		glCallList(skyLists);
		glPopMatrix();
		glPushMatrix();
		glScalef(6,6,1);
		glRotatef(90.0f,0.0f,1.0f,0.0f);
		glRotatef(spin*1000,0.0,0.0,1.0);
		glTranslatef(0.0,0.0,0.5);
		glColor4fv(cloudCol1);
		get_and_set_texture_id(cloud_detail_tex);
		glCallList(skyLists);
		glPopMatrix();
	}
	if ((cloud_2 != CLOUDS_NONE)&& clouds2)
	{
		glPushMatrix();
		glScalef(6,6,1);
		glRotatef(90.0f,0.0f,1.0f,0.0f);
		glRotatef(spin*4000,0.0,0.0,1.0);
		glColor4fv(cloudCol4);
		get_and_set_texture_id(clouds_tex);
		glCallList(skyLists);
		glPopMatrix();
		glPushMatrix();
		glScalef(6,6,1);
		glTranslatef(0,0,1.5);
		glRotatef(90.0f,0.0f,1.0f,0.0f);
		glRotatef(spin*4000,0.0,0.0,1.0);
		glColor4fv(cloudCol3);
		get_and_set_texture_id(cloud_detail_tex);
		glCallList(skyLists);
		glPopMatrix();
	}
	glDisable(GL_TEXTURE_2D);
/* 	if(horizon_fog) */
/* 	{ */
		glTranslatef(0.0,0.0,-0.1);
		glPushMatrix();
		glScalef(100.0,100.0,5.0);
		colorSkyCyl(3,fog);
		glPopMatrix();
		glDisable(GL_BLEND);
/* 	} */
/* 	else */
/* 	{ */
/* 		glDisable(GL_BLEND); */
/* 		glTranslatef(0.0,0.0,camera_z); */
/* 		glPushMatrix(); */
/* 		glScalef(100.0,100.0,-camera_z); */
/* 		colorSkyCyl(1,fog); */
/* 		glPopMatrix(); */
/* 	} */
}


void animated_sky(int reflected)
{
	static float spin = 0;
	float *fog[4];
	float abs_light;
	int i;

	abs_light=light_level;
	if(light_level>59)abs_light=119-light_level;
	abs_light = 1.0f-abs_light/59.0f;

	spin = cur_time%250000;
	spin*=360.0f/250000.0f;
	if(SDL_GetAppState()&SDL_APPACTIVE)
	{
		glPushMatrix();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixd(LongView);
		glMatrixMode(GL_MODELVIEW);

		glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
		glRotatef(rx,1.0,0.0,0.0);
		glRotatef(rz,0.0,0.0,1.0);
        if (reflected)
            glScalef(1.0, 1.0, -1.0);
		if(use_shadow_mapping)
		{
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(shadow_unit);
			glDisable(depth_texture_target);
			disable_texgen();
			ELglActiveTextureARB(GL_TEXTURE0);
			glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}

		glDisable(GL_LIGHTING);
		glDisable(GL_FOG);
		glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);
		glDisable(GL_TEXTURE_2D);
		glTranslatef(0,0,-1);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ZERO);

		glColor4f(0.0,0.0,0.0,0.0);
		glCallList(skyLists);

		glEnable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		get_and_set_texture_id(smokey_cloud_tex);
		glPushMatrix();
		glScalef(6,6,1);
		glPushMatrix();
		glColor4f(1,0,0,1);
		glRotatef(spin,0.0,0.0,1.0);
		glRotatef(90.0f,0.0f,1.0f,0.0f);
		glRotatef(spin,0.0,0.0,1.0);
		glCallList(skyLists);
		glPopMatrix();
		glPushMatrix();
		glColor4f(1,.2,0,1);
		glRotatef(spin*2,0.0,0.0,1.0);
		glRotatef(90.0f,1.0f,0.0f,0.0f);
		glRotatef(spin*2,0.0,0.0,1.0);
		glCallList(skyLists);
		glPopMatrix();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);
		for (i=0; i < 3; i++)
		{
			fog[i] = (float *) malloc (sizeof(float)*4);
		}

		fog[0][0] = fog[1][0] = fog[2][0] = 1;
		fog[0][1] = 1;
		fog[2][1] = 0;
		fog[0][2] = fog[1][2] = fog[2][2] = 0;
		fog[1][1] = .5;
		fog[0][3] = 1.0;
		fog[1][3] = 1.0;
		fog[2][3] = 0.0;
		fog[3] = fog[2];

		glPushMatrix();
		glScalef(100.0,100.0,5.0);
		colorSkyCyl(3,fog);
		glPopMatrix();
		glColor4f(1, 1,1,1);
		if(use_shadow_mapping)
		{
			last_texture=-1;
		}
		glColor4f(1.0,1.0,0.0,1.0);
		glCallList(skyLists+1);

		glPopAttrib();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
}

void reflected_sky(int reflected)
{
/* 	VECTOR4 vec_white={1.0,1.0,1.0,1.0},vec4; */
/* 	vec4[0] = sun_position[0]; */
/* 	vec4[1] = sun_position[1]; */
/* 	vec4[2] = sun_position[2]; */
/* 	vec4[3] = 0.0f; */

	//Disable lights not used for sky just in case
	switch(show_lights)
	{
	case 6:
		glDisable(GL_LIGHT6);
	case 5:
		glDisable(GL_LIGHT5);
	case 4:
		glDisable(GL_LIGHT4);
	case 3:
		glDisable(GL_LIGHT3);
	case 2:
		glDisable(GL_LIGHT2);
	case 1:
		glDisable(GL_LIGHT1);
	default:
		glDisable(GL_LIGHT0);
		break;
	}

	if(have_stencil)
	{
		cur_stencil = 0;
		glClearStencil(cur_stencil);
		glClear(GL_STENCIL_BUFFER_BIT);
	}
	
	if(SDL_GetAppState()&SDL_APPACTIVE)
	{
		glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT|GL_STENCIL_BUFFER_BIT);		
		if(use_shadow_mapping)
		{
			glDisable(GL_TEXTURE_2D);
			ELglActiveTextureARB(shadow_unit);
			glDisable(depth_texture_target);
			disable_texgen();
			ELglActiveTextureARB(GL_TEXTURE0);
			glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
		glDisable(GL_LIGHTING);
		glDisable(GL_FOG);
		glEnable(GL_COLOR_MATERIAL);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixd(LongView);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

        if (!reflected) {
            glRotatef(rx,1.0,0.0,0.0);
            glRotatef(rz,0.0,0.0,1.0);
/*             glLightfv(GL_LIGHT7, GL_POSITION, vec4); */
/*             glLightfv(GL_LIGHT7, GL_DIFFUSE, vec_white); */
/*             glEnable(GL_LIGHT7); */
/*             glFlush(); */
            cloudy_sky(reflected);
			glColor4fv(fog[0]);
			glCallList(skyLists+1);
        }
        else
        {
			if (have_stencil) {
				cur_stencil = 1; //Draw horizon plane for sky reflection with this stencil
				glEnable(GL_STENCIL_TEST);
				glStencilFunc(GL_ALWAYS,cur_stencil,cur_stencil);
				glStencilOp(GL_KEEP,GL_REPLACE,GL_REPLACE);
				glColor4f(0.4,0.6,1.0,1.0);
				glCallList(skyLists+1);
				glPopMatrix();
				glPushMatrix();
				glStencilFunc(GL_EQUAL,cur_stencil,cur_stencil);
				glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
			}
            glRotatef(rx,1.0,0.0,0.0);
            glRotatef(rz,0.0,0.0,1.0);
            glScalef(1.0, 1.0, -1.0);
			glFrontFace(GL_CW);
			cloudy_sky(reflected);
			glFrontFace(GL_CCW);
        }

/* 		if(have_stencil&&reflect_sky) */
/* 		{ */
/* 			cur_stencil = 1; //Draw horizon plane for sky reflection with this stencil */
/* 			glEnable(GL_STENCIL_TEST); */
/* 			glStencilFunc(GL_ALWAYS,cur_stencil,cur_stencil); */
/* 			glStencilOp(GL_KEEP,GL_REPLACE,GL_REPLACE); */
/* 			glColor4f(0.4,0.6,1.0,1.0); */
/* 			glCallList(skyLists+1); */
/* 			glPopMatrix(); */
/* 			glPushMatrix(); */
/* 			glStencilFunc(GL_EQUAL,cur_stencil,cur_stencil); */
/* 			glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP); */
/* 			glRotatef(rx,1.0,0.0,0.0); */
/* 			glRotatef(rz,0.0,0.0,1.0); */
/* 			glScalef(1,1,-1); */
/* 			glFrontFace(GL_CW); */
/* 			glLightfv(GL_LIGHT7, GL_POSITION, vec4); */
/* 			cloudy_sky(); */
/* 			glFrontFace(GL_CCW); */
/* 		}//stencil */
/* 		else//nostencil = no reflection of sky */
/* 		{ */
/* 			glEnable(GL_LIGHTING); */
/* 			glEnable(GL_FOG); */
/* 			glColor4fv(fog[1]); */
/* 			//else glColor4f(0.4,0.6,1.0,1.0); */
/* 			//glColor4f(0.0,0.0,0.0,1.0); */
			
/* 			glCallList(skyLists+1); */
/* 		}//nostencil = no reflection of sky */
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glColor4f(1, 1,1,1);
		glDisable(GL_COLOR_MATERIAL);
		if(use_shadow_mapping)
		{
			last_texture=-1;
		}

		glPopAttrib();
	}
	if (have_stencil) {
		glClearStencil(cur_stencil);
	}
	glClear(GL_DEPTH_BUFFER_BIT);
	if(!is_day||dungeon)
	{
		switch(show_lights)
		{
		case 6:
			glEnable(GL_LIGHT6);
		case 5:
			glEnable(GL_LIGHT5);
		case 4:
			glEnable(GL_LIGHT4);
		case 3:
			glEnable(GL_LIGHT3);
		case 2:
			glEnable(GL_LIGHT2);
		case 1:
			glEnable(GL_LIGHT1);
		default:
			glEnable(GL_LIGHT0);
			break;
		}
	}
}



void init_sky()
{
	GLUquadricObj *qobj;
	GLfloat randx,randy,randz;
	int i;
	float maxr;

	clouds_tex = load_texture_cache("./textures/clouds.bmp",0);
	cloud_detail_tex = load_texture_cache("./textures/cloud_detail.bmp",0);
	smokey_cloud_tex = load_texture_cache("./textures/cloud_detail_alpha.bmp",0);
	moon_tex=load_texture_cache("./textures/moonmap.bmp",0);
	sun_tex=load_texture_cache("./textures/BrightSun.bmp",255);

	skyLists = glGenLists(6);
	qobj = gluNewQuadric();

	gluQuadricOrientation(qobj,GLU_OUTSIDE);	
	gluQuadricNormals(qobj,GLU_SMOOTH);
	gluQuadricTexture(qobj,GL_TRUE);
	glNewList(skyLists,GL_COMPILE);
	gluSphere(qobj,100,8,15);
	glEndList();
	glNewList(skyLists+3,GL_COMPILE);
	gluSphere(qobj,0.03,20,20);
	glEndList();
	glNewList(skyLists+1,GL_COMPILE);
	gluDisk(qobj,0,200,8,10);
	glEndList();
	srand(0);
	maxr = 1.0/RAND_MAX;
	for (i = 0; i < NUM_STARS; i++)
	{
		float norm;
		randx = rand()*(float)(maxr)-0.5f;
		randy = rand()* (float)(maxr)-0.5f;
		randz = rand()*(float)(maxr)-0.5f;
		norm = sqrt(randx*randx + randy*randy + randz*randz);
		if (norm > 0.5f) 
		{
			i--;
		}
		else
		{
			randx /= norm;
			randy /= norm;
			randz /= norm;
			randx *= 90.0;
			randy *= 90.0;
			randz *= 90.0;
			strs[i][0]=randx;
			strs[i][1]=randy;
			strs[i][2]=randz;
		}
	}
	glNewList(skyLists+2,GL_COMPILE);
	glBegin(GL_POINTS);
	for (i = 0; i < NUM_STARS/3; i++)
	{
		glVertex3fv(strs[i]);
	}
	glEnd();
	glEndList();
	glNewList(skyLists+4,GL_COMPILE);
	glBegin(GL_POINTS);
	for (i = 1*NUM_STARS/3; i < 2*NUM_STARS/3; i++)
	{
		glVertex3fv(strs[i]);
	}
	glEnd();
	glEndList();
	glNewList(skyLists+5,GL_COMPILE);
	glBegin(GL_POINTS);
	for (i = 2*NUM_STARS/3; i < NUM_STARS; i++)
	{
		glVertex3fv(strs[i]);
	}
	glEnd();
	glEndList();

	//set up fog blend vectors
	for (i = 0; i < 4; i++)
	{
		fog[i] = (float *) malloc (sizeof(float)*4);
	}

}


void init_skydisk(int tx, int ty){

	//building the area around the map to hide reflected sky
		//int tx=tile_map_size_x*3;
		//int ty=tile_map_size_y*3;
		int ox= tx/2;
		int oy= ty/2;
		int pp,diskpoints;
		float angle=0;
		float anglestep=45*M_PI/5/180;
		float zz=-0.4;
		float dist=ox;//*1.414;
		float kk[4]={0,5,10,20};
		for(diskpoints=0;diskpoints<SKYDISK_DIVS;diskpoints++){
			for(pp=0;pp<4;pp++){
				float xx=(pp) ? ((dist*1.414+pp*kk[pp])*cos(angle)):(ox);
				float yy=(pp) ? ((dist*1.414+pp*kk[pp])*sin(angle)):(ox*tan(angle));
				skydisk[0][diskpoints][pp][0]=ox+ xx;						
				skydisk[0][diskpoints][pp][1]=oy+ yy;	
				skydisk[0][diskpoints][pp][2]=zz;
				skydisk[4][diskpoints][pp][0]=ox- xx;						
				skydisk[4][diskpoints][pp][1]=oy- yy;	
				skydisk[4][diskpoints][pp][2]=zz;
				skydisk[7][diskpoints][pp][0]=ox+ xx;						
				skydisk[7][diskpoints][pp][1]=oy- yy;	
				skydisk[7][diskpoints][pp][2]=zz;
				skydisk[3][diskpoints][pp][0]=ox- xx;						
				skydisk[3][diskpoints][pp][1]=oy+ yy;	
				skydisk[3][diskpoints][pp][2]=zz;				
				SKYSWAP(skydisk[1][diskpoints][pp],skydisk[0][diskpoints][pp]);
				SKYSWAP(skydisk[5][diskpoints][pp],skydisk[4][diskpoints][pp]);
				SKYSWAP(skydisk[6][diskpoints][pp],skydisk[7][diskpoints][pp]);
				SKYSWAP(skydisk[2][diskpoints][pp],skydisk[3][diskpoints][pp]);
			}
			angle+=anglestep;
		}
}



#endif /* SKY_FPV_CURSOR */

#ifdef SKY_FPV_CURSOR

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <SDL.h>
#include <SDL_active.h>
#include "asc.h"
#include "sky.h"
#include "draw_scene.h"
#include "errors.h"
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

float skybox_clouds[360][4];
float skybox_clouds_detail[360][4];
float skybox_clouds_rain[360][4];
float skybox_clouds_detail_rain[360][4];
float skybox_sky1[360][4];
float skybox_sky2[360][4];
float skybox_sky3[360][4];
float skybox_sky4[360][4];
float skybox_sun[360][4];
float skybox_fog[360][4];
float skybox_fog_rain[360][4];
float skybox_light_ambient[360][4];
float skybox_light_diffuse[360][4];
float skybox_light_ambient_rain[360][4];
float skybox_light_diffuse_rain[360][4];
int skybox_no_clouds = 1;
int skybox_no_sun = 1;
int skybox_no_moons = 1;
int skybox_no_stars = 1;
int skybox_clouds_tex = -1;
int skybox_clouds_detail_tex = -1;

int skybox_show_sky = 1;
int skybox_show_clouds = 1;
int skybox_show_sun = 1;
int skybox_show_moons = 1;
int skybox_show_stars = 1;

float skybox_sun_position[4] = {0.0, 0.0, 0.0, 0.0};
double skybox_time_d = 0.0;
double skybox_view[16];

void (*display_sky)(int);

#ifdef NEW_WEATHER
#define fogColor rain_color
#endif //NEW_WEATHER

#define NUM_STARS 3000
float strs[NUM_STARS][3];
float fog1[4], fog2[4], fog3[4], fog4[4];
float *fog[4] = {fog1, fog2, fog3, fog4};
GLuint skyLists;
int smokey_cloud_tex;
int moon_tex;
int sun_tex;

//void simple_sky();
void cloudy_sky();
void animated_sky();

/*
  colorSkyCyl - local utility function.
  
  TODO: make this work on a predefined vertex array stack or something.

  Creates a colored cylinder, one color from the array per stack,
  smooth blending from one stack to the next. Stacks are 1.0 high each,
  cylinder is 1.0 radius. Stacks start a z = 0.0f and go to z = 1.0f * (stacks-1)
  
*/
void colorSkyCyl(int stacks, float *colors[4])
{
	int s,j;
	glBegin(GL_QUAD_STRIP);
	for (s = 0; s < stacks; s++)
	{
		for (j = 0; j < SLICES; j++)
		{
			glColor4fv(colors[s]);
			glVertex3f(slice_list[j][0],slice_list[j][1],s);
			glColor4fv(colors[s+1]);
			glVertex3f(slice_list[j][0],slice_list[j][1],s+1);
		}
	}
	glEnd();
}

void blend_colors(float result[], float orig[], float dest[], float t, int size)
{
    while (size--)
    {
        result[size] = (1.0-t)*orig[size] + t*dest[size];
    }
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
			display_sky = cloudy_sky;
	}
}

/*
void simple_sky()
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
		//glTranslatef(0.0, 0.0, -1.0);
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
		colorSkyCyl(3, fog);
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
*/

void cloudy_sky()
{
	static double spin = 0;
	float col1[4], col2[4], col3[4], col4[4];
	float *colors[4] = {col1, col2, col3, col4};
	float abs_light;
	float rain_coef;
	float day_alpha;
	int i;
	VECTOR4 vec_black={0.0, 0.0, 0.0, 0.0};

	// we compute the fog color
	for (i = 3; i-- ; )
		fog[0][i] = fog[1][i] = fog[2][i] = fog[3][i] = fogColor[i];
	fog[0][3] = 1.0;
	fog[1][3] = 0.75;
	fog[2][3] = 0.40;
	fog[3][3] = 0.0;
	
	abs_light = light_level;
	if(light_level > 59)
	{
		abs_light = 119 - light_level;
	}
	abs_light = 1.0f - abs_light/59.0f;

	spin = cur_time % (1296000*1000);
	spin*=360.0/(1296000.0/*seconds in large month*/ * 1000.0/*millisecond bump*/);
	spin += skybox_time_d;

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

	glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);

/* 	if(use_shadow_mapping) */
/* 	{ */
/* 		glDisable(GL_TEXTURE_2D); */
/* 		ELglActiveTextureARB(shadow_unit); */
/* 		glDisable(depth_texture_target); */
/* 		disable_texgen(); */
/* 		ELglActiveTextureARB(GL_TEXTURE0); */
/* 		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); */
/* 	} */
	
	glEnable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(skybox_view);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
    glTranslatef(tile_map_size_x*1.5, tile_map_size_y*1.5, 0.0);

#ifdef NEW_WEATHER
	if (weather_active() && weather_get_fadein_bias() <= 0.0 && weather_get_fadeout_bias() <= 0.0)
	{
		rain_coef = 1.0;
	}
	if(weather_get_fadeout_bias() < 1.0f && weather_get_fadeout_bias() > 0.0f)
	{
		rain_coef = weather_get_fadeout_bias();
	}
	else if(weather_get_fadein_bias() < 1.0f && weather_get_fadein_bias() > 0.0f)
	{
		rain_coef = 1.0 - weather_get_fadein_bias();
	}
	else
	{
		rain_coef = 0.0f;
	}
#else /* NEW_WATHER */
	rain_coef = weather_rain_intensity;
#endif // NEW_WEATHER
        
	if (rain_coef == 1.0) {
		// we draw the sky with the color of the fog
		glColor4fv(fog[0]);
		glCallList(skyLists);
	}
	else {
		// we draw the sky
		glColor4fv(skybox_sky4[game_minute]);
		glCallList(skyLists);
		
		// we draw the gradient at the horizon
		blend_colors(col1, skybox_sky1[game_minute], skybox_fog_rain[game_minute], rain_coef, 3);
		blend_colors(col2, skybox_sky2[game_minute], skybox_fog_rain[game_minute], rain_coef, 3);
		blend_colors(col3, skybox_sky3[game_minute], skybox_fog_rain[game_minute], rain_coef, 3);
		blend_colors(col4, skybox_sky4[game_minute], skybox_fog_rain[game_minute], rain_coef, 3);
		col1[3] = skybox_sky1[game_minute][3];
		col2[3] = skybox_sky2[game_minute][3];
		col3[3] = skybox_sky3[game_minute][3];
		col4[3] = skybox_sky4[game_minute][3];
		//colors[0] = col1; colors[1] = col2; colors[2] = col3; colors[3] = col4;
		glPushMatrix();
		glScalef(500.0, 500.0, 100.0);
		colorSkyCyl(3, colors);
		glPopMatrix();
	}
	
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	// Alpha adjustment for objects that should fade in daylight
	day_alpha = (1.0-abs_light)*(1.0-rain_coef);
	
	if (skybox_show_stars && !skybox_no_stars && day_alpha > 0.0)
	{
		glPushMatrix();
		glRotatef((float)game_minute, 0.0, -1.0, 0.0);
		glColor4f(1.0, 1.0, 1.0, day_alpha);
		glCallList(skyLists+2);
		glColor4f(1.0, 1.0, 1.0, day_alpha*0.5);
		glCallList(skyLists+4);
		glColor4f(1.0, 1.0, 1.0, day_alpha*0.25);
		glCallList(skyLists+5);
		glPopMatrix();
	}
	
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_COLOR, GL_ONE);

	// Rotate sky for time of day
	if(skybox_show_moons && !skybox_no_moons && day_alpha > 0.0)
	{
		VECTOR4 vec_moon = {day_alpha*0.5 + 0.5,
							day_alpha*0.5 + 0.5,
							day_alpha*0.5 + 0.5,
							1.0};
		
		// the current light color is black so we change it to light the moons
		VECTOR4 vec_light = {0.8, 0.8, 0.8, 1.0};
		glLightfv(GL_LIGHT7, GL_DIFFUSE, vec_light);
		
		glPushMatrix();
		glPushAttrib(GL_ENABLE_BIT);
		glRotatef((float)game_minute, 0.0, -1.0, 0.0);
		
		glEnable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		get_and_set_texture_id(moon_tex);
		glMaterialfv(GL_FRONT, GL_AMBIENT, vec_black);
		
		vec_moon[1] /= 1.2f;
		vec_moon[2] /= 1.5f;
		
		glMaterialfv(GL_FRONT, GL_DIFFUSE, vec_moon);
		
		glPushMatrix();
		glRotatef(20.0, 0.0, 0.0, 1.0);
		glRotatef(10.0*spin, 0.0, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 450.0);
		glScalef(.5,.5,.5);
		glCallList(skyLists+3);
		glPopMatrix();
		
		vec_moon[1] *= 1.2f;
		vec_moon[2] *= 1.5f;
		
		glMaterialfv(GL_FRONT, GL_DIFFUSE, vec_moon);
		
		glPushMatrix();
		glRotatef(spin, 0.0, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 400.0);
		glRotatef(spin-80.0, 0.0, 1.0, 0.0);
		glCallList(skyLists+3);
		glPopMatrix();
		
		glPopAttrib();
		glPopMatrix();
		glLightfv(GL_LIGHT7, GL_DIFFUSE, diffuse_light); // we restore the light color
	}
	
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (skybox_show_clouds && !skybox_no_clouds)
	{
		blend_colors(col1, skybox_clouds[game_minute], skybox_clouds_rain[game_minute], rain_coef, 4);
		blend_colors(col2, skybox_clouds_detail[game_minute], skybox_clouds_detail_rain[game_minute], rain_coef, 4);
		glPushMatrix();
		glTranslatef(0.0, 0.0, -0.5);
		glScalef(1.0, 1.0, 0.17);
		glRotatef(90.0, 1.0, 0.0, 0.0);
		glRotatef(spin*1000, 0.0, 0.0, 1.0);
		glColor4fv(col1);
		get_and_set_texture_id(skybox_clouds_tex);
		glCallList(skyLists);
		glPopMatrix();

		glPushMatrix();
		glScalef(1.0, 1.0, 0.165);
		glRotatef(90.0, 1.0, 0.0, 0.0);
		glRotatef(spin*1000, 0.0, 0.0, 1.0);
		glColor4fv(col2);
		get_and_set_texture_id(skybox_clouds_detail_tex);
		glCallList(skyLists);
		glPopMatrix();

		if (rain_coef > 0.0)
		{
			col1[3] = col2[3] = rain_coef;
			glPushMatrix();
			glScalef(-1.0, 1.0, 0.17);
			glRotatef(90.0, 1.0, 0.0, 0.0);
			glRotatef(spin*4000, 0.0, 0.0, 1.0);
			glColor4fv(col1);
			get_and_set_texture_id(skybox_clouds_tex);
			glCallList(skyLists);
			glPopMatrix();
			
			glPushMatrix();
			glScalef(1.0, 1.0, 0.16);
			glRotatef(90.0, 1.0, 0.0, 0.0);
			glRotatef(spin*4000, 0.0, 0.0, 1.0);
			glColor4fv(col2);
			get_and_set_texture_id(skybox_clouds_detail_tex);
			glCallList(skyLists);
			glPopMatrix();
		}
	}

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);

	// we draw the fog at the horizon
	glPushMatrix();
	glTranslatef(0.0, 0.0, -0.1);
	glScalef(500.0, 500.0, 20.0);
	colorSkyCyl(3, fog);
	glPopMatrix();
    
    // we draw a disk at the level of the ground
    glColor4fv(fog[0]);
    glCallList(skyLists+1);

	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_COLOR, GL_ONE);

	// draw the sun
	if (skybox_show_sun && !skybox_no_sun &&
		(skybox_sun_position[0] != 0.0 ||
		 skybox_sun_position[1] != 0.0 ||
		 skybox_sun_position[2] != 0.0))
	{
		blend_colors(col1, skybox_sun[game_minute], vec_black, rain_coef, 4);
		glColor4fv(col1);
		get_and_set_texture_id(sun_tex);
		
		glPushMatrix();
		glScalef(480.0, 480.0, 480.0);
		glBegin(GL_QUADS);
		{
			//two cross products. Dangerous to use these functions. Float type essential.
			//Better to robustly produce two vectors in perpendicular plane elsewhere.
			VECTOR3 perp1, perp2, someVec = {0.0, 1.0, 0.0};
			VCross(perp1, someVec, skybox_sun_position);
			VCross(perp2, perp1, skybox_sun_position);
			glTexCoord2f(0,0);
			glVertex3f(skybox_sun_position[0]+.08*(perp1[0]+perp2[0]),
					   skybox_sun_position[1]+.08*(perp1[1]+perp2[1]),
					   skybox_sun_position[2]+.08*(perp1[2]+perp2[2]));
			glTexCoord2f(1,0);
			glVertex3f(skybox_sun_position[0]+.08*(-perp1[0]+perp2[0]),
					   skybox_sun_position[1]+.08*(-perp1[1]+perp2[1]),
					   skybox_sun_position[2]+.08*(-perp1[2]+perp2[2]));
			glTexCoord2f(1,1);
			glVertex3f(skybox_sun_position[0]+.08*(-perp1[0]-perp2[0]),
					   skybox_sun_position[1]+.08*(-perp1[1]-perp2[1]),
					   skybox_sun_position[2]+.08*(-perp1[2]-perp2[2]));
			glTexCoord2f(0,1);
			glVertex3f(skybox_sun_position[0]+.08*(perp1[0]-perp2[0]),
					   skybox_sun_position[1]+.08*(perp1[1]-perp2[1]),
					   skybox_sun_position[2]+.08*(perp1[2]-perp2[2]));
		}
		glEnd();
		glPopMatrix();
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

/* 	if(use_shadow_mapping) */
/* 	{ */
/* 		last_texture=-1; */
/* 	} */

	reset_material();

	glClear(GL_DEPTH_BUFFER_BIT);

    // we restore the lights
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

void animated_sky()
{
	static float spin = 0;
	float abs_light;

	abs_light=light_level;
	if(light_level>59)abs_light=119-light_level;
	abs_light = 1.0f-abs_light/59.0f;

	spin = cur_time%250000;
	spin*=360.0f/250000.0f;

	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixd(skybox_view);
	glMatrixMode(GL_MODELVIEW);

	glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
	glTranslatef(tile_map_size_x*1.5, tile_map_size_y*1.5, -40.0);

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
	//glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ZERO);
	
	glColor4f(0.0,0.0,0.0,0.0);
	glCallList(skyLists);
	
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	get_and_set_texture_id(smokey_cloud_tex);
	glPushMatrix();
	glScalef(1.0, 1.0, 0.17);
	glPushMatrix();
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glRotatef(spin, 0.0, 0.0, 1.0);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(spin, 0.0, 0.0, 1.0);
	glCallList(skyLists);
	glPopMatrix();

	glPushMatrix();
	glColor4f(1.0, 0.2, 0.0, 1.0);
	glRotatef(spin*2, 0.0, 0.0, 1.0);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(spin*2, 0.0, 0.0, 1.0);
	glCallList(skyLists);
	glPopMatrix();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);

	fog[0][0] = 1.0;
	fog[0][1] = 0.7;
	fog[0][2] = 0.0;
	fog[0][3] = 1.0;

	fog[1][0] = 1.0;
	fog[1][1] = 0.5;
	fog[1][2] = 0.0;
	fog[1][3] = 0.66;

	fog[2][0] = 1.0;
	fog[2][1] = 0.25;
	fog[2][2] = 0.0;
	fog[2][3] = 0.33;

	fog[3][0] = 1.0;
	fog[3][1] = 0.0;
	fog[3][2] = 0.0;
	fog[3][3] = 0.0;
	
	glPushMatrix();
	glTranslatef(0.0, 0.0, -0.5);
	glScalef(500.0, 500.0, 60.0);
	colorSkyCyl(3, fog);
	glPopMatrix();

	glColor4fv(fog[0]);
	glCallList(skyLists+1);
	
	glPopAttrib();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glColor4f(1.0, 1.0, 1.0, 1.0);
	if(use_shadow_mapping)
	{
		last_texture=-1;
	}
}

#define XML_BOOL(s) (!xmlStrcasecmp((s), (xmlChar*)"yes") ||\
					 !xmlStrcasecmp((s), (xmlChar*)"true") ||\
					 !xmlStrcasecmp((s), (xmlChar*)"1"))

int skybox_parse_color_properties(xmlNode *node, float container[360][4])
{
	xmlAttr *attr;
	int t = -1;
	float r = 0.0, g = 0.0, b = 0.0, a = 1.0;
	int ok = 1;

	for (attr = node->properties; attr; attr = attr->next)
	{
		if (attr->type == XML_ATTRIBUTE_NODE)
		{
			if (xmlStrcasecmp (attr->name, (xmlChar*)"t") == 0)
				t =  atoi((char*)attr->children->content);
			else if (xmlStrcasecmp (attr->name, (xmlChar*)"r") == 0)
				r =  atof((char*)attr->children->content);
			else if (xmlStrcasecmp (attr->name, (xmlChar*)"g") == 0)
				g =  atof((char*)attr->children->content);
			else if (xmlStrcasecmp (attr->name, (xmlChar*)"b") == 0)
				b =  atof((char*)attr->children->content);
			else if (xmlStrcasecmp (attr->name, (xmlChar*)"a") == 0)
				a =  atof((char*)attr->children->content);
			else {
				LOG_ERROR("unknown attribute for color: %s", (char*)attr->name);
				ok = 0;
			}
		}
	}

	if (t >= 0 && t < 360)
	{
		container[t][0] = r <= 1.0 ? r : r / 255.0;
		container[t][1] = g <= 1.0 ? g : g / 255.0;
		container[t][2] = b <= 1.0 ? b : b / 255.0;
		container[t][3] = a <= 1.0 ? a : a / 255.0;
	}
	else
	{
		LOG_ERROR("the time attribute of the color doesn't exist or is wrong!");
		ok = 0;
	}

	return ok;
}

int skybox_parse_colors(xmlNode *node, float container[360][4])
{
	xmlNode	*item;
	xmlAttr *attr;
	int	ok = 1;
    int i, t;
    int reset = 0;

	if(node == NULL || node->children == NULL) return 0;

	for (attr = node->properties; attr; attr = attr->next) {
		if (attr->type == XML_ATTRIBUTE_NODE &&
			(!xmlStrcasecmp(attr->name, (xmlChar*)"reset") ||
			 !xmlStrcasecmp(attr->name, (xmlChar*)"overwrite"))) {
			reset = XML_BOOL(attr->children->content);
		}
		else {
			LOG_ERROR("unknown attribute for element: %s", (char*)attr->name);
			ok = 0;
		}
	}

    if (reset)
    {
        // we erase the previous color keys
        for (t = 360; t--; )
        {
            for (i = 3; i--; )
            {
                container[t][i] = 0.0;
            }
            container[t][3] = -1.0;
        }
    }
        
	for(item = node->children; item; item = item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, (xmlChar*)"color") == 0) {
				ok &= skybox_parse_color_properties(item, container);
			}
			else {
				LOG_ERROR("unknown node for element: %s", item->name);
				ok = 0;
			}
		}
		else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= skybox_parse_colors(item->children, container);
		}
	}

	return ok;
}

int skybox_parse_properties(xmlNode *node)
{
	xmlNode *item;
	xmlAttr *attr;
	int ok = 1;

	for(item = node->children; item; item = item->next) {
		if(item->type == XML_ELEMENT_NODE) {
			if(xmlStrcasecmp(item->name, (xmlChar*)"clouds") == 0) {
				for (attr = item->properties; attr; attr = attr->next)
				{
					if (attr->type == XML_ATTRIBUTE_NODE)
					{
						if (!xmlStrcasecmp (attr->name, (xmlChar*)"show"))
							skybox_no_clouds = !XML_BOOL(attr->children->content);
						else if (xmlStrcasecmp (attr->name, (xmlChar*)"texture") == 0)
							skybox_clouds_tex = load_texture_cache((char*)attr->children->content, 0);
						else if (xmlStrcasecmp (attr->name, (xmlChar*)"texture_detail") == 0)
							skybox_clouds_detail_tex = load_texture_cache((char*)attr->children->content, 0);
						else {
							LOG_ERROR("unknown attribute for clouds: %s", (char*)attr->name);
							ok = 0;
						}
					}
				}
			}
			else if(xmlStrcasecmp(item->name, (xmlChar*)"sun") == 0) {
				for (attr = item->properties; attr; attr = attr->next)
					if (attr->type == XML_ATTRIBUTE_NODE) {
						if (!xmlStrcasecmp (attr->name, (xmlChar*)"show"))
							skybox_no_sun = !XML_BOOL(attr->children->content);
						else {
							LOG_ERROR("unknown attribute for sun: %s", (char*)attr->name);
							ok = 0;
						}
					}
			}
			else if(xmlStrcasecmp(item->name, (xmlChar*)"moons") == 0) {
				for (attr = item->properties; attr; attr = attr->next)
					if (attr->type == XML_ATTRIBUTE_NODE) {
						if (!xmlStrcasecmp (attr->name, (xmlChar*)"show"))
							skybox_no_moons = !XML_BOOL(attr->children->content);
						else {
							LOG_ERROR("unknown attribute for moons: %s", (char*)attr->name);
							ok = 0;
						}
					}
			}
			else if(xmlStrcasecmp(item->name, (xmlChar*)"stars") == 0) {
				for (attr = item->properties; attr; attr = attr->next)
					if (attr->type == XML_ATTRIBUTE_NODE) {
						if (!xmlStrcasecmp (attr->name, (xmlChar*)"show"))
							skybox_no_stars = !XML_BOOL(attr->children->content);
						else {
							LOG_ERROR("unknown attribute for stars: %s", (char*)attr->name);
							ok = 0;
						}
					}
			}
			else {
				LOG_ERROR("unknown node for properties: %s", item->name);
				ok = 0;
			}
		}
		else if (item->type == XML_ENTITY_REF_NODE) {
			ok &= skybox_parse_properties(item->children);
		}
	}

	return ok;
}

int skybox_parse_defs(xmlNode *node, const char *map_name)
{
	xmlNode *def;
	int ok = 1;

	for (def = node->children; def; def = def->next) {
		if (def->type == XML_ELEMENT_NODE)
			if (xmlStrcasecmp(def->name, (xmlChar*)"properties") == 0) {
				ok &= skybox_parse_properties(def);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"clouds") == 0) {
				ok &= skybox_parse_colors(def, skybox_clouds);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"clouds_detail") == 0) {
				ok &= skybox_parse_colors(def, skybox_clouds_detail);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"clouds_rain") == 0) {
				ok &= skybox_parse_colors(def, skybox_clouds_rain);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"clouds_detail_rain") == 0) {
				ok &= skybox_parse_colors(def, skybox_clouds_detail_rain);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky1") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky1);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky2") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky2);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky3") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky3);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sky4") == 0) {
				ok &= skybox_parse_colors(def, skybox_sky4);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"sun") == 0) {
				ok &= skybox_parse_colors(def, skybox_sun);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"fog") == 0) {
				ok &= skybox_parse_colors(def, skybox_fog);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"fog_rain") == 0) {
				ok &= skybox_parse_colors(def, skybox_fog_rain);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"light_ambient") == 0) {
				ok &= skybox_parse_colors(def, skybox_light_ambient);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"light_diffuse") == 0) {
				ok &= skybox_parse_colors(def, skybox_light_diffuse);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"light_ambient_rain") == 0) {
				ok &= skybox_parse_colors(def, skybox_light_ambient_rain);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"light_diffuse_rain") == 0) {
				ok &= skybox_parse_colors(def, skybox_light_diffuse_rain);
			}
			else if (xmlStrcasecmp(def->name, (xmlChar*)"map") == 0) {
                char *name = get_string_property(def, "name");
                if (!strcasecmp(name, map_name)) {
                    printf("Found custom sky defs for the current map!\n");
                    ok &= skybox_parse_defs(def, "");
                }
			}
			else {
				LOG_ERROR("unknown element for skybox: %s", def->name);
				ok = 0;
			}
		else if (def->type == XML_ENTITY_REF_NODE) {
			ok &= skybox_parse_defs(def->children, map_name);
		}
	}

	return ok;
}

int skybox_read_defs(const char *file_name, const char *map_name)
{
	xmlNode *root;
	xmlDoc *doc;
	int ok = 1;

	doc = xmlReadFile(file_name, NULL, 0);
	if (doc == NULL) {
		LOG_ERROR("Unable to read skybox definition file %s", file_name);
		return 0;
	}

	root = xmlDocGetRootElement(doc);
	if (root == NULL) {
		LOG_ERROR("Unable to parse skybox definition file %s", file_name);
		ok = 0;
	} else if (xmlStrcasecmp(root->name, (xmlChar*)"skybox") != 0) {
		LOG_ERROR("Unknown key \"%s\" (\"skybox\" expected).", root->name);
		ok = 0;
	} else {
		ok = skybox_parse_defs(root, map_name);
	}

	xmlFreeDoc(doc);
	return ok;
}

int skybox_build_gradients(float container[360][4])
{
	int t;
	int first = 0;
	int prev, next;

	while (first < 360 && container[first][3] < 0.0) ++first;

	if (first >= 360) return 0;

	prev = first;
	do
	{
		next = (prev+1)%360;
		while (container[next][3] < 0.0) next = (next+1)%360;
		t = prev;
		do
		{
			int diff = (next-prev+360)%360;
/* 			int i; */
/* 			for (i = 4; i--; ) */
/* 				container[t][i] = (container[prev][i] * ((next-t+360)%360) / diff + */
/* 								   container[next][i] * ((t-prev+360)%360) / diff); */
			if (!diff) // prev == next !
				memcpy(container[t], container[prev], 4*sizeof(float));
			else
				blend_colors(container[t], container[prev], container[next],
							 (float)((t-prev+360)%360)/(float)diff, 4);
			t = (t+1)%360;
		}
		while (t != next);
		prev = next;
	}
	while (prev != first);
	
	return 1;
}

void skybox_init_defs(const char *map_name)
{
    static char last_map[256] = "\0";
	int t;
	int i;

	for (t = 360; t--; )
	{
		for (i = 3; i--; )
		{
			skybox_clouds[t][i] = 0.0;
			skybox_clouds_detail[t][i] = 0.0;
			skybox_clouds_rain[t][i] = 0.0;
			skybox_clouds_detail_rain[t][i] = 0.0;
			skybox_sky1[t][i] = 0.0;
			skybox_sky2[t][i] = 0.0;
			skybox_sky3[t][i] = 0.0;
			skybox_sky4[t][i] = 0.0;
			skybox_sun[t][i] = 0.0;
			skybox_fog[t][i] = 0.0;
			skybox_fog_rain[t][i] = 0.0;
			skybox_light_ambient[t][i] = 0.0;
			skybox_light_diffuse[t][i] = 0.0;
			skybox_light_ambient_rain[t][i] = 0.0;
			skybox_light_diffuse_rain[t][i] = 0.0;
		}
		skybox_clouds[t][3] = -1.0;
		skybox_clouds_detail[t][3] = -1.0;
		skybox_clouds_rain[t][3] = -1.0;
		skybox_clouds_detail_rain[t][3] = -1.0;
		skybox_sky1[t][3] = -1.0;
		skybox_sky2[t][3] = -1.0;
		skybox_sky3[t][3] = -1.0;
		skybox_sky4[t][3] = -1.0;
		skybox_sun[t][3] = -1.0;
		skybox_fog[t][3] = -1.0;
		skybox_fog_rain[t][3] = -1.0;
		skybox_light_ambient[t][3] = -1.0;
		skybox_light_diffuse[t][3] = -1.0;
		skybox_light_ambient_rain[t][3] = -1.0;
		skybox_light_diffuse_rain[t][3] = -1.0;
	}

	skybox_no_clouds = 1;
	skybox_no_sun = 1;
	skybox_no_moons = 1;
	skybox_no_stars = 1;
	skybox_clouds_tex = -1;
	skybox_clouds_detail_tex = -1;

    if (map_name) {
        int pos = strlen(map_name)-1;
        while (pos >= 0 && map_name[pos] != '/') --pos;
        strcpy(last_map, map_name+pos+1);
    }

    printf("Loading sky defs for map '%s'\n", last_map);
	if (!skybox_read_defs("skybox/skybox_defs.xml", last_map))
		printf("Error while loading the skybox definitions, check the error log for more details.");
	
	if (!skybox_build_gradients(skybox_clouds))
		LOG_ERROR("no color key defined for 'clouds' element!");
	if (!skybox_build_gradients(skybox_clouds_detail))
		LOG_ERROR("no color key defined for 'clouds_detail' element!");
	if (!skybox_build_gradients(skybox_clouds_rain))
		LOG_ERROR("no color key defined for 'clouds_rain' element!");
	if (!skybox_build_gradients(skybox_clouds_detail_rain))
		LOG_ERROR("no color key defined for 'clouds_detail_rain' element!");
	if (!skybox_build_gradients(skybox_sky1))
		LOG_ERROR("no color key defined for 'sky1' element!");
	if (!skybox_build_gradients(skybox_sky2))
		LOG_ERROR("no color key defined for 'sky2' element!");
	if (!skybox_build_gradients(skybox_sky3))
		LOG_ERROR("no color key defined for 'sky3' element!");
	if (!skybox_build_gradients(skybox_sky4))
		LOG_ERROR("no color key defined for 'sky4' element!");
	if (!skybox_build_gradients(skybox_sun))
		LOG_ERROR("no color key defined for 'sun' element!");
	if (!skybox_build_gradients(skybox_fog))
		LOG_ERROR("no color key defined for 'fog' element!");
	if (!skybox_build_gradients(skybox_fog_rain))
		LOG_ERROR("no color key defined for 'fog_rain' element!");
	if (!skybox_build_gradients(skybox_light_ambient))
		LOG_ERROR("no color key defined for 'light_ambient' element!");
	if (!skybox_build_gradients(skybox_light_diffuse))
		LOG_ERROR("no color key defined for 'light_diffuse' element!");
	if (!skybox_build_gradients(skybox_light_ambient_rain))
		LOG_ERROR("no color key defined for 'light_ambient_rain' element!");
	if (!skybox_build_gradients(skybox_light_diffuse_rain))
		LOG_ERROR("no color key defined for 'light_diffuse_rain' element!");
}

void skybox_init_gl()
{
	GLUquadricObj *qobj;
	GLfloat randx,randy,randz;
	int i;
	float maxr;

	smokey_cloud_tex = load_texture_cache("./textures/cloud_detail_alpha.bmp", 0);
	moon_tex=load_texture_cache("./textures/moonmap.bmp", 0);
	sun_tex=load_texture_cache("./textures/BrightSun.bmp", 255);

	skyLists = glGenLists(6);
	qobj = gluNewQuadric();

	gluQuadricOrientation(qobj, GLU_INSIDE);	
	gluQuadricNormals(qobj, GLU_SMOOTH);
	gluQuadricTexture(qobj, GL_TRUE);
	glNewList(skyLists, GL_COMPILE);
	gluSphere(qobj, 500, 32, 16);
	glEndList();
	glNewList(skyLists+3, GL_COMPILE);
	gluSphere(qobj, 20, 32, 16);
	glEndList();
	gluQuadricOrientation(qobj, GLU_OUTSIDE);	
	glNewList(skyLists+1, GL_COMPILE);
	gluDisk(qobj, 0, 500, 20, 1);
	glEndList();
	gluDeleteQuadric(qobj);

	srand(0);
	maxr = 1.0/RAND_MAX;
	for (i = 0; i < NUM_STARS; i++)
	{
		float norm;
		randx = rand()*(float)(maxr)-0.5f;
		randy = rand()*(float)(maxr)-0.5f;
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
			randx *= 490.0;
			randy *= 490.0;
			randz *= 490.0;
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
}

#endif /* SKY_FPV_CURSOR */

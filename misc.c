#include <stdlib.h>
#include <math.h>
#ifdef PNG_SCREENSHOT
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <png.h>
#endif
#include "global.h"

#define IMG_SetError(a) SDL_SetError(a)

Uint8 last_pixel_color[4];

void reset_under_the_mouse()
	{
		if(!read_mouse_now)return;
		last_pixel_color[0]=0;
		last_pixel_color[1]=0;
		last_pixel_color[2]=0;
		object_under_mouse=-1;
		thing_under_the_mouse=UNDER_MOUSE_NOTHING;
	}

inline float SwapFloat(float t)
{
	union {
		float f;
		int i;
	} intOrFloat;
	intOrFloat.f = t;
	intOrFloat.i = SDL_Swap32(intOrFloat.i);
	return intOrFloat.f;
	/*
	int ftemp = SDL_Swap32(*((int*)(&t)));
	return *((float*)(&ftemp));
	*/
}

int anything_under_the_mouse(int object_id, int object_type)
{
	char pixels[16]={0};

	if(!read_mouse_now)return 0;
	glReadBuffer(GL_BACK);
	glReadPixels(mouse_x, window_height-mouse_y, 1, 1, GL_RGB, GL_BYTE, &pixels);
	if(pixels[0]!=last_pixel_color[0] || pixels[1]!=last_pixel_color[1] || pixels[2]!=last_pixel_color[2])
		{
			last_pixel_color[0]=pixels[0];
			last_pixel_color[1]=pixels[1];
			last_pixel_color[2]=pixels[2];

			if(object_type==UNDER_MOUSE_NO_CHANGE)return 0;

			if(object_type==UNDER_MOUSE_PLAYER || object_type==UNDER_MOUSE_NPC || object_type==UNDER_MOUSE_ANIMAL)
				{
					actor_under_mouse=actors_list[object_id];
					object_id=actors_list[object_id]->actor_id;
				} else {
					actor_under_mouse=NULL;
				}
			object_under_mouse=object_id;

			thing_under_the_mouse=object_type;
			return 1;//there is something
		}
	return 0;//no collision, sorry

}

static GLfloat  model[16];
static GLfloat  proj[16];
static GLint    viewport[4];

void save_scene_matrix()
{
	glGetFloatv(GL_MODELVIEW_MATRIX, model);
	glGetFloatv(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, viewport);
	viewport[2]>>=1;
	viewport[3]>>=1;
}

void project_ortho(GLfloat ox, GLfloat oy, GLfloat oz, GLfloat * wx, GLfloat * wy)
{
	GLfloat tmp[3];

	// apply modelview matrix
	tmp[0] = model[0*4+0] * ox + model[1*4+0] * oy + model[2*4+0] * oz + model[3*4+0];
	tmp[1] = model[0*4+1] * ox + model[1*4+1] * oy + model[2*4+1] * oz + model[3*4+1];
	tmp[2] = model[0*4+2] * ox + model[1*4+2] * oy + model[2*4+2] * oz + model[3*4+2];
	
	// apply projection matrix
	ox = proj[0*4+0] * tmp[0] + proj[3*4+0];
	oy = proj[1*4+1] * tmp[1] + proj[3*4+1];
	oz = proj[2*4+2] * tmp[2] + proj[3*4+2];
	
	// viewport
	*wx = viewport[0] + (1 + ox) * viewport[2];
	*wy = viewport[1] + (1 + oy) * viewport[3];
}

void unproject_ortho(GLfloat wx,GLfloat wy,GLfloat wz,GLfloat *ox,GLfloat *oy,GLfloat *oz)
{
	GLfloat tmp[3];

	// Inverse viewport
	tmp[0]=(wx-viewport[0])/(float)viewport[2]-1.0f;
	tmp[1]=(wy-viewport[1])/(float)viewport[3]-1.0f;
	tmp[2]=2.0f*wz-1.0f;

	// Inverse projection
	tmp[0]=(tmp[0]-proj[3*4+0])/proj[0*4+0];
	tmp[1]=(tmp[1]-proj[3*4+1])/proj[1*4+1];
	tmp[2]=(tmp[2]-proj[3*4+2])/proj[2*4+2];

	// Inverse modelview
	tmp[0]-=model[3*4+0];
	tmp[1]-=model[3*4+1];
	tmp[2]-=model[3*4+2];
	*ox = model[0*4+0]*tmp[0]+model[0*4+1]*tmp[1]+model[0*4+2]*tmp[2];
	*oy = model[1*4+0]*tmp[0]+model[1*4+1]*tmp[1]+model[1*4+2]*tmp[2];
	*oz = model[2*4+0]*tmp[0]+model[2*4+1]*tmp[1]+model[2*4+2]*tmp[2];
}

int mouse_in_sphere(float x, float y, float z, float radius)
{
	GLfloat  winx, winy;
	int m_y = window_height - mouse_y;
	
	project_ortho(x, y, z, &winx, &winy);
	
	radius = proj[0*4+0]*radius+proj[3*4+0];
	radius = (1.0+radius)*viewport[2];
	
	return (mouse_x >= winx - radius  &&  mouse_x <= winx + radius  &&
			m_y     >= winy - radius  &&  m_y     <= winy + radius);
}

void find_last_url(char * source_string, int len)
{
	Uint8 cur_char;
	int i,j,url_start;
	int last_url_start=0;
	int final_url_start=0;
	int final_url_2=0;
	int www=1;

	while(1)
		{
			url_start=get_string_occurance("www.",source_string+final_url_start+1,len-last_url_start,1);
			if(url_start==-1)break;
			if(final_url_start<url_start+last_url_start)final_url_start=url_start+last_url_start;
			last_url_start+=url_start;
		}

	last_url_start=0;
	while(1)
		{
			url_start=get_string_occurance("http://",source_string+final_url_2+1,len-last_url_start,1);
			if(url_start<0)
			url_start=get_string_occurance("https://",source_string+final_url_2+1,len-last_url_start,1);
			if(url_start<0)
			url_start=get_string_occurance("ftp://",source_string+final_url_2+1,len-last_url_start,1);
			if(url_start<0)break;
			if(final_url_2<url_start+last_url_start)final_url_2=url_start+last_url_start;
			last_url_start+=url_start;
		}


	if(!final_url_start && !final_url_2)return;//no URL found

	//see if the last url was the www or http:// one
	if(final_url_2>final_url_start)
		{
			final_url_start=final_url_2;
			www=0;//means the URL starts with http
		}


	//ok, we have an URL, now get it
	j=0;
	if(www)
		{
			my_strcp(current_url,"http://");
			j=7;
		}

	for(i=final_url_start;i<len;i++)
		{
			if(j>158)return;//URL too long, perhaps an exploit attempt
			cur_char=source_string[i];
			if(!cur_char || cur_char==' ' || cur_char==0x0a || cur_char=='<'
			|| cur_char=='>' || cur_char=='|' || cur_char=='"' || cur_char==']')break;
			current_url[j]=cur_char;
			j++;
		}
	current_url[j]=0;
	have_url=1;


}


int go_to_url(void *dummy)
{

	char browser_command[400];

	if(!have_url)return 0;

	snprintf (browser_command, sizeof (browser_command), "%s \"%s\"", browser_name, current_url), 
	system(browser_command);

	return 0;
}

FILE *my_fopen (const char *fname, const char *mode)
{
	FILE *file = fopen (fname, mode);
	if (file == NULL)
	{
		LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, cant_open_file, fname);
	}
	return file;
}

#ifdef PNG_SCREENSHOT
/* Save a PNG type image to an SDL datasource */
static void png_write_data(png_structp ctx, png_bytep area, png_size_t size)
{
	SDL_RWops *src;
	
	src = (SDL_RWops *) png_get_io_ptr (ctx);
	SDL_RWwrite(src, area, size, 1);
}

static void png_io_flush (png_structp ctx)
{
	SDL_RWops *src;
	
	src = (SDL_RWops *) png_get_io_ptr(ctx);
	/* how do I flush src? */
}

static int png_colortype_from_surface(SDL_Surface *surface)
{
	int colortype = PNG_COLOR_MASK_COLOR; /* grayscale not supported */
	
	if (surface->format->palette)
		colortype |= PNG_COLOR_MASK_PALETTE;
	else if (surface->format->Amask)
		colortype |= PNG_COLOR_MASK_ALPHA;
        
	return colortype;
}

static void png_user_warn (png_structp ctx, png_const_charp str)
{
	//fprintf(stderr, "libpng: warning: %s\n", str);
	LOG_ERROR("libpng: warning: %s\n", str);
}

static void png_user_error(png_structp ctx, png_const_charp str)
{
	//fprintf(stderr, "libpng: error: %s\n", str);
	LOG_ERROR("libpng: error: %s\n", str);
}

int IMG_SavePNG_RW (SDL_Surface *face, SDL_RWops *src)
{
	png_structp png_ptr = 0;
	png_infop info_ptr = 0;
	//png_colorp palette = 0;
	png_bytep *row_pointers = 0;
	int i;
	int colortype;
	int result = -1;
        
	png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, png_user_error, png_user_warn);
	
	if (png_ptr == NULL)
	{
		IMG_SetError ("Couldn't allocate memory for PNG file");
		return -1;
	}
	
	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		IMG_SetError("Couldn't create image information for PNG file");
		goto done;
	}
	
	/* Set error handling. */
	if (setjmp(png_ptr->jmpbuf))
	{
		/* If we get here, we had a problem reading the file */
		IMG_SetError("Error writing the PNG file");
		goto done;
	}
	
	png_set_write_fn (png_ptr, src, png_write_data, png_io_flush);
	
	/* Set the image information here.  Width and height are up to 2^31,
	 * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also 
	 * depend on the color_type selected. color_type is one of 
	 * PNG_COLOR_TYPE_GRAY, PNG_COLOR_TYPE_GRAY_ALPHA, 
	 * PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB, or 
	 * PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
	 * PNG_INTERLACE_ADAM7, and the compression_type and filter_type 
	 * MUST currently be PNG_COMPRESSION_TYPE_BASE and  
	 * PNG_FILTER_TYPE_BASE. REQUIRED
	 */
	colortype = png_colortype_from_surface (face);
	png_set_IHDR (png_ptr, info_ptr, face->w, face->h, 8, colortype, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
	/* Write the file header information.  REQUIRED */
	png_write_info (png_ptr, info_ptr);
	
	/* pack pixels into bytes */
	png_set_packing (png_ptr);
	
	/* Create the array of pointers to image data */
	row_pointers = (png_bytep*) malloc (sizeof(png_bytep)*face->h);
	
	if ( (row_pointers == NULL) ) 
	{
		IMG_SetError("Couldn't allocate PNG row pointers");
		goto done;
	}
	
	for (i = 0; i < face->h; i++)
		row_pointers[i] = (png_bytep)(Uint8 *)face->pixels + i*face->pitch;
	
	/* write out the entire image data in one call */
	png_write_image (png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);
	result = 0;  /* success! */
	
done:
	if (row_pointers)
		free (row_pointers);
	
	if (info_ptr->palette)
		free (info_ptr->palette);
	
	png_destroy_write_struct (&png_ptr, (png_infopp)NULL);
	
	return result;
}

int IMG_SavePNG (SDL_Surface *surface, const char *file)
{
	SDL_RWops *out = SDL_RWFromFile (file, "wb");
	int ret;
	if (out == NULL)
		return -1;
	ret = IMG_SavePNG_RW (surface, out);
	SDL_RWclose (out);
	return ret;
}

void makeScreenShot ()
{
	char fname[256];
	int dlen, ishot, iline, w = window_width, h = window_height;
	FILE *f;
	DIR *d;
	unsigned char *pixels;
	SDL_Surface *surf;
	
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	int rmask = 0x00ff0000;
	int gmask = 0x0000ff00;
	int bmask = 0x000000ff;
	int amask = 0x00000000;
#else
	int rmask = 0x000000ff;
	int gmask = 0x0000ff00;
	int bmask = 0x00ff0000;
	int amask = 0x00000000;
#endif

        /* see if the screenshots directory exists */
#ifndef WINDOWS
	snprintf (fname, sizeof (fname), "%sscreenshots/", configdir);
#else
	snprintf (fname, sizeof (fname), "screenshots/");
#endif
	d = opendir(fname);
	if (d == NULL)
#ifndef WINDOWS
	if (mkdir(fname, 0755) < 0)
#else
	if (mkdir(fname) < 0)
#endif
	{
		LOG_ERROR ("Unable to create screenshots directory");
		return;
	}

	dlen = strlen (fname);

	/* try to find a file name which isn't taken yet */
	for (ishot = 1; ishot < 1000; ishot++)
	{
		snprintf (fname+dlen, sizeof(fname)-dlen, "elscreen%03d.png", ishot);
		f = fopen (fname, "r");
		if (f == NULL) break;
		fclose (f);
	}

	/* if all numbered file names have been taken, use the default */
	if (ishot >= 1000)
		snprintf (fname+dlen, sizeof(fname)-dlen, "elscreen.png");

	/* read the pixels from the GL scene */
	pixels = malloc (3 * w * h);
	glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	/* Let SDL worry about creating a BMP from it: create a surface */
	surf = SDL_CreateRGBSurface (SDL_SWSURFACE, w, h, 24, rmask, gmask, bmask, amask);

	/* simply memcpy'ing the pixels results in an upside-down image,
	 * so copy the lines in reverse order */
        for (iline = 0; iline < h; iline++)
		memcpy (surf->pixels + surf->pitch*iline, pixels + surf->pitch*(h-iline-1), 3*w);
	//SDL_SaveBMP (surf, fname);
	IMG_SavePNG (surf, fname);
	free (pixels);
	SDL_FreeSurface (surf);
}
#endif

void draw_circle_ext(int x, int y, int radius, int interval, int angle_from, int angle_to)
{
	const float mul=3.14159265f/180.0f;
	int angle;

	if(radius==0){
		glVertex2f(x, y);
	} else if(interval>0){
		for(angle=angle_from;angle<angle_to;angle+=interval){
			float rad=-mul*angle;
			glVertex2f(x+cos(rad)*radius+radius, y+radius+sin(rad)*radius);
		}
	} else { 
		for(angle=angle_from;angle>angle_to;angle+=interval){
			float rad=-mul*angle;
			glVertex2f(x+cos(rad)*radius+radius, y+radius+sin(rad)*radius);
		}
	}
}

void draw_circle(int x, int y, int radius, int interval)
{
	draw_circle_ext(x, y, radius, interval, 0, 360);
}

void draw_box(char * name, int x, int y, int w, int h, int rad)
{
	int l=0;

	if(name){
		l=(w-10-(get_string_width(name)*8.0f/12.0f))/2.0f;
		draw_string_small(x+l+5, y-6, name, 1);
	}

	glDisable(GL_TEXTURE_2D);
	if(l>0){
		glBegin(GL_LINE_STRIP);
			glVertex2i(x+l, y);
			draw_circle_ext(x, y, rad, 5, 90, 180);
			draw_circle_ext(x, y+h-2*rad, rad, 5, 180, 270);
			draw_circle_ext(x+w-2*rad, y+h-2*rad, rad, 5, 270, 360);
			draw_circle_ext(x+w-2*rad, y, rad, 5, 0, 90);
			glVertex2i(x+w-l, y);
		glEnd();
	} else if(l<0){
		glBegin(GL_LINE_STRIP);
			glVertex2i(x+l, y);
			draw_circle_ext(x+rad, y+h-rad, rad, 5, 180, 270);
			draw_circle_ext(x+w-2*rad, y+h-rad, rad, 5, 270, 360);
			glVertex2i(x+w-l, y);
		glEnd();
	} else {
		glBegin(GL_LINE_LOOP);
			draw_circle_ext(x+rad, y, rad, 5, 90, 180);
			draw_circle_ext(x+rad, y+h-rad, rad, 5, 180, 270);
			draw_circle_ext(x+w-2*rad, y+h-rad, rad, 5, 270, 360);
			draw_circle_ext(x+w-2*rad, y, rad, 5, 0, 90);
		glEnd();
	}
	glEnable(GL_TEXTURE_2D);
}


void draw_smooth_button(char * str, int x, int y, int w, int lines, float r, float g, float b, int highlight, float hr, float hg, float hb, float ha)
{
	int radius=lines*11.0f;
	int xstr=0;
	
	if(str){
		xstr=x+radius+(w-(get_string_width(str)*8.0f/12.0f))/2.0f;
	}

	glDisable(GL_TEXTURE_2D);

	glColor3f(r, g, b);
	glBegin(GL_LINE_LOOP);
		draw_circle_ext(x, y, radius, 10, 90, 270);
		draw_circle_ext(x+w, y, radius, 10, -90, 90);
	glEnd();
	if(highlight) {
		glColor4f(hr,hg,hb,ha);
		glBegin(GL_POLYGON);
			draw_circle_ext(x+1, y+1, radius-1, 10, 90, 270);
			draw_circle_ext(x+w+1, y+1, radius-1, 10, -90, 90);
		glEnd();
	}
	glEnable(GL_TEXTURE_2D);

	if(highlight) {
		glColor3f(r, g, b);
	}

	if(str) {
		draw_string_small(xstr, y+radius/2.0f, str, lines);
	}
}


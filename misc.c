#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifndef _MSC_VER
  #include <dirent.h>
  #include <errno.h>
  #include <unistd.h>
#endif //_MSC_VER
#include <sys/types.h>
#include <sys/stat.h>
#include	<zlib.h>
#ifdef PNG_SCREENSHOT
	#ifdef OSX
		#include <png/png.h>
	#else
		#include <png.h>
	#endif
#endif //PNG_SCREENSHOT
#ifdef OSX
#include <ApplicationServices/ApplicationServices.h>
#endif
#include <zlib.h>
#include <SDL.h>
#include <errno.h>
#include "misc.h"
#include "asc.h"
#include "bbox_tree.h"
#include "cursors.h"
#include "draw_scene.h"
#include "elconfig.h"
#include "errors.h"
#include "gl_init.h"
#include "init.h"
#include "interface.h"
#include "translate.h"

#define IMG_SetError(a) SDL_SetError(a)
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif // S_ISDIR

#ifdef WINDOWS
#define MKDIR(file) mkdir(file)
#else
//#define MKDIR(file) (mkdir(file, S_IRWXU | S_IRWXG) || chmod(file, S_IRWXU | S_IRWXG | S_ISGID))
#define MKDIR(file) (mkdir(file, S_IRWXU | S_IRWXG))
#endif //WINDOWS

LINE click_line;

void get_click_line(LINE* line)
{
	double proj[16];
	double modl[16];
	GLint view[4];
	double x1, x2, y1, y2, z1, z2, len;
	
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetDoublev(GL_MODELVIEW_MATRIX, modl);
	glGetIntegerv(GL_VIEWPORT, view);
	gluUnProject(mouse_x, window_height-mouse_y, 0, modl, proj, view, &x1, &y1, &z1);
	gluUnProject(mouse_x, window_height-mouse_y, 1, modl, proj, view, &x2, &y2, &z2);
	
	VMake(line->center, x1, y1, z1);
	
	x2 -= x1;
	y2 -= y1;
	z2 -= z1;
	
	len = sqrt(x2*x2 + y2*y2 + z2*z2);
	
	VMake(line->direction, x2/len, y2/len, z2/len);
	
	line->length = len;
}

int click_line_bbox_intersection(const AABBOX bbox)
{
	/* ALGORITHM: Use the separating axis
	 * theorem to see if the line segment
	 * and the box overlap. A line
	 * segment is a degenerate OBB. */

	VECTOR3 T, E;
	float r;
	
	VSub(T, bbox.bbmin, click_line.center);
	VSub(E, bbox.bbmax, bbox.bbmin);
	
	// do any of the principal axes
	// form a separating axis?
	
	if (fabs(T[X]) > (E[X] + click_line.length*fabs(click_line.direction[X]))) return 0;
	if (fabs(T[Y]) > (E[Y] + click_line.length*fabs(click_line.direction[Y]))) return 0;
	if (fabs(T[Z]) > (E[Z] + click_line.length*fabs(click_line.direction[Z]))) return 0;
	
	/* NOTE: Since the separating axis is
	 * perpendicular to the line in these
	 * last four cases, the line does not
	 * contribute to the projection. */
	
	// line.cross(x-axis)?
	
	r = E[Y]*fabs(click_line.direction[Z]) + E[Z]*fabs(click_line.direction[Y]);
	if (fabs(T[Y]*click_line.direction[Z] - T[Z]*click_line.direction[Y]) > r) return 0;
	
	// line.cross(y-axis)?
	
	r = E[X]*fabs(click_line.direction[Z]) + E[Z]*fabs(click_line.direction[X]);
	if( fabs(T[Z]*click_line.direction[X] - T[X]*click_line.direction[Z]) > r) return 0;
	
	// line.cross(z-axis)?
	
	r = E[X]*fabs(click_line.direction[Y]) + E[Y]*fabs(click_line.direction[X]);
	if (fabs(T[X]*click_line.direction[Y] - T[Y]*click_line.direction[X]) > r) return 0;

	return 1;
}

void set_click_line()
{
	if (read_mouse_now) get_click_line(&click_line);
}

#ifdef WINDOWS
FILE* my_tmpfile()
{
	FILE* fp;
	char* name = _tempnam(NULL, "elc_");

	if(!name)
		return NULL;
	fp = fopen(name, "wb+TD");
	if(name)
		free(name);
	return fp;
}
#endif

FILE *my_fopen (const char *fname, const char *mode)
{
	FILE *file = fopen (fname, mode);
	if (file == NULL)
	{
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, fname, strerror(errno));
	}
	return file;
}

off_t get_file_size(const char *fname)
{
	struct stat fstat;
	if (stat(fname, &fstat) < 0)
		return -1;
	return fstat.st_size;
}

//warning: when checking directories, do not include the trailing slash, for portability reasons
int file_exists(const char *fname)
{
	int statres;
	struct stat fstat;

	statres= stat(fname, &fstat);
	if(statres < 0)
	{
		statres= errno;
	}
	if(statres != ENOENT && statres != 0)
	{
		//something went wrong...
		LOG_ERROR("Error when checking file or directory %s (error code %d)\n", fname, statres);
		return -1;
	}
	else
	{
		return (statres != ENOENT);
	}
}

int gzfile_exists(const char *fname)
{
	char	gzfname[1024];

	safe_strncpy(gzfname, fname, sizeof(gzfname) - 4);
	strcat(gzfname, ".gz");
	if(file_exists(gzfname)){
		return 1;
	}

	return(file_exists(fname));
}

gzFile my_gzopen(const char * filename, const char * mode)
{
	char gzfilename[1024];
	gzFile result;

	safe_snprintf(gzfilename, sizeof(gzfilename), "%s.gz", filename);
	result= gzopen(gzfilename, mode);
	if(result == NULL) {
		// didn't work, try the name that was specified
		result= gzopen(filename, mode);
	}
	if(result == NULL) {
		LOG_ERROR("%s: %s \"%s\": %s\n", reg_error_str, cant_open_file, filename, strerror(errno));
	}

	return result;
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
	// it appears nobody ever writes a proper flush function but a dummy is required
	// possibly we could just call png_write_flush()
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
	LOG_WARNING("libpng: %s\n", str);
}

static void png_user_error(png_structp ctx, png_const_charp str)
{
	//fprintf(stderr, "libpng: error: %s\n", str);
	LOG_ERROR("libpng: %s\n", str);
}

int IMG_SavePNG_RW (SDL_Surface *face, SDL_RWops *src)
{
	png_structp png_ptr = 0;
	png_infop info_ptr = NULL;
	//png_colorp palette = 0;
	png_bytep *row_pointers = NULL;
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
	if (setjmp(png_jmpbuf(png_ptr)))
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
	
	if (row_pointers == NULL)
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
	if (row_pointers != NULL)
		free (row_pointers);
	

	if (info_ptr != NULL)
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	
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
	int ret;
	int align;
	int dlen, ishot, iline, w = window_width, h = window_height;
	unsigned char *pixels;
	SDL_Surface *surf;
	
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	int rmask = 0x00ff0000;
	int gmask = 0x0000ff00;
	int bmask = 0x000000ff;
#else
	int rmask = 0x000000ff;
	int gmask = 0x0000ff00;
	int bmask = 0x00ff0000;
#endif
	int amask = 0x00000000;

	/* see if the screenshots directory exists */
	safe_snprintf (fname, sizeof (fname), "%sscreenshots", configdir);

	ret = file_exists(fname);
	if(ret == 0)
	{
		if(MKDIR(fname) < 0)
		{
			LOG_ERROR ("Unable to create directory \"%s\"\n", fname);
			return;
		}
	}
	else if (ret == -1)
	{
		return;
	}

	dlen = strlen (fname);

	/* try to find a file name which isn't taken yet */
	for (ishot = 1; ishot < 1000; ishot++)
	{
		safe_snprintf (fname+dlen, sizeof(fname)-dlen, "/elscreen%03d.png", ishot);
		ret = file_exists(fname);
		if(ret == 0)
		{
			break;
		}
		else if(ret == -1)
		{
			return; //we hit an error, it's already reported
		}
	}

	/* if all numbered file names have been taken, use the default */
	if (ishot >= 1000)
	{
		LOG_TO_CONSOLE(c_red2, max_screenshots_warning_str);
		safe_snprintf (fname+dlen, sizeof(fname)-dlen, "/elscreen.png");
	}
	LOG_TO_CONSOLE(c_green1, fname);

	/* read the pixels from the GL scene */
	glGetIntegerv(GL_PACK_ALIGNMENT, &align);
	pixels = malloc(h * align * ((3 * w - 1) / align + 1));
	glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	/* Let SDL worry about creating a BMP from it: create a surface */
	surf = SDL_CreateRGBSurface (SDL_SWSURFACE, w, h, 24, rmask, gmask, bmask, amask);

	/* simply memcpy'ing the pixels results in an upside-down image,
	 * so copy the lines in reverse order */
	if (SDL_MUSTLOCK(surf)) SDL_LockSurface(surf);
	for (iline = 0; iline < h; iline++)
		memcpy ((char *)surf->pixels + surf->pitch*iline, pixels + surf->pitch*(h-iline-1), 3*w);
	if (SDL_MUSTLOCK(surf)) SDL_UnlockSurface(surf);
	//SDL_SaveBMP (surf, fname);
	IMG_SavePNG (surf, fname);
	free (pixels);
	SDL_FreeSurface (surf);
}
#endif

void draw_circle_ext(int x, int y, int radius, int interval, int angle_from, int angle_to)
{
	const float mul=M_PI/180.0f;
	int angle;
	x += gx_adjust;
	y += gy_adjust;

	if(radius==0){
		glVertex2f(x, y);
	} else if(interval>0){
		for(angle=angle_from;angle<angle_to;angle+=interval){
			float rad=-mul*angle;
			glVertex2f((float)x+cos(rad)*radius+radius, (float)y+radius+sin(rad)*radius);
		}
	} else { 
		for(angle=angle_from;angle>angle_to;angle+=interval){
			float rad=-mul*angle;
			glVertex2f((float)x+cos(rad)*radius+radius, (float)y+radius+sin(rad)*radius);
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
		l=(w-10-(get_string_width((unsigned char*)name)*8.0f/12.0f))/2.0f;
		draw_string_small(x+l+5, y-6, (unsigned char*)name, 1);
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
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


void draw_smooth_button(char * str, float size, int x, int y, int w, int lines, float r, float g, float b, int highlight, float hr, float hg, float hb, float ha)
{
	int radius=lines*BUTTONRADIUS*size;
	float width_ratio=(size*DEFAULT_FONT_X_LEN)/12.0f;
	int xstr=0;
	
	if(str){
		xstr=x+radius+(w-(get_string_width((unsigned char*)str)*width_ratio))/2.0f;
	}

	glDisable(GL_TEXTURE_2D);

	if(r>=0.0f)
		glColor3f(r, g, b);
	
#ifdef OSX
	if (square_buttons) {
		glBegin(GL_LINE_LOOP);
		glVertex3i(x,y,0);
		glVertex3i(x + w + radius*2,y,0);
		glVertex3i(x + w + radius*2,y + radius*2,0);
		glVertex3i(x,y + radius*2,0);
		glEnd();
		
		if(highlight) {
			if(hr>=0.0f)
				glColor4f(hr,hg,hb,ha);
			glBegin(GL_POLYGON);
			glVertex3i(x+1,y+1,0);
			glVertex3i(x + w + radius*2 -1,y+1,0);
			glVertex3i(x + w + radius*2 -1,y + radius*2 -1,0);
			glVertex3i(x+1,y + radius*2 -1,0);
			glEnd();
		}
		
		glEnable(GL_TEXTURE_2D);
	} else {
#endif
	glBegin(GL_LINE_LOOP);
		draw_circle_ext(x, y, radius, 10, 90, 270);
		draw_circle_ext(x+w, y, radius, 10, -90, 90);
	glEnd();
	if(highlight) {
		if(hr>=0.0f)
			glColor4f(hr,hg,hb,ha);
		glBegin(GL_POLYGON);
			draw_circle_ext(x+1, y+1, radius-1, 10, 90, 270);
			draw_circle_ext(x+w+1, y+1, radius-1, 10, -90, 90);
		glEnd();
	}
	glEnable(GL_TEXTURE_2D);

#ifdef OSX
	}	// to close off square_buttons conditional
#endif

	if(highlight) {
		glColor3f(r, g, b);
	}

	if(str) {
		draw_string_zoomed(xstr + gx_adjust, y+radius/2.0f + gy_adjust, (unsigned char*)str, lines, size);
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}


int substrtest(const char * haystack, int hlen, int pos, const char * needle, int nlen)
{
	if (pos < 0) pos += hlen;
	if (pos < 0) return -1;
	if (pos + nlen > hlen) return -1;
	return strncasecmp(haystack + pos, needle, nlen);
}

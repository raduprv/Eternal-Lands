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
#ifdef	ZLIB
#include	<zlib.h>
#endif
#ifdef PNG_SCREENSHOT
 #include <png.h>
#endif //PNG_SCREENSHOT
#ifdef OSX
#include <ApplicationServices/ApplicationServices.h>
#endif
#ifdef ZLIB
#include <zlib.h>
#endif // ZLIB
#include "misc.h"
#include "asc.h"
#include "bbox_tree.h"
#include "cursors.h"
#include "draw_scene.h"
#include "errors.h"
#include "font.h"
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

Uint8 last_pixel_color[4];
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

void reset_under_the_mouse()
{
	if(!read_mouse_now) {
		return;
	}
	last_pixel_color[0] = 0;
	last_pixel_color[1] = 0;
	last_pixel_color[2] = 0;
	object_under_mouse = -1;
	thing_under_the_mouse = UNDER_MOUSE_NOTHING;
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

/* find and store all urls in the provided string */
void find_all_url(const char *source_string, const int len)
{
	char search_for[][10] = {"http://", "https://", "ftp://", "www."};
	int next_start = 0;
    
	while (next_start < len)
	{
		int first_found = len-next_start; /* set to max */
		int i;
        
		/* find the first of the url start strings */
		for(i = 0; i < sizeof(search_for)/10; i++)
		{
			int found_at = get_string_occurance(search_for[i], source_string+next_start, len-next_start, 1);
			if ((found_at >= 0) && (found_at < first_found))
				first_found = found_at;
		}
        
		/* if url found, store (if new) it then continue the search straight after the end */
		if (first_found < len-next_start)
		{
			char *new_url = NULL;
			char *add_start = "";
			size_t url_len;
			int url_start = next_start + first_found;
			int have_already = 0;
			
			/* find the url end */
			for (next_start = url_start; next_start < len; next_start++)
			{
				char cur_char = source_string[next_start];
				if(!cur_char || cur_char == ' ' || cur_char == '\n' || cur_char == '<'
					|| cur_char == '>' || cur_char == '|' || cur_char == '"' || cur_char == '\'' || cur_char == '`'
					|| cur_char == ']' || cur_char == ';' || cur_char == '\\' || (cur_char&0x80) != 0)
					break;
			}
            
			/* prefix www. with http:// */
			if (strncmp(&source_string[url_start], "www.", 4) == 0)
				add_start = "http://";
			
			/* extract the string */
			url_len = strlen(add_start) + (next_start-url_start) + 1;
			new_url = (char *)malloc(sizeof(char)*url_len);
			/* could use safe_xxx() functions but I think its simpler not to here */
			strcpy(new_url, add_start);
			strncat(new_url, &source_string[url_start], next_start-url_start );
			new_url[url_len-1] = 0;
			
			/* check the new URL is not already in the list */
			if (have_url_count)
			{
				list_node_t *local_head = newest_url;
				while (local_head != NULL)
				{
					/* if its already stored, just make existing version active */
					if (strcmp(local_head->data, new_url) == 0)
					{
						active_url = local_head;
						have_already = 1;
						free(new_url);
						break;
					}
					local_head = local_head->next;
				}
			}
			
			/* if its a new url, create a new node in the url list */
			if (!have_already)
			{
				/* if these's a max number of url and we've reached it, remove the oldest */
				/* we don't need to worry if its the active_url as thats going to change */
				if (max_url_count && (max_url_count==have_url_count))
				{
					list_node_t *local_head = newest_url;
					/* go to the oldest in the list */
					while (local_head->next != NULL)
						local_head = local_head->next;
					free(local_head->data);
					if (local_head==newest_url)
					{
						/* the special case is when max_url_count=1... */
						free(local_head);
						newest_url = NULL;
					}
					else
					{
						local_head = local_head->prev;
						free(local_head->next);
						local_head->next = NULL;
					}
					have_url_count--;
				}
			
				list_push(&newest_url, new_url);
				active_url = newest_url;
				have_url_count++;
			}
			
		} /* end if url found */
        
		/* no more urls found so stop looking */
		else
			break;        
	}
    
} /* end find_all_url() */


#ifdef  WINDOWS
int go_to_url(void * url)
{
	char browser_command[400];

	if(!have_url_count || !*browser_name){
		return 0;
	}

	// build the command line and execute it
	safe_snprintf (browser_command, sizeof (browser_command), "%s \"%s\"", browser_name, url),
	system(browser_command);	// Do not use this command on UNIX.

	return 0;
}
#endif

void open_web_link(char * url)
{
#ifdef OSX
	CFURLRef newurl = CFURLCreateWithString(kCFAllocatorDefault,CFStringCreateWithCStringNoCopy(NULL,url,kCFStringEncodingMacRoman, NULL),NULL);
	LSOpenCFURLRef(newurl,NULL);
	CFRelease(newurl);
#else
	// browser name can override the windows default, and if not defined in Linux, don't error
	if(*browser_name){
#ifndef WINDOWS
		/* removed because unsafe (can be used to inject commands thru URLs)
		char browser_command[400];
		
		safe_snprintf(browser_command, sizeof (browser_command), "%s \"%s\"&", browser_name, url);
		system(browser_command);
		*/
		/* Lachesis: this is not portable but should do here */
		if (fork() == 0){
			execlp(browser_name, browser_name, url, NULL);
			// in case the exec errors
			_exit(1);
		}
#else
		SDL_Thread *go_to_url_thread;

		// windows needs to spawn it in its own thread
		go_to_url_thread= SDL_CreateThread(go_to_url, url);
	} else {
		ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNOACTIVATE); //this returns an int we could check for errors, but that's mainly when you use shellexecute for local files
#endif  //_WIN32
	}
#endif // OSX
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
#ifdef	ZLIB
	char	gzfname[1024];

	safe_strncpy(gzfname, fname, sizeof(gzfname) - 4);
	strcat(gzfname, ".gz");
	if(file_exists(gzfname)){
		return 1;
	}
#endif

	return(file_exists(fname));
}

#ifdef ZLIB
gzFile * my_gzopen(const char * filename, const char * mode)
{
	char gzfilename[1024];
	gzFile * result;

	safe_snprintf(gzfilename, sizeof(gzfilename), "%s.gz", filename);
	result= gzopen(gzfilename, mode);
	if(result == NULL) {
		// didn't work, try the name that was specified
		result= gzopen(filename, mode);
	}
	if(result == NULL) {
		LOG_ERROR("%s: %s \"%s\"\n", reg_error_str, cant_open_file, filename);
	}

	return result;
}
#endif // ZLIB


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
	if (row_pointers != NULL)
		free (row_pointers);
	
	if (info_ptr != NULL && info_ptr->palette != NULL)
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
		safe_snprintf (fname+dlen, sizeof(fname)-dlen, "/elscreen.png");
	}

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

	if(highlight) {
		glColor3f(r, g, b);
	}

	if(str) {
		draw_string_zoomed(xstr, y+radius/2.0f, (unsigned char*)str, lines, size);
	}
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

#ifndef NEW_FILE_IO
int mkdir_tree(const char *file)
{
	// First, check directory exists
	char dir[1024];
	char *slash;
	struct stat stats;

	safe_strncpy(dir, file, sizeof(dir));
	slash= dir;

	// Skip over leading periods. this also prevents ../ accesses on purpose
	while(*slash == '.'){
		slash++;
	}

	// Skip over leading slashes.
	while(*slash == '/'){
		slash++;
	}

	while(slash){
		// watch for hidden ..
		if(*slash == '.' && slash[1] == '.'){
			log_error("cannot create directory %s", dir);
			return 0;
		}
		// find the next slash
		slash= strchr(slash, '/');
		if(slash == NULL){
			break;
		}

		// place a NULL there to break the string up
		*slash= '\0';
		if(!(stat(dir, &stats) == 0 && S_ISDIR(stats.st_mode) ) )
		if(MKDIR(dir)!= 0) {
			log_error("cannot create directory %s", dir);
			return 0;
		}
		// put the / back in, then advance past it
		*slash++ = '/';

		// Avoid unnecessary calls to mkdir when given
		// file names containing multiple adjacent slashes.
		while (*slash == '/'){
			slash++;
		}
	}
	return 1;
}
#endif //!NEW_FILE_IO

int substrtest(const char * haystack, int hlen, int pos, const char * needle, int nlen)
{
	if (pos < 0) pos += hlen;
	if (pos < 0) return -1;
	if (pos + nlen > hlen) return -1;
	return strncasecmp(haystack + pos, needle, nlen);
}

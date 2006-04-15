#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifndef _MSC_VER
  #include <dirent.h>
  #include <errno.h>
#endif //_MSC_VER
#include <sys/types.h>
#include <sys/stat.h>
#ifdef PNG_SCREENSHOT
 #include <png.h>
#endif //PNG_SCREENSHOT
#include "global.h"

#define IMG_SetError(a) SDL_SetError(a)

Uint8 last_pixel_color[4];
#ifdef	NEW_FRUSTUM
LINE click_line;
#endif

#ifdef	NEW_FRUSTUM
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
#endif

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

static GLfloat  model[16], inv_model[16];
static GLfloat  proj[16], inv_proj[16];
static GLint    viewport[4];

void invert_matrix(GLfloat *src, GLfloat *dst) {
	GLfloat det;
	int i;

	// compute minor matrix
	dst[ 0] = +src[ 5]*src[10]*src[15] +src[ 9]*src[14]*src[ 7] +src[13]*src[ 6]*src[11]
	          -src[13]*src[10]*src[ 7] -src[ 9]*src[ 6]*src[15] -src[ 5]*src[14]*src[11];
	dst[ 1] = -src[ 1]*src[10]*src[15] -src[ 9]*src[14]*src[ 3] -src[13]*src[ 2]*src[11]
	          +src[13]*src[10]*src[ 3] +src[ 9]*src[ 2]*src[15] +src[ 1]*src[14]*src[11];
	dst[ 2] = +src[ 1]*src[ 6]*src[15] +src[ 5]*src[14]*src[ 3] +src[13]*src[ 2]*src[ 7]
	          -src[13]*src[ 6]*src[ 3] -src[ 5]*src[ 2]*src[15] -src[ 1]*src[14]*src[ 7];
	dst[ 3] = -src[ 1]*src[ 6]*src[11] -src[ 5]*src[10]*src[ 3] -src[ 9]*src[ 2]*src[ 7]
	          +src[ 9]*src[ 6]*src[ 3] +src[ 5]*src[ 2]*src[11] +src[ 1]*src[10]*src[ 7];

	dst[ 4] = -src[ 4]*src[10]*src[15] -src[ 8]*src[14]*src[ 7] -src[12]*src[ 6]*src[11]
	          +src[12]*src[10]*src[ 7] +src[ 8]*src[ 6]*src[15] +src[ 4]*src[14]*src[11];
	dst[ 5] = +src[ 0]*src[10]*src[15] +src[ 8]*src[14]*src[ 3] +src[12]*src[ 2]*src[11]
	          -src[12]*src[10]*src[ 3] -src[ 8]*src[ 2]*src[15] -src[ 0]*src[14]*src[11];
	dst[ 6] = -src[ 0]*src[ 6]*src[15] -src[ 4]*src[14]*src[ 3] -src[12]*src[ 2]*src[ 7]
	          +src[12]*src[ 6]*src[ 3] +src[ 4]*src[ 2]*src[15] +src[ 0]*src[14]*src[ 7];
	dst[ 7] = +src[ 0]*src[ 6]*src[11] +src[ 4]*src[10]*src[ 3] +src[ 8]*src[ 2]*src[ 7]
	          -src[ 8]*src[ 6]*src[ 3] -src[ 4]*src[ 2]*src[11] -src[ 0]*src[10]*src[ 7];

	dst[ 8] = +src[ 4]*src[ 9]*src[15] +src[ 8]*src[13]*src[ 7] +src[12]*src[ 5]*src[11]
	          -src[12]*src[ 9]*src[ 7] -src[ 8]*src[ 5]*src[15] -src[ 4]*src[13]*src[11];
	dst[ 9] = -src[ 0]*src[ 9]*src[15] -src[ 8]*src[13]*src[ 3] -src[12]*src[ 1]*src[11]
	          +src[12]*src[ 9]*src[ 3] +src[ 8]*src[ 1]*src[15] +src[ 0]*src[13]*src[11];
	dst[10] = +src[ 0]*src[ 5]*src[15] +src[ 4]*src[13]*src[ 3] +src[12]*src[ 1]*src[ 7]
	          -src[12]*src[ 5]*src[ 3] -src[ 4]*src[ 1]*src[15] -src[ 0]*src[13]*src[ 7];
	dst[11] = -src[ 0]*src[ 5]*src[11] -src[ 4]*src[ 9]*src[ 3] -src[ 8]*src[ 1]*src[ 7]
	          +src[ 8]*src[ 5]*src[ 3] +src[ 4]*src[ 1]*src[11] +src[ 0]*src[ 9]*src[ 7];

	dst[12] = -src[ 4]*src[ 9]*src[14] -src[ 8]*src[13]*src[ 6] -src[12]*src[ 5]*src[10]
	          +src[12]*src[ 9]*src[ 6] +src[ 8]*src[ 5]*src[14] +src[ 4]*src[13]*src[10];
	dst[13] = +src[ 0]*src[ 9]*src[14] +src[ 8]*src[13]*src[ 2] +src[12]*src[ 1]*src[10]
	          -src[12]*src[ 9]*src[ 2] -src[ 8]*src[ 1]*src[14] -src[ 0]*src[13]*src[10];
	dst[14] = -src[ 0]*src[ 5]*src[14] -src[ 4]*src[13]*src[ 2] -src[12]*src[ 1]*src[ 6]
	          +src[12]*src[ 5]*src[ 2] +src[ 4]*src[ 1]*src[14] +src[ 0]*src[13]*src[ 6];
	dst[15] = +src[ 0]*src[ 5]*src[10] +src[ 4]*src[ 9]*src[ 2] +src[ 8]*src[ 1]*src[ 6]
	          -src[ 8]*src[ 5]*src[ 2] -src[ 4]*src[ 1]*src[10] -src[ 0]*src[ 9]*src[ 6];

	// compute determinant using Laplace formula
	det = src[0]*dst[0] + src[1]*dst[4] + src[2]*dst[8] + src[3]*dst[12];

	// compute inverse using minors
	if (det != 0.0f) {
		for (i=0; i < 16; i++) dst[i] /= det;
	} else {
		LOG_ERROR("Warning: computing inverse of singular matrix\n");
	}
}

void save_scene_matrix()
{
	glGetFloatv(GL_MODELVIEW_MATRIX, model); invert_matrix(model, inv_model);
	glGetFloatv(GL_PROJECTION_MATRIX, proj); invert_matrix(proj, inv_proj);
	glGetIntegerv(GL_VIEWPORT, viewport);
	viewport[2]>>=1;
	viewport[3]>>=1;
}

void project_ortho(GLfloat ox, GLfloat oy, GLfloat oz, GLfloat * wx, GLfloat * wy)
{
	GLfloat tmp[4], ow;

	// (convert to homogenous coordinates and) apply modelview matrix
	tmp[0] = model[0*4+0] * ox + model[1*4+0] * oy + model[2*4+0] * oz + model[3*4+0];
	tmp[1] = model[0*4+1] * ox + model[1*4+1] * oy + model[2*4+1] * oz + model[3*4+1];
	tmp[2] = model[0*4+2] * ox + model[1*4+2] * oy + model[2*4+2] * oz + model[3*4+2];
	tmp[3] = model[0*4+3] * ox + model[1*4+3] * oy + model[2*4+3] * oz + model[3*4+3];
	
	// apply projection matrix
	ox = proj[0*4+0] * tmp[0] + proj[1*4+0] * tmp[1] + proj[2*4+0] * tmp[2] + proj[3*4+0] * tmp[3];
	oy = proj[0*4+1] * tmp[0] + proj[1*4+1] * tmp[1] + proj[2*4+1] * tmp[2] + proj[3*4+1] * tmp[3];
	oz = proj[0*4+2] * tmp[0] + proj[1*4+2] * tmp[1] + proj[2*4+2] * tmp[2] + proj[3*4+2] * tmp[3];
	ow = proj[0*4+3] * tmp[0] + proj[1*4+3] * tmp[1] + proj[2*4+3] * tmp[2] + proj[3*4+3] * tmp[3];

	// convert homogenous to affine coordinates
	ox /= ow;
	oy /= ow;
	oz /= ow;
	
	// viewport
	*wx = viewport[0] + (1 + ox) * viewport[2];
	*wy = viewport[1] + (1 + oy) * viewport[3];
}

void unproject_ortho(GLfloat wx,GLfloat wy,GLfloat wz,GLfloat *ox,GLfloat *oy,GLfloat *oz)
{
	GLfloat tmp[4], ow;

	// inverse viewport
	wx=(wx-viewport[0])/(float)viewport[2]-1.0f;
	wy=(wy-viewport[1])/(float)viewport[3]-1.0f;
	wz=2.0f*wz-1.0f;

	// (convert to homogenous coordinates and) inverse projection
	tmp[0] = inv_proj[0*4+0] * wx + inv_proj[1*4+0] * wy + inv_proj[2*4+0] * wz + inv_proj[3*4+0];
	tmp[1] = inv_proj[0*4+1] * wx + inv_proj[1*4+1] * wy + inv_proj[2*4+1] * wz + inv_proj[3*4+1];
	tmp[2] = inv_proj[0*4+2] * wx + inv_proj[1*4+2] * wy + inv_proj[2*4+2] * wz + inv_proj[3*4+2];
	tmp[3] = inv_proj[0*4+3] * wx + inv_proj[1*4+3] * wy + inv_proj[2*4+3] * wz + inv_proj[3*4+3];

	// Inverse modelview
	*ox = inv_model[0*4+0] * tmp[0] + inv_model[1*4+0] * tmp[1] + inv_model[2*4+0] * tmp[2] + inv_model[3*4+0] * tmp[3];
	*oy = inv_model[0*4+1] * tmp[0] + inv_model[1*4+1] * tmp[1] + inv_model[2*4+1] * tmp[2] + inv_model[3*4+1] * tmp[3];
	*oz = inv_model[0*4+2] * tmp[0] + inv_model[1*4+2] * tmp[1] + inv_model[2*4+2] * tmp[2] + inv_model[3*4+2] * tmp[3];
	 ow = inv_model[0*4+3] * tmp[0] + inv_model[1*4+3] * tmp[1] + inv_model[2*4+3] * tmp[2] + inv_model[3*4+3] * tmp[3];
	
	// convert homogenous to affine coordinates
	*ox /= ow;
	*oy /= ow;
	*oz /= ow;
}

#ifndef	NEW_FRUSTUM
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
#endif

void find_last_url(const unsigned char *source_string, const int len)
{
	char cur_char;
	char search_for[][10] = {"http://", "https://", "ftp://"};
	int i, j, url_start;
	int last_url_start = 0;
	int final_url_start = 0;
	int final_url_start_2 = 0;

	/* Search for www. first */
	for(last_url_start = 0; ; last_url_start += url_start) {
		url_start = get_string_occurance("www.", source_string+final_url_start+1, len-last_url_start, 1);
		if(url_start <= 0) {
			/* We either found what we're looking for, or we didn't find anything at all */
			break;
		}
		url_start++;
		if(final_url_start < url_start+last_url_start) {
			final_url_start = url_start+last_url_start;
		}
	}

	/* Now search for the rest */
	for(last_url_start = 0; ; last_url_start += url_start) {
		for(i = 0, url_start = -1; i < sizeof(search_for)/10 && url_start < 0; i++) {
			url_start = get_string_occurance(search_for[i], source_string+final_url_start_2+1, len-last_url_start, 1);
		}
		if(url_start <= 0) {
			/* We either found what we're looking for, or we didn't find anything at all */
			break;
		}
		url_start++;
		if(final_url_start_2 < url_start+last_url_start) {
			final_url_start_2 = url_start+last_url_start;
		}
	}

	if(!final_url_start && !final_url_start_2) { //no URL found
		return;
	}

	//ok, we have an URL, now get it
	j = 0;
	if(final_url_start > final_url_start_2)
	{
		snprintf(current_url, sizeof(current_url), "http://");
		j = 7;
	} else {
		final_url_start = final_url_start_2;
	}

	for(i = final_url_start; i < len; i++)
	{
		if(j > sizeof(current_url)-2) {
			break; //URL too long, perhaps an exploit attempt
		}
		cur_char = source_string[i];
		if(!cur_char || cur_char == ' ' || cur_char == '\n' || cur_char == '<'
			|| cur_char == '>' || cur_char == '|' || cur_char == '"'
			|| cur_char == ']') {
			break;
		}
		current_url[j] = cur_char;
		j++;
	}
	current_url[j] = 0;
	have_url = 1;
}

#ifdef  WINDOWS
int go_to_url(void *dummy)
{
	char browser_command[400];

	if(!have_url || !*browser_name){
		return 0;
	}

	// build the command line and execute it
	snprintf (browser_command, sizeof (browser_command), "%s \"%s\"", browser_name, current_url),
	system(browser_command);

	return 0;
}
#endif

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
	snprintf (fname, sizeof (fname), "%sscreenshots", configdir);

	ret = file_exists(fname);
	if(ret == 0)
	{
#ifndef WINDOWS
		if (mkdir(fname, 0755) < 0)
#else //WINDOWS
		if (mkdir(fname) < 0)
#endif //!WINDOWS
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
		snprintf (fname+dlen, sizeof(fname)-dlen, "/elscreen%03d.png", ishot);
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
		snprintf (fname+dlen, sizeof(fname)-dlen, "/elscreen.png");
	}

	/* read the pixels from the GL scene */
	pixels = malloc (3 * w * h);
	glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	/* Let SDL worry about creating a BMP from it: create a surface */
	surf = SDL_CreateRGBSurface (SDL_SWSURFACE, w, h, 24, rmask, gmask, bmask, amask);

	/* simply memcpy'ing the pixels results in an upside-down image,
	 * so copy the lines in reverse order */
	for (iline = 0; iline < h; iline++)
		memcpy ((char *)surf->pixels + surf->pitch*iline, pixels + surf->pitch*(h-iline-1), 3*w);
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


void draw_smooth_button(char * str, float size, int x, int y, int w, int lines, float r, float g, float b, int highlight, float hr, float hg, float hb, float ha)
{
	int radius=lines*15*size;
	float width_ratio=(size*DEFAULT_FONT_X_LEN)/12.0f;
	int xstr=0;
	
	if(str){
		xstr=x+radius+(w-(get_string_width(str)*width_ratio))/2.0f;
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
		draw_string_zoomed(xstr, y+radius/2.0f, str, lines, size);
	}
}


/*!
 * \file
 * \ingroup misc
 * \brief miscellaneous functions
 */
#ifndef __MISC_H__
#define __MISC_H__

#include "global.h"
#ifdef ZLIB
#include <zlib.h>
#endif // ZLIB

#define BUTTONRADIUS 15

/*!
 * \ingroup misc
 * \brief Swaps a float properly
 *
 *      Swaps the given float \a t
 *
 * \param t         the float to swap
 * \retval float    the swapped float
 */
static __inline__ float SwapFloat(float t)
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


/*!
 * \ingroup misc
 * \brief   Computes the reverse orthographic projection.
 *
 *      Computes the reverse orthographic projection from the parameters (\a wx, \a wy, \a wz) and stores the result in (\a ox, \a oy, \a oz).
 *
 * \param wx    x coordinate of the projection
 * \param wy    y coordinate of the projection
 * \param wz    z coordinate of the projection
 * \param ox    reverse x coordinate of the projection
 * \param oy    reverse y coordinate of the projection
 * \param oz    reverse z coordinate of the projection
 */
void unproject_ortho(GLfloat wx,GLfloat wy,GLfloat wz,GLfloat *ox,GLfloat *oy,GLfloat *oz);

/*!
 * \ingroup misc
 * \brief   Resets the mouse values
 *
 *      Resets the pixel value at the mouse position, as well as the \ref thing_under_the_mouse and \ref object_under_mouse.
 *
 * \pre If \ref read_mouse_now is false, this function returns without performing any action.
 */
void reset_under_the_mouse();

/*!
 * \ingroup misc
 * \brief   Checks if there any objects at the mouse cursor position.
 *
 *      Checks if there are any objects at the current mouse cursor position. If the test succeeds, \ref thing_under_the_mouse will be set to \a object_type and \ref object_under_mouse will be set to \a object_id.
 *
 * \param object_id     the id of the object under the mouse
 * \param object_type   the type of object under the mouse
 * \retval int          0, if there are no objects under the mouse, else 1.
 *
 * \pre If \ref read_mouse_now is false, this function returns 0, without performing any actions.
 * \pre If \a object_type equals \ref UNDER_MOUSE_NO_CHANGE, this function will return 0, after storing the pixel values at the current mouse position.
 */
int anything_under_the_mouse(int object_id, int object_type);

/*!
 * \ingroup misc
 * \brief   Saves the scene matrices for later use.
 *
 *      Saves the scene matrices for the model, projection and viewport for later use.
 *
 */
void save_scene_matrix();

#ifndef	NEW_FRUSTUM
/*!
 * \ingroup misc
 * \brief   Checks if the mouse cursor is within a sphere with center in (\a x, \a y, \a z) and the given \a radius.
 *
 *      Checks the current mouse position against a sphere with center in (\a x, \a y, \a z) and the given \a radius.
 *
 * \param x         x coordinate of the sphere center
 * \param y         y coordinate of the sphere center
 * \param z         z coordinate of the sphere center
 * \param radius    radius of the sphere
 * \retval int      1 (true), if the mouse is inside the given sphere, else 0 (false).
 * \callgraph
 */
int mouse_in_sphere(float x, float y, float z, float radius);
#endif

/*!
 * \ingroup misc
 * \brief   Checks, if the \a source_string contains an URL.
 *
 *      Checks the given \a source_string up to a length of \a len for being an URL and sets \ref have_url if this the case.
 *
 * \param source_string the string that contains the URL
 * \param len           the length of \a source_string.
 * \callgraph
 *
 * \pre If \a source_string does not contain a valid URL, this function will return without setting \ref have_url to true.
 */
void find_last_url(const unsigned char *source_string, const int len);

//some prototypes, that won't fit somewhere else

#ifndef	NEW_FRUSTUM
/*!
 * \ingroup misc
 * \brief   Checks if a sphere with center at (\a x, \a y, \a z) with the given \a radius is inside the view frustum.
 *
 *      Checks if a sphere with center at (\a x, \a y, \a z) with the given \a radius is inside the view frustum.
 *
 * \param x         the x coordinate of the sphere
 * \param y         the y coordinate of the sphere
 * \param z         the z coordinate of the sphere
 * \param radius    the radius of the sphere
 * \retval int      0 (false), if the sphere is outside the frustum, else 1 (true).
 */
int SphereInFrustum(float x, float y, float z, float radius);

/*!
 * \ingroup misc
 * \brief   Checks if the tile with the given coordinates \a x and \a y is inside the view frustum.
 *
 *      Checks if the tile with the given coordinates \a x and \a y is inside the view frustum.
 *
 * \param x         the x coordinate of the tile
 * \param y         the y coordiante of the tile
 * \retval int      1 (true), if the sphere with center in (\a x + 1.5, \a y + 1.5, 0) and radius 2.5 is inside the view frustum, else 0 (false)
 * \callgraph
 */
int check_tile_in_frustrum(float x,float y);
#else
void calculate_reflection_frustum(unsigned int num, float water_height);
void calculate_shadow_frustum();
void enable_reflection_clip_planes();
void disable_reflection_clip_planes();
void set_current_frustum(unsigned int intersect_type);
#endif

/*!
 * \ingroup misc
 * \brief Opens a url in the configured browser
 *
 *      Opens the configured browser to the current URL
 *
 * \param url         the url to open in the browser
 * \retval int  always returns 0
 */
int go_to_url(void * url);

/*!
 * \ingroup misc
 * \brief Opens a url in the configured browser
 *
 *      Opens the configured browser to the current URL
 *
 */
void open_web_link(char * url);

/*!
 * \ingroup misc
 * \brief Opens a file and check the result
 *
 *      Tries to open a file, and logs an error message if it fails
 *
 * \param fname The file name
 * \param mode  The mode in which the file is to be opened
 * \retval FILE* Pointer to the file on success, NULL otherwise
 */
FILE *my_fopen (const char *fname, const char *mode);
int file_exists(const char *fname);
int gzfile_exists(const char *fname);
int	mkdir_tree(const char *file);

#ifdef PNG_SCREENSHOT
/*!
 * \ingroup misc
 * \brief Takes a screenshot
 *
 *      Takes a screenshot
 *
 */
void makeScreenShot ();
#endif

/*!
 * \ingroup misc_utils
 * \brief Draws a circle from angle_from to angle_to
 * 
 * 		Draws a circle from angle_from to angle_to with the given radius from the center x,y. Increments the angle with interval.
 *
 * \param x The center x
 * \param y The center y
 * \param radius The radius
 * \param interval The # of steps to increment the angle. Can be negative, but then angle_from has to be higher than angle_to. When it is positive angle_from has to be lower than angle_to
 * \param angle_from The starting angle
 * \param angle_to The end angle
 */
void draw_circle_ext(int x, int y, int radius, int interval, int angle_from, int angle_to);

/*!
 * \ingroup misc_utils
 * \brief Draws a circle from angle_from to angle_to
 * 
 * 		Draws a circle with the given radius from the center x,y.
 *
 * \param x The center x
 * \param y The center y
 * \param radius The radius
 * 
 * \callgraph
 */
void draw_circle(int x, int y, int radius, int interval);

/*!
 * \ingroup misc_utils
 * \brief Draws a box, that potentially uses rounded corners
 *
 * 	Draws a box that potentially uses rounded corners if a radius is given. The box can have a name, but that is optional. Will look like this:
 * \code
 * 	1)
 * 	+----- box -----+
 * 	|		|
 * 	|		|
 * 	|		|
 * 	|		|
 * 	+---------------+
 *
 * 	2)
 * 	+---------------+
 * 	|		|
 * 	|		|
 * 	|		|
 * 	|		|
 * 	+---------------+
 * \endcode
 *
 * \param name The potential name of the box. If you don't want a name
 * \param x The start x position
 * \param y The start y position
 * \param w The width
 * \param h The height
 * \param rad The radius in the rounded corners - note that they are optional
 */
void draw_box(char * name, int x, int y, int w, int h, int rad);

/*!
 * \ingroup misc_utils
 * \brief Draws a button with round corners. 
 *
 * 	Draws a button with round corners. The box can be highlighted with the chosen highlight colors (r,g,b,a).
 *
 * \param str The name to write within the button, optional
 * \param size The size of the text
 * \param x The start x position
 * \param y The start y position
 * \param w The width
 * \param lines The number of lines (determines the height)
 * \param r The red color for border and text
 * \param g The green color for border and text
 * \param b The blue color for border and text
 * \param highlight If the button is highlighted or not
 * \param hr The red color for highlighted buttons
 * \param hg The green color for highlighted buttons
 * \param hb The blue color for highlighted buttons
 * \param ha The alpha color for highlighted buttons
 */
void draw_smooth_button(char * str, float size, int x, int y, int w, int lines, float r, float g, float b, int highlight, float hr, float hg, float hb, float ha);

#ifdef ZLIB
/*!
 * \ingroup misc
 * \brief Append '.gz' to a filename and try to open it using gzopen
 *
 * Appends the '.gz' to a filename and tries to open the file with that
 * name. If it fails, tries to open the file with the original filename.
 */
gzFile * my_gzopenext(const char * filename);
#endif // ZLIB

static __inline__ int min2i (int x, int y)
{
	return (x <= y)? x : y;
}

static __inline__ int max2i (int x, int y)
{
	return (x >= y)? x : y;
}

static __inline__ unsigned min2u (unsigned x, unsigned y)
{
	return (x <= y)? x : y;
}

static __inline__ unsigned max2u (unsigned x, unsigned y)
{
	return (x >= y)? x : y;
}

static __inline__ float min2f (float x, float y)
{
	return (x <= y)? x : y;
}

static __inline__ float max2f (float x, float y)
{
	return (x >= y)? x : y;
}

static __inline unsigned clampu(unsigned x, unsigned l, unsigned u)
{
	return min2u(max2u(x,l),u);
}

static __inline int clampi(int x, int l, int u)
{
	return min2i(max2i(x,l),u);
}

static __inline float clampf(float x, float l, float u)
{
	return min2f(max2f(x,l),u);
}

#endif

/*!
 * \file
 * \ingroup misc
 * \brief miscellaneous functions
 */
#ifndef __MISC_H__
#define __MISC_H__

#include <SDL_endian.h>
#include "platform.h"
#include <zlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUTTONRADIUS 15

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

//some prototypes, that won't fit somewhere else

void calculate_reflection_frustum(float water_height);
void calculate_shadow_frustum();
void enable_reflection_clip_planes();
void disable_reflection_clip_planes();
void set_current_frustum(unsigned int intersect_type);

#ifdef WINDOWS
/*!
 * \ingroup misc
 * \brief Creates a temporary file
 *
 *      Replacement for tmpfile() to use in Windows (at least in Vista tmpfile() requires Administrator privileges)
 *
 * \retval FILE* Pointer to the file on success, NULL otherwise
 */
FILE *my_tmpfile ();
#endif

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
off_t get_file_size(const char *fname);
int file_exists(const char *fname);
int gzfile_exists(const char *fname);

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

/*!
 * \ingroup misc
 * \brief Append '.gz' to a filename and try to open it using gzopen
 *
 * Appends the '.gz' to a filename and tries to open the file with that
 * name. If it fails, tries to open the file with the original filename.
 *
 * \param filename The file to open
 * \param mode The i/o mode (see open())
 * \return a zlib file handle
 */
gzFile my_gzopen(const char * filename, const char * mode);

/*!
 * \ingroup misc
 * \brief Test whether a string contains another string at a certain position
 *
 * \c substrtest() test whether \p haystack contains \p needle at position \pos.
 *
 * \param haystack the string to test within
 * \param hlen the length of haystack
 * \param pos the position at which to test for \p needle
 * \param needle the string to test for
 * \param nlen the length of \p needle
 * \return Zero if needle was found, nonzero otherwise (sic!).
 */
int substrtest(const char * haystack, int hlen, int pos, const char * needle, int nlen);

/*!
 * \ingroup misc
 * \brief tests whether a string ends on \p suffix
 *
 * Tests whether \p str ends on \p suffix.
 *
 * \param str the string to examine
 * \param len the length of \p str
 * \param suffix the suffix to test for
 * \param slen the length of \p suffix
 * \return nonzero if \p str ends on \p suffix, zero otherwise.
 */
static __inline__ int has_suffix(const char * str, int len, const char * suffix, int slen)
{
	return !substrtest(str, len, -slen, suffix, slen);
}

/*!
 * \name min_max
 * \brief min/max computation (please read docs)
 *
 * These functions compute min's, max's and related things in a safe and fast manner.
 * Why not use a macro like this?
 * \begincode
 * #define min(x,y) ((x) < (y) ? (x) : (y))
 * \endcode
 * Because it requires a lot of care to use properly and inline functions can do
 * the same in a sometimes faster and safe manner. The C preprocessor is only a
 * text replacer, and the above macro will evaluate each of the arguments x and y
 * twice! Not only may this involve tiny performance hits, much more important will it
 * cause undesirable results if the evaluation has side effects, like in
 * \begincode
 *   X = min(rand(), rand())
 * \endcode
 * in order to generate a variate X that is more likely to be small. Using the above macro, 
 * X would still be uniformly distributed. Even if you don't dothat kind of jerk, please
 * use these inline functions in order to help avoiding others making these mistakes.
 *
 *  -Lachesis
 * @{
 */

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

static __inline float max3f (float x, float y, float z)
{
	return max2f(x, max2f(y, z));
}

static __inline__ unsigned clampu(unsigned x, unsigned l, unsigned u)
{
	return min2u(max2u(x,l),u);
}

static __inline__ int clampi(int x, int l, int u)
{
	return min2i(max2i(x,l),u);
}

static __inline__ float clampf(float x, float l, float u)
{
	return min2f(max2f(x,l),u);
}

/*! @} */

void init_reflection_portals(int size);

extern Uint32 use_new_selection;

#ifdef __cplusplus
} // extern "C"
#endif

#endif

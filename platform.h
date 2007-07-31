/*!
 * \file
 * \ingroup     platform
 * \brief       Various defines and constants to make EL compile on different platforms.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <math.h>
#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif //M_PI

#ifdef OSX
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 #include <OpenGL/glext.h>
#else
 #include <GL/gl.h>
 #include <GL/glu.h>
 #include <GL/glext.h>
#endif

#ifdef EL_BIG_ENDIAN
 #define SwapLEFloat(X) SwapFloat(X)
#else
 #define SwapLEFloat(X) (X)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef X86_64
typedef long int point;
#else
typedef int point;
#endif

/*!
 * \ingroup platform
 * \brief Swaps a float properly
 *
 *      Swaps the bytes of the given float \a t
 *
 * \param t         the float to swap
 * \retval float    the swapped float
 */
#include <SDL_endian.h>
static __inline__ float SwapFloat (float t)
{
	union
	{
		float f;
		int i;
	} intOrFloat;

	intOrFloat.f = t;
	intOrFloat.i = SDL_Swap32 (intOrFloat.i);
	return intOrFloat.f;
}


#ifdef __cplusplus
} // extern "C"
#endif

#endif // PLATFORM_H

/*!
 * \file
 * \ingroup     platform
 * \brief       Various defines and constants to make EL compile on different platforms.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

// Try to use compiler macros to detect 64-bitness. According to 
// http://predef.sourceforge.net/prearch.html , these ought to work on
// gcc, Sun Studio and Visual Studio.
// Throw in ia64 as well, though I doubt anyone will play EL on that.
#if defined (__x86_64__) || defined (_M_X64) || defined (__ia64__) || defined (_M_IA64)
 #define X86_64
#endif

#ifdef FASTER_STARTUP
// x86 can do unaligned reads of multi-byte data, not sure about other
// architectures, so split unaligned reads there
#if defined (__i386__) || defined (_M_IX86) || defined (__x86_64__) || defined (_M_X64)
 #undef EL_FORCE_ALIGNED_READ
#else
 #define EL_FORCE_ALIGNED_READ
#endif
#endif // FASTER_STARTUP

// only ever use WINDOWS anywhere else, in case we need to add another 'catch' to 
// enable WINDOWS
#if defined (_WIN32) || defined (_WIN64) || defined (WIN32)
 #ifndef WINDOWS
  #define WINDOWS
 #endif  // !WINDOWS
#endif  // _WIN32 || _WIN64

#ifdef WINDOWS
 #ifndef NOMINMAX
 #define NOMINMAX
 #endif
 #include <windows.h>
 #ifdef _MSC_VER        // now we do test for VC
  // Lachesis: Make sure snprintf is declared before we #define it to be something else,
  // else we'll eventually break C++ headers that use it
  #include <stdio.h>

  #define stat _stat
  #define snprintf safe_snprintf
  #define strncasecmp _strnicmp
  #define strcasecmp _stricmp

  #define __inline__ __inline

  #if _MSC_VER < 1400 // VC 2003 needs these defines, VC 2005 will error with them included
   #define atan2f atan2
   #define acosf acos
   #define ceilf ceil
   #define floorf floor
   #define fabsf fabs
  #endif  // _MSC_VER < 1400

  #define rint(X) floor(X+0.5f)
 #endif // _MSC_VER

 #ifdef __MINGW32__
  // Lachesis: Make sure snprintf is declared before we #define it to be something else,
  // else we'll eventually break C++ headers that use it
  #include <stdio.h>

  #define snprintf safe_snprintf
 #endif // __MINGW32__
#elif defined (OSX)
  #ifndef __MACOSX__
   #define __MACOSX__  //necessary for Ogg on Macs
  #endif

 #ifdef __BIG_ENDIAN__
  #define EL_BIG_ENDIAN
 #endif
#endif //WINDOWS

#ifdef OSX
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 //#include <OpenGL/glext.h>
 #include "elglext.h"
 #define APIENTRY 
 #define APIENTRYP *
#elif !defined(BSD)
 #define GL_GLEXT_LEGACY
 #include <GL/gl.h>
 #include <GL/glu.h>
 #include "glext.h"
#else // BSD
 #include <GL/gl.h>
 #include <GL/glu.h>
#endif

// Inlucde the plaform specific location sound libs
#ifdef OSX
	#include <Carbon/Carbon.h>
	#include <AudioToolbox/AudioToolbox.h>
	#include <AudioUnit/AudioUnit.h>

	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>
	#include <OpenAL/MacOSX_OALExtensions.h>
#else
	#include <AL/al.h>
	#include <AL/alc.h>
#endif //lib location platform checking

#include <math.h>
#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif //M_PI
#ifndef M_SQRT2
 #define M_SQRT2 1.41421356237309504880
#endif

#ifdef __GNUC__
#define UNUSED(x) x __attribute__((unused))
#else
#define UNUSED(x) x
#endif // __GNUC__

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
		Uint32 i;
	} intOrFloat;

	intOrFloat.f = t;
	intOrFloat.i = SDL_Swap32 (intOrFloat.i);
	return intOrFloat.f;
}

#ifdef _MSC_VER
#include <math.h>
static __inline__ double trunc(const double d)
{
    return (d < 0 ? ceil(d) : floor(d));  
}
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // PLATFORM_H

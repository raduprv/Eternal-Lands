/*!
 * \file
 * \ingroup     platform
 * \brief       Various defines and constants to make EL compile on different platforms.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

// only ever use WINDOWS anywhere else, in case we need to add another 'catch' to 
// enable WINDOWS
#if defined (_WIN32) || defined (_WIN64)
 #ifndef WINDOWS
  #define WINDOWS
 #endif  // !WINDOWS
#endif  // _WIN32 || _WIN64

#ifdef WINDOWS
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
 #ifdef OGG_VORBIS
  #ifndef __MACOSX__
   #define __MACOSX__  //necessary for Ogg on Macs
  #endif
 #endif

 #ifdef __BIG_ENDIAN__
  #define EL_BIG_ENDIAN
 #endif
#endif //WINDOWS

#ifdef OSX
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 #include <OpenGL/glext.h>
#else
 #include <GL/gl.h>
 #include <GL/glu.h>
 #include <GL/glext.h>
#endif

// Inlucde the plaform specific location sound libs
#ifdef WINDOWS //lib location platform checking
	#include <al.h>
	#include <alc.h>
	#include <alut.h>
#elif defined(OSX)
	#include <alut.h>		//oddity as of Xcode 2.4
	#include <OpenAL/alc.h>
#else
	#include <AL/al.h>
	#include <AL/alc.h>
	#include <AL/alut.h>
#endif //lib location platform checking

#include <math.h>
#ifndef M_PI
 #define M_PI 3.14159265358979323846
#endif //M_PI

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
